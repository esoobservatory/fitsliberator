/**********************************************************************
 * Component                                                          *
 *    Include file errordef.h                                         *
 * Used By                                                            *
 *    PDS ERRORLIB software.                                          *
 * Detailed Description                                               *
 *    Defines symbols, macros, flags, typedefs, and structures        *
 *    used by PDS ERRORLIB software.                                  *
 * Author and Institution                                             *
 *    David P. Bernath / J.P.L.                                       *
 * Version and Date                                                   *
 *    1.7   March 23, 1992                                            *
 * Change History                                                     *
 *    DPB   07-05-90   Original code.                                 *
 *    DPB   08-17-90   Removed all symbols and typedefs.              *
 *    DPB   08-29-90   Added err_append_stderr_messages prototype.    *
 *    DPB   08-31-90   Added the non-ANSI standard ``#ifdef".         *
 *    MDD   03-14-91   Added err_write_to_stdout                      *
 *    KLM   03-25-91   Added err_write_to_file                        *
 *    DPB   06-12-91   Added err_object_message                       *
 *    MDD   03-23-92   Modified function prototypes                   *
 **********************************************************************/

/*--------------------------------------------------------------------*/
/*                    ERRORLIB Function Prototypes                    */
/*--------------------------------------------------------------------*/

    /*----------------------------------------------------------------*/
    /*  These are the function prototypes that are used by all ANSI   */
    /*  standard C compilers.  At this time, SUN systems running UNIX */
    /*  do not allow this type of prototype declaration.              */
    /*----------------------------------------------------------------*/

#ifndef SUN_UNIX

void err_append_message (int, char *);
void err_deallocate_list (ERROR_LIST *);
void err_deallocate_message (ERROR_LIST *);
LOGICAL err_keyword_message (int, char *, long, long, char *);
LOGICAL err_object_message (int, char *, long, char *);
void err_write_to_file (FILE *, LOGICAL);

    /*----------------------------------------------------------------*/
    /*  These are the function prototypes that are used by C          */
    /*  compilers that do not follow the ANSI standard for the        */
    /*  declaration of function prototypes.                           */
    /*----------------------------------------------------------------*/

#else

void err_append_message ();
void err_deallocate_list ();
void err_deallocate_message ();
LOGICAL err_keyword_message ();
LOGICAL err_object_message ();
void err_write_to_file ();

#endif

extern ERROR_LIST *pds_message_list;
extern ERROR_LIST *pds_last_message;
extern LOGICAL pds_display;
extern long pds_error_count;



/*--------------------------------------------------------------------*/
/*                       End:  "errordef.h"                           */
/*--------------------------------------------------------------------*/
    







