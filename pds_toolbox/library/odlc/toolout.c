/*****************************************************************************

  Description: Routines for writing error messages, warning messages and the
               statements written by PrintLabel and WriteLabel.
 
  Author:  Randy Davis, University of Colorado LASP

  Creation Date:  25 August 1990
  Last Modified:  29 July 1992

  History:

    Creation - This set of routines was introduced in Version 2 of the
    ODLC library.  These routines are 'standard' versions of output
    routines for the error and warning messages generated while reading
    and parsing a label and for the statements written by PrintLabel and
    WriteLabel.  The error and warning routines put out messages to the
    'stderr' file.  ODLPrintStmt writes to the 'stdout' file.  ODLWriteStmt
    writes to a file specified by the first argument to the routine.  These
    routines were introduced in the Version 2.0 release of the ODLC library.
    Software developers using the ODLC package can customize their
    program's output by providing their own versions of these routines.
    For example, user-provided versions of the print routines might be
    used to place output into special windows on a workstation screen.

    Version 2.1 - 14 March 1991 - R. Davis, U. of Colorado LASP
      a) Defined the ODLerror_count and ODLwarning_count global variables in
         this module rather than in the file with the parser action routines.
         Also added a new global variable, ODLlinenumber_flag, to make the
         reporting of line numbers in error/warning messages optional.  This
         allows the error routines to be used for other purposes than
         printing and writing messages from the label parser.
      b) Added routine ODLPrintInfo to print information messages.

    Version 2.2 - 18 May 1991 - M. DeMore, Jet Propulsion Laboratory
       Removed include statements that were Unix specific and placed them
       in odldef.h. Added include file odlinter.h.

    Version 3.0 - 29 July 1992 - M. DeMore, Jet Propulsion Laboratory
       Commented this PDS Toolbox specific version and added fixed length
       record handling to ODLWriteStmt.

	Version 3.1 - 04 March 2003 - M. Cayanan, Jet Propulsion Laboratory
	   Added #ifdef code for LV_TOOL in the ODLPrintError and ODLPrintWarning
	   routines.
*****************************************************************************/

#include "pdsdef.h"
#include "odlinter.h"
#include "errordef.h"

extern int yylineno;

int  ODLerror_count;
int  ODLwarning_count;
int  ODLlinenumber_flag;

extern long pds_default_reclen;
extern long pds_records_needed;
extern long pds_records_written;
extern char pds_record_term[];
extern int ODLlinenumber_flag;

#ifdef LV_TOOL

extern long pds_warning;
extern LOGICAL track_odl_errors;
extern long lvtool_max_errors;
extern long total_error_count;

#endif

/*****************************************************************************

  Routine: ODLPrintError
 
  Description: Appends a message reporting an error detected during the
               parsing of an ODL label to the global message list.
 
  Input:
	  error_msg - Character string with text of error message.

  Output: The error message string is printed to the stderr output file.
  
  Change History:
		03-04-03 MDC
					Added ifdef compilation code for LV_TOOL to keep track of
					ODL errors and to check when the max. amount of messages
					allowed has been exceeded.
*****************************************************************************/

void ODLPrintError (error_msg)

char  error_msg[];
{
    char *temp_str = {NULL};
	
#ifdef LV_TOOL 
	if((track_odl_errors) && (total_error_count <= lvtool_max_errors))
		++total_error_count;
	
	if(total_error_count > lvtool_max_errors)
		return;
#endif

    temp_str = (char *) malloc (50 + String_Size(error_msg));
    if (temp_str == 0)
	exit (1);
    if (ODLlinenumber_flag)
       sprintf (temp_str, "ERROR:   Line %d -- %s", yylineno, error_msg);
    else
       sprintf (temp_str, "ERROR:   %s", error_msg);

    err_append_message (CONTINUE, temp_str);

    ODLerror_count++;
    free (temp_str);
    return;
}

/*****************************************************************************

  Routine: ODLPrintWarning

  Description: Appends a message to warn of a non-fatal problem detected
	       during the parsing of an ODL label to the global message
	       list.

  Input:
	  warning - Character string with text of warning message.

  Output: The warning message string is printed to the stderr output file.

  Change History:
		03-04-03 MDC
					 Added ifdef code for LV_TOOL to check for the pds_warning
					 flag before appending a warning message.
*****************************************************************************/

void ODLPrintWarning (warning)

char warning[];
{
    char *temp_str = {NULL};
	
#ifdef LV_TOOL

	if(pds_warning == FALSE)
		return;

#endif

    temp_str = (char *) malloc (50 + String_Size(warning));
    if (temp_str == 0)
	exit (1);

	if (ODLlinenumber_flag)
       sprintf (temp_str, "WARNING: Line %d -- %s", yylineno, warning);
    else
       sprintf (temp_str, "WARNING: %s", warning);

      err_append_message (CONTINUE, temp_str);  

	ODLwarning_count++;

    free (temp_str);
    return;
}

/*****************************************************************************

  Routine: ODLPrintInfo

  Description: Appends a message to inform the user of a situation that
	       is important but not an error or warning to the global
	       message list.

  Input:
	  info_message - Character string with text of the message.

  Output: The message string is printed to the stdout output file.

*****************************************************************************/

void ODLPrintInfo (info_message)

char info_message [];
{
    char *temp_str = {NULL};

    temp_str = (char *) malloc (50 + String_Size(info_message));
    if (temp_str == 0) 
        exit (1);

    sprintf (temp_str, "INFO:    %s", info_message);
    err_append_message (CONTINUE, temp_str);
    free (temp_str);
    return;
}



/*****************************************************************************

  Routine: ODLPrintStmt
 
  Description: Prints an ODL statement to the screen.
 
  Input:
          stmt - Character string with statement to be printed.
 
  Output: The statement is printed to the stdout output file.

*****************************************************************************/


void ODLPrintStmt (statement)
     char statement[];
{

  fputs (statement, stdout);

  return;
}



/*****************************************************************************

  Routine: ODLWriteStmt
 
  Description: Writes an ODL label statement to the specified file, or buffers
               it until an entire record's worth of data is available. Is
               pds_default_reclen is not 0, then this routine assumes it is
               writing fixed length records.  If so, then the last call to
               ODLWriteStmt must pass NULL as the value for statement, so
               that this routine will flush its buffer and write the final
               record, padded with blanks, to the file.
 
  Input:
          output_file - Pointer to file to which statement is to be written.
          statement - Character string with statement to be written.
 
  Output: The statement is printed to the output file.

*****************************************************************************/


void ODLWriteStmt (output_file,statement)
     FILE *output_file;
     char  statement[];
{

  static long buflen = 0;
  static char *buffer = NULL;
  long pad;
  char *temp_ptr;
  long len;

  /* If it's a stream record, just write the statement */

  if (pds_default_reclen == 0 && statement != NULL)
  {
     fputs (statement, output_file);
     temp_ptr = statement;
     do
     {
        temp_ptr = strstr (temp_ptr, pds_record_term);
        if (temp_ptr) 
        {
	   pds_records_written++;
	   temp_ptr = temp_ptr + strlen (pds_record_term);
	}
     }
     while (temp_ptr);
  }
  /* otherwise we have fixed records */

  else if (pds_default_reclen != 0)
  {
     /* if this isn't the last write to the file */

     if (statement != NULL)
     {
	/* if this is the first write to the file, create a new buffer */

	if (buffer == NULL)
	{
	   Malloc_String(buffer, (int) pds_default_reclen + 1);
	   buflen = 0;
	}

	/* append part of the new statement to the buffer */

	temp_ptr = statement;
	len = (long) strlen (statement);

        /* write out as many complete records as we can */

	while (len != 0)
	{
	   if (len + buflen < pds_default_reclen)
	   {
	       buflen += len;
	       len = 0;
	       strcat (buffer, temp_ptr);
	    }
	    else
	    {
	       strncat (buffer, temp_ptr, (int) (pds_default_reclen - buflen));
	       buffer [pds_default_reclen] = EOS;
	       temp_ptr = temp_ptr + (pds_default_reclen - buflen);
	       len = len - (pds_default_reclen - buflen);
	       fputs (buffer, output_file);
	       strcpy (buffer, "");
	       pds_records_written++;
	       buflen = 0;
	    }
	}
     }

     /* if this is the final write statement and these are fixed records */

     if (statement == NULL && buflen != 0 && pds_default_reclen != 0)
     {
	/* pad the final buffer with blanks and write it  */

        pad = pds_default_reclen - buflen;
	for (; buflen < pds_default_reclen; buflen++) buffer[buflen] = ' ';
	buffer [pds_default_reclen] = '\0';
        fputs (buffer, output_file);
        pds_records_written++;

        /* if there isn't a safe pad then pad another record  */

        if (pad < PDS_LABEL_PAD && pds_records_needed != 0 && 
            pds_records_written < pds_records_needed)
        {
           for (buflen = 0; buflen < pds_default_reclen; buflen++) 
		buffer[buflen] = ' ';
	   buffer [pds_default_reclen] = '\0';
           fputs (buffer, output_file);
           pds_records_written++;
	}

        buflen = 0;
        Lemme_Go (buffer);
     }
  }
  return;
}

