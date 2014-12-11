#ifndef __PARSER__
#define __PARSER__

#include <string>
#include <vector>
#include "ast.h"


/// Error* - These are little helper functions for error handling.
class Parser
{
public:
    Parser(Lexer *pLexer)
        : m_pLexer(pLexer)
        , m_currentLineStr("")
        , m_currentLineNo(0)
    {}

    ~Parser() {}

    ExprAST *Error(const char *Str);
    PrototypeAST *ErrorP(const char *Str);

    ExprAST *ParseExpression();
    ExprAST *ParseIdentifierExpr();
    ExprAST *ParseNumberExpr();
    ExprAST *ParseParenExpr();
    ExprAST *ParsePrimary();
    ExprAST *ParseBinOpRHS(int ExprPrec, ExprAST *LHS);

    PrototypeAST *ParseFunctionDeclaration();

    FunctionAST *ParseDefinition();
    FunctionAST *ParseTopLevelExpr();

    PrototypeAST *ParseExtern();

    void HandleDefinition();
    void HandleExtern();
    void HandleTopLevelExpression();
    void RunStandalone();
    void ParseFile(std::string filename);

private:
    void setLineStr(std::string line, int lineNo);

private:
    std::string                     m_currentLineStr;
    int                             m_currentLineNo;
    Lexer                          *m_pLexer;
};

#endif