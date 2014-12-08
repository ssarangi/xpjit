#include "RearrangeImmOperands.h"
#include <common/debug.h>

char RearrangeImmOps::ID = 0;

// register the printCode class: 
//  - give it a command-line argument
//  - a name
//  - a flag saying that we don't modify the CFG
//  - a flag saying this is not an analysis pass
llvm::RegisterPass<RearrangeImmOps> T("RearrangeImmOpsPass", "Rearrange Imm Ops Pass", false, false);

RearrangeImmOps *createRearrangeImmOpsPass()
{
    RearrangeImmOps *pRearrangeImmOps = new RearrangeImmOps();
    return pRearrangeImmOps;
}

bool RearrangeImmOps::runOnFunction(llvm::Function &F)
{
    bool hasChanged = false;

    ADD_HEADER("Rearrange Immediate Operands");

    for (llvm::Function::iterator bb = F.begin(), be = F.end();
        bb != be;
        ++bb)
    {
        for (llvm::BasicBlock::iterator ii = bb->begin(), ie = bb->end();
            ii != ie;
            ++ii)
        {

            if (ii->getOpcode() == llvm::Instruction::Add  ||
                ii->getOpcode() == llvm::Instruction::FAdd ||
                ii->getOpcode() == llvm::Instruction::Sub  ||
                ii->getOpcode() == llvm::Instruction::FSub ||
                ii->getOpcode() == llvm::Instruction::Mul  ||
                ii->getOpcode() == llvm::Instruction::FMul ||
                ii->getOpcode() == llvm::Instruction::FDiv)
            {
                llvm::Value *pOp1 = ii->getOperand(0);
                llvm::Value *pOp2 = ii->getOperand(1);

                if (llvm::isa<llvm::Constant>(pOp1) && !llvm::isa<llvm::Constant>(pOp2))
                {
                    // We need to swap the operands
                    ii->setOperand(0, pOp2);
                    ii->setOperand(1, pOp1);
                    hasChanged = true;
                }
            }
        }
    }

    F.print(g_outputStream());
    g_outputStream.flush();

    return hasChanged;
}
