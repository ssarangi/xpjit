#ifndef __MIPS_PATTERN_MATCH__
#define __MIPS_PATTERN_MATCH__

#include <common/llvm_warnings_push.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Dominators.h>
#include <llvm/IR/InstVisitor.h>
#include <common/llvm_warnings_pop.h>

class MipsPatternMatch : public llvm::ModulePass, public llvm::InstVisitor<MipsPatternMatch>
{
public:
    MipsPatternMatch()
    {}

    ~MipsPatternMatch()
    {}

    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const
    {
        AU.addRequired<llvm::DominatorTree>();
    }

    virtual bool runOnModule(llvm::Module &M);

    virtual const char *getPassName() const
    {
        return "MipsPatternMatch";
    }
};

#endif