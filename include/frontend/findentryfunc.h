#ifndef __FIND_ENTRY_FUNC__
#define __FIND_ENTRY_FUNC__

#include <common/llvm_warnings_push.h>
#include <llvm/Pass.h>
#include <common/llvm_warnings_pop.h>

class FindEntryFunc : public llvm::ModulePass
{
    static char ID;

public:
    FindEntryFunc()
        : ModulePass(ID)
        , m_pEntryFunc(nullptr)
    {}

    virtual void getAnalysisUsage(llvm::AnalysisUsage& usage)
    {
        usage.setPreservesCFG();
    }

    virtual bool runOnModule(llvm::Module &M);

    llvm::Function* GetEntryFunc() { return m_pEntryFunc; }

private:
    llvm::Function* m_pEntryFunc;
};


#endif