#include "X86InstSet.h"
#include <backend/backendvar.h>

#include <assert.h>

std::string X86InstSet::getBrCmpPredicateString(llvm::CmpInst::Predicate predicate)
{
    std::string predicate_str = "";
    switch (predicate)
    {
    case llvm::CmpInst::ICMP_EQ: return "beq";
    default: assert("Predicate enum not found");
    }

    return predicate_str;
}

void X86InstSet::emitLoad(BackendRegister &dstReg, int offset, BackendRegister &srcReg, std::ostream& s)
{
    s << LW << dstReg << " " << offset << "(" << srcReg << ")" << std::endl;
}

void X86InstSet::emitStore(BackendRegister &srcReg, int offset, BackendRegister &dstReg, std::ostream& s)
{
    s << SW << srcReg << " " << offset << "(" << dstReg << ")" << std::endl;
}

void X86InstSet::emitLoadImm(BackendRegister &dstReg, Immediate &imm, std::ostream& s)
{
    s << LI << dstReg << " " << imm << std::endl;
}

void X86InstSet::emitLoadImm(BackendRegister &dstReg, int imm, std::ostream &s)
{
    s << LI << dstReg << " " << imm << std::endl;
}

void X86InstSet::emitLoadAddress(BackendRegister &dstReg, char* pAddress, std::ostream& s)
{

}

void X86InstSet::emitAddiu(BackendRegister &dstReg, BackendRegister &srcReg, int imm, std::ostream& s)
{
    s << ADDIU << dstReg << " " << srcReg << " " << imm << std::endl;
}

void X86InstSet::emitMove(BackendRegister &dstReg, BackendRegister &srcReg, std::ostream &s)
{
    s << MOVE << dstReg << " " << srcReg << std::endl;
}

void X86InstSet::emitPush(BackendRegister &reg, std::ostream& s)
{

}

void X86InstSet::emitPush(BackendRegister &reg, int offset, std::ostream &s)
{

}

void X86InstSet::emitPop(BackendRegister &reg, std::ostream &s)
{
}

void X86InstSet::emitPop(BackendRegister &reg, int offset, std::ostream &s)
{
}

void X86InstSet::emitSyscall(MIPS_SYSCALLS syscall_code, std::ostream &s)
{

}

void X86InstSet::emitBEQ(BackendRegister &lhs, BackendRegister &rhs, std::string label, std::ostream &s)
{
    s << " beq " << lhs << " " << rhs << " " << label << std::endl;
}

void X86InstSet::emitJ(std::string label, std::ostream &s)
{
    s << " j " << label << std::endl;
}

void X86InstSet::emitJR(BackendRegister &reg, std::ostream &s)
{
    s << JR << reg << std::endl;
}

void X86InstSet::emitJAL(std::string label, std::ostream &s)
{
    s << JAL << label.c_str() << std::endl;
}

void X86InstSet::emitAdd(BackendRegister &dstReg, BackendRegister &srcReg1, BackendRegister &srcReg2, std::ostream &s)
{
    s << ADD << dstReg << " " << srcReg1 << " " << srcReg2 << std::endl;
}

void X86InstSet::emitSub(BackendRegister &dstReg, BackendRegister &srcReg1, BackendRegister &srcReg2, std::ostream &s)
{
    s << SUB << dstReg << " " << srcReg1 << " " << srcReg2 << std::endl;
}

void X86InstSet::emitMul(BackendRegister &dstReg, BackendRegister &srcReg1, BackendRegister &srcReg2, std::ostream &s)
{
    s << MUL << dstReg << " " << srcReg1 << " " << srcReg2 << std::endl;
}

void X86InstSet::emitDiv(BackendRegister &dstReg, BackendRegister &srcReg1, BackendRegister &srcReg2, std::ostream &s)
{
    s << DIV << dstReg << " " << srcReg1 << " " << srcReg2 << std::endl;
}
