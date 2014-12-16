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
    fprintf(stderr, "File \"%s\", line %d , column %d\n", m_currParseFile.c_str(), m_currentLineNo, m_pLexer->getColumnNo());
    fprintf(stderr, "%s\n", m_currentLineStr.c_str());
    std::string error_loc = std::string(" ");
    
    for (int i = 0; i < m_pLexer->getColumnNo() - 3; ++i)
        error_loc += std::string(" ");
    
    fprintf(stderr, "%s^\n", error_loc.c_str());
    fprintf(stderr, "Syntax Error: %s\n", Str);
    fprintf(stderr, "-> Reported from @%s:%d\n", filename, lineNo);

    return nullptr;
}

FunctionDeclAST* Parser::errorP(const char *Str)
{
    return nullptr;
}

bool Parser::assertToken(const Token& token)
{
    bool correct_token = true;
    if (token != curr_token().tokenID)
    {
        SToken stoken;
        stoken.tokenID = token;
        stoken.lineNo = curr_token().lineNo;
        stoken.columnNo = curr_token().columnNo;
        std::string errMsg = "Expected token " + m_pLexer->getTokenStr(token) + std::string(" but got ") + m_pLexer->getTokenStr(m_pLexer->getCurrToken().tokenID);
        error(stoken, errMsg.c_str());
        correct_token = false;
    }

    return correct_token;
}

bool Parser::assertAndAdvance(const Token& token)
{
    bool correct_token = assertToken(token);

    advance();
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
    Token data_ty = curr_token().tokenID;
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
        break;
    default:
        assert(0 && "Unknown datatype");
        break;
    }

    advance();
    return pDatatype;
}

IdentifierAST* Parser::parseIdentifier()
{
    IdentifierAST *pIdentifier = nullptr;
    Token identifier_token = curr_token().tokenID;
    if (assertToken(tok_identifier))
    {
        pIdentifier = new IdentifierAST(m_pLexer->getIdentifierStr());
    }

    advance();
    return pIdentifier;
}

/// identifierexpr
///   ::= identifier
///   ::= identifier '(' expression* ')'
ExprAST* Parser::parseIdentifierExpr()
{
    //std::string IdName = m_pLexer->getIdentifierStr();

    //m_pLexer->getNextToken();  // eat identifier.

    //if (m_pLexer->getCurrToken().tokenID != '(') // Simple variable ref.
    //    return new VariableExprAST(IdName);

    //// Call.
    //m_pLexer->getNextToken();  // eat (
    //std::vector<ExprAST*> Args;
    //if (m_pLexer->getCurrToken().tokenID != ')')
    //{
    //    while (1)
    //    {
    //        ExprAST *Arg = parseExpression();
    //        if (!Arg) return 0;
    //        Args.push_back(Arg);

    //        if (m_pLexer->getCurrToken().tokenID == ')') break;

    //        if (m_pLexer->getCurrToken().tokenID != ',')
    //            return ERROR(m_pLexer->getCurrToken(), "Expected ')' or ',' in argument list");
    //        m_pLexer->getNextToken();
    //    }
    //}

    //// Eat the ')'.
    //m_pLexer->getNextToken();

    //return new CallExprAST(IdName, Args);
    return nullptr;
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

ExprAST* Parser::parseVariableDeclaration()
{
    DataTypeAST *pDatatype = parseDataType();
    IdentifierAST *pIdentifier = parseIdentifier();

    VariableExprAST *pVar = new VariableExprAST(pDatatype, pIdentifier);
    return pVar;
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
FunctionDeclAST* Parser::parseFunctionDeclaration()
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

    return new FunctionDeclAST(FnName, ArgNames);
}

/// args ::= datatype identifier, ... ')'
///      ::= ')'
ArgsAST* Parser::parseFuncArgs(Token open_token, Token close_token)
{
    ArgsAST *pArgAST = new ArgsAST();

    assertToken(tok_lparen);
    advance();

    if (curr_token().tokenID == close_token)
        return pArgAST;

    // Loop until we find the rparen
    while (curr_token().tokenID != close_token)
    {
        ExprAST *pVar = parseVariableDeclaration();
        pArgAST->addArg(pVar);
    }

    return pArgAST;
}

/// definition ::= datatype identifer '(' args ')' '{' block_statement '}'
FunctionAST* Parser::parseFuncDefinition(DataTypeAST *pRetTy, IdentifierAST *pFuncName)
{
    DataTypeAST *pDataType = parseDataType();

    assertAndAdvance(tok_lbrace);
    if (!pDataType)
        return nullptr;  // We did not match a return type. So its not a function definition

    m_pLexer->getNextToken();  // eat def.
    FunctionDeclAST *Proto = parseFunctionDeclaration();
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
        FunctionDeclAST *Proto = new FunctionDeclAST("", std::vector<std::string>());
        return new FunctionAST(Proto, E);
    }
    return 0;
}

/// external ::= 'extern' prototype
FunctionDeclAST* Parser::parseExtern()
{
    m_pLexer->getNextToken();  // eat extern.
    return parseFunctionDeclaration();
}

//===----------------------------------------------------------------------===//
// Top-Level parsing
//===----------------------------------------------------------------------===//

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
    switch (m_pLexer->getCurrToken().tokenID)
    {
    default:
    {
        DataTypeAST *pDataType = parseDataType();
        IdentifierAST *pIdentifier = parseIdentifier();

        switch (m_pLexer->getCurrToken().tokenID)
        {
        case tok_lparen:
        {
            // Parse the args
            ArgsAST *pArgs = parseFuncArgs(tok_lparen, tok_rparen);
            
            // Explicit fall through
        }
        case tok_semicolon:
        {

        }
        case tok_lbrace:
        {
            FunctionAST *pFunction = parseFuncDefinition(pDataType, pIdentifier);
            break;
        }
        default:
            ERROR(m_pLexer->getCurrToken(), "Error parsing. Expected a method definition or variable definition");
            break;
        } // End of switch (m_pLexer->getCurrToken().tokenID)
        
        break;
    } // End of default case
    } // End of outer switch (m_pLexer->getCurrToken().tokenID)
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
        advance();
        handleTopLevelExpression();
    }

    file_handle.close();
}