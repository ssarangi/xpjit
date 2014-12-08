#ifndef __MIPS_INST_SET__
#define __MIPS_INST_SET__

#include <backend/backendvar.h>

#include "common/llvm_warnings_push.h"
#include <llvm/IR/InstrTypes.h>
#include "common/llvm_warnings_pop.h"

#include <ostream>

/*
MIPS Instructions
*/

#define WORD_SIZE 4

namespace MIPS
{
    const std::string SW = " sw ";
    const std::string LW = " lw ";
    const std::string LI = " li ";
    const std::string LA = " la ";

    const std::string MOV = " mov ";
    const std::string NEG = " neg ";
    const std::string ADD = " add ";
    const std::string ADDI = " addi ";
    const std::string ADDIU = " addiu ";
    const std::string DIV = " div ";
    const std::string MUL = " mul ";
    const std::string SUB = " sub ";
    const std::string SLL = " sll ";
    const std::string BEQZ = " beqz ";
    const std::string BR = " br ";
    const std::string BEQ = " beq ";
    const std::string BNE = " bne ";
    const std::string BLEQ = " bleq ";
    const std::string BLT = " blt ";
    const std::string BGT = " bgt ";

    const std::string JR = " jr ";
    const std::string JAL = " jal ";
}

enum MIPS_SYSCALLS
{
    PRINT_INT = 1,
    PRINT_FLOAT = 2,
    PRINT_DOUBLE = 3,
    PRINT_STRING = 4,

    READ_INT = 5,
    READ_FLOAT = 6,
    READ_DOUBLE = 7,
    READ_STRING = 8,

    SBRK = 9,
    EXIT = 10,

    PRINT_CHAR = 11,
    READ_CHAR = 12,

    OPEN_FILE = 13,
    READ_FROM_FILE = 14,
    WRITE_TO_FILE = 15,
    CLOSE_FILE = 16,

    EXIT_WITH_RETVAL = 17,
};

extern BackendRegister A0;
extern BackendRegister T1;
extern BackendRegister FP;
extern BackendRegister SP;
extern BackendRegister V0;
extern BackendRegister RA;

class MipsInstSet
{
public:
    MipsInstSet() {}
    ~MipsInstSet() {}
    
    static void emitLoad(BackendRegister &dstReg, int offset, BackendRegister &srcReg, std::ostream &s);
    static void emitStore(BackendRegister &srcReg, int offset, BackendRegister &dstReg, std::ostream &s);
    
    static void emitLoadImm(BackendRegister &dstReg, Immediate &imm, std::ostream &s);
    static void emitLoadImm(BackendRegister &dstReg, int imm, std::ostream &s);

    static void emitLoadAddress(BackendRegister &dstReg, char* pAddress, std::ostream &s);
    // static void emitLoadBool();
    // static void emitLoadString();
    static void emitMove(BackendRegister &dstReg, BackendRegister &srcReg, std::ostream &s);

    static void emitAddiu(BackendRegister &dstReg, BackendRegister &srcReg, int imm, std::ostream &s);

    static void emitAdd(BackendRegister &dstReg, BackendRegister &srcReg1, BackendRegister &srcReg2, std::ostream &s);
    static void emitSub(BackendRegister &dstReg, BackendRegister &srcReg1, BackendRegister &srcReg2, std::ostream &s);
    static void emitMul(BackendRegister &dstReg, BackendRegister &srcReg1, BackendRegister &srcReg2, std::ostream &s);
    static void emitDiv(BackendRegister &dstReg, BackendRegister &srcReg1, BackendRegister &srcReg2, std::ostream &s);

    static void emitPush(BackendRegister &reg, std::ostream &s);
    static void emitPush(BackendRegister &reg, int offset, std::ostream &s);
    static void emitPop(BackendRegister &reg, std::ostream &s);
    static void emitPop(BackendRegister &reg, int offset, std::ostream &s);
    static void emitSyscall(MIPS_SYSCALLS syscall_code, std::ostream &s);

    static void emitBEQ(BackendRegister &lhs, BackendRegister &rhs, std::string label, std::ostream &s);

    static void emitJ(std::string label, std::ostream &s);
    static void emitJR(BackendRegister &reg, std::ostream &s);
    static void emitJAL(std::string label, std::ostream &s);

    static std::string getBrCmpPredicateString(llvm::CmpInst::Predicate predicate);
};

#endif