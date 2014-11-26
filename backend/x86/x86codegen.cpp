#include <common/debug.h>
#include "x86codegen.h"
#include "x86instset.h"
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
    m_ostream << "# " << os.str() << std::endl; \
}

char X86CodeGen::ID = 0;

void X86CodeGen::storeTemporary(llvm::Instruction *pI)
{

}

BaseVariable* X86CodeGen::getSymbol(llvm::Value* pV)
{
    BaseVariable *pSymbol = nullptr;
    // Check if we already have a symbol in the symbol table. If not the create a symbol
    if (m_symbolTables[m_pCurrentFunction]->get(pV) != nullptr)
    {
        pSymbol = m_symbolTables[m_pCurrentFunction]->get(pV);
        return pSymbol;
    }

    // Check if the llvm value is a constant int
    if (llvm::Constant *pC = llvm::dyn_cast<llvm::Constant>(pV))
    {
        // So this is a constant. So see which one.
        if (llvm::ConstantInt *pCI = llvm::dyn_cast<llvm::ConstantInt>(pC))
        {
            pSymbol = new Immediate((int)pCI->getZExtValue());
            m_symbolTables[m_pCurrentFunction]->set(pV, pSymbol);
        }
    }

    return pSymbol;
}

void X86CodeGen::loadBaseVariable(BaseVariable *pVar, std::ostream &s)
{

}

void X86CodeGen::initializeAssembler()
{
    // First output the data section
    m_ostream << ".data" << std::endl;
    m_ostream << std::endl;
    m_ostream << ".text" << std::endl;
    m_ostream << ".globl main_entry" << std::endl;
    m_ostream << std::endl;
    X86InstSet::emitJ("main_entry", m_ostream);
}

void X86CodeGen::createLabel(llvm::BasicBlock *pBlock)
{
    std::string label = pBlock->getParent()->getName().str() + "_" + pBlock->getName().str();

    m_ostream << label << ":" << std::endl;
}

bool X86CodeGen::runOnModule(llvm::Module& M)
{
    m_pX86PatternMatch = &getAnalysis<X86PatternMatch>();

    initializeAssembler();

    for (llvm::Module::iterator f = M.begin(), fe = M.end(); f != fe; ++f)
    {
        m_pTempStackSize = &getAnalysis<TemporaryStackSize>(*f);
        visitFunction(*f);

        PM_FuncBasicBlock* pFuncBlock = m_pX86PatternMatch->getFuncBlock(f);
        pFuncBlock->m_pFunction = f;

        for (unsigned int i = 0; i < pFuncBlock->m_blocks.size(); ++i)
        {
            PM_BasicBlock* pBlock = pFuncBlock->m_blocks[i];

            // Create a label for this id
            if (pBlock->getName() != "entry")
                createLabel(pBlock->pBB);

            for (auto i = pBlock->m_dags.rbegin(), e = pBlock->m_dags.rend(); i != e; ++i)
            {
                i->m_pPattern->emit(this);
            }
        }
    }

    return false;
}

void X86CodeGen::visitFunction(llvm::Function& F)
{

}

void X86CodeGen::visitBasicBlock(llvm::BasicBlock &BB)
{

}

void X86CodeGen::visitReturnInst(llvm::ReturnInst &I)
{

}

void X86CodeGen::visitBranchInst(llvm::BranchInst &I)
{
    //std::string label = m_pCurrentFunction->getName().str() + I.getName().str();
    //X86InstSet::emitJAL(label, m_ostream);
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