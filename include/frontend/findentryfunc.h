#ifndef __FIND_ENTRY_FUNC__
#define __FIND_ENTRY_FUNC__

#include <llvm/Pass.h>

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