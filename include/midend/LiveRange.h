#ifndef __LIVERANGE__
#define __LIVERANGE__

/*
Based on the paper - Linear Scan Register Allocation on SSA Form
by Christian Wimmer Michael Franz
*/

#include <midend/LoopAnalysis.h>

#include "common/llvm_warnings_push.h"
#include <llvm/Pass.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/InstVisitor.h>
#include <llvm/IR/Instruction.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/ADT/Statistic.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/IR/User.h>
#include <llvm/IR/Instructions.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/SparseSet.h>
#include <llvm/IR/CFG.h>
#include <llvm/IR/Dominators.h>
#include "common/llvm_warnings_pop.h"

#include <set>
#include <stack>

/*
Cases to handle for live range
def-to-end
begin-to-kill
live-through
def-to-kill
*/

struct LiveRangeInfo
{
    /*
       A very rudimentary DS to denote the live range. We do not handle holes at the moment
    */
    int startBlockNo; // start and end are for live through blocks
    int endBlockNo;
    int instructionOffset;  // offset from beginning of basic block

    std::set<unsigned int> range;
    llvm::Value *pValue;

public:
    LiveRangeInfo(llvm::Value *pV)
        : pValue(pV)
        , startBlockNo(-1)
        , endBlockNo(-1)
        , instructionOffset(-1)
    {}

    bool operator<(LiveRangeInfo &LRI)
    {
        bool result = false;
        if (*this->range.begin() < *LRI.range.begin())
            result = true;
        else if (*this->range.begin() > *LRI.range.begin())
            result = false;
        else if (this->instructionOffset < LRI.instructionOffset)
            result = true;

        return result;
    }

    void addBB(int BB_no)
    {
        range.insert(BB_no);
    }

    void removeBB(int BB_no)
    {
        range.erase(BB_no);
    }

    void addUseStartBlockNo(
        int blockNo,
        llvm::DenseMap<llvm::BasicBlock*, unsigned int>& blockToId,
        llvm::DenseMap<unsigned int, llvm::BasicBlock*>& idToBlock,
        llvm::DominatorTree *pDT)
    {
        if (startBlockNo == -1)
        {
            startBlockNo = blockNo;
            return;
        }

        llvm::BasicBlock *pOldStartBlock = idToBlock[(unsigned int)(startBlockNo)];
        llvm::BasicBlock *pNewUseStartBlock = idToBlock[(unsigned int)blockNo];

        if (pDT->dominates(pNewUseStartBlock, pOldStartBlock))
            startBlockNo = blockNo;
    }

    void addUseEndBlockNo(
        int blockNo,
        llvm::DenseMap<llvm::BasicBlock*, unsigned int>& blockToId,
        llvm::DenseMap<unsigned int, llvm::BasicBlock*>& idToBlock,
        llvm::PostDominatorTree *pPDT)
    {
        // If uninitialized then add this
        if (endBlockNo == -1)
        {
            endBlockNo = blockNo;
            return;
        }

        // If the end block has been identified before try to find the block which is the most post dominated.
        llvm::BasicBlock *pOldEndBlock = idToBlock[(unsigned int)(endBlockNo)];
        llvm::BasicBlock *pNewUseEndBlock = idToBlock[(unsigned int)blockNo];

        if (pPDT->dominates(pNewUseEndBlock, pOldEndBlock))
        {
            endBlockNo = blockNo;
        }

        // If the old block doesn't dominate the new use block then both of them could be in control flow.
        /*
                                     Root - x defined here
                                     /  \
                                    B1   \
                                    /     \
                    x used here - oldB   newB - x used here
                                    \    /
                                     \  /
                                 CommonDomRoot
        */
        if (!pPDT->dominates(pOldEndBlock, pNewUseEndBlock))
        {
            llvm::BasicBlock *pCommonDenominatorBlock = pPDT->findNearestCommonDominator(pNewUseEndBlock, pOldEndBlock);
            endBlockNo = blockToId[pCommonDenominatorBlock];
        }
    }

    void addInstructionOffset(int instructionOffset)
    {
        // Update only if this offset is more than before
        if (this->instructionOffset < instructionOffset)
            this->instructionOffset = instructionOffset;
    }
};

struct BasicBlockLiveIn
{
    std::set<llvm::Value*> liveIns;
};

class LiveRange : public llvm::FunctionPass
{
public:
    static char ID; // Pass identification, replacement for typeid
    LiveRange()
        : llvm::FunctionPass(ID)
        , m_pDT(nullptr)
    {
            initializeDominatorTreeWrapperPassPass(*llvm::PassRegistry::getPassRegistry());
            initializePostDominatorTreePass(*llvm::PassRegistry::getPassRegistry());
    }

    virtual bool runOnFunction(llvm::Function &F);

    bool isLive(llvm::Value *pQuery, llvm::Instruction *pInstNode);
    bool isLiveInBlock(llvm::BasicBlock *pDefBB, llvm::BasicBlock *pQueryBB);
    bool isLiveOutBlock(llvm::Value *pQuery, llvm::BasicBlock *pBlock);

    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const
    {
        AU.addRequired<llvm::DominatorTreeWrapperPass>();
        AU.addRequired<llvm::PostDominatorTree>();
        AU.addRequired<LoopAnalysis>();
        AU.setPreservesAll();
    };

    void visitInstruction(llvm::Instruction *pI);
    void visitPhiNode(llvm::PHINode *pPhi);

private:
    void unionLiveInSetsOfSuccessors(llvm::BasicBlock *pBB);
    void addBBToRange(llvm::Value* pV, int bbNo);
    
    std::stack<llvm::BasicBlock*> initializeBlockAndInstructionID(llvm::Function &F);

    LiveRangeInfo* createNewLiveRange(llvm::Value *pV);
    LiveRangeInfo* findOrCreateLiveRange(llvm::Value *pV);

private:
    llvm::DenseMap<llvm::BasicBlock*, unsigned int> m_blockToId;
    llvm::DenseMap<unsigned int, llvm::BasicBlock*> m_idToBlock;
    
    llvm::DenseMap<llvm::BasicBlock*, BasicBlockLiveIn> m_BBLiveIns;
    llvm::DenseMap<llvm::BasicBlock*, llvm::SmallVector<llvm::PHINode*, 5>> m_PhiNodesInBB;
    
    llvm::DenseMap<llvm::Value*, LiveRangeInfo*> m_intervalMap;
    std::set<LiveRangeInfo*> m_intervals;
    
    llvm::DenseMap<llvm::Instruction*, unsigned int> m_instructionOffsets;
    LoopAnalysis *m_pLoopAnalysis;

    llvm::DominatorTree *m_pDT;
    llvm::PostDominatorTree *m_pPDT;
};

LiveRange *createNewLiveRangePass();

#endif