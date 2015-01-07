#include "midend/ConstantFolder.h"
#include "common/debug.h"

char ConstantFolder::ID;

bool ConstantFolder::runOnFunction(llvm::Function &F)
{
    ADD_HEADER("Constant Folding");
    g_outputStream <<"Constant Folding " << F.getName() << "\n";
    return false;
}
