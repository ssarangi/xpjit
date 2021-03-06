#ifndef __X86_DEFINES__
#define __X86_DEFINES__

#include <backend/backendvar.h>

/*
X86 Instructions
*/

#define WORD_SIZE 4

namespace X86
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
    const std::string MUL = " imul ";
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

enum X86_SYSCALLS
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

// General Registers
extern BackendRegister EAX;
extern BackendRegister EBX;
extern BackendRegister ECX;
extern BackendRegister EDX;

// Segment Registers
extern BackendRegister CS;
extern BackendRegister DS;
extern BackendRegister ES;
extern BackendRegister FS;
extern BackendRegister GS;
extern BackendRegister SS;

// Index and pointers
extern BackendRegister ESI;
extern BackendRegister EDI;
extern BackendRegister EBP;
extern BackendRegister EIP;
extern BackendRegister ESP;

///////////////////////////// X86-64 Registers /////////////////////////////////
extern BackendRegister RAX;
extern BackendRegister RBX;
extern BackendRegister RCX;
extern BackendRegister RDX;

extern BackendRegister RSI;
extern BackendRegister RDI;

extern BackendRegister RSP;
extern BackendRegister RBP;

extern BackendRegister R8;
extern BackendRegister R9;
extern BackendRegister R10;
extern BackendRegister R11;
extern BackendRegister R12;
extern BackendRegister R13;
extern BackendRegister R14;
extern BackendRegister R15;

#endif