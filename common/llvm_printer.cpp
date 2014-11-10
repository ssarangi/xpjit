#include <common/llvm_printer.h>

char PrintCFG::ID = 0;
llvm::RegisterPass<PrintCFG> W("print_cfg", "print CFG.");

int PrintCFG::PrintPackedStr(std::string str, int packSize, char c) {
    int strSize = str.size();
    int incSize = strSize < packSize ? packSize - strSize : 0;
    std::string strAux(incSize, c);
    m_fileStream << str << strAux;
    return (strSize > packSize) ? strSize : packSize;
}


int PrintCFG::PrintPackedStr(const char * str, int packSize, char c) {
    int strSize = strlen(str);
    int incSize = strSize < packSize ? packSize - strSize : 0;
    std::string strAux(incSize, c);
    m_fileStream << str << strAux;
    return (strSize > packSize) ? strSize : packSize;
}


void PrintCFG::visitBasicBlock(llvm::BasicBlock &BB) {
    std::string bbName = "\"" + getBBName(BB) + "\"";

    // Prints all the successors:
    for (llvm::succ_iterator succ = llvm::succ_begin(&BB), e = llvm::succ_end(&BB);
        succ != e;
        ++succ)
    {
        llvm::BasicBlock* bb = *succ;
        m_fileStream << bbName << " -> " << "\"" << bb->getName().str() << "\"" << "\n";
    }
}


bool PrintCFG::runOnFunction(llvm::Function &F)
{
    // prints header information: graph name, node shape, etc
    m_fileStream << "digraph foo {\n\n";
    m_fileStream << "node [\n";
    m_fileStream << "  fontname = \"Courier\"\n";
    m_fileStream << "  fontsize = \"10\"\n";
    m_fileStream << "  shape = \"box\"\n";
    m_fileStream << "]\n\n";

    // prints the label (title) of the graph:
    m_fileStream << "label=\"CFG for function " << F.getName().str() << "\"\n\n";

    // prints each basic block:
    for (llvm::Function::iterator BB = F.begin(), BBe = F.end();
        BB != BBe; ++BB)
        visitBasicBlock(*BB);

    m_fileStream << "}\n";
    return false;
}