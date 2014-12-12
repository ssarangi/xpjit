#ifndef __PARSER__
#define __PARSER__

#include <string>
#include <vector>
#include "ast.h"

#define ERROR(TOK, STR) error(TOK, STR, __FILE__, __LINE__);

/// Error* - These are little helper functions for error handling.
class Parser
{
public:
    Parser(Lexer *pLexer)
        : m_pLexer(pLexer)
        , m_currParseFile("")
        , m_currentLineStr("")
        , m_currentLineNo(0)
    {}

    ~Parser() {}

    ExprAST *error(const SToken &token, const char *Str, const char* filename = __FILE__, int lineNo =__LINE__);
    PrototypeAST *errorP(const char *Str);

    DataTypeAST* parseDataType();
    IdentifierTyAST* parseIdentifier();
    ExprAST* parseExpression();
    ExprAST* parseIdentifierExpr();
    ExprAST* parseNumberExpr();
    ExprAST* parseParenExpr();
    ExprAST* parsePrimary();
    ExprAST* parseBinOpRHS(int ExprPrec, ExprAST *LHS);

    PrototypeAST *parseFunctionDeclaration();

    FunctionAST *parseDefinition();
    FunctionAST *parseTopLevelExpr();

    PrototypeAST *parseExtern();

    void handleDefinition();
    void handleExtern();
    void handleTopLevelExpression();
    void runStandalone();
    void parseFile(std::string filename);

private:
    void setLineStr(std::string line, int lineNo);
    bool assertAndAdvance(const Token& token);

private:
    std::string                     m_currParseFile;
    std::string                     m_currentLineStr;
    int                             m_currentLineNo;
    Lexer                          *m_pLexer;
};

#endif