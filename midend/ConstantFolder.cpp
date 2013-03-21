#include "midend/ConstantFolder.h"
#include "common/debug.h"

static Trace& gTrace = Trace::getInstance();

char ConstantFolder::ID;

bool ConstantFolder::runOnFunction(llvm::Function &F)
{
	gTrace<<"Constant Folding "<<F.getName();
	return false;
}
