#include <midend/LiveRange.h>
#include <common/debug.h>

#include <stack>

char LiveRange::ID = 0;

// register the printCode class: 
//  - give it a command-line argument
//  - a name
//  - a flag saying that we don't modify the CFG
//  - a flag saying this is not an analysis pass
llvm::RegisterPass<LiveRange> T("LiveRangeCustomAnalysis", "Live Range Custom analysis", false, true);

LiveRange *createNewLivenessPass()
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

void LiveRange::unionLiveInSetsOfSuccessors(llvm::BasicBlock *pBB)
{
    BasicBlockLiveIn currentBBLiveIn;
    if (m_BBLiveIns.find(pBB) != m_BBLiveIns.end())
        currentBBLiveIn = m_BBLiveIns[pBB];

    for (llvm::succ_iterator si = llvm::succ_begin(pBB), se = llvm::succ_end(pBB);
        si != se;
        ++si)
    {
        if (m_BBLiveIns.find(*si) == m_BBLiveIns.end())
        {
            g_outputStream.stream() << "Successor of " << pBB->getName().str() << " not processed: " << si->getName().str() << std::endl;
            g_outputStream.flush();
            assert(0 && "Expected to find Basic Block to be processed before predecessor");
        }

        BasicBlockLiveIn succLiveIn = m_BBLiveIns[*si];
        currentBBLiveIn.liveIns.insert(succLiveIn.liveIns.begin(), succLiveIn.liveIns.end());
    }

    m_BBLiveIns[pBB] = currentBBLiveIn;
}

void LiveRange::addBBToRange(llvm::Value* pV, int bbNo)
{
    if (m_intervals.find(pV) == m_intervals.end())
    {
        LiveRangeInfo LRI;
        m_intervals[pV] = LRI;
    }

    LiveRangeInfo LRI = m_intervals[pV];
    LRI.addBB(bbNo);
}

bool LiveRange::runOnFunction(llvm::Function &F)
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

    while (!reverse_order_blocks.empty())
    {
        llvm::BasicBlock *pBB = reverse_order_blocks.top();
        reverse_order_blocks.pop();

        unionLiveInSetsOfSuccessors(pBB);

        // Iterate over phi instructions of every successor
        for (llvm::succ_iterator si = llvm::succ_begin(pBB), se = llvm::succ_end(pBB);
            si != se;
            ++si)
        {
            for (llvm::PHINode *pPhi : m_PhiNodesInBB[*si])
            {
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
                intervals[opd].addRange(b.from, loopEnd.to)        */
    }

    return false;
}

void LiveRange::visitInstruction(llvm::Instruction *pI)
{
    llvm::BasicBlock *pBB = pI->getParent();
    unsigned int bbID = m_blockToId[pBB];

    // Set the live range for the output
    int instruction_offset = m_instructionOffsets[pI];
    LiveRangeInfo& LRI = m_intervals[pI];
    LRI.addInstructionOffset(instruction_offset);
    LRI.removeBB(bbID);

    // Remove this instruction from the live set of this block
    m_BBLiveIns[pBB].liveIns.erase(pI);

    // For all the operands of this instruction add to the live in set
    for (llvm::Instruction::op_iterator opi = pI->op_begin(), ope = pI->op_end();
        opi != ope;
        ++opi)
    {
        m_intervals[opi].addBB(bbID);
        m_intervals[opi].addInstructionOffset(instruction_offset);
    }
}

void LiveRange::visitPhiNode(llvm::PHINode *pPhi)
{
    llvm::BasicBlock *pBB = pPhi->getParent();
    m_BBLiveIns[pBB].liveIns.erase(pPhi);
}