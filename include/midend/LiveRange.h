#ifndef __LIVERANGE__
#define __LIVERANGE__

/*
Based on the paper - Linear Scan Register Allocation on SSA Form
by Christian Wimmer Michael Franz
*/

#include <common/debug.h>
#include <midend/LoopAnalysis.h>
#include <midend/BlockLayoutPass.h>

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
#include <llvm/ADT/DenseMapInfo.h>
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

struct Interval
{
    int start;
    int end;
    bool isLive; // Could be a hole.
    void *pReg; // Till I find a good way to do this.

    bool operator<(const Interval &rhs)
    {
        return start < rhs.start;
    }
};

struct LiveRangeInterval
{
    /*
       A very rudimentary DS to denote the live range. We do not handle holes at the moment
       */
    unsigned int def_block; // start and end are for live through blocks
    unsigned int kill_block;
    unsigned int def_offset;  // offset from beginning of basic block
    unsigned int def_id;
    unsigned int kill_offset;    // offset from where it ended in basic block
    unsigned int kill_id;

    std::set<unsigned int> range;
    llvm::Instruction *pInstr;
    unsigned int instruction_id;
    BlockLayoutPass *pBlockLayout;
    std::set<Interval*> intervals;

private:
    void copy_members(const LiveRangeInterval &rhs)
    {
        def_id = rhs.def_id;
        kill_id = rhs.kill_id;

        // Do not copy other members for now.
    }

public:
    LiveRangeInterval(llvm::Instruction *pI, BlockLayoutPass *pBLayout)
        : pInstr(pI)
        , pBlockLayout(pBLayout)
        , def_block(0)
        , kill_block(0)
        , def_offset(0)
        , def_id(0)
        , kill_offset(0)
        , kill_id(0)
        , instruction_id(0)
    {
        this->def_block = pBlockLayout->getBlockID(pI->getParent());
        this->kill_block = this->def_block;
        this->def_offset = pBlockLayout->getInstructionOffset(pI);
        this->kill_offset = this->def_offset;
        instruction_id = pBlockLayout->getInstructionID(pI);
        this->def_id = instruction_id;
    }

    LiveRangeInterval(const LiveRangeInterval &rhs)
    {
        copy_members(rhs);
    }

    void splitInterval(llvm::Instruction *pInst)
    {
        unsigned int split_pt = pBlockLayout->getInstructionID(pInst);

        Interval *pInterval = nullptr;
        if (intervals.empty())
        {
            // This means no intervals have been set.
            pInterval = new Interval();
            pInterval->start = def_id;
            pInterval->end = kill_id;
            intervals.insert(pInterval);
        }

        // Find the last interval
        std::set<Interval*>::iterator i = --intervals.end();

        Interval *pNewInterval = new Interval();
        pNewInterval->start = split_pt;
        pNewInterval->end = (*i)->end;
        (*i)->end = split_pt - 1;

        intervals.insert(pNewInterval);
    }

    void assignRegister(int interval_start, void *pReg)
    {
        for (auto i : intervals)
        {
            if (i->start == interval_start)
                i->pReg = pReg;
        }
    }

    bool interferes(const LiveRangeInterval &LRI)
    {
        return ((this->kill_id < LRI.def_id) | (this->def_id > LRI.kill_id));
    }

    void add_basic_block(llvm::BasicBlock *pBlock)
    {
        unsigned int blockNo = pBlockLayout->getBlockID(pBlock);
        range.insert(blockNo);
    }

    void remove_basic_block(llvm::BasicBlock *pBlock)
    {
        unsigned int blockNo = pBlockLayout->getBlockID(pBlock);
        range.erase(blockNo);
    }

    //void add_def_block(llvm::BasicBlock *pBlock)
    //{
    //    unsigned int blockNo = pBlockLayout->getBlockID(pBlock);
    //    if (def_block != -1)
    //        return;

    //    def_block = blockNo;
    //}

    void add_kill_block(llvm::BasicBlock *pBlock)
    {
        unsigned int blockNo = pBlockLayout->getBlockID(pBlock);

        //// If the end block has been identified before try to find the block which is the most post dominated.
        //llvm::BasicBlock *pOldEndBlock = pBlockLayout->getBlock(kill_block);
        //llvm::BasicBlock *pNewUseEndBlock = pBlockLayout->getBlock(blockNo);

        //if (pBlockLayout->getPostDomTree()->dominates(pNewUseEndBlock, pOldEndBlock))
        //{
        //    kill_block = blockNo;
        //}

        //// If the old block doesn't dominate the new use block then both of them could be in control flow.
        ///*
        //                             Root - x defined here
        //                             /  \
                        //                            B1   \
                        //                            /     \
                        //            x used here - oldB   newB - x used here
        //                            \    /
        //                             \  /
        //                         CommonDomRoot
        //*/
        //if (!pBlockLayout->getPostDomTree()->dominates(pOldEndBlock, pNewUseEndBlock))
        //{
        //    llvm::BasicBlock *pCommonDenominatorBlock = pBlockLayout->getPostDomTree()->findNearestCommonDominator(pNewUseEndBlock, pOldEndBlock);
        //    kill_block = pBlockLayout->getBlockID(pCommonDenominatorBlock);
        //}

        if (this->kill_block < blockNo)
            this->kill_block = blockNo;
    }

    void add_def(llvm::Instruction *pI)
    {
        unsigned int id = pBlockLayout->getInstructionID(pI);
        // Update only if this offset is more than before
        if (this->def_id < id)
        {
            this->def_id = id;
            this->def_offset = pBlockLayout->getInstructionOffset(pI);
        }
    }

    void add_kill(llvm::Instruction *pI)
    {
        unsigned int id = pBlockLayout->getInstructionID(pI);
        if (this->kill_id < id)
        {
            this->kill_id = id;
            this->kill_offset = pBlockLayout->getInstructionOffset(pI);
        }
    }

    bool operator=(const LiveRangeInterval& rhs)
    {
        copy_members(rhs);
    }
};

bool operator<(const LiveRangeInterval& lhs, const LiveRangeInterval &rhs);

struct PointerCompare
{
    bool operator()(const LiveRangeInterval* l, const LiveRangeInterval* r)
    {
        return l->def_id < r->def_id;
    }

    bool operator()(const Interval *pL, const Interval *pR)
    {
        return pL->start < pR->start;
    }
};

struct BasicBlockLiveIn
{
    std::set<llvm::Value*> live;
};

typedef llvm::DenseMap<llvm::Instruction*, LiveRangeInterval*> IntervalMapTy;

class LiveRange : public llvm::FunctionPass
{
public:
    static char ID; // Pass identification, replacement for typeid
    LiveRange()
        : llvm::FunctionPass(ID)
        , m_pBlockLayout(nullptr)
        , m_pLoopAnalysis(nullptr)
    {
            initializeDominatorTreeWrapperPassPass(*llvm::PassRegistry::getPassRegistry());
            initializePostDominatorTreePass(*llvm::PassRegistry::getPassRegistry());
    }

    virtual bool runOnFunction(llvm::Function &F);

    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const
    {
        AU.addRequired<BlockLayoutPass>();
        AU.addRequired<LoopAnalysis>();
        AU.setPreservesAll();
    };

    void visitInstruction(llvm::Instruction *pI);
    void visitPhiNode(llvm::PHINode *pPhi);
    bool interferes(llvm::Instruction *pA, llvm::Instruction *pB);
    const IntervalMapTy& getIntervalMap() const { return m_intervalMap; }
    
    LiveRangeInterval* getInterval(llvm::Instruction *pI)
    {
        assert(m_intervalMap.find(pI) != m_intervalMap.end());
        return m_intervalMap[pI];
    }

private:
    // These are instructions which are black listed i.e. no live range interval needs to be
    // created.
    bool isInstructionBlackListed(llvm::Instruction *pI);
    void unionLiveInSetsOfSuccessors(llvm::BasicBlock *pBB);
    void addBBToRange(llvm::Value* pV, llvm::BasicBlock *pBlock);
    
    std::stack<llvm::BasicBlock*> initializeBlockAndInstructionID(llvm::Function &F);

    LiveRangeInterval* createNewLiveRange(llvm::Value *pV);
    LiveRangeInterval* findOrCreateLiveRange(llvm::Value *pV);

    void printLiveRanges(llvm::Function& F);

private:
    llvm::DenseMap<llvm::BasicBlock*, BasicBlockLiveIn> m_BBLiveIns;
    llvm::DenseMap<llvm::BasicBlock*, llvm::SmallVector<llvm::PHINode*, 5>> m_PhiNodesInBB;
    
    IntervalMapTy m_intervalMap;
    std::set<LiveRangeInterval*> m_intervals;
    
    llvm::DenseMap<llvm::Instruction*, unsigned int> m_instructionOffsets;

    BlockLayoutPass *m_pBlockLayout;
    LoopAnalysis *m_pLoopAnalysis;
};

LiveRange *createNewLiveRangePass();

#endif