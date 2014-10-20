#ifndef __TEMPORARY_STACK_SIZE__
#define __TEMPORARY_STACK_SIZE__

#include <common/llvm_warnings_push.h>
#include <llvm/Pass.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/ADT/DenseMap.h>
#include <common/llvm_warnings_pop.h>

using llvm::FunctionPass;

class TemporaryStackSize : public llvm::FunctionPass
{
public:
    static char ID;
    TemporaryStackSize()
        : FunctionPass(ID)
    {}

    virtual bool runOnFunction(llvm::Function &F);
    int getNumTemporaries(llvm::Function *pFunc);

private:
    llvm::DenseMap<llvm::Function*, unsigned int> m_numTemporaries;
};

#endif