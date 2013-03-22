#include "frontend/codegen.h"
#include "frontend/PrintVisitor.h"
#include "Dot/dotwriter.h"
#include "frontend/genIL.h"
#include "frontend/genllvm.h"
#include "unistd.h"
#include "common/debug.h"
#include "midend/ConstantFolder.h"
#include "midend/DominanceTreeConstructor.h"
#include "midend/DominanceFrontier.h"

#include <fstream>
#include <cstdlib>
#include <iostream>
#include <cstdio>

#include <llvm/Module.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/PassManager.h>

extern IcarusModule* ParseFile(const char *filename); //using this for now. need to create a standard header file for lex

static Trace& gTrace = Trace::getInstance();
static Debug& gDebug = Debug::getInstance();

using namespace std;

//global variables
IcarusModule *module;

void genExecutable()
{
}

int Compile(char *fileName)
{
    gTrace <<"Compiling file: " << fileName;
    module = ParseFile(fileName);
    if(module == NULL)
        return -1; //there was some syntax error. Hence we skip all other checks
    GenIL *myILGen = new GenIL(*module);
    module = myILGen->generateIL();

    if(gDebug.isDotGen())
    {
        DotWriter d;
        std::string filename = "postgenIL.dot";
        d.writeDotFile(filename, *module);
    }
        
    GenLLVM genLLVM;
    genLLVM.generateLLVM(*module);
    llvm::Module& llvmModule = genLLVM.getModule();

    //Dispose old module
    if(module != NULL)
    {
        delete module;
        module = NULL;
    }


    llvmModule.dump();

    if(gDebug.isOptimizing())
    {
        llvm::PassManager passMgr;
        //Analysis Passes
        passMgr.add(new DominanceTreeConstructor());
        passMgr.add(new DominanceFrontier());

        //Optimization Passes
        passMgr.add(new ConstantFolder());
        passMgr.run(llvmModule);
    }
    
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
    genExecutable();
    
    return 0;
}

int main(int argc, char *argv[])
{
    gDebug.setDebug(true);
    gDebug.setTrace(true);
    gDebug.setYaccTrace(true);
    gDebug.setDotGen(true);
    gDebug.setCodeOptimization(true); //we need to allow setting levels

    gTrace<<"Verbose on!\n";
    Compile(argv[1]);	
    return 0;
}
