#ifndef __LEXER__
#define __LEXER__

#include <cctype>
#include <string>
#include <map>

#define X(tok_enum, tok_str, tok_id) tok_##tok_enum = -tok_id,
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

struct SToken
{
    Token tokenID;
    int lineNo;
    int columnNo;
};

class Lexer
{
public:
    Lexer(LEXER_MODE mode);
    ~Lexer() {}

    char getNextChar();
    char peekNextChar();

    SToken gettok();

    /// getNextToken reads another token from the
    /// lexer and updates CurTok with its results.
    SToken getNextToken();

    /// GetTokPrecedence - Get the precedence of the pending binary operator token.
    int getTokPrecedence();

    void setCurrentLine(std::string& line, int lineNo)
    {
        m_currentLineStr = line;
        m_currentLineNo = lineNo;
        m_currentColumnNo = 0;
    }

#define X(tok_enum, tok_str, tok_id)  { if (std::string(tok_str) == token) return tok_##tok_enum; }
    Token       isRecognizedToken(std::string token)
    {
        Token token_found = tok_undefined;
        #include "tokens.h"
        return token_found;
    }
#undef X

#define X(tok_enum, tok_str, tok_id)  if (tok_##tok_enum == token && std::string(#tok_str) != std::string("\"----\"")) return std::string(#tok_str);
    std::string getTokenStr(Token token)
    {
        std::string tok_str = m_identifierStr;
#include "tokens.h"
        return tok_str;
    }
#undef X

    bool isDataType(Token token);

    std::string getIdentifierStr() const { return m_identifierStr; }
    SToken      getCurrToken() const { return m_curTok; }
    double      getNumVal() const { return m_numVal; }

    int         getLineNo() const { return m_currentLineNo; }
    int         getColumnNo() const { return m_currentColumnNo; }

private:
    /// CurTok/getNextToken - Provide a simple token buffer.  CurTok is the current
    /// token the parser is looking at.  
    SToken                  m_curTok;
    std::string             m_currentLineStr;
    int                     m_currentLineNo;
    int                     m_currentColumnNo;


    std::string             m_identifierStr;
    char                    m_lastChar;
    char                    m_thisChar;
    double                  m_numVal;

    /// BinopPrecedence - This holds the precedence for each binary operator that is
    /// defined.
    std::map<char, int>     m_BinopPrecedence;

    LEXER_MODE              m_lexMode;
};


#endif