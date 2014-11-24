#include "SSADeconstructionPass.h"
#include "common/debug.h"

char SSADeconstructionPass::ID = 0;

// register the printCode class: 
//  - give it a command-line argument
//  - a name
//  - a flag saying that we don't modify the CFG
//  - a flag saying this is not an analysis pass
llvm::RegisterPass<SSADeconstructionPass> T("SSADeconstructionPass", "Deconstruction of SSA Pass", false, false);

SSADeconstructionPass *createSSADeconstructionPass()
{
    SSADeconstructionPass *pPass = new SSADeconstructionPass();
    return pPass;
}

bool SSADeconstructionPass::runOnFunction(llvm::Function &F)
{
    ADD_HEADER("SSA Deconstruction Pass");
    m_pEdgeLiveness = &getAnalysis<EdgeLivenessPass>();

    convertToCSSA(F);

    g_outputStream << "CSSA Form\n";
    g_outputStream << "--------------\n";
    F.print(g_outputStream());
    g_outputStream.flush();

    return true;
}

// This function will move the IR out of SSA form. There is absolutely no
// coalescing occurring here.
void SSADeconstructionPass::renameVariables(llvm::PHINode *pPhi)
{
    // Remove the _dash from the name.
    std::string phiName = pPhi->getName().drop_back(5);

    // Now for every variable in the phi rename it with this new name
    for (llvm::User::op_iterator opi = pPhi->op_begin(), ope = pPhi->op_end();
        opi != ope;
        ++opi)
    {
        if (llvm::Instruction *pI = llvm::dyn_cast<llvm::Instruction>(opi))
        {
            pI->setName(phiName);
        }
    }

    // Now replace all the uses of the phi with this same
    for (llvm::Instruction::use_iterator useI = pPhi->use_begin(), useE = pPhi->use_end();
        useI != useE;
        ++useI)
    {
        llvm::Instruction *pUseI = llvm::cast<llvm::Instruction>(*useI);
        pUseI->setName(phiName);
    }

    // pPhi->eraseFromParent();
}

void SSADeconstructionPass::convertToCSSA(llvm::Function &F)
{
    for (llvm::Function::iterator bb = F.begin(), be = F.end();
        bb != be;
        ++bb)
    {
        llvm::Instruction *pInstToInsertMovBefore = nullptr;
        std::set<OldNewPhiNodePair> oldNewPhiNodesPair;

        for (llvm::BasicBlock::iterator i = bb->begin(), ie = bb->end();
            i != ie;
            ++i)
        {
            if (llvm::PHINode *pPhi = llvm::dyn_cast<llvm::PHINode>(i))
            {
                OldNewPhiNodePair phiNodePair = visitPhi(pPhi);
                oldNewPhiNodesPair.insert(phiNodePair);
            }
            else if (pInstToInsertMovBefore == nullptr)
                pInstToInsertMovBefore = i;
        }

        std::set<llvm::PHINode*> phiToRemove;
        // Iterate over all the phi instructions and add the copy next to it
        for (auto oldNewPair : oldNewPhiNodesPair)
        {
            llvm::PHINode *pOldPhi = oldNewPair.first;
            llvm::PHINode *pNewPhi = oldNewPair.second;
            phiToRemove.insert(pNewPhi);

            std::string newPhiName = pNewPhi->getName().substr(0, pNewPhi->getName().size() - std::string("_dash").length());
            llvm::Instruction *pBitcast = llvm::BitCastInst::Create(llvm::Instruction::CastOps::BitCast, pNewPhi, pNewPhi->getType(), newPhiName, pInstToInsertMovBefore);
            pOldPhi->replaceAllUsesWith(pBitcast);
            pOldPhi->removeFromParent();

            renameVariables(pNewPhi);
        }
    }

    F.print(g_outputStream());
    g_outputStream.flush();
}

OldNewPhiNodePair SSADeconstructionPass::visitPhi(llvm::PHINode *pPhi)
{
    llvm::IRBuilder<> builder(pPhi->getContext());

    // For each operand, insert a mov in the predecessor blocks and then use those in the phi
    llvm::PHINode *pNewPhi = llvm::PHINode::Create(pPhi->getType(), pPhi->getNumIncomingValues(), pPhi->getName() + "_dash", pPhi);

    for (llvm::User::op_iterator op = pPhi->op_begin(), ope = pPhi->op_end();
        op != ope;
        ++op)
    {
        llvm::BasicBlock *pPredBB = pPhi->getIncomingBlock(*op);
        llvm::TerminatorInst *pTerminator = pPredBB->getTerminator();

        // Now insert a new copy instruction here.
        llvm::Value *pC = nullptr;

        llvm::Value *pI = llvm::cast<llvm::Value>(op);
        llvm::Type *pType = pI->getType();

        llvm::Type *pIntTy = builder.getInt32Ty();
        llvm::Type *pFloatTy = builder.getFloatTy();
        llvm::Type *pDoubleTy = builder.getDoubleTy();

        if (pI->getType()->isPointerTy())
            assert(0 && "Pointer types are not handled");

        if (pType == pIntTy)
            pC = builder.getInt32(1);
        else if (pType == pFloatTy)
            pC = llvm::ConstantFP::get(builder.getFloatTy(), 1.0f);
        else
            assert(0 && "Other types not handled yet");

        pI = llvm::BitCastInst::Create(llvm::Instruction::CastOps::BitCast, pI, pType, pI->getName() + "_dash", pTerminator);
        // m_phiParameterPartitions.insert(pI);

        pNewPhi->addIncoming(pI, pPredBB);
    }

    return std::make_pair(pPhi, pNewPhi);
}