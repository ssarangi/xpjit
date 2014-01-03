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

    friend std::ostream &operator<<(std::ostream &output, 
                                    const A0Reg &reg )
    { 
        output << "A0";
        return output;
    }
};

#endif