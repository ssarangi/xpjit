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

void LiveRange::printLiveRanges(llvm::Function &F)
{
    for (llvm::Function::iterator bb = F.begin(), bbe = F.end();
        bb != bbe;
        ++bb)
    {
        for (llvm::BasicBlock::iterator ii = bb->begin(), ie = bb->end();
            ii != ie;
            ++ii)
        {
            if (!isInstructionBlackListed(ii))
            {
                LiveRangeInterval *pLRI = findOrCreateLiveRange(ii);
                g_outputStream << "\nLive Range (" << pLRI->instruction_id << "): ";
                pLRI->pInstr->print(g_outputStream());
                g_outputStream.flush();
                g_outputStream << " Start/End Block [ " << pLRI->def_block << " <--> " << pLRI->kill_block << " ]";
                g_outputStream << " BB Live through { ";
                for (int bb : pLRI->range)
                    g_outputStream << bb << ", ";

                g_outputStream << "} kill_id <" << pLRI->def_id << ", " << pLRI->kill_id << ">\n";
            }
        }
    }
}

bool LiveRange::isInstructionBlackListed(llvm::Instruction *pI)
{
    if (llvm::isa<llvm::PHINode>(pI) ||
        llvm::isa<llvm::BranchInst>(pI) ||
        llvm::isa<llvm::ReturnInst>(pI))
        return true;

    return false;
}

bool LiveRange::interferes(llvm::Instruction *pA, llvm::Instruction *pB)
{
    LiveRangeInterval *pLRIa = findOrCreateLiveRange(pA);
    LiveRangeInterval *pLRIb = findOrCreateLiveRange(pB);

    return pLRIa->interferes(*pLRIb);
}

LiveRangeInterval* LiveRange::createNewLiveRange(llvm::Value *pV)
{
    llvm::Instruction *pI = llvm::cast<llvm::Instruction>(pV);

    if (isInstructionBlackListed(pI))
        return nullptr;

    LiveRangeInterval *pLRI = new LiveRangeInterval(pI, m_pBlockLayout);
    m_intervals.insert(pLRI);
    m_intervalMap[pI] = pLRI;
    return pLRI;
}

LiveRangeInterval* LiveRange::findOrCreateLiveRange(llvm::Value *pV)
{
    LiveRangeInterval *pLRI = nullptr;

    if (llvm::isa<llvm::PHINode>(pV))
        return nullptr;

    llvm::Instruction *pI = llvm::cast<llvm::Instruction>(pV);

    if (m_intervalMap.find(pI) == m_intervalMap.end())
        pLRI = createNewLiveRange(pI);
    else
        pLRI = m_intervalMap[pI];

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
            m_pBlockLayout->getDomTree()->dominates(pBB, *si))
        {
            g_outputStream << "Successor of " << pBB->getName().str() << " not processed: " << si->getName().str() << "\n";
            assert(0 && "Expected to find Basic Block to be processed before predecessor");
        }

        BasicBlockLiveIn succLiveIn = m_BBLiveIns[*si];

        g_outputStream << "Live ins for Successor BB: " << si->getName() << "\n";
        for (auto liveIn : succLiveIn.live)
        {
            liveIn->print(g_outputStream()); g_outputStream << "\n";
        }

        g_outputStream.flush();

        currentBBLiveIn.live.insert(succLiveIn.live.begin(), succLiveIn.live.end());
    }

    m_BBLiveIns[pBB] = currentBBLiveIn;
}

void LiveRange::addBBToRange(llvm::Value* pV, llvm::BasicBlock *pBlock)
{
    if (llvm::Instruction *pI = llvm::dyn_cast<llvm::Instruction>(pV))
    {
        if (m_intervalMap.find(pI) == m_intervalMap.end())
            LiveRangeInterval *pLRI = createNewLiveRange(pV);

        LiveRangeInterval* pLRI = m_intervalMap[pI];
        pLRI->add_basic_block(pBlock);
    }
}

bool LiveRange::runOnFunction(llvm::Function &F)
{
    ADD_HEADER("Live Range Analysis");
    m_pBlockLayout = &getAnalysis<BlockLayoutPass>();
    m_pLoopAnalysis = &getAnalysis<LoopAnalysis>();

    std::stack<llvm::BasicBlock*> reverse_order_blocks = m_pBlockLayout->getReverseOrderBlockLayout();

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
                llvm::Value *pPhiOperandFromBB = pPhi->getIncomingValueForBlock(pBB);
                m_BBLiveIns[pBB].live.insert(pPhiOperandFromBB);
            }
        }

        // Iterate over all the livein's currently in BB and add the range
        for (llvm::Value *pV : m_BBLiveIns[pBB].live)
            addBBToRange(pV, pBB);

        // Now iterate through the instructions in reverse order.
        for (llvm::BasicBlock::reverse_iterator i = pBB->rbegin(), e = pBB->rend();
            i != e;
            ++i)
        {
            if (!llvm::isa<llvm::PHINode>(&*i))
                visitInstruction(&*i);
        }

        // Mark the phi operands to have a live through over the entire basic block they are defined in
        llvm::SmallVector<llvm::PHINode*, 5>& phiNodes = m_pBlockLayout->getPhiNodesForBlock(pBB);
        for (llvm::PHINode *pPhi : phiNodes)
            visitPhiNode(pPhi);

        /*
        if b is loop header then
            loopEnd = last block of the loop starting at b
            for each opd in live do
                intervals[opd].addRange(b.from, loopEnd.to)
        */
        NaturalLoopTy *pLoop = nullptr;
        if (m_pLoopAnalysis->isLoopHeader(pBB, pLoop))
        {
            for (llvm::Value* pOpd : m_BBLiveIns[pBB].live)
            {
                if (llvm::Instruction *pI = llvm::dyn_cast<llvm::Instruction>(pOpd))
                {
                    LiveRangeInterval *pLRI = m_intervalMap[pI];
                    pLRI->def_block = std::min((unsigned int)pLRI->def_block, m_pBlockLayout->getBlockID(pLoop->pHeader));
                    pLRI->kill_block = std::max((unsigned int)pLRI->kill_block, m_pBlockLayout->getBlockID(pLoop->pExit));

                    for (auto bb : pLoop->blocks)
                        pLRI->add_basic_block(bb);
                }
            }
        }
    }

    printLiveRanges(F);

    return false;
}

void LiveRange::visitInstruction(llvm::Instruction *pI)
{
    unsigned int iID = m_pBlockLayout->getInstructionID(pI);
    g_outputStream << "Visiting Instruction (ID: " << iID << "): ";
    pI->print(g_outputStream());
    g_outputStream.flush();
    g_outputStream << "\n";

    llvm::BasicBlock *pBB = pI->getParent();

    // Set the live range for the output
    int instruction_offset = m_instructionOffsets[pI];

    // For all the operands of this instruction add to the live set
    for (llvm::Instruction::op_iterator opi = pI->op_begin(), ope = pI->op_end();
        opi != ope;
        ++opi)
    {
        if (llvm::isa<llvm::Instruction>(*opi) && !llvm::isa<llvm::PHINode>(*opi))
        {
            LiveRangeInterval *pLRIOp = findOrCreateLiveRange(*opi);
            pLRIOp->add_kill_block(pI->getParent());
            pLRIOp->add_kill(pI);
            m_BBLiveIns[pBB].live.insert(*opi);
        }
    }

    // If the instruction has no use then there is no point in keeping this instructions live range
    if (pI->getNumUses() > 0)
    {
        LiveRangeInterval *pLRI = findOrCreateLiveRange(pI);
        pLRI->remove_basic_block(pBB);
        // pLRI->add_def_block(pI->getParent());
    }

    // Remove this instruction from the live set of this block
    m_BBLiveIns[pBB].live.erase(pI);
}

void LiveRange::visitPhiNode(llvm::PHINode *pPhi)
{
    g_outputStream << "Visiting Phi: "; pPhi->print(g_outputStream());
    g_outputStream.flush();
    g_outputStream << "\n";

    llvm::BasicBlock *pBB = pPhi->getParent();

    // Remove all the phi operands from this BB
    for (llvm::User::op_iterator opi = pPhi->op_begin(), ope = pPhi->op_end();
        opi != ope;
        ++opi)
    {
        if (!(llvm::isa<llvm::Constant>(opi) || llvm::isa<llvm::PHINode>(opi)))
        {
            LiveRangeInterval *pLRI = findOrCreateLiveRange(*opi);
            llvm::BasicBlock *pDefBB = pPhi->getIncomingBlock(*opi);
            llvm::Instruction *pTerminator = pDefBB->getTerminator();

            pLRI->add_kill_block(pDefBB);
            pLRI->add_kill(pTerminator);

            // Since its a parameter to the phi node it is live throughout the block
            pLRI->add_basic_block(pDefBB);
        }
    }
}