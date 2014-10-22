#ifndef __ARCH_CODEGEN__
#define __ARCH_CODEGEN__

#include <backend/backendvar.h>

#include <common/llvm_warnings_push.h>
#include <llvm/IR/InstVisitor.h>
#include <common/llvm_warnings_pop.h>

#include <sstream>

class TemporaryStackSize;

struct MipsSymbolTable
{
public:
    MipsSymbolTable(int activtionRecordSizeInBytes)
        : m_activationRecordSizeInBytes(activtionRecordSizeInBytes)
    {}

    ~MipsSymbolTable()
    {
        for (auto pBaseVar : m_symbols)
        {
            delete pBaseVar.second;
        }

        m_symbols.clear();
    }

    BaseVariable* get(llvm::Value *pVal)
    {
        if (m_symbols.find(pVal) == m_symbols.end())
            return nullptr;

        return m_symbols[pVal];
    }

    void set(llvm::Value *pVal, BaseVariable *pBaseVar)
    {
        m_symbols[pVal] = pBaseVar;
    }

    void setActivationRecordSize(int size) { m_activationRecordSizeInBytes = size; }
    int getActivationRecordSize() { return m_activationRecordSizeInBytes; }

private:
    llvm::DenseMap<llvm::Value*, BaseVariable*> m_symbols;
    int m_activationRecordSizeInBytes;
};

class IArchCodeGen
{
public:
    IArchCodeGen()
        : m_pCurrentFunction(nullptr)
        , m_pTempStackSize(nullptr)
    {
    }

    ~IArchCodeGen()
    {
        for (auto keyvalpair : m_symbolTables)
        {
            delete keyvalpair.second;
        }

        m_symbolTables.clear();
    }

    virtual void initializeAssembler(llvm::Function *pMainFunc) = 0;
    virtual void createLabel(std::string label) = 0;

    virtual BaseVariable* getSymbol(llvm::Value *pV) = 0;

    std::string getAssembly()
    {
        return m_ostream.str();
    }

protected:
    std::stringstream m_ostream;
    llvm::DenseMap<llvm::Function*, MipsSymbolTable*> m_symbolTables;
    llvm::Function *m_pCurrentFunction;
    TemporaryStackSize *m_pTempStackSize;
};

#endif