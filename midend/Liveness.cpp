
#include "Liveness.h"

char Liveness::ID = 0;

// register the printCode class: 
//  - give it a command-line argument
//  - a name
//  - a flag saying that we don't modify the CFG
//  - a flag saying this is not an analysis pass
RegisterPass<Liveness> X("liveVars", "Live vars analysis",
    false, true);

DenseMap<const Instruction*, int> instMap;

void print_elem(const Instruction* i) {
    errs() << instMap.lookup(i) << " ";
}

Liveness *createNewLivenessPass()
{
    Liveness *pL = new Liveness();
    return pL;
}