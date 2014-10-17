#ifndef __MIPS_INST_SET__
#define __MIPS_INST_SET__

#include <ostream>

class MipsInstSet
{
public:
    MipsInstSet() {}
    ~MipsInstSet() {}
    
    static void emitLoad(Register *pDstReg, int offset, Register *pSrcReg, std::ostream& s);
    static void emitStore(Register *pDstReg, int offset, Register *pSrcReg, std::ostream& s);
    static void emitLoadImm(Register *pDstReg, int val, std::ostream& s);
    static void emitLoadAddress(Register *pDstReg, char* pAddress, std::ostream& s);
    // static void emitLoadBool();
    // static void emitLoadString();
    static void emitMove(Register *pDstReg, Register *pSrcReg, std::ostream& s);
};

#endif