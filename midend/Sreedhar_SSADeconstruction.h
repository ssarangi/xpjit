#ifndef __SREEDHAR_SSA_DECONSTRUCTION__
#define __SREEDHAR_SSA_DECONSTRUCTION__

/*
Adapted from the paper: 
Translating Out of Static Single Assignment Form
Vugranam C. Sreedhar1, Roy Dz-Ching Ju2, David M. Gillies3, and Vatsa Santhanam4
http://download.springer.com/static/pdf/188/chp%253A10.1007%252F3-540-48294-6_13.pdf?auth66=1416734275_564783f8725744da1e6fb367e08241c0&ext=.pdf
*/

#include <midend/EdgeLivenessPass.h>
#include <midend/LiveRange.h>

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

typedef std::set<llvm::Instruction*> InstructionSetTy;
typedef llvm::DenseMap<llvm::Instruction*, InstructionSetTy> OneToManyInstructionRelationTy;

class Sreedhar_SSADeconstructionPass : public llvm::FunctionPass
{
public:
    static char ID; // Pass identification, replacement for typeid
    Sreedhar_SSADeconstructionPass()
        : llvm::FunctionPass(ID)
    {
    }

    virtual bool runOnFunction(llvm::Function &F);

    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const
    {
        AU.addRequired<llvm::DominatorTreeWrapperPass>();
        AU.addRequired<EdgeLivenessPass>();
        AU.addRequired<LiveRange>();
        AU.setPreservesCFG();
    };

private:
    void buildInitialPhiCongruenceClass(llvm::Function &F);
    void createCandidateResourceSet(llvm::Function &F);
    void determineCopiesNeeded(llvm::Instruction *pI, llvm::Instruction *pJ);
    bool intersectPhiCongruenceClassAndLiveOut(llvm::Instruction *pI, llvm::BasicBlock *pLiveOutBB);
    void processUnresolvedNeighbors();

private:
    OneToManyInstructionRelationTy  m_phiCongruenceClass;
    OneToManyInstructionRelationTy  m_unresolvedNeighborMap;
    InstructionSetTy                m_candidateResourceSet;
};

Sreedhar_SSADeconstructionPass *createSreedhar_SSADeconstructionPass();

#endif