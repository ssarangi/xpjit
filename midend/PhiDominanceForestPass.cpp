#include "PhiDominanceForestPass.h"

#include "common/debug.h"

char PhiDominanceForestPass::ID = 0;

// register the printCode class: 
//  - give it a command-line argument
//  - a name
//  - a flag saying that we don't modify the CFG
//  - a flag saying this is not an analysis pass
llvm::RegisterPass<PhiDominanceForestPass> T("PhiDominanceForestPass", "Phi Dominance Forest Pass", false, true);

PhiDominanceForestPass::PhiVarNode* PhiDominanceForestPass::createNode(llvm::Instruction *pI)
{
    assert(m_phiCongruentClass.find(pI) != m_phiCongruentClass.end());

    PhiVarNode *pPhiVar = new PhiVarNode(pI);
    m_phiCongruentClass[pI] = pPhiVar;

    return pPhiVar;
}

void PhiDominanceForestPass::buildDominanceForest(llvm::Function &F)
{
    for (llvm::Function::iterator bb = F.begin(), bbe = F.end();
        bb != bbe;
        ++bb)
    {
        for (llvm::BasicBlock::iterator ii = bb->begin(), ie = bb->end();
            ii != ie;
            ++ii)
        {
            if (llvm::PHINode *pPhi = llvm::dyn_cast<llvm::PHINode>(ii))
            {
                createNode(pPhi);

                for (llvm::User::op_iterator opi = pPhi->op_begin(), ope = pPhi->op_end();
                    opi != ope;
                    ++opi)
                {
                    if (llvm::Instruction *pI = llvm::dyn_cast<llvm::Instruction>(opi))
                    {
                        createNode(pI);
                    }
                }
            }
        }
    }
}

bool PhiDominanceForestPass::runOnFunction(llvm::Function &F)
{
    m_pDT = &getAnalysis<llvm::DominatorTreeWrapperPass>().getDomTree();
    m_pEdgeLiveness = &getAnalysis<EdgeLivenessPass>();
    buildDominanceForest(F);

    return false;
}
