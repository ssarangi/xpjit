#include <midend/BlockLayoutPass.h>
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

char BlockLayoutPass::ID = 0;

// register the printCode class: 
//  - give it a command-line argument
//  - a name
//  - a flag saying that we don't modify the CFG
//  - a flag saying this is not an analysis pass
llvm::RegisterPass<BlockLayoutPass> T("BlockLayoutPass", "Block Layout Analysis Pass", false, true);

bool BlockLayoutPass::runOnFunction(llvm::Function &F)
{
    m_pDT = &getAnalysis<llvm::DominatorTreeWrapperPass>().getDomTree();
    m_pPDT = &getAnalysis<llvm::PostDominatorTree>();

    ADD_HEADER(std::string("Block Layout Pass: ") + F.getName().str());

    llvm::SmallPtrSet<llvm::BasicBlock*, 16> visited;
    llvm::BasicBlock *pEntryBB = F.begin();

    unsigned int bb_no = 1;
    unsigned int instructionID = 0;
    for (llvm::df_ext_iterator<llvm::BasicBlock*, llvm::SmallPtrSet<llvm::BasicBlock*, 16>>
        di = llvm::df_ext_begin(pEntryBB, visited), de = llvm::df_ext_end(pEntryBB, visited);
        di != de;
        ++di)
    {
        g_outputStream << di->getName() << " ; " << bb_no << "\n";
        m_reverseOrderBlockLayout.push(*di);
        m_blockToId[*di] = bb_no;
        m_idToBlock[bb_no++] = *di;

        unsigned int instruction_offset = 0;
        for (llvm::BasicBlock::iterator ii = di->begin(), ie = di->end();
            ii != ie;
            ++ii)
        {
            if (llvm::PHINode *pPhi = llvm::dyn_cast<llvm::PHINode>(ii))
                m_PhiNodesInBB[*di].push_back(pPhi);

            ii->print(g_outputStream());
            g_outputStream << "; {" << instructionID << " , " << instruction_offset << " }\n";
            m_instructionToOffset[ii] = instruction_offset++;
            m_instructionToID[ii] = instructionID++;
        }
    }
    
    return false;
}

unsigned int BlockLayoutPass::getBlockID(llvm::BasicBlock *pBB)
{
    assert(m_blockToId.find(pBB) != m_blockToId.end());
    return m_blockToId[pBB];
}

llvm::BasicBlock* BlockLayoutPass::getBlock(unsigned int blockID)
{
    assert(m_idToBlock.find(blockID) != m_idToBlock.end());
    return m_idToBlock[blockID];
}

unsigned int BlockLayoutPass::getInstructionID(llvm::Instruction *pI)
{
    assert(m_instructionToID.find(pI) != m_instructionToID.end());
    return m_instructionToID[pI];
}

unsigned int BlockLayoutPass::getInstructionID(llvm::Value *pV)
{
    assert(m_instructionToID.find(pV) != m_instructionToID.end());
    return m_instructionToID[pV];
}

unsigned int BlockLayoutPass::getInstructionOffset(llvm::Instruction *pI)
{
    assert(m_instructionToOffset.find(pI) != m_instructionToOffset.end());
    return m_instructionToOffset[pI];
}

llvm::BasicBlock* BlockLayoutPass::getBlockFromInstructionID(unsigned int instr_id)
{
    assert(m_instructionIDtoBlock.find(instr_id) != m_instructionIDtoBlock.end());
    return m_instructionIDtoBlock[instr_id];
}

std::stack<llvm::BasicBlock*> BlockLayoutPass::getReverseOrderBlockLayout()
{
    return m_reverseOrderBlockLayout;
}

llvm::SmallVector<llvm::PHINode*, 5> BlockLayoutPass::getPhiNodesForBlock(llvm::BasicBlock *pBB)
{
    if (m_PhiNodesInBB.find(pBB) == m_PhiNodesInBB.end())
    {
        llvm::SmallVector<llvm::PHINode*, 5> noPhi;
        return noPhi;
    }

    return m_PhiNodesInBB[pBB];
}