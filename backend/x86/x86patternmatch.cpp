#include "common/debug.h"
#include "x86patternmatch.h"
#include "x86codegen.h"

char X86PatternMatch::ID = 0;
static llvm::RegisterPass<X86PatternMatch>
MPM("X86PatternMatch", "Pattern Matching for X86", true, true);

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

void X86PatternMatch::markAsUsed(llvm::Instruction *pI)
{
    m_usedInstructions.insert(pI);
}

bool X86PatternMatch::needInstruction(llvm::Instruction *pI)
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

void X86PatternMatch::setPatternRoot(llvm::Instruction *pI)
{
    m_pRoot = pI;
}

bool X86PatternMatch::runOnModule(llvm::Module &M)
{
    for (llvm::Module::iterator fb = M.begin(), fe = M.end(); fb != fe; ++fb)
    {
        llvm::DominatorTree &DT = getAnalysis<llvm::DominatorTreeWrapperPass>(*fb).getDomTree();
        createBasicBlocks(fb);
        codeGenNode(DT.getRootNode());
    }

    return false;
}

void X86PatternMatch::visitBranchInstruction(llvm::BranchInst &BI)
{
    ICARUS_NOT_IMPLEMENTED("Branch Instruction pattern match not implemented")
}

void X86PatternMatch::codeGenBlock(llvm::BasicBlock *pBB)
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

void X86PatternMatch::codeGenNode(llvm::DomTreeNode *pDomNode)
{
    unsigned int numChildren = pDomNode->getNumChildren();
    for (unsigned int i = 0; i < numChildren; ++i)
    {
        codeGenNode(pDomNode->getChildren()[i]);
    }

    llvm::BasicBlock *pBB = pDomNode->getBlock();
    codeGenBlock(pBB);
}

void X86PatternMatch::createBasicBlocks(llvm::Function *pFunc)
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

Pattern* X86PatternMatch::match(llvm::Instruction *pI)
{
    m_pCurrentPattern = nullptr;
    visit(pI);
    return m_pCurrentPattern;
}

bool X86PatternMatch::matchSingleInstruction(llvm::Instruction* pI)
{
    struct SingleInstPattern : public Pattern
    {
        llvm::Instruction *pInst;
        virtual void emit(X86CodeGen *pCodeGen)
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

bool X86PatternMatch::matchBrWithCmpInstruction(llvm::Instruction *pI)
{
    struct BrWithCmpInstPattern : public Pattern
    {
        llvm::BranchInst *pBrInst;
        llvm::CmpInst *pCmpInst;

        virtual void emit(X86CodeGen *pCodeGen)
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

void X86PatternMatch::visitReturnInst(llvm::ReturnInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitBranchInst(llvm::BranchInst &I)
{
    bool match = matchBrWithCmpInstruction(&I) ||
                 matchSingleInstruction(&I);
}

void X86PatternMatch::visitSwitchInst(llvm::SwitchInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitIndirectBrInst(llvm::IndirectBrInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitResumeInst(llvm::ResumeInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitUnreachableInst(llvm::UnreachableInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitICmpInst(llvm::ICmpInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitFCmpInst(llvm::FCmpInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitAllocaInst(llvm::AllocaInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitLoadInst(llvm::LoadInst     &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitStoreInst(llvm::StoreInst   &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitAtomicCmpXchgInst(llvm::AtomicCmpXchgInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitAtomicRMWInst(llvm::AtomicRMWInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitFenceInst(llvm::FenceInst   &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitGetElementPtrInst(llvm::GetElementPtrInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitPHINode(llvm::PHINode &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitTruncInst(llvm::TruncInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitZExtInst(llvm::ZExtInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitSExtInst(llvm::SExtInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitFPTruncInst(llvm::FPTruncInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitFPExtInst(llvm::FPExtInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitFPToUIInst(llvm::FPToUIInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitFPToSIInst(llvm::FPToSIInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitUIToFPInst(llvm::UIToFPInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitSIToFPInst(llvm::SIToFPInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitPtrToIntInst(llvm::PtrToIntInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitIntToPtrInst(llvm::IntToPtrInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitBitCastInst(llvm::BitCastInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitAddrSpaceCastInst(llvm::AddrSpaceCastInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitSelectInst(llvm::SelectInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitVAArgInst(llvm::VAArgInst   &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitExtractElementInst(llvm::ExtractElementInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitInsertElementInst(llvm::InsertElementInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitShuffleVectorInst(llvm::ShuffleVectorInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitExtractValueInst(llvm::ExtractValueInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitInsertValueInst(llvm::InsertValueInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitLandingPadInst(llvm::LandingPadInst &I)
{
    matchSingleInstruction(&I);
}


// Handle the special intrinsic instruction classes.
void X86PatternMatch::visitDbgDeclareInst(llvm::DbgDeclareInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitDbgValueInst(llvm::DbgValueInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitDbgInfoIntrinsic(llvm::DbgInfoIntrinsic &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitMemSetInst(llvm::MemSetInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitMemCpyInst(llvm::MemCpyInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitMemMoveInst(llvm::MemMoveInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitMemTransferInst(llvm::MemTransferInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitMemIntrinsic(llvm::MemIntrinsic &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitVAStartInst(llvm::VAStartInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitVAEndInst(llvm::VAEndInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitVACopyInst(llvm::VACopyInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitIntrinsicInst(llvm::IntrinsicInst &I)
{
    matchSingleInstruction(&I);
}


// Call and Invoke are slightly different as they delegate first through
// a generic CallSite visitor.
void X86PatternMatch::visitCallInst(llvm::CallInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitInvokeInst(llvm::InvokeInst &I)
{
    matchSingleInstruction(&I);
}


// Next level propagators: If the user does not overload a specific
// instruction type, they can overload one of these to get the whole class
// of instructions...
//
void X86PatternMatch::visitCastInst(llvm::CastInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitBinaryOperator(llvm::BinaryOperator &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitCmpInst(llvm::CmpInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitTerminatorInst(llvm::TerminatorInst &I)
{
    matchSingleInstruction(&I);
}

void X86PatternMatch::visitUnaryInstruction(llvm::UnaryInstruction &I)
{
    matchSingleInstruction(&I);
}

