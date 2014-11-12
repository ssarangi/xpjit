#ifndef __LIVENESS__
#define __LIVENESS__

/*
Based on the paper - Fast Liveness Checking for SSA-Form Programs
by Benoit Boissinot , Sebastian Hack , Benoît Dupont De Dinechin , Daniel Grund , Fabrice Rastello
*/


#include "common/llvm_warnings_push.h"
#include <llvm/Pass.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Instruction.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/ADT/Statistic.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/IR/User.h>
#include <llvm/IR/Instructions.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/CFG.h>
#include <llvm/IR/Dominators.h>
#include "common/llvm_warnings_pop.h"

#include <set>

typedef llvm::SmallVector<llvm::SmallVector<unsigned int, 20>, 20> AdjMatrixTy;

class Liveness : public llvm::FunctionPass
{
public:
    static char ID; // Pass identification, replacement for typeid
    Liveness()
        : llvm::FunctionPass(ID)
        , m_pDT(nullptr)
    {}

    virtual bool runOnFunction(llvm::Function &F);

    bool isLive(llvm::Value *pQuery, llvm::Instruction *pInstNode);
    bool isLiveInBlock(llvm::BasicBlock *pDefBB, llvm::BasicBlock *pQueryBB);
    bool isLiveInBlock(llvm::Value *pQueryV, llvm::BasicBlock *pQueryBB);
    bool isLiveOutBlock(llvm::Value *pQuery, llvm::BasicBlock *pBlock);

    void initializeAdjacencyMatrix(llvm::Function &F);
    void computeWarshallTransitiveClosure(llvm::Function &F);

    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const
    {
        AU.addRequired<llvm::DominatorTreeWrapperPass>();
    };

private:
    void addRelevantBackEdges(AdjMatrixTy& reachabilityMatrix, unsigned int queryBBId, unsigned int defBBId);

private:
    llvm::DenseMap<llvm::BasicBlock*, unsigned int> m_blockToId;
    llvm::DenseMap<unsigned int, llvm::BasicBlock*> m_idToBlock;

    llvm::DominatorTree *m_pDT;
    AdjMatrixTy m_adjacencyMatrix;
    AdjMatrixTy m_backEdgesMatrix;
    AdjMatrixTy m_transitiveClosure;
};

Liveness *createNewLivenessPass();

#endif