#ifndef __CODEGEN_PUBLIC__
#define __CODEGEN_PUBLIC__

#include <string>

namespace llvm
{
    class Function;
    class Module;
}

class CodeGenModule
{
public:
    CodeGenModule() {}
    ~CodeGenModule() {}

    void setLLVMEntryFunction(llvm::Function* pEntryFunc) { m_pEntryFunc = pEntryFunc; }
    void setLLVMModule(llvm::Module* pModule) { m_pModule = pModule; }

    llvm::Function* getLLVMEntryFunction() { return m_pEntryFunc; }
    llvm::Module* getLLVMModule() { return m_pModule; }

private:
    llvm::Function* m_pEntryFunc;
    llvm::Module*   m_pModule;
};

void GenerateCode(CodeGenModule& M, std::string outputFile);

#endif