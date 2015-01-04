#ifndef SSA_CONSTRUCTOR_H
#define SSA_CONSTRUCTOR_H

#include <common/llvm_warnings_push.h>
#include <llvm/Pass.h>
#include <common/llvm_warnings_pop.h>

using llvm::FunctionPass;

struct SSAConstructor : public FunctionPass 
{
public:
    static char ID;
    SSAConstructor() 
        : FunctionPass(ID)
    {}

    virtual bool doInitialization(llvm::Module &M) { return true; }
    virtual bool runOnFunction(llvm::Function &F);
    virtual bool doFinalization(llvm::Module &M) { return true; }

    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const {
        AU.setPreservesAll();
    }
private:
    //set of globals that we figure out using the findGlobalNames function
    
    void findGlobalNames();
    void rename();
};

static llvm::RegisterPass<SSAConstructor> ssac("ssaconstructor", "ssa constructor", false /* true - read only pass */, true /* analysis pass*/);
#endif
