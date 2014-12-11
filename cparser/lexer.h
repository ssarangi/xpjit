#ifndef __LEXER__
#define __LEXER__

#include <cctype>
#include <string>
#include <map>

//enum Token
//{
//    tok_eof = -1,
//
//    // Commands
//    tok_def = -2,
//    tok_extern = -3,
//
//    // primary
//    tok_identifier = -4,
//    tok_number = -5,
//
//    // data types
//    tok_int = -6,
//    tok_float = -7,
//    tok_double = -8,
//    tok_void = -9,
//    tok_string = -10,
//    tok_char = -11,
//
//    tok_undefined = -65535,
//};

#define X(a, b, c) tok_##a = -b,
enum Token
{ 
#include "tokens.h"
};
#undef X

#define COMMA ','

enum LEXER_MODE
{
    FILE_MODE,
    STANDALONE_MODE
};

class Lexer
{
public:
    Lexer(LEXER_MODE mode);
    ~Lexer() {}

    char getNextChar();
    char peekNextChar();

    char gettok();

    /// getNextToken reads another token from the
    /// lexer and updates CurTok with its results.
    char getNextToken();

    /// GetTokPrecedence - Get the precedence of the pending binary operator token.
    int getTokPrecedence();

    void setCurrentLine(std::string& line, int lineNo)
    {
        m_currentLineStr = line;
        m_currentLineNo = lineNo;
        m_currentColumnNo = 0;
    }

#define X(a, b, c)  if (std::string(#a) == token) return tok_##a;
    Token       isRecognizedToken(std::string token)
    {
        Token token_found = tok_undefined;
        #include "tokens.h"
        return token_found;
    }
#undef X

    std::string getIdentifierStr() const { return m_identifierStr; }
    char        getCurrToken() const { return m_curTok; }
    double      getNumVal() const { return m_numVal; }

private:
    /// CurTok/getNextToken - Provide a simple token buffer.  CurTok is the current
    /// token the parser is looking at.  
    char                    m_curTok;
    std::string             m_currentLineStr;
    int                     m_currentLineNo;
    int                     m_currentColumnNo;


    std::string             m_identifierStr;
    double                  m_numVal;

    /// BinopPrecedence - This holds the precedence for each binary operator that is
    /// defined.
    std::map<char, int>     m_BinopPrecedence;

    LEXER_MODE              m_lexMode;
};


#endif