#include "lexer.h"
#include "parser.h"

#include <fstream>

/// Error* - These are little helper functions for error handling.
ExprAST* Parser::Error(const SToken &token, const char *Str, const char* filename, int lineNo)
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

PrototypeAST* Parser::ErrorP(const char *Str)
{
    return nullptr;
}

void Parser::setLineStr(std::string line, int lineNo)
{
    m_currentLineStr = line;
    m_currentLineNo = lineNo;

    m_pLexer->setCurrentLine(line, lineNo);
}

DataTypeAST* Parser::ParseDataType(SToken& token)
{
    DataTypeAST *pDatatype = nullptr;

    switch (token.tokenID)
    {
    case tok_char:
    case tok_string:
    case tok_void:
    case tok_int:
    case tok_float:
    case tok_double:
        pDatatype = new DataTypeAST(token.tokenID);
    }

    return pDatatype;
}

/// identifierexpr
///   ::= identifier
///   ::= identifier '(' expression* ')'
ExprAST* Parser::ParseIdentifierExpr()
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
            ExprAST *Arg = ParseExpression();
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
ExprAST* Parser::ParseNumberExpr()
{
    ExprAST *Result = new NumberExprAST(m_pLexer->getNumVal());
    m_pLexer->getNextToken(); // consume the number
    return Result;
}

/// parenexpr ::= '(' expression ')'
ExprAST* Parser::ParseParenExpr()
{
    m_pLexer->getNextToken();  // eat (.
    ExprAST *V = ParseExpression();
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
ExprAST* Parser::ParsePrimary()
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
ExprAST* Parser::ParseBinOpRHS(int ExprPrec, ExprAST *LHS)
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
        ExprAST *RHS = ParsePrimary();
        if (!RHS) return 0;

        // If BinOp binds less tightly with RHS than the operator after RHS, let
        // the pending operator take RHS as its LHS.
        int NextPrec = m_pLexer->getTokPrecedence();
        if (TokPrec < NextPrec) {
            RHS = ParseBinOpRHS(TokPrec + 1, RHS);
            if (RHS == 0) return 0;
        }

        // Merge LHS/RHS.
        LHS = new BinaryExprAST(BinOp, LHS, RHS);
    }
}

/// expression
///   ::= primary binoprhs
///
ExprAST* Parser::ParseExpression()
{
    ExprAST *LHS = ParsePrimary();
    if (!LHS) return 0;

    return ParseBinOpRHS(0, LHS);
}

/// prototype
///   ::= id '(' id* ')'
PrototypeAST* Parser::ParseFunctionDeclaration()
{
    if (m_pLexer->getCurrToken().tokenID != tok_identifier)
        return ErrorP("Expected function name in prototype");

    std::string FnName = m_pLexer->getIdentifierStr();
    m_pLexer->getNextToken();

    if (m_pLexer->getCurrToken().tokenID != '(')
        return ErrorP("Expected '(' in prototype");

    std::vector<std::string> ArgNames;
    while (m_pLexer->getNextToken().tokenID == tok_identifier)
    {
        ArgNames.push_back(m_pLexer->getIdentifierStr());

        char next_token = m_pLexer->getNextToken().tokenID;

        if (next_token == ')')
            break;

        if (next_token != COMMA)
            return ErrorP("Expected ',' in function declaration");
    }

    if (m_pLexer->getCurrToken().tokenID != ')')
        return ErrorP("Expected ')' in prototype");

    // success.
    m_pLexer->getNextToken();  // eat ')'.

    return new PrototypeAST(FnName, ArgNames);
}

/// definition ::= datatype identifer '(' args ')' '{' block_statement '}'
FunctionAST* Parser::ParseDefinition()
{
    Token func_ret_ty = m_pLexer->getCurrToken().tokenID;
    if (!m_pLexer->isDataType(func_ret_ty))
        ERROR(m_pLexer->getCurrToken(), "Expected a datatype");

    if (!m_pLexer->isDataType(func_ret_ty))
        return nullptr;  // We did not match a return type. So its not a function definition

    m_pLexer->getNextToken();  // eat def.
    PrototypeAST *Proto = ParseFunctionDeclaration();
    if (Proto == 0) return 0;

    if (ExprAST *E = ParseExpression())
        return new FunctionAST(Proto, E);
    return 0;
}

/// toplevelexpr ::= expression
FunctionAST* Parser::ParseTopLevelExpr()
{
    if (ExprAST *E = ParseExpression())
    {
        // Make an anonymous proto.
        PrototypeAST *Proto = new PrototypeAST("", std::vector<std::string>());
        return new FunctionAST(Proto, E);
    }
    return 0;
}

/// external ::= 'extern' prototype
PrototypeAST* Parser::ParseExtern()
{
    m_pLexer->getNextToken();  // eat extern.
    return ParseFunctionDeclaration();
}

//===----------------------------------------------------------------------===//
// Top-Level parsing
//===----------------------------------------------------------------------===//

void Parser::HandleDefinition()
{
    if (ParseDefinition())
    {
        fprintf(stderr, "Parsed a function definition.\n");
    }
    else
    {
        // Skip token for error recovery.
        m_pLexer->getNextToken();
    }
}

void Parser::HandleExtern()
{
    if (ParseExtern())
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
void Parser::HandleTopLevelExpression()
{
    SToken datatype = m_pLexer->getNextToken();
    DataTypeAST *pDataType = ParseDataType(datatype);

    // Evaluate a top-level expression into an anonymous function.
    if (FunctionAST *pFuncAST = ParseDefinition())
    {
        // Do something here.
    }
    else if (ParseTopLevelExpr())
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
void Parser::RunStandalone()
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

void Parser::ParseFile(std::string filename)
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
        HandleTopLevelExpression();
    }

    file_handle.close();
}