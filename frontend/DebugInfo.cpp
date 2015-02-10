#include "DebugInfo.h"

DebugInfo::DebugInfo(llvm::Module &M)
{
    m_pDIBuilder = new llvm::DIBuilder(M);
}