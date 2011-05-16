/*****************************************************************************
 
  Module:  odlinter.h

  Description:  This C-language include file contains prototypes for the
		functions used internally by the PDS Object Description
		Language (ODL) processing software.

  Author:  Marti DeMore, Jet Propulsion Laboratory

  Creation Date: 23 March 1992

  History:

  Creation - This include file was included in the Version 2.2 modification
  of the ODLC library.

  Version 2.3 - M. DeMore, Jet Propulsion Laboratory
     Modified function prototypes ot operate with all platforms.
****************************************************************************/

#ifndef ODLINTER
#define ODLINTER

#ifndef SUN_UNIX

VALUE_DATA ODLConvertInteger  (char [], int);
VALUE_DATA ODLConvertReal     (char [], int);
VALUE_DATA ODLConvertSymbol   (char [], int, int);
VALUE_DATA ODLConvertString   (char [], int);
VALUE_DATA ODLConvertDate     (char [], int);
VALUE_DATA ODLConvertTime     (char [], int);
VALUE_DATA ODLConvertDateTime (char [], int);
VALUE_DATA ODLConvertDate     (char [], int);
VALUE_DATA ODLConvertNull     (char [], int);

void ODLExtractDate  (char *, VALUE_DATA *);
void ODLExtractTime  (char *, VALUE_DATA *);

int ODLFormatInteger  (char [], VALUE_DATA *);
int ODLFormatReal     (char [], VALUE_DATA *);
int ODLFormatUnits    (char [], struct ODLUnits *);
int ODLFormatSymbol   (char [], VALUE_DATA *);
int ODLFormatDate     (char [], VALUE_DATA *);
int ODLFormatTime     (char [], VALUE_DATA *);
int ODLFormatDateTime (char [], VALUE_DATA *);
int ODLFormatString   (char [], VALUE_DATA *, int *, int, int, int, int);
int ODLFormatComment  (char [], char [], int, int);
int ODLFormatNullValue(char [], VALUE_DATA *);

void ODLPrintError   (char []);
void ODLPrintWarning (char []);
void ODLPrintInfo    (char []);
void ODLPrintStmt    (char []);
void ODLWriteStmt    (FILE *, char []);

void ODLBeginAggregate (AGGREGATE_KIND, VALUE_DATA *);
void ODLEndAggregate   (AGGREGATE_KIND, VALUE_DATA *);
void ODLBeginParameter (PARAMETER_KIND, VALUE_DATA *);
void ODLMarkParameter  (VALUE_KIND);
void ODLStoreValue     (VALUE_DATA *);
void ODLStoreUnits1    (VALUE_DATA *);
void ODLStoreUnits2    (VALUE_DATA *, VALUE_DATA *);
void ODLMarkUnits      (int);
void ODLCheckSequence  (void);
void ODLCheckRange     (VALUE_DATA *, VALUE_DATA *);
int ODLEndLabel        (void);
char *ODLNewString     (void);
char ODLPeekString     (int);
char ODLBackupString   (void);
char *ODLStoreString   (char);
void ODLKillString     (void);
int ODLStringLength    (void);

int yylex              (void);
int yywrap             (void);
int yylook             (void);
char *yyGetStringToken (void);
void yyGetComment      (void);
int yyinput            (void);
//int yyoutput           (int);
//int yyunput            (int);
int yyback             (int *, int);
int yyerror            (char *);
int yyparse            (void);


#else

VALUE_DATA ODLConvertInteger  ();
VALUE_DATA ODLConvertReal     ();
VALUE_DATA ODLConvertSymbol   ();
VALUE_DATA ODLConvertString   ();
VALUE_DATA ODLConvertTime     ();
VALUE_DATA ODLConvertDateTime ();
VALUE_DATA ODLConvertDate     ();
VALUE_DATA ODLConvertNull     ();

void ODLExtractDate ();
void ODLExtractTime ();

int ODLFormatInteger  ();
int ODLFormatReal     ();
int ODLFormatUnits    ();
int ODLFormatSymbol   ();
int ODLFormatDate     ();
int ODLFormatTime     ();
int ODLFormatDateTime ();
int ODLFormatString   ();
int ODLFormatComment  ();

void ODLPrintError   ();
void ODLPrintWarning ();
void ODLPrintInfo    ();
void ODLPrintStmt    ();
void ODLWriteStmt    ();

void ODLBeginAggregate ();
void ODLEndAggregate   ();
void ODLBeginParameter ();
void ODLMarkParameter  ();
void ODLStoreValue     ();
void ODLStoreUnits1    ();
void ODLStoreUnits2    ();
void ODLMarkUnits      ();
void ODLCheckSequence  ();
void ODLCheckRange     ();
int ODLEndLabel        ();
char *ODLNewString     ();
char ODLPeekString     ();
char ODLBackupString   ();
char *ODLStoreString   ();
void ODLKillString     ();
int ODLStringLength    ();

int yylex              ();
int yywrap             ();
int yylook             ();
char *yyGetStringToken ();
void yyGetComment      ();
/* These are defined in lexan.c
int yyinput            ();
int yyoutput           ();
int yyunput            ();
int yyback             ();
int yyerror            ();
int yyparse            ();
*/



#endif
#endif
