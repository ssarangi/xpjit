#ifndef __X86_DEFINES__
#define __X86_DEFINES__

#include <backend/backendvar.h>

/*
X86 Instructions
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

#endif