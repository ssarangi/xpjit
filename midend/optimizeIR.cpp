#include "midend/optimizeIR.h"

#include "midend/ConstantFolder.h"
#include "midend/DominanceTreeConstructor.h"
#include "midend/DominanceFrontier.h"

#include <llvm/PassManager.h>
#include <llvm/Pass.h>
#include <llvm/Transforms/Scalar.h>

void OptimizeIR(llvm::Module& llvmModule)
{
    llvm::PassManager* pPassMgr;

    llvm::PassManager passMgr;
    //Analysis Passes
    passMgr.add(new DominanceTreeConstructor());
    passMgr.add(new DominanceFrontier());

    //Optimization Passes
    passMgr.add(new ConstantFolder());
    passMgr.add(llvm::createPromoteMemoryToRegisterPass());
    passMgr.run(llvmModule);
}