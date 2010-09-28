/***************************************************************************/
/*                          lablib3.h                                      */
/*                                                                         */
/*  Version:                                                               */
/*                                                                         */
/*      1.0    March 31, 1994                                              */
/*                                                                         */
/*  Change History:                                                        */
/*                                                                         */
/*      03-31-94    Original code                                          */
/*	01-09-95    jsh - Changed OBJECT to OBJDESC                        */
/*      12-11-95    sm/LASP - Replaced 'class' by 'class_name' for C++     */
/*                            compatibility.                               */
/*                          - Added parameters to OdlGetFileSpec prototype.*/
/*                          - Added prototype for OdlCheckFileSpec.        */
/*                          - Added declaration of odl_errno so visible    */
/*                            to callers of L3 functions.                  */
/*      06-10-97    sm/LASP - Added prototype for OdlExtractFilename.      */
/*      01-29-98    sm/LASP - Added prototypes for routines to get and set */
/*                            comments and filename fields in OBJDESC and  */
/*                            KEYWORD structures.                          */
/*      01-30-98    sm/LASP - Added #defines ODL_SUPPRESS_END_STATEMENT,   */
/*                            ODL_UNEXPAND_STRUCTURE, ODL_UNEXPAND_CATALOG */
/*                            for OdlPrintLabel.                           */
/*                                                                         */
/***************************************************************************/

#ifndef __LABLIB3_LOADED
#define __LABLIB3_LOADED

#include "toolbox.h"

/**************************************************************************/
/*                         Global Variable definitions                    */
/**************************************************************************/

extern int odl_errno;


/**************************************************************************/
/*                         Symbol definitions                             */
/**************************************************************************/

/*  These symbols are used by the label library to determine what, if any,
    type of expanding should be performed on a label.  They are used as masks
    and MUST be in powers of two.
*/

#define ODL_NOEXPAND         0
#define ODL_EXPAND_STRUCTURE 1
#define ODL_EXPAND_CATALOG   2

/*  These symbols are used by OdlPrintLabel to determine what, if any,
    type of un-expanding should be performed on an ODL tree when
    printing it, and whether or not to print the final "END" statement
    (^STRUCTURE include files shouldn't have an "END" statement).
    They are used as bit masks and MUST be in powers of two.  */

#define ODL_UNEXPAND_STRUCTURE      1
#define ODL_UNEXPAND_CATALOG        2
#define ODL_SUPPRESS_END_STATEMENT  4

/*  These symbols are used to restrict the scope of an object search  */

#define ODL_RECURSIVE_DOWN   0
#define ODL_TO_END           1
#define ODL_CHILDREN_ONLY    2
#define ODL_SIBLINGS_ONLY    3
#define ODL_THIS_OBJECT      4

/*  These symbols represent the different data location types  */

#define ODL_RECORD_LOCATION 0
#define ODL_BYTE_LOCATION   1

/*  These symbols represent the data type of a keyword's value  */

#define ODL_UNKNOWN   0
#define ODL_INTEGER   1
#define ODL_REAL      2
#define ODL_SYMBOL    3
#define ODL_TEXT      4
#define ODL_DATE      5
#define ODL_DATE_TIME 6
#define ODL_SEQUENCE  7
#define ODL_SET       8


/**************************************************************************/
/*                              Typedefs                                  */
/**************************************************************************/

typedef struct Object_Structure
{
    char *class_name;
    char *pre_comment;  /* Comments before the OBJECT = line     */
    char *line_comment; /* Comments on the OBJECT = line         */
    char *post_comment; /* Comments before the END_OBJECT = line */
    char *end_comment;  /* Comments on the END_OBJECT = line     */
    char *file_name;    /* the file this node was parsed from, or the   */
                        /* file obj is written to by OdlPrintLabel when */
                        /* ODL_EXPAND_* is specified                    */
    char *appl1;   /*  used internally by OAL                           */
    char *appl2;   /*  free for your application to use as you see fit  */
    unsigned short is_a_group;  /* not currently set by parser          */
    unsigned long level;  /* the number of levels below the root, used  */
                          /* by OdlPrintLabel for indentation, etc.     */
    unsigned long line_number;  /* line in the source file when parsed; */
                                /* for accounting purposes only         */
    unsigned long child_count;
    struct Object_Structure *parent;
    struct Object_Structure *left_sibling;
    struct Object_Structure *right_sibling;
    struct Object_Structure *first_child;
    struct Object_Structure *last_child;
    struct Keyword_Structure *first_keyword;
    struct Keyword_Structure *last_keyword;

} OBJDESC;

typedef struct Keyword_Structure
{
    char *name;
    char *file_name;    /* the file this kwd was parsed from, or the    */
                        /* file it is written to by OdlPrintLabel when  */
                        /* ODL_EXPAND_* is specified                    */
    char *value;
    unsigned long size;
    char *pre_comment;   /* Comments before the KEYWORD = line */
    char *line_comment;  /* Comments on the KEYWORD = line     */
    char *appl1;   /*  free for your application to use as you see fit  */
    char *appl2;   /*  free for your application to use as you see fit  */
    unsigned short is_a_pointer;
    unsigned short is_a_list;
    unsigned long line_number;  /* line in the source file when parsed; */
                                /* for accounting purposes only         */
    struct Object_Structure *parent;
    struct Keyword_Structure *left_sibling;
    struct Keyword_Structure *right_sibling;

} KEYWORD;

/**************************************************************************/
/*                         Function Prototypes                            */
/**************************************************************************/

#ifdef _NO_PROTO

OBJDESC *OdlParseLabelFile();
OBJDESC *OdlParseLabelString();
OBJDESC *OdlExpandLabelFile();
static unsigned short ExpandIsRecursive();
OBJDESC *OdlFindObjDesc();
OBJDESC *OdlNextObjDesc();
OBJDESC *OdlCutObjDesc();
OBJDESC *OdlPasteObjDesc();
OBJDESC *OdlPasteObjDescBefore();
OBJDESC *OdlPasteObjDescAfter();
OBJDESC *OdlCopyObjDesc();
OBJDESC *OdlNewObjDesc();
char *OdlGetLabelVersion();
char *OdlGetObjDescClassName();
void OdlGetObjComments();
char *OdlGetObjPreComment();
char *OdlGetObjLineComment();
char *OdlGetObjPostComment();
char *OdlGetObjEndComment();
char *OdlGetObjFilename();
char *OdlGetKwdPreComment();
char *OdlGetKwdLineComment();
char *OdlGetKwdFilename();
void  OdlSetObjComments();
void  OdlSetObjPreComment();
void  OdlSetObjLineComment();
void  OdlSetObjPostComment();
void  OdlSetObjEndComment();
void  OdlSetObjFilename();
void  OdlSetKwdPreComment();
void  OdlSetKwdLineComment();
void  OdlSetKwdFilename();
int OdlGetObjDescChildCount();
int OdlGetObjDescLevel();
OBJDESC *OdlGetObjDescParent();
void OdlAdjustObjDescLevel();
KEYWORD *OdlFindKwd();
KEYWORD *OdlNextKwd ();
KEYWORD *OdlCutKwd();
KEYWORD *OdlPasteKwd();
KEYWORD *OdlPasteKwdBefore();
KEYWORD *OdlPasteKwdAfter();
KEYWORD *OdlCopyKwd();
KEYWORD *OdlNewKwd();
KEYWORD *OdlGetFirstKwd ();
KEYWORD *OdlGetNextKwd ();
char *OdlGetKwdValue();
unsigned short OdlGetKwdValueType();
char *OdlGetKwdUnit();
char *OdlGetKwdName();
char *OdlGetFileName();
char *OdlGetFileSpec();
char *OdlCheckFileSpec();
char *OdlExtractFilename();
OBJDESC *OdlFreeTree();
KEYWORD *OdlFreeAllKwds();
KEYWORD *OdlFreeKwd();
FILE *OdlLocateStart();
FILE *OdlOpenMessageFile();
short OdlPrintMessage();
static char *OdlFormatMessage();
void OdlPrintHierarchy();
int OdlChangeExpansionFile();
void OdlChangeExpandFile();
OBJDESC *OdlExtractExpansionTree();
void OdlPrintLabel();
static void OdlPrintKeywords();
short OdlPrintLine();
OBJDESC *OdlParseFile();
static short OdlNestingLevel();
static short OdlValidBraces();
static short OdlValidElement();
static short OdlValidEndObjDesc();
static short OdlValidIdentifier();
static short OdlValidKwd();
static short OdlValidObjDesc();
static short OdlValidValueList();
OBJDESC *OdlTraverseTree();
char *OdlFirstWord();
char *OdlNextWord();
char *OdlValueStart();
char *OdlValueEnd();
char *OdlValueRowStart();
char *OdlValueRowEnd();
unsigned short OdlDataType();
char *OdlTypeString();
TB_STRING_LIST *OdlGetAllKwdValues();
char *OdlTempFname();
unsigned short OdlWildCardCompare();
short CheckBalance();
char *OdlGetParentDir();

#else

OBJDESC *OdlParseLabelFile (char *, char *, MASK, unsigned short);
OBJDESC *OdlParseLabelString (char *, char *, MASK, unsigned short);
OBJDESC *OdlExpandLabelFile (OBJDESC *, char *, MASK, unsigned short);
static unsigned short ExpandIsRecursive (KEYWORD *, char *);
OBJDESC *OdlFindObjDesc(OBJDESC *, char *, char *, char *, 
                       unsigned long, unsigned short);
OBJDESC *OdlNextObjDesc (OBJDESC *, unsigned long, unsigned short *);
OBJDESC *OdlTraverseTree (OBJDESC *, unsigned long);
OBJDESC *OdlCutObjDesc (OBJDESC *);
OBJDESC *OdlPasteObjDesc (OBJDESC *, OBJDESC *);
OBJDESC *OdlPasteObjDescBefore (OBJDESC *, OBJDESC *);
OBJDESC *OdlPasteObjDescAfter (OBJDESC *, OBJDESC *);
OBJDESC *OdlCopyObjDesc (OBJDESC *);
OBJDESC *OdlNewObjDesc (char *,char *,char *,char *,char *, char *, short, long);
char *OdlGetLabelVersion (OBJDESC *);
char *OdlGetObjDescClassName (OBJDESC *);
void  OdlGetObjComments (OBJDESC *, char **, char **, char **, char **);
char *OdlGetObjPreComment (OBJDESC *);
char *OdlGetObjLineComment (OBJDESC *);
char *OdlGetObjPostComment (OBJDESC *);
char *OdlGetObjEndComment (OBJDESC *);
char *OdlGetObjFilename (OBJDESC *);
char *OdlGetKwdPreComment (KEYWORD *);
char *OdlGetKwdLineComment (KEYWORD *);
char *OdlGetKwdFilename (KEYWORD *);
void  OdlSetObjComments (OBJDESC *, char *, char *, char *, char *);
void  OdlSetObjPreComment (OBJDESC *, char *);
void  OdlSetObjLineComment (OBJDESC *, char *);
void  OdlSetObjPostComment (OBJDESC *, char *);
void  OdlSetObjEndComment (OBJDESC *, char *);
void  OdlSetObjFilename (OBJDESC *, char *);
void  OdlSetKwdPreComment (KEYWORD *, char *);
void  OdlSetKwdLineComment (KEYWORD *, char *);
void  OdlSetKwdFilename (KEYWORD *, char *);
int OdlGetObjDescChildCount (OBJDESC *);
int OdlGetObjDescLevel (OBJDESC *);
OBJDESC *OdlGetObjDescParent (OBJDESC *);
void OdlAdjustObjDescLevel (OBJDESC *);
KEYWORD *OdlFindKwd (OBJDESC *, char *, char *, unsigned long, unsigned short);
KEYWORD *OdlNextKwd (KEYWORD *, char *, char *, unsigned long, unsigned short);
KEYWORD *OdlCutKwd (KEYWORD *);
KEYWORD *OdlPasteKwd (KEYWORD *, OBJDESC *);
KEYWORD *OdlPasteKwdBefore (KEYWORD *, KEYWORD *);
KEYWORD *OdlPasteKwdAfter (KEYWORD *, KEYWORD *);
KEYWORD *OdlCopyKwd (KEYWORD *);
KEYWORD *OdlNewKwd (char *, char *, char *, char *, char *, long);
KEYWORD *OdlGetFirstKwd (OBJDESC *);
KEYWORD *OdlGetNextKwd (KEYWORD *);
char *OdlGetKwdValue (KEYWORD *);
unsigned short OdlGetKwdValueType (KEYWORD *);
char *OdlGetKwdUnit (KEYWORD *);
char *OdlGetKwdName (KEYWORD *);
char *OdlGetFileName (KEYWORD *, unsigned long *, unsigned short *);
char *OdlGetFileSpec (char *, char *, char *);
char *OdlCheckFileSpec (char *, char *);
char *OdlExtractFilename (char *);
OBJDESC *OdlFreeTree (OBJDESC *);
KEYWORD *OdlFreeAllKwds (OBJDESC *);
KEYWORD *OdlFreeKwd (KEYWORD *);
FILE *OdlOpenMessageFile (char *, FILE *);
FILE *OdlLocateStart(char *, unsigned long, unsigned short);
short OdlPrintMessage (char *, FILE *, long, char *);
static char *OdlFormatMessage (char *);
void OdlPrintHierarchy (OBJDESC *, char *, FILE *);
void OdlChangeExpandFile( OBJDESC *, KEYWORD *, char *, char *);
int OdlChangeExpansionFile( KEYWORD *, char *);
OBJDESC *OdlExtractExpansionTree( KEYWORD *);
void OdlPrintLabel (OBJDESC *, char *, FILE *, unsigned long, MASK);
static void OdlPrintKeywords (OBJDESC *, char *, FILE *);
short OdlPrintLine (char *message_fname, FILE *message_fptr, char *text);
OBJDESC *OdlParseFile (char *, FILE *, char *, char *, FILE *, 
                      unsigned short, unsigned short, unsigned short,
                      unsigned short);
static short OdlNestingLevel (char *, long *, long *);
static short OdlValidBraces (char *, long, long, char *, FILE *, long);
static short OdlValidElement (char *, char *, FILE *, long, long);
static short OdlValidEndObjDesc (OBJDESC *, char *, char *, char *, FILE *,long);
static short OdlValidIdentifier (char *, char *, char *, FILE *, long);
static short OdlValidKwd (OBJDESC *, char *, char *, char *, char *,FILE *,long);
static short OdlValidObjDesc (char *, char *, char *, FILE *,long);
static short OdlValidValueList (char *, char *, FILE *,long);
char *OdlFirstWord (char *);
char *OdlNextWord (char *);
char *OdlValueStart (char *);
char *OdlValueEnd (char *);
char *OdlValueRowStart (char *);
char *OdlValueRowEnd (char *);
unsigned short OdlDataType(char *);
char *OdlTypeString (unsigned short, char *);
TB_STRING_LIST *OdlGetAllKwdValues(KEYWORD *);
char *OdlTempFname();
unsigned short OdlWildCardCompare(char *, char *);
short CheckBalance(char *);
char *OdlGetParentDir(char *);

#endif  /* _NO_PROTO  */
                              
/**************************************************************************/
/*                       End of lablib3.h stuff                          */
/**************************************************************************/

#endif  /*  __LABLIB3_LOADED  */

