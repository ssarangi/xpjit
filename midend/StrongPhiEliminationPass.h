#ifndef __PHI_DOMINANCE_FOREST__
#define __PHI_DOMINANCE_FOREST__

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

#include <map>

typedef std::pair<llvm::PHINode*, llvm::Instruction*> PhiOperandPairTy;

class StrongPhiEliminationPass : public llvm::FunctionPass
{
public:
    static char ID; // Pass identification, replacement for typeid
    StrongPhiEliminationPass()
        : llvm::FunctionPass(ID)
    {
        initializeDominatorTreeWrapperPassPass(*llvm::PassRegistry::getPassRegistry());
    }

    virtual bool runOnFunction(llvm::Function &F);

    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const
    {
        AU.addRequired<llvm::DominatorTreeWrapperPass>();
        AU.addRequired<EdgeLivenessPass>();
        AU.addRequired<LiveRange>();
        AU.setPreservesCFG();
    };

public:
    // DomForestNode - Represents a node in the "dominator forest". This is a forest in which
    // the nodes represent registers and the edges represent a dominance relation in the block
    // defining those registers
    struct DomForestNode
    {
    private:
        std::vector<DomForestNode*> children;

        llvm::Instruction *pI;

        void add_child(DomForestNode *pDFN) { children.push_back(pDFN); }

    public:
        typedef std::vector<DomForestNode*>::iterator iterator;

        // Create a DomForestNode by providing the register it represents, and
        // the node to be its parent. The virtual root node has a register 0
        // and a null parent
        DomForestNode(llvm::Instruction *pInst, DomForestNode *pParent)
            : pI(pInst)
        {
            if (pParent)
                pParent->add_child(this);
        }

        ~DomForestNode()
        {
            for (iterator i = begin(), e = end(); i != e; ++i)
            {
                delete *i;
            }
        }

        llvm::Instruction *getInstruction() { return pI; }

        inline DomForestNode::iterator begin() { return children.begin(); }
        inline DomForestNode::iterator end() { return children.end(); }
    };

private:
    void buildInitialLiveRange(llvm::Function &F);
    void checkInitialInterference(
        llvm::PHINode* pPhi,
        llvm::Instruction *pI,
        llvm::DenseMap<llvm::BasicBlock*, std::set<llvm::BasicBlock*>> &blocksConsidered,
        std::set<llvm::Instruction*> &phiUnion);

    void computeDFS(llvm::Function &F);
    std::vector<StrongPhiEliminationPass::DomForestNode*> computeDominanceForest(std::set<llvm::Instruction*> &phiUnion);
    void processPHIUnion(
        llvm::PHINode *pPhi,
        std::set<llvm::Instruction*> &phiUnion,
        std::vector<StrongPhiEliminationPass::DomForestNode*> &DF,
        std::vector < std::pair<llvm::Instruction*, llvm::Instruction*>> &locals);

private:
    llvm::DenseMap<llvm::BasicBlock*, std::vector<PhiOperandPairTy>> m_waiting;
    std::set<llvm::Instruction*> m_operandsUsedInAnotherPhi;

    llvm::DenseMap<llvm::BasicBlock*, unsigned int> m_preorder;
    llvm::DenseMap<unsigned int, llvm::BasicBlock*> m_reverse_preorder;
    
    llvm::DenseMap<llvm::BasicBlock*, unsigned int> m_maxpreorder;

    // RenameSets are the is a map from a PHI-defined register
    // to the input registers to be coalesced along with the 
    // predecessor block for those input registers.
    std::map<llvm::Instruction*, std::set<llvm::Instruction*>> m_renameSets;
};

typedef std::vector<StrongPhiEliminationPass::DomForestNode*> DomForest;

StrongPhiEliminationPass *createStrongPhiEliminationPass();

#endif