#include <midend/LoopAnalysis.h>
#include <common/debug.h>

#include "common/llvm_warnings_push.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/ADT/SCCIterator.h>
#include <llvm/ADT/PostOrderIterator.h>
#include <llvm/IR/Verifier.h>
#include "common/llvm_warnings_pop.h"

#include <stack>
#include <vector>
#include <queue>

char LoopAnalysis::ID = 0;

// register the printCode class: 
//  - give it a command-line argument
//  - a name
//  - a flag saying that we don't modify the CFG
//  - a flag saying this is not an analysis pass
llvm::RegisterPass<LoopAnalysis> T("LoopAnalysisCustomAnalysis", "Loop Analysis Custom analysis", false, true);

LoopAnalysis *createNewLoopAnalysisPass()
{
    LoopAnalysis *pL = new LoopAnalysis();
    return pL;
}

bool LoopAnalysis::isPhiNodeInductionVar(llvm::PHINode *pPhi)
{
    assert(pPhi->getNumIncomingValues() == 2);

    llvm::BasicBlock *pBB1 = pPhi->getIncomingBlock(0);
    llvm::BasicBlock *pBB2 = pPhi->getIncomingBlock(1);
    llvm::BasicBlock *pCurrentBB = pPhi->getParent();

    bool isFromDominatorBlock = m_pDT->dominates(pCurrentBB, pBB1) | m_pDT->dominates(pCurrentBB, pBB2);
    bool isFromDominatedBlock = m_pDT->dominates(pBB1, pCurrentBB) | m_pDT->dominates(pBB2, pCurrentBB);

    return (isFromDominatorBlock & isFromDominatedBlock);
}

void LoopAnalysis::findBasicLoopInductionVar(NaturalLoopTy *pNaturalLoop)
{
    g_outputStream() << "Induction Variables: \n";

    // In SSA form, we can look at the Phi Nodes at loop header. Any variable
    // whose 1 definition comes from the a dominator block and another one comes
    // from a block it dominates.
    //for (llvm::BasicBlock *pB : pNaturalLoop->blocks)
    //{
    //    for (llvm::BasicBlock::iterator i = pB->begin(), e = pB->end();
    //        i != e;
    //        ++i)
    //    {
    //        if (llvm::PHINode *pPhi = llvm::dyn_cast<llvm::PHINode>(i))
    //        {
    //            if (isPhiNodeInductionVar(pPhi))
    //            {
    //                pNaturalLoop->induction_vars.push_back(pPhi);

    //                pPhi->print(g_outputStream());
    //                g_outputStream() << "\n";
    //                g_outputStream.flush();
    //            }
    //        }
    //    }
    //}

    llvm::BasicBlock *pB = pNaturalLoop->pHeader;
    {
        for (llvm::BasicBlock::iterator i = pB->begin(), e = pB->end();
            i != e;
            ++i)
        {
            if (llvm::PHINode *pPhi = llvm::dyn_cast<llvm::PHINode>(i))
            {
                if (isPhiNodeInductionVar(pPhi))
                {
                    pNaturalLoop->induction_vars.insert(pPhi);

                    pPhi->print(g_outputStream());
                    g_outputStream() << "\n";
                    g_outputStream.flush();
                }
            }
        }
    }
}

void LoopAnalysis::removeInnerLoopNodeDFS(NaturalLoopTy *pLoop, NaturalLoopTy *pParent)
{
    // If there are no inner loops then recursively delete from parent
    if (pLoop->inner_loops.size() == 0)
        pLoop->pParent = pParent;

    std::set<NaturalLoopTy*> loopsToRemove;

    for (NaturalLoopTy *pInnerLoop : pLoop->inner_loops)
    {
        // Run the DFS only if the parent hasn't been found. If the node has no inner loops then wait till all the other nodes finish to see
        // if it belongs to this node or not.
        if (pInnerLoop->pParent == nullptr && pInnerLoop->inner_loops.size() > 0)
            removeInnerLoopNodeDFS(pInnerLoop, pLoop);
    }

    for (NaturalLoopTy *pInnerLoop : pLoop->inner_loops)
    {
        // If the parent has been found and correctly marked, then mark this loop to be removed.
        if (pInnerLoop->pParent && pInnerLoop->pParent != pLoop)
            loopsToRemove.insert(pInnerLoop);
    }

    for (NaturalLoopTy *pTempLoop : loopsToRemove)
        pLoop->inner_loops.erase(pTempLoop);

    for (NaturalLoopTy *pInnerLoop : pLoop->inner_loops)
        pInnerLoop->pParent = pLoop;
}

bool LoopAnalysis::runOnFunction(llvm::Function &F)
{
    ADD_HEADER("Loop Analysis");
    F.getParent()->print(g_outputStream(), nullptr);
    g_outputStream.flush();

    m_pDT = &getAnalysis<llvm::DominatorTreeWrapperPass>().getDomTree();

    llvm::PostDominatorTree *pPDT = &getAnalysis<llvm::PostDominatorTree>();

    // Associate the loops to their outer loops. Currently this is an O(n^2) algorithm till I figure out a better way.
    unsigned int loopID = 0;
    std::set<NaturalLoopTy*> natural_loops_detected;

    for (llvm::Function::iterator bb = F.begin(), be = F.end();
        bb != be;
        ++bb)
    {
        // Identify a back-edge
        for (llvm::succ_iterator succBB = llvm::succ_begin(bb), succBBE = llvm::succ_end(bb);
            succBB != succBBE;
            ++succBB)
        {
            // If the Basic Block loops on itself
            llvm::BasicBlock *pTarget = *succBB;
            llvm::BasicBlock *pSrc = bb;

            // A node can dominate itself too
            if (m_pDT->dominates(pTarget, pSrc))
            {
                g_outputStream() << "ControlFlow BackEdges: " << pSrc->getName().str() << " --> " << (pTarget)->getName().str() << "\n";

                NaturalLoopTy *pNaturalLoop = new NaturalLoopTy();
                pNaturalLoop->ID = loopID++;
                pNaturalLoop->pHeader = pTarget;
                pNaturalLoop->pExit = pSrc;
                pNaturalLoop->blocks.insert(pTarget);
                pNaturalLoop->blocks.insert(pSrc);

                natural_loops_detected.insert(pNaturalLoop);

                if (pTarget != pSrc)
                {
                    std::stack<llvm::BasicBlock*> bb_visited;
                    bb_visited.push(pSrc);

                    while (!bb_visited.empty())
                    {
                        llvm::BasicBlock *pBB = bb_visited.top();
                        bb_visited.pop();

                        for (llvm::pred_iterator pi = llvm::pred_begin(pBB), pe = llvm::pred_end(pBB);
                            pi != pe;
                            ++pi)
                        {
                            if (*pi != pBB && pTarget != pBB && !pNaturalLoop->blocks.count(*pi))
                            {
                                pNaturalLoop->blocks.insert(*pi);
                                bb_visited.push(*pi);

                                g_outputStream() << "\nInserted Block: " << (*pi)->getName() << "\n";
                                g_outputStream.flush();
                            }
                        }
                    }
                }

                findBasicLoopInductionVar(pNaturalLoop);
            }
        }
    }

    llvm::SmallVector<NaturalLoopTy*, 5> loops_to_remove_from_outermost;

    // Now associate inner loops with outer loops if present.
    for (NaturalLoopTy *pLoop : natural_loops_detected)
    {
        for (NaturalLoopTy *pCheckLoop : natural_loops_detected)
        {
            if (pLoop != pCheckLoop)
            {
                // The loop header should dominate the loop header of inner loop and the loop exit should post dominate the loop exit of
                // inner loop
                if (m_pDT->dominates(pLoop->pHeader, pCheckLoop->pHeader) && pPDT->dominates(pLoop->pExit, pCheckLoop->pExit))
                {
                    // Check loop is an inner loop to pLoop. Do not SET PARENT here. It could be wrong due to order of iteration. 
                    // A DFS algorithm is used to compute that.
                    pLoop->inner_loops.insert(pCheckLoop);
                    loops_to_remove_from_outermost.push_back(pCheckLoop);
                }
            }
        }
    }

    for (NaturalLoopTy *pRemoveLoop : loops_to_remove_from_outermost)
        natural_loops_detected.erase(pRemoveLoop);

    // Now do a DFS of the tree structure created to remove some of the innermost loops from the outer loops. for example if Loop 1 --> contains Loop 2 --> contains Loop 3
    // Then Loop 3 will appear both in the list of Loop 2 & Loop 1. We have to remove from Loop 1.
    for (NaturalLoopTy *pLoop : natural_loops_detected)
        removeInnerLoopNodeDFS(pLoop, nullptr);

    m_naturalLoops = natural_loops_detected;
    return false;
}

bool LoopAnalysis::isLoopHeader(llvm::BasicBlock *pBB, NaturalLoopTy* pQueryLoop, NaturalLoopTy* &pLoop)
{
    if (pQueryLoop->pHeader == pBB)
    {
        pLoop = pQueryLoop;
        return true;
    }

    for (NaturalLoopTy *pInnerLoop : pQueryLoop->inner_loops)
    {
        bool is_loop_header = isLoopHeader(pBB, pInnerLoop, pLoop);

        if (is_loop_header)
            return true;
    }

    return false;
}

bool LoopAnalysis::isLoopHeader(llvm::BasicBlock* pBB, NaturalLoopTy* &pLoop)
{
    bool is_loop_header = false;

    for (NaturalLoopTy *pOuterLoop : m_naturalLoops)
    {
        is_loop_header = isLoopHeader(pBB, pOuterLoop, pLoop);
    }

    return is_loop_header;
}