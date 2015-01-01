typedef union
{
    char*	string;
    int 	integer;
    IcaValue*	value;
    IcaStatement* 	statement;
} YYSTYPE;
#define	EQUALS	258
#define	NEQUALS	259
#define	LESSTHAN	260
#define	LESSTHANEQ	261
#define	MORETHAN	262
#define	MORETHANEQ	263
#define	LOWER_THAN_ELSE	264
#define	ELSE	265
#define	INTEGER	266
#define	NUMBER	267
#define	FLOAT	268
#define	VOID	269
#define	RETURN	270
#define	IF	271
#define	WHILE	272
#define	FOR	273
#define	BREAK	274
#define	IDENTIFIER	275


extern YYSTYPE yylval;
