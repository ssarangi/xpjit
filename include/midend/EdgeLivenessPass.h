#ifndef __EDGE_LIVENESS__
#define __EDGE_LIVENESS__

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
#include "common/llvm_warnings_pop.h"

#include <set>
#include <stack>

#define UNDEF_REG 65535

class EdgeLivenessPass : public llvm::FunctionPass
{
public:
    static char ID; // Pass identification, replacement for typeid
    EdgeLivenessPass()
        : llvm::FunctionPass(ID)
        , m_pBlockLayout(nullptr)
        , m_numVReg(0)
    {
    }

    virtual bool runOnFunction(llvm::Function &F);

    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const
    {
        AU.addRequired<BlockLayoutPass>();
        AU.setPreservesAll();
    };

    bool isLiveIn(llvm::Instruction *pI, llvm::BasicBlock *pBlock);
    bool isLiveOut(llvm::Instruction *pI, llvm::BasicBlock *pBlock);

    bool isAlive(llvm::BasicBlock *pOrigin, llvm::BasicBlock *pDst, llvm::Instruction *pI);
    llvm::DenseMap<llvm::Instruction*, bool>& getLiveVariables(llvm::BasicBlock *pOrigin, llvm::BasicBlock *pDestination);
    
    void kill(llvm::BasicBlock *pOrigin, llvm::BasicBlock *pDestination, llvm::Instruction *pI);
    void killForward(llvm::Instruction *pI, llvm::BasicBlock *pBB);
    void killBackward(llvm::Instruction *pI, llvm::BasicBlock *pBB);
    void killLiverange(llvm::Instruction *pI, llvm::BasicBlock *pBB);

    void setAlive(llvm::BasicBlock *pOrigin, llvm::BasicBlock *pDestination, llvm::Instruction *pI);

    void computeLiveness(llvm::BasicBlock *pBB);

    void handleUse(llvm::Instruction *pI, llvm::BasicBlock *pBlock);

private:
    BlockLayoutPass *m_pBlockLayout;

    unsigned int m_numVReg;
    llvm::DenseMap<llvm::BasicBlock*, llvm::DenseMap<llvm::BasicBlock*, llvm::DenseMap<llvm::Instruction*, bool>>> m_liveSets;
};

EdgeLivenessPass *createEdgeLivenessPass();

#endif