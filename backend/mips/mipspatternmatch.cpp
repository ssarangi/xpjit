#include "common/debug.h"
#include "mipspatternmatch.h"

char MipsPatternMatch::ID = 0;
static llvm::RegisterPass<MipsPatternMatch>
MPM("MIPSPatternMatch", "Pattern Matching for MIPS", true, true);

bool MipsPatternMatch::runOnModule(llvm::Module &M)
{

    for (llvm::Module::iterator fb = M.begin(), fe = M.end(); fb != fe; ++fb)
    {
        llvm::DominatorTree &DT = getAnalysis<llvm::DominatorTreeWrapperPass>(*fb).getDomTree();
    }

    return false;
}

void MipsPatternMatch::visitBranchInstruction(llvm::BranchInst &BI)
{
    ICARUS_NOT_IMPLEMENTED("Branch Instruction pattern match not implemented")
}

void MipsPatternMatch::codeGenBlock(llvm::BasicBlock *pBB)
{

}

void MipsPatternMatch::codeGenNode(llvm::DomTreeNode *pDomNode)
{

}

void MipsPatternMatch::createBasicBlocks(llvm::Function *pFunc)
{
    PM_FuncBasicBlocks *pFuncBasicBlocks = new PM_FuncBasicBlocks();

    int i = 0;
    for (llvm::BasicBlock& bb : *pFunc)
    {
        PM_BasicBlock *pBasicBlock = new PM_BasicBlock();
        pFuncBasicBlocks->addBlock(i, &bb, pBasicBlock);
        ++i;
    }
}