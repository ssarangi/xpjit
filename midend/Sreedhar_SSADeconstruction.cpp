#include "Sreedhar_SSADeconstruction.h"

#include "common/debug.h"

char Sreedhar_SSADeconstructionPass::ID = 0;

// register the printCode class: 
//  - give it a command-line argument
//  - a name
//  - a flag saying that we don't modify the CFG
//  - a flag saying this is not an analysis pass
llvm::RegisterPass<Sreedhar_SSADeconstructionPass> T("Sreedhar_SSADeconstructionPass", "Deconstruction of SSA Pass by Sreedhar", false, false);

bool operator<(const InstructionSetTy& d1, const InstructionSetTy& d2)
{
    return d1.size() < d2.size();
}

Sreedhar_SSADeconstructionPass *createSreedhar_SSADeconstructionPass()
{
    Sreedhar_SSADeconstructionPass *pPass = new Sreedhar_SSADeconstructionPass();
    return pPass;
}

bool Sreedhar_SSADeconstructionPass::intersectPhiCongruenceClassAndLiveOut(llvm::Instruction *pI, llvm::BasicBlock *pBB)
{
    EdgeLivenessPass &EL = getAnalysis<EdgeLivenessPass>();

    bool doIntersect = false;
    // find all the congruence classes with these variables.
    for (llvm::Instruction *pYi : m_phiCongruenceClass[pI])
    {
        doIntersect |= EL.isLiveOut(pYi, pBB);
    }

    return doIntersect;
}

void Sreedhar_SSADeconstructionPass::determineCopiesNeeded(llvm::Instruction *pI, llvm::Instruction *pJ)
{
    // Bj is the basic block where pJ is present.
    // Bi is the basic block where pI is present.
    bool intersect_pI_Bj = intersectPhiCongruenceClassAndLiveOut(pI, pJ->getParent());
    bool intersect_pJ_Bi = intersectPhiCongruenceClassAndLiveOut(pJ, pI->getParent());

    /*
    Case 1. The intersection of phiCongruenceClass[xi] and LiveOut[Lj] is not
    empty, and the intersection of phiCongruenceClass[xj] and LiveOut[Li] is empty.
    A new copy, xi’=xi, is needed in Li to ensure that xi and xj are put in different
    phi congruence classes. So xi is added to candidateResourceSet.
    */
    if (intersect_pI_Bj && !intersect_pJ_Bi)
    {
        m_candidateResourceSet.insert(pI);
    }
    /*
    Case 2. The intersection of phiCongruenceClass[xi] and LiveOut[Lj] is empty,
    and the intersection of phiCongruenceClass[xj] and LiveOut[Li] is not empty. A
    new copy, xj’=xj, is needed in Lj to ensure that xi and xj are put in different phi
    congruence classes. So xj is added to candidateResourceSet.
    */
    else if (!intersect_pI_Bj && intersect_pJ_Bi)
    {
        m_candidateResourceSet.insert(pJ);
    }
    /*
    Case 3. The intersection of phiCongruenceClass[xi] and LiveOut[Lj] is not
    empty, and the intersection of phiCongruenceClass[xj] and LiveOut[Li] is not
    empty. Two new copies, xi’=xi in Li and xj’=xj in Lj, are needed to ensure that xi
    and xj are put in different phi congruence classes. So xi and xj are added to
    candidateResourceSet.
    */
    else if (intersect_pI_Bj && intersect_pJ_Bi)
    {
        m_candidateResourceSet.insert(pI);
        m_candidateResourceSet.insert(pJ);
    }
    /*
    Case 4. The intersection of phiCongruenceClass[xi] and LiveOut[Lj] is empty,
    and the intersection of phiCongruenceClass[xj] and LiveOut[Li] is empty. Either
    a copy, xi’=xi in Li, or a copy, xj’=xj in Lj, is sufficient to eliminate the
    interference between xi and xj. However, the final decision of which copy to
    insert is deferred until all pairs of interfering resources in the phi instruction are
    processed.
    */
    else if (!intersect_pI_Bj && !intersect_pJ_Bi)
    {
        m_unresolvedNeighborMap[pI].insert(pJ);
        m_unresolvedNeighborMap[pJ].insert(pI);
    }
}

bool Sreedhar_SSADeconstructionPass::areUnresolvedInstructionsPresent(llvm::Instruction *pI)
{
    for (auto inst : m_unresolvedNeighborMap[pI])
    {
        if (m_resolved.find(inst) != m_resolved.end())
            return true;
    }

    return false;
}

void Sreedhar_SSADeconstructionPass::trimCandidateResourceSet()
{
    // If all the neighbors of this variable have been marked as resolved, then remove it from
    // candidate resource set.
    for (auto iter : m_resolved)
    {
        if (!areUnresolvedInstructionsPresent(iter))
        {
            m_candidateResourceSet.erase(iter);
            ++iter;
        }
    }
}

void Sreedhar_SSADeconstructionPass::processUnresolvedNeighbors()
{
    std::vector<InstToInstSet> instSetList;
    for (auto iter : m_unresolvedNeighborMap)
    {
        InstToInstSet tset;
        tset.pI = iter.first;
        tset.instSet = iter.second;

        instSetList.push_back(tset);
    }

    std::sort(instSetList.begin(), instSetList.end(), [](const InstToInstSet& a, const InstToInstSet& b){ return a.instSet.size() < b.instSet.size(); });

    for (auto instSet : instSetList)
    {
        if (areUnresolvedInstructionsPresent(instSet.pI))
        {
            m_candidateResourceSet.insert(instSet.pI);
            m_resolved.insert(instSet.pI);
        }
    }
}

llvm::Instruction* Sreedhar_SSADeconstructionPass::insertCopy(llvm::Instruction *pI, llvm::Instruction *pLocationBefore)
{
    llvm::IRBuilder<> builder(pI->getContext());

    builder.SetInsertPoint(pLocationBefore);

    // Assume that the builder has set the right insertion point
    llvm::Value *pC = nullptr;

    llvm::Type *pType = pI->getType();

    if (pI->getType()->isPointerTy())
        assert(0 && "Pointer types are not handled");

    llvm::Instruction *pBitcast = llvm::cast<llvm::Instruction>(builder.CreateBitCast(pI, pType, pI->getName() + "_dash"));
    return pBitcast;
}

void Sreedhar_SSADeconstructionPass::insertCopies()
{
    EdgeLivenessPass &EL = getAnalysis<EdgeLivenessPass>();

    for (llvm::Instruction *pI : m_candidateResourceSet)
    {
        if (llvm::isa<llvm::PHINode>(pI))
        {
            // Insert a copy instruction at the beginning of the basic block.
            llvm::Instruction *pCopy = insertCopy(pI, pI);

            // Replace the phi with the copy instruction as the target
            m_phiCongruenceClass[pCopy].insert(pCopy);
        }
        else
        {
            // This instruction is an operand of the phi instruction
            // Get Terminator of BB defining pI.
            llvm::BasicBlock *pDefBlock = pI->getParent();
            llvm::Instruction *pTerminator = pDefBlock->getTerminator();
            llvm::Instruction *pCopy = insertCopy(pI, pTerminator);
            m_phiCongruenceClass[pI].insert(pCopy);

            // TODO: Update the Live Out information accordingly.
            for (llvm::User::op_iterator opi = pI->op_begin(), ope = pI->op_end(); opi != ope; ++opi)
            {
                if (llvm::isa<llvm::PHINode>(opi))
                {
                    EL.kill(pDefBlock, llvm::cast<llvm::PHINode>(opi)->getParent(), pI);
                    EL.setAlive(pDefBlock, llvm::cast<llvm::PHINode>(opi)->getParent(), pCopy);

                    // build interferences edges between copy and LineIn[L0]
                }
            }
        }
    }
}

void Sreedhar_SSADeconstructionPass::createCandidateResourceSet(llvm::Function &F)
{
    for (llvm::Function::iterator bb : F)
    {
        for (llvm::BasicBlock::iterator ii : *bb)
        {
            if (llvm::PHINode *pPhi = llvm::dyn_cast<llvm::PHINode>(ii))
            {
                // Initialize phi congruence class such that each resource in phi
                // belongs to its own congruence class
                llvm::User::op_iterator ope = pPhi->op_end();
                for (llvm::User::op_iterator opi = pPhi->op_begin();
                    opi != ope;
                    ++opi)
                {
                    // assert(!llvm::isa<llvm::Constant>(opi) && "Handle constants properly");
                    if (llvm::Instruction *pOpi = llvm::cast<llvm::Instruction>(opi))
                    {
                        for (llvm::User::op_iterator opj = pPhi->op_begin();
                            opj != ope;
                            ++opj)
                        {
                            // assert(!llvm::isa<llvm::Constant>(opj) && "Handle constants properly");
                            if (llvm::Instruction *pOpj = llvm::cast<llvm::Instruction>(opj))
                                determineCopiesNeeded(pOpi, pOpj);
                        }
                    }
                }

                processUnresolvedNeighbors();
                m_unresolvedNeighborMap.clear();

                insertCopies();
                m_candidateResourceSet.clear();

                std::set<llvm::Instruction*> currentPhiCongruenceClass;

                for (llvm::User::op_iterator opi = pPhi->op_begin();
                    opi != ope;
                    ++opi)
                {
                    llvm::Instruction *pI = llvm::cast<llvm::Instruction>(opi);
                    // std::copy(m_phiCongruenceClass[pI].begin(), m_phiCongruenceClass[pI].end(), currentPhiCongruenceClass.begin());
                }

                // Once all of these have been copied, copy back all elements back to the original classes
                for (llvm::User::op_iterator opi = pPhi->op_begin();
                    opi != ope;
                    ++opi)
                {
                    llvm::Instruction *pI = llvm::cast<llvm::Instruction>(opi);
                    m_phiCongruenceClass[pI].clear();
                    // std::copy(currentPhiCongruenceClass.begin(), currentPhiCongruenceClass.end(), m_phiCongruenceClass[pI].begin());
                }
            }
        }
    }

    // Determine which instructions have all their neighbors resolved. If any of them have all their
    // neighbors resolved, then remove them from candidateResourceSet.
    trimCandidateResourceSet();
}

void Sreedhar_SSADeconstructionPass::buildInitialPhiCongruenceClass(llvm::Function &F)
{
    for (llvm::Function::iterator bb : F)
    {
        for (llvm::BasicBlock::iterator ii : *bb)
        {
            if (llvm::PHINode *pPhi = llvm::dyn_cast<llvm::PHINode>(ii))
            {
                // Initialize phi congruence class such that each resource in phi
                // belongs to its own congruence class
                for (llvm::User::op_iterator opi = pPhi->op_begin(), ope = pPhi->op_end();
                    opi != ope;
                    ++opi)
                {
                    // assert(!llvm::isa<llvm::Constant>(opi) && "Handle constants properly");
                    if (llvm::Instruction *pI = llvm::dyn_cast<llvm::Instruction>(opi))
                    {
                        m_phiCongruenceClass[pI].insert(pI);
                    }
                }
            }
        }
    }
}

bool Sreedhar_SSADeconstructionPass::runOnFunction(llvm::Function &F)
{
    ADD_HEADER("SSA Deconstruction Pass");
    
    buildInitialPhiCongruenceClass(F);
    createCandidateResourceSet(F);

    return true;
}
