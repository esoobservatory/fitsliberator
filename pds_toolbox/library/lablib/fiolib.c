/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Component                                                           *
 *    Library fiolib.c                                                 *
 * Abstract                                                            *
 *    Low-level PDS file utilities                                     *
 * Detailed Description                                                *
 *    The fiolib contains the subroutines used by                      *
 *    PDS software to perform a variety of low-level file              *
 *    manipulation tasks.                                              *
 * Internal References                                                 *
 *    fio_convert_file                                                 *
 *    fio_copy_file                                                    *
 *    fio_exit                                                         *
 *    fio_get_setup_value                                              *
 *    fio_get_term                                                     *
 *    fio_has_var_records                                              *
 *    fio_read_file                                                    *
 *    fio_setup                                                        *
 *    fio_size                                                         *
 * Local (static) Routines                                             *
 *    fiox_read_line                                                   *
 * Authors and Institutions                                            *
 *    David P. Bernath / J.P.L.                                        *
 * Version and Date                                                    *
 *    4.2   May 11, 1992                                               *
 * Change History                                                      *
 *    DPB   08-17-90   Original code.                                  *
 *    DPB   08-23-90   Eliminated individual library include files.    *
 *                     Completely redesigned fio_redirect_output and   *
 *                     fio_restore_output.                             *
 *    DPB   08-29-90   Added fio_flush.                                *
 *    MDD   10-02-90   Humanized error messages                        *
 *    DPB   02-04-91   Added fio_has_var_records and                   *
 *                     fio_convert_var_to_stream.                      *
 *    DPB   03-13-91   Added fio_get_setup_value and fio_setup.        *
 *                     Added more external references.                 *
 *    MDD   03-14-91   Changed util_catenate_directory to              *
 *                     util_create_file_spec                           *
 *    MDD   03-20-91   Added fio_convert_stream_to_fixed               *
 *    DPB   04-05-91   Added fio_cleanup, fio_initialize, and fio_size *
 *    MDD   04-05-91   Added fio_convert_to_rms_stream                 *
 *    MDD   05-13-91   Added fio_exit, fio_convert_file                *
 *    DPB   09-24-91   Added fio_read_line and fio_copy_file, and      *
 *                     removed most of the fio_convert routines.       *
 *    MDD   10-17-91   Removed fio_redirect_output, fio_restore_output,*
 *                     fio_initialize, and fio_cleanup.                *
 *    DPB   10-23-91   Made fio_read_line a static routine, called     *
 *                     fiox_read_line.                                 *
 *    MDD   10-25-91   Removed fio_flush.                              *
 *    MDD   05-11-92   Added fiox_get_term and modified fio_convert... *
 *    MDC   11-18-04   Made change to fio_setup. See notes in function.*
 *    MDC   01-03-05   Made change to fio_setup.                       *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "pdsdef.h"
#include "fiodef.h"
#include "sysdef.h"
#include "utildef.h"
#include "errordef.h"

extern char *pds_scratch_directory;
extern char *pds_temp_data_fname;
extern char *pds_temp_label_fname;
extern ERROR_LIST *pds_message_list;

#ifndef SUN_UNIX

static char *fiox_read_line (FILE *, long, int, long *, LOGICAL *, char *);

#else

static char *fiox_read_line ();

#endif


/**********************************************************************
 *$Component                                                          *
 *    LOGICAL fio_convert_file (file_name, old_record_type,           *
 *                              new_record_type, old_record_length,   *
 *                              new_record_length)                    *
 *$Abstract                                                           *
 *    Converts a file from one record terminator format to another.   *
 *$Keywords                                                           *
 *    FIOLIB                                                          *
 *    CONVERT                                                         *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    file_name:                                                      *
 *        The file_name variable is a general purpose character       *
 *        string containing a file specification.                     *
 *    old_record_type:                                                *
 *        The old_record_type variable is an integer that represents  *
 *        the type of records a file contains: e.g., PDS_RF_STREAM_LF,*
 *        PDS_RF_FIXED_CRLF, etc.                                     *
 *    new_record_type:                                                *
 *        The new_record_type variable is an integer that represents  *
 *        the type of records a file contains: e.g., PDS_RF_STREAM_LF,*
 *        PDS_RF_FIXED_CRLF, etc.                                     *
 *    old_record_length:                                              *
 *        The old_record_length variable is an integer specifying the *
 *        maximum number of characters allowed in a record.           *
 *    new_record_length:                                              *
 *        The new_record_length variable is an integer specifying the *
 *        maximum number of characters allowed in a record.           *
 *$Outputs                                                            *
 *    None                                                            *
 *$Returns                                                            *
 *    success_flag:                                                   *
 *$Detailed_Description                                               *
 *    The fio_convert_file routine converts a file with record type,  *
 *    old_record_type, to a file with record type, new_record_type.   *
 *    If the file cannot be converted, this routine returns FALSE.    *
 *    The record types that can be input (as integer constants) are:  *
 *                   PDS_RF_RMS_VAR                                   *
 *                   PDS_RF_RMS_STREAM                                *
 *                   PDS_RF_STREAM_LF                                 *
 *                   PDS_RF_STREAM_CR                                 *
 *                   PDS_RF_STREAM_CRLF                               *
 *                   PDS_RF_FIXED_LF                                  *
 *                   PDS_RF_FIXED_CR                                  *
 *                   PDS_RF_FIXED_CRLF                                *
 *                   PDS_RF_BINARY                                    *
 *                   PDS_RF_ASCII                                     *
 *    Note that PDS_RF_STREAM_LF is the default file creation mode    *
 *    on Unix systems, and that PDS_RF_RMS_STREAM is the default on   *
 *    VMS systems.                                                    *
 *$Limitations                                                        *
 *    RMS native formats will not be produced unless the VAX flag     *
 *    is defined.                                                     *
 *$External_References                                                *
 *    Item                     Shared-Data           Access           *
 *    ------------------------------------------------------------    *
 *    pds_temp_data_fname      pdsglob.h             read             *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore                                                 *
 *$Version_and_Date                                                   *
 *    3.0   May 11, 1992                                              *
 *$Change_History                                                     *
 *    MDD   05-13-91   Original code.                                 *
 *    DPB   09-24-91   Re-wrote routine to handle converting between  *
 *                     all possible combinations of valid file fomats.*
 *                     (MODIFIED:  Inputs, components, p-code).       *
 *    DPB   10-21-91   Modified routine to accomodate very long       *
 *                     records, and the new read_line routine.  Also  *
 *                     changed its name and moved it out of fiolib.   *
 *    DPB   10-31-91   Removed fio_flush, and changed the main loop.  *
 *    MDD   12-11-91   Changed char_count to a long for PC.           *
 *    MDD   03-16-92   The great int -> long conversion               *
 *    MDD   04-16-92   Added MAC_THINK compiler ifdefs.               *
 *    MDD   05-11-92   Significant updates to support the FFTOOL.     *
 *                     Added ASCII record type. Added call to         *
 *                     fiox_get_term to attempt to find record type   *
 *                     "on the fly." Modified to write record         *
 *                     terminators for all but binary output files.   *
 *                     Modified to ignore input record length in all  *
 *                     cases except when input file is binary and the *
 *                     output file is not fixed format.               *
 **********************************************************************/

LOGICAL fio_convert_file (file_name, old_record_type, new_record_type,
			 old_record_length, new_record_length)

char *file_name;
int old_record_type;
int new_record_type;
long old_record_length;
long new_record_length;

{
    FILE *infile = {NULL};
    FILE *outfile = {NULL};
	char mrs[20] = {0};
    char *c = {NULL};
    char *temp = {NULL};
    char *blanks = {NULL};
    char *buffer = {NULL};
    char err_msg [2*PDS_MAXLINE];
    char terminator [3];
    char *old_terminator = NULL;
    long char_count;
    int term_len;
    int old_term_len;
    long i;
    LOGICAL term_found = {FALSE};
    long read_record_length;
    LOGICAL binary_conversion = {(old_record_type == PDS_RF_BINARY) ||
				 (new_record_type == PDS_RF_BINARY)};

#ifdef VAX
    LOGICAL remove_terminators = {(old_record_type != PDS_RF_BINARY)};
#else
    LOGICAL remove_terminators = {(old_record_type != PDS_RF_RMS_VAR) &&
				  (old_record_type != PDS_RF_BINARY)};
#endif

    LOGICAL new_has_fixed_records = {(new_record_type == PDS_RF_FIXED_CR) ||
				     (new_record_type == PDS_RF_FIXED_LF) ||
				     (new_record_type == PDS_RF_FIXED_CRLF) ||
				     (new_record_type == PDS_RF_BINARY)};

    LOGICAL lines_truncated = {FALSE};
    LOGICAL success = {FALSE};

/** BEGIN **/
    /*-----------------------------------------------------------------------*/
    /** IF the old and new record types are the same THEN                   **/
    /**     IF the old and new record lengths are the same (or not needed)  **/
    /**         no conversion is needed so RETURN (Sorry, Dave, but I was   **/
    /**            too lazy to add another IF and indent.)                  **/
    /**     ENDIF                                                           **/
    /** ENDIF                                                               **/
    /*-----------------------------------------------------------------------*/

    if (old_record_type == new_record_type)
       if ((new_has_fixed_records && old_record_length == new_record_length)
	   || (!new_has_fixed_records))
	   return (TRUE);

    /*-----------------------------------------------------------------------*/
    /** IF we're compiling on anything other than a VAX THEN                **/
    /**     Make sure the new record type passed in is not one of the VAX   **/
    /**         specific record types.                                      **/
    /** ELSE                                                                **/
    /*-----------------------------------------------------------------------*/

#ifndef VAX
    if ((new_record_type == PDS_RF_RMS_STREAM) ||
	    (new_record_type == PDS_RF_RMS_VAR))
    {
	strcpy (err_msg,
		"RMS Stream and Variable format files can only be produced in a VMS environment.  ");
	strcat (err_msg,
		"File could not be converted to new record type");
	err_append_message (ERROR1, err_msg);
	return (FALSE);
    }
    else
#endif

    /*-----------------------------------------------------------------------*/
    /** Flush the output file.                                              **/
    /*-----------------------------------------------------------------------*/

    Flush_File(pds_temp_data_fname);

    /*-----------------------------------------------------------------------*/
    /** For VMS/VAX machines only:                                          **/
    /*-----------------------------------------------------------------------*/

#ifdef VAX

    /*-----------------------------------------------------------------------*/
    /** Open the input file, and assign the appropriate record              **/
    /** terminator character for the new file.                              **/
    /*-----------------------------------------------------------------------*/

    infile = fopen (file_name, "r");

    switch (new_record_type)
    {
	case PDS_RF_STREAM_CR   :
	case PDS_RF_FIXED_CR    : strcpy (terminator, "\n");
				  term_len = 1;
				  outfile = fopen(pds_temp_data_fname,"w","RFM=STMCR");
				  break;

	case PDS_RF_STREAM_LF   :
	case PDS_RF_FIXED_LF    : strcpy (terminator, "\n");
				  term_len = 1;
				  outfile = fopen(pds_temp_data_fname,"w","RFM=STMLF");
				  break;

	case PDS_RF_STREAM_CRLF :
	case PDS_RF_FIXED_CRLF  : strcpy (terminator, "\n");
				  term_len = 2;
				  outfile = fopen(pds_temp_data_fname,"w","RFM=STM");
				  break;

	case PDS_RF_BINARY      : strcpy (terminator, "");
				  term_len = 0;
				  sprintf (mrs, "MRS=%ld", new_record_length);
				  outfile = fopen(pds_temp_data_fname,"w",
						  "RFM=FIX", mrs);
				  break;

	case PDS_RF_RMS_STREAM  : strcpy (terminator, "\n");
				  term_len = 1;
				  outfile = fopen(pds_temp_data_fname,"w","RFM=STM");
				  break;

	case PDS_RF_RMS_VAR     : strcpy (terminator, "\n");
				  term_len = 1;
				  outfile = fopen(pds_temp_data_fname,"w",
						 "RFM=VAR","RAT=CR");
				  break;

	default                 : strcpy (terminator, "\n");
				  term_len = 1;
				  outfile = fopen(pds_temp_data_fname,"w");
				  break;

    }  /*  End:  "switch (new_record_type) ..."  */

    /*-----------------------------------------------------------------------*/
    /** End of the VMS-specific code.                                       **/
    /*-----------------------------------------------------------------------*/

#else

    /*-----------------------------------------------------------------------*/
    /** For non-VMS/VAX systems:                                            **/
    /** Open the input and output files in binary mode.                     **/
    /*-----------------------------------------------------------------------*/

    infile = fopen (file_name, "rb");

    /* This little nonsense here forces the file to be created in text mode
       on the macintosh before it is opened in binary mode.  Otherwise, the
       file type doesn't get set and most Mac applications won't see the
       resulting file. */

#ifndef MAC_THINK
    outfile = fopen (pds_temp_data_fname, "wb");
#else
    outfile = fopen (pds_temp_data_fname, "w");
    if (outfile != NULL)
    {
       fclose (outfile);
       outfile = fopen (pds_temp_data_fname, "ab+");
    }
#endif

    /*-----------------------------------------------------------------------*/
    /** Determine the appropriate terminator characters for the new file.   **/
    /*-----------------------------------------------------------------------*/

    switch (new_record_type)
    {
	case PDS_RF_STREAM_CR   :
	case PDS_RF_FIXED_CR    : strcpy (terminator, PDS_CR_STRING);
				  break;
	case PDS_RF_STREAM_LF   :
	case PDS_RF_FIXED_LF    : strcpy (terminator, PDS_LF_STRING);
				  break;
	case PDS_RF_STREAM_CRLF :
	case PDS_RF_FIXED_CRLF  : strcpy (terminator, PDS_CRLF_STRING);
				  break;
	default                 : strcpy (terminator, "");
				  break;

    }  /*  End:  "switch (new_record_type) ..."  */

    term_len = (int) strlen (terminator);

    /*-----------------------------------------------------------------------*/
    /** End of the non-VAX/VMS specific code.                               **/
    /*-----------------------------------------------------------------------*/

#endif

    /*-----------------------------------------------------------------------*/
    /** IF either of the files couldn't be opened THEN                      **/
    /**     Append a message onto the global list of messages.              **/
    /** ELSE                                                                **/
    /*-----------------------------------------------------------------------*/

    if (infile == NULL || outfile == NULL)
    {
	sprintf (err_msg,
		 "File %s cannot be opened for reading.  It will not be converted.",
		 file_name);
	err_append_message (ERROR1, err_msg);
    }
    else
    {

        /*-------------------------------------------------------------------*/
        /** Determine the terminators used in the old file.                 **/
        /*-------------------------------------------------------------------*/

        old_terminator = fio_get_term (infile, old_record_type);
        old_term_len = (int) strlen (old_terminator);

	/*-------------------------------------------------------------------*/
	/** Adjust the old and new record lengths passed in.                **/
	/**   (Ignore input record length if input file had terminators)    **/
	/**   (Make sure output record length isn't smaller than terminator)**/
	/**   (Set actual record length to be read from input file to be    **/
	/**      the output record length if the input file had no          **/
	/**      terminators.)                                              **/
	/*-------------------------------------------------------------------*/

	if (old_record_length <= term_len || old_term_len != 0)
	    old_record_length = 0;
	if (new_record_length <= term_len) new_record_length = 0;

	if ((old_record_type == PDS_RF_BINARY || old_term_len == 0)
	     && new_has_fixed_records)
	   read_record_length = new_record_length - term_len;
	else
	   read_record_length = old_record_length;

	/*-------------------------------------------------------------------*/
	/** IF the new file is one of the fixed length formats THEN         **/
	/**     Initialize a string of blanks which will be used to         **/
	/**         pad records out to their proper length.                 **/
	/** ENDIF                                                           **/
	/*-------------------------------------------------------------------*/

	if (new_has_fixed_records)
	{
	    Malloc_String(blanks, (int) (new_record_length + term_len + 1))
	    for (temp = blanks; temp < (blanks + new_record_length) - term_len; temp++)
		    *temp = ' ';
	    *temp = EOS;
	}

	/*-------------------------------------------------------------------*/
	/** WHILE there are records left to be read DO                      **/
	/*-------------------------------------------------------------------*/

	while ((buffer = fiox_read_line (infile, read_record_length,
			 old_record_type, &char_count, &term_found,
			 old_terminator)) != NULL)
	{
	    /*---------------------------------------------------------------*/
	    /** Clean up the string.                                        **/
	    /*---------------------------------------------------------------*/

	    if (remove_terminators && term_found && (old_term_len > 0))
	    {
		c = buffer + char_count - old_term_len;
		if ((*c == '\n') || (*c == '\r'))
		{
		    *c = EOS;
		    char_count -= old_term_len;
		}
		else
		{
		    ++c;
		    if ((*c == '\n') || (*c == '\r'))
		    {
			*c = EOS;
			--char_count;
		    }
		}
	    }

	    if (! binary_conversion && term_found)
	    {
		Strip_Trailing(buffer, ' ')
		char_count = (long) strlen(buffer);
	    }

	    /*---------------------------------------------------------------*/
	    /** IF the old record is too long THEN                          **/
	    /**     Truncate it.                                            **/
	    /** ENDIF                                                       **/
	    /*---------------------------------------------------------------*/

	    if ((new_record_length > 0) &&
		    (char_count > new_record_length - term_len))
	    {
		lines_truncated = TRUE;
		char_count = new_record_length - term_len;
		*(buffer + char_count) = EOS;
	    }

	    /*---------------------------------------------------------------*/
	    /** IF the new file is one of the fixed length formats THEN     **/
	    /**     Pad the record with blanks.                             **/
	    /** ENDIF                                                       **/
	    /*---------------------------------------------------------------*/

	    if (new_has_fixed_records)
	    {
		if (! binary_conversion)
		    Realloc_String(buffer, (int) (old_record_length + new_record_length + 1))
		else
		{
		    Malloc_String(temp, (int) (old_record_length + new_record_length + term_len + 1))
		    util_byte_copy(temp, buffer, char_count);
		    Lemme_Go(buffer)
		    buffer = temp;
		}
		strcpy ((buffer + char_count), (char *) (blanks + char_count));
		char_count = new_record_length - term_len;
	    }

	    /*---------------------------------------------------------------*/
	    /** Write the record to the new file.                           **/
	    /*---------------------------------------------------------------*/

	    for (i=0; i < char_count; ++i) fputc (*(buffer+i), outfile);
	    if (term_len > 0) fprintf (outfile, "%s", terminator);

	    Lemme_Go(buffer)

	/*-------------------------------------------------------------------*/
	/** ENDWHILE                                                        **/
	/*-------------------------------------------------------------------*/

	}  /*  End:  "while (buffer = fiox_read_line( ..."  */

	Lemme_Go(buffer)

	/*-------------------------------------------------------------------*/
	/** IF any records were truncated THEN                              **/
	/**     Append a message onto the global list of messages.          **/
	/** ENDIF                                                           **/
	/*-------------------------------------------------------------------*/

	if (lines_truncated)
	{
	    sprintf (err_msg,
		     "Lines were truncated when converting the record format of %s",
		     file_name);
	    err_append_message (WARNING, err_msg);
	}

	/*-------------------------------------------------------------------*/
	/** Close both files.                                               **/
	/*-------------------------------------------------------------------*/

	Close_Me(infile)
	Close_Me(outfile)

	/*-------------------------------------------------------------------*/
	/** Copy the temporary outfile back to the input file.              **/
	/*-------------------------------------------------------------------*/

	success = sys_copy_file (pds_temp_data_fname, file_name);

    /*-----------------------------------------------------------------------*/
    /** ENDIF                                                               **/
    /*-----------------------------------------------------------------------*/

    }  /*  End:  "if (infile == NULL || ... else ..."  */

    /*-----------------------------------------------------------------------*/
    /** Final cleanup.                                                      **/
    /*-----------------------------------------------------------------------*/

    Lemme_Go(buffer);
    Lemme_Go(blanks);
    Lemme_Go(old_terminator);

    Close_Me(infile)
    Close_Me(outfile)

    /*-----------------------------------------------------------------------*/
    /** RETURN the success flag.                                            **/
    /*-----------------------------------------------------------------------*/

    return (success);

/** END **/

}  /*  "fio_convert_file"  */




/**********************************************************************
 *$Component                                                          *
 *    LOGICAL fio_copy_file (old_file_name, new_file_name)            *
 *$Abstract                                                           *
 *    Copies a file.                                                  *
 *$Keywords                                                           *
 *    FIOLIB                                                          *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    old_file_name:                                                  *
 *        The old_file_name variable is a general purpose character   *
 *        string containing a file specification.                     *
 *    new_file_name:                                                  *
 *        The old_file_name variable is a general purpose character   *
 *        string containing a file specification.                     *
 *$Outputs                                                            *
 *    None                                                            *
 *$Returns                                                            *
 *    success_flag:                                                   *
 *$Detailed_Description                                               *
 *    The fio_copy_file routine makes a copy of a file.               *
 *$Limitations                                                        *
 *    On VAX/VMS systems, this routine will only work with files in   *
 *    STREAM_LF format.                                               *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    David P. Bernath / J.P.L.                                       *
 *$Version_and_Date                                                   *
 *    1.1   April 16, 1992                                            *
 *$Change_History                                                     *
 *    DPB   09-24-91   Original code.                                 *
 *    MDD   04-16-92   Added MAC_THINK compiler ifdefs.               *
 **********************************************************************/

LOGICAL fio_copy_file (old_file_name, new_file_name)

char *old_file_name;
char *new_file_name;

{
    FILE *infile = {NULL};
    FILE *outfile = {NULL};
    char buffer [PDS_BUFFLEN + 1];
    char err_msg [PDS_MAXLINE];
    long char_count = 0;
    LOGICAL done = {FALSE};
    LOGICAL success = {FALSE};

/** BEGIN **/

    /*-----------------------------------------------------------------------*/
    /** IF file names were passed in, and the new file could be flushed THEN **/
    /*-----------------------------------------------------------------------*/

    if ((new_file_name != NULL) && 
            (old_file_name != NULL) && (Flush_File(new_file_name)))
    {
        /*-------------------------------------------------------------------*/
        /** Open the input and output files in binary mode.                 **/
        /*-------------------------------------------------------------------*/

        infile = fopen (old_file_name, "rb");
#ifndef MAC_THINK        
        outfile = fopen (new_file_name, "wb");
#else
        outfile = fopen (new_file_name, "w");
        if (outfile != NULL)
        {
          fclose (outfile);
          outfile = fopen (new_file_name, "ab+");
        }
       
#endif            
        /*-------------------------------------------------------------------*/
        /** IF either file couldn't be opened THEN                          **/
        /**    Append a message onto the global list of messages.           **/
        /** ELSE                                                            **/
        /*-------------------------------------------------------------------*/

        if (infile == NULL || outfile == NULL)
        {
            sprintf (err_msg, 
                     "File %s cannot be copied to %s.", 
                     old_file_name, new_file_name);
            err_append_message (ERROR1, err_msg);
        }
        else
        {
            /*---------------------------------------------------------------*/
            /** Read a record and write it to the output file.              **/
            /*---------------------------------------------------------------*/

            char_count = (long) fread (buffer, 1, PDS_BUFFLEN, infile);
            while (! done)
            {
		fwrite (buffer, char_count, 1, outfile);
                char_count = (long) fread (buffer, 1, PDS_BUFFLEN, infile);
                done = (feof(infile) && (char_count == 0));

            }  /*  End:  "while (! feof(infile)) ..."  */

            success = (! ferror(infile) && ! ferror(outfile));
    
        /*-------------------------------------------------------------------*/
        /** ENDIF                                                           **/
        /*-------------------------------------------------------------------*/

        }  /*  End:  "if (infile == NULL || ... else ..."  */

    /*-----------------------------------------------------------------------*/
    /** ENDIF                                                               **/
    /*-----------------------------------------------------------------------*/

    }  /*  End:  "if ((outfile_name != NULL) && ..."  */
    
    /*-----------------------------------------------------------------------*/
    /** Close both files.                                                   **/
    /*-----------------------------------------------------------------------*/

    Close_Me(infile)
    Close_Me(outfile)
   

    /*-----------------------------------------------------------------------*/
    /** RETURN the success flag.                                            **/
    /*-----------------------------------------------------------------------*/

    return (success);

/** END **/

}  /*  "fio_copy_file"  */



/**********************************************************************
 *$Component                                                          *
 *    void fio_exit ()                                                *
 *$Abstract                                                           *
 *    Deletes temporary files used by the toolbox                     *
 *$Keywords                                                           *
 *    FIOLIB                                                          *
 *    SETUP                                                           *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    None                                                            *
 *$Outputs                                                            *
 *    None                                                            *
 *$Returns                                                            *
 *    None                                                            *
 *$Detailed_Description                                               *
 *    The fio_exit routine deletes all of the temporary files used    *
 *    by the toolbox.                                                 *
 *$External_References                                                *
 *    Item                     Shared-Data           Access           *
 *    ------------------------------------------------------------    *
 *    pds_temp_data_fname      pdsglob.h             update           *
 *    pds_temp_label_fname     pdsglob.h             update           *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore                                                 *
 *$Version_and_Date                                                   *
 *    1.1   October 21, 1991                                          *
 *$Change_History                                                     *
 *    DPB   05-13-91   Original code.                                 *
 *    MDD   10-17-91   Removed redirect stuff and map filename stuff. *
 **********************************************************************/

void fio_exit ()

{
   sys_delete_file (pds_temp_data_fname);
   sys_delete_file (pds_temp_label_fname);

   Lemme_Go(pds_temp_data_fname);
   Lemme_Go(pds_temp_label_fname);
   Lemme_Go(pds_scratch_directory);

   return;
}



/**********************************************************************
 *$Component                                                          *
 *    LOGICAL fio_get_setup_value (keyword, text)                     *
 *$Abstract                                                           *
 *    Retrieves a value associated with a keyword from setup file.    *
 *$Keywords                                                           *
 *    FIOLIB                                                          *
 *    SETUP                                                           *
 *    READ                                                            *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    keyword:                                                        *
 *        The keyword variable is a general purpose character string  *
 *        which contains the first part of a KEYWORD=VALUE pair.      *
 *$Outputs                                                            *
 *    text:                                                           *
 *        The text variable is a general purpose character string     *
 *        that may contain one or more characters.                    *
 *$Returns                                                            *
 *    success_flag:                                                   *
 *        The success_flag variable is a general purpose variable     *
 *        which indicates success (a value of TRUE) or failure        *
 *        (a value of FALSE).                                         *
 *$Detailed_Description                                               *
 *    The fio_get_setup_value routine searches the KEYWORD=VALUE      *
 *    pairs in the toolbox setup file for the keyword passed in,      *
 *    and extracts the value associated with it.                      *
 *$Side_Effects                                                       *
 *    1. This routine writes messages to stdout.                      *
 *    2. The text output variable is not malloc'd.  It is assumed to  *
 *       be an array of characters.                                   *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    David P. Bernath                                                *
 *$Version_and_Date                                                   *
 *    1.1   March 20, 1992                                            *
 *$Change_History                                                     *
 *    DPB   03-13-91   Original code.                                 *
 *    MDD   03-20-92   Changed to search three directories for the    *
 *                     setup file                                     *
 **********************************************************************/

LOGICAL fio_get_setup_value (keyword, text)

char *keyword;
char *text;

{
    FILE *file_ptr = {NULL};
    char *c = {NULL};
    char test_keyword [PDS_MAXLINE];
    char line_in [PDS_MAXLINE];
    LOGICAL found = {FALSE};
    char *file_name = {NULL};

/*--------------------------------------------------------------------------*/
/** BEGIN                                                                  **/
/**                                                                        **/
/** Attempt to locate and open the setup file                              **/
/*--------------------------------------------------------------------------*/

    *text = EOS;

    if ((file_ptr = fopen (PDS_TOOLBOX_SETUP_FNAME, "r")) == NULL)
    {
        file_name = util_create_file_spec (PDS_DEFAULT_LOGIN, 
                                           PDS_TOOLBOX_SETUP_FNAME);
        if ((file_ptr = fopen (file_name, "r")) == NULL)
        {
           Lemme_Go(file_name);
           file_name = util_create_file_spec (PDS_SETUP_DIRECTORY, 
                                              PDS_TOOLBOX_SETUP_FNAME);
           file_ptr = fopen (file_name, "r");
	}
    } 

/*--------------------------------------------------------------------------*/
/** IF the setup file was found THEN                                       **/
/*--------------------------------------------------------------------------*/

    if (file_ptr != NULL)
    { 
/*--------------------------------------------------------------------------*/
/**     WHILE the right KEYWORD=VALUE pair has not been found DO           **/
/*--------------------------------------------------------------------------*/

        while ((! found) && (fgets (line_in, PDS_MAXLINE, file_ptr)))
        {
/*--------------------------------------------------------------------------*/
/**         Clean up the line read and remove any embedded blanks.         **/
/*--------------------------------------------------------------------------*/

            util_clean_up_string (line_in);

/*--------------------------------------------------------------------------*/
/**         Attempt to locate the equals ('=') character.                  **/
/*--------------------------------------------------------------------------*/

            c = (char *) strchr (line_in, '=');

/*--------------------------------------------------------------------------*/
/**         IF the attempt was successful THEN                             **/
/*--------------------------------------------------------------------------*/

            if (c != NULL)
            {
/*--------------------------------------------------------------------------*/
/**             Extract the KEYWORD part from the line.                    **/
/*--------------------------------------------------------------------------*/

                *c = EOS;
                strcpy (test_keyword, line_in);
                util_remove_char (test_keyword, ' ');

/*--------------------------------------------------------------------------*/
/**             IF the keyword extracted matches the one passed in THEN    **/
/**                 Extract the VALUE part from the line and prepare       **/
/**                     to return.                                         **/
/**             ENDIF                                                      **/
/*--------------------------------------------------------------------------*/

                if ((strcmp (test_keyword, keyword)) == 0)
                {
                    found = TRUE;
                    strcpy (text, (c + 1));
                    util_strip_lead_and_trail (text, ' ');

                }  /*  End:  "if ((strcmp (test_keyword, ..."  */

/*--------------------------------------------------------------------------*/
/**         ENDIF                                                          **/
/*--------------------------------------------------------------------------*/

			}  /*  End:  "if (c != NULL) ..."  */

/*--------------------------------------------------------------------------*/
/**     ENDWHILE                                                           **/
/*--------------------------------------------------------------------------*/

        }  /*  End:  "while ((fscanf (file_ptr,"%s = %s", ..."  */

/*--------------------------------------------------------------------------*/
/** ENDIF                                                                  **/
/*--------------------------------------------------------------------------*/
       fclose(file_ptr);
    }

/*--------------------------------------------------------------------------*/
/** RETURN success_flag                                                    **/
/*--------------------------------------------------------------------------*/

    Lemme_Go(file_name);
    return (found);

/*--------------------------------------------------------------------------*/
/** END                                                                    **/
/*--------------------------------------------------------------------------*/

}  /*  End:  "fio_get_setup_value"  */



/**********************************************************************
 *$Component                                                          *
 *    LOGICAL fio_has_var_records (file_name)                         *
 *$Abstract                                                           *
 *    Determines whether or not a file has VMS variable records       *
 *$Keywords                                                           *
 *    FIOLIB                                                          *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    file_name:                                                      *
 *        The file_name variable is a general purpose character       *
 *        string containing a file specification.                     *
 *$Outputs                                                            *
 *    NONE                                                            *
 *$Returns                                                            *
 *    success_flag:                                                   *
 *        The success_flag variable is a general purpose variable     *
 *        which indicates success (a value of TRUE) or failure        *
 *        (a value of FALSE).                                         *
 *$Detailed_Description                                               *
 *    The fio_has_var_records routine determines whether or not a     *
 *    file contains VMS variable length records.  These records       *
 *    consist of a two byte integer prefix indicating the number      *
 *    of bytes in the record that follows it.                         *
 *$Side_Effects                                                       *
 *    Since there is no completely unambiguous method of identifying  *
 *    a file which contains variable length records, this routine     *
 *    must make a number of assumptions about the sort of files it    *
 *    is likely to receive.  1)  If the first two bytes are both      *
 *    null (binary zero) then the file is assumed to contain variable *
 *    length records.  2)  If the first two bytes are some            *
 *    combination of filler characters (spaces, tabs, or newlines)    *
 *    then the file is assumed to be stream.  If these two conditions *
 *    fail, then a series of tests are performed to determine the     *
 *    position of the first label keywords in the file.               *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    David P. Bernath/ J.P.L.                                        *
 *$Version_and_Date                                                   *
 *    1.1   May 1, 1991                                               *
 *$Change_History                                                     *
 *    DPB   02-04-91   Original code.                                 *
 *    MDD   05-01-91   Changed to return FALSE if the file is a       *
 *                     directory.                                     *
 **********************************************************************/

LOGICAL fio_has_var_records (file_name)

char *file_name;

{
    FILE *file_ptr = {NULL};
    char keyword_list [PDS_NUMBER_OF_KEYWORDS][PDS_SIZE_OF_KEYWORDS + 1];
    char keyword_str [PDS_SIZE_OF_KEYWORDS + 1];
    char first_str [PDS_SIZE_OF_KEYWORDS + PDS_NUMBER_OF_VAR_BYTES + 1];
    char byte_str [PDS_NUMBER_OF_VAR_BYTES + 1];
    int i;
    int char_size = {sizeof(char)};
    LOGICAL is_var_record_file = {FALSE};
    LOGICAL first_bytes_are_null = {FALSE};    
    LOGICAL filler_in_byte_area = {FALSE};    
    LOGICAL filler_in_keyword_area = {FALSE};    
    LOGICAL keyword_after_bytes = {FALSE};    
    LOGICAL keyword_found_first = {FALSE};    


/*--------------------------------------------------------------------------*/
/** BEGIN                                                                  **/
/** IF the file is not a directory THEN                                    **/
/**    Initialize the list of keywords used to determine the type of file. **/
/*--------------------------------------------------------------------------*/

    if (fio_size (file_name) != -1)
    {
       sprintf (keyword_list [0], "NJPL");           /* SFDU label        */
       sprintf (keyword_list [1], "CCSD");           /* SFDU label        */
       sprintf (keyword_list [2], "OBJECT");         /* ODL label         */
       sprintf (keyword_list [3], "/*");             /* ODL comment       */

/*--------------------------------------------------------------------------*/
/**    IF the input file cannot be opened THEN                             **/
/**        Append an error message onto the global list of messages.       **/
/**    ELSE                                                                **/
/*--------------------------------------------------------------------------*/

       if ((file_ptr = fopen (file_name, "rb")) == NULL)
           err_append_message (ERROR1, 
              "Unable to determine the record type of the label file.");
       else
       {
/*--------------------------------------------------------------------------*/
/**        Read in the first two bytes from the label file.                **/
/*--------------------------------------------------------------------------*/

           fread (byte_str, char_size, PDS_NUMBER_OF_VAR_BYTES, file_ptr);
           byte_str [PDS_NUMBER_OF_VAR_BYTES] = EOS;
           util_byte_copy (first_str, byte_str, (long) PDS_NUMBER_OF_VAR_BYTES);

/*--------------------------------------------------------------------------*/
/**        IF the read went well THEN                                      **/
/**            Read in the next few bytes.                                 **/
/**        ELSE                                                            **/
/*--------------------------------------------------------------------------*/

           if ((feof (file_ptr) == 0) && (ferror (file_ptr) == 0))
           {
               fread (keyword_str, char_size, PDS_SIZE_OF_KEYWORDS, file_ptr);
               keyword_str [PDS_SIZE_OF_KEYWORDS] = EOS;
               util_byte_copy (&first_str[PDS_NUMBER_OF_VAR_BYTES],
                               keyword_str, (long) (PDS_SIZE_OF_KEYWORDS + 1));

           }  /*  End:  "if ((feof (file_ptr) == 0) && ..."  */

/*--------------------------------------------------------------------------*/
/**        IF both reads went well THEN                                    **/
/*--------------------------------------------------------------------------*/

           if ((feof (file_ptr) == 0) && (ferror (file_ptr) == 0))
           {
/*--------------------------------------------------------------------------*/
/**            Determine whether or not the first two bytes are both null. **/
/*--------------------------------------------------------------------------*/

               first_bytes_are_null = ((byte_str [0] == EOS) && 
                                          (byte_str [1] == EOS));

/*--------------------------------------------------------------------------*/
/**            Determine whether or not the keyword area (the area         **/
/**                immediately following the first two bytes) contains any **/
/**                filler characters (spaces, tabs, or newlines).          **/
/*--------------------------------------------------------------------------*/

               filler_in_keyword_area = isspace (keyword_str[0]);
        
/*--------------------------------------------------------------------------*/
/**            Determine whether or not both of the first two bytes are    **/
/**                filler characters.                                      **/
/*--------------------------------------------------------------------------*/

               filler_in_byte_area = (isspace (byte_str[0]) && 
                                         isspace (byte_str[1]));

/*--------------------------------------------------------------------------*/
/**            Determine whether or not one of the keywords on the keyword **/
/**                list is the first thing in the file.                    **/
/*--------------------------------------------------------------------------*/

               for (i=0; ((i < PDS_NUMBER_OF_KEYWORDS) && 
                               (! keyword_found_first)); ++i)
                   keyword_found_first = (strncmp (first_str, 
                                             keyword_list [i], 
                                                strlen (keyword_list [i])) == 0);
            
/*--------------------------------------------------------------------------*/
/**            Determine whether or not one of the keywords is the first   **/
/**                thing in the keyword area.                              **/
/*--------------------------------------------------------------------------*/

               for (i=0; ((i < PDS_NUMBER_OF_KEYWORDS) && 
                                (! keyword_after_bytes)); ++i)
                   keyword_after_bytes = (strncmp (keyword_str, 
                                              keyword_list [i], 
                                                  strlen (keyword_list [i])) == 0);
        
/*--------------------------------------------------------------------------*/
/**     ENDIF                                                              **/
/*--------------------------------------------------------------------------*/

           }  /*  End:  "if ((feof (file_ptr) == 0) && ..."  */


/*--------------------------------------------------------------------------*/
/**        IF filler was found in the first two bytes, OR a keyword was    **/
/**                the first word in the file, THEN                        **/
/**            The file contains stream records.                           **/
/**        ELSE                                                            **/
/**            IF the first two bytes are null, there is filler in the     **/
/**                    keyword area, or a keyword is found after the first **/
/**                    two bytes, THEN                                     **/
/**                The file contains VMS variable length records.          **/
/**            ENDIF                                                       **/
/**        ENDIF                                                           **/
/*--------------------------------------------------------------------------*/

           if (filler_in_byte_area || keyword_found_first)
               is_var_record_file = FALSE;
           else
               if (first_bytes_are_null || filler_in_keyword_area || 
                                                   keyword_after_bytes)
                   is_var_record_file = TRUE;


/*--------------------------------------------------------------------------*/
/**        Close the input file.                                           **/
/*--------------------------------------------------------------------------*/

           fclose (file_ptr);

/*--------------------------------------------------------------------------*/
/**    ENDIF                                                               **/
/*--------------------------------------------------------------------------*/

       }  /*  End:  "if (file_ptr = fopen (file_name ..."  */

/*--------------------------------------------------------------------------*/
/** ENDIF                                                                  **/
/*--------------------------------------------------------------------------*/

    }  /*  End:  "if (fio_size (file_name..."  */

/*--------------------------------------------------------------------------*/
/** RETURN a flag indicating the type of file.                             **/
/*--------------------------------------------------------------------------*/

    return (is_var_record_file);

/*--------------------------------------------------------------------------*/
/** END                                                                    **/
/*--------------------------------------------------------------------------*/

}  /*  End:  "fio_has_var_records"  */  


/**********************************************************************
 *$Component                                                          *
 *    LOGICAL fio_read_file (file_name)                               *
 *$Abstract                                                           *
 *    Reads the contents of a file into memory                        *
 *$Keywords                                                           *
 *    FIOLIB                                                          *
 *    READ                                                            *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    file_name:                                                      *
 *        The file_name variable is a general purpose character       *
 *        string containing a file specification.                     *
 *$Outputs                                                            *
 *    NONE                                                            *
 *$Returns                                                            *
 *    file_string:                                                    *
 *        The file_string variable is a pointer to a block of memory  *
 *        containing the entire contents of an ASCII file.            *
 *$Detailed_Description                                               *
 *    The fio_read_file routine reads the contents of a file into     *
 *    dynamic memory and returns a pointer to the memory block.       *
 *$Side_Effects                                                       *
 *    This routine allocates memory which must be deallocated later.  *
 *$Error_Handling                                                     *
 *    If this routine cannot allocate the memory it needs, the global *
 *    out of memory flag is set.  If the file cannot be opened or     *
 *    its size cannot be determined, messages are appended onto the   *
 *    global list.                                                    *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore/JPL                                             *
 *$Version_and_Date                                                   *
 *    1.6   March 16, 1992                                            *
 *$Change_History                                                     *
 *    MDD   09-18-90   Original code.                                 *
 *    MDD   10-01-90   Added check for directories                    *
 *    MDD   10-02-90   Humanized error messages                       *
 *    DPB   02-04-91   Removed reference to pds_out_of_memory.        *
 *                     (Modified external references)                 *
 *    MDD   05-01-91   Changed to use fio_size                        *
 *    MDD   10-17-91   Replaced malloc with Malloc_String             *
 *    MDD   03-16-92   The great int -> long conversion               *
 **********************************************************************/

char *fio_read_file (file_name)

char *file_name;
{
   long file_length;
   char *file_string = NULL;
   FILE *file_ptr;	

   /** BEGIN                                                         **/
   /*-----------------------------------------------------------------*/
   /** get status info about the file (size and mode)                **/
   /*-----------------------------------------------------------------*/

   file_length = fio_size (file_name);

   /*-----------------------------------------------------------------*/
   /** IF the input file is not a directory and can be opened THEN   **/
   /*-----------------------------------------------------------------*/

   if (file_length != -1 && (file_ptr = fopen (file_name, "rb")) != NULL) 
   {

      /*--------------------------------------------------------------*/
      /** allocate memory for contents of entire file                **/
      /** read the file into memory                                  **/
      /** close the input file                                       **/
      /*--------------------------------------------------------------*/

      Malloc_String(file_string, (int) file_length);
      fread (file_string, sizeof(char), (int) file_length, file_ptr);
      *(file_string + file_length) = '\0';
      fclose (file_ptr);
   }                                                                         
   /*-----------------------------------------------------------------*/
   /** ELSE                                                          **/
   /**    add an error message to global list                        **/
   /*-----------------------------------------------------------------*/

   else
   {
       err_append_message (ERROR1, "The file you specified could not be found.");
   }
   /*-----------------------------------------------------------------*/
   /** ENDIF input file can be opened...                             **/      
   /** RETURN pointer to contents of file                            **/
   /*-----------------------------------------------------------------*/

   return (file_string);

   /** END fio_read_file                                             **/  

}


/**********************************************************************
 *$Component                                                          *
 *    LOGICAL fio_setup ()                                            *
 *$Abstract                                                           *
 *    Performs general toolbox setup functions.                       *
 *$Keywords                                                           *
 *    FIOLIB                                                          *
 *    SETUP                                                           *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    None                                                            *
 *$Outputs                                                            *
 *    None                                                            *
 *$Returns                                                            *
 *    None                                                            *
 *$Detailed_Description                                               *
 *    The fio_setup routine performs all of the general setup         *
 *    functions required by the PDS toolbox utilities.                *
 *$Error_Handling                                                     *
 *    If the setup file cannot be found or read, or if the scratch    *
 *    directory keyword is missing, then the user is informed and     *
 *    the scratch directory is assumed to be the user's default       *
 *    login.  If there is something wrong with the scratch directory  *
 *    then the user is informed via a warning on the global message   *
 *    list and on the screen, and this routine returns NULL.          *
 *$External_References                                                *
 *    Item                     Shared-Data           Access           *
 *    ------------------------------------------------------------    *
 *    pds_temp_data_fname      pdsglob.h             update           *
 *    pds_temp_label_fname     pdsglob.h             update           *
 *    pds_scratch_directory    pdsglob.h             update           *
 *$Author_and_Institution                                             *
 *    David P. Bernath                                                *
 *$Version_and_Date                                                   *
 *    2.0   March 20, 1992                                            *
 *$Change_History                                                     *
 *    DPB   03-13-91   Original code.                                 *
 *    MDD   03-14-91   Changed called to util_catenate_directory to   *
 *                     util_create_file_spec                          *
 *    MDD   05-05-91   Removed two unnecessary files                  *
 *    MDD   10-17-91   Replaced sys_exit_system Exit_System() and     *
 *                     removed redirect and map file name stuff.      *
 *    MDD   03-20-92   Changed to setup pds_scratch_directory.        *
 *    MDC   11-18-04   De-allocate temp files if scratch directory is *
 *                     not writeable.                                 *
 *    MDC   01-03-05   Modify routine to create scratch files onto    *
 *                     the current working directory since some users *
 *                     run the PDS tools on a multi-user Windows      *
 *                     system without access to the root hard drive   *
 **********************************************************************/

LOGICAL fio_setup ()

{
    LOGICAL success = {TRUE};
    char scratch_directory [PDS_MAXLINE + 1];
    LOGICAL setup_found = {FALSE};
    FILE *test_file = {NULL};
    char err_msg [PDS_MAXLINE + 1];

/** BEGIN **/

    /*----------------------------------------------------------------------*/
    /** Get the name of the scratch directory from the setup file.         **/
    /** If it is supposed to be system scratch, then set it to the system  **/
    /**    scratch directory name.                                         **/
    /** If it is supposed to be the default login, then set it.            **/
    /** If the setup didn't specify, and there is a system scratch         **/
    /**    then use that. Otherwise, use the default login.                **/
    /*----------------------------------------------------------------------*/
    setup_found = fio_get_setup_value (PDS_SCRATCH_DIR_KEYWORD, 
                  scratch_directory);
    if (setup_found && strcmp (scratch_directory, "SYSTEM_SCRATCH") == 0)
       strcpy (scratch_directory, PDS_SYSTEM_SCRATCH);

    else if (setup_found && strcmp (scratch_directory, "DEFAULT_LOGIN") == 0)
       strcpy (scratch_directory, PDS_DEFAULT_LOGIN);

    else if (!setup_found && !util_string_is_empty (PDS_SYSTEM_SCRATCH))
       strcpy (scratch_directory, PDS_SYSTEM_SCRATCH);

#ifndef MSDOS_TC
    else if (!setup_found)
       strcpy (scratch_directory, PDS_DEFAULT_LOGIN);
#endif
    /*----------------------------------------------------------------------*/
    /** If we ended up with any scratch directory at all, set the global   **/
    /**   scratch directory variable to it. Otherwise, we'll leave the     **/
    /**   global empty, which to some systems will mean "use the current   **/
    /**   directory, and to others will mean "use the system scratch."     **/
    /*----------------------------------------------------------------------*/

	/* 01-03-2005 MDC - We want to create the scratch files into the current
	   working directory since recently users have complained that in the
	   Windows versions of the PDS tools, they cannot write to a C drive
	   because of permissions issues.
    */


    if (!util_string_is_empty (scratch_directory))
    {
       Malloc_String(pds_scratch_directory, String_Size(scratch_directory));
       strcpy (pds_scratch_directory, scratch_directory);
    }


    /*----------------------------------------------------------------------*/
    /** Construct the filespecs for all toolbox scratch files.             **/
    /*----------------------------------------------------------------------*/

    pds_temp_data_fname = sys_make_temp_fname (scratch_directory);
    pds_temp_label_fname = sys_make_temp_fname (scratch_directory);

    /*----------------------------------------------------------------------*/
    /** Attempt to open one of them. If it doesn't work, all we can do at  **/
    /** This point is attempt to warn the user.                            **/
    /*----------------------------------------------------------------------*/

    test_file = fopen (pds_temp_data_fname, "w");
    success = (test_file != NULL);
    if (!success)
    {
       printf ("WARNING: Scratch directory %s does not appear to be writeable\n", 
                pds_scratch_directory);
       sprintf (err_msg, "Scratch directory %s does not appear to be writeable", 
                pds_scratch_directory);
       err_append_message (WARNING, err_msg);

	   /* 11-18-04 MDC - NOT SURE IF THIS IS THE RIGHT SOLUTION YET!!! */
	   Lemme_Go(pds_temp_data_fname);
	   Lemme_Go(pds_temp_label_fname);
	   Lemme_Go(pds_scratch_directory);
    }
    else
       success = fclose (test_file);

    /*----------------------------------------------------------------------*/
    /** RETURN success flag                                                **/
    /*----------------------------------------------------------------------*/

    return (success);

/** END **/

}  /*  "fio_setup"  */






/**********************************************************************
 *$Component                                                          *
 *    long fio_size (file_name)                                       *
 *$Abstract                                                           *
 *    Counts the total bytes in a file.                               *
 *$Keywords                                                           *
 *    FILE                                                            *
 *    BYTE                                                            *
 *    COUNT                                                           *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    file_name:                                                      *
 *        The file_name variable is a general purpose character       *
 *        string containing a file specification.                     *
 *$Outputs                                                            *
 *    None                                                            *
 *$Returns                                                            *
 *    file_size:                                                      *
 *        The file_size variable is the number of bytes in a file.    *
 *$Detailed_Description                                               *
 *    The fio_size routine determines the total number of bytes in    *
 *    a file.  If it is unable to do this, it returns -1.  (It can't  *
 *    use zero to indicate failure since zero is a valid file size).  *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Herbert C. Gamble / J.P.L.                                      *
 *$Version_and_Date                                                   *
 *    2.1   May 26, 1991                                              *
 *$Change_History                                                     *
 *    HCG   03-08-91   Original Code.                                 *
 *    DPB   04-05-91   Modified header, re-structured code, and moved *
 *                     routine from lablib.                           *
 *    MDD   05-16-91   Changed file_size from unsigned to int because *
 *                     the PC didn't like it. Then added cast to the  *
 *                     file size assignment.                          *
 **********************************************************************/

long fio_size (file_name)

char *file_name;

{
    FILE *file_ptr = {NULL};
    char bad_file_message [PDS_MAXLINE];
    char file_is_dir_message [PDS_MAXLINE];
    long file_size = {-1};

#ifndef MAC_THINK
    struct stat statbuf;
  
    sprintf (bad_file_message,
             "Unable to determine the size of file (%s)", file_name);

    sprintf (file_is_dir_message, "File (%s) is a directory", file_name);

    if ((file_ptr = fopen (file_name, "r")) == NULL)
                      err_append_message (WARNING, bad_file_message);
    else
    {
        if (stat (file_name, &statbuf) != 0)
            err_append_message (WARNING, bad_file_message);
        else
        {
            if (statbuf.st_mode & S_IFDIR)
            {
                err_append_message (WARNING, bad_file_message);
                err_append_message (WARNING, file_is_dir_message);
            }
            else
		 file_size = statbuf.st_size;

         }  
         fclose (file_ptr);
         
   } 

#endif
    return (file_size);

}

/**********************************************************************
 *$Component                                                          *
 *    static char *fio_get_term (file_ptr, record_type)               *
 *$Abstract                                                           *
 *    Returns the record terminator for the given file.               *
 *$Keywords                                                           *
 *    FIOLIB                                                          *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    file_ptr:                                                       *
 *        The file_ptr variable contains a pointer to a FILE          *
 *        structure.                                                  *
 *    record_type:                                                    *
 *        The record_type variable is an integer that represents      *
 *        the type of records a file contains: e.g., PDS_RF_STREAM_LF,*
 *        PDS_RF_FIXED_CRLF, etc.                                     *
 *$Outputs                                                            *
 *    None                                                            *
 *$Returns                                                            *
 *    term_string:                                                    *
 *        The term_string variable is a string containing a sequence  *
 *        of characters used as a record terminator in a file. The    *
 *        possible values are <CR>, <CR><LF>, <LF> and the empty      *
 *        string.                                                     *
 *$Detailed_Description                                               *
 *    The fio_get_term routine attempts to determine the terminator   *
 *    used to separate records in the given file. If the input        *
 *    record type is specific enough to provide this information,     *
 *    then this routine will return the record terminator that matches*
 *    The record type.  Otherwise, it will attempt to determine the   *
 *    record terminator by reading the first record of the file. The  *
 *    file will then be rewound to the beginning.                     *
 *    The record types that can be input (as integer constants) are:  *
 *                   PDS_RF_RMS_VAR                                   *
 *                   PDS_RF_RMS_STREAM                                *
 *                   PDS_RF_STREAM_LF                                 *
 *                   PDS_RF_STREAM_CR                                 *
 *                   PDS_RF_STREAM_CRLF                               *
 *                   PDS_RF_FIXED_LF                                  *
 *                   PDS_RF_FIXED_CR                                  *
 *                   PDS_RF_FIXED_CRLF                                *
 *                   PDS_RF_BINARY                                    *
 *                   PDS_RF_ASCII                                     *
 *$External_References                                                *
 *    None                                                            *
 *$Error_Handling                                                     *
 *    If no record terminator is found, then this routine returns the *
 *    empty string, and a message is appended to the global message   *
 *    list. This routine will give up trying to find a terminator     *
 *    after it has read PDS_MAX_SANE_RECORD characters.               *
 *$Side_Effects                                                       *
 *     This routine allocates memory for the return value which must  *
 *     be deallocated elsewhere.                                      *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore / J.P.L.                                        *
 *$Version_and_Date                                                   *
 *    1.0   May 11, 1992                                              *
 *$Change_History                                                     *
 *    MDD   05-11-92   Original code.                                 *
 **********************************************************************/

char *fio_get_term (file_ptr, input_record_type)

FILE *file_ptr;
int input_record_type;

{
   char *term_string = NULL;
   LOGICAL done = FALSE;
   LOGICAL find_it = FALSE;
   LOGICAL cr_found = FALSE;
   LOGICAL lf_found = FALSE;
   long sanity_count = 0;
   int c;

/** BEGIN **/

   /*-----------------------------------------------------------------*/
   /** Allocate memory for the return value                          **/
   /*-----------------------------------------------------------------*/

   Malloc_String(term_string, 3);

   /*-----------------------------------------------------------------*/
   /** IF this is a VAX THEN                                         **/
   /**    records are either binary (no terminator) or ASCII (with   **/
   /**       a LF terminator) so set the return value appropriately. **/
   /*-----------------------------------------------------------------*/

#ifdef VAX
   switch (input_record_type)
   {
      case PDS_RF_BINARY:     strcpy (term_string, "");
			      break;

      default:                strcpy (term_string, PDS_LF_STRING);
			      break;
   }

#else

   /*-----------------------------------------------------------------*/
   /** ELSE                                                          **/
   /**    assign the terminator based on record type, OR set the     **/
   /**       flag indicating it must be searched for.                **/
   /*-----------------------------------------------------------------*/

   switch (input_record_type)
   {
      case PDS_RF_RMS_VAR:
      case PDS_RF_BINARY:     strcpy (term_string, "");
			      break;

      case PDS_RF_STREAM_CR:
      case PDS_RF_FIXED_CR:   strcpy (term_string, PDS_CR_STRING);
			      break;

      case PDS_RF_RMS_STREAM:
      case PDS_RF_STREAM_CRLF:
      case PDS_RF_FIXED_CRLF: strcpy (term_string, PDS_CRLF_STRING);
                              break;

      case PDS_RF_STREAM_LF:
      case PDS_RF_FIXED_LF:   strcpy (term_string, PDS_LF_STRING);
			      break;

      case PDS_RF_ASCII:
      case PDS_RF_UNKNOWN:    find_it = TRUE;
			      break;

      default:                find_it = TRUE;
			      break;
   }

#endif

   /*-----------------------------------------------------------------*/
   /** ENDIF                                                         **/
   /** IF we must search the input file for terminators THEN         **/
   /*-----------------------------------------------------------------*/

   if (find_it)
   {

      /*--------------------------------------------------------------*/
      /** WHILE not at end of file and terminator not found and      **/
      /**    it's not time to give up DO                             **/
      /*--------------------------------------------------------------*/

       while (!feof (file_ptr) && !done && sanity_count < PDS_MAX_SANE_RECORD)
       {

	 /*--------------------- -------------------------------------*/
	 /** Get a character                                         **/
	 /** IF it's a CR THEN set CR flag && get another character  **/
	 /** IF we have a LF then set LF flag                        **/
	 /** IF we found either THEN we are done                     **/
	 /*-----------------------------------------------------------*/

	  c = fgetc (file_ptr);
	  sanity_count++;
	  if ((char) c == '\r')
	  {
	     cr_found = TRUE;
	     if (!feof (file_ptr)) c = fgetc (file_ptr);
	  }
	  if ((char) c == '\n')
	     lf_found = TRUE;
	  done = (cr_found || lf_found);
       }

      /*--------------------------------------------------------------*/
      /** ENDWHILE                                                   **/
      /** IF no terminator was found THEN                            **/
      /**    issue a warning                                         **/
      /*--------------------------------------------------------------*/

       if (!done)
       {
	  strcpy (term_string, "");
	  err_append_message (WARNING,
	     "No record terminators found in file during conversion.");
	  err_append_message (WARNING,
	     "Results of file conversion may be incorrect.");
       }

      /*--------------------------------------------------------------*/
      /** ELSE                                                       **/
      /**    assign the terminator based on the flags we set         **/
      /*--------------------------------------------------------------*/

       else if (cr_found && lf_found)
	  strcpy (term_string, PDS_CRLF_STRING);
       else if (cr_found)
	  strcpy (term_string, PDS_CR_STRING);
       else if (lf_found)
	  strcpy (term_string, PDS_LF_STRING);

      /*--------------------------------------------------------------*/
      /** ENDIF                                                      **/
      /** reset the file to the beginning                            **/
      /*--------------------------------------------------------------*/

       rewind (file_ptr);
   }
   /*-----------------------------------------------------------------*/
   /** ENDIF we must search...                                       **/
   /*-----------------------------------------------------------------*/

   return (term_string);

/** END **/
}


/**********************************************************************
 *$Component                                                          *
 *    static char *fiox_read_line (file_ptr, record_length,           *
 *                               record_type, char_count, term_found) *
 *$Abstract                                                           *
 *    Reads a line from a file.                                       *
 *$Keywords                                                           *
 *    FIOLIB                                                          *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    file_ptr:                                                       *
 *        The file_ptr variable contains a pointer to a FILE          *
 *        structure.                                                  *
 *    record_length:                                                  *
 *        The record_length variable is an integer specifying the     *
 *        maximum number of characters allowed in a record.           *
 *    record_type:                                                    *
 *        The record_type variable is an integer that represents      *
 *        the type of records a file contains: e.g., PDS_RF_STREAM_LF,*
 *        PDS_RF_FIXED_CRLF, etc.                                     *
 *    term_string:                                                    *
 *        The term_string variable is a string containing a sequence  *
 *        of characters used as a record terminator in a file. The    *
 *        possible values are <CR>, <CR><LF>, <LF> and the empty      *
 *        string.                                                     *
 *$Outputs                                                            *
 *    char_count:                                                     *
 *        The char_count variable is a count of the number of         *
 *        characters read.                                            *
 *    term_found:                                                     *
 *        The term_found variable is a flag which indicates that a    *
 *        record terminator was found while reading the line.         *
 *$Returns                                                            *
 *    text:                                                           *
 *        The text variable is a general purpose character string     *
 *        that may contain one or more characters.                    *
 *$Detailed_Description                                               *
 *    The fiox_read_line routine reads a line from a file, using the  *
 *    method appropriate to the record type passed in and the         *
 *    system on which the program is being run.                       *
 *    The record types that can be input (as integer constants) are:  *
 *                   PDS_RF_RMS_VAR                                   *
 *                   PDS_RF_RMS_STREAM                                *
 *                   PDS_RF_STREAM_LF                                 *
 *                   PDS_RF_STREAM_CR                                 *
 *                   PDS_RF_STREAM_CRLF                               *
 *                   PDS_RF_FIXED_LF                                  *
 *                   PDS_RF_FIXED_CR                                  *
 *                   PDS_RF_FIXED_CRLF                                *
 *                   PDS_RF_BINARY                                    *
 *$Limitations                                                        *
 *    RMS variable length records processed on non VMS systems will   *
 *    return however many characters are in the record, regardless    *
 *    of the record length passed in.                                 *
 *$Side_Effects                                                       *
 *    Storage is allocated which must be deallocated elsewhere.       *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    David P. Bernath / J.P.L.                                       *
 *$Version_and_Date                                                   *
 *    2.3   May 11, 1992                                              *
 *$Change_History                                                     *
 *    DPB   09-24-91   Original code.                                 *
 *    DPB   10-21-91   Re-wrote routine to accomodate very long       *
 *                     records.  Also changed its name and moved it   *
 *                     out of fiolib.                                 *
 *    DPB   10-31-91   Added two new outputs.                         *
 *    MDD   03-16-92   The great int -> long conversion               *
 *    MDD   05-11-92   Added term_string input and removed the switch *
 *                     statement that used to set the terminator.     *
 *                     Corrected bug that left an extra NULL char on  *
 *                     records with odd length.                       *
 **********************************************************************/

static char *fiox_read_line (file_ptr, record_length,
			     record_type, char_count, term_found,
			     term_string)

FILE *file_ptr;
long record_length;
int record_type;
long *char_count;
LOGICAL *term_found;
char *term_string;

{
			       /*-------------------------------------*/
    union convert_union        /*  This union is used to convert the  */
    {                          /*  two byte record size into an       */
	char char_part [4];    /*  integer value.  The character part */
	int int_part;          /*  is 4 bytes long because on         */
			       /*  machines with 4 byte integers, the */
    } conversion;              /*  first two bytes must be padded     */
			       /*  with binary zeros.                 */
			       /*-------------------------------------*/

    char *text = {NULL};
    char *temp = {NULL};
    char bytes [3];
    char c;
    char term_char;
    long i;
    long len;
    int int_size = {sizeof(int)};
    LOGICAL done = {FALSE};
    LOGICAL success = {FALSE};
    LOGICAL record_was_odd = FALSE;

/** BEGIN **/

    /*-----------------------------------------------------------------------*/
    /** Adjust the record length, initialize the text string and term_char  **/
    /*-----------------------------------------------------------------------*/

    *term_found = FALSE;
    if (record_length <= 1) record_length = 0;
    Malloc_String(text, 1)
    if (term_string && *term_string)
       term_char = *String_End(term_string);
    else
       term_char = EOS;

#ifndef VAX
    /*-----------------------------------------------------------------------*/
    /** IF this is not a VAX, OR its a VAX and the record type is not       **/
    /**     RMS_VAR THEN                                                    **/
    /*-----------------------------------------------------------------------*/

    if (record_type != PDS_RF_RMS_VAR)
    {
#endif
	/*-------------------------------------------------------------------*/
	/** LOOP through the file, one character at a time . . .            **/
	/*-------------------------------------------------------------------*/

	for (i=0, len=0, *char_count=0; ((*char_count < PDS_MAX_SANE_RECORD) &&
		! done && ! feof(file_ptr) && ! ferror(file_ptr)); ++len)
	{
	    /*---------------------------------------------------------------*/
	    /** Read a character from the file                              **/
	    /*---------------------------------------------------------------*/

	    c = fgetc (file_ptr);
	    ++(*char_count);

	    if ((c == EOF) && feof(file_ptr))
	    {
		c = EOS;
		--(*char_count);
	    }

	    /*---------------------------------------------------------------*/
	    /** IF the end of the record or the end of the file has been    **/
	    /**     reached THEN prepare to exit the loop                   **/
	    /** ELSE                                                        **/
            /**     Allocate more space if necessary and store the          **/
            /**         character read into the text string.                **/
            /** ENDIF                                                       **/
            /*---------------------------------------------------------------*/

            if (((term_char != EOS) && (c == term_char)) ||
                   ((record_length > 0) && (len >= (record_length-1))))
            {
                done = TRUE;
                *term_found = (c == term_char);
            }

            if (len%(PDS_BUFFLEN - 1) == 0) 
            {
                if (record_type != PDS_RF_BINARY)
		    Realloc_String(text, ((++i)*PDS_BUFFLEN) + 1)
                else
                {
		    Malloc_String(temp, ((++i)*PDS_BUFFLEN) + 1)
                    util_byte_copy(temp, text, len);
                    Lemme_Go(text)
                    text = temp;
                }
            }

	    *(text + len) = c;

        /*-------------------------------------------------------------------*/
        /** ENDLOOP                                                         **/
        /*-------------------------------------------------------------------*/

        }  /*  End:  "for (i=0, len=0; ! done; ++len) ..."  */

	*(text + len) = EOS;
    
        /*-------------------------------------------------------------------*/
        /** Set the success flag and terminate the text string              **/
        /*-------------------------------------------------------------------*/

        if (! ferror(file_ptr))
            success = (! feof(file_ptr) || (feof(file_ptr) && (*text != EOS)));

#ifndef VAX

    /*-----------------------------------------------------------------------*/
    /** ELSE                                                                **/
    /*-----------------------------------------------------------------------*/
    }
    else
    {
        /*-------------------------------------------------------------------*/
        /** Read in two bytes.  These two bytes, when translated into an    **/
        /**     integer, are the number of bytes in the actual record.      **/
        /*-------------------------------------------------------------------*/

        fread (bytes, sizeof(char), 2, file_ptr);

        /*-------------------------------------------------------------------*/
        /** IF they were read in ok THEN                                    **/
        /*-------------------------------------------------------------------*/

        if (! feof(file_ptr) && ! ferror(file_ptr))
        {
            /*---------------------------------------------------------------*/
            /** Load the two bytes into the used to convert them into an    **/
            /**     integer.  Please note that the following IF statement   **/
            /**     is here to accomodate both two byte integer and four    **/
            /**     byte integer machines.  On two byte machines, the       **/
            /**     record size bytes are swapped, then copied directly     **/
            /**     into the leading characters of the conversion union.    **/
            /**     On four byte machines, the leading characters must be   **/
            /**     padded with binary zeros before the record size bytes   **/
            /**     are swapped then copied into the trailing characters.   **/
	    /*---------------------------------------------------------------*/

	    if (int_size == 2)
	    {
#ifdef MSDOS_TC
		conversion.char_part[0] = bytes [0];
		conversion.char_part[1] = bytes [1];
#else
		conversion.char_part[0] = bytes [1];
		conversion.char_part[1] = bytes [0];
#endif

            }
            else
            {
                conversion.char_part[0] = EOS;
                conversion.char_part[1] = EOS;
                conversion.char_part[2] = bytes [1];
                conversion.char_part[3] = bytes [0];
            }
            
            /*---------------------------------------------------------------*/
            /** This is where the bytes are converted into an integer.      **/
            /*---------------------------------------------------------------*/

	    *char_count = (long) conversion.int_part;

            /*---------------------------------------------------------------*/
            /** If the byte count is odd, the trailing ASCII NULL character **/
            /**     must be skipped.                                        **/
            /*---------------------------------------------------------------*/
            
            if (IsOdd(*char_count)) 
            {
               record_was_odd = TRUE;
               ++*char_count;
	    }
            
            /*---------------------------------------------------------------*/
            /** IF the byte count is zero, then we have a blank line, and   **/
            /**     no characters need to be read.                          **/
            /** ELSE                                                        **/
            /*---------------------------------------------------------------*/

            if (*char_count <= 0)
                *text = EOS;
            else
            {
                /*-----------------------------------------------------------*/
                /** Allocate storage for the record, and read it in.        **/
                /*-----------------------------------------------------------*/

		Malloc_String(text, (int) *char_count + 2);
		fread (text, sizeof(char), *char_count, file_ptr);
                if (record_was_odd) --*char_count;
		*(text + *char_count) = EOS;

            /*---------------------------------------------------------------*/
            /** ENDIF                                                       **/
            /*---------------------------------------------------------------*/

            }  /*  End:  "if (byte_  ... else ..."  */
    
        /*-------------------------------------------------------------------*/
        /** ENDIF                                                           **/
        /*-------------------------------------------------------------------*/

        }  /*  End:  "if (! feof(file_ptr) && ..."  */

        /*-------------------------------------------------------------------*/
        /** Set the success flag.                                           **/
        /*-------------------------------------------------------------------*/

        success = (! ferror(file_ptr) && ! feof(file_ptr));

    /*-----------------------------------------------------------------------*/
    /** ENDIF                                                               **/
    /*-----------------------------------------------------------------------*/

    }

    /*-----------------------------------------------------------------------*/
    /** End of the non-VMS/VAX code.                                        **/
    /*-----------------------------------------------------------------------*/

#endif

    /*-----------------------------------------------------------------------*/
    /** RETURN a text pointer.                                              **/
    /*-----------------------------------------------------------------------*/

    if (! success) Lemme_Go(text);

    return (text);

/** END **/

}  /*  "fiox_read_line"  */

