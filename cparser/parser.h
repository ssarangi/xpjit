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
    FunctionDeclAST *errorP(const char *Str);

    DataTypeAST* parseDataType();
    IdentifierAST* parseIdentifier();
    ExprAST* parseExpression();
    ExprAST* parseIdentifierExpr();
    ExprAST* parseNumberExpr();
    ExprAST* parseParenExpr();
    ExprAST* parsePrimary();
    ExprAST* parseBinOpRHS(int ExprPrec, ExprAST *LHS);

    ExprAST* parseVariableDeclaration();

    FunctionDeclAST *parseFunctionDeclaration();

    ArgsAST     *parseFuncArgs(Token open_token, Token close_token);
    FunctionAST *parseFuncDefinition(DataTypeAST *pRetTy, IdentifierAST *pFuncName);
    FunctionAST *parseTopLevelExpr();

    FunctionDeclAST *parseExtern();

    void handleExtern();
    void handleTopLevelExpression();
    void runStandalone();
    void parseFile(std::string filename);

    void advance() { m_pLexer->getNextToken(); }
    SToken curr_token() { return m_pLexer->getCurrToken(); }

private:
    void setLineStr(std::string line, int lineNo);
    bool assertToken(const Token& token);
    bool assertAndAdvance(const Token& token);

private:
    std::string                     m_currParseFile;
    std::string                     m_currentLineStr;
    int                             m_currentLineNo;
    Lexer                          *m_pLexer;
};

#endif