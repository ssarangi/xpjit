#ifndef __ARCH_CODEGEN__
#define __ARCH_CODEGEN__

#include <backend/backendvar.h>

#include <common/llvm_warnings_push.h>
#include <llvm/IR/InstVisitor.h>
#include <common/llvm_warnings_pop.h>

#include <sstream>

class TemporaryStackSize;

struct MipsSymbolTable
{
public:
    MipsSymbolTable(int activtionRecordSizeInBytes)
        : m_activationRecordSizeInBytes(activtionRecordSizeInBytes)
    {}

    ~MipsSymbolTable()
    {
        for (auto pBaseVar : m_symbols)
        {
            delete pBaseVar.second;
        }

        m_symbols.clear();
    }

    BaseVariable* get(llvm::Value *pVal)
    {
        if (m_symbols.find(pVal) == m_symbols.end())
            return nullptr;

        return m_symbols[pVal];
    }

    void set(llvm::Value *pVal, BaseVariable *pBaseVar)
    {
        m_symbols[pVal] = pBaseVar;
    }

    void setActivationRecordSize(int size) { m_activationRecordSizeInBytes = size; }
    int getActivationRecordSize() { return m_activationRecordSizeInBytes; }

private:
    llvm::DenseMap<llvm::Value*, BaseVariable*> m_symbols;
    int m_activationRecordSizeInBytes;
};

class IArchCodeGen
{
public:
    IArchCodeGen()
        : m_pCurrentFunction(nullptr)
        , m_pTempStackSize(nullptr)
    {
    }

    ~IArchCodeGen()
    {
        for (auto keyvalpair : m_symbolTables)
        {
            delete keyvalpair.second;
        }

        m_symbolTables.clear();
    }

    virtual void initializeAssembler(llvm::Function *pMainFunc) = 0;

    virtual BaseVariable* getSymbol(llvm::Value *pV) = 0;

    virtual void visitFunction(llvm::Function &F) = 0;
    virtual void visitBasicBlock(llvm::BasicBlock &BB) = 0;
    virtual void visitReturnInst(llvm::ReturnInst &I) = 0;
    virtual void visitBranchInst(llvm::BranchInst &I) = 0;
    virtual void visitSwitchInst(llvm::SwitchInst &I) = 0;
    virtual void visitIndirectBrInst(llvm::IndirectBrInst &I) = 0;
    virtual void visitResumeInst(llvm::ResumeInst &I) = 0;
    virtual void visitUnreachableInst(llvm::UnreachableInst &I) = 0;
    virtual void visitICmpInst(llvm::ICmpInst &I) = 0;
    virtual void visitFCmpInst(llvm::FCmpInst &I) = 0;
    virtual void visitAllocaInst(llvm::AllocaInst &I) = 0;
    virtual void visitLoadInst(llvm::LoadInst     &I) = 0;
    virtual void visitStoreInst(llvm::StoreInst   &I) = 0;
    virtual void visitAtomicCmpXchgInst(llvm::AtomicCmpXchgInst &I) = 0;
    virtual void visitAtomicRMWInst(llvm::AtomicRMWInst &I) = 0;
    virtual void visitFenceInst(llvm::FenceInst   &I) = 0;
    virtual void visitGetElementPtrInst(llvm::GetElementPtrInst &I) = 0;
    virtual void visitPHINode(llvm::PHINode &I) = 0;
    virtual void visitTruncInst(llvm::TruncInst &I) = 0;
    virtual void visitZExtInst(llvm::ZExtInst &I) = 0;
    virtual void visitSExtInst(llvm::SExtInst &I) = 0;
    virtual void visitFPTruncInst(llvm::FPTruncInst &I) = 0;
    virtual void visitFPExtInst(llvm::FPExtInst &I) = 0;
    virtual void visitFPToUIInst(llvm::FPToUIInst &I) = 0;
    virtual void visitFPToSIInst(llvm::FPToSIInst &I) = 0;
    virtual void visitUIToFPInst(llvm::UIToFPInst &I) = 0;
    virtual void visitSIToFPInst(llvm::SIToFPInst &I) = 0;
    virtual void visitPtrToIntInst(llvm::PtrToIntInst &I) = 0;
    virtual void visitIntToPtrInst(llvm::IntToPtrInst &I) = 0;
    virtual void visitBitCastInst(llvm::BitCastInst &I) = 0;
    virtual void visitAddrSpaceCastInst(llvm::AddrSpaceCastInst &I) = 0;
    virtual void visitSelectInst(llvm::SelectInst &I) = 0;
    virtual void visitVAArgInst(llvm::VAArgInst   &I) = 0;
    virtual void visitExtractElementInst(llvm::ExtractElementInst &I) = 0;
    virtual void visitInsertElementInst(llvm::InsertElementInst &I) = 0;
    virtual void visitShuffleVectorInst(llvm::ShuffleVectorInst &I) = 0;
    virtual void visitExtractValueInst(llvm::ExtractValueInst &I) = 0;
    virtual void visitInsertValueInst(llvm::InsertValueInst &I) = 0;
    virtual void visitLandingPadInst(llvm::LandingPadInst &I) = 0;

    // Handle the special intrinsic instruction classes.
    virtual void visitDbgDeclareInst(llvm::DbgDeclareInst &I) = 0;
    virtual void visitDbgValueInst(llvm::DbgValueInst &I) = 0;
    virtual void visitDbgInfoIntrinsic(llvm::DbgInfoIntrinsic &I) = 0;
    virtual void visitMemSetInst(llvm::MemSetInst &I) = 0;
    virtual void visitMemCpyInst(llvm::MemCpyInst &I) = 0;
    virtual void visitMemMoveInst(llvm::MemMoveInst &I) = 0;
    virtual void visitMemTransferInst(llvm::MemTransferInst &I) = 0;
    virtual void visitMemIntrinsic(llvm::MemIntrinsic &I) = 0;
    virtual void visitVAStartInst(llvm::VAStartInst &I) = 0;
    virtual void visitVAEndInst(llvm::VAEndInst &I) = 0;
    virtual void visitVACopyInst(llvm::VACopyInst &I) = 0;
    virtual void visitIntrinsicInst(llvm::IntrinsicInst &I) = 0;

    // Call and Invoke are slightly different as they delegate first through
    // a generic CallSite visitor.
    virtual void visitCallInst(llvm::CallInst &I) = 0;
    virtual void visitInvokeInst(llvm::InvokeInst &I) = 0;

    // Next level propagators: If the user does not overload a specific
    // instruction type, they can overload one of these to get the whole class
    // of instructions...
    //
    virtual void visitCastInst(llvm::CastInst &I) = 0;
    virtual void visitBinaryOperator(llvm::BinaryOperator &I) = 0;
    virtual void visitCmpInst(llvm::CmpInst &I) = 0;
    virtual void visitTerminatorInst(llvm::TerminatorInst &I) = 0;
    virtual void visitUnaryInstruction(llvm::UnaryInstruction &I) = 0;

    // Provide a special visitor for a 'callsite' that visits both calls and
    // invokes. When unimplemented, properly delegates to either the terminator or
    // regular instruction visitor.
    virtual void visitCallSite(llvm::CallSite CS) = 0;

    // If the user wants a 'default' case, they can choose to override this
    // function.  If this function is not overloaded in the user's subclass, then
    // this instruction just gets ignored.
    //
    // Note that you MUST override this function if your return type is not void.
    //
    virtual void visitInstruction(llvm::Instruction &I) = 0;

    std::string getAssembly()
    {
        return m_ostream.str();
    }

    void setTempStackSize(TemporaryStackSize *pTempStackSize) { m_pTempStackSize = pTempStackSize; }

protected:
    std::stringstream m_ostream;
    llvm::DenseMap<llvm::Function*, MipsSymbolTable*> m_symbolTables;
    llvm::Function *m_pCurrentFunction;
    TemporaryStackSize *m_pTempStackSize;
};

#endif