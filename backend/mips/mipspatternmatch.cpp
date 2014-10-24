#include "common/debug.h"
#include "mipspatternmatch.h"
#include "mipscodegen.h"

char MipsPatternMatch::ID = 0;
static llvm::RegisterPass<MipsPatternMatch>
MPM("MIPSPatternMatch", "Pattern Matching for MIPS", true, true);

bool instHasPhiUse(llvm::Instruction *pI)
{
    for (auto UI = pI->use_begin(), UE = pI->use_end(); UI != UE; ++UI)
    {
        
        if (llvm::isa<llvm::PHINode>(llvm::cast<llvm::Instruction>(*UI)))
        {
            return true;
        }
    }

    return false;
}

bool instHasSideEffects(llvm::Instruction *pI)
{
    if (pI->mayWriteToMemory() || llvm::isa<llvm::TerminatorInst>(pI))
        return true;

    return false;
}

bool instIsDbgInst(llvm::Instruction *pI)
{
    if (llvm::isa<llvm::DbgInfoIntrinsic>(pI))
        return true;

    return false;
}

void MipsPatternMatch::markAsUsed(llvm::Instruction *pI)
{
    m_usedInstructions.insert(pI);
}

bool MipsPatternMatch::needInstruction(llvm::Instruction *pI)
{
    /*
    if (instHasPhiUse(pI) || instHasSideEffects(pI) || instIsDbgInst(pI) ||
    (m_usedInstructions.find(pI) != m_usedInstructions.end()))
    return true;

    return false;
    */

    if (m_usedInstructions.find(pI) == m_usedInstructions.end())
        return true;

    return false;
}

void MipsPatternMatch::setPatternRoot(llvm::Instruction *pI)
{
    m_pRoot = pI;
}

bool MipsPatternMatch::runOnModule(llvm::Module &M)
{
    for (llvm::Module::iterator fb = M.begin(), fe = M.end(); fb != fe; ++fb)
    {
        llvm::DominatorTree &DT = getAnalysis<llvm::DominatorTreeWrapperPass>(*fb).getDomTree();
        createBasicBlocks(fb);
        codeGenNode(DT.getRootNode());
    }

    return false;
}

void MipsPatternMatch::visitBranchInstruction(llvm::BranchInst &BI)
{
    ICARUS_NOT_IMPLEMENTED("Branch Instruction pattern match not implemented")
}

void MipsPatternMatch::codeGenBlock(llvm::BasicBlock *pBB)
{
    llvm::BasicBlock::InstListType &instructionList = pBB->getInstList();
    llvm::BasicBlock::InstListType::reverse_iterator i, e;

    auto it = m_funcBlocks[pBB->getParent()]->m_blockMap.find(pBB);
    assert(it != m_funcBlocks[pBB->getParent()]->m_blockMap.end());

    PM_BasicBlock *pBlock = it->second;

    // Loop through the instructions bottom up
    for (i = instructionList.rbegin(), e = instructionList.rend(); i != e; ++i)
    {
        llvm::Instruction& inst = (*i);

        if (needInstruction(&inst))
        {
            setPatternRoot(&inst);
            Pattern *pPattern = match(&inst);
            if (pPattern)
                pBlock->m_dags.push_back(PM_SDAG(pPattern, m_pRoot));
        }
    }
}

void MipsPatternMatch::codeGenNode(llvm::DomTreeNode *pDomNode)
{
    unsigned int numChildren = pDomNode->getNumChildren();
    for (unsigned int i = 0; i < numChildren; ++i)
    {
        codeGenNode(pDomNode->getChildren()[i]);
    }

    llvm::BasicBlock *pBB = pDomNode->getBlock();
    codeGenBlock(pBB);
}

void MipsPatternMatch::createBasicBlocks(llvm::Function *pFunc)
{
    PM_FuncBasicBlock *pFuncBasicBlocks = new PM_FuncBasicBlock();

    int i = 0;
    for (llvm::BasicBlock& bb : *pFunc)
    {
        PM_BasicBlock *pBasicBlock = new PM_BasicBlock();
        pFuncBasicBlocks->addBlock(i, &bb, pBasicBlock);
        ++i;
    }

    m_funcBlocks[pFunc] = pFuncBasicBlocks;
}

Pattern* MipsPatternMatch::match(llvm::Instruction *pI)
{
    m_pCurrentPattern = nullptr;
    visit(pI);
    return m_pCurrentPattern;
}

bool MipsPatternMatch::matchSingleInstruction(llvm::Instruction* pI)
{
    struct SingleInstPattern : public Pattern
    {
        llvm::Instruction *pInst;
        virtual void emit(MipsCodeGen *pCodeGen)
        {
            pCodeGen->visit(pInst);
        }
    };

    SingleInstPattern *pPattern = new SingleInstPattern();
    pPattern->pInst = pI;

    markAsUsed(pI);
    addPattern(pPattern);

    return true;
}

bool MipsPatternMatch::matchBrWithCmpInstruction(llvm::Instruction *pI)
{
    struct BrWithCmpInstPattern : public Pattern
    {
        llvm::BranchInst *pBrInst;
        llvm::CmpInst *pCmpInst;

        virtual void emit(MipsCodeGen *pCodeGen)
        {
            pCodeGen->emitBrWithCmpInstruction(pBrInst, pCmpInst);
        }
    };

    llvm::BranchInst *pBrInst = llvm::cast<llvm::BranchInst>(pI);
    if (pBrInst->isUnconditional())
        return false;

    llvm::Value* pCond = pBrInst->getCondition();

    llvm::CmpInst *pCmpInst = nullptr;
    bool match = false;

    if (pCmpInst = llvm::cast<llvm::ICmpInst>(pCond))
    {
        match = true;
    }

    if (match)
    {
        markAsUsed(llvm::cast<llvm::Instruction>(pCond));

        BrWithCmpInstPattern *pBrWithCmpInstPattern = new BrWithCmpInstPattern();
        pBrWithCmpInstPattern->pBrInst = pBrInst;
        pBrWithCmpInstPattern->pCmpInst = pCmpInst;

        addPattern(pBrWithCmpInstPattern);
        return true;
    }
    
    return false;
}

void MipsPatternMatch::visitReturnInst(llvm::ReturnInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitBranchInst(llvm::BranchInst &I)
{
    bool match = matchBrWithCmpInstruction(&I) ||
                 matchSingleInstruction(&I);
}

void MipsPatternMatch::visitSwitchInst(llvm::SwitchInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitIndirectBrInst(llvm::IndirectBrInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitResumeInst(llvm::ResumeInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitUnreachableInst(llvm::UnreachableInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitICmpInst(llvm::ICmpInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitFCmpInst(llvm::FCmpInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitAllocaInst(llvm::AllocaInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitLoadInst(llvm::LoadInst     &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitStoreInst(llvm::StoreInst   &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitAtomicCmpXchgInst(llvm::AtomicCmpXchgInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitAtomicRMWInst(llvm::AtomicRMWInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitFenceInst(llvm::FenceInst   &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitGetElementPtrInst(llvm::GetElementPtrInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitPHINode(llvm::PHINode &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitTruncInst(llvm::TruncInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitZExtInst(llvm::ZExtInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitSExtInst(llvm::SExtInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitFPTruncInst(llvm::FPTruncInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitFPExtInst(llvm::FPExtInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitFPToUIInst(llvm::FPToUIInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitFPToSIInst(llvm::FPToSIInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitUIToFPInst(llvm::UIToFPInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitSIToFPInst(llvm::SIToFPInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitPtrToIntInst(llvm::PtrToIntInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitIntToPtrInst(llvm::IntToPtrInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitBitCastInst(llvm::BitCastInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitAddrSpaceCastInst(llvm::AddrSpaceCastInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitSelectInst(llvm::SelectInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitVAArgInst(llvm::VAArgInst   &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitExtractElementInst(llvm::ExtractElementInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitInsertElementInst(llvm::InsertElementInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitShuffleVectorInst(llvm::ShuffleVectorInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitExtractValueInst(llvm::ExtractValueInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitInsertValueInst(llvm::InsertValueInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitLandingPadInst(llvm::LandingPadInst &I)
{
    matchSingleInstruction(&I);
}


// Handle the special intrinsic instruction classes.
void MipsPatternMatch::visitDbgDeclareInst(llvm::DbgDeclareInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitDbgValueInst(llvm::DbgValueInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitDbgInfoIntrinsic(llvm::DbgInfoIntrinsic &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitMemSetInst(llvm::MemSetInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitMemCpyInst(llvm::MemCpyInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitMemMoveInst(llvm::MemMoveInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitMemTransferInst(llvm::MemTransferInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitMemIntrinsic(llvm::MemIntrinsic &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitVAStartInst(llvm::VAStartInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitVAEndInst(llvm::VAEndInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitVACopyInst(llvm::VACopyInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitIntrinsicInst(llvm::IntrinsicInst &I)
{
    matchSingleInstruction(&I);
}


// Call and Invoke are slightly different as they delegate first through
// a generic CallSite visitor.
void MipsPatternMatch::visitCallInst(llvm::CallInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitInvokeInst(llvm::InvokeInst &I)
{
    matchSingleInstruction(&I);
}


// Next level propagators: If the user does not overload a specific
// instruction type, they can overload one of these to get the whole class
// of instructions...
//
void MipsPatternMatch::visitCastInst(llvm::CastInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitBinaryOperator(llvm::BinaryOperator &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitCmpInst(llvm::CmpInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitTerminatorInst(llvm::TerminatorInst &I)
{
    matchSingleInstruction(&I);
}

void MipsPatternMatch::visitUnaryInstruction(llvm::UnaryInstruction &I)
{
    matchSingleInstruction(&I);
}

