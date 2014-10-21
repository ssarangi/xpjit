#ifndef __MIPS_INST_SET__
#define __MIPS_INST_SET__

#include <backend/backendvar.h>

#include <ostream>

/*
MIPS Instructions
*/

#define WORD_SIZE 4

#define SW " sw "
#define LW " lw "
#define LI " li "
#define LA " la "

#define MOVE " move "
#define NEG " neg "
#define ADD " add "
#define ADDI " addi "
#define ADDIU " addiu "
#define DIV " div "
#define MUL " mul "
#define SUB " sub "
#define SLL " sll "
#define BEQZ " beqz "
#define BR " br "
#define BEQ " beq "
#define BNE " bne "
#define BLEQ " bleq "
#define BLT " blt "
#define BGT " bgt "

#define JR " jr "
#define JAL " jal "

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

extern MipsRegister A0;
extern MipsRegister T1;
extern MipsRegister FP;
extern MipsRegister SP;
extern MipsRegister V0;
extern MipsRegister RA;

class MipsInstSet
{
public:
    MipsInstSet() {}
    ~MipsInstSet() {}
    
    static void emitLoad(MipsRegister &dstReg, int offset, MipsRegister &srcReg, std::ostream &s);
    static void emitStore(MipsRegister &srcReg, int offset, MipsRegister &dstReg, std::ostream &s);
    
    static void emitLoadImm(MipsRegister &dstReg, Immediate &imm, std::ostream &s);
    static void emitLoadImm(MipsRegister &dstReg, int imm, std::ostream &s);

    static void emitLoadAddress(MipsRegister &dstReg, char* pAddress, std::ostream &s);
    // static void emitLoadBool();
    // static void emitLoadString();
    static void emitMove(MipsRegister &dstReg, MipsRegister &srcReg, std::ostream &s);

    static void emitAddiu(MipsRegister &dstReg, MipsRegister &srcReg, int imm, std::ostream &s);

    static void emitAdd(MipsRegister &dstReg, MipsRegister &srcReg1, MipsRegister &srcReg2, std::ostream &s);
    static void emitSub(MipsRegister &dstReg, MipsRegister &srcReg1, MipsRegister &srcReg2, std::ostream &s);
    static void emitMul(MipsRegister &dstReg, MipsRegister &srcReg1, MipsRegister &srcReg2, std::ostream &s);
    static void emitDiv(MipsRegister &dstReg, MipsRegister &srcReg1, MipsRegister &srcReg2, std::ostream &s);

    static void emitPush(MipsRegister &reg, std::ostream &s);
    static void emitPush(MipsRegister &reg, int offset, std::ostream &s);
    static void emitPop(MipsRegister &reg, std::ostream &s);
    static void emitPop(MipsRegister &reg, int offset, std::ostream &s);
    static void emitSyscall(MIPS_SYSCALLS syscall_code, std::ostream &s);
    static void emitJR(MipsRegister &reg, std::ostream &s);
    static void emitJAL(std::string label, std::ostream &s);
};

#endif