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

char Lexer::gettok()
{
    static char LastChar = ' ';

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
            return tok_identifier;
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
        return tok_number;
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
        return tok_eof;

    // Otherwise, just return the character as its ascii value
    char ThisChar = LastChar;
    LastChar = getNextChar();
    return ThisChar;
}

char Lexer::getNextToken()
{
    return m_curTok = gettok();
}

/// GetTokPrecedence - Get the precedence of the pending binary operator token.
int Lexer::getTokPrecedence()
{
    if (!isascii(m_curTok))
        return -1;

    // Make sure it's a declared binop.
    int TokPrec = m_BinopPrecedence[m_curTok];
    if (TokPrec <= 0) return -1;
    return TokPrec;
}

