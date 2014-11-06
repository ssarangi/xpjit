// 15-745 S14 Assignment 2: dataflow.h
// Group: akbhanda, zheq
///////////////////////////////////////////////////////////////////////////////

#ifndef __CLASSICAL_DATAFLOW_DATAFLOW_H__
#define __CLASSICAL_DATAFLOW_DATAFLOW_H__

#include <iostream>
#include <queue>
#include <set>

#include "common/llvm_warnings_push.h"
#include <llvm/IR/Instructions.h>
#include <llvm/ADT/BitVector.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/DepthFirstIterator.h>
#include <llvm/ADT/SmallSet.h>
#include <llvm/IR/ValueMap.h>
#include <llvm/IR/CFG.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Pass.h>
#include "common/llvm_warnings_pop.h"

#include "util.h"

enum Meet {
  INTERSECTION,
  UNION
};

enum Direction {
  FORWARDS,
  BACKWARDS
};

enum Top {
  ALL,
  NONE
};

class DataFlowPass : public llvm::FunctionPass
{
 public:
  DataFlowPass(char id, Top top, Meet meet, Direction direction);
  void computeGenKill(const llvm::Function& fn, BlockStates& states);
  void traverseForwards(const llvm::Function& fn, BlockStates& states);
  void traverseBackwards(const llvm::Function& fn, BlockStates& states);
  void meetFunction(const Assignments& in, Assignments& out);
  Assignments getTop(const llvm::Function& fn);
  void display(const llvm::Function& fn, BlockStates& states);
  // data flow API
  virtual Assignments generate(const llvm::BasicBlock& block) = 0;
  virtual Assignments kill(const llvm::BasicBlock& block) = 0;
  virtual void transferFn(const Assignments& generate, const Assignments& kill,
      const Assignments& input, Assignments& output) = 0;
  // pass API
  virtual bool runOnFunction(llvm::Function& F);
  virtual void getAnalysisUsage(llvm::AnalysisUsage& AU) const;

 protected:
  const Top _top;
  const Meet _meet;
  const Direction _direction;
};

#endif
