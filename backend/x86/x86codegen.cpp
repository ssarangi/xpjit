#include <common/debug.h>
#include "x86codegen.h"
#include "x86instset.h"
#include "../tempstacksize.h"

#include <common/llvm_warnings_push.h>
#include <common/llvm_warnings_pop.h>

#include <asmjit/asmjit.h>
#include <asmjit/x86/x86assembler.h>

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

char X86CodeGen::ID = 0;

void X86CodeGen::storeTemporary(llvm::Instruction *pI)
{

}

BaseVariable* X86CodeGen::getSymbol(llvm::Value* pV)
{
    BaseVariable *pSymbol = nullptr;

    // Check if the llvm value is a constant int
    if (llvm::Constant *pC = llvm::dyn_cast<llvm::Constant>(pV))
    {
        // So this is a constant. So see which one.
        if (llvm::ConstantInt *pCI = llvm::dyn_cast<llvm::ConstantInt>(pC))
            pSymbol = new Immediate((int)pCI->getZExtValue());
    }
    else if (llvm::Argument *pArg = llvm::dyn_cast<llvm::Argument>(pV))
    {
        TODO("Assign registers in the register allocator. For now just map to variables")

        assert(0 && "Not handled yet");
    }
    else
    {
        // Check if we already have a symbol in the symbol table. If not the create a symbol
        pSymbol = m_pRegAllocator->getSymbol(pV);
    }

    return pSymbol;
}

void X86CodeGen::loadBaseVariable(BaseVariable *pVar, std::ostream &s)
{
}

void X86CodeGen::initializeAssembler()
{
}

void X86CodeGen::destroyAssembler()
{
}

void X86CodeGen::createLabel(llvm::BasicBlock *pBlock)
{
    ICARUS_NOT_IMPLEMENTED("Label Inst not implemented");
}

void X86CodeGen::jit()
{
    typedef int(*FuncType)(void);

    FuncType func = asmjit_cast<FuncType>(m_pAssembler->make());
    func();

    m_pRuntime->release((void*)func);
}

bool X86CodeGen::runOnModule(llvm::Module& M)
{
    m_pX86PatternMatch = &getAnalysis<X86PatternMatch>();

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

        m_pRegAllocator = &getAnalysis<LinearScanAllocator>(*f);
        // m_pTempStackSize = &getAnalysis<TemporaryStackSize>(*f);
        visitFunction(*f);

        x86_PM_FuncBasicBlock* pFuncBlock = m_pX86PatternMatch->getFuncBlock(f);
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

void X86CodeGen::visitFunction(llvm::Function& F)
{
    // Emit the prologue for the function
    m_pAssembler->push(m_pAssembler->zbp);
    m_pAssembler->mov(m_pAssembler->zbp, m_pAssembler->zsp);
    m_pAssembler->sub(m_pAssembler->zsp, 192);
    m_pAssembler->push(m_pAssembler->zbx);
    m_pAssembler->push(m_pAssembler->zsi);
    m_pAssembler->push(m_pAssembler->zdi);

    m_pAssembler->lea(m_pAssembler->zdi, asmjit::X86Mem(m_pAssembler->zbp, 102));
    m_pAssembler->mov(m_pAssembler->zcx, 48);
    m_pAssembler->mov(m_pAssembler->zax, 0xCCCCCCCC);
    m_pAssembler->stosd();
    uint8_t *oldCursor = m_pAssembler->getCursor();

    // Create a new label for the function exit
    asmjit::Label exitLabel = m_pAssembler->newLabel();
    m_pAssembler->pop(m_pAssembler->zdi);
    m_pAssembler->pop(m_pAssembler->zsi);
    m_pAssembler->pop(m_pAssembler->zbx);
    m_pAssembler->mov(m_pAssembler->zsp, m_pAssembler->zbp);
    m_pAssembler->pop(m_pAssembler->zbp);
    m_pAssembler->ret(0);

    // Reset the assembler to go to the previous location
    m_pAssembler->setCursor(oldCursor);
}

void X86CodeGen::visitBasicBlock(llvm::BasicBlock &BB)
{
    // ICARUS_NOT_IMPLEMENTED("Basic Block CodeGen not implemented");
}

void X86CodeGen::visitReturnInst(llvm::ReturnInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Return Inst not implemented");
}

void X86CodeGen::visitBranchInst(llvm::BranchInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Branch Inst not implemented");
}

void X86CodeGen::visitSwitchInst(llvm::SwitchInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Switch Inst not implemented");
}

void X86CodeGen::visitIndirectBrInst(llvm::IndirectBrInst &I)
{
    ICARUS_NOT_IMPLEMENTED("IndirectBrInst Inst not implemented");
}

void X86CodeGen::visitResumeInst(llvm::ResumeInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Resume Inst not implemented");
}

void X86CodeGen::visitUnreachableInst(llvm::UnreachableInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Unreachable Inst not implemented");
}

void X86CodeGen::visitICmpInst(llvm::ICmpInst &I)
{
    ICARUS_NOT_IMPLEMENTED("ICmp Inst not implemented");
}

void X86CodeGen::visitFCmpInst(llvm::FCmpInst &I)
{
    ICARUS_NOT_IMPLEMENTED("FCmp Inst not implemented");
}

void X86CodeGen::visitAllocaInst(llvm::AllocaInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Alloca Inst not implemented");
}

void X86CodeGen::visitLoadInst(llvm::LoadInst     &I)
{
    ICARUS_NOT_IMPLEMENTED("Load Inst not implemented");
}

void X86CodeGen::visitStoreInst(llvm::StoreInst   &I)
{
    ICARUS_NOT_IMPLEMENTED("Store Inst not implemented");
}

void X86CodeGen::visitAtomicCmpXchgInst(llvm::AtomicCmpXchgInst &I)
{
    ICARUS_NOT_IMPLEMENTED("AtomicCmpXchg Inst not implemented");
}

void X86CodeGen::visitAtomicRMWInst(llvm::AtomicRMWInst &I)
{
    ICARUS_NOT_IMPLEMENTED("AtomicRMW Inst not implemented");
}

void X86CodeGen::visitFenceInst(llvm::FenceInst   &I)
{
    ICARUS_NOT_IMPLEMENTED("Fence Inst not implemented");
}

void X86CodeGen::visitGetElementPtrInst(llvm::GetElementPtrInst &I)
{
    ICARUS_NOT_IMPLEMENTED("GEP Inst not implemented");
}

void X86CodeGen::visitPHINode(llvm::PHINode &I)
{
    ICARUS_NOT_IMPLEMENTED("Phi Inst not implemented");
}

void X86CodeGen::visitTruncInst(llvm::TruncInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Trunc Inst not implemented");
}

void X86CodeGen::visitZExtInst(llvm::ZExtInst &I)
{
    ICARUS_NOT_IMPLEMENTED("ZExt Inst not implemented");
}

void X86CodeGen::visitSExtInst(llvm::SExtInst &I)
{
    ICARUS_NOT_IMPLEMENTED("SExt Inst not implemented");
}

void X86CodeGen::visitFPTruncInst(llvm::FPTruncInst &I)
{
    ICARUS_NOT_IMPLEMENTED("FPTrunc Inst not implemented");
}

void X86CodeGen::visitFPExtInst(llvm::FPExtInst &I)
{
    ICARUS_NOT_IMPLEMENTED("FPExt Inst not implemented");
}

void X86CodeGen::visitFPToUIInst(llvm::FPToUIInst &I)
{
    ICARUS_NOT_IMPLEMENTED("FPToUI Inst not implemented");
}

void X86CodeGen::visitFPToSIInst(llvm::FPToSIInst &I)
{
    ICARUS_NOT_IMPLEMENTED("FPToSI Inst not implemented");
}

void X86CodeGen::visitUIToFPInst(llvm::UIToFPInst &I)
{
    ICARUS_NOT_IMPLEMENTED("UIToFP Inst not implemented");
}

void X86CodeGen::visitSIToFPInst(llvm::SIToFPInst &I)
{
    ICARUS_NOT_IMPLEMENTED("SIToFP Inst not implemented");
}

void X86CodeGen::visitPtrToIntInst(llvm::PtrToIntInst &I)
{
    ICARUS_NOT_IMPLEMENTED("PtrToInt Inst not implemented");
}

void X86CodeGen::visitIntToPtrInst(llvm::IntToPtrInst &I)
{
    ICARUS_NOT_IMPLEMENTED("IntToPtr Inst not implemented");
}

void X86CodeGen::visitBitCastInst(llvm::BitCastInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Bitcast Inst not implemented");
}

void X86CodeGen::visitAddrSpaceCastInst(llvm::AddrSpaceCastInst &I)
{
    ICARUS_NOT_IMPLEMENTED("AddrSpaceCast Inst not implemented");
}

void X86CodeGen::visitSelectInst(llvm::SelectInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Select Inst not implemented");
}

void X86CodeGen::visitVAArgInst(llvm::VAArgInst   &I)
{
    ICARUS_NOT_IMPLEMENTED("VAArg Inst not implemented");
}

void X86CodeGen::visitExtractElementInst(llvm::ExtractElementInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Extract Element Inst not implemented");
}

void X86CodeGen::visitInsertElementInst(llvm::InsertElementInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Insert Element Inst not implemented");
}

void X86CodeGen::visitShuffleVectorInst(llvm::ShuffleVectorInst &I)
{
    ICARUS_NOT_IMPLEMENTED("ShuffleVector Inst not implemented");
}

void X86CodeGen::visitExtractValueInst(llvm::ExtractValueInst &I)
{
    ICARUS_NOT_IMPLEMENTED("ExtractValue Inst not implemented");
}

void X86CodeGen::visitInsertValueInst(llvm::InsertValueInst &I)
{
    ICARUS_NOT_IMPLEMENTED("InsertValue Inst not implemented");
}

void X86CodeGen::visitLandingPadInst(llvm::LandingPadInst &I)
{
    ICARUS_NOT_IMPLEMENTED("LandingPad Inst not implemented");
}

// Handle the special intrinsic instruction classes.
void X86CodeGen::visitDbgDeclareInst(llvm::DbgDeclareInst &I)
{
    ICARUS_NOT_IMPLEMENTED("DbgDeclare Inst not implemented");
}

void X86CodeGen::visitDbgValueInst(llvm::DbgValueInst &I)
{
    ICARUS_NOT_IMPLEMENTED("DbgValue Inst not implemented");
}

void X86CodeGen::visitDbgInfoIntrinsic(llvm::DbgInfoIntrinsic &I)
{
    ICARUS_NOT_IMPLEMENTED("DbgInfo Inst not implemented");
}

void X86CodeGen::visitMemSetInst(llvm::MemSetInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Memset Inst not implemented");
}

void X86CodeGen::visitMemCpyInst(llvm::MemCpyInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Memcpy Inst not implemented");
}

void X86CodeGen::visitMemMoveInst(llvm::MemMoveInst &I)
{
    ICARUS_NOT_IMPLEMENTED("MemMove Inst not implemented");
}

void X86CodeGen::visitMemTransferInst(llvm::MemTransferInst &I)
{
    ICARUS_NOT_IMPLEMENTED("MemTransfer Inst not implemented");
}

void X86CodeGen::visitMemIntrinsic(llvm::MemIntrinsic &I)
{
    ICARUS_NOT_IMPLEMENTED("Mem Inst not implemented");
}

void X86CodeGen::visitVAStartInst(llvm::VAStartInst &I)
{
    ICARUS_NOT_IMPLEMENTED("VAStart Inst not implemented");
}

void X86CodeGen::visitVAEndInst(llvm::VAEndInst &I)
{
    ICARUS_NOT_IMPLEMENTED("VA End Inst not implemented");
}

void X86CodeGen::visitVACopyInst(llvm::VACopyInst &I)
{
    ICARUS_NOT_IMPLEMENTED("VA Copy Inst not implemented");
}

void X86CodeGen::visitIntrinsicInst(llvm::IntrinsicInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Intrinsic Inst not implemented");
}


// Call and Invoke are slightly different as they delegate first through
// a generic CallSite visitor.
void X86CodeGen::visitCallInst(llvm::CallInst &I)
{
    
}

void X86CodeGen::visitInvokeInst(llvm::InvokeInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Invoke Inst not implemented");
}


// Next level propagators: If the user does not overload a specific
// instruction type, they can overload one of these to get the whole class
// of instructions...
//
void X86CodeGen::visitCastInst(llvm::CastInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Cast Inst not implemented");
}

void X86CodeGen::visitBinaryOperator(llvm::BinaryOperator &I)
{
    ICARUS_NOT_IMPLEMENTED("Binary Operator Inst not implemented");
}

void X86CodeGen::visitCmpInst(llvm::CmpInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Cmp Inst not implemented");
}

void X86CodeGen::visitTerminatorInst(llvm::TerminatorInst &I)
{
    ICARUS_NOT_IMPLEMENTED("Terminator Inst not implemented");
}

void X86CodeGen::visitUnaryInstruction(llvm::UnaryInstruction &I)
{
    ICARUS_NOT_IMPLEMENTED("Unary Inst not implemented");
}


// Provide a special visitor for a 'callsite' that visits both calls and
// invokes. When unimplemented, properly delegates to either the terminator or
// regular instruction visitor.
void X86CodeGen::visitCallSite(llvm::CallSite CS)
{
    ICARUS_NOT_IMPLEMENTED("CallSite Inst not implemented");
}


// If the user wants a 'default' case, they can choose to override this
// function.  If this function is not overloaded in the user's subclass, then
// this instruction just gets ignored.
//
// Note that you MUST override this function if your return type is not void.MipsCodeGen::
//
void X86CodeGen::visitInstruction(llvm::Instruction &I)
{
    I.dump();
    ICARUS_NOT_IMPLEMENTED("Instruction not implemented");
}


void X86CodeGen::emitBrWithCmpInstruction(llvm::BranchInst *pBrInst, llvm::CmpInst *pCmpInst)
{
}