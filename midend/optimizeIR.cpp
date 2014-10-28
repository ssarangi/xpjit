#include "midend/optimizeIR.h"

#include "midend/ConstantFolder.h"
#include "midend/DominanceTreeConstructor.h"
#include "midend/DominanceFrontier.h"
#include "liveness.h"

#include <common/llvm_warnings_push.h>
#include <llvm/PassManager.h>
#include <llvm/Pass.h>
#include <llvm/Transforms/Scalar.h>
#include <common/llvm_warnings_pop.h>

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
    passMgr.add(createNewLivenessPass());
    passMgr.run(llvmModule);
}