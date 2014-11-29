#include <backend/codegenpublic.h>
#include <common/debug.h>

#include <backend/backendcodegen.h>
#include "mips/mipscodegen.h"
#include "mips/mipspatternmatch.h"
#include "x86/LinearScan.h"
#include "x86/x86codegen.h"
#include "regalloc.h"

#include <common/llvm_warnings_push.h>
#include <llvm/PassManager.h>
#include <llvm/ADT/PostOrderIterator.h>
#include <llvm/IR/InstIterator.h>
#include <common/llvm_warnings_pop.h>

#include <iostream>
#include <fstream>

void GenerateCode(CodeGenModule& M, std::string outputFile)
{
    llvm::PassManager mpm;
    // MipsCodeGen *pMipsCodeGen = new MipsCodeGen();
    // CodeGenPass *pCodeGenPass = new CodeGenPass(pMipsCodeGen);
    // mpm.add(new RegAlloc());
    // mpm.add(pMipsCodeGen);
    // mpm.add(pCodeGenPass);

    X86CodeGen *pX86CodeGen = new X86CodeGen();
    mpm.add(pX86CodeGen);
    mpm.run(*M.getLLVMModule());

    // Open the file and write into it.
    std::ofstream file;
    file.open(outputFile);

    file.write(pX86CodeGen->getAssembly().c_str(), pX86CodeGen->getAssembly().length());
    file.close();

    g_outputStream << "-------------------------------------------------------------------\n";
    g_outputStream << "                         Code Generation                           \n";
    g_outputStream << "-------------------------------------------------------------------\n";
    g_outputStream << pX86CodeGen->getAssembly();
}