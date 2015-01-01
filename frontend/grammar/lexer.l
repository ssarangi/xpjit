%option nounistd
%option noyywrap
%option stack
%option yylineno
%option never-interactive
%option outfile="lexer.cpp"

%{
/* External Symbols */
class IcaValue;
class IcaStatement;

#include "parser.hpp"
#include <stdio.h>
#include <string>

#define YY_NEVER_INTERACTIVE 1 
// #define isatty _isatty

/* handle locations */
int yycolumn = 1;

int column = 1;
int lineNo = 1;

void count()
{
    int i;

    for (i = 0; yytext[i] != '\0'; i++)
        if (yytext[i] == '\n')
            column = 0;
        else if (yytext[i] == '\t')
            column += 8 - (column % 8);
        else
            column++;
}


int check_type()
{
/*
* pseudo code --- this is what it should check
*
*   if (yytext == type_name)
*       return(TYPE_NAME);
*
*   return(IDENTIFIER);
*/

/*
*   it actually will only return IDENTIFIER
*/

    return(IDENTIFIER);
}

%}

%% 

"[" { count(); return LSQUARE; }

"]" { count(); return RSQUARE; }

"function" { count(); return FUNCTION; }

"int" { count(); return INTEGER; }

"float" { count(); return FLOAT; }

"void" { count(); return VOID; }

"return" { count(); return RETURN; }

"if" { count(); return IF; }

"else" { count(); return ELSE; }

"while" { count(); return WHILE; }

"for" { count(); return FOR; }

"break" { count(); return BREAK; }

"printf" { count(); return PRINTF; }

"do" { count(); return DO; }

"//".*\n ;

"==" { count(); return EQUALS; }

"!=" { count(); return NEQUALS; }

"<" { count(); return LESSTHAN; }

"<=" { count(); return LESSTHANEQ; }

">" { count(); return MORETHAN; }

">=" { count(); return MORETHANEQ; }

L?\"(\\.|[^\\"])*\" { count(); return(STRING_LITERAL); }

[-+*/;=,)(\{\}] { count(); return *yytext; }

[0-9]+ { yylval.integer = atoi(yytext);  count(); return NUMBER; }

[a-zA-Z][a-zA-Z0-9]* { 
    yylval.string = (char*)calloc(yyleng+1, sizeof(char));
        strncpy(yylval.string, yytext, yyleng); // Remember you need to free this.
    return IDENTIFIER; 
    }

[\n\r]   { lineNo++; column = 0; }

%%

