#include "regalloc.h"
#include <midend/Liveness.h>

char RegAlloc::ID = 0;

// register the printCode class: 
//  - give it a command-line argument
//  - a name
//  - a flag saying that we don't modify the CFG
//  - a flag saying this is not an analysis pass
llvm::RegisterPass<RegAlloc> X("liveVars", "Live vars analysis", false, false);

bool RegAlloc::runOnFunction(llvm::Function &F)
{
    m_pLiveness = &getAnalysis<Liveness>();

    return false;
}