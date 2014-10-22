#include <common/debug.h>
#include "mipscodegen.h"
#include "mipsinstset.h"
#include "../tempstacksize.h"

#include <common/llvm_warnings_push.h>
#include <common/llvm_warnings_pop.h>

#include <stack>

MipsRegister A0("$a0");
MipsRegister T1("$t1");
MipsRegister FP("$fp");
MipsRegister SP("$sp");
MipsRegister V0("$v0");
MipsRegister RA("$ra");

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

char MipsCodeGen::ID = 0;

void MipsCodeGen::storeTemporary(llvm::Instruction *pI)
{
    // Store the temporary in the specified register.
    // Assumption is that the result is in A0.
    MipsVariable *pMipsVar = new MipsVariable(m_temporaryBytesUsed);
    m_symbolTables[m_pCurrentFunction]->set(pI, pMipsVar);
    MipsInstSet::emitStore(A0, -(m_temporaryBytesUsed), FP, m_ostream);
    m_temporaryBytesUsed += 4;
}

BaseVariable* MipsCodeGen::getSymbol(llvm::Value* pV)
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

void MipsCodeGen::loadBaseVariable(BaseVariable *pVar, std::ostream &s)
{
    if (pVar->isInstanceOf(IMMEDIATE))
    {
        MipsInstSet::emitLoadImm(A0, *(Immediate*)pVar, s);
    }
    else if (pVar->isInstanceOf(BACKEND_VARIABLE))
    {
        MipsInstSet::emitLoad(A0, -(((MipsVariable*)pVar)->getTempLocation()), FP, s);
    }
}

void MipsCodeGen::initializeAssembler(llvm::Function *pMainFunc)
{
    // First output the data section
    m_ostream << ".data" << std::endl;
    m_ostream << std::endl;
    m_ostream << ".text" << std::endl;
    m_ostream << ".globl main_entry" << std::endl;
    m_ostream << std::endl;
}

void MipsCodeGen::createLabel(std::string label)
{
    m_ostream << label << std::endl;
}

bool MipsCodeGen::runOnModule(llvm::Module& M)
{
    m_pMipsPatternMatch = &getAnalysis<MipsPatternMatch>();

    for (llvm::Module::iterator f = M.begin(), fe = M.end(); f != fe; ++f)
    {
        m_pTempStackSize = &getAnalysis<TemporaryStackSize>(*f);
        visitFunction(*f);

        PM_FuncBasicBlock* pFuncBlock = m_pMipsPatternMatch->getFuncBlock(f);
        for (unsigned int i = 0; i < pFuncBlock->m_blocks.size(); ++i)
        {
            PM_BasicBlock* pBlock = pFuncBlock->m_blocks[i];

            // Create a label for this id
            if (pBlock->getName() != "entry")
                createLabel(pBlock->getName());

            for (auto i = pBlock->m_dags.rbegin(), e = pBlock->m_dags.rend(); i != e; ++i)
            {
                i->m_pPattern->emit(this);
            }
        }
    }

    return false;
}

void MipsCodeGen::visitFunction(llvm::Function& F)
{
    std::string comment = std::string("Entry Code: ") + std::string(F.getName().str());
    COMMENT_STR(comment);
    
    m_ostream << F.getName().str() << "_entry:" << std::endl;

    // Initialize the symbol table for this function
    int activationRecordSizeInBytes = F.getArgumentList().size() * 4 + 8;
    m_symbolTables[&F] = new MipsSymbolTable(activationRecordSizeInBytes);
    m_pCurrentFunction = &F;

    // Reset the temporary bytes used
    m_temporaryBytesUsed = 0;
    MipsInstSet::emitMove(FP, SP, m_ostream);
    m_temporaryBytesUsed += 4; // Increment by 4 bytes since FP is stored

    // Now increment the temporaries because the arguments will be stored.
    int numArgs = F.getArgumentList().size();

    std::stack<llvm::Value*> reverse_args;
    for (llvm::Function::arg_iterator ai = F.args().begin(), ae = F.args().end(); ai != ae; ++ai)
    {
        reverse_args.push(ai);
    }

    while (!reverse_args.empty())
    {
        llvm::Value *pArg = reverse_args.top();
        reverse_args.pop();

        // Start from right to left. Also the offsets are negative because we are looking at the opposite direction from fp.
        // i.e. going up onto higher addresses since we are moving from the current frame pointer.
        MipsVariable *pMipsVar = new MipsVariable(-m_temporaryBytesUsed);
        m_symbolTables[m_pCurrentFunction]->set(pArg, pMipsVar);
        m_temporaryBytesUsed += 4;
    }

    COMMENT_STR("Generated Code for RA")
    // MipsInstSet::emitStore(RA, 0, SP, m_ostream);
    MipsInstSet::emitPush(RA, m_ostream);
    m_temporaryBytesUsed += 4; // This is for the RA

    COMMENT_STR("Generated Code for Temporaries")
    // Move the stack pointer by the number of temp variables we need
    int numTemporaries = m_pTempStackSize->getNumTemporaries(&F);
    int stackOffset = numTemporaries * WORD_SIZE;
    MipsInstSet::emitAddiu(SP, SP, -stackOffset, m_ostream);
}

void MipsCodeGen::visitBasicBlock(llvm::BasicBlock &BB)
{
    // ICARUS_NOT_IMPLEMENTED("Basic block code not implemented");
    std::string bb_name = BB.getName();
    if (bb_name == "entry")
        return;

    m_ostream << m_pCurrentFunction->getName().str() << "_" << bb_name << ":" << std::endl;
}

void MipsCodeGen::visitReturnInst(llvm::ReturnInst &I)
{
    // Check if the current function is the main function. If so emit a 
    // different instruction than if its a normal subroutine
    if (I.getParent()->getParent()->getName() == "main")
    {
        MipsInstSet::emitSyscall(PRINT_INT, m_ostream);
        MipsInstSet::emitSyscall(EXIT_WITH_RETVAL, m_ostream);
    }
    else
    {
        // Size of Activation record.
        llvm::Function *pF = I.getParent()->getParent();

        int numTemporaries = m_pTempStackSize->getNumTemporaries(I.getParent()->getParent());
        int stackOffset = numTemporaries * WORD_SIZE;

        // To get to RA we also need to subtract the temporaries that we have defined.
        MipsInstSet::emitLoad(RA, stackOffset + 4, SP, m_ostream);

        int size_of_ar = 4 * pF->getArgumentList().size() + 8 + stackOffset;
        MipsInstSet::emitPop(SP, size_of_ar, m_ostream);
        MipsInstSet::emitLoad(FP, 0, SP, m_ostream);
        MipsInstSet::emitJR(RA, m_ostream);
    }
}

void MipsCodeGen::visitBranchInst(llvm::BranchInst &I)
{
    std::string label = m_pCurrentFunction->getName().str() + I.getName().str();
    MipsInstSet::emitJAL(label, m_ostream);
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
    COMMENT_STR_WITH_INSTR(I);

    MipsInstSet::emitPush(FP, m_ostream);

    llvm::Function *pCalledFunc = I.getCalledFunction();
    int num_args = I.getNumArgOperands();
    for (int i = 0; i < num_args; ++i)
    {
        llvm::Value *pllArg = I.getArgOperand(i);
        BaseVariable *pArg = getSymbol(pllArg);
        loadBaseVariable(pArg, m_ostream);
        MipsInstSet::emitPush(A0, m_ostream);
    }

    std::string label_name = std::string(pCalledFunc->getName()) + std::string("_entry");
    MipsInstSet::emitJAL(label_name, m_ostream);
    storeTemporary(&I);
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
    COMMENT_STR_WITH_INSTR(I);

    llvm::Instruction *pInst = llvm::cast<llvm::Instruction>(&I);

    llvm::Value *pOp1 = I.getOperand(0);
    llvm::Value *pOp2 = I.getOperand(1);

    BaseVariable *pBop1 = getSymbol(pOp1);
    BaseVariable *pBop2 = getSymbol(pOp2);

    I.dump();
    loadBaseVariable(pBop1, m_ostream);
    MipsInstSet::emitPush(A0, m_ostream);

    loadBaseVariable(pBop2, m_ostream);
    MipsInstSet::emitLoad(T1, 4, SP, m_ostream);

    switch (pInst->getOpcode())
    {
    case llvm::Instruction::Add:
    case llvm::Instruction::FAdd:
        MipsInstSet::emitAdd(A0, A0, T1, m_ostream);
        break;
    case llvm::Instruction::Sub:
    case llvm::Instruction::FSub:
        MipsInstSet::emitSub(A0, A0, T1, m_ostream);
        break;
    case llvm::Instruction::Mul:
    case llvm::Instruction::FMul:
        MipsInstSet::emitMul(A0, A0, T1, m_ostream);
        break;
    case llvm::Instruction::FDiv:
        MipsInstSet::emitDiv(A0, A0, T1, m_ostream);
        break;
    }

    MipsInstSet::emitPop(SP, m_ostream);
    storeTemporary(pInst);
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
