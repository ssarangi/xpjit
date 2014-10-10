#ifndef __MIPS_INST_SET__
#define __MIPS_INST_SET__

#include <ostream>

class MipsInstSet
{
public:
    MipsInstSet() {}
    ~MipsInstSet() {}
    
    void emitLoadWord();
    void emitAdd();
    void emitStoreWord();
    void emitAddiu();
    void emitLoadImmediate();
};

#endif