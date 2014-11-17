#include "LICM.h"
#include <common/debug.h>

#include "common/llvm_warnings_push.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/ADT/SCCIterator.h>
#include <llvm/ADT/PostOrderIterator.h>
#include <llvm/IR/Verifier.h>
#include "common/llvm_warnings_pop.h"

#include <stack>

char LICM::ID = 0;

// register the printCode class: 
//  - give it a command-line argument
//  - a name
//  - a flag saying that we don't modify the CFG
//  - a flag saying this is not an analysis pass
llvm::RegisterPass<LICM> T("LoopInvariantCodeMotion", "Loop Invariant Code motion", false, false);

LICM *createNewLoopInvariantCodeMotionPass()
{
    LICM *pLICM = new LICM();
    return pLICM;
}

bool LICM::notToRemoveInst(llvm::Instruction *pI)
{
    if (llvm::isa<llvm::BranchInst>(pI) ||
        llvm::isa<llvm::PHINode>(pI))
        return true;

    return false;
}

//bool LICM::doesInstructionUseLoopInductionVar(llvm::Instruction *pI, llvm::DenseMap<llvm::Value*, bool>& induction_var_map)
//{
//    g_outputStream() << "\nQuery Instruction: ";
//    pI->print(g_outputStream());
//    g_outputStream.flush();
//
//    if (induction_var_map.find(pI) != induction_var_map.end())
//        return true;
//
//    bool usesInductionVar = false;
//    unsigned int numOps = pI->getNumOperands();
//    for (unsigned int i = 0; i < numOps; ++i)
//    {
//        if (llvm::Instruction *pOpI = llvm::dyn_cast<llvm::Instruction>(pI->getOperand(i)))
//            usesInductionVar |= doesInstructionUseLoopInductionVar(pOpI, induction_var_map);
//
//        if (usesInductionVar)
//            break;
//    }
//
//    if (usesInductionVar)
//        induction_var_map[pI] = true;
//
//    return usesInductionVar;
//}
NaturalLoopTy* LICM::findInnerMostLoopContainingBB(llvm::BasicBlock *pBB, NaturalLoopTy *pOuterLoop)
{
    NaturalLoopTy *pLoop = nullptr;

    for (NaturalLoopTy *pInnerLoop : pOuterLoop->inner_loops)
    {
        if (pLoop == nullptr)
            pLoop = findInnerMostLoopContainingBB(pBB, pInnerLoop);
    }

    if (pLoop == nullptr)
    {
        pLoop = (pOuterLoop->blocks.find(pBB) == pOuterLoop->blocks.end()) ? nullptr : pOuterLoop;
    }

    return pLoop;
}

bool LICM::doesInstructionUseInductionVarInLoop(llvm::Instruction *pI, NaturalLoopTy *pLoop, std::set<llvm::Instruction*>& visitedInst)
{
    if (pI == nullptr || pLoop == nullptr)
        return false;

    if (visitedInst.find(pI) != visitedInst.end())
        return false;

    visitedInst.insert(pI);

    bool usesInductionVar = false;

    if (pLoop->induction_vars.find(pI) == pLoop->induction_vars.end())
    {
        unsigned int numOps = pI->getNumOperands();
        for (unsigned int i = 0; i < numOps; ++i)
        {
            if (llvm::Instruction *pOpI = llvm::dyn_cast<llvm::Instruction>(pI->getOperand(i)))
                usesInductionVar |= doesInstructionUseInductionVarInLoop(pOpI, pLoop, visitedInst);

            if (usesInductionVar)
                break;
        }
    }
    else
        usesInductionVar = true;

    return usesInductionVar;
}

llvm::BasicBlock* LICM::returnTargetBBForInstMove(llvm::Instruction *pI, NaturalLoopTy *pLoop)
{
    if (pLoop == nullptr)
        return nullptr;

    llvm::BasicBlock *pTargetBlockToMoveTo = nullptr;
    //g_outputStream() << "\nQuery Instruction: ";
    //pI->print(g_outputStream());
    //g_outputStream.flush();

    bool uses_induction_var = false;
    NaturalLoopTy *pOutermostLoopToMoveInst = nullptr;

    while (pLoop != nullptr && uses_induction_var == false)
    {
        std::set<llvm::Instruction*> visitedInst;
        uses_induction_var = doesInstructionUseInductionVarInLoop(pI, pLoop, visitedInst);
        
        // Set the last loop found which doesn't use the induction var
        if (uses_induction_var == false)
            pOutermostLoopToMoveInst = pLoop;

        pLoop = pLoop->pParent;
    }

    if (pOutermostLoopToMoveInst != nullptr)
    {
        if (pOutermostLoopToMoveInst->pParent != nullptr)
            pTargetBlockToMoveTo = pOutermostLoopToMoveInst->pParent->pHeader;
        else
            pTargetBlockToMoveTo = m_pDT->getNode(pOutermostLoopToMoveInst->pHeader)->getIDom()->getBlock();
    }

    return pTargetBlockToMoveTo;
}

void LICM::performStrengthReduction(
    NaturalLoopTy *pNaturalLoop,
    std::queue<llvm::Instruction*>& instMoveOrder,
    llvm::DenseMap<llvm::Instruction*, llvm::Instruction*>& instToMoveBefore)
{
    g_outputStream() << "\nStrength Reduction Instruction List for Loop ID: " << pNaturalLoop->ID << "\n";

    llvm::DenseMap<llvm::Value*, bool> induction_var_map;

    for (llvm::Value *pIV : pNaturalLoop->induction_vars)
        induction_var_map[pIV] = true;

    llvm::BasicBlock *pB = *pNaturalLoop->blocks.begin();
    llvm::LLVMContext *pContext = &pB->getContext();

    // Loop over all instructions and if they use any of the induction var then 
    // leave it there otherwise move it to the dominator block outside the loop header
    llvm::BasicBlock *pLoopHeader = *pNaturalLoop->blocks.begin();

    llvm::BasicBlock *pLoopHeaderParent = m_pDT->getNode(pLoopHeader)->getIDom()->getBlock();

    for (llvm::BasicBlock *pB : pNaturalLoop->blocks)
    {
        g_outputStream() << "\nLooking at instructions in BB: " << pB->getName() << "\n";
        g_outputStream.flush();

        for (llvm::BasicBlock::iterator i = pB->begin(), e = pB->end();
            i != e;
            ++i)
        {
            // Find the innermost loop containing the BB for this instruction
            llvm::BasicBlock *pBB = i->getParent();
            NaturalLoopTy *pInnermostLoop = findInnerMostLoopContainingBB(pBB, pNaturalLoop);

            llvm::BasicBlock *pTargetBB = returnTargetBBForInstMove(i, pInnermostLoop);
            if (pTargetBB && !notToRemoveInst(i))
            {
                g_outputStream() << "\nMoving out of Loop: ";
                i->print(g_outputStream());
                g_outputStream() << " { " << pBB->getName() << " --> " << pTargetBB->getName() << " }";
                g_outputStream.flush();

                llvm::Instruction *pTerminator = pTargetBB->getTerminator();
                instToMoveBefore[i] = pTerminator;
                instMoveOrder.push(i);
            }
        }
    }

    for (NaturalLoopTy *pInnerLoop : pNaturalLoop->inner_loops)
        performStrengthReduction(pInnerLoop, instMoveOrder, instToMoveBefore);

    g_outputStream.flush();
}

bool LICM::runOnFunction(llvm::Function &F)
{
    ADD_HEADER("Loop Invariant Code Motion");
    m_pDT = &getAnalysis<llvm::DominatorTreeWrapperPass>().getDomTree();
    m_pLoopAnalysis = &getAnalysis<LoopAnalysis>();

    LoopAnalysis *pLoopAnalysis = &(*m_pLoopAnalysis);

    llvm::DenseMap<llvm::Instruction*, llvm::Instruction*> instMoveMap;
    std::queue<llvm::Instruction*> instMoveOrder;

    unsigned int numOuterLoops = pLoopAnalysis->getNumOuterLoops();

    for (std::set<NaturalLoopTy*>::const_iterator nl = pLoopAnalysis->begin(), nle = pLoopAnalysis->end();
        nl != nle;
        ++nl)
    {
        NaturalLoopTy *pNaturalLoop = *nl;
        std::set<llvm::BasicBlock*> blocksVisited;
        performStrengthReduction(pNaturalLoop, instMoveOrder, instMoveMap);
    }

    while (!instMoveOrder.empty())
    {
        llvm::Instruction *pItoMove = instMoveOrder.front();
        instMoveOrder.pop();

        llvm::Instruction *pWhereToMove = instMoveMap[pItoMove];
        pItoMove->moveBefore(pWhereToMove);
    }

    g_outputStream() << "\n---------------------------------\n";
    g_outputStream() << "After Strength Reduction:\n";
    g_outputStream() << "---------------------------------\n";
    F.getParent()->print(g_outputStream(), nullptr);
    g_outputStream.flush();

    llvm::verifyModule(*F.getParent());

    return true;
}
