#ifndef PRINTVISITOR_H
#define PRINTVISITOR_H
#include <frontend/IClassVisitor.h>
#include <iostream>

class PrintVisitor : public IClassVisitor
{
public:
    virtual void Visit(IcaValue& );
    virtual void Visit(IcaExpression&);
    virtual void Visit(IcaVariable&);
    virtual void Visit(IcaBinopExpression&);
    virtual void Visit(IcaFunctionCall&);
    virtual void Visit(IcaStatement&);
    virtual void Visit(IcaExpressionStatement&);	
    virtual void Visit(IcaAssignment&);
    virtual void Visit(IcaReturnStatement&);
    virtual void Visit(IcaFunctionProtoType&);
    virtual void Visit(IcaFunction&);
    virtual void Visit(IcaSymbolTable&);	
    virtual void Visit(IcaSymbol& );
    virtual void Visit(IcarusModule& );

    //	PrintVisitor(const std::ostream& pout) : m_pout(pout){}
    //	PrintVisitor(): m_pout(std::cout){}
private:
    //	we need a stream to print to
    //	const std::ostream& m_pout;	
};

#endif //PRINTVISITOR_H
