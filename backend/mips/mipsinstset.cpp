#include "mipsinstset.h"
#include <backend/backendvar.h>

#include <assert.h>

std::string MipsInstSet::getBrCmpPredicateString(llvm::CmpInst::Predicate predicate)
{
    std::string predicate_str = "";
    switch (predicate)
    {
    case llvm::CmpInst::ICMP_EQ: return "beq";
    default: assert("Predicate enum not found");
    }

    return predicate_str;
}

void MipsInstSet::emitLoad(BackendRegister &dstReg, int offset, BackendRegister &srcReg, std::ostream& s)
{
    s << LW << dstReg << " " << offset << "(" << srcReg << ")" << std::endl;
}

void MipsInstSet::emitStore(BackendRegister &srcReg, int offset, BackendRegister &dstReg, std::ostream& s)
{
    s << SW << srcReg << " " << offset << "(" << dstReg << ")" << std::endl;
}

void MipsInstSet::emitLoadImm(BackendRegister &dstReg, Immediate &imm, std::ostream& s)
{
    s << LI << dstReg << " " << imm << std::endl;
}

void MipsInstSet::emitLoadImm(BackendRegister &dstReg, int imm, std::ostream &s)
{
    s << LI << dstReg << " " << imm << std::endl;
}

void MipsInstSet::emitLoadAddress(BackendRegister &dstReg, char* pAddress, std::ostream& s)
{

}

void MipsInstSet::emitAddiu(BackendRegister &dstReg, BackendRegister &srcReg, int imm, std::ostream& s)
{
    s << ADDIU << dstReg << " " << srcReg << " " << imm << std::endl;
}

void MipsInstSet::emitMove(BackendRegister &dstReg, BackendRegister &srcReg, std::ostream &s)
{
    s << MOVE << dstReg << " " << srcReg << std::endl;
}

void MipsInstSet::emitPush(BackendRegister &reg, std::ostream& s)
{
    emitStore(reg, 0, SP, s);
    emitAddiu(SP, SP, -4, s);
}

void MipsInstSet::emitPush(BackendRegister &reg, int offset, std::ostream &s)
{
    assert(offset < 0);
    emitStore(reg, 0, SP, s);
    emitAddiu(SP, SP, offset, s);
}

void MipsInstSet::emitPop(BackendRegister &reg, std::ostream &s)
{
    emitAddiu(reg, reg, 4, s);
}

void MipsInstSet::emitPop(BackendRegister &reg, int offset, std::ostream &s)
{
    emitAddiu(reg, reg, offset, s);
}

void MipsInstSet::emitSyscall(MIPS_SYSCALLS syscall_code, std::ostream &s)
{
    emitLoadImm(V0, (int)syscall_code, s);
    s << "syscall" << std::endl;
}

void MipsInstSet::emitBEQ(BackendRegister &lhs, BackendRegister &rhs, std::string label, std::ostream &s)
{
    s << " beq " << lhs << " " << rhs << " " << label << std::endl;
}

void MipsInstSet::emitJ(std::string label, std::ostream &s)
{
    s << " j " << label << std::endl;
}

void MipsInstSet::emitJR(BackendRegister &reg, std::ostream &s)
{
    s << JR << reg << std::endl;
}

void MipsInstSet::emitJAL(std::string label, std::ostream &s)
{
    s << JAL << label.c_str() << std::endl;
}

void MipsInstSet::emitAdd(BackendRegister &dstReg, BackendRegister &srcReg1, BackendRegister &srcReg2, std::ostream &s)
{
    s << ADD << dstReg << " " << srcReg1 << " " << srcReg2 << std::endl;
}

void MipsInstSet::emitSub(BackendRegister &dstReg, BackendRegister &srcReg1, BackendRegister &srcReg2, std::ostream &s)
{
    s << SUB << dstReg << " " << srcReg1 << " " << srcReg2 << std::endl;
}

void MipsInstSet::emitMul(BackendRegister &dstReg, BackendRegister &srcReg1, BackendRegister &srcReg2, std::ostream &s)
{
    s << MUL << dstReg << " " << srcReg1 << " " << srcReg2 << std::endl;
}

void MipsInstSet::emitDiv(BackendRegister &dstReg, BackendRegister &srcReg1, BackendRegister &srcReg2, std::ostream &s)
{
    s << DIV << dstReg << " " << srcReg1 << " " << srcReg2 << std::endl;
}
