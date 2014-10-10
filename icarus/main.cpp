#include <backend/codegenpublic.h>

#include <frontend/irtranslation.h>
#include <frontend/PrintVisitor.h>
#include <frontend/genIL.h>
#include <frontend/genllvm.h>
#include <frontend/findentryfunc.h>

#include "unistd.h"
#include <common/debug.h>
#include <midend/optimizeIR.h>
#include "Dot/dotwriter.h"

#include <fstream>
#include <cstdlib>
#include <iostream>
#include <cstdio>

#include <common/llvm_warnings_push.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/PassManager.h>
#include <common/llvm_warnings_pop.h>

extern IcarusModule* ParseFile(const char *filename); //using this for now. need to create a standard header file for lex

static Trace& gTrace = Trace::getInstance();
static Debug& gDebug = Debug::getInstance();

using namespace std;

//global variables
IcarusModule *pIcarusModule;

llvm::Function* findEntryFunc(llvm::Module& M)
{
    llvm::PassManager mpm;
    FindEntryFunc* pEntryFunc = new FindEntryFunc();
    mpm.add(pEntryFunc);
    mpm.run(M);
    return pEntryFunc->GetEntryFunc();
}

void genExecutable(CodeGenModule& module)
{
    GenerateCode(module);
}

int Compile(char *fileName)
{
    gTrace <<"Compiling file: " << fileName;
    pIcarusModule = ParseFile(fileName);
    
    if(pIcarusModule == nullptr)
        return -1; //there was some syntax error. Hence we skip all other checks

    GenIL *myILGen = new GenIL(*pIcarusModule);
    pIcarusModule = myILGen->generateIL();

    if(gDebug.isDotGen())
    {
        DotWriter d;
        std::string filename = "postgenIL.dot";
        d.writeDotFile(filename, *pIcarusModule);
    }

    GenLLVM genLLVM;
    genLLVM.generateLLVM(*pIcarusModule);
    llvm::Module& llvmModule = genLLVM.getModule();

    // Find the Entry Function
    llvm::Function* pEntryFunc = findEntryFunc(llvmModule);

    //Dispose old module
    if(pIcarusModule != nullptr)
    {
        delete pIcarusModule;
        pIcarusModule = nullptr;
    }

    std::cout << "-------------------------------------------------------------------" << std::endl;
    std::cout << "Before Optimization" << std::endl;
    std::cout << "-------------------------------------------------------------------" << std::endl;
    if (gDebug.isDebuggable())
        llvmModule.dump();

    if(gDebug.isOptimizing())
    {
        OptimizeIR(llvmModule);
    }
    
    std::cout << "-------------------------------------------------------------------" << std::endl;
    std::cout << "After Optimization" << std::endl;
    std::cout << "-------------------------------------------------------------------" << std::endl;
    if(gDebug.isDebuggable())
        llvmModule.dump();

    std::string moduleStr;
    llvm::raw_string_ostream string(moduleStr);
    fstream moduleDumpFile;
    moduleDumpFile.open("temp.ll", fstream::in | fstream::out | fstream::trunc);
    if(moduleDumpFile.is_open())
    {
        llvmModule.print(string, NULL);
        moduleDumpFile<<moduleStr;
        moduleDumpFile.close();
    }

    CodeGenModule codeGenModule;
    codeGenModule.setLLVMModule(&llvmModule);
    codeGenModule.setLLVMEntryFunction(pEntryFunc);

    genExecutable(codeGenModule);
    
    return 0;
}

int main(int argc, char *argv[])
{
    gDebug.setDebug(true);
    gDebug.setTrace(false);
    gDebug.setYaccTrace(false);
    gDebug.setDotGen(true);
    gDebug.setCodeOptimization(true); //we need to allow setting levels

    gTrace << "Verbose on!\n";

    char *pfilename = argv[1];

    assert(pfilename != nullptr && "No input file specified");

    Compile(pfilename);
    system("pause");
    return 0;
}
