#include "lexer.h"
#include "parser.h"

//===----------------------------------------------------------------------===//
// Main driver code.
//===----------------------------------------------------------------------===//

int main(int argc, char **argv)
{
    if (argc > 1)
    {
        std::string filename = argv[1];
        Lexer *pLexer = new Lexer(FILE_MODE);
        Parser *pParser = new Parser(pLexer);
        pParser->parseFile(filename);
    }
    else
    {
        Lexer *pLexer = new Lexer(STANDALONE_MODE);
        
        Parser *pParser = new Parser(pLexer);
        pLexer->getNextToken();

        // Run the main "interpreter loop" now.
        pParser->runStandalone();
    }

    return 0;
}