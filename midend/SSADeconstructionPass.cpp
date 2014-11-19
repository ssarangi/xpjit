#include "SSADeconstructionPass.h"
#include "common/debug.h"

char SSADeconstructionPass::ID = 0;

// register the printCode class: 
//  - give it a command-line argument
//  - a name
//  - a flag saying that we don't modify the CFG
//  - a flag saying this is not an analysis pass
llvm::RegisterPass<SSADeconstructionPass> T("SSADeconstructionPass", "Deconstruction of SSA Pass", false, false);

SSADeconstructionPass *createSSADeconstructionPass()
{
    SSADeconstructionPass *pPass = new SSADeconstructionPass();
    return pPass;
}

bool SSADeconstructionPass::runOnFunction(llvm::Function &F)
{
    ADD_HEADER("SSA Deconstruction Pass");

    convertToCSSA(F);

    g_outputStream() << "CSSA Form\n";
    g_outputStream() << "--------------\n";
    F.print(g_outputStream());
    g_outputStream.flush();

    return true;
}

void SSADeconstructionPass::convertToCSSA(llvm::Function &F)
{
    for (llvm::Function::iterator bb = F.begin(), be = F.end();
        bb != be;
        ++bb)
    {
        for (llvm::BasicBlock::iterator i = bb->begin(), ie = bb->end();
            i != ie;
            ++i)
        {
            if (llvm::PHINode *pPhi = llvm::dyn_cast<llvm::PHINode>(i))
                visitPhi(pPhi);
        }
    }
}

void SSADeconstructionPass::visitPhi(llvm::PHINode *pPhi)
{
    llvm::IRBuilder<> builder(pPhi->getContext());

    // For each operand, insert a mov in the predecessor blocks and then use those in the phi
    llvm::DenseMap<llvm::BasicBlock*, llvm::Value*> pPhiToRemove;

    for (llvm::User::op_iterator op = pPhi->op_begin(), ope = pPhi->op_end();
        op != ope;
        ++op)
    {
        llvm::BasicBlock *pPredBB = pPhi->getIncomingBlock(*op);
        llvm::TerminatorInst *pTerminator = pPredBB->getTerminator();

        // Now insert a new copy instruction here.
        llvm::Value *pC = nullptr;

        llvm::Value *pI = llvm::cast<llvm::Value>(op);
        llvm::Type *pType = pI->getType();

        llvm::Type *pIntTy = builder.getInt32Ty();
        llvm::Type *pFloatTy = builder.getFloatTy();
        llvm::Type *pDoubleTy = builder.getDoubleTy();

        if (pI->getType()->isPointerTy())
            assert(0 && "Pointer types are not handled");

        if (pType == pIntTy)
            pC = builder.getInt32(1);
        else if (pType == pFloatTy)
            pC = llvm::ConstantFP::get(builder.getFloatTy(), 1.0f);
        else
            assert(0 && "Other types not handled yet");

        llvm::Value *pMul = llvm::BitCastInst::Create(llvm::Instruction::CastOps::BitCast, pI, pType, "", pTerminator);

        // Replace the original value in phi with this new instruction we created.
        pPhiToRemove[pPredBB] = pMul;
    }

    for (auto keyval : pPhiToRemove)
    {
        pPhi->removeIncomingValue(keyval.first);
        pPhi->addIncoming(keyval.second, keyval.first);
    }
}