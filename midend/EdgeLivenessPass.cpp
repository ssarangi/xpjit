#include <midend/EdgeLivenessPass.h>

#include <common/debug.h>

#include "common/llvm_warnings_push.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/ADT/SCCIterator.h>
#include <llvm/ADT/PostOrderIterator.h>
#include <llvm/IR/Verifier.h>
#include "common/llvm_warnings_pop.h"

#include <stack>

char EdgeLivenessPass::ID = 0;

// register the printCode class: 
//  - give it a command-line argument
//  - a name
//  - a flag saying that we don't modify the CFG
//  - a flag saying this is not an analysis pass
llvm::RegisterPass<EdgeLivenessPass> T("EdgeLivenessPass", "Edge Liveness Pass", false, true);

EdgeLivenessPass *createEdgeLivenessPass()
{
    EdgeLivenessPass *pLICM = new EdgeLivenessPass();
    return pLICM;
}

void EdgeLivenessPass::initializeDataStructures(llvm::Function &F)
{
    unsigned int numBlocks = F.size();
    m_liveSets.resize(numBlocks);
    for (unsigned int i = 0; i < numBlocks; ++i)
        m_liveSets[i].resize(numBlocks);
}

bool EdgeLivenessPass::isAlive(llvm::BasicBlock *pOrigin, llvm::BasicBlock *pDst, llvm::Instruction *pI)
{
    unsigned int row = m_pBlockLayout->getBlockID(pOrigin);
    unsigned int col = m_pBlockLayout->getBlockID(pDst);
    unsigned int instructionID = m_pBlockLayout->getInstructionID(pI);

    llvm::SmallVector<bool, 50>& edge = m_liveSets[row][col];
    if (edge.size() == 0)
        return false;

    return edge[instructionID];
}

const llvm::SmallVector<bool, 50>& EdgeLivenessPass::getLiveVariables(llvm::BasicBlock *pOrigin, llvm::BasicBlock *pDestination) const
{
    unsigned int row = m_pBlockLayout->getBlockID(pOrigin);
    unsigned int col = m_pBlockLayout->getBlockID(pDestination);

    return m_liveSets[row][col];
}

void EdgeLivenessPass::kill(llvm::BasicBlock *pOrigin, llvm::BasicBlock *pDestination, llvm::Instruction *pI)
{
    unsigned int row = m_pBlockLayout->getBlockID(pOrigin);
    unsigned int col = m_pBlockLayout->getBlockID(pDestination);
    unsigned int instructionID = m_pBlockLayout->getInstructionID(pI);
    llvm::SmallVector<bool, 50>& edge = m_liveSets[row][col];

    edge[instructionID] = false;
}

void EdgeLivenessPass::killForward(llvm::Instruction *pI, llvm::BasicBlock *pBB)
{
    unsigned int instructionID = m_pBlockLayout->getInstructionID(pI);
    for (llvm::succ_iterator succ_bb = llvm::succ_begin(pBB), succ_bb_end = llvm::succ_end(pBB);
        succ_bb != succ_bb_end;
        ++succ_bb)
    {
        if (isAlive(pBB, *succ_bb, pI))
        {
            kill(pBB, *succ_bb, pI);
            killForward(pI, *succ_bb);
        }
    }
}

void EdgeLivenessPass::killBackward(llvm::Instruction *pI, llvm::BasicBlock *pBB)
{
    unsigned int instructionID = m_pBlockLayout->getInstructionID(pI);
    for (llvm::pred_iterator pred_bb = llvm::pred_begin(pBB), pred_bb_end = llvm::pred_end(pBB);
        pred_bb != pred_bb_end;
        ++pred_bb)
    {
        if (isAlive(pBB, *pred_bb, pI))
        {
            kill(pBB, *pred_bb, pI);
            killBackward(pI, *pred_bb);
        }
    }
}

void EdgeLivenessPass::killLiverange(llvm::Instruction *pI, llvm::BasicBlock *pBB)
{
    killForward(pI, pBB);
    killBackward(pI, pBB);
}

void EdgeLivenessPass::setAlive(llvm::BasicBlock *pOrigin, llvm::BasicBlock *pDestination, llvm::Instruction *pI)
{
    unsigned int row = m_pBlockLayout->getBlockID(pOrigin);
    unsigned int col = m_pBlockLayout->getBlockID(pDestination);
    unsigned int instructionID = m_pBlockLayout->getInstructionID(pI);

    llvm::SmallVector<bool, 50>& edge = m_liveSets[row][col];
    if (edge.size() == 0)
        edge.assign(m_numVReg, false);

    edge[instructionID] = true;
}

void EdgeLivenessPass::handleUse(llvm::Instruction *pI, llvm::BasicBlock *pBlock)
{
    if (pI->getParent() != pBlock)
    {
        // If the instruction block number doesn't match the block, then this means that the instruction
        // is defined in a different block and hence all the blocks which lead to this block need to set
        // this instruction as being alive
        for (llvm::pred_iterator pred = llvm::pred_begin(pBlock), pred_end = llvm::pred_end(pBlock);
            pred != pred_end;
            ++pred)
        {
            if (!isAlive(*pred, pBlock, pI))
            {
                setAlive(*pred, pBlock, pI);
                handleUse(pI, *pred);
            }
        }
    }
}

void EdgeLivenessPass::computeLiveness(llvm::BasicBlock *pBB)
{
    for (llvm::BasicBlock::iterator i = pBB->begin(), ie = pBB->end();
        i != ie;
        ++i)
    {
        for (llvm::User::op_iterator op = i->op_begin(), ope = i->op_end();
            op != ope;
            ++op)
        {
            if (!llvm::isa<llvm::Constant>(op))
            {
                unsigned int instrNum = m_pBlockLayout->getInstructionID(*op);
                if (llvm::PHINode *pPhi = llvm::dyn_cast<llvm::PHINode>(i))
                {
                    setAlive(pPhi->getIncomingBlock(*op), pBB, pPhi);
                    handleUse(pPhi, pBB);
                }
                else
                {
                    llvm::Instruction *pI = llvm::dyn_cast<llvm::Instruction>(op);
                    handleUse(pI, pBB);
                }
            }
        }
    }
}

bool EdgeLivenessPass::runOnFunction(llvm::Function &F)
{
    m_pBlockLayout = &getAnalysis<BlockLayoutPass>();

    initializeDataStructures(F);

    for (llvm::Function::iterator bb = F.begin(), be = F.end();
        bb != be;
        ++bb)
    {
        computeLiveness(bb);
    }

    return false;
}


