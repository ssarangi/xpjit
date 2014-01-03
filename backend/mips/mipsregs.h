#ifndef __MIPS_REGS__
#define __MIPS_REGS__

#include <ostream>

class MipsReg
{
public:
    MipsReg() {}
    ~MipsReg() {}

    virtual std::ostream& operator<<(std::ostream& stream) = 0;
};

class A0Reg : public MipsReg
{
    std::ostream& operator<<(std::ostream& stream)
    {
        stream << "A0";
        return stream;
    }
};

class TReg : public MipsReg
{
    std::ostream& operator<<(std::ostream& stream)
    {
        stream << "T";
        return stream;
    }
};

#endif