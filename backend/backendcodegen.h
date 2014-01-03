#ifndef __CODEGEN__
#define __CODEGEN__

#include <llvm/Pass.h>
#include <llvm/InstVisitor.h>

class CodeGenPass : public llvm::ModulePass, public llvm::InstVisitor<CodeGenPass>
{
    static char ID;

public:
    CodeGenPass() : ModulePass(ID) {}

    virtual void getAnalysisUsage(llvm::AnalysisUsage& usage)
    {
        usage.setPreservesCFG();
    }

    bool runOnFunction(llvm::Function& F);
    virtual bool runOnModule(llvm::Module &M);

    virtual const char *getPassName() const
    {
        return "CodeGenPass";
    }
};

#endif