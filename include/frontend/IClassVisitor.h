#ifndef ICLASSVISITOR_H
#define ICLASSVISITOR_H

//class forward declarations
class IcaValue;
class IcaExpression;
class IcaVariable;
class IcaBinopExpression;
class IcaFunctionCall;
class IcaStatement;
class IcaAssignment;
class IcaReturnStatement;
class IcaFunctionProtoType;
class IcaFunction;
class IcaSymbolTable;
class IcaSymbol;
class IcarusModule;
class IcaExpressionStatement;

class IClassVisitor
{
public:
    virtual void Visit(IcaValue& ) = 0;
    virtual void Visit(IcaExpression&) = 0;
    virtual void Visit(IcaVariable&) = 0;
    virtual void Visit(IcaBinopExpression&) = 0;
    virtual void Visit(IcaFunctionCall&) = 0;
    virtual void Visit(IcaStatement&) = 0;
    virtual void Visit(IcaAssignment&) = 0;
    virtual void Visit(IcaExpressionStatement&) = 0;
    virtual void Visit(IcaReturnStatement&) = 0;
    virtual void Visit(IcaFunctionProtoType&) = 0;
    virtual void Visit(IcaFunction&) = 0;
    virtual void Visit(IcaSymbolTable&) = 0;	
    virtual void Visit(IcaSymbol& ) = 0;
    virtual void Visit(IcarusModule& ) = 0;
};
//don't forget to add the const visitor
#endif //ICLASSVISITOR_H
