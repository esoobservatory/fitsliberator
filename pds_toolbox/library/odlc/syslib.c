/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Component                                                           * 
 *    Library syslib.c                                                 *
 * Abstract                                                            *
 *    System-level PDS utilities                                       *
 * Detailed Description                                                *
 *    The syslib contains the subroutines used by PDS software to      *
 *    perform and manipulate a variety of System-level calls to the    *
 *    operating system.                                                *
 * Internal References                                                 *
 *    System manipulation routines:                                    *
 *        sys_copy_file                                                *
 *        sys_delete_file                                              *
 *        sys_do_command                                               *
 *        sys_get_ascii_date                                           *
 *        sys_get_date                                                 *
 *        sys_get_file_list                                            *
 *        sys_get_path                                                 *
 *        sys_make_temp_fname                                          *
 * Authors and Institutions                                            *
 *    Herbert C. Gamble / J.P.L.                                       *
 *    David P. Bernath / J.P.L.                                        *
 *    Marti D. DeMore / J.P.L.                                         *
 *    Kristy L. Marski / J.P.L.                                        * 
 * Version and Date                                                    *
 *    4.0 April 21, 1992                                               *
 * Change History                                                      *
 *    HCG   06-30-90   Gathered together the various routines that     *
 *                     make up the library.                            *
 *    MDD   10-02-90   Added casting for SUN compiler and humanized    *
 *                     error messages                                  *
 *    DPB   10-04-90   Added sys_exit_system.                          *
 *    HCG   10-12-90   Added sys_check_directory_integrity.            *
 *    HCG   10-22-90   Added sys_browser_setup.                        *
 *    MDD   10-25-90   Removed sys_browser_setup                       *
 *    MDD   03-15-91   Added sys_get_date, sys_get_ascii_date,         *
 *                     sys_copy_file, sys_get_user_id, and             *
 *                     sys_get_temp_fname                              *
 *    KLM   03-19-91   Rewrote sys_delete_file using sys_do_command.   *
 *    DPB   07-09-91   Added sys_get_path.                             *
 *    MDD   10-17-91   Removed sys_exit_system                         *
 *    MDD   03-20-92   Remove sys_check_directory_integrity            *
 *    MDD   04-16-92   Port to Macintosh, first round.                 *
 *    MDD   04-21-92   Removed sys_get_directory_list, sys_get_user_id,*
 *                     sys_get_current_directory, and sys_change_      *
 *                     directory to file syssave.c                     *          
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "pdsdef.h"
#include "sysdef.h"
#include "utildef.h"
#include "errordef.h"
#include "fiodef.h"

extern char *pds_temp_data_fname;
/**********************************************************************
 *$Component                                                          *
 *    LOGICAL sys_copy_file (source_file, dest_file)                  *
 *$Abstract                                                           *
 *    Copies a file to a given destination.                           *
 *$Keywords                                                           *
 *    SYSLIB                                                          *
 *    OS                                                              *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    source_file:                                                    *
 *        The source_file variable is a string containing the name of *
 *        a file that is to be used as the source, or original, for   *
 *        a file operation.                                           *
 *    dest_file:                                                      *
 *        The dest_file variable is a string containing the name of   *
 *        a file that is to be used as the destination, or result,    *
 *        for a file operation.                                       *
 *$Outputs                                                            *
 *    NONE.                                                           *
 *$Returns                                                            *
 *    success_flag:                                                   *
 *        The success_flag variable is a general purpose variable     *
 *        which indicates success (a value of TRUE) or failure.       *
 *$Detailed_Description                                               *
 *    The sys_copy_file routine will copy the file passed in as input *
 *    source file to the location passed in as dest file.  This       *
 *    function will work in VMS, SUN UNIX, and MSDOS (Turbo C).  On   *
 *    VMS and Unix systems, it spawns a process and uses the system   *
 *    command for copying. On PC and MAC systems, the file is copied  *
 *    by reading the source file and writing to the destination file. *
 *$External_References                                                *
 *    None                                                            *
 *$Error_Handling                                                     *
 *    If any error occurs while issuing the copy command, the return  *
 *    value is set to FALSE and a message is appended to the global   *
 *    list.                                                           *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore/JPL                                             *
 *$Version_and_Date                                                   *
 *    1.1   April 16, 1992                                            *
 *$Change_History                                                     *
 *    MDD   03-15-91   Original Code.                                 *
 *    MDD   04-16-92   Added MAC_THINK compiler ifdefs.               *
 **********************************************************************/

LOGICAL sys_copy_file (source_file, dest_file)

char *source_file;
char *dest_file;

{
   char command_str [PDS_MAXLINE];
   LOGICAL success = TRUE;

   /*----------------------------------------------------------------*/
   /** BEGIN                                                        **/
   /**    issue the copy command based on the current OS            **/
   /*----------------------------------------------------------------*/

#ifdef VAX
   sprintf (command_str, "copy %s %s", source_file, dest_file);
   success = sys_do_command (command_str);
#endif

#ifdef MSDOS_TC
   success = fio_copy_file (source_file, dest_file);
#endif

#ifdef MAC_THINK
   success = fio_copy_file (source_file, dest_file);
#endif

#ifdef SUN_UNIX
   sprintf (command_str, "cp %s %s", source_file, dest_file);
   success = sys_do_command (command_str);
#endif

   /*----------------------------------------------------------------*/
   /**    IF there was an error in the system command THEN          **/
   /**       append an error message to the global list             **/ 
   /*----------------------------------------------------------------*/

   if (success != TRUE)
   {
      sprintf (command_str, "Unable to copy file %s to %s", source_file, 
                  dest_file);
      err_append_message (ERROR1, command_str);   
   }

   /*----------------------------------------------------------------*/
   /**    ENDIF there was an error...                               **/
   /** END sys_copy_file                                            **/  
   /*----------------------------------------------------------------*/

   return (success);
} 


/**********************************************************************
 *$Component                                                          *
 *    LOGICAL sys_delete_file (file_name)                             *
 *$Abstract                                                           *
 *    Deletes a file.                                                 *
 *$Keywords                                                           *
 *    SYSLIB                                                          *
 *    OS                                                              *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    file_name:                                                      *
 *        The file_name variable is a string containing the name of   *
 *        a file.                                                     *
 *$Outputs                                                            *
 *    NONE.                                                           *
 *$Returns                                                            *
 *    success_flag:                                                   *
 *        The success_flag variable is a general purpose variable     *
 *        which indicates success (a value of TRUE) or failure.       *
 *$Detailed_Description                                               *
 *    The sys_delete_file function will delete the file passed in as  *
 *    input. This function will work in VMS, SUN UNIX, and            * 
 *    MSDOS (Turbo C).                                                *
 *$External_References                                                *
 *    None                                                            *
 *$Error_Handling                                                     *
 *    If any error occurs while issuing the delete command, the       *
 *    return value is set to FALSE and a message is appended to the   *
 *    global list.                                                    *
 *$Author_and_Institution                                             *
 *    Herbert C. Gamble / J.P.L.                                      *
 *$Version_and_Date                                                   *
 *    2.1   April 16, 1992                                            *
 *$Change_History                                                     *
 *    HCG   08-31-90   Original Code.                                 *
 *    MDD   10-02-90   Added casting for SUN compiler and humanized   *
 *                     error messages                                 *
 *    HCG   10-23-90   Deleted syslib definitions.                    *
 *    KLM   03-19-91   Rewrote the routine using sys_do_command.      *
 *    MDD   10-25-91   Removed Flush_File call.                       *
 *    MDD   04-16-92   Added MAC_THINK ifdefs.                        *
 **********************************************************************/

LOGICAL sys_delete_file (file_name)
char *file_name;
{
  char command_str[PDS_MAXLINE];
  LOGICAL success = TRUE;

   /*----------------------------------------------------------------*/
   /** BEGIN                                                        **/
   /**   IF a file name was given THEN                              **/
   /*----------------------------------------------------------------*/

   if (file_name != NULL)
   {

      /*-------------------------------------------------------------*/
      /**   Clear the input/output files.                           **/
      /**   create the delete command based on the current OS       **/
      /*-------------------------------------------------------------*/

      Flush_File(file_name);

#ifdef VAX
      sprintf (command_str, "delete %s;*", file_name);
      success = sys_do_command (command_str);
#endif


#ifdef MSDOS_TC
      success = remove (file_name) == 0;
#endif

#ifdef MAC_THINK
      success = remove (file_name) == 0;
#endif


#ifdef SUN_UNIX
      sprintf (command_str, "rm -f %s", file_name);
      success = sys_do_command (command_str);
#endif

      /*-------------------------------------------------------------*/
      /**    IF there was an error in the system command THEN       **/
      /**       append an error message to the global list          **/ 
      /**    ENDIF there was an error...                            **/
      /*-------------------------------------------------------------*/

      if (!success)
      {
         sprintf (command_str, "Unable to delete file %s", file_name);
         err_append_message (ERROR1, command_str);
      }   
   }

   /*----------------------------------------------------------------*/
   /** ENDIF a file name was given...                               **/
   /*----------------------------------------------------------------*/

   return (success);

   /*----------------------------------------------------------------*/
   /** END                                                          **/
   /*----------------------------------------------------------------*/
}


/**********************************************************************
 *$Component                                                          *
 *    LOGICAL sys_do_command (os_command)                             *
 *$Abstract                                                           *
 *    Issues the given operating system command.                      *
 *$Keywords                                                           *
 *    SYSLIB                                                          *
 *    OS                                                              *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    os_command:                                                     *
 *        The os_command variable is a string containing a valid      *
 *        operating system command line.                              *
 *$Outputs                                                            *
 *    NONE.                                                           *
 *$Returns                                                            *
 *    success_flag:                                                   *
 *        The success_flag variable is a general purpose variable     *
 *        which indicates success (a value of TRUE) or failure.       *
 *$Detailed_Description                                               *
 *    The sys_do_command routine issues a command to the operating    *
 *    system, interprets the return code, and returns TRUE if the     *
 *    command was a success or FALSE if it was a failure.             *
 *$External_References                                                *
 *    None                                                            *
 *$Error_Handling                                                     *
 *    If any error occurs when trying to redirect standard output or  *
 *    standard error, this routine returns FALSE and appends a        *
 *    message to the global error message list.                       *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore/JPL                                             *
 *$Version_and_Date                                                   *
 *    1.0   March 15, 1991                                            *
 *$Change_History                                                     *
 *    MDD   03-15-91   Original Code.                                 *
 *    MDD   05-05-91   Removed redirection                            *
 **********************************************************************/

LOGICAL sys_do_command (os_command)

char *os_command;
{
  int status = 0;
  LOGICAL success = TRUE;
   
  status = system (os_command);

#ifdef VAX
  if Vms_Error(status) success = FALSE;
#else          
  if (status != 0) success = FALSE;
#endif

  return (success);
} 


/**********************************************************************
 *$Component                                                          *
 *    char *sys_get_ascii_date ()                                     *
 *$Abstract                                                           *
 *    Gets an ascii string containing the date and time.              *
 *$Keywords                                                           *
 *    SYSLIB                                                          *
 *    OS                                                              *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    None                                                            *
 *$Outputs                                                            *
 *    None                                                            *
 *$Returns                                                            *
 *    date_string                                                     *
 *        The date_string variable contains an ascii string that      *
 *        represents the date and time in the format.                 *
 *$Detailed_Description                                               *
 *    The sys_get_ascii_date routine returns  an ascii string that    *
 *    contains the current date and time in the format:               *
 *        day mon dd hh:mm:ss yyyy, e.g., Thu Mar 14 16:51:46 1991.   *
 *$External_References                                                *
 *    None                                                            *
 *$Side_Effects                                                       *
 *    This routine allocates memory for the returned value which must *
 *    be freed elsewhere.                                             *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore/JPL                                             *
 *$Version_and_Date                                                   *
 *    1.1   October 17, 1991                                          *
 *$Change_History                                                     *
 *    MDD   03-15-91   Original Code.                                 *
 *    MDD   10-17-91   Fixed mallocs, frees, and sys_exit_system calls*
 **********************************************************************/

char *sys_get_ascii_date ()
{
   struct tm *tblock;
   char *date;
   char *temp_date;

   /*----------------------------------------------------------------*/
   /** BEGIN                                                        **/
   /**   get the system date and time and convert it to ascii       **/
   /*----------------------------------------------------------------*/
   
   tblock = sys_get_date ();
   temp_date = asctime (tblock);
   Lemme_Go(tblock);

   /*----------------------------------------------------------------*/
   /**   allocate memory for the string and copy it                 **/
   /**   remove the dumb carriage return                            **/
   /*----------------------------------------------------------------*/
   
   Malloc_String(date, (int) String_Size(temp_date));
   strcpy (date, temp_date);
   util_remove_char (date, '\n');
   return (date);

   /*----------------------------------------------------------------*/
   /** END sys_get_ascii_date                                       **/
   /*----------------------------------------------------------------*/
}  

/**********************************************************************
 *$Component                                                          *
 *    struct tm *sys_get_date ()                                      *
 *$Abstract                                                           *
 *    Gets  a structure containing the date and time                  *
 *$Keywords                                                           *
 *    SYSLIB                                                          *
 *    OS                                                              *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    None                                                            *
 *$Outputs                                                            *
 *    None                                                            *
 *$Returns                                                            *
 *    date_struct                                                     *
 *        The date_struct structure contains the components of a      *
 *        date and time.  It is defined in time.h.                    *
 *$Detailed_Description                                               *
 *    The sys_get_date routine returns a structure that contains the  *
 *    broken down date and time in a tm structure, e.g,               *
 *        tm_struct->year, tm_struct->hour, etc.                      *
 *$External_References                                                *
 *    None                                                            *
 *$Side_Effects                                                       *
 *    This routine allocates memory for the returned value which must *
 *    be freed elsewhere.                                             *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore/JPL                                             *
 *$Version_and_Date                                                   *
 *    1.2   October 17, 1991                                          *
 *$Change_History                                                     *
 *    MDD   03-15-91   Original Code.                                 *
 *    MDD   05-26-91   Added casts needed by PC compiler              *
 *    MDD   10-17-91   Fixed malloc, free, and sys_exit_system calls. *
 **********************************************************************/

struct tm *sys_get_date ()
{
   time_t timer;
   struct tm *tblock;
   struct tm *temp_tblock;

   /*-----------------------------------------------------------------*/
   /** BEGIN                                                         **/
   /**   get a system date and time                                  **/
   /**   allocate memory for the return value and copy the info to it**/
   /*-----------------------------------------------------------------*/

   timer = time (NULL);
   temp_tblock = localtime (&timer);
   tblock = (struct tm *) malloc (sizeof(struct tm));
   Check_Malloc(tblock);
   util_byte_copy ((char *) tblock, (char *) temp_tblock, 
                   (long) sizeof(struct tm));
   return (tblock);

   /*-----------------------------------------------------------------*/
   /** END sys_get_date                                              **/
   /*-----------------------------------------------------------------*/
}  


/**********************************************************************
 *$Component                                                          *
 *   STRING_LIST *sys_get_file_list (dir_mask)                        *
 *$Abstract                                                           *
 *    Make a list of file names.                                      *
 *$Keywords                                                           *
 *    SYSLIB                                                          *
 *    OS                                                              *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    dir_mask:                                                       *
 *        The dir_mask variable is a character string that contains   *
 *        a system specific file specification, including wildcards.  *
 *$Outputs                                                            *
 *    NONE.                                                           *
 *$Returns                                                            *
 *    file_list:                                                      *
 *        The file_list variable is a pointer to a STRING_LIST        *
 *        structure that contains a list of file names.               *
 *$Detailed_Description                                               *
 *    The sys_get_file_list routine will return a list of file names  *
 *    that match the directory mask passed in as a STRING_LIST.       *
 *    This routine is compatible with VMS, SUN UNIX, and              *
 *    MSDOS (Turbo C).                                                *
 *$External_References                                                *
 *   None                                                             *
 *$Error_Handling                                                     *
 *   The routine will return NULL if it finds no files that match     *
 *   the dir_mask or if it cannot obtain a directory listing.         *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore/JPL                                             *
 *$Version_and_Date                                                   *
 *    1.1   June 20, 1991                                             *
 *$Change_History                                                     *
 *    MDD   05-06-91   Original Code.                                 *
 *    DPB   06-20-91   Fixed a minor bug in the VAX version.          *
 **********************************************************************/

STRING_LIST *sys_get_file_list (dir_mask)

char *dir_mask;
{
  STRING_LIST *file_list = {NULL};
  char *temp_str = NULL;

/** BEGIN **/

#ifdef MSDOS_TC
{
   HANDLE            hSearch;
   HANDLE            handle_save;
   WIN32_FIND_DATA   ffblk;                               /*11-07-97*/

   char *directory = NULL;
   int got_handle = 0;
   BOOL got_another;

   directory = sys_get_path (dir_mask);
   hSearch = FindFirstFile(dir_mask, &ffblk);
   if (hSearch !=  INVALID_HANDLE_VALUE)
   {
	   got_handle = 1;
	   handle_save = hSearch;
   }
   while (got_handle == 1)
   {
      if (ffblk.dwFileAttributes != FA_DIREC)
      {
         temp_str = util_create_file_spec (directory, ffblk.cFileName);
         file_list = util_append_string_list (file_list, temp_str,
                     STRING_TYPE);
         Lemme_Go(temp_str);
      }
      got_another = FindNextFile (hSearch, &ffblk);
	  if (got_another != TRUE)
	  {
		  got_handle = 0;
	  }
   }
   if (got_handle == 0)
   {
	   FindClose(handle_save);
   }
   Lemme_Go(directory);
}

#else
{
  FILE *fp = NULL;
  char command_str [PDS_MAXLINE + 1];
  char data_str [PDS_MAXLINE + 1];
  LOGICAL success = TRUE;

   /*----------------------------------------------------------------*/
   /** create the directory command based on the OS,                **/
   /**    redirecting the output to a file                          **/
   /*----------------------------------------------------------------*/

#ifdef VAX
   sprintf (command_str, 
               "dir/col=1/ver=1/nohead/notrail/exclude=*.dir/out=%s %s",
               pds_temp_data_fname, dir_mask);
#endif

#ifdef SUN_UNIX
   sprintf (command_str, "ls -1F %s > %s", dir_mask, pds_temp_data_fname);
#endif

   /*----------------------------------------------------------------*/
   /** issue the command to the system                              **/
   /** IF there was an error in the system command THEN             **/
   /**    append an error message to the global list                **/ 
   /*----------------------------------------------------------------*/

   success = sys_do_command (command_str);
   if (success != TRUE)
   {
      sprintf (command_str, "Unable to get list of files specified by: %s", 
                             dir_mask);
      err_append_message (ERROR1, command_str);   
   }
   /*----------------------------------------------------------------*/
   /** ELSE                                                         **/
   /*----------------------------------------------------------------*/

   else
   {
      /*-------------------------------------------------------------*/
      /** IF the redirect file can't be opened THEN                 **/
      /**    append an error to the list                            **/
      /*-------------------------------------------------------------*/

      if ((fp = fopen (pds_temp_data_fname, "r")) == NULL)
      {
         err_append_message (ERROR1, "Unable to open scratch file.");
      }
      /*-------------------------------------------------------------*/
      /** ELSE                                                      **/
      /*-------------------------------------------------------------*/

      else
      {
         /*----------------------------------------------------------*/
         /** WHILE there are more lines in the redirect file DO     **/
         /*----------------------------------------------------------*/

         while ((fgets (data_str, PDS_MAXLINE, fp)) != NULL)
         {
            util_clean_up_string (data_str);
            util_compress_char (data_str, ' ');
            if (util_string_is_empty (data_str)) break;

            /*-------------------------------------------------------*/
            /** IF this is Unix and the string does not have "/"    **/
            /**     at the end THEN                                 **/
            /**   remove the file mode characters and add the name  **/
            /**   to the list of files                              **/
            /** ENDIF                                               **/
            /*-------------------------------------------------------*/

#ifdef SUN_UNIX

            temp_str = String_End(data_str);
            if (*temp_str != '/')
            {
              if (*temp_str == '*' || *temp_str == '=' || *temp_str == '@')
                 *temp_str = EOS;
              file_list = util_append_string_list (file_list,
                                                       data_str,
                                                           STRING_TYPE); 
            }
#endif
            /*-------------------------------------------------------*/
            /** IF this is VMS THEN                                 **/
            /**   add the file to the file list                     **/
            /** ENDIF                                               **/
            /*-------------------------------------------------------*/

#ifdef VAX
            file_list = util_append_string_list (file_list,
                                                     data_str, 
                                                         STRING_TYPE);
#endif

         }
         /*----------------------------------------------------------*/
         /** ENDWHILE there are more lines...                       **/
         /*----------------------------------------------------------*/

         fclose (fp);
      }
      /*-------------------------------------------------------------*/
      /** ENDIF the redirect file can't be opened...                **/
      /*-------------------------------------------------------------*/

    }
   /*----------------------------------------------------------------*/
   /** ENDIF there was an error in the system command...            **/
   /*----------------------------------------------------------------*/

}
#endif

   return (file_list);
/** END sys_get_file_list **/
}

/**********************************************************************
 *$Component                                                          *
 *    char *sys_get_path (fname)                                      *
 *$Abstract                                                           *
 *    Extracts and returns the directory path from a file spec.       *
 *$Keywords                                                           *
 *    PATH                                                            *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    fname:                                                          *
 *        The fname variable is a general purpose character           *
 *        string containing a file specification.                     *
 *$Outputs                                                            *
 *    NONE.                                                           *
 *$Returns                                                            *
 *    directory:                                                      *
 *        The directory variable is a string that contains a          *
 *        directory name.                                             *
 *$Detailed_Description                                               *
 *    The sys_get_path routine extracts a directory path from the     *
 *    file name passed in.  If no path is found, this routine         *
 *    returns NULL.                                                   *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore / J.P.L.                                        *
 *$Version_and_Date                                                   *
 *    1.2   April 16, 1992                                            *
 *$Change_History                                                     *
 *    DPB   07-09-91   Original code.                                 *
 *    MDD   10-17-91   Fixed malloc, free, and sys_exit_system calls  *
 *    MDD   01-13-92   Bug fix -was returning empty string rather than*
 *                     NULL when no path was found.                   *
 *    MDD   04-16-92   Added macintosh file handling                  *
 **********************************************************************/

char *sys_get_path (fname)

char *fname;

{
    char *c = {NULL};
    char *path_name = {NULL};
    char save_char;

    if (fname != NULL)
    {
        /*  This loop starts at the end of the file spec and
         *  works backwards until it either finds a directory
         *  path, or runs out of string to check.
         */

        for (c = String_End(fname); 
                ((c >= fname) && (*c != ']') && 
                 (*c != ':') && (*c != '/') && (*c != '\\')); --c) ;

        ++c;
        save_char = *c;
        *c = EOS;
        Malloc_String(path_name, (int) String_Size(fname));
        strcpy (path_name, fname);
        *c = save_char;
        if (strcmp (path_name, "") == 0) Lemme_Go(path_name);
          
    }  /*  End:  "if (fname != NULL) ..."  */

    return (path_name);

}  /* "sys_get_path"  */


/**********************************************************************
 *$Component                                                          *
 *    char *sys_make_temp_fname (directory)                           *
 *$Abstract                                                           *
 *    Creates a temporary file name                                   *
 *$Keywords                                                           *
 *    SYSLIB                                                          *
 *    OS                                                              *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    directory:                                                      *
 *        The directory variable is a string that contains a          *
 *        directory name.                                             *
 *$Outputs                                                            *
 *    None                                                            *
 *$Returns                                                            *
 *    file_name:                                                      *
 *        The variable file_name is a string containing the name of   *
 *        a file.                                                     *
 *$Detailed_Description                                               *
 *    The sys_make_temp_fname routine creates a unique file name.     *
 *    The directory name passed in will be appended to the front of   *
 *    the temporary file name returned. However, the directory may be *
 *    NULL or the empty string.                                       *
 *$External_References                                                *
 *    None                                                            *
 *$Side_Effects                                                       *
 *    This routine allocates memory for the returned value which must *
 *    be freed elsewhere.                                             *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore/JPL                                             *
 *$Version_and_Date                                                   *
 *    2.1   April 16, 1992                                            *
 *$Change_History                                                     *
 *    MDD   03-15-91   Original Code.                                 *
 *    MDD   10-17-91   Fixed malloc, free, and sys_exit_system calls  *
 *    MDD   12-12-91   Rewrote to provide better handling of multiple *
 *                     processes.                                     *
 *    MDD   04-16-92   Added MAC_THINK compiler ifdefs.               *
 **********************************************************************/


char *sys_make_temp_fname (directory)

char *directory;
{

char *file_name;
char base_name [PDS_MAXLINE];

/** BEGIN **/

/*------------------------------------------------------------------*/
/** IF this is SUN Unix THEN                                       **/
/**    use the tempnam function to get a unique file name          **/
/*------------------------------------------------------------------*/

#ifdef SUN_UNIX
   file_name = tempnam (directory, NULL);
#endif

/*------------------------------------------------------------------*/
/** IF this is a VAX THEN                                          **/
/**   use the tmpnam function to get a base name                   **/
/**   tack on an extension so VMS won't, then add the directory    **/
/*------------------------------------------------------------------*/

#ifdef VAX
   tmpnam (base_name);
   strcat (base_name, ".PDS");
   file_name = util_create_file_spec (directory, base_name);
#endif

/*------------------------------------------------------------------*/
/** IF this is a VAX THEN                                          **/
/**   use the tmpnam function to get a file name                   **/
/**   add the folder name                                          **/
/*------------------------------------------------------------------*/

#ifdef MAC_THINK
   tmpnam (base_name);
   file_name = util_create_file_spec (directory, base_name);
#endif

/*------------------------------------------------------------------*/
/** IF this is DOS TURBO C THEN                                    **/
/**   use the time function to get a base name (the tmpnam function**/
/**      doesn't work when more than one program is running)       **/
/**   make a file name using the time for both name and extension  **/
/**   shift name to the left, because times are longer than 8 chars**/
/**      and add the directory                                     **/
/*------------------------------------------------------------------*/

#ifdef MSDOS_TC
   {
      time_t t;

      t = time (NULL);
      sprintf (base_name, "%ld.%ld", t, t);
      base_name[13] = EOS;
      file_name = util_create_file_spec (directory, &base_name [1]);
   }
#endif

/*------------------------------------------------------------------*/
/** ENDIF                                                          **/
/*------------------------------------------------------------------*/

   return (file_name);

/** END sys_make_temp_fname                                       **/
}

