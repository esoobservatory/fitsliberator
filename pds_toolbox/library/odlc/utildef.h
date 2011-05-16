/**********************************************************************
 * Component                                                          *
 *    Include file utildef.h                                          *
 * Used By                                                            *
 *    PDS UTILLIB software.                                           *
 * Detailed Description                                               *
 *    Defines symbols, macros, flags, typedefs, and structures        *
 *    used by PDS UTILLIB software.                                   *
 * Author and Institution                                             *
 *    David P. Bernath / J.P.L.                                       *
 * Version and Date                                                   *
 *    1.5   March 16, 1992                                            *
 * Change History                                                     *
 *    DPB   07-05-90   Original code.                                 *
 *    DPB   08-09-90   Added util_find_non_blank_char, util_locate_   *
 *                     string, and util_locate_substring.  Also added *
 *                     the STRING_LIST typedef and symbols.           *
 *    DPB   08-17-90   Removed all symbols and typedefs.              *
 *    DPB   08-31-90   Added the non-ANSI standard ``#ifdef".         *
 *    DPB   10-04-90   Added util_format_string and util_last_word.   *
 *    MDD   03-16-92   The great int -> long conversion.              *
 *                     Modified function prototypes                   *
 **********************************************************************/ 

/*--------------------------------------------------------------------*/
/*                     UTILLIB Function Prototypes                    */
/*--------------------------------------------------------------------*/

    /*----------------------------------------------------------------*/
    /*  These are the function prototypes that are used by all ANSI   */
    /*  standard C compilers.  At this time, SUN systems running UNIX */
    /*  do not allow this type of prototype declaration.              */
    /*----------------------------------------------------------------*/

#ifndef SUN_UNIX

char *util_byte_copy (char *, char *, long);
char *util_clean_up_string (char *);
char *util_compress_char (char *, char);
char *util_create_file_spec (char *, char *);
char *util_find_non_blank_char (char *);
char *util_format_string (char *, long);
LOGICAL util_is_upper (char *);
char *util_last_word (char *, long);
char *util_locate_string (char *, char *);
char *util_locate_substring (char *, char *);
char *util_lower_case (char *);
char *util_remove_char (char *, char);
char *util_replace_char (char *, char, char);
char *util_replace_formatters (char *);
char *util_save_to_last_occurrence (char *, char);
char *util_strip_lead_and_trail (char *, char);
char *util_strip_to_char (char *, char);
char *util_upper_case (char *);
LOGICAL util_string_is_empty (char *);
STRING_LIST *util_append_string_list (STRING_LIST *, char *, int);
STRING_LIST *util_deallocate_string_list (STRING_LIST *);
STRING_LIST *util_format_string_list (STRING_LIST *, char *, long);

    /*----------------------------------------------------------------*/
    /*  These are the function prototypes that are used by C          */
    /*  compilers that do not follow the ANSI standard for the        */
    /*  declaration of function prototypes.                           */
    /*----------------------------------------------------------------*/

#else

char *util_byte_copy ();
char *util_clean_up_string ();
char *util_compress_char ();
char *util_create_file_spec ();
char *util_find_non_blank_char ();
char *util_format_string ();
LOGICAL util_is_upper ();
char *util_last_word ();
char *util_locate_string ();
char *util_locate_substring ();
char *util_lower_case ();
char *util_remove_char ();
char *util_replace_char ();
char *util_replace_formatters ();
char *util_save_to_last_occurrence ();
char *util_strip_lead_and_trail ();
char *util_strip_to_char ();
char *util_upper_case ();
LOGICAL util_string_is_empty ();
STRING_LIST *util_append_string_list ();
STRING_LIST *util_deallocate_string_list ();
STRING_LIST *util_format_string_list ();

#endif

/*--------------------------------------------------------------------*/
/*                        End:  "utildef.h"                           */
/*--------------------------------------------------------------------*/

