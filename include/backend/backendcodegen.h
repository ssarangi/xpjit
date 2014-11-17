#ifndef __CODEGEN__
#define __CODEGEN__

#include <backend/iarchcodegen.h>
#include "tempstacksize.h"
#include "backendvar.h"

#include <common/llvm_warnings_push.h>
#include <llvm/Pass.h>
#include <llvm/IR/InstVisitor.h>
#include <llvm/IR/Dominators.h>
#include <llvm/Analysis/PostDominators.h>
#include <llvm/Analysis/MemoryDependenceAnalysis.h>
#include <common/llvm_warnings_pop.h>

void GenerateCode(CodeGenModule& M, std::string outputFile);

#endif