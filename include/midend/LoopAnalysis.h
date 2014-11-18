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
#include <llvm/Analysis/PostDominators.h>
#include <llvm/IR/Constants.h>
#include <llvm/PassRegistry.h>
#include "common/llvm_warnings_pop.h"

#include <queue>

struct NaturalLoopTy
{
    NaturalLoopTy() {}

    unsigned int ID;
    llvm::BasicBlock *pHeader = nullptr;
    llvm::BasicBlock *pExit = nullptr;
    std::set<llvm::BasicBlock*> blocks;
    std::set<llvm::Instruction*> induction_vars;
    std::set<NaturalLoopTy*> inner_loops;
    NaturalLoopTy *pParent = nullptr;

    bool operator=(const NaturalLoopTy &nloop)
    {
        return ((pHeader == nloop.pHeader) & (pExit == nloop.pExit));
    }
};

class LoopAnalysis : public llvm::FunctionPass
{
public:
    static char ID; // Pass identification, replacement for typeid
    LoopAnalysis()
        : llvm::FunctionPass(ID)
        , m_pDT(nullptr)
    {
        llvm::initializePostDominatorTreePass(*llvm::PassRegistry::getPassRegistry());
    }

    virtual bool runOnFunction(llvm::Function &F);

    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const
    {
        AU.addRequired<llvm::DominatorTreeWrapperPass>();
        AU.addRequired<llvm::PostDominatorTree>();
        AU.setPreservesAll();
    };

    unsigned int getNumOuterLoops() const { return m_naturalLoops.size(); }
    std::set<NaturalLoopTy*>::const_iterator begin()
    {
        return m_naturalLoops.cbegin();
    }

    std::set<NaturalLoopTy*>::const_iterator end()
    {
        return m_naturalLoops.cend();
    }

    /*
    Loop through all the loops we have and figure out if we find a match for the basic block
    as being the header of some block
    */
    bool isLoopHeader(llvm::BasicBlock *pBB, NaturalLoopTy* &pLoop);

    /*
    Given a loop find out if the basic block is a loop header of either the outermost loop or any of
    its inner loop
    */
    bool isLoopHeader(llvm::BasicBlock *pBB, NaturalLoopTy* pQueryLoop, NaturalLoopTy* &pLoop);

private:
    void removeInnerLoopNodeDFS(NaturalLoopTy *pLoop, NaturalLoopTy *pParent);
    void findBasicLoopInductionVar(NaturalLoopTy *pNaturalLoop);
    bool isPhiNodeInductionVar(llvm::PHINode *pPhi);

private:
    llvm::DominatorTree *m_pDT;
    std::set<NaturalLoopTy*> m_naturalLoops;
};

LoopAnalysis *createNewLoopAnalysisPass();

#endif