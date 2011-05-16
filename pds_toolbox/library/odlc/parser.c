


/*****************************************************************************

 Description: This file contains the parser for the Object Description
              Language (ODL).  The parser is produced using Yacc and
              all changes to the parsing scheme should be made by
              modifying the Yacc input file rather than the
              C-language code produced from by Yacc.

 Author:  Randy Davis, University of Colorado LASP

 Creation Date: 17 April 1990
 Last Modified: 18 May 1991

 History:

   Creation - This module was introduced in the Version 1 ODLC library.

   Version 2.0 - 30 August 1990 - R. Davis, U. of Colorado LASP
     a) Modified to comply with ODL Version 2.  This includes adding
        support for GROUP statements.

   Version 2.0.1 - 26 November 1990 - R. Davis, U. of Colorado LASP
     a) Changed parsing scheme to provide better error reporting and
        recovery.

   Version 2.1 - 13 March 1991
     a) Modified calls to parser action routines to pass pointers to
        value structures rather than copying in the entire structure.

  Version 2.2 - 18 May 1991 - M. DeMore, Jet Propulsion Laboratory
    Added include file odlinter.h.

  Version 2.2 - 18 May 1991 - M. DeMore, Jet Propulsion Laboratory
    Added include file odlinter.h. Added pds_watch_ends flag for
    toolbox users, to eliminate the "missing END" warning.

  Version 2.3 - 10 July 1992 - M. DeMore, Jet Propulsion Laboratory
    Added handling for semicolons.

  Version 2.4 - 17 Oct 2003 - D. Schultz, Jet Propulsion Laboratory
    Added handling for NULL values and name space ie: MISSION:TARGET

*****************************************************************************/

/*****************************************************************************
 To allow the use of the resulting file on multiple platforms, the following
 changes should be performed on the resulting C file after its generation
 by the Sun Version of Yacc:

    1) Delete the line

          extern char *malloc(), *realloc();

*****************************************************************************/


#include "odldef.h"
#include "odlinter.h"

#ifdef PDS_TOOLBOX
    extern int pds_watch_ends;
#else
    int pds_watch_ends = TRUE;
#endif

/* A global flag telling whether we're inside a units expression. If so,
   we don't want "." interpreted as part of a value. Instead, it's a synomym
   for "*" (_multiply_operator). */

int in_units = 0;


/* A special lexer function that respects whether we're inside a units
   expression. */

extern int yylex_lvtool(int units_flag);

#define yylex() yylex_lvtool(in_units)



typedef union
#ifdef __cplusplus
	YYSTYPE
#endif
        {
               struct Value_Data  item;
               int                flag;
	      } YYSTYPE;
# define _OBJECT 257
# define _END_OBJECT 258
# define _GROUP 259
# define _END_GROUP 260
# define _END 261
# define _semi 262
# define _sequence_opening 263
# define _sequence_closing 264
# define _set_opening 265
# define _set_closing 266
# define _units_opening 267
# define _units_closing 268
# define _list_separator 269
# define _point_operator 270
# define _assignment_operator 271
# define _multiply_operator 272
# define _alt_multiply_operator 273
# define _divide_operator 274
# define _exponentiate_operator 275
# define _range_operator 276
# define _date 277
# define _date_time 278
# define _date_timeV0 279
# define _integer 280
# define _name 281
# define _real 282
# define _symbol 283
# define _text_string 284
# define _time 285
# define _null_str 286

#ifndef MSDOS_TC
#include <inttypes.h>
#else
#ifdef MSDOS_TC
#include <stddef.h>
#endif
#endif

#ifdef __STDC__
#include <stdlib.h>
#include <string.h>
#else
#include <malloc.h>
#include <memory.h>
#endif

#ifndef MSDOS_TC
//#include <values.h>
#endif

#if defined(__cplusplus) || defined(__STDC__)

#if defined(__cplusplus) && defined(__EXTERN_C__)
extern "C" {
#endif
#ifndef yyerror
#if defined(__cplusplus)
	void yyerror(const char *);
#endif
#endif
#ifndef yylex
	int yylex(void);
#endif
	int yyparse(void);
#if defined(__cplusplus) && defined(__EXTERN_C__)
}
#endif

#endif

#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern int yyerrflag;
YYSTYPE yylval;
YYSTYPE yyval;
typedef int yytabelem;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
#if YYMAXDEPTH > 0
int yy_yys[YYMAXDEPTH], *yys = yy_yys;
YYSTYPE yy_yyv[YYMAXDEPTH], *yyv = yy_yyv;
#else	/* user does initial allocation */
int *yys;
YYSTYPE *yyv;
#endif
static int yymaxdepth = YYMAXDEPTH;
# define YYERRCODE 256





/* Error handling routine.  This routine is called, explicitly or
   implicitly, whenever a syntax error is detected.                         */

yyerror (error_msg)
  char *error_msg;                 /* Error message text                    */

{
  ODLPrintError (error_msg);

  return;
}
static const yytabelem yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
-1, 2,
	0, 1,
	-2, 0,
-1, 31,
	263, 31,
	265, 31,
	277, 31,
	278, 31,
	279, 31,
	280, 31,
	281, 31,
	282, 31,
	283, 31,
	284, 31,
	285, 31,
	286, 31,
	-2, 0,
	};
# define YYNPROD 105
# define YYLAST 240
static const yytabelem yyact[]={

    75,   123,    96,    44,    43,    42,    41,    86,    82,    33,
    75,    59,    71,   121,    40,   105,    37,    36,    82,    35,
    34,    61,    63,    64,    59,    65,    60,    66,    67,    62,
    68,    61,    63,    64,    59,    65,    60,    66,    67,    62,
    68,    69,    94,    58,    75,   100,    80,    86,    30,    29,
   102,    28,    27,    26,    72,    61,    63,    64,    59,    65,
    60,    66,    67,    62,    68,    61,    63,    64,    59,    65,
    60,    66,    67,    62,    68,    61,    63,    64,    59,    65,
    60,    66,    67,    62,    68,     7,    19,    20,    21,    22,
    12,   130,    32,   112,    25,   134,   119,    39,   127,    18,
   110,    93,   132,   120,   141,   107,   111,    31,    56,   119,
    17,    74,   116,   117,   118,   129,   113,   135,   113,   133,
   119,    99,    76,    92,    45,   116,   117,   118,    83,    98,
    91,   101,   114,    84,    89,    57,   116,   117,   118,     3,
    90,    79,    23,    89,   125,   108,   104,    24,    81,    78,
    46,    77,    54,    53,    52,    51,    49,    48,    47,    70,
    38,    50,    15,    13,    11,    10,     9,     8,     6,     5,
     4,     2,   115,    55,   126,   109,    14,    16,    85,     1,
     0,     0,    46,     0,     0,     0,     0,    73,     0,     0,
     0,     0,     0,    50,    88,    87,     0,     0,     0,     0,
     0,   103,    95,     0,     0,    97,     0,     0,     0,     0,
   106,     0,     0,     0,   124,     0,   122,     0,     0,     0,
     0,   131,   128,     0,     0,     0,     0,     0,     0,     0,
   136,     0,   138,   137,     0,     0,   139,     0,     0,   140 };
static const yytabelem yypact[]={

  -171,-10000000,  -171,-10000000,-10000000,-10000000,-10000000,  -171,-10000000,-10000000,
  -168,  -209,-10000000,  -210,  -211,  -213,  -214,  -164,  -272,  -251,
  -252,  -254,  -255,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,  -159,-10000000,  -257,  -275,  -276,  -277,  -278,  -222,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
  -264,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,  -212,-10000000,
-10000000,-10000000,-10000000,-10000000,  -221,-10000000,-10000000,-10000000,-10000000,  -256,
  -222,  -269,-10000000,  -126,-10000000,-10000000,-10000000,  -225,  -225,-10000000,
  -279,  -225,-10000000,  -135,  -219,-10000000,  -246,-10000000,-10000000,  -202,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,  -253,-10000000,-10000000,-10000000,
-10000000,  -216,-10000000,-10000000,  -163,-10000000,-10000000,  -136,-10000000,  -262,
  -163,  -280,  -225,-10000000,-10000000,  -165,-10000000,-10000000,-10000000,  -225,
  -161,-10000000,  -147,-10000000,-10000000,-10000000,  -262,  -163,  -280,-10000000,
  -225,-10000000,-10000000,-10000000,  -225,-10000000,  -161,  -160,-10000000,-10000000,
-10000000,-10000000 };
static const yytabelem yypgo[]={

     0,   179,   122,   177,   176,   175,   174,   106,   102,   173,
   172,   171,   139,   170,   169,   168,   167,   166,   165,   164,
   163,   162,   160,   124,   159,   111,   158,   157,   156,   155,
   154,   153,   152,   151,   123,   149,   148,   101,   146,   105,
   145,   144,   103,   141,   108,   135,   128,   133,   131 };
static const yytabelem yyr1[]={

     0,     1,    11,    11,    12,    12,    12,    12,    13,    13,
    14,    14,    14,    14,    15,    16,    16,    16,    16,    20,
    20,     4,     4,    17,    17,    17,    17,    21,    21,     3,
     3,    22,    18,    18,    18,    24,    19,    23,    23,    23,
    23,    25,    25,    25,    25,    25,    25,    33,     2,    35,
    29,    36,     9,    34,    34,    38,    37,    39,    39,    40,
    40,    40,     5,     5,    41,    41,    41,     6,     6,     6,
     7,     7,    10,    10,    10,    10,    42,     8,     8,    30,
    30,    30,    30,    43,    43,    31,    31,    32,    26,    26,
    44,    44,    44,    46,    46,    46,    45,    47,    47,    48,
    48,    27,    27,    27,    28 };
static const yytabelem yyr2[]={

     0,     3,     2,     4,     2,     2,     2,     4,     2,     2,
     2,     2,     4,     4,     3,     2,     2,     4,     4,     3,
     7,     3,     7,     2,     2,     4,     4,     3,     7,     3,
     7,     1,     8,     7,     5,     1,    10,     3,     3,     3,
     3,     2,     2,     2,     2,     2,     2,     1,     6,     1,
     6,     1,     6,     0,     2,     1,     9,     2,     7,     3,
     7,     6,     2,     5,     3,     7,     6,     2,     3,     5,
     2,     5,     3,     3,     3,     5,     2,     2,     5,     3,
     3,     3,     5,     0,     6,     3,     3,     3,     2,     2,
     5,     7,     7,     2,     6,     3,     6,     2,     6,     0,
     2,     4,     6,     7,     7 };
static const yytabelem yychk[]={

-10000000,    -1,   -11,   -12,   -13,   -14,   -15,   256,   -16,   -17,
   -18,   -19,   261,   -20,    -4,   -21,    -3,   281,   270,   257,
   258,   259,   260,   -12,   -12,   262,   262,   262,   262,   262,
   262,   271,   256,   281,   271,   271,   271,   271,   -22,   256,
   271,   281,   281,   281,   281,   -23,   -25,   -26,   -27,   -28,
    -2,   -29,   -30,   -31,   -32,    -9,   -44,   -45,   265,   280,
   282,   277,   285,   278,   279,   281,   283,   284,   286,   263,
   -24,   276,   266,   -46,   -25,   256,    -2,   -33,   -35,   -43,
   267,   -36,   264,   -46,   -47,   -44,   263,   -23,    -2,   269,
   266,   256,   -34,   -37,   267,   -34,   281,   -34,   264,   256,
   264,   -48,   269,   -25,   -38,   268,   -44,   -39,   -40,    -5,
   263,    -7,   256,   281,   268,   -10,   272,   273,   274,   256,
   -42,   275,   -39,   281,   -37,   -41,    -6,   263,    -7,   280,
   256,   -37,    -8,   280,   256,   264,   -42,   -39,   -37,   -37,
    -8,   264 };
static const yytabelem yydef[]={

     0,    -2,    -2,     2,     4,     5,     6,     0,     8,     9,
    10,    11,    14,    15,    16,    23,    24,     0,     0,    19,
    21,    27,    29,     3,     7,    12,    13,    17,    18,    25,
    26,    -2,    34,     0,     0,     0,     0,     0,     0,    33,
    35,    20,    22,    28,    30,    32,    37,    38,    39,    40,
    41,    42,    43,    44,    45,    46,    88,    89,     0,    47,
    49,    79,    80,    81,    83,    85,    86,    87,    51,     0,
     0,     0,   101,     0,    93,    95,    41,    53,    53,    82,
     0,    53,    90,     0,    99,    97,     0,    36,   104,     0,
   102,   103,    48,    54,    55,    50,     0,    52,    91,    92,
    96,     0,   100,    94,     0,    84,    98,     0,    57,    59,
     0,    62,     0,    70,    56,     0,    72,    73,    74,     0,
     0,    76,     0,    71,    63,    58,    64,     0,    67,    68,
     0,    75,    60,    77,     0,    61,     0,     0,    69,    78,
    65,    66 };
typedef struct
#ifdef __cplusplus
	yytoktype
#endif
{ char *t_name; int t_val; } yytoktype;
#ifndef YYDEBUG
#	define YYDEBUG	0	/* don't allow debugging */
#endif

#if YYDEBUG

yytoktype yytoks[] =
{
	"_OBJECT",	257,
	"_END_OBJECT",	258,
	"_GROUP",	259,
	"_END_GROUP",	260,
	"_END",	261,
	"_semi",	262,
	"_sequence_opening",	263,
	"_sequence_closing",	264,
	"_set_opening",	265,
	"_set_closing",	266,
	"_units_opening",	267,
	"_units_closing",	268,
	"_list_separator",	269,
	"_point_operator",	270,
	"_assignment_operator",	271,
	"_multiply_operator",	272,
	"_alt_multiply_operator",	273,
	"_divide_operator",	274,
	"_exponentiate_operator",	275,
	"_range_operator",	276,
	"_date",	277,
	"_date_time",	278,
	"_date_timeV0",	279,
	"_integer",	280,
	"_name",	281,
	"_real",	282,
	"_symbol",	283,
	"_text_string",	284,
	"_time",	285,
	"_null_str",	286,
	"-unknown-",	-1	/* ends search */
};

char * yyreds[] =
{
	"-no such reduction-",
	"label : statement_list",
	"statement_list : statement",
	"statement_list : statement_list statement",
	"statement : aggregation_stmt",
	"statement : assignment_stmt",
	"statement : end_statement",
	"statement : error statement",
	"aggregation_stmt : object_stmt",
	"aggregation_stmt : group_stmt",
	"assignment_stmt : attribute_stmt",
	"assignment_stmt : pointer_stmt",
	"assignment_stmt : attribute_stmt _semi",
	"assignment_stmt : pointer_stmt _semi",
	"end_statement : _END",
	"object_stmt : object_opening",
	"object_stmt : object_closing",
	"object_stmt : object_opening _semi",
	"object_stmt : object_closing _semi",
	"object_opening : _OBJECT",
	"object_opening : _OBJECT _assignment_operator _name",
	"object_closing : _END_OBJECT",
	"object_closing : _END_OBJECT _assignment_operator _name",
	"group_stmt : group_opening",
	"group_stmt : group_closing",
	"group_stmt : group_opening _semi",
	"group_stmt : group_closing _semi",
	"group_opening : _GROUP",
	"group_opening : _GROUP _assignment_operator _name",
	"group_closing : _END_GROUP",
	"group_closing : _END_GROUP _assignment_operator _name",
	"attribute_stmt : _name _assignment_operator",
	"attribute_stmt : _name _assignment_operator value",
	"attribute_stmt : _name _assignment_operator error",
	"attribute_stmt : _name error",
	"pointer_stmt : _point_operator _name _assignment_operator",
	"pointer_stmt : _point_operator _name _assignment_operator value",
	"value : scalar_value",
	"value : sequence_value",
	"value : set_value",
	"value : range_value",
	"scalar_value : integer_value",
	"scalar_value : real_value",
	"scalar_value : date_time_value",
	"scalar_value : symbolic_value",
	"scalar_value : text_string_value",
	"scalar_value : null_value",
	"integer_value : _integer",
	"integer_value : _integer units_part",
	"real_value : _real",
	"real_value : _real units_part",
	"null_value : _null_str",
	"null_value : _null_str units_part",
	"units_part : /* empty */",
	"units_part : units_expression",
	"units_expression : _units_opening",
	"units_expression : _units_opening units_expr _units_closing",
	"units_expr : units_factor",
	"units_expr : units_expr units_mult_op x_units_factor",
	"units_factor : units",
	"units_factor : units units_exp_op units_exponent",
	"units_factor : _sequence_opening units_expr _sequence_closing",
	"units : name_sequence",
	"units : error units_expression",
	"x_units_factor : x_units",
	"x_units_factor : x_units units_exp_op units_exponent",
	"x_units_factor : _sequence_opening units_expr _sequence_closing",
	"x_units : name_sequence",
	"x_units : _integer",
	"x_units : error units_expression",
	"name_sequence : _name",
	"name_sequence : name_sequence _name",
	"units_mult_op : _multiply_operator",
	"units_mult_op : _alt_multiply_operator",
	"units_mult_op : _divide_operator",
	"units_mult_op : error units_expression",
	"units_exp_op : _exponentiate_operator",
	"units_exponent : _integer",
	"units_exponent : error units_expression",
	"date_time_value : _date",
	"date_time_value : _time",
	"date_time_value : _date_time",
	"date_time_value : _date_timeV0 time_zoneV0",
	"time_zoneV0 : /* empty */",
	"time_zoneV0 : _units_opening _name _units_closing",
	"symbolic_value : _name",
	"symbolic_value : _symbol",
	"text_string_value : _text_string",
	"sequence_value : sequence_1D",
	"sequence_value : sequence_2D",
	"sequence_1D : _sequence_opening _sequence_closing",
	"sequence_1D : _sequence_opening value_list _sequence_closing",
	"sequence_1D : _sequence_opening value_list error",
	"value_list : scalar_value",
	"value_list : value_list _list_separator scalar_value",
	"value_list : error",
	"sequence_2D : _sequence_opening sequence_1D_list _sequence_closing",
	"sequence_1D_list : sequence_1D",
	"sequence_1D_list : sequence_1D_list list_separator_opt sequence_1D",
	"list_separator_opt : /* empty */",
	"list_separator_opt : _list_separator",
	"set_value : _set_opening _set_closing",
	"set_value : _set_opening value_list _set_closing",
	"set_value : _set_opening value_list error",
	"range_value : integer_value _range_operator integer_value",
};
#endif /* YYDEBUG */
# line	1 "/usr/ccs/bin/yaccpar"
/*
 * Copyright (c) 1993 by Sun Microsystems, Inc.
 */

#pragma ident	"@(#)yaccpar	6.15	97/12/08 SMI"

/*
** Skeleton parser driver for yacc output
*/

/*
** yacc user known macros and defines
*/
#define YYERROR		goto yyerrlab
#define YYACCEPT	return(0)
#define YYABORT		return(1)
#define YYBACKUP( newtoken, newvalue )\
{\
	if ( yychar >= 0 || ( yyr2[ yytmp ] >> 1 ) != 1 )\
	{\
		yyerror( "syntax error - cannot backup" );\
		goto yyerrlab;\
	}\
	yychar = newtoken;\
	yystate = *yyps;\
	yylval = newvalue;\
	goto yynewstate;\
}
#define YYRECOVERING()	(!!yyerrflag)
#define YYNEW(type)	malloc(sizeof(type) * yynewmax)
#define YYCOPY(to, from, type) \
	(type *) memcpy(to, (char *) from, yymaxdepth * sizeof (type))
#define YYENLARGE( from, type) \
	(type *) realloc((char *) from, yynewmax * sizeof(type))
#ifndef YYDEBUG
#	define YYDEBUG	1	/* make debugging available */
#endif

/*
** user known globals
*/
int yydebug;			/* set to 1 to get debugging */

/*
** driver internal defines
*/
#define YYFLAG		(-10000000)

/*
** global variables used by the parser
*/
YYSTYPE *yypv;			/* top of value stack */
int *yyps;			/* top of state stack */

int yystate;			/* current state */
int yytmp;			/* extra var (lasts between blocks) */

int yynerrs;			/* number of errors */
int yyerrflag;			/* error recovery flag */
int yychar;			/* current input token number */



#ifdef YYNMBCHARS
#define YYLEX()		yycvtok(yylex())
/*
** yycvtok - return a token if i is a wchar_t value that exceeds 255.
**	If i<255, i itself is the token.  If i>255 but the neither 
**	of the 30th or 31st bit is on, i is already a token.
*/
#if defined(__STDC__) || defined(__cplusplus)
int yycvtok(int i)
#else
int yycvtok(i) int i;
#endif
{
	int first = 0;
	int last = YYNMBCHARS - 1;
	int mid;
	wchar_t j;

	if(i&0x60000000){/*Must convert to a token. */
		if( yymbchars[last].character < i ){
			return i;/*Giving up*/
		}
		while ((last>=first)&&(first>=0)) {/*Binary search loop*/
			mid = (first+last)/2;
			j = yymbchars[mid].character;
			if( j==i ){/*Found*/ 
				return yymbchars[mid].tvalue;
			}else if( j<i ){
				first = mid + 1;
			}else{
				last = mid -1;
			}
		}
		/*No entry in the table.*/
		return i;/* Giving up.*/
	}else{/* i is already a token. */
		return i;
	}
}
#else/*!YYNMBCHARS*/
#define YYLEX()		yylex()
#endif/*!YYNMBCHARS*/

/*
** yyparse - return 0 if worked, 1 if syntax error not recovered from
*/
#if defined(__STDC__) || defined(__cplusplus)
int yyparse(void)
#else
int yyparse()
#endif
{
	register YYSTYPE *yypvt = 0;	/* top of value stack for $vars */

#if defined(__cplusplus) || defined(lint)
/*
	hacks to please C++ and lint - goto's inside
	switch should never be executed
*/
	static int __yaccpar_lint_hack__ = 0;
	switch (__yaccpar_lint_hack__)
	{
		case 1: goto yyerrlab;
		case 2: goto yynewstate;
	}
#endif

	/*
	** Initialize externals - yyparse may be called more than once
	*/
	yypv = &yyv[-1];
	yyps = &yys[-1];
	yystate = 0;
	yytmp = 0;
	yynerrs = 0;
	yyerrflag = 0;
	yychar = -1;

#if YYMAXDEPTH <= 0
	if (yymaxdepth <= 0)
	{
		if ((yymaxdepth = YYEXPAND(0)) <= 0)
		{
			yyerror("yacc initialization error");
			YYABORT;
		}
	}
#endif

	{
		register YYSTYPE *yy_pv;	/* top of value stack */
		register int *yy_ps;		/* top of state stack */
		register int yy_state;		/* current state */
		register int  yy_n;		/* internal state number info */
	goto yystack;	/* moved from 6 lines above to here to please C++ */

		/*
		** get globals into registers.
		** branch to here only if YYBACKUP was called.
		*/
	yynewstate:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;
		goto yy_newstate;

		/*
		** get globals into registers.
		** either we just started, or we just finished a reduction
		*/
	yystack:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;

		/*
		** top of for (;;) loop while no reductions done
		*/
	yy_stack:
		/*
		** put a state and value onto the stacks
		*/
#if YYDEBUG
		/*
		** if debugging, look up token value in list of value vs.
		** name pairs.  0 and negative (-1) are special values.
		** Note: linear search is used since time is not a real
		** consideration while debugging.
		*/
		if ( yydebug )
		{
			register int yy_i;

			printf( "State %d, token ", yy_state );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ++yy_ps >= &yys[ yymaxdepth ] )	/* room on stack? */
		{
			/*
			** reallocate and recover.  Note that pointers
			** have to be reset, or bad things will happen
			*/
			long yyps_index = (yy_ps - yys);
			long yypv_index = (yy_pv - yyv);
			long yypvt_index = (yypvt - yyv);
			int yynewmax;
#ifdef YYEXPAND
			yynewmax = YYEXPAND(yymaxdepth);
#else
			yynewmax = 2 * yymaxdepth;	/* double table size */
			if (yymaxdepth == YYMAXDEPTH)	/* first time growth */
			{
				char *newyys = (char *)YYNEW(int);
				char *newyyv = (char *)YYNEW(YYSTYPE);
				if (newyys != 0 && newyyv != 0)
				{
					yys = YYCOPY(newyys, yys, int);
					yyv = YYCOPY(newyyv, yyv, YYSTYPE);
				}
				else
					yynewmax = 0;	/* failed */
			}
			else				/* not first time */
			{
				yys = YYENLARGE(yys, int);
				yyv = YYENLARGE(yyv, YYSTYPE);
				if (yys == 0 || yyv == 0)
					yynewmax = 0;	/* failed */
			}
#endif
			if (yynewmax <= yymaxdepth)	/* tables not expanded */
			{
				yyerror( "yacc stack overflow" );
				YYABORT;
			}
			yymaxdepth = yynewmax;

			yy_ps = yys + yyps_index;
			yy_pv = yyv + yypv_index;
			yypvt = yyv + yypvt_index;
		}
		*yy_ps = yy_state;
		*++yy_pv = yyval;

		/*
		** we have a new state - find out what to do
		*/
	yy_newstate:
		if ( ( yy_n = yypact[ yy_state ] ) <= YYFLAG )
			goto yydefault;		/* simple state */
#if YYDEBUG
		/*
		** if debugging, need to mark whether new token grabbed
		*/
		yytmp = yychar < 0;
#endif
		if ( ( yychar < 0 ) && ( ( yychar = YYLEX() ) < 0 ) )
			yychar = 0;		/* reached EOF */
#if YYDEBUG
		if ( yydebug && yytmp )
		{
			register int yy_i;

			printf( "Received token " );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ( ( yy_n += yychar ) < 0 ) || ( yy_n >= YYLAST ) )
			goto yydefault;
		if ( yychk[ yy_n = yyact[ yy_n ] ] == yychar )	/*valid shift*/
		{
			yychar = -1;
			yyval = yylval;
			yy_state = yy_n;
			if ( yyerrflag > 0 )
				yyerrflag--;
			goto yy_stack;
		}

	yydefault:
		if ( ( yy_n = yydef[ yy_state ] ) == -2 )
		{
#if YYDEBUG
			yytmp = yychar < 0;
#endif
			if ( ( yychar < 0 ) && ( ( yychar = YYLEX() ) < 0 ) )
				yychar = 0;		/* reached EOF */
#if YYDEBUG
			if ( yydebug && yytmp )
			{
				register int yy_i;

				printf( "Received token " );
				if ( yychar == 0 )
					printf( "end-of-file\n" );
				else if ( yychar < 0 )
					printf( "-none-\n" );
				else
				{
					for ( yy_i = 0;
						yytoks[yy_i].t_val >= 0;
						yy_i++ )
					{
						if ( yytoks[yy_i].t_val
							== yychar )
						{
							break;
						}
					}
					printf( "%s\n", yytoks[yy_i].t_name );
				}
			}
#endif /* YYDEBUG */
			/*
			** look through exception table
			*/
			{
				register const int *yyxi = yyexca;

				while ( ( *yyxi != -1 ) ||
					( yyxi[1] != yy_state ) )
				{
					yyxi += 2;
				}
				while ( ( *(yyxi += 2) >= 0 ) &&
					( *yyxi != yychar ) )
					;
				if ( ( yy_n = yyxi[1] ) < 0 )
					YYACCEPT;
			}
		}

		/*
		** check for syntax error
		*/
		if ( yy_n == 0 )	/* have an error */
		{
			/* no worry about speed here! */
			switch ( yyerrflag )
			{
			case 0:		/* new error */
				yyerror( "syntax error" );
				goto skip_init;
			yyerrlab:
				/*
				** get globals into registers.
				** we have a user generated syntax type error
				*/
				yy_pv = yypv;
				yy_ps = yyps;
				yy_state = yystate;
			skip_init:
				yynerrs++;
				/* FALLTHRU */
			case 1:
			case 2:		/* incompletely recovered error */
					/* try again... */
				yyerrflag = 3;
				/*
				** find state where "error" is a legal
				** shift action
				*/
				while ( yy_ps >= yys )
				{
					yy_n = yypact[ *yy_ps ] + YYERRCODE;
					if ( yy_n >= 0 && yy_n < YYLAST &&
						yychk[yyact[yy_n]] == YYERRCODE)					{
						/*
						** simulate shift of "error"
						*/
						yy_state = yyact[ yy_n ];
						goto yy_stack;
					}
					/*
					** current state has no shift on
					** "error", pop stack
					*/
#if YYDEBUG
#	define _POP_ "Error recovery pops state %d, uncovers state %d\n"
					if ( yydebug )
						printf( _POP_, *yy_ps,
							yy_ps[-1] );
#	undef _POP_
#endif
					yy_ps--;
					yy_pv--;
				}
				/*
				** there is no state on stack with "error" as
				** a valid shift.  give up.
				*/
				YYABORT;
			case 3:		/* no shift yet; eat a token */
#if YYDEBUG
				/*
				** if debugging, look up token in list of
				** pairs.  0 and negative shouldn't occur,
				** but since timing doesn't matter when
				** debugging, it doesn't hurt to leave the
				** tests here.
				*/
				if ( yydebug )
				{
					register int yy_i;

					printf( "Error recovery discards " );
					if ( yychar == 0 )
						printf( "token end-of-file\n" );
					else if ( yychar < 0 )
						printf( "token -none-\n" );
					else
					{
						for ( yy_i = 0;
							yytoks[yy_i].t_val >= 0;
							yy_i++ )
						{
							if ( yytoks[yy_i].t_val
								== yychar )
							{
								break;
							}
						}
						printf( "token %s\n",
							yytoks[yy_i].t_name );
					}
				}
#endif /* YYDEBUG */
				if ( yychar == 0 )	/* reached EOF. quit */
					YYABORT;
				yychar = -1;
				goto yy_newstate;
			}
		}/* end if ( yy_n == 0 ) */
		/*
		** reduction by production yy_n
		** put stack tops, etc. so things right after switch
		*/
#if YYDEBUG
		/*
		** if debugging, print the string that is the user's
		** specification of the reduction which is just about
		** to be done.
		*/
		if ( yydebug )
			printf( "Reduce by (%d) \"%s\"\n",
				yy_n, yyreds[ yy_n ] );
#endif
		yytmp = yy_n;			/* value to switch over */
		yypvt = yy_pv;			/* $vars top of value stack */
		/*
		** Look in goto table for next state
		** Sorry about using yy_state here as temporary
		** register variable, but why not, if it works...
		** If yyr2[ yy_n ] doesn't have the low order bit
		** set, then there is no action to be done for
		** this reduction.  So, no saving & unsaving of
		** registers done.  The only difference between the
		** code just after the if and the body of the if is
		** the goto yy_stack in the body.  This way the test
		** can be made before the choice of what to do is needed.
		*/
		{
			/* length of production doubled with extra bit */
			register int yy_len = yyr2[ yy_n ];

			if ( !( yy_len & 01 ) )
			{
				yy_len >>= 1;
				yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
				yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
					*( yy_ps -= yy_len ) + 1;
				if ( yy_state >= YYLAST ||
					yychk[ yy_state =
					yyact[ yy_state ] ] != -yy_n )
				{
					yy_state = yyact[ yypgo[ yy_n ] ];
				}
				goto yy_stack;
			}
			yy_len >>= 1;
			yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
			yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
				*( yy_ps -= yy_len ) + 1;
			if ( yy_state >= YYLAST ||
				yychk[ yy_state = yyact[ yy_state ] ] != -yy_n )
			{
				yy_state = yyact[ yypgo[ yy_n ] ];
			}
		}
					/* save until reenter driver code */
		yystate = yy_state;
		yyps = yy_ps;
		yypv = yy_pv;
	}
	/*
	** code supplied by user is placed in this switch
	*/
	switch( yytmp )
	{
		
case 1:

{
                            /* End-of-file hit before END statement found */
                            ODLEndLabel ();
                            if (pds_watch_ends)
                            {
                               yyerror ("END statement is missing");
                               YYABORT;
			    }
                            else
                               YYACCEPT;
                          } break;
case 14:

{
                            /* This is the normal termination of parsing */
                            if (ODLEndLabel ())
                              {
                                YYACCEPT;
                              }
                            else
                              {
                                YYABORT;
                              }
                          } break;
case 19:

{ yyerror ("Missing '=' operator after OBJECT"); } break;
case 20:

{ ODLBeginAggregate (KA_OBJECT, &yypvt[-0].item); } break;
case 21:

{ 
                             yyval.item.value.string = NULL;
                             ODLEndAggregate (KA_OBJECT, &yyval.item);
                           } break;
case 22:

{ ODLEndAggregate (KA_OBJECT, &yypvt[-0].item); } break;
case 27:

{ yyerror ("Missing '=' operator after GROUP"); } break;
case 28:

{ ODLBeginAggregate (KA_GROUP, &yypvt[-0].item); } break;
case 29:

{ 
                             yyval.item.value.string = NULL;
                             ODLEndAggregate (KA_GROUP, &yyval.item);
                           } break;
case 30:

{ ODLEndAggregate (KA_GROUP, &yypvt[-0].item); } break;
case 31:

{ ODLBeginParameter (KP_ATTRIBUTE, &yypvt[-1].item); } break;
case 33:

{ 
                             yyerror ("Bad value in assignment statement");
                             yyclearin;
                           } break;
case 34:

{ 
                              yyerror ("Expected '=' after name");
                              free (yypvt[-1].item.value.string);
                           } break;
case 35:

{ ODLBeginParameter (KP_POINTER, &yypvt[-1].item); } break;
case 37:

{ ODLMarkParameter (KV_SCALAR); } break;
case 38:

{ ODLMarkParameter (KV_SEQUENCE); } break;
case 39:

{ ODLMarkParameter (KV_SET); } break;
case 40:

{ ODLMarkParameter (KV_SEQUENCE); } break;
case 47:

{ ODLStoreValue (&yypvt[-0].item); } break;
case 49:

{ ODLStoreValue (&yypvt[-0].item); } break;
case 51:

{ ODLStoreValue (&yypvt[-0].item); } break;
case 55:

{ in_units = 1; } break;
case 56:

{ in_units = 0; } break;
case 58:

{ ODLMarkUnits (yypvt[-1].flag); } break;
case 59:

{ ODLStoreUnits1 (&yypvt[-0].item); } break;
case 60:

{ ODLStoreUnits2 (&yypvt[-2].item, &yypvt[-0].item); } break;
case 63:

{ yyerror ("Units designator must be a name (alphanumeric)"); } break;
case 64:

{ ODLStoreUnits1 (&yypvt[-0].item); } break;
case 65:

{ ODLStoreUnits2 (&yypvt[-2].item, &yypvt[-0].item); } break;
case 68:

{
                            /* We must convert the integer value to
                               a string value. Our temporary buffer
                               must be big enough to hold the largest
                               possible integer. */

                            char value[20];
                            sprintf(value, "%ld", yypvt[-0].item.value.integer.number);
                            yyval.item = ODLConvertSymbol(value, strlen(value), 1);
                           } break;
case 69:

{ yyerror ("Units designator must be a name (alphanumeric) or integer"); } break;
case 71:

{
                            /* Concatenate the names together, separated by
                               a space, up to a maximum of 1000 characters. */
                            char temp[1000+1];
                            strncpy(temp, yypvt[-1].item.value.string, sizeof(temp)-1);
                            temp[sizeof(temp)-1] = '\0';
                            strncat(temp, " ", sizeof(temp)-1-strlen(temp));
                            strncat(temp, yypvt[-0].item.value.string,
                                    sizeof(temp)-1-strlen(temp));
                            yyval.item = ODLConvertSymbol(temp, strlen(temp), 1);
                           } break;
case 72:

{ yyval.flag = 1; } break;
case 73:

{ yyval.flag = 1; } break;
case 74:

{ yyval.flag = -1; } break;
case 75:

{ yyerror ("Expected a '*', '/' or '**' operator");} break;
case 78:

{ yyerror("Exponent in units expression must be integer");} break;
case 79:

{ ODLStoreValue (&yypvt[-0].item); } break;
case 80:

{ ODLStoreValue (&yypvt[-0].item); } break;
case 81:

{ ODLStoreValue (&yypvt[-0].item); } break;
case 82:

{ ODLStoreValue (&yypvt[-1].item); } break;
case 85:

{ ODLStoreValue (&yypvt[-0].item); } break;
case 86:

{ ODLStoreValue (&yypvt[-0].item); } break;
case 87:

{ ODLStoreValue (&yypvt[-0].item); } break;
case 90:

{ yyerror("Sequences with no values not allowed"); } break;
case 91:

{ ODLCheckSequence (); } break;
case 92:

{ 
                             yyerror("')' at end of a sequence is missing");
                             ODLCheckSequence ();
                           } break;
case 95:

{ yyerror ("Error in value list"); } break;
case 103:

{ yyerror ("The '}' is missing at end of set"); } break;
case 104:

{ ODLCheckRange (&yypvt[-2].item, &yypvt[-0].item); } break;

	}
	goto yystack;		/* reset registers in driver code */
}

