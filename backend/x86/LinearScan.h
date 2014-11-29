#ifndef __LINEAR_SCAN__
#define __LINEAR_SCAN__

#include <midend/LiveRange.h>
#include "x86defines.h"

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
#include <llvm/IR/IRBuilder.h>
#include "common/llvm_warnings_pop.h"

enum LiveRangeType
{
    UNHANDLED,
    ACTIVE,
    INACTIVE,
    HANDLED
};

class LinearScanAllocator : public llvm::FunctionPass
{
public:
    static char ID; // Pass identification, replacement for typeid
    LinearScanAllocator()
        : llvm::FunctionPass(ID)
        , m_stackLocation(0)
    {
    }

    virtual bool runOnFunction(llvm::Function &F);

    std::vector<LiveRangeInterval*> sortLiveInterval();
    void expireOldIntervals(LiveRangeInterval *pInterval, std::set<LiveRangeInterval*> &active, std::stack<BackendRegister*> &free_registers);
    void spillAtInterval(LiveRangeInterval *pInterval, std::set<LiveRangeInterval*> &active);
    void performLinearScan();

    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const
    {
        AU.addRequired<llvm::DominatorTreeWrapperPass>();
        AU.addRequired<LiveRange>();
        AU.setPreservesCFG();
    };

private:
    llvm::DenseMap<LiveRangeInterval*, BackendRegister*> m_liveRangeIntervalToBackendRegister;
    llvm::DenseMap<LiveRangeInterval*, unsigned int> m_stackLocationMapping;
    std::set<LiveRangeInterval*> m_seenInterval;
    unsigned int m_stackLocation;
};

LinearScanAllocator *createLinearScanRegisterAllocationPass();

#endif