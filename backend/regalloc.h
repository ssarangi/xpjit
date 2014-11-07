#ifndef __REGALLOC__
#define __REGALLOC__

#include <midend/Liveness.h>

#include "common/llvm_warnings_push.h"
#include <llvm/Pass.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Instruction.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/ADT/Statistic.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/IR/User.h>
#include <llvm/IR/Instructions.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/CFG.h>
#include <llvm/IR/Dominators.h>
#include "common/llvm_warnings_pop.h"

#include <set>

class RegAlloc : public llvm::FunctionPass
{
public:
    static char ID; // Pass identification, replacement for typeid
    RegAlloc()
        : llvm::FunctionPass(ID)
    {}

    virtual bool runOnFunction(llvm::Function &F);

    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const
    {
        AU.addRequired<Liveness>();
    };

private:

private:
    Liveness *m_pLiveness;
};

RegAlloc *createNewRegisterAllocatorPass();


#endif