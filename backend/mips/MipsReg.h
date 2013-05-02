#ifndef MIPS_REG
#define MIPS_REG

#include <llvm/Support/raw_ostream.h>

enum RegisterName
{
    A0 = 0, /// Accumulator
    SP,
};

class Register final
{
public:
    Register(RegisterName regName);
    ~Register();
    friend std::ostream& operator<<(std::ostream& os, const Register& reg);
    RegisterName get_register_name() const { return m_regName; }

private:
    RegisterName m_regName;
};

#endif