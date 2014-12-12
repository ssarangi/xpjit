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
Type& getType(int parsedType);
int currentType = -1;

static Debug& gDebug = Debug::getInstance();

static ASTBuilder* builder;
std::list<IcaValue*> parameterList;

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
}

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

%locations
%nonassoc '(' ')'
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%token IDENTIFIER CONSTANT STRING_LITERAL SIZEOF
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN TYPE_NAME

%token TYPEDEF EXTERN STATIC AUTO REGISTER
%token CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID
%token STRUCT UNION ENUM ELLIPSIS

%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN


%token<integer> INTEGER NUMBER FLOAT VOID RETURN IF ELSE DO WHILE FOR BREAK EQUALS NEQUALS PRINTF NEWLINE
%token<string> IDENTIFIER
%token<string> STRING_LITERAL

%type<integer> datatype
%type<value> expression func_call
%type<statement> return_stmt assignment break_statement

%start program

%%
primary_expression
    : IDENTIFIER
    | CONSTANT
    | STRING_LITERAL
    | '(' expression ')'
    ;

postfix_expression
    : primary_expression
    | postfix_expression '[' expression ']'
    | postfix_expression '(' ')'
    | postfix_expression '(' argument_expression_list ')'
    | postfix_expression '.' IDENTIFIER
    | postfix_expression PTR_OP IDENTIFIER
    | postfix_expression INC_OP
    | postfix_expression DEC_OP
    ;

argument_expression_list
    : assignment_expression
    | argument_expression_list ',' assignment_expression
    ;

unary_expression
    : postfix_expression
    | INC_OP unary_expression
    | DEC_OP unary_expression
    | unary_operator cast_expression
    | SIZEOF unary_expression
    | SIZEOF '(' type_name ')'
    ;

unary_operator
    : '&'
    | '*'
    | '+'
    | '-'
    | '~'
    | '!'
    ;

cast_expression
    : unary_expression
    | '(' type_name ')' cast_expression
    ;

multiplicative_expression
    : cast_expression
    | multiplicative_expression '*' cast_expression
    | multiplicative_expression '/' cast_expression
    | multiplicative_expression '%' cast_expression
    ;

additive_expression
    : multiplicative_expression
    | additive_expression '+' multiplicative_expression
    | additive_expression '-' multiplicative_expression
    ;

shift_expression
    : additive_expression
    | shift_expression LEFT_OP additive_expression
    | shift_expression RIGHT_OP additive_expression
    ;

relational_expression
    : shift_expression
    | relational_expression '<' shift_expression
    | relational_expression '>' shift_expression
    | relational_expression LE_OP shift_expression
    | relational_expression GE_OP shift_expression
    ;

equality_expression
    : relational_expression
    | equality_expression EQ_OP relational_expression
    | equality_expression NE_OP relational_expression
    ;

and_expression
    : equality_expression
    | and_expression '&' equality_expression
    ;

exclusive_or_expression
    : and_expression
    | exclusive_or_expression '^' and_expression
    ;

inclusive_or_expression
    : exclusive_or_expression
    | inclusive_or_expression '|' exclusive_or_expression
    ;

logical_and_expression
    : inclusive_or_expression
    | logical_and_expression AND_OP inclusive_or_expression
    ;

logical_or_expression
    : logical_and_expression
    | logical_or_expression OR_OP logical_and_expression
    ;

conditional_expression
    : logical_or_expression
    | logical_or_expression '?' expression ':' conditional_expression
    ;

assignment_expression
    : conditional_expression
    | unary_expression assignment_operator assignment_expression
    ;

assignment_operator
    : '='
    | MUL_ASSIGN
    | DIV_ASSIGN
    | MOD_ASSIGN
    | ADD_ASSIGN
    | SUB_ASSIGN
    | LEFT_ASSIGN
    | RIGHT_ASSIGN
    | AND_ASSIGN
    | XOR_ASSIGN
    | OR_ASSIGN
    ;

expression
    : assignment_expression
    | expression ',' assignment_expression
    ;

constant_expression
    : conditional_expression
    ;

declaration
    : declaration_specifiers ';'
    | declaration_specifiers init_declarator_list ';'
    ;

declaration_specifiers
    : storage_class_specifier
    | storage_class_specifier declaration_specifiers
    | type_specifier
    | type_specifier declaration_specifiers
    | type_qualifier
    | type_qualifier declaration_specifiers
    ;

init_declarator_list
    : init_declarator
    | init_declarator_list ',' init_declarator
    ;

init_declarator
    : declarator
    | declarator '=' initializer
    ;

storage_class_specifier
    : TYPEDEF
    | EXTERN
    | STATIC
    | AUTO
    | REGISTER
    ;

type_specifier
    : VOID
    | CHAR
    | SHORT
    | INT
    | LONG
    | FLOAT
    | DOUBLE
    | SIGNED
    | UNSIGNED
    | struct_or_union_specifier
    | enum_specifier
    | TYPE_NAME
    ;

struct_or_union_specifier
    : struct_or_union IDENTIFIER '{' struct_declaration_list '}'
    | struct_or_union '{' struct_declaration_list '}'
    | struct_or_union IDENTIFIER
    ;

struct_or_union
    : STRUCT
    | UNION
    ;

struct_declaration_list
    : struct_declaration
    | struct_declaration_list struct_declaration
    ;

struct_declaration
    : specifier_qualifier_list struct_declarator_list ';'
    ;

specifier_qualifier_list
    : type_specifier specifier_qualifier_list
    | type_specifier
    | type_qualifier specifier_qualifier_list
    | type_qualifier
    ;

struct_declarator_list
    : struct_declarator
    | struct_declarator_list ',' struct_declarator
    ;

struct_declarator
    : declarator
    | ':' constant_expression
    | declarator ':' constant_expression
    ;

enum_specifier
    : ENUM '{' enumerator_list '}'
    | ENUM IDENTIFIER '{' enumerator_list '}'
    | ENUM IDENTIFIER
    ;

enumerator_list
    : enumerator
    | enumerator_list ',' enumerator
    ;

enumerator
    : IDENTIFIER
    | IDENTIFIER '=' constant_expression
    ;

type_qualifier
    : CONST
    | VOLATILE
    ;

declarator
    : pointer direct_declarator
    | direct_declarator
    ;

direct_declarator
    : IDENTIFIER
    | '(' declarator ')'
    | direct_declarator '[' constant_expression ']'
    | direct_declarator '[' ']'
    | direct_declarator '(' parameter_type_list ')'
    | direct_declarator '(' identifier_list ')'
    | direct_declarator '(' ')'
    ;

pointer
    : '*'
    | '*' type_qualifier_list
    | '*' pointer
    | '*' type_qualifier_list pointer
    ;

type_qualifier_list
    : type_qualifier
    | type_qualifier_list type_qualifier
    ;


parameter_type_list
    : parameter_list
    | parameter_list ',' ELLIPSIS
    ;

parameter_list
    : parameter_declaration
    | parameter_list ',' parameter_declaration
    ;

parameter_declaration
    : declaration_specifiers declarator
    | declaration_specifiers abstract_declarator
    | declaration_specifiers
    ;

identifier_list
    : IDENTIFIER
    | identifier_list ',' IDENTIFIER
    ;

type_name
    : specifier_qualifier_list
    | specifier_qualifier_list abstract_declarator
    ;

abstract_declarator
    : pointer
    | direct_abstract_declarator
    | pointer direct_abstract_declarator
    ;

direct_abstract_declarator
    : '(' abstract_declarator ')'
    | '[' ']'
    | '[' constant_expression ']'
    | direct_abstract_declarator '[' ']'
    | direct_abstract_declarator '[' constant_expression ']'
    | '(' ')'
    | '(' parameter_type_list ')'
    | direct_abstract_declarator '(' ')'
    | direct_abstract_declarator '(' parameter_type_list ')'
    ;

initializer
    : assignment_expression
    | '{' initializer_list '}'
    | '{' initializer_list ',' '}'
    ;

initializer_list
    : initializer
    | initializer_list ',' initializer
    ;

statement
    : labeled_statement
    | compound_statement
    | expression_statement
    | selection_statement
    | iteration_statement
    | jump_statement
    ;

labeled_statement
    : IDENTIFIER ':' statement
    | CASE constant_expression ':' statement
    | DEFAULT ':' statement
    ;

compound_statement
    : '{' '}'
    | '{' statement_list '}'
    | '{' declaration_list '}'
    | '{' declaration_list statement_list '}'
    ;

declaration_list
    : declaration
    | declaration_list declaration
    ;

statement_list
    : statement
    | statement_list statement
    ;

expression_statement
    : ';'
    | expression ';'
    ;

selection_statement
    : IF '(' expression ')' statement
    | IF '(' expression ')' statement ELSE statement
    | SWITCH '(' expression ')' statement
    ;

iteration_statement
    : WHILE '(' expression ')' statement
    | DO statement WHILE '(' expression ')' ';'
    | FOR '(' expression_statement expression_statement ')' statement
    | FOR '(' expression_statement expression_statement expression ')' statement
    ;

jump_statement
    : GOTO IDENTIFIER ';'
    | CONTINUE ';'
    | BREAK ';'
    | RETURN ';'
    | RETURN expression ';'
    ;

translation_unit
    : external_declaration
    | translation_unit external_declaration
    ;

external_declaration
    : function_definition
    | declaration
    ;

function_definition
    : declaration_specifiers declarator declaration_list compound_statement
    | declaration_specifiers declarator compound_statement
    | declarator declaration_list compound_statement
    | declarator compound_statement
    ;

program: program statement
    | program func_decl
    | program func_defn
    |
    ;

func_decl: datatype IDENTIFIER '(' arglist ')' ';' 
    { 
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
        builder->insertStatement(*new ExpressionStatement(*(Expression *)$1));
    }
    | return_stmt ';'{ builder->insertStatement(*$1);}
    | while_statement { }
    | break_statement ';' { builder->insertStatement(*$1); }
    | if_else_statement {  }
    | print_statement {  }
    | ';' { g_outputStream <<"empty statement\n";}
    ;

print_statement: PRINTF '(' expression ')' ';'
    {
    }

if_else_statement: IF '(' expression ')' 
    {
        builder->insertStatement(*new BranchStatement(*(Expression*)$3));
    }
        codeblock {  }
    iftail
    ;

iftail: ELSE {
        builder->addBranch(*(Expression*)new Constant(1)); //a 'true' expression
    }
    codeblock { builder->endCodeBlock(); }
    | %prec LOWER_THAN_ELSE //Refer http://stackoverflow.com/questions/1737460/how-to-find-shift-reduce-conflict-in-this-yacc-file
     { builder->endCodeBlock(); }
    ;


while_statement: WHILE '(' expression ')' { builder->insertStatement(*new WhileStatement(*(Expression*)$3)); }
    codeblock { builder->endCodeBlock(); }
    ;

codeblock: '{' statement_block '}'
    | statement
    ;

break_statement: BREAK { $$ = new BreakStatement(); }
    
declaration: datatype varList ';' { currentType = -1; }

varList: IDENTIFIER { builder->addSymbol($1, getType(currentType)); }
    | varList',' IDENTIFIER { builder->addSymbol($3, getType(currentType)); }
    ;
    
datatype: INTEGER          { $$ = currentType = Type::IntegerTy; }
    | FLOAT                { $$ = currentType = Type::FloatTy; }
    | VOID                 { $$ = currentType = Type::VoidTy; }
    | STRING_LITERAL       { $$ = currentType = Type::StringTy; }
    ;
    
assignment: IDENTIFIER '=' expression ';'
    { 
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
    | STRING_LITERAL { }
    | IDENTIFIER
    {
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
    | print_statement { }
    | func_call { $$ = $1; }
    | '('expression')' { $$ = $2; }
    ;
    
func_call: IDENTIFIER'('paramlist')'
    {
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

int yywrap (void )
{
    return 1;
}

Type& getType(int parsedType)
{
    switch(parsedType)
    {
        case Type::IntegerTy:
        case Type::FloatTy:
        case Type::VoidTy:
            return *new Type((Type::TypeID)parsedType);
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
    
    yyparse();
    
    if(builder->hasErrors())
    {
        fprintf(stderr, "Stopping compilation as we found some syntax errors in %s\n!", filename);
        return NULL;
    }
    return &builder->getModule();
}
