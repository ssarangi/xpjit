#include "DebugInfo.h"

DebugInfo::DebugInfo(llvm::Module &M)
{
    m_pDIBuilder = new llvm::DIBuilder(M);
    m_compileUnit = m_pDIBuilder->createCompileUnit(llvm::dwarf::DW_LANG_C, "fib.ks", ".", "Icarus Compiler", 0, "", 0);
}

void DebugInfo::finalize()
{
    m_pDIBuilder->finalize();
}