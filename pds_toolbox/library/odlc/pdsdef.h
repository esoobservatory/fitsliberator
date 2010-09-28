
/**********************************************************************
 * Component                                                          *
 *    Include file pdsdef.h                                           *
 * Used By                                                            *
 *    PDS software.                                                   *
 * Detailed Description                                               *
 *    Defines generic symbols, macros, flags, typedefs, and           *
 *    structures used by PDS software.                                *
 * Author and Institution                                             *
 *    David P. Bernath / J.P.L.                                       *
 * Version and Date                                                   *
 *    2.7   August 28, 1992                                           *
 * Change History                                                     *
 *    DPB   07-05-90   Original code.                                 *
 *    DPB   08-17-90   Added symbols and typedefs that were removed   *
 *                     from utildef, convertdef, and errordef.        *
 *    MDD   09-20-90   Made changes for SUN C compatibility           *
 *    DPB   09-26-90   Added new symbols for prog_make_tree.          *
 *    DPB   10-04-90   Added PDS_LINE_LENGTH.                         *
 *    DPB   10-12-90   Added OPTIONS and COMBINED_TREE_INFO typedefs. *
 *    DPB   10-25-90   Added a file_name field to the tree_info       *
 *                     typedef.                                       *
 *    DPB   02-04-91   Added PDS_TEMP_LABEL_FNAME.                    *
 *    DPB   03-13-91   Added PDS_DEFAULT_LOGIN and PDS_SETUP_DIRECTORY*
 *                     and changed PDS_BROWSER_SETUP_FNAME to         *
 *                     PDS_TOOLBOX_SETUP_FNAME.                       *
 *    MDD   03-14-91   Added symbols used by DD index routines.       *
 *    DPB   03-27-91   Added PDS error symbols.                       *
 *    MDD   04-05-91   Added ddaccess.h and more DD symbols           *
 *    DPB   04-10-91   Added label status symbols.                    *
 *    MDD   04-15-91   Changed names of lib include files             *
 *    DPB   05-13-91   Added verkey.h, verobj.h, and verlabel.h       *
 *    DPB   06-17-91   Added PDS_MAX_ERRORS.                          *
 *    KLM   06-27-91   Added PDS_MAXRECORD.                           *
 *    DPB   09-27-91   Added a whole slew of new macros and symbols,  *
 *                     removed all browser-specific stuff, and        *
 *                     re-organized symbols and typedefs by file.     *
 *    MDD   10-10-91   Added pointer_Info structure                   *
 *    MDD   10-16-91   Updated index_entry structure for PC           *
 *    MDD   10-17-91   Added Check_Malloc macro and remove references *
 *                     to sys_exit_system.                            *
 *    DPB   10-23-91   Changed PDS_RF_FIXED_NOCR to PDS_RF_BINARY,    *
 *                     and added a bunch of new macros.               *
 *    DPB   05-19-92   Added NULL definition and a gaggle of UNSIGNED *
 *                     macros.                                        *
 *    MDD   05-20-92   Added stuff for FFTOOL                         *
 *    MDD   08-28-92   Added stuff for lab_attach                     *
 *    DWS   12-17-97   Changed PDS_MAX_ERRORS from 150 to 300         *
 *    MDC   04-07-04   Changed PDS_ELEMENT_LEN from 30 to 60          *
 *    MDC   01-06-05   Added MACRO for pdsdd.full                     *
 *    MDC   01-06-05   Added MACRO for .idx, DD_IDX_EXT               *
 *    MDC   12-08-05   Changed PDS_ELEMENT_LEN from 60 to 61          *
 **********************************************************************/

#ifndef PDSDEF
#define PDSDEF


#ifdef MSDOS_TC
#include <time.h>                                                                 /*10-29-97*/
#endif
#include <sys/types.h>                                                            /*10-15-97*/
#include <sys/stat.h>


#include <stdio.h>

#ifdef MSDOS_TC
#include <windows.h>                                                                              /*10-15-97*/
#endif

#include <ctype.h>
#include "odldef.h" 
typedef int LOGICAL;

#ifndef SUN_UNIX 
typedef unsigned long  u_long;                                 /*10-9-97*/
typedef unsigned short u_short;                                /*10-9-97*/
typedef unsigned char  u_char;                                 /*10-9-97*/
#endif

#define FA_DIREC        0x10                                   /*10-10-97*/
#ifdef MSDOS_TC


#include <string.h>
#include <stdlib.h>
#include <direct.h>                                                                                        /*10-13-97*/
#endif

#ifdef VAX
#include <stat.h>
#endif

#ifdef SUN_UNIX
#include "time.h"
#endif

#ifdef MAC_THINK
#endif

#ifdef OSX
#endif

/*--------------------------------------------------------------------*/
/*                    General Symbol Definitions                      */
/*--------------------------------------------------------------------*/

#define EOS                  0      /*  Your basic end-of-string.     */
#define FALSE                0      /*  I'll let you guess.           */
#define TRUE                 1      /*  Ditto.                        */
#define NUL                  0      /*  ASCII zero character.         */
#define PDS_BUFFLEN        512      /*  Length of a read buffer.      */
#define PDS_MAX_BITS        64      /*  Number of bits in a long int  */
#define PDS_MAX_ERRORS     300      /*  Number of errors allowed.     */
#define PDS_LINE_LENGTH     60      /*  Length of line displayed in   */
				    /*  interface window.             */
#define PDS_MAXLINE        257      /*  Length of a line read from    */
				    /*  an input file.                */
#define PDS_MAXRECORD     2048      /*  Length of a record read from  */
				    /*  a label file.                 */
#define PDS_MAX_SANE_RECORD 33000L  /*  Length of longest possible    */
				    /*  data record                   */

#define PDS_DEFAULT_RECLEN  80      /* default record length for fixed*/
				    /* length label records           */

/* 04-12-06 MDC - Added macro definition. The max length a keyword can be */
#define PDS_MAX_KWD_LENGTH   30 

#define MAX_PATH_LEN 256
#define PDS_NULL             "NULL"
#define PDS_UNKNOWN          "UNK"
#define PDS_NOT_APPLICABLE   "N/A"
#define PDS_TBD              "TBD"

#define PDS_NULL_DATE        "NULL:"
#define PDS_UNKNOWN_DATE     "UNK:"
#define PDS_NOT_APP_DATE     "N/A:"

#define PDS_LABEL_PAD           40      /* safety pad for fixed length   */
				    /* labels                        */
#define PDS_MAX_POINTERS        40  /* Maximum number of pointers    */

#ifdef LV_DDICT
#define DDICT_KW_ARRAY_SIZE 1000000  /* size of keword array            */
#ifndef LV_KWVTOOL
#define DDICT_VAL_STR_LEN   100     /* size of second element in array */
#else
#define DDICT_VAL_STR_LEN   150     /* size of second element in array */
#endif
char    Sarray[DDICT_KW_ARRAY_SIZE][DDICT_VAL_STR_LEN];  /* 100000 and 100, 150 for kwvtool */
#endif
				    /* in a PDS label root object    */

/*--------------------------------------------------------------------*/
/*                        General typedefs                            */
/*--------------------------------------------------------------------*/

	/*------------------------------------------------------------*/
	/*  The LOGICAL typedef is used for routines and variables    */
	/*  that may return, or take on, the values TRUE or FALSE.    */
	/*------------------------------------------------------------*/

/*typedef int LOGICAL;*/                                     /*10-9-97*/

/*--------------------------------------------------------------------*/
/*            Symbols and typedefs used by:  DDACCESS.C               */
/*--------------------------------------------------------------------*/

/* 12-08-05 MDC - Changed PDS_ELEMENT_LEN to 61 instead of 60 since we can have
      local data dictionary keywords of 61 max characters (30 for the namespace,
	  30 for the element, and 1 for the colon).
*/
#define PDS_ELEMENT_LEN                 61
#define PDS_DD_TYPE_LEN                 30
#define PDS_DD_NO_MINIMUM               2147483647L
#define PDS_DD_NO_MAXIMUM               -2147483647L
#define DD_REPLACE_DEFS                 0
#define DD_SAVE_DEFS                    1

#ifdef LV_DDICT                          /* for data dictionary instead of lvtool*/
#define DD_WRITE_DEFS_TO_FILE           2
#endif

#define PDS_DEFAULT_DD                  "pdsdd.idx"
#define PDS_DEFAULT_DD_FULL             "pdsdd.full"
#define DD_IDX_EXT                      ".idx"
#define DD_GENERIC_OBJECT               1
#define DD_SPECIFIC_OBJECT              2


/*--------------------------------------------------------------------*/
/*            Symbols and typedefs used by:  ERRORLIB.C               */
/*--------------------------------------------------------------------*/

#define INFO                 0 
#define WARNING              1      
#define ERROR1               2
#define FATAL_ERROR          3
#define CONTINUE             4
#ifdef SUN_UNIX                                          /*10-21-98*/
#define ERROR                2                           /*10-21-98*/
#endif                                                   /*10-21-98*/

	/*------------------------------------------------------------*/
	/*  The ERROR_LIST typedef is used to form a doubly linked    */
	/*  list of error messages.                                   */
	/*------------------------------------------------------------*/

typedef struct error_list 
{
    char *message;
    int severity;
    struct error_list *next;
    struct error_list *prev;
	
} ERROR_LIST;

/*--------------------------------------------------------------------*/
/*              Symbols and typedefs used by:  FIOLIB.C               */
/*--------------------------------------------------------------------*/

#define PDS_RF_UNKNOWN                  0
#define PDS_RF_STREAM_LF                1
#define PDS_RF_STREAM_CR                2
#define PDS_RF_STREAM_CRLF              3
#define PDS_RF_FIXED_LF                 4
#define PDS_RF_FIXED_CR                 5
#define PDS_RF_FIXED_CRLF               6
#define PDS_RF_BINARY                   7
#define PDS_RF_RMS_STREAM               8
#define PDS_RF_RMS_VAR                  9
#define PDS_RF_ASCII                   10

#ifdef MSDOS_TC
#define DEFAULT_REC_TYPE    PDS_RF_STREAM_CRLF
#endif

#ifdef VAX
#define DEFAULT_REC_TYPE    PDS_RF_STREAM_LF
#endif

#ifdef SUN_UNIX
#define DEFAULT_REC_TYPE    PDS_RF_STREAM_LF
#endif

#ifdef MAC_THINK
#define DEFAULT_REC_TYPE    PDS_RF_STREAM_CR
#endif

#ifdef OSX
#define DEFAULT_REC_TYPE	PDS_RF_STREAM_LF
#endif


/*--------------------------------------------------------------------*/
/*               Symbols and typedefs used by:  LABEL.C               */
/*--------------------------------------------------------------------*/

#define PDS_ERROR               0   /* Routine unable to perform task */
#define PDS_SUCCESS             1   /* Routine executed successfully  */
#define PDS_MULTIPLE_OBJECTS    2   /* Ambiguous reference to an object */
#define PDS_MULTIPLE_PARMS      3   /* Ambiguous reference to a parm. */
#define PDS_WARNING             4   /* Routine successful, check messages */

/*--------------------------------------------------------------------*/
/*              Symbols and typedefs used by:  SEARCHLIB.C            */
/*--------------------------------------------------------------------*/

#define STOP_SEARCH_FLAG        -1
#define PDS_SEARCH_FAIL         -1

/*--------------------------------------------------------------------*/
/*               Symbols and typedefs used by:  UTILLIB.C             */
/*--------------------------------------------------------------------*/

#define STRING_TYPE          0      /*  Append a character string.    */
#define LIST_TYPE            1      /*  Append a STRING_LIST.         */
				    
	/*------------------------------------------------------------*/
	/*  The STRING_LIST typedef is used to form doubly linked     */
	/*  lists of character strings.                               */
	/*------------------------------------------------------------*/

typedef struct string_list
{
    char *text;
    struct string_list *next;
    struct string_list *prev;
	
} STRING_LIST;


	/*------------------------------------------------------------*/
	/*  The POINTER_INFO typedef is used to store a single PDS    */
	/*  pointer value from a PDS label.                           */
	/*------------------------------------------------------------*/

typedef struct
{
   char name [PDS_MAXLINE + 1];
   long location;
   char file_name [PDS_MAXLINE + 1];
   LOGICAL is_attached;
   LOGICAL has_byte_loc;
} POINTER_INFO;



/*--------------------------------------------------------------------*/
/*       Symbols and typedefs used by:  Toolbox setup routines        */
/*--------------------------------------------------------------------*/

#define PDS_SCRATCH_DIR_KEYWORD      "SCRATCH_DIR"
#define PDS_HELP_DIR_KEYWORD         "HELP_DIR"

#ifdef VAX
#define PDS_DEFAULT_LOGIN            "SYS$LOGIN"

#ifndef PDS_SETUP_DIRECTORY 
#define PDS_SETUP_DIRECTORY          "SYS$LIBRARY"
#endif

#ifndef PDS_SYSTEM_SCRATCH
#define PDS_SYSTEM_SCRATCH           "SYS$SCRATCH"
#endif

#endif

#ifdef SUN_UNIX
#define PDS_DEFAULT_LOGIN            (char *)getenv("HOME")

#ifndef PDS_SETUP_DIRECTORY 
#define PDS_SETUP_DIRECTORY          "/usr/lib"
#endif

#ifndef PDS_SYSTEM_SCRATCH
#define PDS_SYSTEM_SCRATCH           "/usr/tmp"
#endif

#endif

#ifdef MSDOS_TC
#define PDS_DEFAULT_LOGIN            "c:\\"

#ifndef PDS_SETUP_DIRECTORY 
#define PDS_SETUP_DIRECTORY          ""
#endif

#ifndef PDS_SYSTEM_SCRATCH
#define PDS_SYSTEM_SCRATCH           ""
#endif

#endif

#ifdef MAC_THINK
#define PDS_DEFAULT_LOGIN            ""

#ifndef PDS_SETUP_DIRECTORY 
#define PDS_SETUP_DIRECTORY          ""
#endif

#ifndef PDS_SYSTEM_SCRATCH
#define PDS_SYSTEM_SCRATCH           ""
#endif
#endif

#ifdef OSX
#define PDS_DEFAULT_LOGIN            (char *)getenv("HOME")

#ifndef PDS_SETUP_DIRECTORY 
#define PDS_SETUP_DIRECTORY          ""
#endif

#ifndef PDS_SYSTEM_SCRATCH
#define PDS_SYSTEM_SCRATCH           "/tmp"
#endif
#endif

#define PDS_TOOLBOX_SETUP_FNAME      "pdstool.set"

/*--------------------------------------------------------------------*/
/*                       Macro Definitions                            */
/*--------------------------------------------------------------------*/

	/*------------------------------------------------------------*/
	/*  The Close_Me macro closes a file and sets its pointer to  */
	/*  NULL.                                                     */
	/*------------------------------------------------------------*/

#define Close_Me(zfilez) \
    { if (zfilez != NULL) \
       fclose(zfilez); \
       zfilez = NULL; \
    }

	/*------------------------------------------------------------*/
	/*  The IsNull macro will return TRUE is the string passed to */
	/*  it is PDS_NULL or PDS_NOT_APPLICABLE, and FALSE otherwise.*/
	/*------------------------------------------------------------*/

#define IsNull(x)  \
        ((x) != NULL && ((strcmp((x),PDS_NULL)==0)||\
        (strcmp((x),PDS_NOT_APPLICABLE)==0)||\
        (strcmp((x),PDS_TBD)==0)||(strcmp((x),PDS_UNKNOWN)==0)))

	/*------------------------------------------------------------*/
	/*  The IsNull macro will return TRUE is the string passed to */
	/*  it is PDS_NULL or PDS_NOT_APPLICABLE, and FALSE otherwise.*/
	/*------------------------------------------------------------*/

#define IsNullDate(x)\
	    ((x) != NULL && ((IsNull(x))||(strcmp((x),PDS_NULL_DATE)==0)||\
	    (strcmp((x),PDS_NOT_APP_DATE)==0)||(strcmp((x),PDS_TBD)==0)||\
	    (strcmp((x),PDS_UNKNOWN_DATE)==0)))

	/*------------------------------------------------------------*/
	/*  The IsOdd macro returns TRUE if an integer is odd and     */
	/*  FALSE if it is even.                                      */
	/*------------------------------------------------------------*/

#define IsOdd(zintz) ((zintz%2) == 1)

	/*------------------------------------------------------------*/
	/*  The Lemme_Go macro frees the memory pointed to by the     */
	/*  input pointer, if it is not NULL.                         */
	/*------------------------------------------------------------*/

#define Lemme_Go(ptr) { if (ptr != NULL) free (ptr); ptr = NULL; }

	/*------------------------------------------------------------*/
	/*  The Make_Long macro is designed for use with lab routines.*/
	/*  It returns a 0 if the string passed in is NULL, or the    */
	/*  long integer representation of the string otherwise.      */ 
	/*------------------------------------------------------------*/

#define Make_Long(x) ((x == NULL) ? 0 : atol (x))

	/*------------------------------------------------------------*/
	/*  The Make_Long_Str macro is designed for use with lab      */
	/*  routines.  If the integer passed in is 0, then the string */
	/*  UNKNOWN is assigned to the string array. Otherwise, the   */
	/*  string representation of the long number x is copied into */
	/*  the array.  The string space must be pre-allocated.       */
	/*------------------------------------------------------------*/

#define Make_Long_Str(str,x) \
        { \
        if (x == 0)\
		  strcpy (str, "UNKNOWN"); \
        else \
           sprintf (str, "%ld", (x));\
        }

	/*------------------------------------------------------------*/
	/*  The Make_Str_Str macro is designed for use with lab       */
	/*  routines.  If str2 is NULL, then str1 is assigned the     */
	/*  value UNKNOWN.  Otherwise, str2 is copied to str1. The    */
	/*  space for str1 must be pre-allocated.                     */
	/*------------------------------------------------------------*/

#define Make_Str_Str(str1,str2) \
        {\
            if (str2 == NULL) \
                 strcpy (str1, "UNKNOWN"); \
            else \
                 strcpy (str1, str2); \
        }

	/*------------------------------------------------------------*/
	/*  The Exit_System macro exits with the exit value that      */
	/*  applies to the current environment.                       */
	/*------------------------------------------------------------*/

#ifdef VAX
#define Exit_System() \
        { \
         printf ("FATAL ERROR: Something awful happened. I'm leaving!\n");\
         exit(1);\
        }
#else
#define Exit_System() \
            {\
			printf ("FATAL ERROR: Something awful happened. I'm leaving!\n");\
            exit(0); \
            }
#endif

	/*------------------------------------------------------------*/
	/*  The String_End macro calculates the address of the last   */
	/*  character in a string.  PLEASE NOTE THAT IF THE STRING IS */
	/*  EMPTY, THIS MACRO WILL RETURN A -1.                       */
	/*------------------------------------------------------------*/

#define String_End(zxz) ((zxz+strlen(zxz)-sizeof(char)))

	/*------------------------------------------------------------*/
	/*  The String_Size macro calculates the number of characters */
	/*  in a string (including the character position used by the */
	/*  EOS character.  This can be used whenever it is necessary */
	/*  to malloc storage for a character string, for example:    */
	/*  new_str = malloc (String_Size(old_str));                  */
	/*------------------------------------------------------------*/

#define String_Size(zxz) (strlen(zxz)+sizeof(char)) 

	/*------------------------------------------------------------*/
	/*  The Check_Malloc macro checks to given pointer to see if  */
	/*  it is NULL. If it is, the macro prints an error message   */
	/*  and exits.                                                */
	/*------------------------------------------------------------*/

#define Check_Malloc(zptrz) if (zptrz == NULL) \
          { \
           printf ("FATAL ERROR: Program is out of memory.\n");\
           Exit_System();\
          }
	 
	/*------------------------------------------------------------*/
	/*  The Malloc_String macro allocates space for a string.  If */
	/*  no space is available, the program exits.  The first      */
	/*  character of the string is initialized to EOS.            */
	/*------------------------------------------------------------*/

#define Malloc_String(zstrz, zlenz) \
           { \
             int znumz = {zlenz};\
             if (znumz <= 0) znumz = 1;\
             zstrz = (char *) malloc(znumz);\
             Check_Malloc(zstrz);\
             *zstrz = EOS;\
           }

	/*------------------------------------------------------------*/
	/*  The Realloc_String macro re-allocates space for a string. */
	/*  If no space is available, the program exits.              */
	/*------------------------------------------------------------*/

#define Realloc_String(zstrz, zlenz) \
           {\
             if ((zstrz != NULL) && (zlenz > strlen(zstrz)))\
               {\
               zstrz = (char *) realloc(zstrz, zlenz);\
               Check_Malloc(zstrz)\
               }\
           }

	/*------------------------------------------------------------*/
	/*  The Read_Char macro loops until a RETURN is pressed and   */
	/*  returns the last character entered.                       */
	/*------------------------------------------------------------*/

#define Read_Char(zcharz) \
         {\
            short zcz = {NULL};\
            zcharz = NULL;\
            while ((zcz = getchar()) != '\n')zcharz = zcz;\
         }

	/*------------------------------------------------------------*/
	/*  The New_String macro allocates storage for a string, then */
	/*  copies it into the newly allocated space.  It does not    */
	/*  free the string pointer first.                            */
	/*------------------------------------------------------------*/

#define New_String(zoldz, znewz)\
         {\
           if (znewz == NULL) \
             Malloc_String(zoldz, 1)\
           else\
             {\
             Malloc_String(zoldz, String_Size(znewz))strcpy (zoldz, znewz);\
             }\
         }

	/*------------------------------------------------------------*/
	/*  The Replace_String macro frees the old value of a string, */
	/*  allocates space for a new one, and copies it into the new */
	/*  space.                                                    */
	/*------------------------------------------------------------*/

#define Replace_String(zoldz, znewz){ Lemme_Go(zoldz) New_String(zoldz, znewz)}

	/*------------------------------------------------------------*/
	/*  The Strip_Trailing macro remove all occurrences of the    */
	/*  character passed in from the end of the string passed in. */
	/*------------------------------------------------------------*/

#define Strip_Trailing(zstrz, zcharz)\
         {\
            char *zcz = {NULL};\
            if (zstrz != NULL)\
               {\
               for (zcz=String_End(zstrz); ((zcz >= zstrz) && (*zcz == zcharz)); --zcz) ;\
               *(++zcz) = EOS;\
               }\
         }

	/*------------------------------------------------------------*/
	/*  The Vms_Error macro will return true if an error or fatal */
	/*  error code is passed to it, and false otherwise.          */
	/*------------------------------------------------------------*/

#define Vms_Error(x)  (!((x)&1))

	/*------------------------------------------------------------*/
	/*  The Flush_File macro opens a file for writing and then    */
	/*  closing it, effectively creating an empty copy of it      */
	/*------------------------------------------------------------*/

#define Flush_File(zfilez)  (fclose(fopen((zfilez),"w")) == 0)

	/*------------------------------------------------------------*/
	/*  These are UNSIGNED versions of some of the above macros   */
	/*------------------------------------------------------------*/

#define U_Malloc_String(zstrz, zlenz)\
        {\
            int znumz = {zlenz};\
            if (znumz <= 0) znumz = 1;\
            zstrz = (unsigned char *) malloc(znumz);\
            Check_Malloc(zstrz);\
            *zstrz = EOS;\
        }

#define U_New_String(zoldz, znewz) \
        {\
           if (znewz == NULL) U_Malloc_String(zoldz, 1)\
           else \
              {\
              U_Malloc_String(zoldz, String_Size(znewz)) strcpy (zoldz, znewz);\
              }\
        }

#define U_Replace_String(zoldz, znewz) { Lemme_Go(zoldz) U_New_String(zoldz, znewz) }

#define U_Strip_Trailing(zstrz, zcharz) \
        { \
        unsigned char *zcz = {NULL}; \
        if (zstrz != NULL) \
          { \
            for (zcz=String_End(zstrz); ((zcz >= zstrz) && (*zcz == zcharz)); --zcz) ; \
			*(++zcz) = EOS; \
          } \
        }

/*--------------------------------------------------------------------*/
/*                        End:  "pdsdef.h"                            */
/*--------------------------------------------------------------------*/

#endif

