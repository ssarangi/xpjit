#include <midend/LiveRange.h>
#include <common/debug.h>

#include <algorithm>
#include <stack>

char LiveRange::ID = 0;

// register the printCode class: 
//  - give it a command-line argument
//  - a name
//  - a flag saying that we don't modify the CFG
//  - a flag saying this is not an analysis pass
llvm::RegisterPass<LiveRange> T("LiveRangeCustomAnalysis", "Live Range Custom analysis", false, true);

LiveRange *createNewLiveRangePass()
{
    LiveRange *pL = new LiveRange();
    return pL;
}

bool LiveRange::isLive(llvm::Value *pQueryValue, llvm::Instruction *pQueryPoint)
{
    return true;
}

bool LiveRange::isLiveInBlock(llvm::BasicBlock *pDefBB, llvm::BasicBlock *pQueryBB)
{
    return true;
}

bool LiveRange::isLiveOutBlock(llvm::Value *pQuery, llvm::BasicBlock *pBlock)
{
    return true;
}

LiveRangeInfo* LiveRange::createNewLiveRange(llvm::Value *pV)
{
    llvm::Instruction *pI = llvm::cast<llvm::Instruction>(pV);
    LiveRangeInfo *pLRI = new LiveRangeInfo(pV);
    pLRI->startBlockNo = m_blockToId[pI->getParent()];
    m_intervals.insert(pLRI);
    m_intervalMap[pV] = pLRI;
    return pLRI;
}

LiveRangeInfo* LiveRange::findOrCreateLiveRange(llvm::Value *pV)
{
    LiveRangeInfo *pLRI = nullptr;
    if (m_intervalMap.find(pV) == m_intervalMap.end())
        pLRI = createNewLiveRange(pV);
    else
        pLRI = m_intervalMap[pV];

    return pLRI;
}

void LiveRange::unionLiveInSetsOfSuccessors(llvm::BasicBlock *pBB)
{
    BasicBlockLiveIn currentBBLiveIn;
    if (m_BBLiveIns.find(pBB) != m_BBLiveIns.end())
        currentBBLiveIn = m_BBLiveIns[pBB];

    for (llvm::succ_iterator si = llvm::succ_begin(pBB), se = llvm::succ_end(pBB);
        si != se;
        ++si)
    {
        // Loop headers will be processed separately. So if this BB is a part of a basic block whose successor is the loop header
        // then just ignore it for now.
        if (m_BBLiveIns.find(*si) == m_BBLiveIns.end() &&
            m_pDT->dominates(pBB, *si))
        {
            g_outputStream << "Successor of " << pBB->getName().str() << " not processed: " << si->getName().str() << "\n";
            assert(0 && "Expected to find Basic Block to be processed before predecessor");
        }

        BasicBlockLiveIn succLiveIn = m_BBLiveIns[*si];
        currentBBLiveIn.liveIns.insert(succLiveIn.liveIns.begin(), succLiveIn.liveIns.end());
    }

    m_BBLiveIns[pBB] = currentBBLiveIn;
}

void LiveRange::addBBToRange(llvm::Value* pV, int bbNo)
{
    if (llvm::isa<llvm::Instruction>(pV))
    {
        if (m_intervalMap.find(pV) == m_intervalMap.end())
            LiveRangeInfo *pLRI = createNewLiveRange(pV);

        LiveRangeInfo* pLRI = m_intervalMap[pV];
        pLRI->addBB(bbNo);
        pLRI->addUseStartBlockNo(bbNo, m_blockToId, m_idToBlock, m_pDT);
    }
}

std::stack<llvm::BasicBlock*> LiveRange::initializeBlockAndInstructionID(llvm::Function &F)
{
    llvm::SmallPtrSet<llvm::BasicBlock*, 16> visited;
    llvm::BasicBlock *pEntryBB = F.begin();

    std::stack<llvm::BasicBlock*> reverse_order_blocks;

    unsigned int bb_no = 1;
    for (llvm::df_ext_iterator<llvm::BasicBlock*, llvm::SmallPtrSet<llvm::BasicBlock*, 16>>
        di = llvm::df_ext_begin(pEntryBB, visited), de = llvm::df_ext_end(pEntryBB, visited);
        di != de;
    ++di)
    {
        reverse_order_blocks.push(*di);
        m_blockToId[*di] = bb_no;
        m_idToBlock[bb_no++] = *di;

        unsigned int instruction_offset = 0;
        for (llvm::BasicBlock::iterator ii = di->begin(), ie = di->end();
            ii != ie;
            ++ii)
        {
            if (llvm::PHINode *pPhi = llvm::dyn_cast<llvm::PHINode>(ii))
                m_PhiNodesInBB[*di].push_back(pPhi);

            m_instructionOffsets[ii] = instruction_offset++;
        }
    }

    return reverse_order_blocks;
}

bool LiveRange::runOnFunction(llvm::Function &F)
{
    ADD_HEADER("Live Range Analysis");
    m_pLoopAnalysis = &getAnalysis<LoopAnalysis>();

    m_pDT = &getAnalysis<llvm::DominatorTreeWrapperPass>().getDomTree();
    m_pPDT = &getAnalysis<llvm::PostDominatorTree>();

    std::stack<llvm::BasicBlock*> reverse_order_blocks = initializeBlockAndInstructionID(F);

    while (!reverse_order_blocks.empty())
    {
        llvm::BasicBlock *pBB = reverse_order_blocks.top();
        reverse_order_blocks.pop();

        g_outputStream << "Visiting BB: " << pBB->getName() << "\n";

        unionLiveInSetsOfSuccessors(pBB);

        // Iterate over phi instructions of every successor
        for (llvm::succ_iterator si = llvm::succ_begin(pBB), se = llvm::succ_end(pBB);
            si != se;
            ++si)
        {
            for (llvm::PHINode *pPhi : m_PhiNodesInBB[*si])
            {
                g_outputStream << "Visiting Phi: "; pPhi->print(g_outputStream());
                g_outputStream.flush();
                g_outputStream << "\n";

                llvm::Value *pPhiOperandFromBB = pPhi->getIncomingValueForBlock(pBB);
                m_BBLiveIns[pBB].liveIns.insert(pPhiOperandFromBB);
            }
        }

        // Iterate over all the livein's currently in BB and add the range
        for (llvm::Value *pV : m_BBLiveIns[pBB].liveIns)
            addBBToRange(pV, m_blockToId[pBB]);

        // Now iterate through the instructions in reverse order.
        for (llvm::BasicBlock::reverse_iterator i = pBB->rbegin(), e = pBB->rend();
            i != e;
            ++i)
        {
            if (llvm::PHINode *pPhi = llvm::dyn_cast<llvm::PHINode>(&*i))
                visitPhiNode(pPhi);
            else
                visitInstruction(&*i);
        }

        /*
        if b is loop header then
            loopEnd = last block of the loop starting at b
            for each opd in live do
                intervals[opd].addRange(b.from, loopEnd.to)
        */
        NaturalLoopTy *pLoop = nullptr;
        if (m_pLoopAnalysis->isLoopHeader(pBB, pLoop))
        {
            for (llvm::Value* pOpd : m_BBLiveIns[pBB].liveIns)
            {
                LiveRangeInfo *pLRI = m_intervalMap[pOpd];
                pLRI->startBlockNo = std::min(pLRI->startBlockNo, (int)m_blockToId[pLoop->pHeader]);
                pLRI->endBlockNo = std::max(pLRI->endBlockNo, (int)m_blockToId[pLoop->pExit]);

                for (auto bb : pLoop->blocks)
                    pLRI->addBB(m_blockToId[bb]);
            }
        }
    }

    // View the Live Ranges by sorting them by starting point
    for (auto interval : m_intervals)
    {
        g_outputStream << "\nLive Range: ";
        (*interval).pValue->print(g_outputStream());
        g_outputStream.flush();
        g_outputStream << " [ " << (*interval).startBlockNo << " <--> " << (*interval).endBlockNo << " ]";
        g_outputStream << " { ";
        for (int bb : (*interval).range)
            g_outputStream << bb << ", ";

        g_outputStream << "}\n";
    }

    return false;
}

void LiveRange::visitInstruction(llvm::Instruction *pI)
{
    g_outputStream << "Visiting Instruction: ";
    pI->print(g_outputStream());
    g_outputStream.flush();
    g_outputStream << "\n";

    llvm::BasicBlock *pBB = pI->getParent();
    unsigned int bbID = m_blockToId[pBB];

    // Set the live range for the output
    int instruction_offset = m_instructionOffsets[pI];

    // For all the operands of this instruction add to the live in set
    for (llvm::Instruction::op_iterator opi = pI->op_begin(), ope = pI->op_end();
        opi != ope;
        ++opi)
    {
        if (llvm::isa<llvm::Instruction>(*opi))
        {
            LiveRangeInfo *pLRIOp = findOrCreateLiveRange(*opi);
            pLRIOp->addBB(bbID);
            pLRIOp->addInstructionOffset(instruction_offset);
            pLRIOp->addUseEndBlockNo(m_blockToId[pI->getParent()], m_blockToId, m_idToBlock, m_pPDT);
        }
    }

    // If the instruction has no use then there is no point in keeping this instructions live range
    if (pI->getNumUses() > 0)
    {
        LiveRangeInfo *pLRI = findOrCreateLiveRange(pI);
        pLRI->addInstructionOffset(instruction_offset);
        pLRI->removeBB(bbID);
        pLRI->addUseEndBlockNo(m_blockToId[pI->getParent()], m_blockToId, m_idToBlock, m_pPDT);
    }

    // Remove this instruction from the live set of this block
    m_BBLiveIns[pBB].liveIns.erase(pI);
}

void LiveRange::visitPhiNode(llvm::PHINode *pPhi)
{
    llvm::BasicBlock *pBB = pPhi->getParent();
    m_BBLiveIns[pBB].liveIns.erase(pPhi);
}