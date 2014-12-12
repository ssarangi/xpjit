#define TOKENS \
    X(eof,          "----",                        1)     \
    X(extern,       "extern",                      3)     \
    X(identifier,   "----",                        4)     \
    X(number,       "----",                        5)     \
    X(int,          "int",                         6)     \
    X(float,        "float",                       7)     \
    X(double,       "double",                      8)     \
    X(void,         "void",                        9)     \
    X(string,       "string",                     10)    \
    X(char,         "char",                       11)    \
    X(assign,       "=",                          12)    \
    X(lbrace,       "{",                          13)    \
    X(rbrace,       "}",                          14)    \
    X(lparen,       "(",                          15)    \
    X(rparen,       ")",                          16)    \
    X(ascii,        "----",                       65534)    \
    X(undefined,    "----",                       65535)

TOKENS