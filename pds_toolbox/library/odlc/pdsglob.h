/**********************************************************************
 * Component                                                          *
 *    Include file pdsglob.h                                          *
 * Used By                                                            *
 *    PDS software.                                                   *
 * Detailed Description                                               *
 *    Defines global variables used by PDS software.                  *
 * Author and Institution                                             *
 *    David P. Bernath / J.P.L.                                       *
 * Version and Date                                                   *
 *    2.0   September 27, 1991                                        *
 * Change History                                                     *
 *    DPB   08-17-90   Original code.                                 *
 *    DPB   08-23-90   Removed pds_save_stdout and pds_save_stderr,   *
 *                     and added pds_stdout_redirected and            *
 *                     pds_stderr_redirected.                         *
 *    MDD   09-06-90   Added sfdu_class_map                           *
 *    DPB   10-11-90   Added pds_object_def_fname, and made all file  *
 *                     file names char*.                              *
 *    HCG   10-12-90   Added pds_scratch_directory,                   *
 *                     pds_help_files_directory, pds_exe_directory,   *
 *                     and pds_setup_file_directory.                  *
 *    DPB   02-04-91   Added pds_temp_label_fname.                    *
 *    DPB   03-13-91   Removed all of the directory variables and     *
 *                     added pds_setup_fname.                         *
 *    MDD   05-01-91   Removed unecessary globals                     *
 *    DPB   06-13-91   Added pds_display, pds_verbose, and            *
 *                     pds_error_count.                               *
 *    DPB   07-25-91   Added pds_generic_class.                       *
 *    DPB   09-27-91   Removed pds_out_of_memory and all browser-     *
 *                     specific variables.                            *
 *	  MDC	12-26-02   Added pds_no_warning variable to indicate	  *
 *					   whether warning messages should be displayed.  *
 **********************************************************************/
#include "pdsdef.h"

        /*------------------------------------------------------------*/
        /*  The pds_display variable is a flag which indicates        */
        /*  that messages appended onto the global message list       */
        /*  should also be displayed to the user's terminal.          */
        /*------------------------------------------------------------*/

LOGICAL pds_display = {FALSE};

        /*------------------------------------------------------------*/
        /*  The pds_error_count variable is a running count of the    */
        /*  number of errors encountered during processing.           */
        /*------------------------------------------------------------*/

long pds_error_count = {0};

	/*------------------------------------------------------------*/
	/*  The pds_generic_class variable is a flag which            */
	/*  indicates whether or not pds label software should use    */
	/*  an object's generic class name or it's specific class.    */
	/*------------------------------------------------------------*/

LOGICAL pds_generic_class = {FALSE};

	/*------------------------------------------------------------*/
	/*  The pds_last_message variable points to the last          */
	/*  structure on a list of ERROR_LIST structures.             */
	/*------------------------------------------------------------*/

ERROR_LIST *pds_last_message = {NUL};

	/*------------------------------------------------------------*/
	/*  The pds_message_list variable points to the first         */
	/*  structure on a list of ERROR_LIST structures.             */
	/*------------------------------------------------------------*/

ERROR_LIST *pds_message_list = {NUL};

	/*------------------------------------------------------------*/
	/*  The pds_scratch_directory variable contains the directory */
	/*  name to be used for creating scratch files.               */
	/*------------------------------------------------------------*/

char *pds_scratch_directory = {NULL};

	/*------------------------------------------------------------*/
	/*  The pds_temp_data_fname variable contains the name of the */
	/*  file used to hold temporary data.                         */
	/*------------------------------------------------------------*/

char *pds_temp_data_fname = {NULL};

	/*------------------------------------------------------------*/
	/*  The pds_temp_label_fname variable contains the name of    */
	/*  the temporary file used when a VMS variable length record */
	/*  file is converted to a fixed file.                        */
	/*------------------------------------------------------------*/

char *pds_temp_label_fname = {NULL};

	/*------------------------------------------------------------*/
	/*  The pds_verbose variable is a flag which indicates        */
	/*  whether or not all messages should be appended onto the   */
	/*  global message list.                                      */
	/*------------------------------------------------------------*/

LOGICAL pds_verbose = {TRUE};

	/*------------------------------------------------------------*/
	/* 12-26-02 MDC												  */
	/* The pds_warning variable is a flag which indicates		  */
	/* whether or not warning messages should be displayed.		  */
	/*------------------------------------------------------------*/
LOGICAL pds_warning = {TRUE};

	/*------------------------------------------------------------*/
	/* 02-13-03 MDC												  */
	/* The pds_info variable is a flag which indicates whether    */
	/* or not info messages should be displayed.				  */
	/*------------------------------------------------------------*/
LOGICAL pds_info = {TRUE};

	/*------------------------------------------------------------*/
	/*  The pds_finish_label variable is a flag which indicates   */
	/*  whether or not an end statement should be placed at the   */
	/*  end of a label                                            */
	/*------------------------------------------------------------*/

LOGICAL pds_finish_label = {TRUE};

	/*------------------------------------------------------------*/
	/*  The pds_watch_ends variable is a flag which indicates     */
	/*  whether or not the ODL parser should issue a warning when */
	/*  a label terminates without an end statement.              */
	/*------------------------------------------------------------*/

LOGICAL pds_watch_ends = {TRUE};

	/*------------------------------------------------------------*/
	/*  The pds_help_width variable indicates the number of       */
	/*  columns that should be used when formatting help text     */
	/*  for display                                               */
	/*------------------------------------------------------------*/

int pds_help_width = {60};

	/*------------------------------------------------------------*/
	/*  The pds_use_sfdus variable is used to turn SFDU checking  */
	/*  on or off in the label library.                           */
	/*------------------------------------------------------------*/

LOGICAL pds_use_sfdus = {TRUE};

	/*------------------------------------------------------------*/
	/*  The pds_default_stream variable is used to set the default*/
	/*  stream record type used in label output.                  */
	/*------------------------------------------------------------*/

int pds_default_rectype = DEFAULT_REC_TYPE;

	/*------------------------------------------------------------*/
	/*  The pds_record_term variable is used to store the record  */
	/*  terminator used when writing a label.                     */
	/*------------------------------------------------------------*/

char pds_record_term [3] = "\r\n";

long pds_default_reclen = 80;

long pds_records_needed = 0;

long pds_records_written = 0;

/*--------------------------------------------------------------------*/
/*                        End:  "pdsglob.h"                           */
/*--------------------------------------------------------------------*/

