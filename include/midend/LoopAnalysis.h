#ifndef __LOOPANALYSIS__
#define __LOOPANALYSIS__

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
#include <llvm/IR/Constants.h>
#include "common/llvm_warnings_pop.h"

#include <queue>

struct NaturalLoopTy
{
    unsigned int ID;
    std::set<llvm::BasicBlock*> blocks;
    std::vector<llvm::PHINode*> induction_vars;
    llvm::SmallVector<NaturalLoopTy*, 10> inner_loops;
};

class LoopAnalysis : public llvm::FunctionPass
{
public:
    static char ID; // Pass identification, replacement for typeid
    LoopAnalysis()
        : llvm::FunctionPass(ID)
        , m_pDT(nullptr)
    {}

    virtual bool runOnFunction(llvm::Function &F);

    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const
    {
        AU.addRequired<llvm::DominatorTreeWrapperPass>();
        AU.setPreservesAll();
    };

private:
    void findBasicLoopInductionVar(NaturalLoopTy *pNaturalLoop);
    bool isPhiNodeInductionVar(llvm::PHINode *pPhi);
    
    void performStrengthReduction(
        NaturalLoopTy *pNaturalLoop,
        std::queue<llvm::Instruction*>& instMoveOrder,
        llvm::DenseMap<llvm::Instruction*, llvm::Instruction*>& instMovMap);

    bool doesInstructionUseLoopInductionVar(llvm::Instruction *pI, llvm::DenseMap<llvm::Value*, bool>& induction_var_map);
    bool notToRemoveInst(llvm::Instruction *pI);

private:
    llvm::DominatorTree *m_pDT;
    llvm::SmallVector<NaturalLoopTy*, 10> m_naturalLoops;
};

LoopAnalysis *createNewLoopAnalysisPass();

#endif