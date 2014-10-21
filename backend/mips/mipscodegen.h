#ifndef __MIPS_CODE_GEN__
#define __MIPS_CODE_GEN__

#include <backend/iarchcodegen.h>

class TemporaryStackSize;

class MipsCodeGen : public IArchCodeGen
{
public:
    MipsCodeGen()
        : m_temporaryBytesUsed(0)
    {
    }

    virtual void initializeAssembler(llvm::Function *pMainFunc);

    virtual BaseVariable* getSymbol(llvm::Value* pV);
    virtual void loadBaseVariable(BaseVariable *pVar, std::ostream &s);

    void emitPreInstructions(BaseVariable* pBaseVar);
    void genCommentStr(llvm::Instruction *pI);

    virtual void visitFunction(llvm::Function& F);
    virtual void visitReturnInst(llvm::ReturnInst &I);
    virtual void visitBranchInst(llvm::BranchInst &I);
    virtual void visitSwitchInst(llvm::SwitchInst &I);
    virtual void visitIndirectBrInst(llvm::IndirectBrInst &I);
    virtual void visitResumeInst(llvm::ResumeInst &I);
    virtual void visitUnreachableInst(llvm::UnreachableInst &I);
    virtual void visitICmpInst(llvm::ICmpInst &I);
    virtual void visitFCmpInst(llvm::FCmpInst &I);
    virtual void visitAllocaInst(llvm::AllocaInst &I);
    virtual void visitLoadInst(llvm::LoadInst     &I);
    virtual void visitStoreInst(llvm::StoreInst   &I);
    virtual void visitAtomicCmpXchgInst(llvm::AtomicCmpXchgInst &I);
    virtual void visitAtomicRMWInst(llvm::AtomicRMWInst &I);
    virtual void visitFenceInst(llvm::FenceInst   &I);
    virtual void visitGetElementPtrInst(llvm::GetElementPtrInst &I);
    virtual void visitPHINode(llvm::PHINode &I);
    virtual void visitTruncInst(llvm::TruncInst &I);
    virtual void visitZExtInst(llvm::ZExtInst &I);
    virtual void visitSExtInst(llvm::SExtInst &I);
    virtual void visitFPTruncInst(llvm::FPTruncInst &I);
    virtual void visitFPExtInst(llvm::FPExtInst &I);
    virtual void visitFPToUIInst(llvm::FPToUIInst &I);
    virtual void visitFPToSIInst(llvm::FPToSIInst &I);
    virtual void visitUIToFPInst(llvm::UIToFPInst &I);
    virtual void visitSIToFPInst(llvm::SIToFPInst &I);
    virtual void visitPtrToIntInst(llvm::PtrToIntInst &I);
    virtual void visitIntToPtrInst(llvm::IntToPtrInst &I);
    virtual void visitBitCastInst(llvm::BitCastInst &I);
    virtual void visitAddrSpaceCastInst(llvm::AddrSpaceCastInst &I);
    virtual void visitSelectInst(llvm::SelectInst &I);
    virtual void visitVAArgInst(llvm::VAArgInst   &I);
    virtual void visitExtractElementInst(llvm::ExtractElementInst &I);
    virtual void visitInsertElementInst(llvm::InsertElementInst &I);
    virtual void visitShuffleVectorInst(llvm::ShuffleVectorInst &I);
    virtual void visitExtractValueInst(llvm::ExtractValueInst &I);
    virtual void visitInsertValueInst(llvm::InsertValueInst &I);
    virtual void visitLandingPadInst(llvm::LandingPadInst &I);

    // Handle the special intrinsic instruction classes.
    virtual void visitDbgDeclareInst(llvm::DbgDeclareInst &I);
    virtual void visitDbgValueInst(llvm::DbgValueInst &I);
    virtual void visitDbgInfoIntrinsic(llvm::DbgInfoIntrinsic &I);
    virtual void visitMemSetInst(llvm::MemSetInst &I);
    virtual void visitMemCpyInst(llvm::MemCpyInst &I);
    virtual void visitMemMoveInst(llvm::MemMoveInst &I);
    virtual void visitMemTransferInst(llvm::MemTransferInst &I);
    virtual void visitMemIntrinsic(llvm::MemIntrinsic &I);
    virtual void visitVAStartInst(llvm::VAStartInst &I);
    virtual void visitVAEndInst(llvm::VAEndInst &I);
    virtual void visitVACopyInst(llvm::VACopyInst &I);
    virtual void visitIntrinsicInst(llvm::IntrinsicInst &I);

    // Call and Invoke are slightly different as they delegate first through
    // a generic CallSite visitor.
    virtual void visitCallInst(llvm::CallInst &I);
    virtual void visitInvokeInst(llvm::InvokeInst &I);

    // Next level propagators: If the user does not overload a specific
    // instruction type, they can overload one of these to get the whole class
    // of instructions...
    //
    virtual void visitCastInst(llvm::CastInst &I);
    virtual void visitBinaryOperator(llvm::BinaryOperator &I);
    virtual void visitCmpInst(llvm::CmpInst &I);
    virtual void visitTerminatorInst(llvm::TerminatorInst &I);
    virtual void visitUnaryInstruction(llvm::UnaryInstruction &I);

    // Provide a special visitor for a 'callsite' that visits both calls and
    // invokes. When unimplemented, properly delegates to either the terminator or
    // regular instruction visitor.
    virtual void visitCallSite(llvm::CallSite CS);

    // If the user wants a 'default' case, they can choose to override this
    // function.  If this function is not overloaded in the user's subclass, then
    // this instruction just gets ignored.
    //
    // Note that you MUST override this function if your return type is not void.
    //
    virtual void visitInstruction(llvm::Instruction &I);

private:
    void storeTemporary(llvm::Instruction *pI);

private:
    unsigned int m_temporaryBytesUsed;
};



#endif