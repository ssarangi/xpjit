%{
#include "frontend/irtranslation.h"
#include "frontend/ASTBuilder.h"
#include "common/debug.h"

#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <stdarg.h>

using namespace std;

int yylex(void);
// extern void yyerror(string);

extern "C" FILE *yyin;
void yyrestart( FILE *new_file );

//for types
IcaType& getType(int parsedType);
int currentType = -1;

static Debug& gDebug = Debug::getInstance();

static ASTBuilder* builder;
std::vector<IcaValue*> parameterList;

#include <stdio.h>

extern char yytext[];
extern int column;
extern int lineNo;
std::vector<std::string> lines;

void yyerror(std::string s)
{
    fflush(stdout);
    fprintf(stderr, "\n%s", lines[lineNo - 1].c_str());
    std::string error_loc = std::string(" ");
    
    for (int i = 0; i < column; ++i)
        error_loc += std::string(" ");
    
    fprintf(stderr, "\n%s^", error_loc.c_str());
    fprintf(stderr, "\nError on Line %d, Column %d: %s", lineNo, column, s.c_str());
    builder->pushError(s);
}

%}

%union
{
    char         *string;
    int           integer;
    IcaValue     *value;
    IcaStatement *statement;
    std::vector<IcaValue*> *exprList;
}

%left EQUALS NEQUALS LESSTHAN LESSTHANEQ MORETHAN MORETHANEQ
%left '+' '-'
%left  '*' '/'

%locations
%nonassoc '(' ')'
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%token<integer> FUNCTION
%token<integer> INTEGER
%token<integer> NUMBER
%token<integer> FLOAT
%token<integer> VOID
%token<integer> RETURN
%token<integer> IF
%token<integer> ELSE
%token<integer> DO
%token<integer> WHILE
%token<integer> FOR 
%token<integer> BREAK
%token<integer> EQUALS
%token<integer> NEQUALS
%token<integer> PRINTF
%token<integer> NEWLINE

%token<string> IDENTIFIER
%token<string> STRING_LITERAL
%token<string> LSQUARE
%token<string> RSQUARE

%type<integer> datatype

%type<exprList> expression_list

%type<value> expression
%type<value> func_call

%type<statement> return_stmt
%type<statement> assignment
%type<statement> break_statement
%type<statement> iteration_statement

%start program

%%
program:
    | program func_defn
    ;

datatype: INTEGER          { $$ = currentType = IcaType::IntegerTy; }
    | FLOAT                { $$ = currentType = IcaType::FloatTy; }
    | VOID                 { $$ = currentType = IcaType::VoidTy; }
    | STRING_LITERAL       { $$ = currentType = IcaType::StringTy; }
    ;

retlist : datatype
    {
        builder->pushRetType(&getType($1));
    }
    | retlist ',' datatype
    {
        builder->pushDataType(&getType($3));
    }
    ;
    
arglist: datatype IDENTIFIER 
    {
        builder->pushDataType(&getType($1));
        IcaSymbol *sym = builder->addSymbol($2, getType($1));
        builder->pushArgName(sym);
    }
    | arglist ',' datatype IDENTIFIER 
    {
        builder->pushDataType(&getType($3));
        IcaSymbol *sym = builder->addSymbol($4, getType($3));
        builder->pushArgName(sym);
    }
    |
    ;
    
    // as and when we find the identifiers , we need to add them to a list and use them while
    // constructing the prototype/func

func_defn: FUNCTION IDENTIFIER LSQUARE retlist RSQUARE '(' arglist ')' '{' 
    {
        const std::string& funcName = $2;
        IcaFunctionProtoType* fp = builder->getProtoType(funcName); //use current dataTypeList
        
        // find the prototype in the module. if not found, add a new one
        if(fp == NULL)
            builder->addProtoType(funcName, &fp);

        IcErr err = builder->addFunction(*fp);
        
        if (err != eNoErr)
            yyerror(errMsg[err]);
    }

    statement_block '}' 
    {
        // we should clear the m_curFunction after this, so that any global decl will not be a
        // part of prev function's symtab
        builder->endCodeBlock();
    }
    ;

statement_block: statement_block statement |  ;
    
statement: declaration 
    | assignment  { builder->insertStatement(*$1);}
    | expression';' 
    { 
        builder->insertStatement(*new IcaExpressionStatement(*(IcaExpression *)$1));
    }
    | return_stmt ';'{ builder->insertStatement(*$1);}
    | while_statement { }
    | break_statement ';' { builder->insertStatement(*$1); }
    | if_else_statement {  }
    | print_statement {  }
    | iteration_statement
    | ';' { g_outputStream <<"empty statement\n";}
    ;

iteration_statement
    : WHILE '(' expression ')' statement
    | DO statement WHILE '(' expression ')' ';'
    ;


print_statement: PRINTF '(' expression ')' ';'
    {
    }

if_else_statement: IF '(' expression ')' 
    {
        builder->insertStatement(*new IcaBranchStatement(*(IcaExpression*)$3));
    }
        codeblock {  }
    iftail
    ;

iftail: ELSE {
        builder->addBranch(*(IcaExpression*)new IcaConstant(1)); //a 'true' expression
    }
    codeblock { builder->endCodeBlock(); }
    | %prec LOWER_THAN_ELSE //Refer http://stackoverflow.com/questions/1737460/how-to-find-shift-reduce-conflict-in-this-yacc-file
     { builder->endCodeBlock(); }
    ;


while_statement: WHILE '(' expression ')' { builder->insertStatement(*new IcaWhileStatement(*(IcaExpression*)$3)); }
    codeblock { builder->endCodeBlock(); }
    ;

codeblock: '{' statement_block '}'
    | statement
    ;

break_statement: BREAK { $$ = new IcaBreakStatement(); }
    
declaration: datatype varList ';' { currentType = -1; }

varList: IDENTIFIER { builder->addSymbol($1, getType(currentType)); }
    | varList ',' IDENTIFIER { builder->addSymbol($3, getType(currentType)); }
    ;
        
assignment: IDENTIFIER '=' expression ';'
    { 
        IcaSymbol *identifierSymbol = builder->getSymbol($1);
        if(identifierSymbol == NULL)
        {
            std::string error = std::string("Symbol not defined: ") + std::string($1);
            yyerror(error.c_str());
        }

        $$ = new IcaAssignment(*new IcaVariable(*identifierSymbol), *$3);
    }

return_stmt: RETURN LSQUARE expression_list RSQUARE { $$ = new IcaReturnStatement(*$3); }
    | RETURN expression { $$ = new IcaReturnStatement($2); };
    | RETURN { $$ = new IcaReturnStatement(nullptr); }
    ; 

expression: NUMBER { $$ = new IcaConstant($1); }
    | STRING_LITERAL { }
    | IDENTIFIER
    {
        IcaSymbol *identifierSymbol = builder->getSymbol($1);
        if(identifierSymbol == NULL)
        {
            std::string error = std::string("Symbol not defined: ") + std::string($1);
            yyerror(error.c_str());
        }
        $$ = new IcaVariable(*identifierSymbol);
    }
    | expression '+' expression { $$ = new IcaBinopExpression(*$1, *$3, IcaBinopExpression::Add); }
    | expression '-' expression { $$ = new IcaBinopExpression(*$1, *$3, IcaBinopExpression::Sub); }
    | expression '*' expression { $$ = new IcaBinopExpression(*$1, *$3, IcaBinopExpression::Mul); }
    | expression '/' expression { $$ = new IcaBinopExpression(*$1, *$3, IcaBinopExpression::Div); }
    | expression EQUALS expression { $$ = new IcaBinopExpression(*$1, *$3, IcaBinopExpression::EQ); }
    | expression NEQUALS expression { $$ = new IcaBinopExpression(*$1, *$3, IcaBinopExpression::NE); }
    | expression LESSTHAN expression { $$ = new IcaBinopExpression(*$1, *$3, IcaBinopExpression::LT); }
    | expression LESSTHANEQ expression { $$ = new IcaBinopExpression(*$1, *$3, IcaBinopExpression::LTEQ); }
    | expression MORETHAN expression { $$ = new IcaBinopExpression(*$1, *$3, IcaBinopExpression::GT); }
    | expression MORETHANEQ expression { $$ = new IcaBinopExpression(*$1, *$3, IcaBinopExpression::GTEQ); }
    | print_statement { }
    | func_call { $$ = $1; }
    | '('expression')' { $$ = $2; }
    ;

expression_list:
    /*blank*/ { $$=new vector<IcaValue*>(); }
    |expression { $$=new vector<IcaValue*>(); $$->push_back($1); }
    |expression_list ',' expression { $1->push_back($3);$$=$1; }
    ;

func_call: IDENTIFIER'('paramlist')'
    {
        std::vector<IcaType*> paramTypeList;
        
        IcaFunctionProtoType* fp = builder->getFunctionProtoType($1);
        if(fp == NULL)
            yyerror("Function not found");
        $$ = new IcaFunctionCall(*fp, parameterList);
        parameterList.clear();
    }

paramlist: expression { parameterList.push_back($1); }
    | paramlist',' expression { parameterList.push_back($3); }
    |
    ;
%%

int yywrap (void )
{
    return 1;
}

IcaType& getType(int parsedType)
{
    switch(parsedType)
    {
        case IcaType::IntegerTy:
        case IcaType::FloatTy:
        case IcaType::VoidTy:
            return *new IcaType((IcaType::TypeID)parsedType);
            break;
        default: yyerror("Unknown type in getType()");
            break;
    }
}

IcarusModule* ParseFile(const char *filename)
{
    std::ifstream file_handle;
    file_handle.open(filename);

    while (!file_handle.eof())
    {
        std::string current_line = "";
        std::getline(file_handle, current_line);
        lines.push_back(current_line);
    }

    file_handle.close();

    FILE* fp = fopen(filename, "r");
    
    if(!fp)
    {
        fprintf(stderr, "Oops! Couldn't open file %s\n!", filename);
        return NULL;
    }

    builder = new ASTBuilder(); //!!!Check for memory leak
    yyrestart(fp);
    
    if(gDebug.isYaccTraceOn())
        yydebug = 1; //set it to 1 for text based debugging, 5 for graph based debugging

    if (0)
    {
        extern int yydebug;
        yydebug = 1;
    }
    
    yyparse();
    
    if(builder->hasErrors())
    {
        fprintf(stderr, "\nStopping compilation as we found some syntax errors in %s\n!", filename);
        return NULL;
    }
    return &builder->getModule();
}
