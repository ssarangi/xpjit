#ifndef __MIPS_PATTERN_MATCH__
#define __MIPS_PATTERN_MATCH__

#include <common/llvm_warnings_push.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Dominators.h>
#include <llvm/IR/InstVisitor.h>
#include <llvm/ADT/DenseMap.h>
#include <common/llvm_warnings_pop.h>

#include <vector>

struct Pattern
{
    virtual void Emit() = 0;
    virtual ~Pattern() {}
};

struct PM_SDAG
{
    PM_SDAG(Pattern* pattern, llvm::Instruction *pRoot) : m_pPattern(pattern), m_pRoot(pRoot)
    {

    }

    Pattern*           m_pPattern;
    llvm::Instruction* m_pRoot;
};

struct PM_BasicBlock
{
    unsigned int id;
    llvm::BasicBlock* pBB;
    std::vector<PM_SDAG> m_dags;
};

struct PM_FuncBasicBlocks
{
    PM_FuncBasicBlocks() { }
    ~PM_FuncBasicBlocks()
    {
        for (auto block : m_blocks)
        {
            delete block;
        }
    }

    std::vector<PM_BasicBlock*> m_blocks;
    llvm::DenseMap<llvm::BasicBlock*, PM_BasicBlock*> m_blockMap;

    void addBlock(unsigned int id, llvm::BasicBlock* pBB, PM_BasicBlock* pPMBB)
    {
        pPMBB->id = id;
        pPMBB->pBB = pBB;
        m_blocks.push_back(pPMBB);
        m_blockMap[pBB] = pPMBB;
    }
};

class MipsPatternMatch : public llvm::ModulePass, public llvm::InstVisitor<MipsPatternMatch>
{
public:
    static char ID; // Pass identification, replacement for typeid
    MipsPatternMatch() : llvm::ModulePass(ID) {}

    void getAnalysisUsage(llvm::AnalysisUsage &AU) const override
    {
        AU.setPreservesAll();
        AU.addRequired<llvm::DominatorTreeWrapperPass>();
    }

    bool runOnModule(llvm::Module &M) override;


    void visitBranchInstruction(llvm::BranchInst &BI);

private:
    void codeGenBlock(llvm::BasicBlock *pBB);
    void codeGenNode(llvm::DomTreeNode *pDomNode);
    void createBasicBlocks(llvm::Function *pFunc);

private:
    llvm::DenseMap<llvm::Function*, PM_FuncBasicBlocks*> m_funcBlocks;
};

#endif