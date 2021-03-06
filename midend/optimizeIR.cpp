#include "midend/optimizeIR.h"

#include "RearrangeImmOperands.h"
#include "midend/ConstantFolder.h"
#include "midend/DominanceTreeConstructor.h"
#include "midend/DominanceFrontier.h"
#include <midend/Liveness_Benoit.h>
#include <midend/LiveRange.h>
#include "LICM.h"
#include "SSADeconstructionPass.h"
#include "Sreedhar_SSADeconstruction.h"
#include "Budimlic_SSADeconstruction.h"
// #include "StrongPhiEliminationPass.h"
#include <common/llvm_printer.h>

#include <common/llvm_warnings_push.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Pass.h>
#include <llvm/Transforms/Scalar.h>
#include <common/llvm_warnings_pop.h>

void OptimizeIR(llvm::Module& llvmModule)
{
    llvm::legacy::PassManager passMgr;

    PrintCFG *pCFG = new PrintCFG();
    pCFG->setOutputFile(std::string("cfg.dot"));

    //Analysis Passes
    passMgr.add(pCFG);
    passMgr.add(new DominanceTreeConstructor());
    passMgr.add(new DominanceFrontier());

    //Optimization Passes
    passMgr.add(new ConstantFolder());
    passMgr.add(llvm::createPromoteMemoryToRegisterPass());


    passMgr.add(createRearrangeImmOpsPass());
    passMgr.add(createNewLoopInvariantCodeMotionPass());
    // passMgr.add(createStrongPhiEliminationPass());
    // passMgr.add(createSreedhar_SSADeconstructionPass());
    llvm::Pass *pPass = createBudimlic_SSADeconstructionPass();
    passMgr.add(pPass);

    //// Write the output file
    //llvmModule.print(g_outputStream(), nullptr);
    //std::stringstream stringstr = g_outputStream.stringstream();

    //std::ifstream file_handle;
    //file_handle.open()

    passMgr.run(llvmModule);
}