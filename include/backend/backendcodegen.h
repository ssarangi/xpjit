#ifndef __CODEGEN__
#define __CODEGEN__

#include <backend/iarchcodegen.h>
#include "tempstacksize.h"
#include "backendvar.h"

#include <common/llvm_warnings_push.h>
#include <llvm/Pass.h>
#include <llvm/IR/InstVisitor.h>
#include <llvm/IR/Dominators.h>
#include <llvm/Analysis/PostDominators.h>
#include <llvm/Analysis/MemoryDependenceAnalysis.h>
#include <common/llvm_warnings_pop.h>

class CodeGenPass : public llvm::ModulePass, public llvm::InstVisitor<CodeGenPass>
{
    static char ID;

public:
    CodeGenPass(IArchCodeGen *pArchCodeGen)
        : llvm::ModulePass(ID)
        , m_pArchCodeGen(pArchCodeGen)
        , m_pTempStackSize(nullptr)
    {
        llvm::initializePostDominatorTreePass(*llvm::PassRegistry::getPassRegistry());
    }

    virtual void getAnalysisUsage(llvm::AnalysisUsage& usage) const
    {
        usage.setPreservesCFG();
        usage.addRequired<llvm::PostDominatorTree>();
        usage.addRequired<TemporaryStackSize>();
    }

    virtual bool runOnFunction(llvm::Function& F);
    
    virtual bool runOnModule(llvm::Module &M);

    virtual const char *getPassName() const
    {
        return "CodeGenPass";
    }

    std::string getAssembly() const
    {
        return m_assembly;
    }
    
    void visitFunction(llvm::Function &F);
    void visitBasicBlock(llvm::BasicBlock &BB);
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

    // Provide a special visitor for a 'callsite' that visits both calls and
    // invokes. When unimplemented, properly delegates to either the terminator or
    // regular instruction visitor.
    void visitCallSite(llvm::CallSite CS);

    // If the user wants a 'default' case, they can choose to override this
    // function.  If this function is not overloaded in the user's subclass, then
    // this instruction just gets ignored.
    //
    // Note that you MUST override this function if your return type is not void.
    //
    void visitInstruction(llvm::Instruction &I);

private:
    TemporaryStackSize *m_pTempStackSize;
    IArchCodeGen       *m_pArchCodeGen;
    std::string         m_assembly;
};

#endif