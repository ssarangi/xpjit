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

#include <map>

class StrongPhiEliminationPass : public llvm::FunctionPass
{
public:
    static char ID; // Pass identification, replacement for typeid
    StrongPhiEliminationPass()
        : llvm::FunctionPass(ID)
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

    void buildDominanceForest(llvm::Function &F);

private:
    // Waiting stores for each basic block. These are conservative copies meaning that we are 
    // not yet sure if we want to insert them.
    llvm::DenseMap <llvm::BasicBlock*, std::multimap<llvm::Instruction*, llvm::Instruction*>> m_waiting;

    // Place holder for all variables to be renamed
    llvm::DenseMap<unsigned int, std::vector<unsigned int> > m_stacks;

    // Add registers which are Phi nodes and are used as operands in other phi nodes
    std::set<llvm::Instruction*> m_usedByAnother;

    // Rename sets is a map from a Phi defined register to the input registers to be coalesced
    // along with predecessor block for those input registers
    std::map<llvm::Instruction*, std::map<llvm::Instruction*, llvm::BasicBlock*>> m_renameSets;

    // Phi Value Number holds the ID numbers of the Value numbers for each phi that we are
    // eliminating, indexed by the register defined by that phi.
    std::map<unsigned int, unsigned int> m_phiValueNumber;

    // Store the DFS-in number of each block
    llvm::DenseMap<llvm::BasicBlock*, unsigned int> m_preorder;

    // Store the DFS-out number of each block
    llvm::DenseMap<llvm::BasicBlock*, unsigned int> m_maxpreorder;

private:
    class DomForestNode
    {
    private:
        std::vector<DomForestNode*> m_children;

        unsigned int m_reg;

        void addChild(DomForestNode *pDFN) { m_children.push_back(pDFN); }

    public:
        typedef std::vector<DomForestNode*>::iterator iterator;

        // Create a DomForestNode by providing the register it represents and the node to be
        // its parent. The virtual root node has register 0 and a null parent
        DomForestNode(unsigned int r, DomForestNode *pParent)
            : m_reg(r)
        {
            if (pParent)
                pParent->addChild(this);
        }

        ~DomForestNode()
        {
            for (iterator I = begin(), E = end(); I != E; ++I)
            {
                delete *I;
            }
        }

        inline unsigned int getReg() { return m_reg; }

        // Provide iterator access to our children
        inline DomForestNode::iterator begin() { return m_children.begin(); }
        inline DomForestNode::iterator end() { return m_children.end(); }
    };

    void computeDFS(llvm::Function &F);
    void processBlock(llvm::BasicBlock *pBB);

    std::vector<DomForestNode *> computeDomForest(
        std::map<llvm::Instruction*, llvm::BasicBlock*>& instrs,
        llvm::BasicBlock *pBB);

    void processPHIUnion(
        llvm::Instruction *pU,
        std::map<llvm::Instruction*, llvm::BasicBlock*>& phiUnion,
        std::vector<StrongPhiEliminationPass::DomForestNode*> &DF,
        std::vector <std::pair<llvm::Instruction*, llvm::Instruction*>> &locals);

    void scheduleCopies(llvm::BasicBlock *pBB, std::set<unsigned int> &pushed);
    void insertCopies(llvm::DomTreeNode *pDTNode, llvm::SmallPtrSet<llvm::BasicBlock*, 16>& v);
    bool mergeLiveIntervals(unsigned int primary, unsigned int secondary);
};

#endif