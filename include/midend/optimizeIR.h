#ifndef __OPTIMIZE_IR__
#define __OPTIMIZE_IR__

#include <common/llvm_warnings_push.h>
#include <llvm/IR/Module.h>
#include <common/llvm_warnings_pop.h>

void OptimizeIR(llvm::Module&);

#endif