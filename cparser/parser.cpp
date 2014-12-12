#include "lexer.h"
#include "parser.h"

#include <fstream>
#include <assert.h>

/// Error* - These are little helper functions for error handling.
ExprAST* Parser::error(const SToken &token, const char *Str, const char* filename, int lineNo)
{
    /*
    File "syntax_error.py", line 7
    print x y z
            ^
    SyntaxError: Expected new line after statement
    -> Reported @pypa/parser/parser.cc:944 in bool pypa::simple_stmt(pypa::{anonymous}::State&, pypa::AstStmt&)
    */
    fprintf(stderr, "File \"%s\", line %d\n", m_currParseFile.c_str(), m_currentLineNo);
    fprintf(stderr, "%s\n", m_currentLineStr.c_str());
    std::string error_loc = std::string(" ");
    
    for (int i = 0; i < token.tokenID; ++i)
        error_loc += std::string(" ");
    
    fprintf(stderr, "%s^\n", error_loc.c_str());
    fprintf(stderr, "Syntax Error: %s\n", Str);
    fprintf(stderr, "-> Reported from @%s:%d\n", filename, lineNo);

    return nullptr;
}

PrototypeAST* Parser::errorP(const char *Str)
{
    return nullptr;
}

bool Parser::assertAndAdvance(const Token& token)
{
    bool correct_token = true;
    if (token != m_pLexer->getCurrToken().tokenID)
    {
        SToken stoken;
        stoken.tokenID = token;
        stoken.lineNo = m_pLexer->getCurrToken().lineNo;
        stoken.columnNo = m_pLexer->getCurrToken().columnNo;
        std::string errMsg = "Expected token " + m_pLexer->getTokenStr(token) + std::string(" but got ") + m_pLexer->getTokenStr(m_pLexer->getCurrToken().tokenID);
        error(stoken, errMsg.c_str());
        correct_token = false;
    }

    m_pLexer->getNextToken();
    return correct_token;
}

void Parser::setLineStr(std::string line, int lineNo)
{
    m_currentLineStr = line;
    m_currentLineNo = lineNo;

    m_pLexer->setCurrentLine(line, lineNo);
}

DataTypeAST* Parser::parseDataType()
{
    Token data_ty = m_pLexer->getNextToken().tokenID;
    if (!m_pLexer->isDataType(data_ty))
        ERROR(m_pLexer->getCurrToken(), "Expected a datatype");

    DataTypeAST *pDatatype = nullptr;

    switch (data_ty)
    {
    case tok_char:
    case tok_string:
    case tok_void:
    case tok_int:
    case tok_float:
    case tok_double:
        pDatatype = new DataTypeAST(data_ty);
    default:
        assert(0 && "Unknown datatype");
    }

    return pDatatype;
}

IdentifierTyAST* Parser::parseIdentifier()
{
    IdentifierTyAST *pIdentifier = nullptr;
    Token identifier_token = m_pLexer->getNextToken().tokenID;
    if (assertAndAdvance(tok_identifier))
    {
        pIdentifier = new IdentifierTyAST(m_pLexer->getIdentifierStr());
    }

    return pIdentifier;
}

/// identifierexpr
///   ::= identifier
///   ::= identifier '(' expression* ')'
ExprAST* Parser::parseIdentifierExpr()
{
    std::string IdName = m_pLexer->getIdentifierStr();

    m_pLexer->getNextToken();  // eat identifier.

    if (m_pLexer->getCurrToken().tokenID != '(') // Simple variable ref.
        return new VariableExprAST(IdName);

    // Call.
    m_pLexer->getNextToken();  // eat (
    std::vector<ExprAST*> Args;
    if (m_pLexer->getCurrToken().tokenID != ')')
    {
        while (1)
        {
            ExprAST *Arg = parseExpression();
            if (!Arg) return 0;
            Args.push_back(Arg);

            if (m_pLexer->getCurrToken().tokenID == ')') break;

            if (m_pLexer->getCurrToken().tokenID != ',')
                return ERROR(m_pLexer->getCurrToken(), "Expected ')' or ',' in argument list");
            m_pLexer->getNextToken();
        }
    }

    // Eat the ')'.
    m_pLexer->getNextToken();

    return new CallExprAST(IdName, Args);
}

/// numberexpr ::= number
ExprAST* Parser::parseNumberExpr()
{
    ExprAST *Result = new NumberExprAST(m_pLexer->getNumVal());
    m_pLexer->getNextToken(); // consume the number
    return Result;
}

/// parenexpr ::= '(' expression ')'
ExprAST* Parser::parseParenExpr()
{
    m_pLexer->getNextToken();  // eat (.
    ExprAST *V = parseExpression();
    if (!V) return 0;

    if (m_pLexer->getCurrToken().tokenID != ')')
        return ERROR(m_pLexer->getCurrToken(), "expected ')'");
    m_pLexer->getNextToken();  // eat ).
    return V;
}

/// primary
///   ::= identifierexpr
///   ::= numberexpr
///   ::= parenexpr
ExprAST* Parser::parsePrimary()
{
    //switch (m_pLexer->getCurrToken())
    //{
    //default: return Error("unknown token when expecting an expression");
    //case tok_identifier: return ParseIdentifierExpr();
    //case tok_number:     return ParseNumberExpr();
    //case '(':            return ParseParenExpr();
    //}
    return nullptr;
}

/// binoprhs
///   ::= ('+' primary)*
ExprAST* Parser::parseBinOpRHS(int ExprPrec, ExprAST *LHS)
{
    // If this is a binop, find its precedence.
    while (1) {
        int TokPrec = m_pLexer->getTokPrecedence();

        // If this is a binop that binds at least as tightly as the current binop,
        // consume it, otherwise we are done.
        if (TokPrec < ExprPrec)
            return LHS;

        // Okay, we know this is a binop.
        int BinOp = m_pLexer->getCurrToken().tokenID;
        m_pLexer->getNextToken();  // eat binop

        // Parse the primary expression after the binary operator.
        ExprAST *RHS = parsePrimary();
        if (!RHS) return 0;

        // If BinOp binds less tightly with RHS than the operator after RHS, let
        // the pending operator take RHS as its LHS.
        int NextPrec = m_pLexer->getTokPrecedence();
        if (TokPrec < NextPrec) {
            RHS = parseBinOpRHS(TokPrec + 1, RHS);
            if (RHS == 0) return 0;
        }

        // Merge LHS/RHS.
        LHS = new BinaryExprAST(BinOp, LHS, RHS);
    }
}

/// expression
///   ::= primary binoprhs
///
ExprAST* Parser::parseExpression()
{
    ExprAST *LHS = parsePrimary();
    if (!LHS) return 0;

    return parseBinOpRHS(0, LHS);
}

/// prototype
///   ::= id '(' id* ')'
PrototypeAST* Parser::parseFunctionDeclaration()
{
    if (m_pLexer->getCurrToken().tokenID != tok_identifier)
        return errorP("Expected function name in prototype");

    std::string FnName = m_pLexer->getIdentifierStr();
    m_pLexer->getNextToken();

    if (m_pLexer->getCurrToken().tokenID != '(')
        return errorP("Expected '(' in prototype");

    std::vector<std::string> ArgNames;
    while (m_pLexer->getNextToken().tokenID == tok_identifier)
    {
        ArgNames.push_back(m_pLexer->getIdentifierStr());

        char next_token = m_pLexer->getNextToken().tokenID;

        if (next_token == ')')
            break;

        if (next_token != COMMA)
            return errorP("Expected ',' in function declaration");
    }

    if (m_pLexer->getCurrToken().tokenID != ')')
        return errorP("Expected ')' in prototype");

    // success.
    m_pLexer->getNextToken();  // eat ')'.

    return new PrototypeAST(FnName, ArgNames);
}

/// definition ::= datatype identifer '(' args ')' '{' block_statement '}'
FunctionAST* Parser::parseDefinition()
{
    DataTypeAST *pDataType = parseDataType();

    assertAndAdvance(tok_lbrace);
    if (!pDataType)
        return nullptr;  // We did not match a return type. So its not a function definition

    m_pLexer->getNextToken();  // eat def.
    PrototypeAST *Proto = parseFunctionDeclaration();
    if (Proto == 0) return 0;

    if (ExprAST *E = parseExpression())
        return new FunctionAST(Proto, E);
    return 0;
}

/// toplevelexpr ::= expression
FunctionAST* Parser::parseTopLevelExpr()
{
    if (ExprAST *E = parseExpression())
    {
        // Make an anonymous proto.
        PrototypeAST *Proto = new PrototypeAST("", std::vector<std::string>());
        return new FunctionAST(Proto, E);
    }
    return 0;
}

/// external ::= 'extern' prototype
PrototypeAST* Parser::parseExtern()
{
    m_pLexer->getNextToken();  // eat extern.
    return parseFunctionDeclaration();
}

//===----------------------------------------------------------------------===//
// Top-Level parsing
//===----------------------------------------------------------------------===//

void Parser::handleDefinition()
{
    if (parseDefinition())
    {
        fprintf(stderr, "Parsed a function definition.\n");
    }
    else
    {
        // Skip token for error recovery.
        m_pLexer->getNextToken();
    }
}

void Parser::handleExtern()
{
    if (parseExtern())
    {
        fprintf(stderr, "Parsed an extern\n");
    }
    else
    {
        // Skip token for error recovery.
        m_pLexer->getNextToken();
    }
}


/// definition ::= datatype identifier '(' args ')' '{' block_statement '}'
///            ::= datatype identifier = expression ';'
void Parser::handleTopLevelExpression()
{
    DataTypeAST *pDataType = parseDataType();

    // Evaluate a top-level expression into an anonymous function.
    if (FunctionAST *pFuncAST = parseDefinition())
    {
        // Do something here.
    }
    else if (parseTopLevelExpr())
    {
        fprintf(stderr, "Parsed a top-level expr\n");
    }
    else
    {
        // Skip token for error recovery.
        m_pLexer->getNextToken();
    }
}

/// top ::= definition | external | expression | ';'
void Parser::runStandalone()
{
    while (1)
    {
        //fprintf(stderr, "ready> ");
        //switch (m_pLexer->getCurrToken())
        //{
        //case tok_eof:    return;
        //case ';':        m_pLexer->getNextToken(); break;  // ignore top-level semicolons.
        //case tok_extern: HandleExtern(); break;
        //default:         HandleTopLevelExpression(); break;
        //}
    }
}

void Parser::parseFile(std::string filename)
{
    std::ifstream file_handle;
    file_handle.open(filename);

    m_currParseFile = filename;

    while (!file_handle.eof())
    {
        std::string current_line = "";
        std::getline(file_handle, current_line);
        ++m_currentLineNo;
        setLineStr(current_line, m_currentLineNo);

        //switch (m_pLexer->getCurrToken())
        //{
        //case tok_eof:    return;
        //case ';':        m_pLexer->getNextToken(); break;  // ignore top-level semicolons.
        //case tok_def:    HandleDefinition(); break;
        //case tok_extern: HandleExtern(); break;
        //default:         HandleTopLevelExpression(); break;
        //}
        handleTopLevelExpression();
    }

    file_handle.close();
}