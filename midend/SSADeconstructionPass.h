#ifndef __OUT_OF_SSA__
#define __OUT_OF_SSA__

/* 
Adapted from the lecture notes: http://www.cs.rice.edu/~keith/512/Lectures/14SSA-II-1up.pdf
*/

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

#include <utility>

typedef std::pair<llvm::PHINode*, llvm::PHINode*> OldNewPhiNodePair;

class SSADeconstructionPass : public llvm::FunctionPass
{
public:
    static char ID; // Pass identification, replacement for typeid
    SSADeconstructionPass()
        : llvm::FunctionPass(ID)
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

    void convertToCSSA(llvm::Function &F);
    OldNewPhiNodePair visitPhi(llvm::PHINode *pPhi);

private:
    EdgeLivenessPass *m_pEdgeLiveness;
};

SSADeconstructionPass *createSSADeconstructionPass();

#endif