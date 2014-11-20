#ifndef __BLOCK_LAYOUT__
#define __BLOCK_LAYOUT__

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

class BlockLayoutPass : public llvm::FunctionPass
{
public:
    static char ID; // Pass identification, replacement for typeid
    BlockLayoutPass()
        : llvm::FunctionPass(ID)
        , m_pDT(nullptr)
    {
        initializeDominatorTreeWrapperPassPass(*llvm::PassRegistry::getPassRegistry());
    }

    virtual bool runOnFunction(llvm::Function &F);

    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const
    {
        AU.addRequired<llvm::DominatorTreeWrapperPass>();
        AU.setPreservesAll();
    };

    unsigned int getBlockID(llvm::BasicBlock *pBB);
    llvm::BasicBlock* getBlock(unsigned int blockID);
    unsigned int getInstructionID(llvm::Instruction *pI);
    unsigned int getInstructionID(llvm::Value *pV);
    unsigned int getInstructionOffset(llvm::Instruction *pI);
    std::stack<llvm::BasicBlock*> getReverseOrderBlockLayout();
    llvm::SmallVector<llvm::PHINode*, 5> getPhiNodesForBlock(llvm::BasicBlock *pBB);
    
private:
    llvm::DominatorTree *m_pDT;
    llvm::DenseMap<llvm::BasicBlock*, unsigned int> m_blockToId;
    llvm::DenseMap<unsigned int, llvm::BasicBlock*> m_idToBlock;
    llvm::DenseMap<llvm::Value*, unsigned int> m_instructionToID;
    llvm::DenseMap<llvm::Value*, unsigned int> m_instructionToOffset;
    std::stack<llvm::BasicBlock*> m_reverseOrderBlockLayout;
    llvm::DenseMap<llvm::BasicBlock*, llvm::SmallVector<llvm::PHINode*, 5>> m_PhiNodesInBB;
};

BlockLayoutPass *createBlockLayoutPass();


#endif