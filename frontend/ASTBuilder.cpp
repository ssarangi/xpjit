#include "frontend/ASTBuilder.h"

ASTBuilder::ASTBuilder()
    : m_module(*new IcarusModule("globalModule"))
    , m_curFunction(NULL)
    , m_retListStructureCount(0)
{
}

IcErr ASTBuilder::addSymbol(IcaSymbol& s)
{
    if(m_curFunction != NULL) //we are in global space
        return m_curFunction->addSymbol(s);
    return m_module.addSymbol(s);
}

IcErr ASTBuilder::addFunction(const std::string& name)
{
    IcaFunction *pF = new IcaFunction(name, m_dataTypeList, m_retTypeList, m_argNameList);
    IcErr err = m_module.addFunction(*pF);

    if (!err)
        m_curFunction = pF;

    return err;
}

IcErr ASTBuilder::addFunction(IcaFunction& f)
{
    IcErr err = m_module.addFunction(f);
    if(!err)
        m_curFunction = &f;
    return err;
}

IcErr ASTBuilder::insertStatement(IcaStatement& s)
{
    return m_curFunction->addStatement(s);
}

IcaFunction* ASTBuilder::getProtoType(const std::string name, std::vector<IcaType*> dataTypes)
{
    return m_module.getProtoType(name, dataTypes);
}

IcaSymbol* ASTBuilder::getSymbol(std::string name)
{
    if(m_curFunction != NULL) 
    {
        std::vector<IcaSymbol*>& funcSymbolList = m_curFunction->getSymbols();
        std::vector<IcaSymbol*>::const_iterator symIter = funcSymbolList.begin();

        for(; symIter!= funcSymbolList.end() ; ++symIter )
        {
            if((*symIter)->getName() == name)
                return *symIter;
        }
    }

    //search in the global symbol table
    std::vector<IcaSymbol*>& symbolList = m_module.getSymbols();
    std::vector<IcaSymbol*>::const_iterator iter = symbolList.begin();
    for(; iter!= symbolList.end() ; ++iter )
    {
        if((*iter)->getName() == name)
            return *iter;
    }
    return NULL;
}

IcaFunction* ASTBuilder::getFunction(const std::string name)
{
    return m_module.getFunction(name);
}

IcaFunction* ASTBuilder::getFunctionProtoType(const std::string& name)
{
    return m_module.getProtoType(name);
}

IcaSymbol* ASTBuilder::addSymbol(char *s, IcaType& type)
{ 
    // this should have more info like datatype, scope etc
    IcaSymbol *ourSymbol = new IcaSymbol(*new std::string(s), type);
    IcErr err = addSymbol(*ourSymbol);
    //  if(err)
    //      yyerror(errMsg[err]);
    return ourSymbol;
}

IcErr ASTBuilder::addBranch(IcaExpression& e)
{
    IcaStatement* stmt = getCurrentStatement();
    if(stmt != NULL)
    {
        IcaBranchStatement *branchStmt = dynamic_cast<IcaBranchStatement*>(stmt);
        if(branchStmt == NULL)
            throw;
        branchStmt->addBranch(e);
    }
}

IcaStatement* ASTBuilder::getCurrentStatement()
{
    return m_curFunction->getCurrentStatement();
}

bool ASTBuilder::endCodeBlock()
{
    if(!m_curFunction->endCodeBlock())
        m_curFunction = NULL;

    return true;
}
