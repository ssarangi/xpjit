#include "common/debug.h"
#include "asmjit_patternmatch.h"
#include "asmjit_codegen.h"

char AsmJitPatternMatch::ID = 0;
static llvm::RegisterPass<AsmJitPatternMatch>
MPM("AsmJitPatternMatch", "Pattern Matching for X86", true, true);

void AsmJitPatternMatch::markAsUsed(llvm::Instruction *pI)
{
    m_usedInstructions.insert(pI);
}

bool AsmJitPatternMatch::needInstruction(llvm::Instruction *pI)
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

void AsmJitPatternMatch::setPatternRoot(llvm::Instruction *pI)
{
    m_pRoot = pI;
}

bool AsmJitPatternMatch::runOnModule(llvm::Module &M)
{
    for (llvm::Module::iterator fb = M.begin(), fe = M.end(); fb != fe; ++fb)
    {
        if (!fb->isDeclaration())
        {
            llvm::DominatorTree &DT = getAnalysis<llvm::DominatorTreeWrapperPass>(*fb).getDomTree();
            createBasicBlocks(fb);
            codeGenNode(DT.getRootNode());
        }
    }

    return false;
}

void AsmJitPatternMatch::visitBranchInstruction(llvm::BranchInst &BI)
{
    ICARUS_NOT_IMPLEMENTED("Branch Instruction pattern match not implemented")
}

void AsmJitPatternMatch::codeGenBlock(llvm::BasicBlock *pBB)
{
    llvm::BasicBlock::InstListType &instructionList = pBB->getInstList();
    llvm::BasicBlock::InstListType::reverse_iterator i, e;

    auto it = m_funcBlocks[pBB->getParent()]->m_blockMap.find(pBB);
    assert(it != m_funcBlocks[pBB->getParent()]->m_blockMap.end());

    AsmJit_PM_BasicBlock *pBlock = it->second;

    // Loop through the instructions bottom up
    for (i = instructionList.rbegin(), e = instructionList.rend(); i != e; ++i)
    {
        llvm::Instruction& inst = (*i);

        if (needInstruction(&inst))
        {
            setPatternRoot(&inst);
            AsmJitPattern *pPattern = match(&inst);
            if (pPattern)
                pBlock->m_dags.push_back(x86_PM_SDAG(pPattern, m_pRoot));
        }
    }
}

void AsmJitPatternMatch::codeGenNode(llvm::DomTreeNode *pDomNode)
{
    unsigned int numChildren = pDomNode->getNumChildren();
    for (unsigned int i = 0; i < numChildren; ++i)
    {
        codeGenNode(pDomNode->getChildren()[i]);
    }

    llvm::BasicBlock *pBB = pDomNode->getBlock();
    codeGenBlock(pBB);
}

void AsmJitPatternMatch::createBasicBlocks(llvm::Function *pFunc)
{
    x86_PM_FuncBasicBlock *pFuncBasicBlocks = new x86_PM_FuncBasicBlock();

    int i = 0;
    for (llvm::BasicBlock& bb : *pFunc)
    {
        AsmJit_PM_BasicBlock *pBasicBlock = new AsmJit_PM_BasicBlock();
        pFuncBasicBlocks->addBlock(i, &bb, pBasicBlock);
        ++i;
    }

    m_funcBlocks[pFunc] = pFuncBasicBlocks;
}

AsmJitPattern* AsmJitPatternMatch::match(llvm::Instruction *pI)
{
    m_pCurrentPattern = nullptr;
    visit(pI);
    return m_pCurrentPattern;
}

bool AsmJitPatternMatch::matchSingleInstruction(llvm::Instruction* pI)
{
    struct SingleInstPattern : public AsmJitPattern
    {
        llvm::Instruction *pInst;
        virtual void emit(AsmJitCodeGen *pCodeGen)
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

bool AsmJitPatternMatch::matchBrWithCmpInstruction(llvm::Instruction *pI)
{
    struct BrWithCmpInstPattern : public AsmJitPattern
    {
        llvm::BranchInst *pBrInst;
        llvm::CmpInst *pCmpInst;

        virtual void emit(AsmJitCodeGen *pCodeGen)
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

void AsmJitPatternMatch::visitReturnInst(llvm::ReturnInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitBranchInst(llvm::BranchInst &I)
{
    bool match = matchBrWithCmpInstruction(&I) ||
                 matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitSwitchInst(llvm::SwitchInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitIndirectBrInst(llvm::IndirectBrInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitResumeInst(llvm::ResumeInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitUnreachableInst(llvm::UnreachableInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitICmpInst(llvm::ICmpInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitFCmpInst(llvm::FCmpInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitAllocaInst(llvm::AllocaInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitLoadInst(llvm::LoadInst     &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitStoreInst(llvm::StoreInst   &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitAtomicCmpXchgInst(llvm::AtomicCmpXchgInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitAtomicRMWInst(llvm::AtomicRMWInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitFenceInst(llvm::FenceInst   &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitGetElementPtrInst(llvm::GetElementPtrInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitPHINode(llvm::PHINode &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitTruncInst(llvm::TruncInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitZExtInst(llvm::ZExtInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitSExtInst(llvm::SExtInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitFPTruncInst(llvm::FPTruncInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitFPExtInst(llvm::FPExtInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitFPToUIInst(llvm::FPToUIInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitFPToSIInst(llvm::FPToSIInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitUIToFPInst(llvm::UIToFPInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitSIToFPInst(llvm::SIToFPInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitPtrToIntInst(llvm::PtrToIntInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitIntToPtrInst(llvm::IntToPtrInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitBitCastInst(llvm::BitCastInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitAddrSpaceCastInst(llvm::AddrSpaceCastInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitSelectInst(llvm::SelectInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitVAArgInst(llvm::VAArgInst   &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitExtractElementInst(llvm::ExtractElementInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitInsertElementInst(llvm::InsertElementInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitShuffleVectorInst(llvm::ShuffleVectorInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitExtractValueInst(llvm::ExtractValueInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitInsertValueInst(llvm::InsertValueInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitLandingPadInst(llvm::LandingPadInst &I)
{
    matchSingleInstruction(&I);
}


// Handle the special intrinsic instruction classes.
void AsmJitPatternMatch::visitDbgDeclareInst(llvm::DbgDeclareInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitDbgValueInst(llvm::DbgValueInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitDbgInfoIntrinsic(llvm::DbgInfoIntrinsic &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitMemSetInst(llvm::MemSetInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitMemCpyInst(llvm::MemCpyInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitMemMoveInst(llvm::MemMoveInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitMemTransferInst(llvm::MemTransferInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitMemIntrinsic(llvm::MemIntrinsic &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitVAStartInst(llvm::VAStartInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitVAEndInst(llvm::VAEndInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitVACopyInst(llvm::VACopyInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitIntrinsicInst(llvm::IntrinsicInst &I)
{
    matchSingleInstruction(&I);
}


// Call and Invoke are slightly different as they delegate first through
// a generic CallSite visitor.
void AsmJitPatternMatch::visitCallInst(llvm::CallInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitInvokeInst(llvm::InvokeInst &I)
{
    matchSingleInstruction(&I);
}


// Next level propagators: If the user does not overload a specific
// instruction type, they can overload one of these to get the whole class
// of instructions...
//
void AsmJitPatternMatch::visitCastInst(llvm::CastInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitBinaryOperator(llvm::BinaryOperator &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitCmpInst(llvm::CmpInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitTerminatorInst(llvm::TerminatorInst &I)
{
    matchSingleInstruction(&I);
}

void AsmJitPatternMatch::visitUnaryInstruction(llvm::UnaryInstruction &I)
{
    matchSingleInstruction(&I);
}