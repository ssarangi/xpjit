#include "frontend/genllvm.h"
#include "frontend/irtranslation.h"

#include <common/llvm_warnings_push.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Verifier.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/IR/Constants.h>
#include <llvm/ADT/APInt.h>
#include <llvm/ADT/StringRef.h>
#include <common/llvm_warnings_pop.h>

#include <common/debug.h>

using llvm::BasicBlock;
inline llvm::LLVMContext& getGlobalContext()
{
    //just a double dispatch
    return llvm::getGlobalContext();
}

llvm::Value* IcaConstant::genLLVM(GenLLVM* g)
{
    return g->getBuilder().getInt32(getValue());
}

llvm::Value* IcaVariable::genLLVM(GenLLVM* g)
{
    return g->getBuilder().CreateLoad(g->getNamedValues()[getSymbol().getName()],"");
}

llvm::Value* IcaBinopExpression::genLLVM(GenLLVM* g)
{
    llvm::IRBuilder<>& builder = g->getBuilder();

    llvm::Value* leftValue = getLeftValue().genLLVM(g);
    llvm::Value* rightValue = getRightValue().genLLVM(g);

    switch(getOperation())
    {
    case Add:
        return builder.CreateAdd(leftValue, rightValue, "");
        break;
    case Sub:
        return builder.CreateSub(leftValue, rightValue, "");
        break;
    case Mul:
        return builder.CreateMul(leftValue, rightValue, "");
        break;
    case Div:
        return builder.CreateSDiv(leftValue, rightValue, "");
        break;
    case EQ:
        return builder.CreateICmpEQ(leftValue, rightValue, "");
        break;
    case NE:
        return builder.CreateICmpNE(leftValue, rightValue, "");
        break;
    case LT:
        return builder.CreateICmpULT(leftValue, rightValue, "");
        break;
    case LTEQ:
        return builder.CreateICmpULE(leftValue, rightValue, "");
        break;
    case GT:
        return builder.CreateICmpUGT(leftValue, rightValue, "");
        break;
    case GTEQ:
        return builder.CreateICmpUGE(leftValue, rightValue, "");
        break;
    }
}

//Helper function to get a llvm function type from our function
llvm::FunctionType& getFunctionType(IcaFunction* fp, GenLLVM *g)
{
    std::vector<llvm::Type*> args;
    std::vector<IcaType*>::iterator argTypeIter = fp->getTypeList().begin();
    for(; argTypeIter != fp->getTypeList().end(); ++argTypeIter)
    {
        args.push_back(g->getLLVMType(**argTypeIter));
    }

    std::string ret_type_name = fp->getName() + "_ret_type";
    llvm::FunctionType *FT = llvm::FunctionType::get(g->getLLVMType(fp->getReturnType(), ret_type_name), *new llvm::ArrayRef<llvm::Type*>(args), false);
    return *FT;
}

llvm::Value* IcaFunctionCall::genLLVM(GenLLVM* g)
{
    std::vector<llvm::Value*> paramArrayRef;
    std::vector<IcaValue*> paramList = getParamList();
    std::vector<IcaValue*>::iterator paramIter = paramList.begin();
    for(; paramIter != paramList.end(); ++paramIter)
    {
        paramArrayRef.push_back((*paramIter)->genLLVM(g));
    }

    llvm::FunctionType *FT = &getFunctionType(&getFunctionProtoType(), g);
    llvm::Function *F = static_cast<llvm::Function*>(g->getModule().getOrInsertFunction(getFunctionProtoType().getName(), FT));

    llvm::Value *pCall = g->getBuilder().CreateCall(F, *new llvm::ArrayRef<llvm::Value*>(paramArrayRef), "");

    llvm::Value *pRetVal = nullptr;

    if (F->getReturnType()->isAggregateType())
    {
        llvm::Value *pAlloc = g->getBuilder().CreateAlloca(F->getReturnType());
        g->getBuilder().CreateStore(pCall, pAlloc);
        pRetVal = pAlloc;
    }
    else
        pRetVal = pCall;
    
    return pRetVal;
}

llvm::Value* IcaMultiVarAssignment::genLLVM(GenLLVM* g)
{
    // Verify that all the types of the function returns match with function prototype
    auto retTypesList = m_pFuncCall->getFunctionProtoType().getReturnType();

    if (retTypesList.size() != m_multiVarList.size())
    {
        g_outputStream << "Error: Call doesn't match return line numbers\n";
    }

    unsigned int i = 0;

    auto symbols = m_pFuncCall->getFunctionProtoType().getSymbols();

    llvm::Value *pRetVal = m_pFuncCall->genLLVM(g);

    if (m_multiVarList.size() == 0)
    {
        // Do nothing for now
    }
    else if (m_multiVarList.size() == 1)
    {
        g->getBuilder().CreateStore(pRetVal, g->getNamedValues()[m_multiVarList[0]], false);
    }
    else
    {
        for (unsigned int i = 0; i < m_multiVarList.size(); ++i)
        {
            if (m_multiVarList[i] != "")
            {
                llvm::Value *pLoad = g->getBuilder().CreateLoad(g->getBuilder().CreateStructGEP(pRetVal, i));
                g->getBuilder().CreateStore(pLoad, g->getNamedValues()[m_multiVarList[i]], false);
            }
        }
    }

    return nullptr;
}

void IcaPrintStatement::declare_puts(GenLLVM *g)
{
    std::vector<llvm::Type *> putsArgs;
    putsArgs.push_back(g->getBuilder().getInt8Ty()->getPointerTo());
    llvm::ArrayRef<llvm::Type*>  argsRef(putsArgs);

    llvm::FunctionType *putsType =
        llvm::FunctionType::get(g->getBuilder().getInt32Ty(), argsRef, false);
    m_pPutsFunc = g->getModule().getOrInsertFunction("puts", putsType);
}


llvm::Value* IcaPrintStatement::genLLVM(GenLLVM* g)
{
    if (!m_pPutsFunc)
        declare_puts(g);

    if (m_stmtType == PRINT_STRING)
    {
        // Create a global string.
        llvm::Value *pString = g->getBuilder().CreateGlobalStringPtr(m_data.s);
        g->getBuilder().CreateCall(m_pPutsFunc, pString);
    }

    return nullptr;
}

llvm::Value* IcaAssignment::genLLVM(GenLLVM* g)
{
    return g->getBuilder().CreateStore(getRVal().genLLVM(g),g->getNamedValues()[getLVal().getSymbol().getName()], false);
}

llvm::Value* IcaReturnStatement::genLLVM(GenLLVM* g)
{
    if (m_values.size() == 0)
        return g->getBuilder().CreateRetVoid();
    else if (m_values.size() == 1)
        return g->getBuilder().CreateRet(m_values[0]->genLLVM(g));
    else
    {
        llvm::Value *pAlloca = g->getBuilder().CreateAlloca(g->getCurrentFunc()->getReturnType());
        llvm::Type *pFuncRetType = g->getCurrentFunc()->getReturnType();
        
        llvm::Type *pCurrentFuncRetType = g->getCurrentFunc()->getReturnType();

        assert(pCurrentFuncRetType->isAggregateType());
        assert(m_values.size() == pCurrentFuncRetType->getStructNumElements());

        for (unsigned i = 0; i < pCurrentFuncRetType->getStructNumElements(); ++i)
        {
            llvm::Value *pExpr = m_values[i]->genLLVM(g);
            llvm::Value *pElement = g->getBuilder().CreateStructGEP(pAlloca, i);
            g->getBuilder().CreateStore(pExpr, pElement);
        }

        return g->getBuilder().CreateRet(g->getBuilder().CreateLoad(pAlloca));
    }
    
    return nullptr;
}

llvm::Value* IcaExpressionStatement::genLLVM(GenLLVM* g)
{
    return getExpression().genLLVM(g);
}

llvm::Value* IcaWhileStatement::genLLVM(GenLLVM *g)
{
    llvm::IRBuilder<>& builder = g->getBuilder();

    BasicBlock* curBlock = builder.GetInsertBlock();
    llvm::Function *func = curBlock->getParent();
    BasicBlock *whileBB = BasicBlock::Create(getGlobalContext(), "whilecond", func);
    BasicBlock *whileBodyBB = BasicBlock::Create(getGlobalContext(), "whileblock", func);
    BasicBlock *postWhileBB = BasicBlock::Create(getGlobalContext(), "postwhile", func);
    builder.CreateBr(whileBB);
    builder.SetInsertPoint(whileBB);

    llvm::Value* condition = getCondition().genLLVM(g);	
    llvm::Value* zeroConst = llvm::ConstantInt::get(getGlobalContext(), llvm::APInt(32 /*bits*/, 0 /*value*/, false /*isSigned*/));
    llvm::Type* conditionType = condition->getType();	
    if(!conditionType->isIntegerTy(1)) //bool type
        condition = builder.CreateICmpNE(condition, zeroConst, "");
    builder.CreateCondBr(condition, whileBodyBB, postWhileBB);

    builder.SetInsertPoint(whileBodyBB);

    std::vector<IcaStatement*>::iterator sIter = getStatements().begin();
    for(; sIter != getStatements().end(); ++sIter)
        (*sIter)->genLLVM(g);

    builder.CreateBr(whileBB); //loop back to while condition
    builder.SetInsertPoint(postWhileBB);

    return nullptr;
}

llvm::Value* IcaBreakStatement::genLLVM(GenLLVM* g)
{
    return nullptr;
}

//Generation should be 
llvm::Value* IcaBranchStatement::genLLVM(GenLLVM* g)
{
    llvm::IRBuilder<>& builder = g->getBuilder();

    BasicBlock* curBlock = builder.GetInsertBlock();
    llvm::Function *func = curBlock->getParent();

    llvm::Value* zeroConst = llvm::ConstantInt::get(getGlobalContext(), llvm::APInt(32 /*bits*/, 0 /*value*/, false /*isSigned*/));

    //Create basic blocks for each
    unsigned int size = getBranches().size();
    std::vector<BasicBlock*> basicBlocks(size*2+1);
    for(unsigned int i = 0 ; i < size*2; i+=2)
    {
        basicBlocks[i] = BasicBlock::Create(getGlobalContext(), "condblock", func);
        basicBlocks[i+1] = BasicBlock::Create(getGlobalContext(), "codeblock", func);
    }
    
    BasicBlock *postIfElseBB = BasicBlock::Create(getGlobalContext(),"postif", func);
    basicBlocks[size*2] = postIfElseBB;

    builder.CreateBr(basicBlocks[0]); //jump to first if condition

    std::vector<IcaBranch*>::const_iterator branchIter = getBranches().begin();
    unsigned int i = 0;
    for(; branchIter != getBranches().end(); ++branchIter, i+=2)
    {
        IcaBranch* branch = *branchIter;

        builder.SetInsertPoint(basicBlocks[i]);
        llvm::Value* condition = branch->getCondition().genLLVM(g);
        llvm::Type* conditionType = condition->getType();
        if(!conditionType->isIntegerTy(1)) //bool type
            condition = builder.CreateICmpNE(condition, zeroConst, "");		
        builder.CreateCondBr(condition, basicBlocks[i+1], basicBlocks[i+2]);
        builder.SetInsertPoint(basicBlocks[i+1]);

        std::vector<IcaStatement*> statements = branch->getStatements();
        std::vector<IcaStatement*>::const_iterator stmtIter = statements.begin();
        for(; stmtIter != statements.end(); ++stmtIter)
            (*stmtIter)->genLLVM(g);

        if (curBlock->getTerminator() == nullptr)
            builder.CreateBr(postIfElseBB); //Jump to end of if-else after completing this codeblock
    }

    builder.SetInsertPoint(postIfElseBB);

    return nullptr;
}

llvm::Value* IcaFunction::genLLVM(GenLLVM* g)
{
    llvm::FunctionType *FT = &getFunctionType(this, g);
    llvm::Function *F = static_cast<llvm::Function*>(g->getModule().getOrInsertFunction(getName(), FT));

    m_pLLVMFunc = F;

    g->setCurrentFunc(F);

    BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "entry", F);
    g->getBuilder().SetInsertPoint(BB);

    //Set names for the arguments and allocate them
    //assert that number of symbols equal number of elements in type list
    llvm::Function::arg_iterator argIter = F->arg_begin();
    for (std::vector<IcaSymbol*>::iterator symIter = getArgSymbolList().begin();
        argIter != F->arg_end(); ++argIter, ++symIter)
    {
        argIter->setName((*symIter)->getName());

        //allocate a pointer to this variable
        llvm::AllocaInst* allocInst = g->getBuilder().CreateAlloca(g->getLLVMType((*symIter)->getType()), 0, "");
        g->getBuilder().CreateStore(argIter, allocInst);
        g->getNamedValues()[argIter->getName()] = allocInst;
    }

    //assume only the local symbols are visited now
    std::vector<IcaSymbol*>::iterator symIter = getSymbols().begin();
    for(; symIter != getSymbols().end(); ++symIter)
    {
        g->getNamedValues()[(*symIter)->getName()] = g->getBuilder().CreateAlloca(g->getLLVMType((*symIter)->getType()), 0, (*symIter)->getName());
    }

    std::vector<IcaStatement*>::iterator sIter = getStatements().begin();
    for(; sIter != getStatements().end(); ++sIter)
        (*sIter)->genLLVM(g);

    return nullptr;
}

llvm::Value* IcarusModule::genLLVM(GenLLVM* g)
{
    std::vector<IcaFunction*>& funcList = getFunctions();
    for (std::vector<IcaFunction*>::const_iterator funcIter = funcList.begin(); funcIter != funcList.end(); ++funcIter)
    {
        (*funcIter)->genLLVM(g);
    }

    return nullptr; //we wont use it anyway. This function should actually return nothing
}

void GenLLVM::generateLLVM(IcarusModule &m)
{
    m.genLLVM(this);
    llvm::verifyModule(m_module);
}

GenLLVM::GenLLVM() 
    : m_module(*new llvm::Module("IcarusModule", getGlobalContext())), m_irBuilder(*new llvm::IRBuilder<>(getGlobalContext())) 
{
}

//Helpers

llvm::Type* GenLLVM::getLLVMType(IcaType& type)
{
    switch(type.getTypeID())
    {
    case IcaType::IntegerTy:
        return llvm::Type::getInt32Ty(getGlobalContext());
    case IcaType::FloatTy:
        return llvm::Type::getFloatTy(getGlobalContext());
    case IcaType::VoidTy:
        return llvm::Type::getVoidTy(getGlobalContext());

    }
}

llvm::Type* GenLLVM::getLLVMType(std::vector<IcaType*>& typeList, std::string name)
{
    llvm::Type *pRetType = nullptr;

    if (m_types.find(name) != m_types.end())
        return m_types[name];

    if (typeList.size() == 0)
        pRetType = llvm::Type::getVoidTy(m_module.getContext());
    else if (typeList.size() == 1)
        pRetType = getLLVMType(*typeList[0]);
    else
    {
        std::vector<llvm::Type*> typeArrRef;
        for (IcaType* t : typeList)
            typeArrRef.push_back(getLLVMType(*t));

        // Create a structure for this type
        pRetType = llvm::StructType::create(m_module.getContext(), typeArrRef, name);
    }

    m_types[name] = pRetType;

    return pRetType;
}
