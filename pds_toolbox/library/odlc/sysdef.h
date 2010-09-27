/**********************************************************************
 * Component                                                          *
 *    Include file sysdef.h                                           *
 * Used By                                                            *
 *    PDS syslib software.                                            *
 * Detailed Description                                               *
 *    Defines symbols, macros, flags, typedefs, and structures        *
 *    used by PDS syslib software.                                    *
 * Author and Institution                                             *
 *    Herbert C. Gamble / J.P.L.                                      *
 *    David P. Bernath / J.P.L.                                       *
 * Version and Date                                                   *
 *    3.0   April 21, 1992                                            *
 * Change History                                                     *
 *    HCG   09-01-90   Original code.                                 *
 *    DPB   10-04-90   Added header, and added sys_exit_system.       *
 *    HCG   10-23-90   Deleted symbol definitions.                    *
 *    DPB   07-09-91   Added sys_get_path.                            *
 *    MDD   03-23-92   Modified function prototypes                   *
 *    MDD   04-16-92   Removed unused functions              
 **********************************************************************/ 

/*---------------------------------------------------------------------------*/
/*                       SYSLIB Include Statements                           */
/*---------------------------------------------------------------------------*/

#ifdef VAX
#include <descrip>
#include <processes>
#endif

#ifdef MSDOS_TC
#include <process.h>
#endif

#ifdef SUN_UNIX
#endif

#ifdef MAC_THINK
#endif

#ifdef OSX
#include <time.h>
#endif

/*---------------------------------------------------------------------------*/
/*                       SYSLIB Function Prototypes                          */
/*---------------------------------------------------------------------------*/

#ifndef SUN_UNIX

LOGICAL      sys_copy_file (char *, char *);
LOGICAL      sys_do_command (char *);
LOGICAL      sys_delete_file (char *);
char        *sys_get_ascii_date (void);
struct tm   *sys_get_date (void);
STRING_LIST *sys_get_file_list (char *);
char        *sys_get_path (char *);
char        *sys_make_temp_fname (char *);

#else

LOGICAL      sys_copy_file ();
LOGICAL      sys_delete_file ();
LOGICAL      sys_do_command ();
char        *sys_get_ascii_date ();
struct tm   *sys_get_date ();
STRING_LIST *sys_get_file_list ();
char        *sys_get_path ();
char        *sys_make_temp_fname ();

#endif

