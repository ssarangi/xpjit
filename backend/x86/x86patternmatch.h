#ifndef __X86_PATTERN_MATCH__
#define __X86_PATTERN_MATCH__

#include <common/llvm_warnings_push.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Dominators.h>
#include <llvm/IR/InstVisitor.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/DenseSet.h>
#include <common/llvm_warnings_pop.h>

#include <vector>

class X86CodeGen;

struct x86Pattern
{
    virtual void emit(X86CodeGen* pCodeGen) = 0;
    virtual ~x86Pattern() {}
};

struct x86_PM_SDAG
{
    x86_PM_SDAG(x86Pattern* pattern, llvm::Instruction *pRoot) : m_pPattern(pattern), m_pRoot(pRoot)
    {

    }

    x86Pattern*           m_pPattern;
    llvm::Instruction*    m_pRoot;
};

struct x86_PM_BasicBlock
{
    unsigned int id;
    llvm::BasicBlock* pBB;
    std::vector<x86_PM_SDAG> m_dags;

    std::string getName() { return pBB->getName().str(); }
};

struct x86_PM_FuncBasicBlock
{
    x86_PM_FuncBasicBlock() { }
    ~x86_PM_FuncBasicBlock()
    {
        for (auto block : m_blocks)
        {
            delete block;
        }
    }

    std::vector<x86_PM_BasicBlock*> m_blocks;
    llvm::DenseMap<llvm::BasicBlock*, x86_PM_BasicBlock*> m_blockMap;

    llvm::Function *m_pFunction;

    void addBlock(unsigned int id, llvm::BasicBlock* pBB, x86_PM_BasicBlock* pPMBB)
    {
        pPMBB->id = id;
        pPMBB->pBB = pBB;
        m_blocks.push_back(pPMBB);
        m_blockMap[pBB] = pPMBB;
    }
};

class X86PatternMatch : public llvm::ModulePass, public llvm::InstVisitor<X86PatternMatch>
{
public:
    static char ID; // Pass identification, replacement for typeid
    X86PatternMatch()
        : llvm::ModulePass(ID)
        , m_pRoot(nullptr)
        , m_pCurrentPattern(nullptr)
    {}

    void getAnalysisUsage(llvm::AnalysisUsage &AU) const override
    {
        AU.setPreservesAll();
        AU.addRequired<llvm::DominatorTreeWrapperPass>();
    }

    bool runOnModule(llvm::Module &M) override;

    x86_PM_FuncBasicBlock* getFuncBlock(llvm::Function* pF)
    {
        assert(m_funcBlocks.find(pF) != m_funcBlocks.end());
        return m_funcBlocks[pF];
    }

    void visitBranchInstruction(llvm::BranchInst &BI);

private:
    void codeGenBlock(llvm::BasicBlock *pBB);
    void codeGenNode(llvm::DomTreeNode *pDomNode);
    void createBasicBlocks(llvm::Function *pFunc);
    bool needInstruction(llvm::Instruction *pI);
    void setPatternRoot(llvm::Instruction *pI);
    
    x86Pattern* match(llvm::Instruction *pI);
    bool matchSingleInstruction(llvm::Instruction *pI);
    bool matchBrWithCmpInstruction(llvm::Instruction *pI);
    void markAsUsed(llvm::Instruction *pI);
    
    void addPattern(x86Pattern *pPattern)
    {
        m_pCurrentPattern = pPattern;
        m_patterns.push_back(m_pCurrentPattern);
    }
    
public:
    // Visitor Functions
    void visitReturnInst(llvm::ReturnInst &I);
    void visitBranchInst(llvm::BranchInst &I);
    void visitSwitchInst(llvm::SwitchInst &I);
    void visitIndirectBrInst(llvm::IndirectBrInst &I);
    void visitResumeInst(llvm::ResumeInst &I);
    void visitUnreachableInst(llvm::UnreachableInst &I);
    void visitICmpInst(llvm::ICmpInst &I);
    void visitFCmpInst(llvm::FCmpInst &I);
    void visitAllocaInst(llvm::AllocaInst &I);
    void visitLoadInst(llvm::LoadInst     &I);
    void visitStoreInst(llvm::StoreInst   &I);
    void visitAtomicCmpXchgInst(llvm::AtomicCmpXchgInst &I);
    void visitAtomicRMWInst(llvm::AtomicRMWInst &I);
    void visitFenceInst(llvm::FenceInst   &I);
    void visitGetElementPtrInst(llvm::GetElementPtrInst &I);
    void visitPHINode(llvm::PHINode &I);
    void visitTruncInst(llvm::TruncInst &I);
    void visitZExtInst(llvm::ZExtInst &I);
    void visitSExtInst(llvm::SExtInst &I);
    void visitFPTruncInst(llvm::FPTruncInst &I);
    void visitFPExtInst(llvm::FPExtInst &I);
    void visitFPToUIInst(llvm::FPToUIInst &I);
    void visitFPToSIInst(llvm::FPToSIInst &I);
    void visitUIToFPInst(llvm::UIToFPInst &I);
    void visitSIToFPInst(llvm::SIToFPInst &I);
    void visitPtrToIntInst(llvm::PtrToIntInst &I);
    void visitIntToPtrInst(llvm::IntToPtrInst &I);
    void visitBitCastInst(llvm::BitCastInst &I);
    void visitAddrSpaceCastInst(llvm::AddrSpaceCastInst &I);
    void visitSelectInst(llvm::SelectInst &I);
    void visitVAArgInst(llvm::VAArgInst   &I);
    void visitExtractElementInst(llvm::ExtractElementInst &I);
    void visitInsertElementInst(llvm::InsertElementInst &I);
    void visitShuffleVectorInst(llvm::ShuffleVectorInst &I);
    void visitExtractValueInst(llvm::ExtractValueInst &I);
    void visitInsertValueInst(llvm::InsertValueInst &I);
    void visitLandingPadInst(llvm::LandingPadInst &I);

    // Handle the special intrinsic instruction classes.
    void visitDbgDeclareInst(llvm::DbgDeclareInst &I);
    void visitDbgValueInst(llvm::DbgValueInst &I);
    void visitDbgInfoIntrinsic(llvm::DbgInfoIntrinsic &I);
    void visitMemSetInst(llvm::MemSetInst &I);
    void visitMemCpyInst(llvm::MemCpyInst &I);
    void visitMemMoveInst(llvm::MemMoveInst &I);
    void visitMemTransferInst(llvm::MemTransferInst &I);
    void visitMemIntrinsic(llvm::MemIntrinsic &I);
    void visitVAStartInst(llvm::VAStartInst &I);
    void visitVAEndInst(llvm::VAEndInst &I);
    void visitVACopyInst(llvm::VACopyInst &I);
    void visitIntrinsicInst(llvm::IntrinsicInst &I);

    // Call and Invoke are slightly different as they delegate first through
    // a generic CallSite visitor.
    void visitCallInst(llvm::CallInst &I);
    void visitInvokeInst(llvm::InvokeInst &I);

    // Next level propagators: If the user does not overload a specific
    // instruction type, they can overload one of these to get the whole class
    // of instructions...
    //
    void visitCastInst(llvm::CastInst &I);
    void visitBinaryOperator(llvm::BinaryOperator &I);
    void visitCmpInst(llvm::CmpInst &I);
    void visitTerminatorInst(llvm::TerminatorInst &I);
    void visitUnaryInstruction(llvm::UnaryInstruction &I);

private:
    llvm::DenseMap<llvm::Function*, x86_PM_FuncBasicBlock*> m_funcBlocks;
    llvm::DenseSet<llvm::Instruction*>                   m_usedInstructions;
    llvm::Instruction*                                   m_pRoot;
    x86Pattern*                                             m_pCurrentPattern;
    std::vector<x86Pattern*>                                m_patterns;
};

#endif