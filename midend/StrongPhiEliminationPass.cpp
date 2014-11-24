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

StrongPhiEliminationPass *createStrongPhiEliminationPass()
{
    StrongPhiEliminationPass *pPass = new StrongPhiEliminationPass();
    return pPass;
}

void StrongPhiEliminationPass::computeDFS(llvm::Function &F)
{
    llvm::SmallPtrSet<llvm::DomTreeNode*, 8> frontier;
    llvm::SmallPtrSet<llvm::DomTreeNode*, 8> visited;

    unsigned int counter = 0;

    llvm::DominatorTree &DT = getAnalysis<llvm::DominatorTreeWrapperPass>().getDomTree();

    llvm::DomTreeNode *pNode = DT.getRootNode();

    std::vector<llvm::DomTreeNode*> worklist;
    worklist.push_back(pNode);

    while (!worklist.empty())
    {
        llvm::DomTreeNode *pCurrNode = worklist.back();

        if (!frontier.count(pCurrNode))
        {
            frontier.insert(pCurrNode);
            ++counter;
            m_preorder.insert(std::make_pair(pCurrNode->getBlock(), counter));
            m_reverse_preorder.insert(std::make_pair(counter, pCurrNode->getBlock()));
        }

        bool inserted = false;
        for (llvm::DomTreeNode::iterator pDomNode = pCurrNode->begin(), E = pCurrNode->end();
            pDomNode != E;
            ++pDomNode)
        {
            if (!frontier.count(*pDomNode) && !visited.count(*pDomNode))
            {
                worklist.push_back(*pDomNode);
                inserted = true;
                break;
            }
        }

        if (!inserted)
        {
            frontier.erase(pCurrNode);
            visited.insert(pCurrNode);
            m_maxpreorder.insert(std::make_pair(pCurrNode->getBlock(), counter));

            worklist.pop_back();
        }
    }

    g_outputStream << "Preorder" << "\n";
    g_outputStream << "--------------------\n";

    for (auto bb_order_pair : m_preorder)
        g_outputStream << bb_order_pair.first->getName() << ": " << bb_order_pair.second << "\n";

    g_outputStream << "\nMax Preorder" << "\n";
    g_outputStream << "--------------------\n";

    for (auto bb_order_pair : m_maxpreorder)
        g_outputStream << bb_order_pair.first->getName() << ": " << bb_order_pair.second << "\n";
}

// A helper class that is used to sort registers according to the preorder number of their defining
// blocks
class PreorderSorter
{
private:
    llvm::DenseMap<llvm::BasicBlock*, unsigned int> &preorder;
    llvm::DenseMap<unsigned int, llvm::BasicBlock*> &reverse_preorder;

public:
    PreorderSorter(
        llvm::DenseMap<llvm::BasicBlock*, unsigned int> &p,
        llvm::DenseMap<unsigned int, llvm::BasicBlock*> &rp)
        : preorder(p)
        , reverse_preorder(rp)
    {}

    bool operator()(llvm::Instruction *pA, llvm::Instruction *pB)
    {
        if (pA == pB)
            return false;

        llvm::BasicBlock *pABlock = pA->getParent();
        llvm::BasicBlock *pBBlock = pB->getParent();

        if (preorder[pABlock] < preorder[pBBlock])
            return true;
        else if (preorder[pABlock] > preorder[pBBlock])
            return false;

        return false;
    }
};

std::vector<StrongPhiEliminationPass::DomForestNode*> StrongPhiEliminationPass::computeDominanceForest(std::set<llvm::Instruction*> &phiUnion)
{
    // Create a Virtual root Node. Assume this node has the max DFS out number
    DomForestNode *pVirtualRoot = new DomForestNode(nullptr, nullptr);
    m_maxpreorder.insert(std::make_pair(nullptr, ~0UL));

    // Populate a worklist with the registers
    std::vector<llvm::Instruction*> worklist;
    worklist.reserve(phiUnion.size());

    for (auto phiVar : phiUnion)
        worklist.push_back(phiVar);

    // Sort the registers by the DFS-in number of their defining block
    PreorderSorter PS(m_preorder, m_reverse_preorder);
    std::sort(worklist.begin(), worklist.end(), PS);

    // Create a "current parent" stack and put the virtual root on top of it
    DomForestNode *pCurrParent = pVirtualRoot;
    std::vector<DomForestNode*> stack;
    stack.push_back(pVirtualRoot);

    // Iterate over all the registers in the previously computed order
    for (auto work : worklist)
    {
        unsigned int pre = m_preorder[work->getParent()];
        llvm::BasicBlock *pParentBlock = pCurrParent->getInstruction() ?
            pCurrParent->getInstruction()->getParent() : nullptr;

        // If the DFS-in number of the register is greater than the DFS-out number
        // of the current parent, repeatedly pop the parent stack until it isn't.
        while (pre > m_maxpreorder[pParentBlock])
        {
            stack.pop_back();
            pCurrParent = stack.back();

            pParentBlock = pCurrParent->getInstruction() ?
                pCurrParent->getInstruction()->getParent() : nullptr;
        }

        // Now that we've found the appropriate parent, create a DomForestNode for
        // this register and attach it to the forest
        DomForestNode *pChild = new DomForestNode(work, pCurrParent);

        // Push this new node on the "current parent" stack
        stack.push_back(pChild);
        pCurrParent = pChild;
    }

    // Return a vector containing the children of the virtual root node
    std::vector<DomForestNode*> ret;
    ret.insert(ret.end(), pVirtualRoot->begin(), pVirtualRoot->end());
    return ret;
}

void StrongPhiEliminationPass::checkInitialInterference(
    llvm::PHINode* pPhi,
    llvm::Instruction *pI,
    llvm::DenseMap<llvm::BasicBlock*, std::set<llvm::BasicBlock*>>& blocksConsidered,
    std::set<llvm::Instruction*> &phiUnion)
{
    EdgeLivenessPass &EL = getAnalysis<EdgeLivenessPass>();

    llvm::BasicBlock *pPhiBB = pPhi->getParent();
    llvm::BasicBlock *pInstBB = pI->getParent();
    
    if (!blocksConsidered[pPhiBB].count(pInstBB))
        blocksConsidered[pPhiBB].insert(pInstBB);

        // Case 1:
        // If ai is in the live - in set of b, add a copy from ai to p in
        // Waiting[From(ai)].Note that our liveness analysis distinguishes between 
        // values that flow into b’s φ - nodes and values
        // that flow directly to some other use in b or b’s successors.
        // Only in the latter case will ai be in b’s live - in set.
    if ((EL.isLiveIn(pI, pPhiBB)) ||

        // Case 2:
        // If p is in the live - out set of ai’s defining block, add a copy
        // to Waiting[From(ai)].
        (EL.isLiveOut(pPhi, pInstBB)) ||

        // Case 3:
        // If ai is defined by a φ-node and p is in the live-in set of the
        // block defining ai, add a copy to Waiting[From(ai)]
        (llvm::isa<llvm::PHINode>(pI) &&
        EL.isLiveIn(pPhi, pInstBB)) ||

        // Case 4:
        // If ai has already been added to another set of names because of
        // another φ - node in the current block, add a copy
        // to Waiting[From(ai)].
        (m_operandsUsedInAnotherPhi.count(pI)) ||

        // Case 5:
        // If ai and aj are defined in the same block, then add a copy
        // to either Waiting[From(ai)] or Waiting[From(aj)].
        (blocksConsidered[pPhiBB].count(pInstBB)))
    {
        m_waiting[pInstBB].emplace_back(pPhi, pI);
        m_operandsUsedInAnotherPhi.insert(pI);
    }
    else
    {
        phiUnion.insert(pI);
        blocksConsidered[pPhiBB].insert(pInstBB);
    }
}

// Take a set of candidate registers to be coalesced when decomposing the PHI instruction.
// Use the Dominance Forest to remove the ones that are known to interfere, and flag others
// that need to be checked for local interferences.
void StrongPhiEliminationPass::processPHIUnion(
    llvm::PHINode *pPhi,
    std::set<llvm::Instruction*> &phiUnion, std::vector<StrongPhiEliminationPass::DomForestNode*> &DF,
    std::vector < std::pair < llvm::Instruction*,
    llvm::Instruction* >> &locals)
{
    std::vector<DomForestNode*> workList(DF.begin(), DF.end());
    llvm::SmallPtrSet<DomForestNode*, 4> visited;

    LiveRange &LR = getAnalysis<LiveRange>();
    EdgeLivenessPass &EL = getAnalysis<EdgeLivenessPass>();

    // Walk the Domforest in DF mode
    while (!workList.empty())
    {
        DomForestNode *pDFNode = workList.back();

        visited.insert(pDFNode);

        bool inserted = false;
        for (DomForestNode::iterator CI = pDFNode->begin(), CE = pDFNode->end();
            CI != CE;
            ++CI)
        {
            DomForestNode *pChild = *CI;

            // If the current node is a live out of the defining block of one of its
            // children, insert a copy for it. NOTE: The paper actually calls for a more
            // elaborate heuristic for determining whether to insert copies for the child
            // or the parent. In the interest of simplicity, we just always choose the parent
            if (EL.isLiveOut(pDFNode->getInstruction(), pChild->getInstruction()->getParent()))
            {
                // Insert copies for the parent
                for (llvm::User::op_iterator opi = pPhi->op_begin(), ope = pPhi->op_end();
                    opi != ope;
                    ++opi)
                {
                    if (*opi == pDFNode->getInstruction())
                    {
                        llvm::BasicBlock *pFrom = pDFNode->getInstruction()->getParent();
                        m_waiting[pFrom].emplace_back(std::make_pair(pPhi, pDFNode->getInstruction()));
                        m_operandsUsedInAnotherPhi.insert(pDFNode->getInstruction());
                        phiUnion.erase(pDFNode->getInstruction());
                    }
                }

                // If a node is live-in to the defining block of one of its children, but not live-out
                // then we need to scan that block for local interferences
            }
            else if (EL.isLiveIn(pDFNode->getInstruction(), pChild->getInstruction()->getParent()) ||
                pDFNode->getInstruction()->getParent() == pChild->getInstruction()->getParent())
            {
                locals.push_back(std::make_pair(pDFNode->getInstruction(), pChild->getInstruction()));
            }

            if (!visited.count(pChild))
            {
                workList.push_back(pChild);
                inserted = true;
            }
        }

        if (!inserted) workList.pop_back();
    }
}


/*
The first step to coalescing copies and building live ranges is to
union together the φ-node parameters. 
Thus, given a φ-node, p, defined in block b, with parameters a1
through an, we apply the following simple tests of interference.
These five are not exhaustive, but they handle the simple cases
– any interference found will cause a copy to be inserted; otherwise, 
ai is added to the union.

*/
void StrongPhiEliminationPass::buildInitialLiveRange(llvm::Function &F)
{
    llvm::DenseMap<llvm::BasicBlock*, std::set<llvm::BasicBlock*>> blocksConsidered;

    for (llvm::Function::iterator bb : F)
    {
        std::set<llvm::Instruction*> processed_names;

        for (llvm::BasicBlock::iterator ii : *bb)
        {
            if (llvm::PHINode *pPhi = llvm::dyn_cast<llvm::PHINode>(ii))
            {
                std::set<llvm::Instruction*> phiUnion;

                for (llvm::User::op_iterator opi = pPhi->op_begin(), ope = pPhi->op_end();
                    opi != ope;
                    ++opi)
                {
                    if (llvm::Instruction *pI = llvm::dyn_cast<llvm::Instruction>(opi))
                        checkInitialInterference(pPhi, pI, blocksConsidered, phiUnion);
                }

                // Compute the Dominance Forest for the renaming set. This is a forest where
                // the nodes are the registers and the edges represent dominance relations
                // between the defining blocks of the registers
                std::vector<StrongPhiEliminationPass::DomForestNode*> DF = computeDominanceForest(phiUnion);

                // Once the dom forest has been computed for this phi, walk the domForest to resolve
                // interferences at an inter-block level. This will remove registers from the renaming set
                // (and insert copies for them) if interferences are found.
                std::vector<std::pair<llvm::Instruction*, llvm::Instruction*>> local_interferences;
                processPHIUnion(pPhi, phiUnion, DF, local_interferences);

                // If one of the inputs is defined in the same block as the current Phi then we need to
                // check for a local interference between that input and the phi
                for (auto phiUnionVar : phiUnion)
                {
                    if (phiUnionVar->getParent() == pPhi->getParent())
                    {
                        local_interferences.emplace_back(phiUnionVar, pPhi);
                    }
                }

                // The dominator forest walk may have returned some register pairs whose interference
                // cannot be determined from dominator analysis. We now examine these pairs for local 
                // interference
                for (auto interference : local_interferences)
                {
                    llvm::DominatorTree &DT = getAnalysis<llvm::DominatorTreeWrapperPass>().getDomTree();

                    LiveRange &LR = getAnalysis<LiveRange>();
                    // If there is an interference, we need to insert copies
                    if (LR.interferes(interference.first, interference.second))
                    {
                        // Insert copies for first
                        for (llvm::User::op_iterator opi = pPhi->op_begin(), ope = pPhi->op_end();
                            opi != ope;
                            ++opi)
                        {
                            if (*opi == interference.first)
                            {
                                llvm::Instruction *pSrcReg = interference.first;
                                llvm::BasicBlock *pFrom = pSrcReg->getParent();

                                // m_waiting[pFrom].emplace_back(interference.first, pPhi);
                                m_operandsUsedInAnotherPhi.insert(interference.first);

                                phiUnion.erase(interference.first);
                            }
                        }
                    }
                }

                // Add the renaming set for this PHI node to our overall renaming information
                m_renameSets.insert(std::make_pair(pPhi, phiUnion));

                // Remember which registers are already renamed, so we don't try to rename them
                // for another PHI node in this block
                for (auto I : phiUnion)
                {
                    processed_names.insert(I);
                }
            }
        }
    }
}

bool StrongPhiEliminationPass::runOnFunction(llvm::Function &F)
{
    LiveRange& LI = getAnalysis<LiveRange>();

    ADD_HEADER("Strong Phi Elimination Pass")

    computeDFS(F);

    buildInitialLiveRange(F);

    // Break interferences where two different phis want to coalesce in the same register
    std::set<llvm::Instruction*> seen;
    typedef std::map<llvm::Instruction*, std::set<llvm::Instruction*>> RenameSetType;

    for (RenameSetType::iterator I = m_renameSets.begin(), E = m_renameSets.end(); I != E; ++I)
    {
        for (std::set<llvm::Instruction*>::iterator
            OI = I->second.begin(), OE = I->second.end(); OI != OE;)
        {
            if (!seen.count(*OI))
            {
                seen.insert(*OI);
                ++OI;
            }
            else
            {
                // m_waiting[(*OI)->getParent()].emplace_back(OI, I->first);
                llvm::Instruction *pReg = *OI;
                ++OI;
                I->second.erase(pReg);
            }
        }
    }

    llvm::SmallPtrSet<llvm::BasicBlock*, 16> visited;
    llvm::DominatorTree &DT = getAnalysis<llvm::DominatorTreeWrapperPass>().getDomTree();
    // insertCopies(DT.getRootNode(), visited);

    // TODO: Preserve Live Intervals. Update it accordingly
    for (RenameSetType::iterator I = m_renameSets.begin(), E = m_renameSets.end();
        I != E; ++I)
    {
        while (I->second.size())
        {
            std::set<llvm::Instruction*>::iterator SI = I->second.begin();
            g_outputStream << "Renaming: " << (*SI)->getName() << " -> " << I->first->getName() << "\n";

            if (*SI != I->first)
            {

            }
        }
    }

    return true;
}
