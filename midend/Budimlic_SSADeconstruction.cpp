#include "Budimlic_SSADeconstruction.h"

#include "common/debug.h"

char Budimlic_SSADeconstructionPass::ID = 0;

// register the printCode class: 
//  - give it a command-line argument
//  - a name
//  - a flag saying that we don't modify the CFG
//  - a flag saying this is not an analysis pass
llvm::RegisterPass<Budimlic_SSADeconstructionPass> T("Sreedhar_SSADeconstructionPass", "Deconstruction of SSA Pass by Budimlic", false, false);

Budimlic_SSADeconstructionPass *createBudimlic_SSADeconstructionPass()
{
    Budimlic_SSADeconstructionPass *pPass = new Budimlic_SSADeconstructionPass();
    return pPass;
}

void Budimlic_SSADeconstructionPass::dfs(llvm::DomTreeNode *pDomNode, unsigned int &counter)
{
    // We have already found this BB. Return
    if (m_preorder.find(pDomNode->getBlock()) != m_preorder.end())
        return;

    ++counter;
    m_preorder.insert(std::make_pair(pDomNode->getBlock(), counter));

    for (llvm::DomTreeNode::iterator idomnode = pDomNode->begin(), edomnode = pDomNode->end();
        idomnode != edomnode;
        ++idomnode)
    {
        dfs(*idomnode, counter);
    }

    m_maxpreorder.insert(std::make_pair(pDomNode->getBlock(), counter));
}

void Budimlic_SSADeconstructionPass::computeDFS(llvm::Function &F)
{
    unsigned int counter = 0;

    llvm::DominatorTree &DT = getAnalysis<llvm::DominatorTreeWrapperPass>().getDomTree();

    llvm::DomTreeNode *pNode = DT.getRootNode();
    
    dfs(pNode, counter);
}

void Budimlic_SSADeconstructionPass::checkInitialInterference(
    llvm::PHINode* pPhi,
    llvm::Instruction *pI,
    std::set<llvm::BasicBlock*> blocksConsidered,
    std::set<llvm::Instruction*> &phiUnion)
{
    EdgeLivenessPass &EL = getAnalysis<EdgeLivenessPass>();

    llvm::BasicBlock *pPhiBB = pPhi->getParent();
    llvm::BasicBlock *pInstBB = pI->getParent();

    // Case 1:
    // If ai is in the live-in set of b, add a copy from ai to p in
    // Waiting[From(ai)].Note that our liveness analysis distinguishes between 
    // values that flow into b’s f - nodes and values
    // that flow directly to some other use in b or b’s successors.
    // Only in the latter case will ai be in b’s live - in set.
    if ((EL.isLiveIn(pI, pPhiBB)) ||

        // Case 2:
        // If p is in the live-out set of ai’s defining block, add a copy
        // to Waiting[From(ai)].
        (EL.isLiveOut(pPhi, pInstBB)) ||

        // Case 3:
        // If ai is defined by a phi-node and p is in the live-in set of the
        // block defining ai, add a copy to Waiting[From(ai)]
        (llvm::isa<llvm::PHINode>(pI) && EL.isLiveIn(pPhi, pInstBB)) ||

        // Case 4:
        // If ai has already been added to another set of names because of
        // another phi-node in the current block, add a copy
        // to Waiting[From(ai)].
        (m_operandsUsedInAnotherPhi.count(pI)) ||

        // Case 5:
        // If ai and aj are defined in the same block, then add a copy
        // to either Waiting[From(ai)] or Waiting[From(aj)].
        (m_processedNames.count(pI)) ||
        (blocksConsidered.count(pInstBB)))
    {
        g_outputStream << "Interference: " << pPhi->getName() << " -- " << pI->getName() << "\n";
        m_waiting[pInstBB].emplace_back(pI, pPhi);
        m_operandsUsedInAnotherPhi.insert(pI);
    }
    else
    {
        g_outputStream << "Phi Union: " << pPhi->getName() << " -- " << pI->getName() << "\n";
        phiUnion.insert(pI);
        blocksConsidered.insert(pInstBB);
    }
}

// A helper class that is used to sort registers according to the preorder number of their defining
// blocks
class PreorderSorter
{
private:
    llvm::DenseMap<llvm::BasicBlock*, unsigned int> &preorder;

public:
    PreorderSorter(
        llvm::DenseMap<llvm::BasicBlock*, unsigned int> &p)
        : preorder(p)
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

std::vector<Budimlic_SSADeconstructionPass::DomForestNode*>
Budimlic_SSADeconstructionPass::computeDominanceForest(std::set<llvm::Instruction*> &phiUnion)
{
    // Create a virtual root node. Assume this node has the max DFS out number
    DomForestNode *pVirtualRoot = new DomForestNode(nullptr, nullptr);
    m_maxpreorder.insert(std::make_pair(nullptr, ~0UL));

    // Populate a worklist with the registers
    std::vector<llvm::Instruction*> worklist;
    worklist.reserve(phiUnion.size());

    for (auto phiVar : phiUnion)
        worklist.push_back(phiVar);

    // Sort the registers by the DFS-in number of their defining block
    PreorderSorter PS(m_preorder);
    std::sort(worklist.begin(), worklist.end(), PS);

    // Create a "current parent" stack and put the virtual root on top of it
    DomForestNode *pCurrParent = pVirtualRoot;
    std::stack<DomForestNode*> domForestStack;
    domForestStack.push(pVirtualRoot);

    for (auto work : worklist)
    {
        unsigned int preorder_num = m_preorder[work->getParent()];
        llvm::BasicBlock *pParentBlock = pCurrParent->getInstruction() ?
            pCurrParent->getInstruction()->getParent() : nullptr;

        // If the DFS-in number of the register is greater than the DFS-out number of the
        // current parent, repeatedly pop the parent stack until it isn't.
        while (preorder_num > m_maxpreorder[pParentBlock])
        {
            domForestStack.pop();
            pCurrParent = domForestStack.top();

            pParentBlock = pCurrParent->getInstruction() ?
                pCurrParent->getInstruction()->getParent() : nullptr;
        }

        // Now that we have found a parent, create a DomForestNode for this register and
        // attach it to the forest.
        DomForestNode *pChild = new DomForestNode(work, pCurrParent);

        // Push this new node on the "current parent" stack
        domForestStack.push(pChild);
        pCurrParent = pChild;
    }

    // Return a vector containing the children of the virtual root node
    std::vector<DomForestNode*> ret;
    ret.insert(ret.end(), pVirtualRoot->begin(), pVirtualRoot->end());
    return ret;
}

// Take a set of candidate registers to be coalesced when decomposing the PHI instruction.
// Use the Dominance Forest to remove the ones that are known to interfere, and flag others
// that need to be checked for local interferences.
void Budimlic_SSADeconstructionPass::processPhiUnion(
    llvm::PHINode *pPhi,
    std::set<llvm::Instruction*> phiUnion,
    std::vector<Budimlic_SSADeconstructionPass::DomForestNode*>& DF,
    std::vector<std::pair<llvm::Instruction*, llvm::Instruction*>> &local_interferences)
{
    std::vector<Budimlic_SSADeconstructionPass::DomForestNode*> workList(DF.begin(), DF.end());
    std::set<Budimlic_SSADeconstructionPass::DomForestNode*> visited;

    LiveRange &LR = getAnalysis<LiveRange>();
    EdgeLivenessPass &EL = getAnalysis<EdgeLivenessPass>();

    // Walk the DOM Forest in DF mode
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

            // if variable p is c’ s parent and is in the live-out set of c’ s defining block
            if (EL.isLiveOut(pDFNode->getInstruction(), pChild->getInstruction()->getParent()))
            {
                // If the current node is a live out of the defining block of one of its
                // children, insert a copy for it. NOTE: The paper actually calls for a more
                // elaborate heuristic for determining whether to insert copies for the child
                // or the parent. In the interest of simplicity, we just always choose the parent
                for (llvm::User::op_iterator opi = pPhi->op_begin(), ope = pPhi->op_end();
                    opi != ope;
                    ++opi)
                {
                    if (*opi == pDFNode->getInstruction())
                    {
                        llvm::BasicBlock *pFrom = pDFNode->getInstruction()->getParent();
                        m_waiting[pFrom].emplace_back(std::make_pair(pDFNode->getInstruction(), pPhi));
                        m_operandsUsedInAnotherPhi.insert(pDFNode->getInstruction());
                        phiUnion.erase(pDFNode->getInstruction());
                    }
                }
            }
            // else if parent p is in the live   in set of c’ s defining block or p and c have the same defining block
            // If a node is live-in to the defining block of one of its children, but not live-out
            // then we need to scan that block for local interferences
            else if ((EL.isLiveIn(pDFNode->getInstruction(), pChild->getInstruction()->getParent())) ||
                (pDFNode->getInstruction()->getParent() == pChild->getInstruction()->getParent()))
            {
                // Add the variable pair (p, c) to the list to check for local interference later
                local_interferences.push_back(std::make_pair(pDFNode->getInstruction(), pChild->getInstruction()));
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

void Budimlic_SSADeconstructionPass::processBlock(llvm::Function &F)
{
    LiveRange &LR = getAnalysis<LiveRange>();

    for (llvm::Function::iterator bb : F)
    {
        for (llvm::BasicBlock::iterator ii : *bb)
        {
            if (llvm::PHINode *pPhi = llvm::dyn_cast<llvm::PHINode>(ii))
            {
                std::set<llvm::Instruction*> phiUnion;
                std::set<llvm::BasicBlock*> blocksConsidered;

                for (llvm::User::op_iterator opi = pPhi->op_begin(), ope = pPhi->op_end();
                    opi != ope;
                    ++opi)
                {
                    if (llvm::Instruction *pOpI = llvm::dyn_cast<llvm::Instruction>(opi))
                        checkInitialInterference(pPhi, pOpI, blocksConsidered, phiUnion);
                }

                // Compute the Dominance Forest for the renaming set. This is a forest where
                // the nodes are the registers and the edges represent dominance relations
                // between the defining blocks of the registers
                std::vector<Budimlic_SSADeconstructionPass::DomForestNode*> domForest = computeDominanceForest(phiUnion);

                // Once the dom forest has been computed for this phi, walk the domForest to resolve
                // interferences at an inter-block level. This will remove registers from the renaming set
                // (and insert copies for them) if interferences are found.
                std::vector<std::pair<llvm::Instruction*, llvm::Instruction*>> local_interferences;
                processPhiUnion(pPhi, phiUnion, domForest, local_interferences);

                // If one of the inputs is defined in the same block as the current Phi, then we need to
                // check for local interference between that input and the Phi.
                for (auto input : phiUnion)
                {
                    if (input->getParent() == pPhi->getParent())
                        local_interferences.emplace_back(input, pPhi);
                }

                // Now iterate over the local interference
                for (auto local_interference : local_interferences)
                {
                    llvm::DominatorTree &DT = getAnalysis<llvm::DominatorTreeWrapperPass>().getDomTree();

                    if (LR.interferes(local_interference.first, local_interference.second))
                    {
                        // Insert the copies for the first
                        llvm::BasicBlock *pFrom = local_interference.first->getParent();
                        m_waiting[pFrom].emplace_back(local_interference.first, pPhi);
                        m_operandsUsedInAnotherPhi.insert(local_interference.first);
                        phiUnion.erase(local_interference.first);
                    }
                }

                // Add the renaming set for this PHI node to our overall renaming information
                // Remember which registers are already renamed, so that we don't try to rename them for another Phi
                // node in this block
                for (auto QI : phiUnion)
                {
                    g_outputStream << "Adding Renaming: " << QI->getName() << "\n";
                    m_processedNames.insert(QI);
                }

                m_renameSets[pPhi] = phiUnion;
            }
        }
    }
}

bool Budimlic_SSADeconstructionPass::runOnFunction(llvm::Function &F)
{
    ADD_HEADER("Budimlic - SSA Deconstruction Pass");

    computeDFS(F);
    processBlock(F);

    // Break interferences where two different phis want to coalesce in the same register
    std::set<llvm::Instruction*> seen;
    for (auto I : m_renameSets)
    {
        for (auto OI : I.second)
        {
            if (!seen.count(OI))
            {
                seen.insert(OI);
                ++OI;
            }
            else
            {
                m_waiting[OI->getParent()].emplace_back(OI, I.first);
                ++OI;
                I.second.erase(OI);
                g_outputStream << "Removing Renaming: " << OI->getName() << "\n";
            }
        }
    }

    g_outputStream << "\nDisplaying Waiting Stream\n";
    g_outputStream << "---------------------------------------\n";

    for (auto w : m_waiting)
    {
        for (auto I : w.second)
        {
            g_outputStream << "Waiting: " << I.second->getName() << "\n";
        }
    }

    g_outputStream << "\nDisplaying Renaming Stream\n";
    g_outputStream << "---------------------------------------\n";

    for (auto w : m_renameSets)
    {
        for (auto I : w.second)
        {
            g_outputStream << "Renaming: " << I->getName() << "\n";
        }
    }

    return false;
}
