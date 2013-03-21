%option noyywrap
%option stack
%option outfile="lexer.cpp"
%{
class IcaValue;
class Statement;

#include "parser.h"
#include <stdio.h>
#include <string>

#define YY_NEVER_INTERACTIVE 1 
#define isatty _isatty

%}

%% 

"int" { return INTEGER; }

"float" { return FLOAT; }

"void" { return VOID; }

"return" { return RETURN; }

"if" { return IF; }

"else" { return ELSE; }

"while" { return WHILE; }

"for" { return FOR; }

"break" { return BREAK; }

"//".*\n ;

"==" { return EQUALS; }

"!=" { return NEQUALS; }

"<" { return LESSTHAN; }

"<=" { return LESSTHANEQ; }

">" { return MORETHAN; }

">=" { return MORETHANEQ; }

[-+*/;=,)(\{\}] { return *yytext; }

[0-9]+ { yylval.integer = atoi(yytext);  return NUMBER; }

[a-zA-Z][a-zA-Z0-9]* { 
	yylval.string = (char*)calloc(yyleng+1, sizeof(char));
        strncpy(yylval.string, yytext, yyleng); // Remember you need to free this.
	return IDENTIFIER; 
	}

[ \t\n]+ ;
%%

