#ifndef __LIVENESS__
#define __LIVENESS__

/*
Based on the paper - Fast Liveness Checking for SSA-Form Programs
by Benoit Boissinot , Sebastian Hack , Benoît Dupont De Dinechin , Daniel Grund , Fabrice Rastello
*/


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

using namespace llvm;
#include <set>

class Liveness : public FunctionPass
{
private:

public:
    static char ID; // Pass identification, replacement for typeid
    Liveness() : FunctionPass(ID) {}

    virtual bool runOnFunction(llvm::Function &F);

    virtual void getAnalysisUsage(AnalysisUsage &AU) const
    {
        AU.addRequired<llvm::DominatorTreeWrapperPass>();
    };

};

Liveness *createNewLivenessPass();

#endif