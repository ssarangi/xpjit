#ifndef __MIPS_INST__
#define __MIPS_INST__

#include <ostream>

class MipsInst
{
public:
    MipsInst() {}
    ~MipsInst() {}

    virtual std::ostream& operator<<(std::ostream& stream) = 0;
};


#endif