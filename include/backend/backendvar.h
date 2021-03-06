#ifndef __BACKEND_VAR__
#define __BACKEND_VAR__

#include <common/debug.h>
#include <ostream>

enum VarType
{
    BEGIN,
    BASE_VARIABLE,
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
    setClassInstance(VarType::BASE_VARIABLE);

    virtual void print(llvm::raw_ostream& out) = 0;

protected:
    virtual void print(std::ostream &out) = 0;

public:
    friend std::ostream& operator<<(std::ostream &str, BaseVariable& data)
    {
        data.print(str);
        return str;
    }
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

protected:
    void print(std::ostream &out)
    {
        if (m_dataType == I32)
            out << m_data.i32;
        else if (m_dataType == U32)
            out << m_data.u32;
        else
            out << m_data.f32;
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

    void print(llvm::raw_ostream& out)
    {
        if (m_dataType == I32)
            out << "Immediate: " << m_data.i32 << "\n";
        else if (m_dataType == F32)
            out << "Immediate: " << m_data.f32 << "\n";
        else
            out << "Immediate: " << m_data.u32 << "\n";
    }
};

class BackendVariable : public BaseVariable
{
public:
    BackendVariable(int location)
        : m_tempLocation(location)
    {}

    setClassInstance1(VarType::BACKEND_VARIABLE, BaseVariable);

    friend std::ostream& operator<<(std::ostream& out, const BackendVariable& var)
    {
        out << var.m_tempLocation;
        return out;
    }

    void print(llvm::raw_ostream& out)
    {
        out << " Stack Spill Loc: " << m_tempLocation << "\n";
    }

    int getTempLocation() const { return m_tempLocation; }

protected:
    void print(std::ostream& out)
    {
        out << " Stack Spill Loc: " << m_tempLocation << "\n";
    }

private:
    int m_tempLocation;
};

class BackendRegister : public BaseVariable
{
public:
    BackendRegister(std::string regName)
        : m_regName(regName)
    {}

    setClassInstance1(VarType::REGISTER, BaseVariable);

    friend std::ostream& operator<<(std::ostream& out, const BackendRegister &reg)
    {
        out << reg.m_regName.c_str();
        return out;
    }

    void print(llvm::raw_ostream& out)
    {
        out << m_regName;
    }

protected:
    void print(std::ostream& out)
    {
        out << m_regName;
    }

private:
    std::string m_regName;
};

#endif