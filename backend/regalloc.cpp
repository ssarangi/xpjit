#include "regalloc.h"
#include <midend/Liveness_Benoit.h>

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

    llvm::BasicBlock *pDefBlock = nullptr;
    llvm::BasicBlock *pQueryBlock = nullptr;
    for (llvm::Function::iterator bb = F.begin(), be = F.end(); bb != be; ++bb)
    {
        if (bb->getName() == "whileblock2")
            pQueryBlock = bb;

        if (bb->getName() == "whilecond")
            pDefBlock = bb;
    }

    m_pLiveness->isLiveInBlock(pDefBlock, pQueryBlock);

    return false;
}