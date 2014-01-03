#include <backend/codegenpublic.h>
#include "backendcodegen.h"

#include <llvm/PassManager.h>

#include <iostream>

char CodeGenPass::ID = 0;

bool CodeGenPass::runOnFunction(llvm::Function& F)
{
    std::cout << "------------------ Dump --------------------------" << std::endl;
    F.dump();
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