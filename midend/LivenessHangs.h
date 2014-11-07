#ifndef __LIVENESS__
#define __LIVENESS__


#define DEBUG_TYPE "printCode"

#include <llvm/Pass.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Instruction.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/ADT/Statistic.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/IR/User.h>
#include <llvm/IR/Instructions.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/CFG.h>

#include <set>

extern llvm::DenseMap<const llvm::Instruction*, int> instMap;

void print_elem(const llvm::Instruction* i);

class genKill
{
public:
    std::set<const llvm::Instruction*> gen;
    std::set<const llvm::Instruction*> kill;
};

class beforeAfter
{
public:
    std::set<const llvm::Instruction*> before;
    std::set<const llvm::Instruction*> after;
};

class LivenessHangs : public llvm::FunctionPass
{
private:

    void addToMap(llvm::Function &F);

    void computeBBGenKill(llvm::Function &F, llvm::DenseMap<const llvm::BasicBlock*, genKill> &bbMap);

    // Do this using a work-list algorithm (where the items in the work-list are basic blocks).
    void computeBBBeforeAfter(
        llvm::Function &F,
        llvm::DenseMap<const llvm::BasicBlock*, genKill> &bbGKMap,
        llvm::DenseMap<const llvm::BasicBlock*, beforeAfter> &bbBAMap);

    void computeIBeforeAfter(
        llvm::Function &F,
        llvm::DenseMap<const llvm::BasicBlock*, beforeAfter> &bbBAMap,
        llvm::DenseMap<const llvm::Instruction*, beforeAfter> &iBAMap);

public:
    static char ID; // Pass identification, replacement for typeid
    LivenessHangs()
        : llvm::FunctionPass(ID)
    {}

    virtual bool runOnFunction(llvm::Function &F);

    virtual void print(std::ostream &O, const llvm::Module *M) const
    {
        O << "This is printCode.\n";
    }

    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const
    {
    };

};

#endif