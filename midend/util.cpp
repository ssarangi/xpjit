#include "util.h"

using std::cout;
using std::endl;

//
// For a given BasicBlock, compute which variables are used.
//
Assignments DataFlowUtil::uses(const llvm::BasicBlock& block)
{
    Assignments useSet;
    for (llvm::BasicBlock::const_reverse_iterator it = block.rbegin(); it != block.rend(); ++it)
    {
        const llvm::Instruction& instr = *it;
        const llvm::User* user = &instr;
        // iterate through all operands
        llvm::User::const_op_iterator OI, OE;
        for (OI = user->op_begin(), OE = user->op_end(); OI != OE; ++OI)
        {
            llvm::Value* val = *OI;
            // check if the operand is used
            if (llvm::isa<llvm::Instruction>(val) || llvm::isa<llvm::Argument>(val))
            {
                useSet.insert(IRAssignment(val));
            }
        }
        useSet.erase(IRAssignment(&instr));
    }
    return useSet;
}


//
// For a given BasicBlock, compute which variables are defined.
//
Assignments DataFlowUtil::defines(const llvm::BasicBlock& block)
{
    Assignments defSet;
    for (llvm::BasicBlock::const_iterator it = block.begin(); it != block.end(); ++it)
    {
        // there's no result area for an instr, every instruction is actually a definition
        const llvm::Instruction& instr = *it;
        defSet.insert(IRAssignment(&instr));
    }
    return defSet;
}


Assignments DataFlowUtil::kills(const llvm::BasicBlock& block)
{
    Assignments killSet;
    const llvm::Function& function = *block.getParent();
    for (llvm::BasicBlock::const_iterator it = block.begin(); it != block.end(); ++it)
    {
        const llvm::Instruction& inst = *it;

        for (llvm::Function::const_iterator itrF = function.begin(); itrF != function.end(); ++itrF)
        {
            const llvm::BasicBlock& bb = *itrF;
            if (&bb == &block) {
                for (llvm::BasicBlock::const_iterator itrB = bb.begin(); itrB != bb.end(); ++itrB)
                {
                    const llvm::Instruction& instr = *itrB;
                    if (&inst == &instr)
                    {
                        killSet.insert(IRAssignment(&instr));
                    }
                }
            }
        }
    }

    return killSet;
}


Assignments DataFlowUtil::all(const llvm::Function& fn)
{
    Assignments all;

    // Function arguments are values too.
    for (llvm::Function::const_arg_iterator I = fn.arg_begin(), IE = fn.arg_end();
        I != IE; ++I)
    {
        const llvm::Argument* arg = &(*I);
        IRAssignment assign(arg);
        all.insert(assign);
    }

    // Populate with all instructions.
    // TODO: What do we do with loads and stores? Treat the same for now.
    for (llvm::Function::const_iterator I = fn.begin(), IE = fn.end(); I != IE; ++I)
    {
        const llvm::BasicBlock& block = *I;
        for (llvm::BasicBlock::const_iterator J = block.begin(), JE = block.end();
            J != JE; ++J)
        {
            const llvm::Instruction* instr = &(*J);
            IRAssignment assign(instr);
            all.insert(assign);
        }
    }
    return all;
}


//
// The following functions perform basic set operations in O(n*log(m)) time,
// where m and n are the sizes of the sets. For our purposes, this is fast
// enough.
//
// The result of these operations is stored back into the 1st argument.
//
void DataFlowUtil::setSubtract(Assignments& dest, const Assignments& src)
{
    for (Assignments::const_iterator i = src.begin(); i != src.end(); ++i)
    {
        const IRAssignment& sub = *i;
        dest.erase(sub);
    }
}


void DataFlowUtil::setUnion(Assignments& dest, const Assignments& src)
{
    for (Assignments::const_iterator i = src.begin(); i != src.end(); ++i)
    {
        const IRAssignment& add = *i;
        dest.insert(add);
    }
}


void DataFlowUtil::setIntersect(Assignments& dest, const Assignments& src)
{
    for (Assignments::const_iterator i = src.begin(); i != src.end(); ++i)
    {
        const IRAssignment& test = *i;
        if (dest.count(test) < 1)
        {
            dest.erase(test);
        }
    }
}


//
// Determine if 2 sets contain the same elements.
//
bool DataFlowUtil::setEquals(const Assignments& a, const Assignments& b)
{
    // make sure sets are the same length
    if (a.size() != b.size())
    {
        return false;
    }

    // ensure they contain the same elements
    for (Assignments::const_iterator i = a.begin(); i != a.end(); ++i)
    {
        const IRAssignment& test = *i;
        if (b.count(test) < 1)
        {
            return false;
        }
    }
    return true;
}


//
// Helper/debug function to quickly print out a set of assignments.
// Later if we change how we implement assignment sets, we will have to update
// this function.
//
void DataFlowUtil::print(const Assignments& assignments)
{
    cout << "{ ";
    for (Assignments::const_iterator I = assignments.begin(),
        IE = assignments.end(); I != IE; ++I)
    {
        cout << (*I).pointer->getName().data() << " ";
    }
    cout << "}";
}
