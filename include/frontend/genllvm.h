#ifndef GENLLVM_H
#define GENLLVM_H

#include <frontend/IClassVisitor.h>
#include <frontend/IcaType.h>

#include <common/llvm_warnings_push.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <common/llvm_warnings_pop.h>

#include <map>
#include <string>
#include <vector>

class GenLLVM 
{
public:
    GenLLVM();
    void generateLLVM(IcarusModule &);
    llvm::IRBuilder<>& getBuilder() { return m_irBuilder; }
    llvm::Module& getModule() { return m_module; }
    std::map<std::string, llvm::Value*>& getNamedValues() { return m_namedValues; }
    llvm::Type* getLLVMType(IcaType& type);
    llvm::Type* GenLLVM::getLLVMType(std::vector<IcaType*>& typeList);

private:
    llvm::IRBuilder<>& m_irBuilder;
    llvm::Module& m_module;
    std::map<std::string, llvm::Value*> m_namedValues; //to hold the temp allocations in the function
};

#endif //GENLLVM_H
