#include "frontend/genIL.h"

#include <sstream>
#include <string>

IcaValue* IcaConstant::genIL(GenIL*)
{
    return new IcaConstant(getValue());
}

IcaValue* IcaVariable::genIL(GenIL* g)
{
    return new IcaVariable(getSymbol());
}

IcaValue* IcaBinopExpression::genIL(GenIL* g)
{
    //of lval or rval is a binop, generate an assignment statement and insert
    IcaValue *lVal = NULL;
    if(dynamic_cast<IcaBinopExpression*>(&getLeftValue()))
    {
        // we need to check for function calls etc
        IcaVariable& nextVarRef = g->getNextVariable();
        IcaAssignment* tempAssign = new IcaAssignment(nextVarRef, *getLeftValue().genIL(g));
        g->getBuilder().insertStatement(*tempAssign);
        lVal = &nextVarRef;
    }
    else
        lVal = &getLeftValue();

    IcaValue *rVal = NULL;
    if(dynamic_cast<IcaBinopExpression*>(&getRightValue()))
    {
        // we need to check for function calls etc
        IcaVariable& nextVarRef = g->getNextVariable();
        IcaAssignment* tempAssign = new IcaAssignment(nextVarRef, *getRightValue().genIL(g));
        g->getBuilder().insertStatement(*tempAssign);
        rVal = &nextVarRef;
    }
    else
        rVal = &getRightValue();
    return new IcaBinopExpression(*lVal, *rVal, getOperation());
}

IcaValue* IcaFunctionCall::genIL(GenIL* g)
{
    std::vector<IcaValue*> newParams;
    std::vector<IcaValue*>& oldParams = getParamList();
    std::vector<IcaValue*>::iterator paramIter = oldParams.begin();
    for(; paramIter != oldParams.end(); ++paramIter)
        newParams.push_back((*paramIter)->genIL(g));
    return new IcaFunctionCall(getFunctionProtoType(), newParams);
}

IcaValue* IcaAssignment::genIL(GenIL* g)
{
    //LVal would be a variable, we just gen code
    IcaStatement* stmt = new IcaAssignment(getLVal(), *getRVal().genIL(g));
    g->getBuilder().insertStatement(*stmt);
    return stmt;
}

IcaValue* IcaReturnStatement::genIL(GenIL* g)
{
    IcaStatement* stmt = NULL;	
    if(getReturnValue() != NULL)
        stmt = new IcaReturnStatement(getReturnValue()->genIL(g));
    else 
        stmt = new IcaReturnStatement(NULL);
    g->getBuilder().insertStatement(*stmt);
    return stmt;
}

IcaValue* IcaWhileStatement::genIL(GenIL* g)
{
    IcaStatement* stmt = new IcaWhileStatement(*(IcaExpression*)getCondition().genIL(g)); //TODO:this might be a problem. we might not evaluate expression everytime in the loop.
    g->getBuilder().insertStatement(*stmt);
    std::vector<IcaStatement*>::const_iterator iter = getStatements().begin();
    for(; iter != getStatements().end(); ++iter)
        (*iter)->genIL(g);
    g->getBuilder().endCodeBlock();
    return stmt;
}

IcaValue* IcaExpressionStatement::genIL(GenIL* g)
{
    IcaStatement* stmt = new IcaExpressionStatement(*((IcaExpression*)getExpression().genIL(g)));
    g->getBuilder().insertStatement(*stmt);
    return stmt;
}

IcaValue* IcaBranchStatement::genIL(GenIL *g)
{
    ASTBuilder& builder = g->getBuilder();
    std::vector<IcaBranch*>::const_iterator branchIter = getBranches().begin();
    bool branchStatementAdded = false;
    IcaBranchStatement *branchStmt = NULL;
    for(; branchIter != getBranches().end(); ++branchIter)
    {
        IcaBranch* branch = *branchIter;
        IcaExpression& condition = *(IcaExpression*) branch->getCondition().genIL(g);
        if(!branchStatementAdded)
        {
            branchStmt = new IcaBranchStatement(condition);
            builder.insertStatement(*branchStmt);
            branchStatementAdded = true;
        }
        else
            builder.addBranch(condition);

        std::vector<IcaStatement*>::const_iterator stmtIter = branch->getStatements().begin();
        
        for(; stmtIter != branch->getStatements().end(); ++stmtIter)
            (*stmtIter)->genIL(g);
    }
    builder.endCodeBlock();
    return branchStmt;
}

IcaValue *IcaBreakStatement::genIL(GenIL* g)
{
    IcaStatement* stmt = new IcaBreakStatement();
    g->getBuilder().insertStatement(*stmt);
    return stmt;
}

IcaValue* IcaFunction::genIL(GenIL* g)
{
    IcaFunction& funcRef = *(new IcaFunction(getProtoType(), getArgSymbolList()));
    g->getBuilder().addFunction(funcRef);
    std::vector<IcaStatement*>::const_iterator iter = getStatements().begin();
    for(; iter != getStatements().end(); ++iter)
    {
        (*iter)->genIL(g);		
    }
    //we need to copy the symbols too;
    std::vector<IcaSymbol*>::iterator symIter = getSymbols().begin();
    for(; symIter != getSymbols().end(); ++symIter)
    {
        funcRef.addSymbol(*new IcaSymbol(**symIter));
    }

    return nullptr;
}

IcaValue* IcarusModule::genIL(GenIL* g)
{
    std::vector<IcaFunction*>& funcList = getFunctions();
    for (std::vector<IcaFunction*>::const_iterator funcIter = funcList.begin(); funcIter != funcList.end(); ++funcIter)
    {
        (*funcIter)->genIL(g);
    }
    //we need to copy the symbols too;
    return nullptr; //we wont use it anyway. This function should actually return nothing
}

IcarusModule* GenIL::generateIL()
{
    m_module.genIL(this);
    return &(m_astBuilder.getModule());
}

IcaVariable& GenIL::getNextVariable()
{
    std::ostringstream os;
    os << "_M_"<<m_tempSeed++;
    std::string str = os.str();
    IcaSymbol* s = new IcaSymbol(str, *new IcaType(IcaType::IntegerTy)); //Integer for temp
    m_astBuilder.addSymbol(*s);
    return *(new IcaVariable(*s));
}
