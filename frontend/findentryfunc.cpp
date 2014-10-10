#include <frontend/findentryfunc.h>

#include <common/llvm_warnings_push.h>
#include <llvm/IR/Module.h>
#include <common/llvm_warnings_pop.h>

char FindEntryFunc::ID = 0;

bool FindEntryFunc::runOnModule(llvm::Module &M)
{
    for (auto func = M.begin(); func != M.end(); ++func)
    {
        if (func->getName() == "main")
        {
            m_pEntryFunc = func;
            return true;
        }
    }

    return false;
}