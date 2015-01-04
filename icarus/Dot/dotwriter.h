#ifndef DOTWRITER_H
#define DOTWRITER_H
#include "frontend/irtranslation.h"
#include "frontend/IClassVisitor.h"
#include <fstream>
class DotWriter : IClassVisitor 
{
public:
    DotWriter(): m_nameseed(0){}
    virtual void Visit(IcaValue& );
    virtual void Visit(IcaExpression&);
    virtual void Visit(IcaVariable&);
    virtual void Visit(IcaBinopExpression&);
    virtual void Visit(IcaFunctionCall&);
    virtual void Visit(IcaStatement&);
    virtual void Visit(IcaExpressionStatement&);
    virtual void Visit(IcaMultiVarAssignment&);
    virtual void Visit(IcaAssignment&);
    virtual void Visit(IcaReturnStatement&);
    virtual void Visit(IcaFunction&);
    virtual void Visit(IcaSymbolTable&);
    virtual void Visit(IcaSymbol&);
    virtual void Visit(IcarusModule& );
    void writeDotFile(std::string& filename, IcarusModule& m);
private:
    ofstream m_fileStream;
    long m_nameseed;
    std::string& getNextName();
};
#endif
