#ifndef GENLLVM_H
#define GENLLVM_H

#include <frontend/IClassVisitor.h>
#include <frontend/Type.h>

#include <common/llvm_warnings_push.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <common/llvm_warnings_pop.h>

#include <map>
#include <string>

class GenLLVM 
{
public:
    GenLLVM();
    void generateLLVM(IcarusModule &);
    llvm::IRBuilder<>& getBuilder() { return m_irBuilder; }
    llvm::Module& getModule() { return m_module; }
    std::map<std::string, llvm::Value*>& getNamedValues() { return m_namedValues; }
    llvm::Type* getLLVMType(Type& type);

private:
    llvm::IRBuilder<>& m_irBuilder;
    llvm::Module& m_module;	
    std::map<std::string, llvm::Value*> m_namedValues; //to hold the temp allocations in the function
};

#endif //GENLLVM_H
