
#include "Liveness.h"

char Liveness::ID = 0;

// register the printCode class: 
//  - give it a command-line argument
//  - a name
//  - a flag saying that we don't modify the CFG
//  - a flag saying this is not an analysis pass
RegisterPass<Liveness> X("liveVars", "Live vars analysis", false, false);

Liveness *createNewLivenessPass()
{
    Liveness *pL = new Liveness();
    return pL;
}

bool Liveness::runOnFunction(llvm::Function &F)
{
    llvm::DominatorTreeWrapperPass *pDTW = &getAnalysis<llvm::DominatorTreeWrapperPass>();
    llvm::DominatorTree *pDT = &pDTW->getDomTree();

    std::set<llvm::Instruction*> back_edges;

    // Iterate over the basic blocks and find out the branch labels from it.
    for (llvm::Function::iterator bb = F.begin(), bbend = F.end(); bb != bbend; ++bb)
    {
        if (llvm::BranchInst *pBr = llvm::dyn_cast<llvm::BranchInst>(bb->getTerminator()))
        {
            // Get the number of successors of this terminator instruction
            int numSucc = pBr->getNumSuccessors();
            for (uint32_t i = 0; i < numSucc; ++i)
            {
                llvm::BasicBlock *pSucc = pBr->getSuccessor(i);

                // Now if this basic block is a dominator of the original block then it is a back edge.
                if (pDT->dominates(pSucc, &(*bb)))
                    back_edges.insert(pBr);
            }
        }
    }

    return false;
}