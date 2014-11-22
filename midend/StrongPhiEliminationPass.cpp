#include "StrongPhiEliminationPass.h"
#include <midend/LiveRange.h>

#include "common/debug.h"

char StrongPhiEliminationPass::ID = 0;

// register the printCode class: 
//  - give it a command-line argument
//  - a name
//  - a flag saying that we don't modify the CFG
//  - a flag saying this is not an analysis pass
llvm::RegisterPass<StrongPhiEliminationPass> T("PhiDominanceForestPass", "Phi Dominance Forest Pass", false, true);

void StrongPhiEliminationPass::computeDFS(llvm::Function &F)
{
    llvm::SmallPtrSet<llvm::DomTreeNode*, 8> frontier;
    llvm::SmallPtrSet<llvm::DomTreeNode*, 8> visited;

    unsigned int time = 0;

    llvm::DominatorTree& DT = getAnalysis<llvm::DominatorTree>();
    llvm::DomTreeNode *pNode = DT.getRootNode();

    std::vector<llvm::DomTreeNode*> worklist;
    worklist.push_back(pNode);

    while (!worklist.empty())
    {
        llvm::DomTreeNode *pCurrNode = worklist.back();
        if (!frontier.count(pCurrNode))
        {
            frontier.insert(pCurrNode);
            ++time;
            m_preorder.insert(std::make_pair(pCurrNode->getBlock(), time));
        }

        bool inserted = false;
        for (llvm::DomTreeNode::iterator I = pCurrNode->begin(), E = pCurrNode->end();
            I != E;
            ++I)
        {
            if (!frontier.count(*I) && !visited.count(*I))
            {
                worklist.push_back(*I);
                inserted = true;
                break;
            }
        }

        if (!inserted)
        {
            frontier.erase(pCurrNode);
            visited.insert(pCurrNode);
            m_maxpreorder.insert(std::make_pair(pCurrNode->getBlock(), time));

            worklist.pop_back();
        }
    }
}


// Checks the interference between 2 instructions. 
static bool interferes(llvm::Instruction *pA, llvm::Instruction *pB, llvm::BasicBlock *pScan, LiveRange &LR, unsigned int mode)
{
    llvm::Instruction *pDef = nullptr;
    llvm::Instruction *pKill = nullptr;

    // The code is still in SSA form at this point, so there is only one definition
    bool interference = false;

    // Walk the block, checking for interferences
    for (llvm::BasicBlock::iterator ii = pScan->begin(), ie = pScan->end();
        ii != ie;
        ++ii)
    {
        llvm::Instruction *pCurr = ii;

        // Same defining block
        if (mode == 0)
        {
            if (pCurr == pA)
            {
                // If we find our first definition, save it
                if (!pDef)
                {
                    pDef = pCurr;
                    // If there's already an unkilled definition, then this is an interference
                }
                else if (!pKill)
                {
                    interference = true;
                    break;
                    // If there's a definition following by a KillInst, then they can't interfere
                }
                else
                {
                    interference = false;
                    break;
                }
            }
            else if (pCurr == pB)
            {
                if (!pDef)
                {
                    pDef = pCurr;
                }
                else if (!pKill)
                {
                    interference = true;
                    break;
                }
                else
                {
                    interference = false;
                    break;
                }

                // Store the kill instructions if they match up with the definition
            }
            else if (pCurr->killsRegister(pA))
        }
    }
}

// processBlock - Determine how to break up Phi's in the current block. Each Phi is
// broken up by some combination of renaming its operands and inserting copies. This
// method is responsible for determining which operands receive what treatment
void StrongPhiEliminationPass::processBlock(llvm::BasicBlock *pBB)
{
    LiveRange& LR = getAnalysis<LiveRange>();
    EdgeLivenessPass& edgeLiveness = getAnalysis<EdgeLivenessPass>();

    // Hold names that have been added to a set in any PHI within this block before
    // the current one
    std::set<llvm::Instruction*> processed_names;

    llvm::BasicBlock::iterator ii = pBB->begin();
    llvm::BasicBlock::iterator ie = pBB->end();
    while (ii != ie && ii->getOpcode() == llvm::Instruction::PHI)
    {
        llvm::PHINode *pPhi = llvm::cast<llvm::PHINode>(ii);

        std::map<llvm::Instruction*, llvm::BasicBlock*> PHIUnion;
        llvm::SmallPtrSet<llvm::BasicBlock*, 8> UnionedBlocks;

        // Iterate over the operands of the Phi Node
        unsigned int numPhiOps = pPhi->getNumIncomingValues();
        for (int i = 0; i < numPhiOps; ++i)
        {
            if (llvm::Instruction *pSrcReg = llvm::dyn_cast<llvm::Instruction>(pPhi->getIncomingValue(i)))
            {
                // Check for trivial interferences via liveness information, allowing us
                // to avoid extra work later.  Any registers that interfere cannot both
                // be in the renaming set, so choose one and add copies for it instead.
                // The conditions are:
                //   1) if the operand is live into the PHI node's block OR
                //   2) if the PHI node is live out of the operand's defining block OR
                //   3) if the operand is itself a PHI node and the original PHI is
                //      live into the operand's defining block OR
                //   4) if the operand is already being renamed for another PHI node
                //      in this block OR
                //   5) if any two operands are defined in the same block, insert copies
                //      for one of them
                if ((edgeLiveness.isLiveIn(pSrcReg, pBB)) ||
                    (edgeLiveness.isLiveOut(pPhi, pSrcReg->getParent())) ||
                    (llvm::isa<llvm::PHINode>(pSrcReg) && edgeLiveness.isLiveIn(pPhi, pSrcReg->getParent())) ||
                    (processed_names.count(pSrcReg)) ||
                    (UnionedBlocks.count(pSrcReg->getParent())))
                {
                    // Add a copy for the selected register
                    llvm::BasicBlock *pFrom = pSrcReg->getParent();
                    m_waiting[pFrom].insert(std::make_pair(pSrcReg, pPhi));
                    m_usedByAnother.insert(pSrcReg);
                }
                else
                {
                    // Add it to the renaming pair
                    PHIUnion.insert(std::make_pair(pSrcReg, pSrcReg->getParent()));
                    UnionedBlocks.insert(pSrcReg->getParent());
                }
            }
        }

        // Compute the dominator forest for the renaming set. This is a forest where the nodes are the registers
        // and the edges representing dominance relations between the defining blocks of the registers
        std::vector<StrongPhiEliminationPass::DomForestNode*> DF = computeDomForest(PHIUnion, pBB);

        // Walk the DomForest to resolve interferences at an inter-block level. This will remove registers from the
        // renaming set (and insert copies for them) if interferences are found.
        std::vector<std::pair<llvm::Instruction*, llvm::Instruction*>> localInterferences;
        processPHIUnion(pPhi, PHIUnion, DF, localInterferences);

        // If one of the inputs is defined in the same block as the current PHI then we need to check for a local
        // interference between that input and the PHI
        for (std::map<llvm::Instruction*, llvm::BasicBlock*>::iterator I = PHIUnion.begin(), E = PHIUnion.end();
            I != E;
            ++I)
        {
            if (I->first->getParent() == pPhi->getParent())
                localInterferences.push_back(std::make_pair(I->first, pPhi));
        }

        // The dominator forest walk may have returned some register pairs whose interference cannot be determined
        // from dominator analysis. We now examine these pairs for local interferences.
        for (std::vector<std::pair<llvm::Instruction*, llvm::Instruction*>>::iterator I =
            localInterferences.begin(), E = localInterferences.end(); I != E; ++I)
        {
            std::pair<llvm::Instruction*, llvm::Instruction*> p = *I;

            llvm::DominatorTree& DT = getAnalysis<llvm::DominatorTree>();

            // Determine the block we need to scan and the relationship between the two registers
            llvm::BasicBlock *pScan = nullptr;
            unsigned int mode = 0;
            if (p.first->getParent() == p.second->getParent())
            {
                pScan = p.first->getParent();
                mode = 0; // Same block
            }
            else if (DT.dominates(p.first->getParent(), p.second->getParent()))
            {
                pScan = p.second->getParent();
                mode = 1; // First dominates second
            }
            else
            {
                pScan = p.first->getParent();
                mode = 2; // Second dominates first
            }

            // If there's an interference, we need to insert copies
            if (interferes(p.first, p.second, pScan, LI, mode))
            {
                // Insert the copies for first
                for (int i = 0; i < pPhi->getNumIncomingValues(); ++i)
                {
                    if (pPhi->getIncomingValue(i) == p.first)
                    {
                        llvm::Instruction *pSrcReg = p.first;
                        llvm::BasicBlock *pFrom = llvm::cast<llvm::Instruction>(pPhi->getIncomingValue(i))->getParent();

                        m_waiting[pFrom].insert(std::make_pair(pSrcReg, pPhi));
                        m_usedByAnother.insert(pSrcReg);
                        PHIUnion.erase(pSrcReg);
                    }
                }
            }
        }

        // Add the remaining set for this PHI node to our overall renaming information
        for (std::map<llvm::Instruction*, llvm::BasicBlock*>::iterator QI = PHIUnion.begin(), QE = PHIUnion.end();
            QI != QE;
            ++QI)
        {
            g_outputStream << "Adding renaming: " << QI->first << " -> " << pPhi << "\n";
        }

        m_renameSets.insert(std::make_pair(pPhi, PHIUnion));

        // Remember which registers are already renamed, so that we don't try to rename them for another PHI node in this
        // block
        for (std::map<llvm::Instruction*, llvm::BasicBlock*>::iterator I = PHIUnion.begin(),
            E = PHIUnion.end(); I != E;
            ++I)
        {
            processed_names.insert(I->first);
        }

        ++ii;
    }
}

bool StrongPhiEliminationPass::runOnFunction(llvm::Function &F)
{
    LiveRange& LI = getAnalysis<LiveRange>();

    // Compute DFS numbers of each block
    computeDFS(F);

    // Determine which Phi node operands need copies
    for (llvm::Function::iterator bb = F.begin(), bbe = F.end();
        bb != bbe;
        ++bb)
    {
        if (!bb->empty() && bb->begin()->getOpcode() == llvm::Instruction::PHI)
            processBlock(bb);
    }

    return false;
}
