
/*  A Bison parser, made from c:\dev\icarus\frontend\grammar\parser.y with Bison version GNU Bison version 1.24
  */

#define YYBISON 1  /* Identify Bison output.  */

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
#define	PRINT	275
#define	IDENTIFIER	276
#define	STRING_LITERAL	277


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

typedef union
{
    char*   string;
    int     integer;
    IcaValue*   value;
    Statement*  statement;
} YYSTYPE;

#ifndef YYLTYPE
typedef
  struct yyltype
    {
      int timestamp;
      int first_line;
      int first_column;
      int last_line;
      int last_column;
      char *text;
   }
  yyltype;

#define YYLTYPE yyltype
#endif

#ifndef YYDEBUG
#define YYDEBUG 1
#endif

#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		108
#define	YYFLAG		-32768
#define	YYNTBASE	34

#define YYTRANSLATE(x) ((unsigned)(x) <= 277 ? yytranslate[x] : 59)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    13,
    14,    11,     9,    30,    10,     2,    12,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,    29,     2,
    33,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    31,     2,    32,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,     7,     8,    15,    16,    17,    18,    19,    20,    21,
    22,    23,    24,    25,    26,    27,    28
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     3,     6,     9,    10,    17,    20,    25,    26,    27,
    37,    40,    41,    43,    45,    48,    51,    53,    56,    58,
    60,    62,    67,    68,    69,    78,    79,    83,    84,    85,
    92,    96,    98,   100,   104,   106,   110,   112,   114,   116,
   118,   123,   126,   128,   130,   132,   136,   140,   144,   148,
   152,   156,   160,   164,   168,   172,   174,   178,   183,   185,
   189
};

static const short yyrhs[] = {    34,
    40,     0,    34,    35,     0,    34,    37,     0,     0,    53,
    27,    13,    36,    14,    29,     0,    53,    27,     0,    36,
    30,    53,    27,     0,     0,     0,    53,    27,    13,    36,
    14,    31,    38,    39,    32,     0,    39,    40,     0,     0,
    51,     0,    54,     0,    56,    29,     0,    55,    29,     0,
    47,     0,    50,    29,     0,    42,     0,    41,     0,    29,
     0,    26,    13,    56,    14,     0,     0,     0,    22,    13,
    56,    14,    43,    49,    44,    45,     0,     0,    16,    46,
    49,     0,     0,     0,    23,    13,    56,    14,    48,    49,
     0,    31,    39,    32,     0,    40,     0,    25,     0,    53,
    52,    29,     0,    27,     0,    52,    30,    27,     0,    17,
     0,    19,     0,    20,     0,    28,     0,    27,    33,    56,
    29,     0,    21,    56,     0,    21,     0,    18,     0,    27,
     0,    56,     9,    56,     0,    56,    10,    56,     0,    56,
    11,    56,     0,    56,    12,    56,     0,    56,     3,    56,
     0,    56,     4,    56,     0,    56,     5,    56,     0,    56,
     6,    56,     0,    56,     7,    56,     0,    56,     8,    56,
     0,    57,     0,    13,    56,    14,     0,    27,    13,    58,
    14,     0,    56,     0,    58,    30,    56,     0,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    56,    57,    58,    59,    62,    72,    78,    84,    88,   100,
   106,   106,   108,   109,   110,   115,   116,   117,   118,   119,
   120,   123,   128,   133,   135,   137,   140,   141,   146,   147,
   150,   151,   154,   156,   158,   159,   162,   163,   164,   165,
   168,   180,   181,   184,   185,   195,   196,   197,   198,   199,
   200,   201,   202,   203,   204,   205,   206,   209,   221,   222,
   223
};

static const char * const yytname[] = {   "$","error","$undefined.","EQUALS",
"NEQUALS","LESSTHAN","LESSTHANEQ","MORETHAN","MORETHANEQ","'+'","'-'","'*'",
"'/'","'('","')'","LOWER_THAN_ELSE","ELSE","INTEGER","NUMBER","FLOAT","VOID",
"RETURN","IF","WHILE","FOR","BREAK","PRINT","IDENTIFIER","STRING_LITERAL","';'",
"','","'{'","'}'","'='","program","func_decl","arglist","func_defn","@1","statement_block",
"statement","print_statement","if_else_statement","@2","@3","iftail","@4","while_statement",
"@5","codeblock","break_statement","declaration","varList","datatype","assignment",
"return_stmt","expression","func_call","paramlist",""
};
#endif

static const short yyr1[] = {     0,
    34,    34,    34,    34,    35,    36,    36,    36,    38,    37,
    39,    39,    40,    40,    40,    40,    40,    40,    40,    40,
    40,    41,    43,    44,    42,    46,    45,    45,    48,    47,
    49,    49,    50,    51,    52,    52,    53,    53,    53,    53,
    54,    55,    55,    56,    56,    56,    56,    56,    56,    56,
    56,    56,    56,    56,    56,    56,    56,    57,    58,    58,
    58
};

static const short yyr2[] = {     0,
     2,     2,     2,     0,     6,     2,     4,     0,     0,     9,
     2,     0,     1,     1,     2,     2,     1,     2,     1,     1,
     1,     4,     0,     0,     8,     0,     3,     0,     0,     6,
     3,     1,     1,     3,     1,     3,     1,     1,     1,     1,
     4,     2,     1,     1,     1,     3,     3,     3,     3,     3,
     3,     3,     3,     3,     3,     1,     3,     4,     1,     3,
     0
};

static const short yydefact[] = {     4,
     0,     0,    37,    44,    38,    39,    43,     0,     0,    33,
     0,    45,    40,    21,     2,     3,     1,    20,    19,    17,
     0,    13,     0,    14,     0,     0,    56,    45,     0,    42,
     0,     0,     0,    61,     0,    18,    35,     0,    16,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    15,
    57,     0,     0,     0,    59,     0,     0,     8,    34,     0,
    50,    51,    52,    53,    54,    55,    46,    47,    48,    49,
    23,    29,    22,    58,     0,    41,     0,     0,    36,     0,
     0,    60,     0,     0,     6,    12,    32,    24,     0,    30,
     5,     9,     0,     0,    28,    35,    12,     7,    31,    11,
    26,    25,     0,     0,    10,    27,     0,     0
};

static const short yydefgoto[] = {     1,
    15,    77,    16,    97,    94,    87,    18,    19,    80,    95,
   102,   104,    20,    81,    88,    21,    22,    38,    89,    24,
    25,    26,    27,    56
};

static const short yypact[] = {-32768,
     2,    -9,-32768,-32768,-32768,-32768,    -9,    27,    38,-32768,
    39,    -7,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   -17,-32768,   -14,-32768,    28,    56,-32768,    40,   151,   199,
    -9,    -9,    -9,    -9,    -9,-32768,    43,   -19,-32768,    -9,
    -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,-32768,
-32768,   163,   175,   187,   199,    24,   100,    52,-32768,    46,
   104,   104,   104,   104,   104,   104,     5,     5,-32768,-32768,
-32768,-32768,-32768,-32768,    -9,-32768,    25,    47,-32768,   122,
   122,   199,   -24,    52,-32768,-32768,-32768,-32768,    48,-32768,
-32768,-32768,    50,    69,    54,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,   105,   122,-32768,-32768,    78,-32768
};

static const short yypgoto[] = {-32768,
-32768,-32768,-32768,-32768,   -18,    -1,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,   -67,-32768,-32768,-32768,     0,-32768,
-32768,     1,-32768,-32768
};


#define	YYLAST		211


static const short yytable[] = {    17,
    23,   107,    29,     2,    91,    34,    92,    30,     4,    59,
    60,    36,    37,    90,     2,    48,    49,    28,     3,     4,
     5,     6,     7,     8,     9,    35,    10,    11,    12,    13,
    14,    52,    53,    54,    55,    57,   106,    74,    83,    31,
    61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
    32,    33,    34,    75,    84,    58,    39,    78,    40,    41,
    42,    43,    44,    45,    46,    47,    48,    49,     3,   101,
     5,     6,    79,    85,    96,    82,    98,   108,   103,    13,
     0,     2,     0,    93,    50,     3,     4,     5,     6,     7,
     8,     9,   100,    10,    11,    12,    13,    14,     0,     0,
    99,   100,    40,    41,    42,    43,    44,    45,    46,    47,
    48,    49,    46,    47,    48,    49,     0,     2,     0,     0,
     0,     3,     4,     5,     6,     7,     8,     9,    76,    10,
    11,    12,    13,    14,     2,     0,   105,     0,     3,     4,
     5,     6,     7,     8,     9,     0,    10,    11,    12,    13,
    14,     0,    86,    40,    41,    42,    43,    44,    45,    46,
    47,    48,    49,     0,    51,    40,    41,    42,    43,    44,
    45,    46,    47,    48,    49,     0,    71,    40,    41,    42,
    43,    44,    45,    46,    47,    48,    49,     0,    72,    40,
    41,    42,    43,    44,    45,    46,    47,    48,    49,     0,
    73,    40,    41,    42,    43,    44,    45,    46,    47,    48,
    49
};

static const short yycheck[] = {     1,
     1,     0,     2,    13,    29,    13,    31,     7,    18,    29,
    30,    29,    27,    81,    13,    11,    12,    27,    17,    18,
    19,    20,    21,    22,    23,    33,    25,    26,    27,    28,
    29,    31,    32,    33,    34,    35,   104,    14,    14,    13,
    40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
    13,    13,    13,    30,    30,    13,    29,    58,     3,     4,
     5,     6,     7,     8,     9,    10,    11,    12,    17,    16,
    19,    20,    27,    27,    27,    75,    27,     0,    97,    28,
    -1,    13,    -1,    84,    29,    17,    18,    19,    20,    21,
    22,    23,    94,    25,    26,    27,    28,    29,    -1,    -1,
    32,   103,     3,     4,     5,     6,     7,     8,     9,    10,
    11,    12,     9,    10,    11,    12,    -1,    13,    -1,    -1,
    -1,    17,    18,    19,    20,    21,    22,    23,    29,    25,
    26,    27,    28,    29,    13,    -1,    32,    -1,    17,    18,
    19,    20,    21,    22,    23,    -1,    25,    26,    27,    28,
    29,    -1,    31,     3,     4,     5,     6,     7,     8,     9,
    10,    11,    12,    -1,    14,     3,     4,     5,     6,     7,
     8,     9,    10,    11,    12,    -1,    14,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    -1,    14,     3,
     4,     5,     6,     7,     8,     9,    10,    11,    12,    -1,
    14,     3,     4,     5,     6,     7,     8,     9,    10,    11,
    12
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */


/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

#ifndef alloca
#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi)
#include <alloca.h>
#else /* not sparc */
#if defined (MSDOS) && !defined (__TURBOC__)
#include <malloc.h>
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
#include <malloc.h>
 #pragma alloca
#else /* not MSDOS, __TURBOC__, or _AIX */
#ifdef __hpux
#ifdef __cplusplus
extern "C" {
void *alloca (unsigned int);
};
#else /* not __cplusplus */
void *alloca ();
#endif /* not __cplusplus */
#endif /* __hpux */
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc.  */
#endif /* not GNU C.  */
#endif /* alloca not defined.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	return(0)
#define YYABORT 	return(1)
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
int yyparse (void);
#endif

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(FROM,TO,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (from, to, count)
     char *from;
     char *to;
     int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *from, char *to, int count)
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif



/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#else
#define YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#endif

int
yyparse(YYPARSE_PARAM)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      yyvs = (YYSTYPE *) alloca (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) alloca (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 5:
{ 
        g_outputStream <<"function declaration\n";
        const std::string& string = yyvsp[-4].string;
        IcErr err = builder->addProtoType(string, getType(yyvsp[-5].integer), NULL);
        if(err != eNoErr)
            yyerror(errMsg[err]);
    ;
    break;}
case 6:
{
        builder->pushDataType(&getType(yyvsp[-1].integer));
        Symbol *sym = builder->addSymbol(yyvsp[0].string, getType(yyvsp[-1].integer));
        builder->pushArgName(sym);
    ;
    break;}
case 7:
{
        builder->pushDataType(&getType(yyvsp[-1].integer));
        Symbol *sym = builder->addSymbol(yyvsp[0].string, getType(yyvsp[-1].integer));
        builder->pushArgName(sym);
    ;
    break;}
case 9:
{
        g_outputStream <<"function definition\n";
        const std::string& string = yyvsp[-4].string;
        FunctionProtoType* fp = builder->getProtoType(string);//use current dataTypeList
        if(fp == NULL) //find the prototype in the module. if not found, add a new one
            builder->addProtoType(string, getType(yyvsp[-5].integer), &fp);
        IcErr err = builder->addFunction(*fp);
        if(err != eNoErr)
            yyerror(errMsg[err]);
    ;
    break;}
case 10:
{   //we should clear the m_curFunction after this, so that any global decl will not be a part of prev function's symtab
        builder->endCodeBlock();
    ;
    break;}
case 14:
{ builder->insertStatement(*yyvsp[0].statement);;
    break;}
case 15:
{ 
        g_outputStream <<"expression\n";
        builder->insertStatement(*new ExpressionStatement(*(Expression *)yyvsp[-1].value));
    ;
    break;}
case 16:
{ builder->insertStatement(*yyvsp[-1].statement);;
    break;}
case 17:
{ g_outputStream << "done with while loop\n"; ;
    break;}
case 18:
{ g_outputStream << "break\n"; builder->insertStatement(*yyvsp[-1].statement); ;
    break;}
case 19:
{ g_outputStream << "if else"; ;
    break;}
case 20:
{ g_outputStream << "print" ;
    break;}
case 21:
{ g_outputStream <<"empty statement\n";;
    break;}
case 22:
{
        
    ;
    break;}
case 23:
{
        g_outputStream <<"if statement ";
        builder->insertStatement(*new BranchStatement(*(Expression*)yyvsp[-1].value));
    ;
    break;}
case 24:
{ g_outputStream <<"ending if block";  ;
    break;}
case 26:
{
        builder->addBranch(*(Expression*)new Constant(1)); //a 'true' expression
    ;
    break;}
case 27:
{ g_outputStream <<"ending else block"; builder->endCodeBlock(); ;
    break;}
case 28:
{ builder->endCodeBlock(); ;
    break;}
case 29:
{ g_outputStream <<"while statement\n"; builder->insertStatement(*new WhileStatement(*(Expression*)yyvsp[-1].value)); ;
    break;}
case 30:
{ g_outputStream <<"ending while loop\n"; builder->endCodeBlock(); ;
    break;}
case 33:
{ yyval.statement = new BreakStatement(); ;
    break;}
case 34:
{ g_outputStream <<"declaration "; currentType = -1; ;
    break;}
case 35:
{ builder->addSymbol(yyvsp[0].string, getType(currentType)); ;
    break;}
case 36:
{ builder->addSymbol(yyvsp[0].string, getType(currentType)); ;
    break;}
case 37:
{ g_outputStream << "int "; yyval.integer = currentType = Type::IntegerTy; ;
    break;}
case 38:
{ g_outputStream << "float "; yyval.integer = currentType = Type::FloatTy; ;
    break;}
case 39:
{ g_outputStream << "void "; yyval.integer = currentType = Type::VoidTy; ;
    break;}
case 40:
{ g_outputStream << "string"; yyval.integer = currentType = Type::StringTy; ;
    break;}
case 41:
{ 
        g_outputStream <<"assignment";
        Symbol *identifierSymbol = builder->getSymbol(yyvsp[-3].string);
        if(identifierSymbol == NULL)
        {
            std::string error = std::string("Symbol not defined: ") + std::string(yyvsp[-3].string);
            yyerror(error.c_str());
        }
        yyval.statement = new Assignment(*new Variable(*identifierSymbol), *yyvsp[-1].value);
    ;
    break;}
case 42:
{ yyval.statement = new ReturnStatement(yyvsp[0].value);;
    break;}
case 43:
{ yyval.statement = new ReturnStatement(NULL);;
    break;}
case 44:
{ yyval.value = new Constant(yyvsp[0].integer); ;
    break;}
case 45:
{
        g_outputStream <<"identifier";
        Symbol *identifierSymbol = builder->getSymbol(yyvsp[0].string);
        if(identifierSymbol == NULL)
        {
            std::string error = std::string("Symbol not defined: ") + std::string(yyvsp[0].string);
            yyerror(error.c_str());
        }
        yyval.value = new Variable(*identifierSymbol);
    ;
    break;}
case 46:
{ yyval.value = new BinopExpression(*yyvsp[-2].value, *yyvsp[0].value, BinopExpression::Add); ;
    break;}
case 47:
{ yyval.value = new BinopExpression(*yyvsp[-2].value, *yyvsp[0].value, BinopExpression::Sub); ;
    break;}
case 48:
{ yyval.value = new BinopExpression(*yyvsp[-2].value, *yyvsp[0].value, BinopExpression::Mul); ;
    break;}
case 49:
{ yyval.value = new BinopExpression(*yyvsp[-2].value, *yyvsp[0].value, BinopExpression::Div); ;
    break;}
case 50:
{ yyval.value = new BinopExpression(*yyvsp[-2].value, *yyvsp[0].value, BinopExpression::EQ); ;
    break;}
case 51:
{ yyval.value = new BinopExpression(*yyvsp[-2].value, *yyvsp[0].value, BinopExpression::NE); ;
    break;}
case 52:
{ yyval.value = new BinopExpression(*yyvsp[-2].value, *yyvsp[0].value, BinopExpression::LT); ;
    break;}
case 53:
{ yyval.value = new BinopExpression(*yyvsp[-2].value, *yyvsp[0].value, BinopExpression::LTEQ); ;
    break;}
case 54:
{ yyval.value = new BinopExpression(*yyvsp[-2].value, *yyvsp[0].value, BinopExpression::GT); ;
    break;}
case 55:
{ yyval.value = new BinopExpression(*yyvsp[-2].value, *yyvsp[0].value, BinopExpression::GTEQ); ;
    break;}
case 56:
{ yyval.value = yyvsp[0].value; ;
    break;}
case 57:
{ yyval.value = yyvsp[-1].value; ;
    break;}
case 58:
{
        g_outputStream <<"function called";
        std::list<Type*> paramTypeList;
        
        FunctionProtoType* fp = builder->getFunctionProtoType(yyvsp[-3].string);
        if(fp == NULL)
            yyerror("Function not found");
        yyval.value = new FunctionCall(*fp, parameterList);
        parameterList.clear();
    ;
    break;}
case 59:
{ parameterList.push_back(yyvsp[0].value); ;
    break;}
case 60:
{ parameterList.push_back(yyvsp[0].value); ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */


  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;
}


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
