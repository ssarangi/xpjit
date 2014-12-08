#ifndef __MIPS_INST_SET__
#define __MIPS_INST_SET__

#include <backend/backendvar.h>
#include "x86defines.h"

#include "common/llvm_warnings_push.h"
#include <llvm/IR/InstrTypes.h>
#include "common/llvm_warnings_pop.h"

#include <ostream>

class X86InstSet
{
public:
    X86InstSet() {}
    ~X86InstSet() {}
    
    static void emitLoad(BackendRegister &dstReg, int offset, BackendRegister &srcReg, std::ostream &s);
    static void emitStore(BackendRegister &srcReg, int offset, BackendRegister &dstReg, std::ostream &s);
    
    static void emitLoadImm(BackendRegister &dstReg, Immediate &imm, std::ostream &s);
    static void emitLoadImm(BackendRegister &dstReg, int imm, std::ostream &s);

    static void emitLoadAddress(BackendRegister &dstReg, char* pAddress, std::ostream &s);
    // static void emitLoadBool();
    // static void emitLoadString();
    static void emitMov(BaseVariable &dstReg, BaseVariable &srcReg, std::ostream &s);

    static void emitAddiu(BackendRegister &dstReg, BackendRegister &srcReg, int imm, std::ostream &s);

    static void emitAdd(BaseVariable &srcReg1, BaseVariable &srcReg2, std::ostream &s);
    static void emitSub(BaseVariable &srcReg1, BaseVariable &srcReg2, std::ostream &s);
    static void emitMul(BaseVariable &srcReg1, BaseVariable &srcReg2, std::ostream &s);
    static void emitDiv(BaseVariable &srcReg1, BaseVariable &srcReg2, std::ostream &s);

    static void emitPush(BackendRegister &reg, std::ostream &s);
    static void emitPush(BackendRegister &reg, int offset, std::ostream &s);
    static void emitPop(BackendRegister &reg, std::ostream &s);
    static void emitPop(BackendRegister &reg, int offset, std::ostream &s);
    static void emitSyscall(X86_SYSCALLS syscall_code, std::ostream &s);

    static void emitBEQ(BackendRegister &lhs, BackendRegister &rhs, std::string label, std::ostream &s);

    static void emitJ(std::string label, std::ostream &s);
    static void emitJR(BackendRegister &reg, std::ostream &s);
    static void emitJAL(std::string label, std::ostream &s);

    static void emitComment(std::string comment, std::ostream &s);

    static std::string getBrCmpPredicateString(llvm::CmpInst::Predicate predicate);
};

#endif