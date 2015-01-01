#include "dotwriter.h"
#include <iostream>
#include <fstream>
#include <list>
#include <sstream>
using namespace std;

void DotWriter::writeDotFile(std::string& filename, IcarusModule& m)
{
    m_fileStream.open(filename.c_str(), ios::out);
    if(m_fileStream.bad())
    {
        cout<<"Error writing dot file"<<endl;
        return;
    }
    m_fileStream<<" digraph {"<<endl;
    m.accept(*this);
    m_fileStream<<" }";
    m_fileStream.close();
}

void DotWriter::Visit(IcaValue& )
{

}

void DotWriter::Visit(IcaExpression&)
{

}

void DotWriter::Visit(IcaVariable& v)
{
    m_fileStream<<v.getSymbol().getName()<<endl;
}

void DotWriter::Visit(IcaBinopExpression& b)
{
    //m_fileStream<<"Binary Expression: "<<endl;
    b.getLeftValue().accept(*this);
    m_fileStream<<b.getOperation()<<endl;
    b.getRightValue().accept(*this);
}

void DotWriter::Visit(IcaFunctionCall& f)
{
    m_fileStream<<"subgraph {"<<f.getFunctionProtoType().getName();
    std::vector<IcaValue*>::const_iterator iter = f.getParamList().begin();
    for(; iter != f.getParamList().end(); ++iter)
        (*iter)->accept(*this);
    m_fileStream<<"}"<<endl;
}

void DotWriter::Visit(IcaStatement&)
{
}

void DotWriter::Visit(IcaAssignment& a){
    //m_fileStream<<"Assignment: "<<endl;
    a.getLVal().accept(*this);
    a.getRVal().accept(*this);
}

void DotWriter::Visit(IcaReturnStatement& r)
{
    m_fileStream<<"Return: "<<endl;
    if(r.getReturnValue().size() != 0)
    {
        // r.getReturnValue()->accept(*this);
    }
}

void DotWriter::Visit(IcaFunctionProtoType&)
{

}

void DotWriter::Visit(IcaExpressionStatement& e)
{
    m_fileStream<<"ExpressionStatement: "<<endl;
    e.getExpression().accept(*this);
}

void DotWriter::Visit(IcaFunction& f)
{	
    std::vector<IcaStatement*> statementList = f.getStatements();
    std::vector<IcaStatement*>::const_iterator iter = statementList.begin();
    for(; iter != statementList.end(); ++iter)
    {
        (*iter)->accept(*this);
    }
}

void DotWriter::Visit(IcaSymbolTable&)
{

}

void DotWriter::Visit(IcaSymbol&)
{

}

void DotWriter::Visit(IcarusModule& m)
{
    std::string moduleID = getNextName();
    m_fileStream<<moduleID << "[label=\"Module: " << m.getName() << "\"]" << endl;
    std::vector<IcaFunction*>& funcList = m.getFunctions();
    for (std::vector<IcaFunction*>::const_iterator funcIter = funcList.begin(); funcIter != funcList.end(); ++funcIter)
    {
        std::string funcID = getNextName();
        m_fileStream << funcID << "[label=\"Function: " << (*funcIter)->getName() << "\"]" << endl;
        m_fileStream << moduleID << "->" << funcID << endl;
    }
}

std::string& DotWriter::getNextName()
{
    std::ostringstream os;
    os << "n"<<m_nameseed++;
    return *new std::string(os.str());
}
