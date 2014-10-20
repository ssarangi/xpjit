#include "mipsinstset.h"
#include <backend/backendvar.h>

void MipsInstSet::emitLoad(MipsRegister &dstReg, int offset, MipsRegister &srcReg, std::ostream& s)
{
    s << LW << dstReg << " " << offset << "(" << srcReg << ")" << std::endl;
}

void MipsInstSet::emitStore(MipsRegister &srcReg, int offset, MipsRegister &dstReg, std::ostream& s)
{
    s << SW << srcReg << " " << offset << "(" << dstReg << ")" << std::endl;
}

void MipsInstSet::emitLoadImm(MipsRegister &dstReg, Immediate &imm, std::ostream& s)
{
    s << LI << dstReg << " " << imm << std::endl;
}

void MipsInstSet::emitLoadImm(MipsRegister &dstReg, int imm, std::ostream &s)
{
    s << LI << dstReg << " " << imm << std::endl;
}

void MipsInstSet::emitLoadAddress(MipsRegister &dstReg, char* pAddress, std::ostream& s)
{

}

void MipsInstSet::emitAddiu(MipsRegister &dstReg, MipsRegister &srcReg, int imm, std::ostream& s)
{
    s << ADDIU << dstReg << " " << srcReg << " " << imm << std::endl;
}

void MipsInstSet::emitMove(MipsRegister &dstReg, MipsRegister &srcReg, std::ostream &s)
{
    s << MOVE << dstReg << " " << srcReg << std::endl;
}

void MipsInstSet::emitPush(MipsRegister &reg, std::ostream& s)
{
    emitStore(reg, 0, SP, s);
    emitAddiu(SP, SP, -4, s);
}

void MipsInstSet::emitPop(MipsRegister &reg, std::ostream &s)
{
    emitAddiu(SP, SP, 4, s);
}

void MipsInstSet::emitSyscall(MIPS_SYSCALLS syscall_code, std::ostream &s)
{
    emitLoadImm(V0, (int)syscall_code, s);
    s << "syscall" << std::endl;
}

void MipsInstSet::emitAdd(MipsRegister &dstReg, MipsRegister &srcReg1, MipsRegister &srcReg2, std::ostream &s)
{
    s << ADD << dstReg << " " << srcReg1 << " " << srcReg2 << std::endl;
}

void MipsInstSet::emitSub(MipsRegister &dstReg, MipsRegister &srcReg1, MipsRegister &srcReg2, std::ostream &s)
{
    s << SUB << dstReg << " " << srcReg1 << " " << srcReg2 << std::endl;
}

void MipsInstSet::emitMul(MipsRegister &dstReg, MipsRegister &srcReg1, MipsRegister &srcReg2, std::ostream &s)
{
    s << MUL << dstReg << " " << srcReg1 << " " << srcReg2 << std::endl;
}

void MipsInstSet::emitDiv(MipsRegister &dstReg, MipsRegister &srcReg1, MipsRegister &srcReg2, std::ostream &s)
{
    s << DIV << dstReg << " " << srcReg1 << " " << srcReg2 << std::endl;
}
