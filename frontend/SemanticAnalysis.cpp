#include "frontend/SemanticAnalysis.h"

#include "common/debug.h"

char SemanticAnalysis::ID;

// register the printCode class: 
//  - give it a command-line argument
//  - a name
//  - a flag saying that we don't modify the CFG
//  - a flag saying this is not an analysis pass
llvm::RegisterPass<SemanticAnalysis> T("SemanticAnalysis", "Semantic Analysis", false, true);

bool SemanticAnalysis::isMainFunctionPresent(llvm::Module &M)
{
    bool isMainPresent = false;

    for (llvm::Module::iterator fi = M.begin(), fe = M.end();
        fi != fe;
        ++fi)
    {
        if (fi->getName().str() == "main")
            isMainPresent = true;
    }

    return isMainPresent;
}

bool SemanticAnalysis::runOnModule(llvm::Module &M)
{
    ADD_HEADER("Semantic Analysis");

    // First check if there is a main function present in the module or not.
    if (!isMainFunctionPresent(M))
    {
        g_outputStream << "No entry point found in module: main not present\n";
        exit(0);
    }

    g_outputStream << "Everything checks out.... Perfect !!!\n";

    return false;
}
