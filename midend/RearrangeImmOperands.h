#ifndef __REARRANGE_IMM_OPS__
#define __REARRANGE_IMM_OPS__

#include "common/llvm_warnings_push.h"
#include <llvm/Pass.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Instructions.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/SparseSet.h>
#include <llvm/IR/CFG.h>
#include <llvm/IR/Dominators.h>
#include <llvm/Analysis/PostDominators.h>
#include <llvm/IR/Constants.h>
#include <llvm/PassRegistry.h>
#include "common/llvm_warnings_pop.h"

class RearrangeImmOps : public llvm::FunctionPass
{
public:
    static char ID; // Pass identification, replacement for typeid
    RearrangeImmOps()
        : llvm::FunctionPass(ID)
    {
    }

    virtual bool runOnFunction(llvm::Function &F);

    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const
    {
        AU.setPreservesCFG();
    };

private:
};

RearrangeImmOps *createRearrangeImmOpsPass();

#endif