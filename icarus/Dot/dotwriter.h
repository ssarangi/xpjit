#ifndef DOTWRITER_H
#define DOTWRITER_H
#include "frontend/codegen.h"
#include "frontend/IClassVisitor.h"
#include <fstream>
class DotWriter : IClassVisitor 
{
public:
    DotWriter(): m_nameseed(0){}
    virtual void Visit(IcaValue& );
    virtual void Visit(Expression&);
    virtual void Visit(Variable&);
    virtual void Visit(BinopExpression&);
    virtual void Visit(FunctionCall&);
    virtual void Visit(Statement&);
    virtual void Visit(ExpressionStatement&);	
    virtual void Visit(Assignment&);
    virtual void Visit(ReturnStatement&);
    virtual void Visit(FunctionProtoType&);
    virtual void Visit(Function&);
    virtual void Visit(SymbolTable&);	
    virtual void Visit(Symbol& );
    virtual void Visit(IcarusModule& );
    void writeDotFile(std::string& filename, IcarusModule& m);
private:
    ofstream m_fileStream;
    long m_nameseed;
    std::string& getNextName();
};
#endif
