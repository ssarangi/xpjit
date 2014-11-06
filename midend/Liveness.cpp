
#include "Liveness.h"
#include "common/debug.h"

char Liveness::ID = 0;

// register the printCode class: 
//  - give it a command-line argument
//  - a name
//  - a flag saying that we don't modify the CFG
//  - a flag saying this is not an analysis pass
RegisterPass<Liveness> X("liveVars", "Live vars analysis", false, false);

Liveness *createNewLivenessPass()
{
    Liveness *pL = new Liveness();
    return pL;
}

void Liveness::search(llvm::BasicBlock *pNode)
{
    for (llvm::succ_iterator succ = llvm::succ_begin(pNode), succEnd = llvm::succ_end(pNode); succ != succEnd; ++succ)
    {
        if (m_pDT->dominates(*succ, pNode))
            m_backEdges[pNode] = *succ;
        else
            m_Rv[pNode].push_back(*succ);
    }
}

bool Liveness::isLive(llvm::Value *pQuery, llvm::Instruction *pInstNode)
{
    if (llvm::isa<llvm::Instruction>(pQuery))
        return false;

    bool isLive = false;

    llvm::Instruction *pQInst = llvm::cast<llvm::Instruction>(pQuery);

    llvm::BasicBlock *pQueryBB = pInstNode->getParent();

    // Loop through all the uses of this instruction and see if there is a reduced reachable path from pQuery to a use
    for (llvm::Instruction::use_iterator u = pQInst->use_begin(), ue = pQInst->use_end(); u != ue; ++u)
    {
        llvm::BasicBlock *pUseBB = llvm::cast<llvm::Instruction>(*u)->getParent();
        llvm::SmallVector<llvm::BasicBlock*, 4> reduced_reachable_nodes = m_Rv[pUseBB];

        for (llvm::SmallVector<llvm::BasicBlock*, 4>::iterator bb = reduced_reachable_nodes.begin(), be = reduced_reachable_nodes.end(); bb != be; ++bb)
        {
            // We found a reducible reachable node from the query basic block. So the variable is live at this point.
            if (*bb == pUseBB)
                return true;
        }
    }

    return isLive;
}

bool Liveness::isLiveInBlock(const llvm::Value *pQuery, const llvm::BasicBlock *pBlock)
{
    return false;
}

bool Liveness::isLiveOutBlock(const llvm::Value *pQuery, const llvm::BasicBlock *pBlock)
{
    return false;
}

bool Liveness::runOnFunction(llvm::Function &F)
{
    llvm::DominatorTreeWrapperPass *pDTW = &getAnalysis<llvm::DominatorTreeWrapperPass>();
    m_pDT = &pDTW->getDomTree();

    // Algorithm:
    // 1) Figure out the back edges
    //     a) If a node's successor dominates the node then it's a backedge

    for (llvm::Function::iterator bb = F.begin(), bbend = F.end(); bb != bbend; ++bb)
        search(bb);

    // Display the edges which have been identified as the backedges
    g_outputStream.stream() << "--------------- BackEdges ----------------" << std::endl;
    for (auto backedges : m_backEdges)
    {
        std::string str = backedges.first->getName().str() + ": " + backedges.second->getName().str();
        g_outputStream.stream() << str << std::endl;
    }
    g_outputStream.stream() << "--------------- BackEdges ----------------" << std::endl;

    g_outputStream.stream() << "--------------- ForwardEdges ----------------" << std::endl;
    for (auto edges : m_Rv)
    {
        std::string str = edges.first->getName().str() + ": ";
        for (auto edge : edges.second)
            str += edge->getName().str() + ", ";

        g_outputStream.stream() << str << std::endl;
    }
    g_outputStream.stream() << "--------------- ForwardEdges ----------------" << std::endl;

    g_outputStream.flush();

    return false;
}