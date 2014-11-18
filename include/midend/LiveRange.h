#ifndef __LIVERANGE__
#define __LIVERANGE__

/*
Based on the paper - Linear Scan Register Allocation on SSA Form
by Christian Wimmer Michael Franz
*/


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
    std::set<unsigned int> range;
    int instructionOffset;  // offset from beginning of basic block
    llvm::Value *pValue;

public:
    LiveRangeInfo(llvm::Value *pV)
        : pValue(pV)
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
    {}

    virtual bool runOnFunction(llvm::Function &F);

    bool isLive(llvm::Value *pQuery, llvm::Instruction *pInstNode);
    bool isLiveInBlock(llvm::BasicBlock *pDefBB, llvm::BasicBlock *pQueryBB);
    bool isLiveOutBlock(llvm::Value *pQuery, llvm::BasicBlock *pBlock);

    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const
    {
        AU.addRequired<llvm::DominatorTreeWrapperPass>();
        AU.setPreservesAll();
    };

    void visitInstruction(llvm::Instruction *pI);
    void visitPhiNode(llvm::PHINode *pPhi);

private:
    void unionLiveInSetsOfSuccessors(llvm::BasicBlock *pBB);
    void addBBToRange(llvm::Value* pV, int bbNo);
    LiveRangeInfo* createNewLiveRange(llvm::Value *pV);

private:
    llvm::DenseMap<llvm::BasicBlock*, unsigned int> m_blockToId;
    llvm::DenseMap<unsigned int, llvm::BasicBlock*> m_idToBlock;
    
    llvm::DenseMap<llvm::BasicBlock*, BasicBlockLiveIn> m_BBLiveIns;
    llvm::DenseMap<llvm::BasicBlock*, llvm::SmallVector<llvm::PHINode*, 5>> m_PhiNodesInBB;
    
    llvm::DenseMap<llvm::Value*, LiveRangeInfo*> m_intervalMap;
    std::set<LiveRangeInfo*> m_intervals;
    
    llvm::DenseMap<llvm::Instruction*, unsigned int> m_instructionOffsets;
    llvm::DominatorTree *m_pDT;
};

LiveRange *createNewLiveRangePass();

#endif