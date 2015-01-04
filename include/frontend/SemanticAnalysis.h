#ifndef __SEMANTIC_ANALYSIS__
#define __SEMANTIC_ANALYSIS__

#include "frontend/irtranslation.h"

#include <common/llvm_warnings_push.h>
#include <llvm/Pass.h>
#include <llvm/IR/Function.h>
#include <llvm/Support/raw_ostream.h>
#include <common/llvm_warnings_pop.h>

class SemanticAnalysis : public llvm::ModulePass
{
public:
    static char ID;
    SemanticAnalysis()
        : llvm::ModulePass(ID)
    {}

    virtual bool doInitialization(llvm::Module &M) { return true; }
    virtual bool runOnModule(llvm::Module &M);
    virtual bool doFinalization(llvm::Module &M) { return true; }

private:
    bool isMainFunctionPresent(llvm::Module &M);
};


#endif