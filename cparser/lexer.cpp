#include "lexer.h"

#include <assert.h>

Lexer::Lexer(LEXER_MODE mode)
: m_lexMode(mode)
, m_currentLineStr("")
, m_currentLineNo(0)
{
    // Install standard binary operators.
    // 1 is lowest precedence.
    m_BinopPrecedence['<'] = 10;
    m_BinopPrecedence['+'] = 20;
    m_BinopPrecedence['-'] = 20;
    m_BinopPrecedence['*'] = 40;  // highest.
}

char Lexer::getNextChar()
{
    if (m_lexMode == STANDALONE_MODE)
        return getchar();
    else
        return m_currentLineStr[m_currentColumnNo++];
}

char Lexer::peekNextChar()
{
    assert(m_lexMode != STANDALONE_MODE);
    return m_currentLineStr[m_currentColumnNo + 1];
}

bool Lexer::isDataType(Token token)
{
    if (token >= 0)
        return false;

    switch (token)
    {
    case tok_void:
    case tok_int:
    case tok_float:
    case tok_double:
    case tok_string:
    case tok_char:
        return true;
    default:
        return false;
    }

    return false;
}

SToken Lexer::gettok()
{
    static char LastChar = ' ';
    SToken stoken;
    stoken.lineNo = m_currentLineNo;
    stoken.columnNo = m_currentColumnNo;

    // skip any whitespace
    while (isspace(LastChar))
        LastChar = getNextChar();

    // Identifier: [a-zA-Z][a-zA-Z0-9]*
    if (isalpha(LastChar))
    {
        m_identifierStr = LastChar;
        while (isalnum((LastChar = getNextChar())))
            m_identifierStr += LastChar;

        Token token = isRecognizedToken(m_identifierStr);
        if (token == tok_undefined)
            stoken.tokenID = tok_identifier;
        else
            stoken.tokenID = token;

        return stoken;
    }

    // Number: [0-9.]+
    if (isdigit(LastChar) || LastChar == '.')
    {
        std::string NumStr;
        do
        {
            NumStr += LastChar;
            LastChar = getNextChar();
        } while (isdigit(LastChar) || LastChar == '.');

        m_numVal = strtod(NumStr.c_str(), 0);
        stoken.tokenID = tok_number;
        return stoken;
    }

    if (LastChar == '#')
    {
        // Comment until the end of line
        do LastChar = getNextChar();
        while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

        if (LastChar != EOF)
            return gettok();
    }

    // Check for the end of file. Don't eat the EOF
    if (LastChar == EOF)
    {
        stoken.tokenID = tok_eof;
        return stoken;
    }

    // Otherwise, just return the character as its ascii value
    char m_thisChar = LastChar;
    m_lastChar = getNextChar();
    stoken.tokenID = tok_ascii;
    return stoken;
}

SToken Lexer::getNextToken()
{
    return m_curTok = gettok();
}

/// GetTokPrecedence - Get the precedence of the pending binary operator token.
int Lexer::getTokPrecedence()
{
    if (!isascii(m_curTok.tokenID))
        return -1;

    // Make sure it's a declared binop.
    int TokPrec = m_BinopPrecedence[m_curTok.tokenID];
    if (TokPrec <= 0) return -1;
    return TokPrec;
}

