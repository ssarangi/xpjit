#include "midend/ConstantFolder.h"
#include "common/debug.h"

char ConstantFolder::ID;

bool ConstantFolder::runOnFunction(llvm::Function &F)
{
    g_outputStream <<"Constant Folding " << F.getName() << "\n";
    return false;
}
