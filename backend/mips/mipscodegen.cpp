#include <common/debug.h>
#include "mipscodegen.h"

void MipsCodeGen::initializeAssembler(llvm::Function *pMainFunc)
{
    // First output the data section
    m_ostream << ".data" << std::endl;
    m_ostream << std::endl;
    m_ostream << ".text" << std::endl;
    m_ostream << ".globl main" << std::endl;
    m_ostream << std::endl;
}

void MipsCodeGen::visitFunction(llvm::Function& F)
{
    m_ostream << F.getName().str() << ":" << std::endl;
}

void MipsCodeGen::visitReturnInst(llvm::ReturnInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Return Inst not implemented");
}

void MipsCodeGen::visitBranchInst(llvm::BranchInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Branch Inst not implemented");
}

void MipsCodeGen::visitSwitchInst(llvm::SwitchInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Switch Inst not implemented");
}

void MipsCodeGen::visitIndirectBrInst(llvm::IndirectBrInst &I)
{
    ICARUS_NOT_IMPLEMENTED("IndirectBrInst Inst not implemented");
}

void MipsCodeGen::visitResumeInst(llvm::ResumeInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Resume Inst not implemented");
}

void MipsCodeGen::visitUnreachableInst(llvm::UnreachableInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Unreachable Inst not implemented");
}

void MipsCodeGen::visitICmpInst(llvm::ICmpInst &I)
{
    ICARUS_NOT_IMPLEMENTED("ICmp Inst not implemented");
}

void MipsCodeGen::visitFCmpInst(llvm::FCmpInst &I)
{
    ICARUS_NOT_IMPLEMENTED("FCmp Inst not implemented");
}

void MipsCodeGen::visitAllocaInst(llvm::AllocaInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Alloca Inst not implemented");
}

void MipsCodeGen::visitLoadInst(llvm::LoadInst     &I)
{
    ICARUS_NOT_IMPLEMENTED("Load Inst not implemented");
}

void MipsCodeGen::visitStoreInst(llvm::StoreInst   &I)
{
    ICARUS_NOT_IMPLEMENTED("Store Inst not implemented");
}

void MipsCodeGen::visitAtomicCmpXchgInst(llvm::AtomicCmpXchgInst &I)
{
    ICARUS_NOT_IMPLEMENTED("AtomicCmpXchg Inst not implemented");
}

void MipsCodeGen::visitAtomicRMWInst(llvm::AtomicRMWInst &I)
{
    ICARUS_NOT_IMPLEMENTED("AtomicRMW Inst not implemented");
}

void MipsCodeGen::visitFenceInst(llvm::FenceInst   &I)
{
    ICARUS_NOT_IMPLEMENTED("Fence Inst not implemented");
}

void MipsCodeGen::visitGetElementPtrInst(llvm::GetElementPtrInst &I)
{
    ICARUS_NOT_IMPLEMENTED("GEP Inst not implemented");
}

void MipsCodeGen::visitPHINode(llvm::PHINode &I)
{
    ICARUS_NOT_IMPLEMENTED("Phi Inst not implemented");
}

void MipsCodeGen::visitTruncInst(llvm::TruncInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Trunc Inst not implemented");
}

void MipsCodeGen::visitZExtInst(llvm::ZExtInst &I)
{
    ICARUS_NOT_IMPLEMENTED("ZExt Inst not implemented");
}

void MipsCodeGen::visitSExtInst(llvm::SExtInst &I)
{
    ICARUS_NOT_IMPLEMENTED("SExt Inst not implemented");
}

void MipsCodeGen::visitFPTruncInst(llvm::FPTruncInst &I)
{
    ICARUS_NOT_IMPLEMENTED("FPTrunc Inst not implemented");
}

void MipsCodeGen::visitFPExtInst(llvm::FPExtInst &I)
{
    ICARUS_NOT_IMPLEMENTED("FPExt Inst not implemented");
}

void MipsCodeGen::visitFPToUIInst(llvm::FPToUIInst &I)
{
    ICARUS_NOT_IMPLEMENTED("FPToUI Inst not implemented");
}

void MipsCodeGen::visitFPToSIInst(llvm::FPToSIInst &I)
{
    ICARUS_NOT_IMPLEMENTED("FPToSI Inst not implemented");
}

void MipsCodeGen::visitUIToFPInst(llvm::UIToFPInst &I)
{
    ICARUS_NOT_IMPLEMENTED("UIToFP Inst not implemented");
}

void MipsCodeGen::visitSIToFPInst(llvm::SIToFPInst &I)
{
    ICARUS_NOT_IMPLEMENTED("SIToFP Inst not implemented");
}

void MipsCodeGen::visitPtrToIntInst(llvm::PtrToIntInst &I)
{
    ICARUS_NOT_IMPLEMENTED("PtrToInt Inst not implemented");
}

void MipsCodeGen::visitIntToPtrInst(llvm::IntToPtrInst &I)
{
    ICARUS_NOT_IMPLEMENTED("IntToPtr Inst not implemented");
}

void MipsCodeGen::visitBitCastInst(llvm::BitCastInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Bitcast Inst not implemented");
}

void MipsCodeGen::visitAddrSpaceCastInst(llvm::AddrSpaceCastInst &I)
{
    ICARUS_NOT_IMPLEMENTED("AddrSpaceCast Inst not implemented");
}

void MipsCodeGen::visitSelectInst(llvm::SelectInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Select Inst not implemented");
}

void MipsCodeGen::visitVAArgInst(llvm::VAArgInst   &I)
{
    ICARUS_NOT_IMPLEMENTED("VAArg Inst not implemented");
}

void MipsCodeGen::visitExtractElementInst(llvm::ExtractElementInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Extract Element Inst not implemented");
}

void MipsCodeGen::visitInsertElementInst(llvm::InsertElementInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Insert Element Inst not implemented");
}

void MipsCodeGen::visitShuffleVectorInst(llvm::ShuffleVectorInst &I)
{
    ICARUS_NOT_IMPLEMENTED("ShuffleVector Inst not implemented");
}

void MipsCodeGen::visitExtractValueInst(llvm::ExtractValueInst &I)
{
    ICARUS_NOT_IMPLEMENTED("ExtractValue Inst not implemented");
}

void MipsCodeGen::visitInsertValueInst(llvm::InsertValueInst &I)
{
    ICARUS_NOT_IMPLEMENTED("InsertValue Inst not implemented");
}

void MipsCodeGen::visitLandingPadInst(llvm::LandingPadInst &I)
{
    ICARUS_NOT_IMPLEMENTED("LandingPad Inst not implemented");
}


// Handle the special intrinsic instruction classes.
void MipsCodeGen::visitDbgDeclareInst(llvm::DbgDeclareInst &I)
{
    ICARUS_NOT_IMPLEMENTED("DbgDeclare Inst not implemented");
}

void MipsCodeGen::visitDbgValueInst(llvm::DbgValueInst &I)
{
    ICARUS_NOT_IMPLEMENTED("DbgValue Inst not implemented");
}

void MipsCodeGen::visitDbgInfoIntrinsic(llvm::DbgInfoIntrinsic &I)
{
    ICARUS_NOT_IMPLEMENTED("DbgInfo Inst not implemented");
}

void MipsCodeGen::visitMemSetInst(llvm::MemSetInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Memset Inst not implemented");
}

void MipsCodeGen::visitMemCpyInst(llvm::MemCpyInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Memcpy Inst not implemented");
}

void MipsCodeGen::visitMemMoveInst(llvm::MemMoveInst &I)
{
    ICARUS_NOT_IMPLEMENTED("MemMove Inst not implemented");
}

void MipsCodeGen::visitMemTransferInst(llvm::MemTransferInst &I)
{
    ICARUS_NOT_IMPLEMENTED("MemTransfer Inst not implemented");
}

void MipsCodeGen::visitMemIntrinsic(llvm::MemIntrinsic &I)
{
    ICARUS_NOT_IMPLEMENTED("Mem Inst not implemented");
}

void MipsCodeGen::visitVAStartInst(llvm::VAStartInst &I)
{
    ICARUS_NOT_IMPLEMENTED("VAStart Inst not implemented");
}

void MipsCodeGen::visitVAEndInst(llvm::VAEndInst &I)
{
    ICARUS_NOT_IMPLEMENTED("VA End Inst not implemented");
}

void MipsCodeGen::visitVACopyInst(llvm::VACopyInst &I)
{
    ICARUS_NOT_IMPLEMENTED("VA Copy Inst not implemented");
}

void MipsCodeGen::visitIntrinsicInst(llvm::IntrinsicInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Intrinsic Inst not implemented");
}


// Call and Invoke are slightly different as they delegate first through
// a generic CallSite visitor.
void MipsCodeGen::visitCallInst(llvm::CallInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Call Inst not implemented");
}

void MipsCodeGen::visitInvokeInst(llvm::InvokeInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Invoke Inst not implemented");
}


// Next level propagators: If the user does not overload a specific
// instruction type, they can overload one of these to get the whole class
// of instructions...
//
void MipsCodeGen::visitCastInst(llvm::CastInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Cast Inst not implemented");
}

void MipsCodeGen::visitBinaryOperator(llvm::BinaryOperator &I)
{
    ICARUS_NOT_IMPLEMENTED("Binary Operator Inst not implemented");
}

void MipsCodeGen::visitCmpInst(llvm::CmpInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Cmp Inst not implemented");
}

void MipsCodeGen::visitTerminatorInst(llvm::TerminatorInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Terminator Inst not implemented");
}

void MipsCodeGen::visitUnaryInstruction(llvm::UnaryInstruction &I)
{
    ICARUS_NOT_IMPLEMENTED("Unary Inst not implemented");
}


// Provide a special visitor for a 'callsite' that visits both calls and
// invokes. When unimplemented, properly delegates to either the terminator or
// regular instruction visitor.
void MipsCodeGen::visitCallSite(llvm::CallSite CS)
{
    ICARUS_NOT_IMPLEMENTED("CallSite Inst not implemented");
}


// If the user wants a 'default' case, they can choose to override this
// function.  If this function is not overloaded in the user's subclass, then
// this instruction just gets ignored.
//
// Note that you MUST override this function if your return type is not void.MipsCodeGen::
//
void MipsCodeGen::visitInstruction(llvm::Instruction &I)
{
    I.dump();
    ICARUS_NOT_IMPLEMENTED("Instruction not implemented");
}
