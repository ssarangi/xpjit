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

struct LoopInductionVarTriple
{
    llvm::Instruction *pInst;
    llvm::Value *pV;
    int a;
    int b;

    bool operator==(llvm::Value *pVal)
    {
        return this->pV == pVal;
    }
};

struct NaturalLoopTy
{
    unsigned int ID;
    std::set<llvm::BasicBlock*> blocks;
    std::vector<LoopInductionVarTriple> basic_induction_var;
    std::vector<LoopInductionVarTriple> derived_induction_var;
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
    void findBasicLoopInductionVar(NaturalLoopTy &natural_loop);
    void deriveInductionVar(NaturalLoopTy &natural_loop);

private:
    llvm::DominatorTree *m_pDT;
    llvm::SmallVector<NaturalLoopTy, 10> m_naturalLoops;
    llvm::SmallVector<LoopInductionVarTriple, 3> m_basicInductionVar;
};

LoopAnalysis *createNewLoopAnalysisPass();

#endif