#include "mipspatternmatch.h"

bool MipsPatternMatch::runOnModule(llvm::Module &M)
{
    // Get the dominator tree for iterating over the DAG
    llvm::DominatorTree &DT = getAnalysis<llvm::DominatorTree>();
}