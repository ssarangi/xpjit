#include "lexer.h"
#include "parser.h"

//===----------------------------------------------------------------------===//
// Main driver code.
//===----------------------------------------------------------------------===//

int main(int argc, char **argv)
{
    // Prime the first token.
    fprintf(stderr, "ready> ");
    
    if (argc > 1)
    {
        std::string filename = argv[1];
        Lexer *pLexer = new Lexer(FILE_MODE);
        Parser *pParser = new Parser(pLexer);
        pParser->ParseFile(filename);
    }
    else
    {
        Lexer *pLexer = new Lexer(STANDALONE_MODE);
        
        Parser *pParser = new Parser(pLexer);
        pLexer->getNextToken();

        // Run the main "interpreter loop" now.
        pParser->RunStandalone();
    }

    return 0;
}