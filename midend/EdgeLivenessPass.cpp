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
    EdgeLivenessPass *pEdgeLivenessPass = new EdgeLivenessPass();
    return pEdgeLivenessPass;
}

bool EdgeLivenessPass::isAlive(llvm::BasicBlock *pOrigin, llvm::BasicBlock *pDst, llvm::Instruction *pI)
{
    bool is_alive = false;
    if (m_liveSets.find(pOrigin) != m_liveSets.end() &&
        m_liveSets[pOrigin].find(pDst) != m_liveSets[pOrigin].end() &&
        m_liveSets[pOrigin][pDst].find(pI) != m_liveSets[pOrigin][pDst].end())
    {
        is_alive = m_liveSets[pOrigin][pDst][pI];
    }

    return is_alive;
}

bool EdgeLivenessPass::isLiveIn(llvm::Instruction *pI, llvm::BasicBlock *pBlock)
{
    bool is_live_in = false;

    llvm::BasicBlock *pOrigin = pI->getParent();
    llvm::BasicBlock *pDst = pBlock;

    if (m_liveSets.find(pOrigin) != m_liveSets.end() &&
        m_liveSets[pOrigin].find(pDst) != m_liveSets[pOrigin].end() &&
        m_liveSets[pOrigin][pDst].find(pI) != m_liveSets[pOrigin][pDst].end())
    {
        is_live_in = m_liveSets[pOrigin][pDst][pI];
    }

    return is_live_in;
}

bool EdgeLivenessPass::isLiveOut(llvm::Instruction *pI, llvm::BasicBlock *pBlock)
{
    llvm::BasicBlock *pOrigin = pBlock;

    bool present = false;

    for (llvm::succ_iterator succ_bb = llvm::succ_begin(pOrigin), succ_end = llvm::succ_end(pOrigin);
        succ_bb != succ_end;
        ++succ_bb)
    {
        unsigned int col = m_pBlockLayout->getBlockID(*succ_bb);
        llvm::BasicBlock *pDst = *succ_bb;

        present |= m_liveSets[pOrigin][pDst][pI];
        if (present)
            break;
    }

    return present;
}

llvm::DenseMap<llvm::Instruction*, bool>& EdgeLivenessPass::getLiveVariables(llvm::BasicBlock *pOrigin, llvm::BasicBlock *pDestination)
{
    return m_liveSets[pOrigin][pDestination];
}

void EdgeLivenessPass::kill(llvm::BasicBlock *pOrigin, llvm::BasicBlock *pDestination, llvm::Instruction *pI)
{
    m_liveSets[pOrigin][pDestination][pI] = false;
}

void EdgeLivenessPass::killForward(llvm::Instruction *pI, llvm::BasicBlock *pBB)
{
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

void EdgeLivenessPass::setAlive(llvm::BasicBlock *pOrigin, llvm::BasicBlock *pDst, llvm::Instruction *pI)
{
    m_liveSets[pOrigin][pDst][pI] = true;
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
                if (llvm::PHINode *pPhi = llvm::dyn_cast<llvm::PHINode>(i))
                {
                    setAlive(pPhi->getIncomingBlock(*op), pBB, pPhi);
                    handleUse(pPhi, pBB);
                }
                else if (llvm::Instruction *pI = llvm::dyn_cast<llvm::Instruction>(op))
                {
                    handleUse(pI, pBB);
                }
            }
        }
    }
}

bool EdgeLivenessPass::runOnFunction(llvm::Function &F)
{
    m_pBlockLayout = &getAnalysis<BlockLayoutPass>();

    ADD_HEADER("Edge Liveness Pass");

    for (llvm::Function::iterator bb = F.begin(), be = F.end();
        bb != be;
        ++bb)
    {
        computeLiveness(bb);
    }

    return false;
}


