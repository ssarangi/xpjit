//===------------ FernandoPrintCGF.cpp - Printing prototype ---------------===//
//
// Original code by Fernando Magno Quintao Pereira, fernando at ucla
//
//===----------------------------------------------------------------------===//

// Function
#include <common/llvm_warnings_push.h>
#include <llvm/IR/Function.h>
#include <llvm/Pass.h>
#include <llvm/IR/CFG.h>
#include <llvm/IR/InstVisitor.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/BasicBlock.h>
#include <common/llvm_warnings_pop.h>

// cerr
#include <iostream>

// ostringstream
#include <sstream>
#include <fstream>

#define INST_LEN 20
#define OPRT_LEN 8

class PrintCFG : public llvm::FunctionPass, llvm::InstVisitor<PrintCFG>
{
public:
    static char ID;

public:
    explicit PrintCFG()
        : llvm::FunctionPass(ID)
    {}

    void setOutputFile(std::string filename)
    {
        m_fileStream.open(filename.c_str(), std::ios::out);
        if (m_fileStream.bad())
        {
            std::cout << "Error writing dot file" << std::endl;
            return;
        }
    }

    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const {
        AU.setPreservesAll();
    }

    virtual const char *getPassName() const {
        return "Print machine instructions.";
    }

private:
    /// runOnFunction - Prints information about each basic block.
    bool runOnFunction(llvm::Function &Fn);

    /// Prints the name, number, and neat header for the basic block.
    void visitBasicBlock(llvm::BasicBlock &MBB);

    /// Fills str with n characters like 'c', where n = size(str) + packSize.
    /// It returns the number of characters printed.
    int PrintPackedStr(const char * str, int packSize, char c);
    int PrintPackedStr(std::string str, int packSize, char c);

    /// Gives the name of the basic block
    std::string getBBName(llvm::BasicBlock &BB)
    {
        std::ostringstream aux;
        std::string bbName;
        aux << BB.getName().str();
        return aux.str();
    }

private:
    std::ofstream m_fileStream;
};