#include "frontend/irtranslation.h"
#include <iostream>

//----------BranchStatement-------------------------

IcaBranchStatement::IcaBranchStatement(IcaExpression& condition)
{
    addBranch(condition);
}

IcErr IcaBranchStatement::addStatement(IcaStatement& s)
{
    return m_currentBranch->addStatement(s);
}

IcErr IcaBranchStatement::addBranch(IcaExpression& condition)
{
    m_currentBranch = new IcaBranch(condition);
    m_branches.push_back(m_currentBranch);
    return eNoErr;
}

bool IcaBranchStatement::endCodeBlock()
{
    return m_currentBranch->endCodeBlock();
}

IcaStatement* IcaBranchStatement::getCurrentStatement()
{
    IcaStatement* curStatement = m_currentBranch->getCurrentStatement();
    if(curStatement == NULL)
        return this;
}

//----------Branch----------------------------------
IcErr IcaBranch::addStatement(IcaStatement& s)
{
    if(m_currentInsertStatement == NULL)
    {
        m_statementList.push_back(&s);
        IcaControlFlowStatement* ctrlStmt = dynamic_cast<IcaControlFlowStatement*>(&s);
        if(ctrlStmt != NULL)
            m_currentInsertStatement = ctrlStmt;
    } 
    else
        m_currentInsertStatement->addStatement(s);	
    return eNoErr;
}

IcaStatement* IcaBranch::getCurrentStatement()
{
    if(m_currentInsertStatement == NULL)
        return NULL;
    return m_currentInsertStatement->getCurrentStatement();	
}

bool IcaBranch::endCodeBlock()
{
    if(m_currentInsertStatement == NULL)
        return false;
    if(!m_currentInsertStatement->endCodeBlock())
        m_currentInsertStatement = NULL;
    return true;
}

//---------------WhileStatement----------------
IcErr IcaWhileStatement::addStatement(IcaStatement& s)
{
    if(m_currentInsertStatement == NULL)
    { //we are generating statements in the function
        m_statementList.push_back(&s);
        IcaControlFlowStatement* ctrlStmt = dynamic_cast<IcaControlFlowStatement*>(&s);
        if(ctrlStmt != NULL)
            m_currentInsertStatement = ctrlStmt;
    } 
    else
        m_currentInsertStatement->addStatement(s);	
    return eNoErr;
}

//---------------ControlFlowStatement----------------
bool IcaControlFlowStatement::endCodeBlock()
{
    if(m_currentInsertStatement == NULL)
        return false;
    if(!m_currentInsertStatement->endCodeBlock())
        m_currentInsertStatement = NULL;
    return true; //either the inner code block ended or we ended our codeblock just now.
}

IcaStatement* IcaControlFlowStatement::getCurrentStatement()
{
    if(m_currentInsertStatement == NULL)
        return this;
    else
        return m_currentInsertStatement->getCurrentStatement();	
}

//--------------Function------------------
IcErr IcaFunction::addStatement(IcaStatement& s)
{
    if(m_currentInsertStatement == NULL)
    { 
        //we are generating statements in the function
        m_statementList.push_back(&s);
        IcaControlFlowStatement* ctrlStmt = dynamic_cast<IcaControlFlowStatement*>(&s);
        if(ctrlStmt != NULL)
            m_currentInsertStatement = ctrlStmt;
    } 
    else
        m_currentInsertStatement->addStatement(s);

    return eNoErr;
}


IcaStatement* IcaFunction::getCurrentStatement()
{
    if(m_currentInsertStatement == NULL)
        return NULL;
    return m_currentInsertStatement->getCurrentStatement();
}

bool IcaFunction::endCodeBlock()
{
    if(m_currentInsertStatement == NULL)
        return false;
    if(!m_currentInsertStatement->endCodeBlock())
        m_currentInsertStatement = NULL;
    return true; //either the inner code block ended or we ended our codeblock just now.
}

std::ostream& operator<<(std::ostream& stream, const IcaFunction& f)
{
    stream<<"Function: "<<f.getName()<<endl;
    //print the statements here
    return stream;
}

IcErr IcaFunction::addSymbol(IcaSymbol& sym)
{
    m_symbolTable.add(sym);
    return eNoErr;
}

IcaFunction::~IcaFunction()
{
    std::vector<IcaStatement*>::iterator stIter = m_statementList.begin();
    for(; stIter != m_statementList.end(); ++stIter)
        delete (*stIter);
    m_statementList.clear();
}

//--------------SymbolTable---------------

IcErr IcaSymbolTable::add(IcaSymbol& sym)
{
    //do error handling later
    m_symbols.push_back(&sym);
    return eNoErr;
}

//---------------Module----------------
IcarusModule::IcarusModule(const std::string& name): m_name(name), m_symbolTable(*new IcaSymbolTable())
{
}

IcErr IcarusModule::addFunction(IcaFunction& func)
{
    //do error handling later
    m_functionList.push_back(&func);
    return eNoErr;
}

IcErr IcarusModule::addSymbol(IcaSymbol& sym)
{
    //do error handling later
    return m_symbolTable.add(sym);
}

IcaFunction* IcarusModule::getProtoType(const std::string name, std::vector<IcaType*>& dataTypes)
{
    IcaFunction* fp = new IcaFunction(name, dataTypes, std::vector<IcaType*>(), std::vector<IcaSymbol*>()); //no need of return type for comparing prototypes
    std::vector<IcaFunction*>::iterator protoIter = m_functionList.begin();

    for(; protoIter != m_functionList.end(); ++protoIter)
    {
        if(*protoIter == fp)
            return *protoIter;
    }
    return NULL; //not found
}

IcaFunction* IcarusModule::getProtoType(const std::string name)
{
    std::vector<IcaFunction*>::const_iterator protoIter = m_functionList.begin();
    for(; protoIter != m_functionList.end(); ++protoIter)
    {
        if((*protoIter)->getName() == name)
            return *protoIter;
    }
    return NULL; //not found
}

IcErr IcarusModule::insertStatement(IcaFunction& f, IcaStatement& s)
{
    return f.addStatement(s);
}

std::ostream& operator<<(std::ostream& stream, const IcarusModule& m)
{
    stream<<"Module: "<<m.m_name<<endl;
    return stream;
}

IcarusModule::~IcarusModule()
{
    delete &m_symbolTable;
    std::vector<IcaFunction*>::iterator funcIter = m_functionList.begin();
    for(; funcIter != m_functionList.end(); ++funcIter)
    {
        delete (*funcIter);
    }
}

IcaFunction* IcarusModule::getFunction(const std::string name)
{
    std::vector<IcaFunction*>::const_iterator iter = m_functionList.begin();
    for(; iter != m_functionList.end(); ++iter)
    {
        if((*iter)->getName() == name)
            return *iter;
    }
    return NULL;
}

//------------------Code generator methods