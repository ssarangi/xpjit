//===- LiveValues.h - Liveness information for LLVM IR Values. ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interface for the LLVM IR Value liveness
// analysis pass.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_LIVEVALUES_H
#define LLVM_ANALYSIS_LIVEVALUES_H

#include <llvm/Pass.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/IR/Dominators.h>
#include <llvm/IR/Value.h>
#include <llvm/Analysis/LoopInfo.h>

/// LiveValues - Analysis that provides liveness information for
/// LLVM IR Values.
///
class LiveVariables : public llvm::FunctionPass
{
    llvm::DominatorTree *DT;
    llvm::LoopInfo *LI;

    /// Memo - A bunch of state to be associated with a value.
    ///
    struct Memo
    {
        /// Used - The set of blocks which contain a use of the value.
        ///
        llvm::SmallPtrSet<const llvm::BasicBlock *, 4> Used;

        /// LiveThrough - A conservative approximation of the set of blocks in
        /// which the value is live-through, meaning blocks properly dominated
        /// by the definition, and from which blocks containing uses of the
        /// value are reachable.
        ///
        llvm::SmallPtrSet<const llvm::BasicBlock *, 4> LiveThrough;

        /// Killed - A conservative approximation of the set of blocks in which
        /// the value is used and not live-out.
        ///
        llvm::SmallPtrSet<const llvm::BasicBlock *, 4> Killed;
    };

    /// Memos - Remembers the Memo for each Value. This is populated on
    /// demand.
    ///
    llvm::DenseMap<const llvm::Value *, Memo> Memos;

    /// getMemo - Retrieve an existing Memo for the given value if one
    /// is available, otherwise compute a new one.
    ///
    Memo &getMemo(const llvm::Value *V);

    /// compute - Compute a new Memo for the given value.
    ///
    Memo &compute(const llvm::Value *V);

public:
    static char ID;
    LiveVariables();

    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const;
    virtual bool runOnFunction(llvm::Function &F);
    virtual void releaseMemory();

    /// isUsedInBlock - Test if the given value is used in the given block.
    ///
    bool isUsedInBlock(const llvm::Value *V, const llvm::BasicBlock *BB);

    /// isLiveThroughBlock - Test if the given value is known to be
    /// live-through the given block, meaning that the block is properly
    /// dominated by the value's definition, and there exists a block
    /// reachable from it that contains a use. This uses a conservative
    /// approximation that errs on the side of returning false.
    ///
    bool isLiveThroughBlock(const llvm::Value *V, const llvm::BasicBlock *BB);

    /// isKilledInBlock - Test if the given value is known to be killed in
    /// the given block, meaning that the block contains a use of the value,
    /// and no blocks reachable from the block contain a use. This uses a
    /// conservative approximation that errs on the side of returning false.
    ///
    bool isKilledInBlock(const llvm::Value *V, const llvm::BasicBlock *BB);
};


#endif