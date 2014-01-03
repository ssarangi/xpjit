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
        stream << "$a0";
        return stream;
    }
};

class T1Reg : public MipsReg
{
    std::ostream& operator<<(std::ostream& stream)
    {
        stream << "$t1";
        return stream;
    }
};

class SPReg : public MipsReg
{
    std::ostream& operator<<(std::ostream& stream)
    {
        stream << "$sp";
        return stream;
    }
};

class MipsRegMgr
{

};

#endif