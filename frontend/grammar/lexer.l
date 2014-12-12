%option nounistd
%option noyywrap
%option stack
%option yylineno
%option never-interactive
%option outfile="lexer.cpp"

D           [0-9]
L           [a-zA-Z_]
H           [a-fA-F0-9]
E           [Ee][+-]?{D}+
FS          (f|F|l|L)
IS          (u|U|l|L)*

%{
class IcaValue;
class Statement;

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

"/*"            { }

"auto"          { count(); return(AUTO); }
"break"         { count(); return(BREAK); }
"case"          { count(); return(CASE); }
"char"          { count(); return(CHAR); }
"const"         { count(); return(CONST); }
"continue"      { count(); return(CONTINUE); }
"default"       { count(); return(DEFAULT); }
"do"            { count(); return(DO); }
"double"        { count(); return(DOUBLE); }
"else"          { count(); return(ELSE); }
"enum"          { count(); return(ENUM); }
"extern"        { count(); return(EXTERN); }
"float"         { count(); return(FLOAT); }
"for"           { count(); return(FOR); }
"goto"          { count(); return(GOTO); }
"if"            { count(); return(IF); }
"int"           { count(); return(INT); }
"long"          { count(); return(LONG); }
"register"      { count(); return(REGISTER); }
"return"        { count(); return(RETURN); }
"short"         { count(); return(SHORT); }
"signed"        { count(); return(SIGNED); }
"sizeof"        { count(); return(SIZEOF); }
"static"        { count(); return(STATIC); }
"struct"        { count(); return(STRUCT); }
"switch"        { count(); return(SWITCH); }
"typedef"       { count(); return(TYPEDEF); }
"union"         { count(); return(UNION); }
"unsigned"      { count(); return(UNSIGNED); }
"void"          { count(); return(VOID); }
"volatile"      { count(); return(VOLATILE); }
"while"         { count(); return(WHILE); }

{L}({L}|{D})*       { count(); return(check_type()); }

0[xX]{H}+{IS}?      { count(); return(CONSTANT); }
0{D}+{IS}?      { count(); return(CONSTANT); }
{D}+{IS}?       { count(); return(CONSTANT); }
L?'(\\.|[^\\'])+'   { count(); return(CONSTANT); }

{D}+{E}{FS}?        { count(); return(CONSTANT); }
{D}*"."{D}+({E})?{FS}?  { count(); return(CONSTANT); }
{D}+"."{D}*({E})?{FS}?  { count(); return(CONSTANT); }

L?\"(\\.|[^\\"])*\" { count(); return(STRING_LITERAL); }

"..."           { count(); return(ELLIPSIS); }
">>="           { count(); return(RIGHT_ASSIGN); }
"<<="           { count(); return(LEFT_ASSIGN); }
"+="            { count(); return(ADD_ASSIGN); }
"-="            { count(); return(SUB_ASSIGN); }
"*="            { count(); return(MUL_ASSIGN); }
"/="            { count(); return(DIV_ASSIGN); }
"%="            { count(); return(MOD_ASSIGN); }
"&="            { count(); return(AND_ASSIGN); }
"^="            { count(); return(XOR_ASSIGN); }
"|="            { count(); return(OR_ASSIGN); }
">>"            { count(); return(RIGHT_OP); }
"<<"            { count(); return(LEFT_OP); }
"++"            { count(); return(INC_OP); }
"--"            { count(); return(DEC_OP); }
"->"            { count(); return(PTR_OP); }
"&&"            { count(); return(AND_OP); }
"||"            { count(); return(OR_OP); }
"<="            { count(); return(LE_OP); }
">="            { count(); return(GE_OP); }
"=="            { count(); return(EQ_OP); }
"!="            { count(); return(NE_OP); }
";"             { count(); return(';'); }
("{"|"<%")      { count(); return('{'); }
("}"|"%>")      { count(); return('}'); }
","         { count(); return(','); }
":"         { count(); return(':'); }
"="         { count(); return('='); }
"("         { count(); return('('); }
")"         { count(); return(')'); }
("["|"<:")      { count(); return('['); }
("]"|":>")      { count(); return(']'); }
"."         { count(); return('.'); }
"&"         { count(); return('&'); }
"!"         { count(); return('!'); }
"~"         { count(); return('~'); }
"-"         { count(); return('-'); }
"+"         { count(); return('+'); }
"*"         { count(); return('*'); }
"/"         { count(); return('/'); }
"%"         { count(); return('%'); }
"<"         { count(); return('<'); }
">"         { count(); return('>'); }
"^"         { count(); return('^'); }
"|"         { count(); return('|'); }
"?"         { count(); return('?'); }


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

[\n\r]          { lineNo++; column = 0; }
%%

