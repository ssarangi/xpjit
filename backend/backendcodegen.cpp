#include <backend/codegenpublic.h>
#include <common/debug.h>

#include "backendcodegen.h"
#include "mips/mipsregs.h"

#include <llvm/PassManager.h>
#include <llvm/ADT/PostOrderIterator.h>

#include <iostream>

char CodeGenPass::ID = 5;

bool CodeGenPass::runOnFunction(llvm::Function& F)
{
    //llvm::PostDominatorTree* PDT = nullptr;
    //if (!F.isDeclaration())
    //    PDT = &getAnalysis<llvm::PostDominatorTree>(F);
    LiveVariables* pLV = &getAnalysis<LiveVariables>(F);
    for (llvm::po_iterator<llvm::BasicBlock*> I = llvm::po_begin(&F.getEntryBlock()),
                                              IE = llvm::po_end(&F.getEntryBlock());
                                              I != IE; ++I)
    {
        llvm::Value* V = I->begin();
        llvm::BasicBlock* BB = *I;
        bool isKilled = pLV->isKilledInBlock(V, BB);
        llvm::outs() << "Code Generation for: " << F.getName() << "::" << I->getName() << "\n";
    }

    llvm::outs().flush();

    return true;
}

bool CodeGenPass::runOnModule(llvm::Module &M)
{
    // Iterate over every function
    for (auto func = M.begin(); func != M.end(); ++func)
    {
        runOnFunction(*func);
    }

    return true;
}


void GenerateCode(CodeGenModule& M)
{
    llvm::PassManager mpm;
    mpm.add(new CodeGenPass());
    mpm.run(*M.getLLVMModule());
}

void CodeGenPass::visitReturnInst(llvm::ReturnInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Return Inst not implemented");
}
void CodeGenPass::visitBranchInst(llvm::BranchInst &I)
{

}
void CodeGenPass::visitSwitchInst(llvm::SwitchInst &I)
{

}
void CodeGenPass::visitIndirectBrInst(llvm::IndirectBrInst &I)
{

}
void CodeGenPass::visitResumeInst(llvm::ResumeInst &I)
{

}
void CodeGenPass::visitUnreachableInst(llvm::UnreachableInst &I)
{

}
void CodeGenPass::visitICmpInst(llvm::ICmpInst &I)
{

}
void CodeGenPass::visitFCmpInst(llvm::FCmpInst &I)
{

}
void CodeGenPass::visitAllocaInst(llvm::AllocaInst &I)
{

}
void CodeGenPass::visitLoadInst(llvm::LoadInst     &I)
{

}
void CodeGenPass::visitStoreInst(llvm::StoreInst   &I)
{

}
void CodeGenPass::visitAtomicCmpXchgInst(llvm::AtomicCmpXchgInst &I)
{

}
void CodeGenPass::visitAtomicRMWInst(llvm::AtomicRMWInst &I)
{

}
void CodeGenPass::visitFenceInst(llvm::FenceInst   &I)
{

}
void CodeGenPass::visitGetElementPtrInst(llvm::GetElementPtrInst &I)
{

}
void CodeGenPass::visitPHINode(llvm::PHINode &I)
{

}
void CodeGenPass::visitTruncInst(llvm::TruncInst &I)
{

}
void CodeGenPass::visitZExtInst(llvm::ZExtInst &I)
{

}
void CodeGenPass::visitSExtInst(llvm::SExtInst &I)
{

}
void CodeGenPass::visitFPTruncInst(llvm::FPTruncInst &I)
{

}
void CodeGenPass::visitFPExtInst(llvm::FPExtInst &I)
{

}
void CodeGenPass::visitFPToUIInst(llvm::FPToUIInst &I)
{

}
void CodeGenPass::visitFPToSIInst(llvm::FPToSIInst &I)
{

}
void CodeGenPass::visitUIToFPInst(llvm::UIToFPInst &I)
{

}
void CodeGenPass::visitSIToFPInst(llvm::SIToFPInst &I)
{

}
void CodeGenPass::visitPtrToIntInst(llvm::PtrToIntInst &I)
{

}
void CodeGenPass::visitIntToPtrInst(llvm::IntToPtrInst &I)
{

}
void CodeGenPass::visitBitCastInst(llvm::BitCastInst &I)
{

}
void CodeGenPass::visitAddrSpaceCastInst(llvm::AddrSpaceCastInst &I)
{

}
void CodeGenPass::visitSelectInst(llvm::SelectInst &I)
{

}
void CodeGenPass::visitVAArgInst(llvm::VAArgInst   &I)
{

}
void CodeGenPass::visitExtractElementInst(llvm::ExtractElementInst &I)
{

}
void CodeGenPass::visitInsertElementInst(llvm::InsertElementInst &I)
{

}
void CodeGenPass::visitShuffleVectorInst(llvm::ShuffleVectorInst &I)
{

}
void CodeGenPass::visitExtractValueInst(llvm::ExtractValueInst &I)
{

}
void CodeGenPass::visitInsertValueInst(llvm::InsertValueInst &I)
{

}
void CodeGenPass::visitLandingPadInst(llvm::LandingPadInst &I)
{

}

// Handle the special intrinsic instruction classes.
void CodeGenPass::visitDbgDeclareInst(llvm::DbgDeclareInst &I)
{

}
void CodeGenPass::visitDbgValueInst(llvm::DbgValueInst &I)
{

}
void CodeGenPass::visitDbgInfoIntrinsic(llvm::DbgInfoIntrinsic &I)
{

}
void CodeGenPass::visitMemSetInst(llvm::MemSetInst &I)
{

}
void CodeGenPass::visitMemCpyInst(llvm::MemCpyInst &I)
{

}
void CodeGenPass::visitMemMoveInst(llvm::MemMoveInst &I)
{

}
void CodeGenPass::visitMemTransferInst(llvm::MemTransferInst &I)
{

}
void CodeGenPass::visitMemIntrinsic(llvm::MemIntrinsic &I)
{

}
void CodeGenPass::visitVAStartInst(llvm::VAStartInst &I)
{

}
void CodeGenPass::visitVAEndInst(llvm::VAEndInst &I)
{

}
void CodeGenPass::visitVACopyInst(llvm::VACopyInst &I)
{

}
void CodeGenPass::visitIntrinsicInst(llvm::IntrinsicInst &I)
{

}

// Call and Invoke are slightly different as they delegate first through
// a generic CallSite visitor.
void CodeGenPass::visitCallInst(llvm::CallInst &I)
{

}
void CodeGenPass::visitInvokeInst(llvm::InvokeInst &I)
{

}

// Next level propagators: If the user does not overload a specific
// instruction type, they can overload one of these to get the whole class
// of instructions...
//
void CodeGenPass::visitCastInst(llvm::CastInst &I)
{

}
void CodeGenPass::visitBinaryOperator(llvm::BinaryOperator &I)
{

}
void CodeGenPass::visitCmpInst(llvm::CmpInst &I)
{

}
void CodeGenPass::visitTerminatorInst(llvm::TerminatorInst &I)
{

}
void CodeGenPass::visitUnaryInstruction(llvm::UnaryInstruction &I)
{

}

// Provide a special visitor for a 'callsite' that visits both calls and
// invokes. When unimplemented, properly delegates to either the terminator or
// regular instruction visitor.
void CodeGenPass::visitCallSite(llvm::CallSite CS)
{

}

// If the user wants a 'default' case, they can choose to override this
// function.  If this function is not overloaded in the user's subclass, then
// this instruction just gets ignored.
//
// Note that you MUST override this function if your return type is not void.CodeGenPass::
//
void CodeGenPass::visitInstruction(llvm::Instruction &I)
{

}