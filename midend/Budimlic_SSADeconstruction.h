#ifndef __BUDIMLIC_SSA_DECONSTRUCTION__
#define __BUDIMLIC_SSA_DECONSTRUCTION__

/*
Adapted from the paper:
Fast Copy Coalescing and Live-Range Identification
Zoran Budimlic,´ Keith D. Cooper, Timothy J. Harvey,
Ken Kennedy, Timothy S. Oberg, and Steven W. Reeves
http://cseweb.ucsd.edu/classes/sp02/cse231/kenpldi.pdf
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

/// Typedefs
typedef std::pair<llvm::Instruction*, llvm::PHINode*> PhiOperandPairTy;


class Budimlic_SSADeconstructionPass : public llvm::FunctionPass
{
public:
    static char ID; // Pass identification, replacement for typeid
    Budimlic_SSADeconstructionPass()
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
    // Represents a node in the dominator forest. This is a forest in which
    // the nodes represent registers and the edges represent a dominance relation
    // in the block defining those registers.
    struct DomForestNode
    {
    private:
        std::set<DomForestNode*> children;
        llvm::Instruction *pI;

        void add_child(DomForestNode *pDFN) { children.insert(pDFN); }

    public:
        typedef std::set<DomForestNode*>::iterator iterator;

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

        llvm::Instruction* getInstruction() { return pI; }

        inline DomForestNode::iterator begin() { return children.begin(); }
        inline DomForestNode::iterator end() { return children.end(); }
    };

private:
    void computeDFS(llvm::Function &F);
    void dfs(llvm::DomTreeNode *pDomNode, unsigned int &counter);
    std::vector<Budimlic_SSADeconstructionPass::DomForestNode*> computeDominanceForest(std::set<llvm::Instruction*> &phiUnion);
    
    void checkInitialInterference(
        llvm::PHINode* pPhi,
        llvm::Instruction *pI,
        std::set<llvm::BasicBlock*> blocksConsidered,
        std::set<llvm::Instruction*> &phiUnion);

    void processPhiUnion(
        llvm::PHINode *pPhi,
        std::set<llvm::Instruction*> phiUnion,
        std::vector<Budimlic_SSADeconstructionPass::DomForestNode*>& DF,
        std::vector<std::pair<llvm::Instruction*, llvm::Instruction*>> &local_interferences);

    void processBlock(llvm::Function &F);

private:
    llvm::DenseMap<llvm::BasicBlock*, unsigned int> m_preorder;
    llvm::DenseMap<llvm::BasicBlock*, unsigned int> m_maxpreorder;
    
    std::set<llvm::Instruction*> m_operandsUsedInAnotherPhi;
    llvm::DenseMap<llvm::BasicBlock*, std::vector<PhiOperandPairTy>> m_waiting;
    llvm::DenseMap<llvm::PHINode*, std::set<llvm::Instruction*>> m_renameSets;
    std::set<llvm::Instruction*> m_processedNames; // All the names which have already been added to the list to be renamed.
};

Budimlic_SSADeconstructionPass *createBudimlic_SSADeconstructionPass();

#endif