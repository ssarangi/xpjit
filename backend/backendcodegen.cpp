#include <backend/codegenpublic.h>
#include <common/debug.h>

#include <backend/backendcodegen.h>
#include "mips/mipscodegen.h"
#include "mips/mipspatternmatch.h"
#include "regalloc.h"

#include <common/llvm_warnings_push.h>
#include <llvm/PassManager.h>
#include <llvm/ADT/PostOrderIterator.h>
#include <llvm/IR/InstIterator.h>
#include <common/llvm_warnings_pop.h>

#include <iostream>
#include <fstream>

void GenerateCode(CodeGenModule& M, std::string outputFile)
{
    llvm::PassManager mpm;
    MipsCodeGen *pMipsCodeGen = new MipsCodeGen();
    // CodeGenPass *pCodeGenPass = new CodeGenPass(pMipsCodeGen);
    mpm.add(new RegAlloc());
    mpm.add(pMipsCodeGen);
    // mpm.add(pCodeGenPass);
    mpm.run(*M.getLLVMModule());

    // Open the file and write into it.
    std::ofstream file;
    file.open(outputFile);

    file.write(pMipsCodeGen->getAssembly().c_str(), pMipsCodeGen->getAssembly().length());
    file.close();

    g_outputStream() << "-------------------------------------------------------------------\n";
    g_outputStream() << "                         Code Generation                           \n";
    g_outputStream() << "-------------------------------------------------------------------\n";
    g_outputStream() << pMipsCodeGen->getAssembly();
    g_outputStream.flush();
}

//char CodeGenPass::ID = 5;
//
//bool CodeGenPass::runOnFunction(llvm::Function &F)
//{
//    m_pTempStackSize = &getAnalysis<TemporaryStackSize>(F);
//    m_pArchCodeGen->setTempStackSize(m_pTempStackSize);
//
//    std::string funcName = F.getName();
//    if (funcName == "main")
//    {
//        // This is the main function. So call the handle main function
//        m_pArchCodeGen->initializeAssembler(&F);
//    }
//
//    visit(F);
//
//    m_assembly = m_pArchCodeGen->getAssembly();
//
//    llvm::outs().flush();
//
//    return true;
//}
//
//bool CodeGenPass::runOnModule(llvm::Module &M)
//{
//    // Iterate over every function
//    for (auto func = M.begin(); func != M.end(); ++func)
//    {
//        runOnFunction(*func);
//    }
//
//    return true;
//}
//
//void CodeGenPass::visitFunction(llvm::Function &F)
//{
//    m_pArchCodeGen->visitFunction(F);
//}
//
//void CodeGenPass::visitBasicBlock(llvm::BasicBlock &BB)
//{
//    m_pArchCodeGen->visitBasicBlock(BB);
//}
//
//void CodeGenPass::visitReturnInst(llvm::ReturnInst &I)
//{
//    m_pArchCodeGen->visitReturnInst(I);
//}
//
//void CodeGenPass::visitBranchInst(llvm::BranchInst &I)
//{
//    m_pArchCodeGen->visitBranchInst(I);
//}
//
//void CodeGenPass::visitSwitchInst(llvm::SwitchInst &I)
//{
//    m_pArchCodeGen->visitSwitchInst(I);
//}
//
//void CodeGenPass::visitIndirectBrInst(llvm::IndirectBrInst &I)
//{
//    m_pArchCodeGen->visitIndirectBrInst(I);
//}
//
//void CodeGenPass::visitResumeInst(llvm::ResumeInst &I)
//{
//    m_pArchCodeGen->visitResumeInst(I);
//}
//
//void CodeGenPass::visitUnreachableInst(llvm::UnreachableInst &I)
//{
//    m_pArchCodeGen->visitUnreachableInst(I);
//}
//
//void CodeGenPass::visitICmpInst(llvm::ICmpInst &I)
//{
//    m_pArchCodeGen->visitICmpInst(I);
//}
//
//void CodeGenPass::visitFCmpInst(llvm::FCmpInst &I)
//{
//    m_pArchCodeGen->visitFCmpInst(I);
//}
//
//void CodeGenPass::visitAllocaInst(llvm::AllocaInst &I)
//{
//    m_pArchCodeGen->visitAllocaInst(I);
//}
//
//void CodeGenPass::visitLoadInst(llvm::LoadInst     &I)
//{
//    m_pArchCodeGen->visitLoadInst(I);
//}
//
//void CodeGenPass::visitStoreInst(llvm::StoreInst   &I)
//{
//    m_pArchCodeGen->visitStoreInst(I);
//}
//
//void CodeGenPass::visitAtomicCmpXchgInst(llvm::AtomicCmpXchgInst &I)
//{
//    m_pArchCodeGen->visitAtomicCmpXchgInst(I);
//}
//
//void CodeGenPass::visitAtomicRMWInst(llvm::AtomicRMWInst &I)
//{
//    m_pArchCodeGen->visitAtomicRMWInst(I);
//}
//
//void CodeGenPass::visitFenceInst(llvm::FenceInst   &I)
//{
//    m_pArchCodeGen->visitFenceInst(I);
//}
//
//void CodeGenPass::visitGetElementPtrInst(llvm::GetElementPtrInst &I)
//{
//    m_pArchCodeGen->visitGetElementPtrInst(I);
//}
//
//void CodeGenPass::visitPHINode(llvm::PHINode &I)
//{
//    m_pArchCodeGen->visitPHINode(I);
//}
//
//void CodeGenPass::visitTruncInst(llvm::TruncInst &I)
//{
//    m_pArchCodeGen->visitTruncInst(I);
//}
//
//void CodeGenPass::visitZExtInst(llvm::ZExtInst &I)
//{
//    m_pArchCodeGen->visitZExtInst(I);
//}
//
//void CodeGenPass::visitSExtInst(llvm::SExtInst &I)
//{
//    m_pArchCodeGen->visitSExtInst(I);
//}
//
//void CodeGenPass::visitFPTruncInst(llvm::FPTruncInst &I)
//{
//    m_pArchCodeGen->visitFPTruncInst(I);
//}
//
//void CodeGenPass::visitFPExtInst(llvm::FPExtInst &I)
//{
//    m_pArchCodeGen->visitFPExtInst(I);
//}
//
//void CodeGenPass::visitFPToUIInst(llvm::FPToUIInst &I)
//{
//    m_pArchCodeGen->visitFPToUIInst(I);
//}
//
//void CodeGenPass::visitFPToSIInst(llvm::FPToSIInst &I)
//{
//    m_pArchCodeGen->visitFPToSIInst(I);
//}
//
//void CodeGenPass::visitUIToFPInst(llvm::UIToFPInst &I)
//{
//    m_pArchCodeGen->visitUIToFPInst(I);
//}
//
//void CodeGenPass::visitSIToFPInst(llvm::SIToFPInst &I)
//{
//    m_pArchCodeGen->visitSIToFPInst(I);
//}
//
//void CodeGenPass::visitPtrToIntInst(llvm::PtrToIntInst &I)
//{
//    m_pArchCodeGen->visitPtrToIntInst(I);
//}
//
//void CodeGenPass::visitIntToPtrInst(llvm::IntToPtrInst &I)
//{
//    m_pArchCodeGen->visitIntToPtrInst(I);
//}
//
//void CodeGenPass::visitBitCastInst(llvm::BitCastInst &I)
//{
//    m_pArchCodeGen->visitBitCastInst(I);
//}
//
//void CodeGenPass::visitAddrSpaceCastInst(llvm::AddrSpaceCastInst &I)
//{
//    m_pArchCodeGen->visitAddrSpaceCastInst(I);
//}
//
//void CodeGenPass::visitSelectInst(llvm::SelectInst &I)
//{
//    m_pArchCodeGen->visitSelectInst(I);
//}
//
//void CodeGenPass::visitVAArgInst(llvm::VAArgInst   &I)
//{
//    m_pArchCodeGen->visitVAArgInst(I);
//}
//
//void CodeGenPass::visitExtractElementInst(llvm::ExtractElementInst &I)
//{
//    m_pArchCodeGen->visitExtractElementInst(I);
//}
//
//void CodeGenPass::visitInsertElementInst(llvm::InsertElementInst &I)
//{
//    m_pArchCodeGen->visitInsertElementInst(I);
//}
//
//void CodeGenPass::visitShuffleVectorInst(llvm::ShuffleVectorInst &I)
//{
//    m_pArchCodeGen->visitShuffleVectorInst(I);
//}
//
//void CodeGenPass::visitExtractValueInst(llvm::ExtractValueInst &I)
//{
//    m_pArchCodeGen->visitExtractValueInst(I);
//}
//
//void CodeGenPass::visitInsertValueInst(llvm::InsertValueInst &I)
//{
//    m_pArchCodeGen->visitInsertValueInst(I);
//}
//
//void CodeGenPass::visitLandingPadInst(llvm::LandingPadInst &I)
//{
//    m_pArchCodeGen->visitLandingPadInst(I);
//}
//
//// Handle the special intrinsic instruction classes.
//void CodeGenPass::visitDbgDeclareInst(llvm::DbgDeclareInst &I)
//{
//    m_pArchCodeGen->visitDbgDeclareInst(I);
//}
//
//void CodeGenPass::visitDbgValueInst(llvm::DbgValueInst &I)
//{
//    m_pArchCodeGen->visitDbgValueInst(I);
//}
//
//void CodeGenPass::visitDbgInfoIntrinsic(llvm::DbgInfoIntrinsic &I)
//{
//    m_pArchCodeGen->visitDbgInfoIntrinsic(I);
//}
//
//void CodeGenPass::visitMemSetInst(llvm::MemSetInst &I)
//{
//    m_pArchCodeGen->visitMemSetInst(I);
//}
//
//void CodeGenPass::visitMemCpyInst(llvm::MemCpyInst &I)
//{
//    m_pArchCodeGen->visitMemCpyInst(I);
//}
//
//void CodeGenPass::visitMemMoveInst(llvm::MemMoveInst &I)
//{
//    m_pArchCodeGen->visitMemMoveInst(I);
//}
//
//void CodeGenPass::visitMemTransferInst(llvm::MemTransferInst &I)
//{
//    m_pArchCodeGen->visitMemTransferInst(I);
//}
//
//void CodeGenPass::visitMemIntrinsic(llvm::MemIntrinsic &I)
//{
//    m_pArchCodeGen->visitMemIntrinsic(I);
//}
//
//void CodeGenPass::visitVAStartInst(llvm::VAStartInst &I)
//{
//    m_pArchCodeGen->visitVAStartInst(I);
//}
//
//void CodeGenPass::visitVAEndInst(llvm::VAEndInst &I)
//{
//    m_pArchCodeGen->visitVAEndInst(I);
//}
//
//void CodeGenPass::visitVACopyInst(llvm::VACopyInst &I)
//{
//    m_pArchCodeGen->visitVACopyInst(I);
//}
//
//void CodeGenPass::visitIntrinsicInst(llvm::IntrinsicInst &I)
//{
//    m_pArchCodeGen->visitIntrinsicInst(I);
//}
//
//// Call and Invoke are slightly different as they delegate first through
//// a generic CallSite visitor.
//void CodeGenPass::visitCallInst(llvm::CallInst &I)
//{
//    m_pArchCodeGen->visitCallInst(I);
//}
//
//void CodeGenPass::visitInvokeInst(llvm::InvokeInst &I)
//{
//    m_pArchCodeGen->visitInvokeInst(I);
//}
//
//// Next level propagators: If the user does not overload a specific
//// instruction type, they can overload one of these to get the whole class
//// of instructions...
////
//void CodeGenPass::visitCastInst(llvm::CastInst &I)
//{
//    m_pArchCodeGen->visitCastInst(I);
//}
//
//void CodeGenPass::visitBinaryOperator(llvm::BinaryOperator &I)
//{
//    m_pArchCodeGen->visitBinaryOperator(I);
//}
//
//void CodeGenPass::visitCmpInst(llvm::CmpInst &I)
//{
//    m_pArchCodeGen->visitCmpInst(I);
//}
//
//void CodeGenPass::visitTerminatorInst(llvm::TerminatorInst &I)
//{
//    m_pArchCodeGen->visitTerminatorInst(I);
//}
//
//void CodeGenPass::visitUnaryInstruction(llvm::UnaryInstruction &I)
//{
//    m_pArchCodeGen->visitUnaryInstruction(I);
//}
//
//// Provide a special visitor for a 'callsite' that visits both calls and
//// invokes. When unimplemented, properly delegates to either the terminator or
//// regular instruction visitor.
//void CodeGenPass::visitCallSite(llvm::CallSite CS)
//{
//    m_pArchCodeGen->visitCallSite(CS);
//}
//
//// If the user wants a 'default' case, they can choose to override this
//// function.  If this function is not overloaded in the user's subclass, then
//// this instruction just gets ignored.
////
//// Note that you MUST override this function if your return type is not void.CodeGenPass::
////
//void CodeGenPass::visitInstruction(llvm::Instruction &I)
//{
//    m_pArchCodeGen->visitInstruction(I);
//}