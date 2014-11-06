#ifndef __ANALYSIS__
#define __ANALYSIS__

//===- Analysis.cpp - Example code from "Writing an LLVM Pass" ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "Analysis"
#include <llvm/Pass.h>
#include <llvm/PassSupport.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/ADT/Statistic.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/Analysis/ConstantFolding.h>
#include <llvm/ADT/BitVector.h>
#include <llvm/IR/ValueMap.h>
#include <llvm/IR/CFG.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/Casting.h>
#include <cstdio>


struct BasicBlockLivenessInfo
{
    llvm::BitVector *use;
    llvm::BitVector *def;
    llvm::BitVector *in;
    llvm::BitVector *out;
    llvm::BasicBlock *block;

    /**
    * Constructor that builds block information out of LLVM basic block
    * and definition count.
    *
    * @param block LLVM basic block.
    * @param defcount Definition count.
    *
    */
    BasicBlockLivenessInfo(llvm::BasicBlock *block, unsigned defcount)
    {
        this->block = block;
        use = new llvm::BitVector(defcount);
        def = new llvm::BitVector(defcount);
        in = new llvm::BitVector(defcount);
        out = new llvm::BitVector(defcount);
    }
};

struct Analysis : public llvm::FunctionPass
{
    static char ID; // Pass identification, replacement for typeid

    Analysis() :
        llvm::FunctionPass(ID)
    {
    }

    typedef llvm::ValueMap<llvm::Value *, unsigned> LatticeEncoding;

    /**
    * Assign lattice bits to definitions.
    *
    * @param instToLatticeBit Mapping from definitions to lattice bits.
    * @param F LLVM function object.
    *
    */
    void initInstrToLatticeBit(LatticeEncoding &instToLatticeBit, llvm::Function &F);

    typedef llvm::ValueMap<llvm::BasicBlock *, BasicBlockLivenessInfo *> BlockInfoMapping;

    /**
    * Assign information to basic blocks.
    *
    * @param blockToInfo Mapping from LLVM basic blocks to liveness information.
    * @param F LLVM function object.
    *
    */
    void initBlocksInfo(BlockInfoMapping &blockToInfo, unsigned defcount, llvm::Function &F);

    /**
    * Set defs for a given block information object.
    *
    * @param blockInfo Block information object to be filled with def bits.
    * @param instToLatticeBit Lattice bit encoding for the current function.
    *
    */
    void initBlockDef(BasicBlockLivenessInfo *blockInfo, LatticeEncoding &instToLatticeBit);

    /**
    * Set additional defs for the entry block information object.
    *
    * @param blockInfo Entry block information object to be filled with
    *                  argument def bits.
    * @param argCount Current function argument count.
    *
    */
    void initEntryArgDef(BasicBlockLivenessInfo *blockInfo, unsigned argCount);

    /**
    * Set corresponding use bit for all blocks using a given definition.
    *
    * @param use_begin First use of the given definition.
    * @param use_end One-past-last use of the given definition.
    * @param blockToInfo Mapping from LLVM basic blocks to liveness information.
    * @param defInst Definition for which use bits are set
    *                (NULL if argument, not instruction).
    * @param defBit Corresponding bit for the definition in the lattice.
    *
    */
    void markUses(llvm::Value::use_iterator use_begin, llvm::Value::use_iterator use_end,
        BlockInfoMapping &blockToInfo,
        llvm::Instruction *defInst,
        unsigned defBit);

    /**
    * Set use sets for all the blocks.
    *
    * @param blockToInfo Mapping from LLVM basic blocks to liveness information.
    * @param instToLatticeBit Mapping from definitions to lattice bits.
    * @param F LLVM function object.
    */
    void initBlocksUse(BlockInfoMapping &blockToInfo,
        LatticeEncoding &instToLatticeBit,
        llvm::Function &F);

    typedef std::map<std::pair<llvm::BasicBlock *, llvm::BasicBlock *>, llvm::BitVector *> FlowMask;

    /**
    * Set flow mask for the current function.
    *
    * Liveness analysis is tricky using the SSA form because of the phi nodes.
    * A phi node appears to use more than one value, but this is actually
    * flow-sensitive. Basically, a phi node looks - data-flow wise - as a
    * different instruction (using a single value) from each incoming block.
    * Therefore, when computing out set for each incoming block, the in sets
    * for the successors containing phi nodes will be adjusted accordingly by
    * using this mask.
    *
    * @param mask The flow mask between blocks containing phi-nodes and the
    *             incoming blocks.
    * @param blockToInfo Mapping from LLVM basic blocks to liveness information.
    * @param instToLatticeBit Mapping from definitions to lattice bits.
    * @param F LLVM function object.
    */
    void initMask(FlowMask &mask,
        BlockInfoMapping &blockToInfo,
        LatticeEncoding &instToLatticeBit,
        llvm::Function &F);

    void printBitVector(llvm::BitVector *bv);

    virtual bool runOnFunction(llvm::Function &F);

    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const
    {
        AU.setPreservesCFG();
    }

};

llvm::FunctionPass* createAnalysisPass();

#endif