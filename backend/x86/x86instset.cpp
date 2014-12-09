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

void X86InstSet::emitPrologue(std::ostream &s)
{
    // First output the data section
    s << "bits 64" << std::endl;
    s << "section .data" << std::endl;
    s << "section .data" << std::endl;
    s << std::endl;
    s << "section .text" << std::endl;
    s << "global main_entry" << std::endl;
    s << std::endl;
}

void X86InstSet::emitFunctionPrologue(std::ostream &s)
{
    // Emit the next part of prologue
    //    push  rdi
    //    sub   rsp, 16
    //    mov   rdi, rsp
    //    mov   ecx, 4
    //    mov   eax, -858993460; ccccccccH
    //    rep   stosd

    s << "push rdi" << std::endl;
    s << "sub rsp, 16" << std::endl;
    s << "mov rdi, rsp" << std::endl;
    s << "mov ecx, 4" << std::endl;
    s << "mov eax, -858993460" << std::endl;
    s << "rep stosd" << std::endl;
}

void X86InstSet::emitEpilogue(std::ostream &s)
{
    /*
    add rsp, 16
    pop rdi
    ret 0
    */
    s << "add rsp, 16" << std::endl;
    s << "pop rdi" << std::endl;
    s << "ret 0" << std::endl;
}

void X86InstSet::emitLoad(BackendRegister &dstReg, int offset, BackendRegister &srcReg, std::ostream& s)
{
    s << X86::LW << dstReg << " " << offset << "(" << srcReg << ")" << std::endl;
}

void X86InstSet::emitStore(BackendRegister &srcReg, int offset, BackendRegister &dstReg, std::ostream& s)
{
    s << X86::SW << srcReg << " " << offset << "(" << dstReg << ")" << std::endl;
}

void X86InstSet::emitLoadImm(BackendRegister &dstReg, Immediate &imm, std::ostream& s)
{
    s << X86::LI << dstReg << " " << imm << std::endl;
}

void X86InstSet::emitLoadImm(BackendRegister &dstReg, int imm, std::ostream &s)
{
    s << X86::LI << dstReg << " " << imm << std::endl;
}

void X86InstSet::emitLoadAddress(BackendRegister &dstReg, char* pAddress, std::ostream& s)
{

}

void X86InstSet::emitAddiu(BackendRegister &dstReg, BackendRegister &srcReg, int imm, std::ostream& s)
{
    s << X86::ADDIU << dstReg << " " << srcReg << " " << imm << std::endl;
}

void X86InstSet::emitMov(BaseVariable &dstReg, BaseVariable &srcReg, std::ostream &s)
{
    s << X86::MOV << dstReg << ", " << srcReg << std::endl;
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

void X86InstSet::emitSyscall(X86_SYSCALLS syscall_code, std::ostream &s)
{

}

void X86InstSet::emitBEQ(BackendRegister &lhs, BackendRegister &rhs, std::string label, std::ostream &s)
{
    // s << " beq " << lhs << " " << rhs << " " << label << std::endl;
}

void X86InstSet::emitJ(std::string label, std::ostream &s)
{
    // s << " j " << label << std::endl;
}

void X86InstSet::emitJR(BackendRegister &reg, std::ostream &s)
{
    s << X86::JR << reg << std::endl;
}

void X86InstSet::emitJAL(std::string label, std::ostream &s)
{
    s << X86::JAL << label.c_str() << std::endl;
}

void X86InstSet::emitAdd(BaseVariable &srcReg1, BaseVariable &srcReg2, std::ostream &s)
{
    s << X86::ADD << srcReg1 << ", " << srcReg2 << std::endl;
}

void X86InstSet::emitSub(BaseVariable &srcReg1, BaseVariable &srcReg2, std::ostream &s)
{
    s << X86::SUB << srcReg1 << ", " << srcReg2 << std::endl;
}

void X86InstSet::emitMul(BaseVariable &srcReg1, BaseVariable &srcReg2, std::ostream &s)
{
    s << X86::MUL << srcReg1 << ", " << srcReg2 << std::endl;
}

void X86InstSet::emitDiv(BaseVariable &srcReg1, BaseVariable &srcReg2, std::ostream &s)
{
    s << X86::DIV << srcReg1 << ", " << srcReg2 << std::endl;
}

void X86InstSet::emitComment(std::string comment, std::ostream &s)
{
    s << "/* " << comment << " */";
}
