#include <common/debug.h>
#include "asmjit_codegen.h"
#include "../tempstacksize.h"

#include <common/llvm_warnings_push.h>
#include <common/llvm_warnings_pop.h>

#include <stack>

#define COMMENT_STR(STR)                        \
{                                               \
    m_ostream << "# " << STR << std::endl;      \
}

#define COMMENT_STR_WITH_INSTR(INSTR)           \
{                                               \
    std::string str;                            \
    llvm::raw_string_ostream os(str);           \
    INSTR.print(os);                            \
    m_ostream << "; " << os.str() << std::endl; \
}

char AsmJitCodeGen::ID = 0;

void AsmJitCodeGen::storeTemporary(llvm::Instruction *pI)
{

}

BaseVariable* AsmJitCodeGen::getSymbol(llvm::Value* pV)
{
    //BaseVariable *pSymbol = nullptr;

    //// Check if the llvm value is a constant int
    //if (llvm::Constant *pC = llvm::dyn_cast<llvm::Constant>(pV))
    //{
    //    // So this is a constant. So see which one.
    //    if (llvm::ConstantInt *pCI = llvm::dyn_cast<llvm::ConstantInt>(pC))
    //        pSymbol = new Immediate((int)pCI->getZExtValue());
    //}
    //else if (llvm::Argument *pArg = llvm::dyn_cast<llvm::Argument>(pV))
    //{
    //    TODO("Assign registers in the register allocator. For now just map to variables")

    //    assert(0 && "Not handled yet");
    //}
    //else
    //{
    //    // Check if we already have a symbol in the symbol table. If not the create a symbol
    //    pSymbol = m_pRegAllocator->getSymbol(pV);
    //}

    //return pSymbol;
}

void AsmJitCodeGen::loadBaseVariable(BaseVariable *pVar, std::ostream &s)
{

}

void AsmJitCodeGen::initializeAssembler()
{
}

void AsmJitCodeGen::destroyAssembler()
{
}

void AsmJitCodeGen::createLabel(llvm::BasicBlock *pBlock)
{
}

bool AsmJitCodeGen::runOnModule(llvm::Module& M)
{
    m_pAsmJitPatternMatch = &getAnalysis<AsmJitPatternMatch>();

    initializeAssembler();

    // Emit the code for the main function at the end to ensure that the assembler emits the epilogue
    // properly
    llvm::Function *pMainF = nullptr;
    for (llvm::Module::iterator f = M.begin(), fe = M.end(); f != fe; ++f)
    {
        if (f->isDeclaration())
            continue;

        if (f->getName() == llvm::StringRef("main"))
        {
            pMainF = f;
            continue;
        }

        // m_pTempStackSize = &getAnalysis<TemporaryStackSize>(*f);
        visitFunction(*f);

        x86_PM_FuncBasicBlock* pFuncBlock = m_pAsmJitPatternMatch->getFuncBlock(f);
        pFuncBlock->m_pFunction = f;

        for (unsigned int i = 0; i < pFuncBlock->m_blocks.size(); ++i)
        {
            x86_PM_BasicBlock* pBlock = pFuncBlock->m_blocks[i];

            // Create a label for this id
            if (pBlock->getName() != "entry")
                createLabel(pBlock->pBB);

            for (auto i = pBlock->m_dags.rbegin(), e = pBlock->m_dags.rend(); i != e; ++i)
            {
                i->m_pPattern->emit(this);
            }
        }
    }

    visitFunction(*pMainF);
    destroyAssembler();

    return false;
}

void AsmJitCodeGen::visitFunction(llvm::Function& F)
{
    asmjit::FuncPrototype prototype;
    prototype._setPrototype()
    m_pCompiler->addFunc(asmjit::kFuncConvHost, )
}

void AsmJitCodeGen::visitBasicBlock(llvm::BasicBlock &BB)
{
    ICARUS_NOT_IMPLEMENTED("Basic Block not implemented");
}

void AsmJitCodeGen::visitReturnInst(llvm::ReturnInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Return Inst not implemented");
}

void AsmJitCodeGen::visitBranchInst(llvm::BranchInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Branch Inst not implemented");
}

void AsmJitCodeGen::visitSwitchInst(llvm::SwitchInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Switch Inst not implemented");
}

void AsmJitCodeGen::visitIndirectBrInst(llvm::IndirectBrInst &I)
{
    ICARUS_NOT_IMPLEMENTED("IndirectBrInst Inst not implemented");
}

void AsmJitCodeGen::visitResumeInst(llvm::ResumeInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Resume Inst not implemented");
}

void AsmJitCodeGen::visitUnreachableInst(llvm::UnreachableInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Unreachable Inst not implemented");
}

void AsmJitCodeGen::visitICmpInst(llvm::ICmpInst &I)
{
    ICARUS_NOT_IMPLEMENTED("ICmp Inst not implemented");
}

void AsmJitCodeGen::visitFCmpInst(llvm::FCmpInst &I)
{
    ICARUS_NOT_IMPLEMENTED("FCmp Inst not implemented");
}

void AsmJitCodeGen::visitAllocaInst(llvm::AllocaInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Alloca Inst not implemented");
}

void AsmJitCodeGen::visitLoadInst(llvm::LoadInst     &I)
{
    ICARUS_NOT_IMPLEMENTED("Load Inst not implemented");
}

void AsmJitCodeGen::visitStoreInst(llvm::StoreInst   &I)
{
    ICARUS_NOT_IMPLEMENTED("Store Inst not implemented");
}

void AsmJitCodeGen::visitAtomicCmpXchgInst(llvm::AtomicCmpXchgInst &I)
{
    ICARUS_NOT_IMPLEMENTED("AtomicCmpXchg Inst not implemented");
}

void AsmJitCodeGen::visitAtomicRMWInst(llvm::AtomicRMWInst &I)
{
    ICARUS_NOT_IMPLEMENTED("AtomicRMW Inst not implemented");
}

void AsmJitCodeGen::visitFenceInst(llvm::FenceInst   &I)
{
    ICARUS_NOT_IMPLEMENTED("Fence Inst not implemented");
}

void AsmJitCodeGen::visitGetElementPtrInst(llvm::GetElementPtrInst &I)
{
    ICARUS_NOT_IMPLEMENTED("GEP Inst not implemented");
}

void AsmJitCodeGen::visitPHINode(llvm::PHINode &I)
{
    ICARUS_NOT_IMPLEMENTED("Phi Inst not implemented");
}

void AsmJitCodeGen::visitTruncInst(llvm::TruncInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Trunc Inst not implemented");
}

void AsmJitCodeGen::visitZExtInst(llvm::ZExtInst &I)
{
    ICARUS_NOT_IMPLEMENTED("ZExt Inst not implemented");
}

void AsmJitCodeGen::visitSExtInst(llvm::SExtInst &I)
{
    ICARUS_NOT_IMPLEMENTED("SExt Inst not implemented");
}

void AsmJitCodeGen::visitFPTruncInst(llvm::FPTruncInst &I)
{
    ICARUS_NOT_IMPLEMENTED("FPTrunc Inst not implemented");
}

void AsmJitCodeGen::visitFPExtInst(llvm::FPExtInst &I)
{
    ICARUS_NOT_IMPLEMENTED("FPExt Inst not implemented");
}

void AsmJitCodeGen::visitFPToUIInst(llvm::FPToUIInst &I)
{
    ICARUS_NOT_IMPLEMENTED("FPToUI Inst not implemented");
}

void AsmJitCodeGen::visitFPToSIInst(llvm::FPToSIInst &I)
{
    ICARUS_NOT_IMPLEMENTED("FPToSI Inst not implemented");
}

void AsmJitCodeGen::visitUIToFPInst(llvm::UIToFPInst &I)
{
    ICARUS_NOT_IMPLEMENTED("UIToFP Inst not implemented");
}

void AsmJitCodeGen::visitSIToFPInst(llvm::SIToFPInst &I)
{
    ICARUS_NOT_IMPLEMENTED("SIToFP Inst not implemented");
}

void AsmJitCodeGen::visitPtrToIntInst(llvm::PtrToIntInst &I)
{
    ICARUS_NOT_IMPLEMENTED("PtrToInt Inst not implemented");
}

void AsmJitCodeGen::visitIntToPtrInst(llvm::IntToPtrInst &I)
{
    ICARUS_NOT_IMPLEMENTED("IntToPtr Inst not implemented");
}

void AsmJitCodeGen::visitBitCastInst(llvm::BitCastInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Bitcast Inst not implemented");
}

void AsmJitCodeGen::visitAddrSpaceCastInst(llvm::AddrSpaceCastInst &I)
{
    ICARUS_NOT_IMPLEMENTED("AddrSpaceCast Inst not implemented");
}

void AsmJitCodeGen::visitSelectInst(llvm::SelectInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Select Inst not implemented");
}

void AsmJitCodeGen::visitVAArgInst(llvm::VAArgInst   &I)
{
    ICARUS_NOT_IMPLEMENTED("VAArg Inst not implemented");
}

void AsmJitCodeGen::visitExtractElementInst(llvm::ExtractElementInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Extract Element Inst not implemented");
}

void AsmJitCodeGen::visitInsertElementInst(llvm::InsertElementInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Insert Element Inst not implemented");
}

void AsmJitCodeGen::visitShuffleVectorInst(llvm::ShuffleVectorInst &I)
{
    ICARUS_NOT_IMPLEMENTED("ShuffleVector Inst not implemented");
}

void AsmJitCodeGen::visitExtractValueInst(llvm::ExtractValueInst &I)
{
    ICARUS_NOT_IMPLEMENTED("ExtractValue Inst not implemented");
}

void AsmJitCodeGen::visitInsertValueInst(llvm::InsertValueInst &I)
{
    ICARUS_NOT_IMPLEMENTED("InsertValue Inst not implemented");
}

void AsmJitCodeGen::visitLandingPadInst(llvm::LandingPadInst &I)
{
    ICARUS_NOT_IMPLEMENTED("LandingPad Inst not implemented");
}


// Handle the special intrinsic instruction classes.
void AsmJitCodeGen::visitDbgDeclareInst(llvm::DbgDeclareInst &I)
{
    ICARUS_NOT_IMPLEMENTED("DbgDeclare Inst not implemented");
}

void AsmJitCodeGen::visitDbgValueInst(llvm::DbgValueInst &I)
{
    ICARUS_NOT_IMPLEMENTED("DbgValue Inst not implemented");
}

void AsmJitCodeGen::visitDbgInfoIntrinsic(llvm::DbgInfoIntrinsic &I)
{
    ICARUS_NOT_IMPLEMENTED("DbgInfo Inst not implemented");
}

void AsmJitCodeGen::visitMemSetInst(llvm::MemSetInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Memset Inst not implemented");
}

void AsmJitCodeGen::visitMemCpyInst(llvm::MemCpyInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Memcpy Inst not implemented");
}

void AsmJitCodeGen::visitMemMoveInst(llvm::MemMoveInst &I)
{
    ICARUS_NOT_IMPLEMENTED("MemMove Inst not implemented");
}

void AsmJitCodeGen::visitMemTransferInst(llvm::MemTransferInst &I)
{
    ICARUS_NOT_IMPLEMENTED("MemTransfer Inst not implemented");
}

void AsmJitCodeGen::visitMemIntrinsic(llvm::MemIntrinsic &I)
{
    ICARUS_NOT_IMPLEMENTED("Mem Inst not implemented");
}

void AsmJitCodeGen::visitVAStartInst(llvm::VAStartInst &I)
{
    ICARUS_NOT_IMPLEMENTED("VAStart Inst not implemented");
}

void AsmJitCodeGen::visitVAEndInst(llvm::VAEndInst &I)
{
    ICARUS_NOT_IMPLEMENTED("VA End Inst not implemented");
}

void AsmJitCodeGen::visitVACopyInst(llvm::VACopyInst &I)
{
    ICARUS_NOT_IMPLEMENTED("VA Copy Inst not implemented");
}

void AsmJitCodeGen::visitIntrinsicInst(llvm::IntrinsicInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Intrinsic Inst not implemented");
}


// Call and Invoke are slightly different as they delegate first through
// a generic CallSite visitor.
void AsmJitCodeGen::visitCallInst(llvm::CallInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Call Inst not implemented");
}

void AsmJitCodeGen::visitInvokeInst(llvm::InvokeInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Invoke Inst not implemented");
}


// Next level propagators: If the user does not overload a specific
// instruction type, they can overload one of these to get the whole class
// of instructions...
//
void AsmJitCodeGen::visitCastInst(llvm::CastInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Cast Inst not implemented");
}

void AsmJitCodeGen::visitBinaryOperator(llvm::BinaryOperator &I)
{
}

void AsmJitCodeGen::visitCmpInst(llvm::CmpInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Cmp Inst not implemented");
}

void AsmJitCodeGen::visitTerminatorInst(llvm::TerminatorInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Terminator Inst not implemented");
}

void AsmJitCodeGen::visitUnaryInstruction(llvm::UnaryInstruction &I)
{
    ICARUS_NOT_IMPLEMENTED("Unary Inst not implemented");
}


// Provide a special visitor for a 'callsite' that visits both calls and
// invokes. When unimplemented, properly delegates to either the terminator or
// regular instruction visitor.
void AsmJitCodeGen::visitCallSite(llvm::CallSite CS)
{
    ICARUS_NOT_IMPLEMENTED("CallSite Inst not implemented");
}


// If the user wants a 'default' case, they can choose to override this
// function.  If this function is not overloaded in the user's subclass, then
// this instruction just gets ignored.
//
// Note that you MUST override this function if your return type is not void.MipsCodeGen::
//
void AsmJitCodeGen::visitInstruction(llvm::Instruction &I)
{
    I.dump();
    ICARUS_NOT_IMPLEMENTED("Instruction not implemented");
}


void AsmJitCodeGen::emitBrWithCmpInstruction(llvm::BranchInst *pBrInst, llvm::CmpInst *pCmpInst)
{
    ICARUS_NOT_IMPLEMENTED("BrWithCmpInst not implemented");
}