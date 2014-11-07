#include "LivenessHangs.h"

char LivenessHangs::ID = 0;

// register the printCode class: 
//  - give it a command-line argument
//  - a name
//  - a flag saying that we don't modify the CFG
//  - a flag saying this is not an analysis pass
llvm::RegisterPass<LivenessHangs> X("liveVars", "Live vars analysis",
    false, true);

void LivenessHangs::addToMap(llvm::Function &F)
{
    static int id = 1;
    for (llvm::inst_iterator i = inst_begin(F), E = inst_end(F); i != E; ++i, ++id)
        // Convert the iterator to a pointer, and insert the pair
        instMap.insert(std::make_pair(&*i, id));
}

void LivenessHangs::computeBBGenKill(llvm::Function &F, llvm::DenseMap<const llvm::BasicBlock*, genKill> &bbMap)
{
    for (llvm::Function::iterator b = F.begin(), e = F.end(); b != e; ++b)
    {
        genKill s;
        for (llvm::BasicBlock::iterator i = b->begin(), e = b->end(); i != e; ++i)
        {
            // The GEN set is the set of upwards-exposed uses:
            // pseudo-registers that are used in the block before being
            // defined. (Those will be the pseudo-registers that are defined
            // in other blocks, or are defined in the current block and used
            // in a phi function at the start of this block.) 
            unsigned n = i->getNumOperands();
            for (unsigned j = 0; j < n; j++)
            {
                llvm::Value *v = i->getOperand(j);
                if (llvm::isa<llvm::Instruction>(v))
                {
                    llvm::Instruction *op = llvm::cast<llvm::Instruction>(v);
                    if (!s.kill.count(op))
                        s.gen.insert(op);
                }
            }
            // For the KILL set, you can use the set of all instructions
            // that are in the block (which safely includes all of the
            // pseudo-registers assigned to in the block).
            s.kill.insert(&*i);
        }
        bbMap.insert(std::make_pair(&*b, s));
    }
}

void LivenessHangs::computeBBBeforeAfter(
    llvm::Function &F,
    llvm::DenseMap<const llvm::BasicBlock*, genKill> &bbGKMap,
    llvm::DenseMap<const llvm::BasicBlock*, beforeAfter> &bbBAMap)
{
    llvm::SmallVector<llvm::BasicBlock*, 32> workList;
    workList.push_back(--F.end());

    while (!workList.empty())
    {
        llvm::BasicBlock *b = workList.pop_back_val();
        beforeAfter b_beforeAfter = bbBAMap.lookup(b);
        bool shouldAddPred = !bbBAMap.count(b);
        genKill b_genKill = bbGKMap.lookup(b);

        // Take the union of all successors
        std::set<const llvm::Instruction*> a;
        for (llvm::succ_iterator SI = succ_begin(b), E = succ_end(b); SI != E; ++SI)
        {
            std::set<const llvm::Instruction*> s(bbBAMap.lookup(*SI).before);
            a.insert(s.begin(), s.end());
        }

        if (a != b_beforeAfter.after)
        {
            shouldAddPred = true;
            b_beforeAfter.after = a;
            // before = after - KILL + GEN
            b_beforeAfter.before.clear();
            std::set_difference(a.begin(), a.end(), b_genKill.kill.begin(), b_genKill.kill.end(),
                std::inserter(b_beforeAfter.before, b_beforeAfter.before.end()));
            b_beforeAfter.before.insert(b_genKill.gen.begin(), b_genKill.gen.end());
        }

        if (shouldAddPred)
            for (llvm::pred_iterator PI = pred_begin(b), E = pred_end(b); PI != E; ++PI)
                workList.push_back(*PI);
    }
}

void LivenessHangs::computeIBeforeAfter(
    llvm::Function &F,
    llvm::DenseMap<const llvm::BasicBlock*, beforeAfter> &bbBAMap,
    llvm::DenseMap<const llvm::Instruction*, beforeAfter> &iBAMap)
{
    for (llvm::Function::iterator b = F.begin(), e = F.end(); b != e; ++b)
    {
        llvm::BasicBlock::iterator i = --b->end();
        std::set<const llvm::Instruction*> liveAfter(bbBAMap.lookup(b).after);
        std::set<const llvm::Instruction*> liveBefore(liveAfter);

        while (true)
        {
            // before = after - KILL + GEN
            liveBefore.erase(i);

            unsigned n = i->getNumOperands();
            for (unsigned j = 0; j < n; j++)
            {
                llvm::Value *v = i->getOperand(j);
                if (llvm::isa<llvm::Instruction>(v))
                    liveBefore.insert(llvm::cast<llvm::Instruction>(v));
            }

            beforeAfter ba;
            ba.before = liveBefore;
            ba.after = liveAfter;
            iBAMap.insert(std::make_pair(&*i, ba));

            liveAfter = liveBefore;
            if (i == b->begin())
                break;
            --i;
        }
    }
}

bool LivenessHangs::runOnFunction(llvm::Function &F)
{
    // Iterate over the instructions in F, creating a map from instruction address to unique integer.
    addToMap(F);

    bool changed = false;

    // LLVM Value classes already have use information. But for the sake of learning, we will implement the iterative algorithm.

    llvm::DenseMap<const llvm::BasicBlock*, genKill> bbGKMap;
    // For each basic block in the function, compute the block's GEN and KILL sets.
    computeBBGenKill(F, bbGKMap);

    llvm::DenseMap<const llvm::BasicBlock*, beforeAfter> bbBAMap;
    // For each basic block in the function, compute the block's liveBefore and liveAfter sets.
    computeBBBeforeAfter(F, bbGKMap, bbBAMap);

    llvm::DenseMap<const llvm::Instruction*, beforeAfter> iBAMap;
    computeIBeforeAfter(F, bbBAMap, iBAMap);

    for (llvm::inst_iterator i = inst_begin(F), E = inst_end(F); i != E; ++i)
    {
        beforeAfter s = iBAMap.lookup(&*i);
        llvm::errs() << "%" << instMap.lookup(&*i) << ": { ";
        std::for_each(s.before.begin(), s.before.end(), print_elem);
        llvm::errs() << "} { ";
        std::for_each(s.after.begin(), s.after.end(), print_elem);
        llvm::errs() << "}\n";
    }

    return changed;
}