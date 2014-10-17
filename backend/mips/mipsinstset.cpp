#include "mipsinstset.h"
#include <backend/backendvar.h>

/*
    MIPS Instructions
*/

#define WORD_SIZE 4

#define SW " sw "
#define LW " lw "
#define LI " li "
#define LA " la "

#define MOV " mov "
#define NEG " neg "
#define ADD " ADD "
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

void MipsInstSet::emitLoad(Register *pReg, int offset, Register *pSrcReg, std::ostream& s)
{
    s << LW << *pReg << " " << offset * WORD_SIZE << "(" << *pSrcReg << ")" << std::endl;
}