/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Component                                                           * 
 *    Library labutil.c                                                *
 * Abstract                                                            *
 *    Label utility routines.                                          *
 * Detailed_Description                                                *
 *    The labutil is a library of low level subroutines used by PDS    *
 *    software to perform a variety of operations on PDS labels.       *
 * Internal_References                                                 *
 *    lu_append_object                                                 *
 *    lu_append_parameter                                              *
 *    lu_append_value                                                  *
 *    lu_fetch_all_values                                              * 
 *    lu_fetch_value                                                   *
 *    lu_find_object_level                                             *
 *    lu_format_date                                                   *
 *    lu_format_date_time                                              *
 *    lu_format_integer                                                *
 *    lu_format_real                                                   *
 *    lu_format_string                                                 *
 *    lu_format_symbol                                                 *
 *    lu_format_time                                                   *
 *    lu_format_null_data                                              *
 *    lu_format_units                                                  *
 *    lu_locate_sfdus                                                  *
 *    lu_new_value                                                     *
 *    lu_parse_string                                                  *
 *    lu_paste_parm_at_pos                                             *
 *    lu_write_zi1_label                                               *
 *    lu_write_zi3_label                                               *
 *    lu_write_zk3_label                                               *
 * Authors_and_Institutions                                            *
 *    Herbert C. Gamble / J.P.L.                                       *
 *    David P. Bernath / J.P.L.                                        *
 *    Marti D. Demore / J.P.L.                                         *
 * Version and Date                                                    *
 *    1.4   June 3, 1992                                               *
 * Change History                                                      *
 *    KLM   06-11-91   Broke up the label.c file into higher level     *
 *                     label routines and labutil.c, the lower level   *
 *                     label utility routines.                         *
 *    KLM   06-17-91   Renamed all of the label utility routines to    *
 *                     lu_...                                          *
 *    DPB   12-04-91   Added lu_keyword_value.                         *
 *    MDD   06-03-92   Added lu_locate_sfdus.                          *
 *    DWS   02-01-00   fixed malloc in lu_format_units                 *
 *    DWS    5-25-01   Added code in lu_fetch_all_values to remove     *
 *					   beginning /n from keyword value befor it is     *
 *					   checked to determine if value is a standard     *
 *					   value.                                          *
 *	  MDC	11-22-02   Removed a message from the lu_locate_sfdus	   *
 *					   routine that printed out that no SFDUs were     *
 *					   found in a label since PDS no longer	supports   *
 *					   SFDUs in their labels. Also, users have         *
 *					   requested that this be removed to avoid		   *
 *					   confusion. In other words, ignore it if there   *
 *					   is no SFDUs in label. However, if there is one  *
 *					   then make sure that it is correct.			   *
 *	  MDC	03-04-03   Added extra checks in the lu_locate_sfdus	   *
 *					   routine to also check the pds_warning flag      *
 *					   before appending a warning message.			   *
 *    MDC   12-22-03   Modified the lu_fetch_all_values routine. Also, *
 *                     added a new routine called lu_get_units. See    *
 *                     both functions for details.                     *
 **********************************************************************/

#include "pdsdef.h"
#include "labutil.h"
#include "errordef.h"
#include "fiodef.h"
#include "utildef.h"
#include "odlinter.h"

extern AGGREGATE ODLroot_node;
extern LOGICAL pds_finish_label;
extern LOGICAL pds_verbose;
extern LOGICAL pds_warning;  /* 03-04-03 MDC */

/*--------------------------------------------------------------------------*/
/* The following #defines and externs are here for use by lu_parse_string.  */
/* They have been lifted directly from Randy Davis's ODLC library.          */
/* Only their names have been changed to protect the innocent.              */
/*--------------------------------------------------------------------------*/

/* The following statements duplicate two lexical analyzer constants for
   use in these modules */

#define YYLMAX ODLMAXSTMT
#define YYNEWLINE 10

/* The following statements define a routine for placing characters into
   the lexical analyzer's rescan buffer */

#define unput(X) for (nc=strlen(X); nc > 0; yyunput(X[--nc]))

extern int nc;                       /* Number of characters to unput    */

/* The following variables are defined in the module containing the
   parser action routines. */

extern AGGREGATE  ODLcurrent_aggregate; /* Pointer to current aggregate node*/
extern PARAMETER  ODLcurrent_parameter; /* Pointer to current parameter node*/
extern VALUE      ODLcurrent_value;     /* Pointer to current value         */

extern int        ODLerror_count;       /* Cumulative count of errors       */
extern int        ODLwarning_count;     /* Cumulative count of warnings     */

/* The following variables are defined in the lexical analyzer module */

extern FILE *yyin;                  /* Pointer to file for parser input     */
extern FILE *yyout;                 /* Pointer to file for parser output    */

extern char  yysbuf[YYLMAX];        /* Lexical analysis rescan buffer       */
extern char *yysptr;                /* Pointer into lexical rescan buffer   */

extern int   yylineno;              /* Current input line number            */
extern int   yyprevious;            /* Previous lexical analysis state      */


/**********************************************************************
 *$Component                                                          *
 *    AGGREGATE lu_append_object (object_ptr, new_object_class)       *
 *$Abstract                                                           *
 *    Appends a new object to an ODL tree.                            *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    APPEND                                                          *
 *    ODL                                                             *
 *    OBJECT                                                          *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    object_ptr:                                                     *
 *        The object_ptr variable is a pointer to the structure       *
 *        used to represent an object in a PDS label.                 *
 *    new_object_class:                                               *
 *        The new_object_class variable is a character string         *
 *        which contains the class of the new object to be added      *
 *        to a PDS label (e.g., "OBJECT = IMAGE" implies that         *
 *        "IMAGE" is the class of the object).                        *
 *$Outputs                                                            *
 *    None                                                            *
 *$Returns                                                            *
 *    new_object_ptr:                                                 *
 *        The new_object_ptr variable is a pointer to the structure   *
 *        used to represent a new object in a PDS label.              *
 *$Detailed_Description                                               *
 *    The lu_append_object routine mallocs storage for a new object   *
 *    structure and appends it onto the end of the list of children   *
 *    of the object passed in.  If the object passed in is NULL, then *
 *    a new "ROOT" object is created and returned.                    *
 *$Error_Handling                                                     *
 *    If the new object cannot be appended onto the tree then a NULL  *
 *    value is returned.                                              *
 *$Side_Effects                                                       *
 *    This routine causes memory to be allocated which must be freed  *
 *    by the calling routine.                                         *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    David P. Bernath / J.P.L.                                       *
 *$Version_and_Date                                                   *
 *    1.2   June 17, 1991                                             *
 *$Change_History                                                     *
 *    DPB   04-10-91   Original code.                                 *
 *    DPB   04-26-91   Added code to strip blanks from inputs.        *
 *    KLM   06-17-91   Moved from label.c to labutil.c and changed    *
 *                     prefix from lab_ to lu_.                       *
 *    DWS   07-13-00   Commenting out lines 600 thru 605.  They no    *
 *                     longer replace underscores with blanks.        *
 **********************************************************************/

AGGREGATE lu_append_object (object_ptr, new_object_class)

AGGREGATE object_ptr;
char *new_object_class;

{
    AGGREGATE new_object_ptr = {NULL};

/** BEGIN **/

    /*-----------------------------------------------------------------------*/
    /** IF the object pointer passed in is NULL THEN                        **/
    /**     Create a new ROOT object                                        **/
    /** ELSE                                                                **/
    /**     Append a new object onto the list of children of the object     **/
    /**         passed in                                                   **/
    /** ENDIF                                                               **/
    /*-----------------------------------------------------------------------*/

    if (object_ptr == NULL)
        new_object_ptr = NewAggregate (NULL, KA_OBJECT, "ROOT", "");
    else
    {
        util_strip_lead_and_trail (new_object_class, ' ');

        new_object_ptr = NewAggregate (object_ptr, 
                                       KA_OBJECT, new_object_class, "");
    }

    /*-----------------------------------------------------------------------*/
    /** RETURN a pointer to the new object structure                        **/
    /*-----------------------------------------------------------------------*/

    return (new_object_ptr);

/** END **/

}  /*  "lu_append_object"  */ 


/**********************************************************************
 *$Component                                                          *
 *    PARAMETER lu_append_parameter (object_ptr,                      *
 *                                    parameter_name, parameter_value)*
 *$Abstract                                                           *
 *    Appends a new parameter onto a list of parameters.              *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    APPEND                                                          *
 *    PARAMETER                                                       *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    object_ptr:                                                     *
 *        The object_ptr variable is a pointer to the structure       *
 *        used to represent an object in a PDS label.                 *
 *    parameter_name:                                                 *
 *        The parameter_name variable is a character string           *
 *        which contains the name of a parameter in a PDS label       *
 *        (e.g., the line "SCID = VG1" implies that "SCID" is the     *
 *        parameter name).                                            *
 *    parameter_value:                                                *
 *        The parameter_value variable is a character string          *
 *        which contains the value of a parameter in a PDS label      *
 *        (e.g., the line "SCID = VG1" implies that "VG1" is the      *
 *        value of the "SCID" parameter).                             *
 *$Outputs                                                            *
 *    None                                                            *
 *$Returns                                                            *
 *    new_parameter_ptr:                                              *
 *        The new_parameter_ptr variable is a pointer to the          *
 *        structure used to represent a new parameter in a PDS label. *
 *$Detailed_Description                                               *
 *    The lu_append_parameter routine mallocs storage for a new       *
 *    parameter structure and appends it onto the list of parameters  *
 *    for an object.                                                  *
 *$Error_Handling                                                     *
 *    If the object_ptr, parameter_name, or parameter_value are NULL, *
 *    or if memory could not be allocated, then a NULL value is       *
 *    returned.                                                       *
 *$Side_Effects                                                       *
 *    This routine causes memory to be allocated which must be freed  *
 *    by the calling routine.                                         *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    David P. Bernath / J.P.L.                                       *
 *$Version_and_Date                                                   *
 *    1.3   October 21, 1991                                          *
 *$Change_History                                                     *
 *    DPB   04-10-91   Original code.                                 *
 *    DPB   04-26-91   Added code to strip blanks from inputs.        *
 *    KLM   06-17-91   Moved from label.c to labutil.c and changed    *
 *                     prefix from lab_ to lu_.                       *
 *    MDD   10-21-91   Fixed free, malloc, and sys_exit_system calls  *
 **********************************************************************/
   
PARAMETER lu_append_parameter (object_ptr, parameter_name, parameter_value)

AGGREGATE object_ptr;
char *parameter_name;
char *parameter_value;

{
    PARAMETER new_parameter = {NULL};
    char *string;

    /*-----------------------------------------------------------------------*/
    /** IF the object pointer, parm name, and parm value are not NULL THEN  **/
    /*-----------------------------------------------------------------------*/

    if ((object_ptr != NULL) && (parameter_name != NULL) && (parameter_value != 
NULL))
    {
        /*-------------------------------------------------------------------*/
        /** Remove leading and trailing blanks from character inputs.       **/
        /*-------------------------------------------------------------------*/

        util_strip_lead_and_trail (parameter_name, ' ');

        /*-------------------------------------------------------------------*/
        /** Try to allocate memory for the string to be parsed              **/
        /*-------------------------------------------------------------------*/

        string = (char *) malloc (3 + String_Size(parameter_name) + 
                                      String_Size(parameter_value));
        Check_Malloc(string);

        /*---------------------------------------------------------------*/
        /** Save a pointer to the object's last parameter.  This will   **/
        /**     be used later to verify that the new parameter/value    **/
        /**     was parsed and added to the object.                     **/
        /*---------------------------------------------------------------*/

        new_parameter = object_ptr -> last_parameter;

        /*---------------------------------------------------------------*/
        /** Build the "PARAMETER = VALUE" string that is to be parsed.  **/
        /** Attempt to parse the string and add it to the object's      **/
        /**     list of parameters.                                     **/
        /*---------------------------------------------------------------*/

        sprintf (string, "%s = %s", parameter_name, parameter_value);
        lu_parse_string (object_ptr, string);

        /*---------------------------------------------------------------*/
        /** IF a new parameter was added to the object's list of        **/
        /**        parameters THEN                                      **/
        /**     Save a pointer to the new parameter structure.          **/
        /** ELSE                                                        **/
        /**     Set the new parameter pointer to NULL                   **/
        /** ENDIF                                                       **/
        /** (See, I told you we'd be using that parm pointer)           **/
        /*---------------------------------------------------------------*/

        if (new_parameter != object_ptr -> last_parameter)
            new_parameter = object_ptr -> last_parameter;
        else
            new_parameter = NULL;

        /*---------------------------------------------------------------*/
        /** Free the parsed string                                      **/
        /*---------------------------------------------------------------*/

        Lemme_Go(string);

    /*-----------------------------------------------------------------------*/
    /** ENDIF                                                               **/
    /*-----------------------------------------------------------------------*/

    }  /*  End:  "if ((object_ptr != NULL) && ..."  */

    /*-----------------------------------------------------------------------*/
    /** RETURN a pointer to the new parameter structure                     **/
    /*-----------------------------------------------------------------------*/

    return (new_parameter);
       
/** END **/

}  /*  "lu_append_parameter"  */



/**********************************************************************
 *$Component                                                          *
 *    VALUE lu_append_value (parameter_ptr, parameter_value)          *
 *$Abstract                                                           *
 *    Appends a new value onto a list of values.                      *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    APPEND                                                          *
 *    VALUE                                                           *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    parameter_ptr:                                                  *
 *        The parameter_ptr variable is a pointer to the structure    *
 *        used to represent a parameter in a PDS label.               *
 *    parameter_value:                                                *
 *        The parameter_value variable is a character string          *
 *        which contains the value of a parameter in a PDS label      *
 *        (e.g., the line "SCID = VG1" implies that "VG1" is the      *
 *        value of the "SCID" parameter).                             *
 *$Outputs                                                            *
 *    None                                                            *
 *$Returns                                                            *
 *    new_value_ptr:                                                  *
 *        The new_value_ptr variable is a pointer to the structure    *
 *        used to represent a new value of a parameter in a PDS label.*
 *$Detailed_Description                                               *
 *    The lu_append_value routine mallocs storage for a value         *
 *    structure and appends it onto the list of values for a          *
 *    parameter.                                                      *
 *$Error_Handling                                                     *
 *    If the value cannot be appended then a NULL value is returned.  *
 *$Side_Effects                                                       *
 *    This routine causes memory to be allocated which must be freed  *
 *    by the calling routine.                                         *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    David P. Bernath / J.P.L.                                       *
 *$Version_and_Date                                                   *
 *    1.2   June 17, 1991                                             *
 *$Change_History                                                     *
 *    DPB   04-10-91   Original code.                                 *
 *    DPB   04-18-91   Added '&' to pass address of item.             *
 *    KLM   06-17-91   Moved from label.c to labutil.c and changed    *
 *                     prefix from lab_ to lu_.                       *
 **********************************************************************/

VALUE lu_append_value (parameter_ptr, parameter_value)

PARAMETER parameter_ptr;
char *parameter_value;

{                                                                   
    AGGREGATE temp_object_ptr = {NULL};
    PARAMETER temp_parameter_ptr = {NULL};
    VALUE value_ptr = {NULL};

/** BEGIN **/

    /*-----------------------------------------------------------------------*/
    /** IF the parameter pointer passed in is not NULL THEN                 **/
    /*-----------------------------------------------------------------------*/

    if (parameter_ptr != NULL)
    {
        /*-------------------------------------------------------------------*/
        /** Try to create a temporary ROOT object.  We've got to create a   **/
        /**    temporary ROOT object because we will need to add a          **/
        /**    temporary parameter to it whose value is the value passed    **/
        /**    in (yow!).  Are you still with me?  Ok.  All of this         **/
        /**    convoluted logic is necessary because the routine which      **/
        /**    appends a new value onto a list of values requires as input  **/
        /**    (are you ready?) a pointer to the "item" structure inside of **/
        /**    a VALUE structure that has already been created (shades of   **/
        /**    the old "chicken and egg" problem).  So, once we've          **/
        /**    created a temporary object and appended a temporary          **/
        /**    parameter to it whose value is the thing we want to add to   **/
        /**    to the real parameter, then (and only then) can we extract   **/
        /**    the "item" part from this temporary mess and append it to    **/
        /**    the real parameter (mrrffk!).                                **/
        /*-------------------------------------------------------------------*/

        temp_object_ptr = NewAggregate (NULL, KA_OBJECT, "ROOT", "");

        /*-------------------------------------------------------------------*/
        /** IF the temporary object was created THEN                        **/
        /*-------------------------------------------------------------------*/

        if (temp_object_ptr != NULL)
        {
            /*---------------------------------------------------------------*/
            /** Add a temporary parameter to this temporary object whose    **/
            /**     value is the value passed in.                           **/
            /*---------------------------------------------------------------*/

            temp_parameter_ptr = lu_append_parameter (temp_object_ptr,
                                                      "TEMP", parameter_value);

            /*---------------------------------------------------------------*/
            /** IF the temporary parameter was added to the object THEN     **/
            /**     Extract the "item" part from the value structure and    **/
            /**         use it to add a new value to the real parameter.    **/
            /** ENDIF                                                       **/
            /*---------------------------------------------------------------*/

            if (temp_parameter_ptr != NULL)
	    {
		value_ptr = lu_new_value (parameter_ptr,
			    &temp_parameter_ptr->first_value->item);
	    }

            /*---------------------------------------------------------------*/
            /** Remove the temporary object and parameter.                  **/
            /*---------------------------------------------------------------*/

            {
                while (temp_object_ptr != NULL)
                   temp_object_ptr = RemoveAggregate (temp_object_ptr);

            }

        /*-------------------------------------------------------------------*/
        /** ENDIF                                                           **/
        /*-------------------------------------------------------------------*/

        }  /*  End:  "if (temp_object_ptr != NULL) ..."  */

    /*-----------------------------------------------------------------------*/
    /** ENDIF                                                               **/
    /*-----------------------------------------------------------------------*/

    }  /*  End:  "if (parameter_ptr != NULL) ..."  */

    /*-----------------------------------------------------------------------*/
    /** RETURN a pointer to the new value structure                         **/
    /*-----------------------------------------------------------------------*/

    return (value_ptr);

/** END **/

}  /*  "lu_append_value"  */


/**********************************************************************
 *$Component                                                          *
 *    STRING_LIST *lu_fetch_all_values (parameter_ptr,                *
 *                                       group_together, use_quotes)  *
 *$Abstract                                                           *
 *    Gets all the values of a keyword in a PDS label.                *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    VALUE                                                           *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    parameter_ptr:                                                  *
 *        The parameter_ptr variable is a pointer to the structure    *
 *        used to represent a parameter in a PDS label.               *
 *    group_together:                                                 *
 *        The group_together variable is a true/false flag which      *
 *        indicates whether sequence or set values should be          *
 *        combined into a single string, or appended onto the list as *
 *        separate strings.                                           *
 *    use_quotes:                                                     *
 *        The use_quotes variable is a true/false flag which indicates*
 *        whether quotes should be included in a string or not.       *
 *$Outputs                                                            *
 *    None.                                                           *
 *$Returns                                                            *
 *    label_value_list:                                               *
 *        The label_value_list variable is a string list              *
 *        containing all the values of a keyword in a PDS label.      *
 *$Detailed_Description                                               *
 *    The lu_fetch_all_values routine returns a STRING_LIST           *
 *    containing all of the values of a keyword.  If the              *
 *    group_together flag is FALSE, then set and sequence elements    *
 *    will be appended onto the list as separate strings.  If this    *
 *    flag is TRUE, then each row of values will be combined into     *
 *    a single string.                                                *
 *$Error_Handling                                                     *
 *    If the value passed in is NULL, this routine returns NULL.      *
 *$Side_Effects                                                       *
 *    This routine causes memory to be allocated which must be freed  *
 *    by the calling routine.                                         *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    David P. Bernath / J.P.L.                                       *
 *$Version_and_Date                                                   *
 *    1.2   October 21, 1991                                          *
 *$Change_History                                                     *
 *    DPB   05-30-91   Original code.                                 *
 *    KLM   06-17-91   Moved from label.c to labutil.c and changed    *
 *                     prefix from lab_ to lu_.                       *
 *    MDD   10-21-91   Fixed free, malloc, and sys_exit_system calls  *
 *    DWS    5-25-01   Added code to remove beginning /n from keyword *
 *					   value befor it is checked to determine if value*
 *					   is a standard value.                           *
 *    MDC   12-22-03   Added code specifically for kwvtool to append  *
 *                     a value's units onto the string list           *
 **********************************************************************/

STRING_LIST *lu_fetch_all_values (parameter_ptr, group_together, use_quotes)

PARAMETER parameter_ptr;
LOGICAL group_together;
LOGICAL use_quotes;

{
    VALUE value_ptr = {NULL};
    STRING_LIST *value_list = {NULL};
    char *value_string = {NULL};
	char *value_string_1 = {NULL};/* needed for pointer modification dws  052501 */
    char *value_set = {NULL};
    int columns = {0};
    int elements = {0};

#ifdef LV_KWVTOOL
	char *units = NULL;
	int counter = 0;
#endif

/** BEGIN **/

    /*-----------------------------------------------------------------------*/
    /** IF the parameter pointer passed in is not NULL THEN                 **/
    /*-----------------------------------------------------------------------*/
    if (parameter_ptr != NULL)
    {
        /*-------------------------------------------------------------------*/
        /** IF sequence and set elements are to be grouped together THEN    **/
        /**     Extract the number of columns (e.g., the number of elements **/
        /**         in each row) from the parameter structure.              **/
        /** ELSE                                                            **/
        /**     The number of columns is one (e.g., set and sequence        **/
        /**         elements are to be treated as individual values)        **/
        /** ENDIF                                                           **/
        /*-------------------------------------------------------------------*/

        if (group_together)
            columns = parameter_ptr -> columns;
        else
            columns = 1;
    
        /*-------------------------------------------------------------------*/
        /** Initialize the string which will be used to hold the values to  **/
        /**     be appended onto the list.                                  **/
        /*-------------------------------------------------------------------*/

        Malloc_String(value_set, 1);
    
        /*-------------------------------------------------------------------*/
        /** LOOP through all of the keyword's values . . .                  **/
        /*-------------------------------------------------------------------*/

        for (value_ptr = FirstValue (parameter_ptr), elements = 1;
               value_ptr != NULL;
                 value_ptr = NextValue (value_ptr), ++elements)
        {
            /*---------------------------------------------------------------*/
            /** Attempt to fetch a value from the label                     **/
            /*---------------------------------------------------------------*/
            value_string = lu_fetch_value (value_ptr, 
                                        ((use_quotes) && 
                                        (value_ptr -> item.type == TV_STRING)));
										    /*change by dws  052501                        */
			value_string_1 = value_string;  /*make a copy for modification if needed       */
			if((value_string_1[0] == '\\') && (value_string_1[1] == 'n'))/*   must get rid */
			{											      /*\n if there is one at start*/
				value_string_1 += 2;
			}
            /*---------------------------------------------------------------*/
            /** IF the attempt was successful THEN                          **/
            /**     Append the value onto the end of the string which       **/
            /**         contains the values that are to be grouped together.**/
            /**         (If the values are not really supposed to be        **/
            /**          grouped together, then this string will just be    **/
            /**          the value extracted from the label)                **/
            /** ENDIF                                                       **/
            /*---------------------------------------------------------------*/

            if (value_string != NULL)
            {
 /*             if ((value_ptr -> item.type == TV_SYMBOL) && 
                        (value_ptr -> item.format > 0))
                {
                    util_replace_char (value_string, '_', ' ');
                }
*/

/* 12-22-03 MDC - Added ifdef statement for KWVTOOL to print the units after the value */
#ifdef LV_KWVTOOL
				units = lu_get_units(value_ptr);
				
				if(units != NULL)
				{
					Realloc_String(value_set, (String_Size(value_set) +
		                                    String_Size(value_string_1) +
											String_Size(units)));
				}
				else
				{
					Realloc_String(value_set, (String_Size(value_set) + 
                                                    String_Size(value_string_1)));
				}
#else
	            Realloc_String(value_set, (String_Size(value_set) + 
                                                    String_Size(value_string_1)));
#endif
				if (*value_set != EOS)
                    strcat (value_set, " ");
                strcat (value_set, value_string_1);

/* 12-22-03 MDC - Concatenate the units to the end of the string if it was found */
#ifdef LV_KWVTOOL
				if(units != NULL) {

				strcat (value_set, " ");
				strcat (value_set, units);

				Lemme_Go(units);
				}
#endif

                Lemme_Go(value_string);/* this is the one to get rid of not value_string_1)*/
    
            }  /*  End:  "if (value_string != NULL) ..."  */
    
            /*---------------------------------------------------------------*/
            /** IF we have grouped together the requisite number of         **/
            /**         values into a single string THEN                    **/
            /**     Append this string onto the list and re-initialize it   **/
            /** ENDIF                                                       **/
            /*---------------------------------------------------------------*/

            if (elements >= columns)
            {
                elements = 0;
                value_list = util_append_string_list (value_list, 
                                                      value_set, STRING_TYPE);
                Lemme_Go(value_set);
                Malloc_String(value_set, 1);
    
            }  /*  End:  "if (i >= columns) ..."  */
    
        /*-------------------------------------------------------------------*/
        /** ENDLOOP                                                         **/
        /*-------------------------------------------------------------------*/

        }  /*  End:  "for (value_ptr = FirstValue ..."  */
    
        /*-------------------------------------------------------------------*/
        /** IF there is anything left over THEN                             **/
        /**     Append it onto the list                                     **/
        /** ENDIF                                                           **/
        /*-------------------------------------------------------------------*/

        if (*value_set != EOS)
            value_list = util_append_string_list(value_list,value_set,STRING_TYPE);
    
        Lemme_Go(value_set);

    /*-----------------------------------------------------------------------*/
    /** ENDIF                                                               **/
    /*-----------------------------------------------------------------------*/

    }  /*  End:  "if (parameter_ptr != NULL) ..."  */

    /*-----------------------------------------------------------------------*/
    /** RETURN a pointer to the list                                        **/
    /*-----------------------------------------------------------------------*/

    return (value_list);

/** END **/

}  /*  "lu_fetch_all_values"  */



/**********************************************************************
 *$Component                                                          *
 *    VALUE lu_get_units (value_ptr)                                  *
 *$Abstract                                                           *
 *    Gets the units of the value being passed in.                    *
 *$Inputs                                                             *
 *    value_ptr:                                                      *
 *        The value_ptr is a pointer to a structure which holds its   *
 *        data values.                                                *
 *$Outputs                                                            *
 *    None                                                            *
 *$Returns                                                            *
 *    units:                                                          *
 *        The units variable is a pointer to the value's units.       *
 *$Detailed_Description                                               *
 *    This routine will malloc storage to hold the units of the value *
 *    being passed into this function.                                *
 *$Error_Handling                                                     *
 *    NULL value is returned if no units were found                   *
 *$Side_Effects                                                       *
 *    This routine causes memory to be allocated which must be freed  *
 *    by the calling routine.                                         *
 *                                                                    *
 *$Change_History                                                     *
 *    MDC   12-22-03   Original code.                                 *
 **********************************************************************/
char *lu_get_units(VALUE value_ptr)
{
	char *units = NULL;

	switch(value_ptr->item.type)
	{
		case TV_REAL:  units = lu_format_units(value_ptr->item.value.real.units);
					   break;
		case TV_INTEGER:  units = lu_format_units(value_ptr->item.value.integer.units);
						  break;
		case TV_NULL_VALUE:  units = lu_format_units(value_ptr->item.value.null_data.units);
							 break;
	}

	return(units);
}
	
	




/**********************************************************************
 *$Component                                                          *
 *   char *lu_fetch_value (value_ptr, use_quotes)                     *
 *$Abstract                                                           *
 *    Gets a value of a keyword in a PDS label.                       *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    VALUE                                                           *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    value_ptr:                                                      *
 *        The value_ptr variable is a pointer to a VALUE              *
 *        structure, which represents the value of a keyword in a PDS *
 *        label.                                                      *
 *    use_quotes:                                                     *
 *        The use_quotes variable is a true/false flag which indicates*
 *        whether quotes should be included in a string or not.       *
 *$Outputs                                                            *
 *    None.                                                           *
 *$Returns                                                            *
 *    label_value_ptr:                                                *
 *        The label_value_ptr variable is a character string          *
 *        containing the value of a keyword in a PDS label.           *
 *$Detailed_Description                                               *
 *    The lu_fetch_value routine returns the value of a keyword in a  *
 *    PDS label in character format, given a pointer to the structure *
 *    which contains the value.                                       *
 *$Error_Handling                                                     *
 *    If the value passed in is NULL, this routine returns NULL.      *
 *$Side_Effects                                                       *
 *    This routine causes memory to be allocated which must be freed  *
 *    by the calling routine.                                         *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore / J.P.L.                                        *
 *$Version_and_Date                                                   *
 *    1.1   June 17, 1991                                             *
 *$Change_History                                                     *
 *    MDD   04-23-91   Original Code.                                 *
 *    KLM   06-17-91   Moved from label.c to labutil.c and changed    *
 *                     prefix from lab_ to lu_.                       *
 **********************************************************************/

char *lu_fetch_value (value_ptr, use_quotes)

VALUE value_ptr;
LOGICAL use_quotes;
{
   char *data_value = NULL;

   if (value_ptr != NULL)
   {
      switch (value_ptr -> item.type)
      {
         case TV_INTEGER:   data_value = lu_format_integer (&(value_ptr -> item));
                            break;
         case TV_REAL:      data_value = lu_format_real (&(value_ptr -> item));
                            break;
         case TV_SYMBOL:    data_value = lu_format_symbol (&(value_ptr -> item), use_quotes);
                            break;
         case TV_STRING:    data_value = lu_format_string (&(value_ptr -> item), use_quotes);
                            break;
         case TV_DATE:      data_value = lu_format_date (&(value_ptr -> item));
                            break;
         case TV_TIME:      data_value = lu_format_time (&(value_ptr -> item));
                            break;
         case TV_DATE_TIME: data_value = lu_format_date_time (&(value_ptr -> item));
                            break;
         case TV_NULL_VALUE: data_value = lu_format_null_data(&(value_ptr->item), use_quotes);
                            break;

         default:           break;
      }
   }
   return (data_value);

}
  /*  "lu_fetch_value"  */



/**********************************************************************
 *$Component                                                          *
 *    int lu_find_object_level (object_ptr)                           *
 *$Abstract                                                           *
 *    Gets the level of an object in the ODL tree.                    *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    OBJECT                                                          *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    object_ptr:                                                     *
 *        The object_ptr variable is a pointer to the structure       *
 *        used to represent an object in a PDS label.                 *
 *$Outputs                                                            *
 *    None                                                            *
 *$Returns                                                            *
 *    object_level:                                                   *
 *        The object_level variable in an integer representing the    *
 *        nesting level of an object in a PDS label.                  *
 *$Detailed_Description                                               *
 *    The lu_find_object_level routine returns the nesting level of   *
 *    an object in the ODL tree, given a pointer to the object. The   *
 *    nesting level of the ROOT object is 0.                          *
 *$Error_Handling                                                     *
 *    If a NULL pointer is passed in, then this routine returns -1.   *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore/JPL                                             *
 *$Version_and_Date                                                   *
 *    1.1   June 17, 1991                                             *
 *$Change_History                                                     *
 *    DPB   04-25-91   Original code.                                 *
 *    KLM   06-17-91   Moved from label.c to labutil.c and changed    *
 *                     prefix from lab_ to lu_.                       *
 **********************************************************************/

int lu_find_object_level (object_ptr)

AGGREGATE object_ptr;

{
   int level = -1;
 
   for (level = -1; object_ptr != NULL; 
            object_ptr = object_ptr->parent) level++;
   return (level);
}
  /*  "lu_find_object_level"  */



/**********************************************************************
 *$Component                                                          *
 *   char *lu_format_date (item_ptr)                                  *
 *$Abstract                                                           *
 *    Gets the value of a date value in a PDS label.                  *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    VALUE                                                           *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    item_ptr:                                                       *
 *        The item_ptr variable is a pointer to a VALUE_DATA          *
 *        structure, which represents the value of a keyword in a PDS *
 *        label.                                                      *
 *$Outputs                                                            *
 *    None.                                                           *
 *$Returns                                                            *
 *    label_value_ptr:                                                *
 *        The label_value_ptr variable is a character string          *
 *        containing the value of a keyword in a PDS label.           *
 *$Detailed_Description                                               *
 *    The lu_format_date routine translates a date value in a         *
 *    PDS label into character string format.                         *
 *$Error_Handling                                                     *
 *    If the value passed in is NULL or the item_ptr structure passed *
 *    in is invalid, then this routine returns NULL.                  *
 *$Side_Effects                                                       *
 *    This routine causes memory to be allocated which must be freed  *
 *    by the calling routine.                                         *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore / J.P.L.                                        *
 *$Version_and_Date                                                   *
 *    1.2   October 21, 1991                                          *
 *$Change_History                                                     *
 *    MDD   04-24-91   Original Code.                                 *
 *    KLM   06-17-91   Moved from label.c to labutil.c and changed    *
 *                     prefix from lab_ to lu_.                       *
 *    MDD   10-21-91   Fixed malloc, free, and sys_exit_system calls  *
 **********************************************************************/

char *lu_format_date (item_ptr)

VALUE_DATA *item_ptr;

{
   int     day;                  
   int     doy;                  
   int     month;                
   int     year;                 
   char    temp_str [PDS_MAXLINE + 1];
   char    *date = NULL;

/** BEGIN **/

   /*-----------------------------------------------------------------*/
   /** IF a value was passed in and the structure is valid THEN      **/
   /*-----------------------------------------------------------------*/

   if (item_ptr != NULL && item_ptr -> valid != 0)
   {

      /*--------------------------------------------------------------*/
      /** get the date components from the structure                 **/
      /*--------------------------------------------------------------*/
     
      year  = (int) item_ptr -> value.date_time.year;
      doy   = (int) item_ptr -> value.date_time.doy;
      month = (int) item_ptr -> value.date_time.month;
      day   = (int) item_ptr -> value.date_time.day;

      /*--------------------------------------------------------------*/
      /** format the date as year-month-day if the format flag is    **/
      /**   30, 31, 32, 33, or 34 (???) (don't ask me why)           **/
      /*--------------------------------------------------------------*/

      if (item_ptr -> format / 10 == 3)
      {
         sprintf (temp_str, "%d-%02d-%02d", year, month, day);
      }
      /*--------------------------------------------------------------*/
      /** otherwise format the date as year-day-of-year              **/
      /*--------------------------------------------------------------*/

      else
      {
         sprintf (temp_str, "%d-%03d", year, doy);
      } 
      /*--------------------------------------------------------------*/
      /** copy the date to the return value                          **/
      /*--------------------------------------------------------------*/

      Malloc_String(date, String_Size(temp_str));
      strcpy (date, temp_str);
   }
   /*-----------------------------------------------------------------*/
   /** ENDIF a value was passed in...                                **/
   /*-----------------------------------------------------------------*/

   return (date);

/** END lu_format_date **/
}

/**********************************************************************
 *$Component                                                          *
 *   char *lu_format_date_time (item_ptr)                             *
 *$Abstract                                                           *
 *    Gets the value of a date/time value in a PDS label.             *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    VALUE                                                           *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    item_ptr:                                                       *
 *        The item_ptr variable is a pointer to a VALUE_DATA          *
 *        structure, which represents the value of a keyword in a PDS *
 *        label.                                                      *
 *$Outputs                                                            *
 *    None.                                                           *
 *$Returns                                                            *
 *    label_value_ptr:                                                *
 *        The label_value_ptr variable is a character string          *
 *        containing the value of a keyword in a PDS label.           *
 *$Detailed_Description                                               *
 *    The lu_format_date_time routine translates a date/time value    *
 *    in a PDS label into character string format.                    *
 *$Error_Handling                                                     *
 *    If the value passed in is NULL or the item_ptr structure passed *
 *    in is invalid, then this routine returns NULL.                  *
 *$Side_Effects                                                       *
 *    This routine causes memory to be allocated which must be freed  *
 *    by the calling routine.                                         *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore / J.P.L.                                        *
 *$Version_and_Date                                                   *
 *    1.2   October 21, 1991                                          *
 *$Change_History                                                     *
 *    MDD   04-24-91   Original Code.                                 *
 *    KLM   06-17-91   Moved from label.c to labutil.c and changed    *
 *                     prefix from lab_ to lu_.                       *
 *    MDD   10-21-91   Fixed malloc, free, and sys_exit_system calls  *
 **********************************************************************/

char *lu_format_date_time (item_ptr)

VALUE_DATA *item_ptr;
{

   char *date = NULL;
   char *time = NULL;
   char *date_time = NULL;

/** BEGIN **/
   /*----------------------------------------------------------------*/
   /** IF a value was passed in and the structure is valid THEN     **/
   /*----------------------------------------------------------------*/

   if (item_ptr != NULL && item_ptr -> valid != 0)
   {
      /*-------------------------------------------------------------*/
      /** format the date                                           **/
      /** IF that was successful THEN                               **/
      /*-------------------------------------------------------------*/
      
      date = lu_format_date (item_ptr);
      if (date != NULL)
      {
        
         /*---------------------------------------------------------*/
         /** format the time                                       **/
         /** if that was successful then append the date and time  **/
         /**    together and copy the result to the return value   **/
         /*---------------------------------------------------------*/

         time = lu_format_time (item_ptr);
         if (time != NULL)
         {
            Malloc_String(date_time, (String_Size(date) + String_Size(time)));
            sprintf (date_time, "%sT%s", date, time);
            Lemme_Go(time);
	 }
         Lemme_Go(date);
      }
      /*-------------------------------------------------------------*/
      /** ENDIF that was successful...                              **/
      /*-------------------------------------------------------------*/
   }
   /*----------------------------------------------------------------*/
   /** ENDIF a value was passed in...                               **/
   /*----------------------------------------------------------------*/

   return (date_time);

/** END lu_format_date_time **/
}



/**********************************************************************
 *$Component                                                          *
 *   char *lu_format_integer (item_ptr)                               *
 *$Abstract                                                           *
 *    Gets a value of an integer value in a PDS label.                *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    VALUE                                                           *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    item_ptr:                                                       *
 *        The item_ptr variable is a pointer to a VALUE_DATA          *
 *        structure, which represents the value of a keyword in a PDS *
 *        label.                                                      *
 *$Outputs                                                            *
 *    None.                                                           *
 *$Returns                                                            *
 *    label_value_ptr:                                                *
 *        The label_value_ptr variable is a character string          *
 *        containing the value of a keyword in a PDS label.           *
 *$Detailed_Description                                               *
 *    The lu_format_integer routine translates an integer keyword     *
 *    value from a PDS label into character string format, given the  *
 *    structure which contains the value.                             *
 *$Error_Handling                                                     *
 *    If the value passed in is NULL or the item_ptr structure passed *
 *    in is invalid, then this routine returns NULL.                  *
 *$Side_Effects                                                       *
 *    This routine causes memory to be allocated which must be freed  *
 *    by the calling routine.                                         *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore / J.P.L.                                        *
 *$Version_and_Date                                                   *
 *    1.3   November 21, 1991                                         *
 *$Change_History                                                     *
 *    MDD   04-24-91   Original Code.                                 *
 *    KLM   06-17-91   Moved from label.c to labutil.c and changed    *
 *                     prefix from lab_ to lu_.                       *
 *    MDD   10-21-91   Fixed malloc, free, and sys_exit_system calls  *
 *    MDD   11-21-91   Fixed bug with converting long int to string   *
 **********************************************************************/

char *lu_format_integer (item_ptr)

VALUE_DATA *item_ptr;
{
  unsigned long    num;
  char             *number = NULL;
  char             temp_str [PDS_MAXLINE + 1];


/** BEGIN **/
  /*------------------------------------------------------------------*/
  /** IF a value was passed in and it is valid THEN                  **/
  /*------------------------------------------------------------------*/

  if (item_ptr != NULL && item_ptr -> valid != 0)
  {

     /*---------------------------------------------------------------*/
     /** get the basic integer value from the structure              **/
     /** convert it to a string and copy to the return value         **/
     /*---------------------------------------------------------------*/

     num = item_ptr -> value.integer.number;

     /************ BUG ALERT !!! ***************/
     /* The following converts an unsigned int */
     /* to a signed one.  This results in the  */
     /* wrong answer if the original number    */
     /* was too big to be a signed int. But    */
     /* it's not my parser. Sigh.              */
     /******************************************/

     sprintf (temp_str, "%ld", num);
     Malloc_String(number, String_Size(temp_str));
     strcpy (number, temp_str);
  }
  /*------------------------------------------------------------------*/
  /** ENDIF a value was passed in...                                 **/
  /*------------------------------------------------------------------*/

  
  return (number);

/** END lu_format_integer **/
}

/**********************************************************************
 *$Component                                                          *
 *   char *lu_format_real (item_ptr)                                  *
 *$Abstract                                                           *
 *    Gets the value of real value in a PDS label.                    *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    VALUE                                                           *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    item_ptr:                                                       *
 *        The item_ptr variable is a pointer to a VALUE_DATA          *
 *        structure, which represents the value of a keyword in a PDS *
 *        label.                                                      *
 *$Outputs                                                            *
 *    None.                                                           *
 *$Returns                                                            *
 *    label_value_ptr:                                                *
 *        The label_value_ptr variable is a character string          *
 *        containing the value of a keyword in a PDS label.           *
 *$Detailed_Description                                               *
 *    The lu_format_real routine translates a real keyword            *
 *    value from a PDS label into character string format, given the  *
 *    structure which contains the value.                             *
 *$Error_Handling                                                     *
 *    If the value passed in is NULL or the item_ptr structure passed *
 *    in is invalid, then this routine returns NULL.                  *
 *$Side_Effects                                                       *
 *    This routine causes memory to be allocated which must be freed  *
 *    by the calling routine.                                         *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore / J.P.L.                                        *
 *$Version_and_Date                                                   *
 *    1.2   October 21, 1991                                          *
 *$Change_History                                                     *
 *    MDD   04-24-91   Original Code.                                 *
 *    KLM   06-17-91   Moved from label.c to labutil.c and changed    *
 *                     prefix from lab_ to lu_.                       *
 *    MDD   10-21-91   Fixed malloc, free, and sys_exit_system calls  *
 **********************************************************************/

char *lu_format_real (item_ptr)

VALUE_DATA *item_ptr;
{
   char *number = NULL;
   char temp_str [PDS_MAXLINE + 1];


/** BEGIN **/ 
   /*------------------------------------------------------------------*/
   /** IF a value was passed in and it is valid THEN                  **/
   /*------------------------------------------------------------------*/

   if (item_ptr != NULL && item_ptr -> valid != 0)
   { 
      /*---------------------------------------------------------------*/
      /** IF we don't know what type of format the value has THEN     **/
      /**    (which seems to be impossible)                           **/
      /**    convert it to a string using general format              **/
      /*---------------------------------------------------------------*/

      if (item_ptr -> format == 0)
      {
         sprintf (temp_str, "%g", item_ptr -> value.real.number);
      }
      /*---------------------------------------------------------------*/
      /** ELSE IF the value is supposed to be in decimal format THEN  **/
      /**    convert it to a string using float decimal format        **/
      /*---------------------------------------------------------------*/

      else if (item_ptr -> format == 1)
      {
         sprintf (temp_str, "%.*f",
                     (item_ptr -> precision <= 0)? 1 : item_ptr -> precision,
                         item_ptr -> value.real.number);
      }
      /*---------------------------------------------------------------*/
      /** ELSE                                                        **/
      /**    convert it to a string using exponential format          **/
      /*---------------------------------------------------------------*/
      else
      {

         sprintf (temp_str, "%.*e",
                      (item_ptr -> precision <= 0)? 1 : item_ptr -> precision,
                          item_ptr -> value.real.number);
      }
      /*---------------------------------------------------------------*/
      /** ENDIF                                                       **/
      /** copy the value to the return variable                       **/
      /*---------------------------------------------------------------*/

      Malloc_String(number, String_Size (temp_str));
      strcpy (number, temp_str);   
   }
   /*------------------------------------------------------------------*/
   /** ENDIF a value was passed in...                                 **/
   /*------------------------------------------------------------------*/

   return (number);

/** END lu_format_real **/
}


/**********************************************************************
 *$Component                                                          *
 *   char *lu_format_string (item_ptr, use_quotes)                    *
 *$Abstract                                                           *
 *    Gets the value of a string value in a PDS label.                *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    VALUE                                                           *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    item_ptr:                                                       *
 *        The item_ptr variable is a pointer to a VALUE_DATA          *
 *        structure, which represents the value of a keyword in a PDS *
 *        label.                                                      *
 *    use_quotes:                                                     *
 *        The use_quotes variable is a true/false flag which indicates*
 *        whether quotes should be included in a string or not.       *
 *$Outputs                                                            *
 *    None.                                                           *
 *$Returns                                                            *
 *    label_value_ptr:                                                *
 *        The label_value_ptr variable is a character string          *
 *        containing the value of a keyword in a PDS label.           *
 *$Detailed_Description                                               *
 *    The lu_format_string routine translates a string value in a     *
 *    PDS label into character string format. If the use_quotes input *
 *    is TRUE, then double quotes will be included in the string      *
 *    returned.                                                       *
 *$Error_Handling                                                     *
 *    If the value passed in is NULL or the item_ptr structure passed *
 *    in is invalid, then this routine returns NULL.                  *
 *$Side_Effects                                                       *
 *    This routine causes memory to be allocated which must be freed  *
 *    by the calling routine.                                         *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore / J.P.L.                                        *
 *$Version_and_Date                                                   *
 *    1.2   October 21, 1991                                          *
 *$Change_History                                                     *
 *    MDD   04-24-91   Original Code.                                 *
 *    KLM   06-17-91   Moved from label.c to labutil.c and changed    *
 *                     prefix from lab_ to lu_.                       *
 *    MDD   10-21-91   Fixed malloc, free, and sys_exit_system calls  *
 **********************************************************************/

char *lu_format_string (item_ptr, use_quotes)

VALUE_DATA *item_ptr;
LOGICAL use_quotes;
{
  char   *text = NULL;
  
/** BEGIN **/
   /*----------------------------------------------------------------*/
   /** IF a value was passed in and the structure is valid THEN     **/
   /*----------------------------------------------------------------*/

   if (item_ptr != NULL && item_ptr -> valid != 0)
   {
     /*-------------------------------------------------------------*/
     /** IF quotes are not desired THEN                            **/
     /**   format the symbol without quotes                        **/
     /** ELSE                                                      **/
     /**   format it with quotes                                   **/
     /** ENDIF                                                     **/
     /*-------------------------------------------------------------*/

     if (use_quotes)
     {
        Malloc_String(text, (String_Size(item_ptr -> value.string) + 2));
        sprintf (text, "\"%s\"", item_ptr -> value.string);
     }
     /*-------------------------------------------------------------*/
     /** copy the symbol to the return value                       **/
     /*-------------------------------------------------------------*/

     else
     {
        Malloc_String(text, String_Size(item_ptr -> value.string));
        strcpy (text, item_ptr -> value.string);
     }
  }
  /*----------------------------------------------------------------*/
  /** ENDIF a value was passed in...                               **/
  /*----------------------------------------------------------------*/

  return (text);

/** END lu_format_string **/
}


/**********************************************************************
 *$Component                                                          *
 *   char *lu_format_symbol (item_ptr, use_quotes)                    *
 *$Abstract                                                           *
 *    Gets the value of a symbolic value in a PDS label.              *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    VALUE                                                           *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    item_ptr:                                                       *
 *        The item_ptr variable is a pointer to a VALUE_DATA          *
 *        structure, which represents the value of a keyword in a PDS *
 *        label.                                                      *
 *    use_quotes:                                                     *
 *        The use_quotes variable is a true/false flag which indicates*
 *        whether quotes should be included in a string or not.       *
 *$Outputs                                                            *
 *    None.                                                           *
 *$Returns                                                            *
 *    label_value_ptr:                                                *
 *        The label_value_ptr variable is a character string          *
 *        containing the value of a keyword in a PDS label.           *
 *$Detailed_Description                                               *
 *    The lu_format_symbol routine translates a symbols value in a    *
 *    PDS label into character string format. If the use_quotes input *
 *    is TRUE, then single quotes will be included in the string      *
 *    returned.                                                       *
 *$Error_Handling                                                     *
 *    If the value passed in is NULL or the item_ptr structure passed *
 *    in is invalid, then this routine returns NULL.                  *
 *$Side_Effects                                                       *
 *    This routine causes memory to be allocated which must be freed  *
 *    by the calling routine.                                         *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore / J.P.L.                                        *
 *$Version_and_Date                                                   *
 *    1.2   October 21, 1991                                          *
 *$Change_History                                                     *
 *    MDD   04-24-91   Original Code.                                 *
 *    KLM   06-17-91   Moved from label.c to labutil.c and changed    *
 *                     prefix from lab_ to lu_.                       *
 *    MDD   10-21-91   Fixed malloc, free, and sys_exit_system calls  *
 **********************************************************************/

char *lu_format_symbol (item_ptr, use_quotes)

VALUE_DATA *item_ptr;
LOGICAL use_quotes;

{

   char temp_str [PDS_MAXLINE + 1];
   char *symbol = NULL;

/** BEGIN **/
   /*----------------------------------------------------------------*/
   /** IF a value was passed in and the structure is valid THEN     **/
   /*----------------------------------------------------------------*/

   if (item_ptr != NULL && item_ptr -> valid != 0)
   {
      /*-------------------------------------------------------------*/
      /** IF quotes are not desired THEN                            **/
      /**   format the symbol without quotes                        **/
      /** ELSE                                                      **/
      /**   format it with quotes                                   **/
      /** ENDIF                                                     **/
      /*-------------------------------------------------------------*/
 
      if (use_quotes == FALSE)
      {
         sprintf (temp_str, "%s", item_ptr -> value.string);
      }
      else
      {
         sprintf (temp_str, "\'%s\'", item_ptr -> value.string);
      }
      /*-------------------------------------------------------------*/
      /** copy the symbol to the return value                       **/
      /*-------------------------------------------------------------*/

      Malloc_String(symbol, String_Size(temp_str));
      strcpy (symbol, temp_str);
   }
   /*----------------------------------------------------------------*/
   /** ENDIF a value was passed in...                               **/
   /*----------------------------------------------------------------*/

   return (symbol);

/** END lu_format_symbol **/
}


/**********************************************************************
 *$Component                                                          *
 *   char *lu_format_null_data (item_ptr, use_quotes)                 *
 *$Abstract                                                           *
 *    Gets the value of a null data in a PDS label.                   *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    VALUE                                                           *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    item_ptr:                                                       *
 *        The item_ptr variable is a pointer to a VALUE_DATA          *
 *        structure, which represents the value of a keyword in a PDS *
 *        label.                                                      *
 *    use_quotes:                                                     *
 *        The use_quotes variable is a true/false flag which indicates*
 *        whether quotes should be included in a string or not.       *
 *$Outputs                                                            *
 *    None.                                                           *
 *$Returns                                                            *
 *    label_value_ptr:                                                *
 *        The label_value_ptr variable is a character string          *
 *        containing the value of a keyword in a PDS label.           *
 *$Detailed_Description                                               *
 *    The lu_format_null_vaule routine translates a null data in a    *
 *    PDS label into character string format. If the use_quotes input *
 *    is TRUE, then single quotes will be included in the string      *
 *    returned.                                                       *
 *$Error_Handling                                                     *
 *    If the value passed in is NULL or the item_ptr structure passed *
 *    in is invalid, then this routine returns NULL.                  *
 *$Side_Effects                                                       *
 *    This routine causes memory to be allocated which must be freed  *
 *    by the calling routine.                                         *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore / J.P.L.                                        *
 *$Version_and_Date                                                   *
 *    1.0   December 17, 2003                                          *
 *$Change_History                                                     *
 **********************************************************************/

char *lu_format_null_data (item_ptr, use_quotes)

VALUE_DATA *item_ptr;
LOGICAL use_quotes;

{

   char temp_str [PDS_MAXLINE + 1];
   char *symbol = NULL;

/** BEGIN **/
   /*----------------------------------------------------------------*/
   /** IF a value was passed in and the structure is valid THEN     **/
   /*----------------------------------------------------------------*/

   if (item_ptr != NULL && item_ptr -> valid != 0)
   {
      /*-------------------------------------------------------------*/
      /** IF quotes are not desired THEN                            **/
      /**   format the symbol without quotes                        **/
      /** ELSE                                                      **/
      /**   format it with quotes                                   **/
      /** ENDIF                                                     **/
      /*-------------------------------------------------------------*/
 
      if (use_quotes == FALSE)
      {
         sprintf (temp_str, "%s", item_ptr -> value.null_data.string);
      }
      else
      {
         sprintf (temp_str, "\'%s\'", item_ptr -> value.null_data.string);
      }
      /*-------------------------------------------------------------*/
      /** copy the symbol to the return value                       **/
      /*-------------------------------------------------------------*/

      Malloc_String(symbol, String_Size(temp_str));
      strcpy (symbol, temp_str);
   }
   /*----------------------------------------------------------------*/
   /** ENDIF a value was passed in...                               **/
   /*----------------------------------------------------------------*/

   return (symbol);

/** END lu_format_null_data **/
}


/**********************************************************************
 *$Component                                                          *
 *   char *lu_format_time (item_ptr)                                  *
 *$Abstract                                                           *
 *    Gets the value of a time value in a PDS label.                  *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    VALUE                                                           *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    item_ptr:                                                       *
 *        The item_ptr variable is a pointer to a VALUE_DATA          *
 *        structure, which represents the value of a keyword in a PDS *
 *        label.                                                      *
 *$Outputs                                                            *
 *    None.                                                           *
 *$Returns                                                            *
 *    label_value_ptr:                                                *
 *        The label_value_ptr variable is a character string          *
 *        containing the value of a keyword in a PDS label.           *
 *$Detailed_Description                                               *
 *    The lu_format_time routine translates a time value in a         *
 *    PDS label into character string format.                         *
 *$Error_Handling                                                     *
 *    If the value passed in is NULL or the item_ptr structure passed *
 *    in is invalid, then this routine returns NULL.                  *
 *$Side_Effects                                                       *
 *    This routine causes memory to be allocated which must be freed  *
 *    by the calling routine.                                         *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore / J.P.L.                                        *
 *$Version_and_Date                                                   *
 *    1.2   October 21, 1991                                          *
 *$Change_History                                                     *
 *    MDD   04-24-91   Original Code.                                 *
 *    KLM   06-17-91   Moved from label.c to labutil.c and changed    *
 *                     prefix from lab_ to lu_.                       *
 *    MDD   10-21-91   Fixed malloc, free, and sys_exit_system calls  *
 **********************************************************************/

char *lu_format_time (item_ptr)

VALUE_DATA *item_ptr;
{
   int     hours;                
   int     minutes;              
   double  seconds;              
   char    temp_str [PDS_MAXLINE + 1];
   char    *time = NULL;
   int     len;

/** BEGIN **/
   /*-----------------------------------------------------------------*/
   /** IF a value was passed in and the structure is valid THEN      **/
   /*-----------------------------------------------------------------*/

   if (item_ptr != NULL && item_ptr -> valid != 0)
   {

     /*--------------------------------------------------------------*/
     /** get the time components from the structure                 **/
     /** format the hours and minutes                               **/
     /*--------------------------------------------------------------*/

     hours   = (int) item_ptr -> value.date_time.hours;
     minutes = (int) item_ptr -> value.date_time.minutes;
     seconds = (double) item_ptr -> value.date_time.seconds +
               (double) item_ptr -> value.date_time.nanoseconds * 1.0E-9;
     sprintf (temp_str, "%02d:%02d", hours, minutes);
     len = strlen (temp_str);

     /*--------------------------------------------------------------*/
     /** print out seconds and nanoseconds if the format flag is    **/
     /**   13, 23, 33, or 43 etc... or zeros if not                 **/  
     /*--------------------------------------------------------------*/

     if (item_ptr -> format % 10 == 3)
     {
        temp_str [len++] = ':';
        if (seconds < 10.0)
        {
           temp_str [len++] = '0';
        }
        sprintf (&temp_str [len], "%.*f", item_ptr -> precision, seconds);
        len += strlen (&temp_str [len]);
     }
     else
     {
        sprintf (&temp_str [len], ":00.000");
        len += strlen (&temp_str [len]);
     }
     /*--------------------------------------------------------------*/
     /** get the time zone information from the structure           **/
     /*--------------------------------------------------------------*/

     hours   = (int) item_ptr -> value.date_time.zone_hours;
     minutes = (int) item_ptr -> value.date_time.zone_minutes;

     /*--------------------------------------------------------------*/
     /** IF the time is in UTC THEN                                 **/
     /**    add a Z to the time                                     **/
     /*--------------------------------------------------------------*/

     if (hours == 0 && minutes == 0)
     {
        temp_str [len++] = 'Z';
        temp_str [len] = '\0';
     }
     /*--------------------------------------------------------------*/
     /** ELSE                                                       **/
     /**   append the time zone offset to the time                  **/
     /*--------------------------------------------------------------*/

     else 
     {
        sprintf (&temp_str [len], "+%02d:%02d", hours, minutes);
        len += strlen (&temp_str [len]);
     }
     /*--------------------------------------------------------------*/
     /** ENDIF the time is in UTC...                                **/
     /** copy the time to the return value                          **/
     /*--------------------------------------------------------------*/

     Malloc_String(time, String_Size(temp_str));
     strcpy (time, temp_str);
   }
   /*-----------------------------------------------------------------*/
   /** ENDIF a value was passed in...                                **/
   /*-----------------------------------------------------------------*/

  return (time);

/** END lu_format_time **/
}


/**********************************************************************
 *$Component                                                          *
 *   char *lu_format_units (ODL_units)                                *
 *$Abstract                                                           *
 *    Gets a units expression in a PDS label.                         *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    UNITS                                                           *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    ODL_units:                                                      *
 *        The ODL_units variable is a pointer to an ODL_Units         *
 *        structure, which represents a units expression in a PDS     *
 *        label.                                                      *
 *$Outputs                                                            *
 *    None.                                                           *
 *$Returns                                                            *
 *    units_value_ptr:                                                *
 *        The units_value_ptr variable is a character string          *
 *        containing a units expression from a PDS Label.             *
 *$Detailed_Description                                               *
 *    The lu_format_units routine translates the data structure       *
 *    containing a units expression from a PDS label to a character   *
 *    string, e.g., <METER/SECOND>                                    *
 *$Error_Handling                                                     *
 *    If the value passed in is NULL this routine returns NULL.       *
 *$Side_Effects                                                       *
 *    This routine causes memory to be allocated which must be freed  *
 *    by the calling routine.                                         *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore / J.P.L.                                        *
 *$Version_and_Date                                                   *
 *    1.2   October 21, 1991                                          *
 *$Change_History                                                     *
 *    MDD   04-24-91   Original Code.                                 *
 *    KLM   06-17-91   Moved from label.c to labutil.c and changed    *
 *                     prefix from lab_ to lu_.                       *
 *    MDD   10-21-91   Fixed malloc, free, and sys_exit_system calls  *
 *    DWS   02-01-00   changed malloc to len + 1                      *
 **********************************************************************/

char *lu_format_units (ODL_units)

struct ODLUnits *ODL_units;
{
   struct ODLUnits *current_units;   
   int              first_field = 1; 
   int              uexp;            
   char             *unit_str = NULL;
   int              len = 0;
   char temp_str    [PDS_MAXLINE + 1];

/** BEGIN **/
   /*------------------------------------------------------------------*/
   /** IF something was passed in THEN                                **/
   /*------------------------------------------------------------------*/

   if (ODL_units != NULL)
   {
      current_units = ODL_units;
      temp_str [len++] = '<';

      /*---------------------------------------------------------------*/
      /** LOOP through each field in the units expression             **/
      /*---------------------------------------------------------------*/

      while (current_units != NULL)
      {
         /*------------------------------------------------------------*/
         /** Well, now, each field in the units structure consists of **/
         /** the name of the unit and an exponent.  If the exponent is**/
         /** negative then that means the the unit was really in the  **/
         /** BOTTOM half of the expression, otherwise it was in the   **/
         /** TOP half, so:                                            **/
         /** IF this is not the first field THEN                      **/
         /**    add a slash to the expression if the exponent is      **/
         /**    negative and then negate it, or an asterisk if it     **/
         /**    is positive                                           **/
	 /*------------------------------------------------------------*/
         
         uexp = current_units -> exponent;
         if (!first_field)
         {
            if (uexp > 0)
            {
               temp_str [len++] = '*';
            }      
            else
            {
               temp_str [len++] = '/';
               uexp = -uexp;
            }
         }
         /*------------------------------------------------------------*/
         /** ELSE                                                     **/
         /**   this is the first field so do nothing                  **/
         /*------------------------------------------------------------*/

         else
         { 
            first_field = 0;
         }
         /*------------------------------------------------------------*/
         /** ENDIF this not the first field...                        **/
         /** add the unit name and the exponent                       **/
         /*------------------------------------------------------------*/

         sprintf (&temp_str [len], "%s", current_units -> designator);
         len += strlen (&temp_str [len]);
         if (uexp != 1)
         {
            sprintf (&temp_str [len], "**%d", uexp);
            len += strlen (&temp_str [len]);
         }
         current_units = current_units -> next_field;
      }
      /*---------------------------------------------------------------*/
      /** ENDLOOP through each field in the units expression          **/
      /** terminate the expression and copy it to the return value    **/
      /*---------------------------------------------------------------*/

      temp_str [len++] = '>';
      temp_str [len] = '\0';
      Malloc_String(unit_str, len + 1);  /* changed to len + 1 from len  */
      strcpy (unit_str, temp_str);		 /* on 02-01-00  DWS             */
   }
   /*------------------------------------------------------------------*/
   /** ENDIF something was passed in...                               **/
   /*------------------------------------------------------------------*/
   return (unit_str);

/** END lu_format_units **/
}
           
/**********************************************************************
 *$Component                                                          *
 *    char *lu_keyword_value (object_ptr, keyword_name,               *
 *                            keyword_position, specific)             *
 *$Abstract                                                           *
 *    Fetches the value of a keyword.                                 *
 *$Keywords                                                           *
 *    LABUTIL                                                         *
 *    KEYWORD                                                         *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    object_ptr:                                                     *
 *        The object_ptr variable is a pointer to the structure       *
 *        used to represent an object in a PDS label.                 *
 *    keyword_name:                                                   *
 *        The keyword_name variable is a character string             *
 *        which contains the name of a keyword in a PDS label         *
 *        (e.g., the line "SCID = VG1" implies that "SCID" is the     *
 *        keyword name).                                              *
 *    keyword_position:                                               *
 *        The keyword_position variable is an integer which           *
 *        represents the relative position of a keyword in an         *
 *        object in a PDS label.  If this variable is used in         *
 *        conjunction with the keyword_name variable, then it         *
 *        represents a particular occurrence of that keyword          *
 *        in the object (e.g., if keyword_name is "SCID" and          *
 *        keyword_position is 2, then this represents the second      *
 *        "SCID" keyword in the object).  On the other hand, if       *
 *        this variable is used by itself, it represents the absolute *
 *        position of the keyword within the object, starting with    *
 *        first keyword in the object (position = 1).                 *
 *    specific:                                                       *
 *        The specific variable is a flag which indicates whether     *
 *        or not to search for a specific keyword or a class of       *
 *        keywords.                                                   *
 *$Outputs                                                            *
 *    None                                                            *
 *$Returns                                                            *
 *    keyword_value:                                                  *
 *        The keyword_value variable is a character string            *
 *        which contains the value of a keyword in a PDS label        *
 *        (e.g., the line "SCID = VG1" implies that "VG1" is the      *
 *        value of the "SCID" keyword).                               *
 *$Detailed_Description                                               *
 *    The lu_keyword_value routine searches an object for a keyword   *
 *    and returns a character string containing the first value on    *
 *    the list.   Unlike most of the "lab" routines, this one does    *
 *    not search from the beginning of the tree to find the object.   *
 *    The 'specific' input indicates whether the keyword being        *
 *    searched for should match the keyword_name input exactly, or    *
 *    only needs to have the same class word.                         *
 *$Error_Handling                                                     *
 *    None                                                            *
 *$Side_Effects                                                       *
 *    This routine causes memory to be allocated which must be freed  *
 *    by the calling routine.                                         *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    David P. Bernath / J.P.L.                                       *
 *$Version_and_Date                                                   *
 *    1.0   December 4, 1991                                          *
 *$Change_History                                                     *
 *    DPB   12-04-91   New routine.                                   *
 **********************************************************************/

char *lu_keyword_value (object_ptr, keyword_name, keyword_position, specific)

AGGREGATE object_ptr;
char *keyword_name;
int keyword_position;
LOGICAL specific;

{
    PARAMETER keyword_ptr = {NULL};
    char *value_string = {NULL};
    char *c = {NULL};
    char test_name [PDS_MAXLINE];
    int current_position = {0};
    LOGICAL found = {FALSE};

/** BEGIN **/

    /*-----------------------------------------------------------------------*/
    /** LOOP through the keywords of the object until the particular one    **/
    /**         is found.                                                   **/
    /*-----------------------------------------------------------------------*/

    for (keyword_ptr = FirstParameter (object_ptr);
            ((keyword_ptr != NULL) && (! found));
                keyword_ptr = NextParameter (keyword_ptr))
    {
        /*-------------------------------------------------------------------*/
        /** IF we are not looking for a specific keyword, but any keyword   **/
        /**         which is in the class of keywords passed in, THEN       **/
        /**     Extract the class name to be used instead of the keyword    **/
        /**         in subsequent tests.                                    **/
        /** ENDIF                                                           **/
        /*-------------------------------------------------------------------*/

        strcpy (test_name, keyword_ptr -> name);
        if (! specific)
        {
            for (c = String_End(test_name); 
                ((c >= test_name) && ((*c == '_') || (isdigit(*c)))); --c) ;
            *(c + 1) = EOS;
            for ( ; ((c >= test_name) && (*c != '_')); --c) ;
            ++c;
            strcpy (test_name, c);
        }

        /*-------------------------------------------------------------------*/
        /** IF the current keyword, or its class, matches the one passed in **/
        /**     AND the keyword's position matches the one passed in THEN   **/
        /**     Extract the keyword's value from the label.                 **/
        /** ENDIF                                                           **/
        /*-------------------------------------------------------------------*/

        if (strcmp (test_name, keyword_name) == 0)
        {
            if ((++current_position) >= keyword_position)
            {
                found = TRUE;
                value_string = lu_fetch_value(FirstValue(keyword_ptr), FALSE);
            }
        }

    /*-----------------------------------------------------------------------*/
    /** ENDLOOP                                                             **/
    /*-----------------------------------------------------------------------*/

    }  /*  End:  "for (keyword_ptr = ..."  */

    /*-----------------------------------------------------------------------*/
    /** RETURN the value                                                    **/
    /*-----------------------------------------------------------------------*/

    return (value_string);

/** END **/

}  /*  "lu_keyword_value"  */


/**********************************************************************
 *$Component                                                          *
 *    SFDU_INFO *lu_locate_sfdus (file_ptr)                           *
 *$Abstract                                                           *
 *    Locates the SFDUs in a label file.                              *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    SFDU                                                            *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    file_ptr:                                                       *
 *        The file_ptr variable contains a pointer to a FILE          *
 *        structure.                                                  *
 *$Outputs                                                            *
 *    None                                                            *
 *$Returns                                                            *
 *    sfdu_list:                                                      *
 *        The sfdu_list variable is a pointer to a list of sfdu_info  *
 *        structures. It represents all the SFDUs in one file.        *
 *$Detailed_Description                                               *
 *    The lu_locate_sfdus routine searches the given file for a PDS   *
 *    standard SFDU label structure and stores the SFDU labels, in    *
 *    pairs, on the sfdu_list that is returned. Only the ZI and ZKI   *
 *    structures are understood.  The first two labels, plus the      *
 *    offset of the data following them, are stored in the first      *
 *    structure.  If a second pair of SFDUs exists, it is stored in   *
 *    the second structure, with the offset of the data following.    *
 *    End offsets are also set for each, but will be left as -1 if the*
 *    end of the data could not be found or the delimitation method   *
 *    specified by the SFDU was EOF.                                  *
 *$Error_Handling                                                     *
 *    If the SFDU labels found do not seem to adhere to PDS standards,*
 *    this routine returns NULL and appends a message to the global   *
 *    list.  If an end marker cannot be found, this routine returns   *
 *    only one sfdu_info structure and appends a message to the global*
 *    list.  Note that this routine gives up searching for the end    *
 *    marker after it has searched 25K of data.                       *
 *$Side_Effects                                                       *
 *    This routine causes memory to be allocated which must be freed  *
 *    by the calling routine. This routine will rewind the input file.*
 *$External_References                                                *
 *    Item                      Shared-Data             Access        *
 * ------------------------------------------------------------------ *
 *   pds_verbose                pdsglob.h                read         *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore/ JPL                                            *
 *$Version_and_Date                                                   *
 *    1.0   June 1, 1992                                              *
 *$Change_History                                                     *
 *    MDD   06-01-92   Original code.                                 *
 *	  MDC	03-04-03   Added an extra check to see if the pds_warning *
 *					   flag is TRUE or FALSE before appending a       *
 *					   warning message onto the pds_message_list.     *
 *					   This check is made wherever a warning msg. is  *
 *					   about to be appended.						  *
 **********************************************************************/


SFDU_INFO *lu_locate_sfdus (file_ptr)

FILE *file_ptr;

{
   char buffer [PDS_SFDU_BUFSIZE + 1];
   char back_buffer [PDS_SFDU_BUFSIZE * 2 + 1];
   char *temp;
   char *end_marker = NULL;
   long chars;
   char *end_sfdu;
   char sfdu_marker1 [PDS_SFDU_LEN + 1];
   char sfdu_marker2 [PDS_SFDU_LEN + 1];
   char *second_sfdu;
   long buffer_count = 0;
   long begin_offset = 0;
   long end_offset = 0;
   char *top_sfdu = NULL;
   char *bottom_sfdu = NULL;
   LOGICAL has_sfdus = FALSE;
   int chars_left;
   LOGICAL missing_sfdu = FALSE;
   LOGICAL unknown_sfdu = FALSE;
   char *save = NULL;
   SFDU_INFO *sfdu_info = NULL;
   SFDU_INFO *next_sfdu = NULL;
   long next_offset = 0;
   char second_sfdu_ver;
   char second_sfdu_delim;
   char err_msg [PDS_MAXLINE + 1];

/** BEGIN **/

   /*---------------------------------------------------------------------*/
   /** IF the input file is okay THEN                                    **/
   /*---------------------------------------------------------------------*/

   if (file_ptr != NULL)
   {
      /*------------------------------------------------------------------*/
      /** read the first line from the file                              **/
      /** note how many characters have been read                        **/
      /** IF the first SFDU starts with CCSD THEN                        **/
      /*------------------------------------------------------------------*/

      rewind (file_ptr);
      chars = fread (buffer, 1, PDS_SFDU_BUFSIZE, file_ptr);
      end_offset = chars;
      buffer [chars] = EOS;

      if (strncmp (&buffer [PDS_SFDU_CAID_START], "CCSD", PDS_SFDU_CAID_LEN) == 0)
      {

	 has_sfdus = TRUE;

	 /*------------------------------------------------------------*/
	 /** check the sanity of the first sfdu                       **/
	 /*------------------------------------------------------------*/
	 /* 03-04-03 MDC */
	 if(pds_warning)
	 {
		if (buffer [PDS_SFDU_CLASS] != 'Z')
		{
			 err_append_message (CONTINUE, 
               "WARNING: First SFDU in file does not have class Z.");
			 sprintf (err_msg, 
               "         A `Z` was expected in file position %d.", 
               PDS_SFDU_CLASS + 1);
			 err_append_message (CONTINUE, err_msg);
		}
		else
		{
			if (buffer [PDS_SFDU_VERSION] == '3')
			{
				if (buffer [PDS_SFDU_DELIM] != 'F' && buffer [PDS_SFDU_DELIM] != 'A')
				{
                    err_append_message (CONTINUE, 
                     "WARNING: SFDU Z wrapper has unrecognized delimitation type.");
					sprintf (err_msg, 
                     "         An 'A' or 'F' was expected in file position %d.", 
                     PDS_SFDU_DELIM + 1);
					err_append_message (CONTINUE, err_msg);
				}
			}
			else if (buffer [PDS_SFDU_VERSION] == '1')
			{
				if (buffer [PDS_SFDU_SPARE_1] != '0')
				{
					err_append_message (CONTINUE, 
                      "WARNING: SFDU label for Z wrapper does not have 0 as spare byte 1.");
					sprintf (err_msg, 
                      "         A '0' was expected in file position %d.",
                      PDS_SFDU_SPARE_1 + 1);
					err_append_message (CONTINUE, err_msg);
				}
			} 
			else
			{
				err_append_message (CONTINUE, 
                  "WARNING: SFDU label for Z wrapper has an unrecognized version.");
				sprintf (err_msg, 
                  "         A '1' or '3' was expected in file position %d.",
                  PDS_SFDU_VERSION + 1);
				err_append_message (CONTINUE, err_msg);   
			}
        
			if (buffer [PDS_SFDU_SPARE_2] != '0')
			{
				err_append_message (CONTINUE, 
                   "WARNING: SFDU label for Z wrapper does not have 0 as spare byte 2.");
				sprintf (err_msg, 
                   "          A '0' was expected in file position %d.",
                   PDS_SFDU_SPARE_2 + 1);
				err_append_message (CONTINUE, err_msg);
			}

			if (strncmp (&buffer [PDS_SFDU_DDID_START], "0001", 
                         PDS_SFDU_DDID_LEN) != 0)
			{
				err_append_message (CONTINUE, 
                  "WARNING: SFDU Z wrapper does not have a DDID of 0001");
				sprintf (err_msg, 
                  "         '0001' was expected beginning in file position %d.",
                  PDS_SFDU_DDID_START + 1);
				err_append_message (CONTINUE, err_msg);
			}
		}
	 }
	 /*------------------------------------------------------------*/
	 /** check the sanity of the second sfdu                      **/
	 /*------------------------------------------------------------*/

	 second_sfdu = &buffer [PDS_SFDU_LEN];
	 /* 03-04-03 MDC */
	 if (*((char *) (second_sfdu + PDS_SFDU_CLASS)) != 'K' &&
	     *((char *) (second_sfdu + PDS_SFDU_CLASS))  != 'I' && pds_warning)
     {
	    err_append_message (CONTINUE, 
               "WARNING: Second SFDU has unrecognized class. A catalog SFDU was expected.");
            sprintf (err_msg, 
               "         A 'K' or 'I' was expected in file position %d.",
               PDS_SFDU_LEN + PDS_SFDU_CLASS + 1);
            err_append_message (CONTINUE, err_msg);
	 }

	 if (*((char *) (second_sfdu + PDS_SFDU_VERSION)) == '3')
     {
  	    if (*((char *) (second_sfdu + PDS_SFDU_DELIM)) != 'F' &&
	        *((char *) (second_sfdu + PDS_SFDU_DELIM))  != 'A' &&
                *((char *) (second_sfdu + PDS_SFDU_DELIM)) != 'S')
        {
			/*---------------------------------------------------------------*/
			/* 03-04-03 MDC													 */
			/* Changed the first argument from "CONTINUE" to "ERROR1."		 */
			/*---------------------------------------------------------------*/
            err_append_message (ERROR1, 
                  "ERROR:   Catalog SFDU has unknown delimitation type.");
            sprintf (err_msg, 
                  "         An 'F', 'A', or 'S' was expected in file position %d.",
                  PDS_SFDU_LEN + PDS_SFDU_DELIM + 1);
            err_append_message (CONTINUE, err_msg);
	        unknown_sfdu = TRUE;
	    }
	 }
     else if (*((char *) (second_sfdu + PDS_SFDU_VERSION)) == '1')
     {	 /* 03-04-03 MDC */
         if (*((char *) (second_sfdu + PDS_SFDU_SPARE_1)) != '0' && pds_warning)
         {
             err_append_message (CONTINUE, 
                   "WARNING: SFDU label for catalog data does not have 0 as spare byte 1.");
             sprintf (err_msg, 
                   "         A '0' was expected in file position %d.",
                   PDS_SFDU_LEN + PDS_SFDU_SPARE_1 + 1);
             err_append_message (CONTINUE, err_msg);
	     }
	 }
     else
	 {
		 /* 03-04-03 MDC */
		 if(pds_warning)
		 {
			err_append_message (CONTINUE, 
               "WARNING: SFDU label for catalog data has an unrecognized version.");
			sprintf (err_msg, 
               "         A '1' or '3' was expected in file position %d.",
               PDS_SFDU_LEN + PDS_SFDU_VERSION + 1);
			err_append_message (CONTINUE, err_msg);
		 }
	 }
 
	 if ((*((char *) (second_sfdu + PDS_SFDU_SPARE_2)) != '0') && pds_warning)
     {
         err_append_message (CONTINUE, 
                "WARNING: SFDU label for catalog data does not have 0 as spare byte 2.");
         sprintf (err_msg, 
                "         A '0' was expected in file position %d.",
                PDS_SFDU_LEN + PDS_SFDU_SPARE_2 + 1);
         err_append_message (CONTINUE, err_msg);
     }
 
	 /*---------------------------------------------------------------*/
	 /** set a pointer to the (supposed) end of the SFDU string      **/
	 /** IF the second SFDU indicates it is version 3 THEN           **/
	 /*---------------------------------------------------------------*/

	 temp = &buffer [PDS_SFDU_LEN * 2];
	 second_sfdu_ver = *((char *) (second_sfdu + PDS_SFDU_VERSION));
	 if (second_sfdu_ver == '3')
	 {

	    /*------------------------------------------------------------*/
	    /** IF the second SFDU is a start marker THEN                **/
	    /**    save the marker                                       **/
	    /** ELSE                                                     **/
	    /**    set the SFDU end offset to -1, to indicate EOF        **/
	    /** ENDIF                                                    **/
	    /*------------------------------------------------------------*/

	    second_sfdu_delim = *((char *) (second_sfdu + PDS_SFDU_DELIM));
	    if (second_sfdu_delim == 'S')
	    {
	       Malloc_String(end_marker, PDS_SFDU_VALUE_LEN + 1);
	       strncpy (end_marker, (char *)(second_sfdu + PDS_SFDU_VALUE_START), 
                        PDS_SFDU_VALUE_LEN);
	       *((char *) (end_marker + PDS_SFDU_VALUE_LEN)) = EOS;
	    }
            else
	       end_offset = -1;
	  }
	 /*---------------------------------------------------------------*/
	 /** ELSE IF the second SFDU indicates it is version 1 THEN      **/
	 /**    set SFDU end offset to -1, indicating EOF                **/
	 /*---------------------------------------------------------------*/

	 else if (second_sfdu_ver == '1')
	    end_offset = -1;

	 /*---------------------------------------------------------------*/
	 /** ENDIF                                                       **/
	 /*---------------------------------------------------------------*/

	 }

      /*------------------------------------------------------------------*/
      /** ELSE IF the first SFDU starts with NJPL THEN                   **/
      /**    set a pointer to the (supposed) end of the SFDU string      **/
      /**    set the end offset to -1, indicating EOF                    **/
      /*------------------------------------------------------------------*/

      else if (strncmp (&buffer [PDS_SFDU_CAID_START], "NJPL", 
                        PDS_SFDU_CAID_LEN) == 0)
      {
	 has_sfdus = TRUE;
	 temp = &buffer [PDS_SFDU_LEN];
	 end_offset = -1;
       /*  if (pds_verbose) */
		   if (pds_verbose && pds_warning)
            err_append_message (CONTINUE, "WARNING: SFDU label is out of date");
      }

      /*------------------------------------------------------------------*/
      /** ENDIF                                                          **/
      /** IF there were SFDUs THEN                                       **/
      /*------------------------------------------------------------------*/

      if (has_sfdus)
      {
	  /*--------------------------------------------------------------*/
	  /** locate the actual end of the SFDU string, and put a string **/
	  /**    terminator there                                        **/
	  /*--------------------------------------------------------------*/

	  save = temp;
	  while (*temp && (*temp == ' ' || *temp == '\t')) temp++;
	  if (*temp == '=')
	  {
	     temp++;
	     while (*temp && (*temp == ' ' || *temp == '\t')) temp++;
	     if (strncmp (temp, "PDS_SFDU_LABEL", 14) == 0)
		 temp += 14;
	     else if (strncmp (temp, "SFDU_LABEL", 10) == 0)
		 temp += 10;
	     else if (strncmp (temp, "SFDU", 4) == 0)
		 temp += 4;
	     else
		temp = save;
	     *temp = EOS;
	  }
	  else
	  {
	     temp = save;
	     *temp = EOS;
	  }

	  /*--------------------------------------------------------------*/
	  /** save the whole top SFDU string                             **/
	  /** set the SFDU begin offset to the byte after the SFDU string**/
	  /*--------------------------------------------------------------*/

	  Malloc_String(top_sfdu, String_Size(buffer));
	  strcpy (top_sfdu, buffer);
	  begin_offset = strlen (top_sfdu);

	  /*--------------------------------------------------------------*/
	  /** IF there was a marker SFDU THEN                            **/
	  /*--------------------------------------------------------------*/

	  if (end_marker != NULL)
	  {
	     /*-----------------------------------------------------------*/
	     /** prepare 2 marker strings to use for comparisons         **/
	     /** copy remaining data from first read to the overlap buffer**/
	     /** WHILE we are not at end of file and the issue is not    **/
	     /**    becoming ridiculous DO                               **/
	     /*-----------------------------------------------------------*/

	     sprintf (sfdu_marker1, "CCSD3RE00000%s", end_marker);
	     sprintf (sfdu_marker2, "CCSD$$MARKER%s", end_marker);
             Lemme_Go(end_marker);
	     if (temp != String_End(buffer))
		strcpy (back_buffer, (char *) (temp + 1));
	     else
		strcpy (back_buffer, "");
	     do
	     {

		/*--------------------------------------------------------*/
		/** read the next buffer                                 **/
		/** add to the SFDU end offset                           **/
		/** search the buffer for the start marker               **/
		/** IF it was found THEN                                 **/
		/**   reset the SFDU end offset to the byte before the   **/
		/*       marker                                          **/
		/** ELSE                                                 **/
		/**   save 20 chars for overlap with next buffer         **/
		/** ENDIF                                                **/
		/*--------------------------------------------------------*/

		chars = fread (buffer, 1, PDS_SFDU_BUFSIZE, file_ptr);
		end_offset = end_offset + chars;
		buffer[chars] = EOS;
		strcat (back_buffer, buffer);
		if ((end_sfdu = util_locate_substring (back_buffer, sfdu_marker1)) 
                        != NULL ||
		           (end_sfdu = util_locate_substring (back_buffer, 
                              sfdu_marker2)) != NULL)
		{
		   end_offset = (end_offset - strlen (end_sfdu)) - 1;
		   break;
		}
		if (chars >= PDS_SFDU_LEN)
		   strncpy (back_buffer, &buffer[strlen (buffer) - PDS_SFDU_LEN], 
                            PDS_SFDU_LEN);
		back_buffer [PDS_SFDU_LEN] = EOS;
		buffer_count++;
	     }
	     while (!feof (file_ptr) && buffer_count < PDS_SFDU_MAXBUFFERS);

	     /*-----------------------------------------------------------*/
	     /** ENDWHILE                                                **/
	     /** IF there was an end marker THEN                         **/
	     /*-----------------------------------------------------------*/

	     if (end_sfdu != NULL)
	     {
			/*--------------------------------------------------------*/
			/** IF there are more chars after the end marker THEN    **/
			/**   read another buffer from the file                  **/
			/**   concatenate it to the previous buffer              **/
			/**   determine how many characters are left to save     **/
			/** ENDIF                                                **/
			/*--------------------------------------------------------*/

			chars_left = strlen (end_sfdu);
			if (chars_left < 2 * PDS_SFDU_LEN && !feof(file_ptr))
			{
				strcpy (back_buffer, end_sfdu);
				fread (buffer, 1, PDS_SFDU_BUFSIZE, file_ptr);
				strcat (back_buffer, buffer);
				end_sfdu = back_buffer;
				chars_left = strlen (back_buffer);
			}
			/*--------------------------------------------------------*/
			/** determine how many characters are left to save as the**/
			/**   bottom SFDU                                        **/
			/** check the sanity of the second SFDU                  **/
			/** save the complete bottom SFDU string                 **/
			/** save the byte offset of the next SFDU                **/
			/*--------------------------------------------------------*/

			chars_left = (chars_left >= 2 * PDS_SFDU_LEN) ?
			     2 * PDS_SFDU_LEN : PDS_SFDU_LEN;
			
			/* 03-04-03 MDC */
			if ( (chars_left == 2 * PDS_SFDU_LEN) && pds_warning)
			{
				second_sfdu = (char *) (end_sfdu + PDS_SFDU_LEN);
				if (*((char *) (second_sfdu + PDS_SFDU_CLASS)) != 'I')
				{
					err_append_message (CONTINUE, 
                          "WARNING: SFDU following end marker is not data class.");
                    sprintf (err_msg, 
                          "         An 'I' was expected %d bytes after the catalog end marker.",
                           PDS_SFDU_CLASS + 1);
                    err_append_message (CONTINUE, err_msg);
				}
				if (*((char *) (second_sfdu + PDS_SFDU_VERSION)) != '3')
				{
					err_append_message (CONTINUE, 
                          "WARNING: Data SFDU is not version 3.");
                    sprintf (err_msg, 
                         "          A '3' was expected %d bytes after the catalog end marker.",
                          PDS_SFDU_VERSION + 2);
                    err_append_message (CONTINUE, err_msg);
				}
				if (*((char *) (second_sfdu + PDS_SFDU_DELIM)) != 'F' &&
							*((char *) (second_sfdu + PDS_SFDU_DELIM)) != 'A')
				{
					 err_append_message (CONTINUE, 
                          "WARNING: Data SFDU has unknown delimitation type.");
                     sprintf (err_msg, 
                          "         An 'A' or 'F' was expected %d bytes after the catalog end marker.",
                          PDS_SFDU_DELIM + 1);
                     err_append_message (CONTINUE, err_msg);
				}
				if (*((char *) (second_sfdu + PDS_SFDU_SPARE_2)) != '0')
				{
					err_append_message (CONTINUE, 
                          "WARNING: SFDU label for data does not have 0 as spare byte 2.");
                    sprintf (err_msg, 
                          "         A '0' was expected %d bytes after the catalog end marker.",
                          PDS_SFDU_SPARE_2 + 1);
                    err_append_message (CONTINUE, err_msg);
				}
			}
			Malloc_String (bottom_sfdu, chars_left + 1);
			strncpy (bottom_sfdu, end_sfdu, chars_left);
			bottom_sfdu[chars_left] = EOS;
			next_offset = end_offset + chars_left;
		 }

	     /*-----------------------------------------------------------*/
	     /** ELSE                                                    **/
	     /**    set the missing end marker flag                      **/
	     /*-----------------------------------------------------------*/

	     else
	     {
			end_offset = -1;
			missing_sfdu = TRUE;
	     }
	     /*-----------------------------------------------------------*/
	     /** ENDIF                                                   **/
	     /*-----------------------------------------------------------*/
	  }

	  /*--------------------------------------------------------------*/
	  /** ENDIF there was a marker...                                **/
	  /** allocate SFDU structure and store the info for the SFDUs   **/
	  /*--------------------------------------------------------------*/

	  if (!unknown_sfdu)
	  {
	     sfdu_info = (SFDU_INFO *) malloc (sizeof(SFDU_INFO));
	     Check_Malloc(sfdu_info);
	     sfdu_info -> begin_offset = begin_offset;
	     sfdu_info -> end_offset = end_offset;
	     sfdu_info -> sfdu_label = top_sfdu;
	     sfdu_info -> next = NULL;
	     if (bottom_sfdu != NULL)
	     {
			next_sfdu = sfdu_info -> next = 
                            (SFDU_INFO *) malloc (sizeof(SFDU_INFO));
			Check_Malloc(next_sfdu);
			next_sfdu -> sfdu_label = bottom_sfdu;
			next_sfdu -> next = NULL;
			next_sfdu -> end_offset = -1;
			next_sfdu -> begin_offset = next_offset;
	     }
	  }
	}
      /*------------------------------------------------------------------*/
      /** ELSE                                                           **/
      /**    warn the user there were no SFDUs in the file               **/
      /**																 **/
	  /** 11-22-02 MDC													 **/
	  /** Commented out recording this type of an error message since    **/
	  /** PDS no longer supports SFDUs in their labels anymore. In other **/
	  /** words, if an SFDU is missing from the label, just ignore it.   **/
	  /*------------------------------------------------------------------*/

/*
      else if (pds_verbose)
      {
	  err_append_message (CONTINUE, 
             "INFO:    No SFDU labels were found in this file.");
      }
*/
      /*------------------------------------------------------------------*/
      /** ENDIF                                                          **/
      /** warn the user if there were missing or unknown SFDUs           **/
	  /**																 **/
	  /** 02-13-03 MDC													 **/
	  /** Changed the call to err_append_message() to pass in "WARNING"	 **/
	  /** instead of "CONTINUE" since the first parameter really 		 **/
	  /** specifies what type of a message it is.						 **/
	  /*------------------------------------------------------------------*/

 /*   if (missing_sfdu) */
	  if (missing_sfdu && pds_warning)
      {
/*	  err_append_message (CONTINUE,
	    "WARNING: Unable to locate catalog end marker in file.");
          sprintf (err_msg, 
            "         %s or %s was expected after the PDS Label.", 
            sfdu_marker2, sfdu_marker1);
          err_append_message (CONTINUE, err_msg); */
		  err_append_message (WARNING,
			  "WARNING: Unable to locate catalog end marker in file.");
		  sprintf (err_msg, 
		    "        %s or %s was expected after the PDS label.",
			sfdu_marker2, sfdu_marker1);
		  err_append_message (WARNING, err_msg);
      }
      if (unknown_sfdu)
	  {
/*	  err_append_message (CONTINUE,
	  "ERROR:   The SFDU structure of this file is not recognized."); */
		  err_append_message (ERROR1, 
			  "ERROR:   The SFDU structure of this file is not recognized.");
	  }

      rewind (file_ptr);
   }
   /*---------------------------------------------------------------------*/
   /** ENDIF the input file is okay...                                   **/
   /*---------------------------------------------------------------------*/

   return (sfdu_info);

/** END **/
}


/**********************************************************************
 *$Component                                                          *
 *    VALUE lu_new_value (parameter_ptr, item_ptr)                    *
 *$Abstract                                                           *
 *    Creates a new parameter value on an ODL tree.                   *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    VALUE                                                           *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    parameter_ptr:                                                  *
 *        The parameter_ptr variable is a pointer to the structure    *
 *        used to represent a parameter in a PDS label.               *
 *    item_ptr:                                                       *
 *        The item_ptr variable is a pointer to a VALUE_DATA          *
 *        structure, which represents the value of a keyword in a PDS *
 *        label.                                                      *
 *$Outputs                                                            *
 *    None                                                            *
 *$Returns                                                            *
 *    new_value_ptr:                                                  *
 *        The new_value_ptr variable is a pointer to the structure    *
 *        used to represent a new value of a parameter in a PDS label.*
 *$Detailed_Description                                               *
 *    The lu_new_value routine creates a value node out of the item   *
 *    node passed it and appends it as a new value of the given       *
 *    parameter.  This procedure is identical to the ODLC routine     *
 *    new_value, except that it allocates space for string values.    *
 *$Side_Effects                                                       *
 *    This routine causes memory to be allocated which must be freed  *
 *    by the calling routine.                                         *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore                                                 *
 *$Version_and_Date                                                   *
 *    1.2   October 21, 1991                                          *
 *$Change_History                                                     *
 *    MDD   06-04-91   Original code.                                 *
 *    KLM   06-17-91   Moved from label.c to labutil.c and changed    *
 *                     prefix from lab_ to lu_.                       *
 *    MDD   10-21-91   Fixed malloc, free, and sys_exit_system calls  *
 **********************************************************************/

VALUE lu_new_value (parameter_ptr, item_ptr)

PARAMETER   parameter_ptr;
VALUE_DATA *item_ptr;
{
  VALUE value;
  char *temp = NULL;

/** BEGIN **/

  /*---------------------------------------------------------------------*/
  /** IF the item passed in is NULL THEN                                **/
  /**    return NULL                                                    **/
  /** ENDIF                                                             **/
  /*---------------------------------------------------------------------*/

  if (item_ptr == NULL)
  {
      return (NULL);
  }
  /*---------------------------------------------------------------------*/
  /** allocate memory for the new value                                 **/
  /** copy the simple fields from the input item to the new value       **/
  /*---------------------------------------------------------------------*/

  value = (VALUE) malloc (VALUE_NODE_SIZE);
  Check_Malloc(value);
  value -> item = *item_ptr;

  /*---------------------------------------------------------------------*/
  /** IF the value is a text string THEN                                **/
  /*---------------------------------------------------------------------*/

  if (value -> item.type == TV_STRING || value -> item.type == TV_SYMBOL)
  {
      /*-----------------------------------------------------------------*/
      /** allocate memory for the string in the new value structure     **/
      /** copy the string to the new value structure                    **/
      /*-----------------------------------------------------------------*/

      if (item_ptr -> value.string != NULL)
      {
         Malloc_String(temp, String_Size(item_ptr -> value.string));
         value -> item.value.string = temp;
         strcpy (temp, item_ptr -> value.string);
      }
  }
  /*---------------------------------------------------------------------*/
  /** ENDIF                                                             **/
  /** IF there is a parameter to own this value THEN                    **/
  /**    paste the parameter into it                                    **/
  /*---------------------------------------------------------------------*/

  if (parameter_ptr != NULL)
  {
      PasteValue (parameter_ptr, value);
  }
  /*---------------------------------------------------------------------*/
  /** ELSE                                                              **/
  /**    set the owner and sibling pointers of the value to NULL        **/
  /*---------------------------------------------------------------------*/

  else
  {
      value->parameter     = NULL;
      value->left_sibling  = NULL;
      value->right_sibling = NULL;
  }
  /*---------------------------------------------------------------------*/
  /** ENDIF                                                             **/
  /*---------------------------------------------------------------------*/

  return (value);

/** END **/

} /* End lu_new_value */



/**********************************************************************
 *$Component                                                          *
 *    LOGICAL lu_parse_string (label_ptr, odl_string)                 *
 *$Abstract                                                           *
 *    Parses any ODL string                                           *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    label_ptr:                                                      *
 *        The label_ptr variable is a pointer to the root of an       *
 *        ODL tree.  Usually it points to the actual "ROOT" object,   *
 *        but it may point to any other object in the tree.  If it    *
 *        does point to something other than the "ROOT" object, then  *
 *        this object will be treated as the root of a sub-tree, and  *
 *        processing will not be allowed to move above the level of   *
 *        this object.                                                *
 *    odl_string:                                                     *
 *        The odl_string variable is a character string containing a  *
 *        syntactically correct, complete ODL statement.              *
 *$Outputs                                                            *
 *    None                                                            *
 *$Returns                                                            *
 *    success_flag:                                                   *
 *        The success_flag variable is a general purpose variable     *
 *        which indicates success (a value of TRUE) or failure        *
 *        (a value of FALSE).                                         *
 *$Detailed_Description                                               *
 *    The lu_parse_string routine will send an ODL string to the      *
 *    ODL parser in order to add it to the ODL tree. The ODL string   *
 *    passed in must contain complete ODL statements.  The objects    *
 *    and attributes in the string will be added as children of the   *
 *    object passed in as label_ptr. If any errors occur when parsing *
 *    the string, this routine returns FALSE.                         *
 *$Side_Effects                                                       *
 *    This routine modifies global variables used by the label        *
 *    parser.                                                         *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore/JPL                                             *
 *$Version_and_Date                                                   *
 *    2.1   August 17, 1992                                           *
 *$Change_History                                                     *
 *    MDD   05-15-91   Original Code.                                 *
 *    KLM   06-17-91   Moved from label.c to labutil.c and changed    *
 *                     prefix from lab_ to lu_.                       *
 *    MDD   03-23-92   Removed unused inputs save_ and clear_messages *
 *    MDD   08-17-92   Fixed that error about the missing EOF that    *
 *                     should have prevented this routine from *ever* *
 *                     working.                                       *
 **********************************************************************/

LOGICAL lu_parse_string (label_ptr, odl_string)

AGGREGATE label_ptr;
char *odl_string;
{
   LOGICAL success = FALSE;
   char end_str [10];

/* BEGIN */
   /*----------------------------------------------------------------*/
   /** set parser global variables                                  **/
   /*----------------------------------------------------------------*/

   yyin = NULL;
   yyout = NULL;
   yyprevious = YYNEWLINE;
   yysptr = yysbuf;
   yylineno = 1;
   ODLerror_count = 0;
   ODLwarning_count = 0;

   /*----------------------------------------------------------------*/
   /** set the parent object for the statements to be parsed        **/
   /*----------------------------------------------------------------*/

   ODLroot_node = label_ptr;
   ODLcurrent_aggregate = label_ptr;

   /*----------------------------------------------------------------*/
   /** place the ODL string into the parser's input buffer          **/
   /*----------------------------------------------------------------*/
   sprintf (end_str, "\nEND\n%c", EOF);
   unput(end_str);
   unput(odl_string);
   yylineno = 1;

   /*----------------------------------------------------------------*/
   /** parse the string                                             **/
   /*----------------------------------------------------------------*/

   yyparse ();
   if (ODLerror_count == 0) success = TRUE;
   return (success);

/** END lu_parse_string **/

}  /*  "lu_parse_string"  */

/**********************************************************************
 *$Component                                                          *
 *    PARAMETER lu_paste_parm_at_pos (object_ptr, parameter_ptr,      *
 *                                    new_position)                   *
 *$Abstract                                                           *
 *    Pastes a parameter at a given position in a label               *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    PASTE                                                           *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    object_ptr:                                                     *
 *        The object_ptr variable is a pointer to the structure       *
 *        used to represent an object in a PDS label.                 *
 *    parameter_ptr:                                                  *
 *        The parameter_ptr variable is a pointer to the structure    *
 *        used to represent a parameter in a PDS label.               *
 *    new_position:                                                   *
 *        The new_position variable is the same as either             *
 *        object_position or parameter_position, depending upon       *
 *        context, and is used when either an object or parameter is  *
 *        to be moved to a new location within the parent object      *
 *        an a PDS label.                                             *
 *$Outputs                                                            *
 *    None                                                            *
 *$Returns                                                            *
 *    parameter_ptr:                                                  *
 *        The parameter_ptr variable is a pointer to the structure    *
 *        used to represent a parameter in a PDS label.               *
 *$Detailed_Description                                               *
 *    The lu_paste_parm_at_pos routine pastes the given parameter     *
 *    into the given parent object at position new_position. If       *
 *    new_position is greater than than the number of parameters in   *
 *    the object, the parameter is pasted at the end of the object.   *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore/ J.P.L                                          *
 *$Version_and_Date                                                   *
 *    1.0   November 4, 1991                                          *
 *$Change_History                                                     *
 *    MDD   11-04-91   Original code.                                 *
 **********************************************************************/


PARAMETER lu_paste_parm_at_pos (object_ptr, parameter_ptr, new_position)

AGGREGATE object_ptr;
PARAMETER parameter_ptr;
int new_position;

{
   int pos = 1;
   PARAMETER left_ptr = NULL;
   PARAMETER right_ptr;

/** BEGIN **/
  
   /*------------------------------------------------------------------*/
   /** LOOP through the parameters of the given object and find the   **/
   /**   right and left siblings of the new parameter                 **/
   /*------------------------------------------------------------------*/

   for (right_ptr = object_ptr -> first_parameter; 
           pos < new_position && right_ptr != NULL; 
              right_ptr = right_ptr -> right_sibling, pos++) 
       left_ptr = right_ptr;


   /*------------------------------------------------------------------*/
   /** set the right and left siblings                                **/
   /*------------------------------------------------------------------*/

   parameter_ptr -> right_sibling = right_ptr;
   parameter_ptr -> left_sibling = left_ptr;
   parameter_ptr -> owner = object_ptr;

   /*------------------------------------------------------------------*/
   /** reset the pointers in the parent object and the left and right **/
   /**    siblings to point to the new parameter, as necessary        **/
   /*------------------------------------------------------------------*/

   if (right_ptr == NULL)
      object_ptr -> last_parameter = parameter_ptr;
   else
      right_ptr -> left_sibling = parameter_ptr;
   if (left_ptr == NULL)
      object_ptr -> first_parameter = parameter_ptr;
   else
      left_ptr -> right_sibling = parameter_ptr;
   return (parameter_ptr);

/** END **/
}


/**********************************************************************
 *$Component                                                          *
 *    LOGICAL lu_write_zi1_label (label_ptr, top_ddid,                *
 *                                 new_record_type, record_length,    *
 *                                 file_name)                         *
 *$Abstract                                                           *
 *    Writes a PDS label with a ZI version 1 SFDU labelling scheme    *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    WRITE                                                           *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    label_ptr:                                                      *
 *        The label_ptr variable is a pointer to the root of an       *
 *        ODL tree.  Usually it points to the actual "ROOT" object,   *
 *        but it may point to any other object in the tree.  If it    *
 *        does point to something other than the "ROOT" object, then  *
 *        this object will be treated as the root of a sub-tree, and  *
 *        processing will not be allowed to move above the level of   *
 *        this object.                                                *
 *    top_ddid:                                                       *
 *        The top_ddid variable is a pointer to a character           *
 *        string   that holds the data description ID (DDID) of an    *
 *        SFDU label. This variable should be four bytes long.        *
 *    new_record_type:                                                *
 *        The new_record_type variable is an integer that represents  *
 *        the type of records a file contains: e.g., PDS_RF_STREAM_LF,*
 *        PDS_RF_FIXED_CRLF, etc.                                     *
 *    record_length:                                                  *
 *        The record_length variable is an integer that holds the     *
 *        record length value to be used for a fixed file.            *
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
 *   The lu_write_zi1_label routine writes a PDS label to the chosen  *
 *   output file and converts it to the specified record type. The    *
 *   file is written in the form:                                     *
 *                                                                    *
 *                Z label with byte count/I label with byte count/    *
 *                                                                    *
 *                   ODL objects                                      *
 *                                                                    *
 *                EOF                                                 *
 *                                                                    *
 *   If the file cannot be written or converted, then this routine    *
 *   returns FALSE. If the top_ddid is PDS0 or PDS1, then             *
 *   "= SFDU_LABEL" will be added at the end of the first SFDU.       *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore/JPL                                             *
 *$Version_and_Date                                                   *
 *    1.3   March 17, 1992                                            *
 *$Change_History                                                     *
 *    MDD   05-13-91   Original Code.                                 *
 *    KLM   06-17-91   Moved from label.c to labutil.c and changed    *
 *                     prefix from lab_ to lu_.                       *
 *    DPB   09-24-91   Changed call to fio_convert_file.              *
 *    MDD   03-17-92   The great int -> long conversion               *
 **********************************************************************/


LOGICAL lu_write_zi1_label (label_ptr, top_ddid, new_record_type,
                            record_length, file_name)

AGGREGATE label_ptr;
char *top_ddid;
int new_record_type;
long record_length;
char *file_name;
{
   LOGICAL success = FALSE;
   FILE *file_ptr;
   long z_byte_count = 0;


/** BEGIN **/
   /*-------------------------------------------------------------------*/
   /** strip blanks from inputs and convert to upper case, if needed   **/
   /*-------------------------------------------------------------------*/

   util_strip_lead_and_trail (top_ddid, ' ');
   util_upper_case (top_ddid);
   util_strip_lead_and_trail (file_name, ' ');

   /*-------------------------------------------------------------------*/
   /** IF all the required inputs have values THEN                     **/
   /*-------------------------------------------------------------------*/

   if (top_ddid != NULL && file_name != NULL && 
          label_ptr != NULL && strlen (top_ddid) == PDS_SFDU_DDID_LEN)
   {
      /*----------------------------------------------------------------*/
      /** IF the output file could not be opened THEN                  **/
      /**    print an error                                            **/
      /*----------------------------------------------------------------*/

      file_ptr = fopen (file_name, "w");
      if (file_ptr == NULL)
      {
          err_append_message (ERROR1,
             "Could not open output file in order to write label");
      }
      /*----------------------------------------------------------------*/
      /** ELSE                                                         **/  
      /*----------------------------------------------------------------*/

      else
      {
         /*-------------------------------------------------------------*/
         /** write the top SFDU                                        **/
         /** write the ODL label                                       **/
         /** convert the file to the chosen record format              **/
         /** determine the size of the file                            **/
         /*-------------------------------------------------------------*/

         if (strcmp (top_ddid, "PDS0") == 0 || strcmp (top_ddid, "PDS1") == 0)
            fprintf (file_ptr, 
                       "CCSD1Z00000100000000NJPL1IF0%s00000000 = SFDU_LABEL\n", 
                          top_ddid);
         else
            fprintf (file_ptr, "CCSD1Z00000100000000NJPL1IF0%s00000000\n", 
                       top_ddid);
         WriteLabel (file_ptr, label_ptr);
         fclose (file_ptr);
         success = fio_convert_file (file_name, DEFAULT_REC_TYPE,
                                     new_record_type, 0, record_length); 
         z_byte_count = fio_size (file_name);

         /*-------------------------------------------------------------*/
         /** IF the file can be reopened and its size was found THEN   **/
         /*-------------------------------------------------------------*/

         if (success && z_byte_count != -1 && 
                (file_ptr = fopen (file_name, "r+")) != NULL)
         {
            fprintf (file_ptr, "CCSD1Z000001%08ldNJPL1IF0%s%08ld", 
                        z_byte_count, top_ddid, z_byte_count - (long) 20);
            fclose (file_ptr);
         }
         /*-------------------------------------------------------------*/
         /** ELSE                                                      **/
         /*-------------------------------------------------------------*/

         else
         {
            err_append_message (WARNING, 
                "The byte count of the output file could not be determined");
            err_append_message (WARNING, 
                "The ZI SFDU label will contain zeros in the byte count fields");
        }
         /*-------------------------------------------------------------*/
         /** ENDIF                                                     **/
         /*-------------------------------------------------------------*/
      }
      /*----------------------------------------------------------------*/
      /** ENDIF                                                        **/  
      /*----------------------------------------------------------------*/
   }
   /*-------------------------------------------------------------------*/
   /** ELSE                                                            **/
   /*-------------------------------------------------------------------*/

   else
   {
      err_append_message (ERROR1,
         "A file with ZI version 1 SFDU labels could not be written");
      err_append_message (CONTINUE, "to your specifications"); 
   }
   /*-------------------------------------------------------------------*/
   /** ENDIF                                                           **/
   /*-------------------------------------------------------------------*/

   return (success);

/** END **/
} /* End lu_write_zi1_label */


/**********************************************************************
 *$Component                                                          *
 *    LOGICAL lu_write_zi3_label (label_ptr, top_ddid,                *
 *                                 new_record_type, record_length,    *
 *                                 file_name)                         *
 *$Abstract                                                           *
 *    Writes a PDS label with a ZI version 3 SFDU labelling scheme    *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    WRITE                                                           *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    label_ptr:                                                      *
 *        The label_ptr variable is a pointer to the root of an       *
 *        ODL tree.  Usually it points to the actual "ROOT" object,   *
 *        but it may point to any other object in the tree.  If it    *
 *        does point to something other than the "ROOT" object, then  *
 *        this object will be treated as the root of a sub-tree, and  *
 *        processing will not be allowed to move above the level of   *
 *        this object.                                                *
 *    top_ddid:                                                       *
 *        The top_ddid variable is a pointer to a character           *
 *        string   that holds the data description ID (DDID) of an    *
 *        SFDU label. This variable should be four bytes long.        *
 *    new_record_type:                                                *
 *        The new_record_type variable is an integer that represents  *
 *        the type of records a file contains: e.g., PDS_RF_STREAM_LF,*
 *        PDS_RF_FIXED_CRLF, etc.                                     *
 *    record_length:                                                  *
 *        The record_length variable is an integer that holds the     *
 *        record length value to be used for a fixed file.            *
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
 *   The lu_write_zi3_label routine writes a PDS label to the chosen  *
 *   output file and converts it to the specified record type. The    *
 *   file is written in the form:                                     *
 *                                                                    *
 *                Z label/I label/                                    *
 *                                                                    *
 *                   ODL objects                                      *
 *                                                                    *
 *                EOF                                                 *
 *                                                                    *
 *   If the file cannot be written or converted, then this routine    *
 *   returns FALSE.                                                   *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore/JPL                                             *
 *$Version_and_Date                                                   *
 *    1.3   March 17, 1992                                            *
 *$Change_History                                                     *
 *    MDD   05-13-91   Original Code.                                 *
 *    KLM   06-17-91   Moved from label.c to labutil.c and changed    *
 *                     prefix from lab_ to lu_.                       *
 *    DPB   09-24-91   Changed call to fio_convert_file.              *
 *    MDD   03-17-92   The great int -> long conversion               *
 **********************************************************************/

LOGICAL lu_write_zi3_label (label_ptr, top_ddid, new_record_type,
                            record_length, file_name)

AGGREGATE label_ptr;
char *top_ddid;
int new_record_type;
long record_length;
char *file_name;
{
   LOGICAL success = FALSE;
   FILE *file_ptr;


/** BEGIN **/
   /*-------------------------------------------------------------------*/
   /** strip blanks from inputs and convert to upper case, if needed   **/
   /*-------------------------------------------------------------------*/

   util_strip_lead_and_trail (top_ddid, ' ');
   util_upper_case (top_ddid);
   util_strip_lead_and_trail (file_name, ' ');

   /*-------------------------------------------------------------------*/
   /** IF all the required inputs have values THEN                     **/
   /*-------------------------------------------------------------------*/

   if (top_ddid != NULL && file_name != NULL && 
          label_ptr != NULL && strlen (top_ddid) == PDS_SFDU_DDID_LEN)
   {
      /*----------------------------------------------------------------*/
      /** IF the output file could not be opened THEN                  **/
      /**    print an error                                            **/
      /*----------------------------------------------------------------*/

      file_ptr = fopen (file_name, "w");
      if (file_ptr == NULL)
      {
          err_append_message (ERROR1,
             "Could not open output file in order to write label");
      }

      /*----------------------------------------------------------------*/
      /** ELSE                                                         **/
      /**    write the SFDU label to the file                          **/
      /**    write the ODL label to file                               **/
      /**    convert the file to the chosen record type                **/
      /*----------------------------------------------------------------*/

      else
      {
         fprintf (file_ptr,
                    "CCSD3ZF0000100000001NJPL3IF0%s00000001\n",
                            top_ddid);
         WriteLabel (file_ptr, label_ptr);
         fclose (file_ptr);
         success = fio_convert_file (file_name, DEFAULT_REC_TYPE,
                                     new_record_type, 0, record_length); 
      }
      /*----------------------------------------------------------------*/
      /** ENDIF                                                        **/
      /*----------------------------------------------------------------*/

   }
   /*-------------------------------------------------------------------*/
   /** ELSE                                                            **/
   /*-------------------------------------------------------------------*/

   else
   {
      err_append_message (ERROR1,
         "A file with ZI version 3 SFDU labels could not be written");
      err_append_message (CONTINUE, "to your specifications"); 
   }
   /*-------------------------------------------------------------------*/
   /** ENDIF                                                           **/
   /*-------------------------------------------------------------------*/

   return (success);

/** END lu_write_zi3_label **/
}

/**********************************************************************
 *$Component                                                          *
 *    LOGICAL lu_write_zk3_label (label_ptr, top_ddid, bottom_ddid,   *
 *                                 new_record_type, record_length,    *
 *                                 file_name)                         *
 *$Abstract                                                           *
 *    Writes a PDS label with a ZK version 3 SFDU labelling scheme    *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    WRITE                                                           *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    label_ptr:                                                      *
 *        The label_ptr variable is a pointer to the root of an       *
 *        ODL tree.  Usually it points to the actual "ROOT" object,   *
 *        but it may point to any other object in the tree.  If it    *
 *        does point to something other than the "ROOT" object, then  *
 *        this object will be treated as the root of a sub-tree, and  *
 *        processing will not be allowed to move above the level of   *
 *        this object.                                                *
 *    top_ddid:                                                       *
 *        The top_ddid variable is a pointer to a character           *
 *        string   that holds the data description ID (DDID) of an    *
 *        SFDU label. This variable should be four bytes long.        *
 *    bottom_ddid:                                                    *
 *        The bottom_ddid variable is a pointer to a character        *
 *        string   that holds the data description ID (DDID) of an    *
 *        SFDU label. This variable should be four bytes long.        *
 *    new_record_type:                                                *
 *        The new_record_type variable is an integer that represents  *
 *        the type of records a file contains: e.g., PDS_RF_STREAM_LF,*
 *        PDS_RF_FIXED_CRLF, etc.                                     *
 *    record_length:                                                  *
 *        The record_length variable is an integer that holds the     *
 *        record length value to be used for a fixed file.            *
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
 *   The lu_write_zk3_label routine writes a PDS label to the chosen  *
 *   output file and converts it to the specified record type. The    *
 *   file is written in the form:                                     *
 *                                                                    *
 *                Z label/K start marker/                             *
 *                                                                    *
 *                   ODL objects                                      *
 *                                                                    *
 *                K end marker/I label/EOF                            *
 *                                                                    *
 *   If the file cannot be written or converted, then this routine    *
 *   returns FALSE.                                                   *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore/JPL                                             *
 *$Version_and_Date                                                   *
 *    1.5   March 17, 1992                                            *
 *$Change_History                                                     *
 *    MDD   05-13-91   Original Code.                                 *
 *    KLM   06-17-91   Moved from label.c to labutil.c and changed    *
 *                     prefix from lab_ to lu_.                       *
 *    DPB   09-24-91   Changed call to fio_convert_file.              *
 *    MDD   01-13-92   Updated end marker to match new labels         *
 *    MDD   01-24-92   Added use of pds_finish_label to fix fixed     *
 *                     length file bug                                *
 *    MDD   03-17-92   The great int -> long conversion               *
 **********************************************************************/


LOGICAL lu_write_zk3_label (label_ptr, top_ddid, bottom_ddid, 
                             new_record_type, record_length, file_name)
AGGREGATE label_ptr;
char *top_ddid;
char *bottom_ddid;
int new_record_type;
long record_length;
char *file_name;
{
   LOGICAL success = TRUE;
   FILE *file_ptr;
   char blanks [PDS_MAXLINE + 1];
   long i;
   LOGICAL save;


/** BEGIN **/
   /*-------------------------------------------------------------------*/
   /** strip blanks from inputs and convert to upper case, if needed   **/
   /*-------------------------------------------------------------------*/

   util_strip_lead_and_trail (top_ddid, ' ');
   util_upper_case (top_ddid);
   util_strip_lead_and_trail (bottom_ddid, ' ');
   util_upper_case (bottom_ddid);
   util_strip_lead_and_trail (file_name, ' ');
   
   /*-------------------------------------------------------------------*/
   /** IF all the required inputs have values THEN                     **/
   /*-------------------------------------------------------------------*/

   if (top_ddid != NULL && file_name != NULL && 
          label_ptr != NULL && bottom_ddid != NULL && 
             strlen (top_ddid) == PDS_SFDU_DDID_LEN &&
                strlen (bottom_ddid) == PDS_SFDU_DDID_LEN)
   {
      /*----------------------------------------------------------------*/
      /** IF the output file could not be opened THEN                  **/
      /**    print an error                                            **/
      /*----------------------------------------------------------------*/

      file_ptr = fopen (file_name, "w");
      if (file_ptr == NULL)
      {
          err_append_message (ERROR1,
             "Could not open output file in order to write label");
      }
      /*----------------------------------------------------------------*/
      /** ELSE                                                         **/
      /*----------------------------------------------------------------*/

      else
      {
         /*-------------------------------------------------------------*/
         /** write the top (ZK) label to the file                      **/
         /** write the ODL label to the file                           **/
         /*-------------------------------------------------------------*/

         fprintf (file_ptr, "CCSD3ZF0000100000001NJPL3KS0%sAAAAAAAA\n", 
                  top_ddid);

         /*-------------------------------------------------------------*/
         /** IF the file is not supposed to have a fixed format THEN   **/
         /** add the bottom (RI) SFDU label                            **/
         /** convert the file to the chosen record type                **/
         /*-------------------------------------------------------------*/

         if (new_record_type != PDS_RF_FIXED_CR &&
                new_record_type != PDS_RF_FIXED_LF &&
                   new_record_type != PDS_RF_FIXED_CRLF)
         {
            WriteLabel (file_ptr, label_ptr);
            fprintf (file_ptr, "CCSD$$MARKERAAAAAAAANJPL3IF0%s00000001\n", 
                        bottom_ddid);
            fclose (file_ptr);
            success = fio_convert_file (file_name, PDS_RF_STREAM_LF, 
                                        new_record_type, 0, record_length); 
	 }
         /*-------------------------------------------------------------*/
         /** ELSE                                                      **/
         /**  Close the output file and convert it to the chosen type  **/
         /**  open the file again                                      **/
         /**  write a new end statement and the bottom (RI) SFDU label **/
         /**    to the file, inserting blanks in between to pad out to **/
         /**    the required record length                             **/
         /*-------------------------------------------------------------*/

         else
         {
            save = pds_finish_label;
            pds_finish_label = FALSE;
            WriteLabel (file_ptr, label_ptr);
            pds_finish_label = save;
            fclose (file_ptr);
            success = fio_convert_file (file_name, DEFAULT_REC_TYPE,
                                        new_record_type, 0, record_length);  
            if (success && (file_ptr = fopen (file_name, "a+")) != NULL)
            {
               for (i = 0; i < record_length - 43; i++) blanks [i] = ' ';
               blanks [i] = EOS;
               fprintf (file_ptr, "END%sCCSD$$MARKERAAAAAAAANJPL3IF0%s00000001",
                        blanks, bottom_ddid);
               fclose (file_ptr); 
	    }
            else
               success = FALSE;
	 }
         /*-------------------------------------------------------------*/
         /** ENDIF                                                     **/
         /*-------------------------------------------------------------*/
      }
      /*----------------------------------------------------------------*/
      /** ENDIF                                                        **/
      /*----------------------------------------------------------------*/
   }
   /*-------------------------------------------------------------------*/
   /** ELSE                                                            **/
   /**    an input was bad so print an error                           **/
   /*-------------------------------------------------------------------*/

   else
   {
      err_append_message (ERROR1,
         "A file with ZK version 3 SFDU labels could not be written");
      err_append_message (CONTINUE, "to your specifications"); 
   }
   /*-------------------------------------------------------------------*/
   /** ENDIF                                                           **/
   /*-------------------------------------------------------------------*/

   return (success);

}

