#ifndef __DEBUG_INFO__
#define __DEBUG_INFO__

#include <llvm/IR/DIBuilder.h>
#include <llvm/IR/Module.h>

class DebugInfo
{
public:
    DebugInfo(llvm::Module &M);
    ~DebugInfo();

    llvm::DIType getDoubleTy();
    llvm::DIType getFloatTy();
    llvm::DIType getIntTy();

private:
    llvm::DICompileUnit   m_compileUnit;
    llvm::DIType          m_dblType;
    llvm::DIBuilder      *m_pDIBuilder;
};

#endif