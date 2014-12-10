%{
#include "frontend/irtranslation.h"
#include "frontend/ASTBuilder.h"
#include "common/debug.h"

#include <cstdio>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

using namespace std;

int yylex(void);
extern void yyerror(string);
extern "C" FILE *yyin;
void yyrestart( FILE *new_file );

//for types
Type& getType(int parsedType);
int currentType = -1;

static Debug& gDebug = Debug::getInstance();

static ASTBuilder* builder;
std::list<IcaValue*> parameterList;
%}

%union
{
    char*   string;
    int     integer;
    IcaValue*   value;
    Statement*  statement;
}

%left EQUALS NEQUALS LESSTHAN LESSTHANEQ MORETHAN MORETHANEQ
%left '+' '-'
%left  '*' '/'

%nonassoc '(' ')'
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%token<integer> INTEGER NUMBER FLOAT VOID RETURN IF ELSE WHILE FOR BREAK EQUALS NEQUALS PRINT
%token<string> IDENTIFIER
%token<string> STRING_LITERAL

%type<integer> datatype
%type<value> expression func_call
%type<statement> return_stmt assignment break_statement

%start program

%%
program: program statement
    | program func_decl
    | program func_defn
    |
    ;

func_decl: datatype IDENTIFIER '(' arglist ')' ';' 
    { 
        g_outputStream <<"function declaration\n";
        const std::string& string = $2;
        IcErr err = builder->addProtoType(string, getType($1), NULL);
        if(err != eNoErr)
            yyerror(errMsg[err]);
    }
    ;
    
arglist: datatype IDENTIFIER 
    {
        builder->pushDataType(&getType($1));
        Symbol *sym = builder->addSymbol($2, getType($1));
        builder->pushArgName(sym);
    }
    | arglist ',' datatype IDENTIFIER 
    {
        builder->pushDataType(&getType($3));
        Symbol *sym = builder->addSymbol($4, getType($3));
        builder->pushArgName(sym);
    }
    |
    ;
//as and when we find the identifiers , we need to add them to a list and use them while constructing the prototype/func

func_defn: datatype IDENTIFIER '(' arglist ')' '{' 
    {
        g_outputStream <<"function definition\n";
        const std::string& string = $2;
        FunctionProtoType* fp = builder->getProtoType(string);//use current dataTypeList
        if(fp == NULL) //find the prototype in the module. if not found, add a new one
            builder->addProtoType(string, getType($1), &fp);
        IcErr err = builder->addFunction(*fp);
        if(err != eNoErr)
            yyerror(errMsg[err]);
    }

    statement_block '}' 
    {   //we should clear the m_curFunction after this, so that any global decl will not be a part of prev function's symtab
        builder->endCodeBlock();
    }
    ;
    
statement_block: statement_block statement |  ;
    
statement: declaration 
    | assignment  { builder->insertStatement(*$1);}
    | expression';' 
    { 
        g_outputStream <<"expression\n";
        builder->insertStatement(*new ExpressionStatement(*(Expression *)$1));
    }
    | return_stmt ';'{ builder->insertStatement(*$1);}
    | while_statement { g_outputStream << "done with while loop\n"; }
    | break_statement ';' { g_outputStream << "break\n"; builder->insertStatement(*$1); }
    | if_else_statement { g_outputStream << "if else"; }
    | print_statement { g_outputStream << "print" }
    | ';' { g_outputStream <<"empty statement\n";}
    ;

print_statement: PRINT '(' expression ')'
    {
        
    }

if_else_statement: IF '(' expression ')' 
    {
        g_outputStream <<"if statement ";
        builder->insertStatement(*new BranchStatement(*(Expression*)$3));
    }
        codeblock { g_outputStream <<"ending if block";  }
    iftail
    ;

iftail: ELSE {
        builder->addBranch(*(Expression*)new Constant(1)); //a 'true' expression
    }
    codeblock { g_outputStream <<"ending else block"; builder->endCodeBlock(); }
    | %prec LOWER_THAN_ELSE //Refer http://stackoverflow.com/questions/1737460/how-to-find-shift-reduce-conflict-in-this-yacc-file
     { builder->endCodeBlock(); }
    ;


while_statement: WHILE '(' expression ')' { g_outputStream <<"while statement\n"; builder->insertStatement(*new WhileStatement(*(Expression*)$3)); }
    codeblock { g_outputStream <<"ending while loop\n"; builder->endCodeBlock(); }
    ;

codeblock: '{' statement_block '}'
    | statement
    ;

break_statement: BREAK { $$ = new BreakStatement(); }
    
declaration: datatype varList ';' { g_outputStream <<"declaration "; currentType = -1; }

varList: IDENTIFIER { builder->addSymbol($1, getType(currentType)); }
    | varList',' IDENTIFIER { builder->addSymbol($3, getType(currentType)); }
    ;
    
datatype: INTEGER   { g_outputStream << "int "; $$ = currentType = Type::IntegerTy; }
    | FLOAT     { g_outputStream << "float "; $$ = currentType = Type::FloatTy; }
    | VOID      { g_outputStream << "void "; $$ = currentType = Type::VoidTy; }
    | STRING_LITERAL       { g_outputStream << "string"; $$ = currentType = Type::StringTy; }
    ;
    
assignment: IDENTIFIER '=' expression ';'
    { 
        g_outputStream <<"assignment";
        Symbol *identifierSymbol = builder->getSymbol($1);
        if(identifierSymbol == NULL)
        {
            std::string error = std::string("Symbol not defined: ") + std::string($1);
            yyerror(error.c_str());
        }
        $$ = new Assignment(*new Variable(*identifierSymbol), *$3);
    }

return_stmt: RETURN expression { $$ = new ReturnStatement($2);};
    | RETURN { $$ = new ReturnStatement(NULL);}
    ; 

expression: NUMBER { $$ = new Constant($1); }
    | IDENTIFIER {
        g_outputStream <<"identifier";
        Symbol *identifierSymbol = builder->getSymbol($1);
        if(identifierSymbol == NULL)
        {
            std::string error = std::string("Symbol not defined: ") + std::string($1);
            yyerror(error.c_str());
        }
        $$ = new Variable(*identifierSymbol);
    }
    | expression '+' expression { $$ = new BinopExpression(*$1, *$3, BinopExpression::Add); }
    | expression '-' expression { $$ = new BinopExpression(*$1, *$3, BinopExpression::Sub); }
    | expression '*' expression { $$ = new BinopExpression(*$1, *$3, BinopExpression::Mul); }
    | expression '/' expression { $$ = new BinopExpression(*$1, *$3, BinopExpression::Div); }
    | expression EQUALS expression { $$ = new BinopExpression(*$1, *$3, BinopExpression::EQ); }
    | expression NEQUALS expression { $$ = new BinopExpression(*$1, *$3, BinopExpression::NE); }
    | expression LESSTHAN expression { $$ = new BinopExpression(*$1, *$3, BinopExpression::LT); }
    | expression LESSTHANEQ expression { $$ = new BinopExpression(*$1, *$3, BinopExpression::LTEQ); }
    | expression MORETHAN expression { $$ = new BinopExpression(*$1, *$3, BinopExpression::GT); }
    | expression MORETHANEQ expression { $$ = new BinopExpression(*$1, *$3, BinopExpression::GTEQ); }
    | func_call { $$ = $1; }
    | '('expression')' { $$ = $2; }
    ;
    
func_call: IDENTIFIER'('paramlist')'
    {
        g_outputStream <<"function called";
        std::list<Type*> paramTypeList;
        
        FunctionProtoType* fp = builder->getFunctionProtoType($1);
        if(fp == NULL)
            yyerror("Function not found");
        $$ = new FunctionCall(*fp, parameterList);
        parameterList.clear();
    }

paramlist: expression { parameterList.push_back($1); }
    | paramlist',' expression { parameterList.push_back($3); }
    |
    ;
%%

void yyerror(string s) {
    fprintf(stderr, "%s\n", s.c_str());
    builder->pushError(s);
}

int yywrap (void ) {
    return 1;
}

Type& getType(int parsedType){
    switch(parsedType){
        case Type::IntegerTy:
        case Type::FloatTy:
        case Type::VoidTy:		
            return *new Type((Type::TypeID)parsedType);
            break;
        default: yyerror("Unknown type in getType()");
            break;
    }
}

IcarusModule* ParseFile(const char *filename){
    FILE* fp = fopen(filename, "r");
    
    if(!fp){
        fprintf(stderr, "Oops! Couldn't open file %s\n!", filename);
        return NULL;
    }
    
    builder = new ASTBuilder(); //!!!Check for memory leak
    yyrestart(fp);
    if(gDebug.isYaccTraceOn())
        yydebug = 1; //set it to 1 for text based debugging, 5 for graph based debugging
    yyparse();
    if(builder->hasErrors()){
        fprintf(stderr, "Stopping compilation as we found some syntax errors in %s\n!", filename);
        return NULL;
    }
    return &builder->getModule();	
}
