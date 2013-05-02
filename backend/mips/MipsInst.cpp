#include "MipsInst.h"

#include <assert.h>

std::ostream & operator<<(std::ostream& os, const Register& reg)
{
#define CASE( E ) case E: return os << #E
    switch(reg.get_register_name())
    {
        CASE(A0);
        CASE(SP);
    default:
        assert(0 && "Invalid register");
    }

    return os << "Null";
}

Register::Register(RegisterName regName)
    : m_regName(regName)
{
}

Register::~Register()
{
}