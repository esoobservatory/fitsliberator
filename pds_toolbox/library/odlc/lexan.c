#include <stdio.h>
#include <stdlib.h>

#ifndef MSDOS_TC
#include <inttypes.h>
#else
#ifdef MSDOS_TC
#include <stddef.h>
#endif
#endif

# define U(x) x
# define NLSTATE yyprevious=YYNEWLINE
# define BEGIN yybgin = yysvec + 1 +
# define INITIAL 0
# define YYLERR yysvec
# define YYSTATE (yyestate-yysvec-1)
# define YYOPTIM 1
# ifndef YYLMAX 
# define YYLMAX BUFSIZ
# endif 
#ifndef __cplusplus
# define output(c) (void)putc(c,yyout)
#else
# define lex_output(c) (void)putc(c,yyout)
#endif

#if defined(__cplusplus) || defined(__STDC__)

#if defined(__cplusplus) && defined(__EXTERN_C__)
extern "C" {
#endif
	int yyback(int *, int);
	int yyinput(void);
	int yylook(void);
	void yyoutput(int);
	int yyracc(int);
	int yyreject(void);
	void yyunput(int);
	int yylex(void);
#ifdef YYLEX_E
	void yywoutput(wchar_t);
	wchar_t yywinput(void);
#endif
#ifndef yyless
	int yyless(int);
#endif
#ifndef yywrap
	int yywrap(void);
#endif
#ifdef LEXDEBUG
	void allprint(char);
	void sprint(char *);
#endif
#if defined(__cplusplus) && defined(__EXTERN_C__)
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
	void exit(int);
#ifdef __cplusplus
}
#endif

#endif
# define unput(c) {yytchar= (c);if(yytchar=='\n')yylineno--;*yysptr++=yytchar;}
# define yymore() (yymorfg=1)
#ifndef __cplusplus
# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
#else
# define lex_input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
#endif
#define ECHO fprintf(yyout, "%s",yytext)
# define REJECT { nstr = yyreject(); goto yyfussy;}
int yyleng;
#define YYISARRAY
char yytext[YYLMAX];
int yymorfg;
extern char *yysptr, yysbuf[];
int yytchar;
FILE *yyin, *yyout;
extern int yylineno;
struct yysvf { 
	struct yywork *yystoff;
	struct yysvf *yyother;
	int *yystops;};
struct yysvf *yyestate;
extern struct yysvf yysvec[], *yybgin;



/*****************************************************************************

 Description: This file contains the lexical analyzer for the Object
              Description Language. The lexical analyzer is created
              using Lex and modifications to the lexical analysis of
              ODL should be made by modifying the Lex input file and
              not the C-language version of the analyzer produced by
              Lex.

 Author:  Randy Davis, University of Colorado LASP

 Creation Date: 17 April 1990
 Last Modified: 18 May 1991
 
 History:

   Creation - This routine was introduced in Version 1 of the ODLC library.

   Version 2.0 - 30 August 1990 - R. Davis, U. of Colorado LASP
     a) Upgraded to ODL Version 2.  The biggest change is the support
        of groups.  Support for ODL Version 1 features not found in
        Version 2 -- like the range operator -- remain so that the
        lexical analyzer can handle older labels as well as new ones.
     b) Added support for ODL Version 0 date and time format. 

   Version 2.1 - 13 March 1991 - R. Davis, U. of Colorado LASP
     a) Changed to a more general way to turn tokens into 'value data'
        structures by converting from the older ODLxToken routines to
        ODLConvertX routines.  Processing of string tokens was moved into
        a new action routine named yyGetStringToken.
     b) Added recognition of two values that are often entered incorrectly
        by users: file names without quotation marks (which are turned into
        strings); and the symbol N/A (for Not Applicable, which is turned 
        into a quoted symbol).
     c) Saved comments so they can be attached as annotation to the ODL tree.

  Version 2.2 - 18 May 1991 - M. DeMore, Jet Propulsion Laboratory
    Removed ODL function prototypes which are now in include files.
    Added include file odlinter.h.

  Version 2.3 - 13 October 1991 - M. DeMore, Jet Propulsion Laboratory
     Removed code in yyGetStringToken which used to process '\t' and
     `\n`.  They are now transferred exactly as is to the output string
     and are handled by the output routines instead.  This was done to
     prevent the lexer from eating backslashes in DOS file names.

  Version 2.4 - 8 July 1992 - M. DeMore, Jet Propulsion Laboratory
     Updated definition of name token to allow individual words to
     begin with letters.

  Version 2.5 - 17 Oct 2003 - D. Schultz, Jet Propulstion Laboratory
     Added the null_str token.  The recognized NULL patterns are -
     "NULL", 'NULL', NULL  
     The search order for these patterns are before a ' or " 
     so they are not interpreted as a name or symbol

******************************************************************************/



/*****************************************************************************
 To allow the use of the resulting file on multiple platforms, the following
 changes should be performed on the resulting C file after its generation
 by the Sun Version of Lex:

    1) Change the line
   
          # include "stdio.h" 

                to
  
          # include <stdio.h>

    2) Change the line
          
          # define YYTYPE int

                 to
          # define YYTYPE unsigned char

    3) Change the lines
          
          yyback(p, m)
              int *p;

              to
  
          yyback (p, m)
              int *p;
              int m;

    4) Change the line

          FILE *yyin = {stdin}, *yyout = {stdout};

               to

          FILE *yyin, *yyout;

*****************************************************************************/

#include "odldef.h"
#include "odlparse.h"
#include "odlinter.h"

#include  <ctype.h>



/* This was moved from rdvalue.c so that other modules could be independent
   of rdvalue */

int nc;



/* The following are for the dynamic string allocation routines */

# define ODL_BUFF_INCREMENT BUFSIZ

int ODL_buf_pos = 0;
long ODL_buf_size = 0;
char *temp_buf = NULL;
char *ODL_buffer = NULL;




/* The following are warning messages */

#define MESSAGE1 "Value is assumed to be a file name -- converted to a string"
#define MESSAGE2 "Value N/A is not a name -- will appear within single quotes"
#define MESSAGE3 "BEGIN_GROUP statement found.  Will be converted to GROUP"
#define MESSAGE4 "BEGIN_OBJECT statement found.  Will be converted to OBJECT"
#define MESSAGE5 "Numbers in exponential notation require a decimal point"



/* The following global variable is defined in the module containing the
   parser action routines */



/* The following global variable is defined in the module containing the
   parser action routines */

extern char       *ODLcurrent_comment;   /* Most recently recognized comment */



/* The following routine processes string tokens.  This routine is used
   because ODL strings can be very long, rendering LEX's regular
   expression mechanism for getting tokens inefficient */

char *yyGetStringToken ();



/* The following routine processes comments within an ODL label */

void  yyGetComment ();


# define not_in_units 2
# define YYNEWLINE 10
yylex(){
int nstr; extern int yyprevious;
#ifdef __cplusplus
/* to avoid CC and lint complaining yyfussy not being used ...*/
static int __lex_hack = 0;
if (__lex_hack) goto yyfussy;
#endif
while((nstr = yylook()) >= 0)
yyfussy: switch(nstr){
case 0:
if(yywrap()) return(0); break;
case 1:


                             {
                      
               return (_END);
                      
                                    }
break;
case 2:


                    {
                                     return (_END_GROUP);
                                    }
break;
case 3:


                   {
                                     return (_END_OBJECT);
                                    }
break;
case 4:


                            {
                                     return (_GROUP);
                                    }
break;
case 5:


                  {
                                     ODLPrintWarning(MESSAGE3);
                                     return (_GROUP);
                                    }
break;
case 6:


                           {
                                     return (_OBJECT);
                                    }
break;
case 7:


                 {
                                     ODLPrintWarning(MESSAGE4);
                                     return (_OBJECT);
                                    }
break;
case 8:


                           {
                                     yylval.item = 
                                        ODLConvertNull ("NULL", 4);
                                        return(_null_str);
                                    }
break;
case 9:


                           {
                                     yylval.item = 
                                        ODLConvertNull ("NULL", 4);
                                        return(_null_str);
                                    }
break;
case 10:


                           {
                                     yylval.item = 
                                        ODLConvertNull ("NULL", 4);
                                        return(_null_str);
                                    }
break;
case 11:


{
                                     yylval.item =
                                        ODLConvertSymbol (yytext, yyleng, 1);
                                     return (_name);
                                    }
break;
case 12:


                          {
                                     yylval.item =
                                       ODLConvertSymbol(&yytext[1],yyleng-2,2);
                                     return (_symbol);
                                    }
break;
case 13:


                                 {
                                     temp_buf = yyGetStringToken ();
                                     yylval.item = 
                                        ODLConvertString (temp_buf, 
                                              ODLStringLength ());
                                     ODLKillString ();
                                     return (_text_string);
                                    }
break;
case 14:


          {
                                       /* This is the low part of an ODL
                                          Version 1 range value */
                                     yylval.item =
                                        ODLConvertInteger (yytext, yyleng);
                                     return (_integer);
                                    }
break;
case 15:


               {
                                     yylval.item =
                                        ODLConvertInteger (yytext, yyleng);
                                     return (_integer);
                                    }
break;
case 16:


{
                                     yylval.item =
                                        ODLConvertInteger (yytext, yyleng);
                                     return (_integer);
                                    }
break;
case 17:


 {
                                     yylval.item = 
                                        ODLConvertReal (yytext, yyleng);
                                     return (_real);
                                    }
break;
case 18:


 {
				     /* Insert the decimal point, if there is
					room. Otherwise replace with "1.0".
					We assume that yytext[] is at least
					1000 characters long (including room
					for the '\0' at the end. */

                                     int exponentPos = strcspn(yytext, "Ee");
				     if (yyleng+1 >= 1000) {
					 strcpy(yytext, "1.0");
					 yyleng = 3;
				     } else {
					 /* Must use memmove() since the source
					    and destination overlap. */
					 memmove(&yytext[exponentPos+1],
						 &yytext[exponentPos],
						 yyleng-exponentPos);
					 yytext[exponentPos] = '.';
				         yytext[++yyleng] = '\0';
				     }

                                     ODLPrintError(MESSAGE5);
                                     yylval.item = 
                                        ODLConvertReal (yytext, yyleng);
                                     return (_real);
                                    }
break;
case 19:


                             {
                                     yylval.item =
                                        ODLConvertDate (yytext, yyleng);
                                     return (_date);
                                    }
break;
case 20:


                           {
                                     yylval.item =
                                        ODLConvertDate (yytext, yyleng);
                                     return (_date);
                                    }
break;
case 21:


                    {
                                     yylval.item =
                                        ODLConvertTime (yytext, yyleng);
                                     return (_time);
                                    }
break;
case 22:


          {
                                     yylval.item =
                                        ODLConvertDateTime (yytext, yyleng);
                                     return (_date_time);
                                    }
break;
case 23:


                  {
                                     yylval.item =
                                        ODLConvertDateTime (yytext, yyleng);
                                     return (_date_timeV0);
                                    }
break;
case 24:


{
                                     ODLPrintWarning (MESSAGE1);
                                     yylval.item =
                                        ODLConvertString (yytext, yyleng);
                                     return (_text_string);
                                    }
break;
case 25:


                        {
                                     ODLPrintWarning (MESSAGE2);
                                     yylval.item =
                                        ODLConvertSymbol (yytext, yyleng, 2);
                                     return (_symbol);
                                    }
break;
case 26:


                                {
                                     return (_semi);
                                    }
break;
case 27:


                                {
                                     return (_sequence_opening);
                                    }
break;
case 28:


                                {
                                     return (_sequence_closing);
                                    }
break;
case 29:


                                {
                                     return (_set_opening);
                                    }
break;
case 30:


                                {
                                     return (_set_closing);
                                    }
break;
case 31:


                                {
                                     return (_units_opening);
                                    }
break;
case 32:


                                {
                                     return (_units_closing);
                                    }
break;
case 33:


                                {
                                     return (_list_separator);
                                    }
break;
case 34:


                                {
                                     return (_point_operator);
                                    }
break;
case 35:


                                {
                                     return (_assignment_operator);
                                    }
break;
case 36:


                                {
                                     return (_multiply_operator);
                                    }
break;
case 37:


                                {
                                     return (_alt_multiply_operator);
                                    }
break;
case 38:


                                {
                                     return (_divide_operator);
                                    }
break;
case 39:


                               {
                                     return (_exponentiate_operator);
                                    }
break;
case 40:


                               {
                                     return (_range_operator);
                                    }
break;
case 41:


               {
                                     /* This is a comment line */

                                     yyGetComment ();
                                    }
break;
case 42:


                             {
                                     /* This is a comment at the end of a
                                        line of ODL code -- ignore it */
                                    }
break;
case 43:


                       {}
break;
case 44:


                        {}
break;
case 45:


                                  { /* Return other characters verbatim */
                                     return (yytext[0]);
                                    }
break;
case -1:
break;
default:
(void)fprintf(yyout,"bad switch yylook %d",nstr);
} return(0); }
/* end of yylex */



/*****************************************************************************

  Routine: yylex_lvtool

  Description:  A special wrapper to yylex() that takes an argument indicating
                whether we're inside a units expression. If so, a "." is never
                part of a name or value.

  Input:        in_units==1 => we're inside a units expression

  Output:       The next token

*****************************************************************************/

int yylex_lvtool(int in_units) {
    if (in_units) {
        BEGIN 0;
    } else {
        BEGIN not_in_units;
    }

    return yylex();
}

/*****************************************************************************

  Routine: yywrap

  Description:  Required wrap-up routine for lexical processing.  No
                special wrap-up is required for ODL parsing.

  Input:  None.

  Output: Return value is set to TRUE to indicate parsing completed OK.

*****************************************************************************/

yywrap ()
{
 return(1);
}


/*****************************************************************************

  Routine: yyGetStringToken

  Description:  Get a text string token.  The opening delimiter (") was
                recognized by the lexical analyzer before this routine
                was called.  This routine will read in the remainder of
                the string token up to the end delimiter and it will
                reformat the text string as it goes into an ODL string
                value.

  Input:  No arguments required.  The text of the string token is gotten
          using the lexical analyzer input function (yyinput).

  Output: The text string is placed in the lexical analyzer's token buffer
          (pointed to by yytext) and the token character count (yyleng) is
          set to reflect the string length.

  MDD - October 22, 1991
        Modified to use dynamic memory allocation routines rather than
        a static array.

*****************************************************************************/

char *yyGetStringToken ()
{
  char    c;                       /* Current input character               */
  int     newline;                 /* New line flag:                        */
                                   /*   0 - Not at the start of a line      */
                                   /*  +1 - Newline found in input string   */
                                   /*  -1 - Newline placed in output string */

  newline = 1;

  ODLNewString();

  while ((c = yyinput()) != '"' && c != '\0')
   {
    switch (c)
     {
      case '\n':

       /* End of current line : get rid of any trailing blanks or CRs on the line */

       while (ODLPeekString (1) == ' ' || ODLPeekString (1) == '\r')
              ODLBackupString ();

       /* If the last non-blank character on the line is a '-', then this
          is a word hyphen and we can delete it. If it is an '&' then this
          indicates that all characters to the left are to be left intact,
          although we do delete the '&'.  Otherwise we add a blank to
          separate the last word on the current input line from the first
          word on the next input line. If there are two or more newlines
          in a row, then we retain the newlines to separate paragraphs */

       if (newline > 0)
       {
           if (ODLPeekString (1) &&
                  !(ODLPeekString (1) == 'n' && ODLPeekString (2) == '\\'))
           {
              ODLStoreString ('\\');
              ODLStoreString ('n');
           }
           ODLStoreString ('\\');
           ODLStoreString ('n');
       }
       else if (newline < 0)
       {
          newline = 1;
       }
       else
       {
           newline = 1;
           if (ODLPeekString (1) == '-' || ODLPeekString (1) == '&')
           {
              ODLBackupString ();
           }
           else if (ODLPeekString (1) &&
                    !(ODLPeekString (1) == 'n' && ODLPeekString (2) == '\\'))
           {
              ODLStoreString (' ');
           }
       }
       break;

      case ' ':  case '\t': case '\r':

       /* Ignore a blank or tab at the beginning of an input line; otherwise
          copy a blank character into the output string */

       if (newline == 0)
       {
         ODLStoreString (' ');
       }
       break;

      case '\\':
         ODLStoreString (c);
         ODLStoreString (yyinput ());
         if (ODLPeekString (1) == 'n' && ODLPeekString (2) == '\\')
            newline = -1;
         else if (ODLPeekString (1) &&
              !(ODLPeekString (1) == 't' && ODLPeekString (2) == '\\'))
            newline = 0;
         break;

      default:

       /* Copy the input character to the token buffer */
       ODLStoreString (c);
       newline = 0;

     }
   }

  /* Terminate the token buffer */

  return (ODLStoreString ('\0'));
}


/*****************************************************************************

  Routine: yyGetComment

  Description:  Get a comment and either attach it to the ODL tree or
                put it where other routines can get at it and do so.

  Input:  No arguments required.  The text and length of the comment
          come from the global variables yyinput and yyleng.

  Output: No output parameters.  The comment is copied and saved for
          later processing.

*****************************************************************************/


void yyGetComment ()
{
  int   ib;                   /* Index to first character in comment text   */
  int   ie;                   /* Index to last character in comment text    */
  int   il;                   /* Count of characters in comment text        */
  char *string;               /* Pointer to space allocated for comment     */
  int temp_end;

  /* Skip over any whitespace prior to the start of the comment */

  ib = 0;
  ie = yyleng - 1;

  for ( ; ib <= ie && isspace (yytext[ib]) ; ib++);

  /* Skip over the slash and asterisk that introduce the comment */

  ib += 2;

  /* Eliminate any trailing whitespace */

  for ( ; ie >= ib && isspace (yytext[ie]); ie--);

  /* Eliminate any trailing characters */

  for (temp_end = ie; temp_end > ib && yytext[temp_end] != '/'; temp_end--);
  if (yytext [temp_end] == '/' && yytext [temp_end - 1] == '*')
  {
     if (temp_end != ie)
     {
        ie = temp_end;
        ODLPrintWarning ("Characters found on line after comment ignored");
     }
  }

  /* Skip backward over ending comment delimiter */

  if (ie > ib && yytext[ie] == '/')
    {
      if (yytext[ie-1] == '*')
        {
          ie -= 2;
        }
    }

  /* Eliminate any trailing whitespace */

/*  for ( ; ie >= ib && isspace (yytext[ie]); ie--); */

  /* Get the number of characters in the comment string */

  yytext[ie+1] = '\0';
  il = (ie >= ib)? ie-ib+1 : 0;

  if (ODLcurrent_comment == NULL)
    {
      /* There is no comment currently.  Allocate space for a new
         comment and copy the text */

      string = (char *) malloc (il+1);
      if (string != NULL)
        {
          ODLcurrent_comment = strcpy (string, &yytext[ib]);
        }
    }
  else
    {
      /* There is already some comment there: put in a newline character
         to end the previous comment line and append the current text
         to the comment */

      string = (char *) realloc (ODLcurrent_comment,
                                 strlen (ODLcurrent_comment)+il+2);
      if (string != NULL)
        {
          strcat (string, "\n");
          ODLcurrent_comment = strcat (string, &yytext[ib]);
        }
    }

return;
}


char *ODLStoreString (c)
char c;
{
   if (ODL_buffer != NULL)
   {
      if (ODL_buf_pos < ODL_buf_size)
         *(ODL_buffer + ODL_buf_pos++) = c;
      else
      {
         ODL_buf_size = ODL_buf_size + ODL_BUFF_INCREMENT;
         ODL_buffer = (char *) realloc (ODL_buffer, ODL_buf_size);
         if (ODL_buffer == NULL)
         {
            printf ("Out of memory for string storage.");
            exit (1);
         }
         *(ODL_buffer + ODL_buf_pos++) = c;
      }
   }
   return (ODL_buffer);
}

char *ODLNewString ()
{
   ODLKillString ();
   ODL_buffer = (char *) malloc (ODL_BUFF_INCREMENT);
   if (ODL_buffer == NULL)
   {
      printf ("Out of memory for string storage.");
      exit (1);
   }
   ODL_buf_size = ODL_BUFF_INCREMENT;
}

char ODLBackupString ()
{
   if (ODL_buf_pos > 0)
   {
       ODL_buf_pos--;
       return (*(ODL_buffer + ODL_buf_pos));
   }
   else
      return (0);
}

char ODLPeekString (pos)
int pos;
{
   if (pos != 0 && ODL_buffer != NULL && pos <= ODL_buf_pos)
      return (*(ODL_buffer + (ODL_buf_pos - pos)));
   else
      return (0);
}

void ODLKillString ()
{
   if (ODL_buffer != NULL) free (ODL_buffer);
   ODL_buffer = NULL;
   ODL_buf_pos = 0;
   ODL_buf_size = 0;
}

int ODLStringLength ()
{
   return (ODL_buf_pos - 1);
}
int yyvstop[] = {
0,

45,
0, 

43,
45,
0, 

44,
0, 

44,
45,
0, 

13,
45,
0, 

45,
0, 

27,
45,
0, 

28,
45,
0, 

36,
45,
0, 

45,
0, 

33,
45,
0, 

37,
45,
0, 

38,
45,
0, 

15,
45,
-14,
0, 

26,
45,
0, 

31,
45,
0, 

35,
45,
0, 

32,
45,
0, 

11,
45,
0, 

11,
45,
0, 

11,
45,
0, 

11,
45,
0, 

11,
45,
0, 

11,
45,
0, 

34,
45,
0, 

11,
45,
0, 

29,
45,
0, 

30,
45,
0, 

43,
45,
0, 

38,
45,
0, 

45,
0, 

37,
45,
0, 

15,
45,
-14,
0, 

11,
45,
0, 

11,
45,
0, 

11,
45,
0, 

11,
45,
0, 

11,
45,
0, 

11,
45,
0, 

11,
45,
0, 

12,
0, 

39,
0, 

15,
-14,
0, 

40,
0, 

42,
0, 

15,
-14,
0, 

11,
0, 

11,
0, 

11,
0, 

11,
0, 

11,
0, 

11,
0, 

41,
42,
0, 

15,
-14,
0, 

17,
0, 

17,
0, 

15,
-14,
0, 

24,
0, 

11,
0, 

11,
0, 

11,
0, 

11,
0, 

11,
0, 

11,
0, 

19,
0, 

14,
0, 

20,
0, 

21,
0, 

11,
0, 

11,
0, 

1,
11,
0, 

11,
0, 

25,
0, 

11,
0, 

11,
0, 

41,
0, 

17,
0, 

18,
0, 

11,
0, 

1,
11,
0, 

11,
0, 

11,
0, 

11,
0, 

16,
0, 

21,
0, 

11,
0, 

11,
0, 

8,
11,
0, 

11,
0, 

17,
0, 

18,
0, 

11,
0, 

11,
0, 

8,
11,
0, 

11,
0, 

19,
0, 

20,
0, 

21,
0, 

21,
0, 

11,
0, 

11,
0, 

11,
0, 

4,
11,
0, 

11,
0, 

17,
0, 

11,
0, 

11,
0, 

11,
0, 

4,
11,
0, 

11,
0, 

10,
0, 

9,
12,
0, 

21,
0, 

21,
0, 

11,
0, 

11,
0, 

6,
11,
0, 

11,
0, 

11,
0, 

6,
11,
0, 

22,
0, 

23,
0, 

21,
0, 

11,
0, 

11,
0, 

11,
0, 

11,
0, 

11,
0, 

11,
0, 

11,
0, 

11,
0, 

22,
0, 

11,
0, 

11,
0, 

11,
0, 

11,
0, 

11,
0, 

11,
0, 

11,
0, 

11,
0, 

22,
0, 

22,
0, 

23,
0, 

11,
0, 

11,
0, 

2,
11,
0, 

11,
0, 

11,
0, 

11,
0, 

2,
11,
0, 

11,
0, 

22,
0, 

22,
0, 

23,
0, 

23,
0, 

11,
0, 

11,
0, 

3,
11,
0, 

11,
0, 

11,
0, 

3,
11,
0, 

22,
0, 

5,
11,
0, 

11,
0, 

5,
11,
0, 

11,
0, 

7,
11,
0, 

7,
11,
0, 
0};
# define YYTYPE unsigned char
struct yywork { YYTYPE verify, advance; } yycrank[] = {
0,0,	0,0,	1,5,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	1,6,	1,7,	
1,8,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	1,9,	
88,113,	0,0,	0,0,	0,0,	
1,10,	1,11,	1,12,	1,13,	
1,14,	1,15,	13,49,	1,16,	
1,17,	1,18,	14,50,	14,50,	
14,50,	14,50,	14,50,	14,50,	
14,50,	14,50,	14,50,	14,50,	
1,19,	1,20,	1,21,	1,22,	
16,51,	2,33,	1,23,	1,24,	
1,23,	1,23,	1,25,	1,23,	
1,26,	1,23,	1,23,	1,23,	
17,52,	24,62,	28,67,	1,27,	
1,28,	1,23,	9,45,	1,23,	
27,65,	1,23,	1,23,	30,65,	
25,63,	26,64,	2,9,	34,70,	
44,65,	39,80,	45,85,	1,29,	
2,11,	2,12,	2,13,	50,55,	
2,15,	55,90,	2,16,	2,34,	
66,98,	33,68,	69,100,	48,0,	
42,65,	24,62,	28,67,	1,30,	
40,81,	41,82,	83,109,	2,19,	
2,20,	2,21,	2,22,	18,53,	
25,63,	26,64,	27,66,	85,111,	
1,31,	39,80,	1,32,	95,123,	
33,68,	18,54,	18,55,	18,56,	
18,57,	18,57,	18,57,	18,57,	
18,57,	18,57,	18,57,	18,57,	
18,57,	18,57,	18,58,	33,69,	
40,81,	41,82,	42,83,	86,0,	
43,84,	35,71,	2,29,	35,72,	
35,72,	35,72,	35,72,	35,72,	
35,72,	35,72,	35,72,	35,72,	
35,72,	3,9,	91,116,	89,114,	
91,117,	98,125,	2,30,	3,11,	
3,12,	3,13,	3,35,	3,15,	
62,94,	3,36,	3,17,	3,37,	
107,131,	109,133,	63,95,	2,31,	
43,84,	2,32,	48,86,	64,96,	
65,97,	67,99,	3,19,	3,20,	
3,21,	3,22,	111,135,	135,155,	
3,38,	3,39,	3,38,	3,38,	
3,40,	3,38,	3,41,	3,38,	
3,38,	3,38,	89,115,	73,101,	
62,94,	3,42,	3,43,	3,38,	
4,33,	3,38,	63,95,	3,38,	
3,38,	86,112,	48,46,	64,96,	
65,97,	67,99,	137,115,	80,106,	
36,51,	3,29,	36,73,	36,73,	
36,73,	36,73,	36,73,	36,73,	
36,73,	36,73,	36,73,	36,73,	
138,157,	4,9,	72,74,	73,101,	
139,158,	3,44,	140,116,	4,11,	
4,12,	4,13,	4,35,	4,15,	
81,107,	4,36,	4,34,	4,37,	
82,108,	141,159,	3,31,	80,106,	
3,32,	84,110,	94,122,	112,0,	
96,124,	72,76,	4,19,	4,20,	
4,21,	4,22,	99,126,	136,0,	
4,38,	4,39,	4,38,	4,38,	
4,40,	4,38,	4,41,	4,38,	
4,38,	4,38,	144,162,	150,166,	
81,107,	4,42,	4,43,	4,38,	
82,108,	4,38,	171,184,	4,38,	
4,38,	84,110,	94,122,	10,46,	
96,124,	72,76,	193,206,	106,130,	
136,156,	4,29,	99,126,	10,46,	
10,0,	10,46,	54,89,	54,89,	
54,89,	54,89,	54,89,	54,89,	
54,89,	54,89,	54,89,	54,89,	
37,74,	4,44,	37,75,	37,75,	
37,75,	37,75,	37,75,	37,75,	
37,75,	37,75,	37,75,	37,75,	
197,210,	112,136,	4,31,	106,130,	
4,32,	10,47,	160,118,	0,0,	
160,118,	10,46,	92,118,	37,76,	
92,118,	0,0,	10,46,	56,91,	
56,91,	56,91,	56,91,	56,91,	
56,91,	56,91,	56,91,	56,91,	
56,91,	92,119,	108,132,	110,134,	
207,181,	0,0,	207,181,	10,46,	
10,46,	10,46,	10,46,	10,46,	
10,46,	10,46,	10,46,	10,46,	
10,46,	122,144,	124,147,	37,76,	
10,48,	10,46,	10,46,	0,0,	
10,46,	0,0,	10,46,	10,46,	
0,0,	160,120,	0,0,	0,0,	
0,0,	92,120,	108,132,	110,134,	
23,59,	23,59,	23,59,	23,59,	
23,59,	23,59,	23,59,	23,59,	
23,59,	23,59,	23,60,	126,148,	
130,150,	122,144,	124,147,	207,183,	
10,46,	23,59,	23,59,	23,59,	
23,59,	23,59,	23,59,	23,59,	
23,59,	23,59,	23,59,	23,59,	
23,59,	23,59,	23,59,	23,59,	
23,59,	23,59,	23,59,	23,59,	
23,59,	23,59,	23,59,	23,59,	
23,59,	23,59,	23,59,	126,148,	
130,150,	0,0,	0,0,	23,61,	
0,0,	23,59,	23,59,	23,59,	
23,59,	23,59,	23,59,	23,59,	
23,59,	23,59,	23,59,	23,59,	
23,59,	23,59,	23,59,	23,59,	
23,59,	23,59,	23,59,	23,59,	
23,59,	23,59,	23,59,	23,59,	
23,59,	23,59,	23,59,	38,77,	
0,0,	38,78,	38,78,	38,78,	
38,78,	38,78,	38,78,	38,78,	
38,78,	38,78,	38,78,	38,60,	
132,153,	134,154,	145,163,	146,164,	
0,0,	148,165,	38,78,	38,78,	
38,78,	38,78,	38,78,	38,78,	
38,78,	38,78,	38,78,	38,78,	
38,78,	38,78,	38,78,	38,78,	
38,78,	38,78,	38,78,	38,78,	
38,78,	38,78,	38,78,	38,78,	
38,78,	38,78,	38,78,	38,78,	
132,153,	134,154,	145,163,	146,164,	
38,79,	148,165,	38,78,	38,78,	
38,78,	38,78,	38,78,	38,78,	
38,78,	38,78,	38,78,	38,78,	
38,78,	38,78,	38,78,	38,78,	
38,78,	38,78,	38,78,	38,78,	
38,78,	38,78,	38,78,	38,78,	
38,78,	38,78,	38,78,	38,78,	
46,46,	151,167,	143,118,	152,168,	
143,118,	143,161,	0,0,	154,169,	
46,46,	46,0,	46,46,	58,92,	
58,92,	58,92,	58,92,	58,92,	
58,92,	58,92,	58,92,	58,92,	
58,92,	71,73,	71,73,	71,73,	
71,73,	71,73,	71,73,	71,73,	
71,73,	71,73,	71,73,	163,175,	
0,0,	151,167,	0,0,	152,168,	
0,0,	0,0,	46,47,	154,169,	
0,0,	0,0,	46,46,	164,176,	
52,52,	0,0,	0,0,	46,46,	
123,145,	143,120,	0,0,	0,0,	
52,52,	52,0,	52,52,	0,0,	
123,146,	195,181,	167,179,	195,181,	
195,208,	0,0,	0,0,	163,175,	
46,46,	46,46,	46,46,	46,46,	
46,46,	46,46,	46,46,	46,46,	
46,46,	46,46,	168,180,	164,176,	
0,0,	46,46,	46,46,	46,46,	
123,145,	46,46,	52,52,	46,46,	
46,46,	0,0,	52,52,	76,103,	
123,146,	76,103,	167,179,	52,52,	
76,104,	76,104,	76,104,	76,104,	
76,104,	76,104,	76,104,	76,104,	
76,104,	76,104,	0,0,	0,0,	
195,183,	0,0,	168,180,	173,185,	
52,52,	52,52,	52,52,	52,52,	
52,52,	52,52,	52,52,	52,52,	
52,52,	52,52,	0,0,	0,0,	
0,0,	52,52,	52,52,	52,52,	
53,87,	52,52,	53,87,	52,52,	
52,52,	53,88,	53,88,	53,88,	
53,88,	53,88,	53,88,	53,88,	
53,88,	53,88,	53,88,	173,185,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	53,88,	53,88,	
53,88,	53,88,	53,88,	53,88,	
60,93,	60,93,	60,93,	60,93,	
60,93,	60,93,	60,93,	60,93,	
60,93,	60,93,	60,93,	60,93,	
60,93,	60,93,	60,93,	60,93,	
60,93,	60,93,	60,93,	60,93,	
60,93,	60,93,	60,93,	60,93,	
60,93,	60,93,	53,88,	53,88,	
53,88,	53,88,	53,88,	53,88,	
60,93,	60,93,	60,93,	60,93,	
60,93,	60,93,	60,93,	60,93,	
60,93,	60,93,	60,93,	60,93,	
60,93,	60,93,	60,93,	60,93,	
60,93,	60,93,	60,93,	60,93,	
60,93,	60,93,	60,93,	60,93,	
60,93,	60,93,	61,59,	61,59,	
61,59,	61,59,	61,59,	61,59,	
61,59,	61,59,	61,59,	61,59,	
0,0,	174,186,	175,187,	176,188,	
177,189,	178,190,	179,191,	61,59,	
61,59,	61,59,	61,59,	61,59,	
61,59,	61,59,	61,59,	61,59,	
61,59,	61,59,	61,59,	61,59,	
61,59,	61,59,	61,59,	61,59,	
61,59,	61,59,	61,59,	61,59,	
61,59,	61,59,	61,59,	61,59,	
61,59,	174,186,	175,187,	176,188,	
177,189,	178,190,	179,191,	61,59,	
61,59,	61,59,	61,59,	61,59,	
61,59,	61,59,	61,59,	61,59,	
61,59,	61,59,	61,59,	61,59,	
61,59,	61,59,	61,59,	61,59,	
61,59,	61,59,	61,59,	61,59,	
61,59,	61,59,	61,59,	61,59,	
61,59,	70,70,	170,181,	180,192,	
170,181,	131,151,	162,173,	0,0,	
185,198,	70,70,	70,0,	70,70,	
0,0,	131,152,	162,174,	186,199,	
0,0,	170,182,	187,200,	188,201,	
0,0,	74,90,	0,0,	74,102,	
74,102,	74,102,	74,102,	74,102,	
74,102,	74,102,	74,102,	74,102,	
74,102,	0,0,	0,0,	180,192,	
0,0,	131,151,	162,173,	70,70,	
185,198,	0,0,	0,0,	70,70,	
74,101,	131,152,	162,174,	186,199,	
70,70,	170,183,	187,200,	188,201,	
75,74,	0,0,	75,75,	75,75,	
75,75,	75,75,	75,75,	75,75,	
75,75,	75,75,	75,75,	75,75,	
0,0,	70,70,	70,70,	70,70,	
70,70,	70,70,	70,70,	70,70,	
70,70,	70,70,	70,70,	75,76,	
74,101,	0,0,	70,70,	70,70,	
70,70,	0,0,	70,70,	0,0,	
70,70,	70,70,	0,0,	0,0,	
0,0,	0,0,	0,0,	77,77,	
77,77,	77,77,	77,77,	77,77,	
77,77,	77,77,	77,77,	77,77,	
77,77,	0,0,	189,202,	190,203,	
191,204,	192,205,	0,0,	75,76,	
77,77,	77,77,	77,77,	77,77,	
77,77,	77,77,	77,77,	77,77,	
77,77,	77,77,	77,77,	77,77,	
77,77,	77,77,	77,77,	77,77,	
77,77,	77,77,	77,77,	77,77,	
77,77,	77,77,	77,77,	77,77,	
77,77,	77,77,	189,202,	190,203,	
191,204,	192,205,	77,77,	0,0,	
77,77,	77,77,	77,77,	77,77,	
77,77,	77,77,	77,77,	77,77,	
77,77,	77,77,	77,77,	77,77,	
77,77,	77,77,	77,77,	77,77,	
77,77,	77,77,	77,77,	77,77,	
77,77,	77,77,	77,77,	77,77,	
77,77,	77,77,	79,77,	0,0,	
79,78,	79,78,	79,78,	79,78,	
79,78,	79,78,	79,78,	79,78,	
79,78,	79,78,	0,0,	198,211,	
199,212,	201,213,	202,214,	0,0,	
203,215,	79,78,	79,78,	79,78,	
79,78,	79,78,	79,78,	79,78,	
79,78,	79,78,	79,78,	79,78,	
79,78,	79,78,	79,78,	79,78,	
79,78,	79,78,	79,78,	79,78,	
79,78,	79,78,	79,78,	79,78,	
79,78,	79,78,	79,78,	198,211,	
199,212,	201,213,	202,214,	79,105,	
203,215,	79,78,	79,78,	79,78,	
79,78,	79,78,	79,78,	79,78,	
79,78,	79,78,	79,78,	79,78,	
79,78,	79,78,	79,78,	79,78,	
79,78,	79,78,	79,78,	79,78,	
79,78,	79,78,	79,78,	79,78,	
79,78,	79,78,	79,78,	87,88,	
87,88,	87,88,	87,88,	87,88,	
87,88,	87,88,	87,88,	87,88,	
87,88,	0,0,	0,0,	205,216,	
211,218,	212,219,	214,220,	215,221,	
87,88,	87,88,	87,88,	87,88,	
87,88,	87,88,	93,93,	93,93,	
93,93,	93,93,	93,93,	93,93,	
93,93,	93,93,	93,93,	93,93,	
100,100,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	219,222,	
100,100,	100,0,	100,100,	205,216,	
211,218,	212,219,	214,220,	215,221,	
87,88,	87,88,	87,88,	87,88,	
87,88,	87,88,	101,127,	0,0,	
101,127,	0,0,	0,0,	101,128,	
101,128,	101,128,	101,128,	101,128,	
101,128,	101,128,	101,128,	101,128,	
101,128,	93,121,	100,100,	219,222,	
0,0,	0,0,	100,100,	221,223,	
0,0,	0,0,	0,0,	100,100,	
0,0,	0,0,	0,0,	0,0,	
0,0,	102,102,	102,102,	102,102,	
102,102,	102,102,	102,102,	102,102,	
102,102,	102,102,	102,102,	0,0,	
100,100,	100,100,	100,100,	100,100,	
100,100,	100,100,	100,100,	100,100,	
100,100,	100,100,	102,101,	221,223,	
0,0,	100,100,	100,100,	100,100,	
0,0,	100,100,	0,0,	100,100,	
100,100,	103,129,	103,129,	103,129,	
103,129,	103,129,	103,129,	103,129,	
103,129,	103,129,	103,129,	104,104,	
104,104,	104,104,	104,104,	104,104,	
104,104,	104,104,	104,104,	104,104,	
104,104,	0,0,	102,101,	105,105,	
105,105,	105,105,	105,105,	105,105,	
105,105,	105,105,	105,105,	105,105,	
105,105,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
105,105,	105,105,	105,105,	105,105,	
105,105,	105,105,	105,105,	105,105,	
105,105,	105,105,	105,105,	105,105,	
105,105,	105,105,	105,105,	105,105,	
105,105,	105,105,	105,105,	105,105,	
105,105,	105,105,	105,105,	105,105,	
105,105,	105,105,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
105,105,	105,105,	105,105,	105,105,	
105,105,	105,105,	105,105,	105,105,	
105,105,	105,105,	105,105,	105,105,	
105,105,	105,105,	105,105,	105,105,	
105,105,	105,105,	105,105,	105,105,	
105,105,	105,105,	105,105,	105,105,	
105,105,	105,105,	114,137,	114,137,	
114,137,	114,137,	114,137,	114,137,	
114,137,	114,137,	114,137,	114,137,	
115,138,	115,138,	115,138,	115,138,	
115,138,	115,138,	115,138,	115,138,	
115,138,	115,138,	116,139,	116,139,	
116,139,	116,139,	116,139,	116,139,	
116,139,	116,139,	116,139,	116,139,	
117,140,	117,140,	117,140,	117,140,	
117,140,	117,140,	117,140,	117,140,	
117,140,	117,140,	118,141,	118,141,	
118,141,	118,141,	118,141,	118,141,	
118,141,	118,141,	118,141,	118,141,	
119,142,	0,0,	119,143,	119,143,	
119,143,	119,143,	119,143,	119,143,	
119,143,	119,143,	119,143,	119,143,	
121,93,	121,93,	121,93,	121,93,	
121,93,	121,93,	121,93,	121,93,	
121,93,	121,93,	127,149,	127,149,	
127,149,	127,149,	127,149,	127,149,	
127,149,	127,149,	127,149,	127,149,	
128,128,	128,128,	128,128,	128,128,	
128,128,	128,128,	128,128,	128,128,	
128,128,	128,128,	142,160,	142,160,	
142,160,	142,160,	142,160,	142,160,	
142,160,	142,160,	142,160,	142,160,	
157,170,	157,170,	157,170,	157,170,	
157,170,	157,170,	157,170,	157,170,	
157,170,	157,170,	158,171,	158,171,	
158,171,	158,171,	158,171,	158,171,	
158,171,	158,171,	158,171,	158,171,	
159,172,	159,172,	159,172,	159,172,	
159,172,	159,172,	159,172,	159,172,	
159,172,	159,172,	161,118,	0,0,	
161,118,	0,0,	0,0,	161,161,	
161,161,	161,161,	161,161,	161,161,	
161,161,	161,161,	161,161,	161,161,	
161,161,	0,0,	166,177,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	166,178,	181,193,	
181,193,	181,193,	181,193,	181,193,	
181,193,	181,193,	181,193,	181,193,	
181,193,	182,194,	0,0,	182,195,	
182,195,	182,195,	182,195,	182,195,	
182,195,	182,195,	182,195,	182,195,	
182,195,	161,120,	166,177,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	184,196,	166,178,	184,197,	
184,197,	184,197,	184,197,	184,197,	
184,197,	184,197,	184,197,	184,197,	
184,197,	194,207,	194,207,	194,207,	
194,207,	194,207,	194,207,	194,207,	
194,207,	194,207,	194,207,	196,209,	
196,209,	196,209,	196,209,	196,209,	
196,209,	196,209,	196,209,	196,209,	
196,209,	206,217,	206,217,	206,217,	
206,217,	206,217,	206,217,	206,217,	
206,217,	206,217,	206,217,	208,181,	
0,0,	208,181,	0,0,	0,0,	
208,208,	208,208,	208,208,	208,208,	
208,208,	208,208,	208,208,	208,208,	
208,208,	208,208,	210,210,	210,210,	
210,210,	210,210,	210,210,	210,210,	
210,210,	210,210,	210,210,	210,210,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	208,183,	0,0,	
0,0};
struct yysvf yysvec[] = {
0,	0,	0,
yycrank+-1,	0,		0,	
yycrank+-56,	yysvec+1,	0,	
yycrank+-127,	yysvec+1,	0,	
yycrank+-199,	yysvec+1,	0,	
yycrank+0,	0,		yyvstop+1,
yycrank+0,	0,		yyvstop+3,
yycrank+0,	0,		yyvstop+6,
yycrank+0,	0,		yyvstop+8,
yycrank+4,	0,		yyvstop+11,
yycrank+-286,	0,		yyvstop+14,
yycrank+0,	0,		yyvstop+16,
yycrank+0,	0,		yyvstop+19,
yycrank+4,	0,		yyvstop+22,
yycrank+2,	0,		yyvstop+25,
yycrank+0,	0,		yyvstop+27,
yycrank+18,	0,		yyvstop+30,
yycrank+34,	0,		yyvstop+33,
yycrank+84,	0,		yyvstop+36,
yycrank+0,	0,		yyvstop+40,
yycrank+0,	0,		yyvstop+43,
yycrank+0,	0,		yyvstop+46,
yycrank+0,	0,		yyvstop+49,
yycrank+332,	0,		yyvstop+52,
yycrank+8,	yysvec+23,	yyvstop+55,
yycrank+10,	yysvec+23,	yyvstop+58,
yycrank+7,	yysvec+23,	yyvstop+61,
yycrank+37,	yysvec+23,	yyvstop+64,
yycrank+12,	yysvec+23,	yyvstop+67,
yycrank+0,	0,		yyvstop+70,
yycrank+40,	yysvec+23,	yyvstop+73,
yycrank+0,	0,		yyvstop+76,
yycrank+0,	0,		yyvstop+79,
yycrank+96,	0,		yyvstop+82,
yycrank+49,	0,		yyvstop+85,
yycrank+103,	0,		yyvstop+88,
yycrank+174,	0,		yyvstop+90,
yycrank+262,	yysvec+18,	yyvstop+93,
yycrank+409,	0,		yyvstop+97,
yycrank+24,	yysvec+38,	yyvstop+100,
yycrank+34,	yysvec+38,	yyvstop+103,
yycrank+31,	yysvec+38,	yyvstop+106,
yycrank+61,	yysvec+38,	yyvstop+109,
yycrank+82,	yysvec+38,	yyvstop+112,
yycrank+45,	yysvec+38,	yyvstop+115,
yycrank+9,	0,		0,	
yycrank+-531,	0,		0,	
yycrank+0,	0,		yyvstop+118,
yycrank+-97,	yysvec+46,	0,	
yycrank+0,	0,		yyvstop+120,
yycrank+53,	yysvec+14,	yyvstop+122,
yycrank+0,	0,		yyvstop+125,
yycrank+-575,	0,		yyvstop+127,
yycrank+613,	0,		0,	
yycrank+250,	0,		0,	
yycrank+55,	0,		0,	
yycrank+287,	0,		0,	
yycrank+0,	yysvec+18,	yyvstop+129,
yycrank+495,	0,		0,	
yycrank+0,	yysvec+23,	yyvstop+132,
yycrank+619,	0,		0,	
yycrank+694,	0,		0,	
yycrank+101,	yysvec+23,	yyvstop+134,
yycrank+110,	yysvec+23,	yyvstop+136,
yycrank+104,	yysvec+23,	yyvstop+138,
yycrank+119,	0,		0,	
yycrank+28,	yysvec+23,	yyvstop+140,
yycrank+111,	yysvec+23,	yyvstop+142,
yycrank+0,	yysvec+33,	0,	
yycrank+64,	0,		0,	
yycrank+-816,	0,		yyvstop+144,
yycrank+505,	0,		0,	
yycrank+188,	yysvec+35,	yyvstop+147,
yycrank+134,	yysvec+71,	yyvstop+150,
yycrank+791,	0,		yyvstop+152,
yycrank+822,	yysvec+18,	yyvstop+154,
yycrank+576,	0,		0,	
yycrank+859,	0,		yyvstop+157,
yycrank+0,	yysvec+38,	yyvstop+159,
yycrank+936,	0,		0,	
yycrank+148,	yysvec+38,	yyvstop+161,
yycrank+176,	yysvec+38,	yyvstop+163,
yycrank+169,	yysvec+38,	yyvstop+165,
yycrank+38,	yysvec+38,	yyvstop+167,
yycrank+179,	yysvec+38,	yyvstop+169,
yycrank+47,	0,		0,	
yycrank+-137,	yysvec+46,	0,	
yycrank+1011,	0,		0,	
yycrank+1,	yysvec+87,	0,	
yycrank+118,	yysvec+54,	yyvstop+171,
yycrank+0,	0,		yyvstop+173,
yycrank+117,	yysvec+56,	yyvstop+175,
yycrank+287,	yysvec+58,	yyvstop+177,
yycrank+1034,	yysvec+60,	yyvstop+179,
yycrank+181,	yysvec+23,	yyvstop+181,
yycrank+32,	yysvec+23,	yyvstop+183,
yycrank+171,	yysvec+23,	yyvstop+186,
yycrank+0,	0,		yyvstop+188,
yycrank+89,	yysvec+23,	yyvstop+190,
yycrank+193,	yysvec+23,	yyvstop+192,
yycrank+-1091,	0,		yyvstop+194,
yycrank+1071,	0,		0,	
yycrank+1097,	0,		yyvstop+196,
yycrank+1129,	0,		0,	
yycrank+1139,	0,		yyvstop+198,
yycrank+1151,	yysvec+79,	0,	
yycrank+218,	yysvec+38,	yyvstop+200,
yycrank+81,	yysvec+38,	yyvstop+202,
yycrank+261,	yysvec+38,	yyvstop+205,
yycrank+101,	yysvec+38,	yyvstop+207,
yycrank+278,	yysvec+38,	yyvstop+209,
yycrank+114,	0,		0,	
yycrank+-245,	yysvec+46,	0,	
yycrank+0,	0,		yyvstop+211,
yycrank+1226,	0,		0,	
yycrank+1236,	0,		0,	
yycrank+1246,	0,		0,	
yycrank+1256,	0,		0,	
yycrank+1266,	0,		0,	
yycrank+1278,	0,		0,	
yycrank+0,	0,		yyvstop+213,
yycrank+1288,	yysvec+60,	0,	
yycrank+283,	yysvec+23,	yyvstop+215,
yycrank+509,	yysvec+61,	0,	
yycrank+282,	yysvec+23,	yyvstop+217,
yycrank+0,	yysvec+23,	yyvstop+219,
yycrank+324,	yysvec+23,	yyvstop+222,
yycrank+1298,	0,		0,	
yycrank+1308,	0,		yyvstop+224,
yycrank+0,	yysvec+103,	yyvstop+226,
yycrank+314,	yysvec+38,	yyvstop+228,
yycrank+750,	yysvec+79,	0,	
yycrank+388,	yysvec+38,	yyvstop+230,
yycrank+0,	yysvec+38,	yyvstop+232,
yycrank+402,	yysvec+38,	yyvstop+235,
yycrank+157,	0,		0,	
yycrank+-253,	yysvec+46,	0,	
yycrank+134,	yysvec+114,	yyvstop+237,
yycrank+174,	yysvec+115,	0,	
yycrank+178,	yysvec+116,	0,	
yycrank+193,	yysvec+117,	yyvstop+239,
yycrank+191,	yysvec+118,	yyvstop+241,
yycrank+1318,	0,		0,	
yycrank+491,	yysvec+119,	yyvstop+243,
yycrank+179,	yysvec+23,	yyvstop+245,
yycrank+388,	yysvec+23,	yyvstop+247,
yycrank+405,	yysvec+23,	yyvstop+249,
yycrank+0,	yysvec+23,	yyvstop+251,
yycrank+389,	yysvec+23,	yyvstop+254,
yycrank+0,	yysvec+127,	yyvstop+256,
yycrank+180,	yysvec+38,	yyvstop+258,
yycrank+451,	yysvec+38,	yyvstop+260,
yycrank+469,	yysvec+38,	yyvstop+262,
yycrank+0,	yysvec+38,	yyvstop+264,
yycrank+455,	yysvec+38,	yyvstop+267,
yycrank+0,	0,		yyvstop+269,
yycrank+0,	0,		yyvstop+271,
yycrank+1328,	0,		0,	
yycrank+1338,	0,		0,	
yycrank+1348,	0,		0,	
yycrank+283,	yysvec+142,	yyvstop+274,
yycrank+1363,	0,		yyvstop+276,
yycrank+751,	yysvec+61,	0,	
yycrank+484,	yysvec+23,	yyvstop+278,
yycrank+501,	yysvec+23,	yyvstop+280,
yycrank+0,	yysvec+23,	yyvstop+282,
yycrank+1351,	yysvec+79,	0,	
yycrank+511,	yysvec+38,	yyvstop+285,
yycrank+532,	yysvec+38,	yyvstop+287,
yycrank+0,	yysvec+38,	yyvstop+289,
yycrank+775,	yysvec+157,	yyvstop+292,
yycrank+224,	yysvec+158,	yyvstop+294,
yycrank+0,	yysvec+159,	yyvstop+296,
yycrank+557,	yysvec+23,	yyvstop+298,
yycrank+687,	yysvec+23,	yyvstop+300,
yycrank+669,	yysvec+23,	yyvstop+302,
yycrank+686,	yysvec+23,	yyvstop+304,
yycrank+674,	yysvec+38,	yyvstop+306,
yycrank+691,	yysvec+38,	yyvstop+308,
yycrank+673,	yysvec+38,	yyvstop+310,
yycrank+750,	yysvec+38,	yyvstop+312,
yycrank+1383,	0,		0,	
yycrank+1395,	0,		0,	
yycrank+0,	0,		yyvstop+314,
yycrank+1415,	0,		0,	
yycrank+745,	yysvec+23,	yyvstop+316,
yycrank+757,	yysvec+23,	yyvstop+318,
yycrank+754,	yysvec+23,	yyvstop+320,
yycrank+768,	yysvec+23,	yyvstop+322,
yycrank+839,	yysvec+38,	yyvstop+324,
yycrank+845,	yysvec+38,	yyvstop+326,
yycrank+840,	yysvec+38,	yyvstop+328,
yycrank+854,	yysvec+38,	yyvstop+330,
yycrank+232,	yysvec+181,	yyvstop+332,
yycrank+1425,	0,		0,	
yycrank+546,	yysvec+182,	yyvstop+334,
yycrank+1435,	0,		0,	
yycrank+274,	yysvec+184,	yyvstop+336,
yycrank+910,	yysvec+23,	yyvstop+338,
yycrank+927,	yysvec+23,	yyvstop+340,
yycrank+0,	yysvec+23,	yyvstop+342,
yycrank+913,	yysvec+23,	yyvstop+345,
yycrank+913,	yysvec+38,	yyvstop+347,
yycrank+931,	yysvec+38,	yyvstop+349,
yycrank+0,	yysvec+38,	yyvstop+351,
yycrank+987,	yysvec+38,	yyvstop+354,
yycrank+1445,	0,		0,	
yycrank+305,	yysvec+194,	yyvstop+356,
yycrank+1460,	0,		yyvstop+358,
yycrank+0,	yysvec+196,	yyvstop+360,
yycrank+1470,	0,		yyvstop+362,
yycrank+992,	yysvec+23,	yyvstop+364,
yycrank+1006,	yysvec+23,	yyvstop+366,
yycrank+0,	yysvec+23,	yyvstop+368,
yycrank+994,	yysvec+38,	yyvstop+371,
yycrank+1008,	yysvec+38,	yyvstop+373,
yycrank+0,	yysvec+38,	yyvstop+375,
yycrank+0,	yysvec+206,	yyvstop+378,
yycrank+0,	yysvec+23,	yyvstop+380,
yycrank+1015,	yysvec+23,	yyvstop+383,
yycrank+0,	yysvec+38,	yyvstop+385,
yycrank+1051,	yysvec+38,	yyvstop+388,
yycrank+0,	yysvec+23,	yyvstop+390,
yycrank+0,	yysvec+38,	yyvstop+393,
0,	0,	0};
struct yywork *yytop = yycrank+1550;
struct yysvf *yybgin = yysvec+1;
char yymatch[] = {
  0,   1,   1,   1,   1,   1,   1,   1, 
  1,   9,  10,  11,  11,  11,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  9,   1,   1,   1,   1,   1,   1,  39, 
  1,   1,   1,  43,   1,  43,   1,   1, 
 48,  48,  48,  48,  48,  48,  48,  48, 
 48,  48,   1,   1,   1,   1,   1,   1, 
  1,  65,  66,  67,  68,  69,  70,  71, 
 72,  73,  74,  72,  72,  72,  78,  79, 
 80,  72,  82,  72,  84,  85,  72,  72, 
 72,  72,  72,   1,   1,   1,   1,   1, 
  1,  65,  66,  67,  68,  69,  70,  71, 
 72,  73,  74,  72,  72,  72,  78,  79, 
 80,  72,  82,  72,  84,  85,  72,  72, 
 72,  72,  72,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
0};
char yyextra[] = {
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,1,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0};
/*	Copyright (c) 1989 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#pragma ident	"@(#)ncform	6.12	97/12/08 SMI"

int yylineno =1;
# define YYU(x) x
# define NLSTATE yyprevious=YYNEWLINE
struct yysvf *yylstate [YYLMAX], **yylsp, **yyolsp;
char yysbuf[YYLMAX];
char *yysptr = yysbuf;
int *yyfnd;
extern struct yysvf *yyestate;
int yyprevious = YYNEWLINE;
#if defined(__cplusplus) || defined(__STDC__)
int yylook(void)
#else
yylook()
#endif
{
	register struct yysvf *yystate, **lsp;
	register struct yywork *yyt;
	struct yysvf *yyz;
	int yych, yyfirst;
	struct yywork *yyr;
# ifdef LEXDEBUG
	int debug;
# endif
	char *yylastch;
	/* start off machines */
# ifdef LEXDEBUG
	debug = 0;
# endif
	yyfirst=1;
	if (!yymorfg)
		yylastch = yytext;
	else {
		yymorfg=0;
		yylastch = yytext+yyleng;
		}
	for(;;){
		lsp = yylstate;
		yyestate = yystate = yybgin;
		if (yyprevious==YYNEWLINE) yystate++;
		for (;;){
# ifdef LEXDEBUG
			if(debug)fprintf(yyout,"state %d\n",yystate-yysvec-1);
# endif
			yyt = yystate->yystoff;
			if(yyt == yycrank && !yyfirst){  /* may not be any transitions */
				yyz = yystate->yyother;
				if(yyz == 0)break;
				if(yyz->yystoff == yycrank)break;
				}
#ifndef __cplusplus
			*yylastch++ = yych = input();
#else
			*yylastch++ = yych = lex_input();
#endif
#ifdef YYISARRAY
			if(yylastch > &yytext[YYLMAX]) {
				fprintf(yyout,"Input string too long, limit %d\n",YYLMAX);
				exit(1);
			}
#else
			if (yylastch >= &yytext[ yytextsz ]) {
				int	x = yylastch - yytext;

				yytextsz += YYTEXTSZINC;
				if (yytext == yy_tbuf) {
				    yytext = (char *) malloc(yytextsz);
				    memcpy(yytext, yy_tbuf, sizeof (yy_tbuf));
				}
				else
				    yytext = (char *) realloc(yytext, yytextsz);
				if (!yytext) {
				    fprintf(yyout,
					"Cannot realloc yytext\n");
				    exit(1);
				}
				yylastch = yytext + x;
			}
#endif
			yyfirst=0;
		tryagain:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"char ");
				allprint(yych);
				putchar('\n');
				}
# endif
			yyr = yyt;
			if ( (uintptr_t)yyt > (uintptr_t)yycrank){
				yyt = yyr + yych;
				if (yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					if(lsp > &yylstate[YYLMAX]) {
						fprintf(yyout,"Input string too long, limit %d\n",YYLMAX);
						exit(1);
					}
					goto contin;
					}
				}
# ifdef YYOPTIM
			else if((uintptr_t)yyt < (uintptr_t)yycrank) {	/* r < yycrank */
				yyt = yyr = yycrank+(yycrank-yyt);
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"compressed state\n");
# endif
				yyt = yyt + yych;
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					if(lsp > &yylstate[YYLMAX]) {
						fprintf(yyout,"Input string too long, limit %d\n",YYLMAX);
						exit(1);
					}
					goto contin;
					}
				yyt = yyr + YYU(yymatch[yych]);
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"try fall back character ");
					allprint(YYU(yymatch[yych]));
					putchar('\n');
					}
# endif
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transition */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					if(lsp > &yylstate[YYLMAX]) {
						fprintf(yyout,"Input string too long, limit %d\n",YYLMAX);
						exit(1);
					}
					goto contin;
					}
				}
			if ((yystate = yystate->yyother) && (yyt= yystate->yystoff) != yycrank){
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"fall back to state %d\n",yystate-yysvec-1);
# endif
				goto tryagain;
				}
# endif
			else
				{unput(*--yylastch);break;}
		contin:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"state %d char ",yystate-yysvec-1);
				allprint(yych);
				putchar('\n');
				}
# endif
			;
			}
# ifdef LEXDEBUG
		if(debug){
			fprintf(yyout,"stopped at %d with ",*(lsp-1)-yysvec-1);
			allprint(yych);
			putchar('\n');
			}
# endif
		while (lsp-- > yylstate){
			*yylastch-- = 0;
			if (*lsp != 0 && (yyfnd= (*lsp)->yystops) && *yyfnd > 0){
				yyolsp = lsp;
				if(yyextra[*yyfnd]){		/* must backup */
					while(yyback((*lsp)->yystops,-*yyfnd) != 1 && lsp > yylstate){
						lsp--;
						unput(*yylastch--);
						}
					}
				yyprevious = YYU(*yylastch);
				yylsp = lsp;
				yyleng = yylastch-yytext+1;
				yytext[yyleng] = 0;
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"\nmatch ");
					sprint(yytext);
					fprintf(yyout," action %d\n",*yyfnd);
					}
# endif
				return(*yyfnd++);
				}
			unput(*yylastch);
			}
		if (yytext[0] == 0  /* && feof(yyin) */)
			{
			yysptr=yysbuf;
			return(0);
			}
#ifndef __cplusplus
		yyprevious = yytext[0] = input();
		if (yyprevious>0)
			output(yyprevious);
#else
		yyprevious = yytext[0] = lex_input();
		if (yyprevious>0)
			lex_output(yyprevious);
#endif
		yylastch=yytext;
# ifdef LEXDEBUG
		if(debug)putchar('\n');
# endif
		}
	}
#if defined(__cplusplus) || defined(__STDC__)
int yyback(int *p, int m)
#else
yyback(p, m)
	int *p;
    int *m;
#endif
{
	if (p==0) return(0);
	while (*p) {
		if (*p++ == m)
			return(1);
	}
	return(0);
}
	/* the following are only used in the lex library */
#if defined(__cplusplus) || defined(__STDC__)
int yyinput(void)
#else
yyinput()
#endif
{
#ifndef __cplusplus
	return(input());
#else
	return(lex_input());
#endif
	}
#if defined(__cplusplus) || defined(__STDC__)
void yyoutput(int c)
#else
yyoutput(c)
  int c; 
#endif
{
#ifndef __cplusplus
	output(c);
#else
	lex_output(c);
#endif
	}
#if defined(__cplusplus) || defined(__STDC__)
void yyunput(int c)
#else
yyunput(c)
   int c; 
#endif
{
	unput(c);
	}
