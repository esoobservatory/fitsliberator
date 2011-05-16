/*****************************************************************************

  File:  rprt_err.c

  Description: This file contains two C routines for error message routing
               and reporting: OaRouteErrorMessages and OaReportError.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:   1 Sept  1994

  History:

    Creation - This set of routines was part of the Alpha Release of the
               OA library.

*****************************************************************************/

#include <stdio.h>

/* These are prototypes for functions in this file.  */

#ifdef _NO_PROTO
int OaRouteErrorMessages();
int OaReportError();
#else
int OaRouteErrorMessages( char *message_fname, FILE *message_fptr);
int OaReportError( char *input_error_string);
#endif

/* Global variables */

char error_string[400];
FILE *Oa_message_fptr = NULL;


/*****************************************************************************

  Routine: OaRouteErrorMessages
 
  Description: Sets the destination for messages written by OaReportError, 
               either to a file the caller has opened, to a file the caller
               specifies by name, or to the standard error output (default).  
               Subsequent calls to OaReportError will write to the specified 
               destination.
   
  Input:  
          message_fname  - A file pathname, valid on the platform being run on.

          message_fptr   - Optional file pointer; the file must be opened for
                           write or append.

  Output: 

  Notes:

  1) If message_fptr is not NULL, subsequent calls to OaReportError will write
     to message_fptr.
     If message_fptr is NULL, and message_fname is not NULL, the message_fname
     file will be opened, and subsequent calls to OaReportError will write to
     this file.
     If both inputs are NULL, then OaReportError will write to standard
     error; this is also the default before OaRouteErrorMessages is called.

*****************************************************************************/

#ifdef _NO_PROTO
int OaRouteErrorMessages( message_fname, message_fptr)
char *message_fname;
FILE *message_fptr;
#else
int OaRouteErrorMessages( char *message_fname,
                          FILE *message_fptr)
#endif
{

if (message_fptr != NULL)
  Oa_message_fptr = message_fptr;
else {
  if (message_fname != NULL) {
    if ((Oa_message_fptr = (FILE *) fopen( message_fname, "a")) == NULL) {
      Oa_message_fptr = stderr;
      fprintf( stderr,
               "Couldn't open file: %s. Messages will be written to stderr\n",
               message_fname);
    }
  } else {
    Oa_message_fptr = stderr;
  }
}
return(0);
}




/*****************************************************************************

  Routine: OaReportError
 
  Description: Writes an error message to the file pointer Oa_message_fptr
               which was previously set by OaRouteErrorMessages, or if this
               hasn't been called, to stdout.
   
  Input:  
          input_error_string - A NULL-terminated string.

  Output: The string is written to Oa_message_fptr; if a write error occurs,
          the message is written to stdout.

  Notes:

*****************************************************************************/

#ifdef _NO_PROTO
int OaReportError( input_error_string)
char *input_error_string;
#else
int OaReportError( char *input_error_string)
#endif
{

  if (Oa_message_fptr == NULL) Oa_message_fptr = stderr;

  /* If successful, fprintf returns the number of characters written
     (0 under ULTRIX), so if the return value is negative, there was
     an error.  */

  if (fprintf( Oa_message_fptr, "%s\n", input_error_string) < 0) {
    fprintf( stderr, "%s\n", input_error_string);
    fflush( stderr);
  }
  else {
    fflush( Oa_message_fptr);
  }
  return(0);
}
