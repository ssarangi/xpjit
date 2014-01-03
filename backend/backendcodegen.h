#ifndef __CODEGEN__
#define __CODEGEN__

#include <llvm/Pass.h>
#include <llvm/InstVisitor.h>
#include <llvm/Analysis/Dominators.h>
#include <llvm/Analysis/PostDominators.h>
#include <llvm/Analysis/MemoryDependenceAnalysis.h>

class CodeGenPass : public llvm::ModulePass, public llvm::InstVisitor<CodeGenPass>
{
    static char ID;

public:
    CodeGenPass() : llvm::ModulePass(ID)
    {
        llvm::initializePostDominatorTreePass(*llvm::PassRegistry::getPassRegistry());
    }

    virtual void getAnalysisUsage(llvm::AnalysisUsage& usage) const
    {
        usage.setPreservesCFG();
        usage.addRequired<llvm::PostDominatorTree>();
    }

    virtual bool runOnFunction(llvm::Function& F);
    
    virtual bool runOnModule(llvm::Module &M);

    virtual const char *getPassName() const
    {
        return "CodeGenPass";
    }
};

#endif