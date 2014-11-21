#ifndef __PHI_DOMINANCE_FOREST__
#define __PHI_DOMINANCE_FOREST__

#include <midend/EdgeLivenessPass.h>

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

class PhiDominanceForestPass : public llvm::FunctionPass
{
public:
    static char ID; // Pass identification, replacement for typeid
    PhiDominanceForestPass()
        : llvm::FunctionPass(ID)
        , m_pDT(nullptr)
        , m_pEdgeLiveness(nullptr)
    {
    }

    virtual bool runOnFunction(llvm::Function &F);

    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const
    {
        AU.addRequired<llvm::DominatorTreeWrapperPass>();
        AU.addRequired<EdgeLivenessPass>();
        AU.setPreservesCFG();
    };

private:
    struct PhiVarNode
    {
        PhiVarNode *pParent;
        llvm::Instruction *pInst;

        PhiVarNode(llvm::Instruction *pI)
            : pInst(pI)
            , pParent(nullptr)
        {}
    };

private:
    PhiVarNode* createNode(llvm::Instruction *pI);

    void buildDominanceForest(llvm::Function &F);

private:
    llvm::DominatorTree *m_pDT;
    EdgeLivenessPass *m_pEdgeLiveness;

    llvm::DenseMap<llvm::BasicBlock*, std::set<llvm::Instruction*>> m_congruenceClass;
    llvm::DenseMap<llvm::Instruction*, PhiVarNode*> m_phiCongruentClass;
};

#endif