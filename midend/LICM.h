#ifndef __LICM__
#define __LICM__

#include <midend/LoopAnalysis.h>

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

class LICM : public llvm::FunctionPass
{
public:
    static char ID; // Pass identification, replacement for typeid
    LICM()
        : llvm::FunctionPass(ID)
    {
        llvm::initializePostDominatorTreePass(*llvm::PassRegistry::getPassRegistry());
    }

    virtual bool runOnFunction(llvm::Function &F);

    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const
    {
        AU.addRequired<llvm::DominatorTreeWrapperPass>();
        AU.addRequired<LoopAnalysis>();
        AU.setPreservesCFG();
    };

private:
    void performLICM(
        NaturalLoopTy *pNaturalLoop,
        std::queue<llvm::Instruction*>& instMoveOrder,
        llvm::DenseMap<llvm::Instruction*, llvm::Instruction*>& instMovMap);

    // bool doesInstructionUseLoopInductionVar(llvm::Instruction *pI, llvm::DenseMap<llvm::Value*, bool>& induction_var_map);
    bool doesInstructionUseInductionVarInLoop(llvm::Instruction *pI, NaturalLoopTy *pLoop, std::set<llvm::Instruction*>& visitedInst);
    llvm::BasicBlock* returnTargetBBForInstMove(llvm::Instruction *pI, NaturalLoopTy *pLoop);
    NaturalLoopTy* findInnerMostLoopContainingBB(llvm::BasicBlock *pBB, NaturalLoopTy *pOuterLoop);
    bool notToRemoveInst(llvm::Instruction *pI);

private:
    llvm::DominatorTree *m_pDT;
    LoopAnalysis *m_pLoopAnalysis;
};

LICM *createNewLoopInvariantCodeMotionPass();

#endif