#ifndef ASTBUILDER_H
#define ASTBUILDER_H

#include <frontend/irtranslation.h>
#include <frontend/IcaType.h>

#include <unordered_map>

class ASTBuilder
{ 
    //This will be a builder class only and won't be necessary for further optimizations in the code
public:
    ASTBuilder();

    IcErr addSymbol(IcaSymbol& s);
    IcaSymbol* addSymbol(char *name, IcaType& type);
    IcaSymbol* getSymbol(std::string name); //temporary. Need more than a name, like scope etc.
    
    IcErr addFunction(const std::string& name);
    IcErr addFunction(IcaFunction& f);
    
    IcErr insertStatement(IcaStatement& s);
    IcaStatement* getCurrentStatement();
    IcErr addBranch(IcaExpression& s);
    
    IcaFunction* getProtoType(const std::string name, std::vector<IcaType*> dataTypes);
    IcaFunction* getProtoType(const std::string name) { return getProtoType(name, m_dataTypeList); }
    
    IcaFunction* getFunction(const std::string name);
    IcaFunction* getFunctionProtoType(const std::string& name);
    bool endCodeBlock();

    IcarusModule& getModule() { return m_module; }

    //some helpers
    void pushDataType(IcaType* type) { m_dataTypeList.push_back(type); }
    void pushRetType(IcaType* type) { m_retTypeList.push_back(type); }
    void pushArgName(IcaSymbol *arg) { m_argNameList.push_back(arg); }
    void incrementRetListCount() { ++m_retListStructureCount; }

    void pushError(std::string error) { m_errorList.push_back(error); }
    bool hasErrors() { return m_errorList.size() != 0; }

    void reset() { m_retTypeList.clear(); m_dataTypeList.clear(); m_argNameList.clear(); }

    IcaFunction *getCurrentFunction() { return m_curFunction; }
private:
    IcaFunction* m_curFunction;//can be null
    IcarusModule& m_module;

    int m_retListStructureCount;

    //some helpers during building of AST
    std::vector<IcaType*> m_dataTypeList; //to store the data types of the arguments while constructing the argList
    std::vector<IcaType*> m_retTypeList;
    std::vector<IcaSymbol*> m_argNameList;
    std::vector<std::string> m_errorList;
};

#endif //ASTBUILDER_H
