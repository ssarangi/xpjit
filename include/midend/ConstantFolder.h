#ifndef CONSTANT_FOLDER_H
#define CONSTANT_FOLDER_H

#include "frontend/irtranslation.h"

#include <llvm/Pass.h>
#include <llvm/IR/Function.h>
#include <llvm/Support/raw_ostream.h>

using llvm::FunctionPass;

class ConstantFolder : public FunctionPass 
{
public:
    static char ID;
    ConstantFolder() 
        : FunctionPass(ID) 
    {}

    virtual bool doInitialization(llvm::Module &M) { return true; }
    virtual bool runOnFunction(llvm::Function &F);
    virtual bool doFinalization(llvm::Module &M) { return true; }
};

#endif //CONSTANT_FOLDER_H
