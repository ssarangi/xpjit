#include "frontend/PrintVisitor.h"
#include "frontend/irtranslation.h"
#include <list>
using namespace std;

void PrintVisitor::Visit(IcaValue& )
{

}

void PrintVisitor::Visit(IcaExpression&)
{

}

void PrintVisitor::Visit(IcaVariable& v)
{
    cout<<"Variable: "<<v.getSymbol().getName()<<endl;
}

void PrintVisitor::Visit(IcaBinopExpression& b)
{
    cout<<"Binary Expression: "<<endl;
    b.getLeftValue().accept(*this);
    cout<<b.getOperation()<<endl;
    b.getRightValue().accept(*this);
}

void PrintVisitor::Visit(IcaFunctionCall& f)
{
    cout<<"Function Call: "<<f.getFunctionProtoType().getName()<<endl;
    std::vector<IcaValue*>::const_iterator iter = f.getParamList().begin();
    for(; iter != f.getParamList().end(); ++iter)
        (*iter)->accept(*this);
}

void PrintVisitor::Visit(IcaStatement&)
{
    cout<<"Statement :"<<endl;
}

void PrintVisitor::Visit(IcaAssignment& a)
{
    cout<<"Assignment: "<<endl;
    a.getLVal().accept(*this);
    a.getRVal().accept(*this);
}

void PrintVisitor::Visit(IcaReturnStatement& r)
{
    cout<<"Return: "<<endl;
    if(r.getReturnValue() != NULL)
        r.getReturnValue()->accept(*this);
}

void PrintVisitor::Visit(IcaFunctionProtoType&)
{

}

void PrintVisitor::Visit(IcaExpressionStatement& e)
{
    cout<<"ExpressionStatement: "<<endl;
    e.getExpression().accept(*this);
}

void PrintVisitor::Visit(IcaFunction& f)
{
    cout<<"Function: "<<f.getName()<<endl;
    std::vector<IcaStatement*> statementList = f.getStatements();
    std::vector<IcaStatement*>::const_iterator iter = statementList.begin();
    for(; iter != statementList.end(); ++iter)
    {
        (*iter)->accept(*this);
    }
}

void PrintVisitor::Visit(IcaSymbolTable&)
{

}

void PrintVisitor::Visit(IcaSymbol& )
{

}

void PrintVisitor::Visit(IcarusModule& m)
{
    cout<<"Module: "<<m.getName()<<endl;
    std::vector<IcaFunction*>& funcList = m.getFunctions();
    for (std::vector<IcaFunction*>::const_iterator funcIter = funcList.begin(); funcIter != funcList.end(); ++funcIter)
    {
        (*funcIter)->accept(*this);
    }
}