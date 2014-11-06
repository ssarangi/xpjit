#ifndef __UTIL_H__
#define __UTIL_H__

#include <iostream>
#include <set>
#include <utility>

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"

class IRAssignment
{
public:
    IRAssignment(const llvm::Value *ptr) : pointer(ptr) { };
    const llvm::Value* pointer;
    
    inline bool operator==(const IRAssignment& rhs) const
    {
        return pointer == rhs.pointer;
    }
    
    inline bool operator<(const IRAssignment& rhs) const
    {
        return pointer < rhs.pointer;
    }
};

typedef std::set<IRAssignment> Assignments;

class BlockState
{
public:
    Assignments in;
    Assignments out;
    Assignments generates;
    Assignments kills;
};

typedef llvm::DenseMap<const llvm::BasicBlock*, BlockState> BlockStates;
typedef std::pair<const llvm::BasicBlock*, BlockState> BlockStatePair;


//
// Static library functions (for reuse)
//

class DataFlowUtil
{
public:
    static Assignments uses(const llvm::BasicBlock& block);
    static Assignments defines(const llvm::BasicBlock& block);
    static Assignments kills(const llvm::BasicBlock& block);
    static Assignments all(const llvm::Function& fn);
    static void setSubtract(Assignments& dest, const Assignments& src);
    static void setUnion(Assignments& dest, const Assignments& src);
    static void setIntersect(Assignments& dest, const Assignments& src);
    static bool setEquals(const Assignments& a, const Assignments& b);
    static void print(const Assignments& assignments);
};

#endif
