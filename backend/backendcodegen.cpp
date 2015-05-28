#include <backend/codegenpublic.h>
#include <common/debug.h>

#include <backend/backendcodegen.h>
#include "mips/mipscodegen.h"
#include "mips/mipspatternmatch.h"
#include "x86/LinearScan.h"
#include "x86/x86codegen.h"
#include "asmjit/asmjit_codegen.h"
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

    AsmJitCodeGen *pCodeGen = new AsmJitCodeGen();
    mpm.add(pCodeGen);
    mpm.run(*M.getLLVMModule());

    //// Open the file and write into it.
    //std::ofstream file;
    //file.open(outputFile);

    //// Add an end directive to end of asm
    //std::string asm_str = pCodeGen->getAssembly();

    //file.write(asm_str.c_str(), asm_str.length());
    //file.close();

    //file.open("tests\\ASMRunner\\test.asm");
    //file.write(asm_str.c_str(), asm_str.length());
    //file.close();

    //g_outputStream << "-------------------------------------------------------------------\n";
    //g_outputStream << "                         Code Generation                           \n";
    //g_outputStream << "-------------------------------------------------------------------\n";
    //g_outputStream << asm_str;
}