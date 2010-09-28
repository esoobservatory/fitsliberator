/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Component                                                           * 
 *    Library utillib.c                                                *
 * Abstract                                                            *
 *    Low-level PDS utilities                                          *
 * Detailed Description                                                *
 *    The utillib contains the subroutines used by PDS software to     *
 *    perform a variety of low-level string and STRING_LIST            *
 *    manipulation tasks.                                              *
 * Internal References                                                 *
 *    String manipulation routines:                                    *
 *        util_byte_copy                                               *
 *        util_clean_up_string                                         *
 *        util_compress_char                                           *
 *        util_create_file_spec                                        *
 *        util_find_non_blank_char                                     *
 *        util_format_string                                           *
 *        util_is_upper                                                *
 *        util_last_word                                               *
 *        util_locate_string                                           *
 *        util_locate_substring                                        *
 *        util_lower_case                                              *
 *        util_remove_char                                             *
 *        util_replace_char                                            *
 *        util_replace_formatters                                      *
 *        util_save_to_last_occurrence                                 *
 *        util_string_is_empty                                         *
 *        util_strip_lead_and_trail                                    *
 *        util_strip_to_char                                           *
 *        util_upper_case                                              *
 *                                                                     *
 *    STRING_LIST manipulation routines:                               *
 *        util_append_string_list                                      *
 *        util_deallocate_string_list                                  *
 *        util_format_string_list                                      *
 *                                                                     *
 * Authors and Institutions                                            *
 *    David P. Bernath / J.P.L.                                        *
 *    Marti D. DeMore / J.P.L.                                         *
 *    Ann M. Farny   / J.P.L.                                          *
 *    Kristy L. Marski /J.P.L.                                         *
 * Version and Date                                                    *
 *    2.0   October 22, 1991                                           *
 * Change History                                                      *
 *    DPB   06-30-90   Gathered together the various routines that     *
 *                     make up the library.                            *
 *    KLM   07-17-90   Added util_replace_char and util_strip_to_char  *
 *    DPB   08-09-90   Added util_find_non_blank_char, util_locate_    *
 *                     string, and util_locate_substring.              *
 *    DPB   08-22-90   The great de-shalling.                          *
 *    DPB   08-23-90   Eliminated individual library include files.    *
 *    MDD   10-02-90   Added casting for the SUN compiler              *
 *    DPB   10-04-90   Added util_format_string and util_last_word.    *
 *    MDD   03-14-91   Added util_is_upper                             *
 *    MDD   10-22-91   Deleted util_compress_blanks, in favor of       *
 *                     util_compress_char.                             *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "pdsdef.h"
#include "utildef.h"
 
/**********************************************************************
 *$Component                                                          *
 *   char *util_byte_copy (destination_address,                       *
 *                         source_address, num_bytes)                 *
 *$Abstract                                                           *
 *   Copy memory from one location to another.                        *
 *$Keywords                                                           *
 *    UTILLIB                                                         *
 *    MEMORY                                                          *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    source_address:                                                 *
 *        The source_address variable contains the address of the     *
 *        memory location where the bytes are to be copied from.      *
 *    num_bytes:                                                      *
 *        The num_bytes element is the number of bytes to be moved,   *
 *        copied, or changed.                                         *
 *$Outputs                                                            *
 *    destination_address:                                            *
 *        The destination_address variable contains the address of    *
 *        the memory location where the bytes are to be copied to.    *
 *$Returns                                                            *
 *    destination_address:                                            *
 *        The destination_address variable contains the address of    *
 *        the memory location where the bytes are to be copied to.    *
 *$Detailed_Description                                               *
 *    The util_byte_copy subroutine copies a block of memory          *
 *    from one location to another. It second argument is the source  *
 *    address, and its first argument is the destination address.     *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore / J.P.L.                                        *
 *$Version_and_Date                                                   *
 *    1.2   March 16, 1992                                            *
 *$Change_History                                                     *
 *    DPB   07-05-90   Moved to utillib and updated.  This routine    *
 *                     used to be called misc_byte_copy.              *
 *    DPB   08-22-90   The great de-shalling.                         *
 *    MDD   03-16-92   The great int -> long conversion               *
 **********************************************************************/

char *util_byte_copy (destination_address, source_address, num_bytes)
                                      
char *destination_address;
char *source_address;
long num_bytes;

{
    long i;
    char *temp;
 
    temp = destination_address;
    for (i = 0; i < num_bytes; i++)
    {
       *destination_address = *source_address;
       ++destination_address;
       ++source_address;
    }
    return (temp);

}  /*  End:  "util_byte_copy"  */



/**********************************************************************
 *$Component                                                          *
 *    char *util_clean_up_string (string)                             *
 *$Abstract                                                           *
 *    Removes non-printable characters.                               *
 *$Keywords                                                           *
 *    UTILLIB                                                         *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    string:                                                         *
 *        The string variable is a general purpose character string   *
 *        that may contain one or more characters.                    *
 *$Outputs                                                            *
 *    string:                                                         *
 *        The string variable is a general purpose character string   *
 *        that may contain one or more characters.                    *
 *$Returns                                                            *
 *    string:                                                         *
 *        The string variable is a general purpose character string   *
 *        that may contain one or more characters.                    *
 *$Detailed_Description                                               *
 *    The util_clean_up_string subroutine is responsible for          *
 *    stripping non-printable characters off the end of a string,     *
 *    and replacing embedded non-printable characters with blanks.    *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    David P. Bernath /J.P.L.                                        *
 *$Version_and_Date                                                   *
 *    1.2   April 29, 1991                                            *
 *$Change_history                                                     *
 *    DPB   07-05-90   Moved to utillib and updated.  This routine    *
 *                     used to be part of the code listing parser.    *
 *    DPB   08-22-90   The great de-shalling.                         *
 *    DPB   04-29-91   Added code to handle NULL strings.             *
 **********************************************************************/

char *util_clean_up_string (string)

char *string;
                                    
{
    char *c;
     
        /* NOTE:  This loop starts at the end of the string and 
         *        replaces all non-printable characters with EOS.
         */

    if (string != NULL)
    {
        for (c = String_End(string); ((c != string) && (! isprint (*c))); --c)
            *c = EOS;
    
    
            /* NOTE:  This loop starts at the beginning of the string and 
             *        replaces all non-printable characters with a blank.
             */
    
        for (c = string; *c != EOS; ++c)
        {
            if (! isprint (*c))
                *c = ' ';
        }

    }  /*  End:  "if (string != NULL) ..."  */

    return (string);


}  /*  End:  "util_clean_up_string"  */
                                                                      

/**********************************************************************
 *$Component                                                          *
 *    char *util_compress_char (string, compress_char)                *
 *$Abstract                                                           *
 *    Compresses multiple occurances of char to a single char.        *
 *$Keywords                                                           *
 *    UTILLIB                                                         *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    string:                                                         *
 *        The string variable is a general purpose character string   *
 *        that may contain one or more characters.                    *
 *    compress_char:                                                  *
 *        The compress_char variable contains the character in the    *
 *        string to be compressed.                                    *
 *$Outputs                                                            *
 *    string:                                                         *
 *        The string variable is a general purpose character string   *
 *        that may contain one or more characters.                    *
 *$Returns                                                            *
 *    string:                                                         *
 *        The string variable is a general purpose character string   *
 *        that may contain one or more characters.                    *
 *$Detailed_Description                                               *
 *    The util_compress_char subroutine compresses multiple           *
 *    occurances of the input character into a single occurence       *
 *    of that character in a string.                                  *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Kristy L. Marski /J.P.L.                                        *
 *$Version_and_Date                                                   *
 *    1.2   April 29, 1991                                            *
 *$Change_history                                                     *
 *    KLM   07-30-90   Modified from util_compress_blanks.            *
 *    DPB   08-22-90   The great de-shalling.                         *
 *    DPB   04-29-91   Added code to handle NULL strings.             *
 **********************************************************************/

char *util_compress_char (string, compress_char)

char *string;                                  
char compress_char;
         
{
    char *source_char;
    char *dest_char;         
    LOGICAL compress = {FALSE};

/** BEGIN **/

/*--------------------------------------------------------------------------*/
/**                                                                        **/
/** IF the string is not NUL THEN                                          **/
/*--------------------------------------------------------------------------*/

    if ((string != NULL) && (*string != EOS))
    {
/*--------------------------------------------------------------------------*/
/**     LOOP through the string ...                                        **/
/*--------------------------------------------------------------------------*/

        dest_char = string;

        for (source_char = string; *source_char != EOS; ++source_char)
        {
/*--------------------------------------------------------------------------*/
/**         IF the current character is not a the compression char THEN    **/
/**             Turn off the 'compress' flag and copy the character.       **/
/**         ELSE                                                           **/
/*--------------------------------------------------------------------------*/

            if (*source_char != compress_char)
            {   
                compress = FALSE;
                *dest_char = *source_char;
                ++dest_char;
            }
            else
/*--------------------------------------------------------------------------*/
/**             IF this is the first in a group of compression chars THEN  **/
/**                 Turn on the 'compress' flag, and copy a single         **/
/**                 occurance of the compression character.                **/
/*--------------------------------------------------------------------------*/

                if (compress == FALSE)
                {
                    compress = TRUE;
                    *dest_char = compress_char;
                    ++dest_char;
                }
    
/*--------------------------------------------------------------------------*/
/**             ENDIF                                                      **/
/**         ENDIF                                                          **/
/**     ENDLOOP                                                            **/
/*--------------------------------------------------------------------------*/

        }  /*  End:  "for (source_char = string; ..."  */
    
/*--------------------------------------------------------------------------*/
/**     Terminate the copied string.                                       **/
/*--------------------------------------------------------------------------*/

        *dest_char = EOS;

/*--------------------------------------------------------------------------*/
/** ENDIF                                                                  **/
/*--------------------------------------------------------------------------*/

    }  /*  End:  "if ((string != NULL) && ..."  */

/*--------------------------------------------------------------------------*/
/** RETURN a pointer to the string.                                        **/
/*--------------------------------------------------------------------------*/

    return (string);

/*--------------------------------------------------------------------------*/
/** END **/


}  /*  End: "util_compress_char"  */



/**********************************************************************
 *$Component                                                          *
 *    char *util_create_file_spec (directory, file_name)              *
 *$Abstract                                                           *
 *    Create a file spec from a directory and file name               *
 *$Keywords                                                           *
 *    UTILLIB                                                         *
 *    DIRECTORY                                                       *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    directory:                                                      *
 *       The directory variable is a string that contains a           *
 *       directory name.                                              *
 *    file_name:                                                      *
 *       The file_name variable is a string that contains a file      *
 *       name.                                                        *
 *$Outputs                                                            *
 *    NONE                                                            *
 *$Returns                                                            *
 *    file_spec:                                                      *
 *       The file_spec variable is a string that contains a file      *
 *       specification, i.e., a directory name and a file name.       *
 *$Detailed_Description                                               *
 *    The util_create_file_spec routine creates a complete file       *
 *    specification from a directory name and a file name.  If the    *
 *    directory name is empty or NULL, then only the file name is     *
 *    returned. If the file name is NULL or empty then only the       *
 *    directory name is returned. If both are empty or NULL, then     *
 *    NULL is returned.                                               *
 *$Side_Effects                                                       *
 *    This routine allocates memory for its return value which must   *
 *    be deallocated elsewhere.                                       *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Herbert C. Gamble / J.P.L.                                      *
 *$Version_and_Date                                                   *
 *    2.1  October 17, 1991                                           *
 *$Change_History                                                     *
 *    HCG   10-19-90   Original Code                                  *
 *    MDD   03-14-91   Changed name from util_catenate_directory and  *
 *                     added ability to handle VMS and DOS files      *
 *    MDD   10-17-91   Fixed malloc, free, and sys_exit_system calls  *
 **********************************************************************/

char *util_create_file_spec (directory, file_name)

char *directory;
char *file_name;

{
  char *temp_str = NULL;
  /*-------------------------------------------------------------------------*/
  /** BEGIN                                                                 **/
  /**   IF there is no directory but there is a filename THEN               **/
  /*-------------------------------------------------------------------------*/

  if ((directory == NULL || util_string_is_empty (directory)) 
         && file_name != NULL && !util_string_is_empty (file_name))
  { 
     Malloc_String(temp_str, (int) String_Size (file_name));
     strcpy (temp_str, file_name);
  }
  /*-------------------------------------------------------------------------*/
  /**   ELSE IF the file name is not empty THEN                             **/
  /**     copy the directory to temp_str.                                   **/
  /**     append the file name to the directory name                        **/
  /*-------------------------------------------------------------------------*/

  else if (file_name != NULL && !util_string_is_empty (file_name))
  {
     Malloc_String(temp_str, (int) (String_Size(file_name) + String_Size(directory)));
	 strcpy (temp_str, directory);
#ifdef VAX
     if (*(String_End(temp_str)) != ':' &&
            *(String_End(temp_str)) != ']')
        strcat (temp_str, ":");
     strcat (temp_str, file_name);
#endif

#ifdef SUN_UNIX 
     if (*(String_End(temp_str)) != '/')
       strcat (temp_str, "/");
     strcat (temp_str, file_name);
#endif

#ifdef MSDOS_TC

     if (*(String_End(temp_str)) != '\\')
	 strcat (temp_str, "\\");
     strcat (temp_str, file_name);
#endif

#ifdef MAC_THINK
     if (*(String_End(temp_str)) != ':')
        strcat (temp_str, ":");
     strcat (temp_str, file_name);
#endif
  }

  /*-------------------------------------------------------------------------*/
  /**   ELSE IF the directory name exists THEN                              **/
  /**     copy the directory name only to temp_str                          **/
  /*-------------------------------------------------------------------------*/

  else if (directory != NULL && !util_string_is_empty (directory))
  {
   Malloc_String(temp_str, (int) String_Size(directory));
     strcpy (temp_str, directory);
  }
  /*-------------------------------------------------------------------------*/
  /**   ENDIF the directory name exists...                                  **/
  /**   RETURN temp_str.                                                    **/
  /*-------------------------------------------------------------------------*/
  return (temp_str);

} /** END                                                                   **/




/**********************************************************************
 *$Component                                                          *
 *    char *util_find_non_blank_char (string)                         *
 *$Abstract                                                           *
 *    Finds the next non blank character in a string.                 *
 *$Keywords                                                           *
 *    UTILLIB                                                         *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    string:                                                         *
 *        The string variable is a general purpose character string   *
 *        that may contain one or more characters.                    *
 *$Outputs                                                            *
 *    None                                                            *
 *$Returns                                                            *
 *    character_address:                                              *
 *        The character_address variable is the address of a          *
 *        character within a string.                                  *
 *$Detailed_Description                                               *
 *    The util_find_non_blank_char subroutine returns a pointer to    *
 *    the first occurence of a non-blank (SPACE) character.  If no    *
 *    non-blank characters are found, this subroutine returns a       *
 *    pointer to the end-of-string character.                         *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    David P. Bernath / J.P.L.                                       *
 *$Version_and_Date                                                   *
 *    1.2   April 29, 1991                                            *
 *$Change_history                                                     *
 *    DPB   08-09-90   Original Code                                  *
 *    DPB   08-22-90   The great de-shalling.                         *
 *    DPB   04-29-91   Added code to handle NULL strings.             *
 **********************************************************************/

char *util_find_non_blank_char (string)

char *string;

{
    char *c;

    for (c = string; ((c != NULL) && (*c != EOS) && (*c == ' ')); ++c) ;

    return (c);

}  /*  End:  "util_find_non_blank_char"  */




/**********************************************************************
 *$Component                                                          *
 *    char *util_format_string (string, maxlen)                       *
 *$Abstract                                                           *
 *    Formats a string of text.                                       *
 *$Keywords                                                           *
 *    UTILLIB                                                         *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    string:                                                         *
 *        The string variable is a general purpose character string   *
 *        that may contain one or more characters.                    *
 *    maxlen:                                                         *
 *        The maxlen variable contains the maximum number of          *
 *        characters that can be read, or the maximum size of a       *
 *        formatted string of text.                                   *
 *$Outputs                                                            *
 *    None                                                            *
 *$Returns                                                            *
 *    formatted_string:                                               *
 *        The formatted_string variable is a character string that    *
 *        has been formatted into lines separated by a NEWLINE        *
 *        character.                                                  *
 *$Detailed_Description                                               *
 *    The util_format_string subroutine returns a pointer to          *
 *    a formatted string consisting of lines MAXLEN characters long.  *
 *    Words that extend beyond the MAXLEN boundary are wrapped onto   *
 *    the next line.  If a word is too long to be wrapped, it is      *
 *    truncated at the MAXLENth character, and the rest of the word   *
 *    is placed on the next line.                                     *
 *$External_References                                                *
 *    None                                                            *
 *$Error_Handling                                                     *
 *    If the line length is invalid, or if storage for any local      *
 *    strings cannot be allocated, the program will terminate.        *
 *$Side_Effects                                                       *
 *    Memory is allocated for the return string, and must be freed    *
 *    by the calling routine.                                         *
 *$Author_and_Institution                                             *
 *    David P. Bernath / J.P.L.                                       *
 *$Version_and_Date                                                   *
 *    1.2   March 16, 1992                                            *
 *$Change_history                                                     *
 *    DPB   10-04-90   Original Code                                  *
 *    MDD   10-17-91   Fixed malloc, free, sys_exit_system            *
 *    MDD   03-16-92   The great int -> long conversion               *
 **********************************************************************/

char *util_format_string (string, maxlen)

char *string;
long maxlen;
                           
{
    char *temp_str = {NUL};
    char *new_str = {NUL};
    char *old_str = {NUL};
    char *start = {NUL};
    char *end = {NUL};
    long size = {(long) String_Size(string)};
    int len;

/*--------------------------------------------------------------------------*/
/** BEGIN                                                                  **/
/**                                                                        **/
/** IF the length passed in is invalid THEN                                **/
/**     Exit the system                                                    **/
/** ENDIF                                                                  **/
/*--------------------------------------------------------------------------*/

    if (maxlen <= 1)
        Exit_System();

/*--------------------------------------------------------------------------*/
/** Attempt to allocate storage for local strings.                         **/
/*--------------------------------------------------------------------------*/

    old_str = (char *) malloc ((int) size);
    temp_str = (char *) malloc ((int) maxlen + 2);
    new_str = (char *) malloc ((int) (4 + size + (long) (size/maxlen)));
/*--------------------------------------------------------------------------*/
/** Check the allocations                                                  **/
/*--------------------------------------------------------------------------*/

    Check_Malloc(temp_str);
    Check_Malloc(old_str);
    Check_Malloc(new_str);

/*--------------------------------------------------------------------------*/
/** Initialize the local strings and remove any existing NEWLINE           **/
/**     characters from the input string -> CALL UTIL_REMOVE_CHAR          **/
/*--------------------------------------------------------------------------*/

    *new_str = EOS;
    strcpy (old_str, string);
    util_remove_char (old_str, '\n');

/*--------------------------------------------------------------------------*/
/** LOOP through the input string ...                                      **/
/**     Extract each line, append a NEWLINE character onto the end of it,  **/
/**     and concatenate it onto the formatted string                       **/
/**         -> CALL UTIL_LAST_WORD                                         **/
/**         -> CALL UTIL_STRIP_LEAD_AND_TRAIL                              **/
/** ENDLOOP                                                                **/
/*--------------------------------------------------------------------------*/
    for (start = old_str; *start != EOS; start = end)
    {
        end = util_last_word (start, maxlen);   /* Locate end of line.      */
        len = (int) (end - start);                      /* Length of line.          */
	    strncpy (temp_str, start, (int) len);   /* Copy line to temp str.   */
	    temp_str [len] = '\n';                  /* Add newline char.        */
	    temp_str [len + 1] = EOS;               /* Terminate temp str.      */
        strcat (new_str, temp_str);             /* Concat temp onto new str.*/
        util_strip_lead_and_trail (end, ' ');   /* Remove leading blanks.   */

    }  /*  End:  "for (start = old_str; ..."  */
/*--------------------------------------------------------------------------*/
/** Free the local strings.                                                **/
/*--------------------------------------------------------------------------*/

    Lemme_Go(temp_str);
    Lemme_Go(old_str);

/*--------------------------------------------------------------------------*/
/** RETURN a pointer to the formatted string.                              **/
/*--------------------------------------------------------------------------*/

    return (new_str);

/*--------------------------------------------------------------------------*/
/** END                                                                    **/
/*--------------------------------------------------------------------------*/

}  /*  End:  "util_format_string"  */


 
/**********************************************************************
 *$Component                                                          *
 *   LOGICAL util_is_upper (string)                                   *
 *$Abstract                                                           *
 *   Determine if a string is all upper case.                         *
 *$Keywords                                                           *
 *    UTILLIB                                                         *
 *    STRING                                                          *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    string:                                                         *
 *        The string variable is a general purpose character string   *
 *        that may contain one or more characters.                    *
 *$Outputs                                                            *
 *    None                                                            *
 *$Returns                                                            *
 *    success_flag:                                                   *
 *        The success_flag variable is a general purpose variable     *
 *        which indicates success (a value of TRUE) or failure        *
 *        (a value of FALSE).                                         *
 *$Detailed_Description                                               *
 *    The util_is_upper routine returns a value of true if all the    *
 *    alphabetic characters in the input string are upper case.       *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore / J.P.L.                                        *
 *$Version_and_Date                                                   *
 *    1.0   March 15, 1991                                            *
 *$Change_History                                                     *
 *    MDD   03-15-91   Original code                                  *
 **********************************************************************/

LOGICAL util_is_upper (string)

char *string;
{
   LOGICAL upper = TRUE;

   if (string != NULL)
   {
      for (;*string; string++)
      {
          if (isalpha ((int) *string) && (islower ((int) *string)))
             break; 
      }
      if (*string != EOS) upper = FALSE;
   }
   return (upper);
}


/**********************************************************************
 *$Component                                                          *
 *    char *util_last_word (string, maxlen)                           *
 *$Abstract                                                           *
 *    Locates the last word in a substring of a string.               *
 *$Keywords                                                           *
 *    UTILLIB                                                         *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    string:                                                         *
 *        The string variable is a general purpose character string   *
 *        that may contain one or more characters.                    *
 *    maxlen:                                                         *
 *        The maxlen variable contains the maximum number of          *
 *        characters that can be read, or the maximum size of a       *
 *        formatted string of text.                                   *
 *$Outputs                                                            *
 *    None                                                            *
 *$Returns                                                            *
 *    character_address:                                              *
 *        The character_address variable is the address of a          *
 *        character within a string.                                  *
 *$Detailed_Description                                               *
 *    The util_last_word subroutine returns a pointer to the last     *
 *    word in a substring of a string.  The substring consists of     *
 *    the first MAXLEN characters in the string.  If a word           *
 *    extends over the MAXLEN boundary, then the return value is a    *
 *    pointer to the blank that preceeds that word.  If there are no  *
 *    blanks in the line, then the return value is a pointer to the   *
 *    MAXLENth character in the string (e.g., the word is truncated). *
 *    If the MAXLENth character is a blank (e.g., the line happens    *
 *    to end between words), then the return value is a pointer to    *
 *    that character.  If the string is less than MAXLEN characters   *
 *    long, then the return value is a pointer to the end-of-string   *
 *    character.                                                      *
 *$External_References                                                *
 *    None                                                            *
 *$Error_Handling                                                     *
 *    If the line length is invalid, or if the string passed in is    *
 *    NUL, the program will terminate.                                *
 *$Author_and_Institution                                             *
 *    David P. Bernath / J.P.L.                                       *
 *$Version_and_Date                                                   *
 *    1.2   March 16, 1992                                            *
 *$Change_history                                                     *
 *    DPB   10-04-90   Original Code                                  *
 *    MDD   10-17-91   Fixed malloc, free, and sys_exit_system        *
 *    MDD   03-16-92   The great int -> long conversion               *
 *    DWS   12-17-97   Added temp variable to clear up warning msgs   *
 **********************************************************************/

char *util_last_word (string, maxlen)

char *string;
long maxlen;
                           
{
    char *c = {NUL};
    char *end = {NUL};
    long i = {0};
	long temp;
                                                                              
/*--------------------------------------------------------------------------*/
/** BEGIN                                                                  **/
/**                                                                        **/
/** IF the length of the line is invalid, OR the string passed in is NUL THEN**/
/**     Exit the system                                                    **/
/** ENDIF                                                                  **/
/*--------------------------------------------------------------------------*/

    if ((maxlen <= 1) || (string == NUL))
        Exit_System();

/*--------------------------------------------------------------------------*/
/** IF the length of the string is less then the maximum length of the     **/
/**         line THEN                                                      **/
/**     Set the character pointer to point to the end-of-string character. **/
/** ELSE                                                                   **/
/*--------------------------------------------------------------------------*/
    temp = (long) strlen(string);
    if (temp < maxlen)
    {
        end = String_End(string);
        ++end;
    }
    else
    {
/*--------------------------------------------------------------------------*/
/**     Initialize the character pointer to point to the last character    **/
/**         in the line.                                                   **/
/*--------------------------------------------------------------------------*/

	end = &string[maxlen - 1];

/*--------------------------------------------------------------------------*/
/**     Attempt to locate the last blank in the line and set the           **/
/**         character pointer to point to it.                              **/
/*--------------------------------------------------------------------------*/

        for (c = string; i < maxlen; ++c, ++i)
        {
            if (*c == ' ')
                end = c;
    
        }  /*  End:  "for (c = string, ..."  */

/*--------------------------------------------------------------------------*/
/** ENDIF
/*--------------------------------------------------------------------------*/

    }  /*  End:  "if (strlen (string) ... else ..."  */

/*--------------------------------------------------------------------------*/
/** RETURN the character pointer.                                          **/
/*--------------------------------------------------------------------------*/

    return (end);

/*--------------------------------------------------------------------------*/
/** END                                                                    **/
/*--------------------------------------------------------------------------*/

}  /*  End:  "util_last_word"  */




/**********************************************************************
 *$Component                                                          *
 *    char *util_locate_string (string, substring)                    *
 *$Abstract                                                           *
 *    Finds an isolated substring within a string.                    *
 *$Keywords                                                           *
 *    UTILLIB                                                         *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    string:                                                         *
 *        The string variable is a general purpose character string   *
 *        that may contain one or more characters.                    *
 *    substring:                                                      *
 *        The substring variable is a general purpose character       *
 *        string variable that may contain one or more characters.    *
 *        It is used refer to a string within a string.               *
 *$Outputs                                                            *
 *    None                                                            *
 *$Returns                                                            *
 *    character_address:                                              *
 *        The character_address variable is the address of a          *
 *        character within a string.                                  *
 *$Detailed_Description                                               *
 *    The util_locate_string subroutine returns a pointer to          *
 *    the first occurence of substring that is not embedded within    *
 *    a longer string.  For example, if the string is                 *
 *    ``THIS IS A TEST" and the substring is ``IS", this subroutine   *
 *    will return the address of the sixth character in the string    *
 *    (the word ``IS").  A substring is not embedded if it is         *
 *    surrounded by non-alphanumeric characters.                      *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    David P. Bernath / J.P.L.                                       *
 *$Version_and_Date                                                   *
 *    1.1   August 22, 1990                                           *
 *$Change_history                                                     *
 *    DPB   08-09-90   Original Code                                  *
 *    DPB   08-22-90   The great de-shalling.                         *
 **********************************************************************/

char *util_locate_string (string, substring)

char *string;
char *substring;

{
    char *c;
    char char_before;
    char char_after;
    LOGICAL found = {FALSE};


/*--------------------------------------------------------------------------*/
/** BEGIN                                                                  **/
/**                                                                        **/
/** LOOP through the string ...                                            **/
/*--------------------------------------------------------------------------*/

    for (c = string; ((! found) && (c != NUL)); )
    {
/*--------------------------------------------------------------------------*/
/**     Attempt to locate the substring within the string                  **/
/**             -> CALL UTIL_LOCATE_SUBSTRING                              **/
/*--------------------------------------------------------------------------*/

        c = util_locate_substring (c, substring);           

/*--------------------------------------------------------------------------*/
/**     IF the substring was located THEN                                  **/
/*--------------------------------------------------------------------------*/

        if (c != NUL)
        {
/*--------------------------------------------------------------------------*/
/**         Save the character that preceeds the first character           **/
/**                 and the character that follows the last                **/
/**                 character in the substring.                            **/
/*--------------------------------------------------------------------------*/

            if (*c == EOS)
            {
                char_before = EOS;
                char_after = EOS;
            }
            else
            {
                if (c == string)
                    char_before = NUL;
                else
                    char_before = *(c - 1);

                char_after = *(c + strlen (substring));

            }  /*  End:  "if (*substring == EOS) ... else ..."  */
    
/*--------------------------------------------------------------------------*/
/**         Set the found flag to TRUE only if the character               **/
/**                 preceeding the substring and the character             **/
/**                 following the substring are both                       **/
/**                 non-alphanumeric, otherwise set the flag to FALSE.     **/
/*--------------------------------------------------------------------------*/

            found = ((! isalnum (char_before)) && (! isalnum (char_after)));
    
/*--------------------------------------------------------------------------*/
/**         IF the substring was embedded THEN                             **/
/**              Increment the address into the string and continue.       **/
/**         ENDIF                                                          **/
/*--------------------------------------------------------------------------*/

            if (! found) 
                ++c;

/*--------------------------------------------------------------------------*/
/**     ENDIF                                                              **/
/*--------------------------------------------------------------------------*/

        }  /*  End:  "if (c != NUL) ..."  */

/*--------------------------------------------------------------------------*/
/** ENDLOOP                                                                **/
/*--------------------------------------------------------------------------*/

    }  /*  End:  "for (c = string; ..."  */

/*--------------------------------------------------------------------------*/
/** RETURN the address of the first character of the substring.            **/
/*--------------------------------------------------------------------------*/

    return (c);

/*--------------------------------------------------------------------------*/
/** END                                                                    **/
/*--------------------------------------------------------------------------*/

}  /*  End:  "util_locate_string"  */




/**********************************************************************
 *$Component                                                          *
 *    char *util_locate_substring (string, substring)                 *
 *$Abstract                                                           *
 *    Finds a substring within a string.                              *
 *$Keywords                                                           *
 *    UTILLIB                                                         *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    string:                                                         *
 *        The string variable is a general purpose character string   *
 *        that may contain one or more characters.                    *
 *    substring:                                                      *
 *        The substring variable is a general purpose character       *
 *        string variable that may contain one or more characters.    *
 *        It will be used refer to a string within a string.          *
 *$Outputs                                                            *
 *    None                                                            *
 *$Returns                                                            *
 *    substring:                                                      *
 *        The substring variable is a general purpose character       *
 *        string variable that may contain one or more characters.    *
 *        It will be used refer to a string within a string.          *
 *$Detailed_Description                                               *
 *    The util_locate_substring subroutine returns a pointer to       *
 *    the first occurence of substring within string, or NULL if      *
 *    substring is not found within string.  Neither of the two       *
 *    inputs is altered.                                              *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore / J.P.L.                                        *
 *$Version_and_Date                                                   *
 *    1.2   October, 1990                                             *
 *$Change_history                                                     *
 *    MDD   08-07-90   Original Code                                  *
 *    DPB   08-22-90   The great de-shalling.                         *
 *    MDD   10-02-90   Added casting for SUN compiler.                *
 **********************************************************************/

char *util_locate_substring (string, substring)

char *string;
char *substring;

{

   LOGICAL found = FALSE;

   if (substring && string)
   {
      while (!found && (string = (char *) strchr (string, (int) substring [0])) 
             != NULL)
      {
	 found = !(strncmp (string, substring, (strlen (substring))));
	 if (!found) ++string;
      }
      return (string);
   }
   else
   {
      return (NULL);
   }

}  /*  End:  "util_locate_substring"  */




/**********************************************************************  
 *$Component                                                          * 
 *    char *util_lower_case (string)                                  *
 *$Abstract                                                           * 
 *    Convert a string to lowercase (in situ).                        *
 *$Keywords                                                           * 
 *    UTILLIB                                                         *
 *    CONVERT                                                         * 
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             * 
 *    string:                                                         *
 *        The string variable is a general purpose character string   *
 *        that may contain one or more characters.                    *
 *$Outputs                                                            * 
 *    string:                                                         *
 *        The string variable is a general purpose character string   *
 *        that may contain one or more characters.                    *
 *$Returns                                                            * 
 *    string:                                                         *
 *        The string variable is a general purpose character string   *
 *        that may contain one or more characters.                    *
 *$Detailed_Description                                               * 
 *    The util_lower_case subroutine converts a string to lower case  *
 *    in its original position.                                       *
 *$External_References                                                * 
 *    NONE                                                            * 
 *$Author_and_Institution                                             * 
 *    David P. Bernath/J.P.L.                                         *
 *$Version_and_Date                                                   * 
 *    1.2   April 29, 1991                                            *
 *$Change_History                                                     * 
 *    DPB   07-05-90   Moved to utillib and updated.  This routine    *
 *                     used to be part of the code listing parser.    *
 *    DPB   08-22-90   The great de-shalling.                         *
 *    DPB   04-29-91   Added code to handle NULL strings.             *
 **********************************************************************/ 
 
char *util_lower_case (string)	

char *string;

{
    char *c;

    for (c = string; ((c != NULL) && (*c != EOS)); ++c)
    {
        if (isupper (*c))
            *c = tolower (*c);
    }

    return (string);

}  /*  End:  "util_lower_case"  */




/**********************************************************************
 *$Component                                                          *
 *   char *util_remove_char (string, remove_char)                     *
 *$Abstract                                                           *
 *   Remove all occurrences of a character from a string              *
 *$Keywords                                                           *
 *    UTILLIB                                                         *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    remove_char:                                                    *
 *        The remove_char variable contains a character to be removed *
 *        from a string.                                              *
 *    string:                                                         *
 *        The string variable is a general purpose character string   *
 *        that may contain one or more characters.                    *
 *$Outputs                                                            *
 *    string:                                                         *
 *        The string variable is a general purpose character string   *
 *        that may contain one or more characters.                    *
 *$Returns                                                            *
 *    string:                                                         *
 *        The string variable is a general purpose character string   *
 *        that may contain one or more characters.                    *
 *$Detailed_Description                                               *
 *    The util_remove_char subroutine removes all occurrences of      *
 *    a character from a string.                                      *
 *$External_References                                                *
 *    None                                                            *
 *$Limitations                                                        *
 *    None.                                                           *
 *$Error_Handling                                                     *
 *    None.                                                           *
 *$Side_Effects                                                       *
 *    None.                                                           *
 *$Author_and_Institution                                             *
 *    David P. Bernath / J.P.L.                                       *
 *$Version_and_Date                                                   *
 *    1.2   April 29, 1991                                            *
 *$Change_History                                                     *
 *    DPB   07-05-90   Moved to utillib and updated.  This routine    *
 *                     used to be called misc_strrem.                 *
 *    DPB   08-22-90   The great de-shalling.                         *
 *    DPB   04-29-91   Added code to handle NULL strings.             *
 **********************************************************************/

char *util_remove_char (string, remove_char)

char *string;
char remove_char;

{
    char *source_char;
    char *dest_char;

    dest_char = string;

    if (string != NULL)
    {
        for (source_char = string; *source_char != EOS; ++source_char)
        {
            if (*source_char != remove_char)
            {
                *dest_char = *source_char;
                ++dest_char;
            }
                
        }  /*  End:  "for (source_char = string; ..."  */
    
        *dest_char = EOS;

    }  /*  End:  "if (string != NULL) ..."  */

    return (string);

}  /*  End:  "util_remove_char"  */




/**********************************************************************
 *$Component                                                          *
 *   char *util_replace_char (string, old_char, new_char)             *
 *$Abstract                                                           *
 *   Replace all occurrences of a character from a string             *
 *$Keywords                                                           *
 *    UTILLIB                                                         *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    string:                                                         *
 *        The string variable is a general purpose character string   *
 *        that may contain one or more characters.                    *
 *    old_char:                                                       *
 *        The old_char variable contains a character to be replaced   *
 *        within a string.                                            *
 *    new_char:                                                       *
 *        The new_char variable contains a character that replaces    *
 *        another character within a string.                          *
 *$Outputs                                                            *
 *    string:                                                         *
 *        The string variable is a general purpose character string   *
 *        that may contain one or more characters.                    *
 *$Returns                                                            *
 *    string:                                                         *
 *        The string variable is a general purpose character string   *
 *        that may contain one or more characters.                    *
 *$Detailed_Description                                               *
 *    The util_replace_char subroutine replaces all occurrences       *
 *    of a one character with another character within a string.      *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Kristy L. Marski / J.P.L.                                       *
 *$Version_and_Date                                                   *
 *    1.2   April 29, 1991                                            *
 *$Change_History                                                     *
 *    KLM   07-16-90   Original generation.                           *
 *    DPB   08-22-90   The great de-shalling.                         *
 *    DPB   04-29-91   Added code to handle NULL strings.             *
 **********************************************************************/

char *util_replace_char (string, old_char, new_char)

char *string;
char old_char;
char new_char;

{
    char *source_char;
  
    if (string != NULL)
    {
        for (source_char = string; *source_char != EOS; ++source_char)
        {
            if (*source_char == old_char)
            {
                *source_char = new_char;
            }
                
        }  /*  End:  "for (source_char = string; ..."  */

    }  /*  End:  "if (string != NULL) ..."  */

    return (string);

}  /*  End:  "util_replace_char"  */
   


/**********************************************************************
 *$Component                                                          *
 *    char *util_save_to_last_occurrence (string, search_char)        *
 *$Abstract                                                           *
 *    Saves a string to the last occurrence of a given char.          *
 *$Keywords                                                           *
 *    UTILLIB                                                         *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    string:                                                         *
 *        The string variable is a general purpose character string   *
 *        that may contain one or more characters.                    *
 *    search_char:                                                    *
 *        The search_char variable is the character to be searched    *
 *        for within a string.                                        *
 *$Outputs                                                            *
 *    None                                                            *
 *$Returns                                                            *
 *    save_string:                                                    *
 *        The save_string variable is a character string which        *
 *        contains either a sub-string of the original string, or a   *      
 *        NULL value if the search character is not found.            *
 *$Detailed_Description                                               *
 *    The util_save_to_last_occurrence subroutine saves all of the    *
 *    characters up to and including the last occurrence of the       *
 *    search character passed in. If the search character is not      *
 *    found, NULL is returned.                                        *
 *$Side_Effects                                                       *
 *    This routine allocates memory for its return value which must   *
 *    be deallocated elsewhere.                                       *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Kristy L. Marski /JPL                                           *
 *$Version_and_Date                                                   *
 *    1.3   March 16, 1992                                            *
 *$Change_history                                                     *
 *    KLM   04-01-91   Original generation.                           *
 *    DPB   04-29-91   Added code to handle NULL strings.             *
 *    MDD   10-17-91   Fixed malloc, free, and sys_exit_system calls  *
 *    MDD   03-16-92   The great int -> long conversion               * 
 **********************************************************************/

char *util_save_to_last_occurrence (string, search_char)
 
char *string;
char search_char;
         
{
    char *save_string = NULL;
    char *c;                             
    long len = 0;

    if (string != NULL)
    {
        c = (char *) strrchr (string, search_char);
        
        if (c != NULL)
        {
           
           len = c - string + 1;
	   Malloc_String(save_string, (int) len + 1);
	   strncpy (save_string, string, (int) len);
	   *(save_string + len) = EOS;
    
        }

    }  /*  End:  "if (string != NULL) ..."  */

    return (save_string);


}  /*  End: "util_save_to_last_occurrence"  */


/**********************************************************************
 *$Component                                                          *
 *    char *util_replace_formatters (string)                          *
 *$Abstract                                                           *
 *    Strip and replace ODL formatters from a string                  *
 *$Keywords                                                           *
 *    UTILLIB                                                         *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    string:                                                         *
 *        The string variable is a general purpose character string   *
 *        that may contain one or more characters.                    *
 *$Outputs                                                            *
 *    string:                                                         *
 *        The string variable is a general purpose character string   *
 *        that may contain one or more characters.                    *
 *$Returns                                                            *
 *    string:                                                         *
 *        The string variable is a general purpose character string   *
 *        that may contain one or more characters.                    *
 *$Detailed_Description                                               *
 *    The util_replace_formatters routine replaces ODL formatters     *
 *    in a string.  ODL formatters, "\n" and "\t" each occupy two     *
 *    characters in the string, and so are not interpreted by C.      *
 *    This routine replaces them with the actual C escape codes for   *
 *    these characters and deletes the extra characters.              *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore/JPL                                             *
 *$Version_and_Date                                                   *
 *    1.0   January 24, 1992                                          *
 *$Change_history                                                     *
 *    MDD   01-24-92   Original code.                                 *
 **********************************************************************/


char *util_replace_formatters (string)

char *string;

{
   char *temp;

   for (temp = string; temp && *temp; temp++)
   {
      if (*temp == '\\')
      {
         temp++;
         if (*temp == 't')
         {
	    util_byte_copy (temp, temp + 1, (long) strlen (temp + 1) + 1);
	    *--temp  = '\t';
	 }
	 else if (*temp == 'n')
	 {
	    util_byte_copy (temp, temp + 1, (long) strlen (temp + 1) + 1);
	    *--temp = '\n';
	 }
      }
   }
   return (string);
}

/**********************************************************************
 *$Component                                                          *
 *    char *util_strip_lead_and_trail (string, remove_char)           *
 *$Abstract                                                           *
 *    Strip off the leading and trailing characters from a string.    *
 *$Keywords                                                           *
 *    UTILLIB                                                         *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    string:                                                         *
 *        The string variable is a general purpose character string   *
 *        that may contain one or more characters.                    *
 *    remove_char:                                                    *
 *        The remove_char variable contains a character to be removed *
 *        from a string.                                              *
 *$Outputs                                                            *
 *    string:                                                         *
 *        The string variable is a general purpose character string   *
 *        that may contain one or more characters.                    *
 *$Returns                                                            *
 *    string:                                                         *
 *        The string variable is a general purpose character string   *
 *        that may contain one or more characters.                    *
 *$Detailed_Description                                               *
 *    The util_strip_lead_and_trail subroutine strips all of the      *
 *    occurences of a particular character from the front and rear    *
 *    of a string.                                                    *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    David P. Bernath /J.P.L.                                        *
 *$Version_and_Date                                                   *
 *    1.2   April 29, 1991                                            *
 *$Change_history                                                     *
 *    DPB   07-05-90   Moved to utillib and updated.  This routine    *
 *                     used to be called misc_strip_lead_and_trail.   *
 *    DPB   08-22-90   The great de-shalling.                         *
 *    DPB   04-29-91   Added code to handle NULL strings.             *
 **********************************************************************/

char *util_strip_lead_and_trail (string, remove_char)

char *string;
char remove_char;
         
{
    char *c;
    if (string != NULL)
    {

        for (c = string; ((*c != EOS) && (*c == remove_char)); ++c) ;
		if(c != string)	strcpy (string, c);

/*        strcpy (string, c); */
        for (c=String_End(string); ((c!=string) && (*c == remove_char)); --c) ;
    
        if(String_End(string) != c) *(++c) = EOS;

    }  /*  End:  "if (string != NULL) ..."  */
    return (string);


}  /*  End: "util_strip_lead_and_trail"  */



/**********************************************************************
 *$Component                                                          *
 *    char *util_strip_to_char (string, remove_char)                  *
 *$Abstract                                                           *
 *    Strip off the leading characters from a string.                 *
 *$Keywords                                                           *
 *    UTILLIB                                                         *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    string:                                                         *
 *        The string variable is a general purpose character string   *
 *        that may contain one or more characters.                    *
 *    remove_char:                                                    *
 *        The remove_char variable contains a character to be removed *
 *        from a string.                                              *
 *$Outputs                                                            *
 *    string:                                                         *
 *        The string variable is a general purpose character string   *
 *        that may contain one or more characters.                    *
 *$Returns                                                            *
 *    string:                                                         *
 *        The string variable is a general purpose character string   *
 *        that may contain one or more characters.                    *
 *$Detailed_Description                                               *
 *    The util_strip_to_char subroutine strips all of the characters  *
 *    in a string that occur before a particular character is reached.*
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Kristy L. Marski /J.P.L.                                        *
 *$Version_and_Date                                                   *
 *    1.2   April 29, 1991                                            *
 *$Change_history                                                     *
 *    KLM   07-17-90   Original generation.                           *
 *    DPB   08-22-90   The great de-shalling.                         *
 *    DPB   04-29-91   Added code to handle NULL strings.             *
 **********************************************************************/

char *util_strip_to_char (string, remove_char)

char *string;
char remove_char;
         
{
    char *c;

    if (string != NULL)
    {
        for (c = string; ((*c != EOS) && (*c != remove_char)); ++c) ;
        
        if (*c == remove_char)
    
            strcpy (string, c);

    }  /*  End:  "if (string != NULL) ..."  */

    return (string);


}  /*  End: "util_strip_to_char"  */



/**********************************************************************
 *$Component                                                          *
 *   char *util_upper_case (string)                                   *
 *$Abstract                                                           *
 *    Translate a string to upper case (in situ).                     *
 *$Keywords                                                           *
 *    UTILLIB                                                         *
 *    CONVERT                                                         *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    string:                                                         *
 *        The string variable is a general purpose character string   *
 *        that may contain one or more characters.                    *
 *$Outputs                                                            *
 *    string:                                                         *
 *        The string variable is a general purpose character string   *
 *        that may contain one or more characters.                    *
 *$Returns                                                            *
 *    string:                                                         *
 *        The string variable is a general purpose character string   *
 *        that may contain one or more characters.                    *
 *$Detailed_Description                                               *
 *    The util_upper_case subroutine converts a string to upper       *
 *    case in its original position.                                  *
 *$External_References                                                *
 *    None.                                                           *
 *$Author_and_Institution                                             *
 *    Ann M. Farny / J.P.L.                                           *
 *$Version_and_Date                                                   *
 *    1.2   April 29, 1991                                            *
 *$Change_History                                                     *
 *    DPB   07-05-90   Moved to utillib and updated.  This routine    *
 *                     used to be called misc_strup.                  *
 *    DPB   08-22-90   The great de-shalling.                         *
 *    DPB   04-29-91   Added code to handle NULL strings.             *
 **********************************************************************/

char *util_upper_case (string)	

char *string;

{
    char *c;

    if (string != NULL)
    {
        for (c = string; *c != EOS; ++c)
        {
            if (islower (*c))
                *c = toupper (*c);
        }

    }  /*  End:  "if (string != NULL) ..."  */

    return (string);

}  /*  End:  "util_upper_case"  */




/**********************************************************************
 *$Component                                                          *
 *   LOGICAL util_string_is_empty (string)                            *
 *$Abstract                                                           *
 *    Checks a string to see if it is empty.                          *
 *$Keywords                                                           *
 *    UTILLIB                                                         *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    string:                                                         *
 *        The string variable is a general purpose character string   *
 *        that may contain one or more characters.                    *
 *$Outputs                                                            *
 *    None                                                            *
 *$Returns                                                            *
 *    success_flag:                                                   *
 *        The success_flag variable is a general purpose variable     *
 *        which indicates success (a value of TRUE) or failure        *
 *        (a value of FALSE).                                         *
 *$Detailed_Description                                               *
 *    The util_string_is_empty subroutine returns TRUE if a string    *
 *    contains either blanks or the NUL character, or FALSE if the    *
 *    string contains anything else.                                  *
 *$External_References                                                *
 *    None.                                                           *
 *$Author_and_Institution                                             *
 *    David P. Bernath / J.P.L.                                       *
 *$Version_and_Date                                                   *
 *    1.2   April 29, 1991                                            *
 *$Change_History                                                     *
 *    DPB   07-05-90   Moved to utillib and updated.  This routine    *
 *                     used to be called misc_line_is_empty.          *
 *    DPB   08-22-90   The great de-shalling.                         *
 *    DPB   04-29-91   Added code to handle NULL strings.             *
 **********************************************************************/

LOGICAL util_string_is_empty (string)

char *string;

{
    char *c;
    LOGICAL empty = {TRUE};

    if (string != NULL)
    {
        for (c = string; ((*c != EOS) && (empty)); ++c)
            empty = (*c == ' ');

    }  /*  End:  "if (string != NULL) ..."  */

    return (empty);

}  /*  End: "util_string_is_empty"  */




/**********************************************************************
 *$Component                                                          *
 *   STRING_LIST *util_append_string_list (text_ptr,text,string_type) *
 *$Abstract                                                           *
 *    Append a string or list of strings to an existing list.         *
 *$Keywords                                                           *
 *    UTILLIB                                                         *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    text_ptr:                                                       *
 *        The text_ptr variable is a pointer to a linked list of      *
 *        STRING_LIST structures.  These structures contain ordered   *
 *        lines of text.                                              *
 *    text:                                                           *
 *        The text variable is a general purpose character string     *
 *        that may contain one or more characters.                    *
 *    string_type:                                                    *
 *        The string_type variable is a flag which indicates whether  *
 *        the input should be treated as a character string or a      *
 *        linked list of STRING_LIST structures. Possible values are  *
 *        STRING_TYPE and LIST_TYPE.                                  *
 *$Outputs                                                            *
 *    None                                                            *
 *$Returns                                                            *
 *    text_ptr:                                                       *
 *        The text_ptr variable is a pointer to a linked list of      *
 *        STRING_LIST structures.  These structures contain ordered   *
 *        lines of text.                                              *
 *$Detailed_Description                                               *
 *    The util_append_string_list subroutine appends a string or a    *
 *    linked list of STRING_LIST structures onto the end of a list of *
 *    these structures.                                               *
 *$External_References                                                *
 *    None.                                                           *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore / J.P.L.                                        *
 *$Version_and_Date                                                   *
 *    1.3   June 24, 1993                                             *
 *$Change_History                                                     *
 *    DPB   07-05-90   Moved to utillib and updated.  This routine    *
 *                     used to be called misc_append_string.          *
 *    DPB   08-22-90   The great de-shalling.                         *
 *    MDD   10-17-91   Fixed malloc, free, and sys_exit_system calls  *
 *    MDD   06-24-93   Fixed bug when text is NULL.                   *
 **********************************************************************/

STRING_LIST *util_append_string_list (text_ptr, text, string_type)

STRING_LIST *text_ptr;
char *text;
int string_type;
{
   STRING_LIST *temp = {NUL};
   STRING_LIST *new_ptr = {NUL};

/** BEGIN **/

   if (text == NULL) return (text_ptr);

   /** find the last node in the list **/

   temp = text_ptr;
   while (text_ptr && text_ptr -> next)
      text_ptr = text_ptr -> next;

   /** IF the new text is a list **/
   /**    set the new node pointer to the head of the list **/

   if (string_type == LIST_TYPE)
   {
      new_ptr = (STRING_LIST *) text;
   }

   /** ELSE **/
   /**    allocate a new node for the text **/

   else
   {
      new_ptr = (STRING_LIST *) malloc (sizeof (STRING_LIST));
      Check_Malloc(new_ptr);
      Malloc_String(new_ptr -> text, (int) String_Size(text));
      strcpy (new_ptr -> text, text);
      new_ptr -> next = NUL;

   }
   /** ENDIF **/

   /** set the new node's previous pointer to the end of the old list **/

   new_ptr -> prev = text_ptr;

   /** IF the old list is empty **/
   /**    start a new list with the new node **/

   if (text_ptr == NUL)
   {
      text_ptr = new_ptr;
      temp = text_ptr;
   }

   /** ELSE **/
   /**    attach the new node to the end of the old list **/

   else
   {
      text_ptr -> next = new_ptr;

   }
   /** ENDIF **/

   /** RETURN the new pointer to the front of the list **/

   return (temp);

/** END **/

}  /*  End:  "util_append_string_list"  */




/**********************************************************************
 *$Component                                                          *
 *   STRING_LIST *util_deallocate_string_list (text_ptr)              *
 *$Abstract                                                           *
 *    Delete a list of formatted text.                                *
 *$Keywords                                                           *
 *    UTILLIB                                                         *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    text_ptr:                                                       *
 *        The text_ptr variable is a pointer to a linked list of      *
 *        STRING_LIST structures.  These structures contain ordered   *
 *        lines of text.                                              *
 *$Outputs                                                            *
 *    None                                                            *
 *$Returns                                                            *
 *    text_ptr:                                                       *
 *        The text_ptr variable is a pointer to a linked list of      *
 *        STRING_LIST structures.  These structures contain ordered   *
 *        lines of text.                                              *
 *$Detailed_Description                                               *
 *    The util_deallocate_string_list subroutine deallocates a linked *
 *    list of STRING_LIST structures returns a NUL value to the       *
 *    calling routine.  This NUL value can be used to initialize      *
 *    the list pointer that is passed in.                             *
 *$External_References                                                *
 *    None.                                                           *
 *$Side_Effects                                                       *
 *    The util_deallocate_string_list subroutine does not actually    *
 *    change the text_ptr input. However, it does deallocate the      *
 *    list stored at this pointer, thus effectively destroying its    *
 *    contents.                                                       *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore / J.P.L.                                        *
 *$Version_and_Date                                                   *
 *    1.2   October 17, 1991                                          *
 *$Change_History                                                     *
 *    DPB   07-05-90   Moved to utillib and updated.  This routine    *
 *                     used to be called misc_delete_text.            *
 *    DPB   08-22-90   The great de-shalling.                         *
 *    MDD   10-17-91   Fixed free, malloc, and sys_exit_system calls  *
 **********************************************************************/

STRING_LIST *util_deallocate_string_list (text_ptr)

STRING_LIST *text_ptr;
     
{
    STRING_LIST *next_ptr;

    while (text_ptr)
    {
        next_ptr = text_ptr -> next; 
		if(next_ptr == NULL) break;
        Lemme_Go(text_ptr -> text);
        Lemme_Go(text_ptr);
        text_ptr = next_ptr;
    }

    return (NUL);

}  /*  End:  "util_deallocate_string_list"  */




/**********************************************************************
 *$Component                                                          *
 *   STRING_LIST *util_format_string_list (text_ptr, text, maxlen)    *
 *$Abstract                                                           *
 *    Create a list of formatted text.                                *
 *$Keywords                                                           *
 *    STRING                                                          *
 *    CREATE                                                          *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    text_ptr:                                                       *
 *        The text_ptr variable is a pointer to a linked list of      *
 *        STRING_LIST structures.  These structures contain ordered   *
 *        lines of text.                                              *
 *    text:                                                           *
 *        The text variable is a general purpose character string     *
 *        that may contain one or more characters.                    *
 *    maxlen:                                                         *
 *        The maxlen variable contains the maximum number of          *
 *        characters that can be read, or the maximum size of a       *
 *        formatted string of text.                                   *
 *$Outputs                                                            *
 *    None                                                            *
 *$Returns                                                            *
 *    text_ptr:                                                       *
 *        The text_ptr variable is a pointer to a linked list of      *
 *        STRING_LIST structures.  These structures contain ordered   *
 *        lines of text.                                              *
 *$Detailed_Description                                               *
 *    The util_format_string_list subroutine formats a long,          *
 *    continuous string of text into a linked list of STRING_LIST     *
 *    structures.  The long string of text will be broken into lines, *
 *    each having less than a specified maximum length.               *
 *$External_References                                                *
 *    None.                                                           *
 *$Side_Effects                                                       *
 *    The util_format_string_list subroutine allocates memory         *
 *    dynamically for the storage of the formatted text. The          *
 *    misc_delete_string_list subroutine should be called to delete   *
 *    the text after its use.                                         *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore / J.P.L.                                        *
 *$Version_and_Date                                                   *
 *    1.2   March 16, 1992                                            *
 *$Change_History                                                     *
 *    DPB   07-05-90   Moved to utillib and updated.  This routine    *
 *                     used to be called misc_format_text.            *
 *    DPB   08-22-90   The great de-shalling.                         *
 *    MDD   03-16-92   The great int -> long conversion               *
 *    DWS   12-17-97   Added temp variable to clear up warning msgs   *
 **********************************************************************/

STRING_LIST *util_format_string_list (text_ptr, text, maxlen)

STRING_LIST *text_ptr;
char *text;
long maxlen;

{
   char *c;
   char *last_char;
   char *save_char;
   long temp;
 
/** BEGIN **/

   /** Locate the first non-blank character in the TEXT string. **/

   for ( ; ((*text != EOS) && (*text == ' ')); ++text) ;

   /** IF more characters exist in TEXT string, AND the break point is **/
   /**     greater than zero **/

   if ((*text != EOS) && (maxlen > 0))
   {
      /**    IF length of the TEXT string is greater than MAXLEN **/
      /**       Search backwards in TEXT from location MAXLEN to find blank. **/
      /**       Truncate at MAXLEN if no blanks found. **/
      temp = (long) strlen (text);
      if ( temp > maxlen)
      {
	 last_char = text + maxlen - sizeof(char);

         for (c = last_char; ((c != text) && (*c != ' ')); --c) ;

         if (c <= text)
             c = last_char;
      }
                                 
      /**    ELSE **/
      /**       use the rest of TEXT string for this node **/

      else
      {
          c = String_End(text) + sizeof(char);
      }
      /**    ENDIF **/

      /**    Terminate the string at the break point and append it onto **/
      /**        the STRING_LIST -> CALL UTIL_APPEND_STRING_LIST **/

      *save_char = *c;
      *c = EOS;
      text_ptr = util_append_string_list (text_ptr, text, STRING_TYPE);

      /**    Restore the character that was overwritten at the **/
      /**        break point. **/

      *c = *save_char;

      /**    format the remaining text -> CALL UTIL_FORMAT_STRING_LIST  **/

      util_format_string_list (text_ptr, c, maxlen);

      /**    RETURN pointer to beginning of list **/

      return (text_ptr);

   }

   /** ELSE **/
   /**    there is no more text, so RETURN null **/

   else
   {
      return (NUL);
   }
   /** ENDIF **/

/** END **/

}  /*  End:  "util_format_string_list"  */


