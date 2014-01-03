#include <backend/codegenpublic.h>
#include "backendcodegen.h"

#include <llvm/PassManager.h>

#include <iostream>

char CodeGenPass::ID = 5;

bool CodeGenPass::runOnFunction(llvm::Function& F)
{
    llvm::PostDominatorTree* PDT = nullptr;
    if (!F.isDeclaration())
        PDT = &getAnalysis<llvm::PostDominatorTree>(F);

    return false;
}

bool CodeGenPass::runOnModule(llvm::Module &M)
{
    // Iterate over every function
    for (auto func = M.begin(); func != M.end(); ++func)
    {
        runOnFunction(*func);
    }

    return true;
}


void GenerateCode(CodeGenModule& M)
{
    llvm::PassManager mpm;
    mpm.add(new CodeGenPass());
    mpm.run(*M.getLLVMModule());
}