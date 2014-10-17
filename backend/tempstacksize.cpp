#include "tempstacksize.h"

static llvm::RegisterPass<TemporaryStackSize> X("TemporaryStackSize", "Calculate the temporaries used in function", false, false);

char TemporaryStackSize::ID = 0;

bool TemporaryStackSize::runOnFunction(llvm::Function &F)
{
    unsigned int numTemporaries = 0;

    // Loop through all the functions and figure out how many temporaries we would need
    for (llvm::Function::iterator bb = F.begin(), bbend = F.end(); bb != bbend; ++bb)
    {
        // Look at every instruction and see if it has a return value
        for (llvm::BasicBlock::iterator i = bb->begin(), e = bb->end(); i != e; ++i)
        {
            if (i->getNumUses() > 0)
                ++numTemporaries;
        }
    }

    m_numTemporaries[&F] = numTemporaries;

    return false;
}
