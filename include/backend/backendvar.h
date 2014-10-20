#ifndef __BACKEND_VAR__
#define __BACKEND_VAR__

#include <ostream>

enum VarType
{
    BEGIN,
    BACKEND_VARIABLE,
    IMMEDIATE,
    REGISTER,
    END,
};

enum DataType
{
    I32,
    U32,
    F32,
};

#define setClassInstance(varType) \
    virtual VarType getClassInstance() { return varType; } \
    virtual bool    isInstanceOf(VarType type) { return varType == type; }

#define setClassInstance1(varType, parent) \
    virtual VarType getClassInstance() { return varType; } \
    virtual bool    isInstanceOf(VarType type) { return ((varType == type) ? true : parent::isInstanceOf(type)); }

#define setClassInstance2(varType, parent1, parent2) \
    virtual VarType getClassInstance() { return varType; } \
    virtual bool    isInstanceOf(VarType type) { return ((varType == type) ? true : (parent1::isInstanceOf(type) || parent2::isInstanceOf(type))); }

class BaseVariable
{
public:
    setClassInstance(VarType::BACKEND_VARIABLE);
};

class Immediate : public BaseVariable
{
public:
    Immediate(unsigned int u32)
    {
        m_data.u32 = u32;
        m_dataType = U32;
    }

    Immediate(int i32)
    {
        m_data.i32 = i32;
        m_dataType = I32;
    }

    Immediate(float f32)
    {
        m_data.f32 = f32;
        m_dataType = F32;
    }

    setClassInstance1(VarType::IMMEDIATE, BaseVariable);

    friend std::ostream& operator<<(std::ostream& out, const Immediate& imm)
    {
        if (imm.m_dataType == I32)
            out << imm.m_data.i32;
        else if (imm.m_dataType == U32)
            out << imm.m_data.u32;
        else
            out << imm.m_data.f32;
        return out;
    }

private:
    union data
    {
        unsigned int u32;
        int          i32;
        float        f32;
    };

    data m_data;
    DataType m_dataType;

public:
    data getData() { return m_data; }
    
    template <typename M>
    M getValue()
    {
        if (m_dataType == I32)
            return m_data.i32;
        else if (m_dataType == F32)
            return m_data.f32;
        else
            return m_data.u32;
    }
};

class MipsRegister : public BaseVariable
{
public:
    MipsRegister(std::string regName)
        : m_regName(regName)
    {}

    setClassInstance1(VarType::REGISTER, BaseVariable);

    friend std::ostream& operator<<(std::ostream& out, const MipsRegister& reg)
    {
        out << reg.m_regName.c_str();
        return out;
    }

private:
    std::string m_regName;
};

#endif