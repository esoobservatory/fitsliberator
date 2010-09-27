/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Component                                                           * 
 *    Library label.c                                                  *
 * Abstract                                                            *
 *    Label utility routines.                                          *
 * Detailed_Description                                                *
 *    The lablib is a library of subroutines used by PDS software      *
 *    to perform a variety of operations on PDS labels.                *
 * Internal_References                                                 *
 *    lab_add_object                                                   *
 *    lab_add_parameter                                                *
 *    lab_add_value                                                    *
 *    lab_adjust_pointers                                              *
 *    lab_change_object_class                                          *
 *    lab_change_parameter_name                                        *
 *    lab_change_value                                                 *
 *    lab_clear_messages                                               *
 *    lab_exit                                                         *
 *    lab_find_object                                                  *
 *    lab_find_parameter                                               *
 *    lab_get_all_values                                               *
 *    lab_get_object_class                                             *
 *    lab_get_pointer                                                  *
 *    lab_get_value                                                    *
 *    lab_has_messages                                                 *
 *    lab_match_classes                                                *
 *    lab_move_parameter                                               *
 *    lab_print_messages                                               *
 *    lab_print_label                                                  *
 *    lab_print_pds_label                                              *
 *    lab_read_label                                                   *
 *    lab_read_label_or_template                                       *
 *    lab_remove_label                                                 *
 *    lab_remove_label_or_template                                     *
 *    lab_remove_object                                                *
 *    lab_remove_parameter                                             *
 *    lab_remove_sfdu_labels                                           *
 *    lab_replace_pointers                                             *
 *    lab_setup                                                        *
 *    lab_start_label                                                  *
 *    lab_write_attached_label                                         *
 *    lab_write_label                                                  *
 *    lab_write_label_or_template                                      *
 *    lab_write_product_label                                          *
 * Authors_and_Institutions                                            *
 *    Herbert C. Gamble / J.P.L.                                       *
 *    David P. Bernath / J.P.L.                                        *
 *    Marti D. Demore / J.P.L.                                         *
 * Version and Date                                                    *
 *    7.0   August 28, 1992                                            *
 * Change History                                                      *
 *    HCG   03-08-91   Library created.                                *
 *    DPB   04-10-91   Routines restructured and a whole slew of new   *
 *                     ones added.                                     *
 *    MDD   04-29-91   Added get and format value routines             *
 *    MDD   05-31-91   Added new lab_write routines                    *
 *    DPB   05-30-91   Added lab_fetch_all_values                      *
 *    KLM   06-11-91   Broke up the label.c file into higer level label*
 *                     routines and lower level label utility routines.*
 *    KLM   06-25-91   Added lab_skip_sfdus                            *
 *    DPB   07-25-91   Added lab_match_classes and pds_generic_class.  *
 *    KLM   09-12-91   Made some minor changes to the headers of a few *
 *                     routines. The errors were found while parsing   *
 *                     the files.                                      *
 *    DPB   09-27-91   Removed lab_get_object_class.                   *
 *    MDD   06-01-92   Added SFDU read routines, and deleted lab_skip..*
 *    MDD   06-19-92   Added a slew of new routines to make interface  *
 *                     more consistent.                                *
 *    MDD   08-28-92   Added new lab_write and lab_attach stuff        *
 *    DWS   07-13-00   Commented out lines 4293 and 4567.  They no     *
 *                     longer replace underscores with blanks.         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "pdsdef.h"
#include "label.h"
#include "labutil.h"
#include "errordef.h"
#include "fiodef.h"
#include "utildef.h"
#include "odlinter.h"

extern AGGREGATE ODLroot_node;
extern ERROR_LIST *pds_message_list;
extern ERROR_LIST *pds_last_message;
extern LOGICAL pds_verbose;
extern LOGICAL pds_generic_class;
extern LOGICAL pds_finish_label;
extern LOGICAL pds_use_sfdus;
extern int pds_default_rectype;
extern long pds_default_reclen;
extern char pds_record_term [];
extern long pds_records_written;
extern long pds_records_needed;

/* MDD - I refuse to write a header for a one-liner.  I won't, I won't! */

LOGICAL lab_has_messages () {return (pds_message_list != NULL);}

/*----------------------------------------------------------------------*/
/* The following routines are being phased out. New programs should use */
/* lab_read_label, lab_print_label, lab_remove_label, lab_write_label.  */
/*----------------------------------------------------------------------*/

LOGICAL lab_print_pds_label (label_ptr)

AGGREGATE label_ptr;
{
   int label_status;
   return lab_print_label (label_ptr, &label_status);
}

AGGREGATE lab_read_label_or_template (input_fname)

char *input_fname;
{
   int label_status;
   return lab_read_label (input_fname, &label_status);
}

AGGREGATE lab_remove_label_or_template (label_ptr)
 
AGGREGATE label_ptr;
{
   int label_status;
   return lab_remove_label (label_ptr, &label_status);
}

LOGICAL lab_write_label_or_template (label_ptr, record_type, record_length, file_name)

AGGREGATE label_ptr;
int record_type;
long record_length;
char *file_name;
{
   char save_term [3];
   long save_length = pds_default_reclen;
   int save_type = pds_default_rectype;
   int label_status;

   strcpy (save_term, pds_record_term);
   strcpy (pds_record_term, "\n");
   pds_default_reclen = record_length;
   pds_default_rectype = record_type;
   lab_write_label (label_ptr, file_name, FALSE, 0, &label_status);
   strcpy (pds_record_term, save_term);
   pds_default_reclen = save_length;
   pds_default_rectype = save_type;
   return (label_status == PDS_SUCCESS);
}


/**********************************************************************
 *$Component                                                          *
 *    AGGREGATE lab_add_object (label_ptr, object_class, object_name, *
 *                              object_position, new_object_class,    *
 *                              label_status)                         *
 *$Abstract                                                           *
 *    Adds a new object to a PDS Label.                               *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    ADD                                                             *
 *    OBJECT                                                          *
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
 *    object_class:                                                   *
 *        The object_class variable is a character string             *
 *        which contains the pds_class of an object in a PDS label        *
 *        (e.g., "OBJECT = IMAGE" implies that "IMAGE" is the pds_class   *
 *        of the object).                                             *
 *    object_name:                                                    *
 *        The object_name variable is a character string              *
 *        which contains the name of an object in a PDS label.        *
 *        This is assumed to be the value of the "NAME" parameter     *
 *        in the object (e.g., "NAME = VOYAGER" implies that          *
 *        "VOYAGER" is the name of the object).                       *
 *    object_position:                                                *
 *        The object_position variable is an integer which            *
 *        represents the relative position of an object in a PDS      *
 *        label.  If this variable is used in conjunction with either *
 *        the object_class or object_name variables, then it          *
 *        represents a particular occurrence of that thing in the     *
 *        label (e.g., if object_class is "TABLE" and object_position *
 *        is 3, then this represents the third "TABLE" object in the  *
 *        label).  On the other hand, if this variable is used by     *
 *        itself, it represents the absolute position of the object,  *
 *        starting from the "ROOT" object (position = 1).  The        *
 *        sequence follows the structure of the label as it looks     *
 *        in a flat file.                                             *
 *    new_object_class:                                               *
 *        The new_object_class variable is a character string         *
 *        which contains the pds_class of an object to be added to        *
 *        a PDS label (e.g., "OBJECT = IMAGE" implies that "IMAGE"    *
 *        is the pds_class of the object).                                *
 *$Outputs                                                            *
 *    label_status:                                                   *
 *        The label_status variable is an integer which is used to    *
 *        indicate the success or failure of a PDS label routine,     *
 *        and whether or not the object or parameter specifications   *
 *        passed in were ambiguous.  Valid values are:  PDS_SUCCESS,  *
 *        PDS_ERROR, PDS_MULTIPLE_OBJECTS, PDS_MULTIPLE_PARMS, and    *
 *        PDS_WARNING.                                                *
 *$Returns                                                            *
 *    new_object_ptr:                                                 *
 *        The new_object_ptr variable is a pointer to the structure   *
 *        used to represent a new object in a PDS label.              *
 *$Detailed_Description                                               *
 *    The lab_add_object routine adds a new object to a PDS label.    *
 *    It searches for the parent of the new object, mallocs storage   *
 *    for the new object, then appends the new object onto the list   *
 *    of children of the parent.  The parent object may by specified  *
 *    by pds_class, name, or position, or by any combination of these.    *
 *    The search for the parent begins at the object pointed to by    *
 *    the label_ptr variable.  This is usually the "ROOT" object, but *
 *    may be any other object in the tree.  Please note that it is    *
 *    not necessary to specify the object's name, pds_class, and          *
 *    position at the same time.  In fact, if you specify only one    *
 *    of them and pass in zero for the others, the routine will work  *
 *    just fine.                                                      *
 *$Error_Handling                                                     *
 *    1) If the parent object cannot be found, or memory cannot be    *
 *       allocated for the new object, then the label_status          *
 *       is set to PDS_ERROR and a NULL value is returned.            *
 *    2) If more than one object is found that matches the            *
 *       specifications passed in for the parent object, then the     *
 *       label_status is set to PDS_MULTIPLE_OBJECTS and a NULL       *
 *       value is returned.                                           *
 *    3) If everything went according to plan, then the label_status  *
 *       is set to PDS_SUCCESS and a pointer to the new object is     *
 *       returned.                                                    *
 *$Side_Effects                                                       *
 *    This routine causes memory to be allocated which must be freed  *
 *    by the calling routine.                                         *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Herbert C. Gamble / J.P.L.                                      *
 *    David P. Bernath / J.P.L.                                       *
 *$Version_and_Date                                                   *
 *    1.2   June 17, 1991                                             *
 *$Change_History                                                     *
 *    HCG   03-08-91   Original Code.                                 *
 *    DPB   04-10-91   Restructured routine.                          *
 *    DPB   04-26-91   Added code to strip blanks from inputs.        *
 *    KLM   06-17-91   Changed call of lab_append_object to           *
 *                     lu_append_object.                              *
 **********************************************************************/

AGGREGATE lab_add_object (label_ptr, object_class, object_name, 
                          object_position, new_object_class, label_status)

AGGREGATE label_ptr;
char *object_class;
char *object_name;
int object_position;
char *new_object_class;
int *label_status;

{
    AGGREGATE parent_object_ptr = {label_ptr};
    AGGREGATE new_object_ptr = {NULL};

/** BEGIN **/

    /*-----------------------------------------------------------------------*/
    /** Initialize label_status                                             **/
    /*-----------------------------------------------------------------------*/
 
    *label_status = PDS_ERROR;

    /*-----------------------------------------------------------------------*/
    /** Remove leading and trailing blanks from character inputs.           **/
    /*-----------------------------------------------------------------------*/

    util_strip_lead_and_trail (object_class, ' ');
    util_strip_lead_and_trail (object_name, ' ');
    util_strip_lead_and_trail (new_object_class, ' ');

    /*-----------------------------------------------------------------------*/
    /** Find the parent object and retrieve a pointer to it                 **/
    /*-----------------------------------------------------------------------*/

    parent_object_ptr = lab_find_object (label_ptr, object_class, 
                                         object_name, object_position, 
                                         label_status);

    /*-----------------------------------------------------------------------*/
    /** IF the parent object was found THEN                                 **/
    /**     Append a new child object onto the parent's list of children    **/
    /** ENDIF                                                               **/
    /*-----------------------------------------------------------------------*/

    if ((parent_object_ptr != NULL) && (*label_status == PDS_SUCCESS))
        new_object_ptr = lu_append_object(parent_object_ptr,new_object_class);

    /*-----------------------------------------------------------------------*/
    /** RETURN a pointer to the new object structure                        **/
    /*-----------------------------------------------------------------------*/

    return (new_object_ptr);

/** END **/

}  /*  "lab_add_object"  */ 


/**********************************************************************
 *$Component                                                          *
 *    PARAMETER lab_add_parameter (label_ptr, object_class,           *
 *                                 object_name, object_position,      *
 *                                 parameter_name, parameter_value,   *
 *                                 label_status)                      *
 *$Abstract                                                           *
 *    Adds a new parameter to a PDS Label.                            *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    ADD                                                             *
 *    PARAMETER                                                       *
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
 *    object_class:                                                   *
 *        The object_class variable is a character string             *
 *        which contains the pds_class of an object in a PDS label        *
 *        (e.g., "OBJECT = IMAGE" implies that "IMAGE" is the pds_class   *
 *        of the object).                                             *
 *    object_name:                                                    *
 *        The object_name variable is a character string              *
 *        which contains the name of an object in a PDS label.        *
 *        This is assumed to be the value of the "NAME" parameter     *
 *        in the object (e.g., "NAME = VOYAGER" implies that          *
 *        "VOYAGER" is the name of the object).                       *
 *    object_position:                                                *
 *        The object_position variable is an integer which            *
 *        represents the relative position of an object in a PDS      *
 *        label.  If this variable is used in conjunction with either *
 *        the object_class or object_name variables, then it          *
 *        represents a particular occurrence of that thing in the     *
 *        label (e.g., if object_class is "TABLE" and object_position *
 *        is 3, then this represents the third "TABLE" object in the  *
 *        label).  On the other hand, if this variable is used by     *
 *        itself, it represents the absolute position of the object,  *
 *        starting from the "ROOT" object (position = 1).  The        *
 *        sequence follows the structure of the label as it looks     *
 *        in a flat file.                                             *
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
 *    label_status:                                                   *
 *        The label_status variable is an integer which is used to    *
 *        indicate the success or failure of a PDS label routine,     *
 *        and whether or not the object or parameter specifications   *
 *        passed in were ambiguous.  Valid values are:  PDS_SUCCESS,  *
 *        PDS_ERROR, PDS_MULTIPLE_OBJECTS, PDS_MULTIPLE_PARMS, and    *
 *        PDS_WARNING.                                                *
 *$Returns                                                            *
 *    new_parameter_ptr:                                              *
 *        The new_parameter_ptr variable is a pointer to the          *
 *        structure used to represent a new parameter in a PDS label. *
 *$Detailed_Description                                               *
 *    The lab_add_parameter routine adds a new parameter to an        *
 *    object in a PDS label.  It searches for the object, mallocs     *
 *    storage for the new parameter and its value, and appends the    *
 *    new parameter onto the list of parameters attached to that      *
 *    object.  The parent object may by specified by pds_class, name,     *
 *    or position, or by any combination of these.  The search for    *
 *    the parent begins at the object pointed to by the label_ptr     *
 *    variable.  This is usually the "ROOT" object, but may be any    *
 *    other object in the tree.  Please note that it is not necessary *
 *    to specify the object's name, pds_class, and position at the same   *
 *    time.  In fact, if you specify only one of them and pass in     *
 *    zero for the others, the routine will work just fine.           *
 *$Error_Handling                                                     *
 *    1) If the object cannot be found, or memory cannot be           *
 *       allocated for the new parameter, then the label_status       *
 *       is set to PDS_ERROR and a NULL value is returned.            *
 *    2) If more than one object is found that matches the            *
 *       specifications passed in for the object, then the            *
 *       label_status is set to PDS_MULTIPLE_OBJECTS and a NULL       *
 *       value is returned.                                           *
 *    3) If everything went according to plan but syntax errors       *
 *       occurred when parsing the keyword or the value string, then  *
 *       label_status is set to PDS_WARNING and a pointer to the new  *
 *       parameter is returned.                                       *
 *    4) If everything went according to plan, then the label_status  *
 *       is set to PDS_SUCCESS and a pointer to the new parameter is  *
 *       returned.                                                    *
 *$Side_Effects                                                       *
 *    This routine causes memory to be allocated which must be freed  *
 *    by the calling routine.                                         *
 *$External_References                                                *
 *    Item                   Shared-Data             Access           *
 * -----------------------------------------------------------------  *
 *    pds_last_message       pdsglob.h               read             *
 *$Author_and_Institution                                             *
 *    Herbert C. Gamble / J.P.L.                                      *
 *    David P. Bernath / J.P.L.                                       *
 *$Version_and_Date                                                   *
 *    1.3   June 19, 1991                                             *
 *$Change_History                                                     *
 *    HCG   03-08-91   Original Code.                                 *
 *    DPB   04-10-91   Restructured routine.                          *
 *    DPB   04-26-91   Added code to strip blanks from inputs.        *
 *    KLM   06-17-91   Changed call from lab_append_parameter to      *
 *                     lu_append_parameter.                           * 
 *    MDD   06-19-92   Changed label_status to PDS_WARNING when there *
 *                     are syntax errors in the new value.            *
 **********************************************************************/

PARAMETER lab_add_parameter (label_ptr, object_class, object_name,        
                             object_position, parameter_name, 
                             parameter_value, label_status)

AGGREGATE label_ptr;
char *object_class;
char *object_name;
int object_position;
char *parameter_name;
char *parameter_value;
int *label_status;

{
    AGGREGATE object_ptr = {NULL};
    PARAMETER parameter_ptr = {NULL};
    ERROR_LIST *temp_msg = pds_last_message;

/** BEGIN **/

    /*-----------------------------------------------------------------------*/
    /** Initialize the label_status variable                                **/
    /*-----------------------------------------------------------------------*/

    *label_status = PDS_ERROR;

    /*-----------------------------------------------------------------------*/
    /** IF the parameter name passed in is not NULL THEN                    **/
    /*-----------------------------------------------------------------------*/

    if (parameter_name != NULL) 
    {
        /*-------------------------------------------------------------------*/
        /** Remove leading and trailing blanks from character inputs.       **/
        /*-------------------------------------------------------------------*/

        util_strip_lead_and_trail (object_class, ' ');
        util_strip_lead_and_trail (object_name, ' ');
        util_strip_lead_and_trail (parameter_name, ' ');

        /*-------------------------------------------------------------------*/
        /** Try to find the object which contains the parameter             **/
        /*-------------------------------------------------------------------*/

        object_ptr = lab_find_object (label_ptr, object_class, 
                                      object_name, object_position, label_status
);

        /*-------------------------------------------------------------------*/
        /** IF the object was found THEN                                    **/
        /**     Append the new parameter onto the object's list of parms    **/
        /** ENDIF                                                           **/
        /*-------------------------------------------------------------------*/

        if ((object_ptr != NULL) && (*label_status == PDS_SUCCESS))
        {
            parameter_ptr = lu_append_parameter (object_ptr, parameter_name, 
                                                  parameter_value);
        }  /*  End:  "if ((object_ptr != NULL) && ..."  */

    /*-----------------------------------------------------------------------*/
    /** ENDIF                                                               **/
    /*-----------------------------------------------------------------------*/

    }  /*  End:  "if (parameter_name != NULL) ..."  */

    /*-----------------------------------------------------------------------*/
    /** RETURN a pointer to the new parameter structure                     **/
    /*-----------------------------------------------------------------------*/

    if (*label_status == PDS_SUCCESS)
        if (pds_last_message != temp_msg) *label_status = PDS_WARNING;
    return (parameter_ptr);

/** END **/

}  /*  "lab_add_parameter"  */



/**********************************************************************
 *$Component                                                          *
 *    VALUE lab_add_value (label_ptr, object_class, object_name,      *
 *                         object_position, parameter_name,           *
 *                         parameter_position, parameter_value,       *
 *                         label_status)                              *
 *$Abstract                                                           *
 *    Adds a value to a parameter in a PDS Label.                     *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    ADD                                                             *
 *    VALUE                                                           *
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
 *    object_class:                                                   *
 *        The object_class variable is a character string             *
 *        which contains the pds_class of an object in a PDS label        *
 *        (e.g., "OBJECT = IMAGE" implies that "IMAGE" is the pds_class   *
 *        of the object).                                             *
 *    object_name:                                                    *
 *        The object_name variable is a character string              *
 *        which contains the name of an object in a PDS label.        *
 *        This is assumed to be the value of the "NAME" parameter     *
 *        in the object (e.g., "NAME = VOYAGER" implies that          *
 *        "VOYAGER" is the name of the object).                       *
 *    object_position:                                                *
 *        The object_position variable is an integer which            *
 *        represents the relative position of an object in a PDS      *
 *        label.  If this variable is used in conjunction with either *
 *        the object_class or object_name variables, then it          *
 *        represents a particular occurrence of that thing in the     *
 *        label (e.g., if object_class is "TABLE" and object_position *
 *        is 3, then this represents the third "TABLE" object in the  *
 *        label).  On the other hand, if this variable is used by     *
 *        itself, it represents the absolute position of the object,  *
 *        starting from the "ROOT" object (position = 1).  The        *
 *        sequence follows the structure of the label as it looks     *
 *        in a flat file.                                             *
 *    parameter_name:                                                 *
 *        The parameter_name variable is a character string           *
 *        which contains the name of a parameter in a PDS label       *
 *        (e.g., the line "SCID = VG1" implies that "SCID" is the     *
 *        parameter name).                                            *
 *    parameter_position:                                             *
 *        The parameter_position variable is an integer which         *
 *        represents the relative position of a parameter in an       *
 *        object in a PDS label.  If this variable is used in         *
 *        conjunction with the parameter_name variable, then it       *
 *        represents a particular occurrence of that parameter        *
 *        in the object (e.g., if parameter_name is "SCID" and        *
 *        parameter_position is 2, then this represents the second    *
 *        "SCID" parameter in the object).  On the other hand, if     *
 *        this variable is used by itself, it represents the absolute *
 *        position of the parameter within the object, starting with  *
 *        first parameter in the object (position = 1).               *
 *    parameter_value:                                                *
 *        The parameter_value variable is a character string          *
 *        which contains the value of a parameter in a PDS label      *
 *        (e.g., the line "SCID = VG1" implies that "VG1" is the      *
 *        value of the "SCID" parameter).                             *
 *$Outputs                                                            *
 *    label_status:                                                   *
 *        The label_status variable is an integer which is used to    *
 *        indicate the success or failure of a PDS label routine,     *
 *        and whether or not the object or parameter specifications   *
 *        passed in were ambiguous.  Valid values are:  PDS_SUCCESS,  *
 *        PDS_ERROR, PDS_MULTIPLE_OBJECTS, PDS_MULTIPLE_PARMS, and    *
 *        PDS_WARNING.                                                *
 *$Returns                                                            *
 *    new_value_ptr:                                                  *
 *        The new_value_ptr variable is a pointer to the structure    *
 *        used to represent a new value of a parameter in a PDS label.*
 *$Detailed_Description                                               *
 *    The lab_add_value routine adds a new value to a parameter of    *
 *    an object in a PDS label.  It searches for the object,          *
 *    searches for the parameter attached to the object, then mallocs *
 *    storage for the new value and appends it onto the list of       *
 *    values attached to the parameter.  The parent object may by     *
 *    specified by pds_class, name, or position, or by any combination    *
 *    of these, and the parameter may be specified by name or         *
 *    position, or both.   The search for the object begins at the    *
 *    object pointed to by the label_ptr variable.  This is usually   *
 *    the "ROOT" object, but may be any other object in the tree.     *
 *    Please note that it is not necessary to specify the object's    *
 *    name, pds_class, and position at the same time.  In fact, if you    *
 *    specify only one of them and pass in zero for the others, the   *
 *    routine will work just fine.  This also applies to the          *
 *    parameter name and position.                                    *
 *                                                                    *
 *    Please note that lab_add_value has been left in this library    *
 *    only for backwards compatibility.  It is NOT functioning        *
 *    correctly.  The resulting format of the output may be incorrect.*
 *$Error_Handling                                                     *
 *    1) If the object or parameter cannot be found, or memory cannot *
 *       be allocated for the new value, then the label_status        *
 *       is set to PDS_ERROR and a NULL value is returned.            *
 *    2) If more than one object is found that matches the            *
 *       specifications passed in for the object, then the            *
 *       label_status is set to PDS_MULTIPLE_OBJECTS and a NULL       *
 *       value is returned.                                           *
 *    3) If more than one parameter is found that matches the         *
 *       specifications passed in for the parameter, then the         *
 *       label_status is set to PDS_MULTIPLE_PARMS and a NULL         *
 *       value is returned.                                           *
 *    4) If everything went according to plan, then the label_status  *
 *       is set to PDS_SUCCESS and a pointer to the new value is      *
 *       returned.                                                    *
 *$Side_Effects                                                       *
 *    This routine causes memory to be allocated which must be freed  *
 *    by the calling routine.                                         *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Herbert C. Gamble / J.P.L.                                      *
 *    David P. Bernath / J.P.L.                                       *
 *$Version_and_Date                                                   *
 *    1.2   June 17, 1991                                             *
 *$Change_History                                                     *
 *    HCG   03-08-91   Original Code.                                 *
 *    DPB   04-10-91   Restructured routine.                          *
 *    DPB   04-26-91   Added code to strip blanks from inputs.        *
 *    KLM   06-17-91   Changed call from lab_append_value to          *
 *                     lu_append_value.                               *
 **********************************************************************/

VALUE lab_add_value (label_ptr, object_class, object_name, 
                     object_position, parameter_name, 
                     parameter_position, parameter_value, label_status)

AGGREGATE label_ptr;
char *object_class;
char *object_name;
int object_position;
char *parameter_name;
int parameter_position;
char *parameter_value;
int *label_status;

{
    PARAMETER parameter_ptr = {NULL};
    VALUE value_ptr = {NULL};

/** BEGIN **/

    /*-----------------------------------------------------------------------*/
    /** Initialize the label_status variable                                **/
    /*-----------------------------------------------------------------------*/

    *label_status = PDS_ERROR;

    /*-----------------------------------------------------------------------*/
    /** Remove leading and trailing blanks from character inputs.           **/
    /*-----------------------------------------------------------------------*/

    util_strip_lead_and_trail (object_class, ' ');
    util_strip_lead_and_trail (object_name, ' ');
    util_strip_lead_and_trail (parameter_name, ' ');

    /*-----------------------------------------------------------------------*/
    /** Try to find the object and parameter in the label                   **/
    /*-----------------------------------------------------------------------*/

    parameter_ptr = lab_find_parameter (label_ptr, object_class, 
                                        object_name, object_position, 
                                        parameter_name, parameter_position, 
                                        label_status);

    /*-----------------------------------------------------------------------*/
    /** IF the object and parameter were found THEN                         **/
    /**     Append the new value onto the parameter's list of values        **/
    /** ENDIF                                                               **/
    /*-----------------------------------------------------------------------*/

    if ((parameter_ptr != NULL) && (*label_status == PDS_SUCCESS))
        value_ptr = lu_append_value (parameter_ptr, parameter_value);

    /*-----------------------------------------------------------------------*/
    /** RETURN a pointer to the new value structure                         **/
    /*-----------------------------------------------------------------------*/

    return (value_ptr);

/** END **/

}  /*  "lab_add_value"  */


/**********************************************************************
 *$Component                                                          *
 *    LOGICAL lab_adjust_pointers (label_ptr, label_records,          *
 *                                 label_status)                      *
 *$Abstract                                                           *
 *    Adjusts attached pointers in a PDS label.                       *
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
 *    label_records:                                                  *
 *        The label_records variable is an integer containing the     *
 *        number of records in a PDS label.                           *
 *$Outputs                                                            *
 *    label_status:                                                   *
 *        The label_status variable is an integer which is used to    *
 *        indicate the success or failure of a PDS label routine,     *
 *        and whether or not the object or parameter specifications   *
 *        passed in were ambiguous.  Valid values are:  PDS_SUCCESS,  *
 *        PDS_ERROR, PDS_MULTIPLE_OBJECTS, PDS_MULTIPLE_PARMS, and    *
 *        PDS_WARNING.                                                *
 *$Returns                                                            *
 *    success_flag:                                                   *
 *        The success_flag variable is a general purpose variable     *
 *        which indicates success (a value of TRUE) or failure        *
 *        (a value of FALSE).                                         *
 *$Detailed_Description                                               *
 *    The lab_adjust_pointers routine searches the given label tree   *
 *    for "attached" style data object pointers of the form ^X = N    *
 *    and adjusts them according to the value of label_records given  *
 *    as input. If LABEL_RECORDS and FILE_RECORDS are in the label,   *
 *    they are also modified.                                         *
 *                                                                    *
 *    This routine depends upon the old value of LABEL_RECORDS in the *
 *    label to do its work. For this reason, the new value of label   *
 *    records should NOT be placed into the label before calling this *
 *    routine.  The same is true of FILE_RECORDS.  If LABEL_RECORDS   *
 *    is not present in the label, then the value is assumed to be    *
 *    0.  A LABEL_RECORDS keyword is NOT added to the label. The      *
 *    new value of each pointer is computed by subtracting the old    *
 *    value of LABEL_RECORDS from the pointer location, and then      *
 *    adding the new value of label records to it.  This means that   *
 *    "new" labels should be created with LABEL_RECORDS = 0, and      *
 *    FILE_RECORDS =<number of data records only>, with every pointer *
 *    set to the absolute location of the associated data object      *
 *    IN THE UNLABELLED DATA FILE.                                    *
 *                                                                    *
 *    Note that the label_records input, if unknown, can be obtained  *
 *    by calling lab_write_label and then examining the global        *
 *    variable pds_records_written.                                   *
 *                                                                    *
 *    This routine returns TRUE if all the pointers are replaced, and *
 *    FALSE otherwise.                                                *
 *$Side_Effects                                                       *
 *    None                                                            *
 *$Error_Handling                                                     *
 *    If this routine fails, messages will be appended to the global  *
 *    message list.                                                   *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Kristy L. Marski / J.P.L.                                       *
 *$Version_and_Date                                                   *
 *    1.1   October 21, 1991                                          *
 *$Change_History                                                     *
 *    KLM   05-16-91   Original Code.                                 *
 *    MDD   10-21-91   FIxed malloc, free, and sys_exit_system calls  *
 **********************************************************************/

LOGICAL lab_adjust_pointers (label_ptr, label_records, label_status)

AGGREGATE label_ptr;
long label_records;
int *label_status;
{
    int local_status;
    char new_label_rec_str [PDS_MAXLINE + 1];
    char new_file_rec_str [PDS_MAXLINE + 1];
    char *temp_str = NULL;
    POINTER_INFO *pointer = NULL;
    long old_label_records = 0;
    long old_file_records;
    long new_file_records;
    PARAMETER root_param;
    PARAMETER next_param;
    LOGICAL success = TRUE;

/** BEGIN **/

    /*------------------------------------------------------------------*/
    /** copy the new value of label records to a string                **/
    /*------------------------------------------------------------------*/

    *label_status = PDS_SUCCESS;
    sprintf (new_label_rec_str, "%ld", label_records);

    /*------------------------------------------------------------------*/
    /** get the old value of label records                             **/
    /** convert it to a string and replace it with the new value       **/
    /*------------------------------------------------------------------*/

    temp_str = lab_get_value (label_ptr, "ROOT", NULL, 1, "LABEL_RECORDS", 
                              1, 1, FALSE, &local_status);
    if (temp_str != NULL)
    {
       sscanf (temp_str, "%ld", &old_label_records);
       lab_change_value (label_ptr, "ROOT", NULL, 1, "LABEL_RECORDS",
                         1, new_label_rec_str, &local_status);
       Lemme_Go(temp_str);
    }

    /*------------------------------------------------------------------*/
    /** get the old value of file records                              **/
    /** convert it to a string                                         **/
    /** compute the new value and replace the value in the label       **/
    /*------------------------------------------------------------------*/

    temp_str = lab_get_value (label_ptr, "ROOT", NULL, 1, "FILE_RECORDS", 
                              1, 1, FALSE, &local_status);
    if (temp_str != NULL)
    {
       sscanf (temp_str, "%ld", &old_file_records);
       new_file_records = (old_file_records - old_label_records) + label_records;
       sprintf (new_file_rec_str, "%ld", new_file_records);
       lab_change_value (label_ptr, "ROOT", NULL, 1, "FILE_RECORDS",
                         1, new_file_rec_str, &local_status);
       Lemme_Go(temp_str);
    }

    /*------------------------------------------------------------------*/
    /** LOOP through all the parameters in the label root              **/
    /**    IF the current parameter is a pointer THEN                  **/
    /**       get the pointer value into a structure                   **/
    /**    ENDIF                                                       **/
    /*------------------------------------------------------------------*/

    root_param = FirstParameter (label_ptr); 
    while (root_param != NULL)
    {
       /* you have to get the next parameter now because lab_replace_pointer
          will mess with the root_param pointer */

       next_param = NextParameter (root_param);
       if (root_param -> node_kind == KP_POINTER)
       {
          pointer = lab_get_pointer (label_ptr, root_param -> name, 1, &local_status);
          if (local_status == PDS_SUCCESS)
          {
             if (pointer != NULL && pointer -> is_attached)
             {
                pointer -> location = (pointer -> location - old_label_records)
                                      + label_records;
                lab_replace_pointer (label_ptr, pointer, &local_status);
	     }
	  }
          if (local_status != PDS_SUCCESS) *label_status = PDS_ERROR;
          Lemme_Go(pointer);
       }
       root_param = next_param;
    }
    if (*label_status != PDS_SUCCESS)
    {
       err_append_message (ERROR1, "Unable to update attached pointer(s) in label");
       success = FALSE;
    }
    return (success);
}

/**********************************************************************
 *$Component                                                          *
 *    LOGICAL lab_change_object_class (label_ptr, object_class,       *
 *                       object_name, object_position,                *
 *                       new_object_class, label_status)              *
 *$Abstract                                                           *
 *    Locates an object in a PDS label.                               *
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
 *    object_class:                                                   *
 *        The object_class variable is a character string             *
 *        which contains the pds_class of an object in a PDS label        *
 *        (e.g., "OBJECT = IMAGE" implies that "IMAGE" is the pds_class   *
 *        of the object).                                             *
 *    object_name:                                                    *
 *        The object_name variable is a character string              *
 *        which contains the name of an object in a PDS label.        *
 *        This is assumed to be the value of the "NAME" parameter     *
 *        in the object (e.g., "NAME = VOYAGER" implies that          *
 *        "VOYAGER" is the name of the object).                       *
 *    object_position:                                                *
 *        The object_position variable is an integer which            *
 *        represents the relative position of an object in a PDS      *
 *        label.  If this variable is used in conjunction with either *
 *        the object_class or object_name variables, then it          *
 *        represents a particular occurrence of that thing in the     *
 *        label (e.g., if object_class is "TABLE" and object_position *
 *        is 3, then this represents the third "TABLE" object in the  *
 *        label).  On the other hand, if this variable is used by     *
 *        itself, it represents the absolute position of the object,  *
 *        starting from the "ROOT" object (position = 1).  The        *
 *        sequence follows the structure of the label as it looks     *
 *        in a flat file.                                             *
 *    new_object_class:                                               *
 *        The new_object_class variable is a character string         *
 *        which contains the new pds_class of an object in a PDS label    *
 *        (e.g., "OBJECT = IMAGE" implies that "IMAGE" is the pds_class   *
 *        of the object).                                             *
 *$Outputs                                                            *
 *    label_status:                                                   *
 *        The label_status variable is an integer which is used to    *
 *        indicate the success or failure of a PDS label routine,     *
 *        and whether or not the object or parameter specifications   *
 *        passed in were ambiguous.  Valid values are:  PDS_SUCCESS,  *
 *        PDS_ERROR, PDS_MULTIPLE_OBJECTS, PDS_MULTIPLE_PARMS, and    *
 *        PDS_WARNING.                                                *
 *$Returns                                                            *
 *    success_flag:                                                   *
 *        The success_flag variable is a general purpose variable     *
 *        which indicates success (a value of TRUE) or failure        *
 *        (a value of FALSE).                                         *
 *$Detailed_Description                                               *
 *    The lab_change_object_class routine finds an object and changes *
 *    its pds_class. The object may by specified by pds_class, name, or       *
 *    position, or by any combination of these. Please note that it   *
 *    is not necessary to specify the object's name, pds_class, and       *
 *    position at the same time.  In fact, if you specify only one of *
 *    them and pass in zero for the others, the routine will work     *
 *    just fine.                                                      *
 *$Side_Effects                                                       *
 *    None                                                            *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Kristy L. Marski / J.P.L.                                       *
 *$Version_and_Date                                                   *
 *    1.1   October 21, 1991                                          *
 *$Change_History                                                     *
 *    KLM   05-16-91   Original Code.                                 *
 *    MDD   10-21-91   FIxed malloc, free, and sys_exit_system calls  *
 **********************************************************************/

LOGICAL lab_change_object_class (label_ptr, object_class, object_name, 
                                 object_position, new_object_class, 
                                 label_status)

AGGREGATE label_ptr;
char *object_class;
char *object_name;
int object_position;
char *new_object_class;
int *label_status;

{            

    AGGREGATE object_ptr = {NULL};
    LOGICAL success = {TRUE};

/** BEGIN **/           
   /*----------------------------------------------------------------------*/
   /** strip blanks from string arguments                                 **/
   /** Try to find the object                                             **/  
   /*----------------------------------------------------------------------*/

   util_strip_lead_and_trail (new_object_class, ' ');
   object_ptr = lab_find_object (label_ptr, object_class, object_name, 
                                 object_position, label_status);

   /*----------------------------------------------------------------------*/
   /** IF the object was found and the pointer isn't NULL THEN            **/  
   /**    free the current object name                                    **/
   /**    allocate memory for the new object name                         **/
   /**    copy the new object name to the old name location               **/
   /*----------------------------------------------------------------------*/

   if (*label_status == PDS_SUCCESS && object_ptr != NULL)
   {
      Lemme_Go(object_ptr -> name);
      Malloc_String(object_ptr -> name,(int) String_Size(new_object_class));
      strcpy (object_ptr -> name, new_object_class);
   }

   /*----------------------------------------------------------------------*/
   /** ELSE IF the there was an error locating the object THEN            **/  
   /**    set the success/failure to FALSE                                **/
   /** ENDIF                                                              **/
   /*----------------------------------------------------------------------*/ 

   else 
   {
        success = FALSE;
   }

   /*----------------------------------------------------------------------*/
   /** return success/failure                                             **/
   /*----------------------------------------------------------------------*/

   return (success);

/** END **/

}  /*  "lab_change_object_class"  */                          


/**********************************************************************
 *$Component                                                          *
 *    LOGICAL lab_change_parameter_name (label_ptr, object_class,     *
 *                                object_name, object_position,       *
 *                                parameter_name, parameter_position, *
 *                                new_parameter_name, label_status)   *
 *$Abstract                                                           *
 *    Changes a parameter in a PDS label.                             *
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
 *    object_class:                                                   *
 *        The object_class variable is a character string             *
 *        which contains the pds_class of an object in a PDS label        *
 *        (e.g., "OBJECT = IMAGE" implies that "IMAGE" is the pds_class   *
 *        of the object).                                             *
 *    object_name:                                                    *
 *        The object_name variable is a character string              *
 *        which contains the name of an object in a PDS label.        *
 *        This is assumed to be the value of the "NAME" parameter     *
 *        in the object (e.g., "NAME = VOYAGER" implies that          *
 *        "VOYAGER" is the name of the object).                       *
 *    object_position:                                                *
 *        The object_position variable is an integer which            *
 *        represents the relative position of an object in a PDS      *
 *        label.  If this variable is used in conjunction with either *
 *        the object_class or object_name variables, then it          *
 *        represents a particular occurrence of that thing in the     *
 *        label (e.g., if object_class is "TABLE" and object_position *
 *        is 3, then this represents the third "TABLE" object in the  *
 *        label).  On the other hand, if this variable is used by     *
 *        itself, it represents the absolute position of the object,  *
 *        starting from the "ROOT" object (position = 1).  The        *
 *        sequence follows the structure of the label as it looks     *
 *        in a flat file.                                             *
 *    parameter_name:                                                 *
 *        The parameter_name variable is a character string           *
 *        which contains the name of a parameter in a PDS label       *
 *        (e.g., the line "SCID = VG1" implies that "SCID" is the     *
 *        parameter name).                                            *
 *    parameter_position:                                             *
 *        The parameter_position variable is an integer which         *
 *        represents the relative position of a parameter in an       *
 *        object in a PDS label.  If this variable is used in         *
 *        conjunction with the parameter_name variable, then it       *
 *        represents a particular occurrence of that parameter        *
 *        in the object (e.g., if parameter_name is "SCID" and        *
 *        parameter_position is 2, then this represents the second    *
 *        "SCID" parameter in the object).  On the other hand, if     *
 *        this variable is used by itself, it represents the absolute *
 *        position of the parameter within the object, starting with  *
 *        first parameter in the object (position = 1).               *
 *    new_parameter_name:                                             *
 *        The new_parameter_name variable is a character string       *
 *        which contains the new name for a parameter in a PDS label  *
 *        (e.g., the line "SCID = VG1" implies that "SCID" is the     *
 *        parameter name).                                            *
 *$Outputs                                                            *
 *    label_status:                                                   *
 *        The label_status variable is an integer which is used to    *
 *        indicate the success or failure of a PDS label routine,     *
 *        and whether or not the object or parameter specifications   *
 *        passed in were ambiguous.  Valid values are:  PDS_SUCCESS,  *
 *        PDS_ERROR, PDS_MULTIPLE_OBJECTS, PDS_MULTIPLE_PARMS, and    *
 *        PDS_WARNING.                                                *
 *$Returns                                                            *
 *    success_flag:                                                   *
 *        The success_flag variable is a general purpose variable     *
 *        which indicates success (a value of TRUE) or failure        *
 *        (a value of FALSE).                                         *
 *$Detailed_Description                                               *
 *    The lab_change_parameter_name routine finds a parameter and     *
 *    changes its name. The object may by specified by pds_class, name, or*
 *    position, or by any combination of these, and the parameter may *
 *    be specified by name or position or both. Please note that it   *
 *    is not necessary to specify the object's name, pds_class, and       *
 *    position at the same time.  In fact, if you specify only one of *
 *    them and pass in zero for the others, the routine will work     *
 *    just fine.  This also applies to the parameter name and         *
 *    position.                                                       *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Kristy L. Marski / J.P.L.                                       *
 *$Version_and_Date                                                   *
 *    1.1   October 21, 1991                                          *
 *$Change_History                                                     *
 *    KLM   05-17-91   Original Code.                                 *
 *    MDD   10-17-91   Fixed malloc, free, and sys_exit_system calls  *
 **********************************************************************/

LOGICAL lab_change_parameter_name (label_ptr, object_class, object_name, 
                                   object_position, parameter_name, 
                                   parameter_position, new_parameter_name,
                                   label_status)

AGGREGATE label_ptr;
char *object_class;
char *object_name;
int object_position;
char *parameter_name;
int parameter_position;
char *new_parameter_name;
int *label_status;

{
    PARAMETER parameter_ptr = {NULL};       
    LOGICAL success = {TRUE};

/** BEGIN **/
   /*----------------------------------------------------------------------*/
   /** strip blanks from string arguments                                 **/  
   /** Try to find the parameter                                          **/  
   /*----------------------------------------------------------------------*/

   util_strip_lead_and_trail (new_parameter_name, ' ');
   parameter_ptr = lab_find_parameter (label_ptr, object_class, 
                      object_name, object_position, parameter_name, 
                      parameter_position, label_status);

   /*----------------------------------------------------------------------*/
   /** IF the parameter was found and the pointer isn't NULL THEN         **/  
   /**    free the current parameter name                                 **/
   /**    allocate memory for the new parameter name                      **/
   /**    copy the new parameter name to the old name location            **/
   /*----------------------------------------------------------------------*/
 
   if (*label_status == PDS_SUCCESS && parameter_ptr != NULL)
   {
      Lemme_Go(parameter_ptr -> name);
      Malloc_String(parameter_ptr -> name, (int) String_Size(new_parameter_name));
      strcpy (parameter_ptr -> name, new_parameter_name);
   }
   /*----------------------------------------------------------------------*/
   /** ELSE IF the there was an error locating the object THEN            **/  
   /**    set the success/failure to FALSE                                **/
   /** ENDIF                                                              **/
   /*----------------------------------------------------------------------*/
         
   else
   {
        success = FALSE;
   }

   /*----------------------------------------------------------------------*/
   /** return success/failure                                             **/
   /*----------------------------------------------------------------------*/

   return (success);

/** END **/

}  /*  "lab_change_parameter_name"  */                          


/**********************************************************************
 *$Component                                                          *
 *    VALUE lab_change_value (label_ptr, object_class, object_name,   *
 *                            object_position, parameter_name,        *
 *                            parameter_position, parameter_value,    *
 *                            label_status)                           *
 *$Abstract                                                           *
 *    Changes the value of a parameter in a PDS label.                *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    CHANGE                                                          *
 *    PARAMETER                                                       *
 *    VALUE                                                           *
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
 *    object_class:                                                   *
 *        The object_class variable is a character string             *
 *        which contains the pds_class of an object in a PDS label        *
 *        (e.g., "OBJECT = IMAGE" implies that "IMAGE" is the pds_class   *
 *        of the object).                                             *
 *    object_name:                                                    *
 *        The object_name variable is a character string              *
 *        which contains the name of an object in a PDS label.        *
 *        This is assumed to be the value of the "NAME" parameter     *
 *        in the object (e.g., "NAME = VOYAGER" implies that          *
 *        "VOYAGER" is the name of the object).                       *
 *    object_position:                                                *
 *        The object_position variable is an integer which            *
 *        represents the relative position of an object in a PDS      *
 *        label.  If this variable is used in conjunction with either *
 *        the object_class or object_name variables, then it          *
 *        represents a particular occurrence of that thing in the     *
 *        label (e.g., if object_class is "TABLE" and object_position *
 *        is 3, then this represents the third "TABLE" object in the  *
 *        label).  On the other hand, if this variable is used by     *
 *        itself, it represents the absolute position of the object,  *
 *        starting from the "ROOT" object (position = 1).  The        *
 *        sequence follows the structure of the label as it looks     *
 *        in a flat file.                                             *
 *    parameter_name:                                                 *
 *        The parameter_name variable is a character string           *
 *        which contains the name of a parameter in a PDS label       *
 *        (e.g., the line "SCID = VG1" implies that "SCID" is the     *
 *        parameter name).                                            *
 *    parameter_position:                                             *
 *        The parameter_position variable is an integer which         *
 *        represents the relative position of a parameter in an       *
 *        object in a PDS label.  If this variable is used in         *
 *        conjunction with the parameter_name variable, then it       *
 *        represents a particular occurrence of that parameter        *
 *        in the object (e.g., if parameter_name is "SCID" and        *
 *        parameter_position is 2, then this represents the second    *
 *        "SCID" parameter in the object).  On the other hand, if     *
 *        this variable is used by itself, it represents the absolute *
 *        position of the parameter within the object, starting with  *
 *        first parameter in the object (position = 1).               *
 *    parameter_value:                                                *
 *        The parameter_value variable is a character string          *
 *        which contains the value of a parameter in a PDS label      *
 *        (e.g., the line "SCID = VG1" implies that "VG1" is the      *
 *        value of the "SCID" parameter).                             *
 *$Outputs                                                            *
 *    label_status:                                                   *
 *        The label_status variable is an integer which is used to    *
 *        indicate the success or failure of a PDS label routine,     *
 *        and whether or not the object or parameter specifications   *
 *        passed in were ambiguous.  Valid values are:  PDS_SUCCESS,  *
 *        PDS_ERROR, PDS_MULTIPLE_OBJECTS, PDS_MULTIPLE_PARMS, and    *
 *        PDS_WARNING.                                                *
 *$Returns                                                            *
 *    new_value_ptr:                                                  *
 *        The new_value_ptr variable is a pointer to the structure    *
 *        used to represent a new value of a parameter in a PDS label.*
 *$Detailed_Description                                               *
 *    The lab_change_value routine removes a list of values from      *
 *    a parameter structure in a PDS label and replaces it with       *
 *    the new value passed in.  It searches for the object,           *
 *    searches for the parameter attached to the object, then         *
 *    verifies whether or not the value passed in is valid.  If it    *
 *    is, then the old value is removed, storage for the new value    *
 *    is malloc'd, and the new value is added to the parameter.       *
 *    The object may by specified by pds_class, name, or position, or     *
 *    by any combination of these, and the parameter may be           *
 *    specified by name or position, or both.   The search for the    *
 *    object begins at the object pointed to by the label_ptr         *
 *    variable.  This is usually the "ROOT" object, but may be any    *
 *    other object in the tree.  Please note that it is not necessary *
 *    to specify the object's name, pds_class, and position at the same   *
 *    time.  In fact, if you specify only one of them and pass in     *
 *    zero for the others, the routine will work just fine.  This     *
 *    also applies to the parameter name and position.                *
 *                                                                    *
 *    This routine now leaves the relative position of the parameter  *
 *    unchanged, rather than moving it to the end of the parent       *
 *    object's parameter list.                                        *
 *                                                                    *
 *    If there are syntax errors in the new value, then messages      *
 *    will be appended to the global message list.                    *
 *$Error_Handling                                                     *
 *    1) If the object or parameter cannot be found, or memory cannot *
 *       be allocated for the new value, or the new value is invalid, *
 *       then the label_status is set to PDS_ERROR and a NULL value   *
 *       is returned.                                                 *
 *    2) If more than one object is found that matches the            *
 *       specifications passed in for the object, then the            *
 *       label_status is set to PDS_MULTIPLE_OBJECTS and a NULL       *
 *       value is returned.                                           *
 *    3) If more than one parameter is found that matches the         *
 *       specifications passed in for the parameter, then the         *
 *       label_status is set to PDS_MULTIPLE_PARMS and a NULL         *
 *       value is returned.                                           *
 *    4) If everything went according to plan but syntax errors       *
 *       occurred when parsing the keyword or the value string, then  *
 *       label_status is set to PDS_WARNING and a pointer to the new  *
 *       value is returned.                                           *
 *    5) If everything went according to plan, then the label_status  *
 *       is set to PDS_SUCCESS and a pointer to the new value is      *
 *       returned.                                                    *
 *$Side_Effects                                                       *
 *    This routine causes memory to be allocated which must be freed  *
 *    by the calling routine.                                         *
 *$External_References                                                *
 *    Item                   Shared-Data             Access           *
 * -----------------------------------------------------------------  *
 *    pds_last_message       pdsglob.h               read             *
 *$Author_and_Institution                                             *
 *    Herbert C. Gamble / J.P.L.                                      *
 *    David P. Bernath / J.P.L.                                       *
 *$Version_and_Date                                                   *
 *    3.1  October 27, 1992                                           *
 *$Change_History                                                     *
 *    HCG   03-08-91   Original Code.                                 *
 *    DPB   04-10-91   Restructured routine.                          *
 *    DPB   04-26-91   Added code to strip blanks from inputs.        *
 *    KLM   06-17-91   Changed call from lab_append_value to          *
 *                     lu_append_value.                               *
 *    MDD   08-22-91   Rewrote to use cut and paste parameter routines*
 *    MDD   01-13-92   Added transfer of node_kind from old to new    *
 *                     parameter.                                     *
 *    MDD   06-19-92   Changed label_status to PDS_WARNING when there *
 *                     are syntax errors in the new value.            *
 *    MDD   08-17-92   Changed to leave parameter in same position    *
 *    MDD   10-27-92   Fixed bug with "position only" inserts.        *
 **********************************************************************/

VALUE lab_change_value (label_ptr, object_class, object_name, object_position, 
                        parameter_name, parameter_position, parameter_value,
                        label_status)

AGGREGATE label_ptr;
char *object_class;
char *object_name;
int object_position;
char *parameter_name;
int parameter_position;
char *parameter_value;
int *label_status;

{
    AGGREGATE object_ptr = {NULL};
    AGGREGATE temp_object_ptr = {NULL};
    PARAMETER temp_parameter_ptr = {NULL};
    PARAMETER parameter_ptr = {NULL};
    VALUE value_ptr = {NULL};
    PARAMETER_KIND temp_kind;
    ERROR_LIST *temp_msg = pds_last_message;
    int parm_pos;

/** BEGIN **/

    /*-----------------------------------------------------------------------*/
    /** Initialize the label_status variable.                               **/
    /*-----------------------------------------------------------------------*/

    *label_status = PDS_ERROR;

    /*-----------------------------------------------------------------------*/
    /** Remove leading and trailing blanks from character inputs.           **/
    /*-----------------------------------------------------------------------*/

    util_strip_lead_and_trail (object_class, ' ');
    util_strip_lead_and_trail (object_name, ' ');
    util_strip_lead_and_trail (parameter_name, ' ');

    /*-----------------------------------------------------------------------*/
    /** Try to find the parameter whose value is to                         **/
    /**    be changed.                                                      **/
    /*-----------------------------------------------------------------------*/

    parameter_ptr = lab_find_parameter (label_ptr, object_class, object_name,
	      			     object_position, parameter_name,
				     parameter_position, label_status);


    /*-----------------------------------------------------------------------*/
    /** IF the parameter was found THEN                                     **/
    /*-----------------------------------------------------------------------*/

    if ((parameter_ptr != NULL) && (*label_status == PDS_SUCCESS))
    {

	  object_ptr = parameter_ptr -> owner;

	  /*-----------------------------------------------------------------*/
	  /** Get the position of the parameter                             **/
	  /*-----------------------------------------------------------------*/

          for (parm_pos = 1, temp_parameter_ptr = FirstParameter (object_ptr); 
               temp_parameter_ptr != NULL && temp_parameter_ptr != parameter_ptr; 
               temp_parameter_ptr = NextParameter(temp_parameter_ptr), parm_pos++);
               
	  /*-----------------------------------------------------------------*/
	  /** Create a one object tree and add the new parameter and value  **/
	  /**     to it.   We need to verify that the new value is valid    **/
	  /**     (and that there is enough memory left to malloc it)       **/
	  /**     BEFORE we remove the old value.                           **/
	  /*-----------------------------------------------------------------*/

	  *label_status = PDS_ERROR;
	  temp_object_ptr = NewAggregate (NULL, KA_OBJECT, "ROOT", "");

	  /*-----------------------------------------------------------------*/
	  /** IF the temporary object was created THEN                      **/
	  /*-----------------------------------------------------------------*/

	  if (temp_object_ptr != NULL)
	  {
	     /*--------------------------------------------------------------*/
	     /** Add a temporary parameter to this temporary object whose   **/
	     /**     value is the value passed in.                          **/
	     /*--------------------------------------------------------------*/

	     temp_parameter_ptr = lu_append_parameter (temp_object_ptr,
						       parameter_ptr -> name,
						       parameter_value);

	     /*--------------------------------------------------------------*/
	     /** IF the temporary parameter was added to the object THEN    **/
	     /**     Cut the parameter from the temporary object and        **/
	     /**         paste it into the real object                      **/
	     /** ENDIF                                                      **/
	     /*--------------------------------------------------------------*/

	     if (temp_parameter_ptr != NULL)
	     {
		temp_parameter_ptr = CutParameter (temp_parameter_ptr);
		if (temp_parameter_ptr != NULL)
		{

                   temp_kind = parameter_ptr -> node_kind;
		   RemoveParameter (parameter_ptr);
		   temp_parameter_ptr = lu_paste_parm_at_pos (object_ptr,
							temp_parameter_ptr, parm_pos);
		   if (temp_parameter_ptr != NULL)
		   {
		      *label_status = PDS_SUCCESS;
		      value_ptr = temp_parameter_ptr -> first_value;
                      temp_parameter_ptr -> node_kind = temp_kind;
		   }
		}
	     }

	     /*--------------------------------------------------------------*/
	     /** Remove the temporary object.                               **/
	     /*--------------------------------------------------------------*/

	     temp_object_ptr = RemoveAggregate (temp_object_ptr);

	  }
	  /*-----------------------------------------------------------------*/
	  /** ENDIF the temporary object was created                        **/
	  /*-----------------------------------------------------------------*/
    }
    /*-----------------------------------------------------------------------*/
    /** ENDIF the parameter was found                                       **/
    /** RETURN a pointer to the new value structure                         **/
    /*-----------------------------------------------------------------------*/

    if (*label_status == PDS_SUCCESS)
       if (pds_last_message != temp_msg) *label_status = PDS_WARNING;
    return (value_ptr);

/** END **/

}  /*  "lab_change_value"  */



/**********************************************************************
 *$Component                                                          *
 *    void lab_clear_messages ()                                      *
 *$Abstract                                                           *
 *    Clears the global message list.                                 *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    ERROR1                                                          *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    None                                                            *
 *$Outputs                                                            *
 *    None                                                            *
 *$Returns                                                            *
 *    None                                                            *
 *$Detailed_Description                                               *
 *    The lab_clear_messages routine deallocates the global message   *
 *    list.                                                           *
 *$Error_Handling                                                     *
 *    None                                                            *
 *$External_References                                                *
 *    Item                     Shared-Data           Access           *
 *    ------------------------------------------------------------    *
 *    pds_message_list         pdsglob.h             read             *
 *$Author_and_Institution                                             *
 *    David P. Bernath / J.P.L.                                       *
 *$Version_and_Date                                                   *
 *    1.1   October 21, 1991                                          *
 *$Change_History                                                     *
 *    DPB   04-10-91   Original Code.                                 *
 *    MDD   10-21-91   Removed redirect stuff.                        *
 **********************************************************************/

void lab_clear_messages ()

{
    err_deallocate_list (pds_message_list);

}  /*  "lab_clear_messages"  */



/**********************************************************************
 *$Component                                                          *
 *    LOGICAL lab_exit ()                                             *
 *$Abstract                                                           *
 *    Cleans up after the Label Library                               *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    None.                                                           *
 *$Outputs                                                            *
 *    None.                                                           *
 *$Returns                                                            *
 *    success_flag:                                                   *
 *        The success_flag variable is a general purpose variable     *
 *        which indicates success (a value of TRUE) or failure        *
 *        (a value of FALSE).                                         *
 *$Detailed_Description                                               *
 *    The lab_exit routine clears the global message list, calls      *
 *    fio_exit to delete temporary files used by the label library,   *
 *    and returns a success or failure flag. If errors occurred, then *
 *    specific errors will be added to the message list, and can be   *
 *    checked after this routine returns.                             *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore/ JPL                                            *
 *$Version_and_Date                                                   *
 *    1.0   June 24, 1992                                             *
 *$Change_History                                                     *
 *    MDD   06-24-92   Original Code.                                 *
 **********************************************************************/

LOGICAL lab_exit () 
{
   lab_clear_messages (); 
   fio_exit (); 
   return (!lab_has_messages ());
}


/**********************************************************************
 *$Component                                                          *
 *    AGGREGATE lab_find_object (label_ptr, object_class,             *
 *                       object_name, object_position, label_status)  *
 *$Abstract                                                           *
 *    Locates an object in a PDS label.                               *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    FIND                                                            *
 *    OBJECT                                                          *
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
 *    object_class:                                                   *
 *        The object_class variable is a character string             *
 *        which contains the pds_class of an object in a PDS label        *
 *        (e.g., "OBJECT = IMAGE" implies that "IMAGE" is the pds_class   *
 *        of the object).                                             *
 *    object_name:                                                    *
 *        The object_name variable is a character string              *
 *        which contains the name of an object in a PDS label.        *
 *        This is assumed to be the value of the "NAME" parameter     *
 *        in the object (e.g., "NAME = VOYAGER" implies that          *
 *        "VOYAGER" is the name of the object).                       *
 *    object_position:                                                *
 *        The object_position variable is an integer which            *
 *        represents the relative position of an object in a PDS      *
 *        label.  If this variable is used in conjunction with either *
 *        the object_class or object_name variables, then it          *
 *        represents a particular occurrence of that thing in the     *
 *        label (e.g., if object_class is "TABLE" and object_position *
 *        is 3, then this represents the third "TABLE" object in the  *
 *        label).  On the other hand, if this variable is used by     *
 *        itself, it represents the absolute position of the object,  *
 *        starting from the "ROOT" object (position = 1).  The        *
 *        sequence follows the structure of the label as it looks     *
 *        in a flat file.                                             *
 *$Outputs                                                            *
 *    label_status:                                                   *
 *        The label_status variable is an integer which is used to    *
 *        indicate the success or failure of a PDS label routine,     *
 *        and whether or not the object or parameter specifications   *
 *        passed in were ambiguous.  Valid values are:  PDS_SUCCESS,  *
 *        PDS_ERROR, PDS_MULTIPLE_OBJECTS, PDS_MULTIPLE_PARMS, and    *
 *        PDS_WARNING.                                                *
 *$Returns                                                            *
 *    object_ptr:                                                     *
 *        The object_ptr variable is a pointer to the structure       *
 *        used to represent an object in a PDS label.                 *
 *$Detailed_Description                                               *
 *    The lab_find_object routine traverses an ODL tree, which        *
 *    represents the structure of a PDS label, and returns a pointer  *
 *    to the object that meets the specifications passed in.          *
 *    The object may by specified by pds_class, name, or position, or     *
 *    by any combination of these.  The search for the object         *
 *    begins at the object pointed to by the label_ptr variable.      *
 *    This is usually the "ROOT" object, but may be any other object  *
 *    in the tree.  Please note that this routine only searches the   *
 *    tree at or below the level of the label_ptr object passed in.   *
 *    Please note that it is not necessary to specify the object's    *
 *    name, pds_class, and position at the same time.  In fact, if you    *
 *    specify only one of them and pass in zero for the others, the   *
 *    routine will work just fine.                                    *
 *                                                                    *
 *    The "pds_class matching" behavior of this routine is determined     *
 *    by setting the global variable pds_generic_class, which is used *
 *    by lab_match_classes to determine if an object pds_class match has  *
 *    been found.  If pds_generic_class is FALSE, then the object     *
 *    pds_class input must exactly match the pds_class of an object in the    *
 *    tree. If it is TRUE, then only a partial match, starting from   *
 *    the end of the pds_class name is needed: a search for pds_class IMAGE   *
 *    will then match both IMAGE and BANDED_IMAGE pds_class objects.      *
 *$Error_Handling                                                     *
 *    1) If the object cannot be found, then label_status is set to   *
 *       PDS_ERROR and a NULL value is returned.                      *
 *    2) If more than one object is found that matches the            *
 *       specifications passed in, then the label_status is set       *
 *       to PDS_MULTIPLE_OBJECTS and a pointer to the FIRST one       *
 *       is returned.                                                 *
 *    3) If everything went according to plan, then the label_status  *
 *       is set to PDS_SUCCESS and a pointer to the object is         *
 *       returned.                                                    *
 *$Side_Effects                                                       *
 *    None                                                            *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    David P. Bernath / J.P.L.                                       *
 *$Version_and_Date                                                   *
 *    1.4   October 30, 1991                                          *
 *$Change_History                                                     *
 *    DPB   04-10-91   Original Code.                                 *
 *    DPB   04-26-91   Added code to strip blanks from inputs.        *
 *    MDD   04-30-91   Changed to use lab_find_object_level rather    *
 *                     than the AGGREGATE -> level field.             *
 *    KLM   06-17-91   Changed call from lab_find_object_level to     *
 *                     lu_find_object_level.                          *
 *    MDD   10-30-91   Changed NextAggregate call to a NextSubAgg..   *
 *                     call, to avoid processing siblings of the      *
 *                     root.                                          *
 **********************************************************************/

AGGREGATE lab_find_object (label_ptr, object_class, 
                           object_name, object_position, label_status)

AGGREGATE label_ptr;
char *object_class;
char *object_name;
int object_position;
int *label_status;

{
    AGGREGATE object_ptr = {NULL};
    AGGREGATE current_object = {NULL};
    PARAMETER parameter_ptr = {NULL};
    int curr_position = {1};
    int root_level = 0;
    LOGICAL done = {FALSE};
    LOGICAL class_found;
    LOGICAL name_found;
    LOGICAL position_found;
    LOGICAL this_is_the_first_one = {TRUE};
    
/** BEGIN **/

    /*-----------------------------------------------------------------------*/
    /** Initialize the label_status variable and level indicator            **/
    /*-----------------------------------------------------------------------*/
    *label_status = PDS_ERROR;
    root_level = lu_find_object_level (label_ptr);

    /*-----------------------------------------------------------------------*/
    /** Remove leading and trailing blanks from character inputs.           **/
    /*-----------------------------------------------------------------------*/

    util_strip_lead_and_trail (object_class, ' ');
    util_strip_lead_and_trail (object_name, ' ');

    /*-----------------------------------------------------------------------*/
    /** Figure out if we should stop right now or continue boldly on.       **/
    /*-----------------------------------------------------------------------*/

    done = ((label_ptr == NULL) || ((object_class == NULL) && 
             (object_name == NULL) && (object_position <= 0)));

    /*-----------------------------------------------------------------------*/
    /** LOOP through the tree until the object is found or the end is nigh  **/
    /*-----------------------------------------------------------------------*/

    for (current_object = label_ptr; 
            ((! done) && (current_object != NULL) &&
                    (lu_find_object_level (current_object) >= root_level));
                              current_object = NextSubAggregate (label_ptr, current_object))
    {

        /*-------------------------------------------------------------------*/
        /** Initialize the three "found" flags.  These are set based on the **/
        /**     values passed in.  If the pds_class or name are NULL, or the    **/
        /**     position is zero, we assume that we have already found an   **/
        /**     object which meets our criteria.  Or, to put it another     **/
        /**     way, if the calling routine did not specify a pds_class, a name **/
        /**     or a position for the object, then any pds_class, name, or      **/
        /**     position will do.                                           **/
        /*-------------------------------------------------------------------*/

        class_found = (object_class == NULL);
        name_found = (object_name == NULL);
        position_found = (object_position <= 0);

        /*-------------------------------------------------------------------*/
        /** IF we have not already found an object whose pds_class matches      **/
        /**         the one passed in THEN                                  **/
        /**     Check the pds_class of the current object.                      **/
        /** ENDIF                                                           **/
        /*-------------------------------------------------------------------*/

        if (! class_found)
        {
            class_found = lab_match_classes (object_class, 
                                             current_object -> name);
        }

        /*-------------------------------------------------------------------*/
        /** IF we've found an object whose pds_class matches the one            **/
        /**         passed in, AND we're supposed to look for the           **/
        /**         name too THEN                                           **/
        /*-------------------------------------------------------------------*/

        if (class_found && ! name_found)
        {
            /*---------------------------------------------------------------*/
            /** Look at the current object and try to find a parameter      **/
            /**     called "NAME"                                           **/
            /*---------------------------------------------------------------*/

            parameter_ptr = FindParameter (current_object, "NAME");

            /*---------------------------------------------------------------*/
            /** IF we have found one THEN                                   **/
            /**     Compare the value of the "NAME" parameter with the      **/
            /**         name passed in.                                     **/
            /** ENDIF                                                       **/
            /*---------------------------------------------------------------*/

            if (parameter_ptr != NULL)
            {
                name_found = (strcmp (object_name, 
                          parameter_ptr->first_value->item.value.string) == 0);
            }

        /*-------------------------------------------------------------------*/
        /** ENDIF                                                           **/
        /*-------------------------------------------------------------------*/

        }  /*  End:  "if (class_found && ..."  */

        /*-------------------------------------------------------------------*/
        /** IF we've found an object with the right pds_class and name, AND     **/
        /**     we're supposed to check its position in the label too THEN  **/
        /*-------------------------------------------------------------------*/

        if (class_found && name_found && ! position_found)
        {
            /*---------------------------------------------------------------*/
            /** Check the position of the current object against the        **/
            /**     against the position passed in.                         **/
            /*---------------------------------------------------------------*/

            position_found = (curr_position == object_position);

            /*---------------------------------------------------------------*/
            /** IF the current object is not at the right position THEN     **/
            /**     We had better increment the position counter, since     **/
            /**         this is not the object we're looking for.           **/
            /** ENDIF                                                       **/
            /*---------------------------------------------------------------*/

            if (! position_found)
                ++curr_position;

        /*-------------------------------------------------------------------*/
        /** ENDIF                                                           **/
        /*-------------------------------------------------------------------*/

        }  /*  End:  "if (class_found && ..."  */

        /*-------------------------------------------------------------------*/
        /** IF (eureka!) we've found an object which meets all of the       **/
        /**         specifications passed in THEN                           **/
        /*-------------------------------------------------------------------*/

        if (class_found && name_found && position_found)
        {
            /*---------------------------------------------------------------*/
            /** IF this is the first object encountered which meets all     **/
            /**         of the specifications THEN                          **/
            /**     Set the label_status to SUCCESS and save a pointer to   **/
            /**         the current object.  If a position was specified,   **/
            /**         then we're done.  There can't be any confusion      **/
            /**         about redundant objects.  On the other hand, if     **/
            /**         a position was not specified, then we've got to     **/
            /**         keep looping through the tree until we find         **/
            /**         another object which satisfies the same criteria,   **/
            /**         reach the end of the tree, or hit an object whose   **/
            /**         level is higher than the label_ptr object passed in **/
            /**         (i.e., it's a sibling of the parent of the          **/
            /**         label_ptr object).                                  **/
            /** ELSE                                                        **/
            /**     We have two objects that meet the specifications passed **/
            /**         in.  Since we can't differentiate between them,     **/
            /**         all we can do is set the label_status value to      **/
            /**         indicate multiple objects, and return a pointer to  **/
            /**         the first one.  We'll let the calling routine       **/
            /**         decide what to do with the other one.               **/
            /** ENDIF                                                       **/
            /*---------------------------------------------------------------*/

            if (this_is_the_first_one)
            {
                *label_status = PDS_SUCCESS;
                object_ptr = current_object;

                if (object_position > 0)
                    done = TRUE;
            }
            else
            {
                *label_status = PDS_MULTIPLE_OBJECTS;
                done = TRUE;
            }

            /*---------------------------------------------------------------*/
            /** Indicate that we've already found one object which meets    **/
            /**     the specifications passed in.                           **/
            /*---------------------------------------------------------------*/

            this_is_the_first_one = FALSE;

        /*-------------------------------------------------------------------*/
        /** ENDIF                                                           **/
        /*-------------------------------------------------------------------*/

        }  /*  End:  "if (class_found && ..."  */

    /*-----------------------------------------------------------------------*/
    /** ENDLOOP                                                             **?
    /*-----------------------------------------------------------------------*/

    }  /*  End:  "for (current_object = label_ptr, ..."  */

    /*-----------------------------------------------------------------------*/
    /** RETURN a pointer to the object found                                **/
    /*-----------------------------------------------------------------------*/

    return (object_ptr);

/** END **/

}  /*  "lab_find_object"  */


/**********************************************************************
 *$Component                                                          *
 *    PARAMETER lab_find_parameter (label_ptr, object_class,          *
 *                                object_name, object_position,       *
 *                                parameter_name, parameter_position, *
 *                                label_status)                       *
 *$Abstract                                                           *
 *    Locates a parameter in a PDS label.                             *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    ADD                                                             *
 *    PARAMETER                                                       *
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
 *    object_class:                                                   *
 *        The object_class variable is a character string             *
 *        which contains the pds_class of an object in a PDS label        *
 *        (e.g., "OBJECT = IMAGE" implies that "IMAGE" is the pds_class   *
 *        of the object).                                             *
 *    object_name:                                                    *
 *        The object_name variable is a character string              *
 *        which contains the name of an object in a PDS label.        *
 *        This is assumed to be the value of the "NAME" parameter     *
 *        in the object (e.g., "NAME = VOYAGER" implies that          *
 *        "VOYAGER" is the name of the object).                       *
 *    object_position:                                                *
 *        The object_position variable is an integer which            *
 *        represents the relative position of an object in a PDS      *
 *        label.  If this variable is used in conjunction with either *
 *        the object_class or object_name variables, then it          *
 *        represents a particular occurrence of that thing in the     *
 *        label (e.g., if object_class is "TABLE" and object_position *
 *        is 3, then this represents the third "TABLE" object in the  *
 *        label).  On the other hand, if this variable is used by     *
 *        itself, it represents the absolute position of the object,  *
 *        starting from the "ROOT" object (position = 1).  The        *
 *        sequence follows the structure of the label as it looks     *
 *        in a flat file.                                             *
 *    parameter_name:                                                 *
 *        The parameter_name variable is a character string           *
 *        which contains the name of a parameter in a PDS label       *
 *        (e.g., the line "SCID = VG1" implies that "SCID" is the     *
 *        parameter name).                                            *
 *    parameter_position:                                             *
 *        The parameter_position variable is an integer which         *
 *        represents the relative position of a parameter in an       *
 *        object in a PDS label.  If this variable is used in         *
 *        conjunction with the parameter_name variable, then it       *
 *        represents a particular occurrence of that parameter        *
 *        in the object (e.g., if parameter_name is "SCID" and        *
 *        parameter_position is 2, then this represents the second    *
 *        "SCID" parameter in the object).  On the other hand, if     *
 *        this variable is used by itself, it represents the absolute *
 *        position of the parameter within the object, starting with  *
 *        first parameter in the object (position = 1).               *
 *$Outputs                                                            *
 *    label_status:                                                   *
 *        The label_status variable is an integer which is used to    *
 *        indicate the success or failure of a PDS label routine,     *
 *        and whether or not the object or parameter specifications   *
 *        passed in were ambiguous.  Valid values are:  PDS_SUCCESS,  *
 *        PDS_ERROR, PDS_MULTIPLE_OBJECTS, PDS_MULTIPLE_PARMS, and    *
 *        PDS_WARNING.                                                *
 *$Returns                                                            *
 *    parameter_ptr:                                                  *
 *        The parameter_ptr variable is a pointer to the structure    *
 *        used to represent a parameter in a PDS label.               *
 *$Detailed_Description                                               *
 *    The lab_find_parameter routine traverses an ODL tree, which     *
 *    represents the structure of a PDS label, finds the object       *
 *    which meets the specifications passed in, and returns a pointer *
 *    to the parameter that meets the specifications passed in.       *
 *    The object may by specified by pds_class, name, or position, or     *
 *    by any combination of these, and the parameter may be specified *
 *    by name or position or both.  The search for the object begins  *
 *    at the object pointed to by the label_ptr variable.  This is    *
 *    usually the "ROOT" object, but may be any other object in the   *
 *    tree.  Please note that it is not necessary to specify the      *
 *    object's name, pds_class, and position at the same time.  In fact,  *
 *    if you specify only one of them and pass in zero for the        *
 *    others, the routine will work just fine.  This also applies to  *
 *    the parameter name and position.                                *
 *$Error_Handling                                                     *
 *    1) If the object or parameter cannot be found, then the         *
 *       label_status variable is set to PDS_ERROR and a NULL value   *
 *       is returned.                                                 *
 *    2) If more than one object is found that matches the            *
 *       specifications passed in for the object, then the            *
 *       label_status is set to PDS_MULTIPLE_OBJECTS and a NULL       *
 *       value is returned.                                           *
 *    3) If more than one parameter is found that matches the         *
 *       specifications passed in for the parameter, then the         *
 *       label_status is set to PDS_MULTIPLE_PARMS and a pointer to   *
 *       the FIRST one is returned.                                   *
 *    4) If everything went according to plan, then the label_status  *
 *       is set to PDS_SUCCESS and a pointer to the parameter is      *
 *       returned.                                                    *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    David P. Bernath / J.P.L.                                       *
 *$Version_and_Date                                                   *
 *    1.1   April 26, 1991                                            *
 *$Change_History                                                     *
 *    DPB   04-10-91   Original Code.                                 *
 *    DPB   04-26-91   Added code to strip blanks from inputs.        *
 **********************************************************************/

PARAMETER lab_find_parameter (label_ptr, object_class, object_name, 
                              object_position, parameter_name, 
                              parameter_position, label_status)

AGGREGATE label_ptr;
char *object_class;
char *object_name;
int object_position;
char *parameter_name;
int parameter_position;
int *label_status;

{
    OBJECT object_ptr = {label_ptr};
    PARAMETER parameter_ptr = {NULL};       
    PARAMETER current_parameter = {NULL};
    int curr_position = {1};
    LOGICAL done = {FALSE};
    LOGICAL name_found;
    LOGICAL position_found;
    LOGICAL this_is_the_first_one = {TRUE};

/** BEGIN **/

    /*-----------------------------------------------------------------------*/
    /** Initialize the label_status variable.                               **/
    /*-----------------------------------------------------------------------*/

    *label_status = PDS_ERROR;

    /*-----------------------------------------------------------------------*/
    /** Remove leading and trailing blanks from character inputs.           **/
    /*-----------------------------------------------------------------------*/

    util_strip_lead_and_trail (object_class, ' ');
    util_strip_lead_and_trail (object_name, ' ');
    util_strip_lead_and_trail (parameter_name, ' ');

    /*-----------------------------------------------------------------------*/
    /** IF any parameter info was passed in THEN                            **/
    /**     Try to find the specified object.                               **/
    /** ENDIF                                                               **/
    /*-----------------------------------------------------------------------*/

    if ((parameter_name != NULL) || (parameter_position > 0))
    {
        object_ptr = lab_find_object (label_ptr, object_class, 
                                      object_name, object_position, label_status
);
    }

    /*-----------------------------------------------------------------------*/
    /** IF the object was found THEN                                        **/
    /*-----------------------------------------------------------------------*/

    if ((object_ptr != NULL) && (*label_status == PDS_SUCCESS))
    {
        /*-------------------------------------------------------------------*/
        /** Re-initialize the label_status variable.                        **/
        /*-------------------------------------------------------------------*/

        *label_status = PDS_ERROR;
            
        /*-------------------------------------------------------------------*/
        /** LOOP through the parameter list until the parameter is found    **/
        /**         or the list has ended.                                  **/
        /*-------------------------------------------------------------------*/

        for (current_parameter = object_ptr -> first_parameter; 
                ((! done) && (current_parameter != NULL));
                    current_parameter = NextParameter (current_parameter))
        {
            /*---------------------------------------------------------------*/
            /** Initialize the two "found" flags.  These are set based on   **/
            /**     the values passed in.  If the name is NULL, or the      **/
            /**     position is zero, we assume that we have already        **/
            /**     found a parameter which meets our criteria.  Or, to     **/
            /**     put it another way, if the calling routine did not      **/
            /**     specify a name or a position, then any name or position **/
            /**     will do.                                                **/
            /*---------------------------------------------------------------*/

            name_found = (parameter_name == NULL);
            position_found = (parameter_position <= 0);

            /*---------------------------------------------------------------*/
            /** IF we haven't already found the parameter THEN              **/
            /**     Compare the current parameter's name with the name      **/
            /**         passed in.                                          **/
            /** ENDIF                                                       **/
            /*---------------------------------------------------------------*/

            if (! name_found)
            {
                name_found = (strcmp (parameter_name, 
                                          current_parameter -> name) == 0);
            }

            /*---------------------------------------------------------------*/
            /** IF we've found a parameter with the right name, AND we're   **/
            /**     supposed to check its position in the object too THEN   **/
            /*---------------------------------------------------------------*/

            if (name_found && ! position_found)
            {
                /*-----------------------------------------------------------*/
                /** Check the position of the current parameter against the **/
                /**     position passed in.                                 **/
                /*-----------------------------------------------------------*/

                position_found = (curr_position == parameter_position);

                /*-----------------------------------------------------------*/
                /** IF the current parameter is not at the right            **/
                /**         position THEN                                   **/
                /**     We had better increment the position counter,       **/
                /**         since this is not the parameter we're looking   **/
                /**         for.                                            **/
                /** ENDIF                                                   **/
                /*-----------------------------------------------------------*/

                if (! position_found)
                    ++curr_position;
    
            /*---------------------------------------------------------------*/
            /** ENDIF                                                       **/
            /*---------------------------------------------------------------*/

            }  /*  End:  "if (name_found && ..."  */

            /*---------------------------------------------------------------*/
            /** IF we've found a parameter which meets all of the           **/
            /**         specifications passed in THEN                       **/
            /*---------------------------------------------------------------*/

            if (name_found && position_found)
            {
                /*-----------------------------------------------------------*/
                /** IF this is the first parameter encountered which meets  **/
                /**         all of the specifications THEN                  **/
                /**     Set the label_status to SUCCESS and save a pointer  **/
                /**         to the current parameter.  If a position was    **/
                /**         specified, then we're done.  There can't be     **/
                /**         any confusion about redundant parameters.  On   **/
                /**         the other hand, if a position was not specified **/
                /**         then we've got to keep looping through the list **/
                /**         of parameters until we find another parameter   **/
                /**         which satisfies the same criteria, or run out   **/
                /**         of parameters to check.                         **/
                /** ELSE                                                    **/
                /**     We have two parameters that meet the specifications **/
                /**         passed in.  Since we can't differentiate        **/
                /**         between them, all we can do is set the          **/
                /**         label_status to indicate multiple paramaters,   **/
                /**         and return a pointer to the first one.  We'll   **/
                /**         let the calling routine decide what to do with  **/
                /**         the other one.                                  **/
                /** ENDIF                                                   **/
                /*-----------------------------------------------------------*/

                if (this_is_the_first_one)
                {
                    *label_status = PDS_SUCCESS;
                    parameter_ptr = current_parameter;

                    if (parameter_position > 0) 
                        done = TRUE;
                }
                else
                {
                    *label_status = PDS_MULTIPLE_PARMS;
                    done = TRUE;
                }

                /*-----------------------------------------------------------*/
                /** Indicate that we've already found one parameter which   **/
                /**     meets the specifications passed in.                 **/
                /*-----------------------------------------------------------*/

                this_is_the_first_one = FALSE;

            /*---------------------------------------------------------------*/
            /** ENDIF                                                       **/
            /*---------------------------------------------------------------*/

            }  /*  End:  "if (name_found && ..."  */

        /*-------------------------------------------------------------------*/
        /** ENDLOOP                                                         **/
        /*-------------------------------------------------------------------*/

        }  /*  End:  "for (current_parameter = ..."  */

    /*-----------------------------------------------------------------------*/
    /** ENDIF                                                               **/
    /*-----------------------------------------------------------------------*/

    }  /*  End:  "if ((parameter_name != NULL) || ..."  */

    /*-----------------------------------------------------------------------*/
    /** RETURN a pointer to the parameter structure                         **/
    /*-----------------------------------------------------------------------*/

    return (parameter_ptr);

/** END **/

}  /*  "lab_find_parameter"  */


/**********************************************************************
 *$Component                                                          *
 *   STRING_LIST *lab_get_all_values (label_ptr, object_class,        *
 *                       object_name, object_position, parameter_name,*
 *                       parameter_position, use_quotes,              *
 *                       label_status)                                *
 *$Abstract                                                           *
 *    Gets all the values of a keyword in a label.                    *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    VALUE                                                           *
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
 *    object_class:                                                   *
 *        The object_class variable is a character string             *
 *        which contains the pds_class of an object in a PDS label        *
 *        (e.g., "OBJECT = IMAGE" implies that "IMAGE" is the pds_class   *
 *        of the object).                                             *
 *    object_name:                                                    *
 *        The object_name variable is a character string              *
 *        which contains the name of an object in a PDS label.        *
 *        This is assumed to be the value of the "NAME" parameter     *
 *        in the object (e.g., "NAME = VOYAGER" implies that          *
 *        "VOYAGER" is the name of the object).                       *
 *    object_position:                                                *
 *        The object_position variable is an integer which            *
 *        represents the relative position of an object in a PDS      *
 *        label.  If this variable is used in conjunction with either *
 *        the object_class or object_name variables, then it          *
 *        represents a particular occurrence of that thing in the     *
 *        label (e.g., if object_class is "TABLE" and object_position *
 *        is 3, then this represents the third "TABLE" object in the  *
 *        label).  On the other hand, if this variable is used by     *
 *        itself, it represents the absolute position of the object,  *
 *        starting from the "ROOT" object (position = 1).  The        *
 *        sequence follows the structure of the label as it looks     *
 *        in a flat file.                                             *
 *    parameter_name:                                                 *
 *        The parameter_name variable is a character string           *
 *        which contains the name of a parameter in a PDS label       *
 *        (e.g., the line "SCID = VG1" implies that "SCID" is the     *
 *        parameter name).                                            *
 *    parameter_position:                                             *
 *        The parameter_position variable is an integer which         *
 *        represents the relative position of a parameter in an       *
 *        object in a PDS label.  If this variable is used in         *
 *        conjunction with the parameter_name variable, then it       *
 *        represents a particular occurrence of that parameter        *
 *        in the object (e.g., if parameter_name is "SCID" and        *
 *        parameter_position is 2, then this represents the second    *
 *        "SCID" parameter in the object).  On the other hand, if     *
 *        this variable is used by itself, it represents the absolute *
 *        position of the parameter within the object, starting with  *
 *        first parameter in the object (position = 1).               *
 *    use_quotes:                                                     *
 *        The use_quotes variable is a true/false flag which indicates*
 *        whether quotes should be included in a string or not.       *
 *$Outputs                                                            *
 *    label_status:                                                   *
 *        The label_status variable is an integer which is used to    *
 *        indicate the success or failure of a PDS label routine,     *
 *        and whether or not the object or parameter specifications   *
 *        passed in were ambiguous.  Valid values are:  PDS_SUCCESS,  *
 *        PDS_ERROR, PDS_MULTIPLE_OBJECTS, PDS_MULTIPLE_PARMS, and    *
 *        PDS_WARNING.                                                *
 *$Returns                                                            *
 *    label_value_list:                                               *
 *        The label_value_list variable is a string list              *
 *        containing all the values of a keyword in a PDS label.      *
 *$Detailed_Description                                               *
 *    The lab_get_all_values routine gets the values of a keyword in  *
 *    a PDS label.  It searches for the parent object of a parameter, *
 *    then searches for the parameter.  The object may be specified   *
 *    by pds_class, name, or position, or by any combination of these.    *
 *    The search for the parent begins at the object pointed to by    *
 *    the label_ptr variable.  This is usually the "ROOT" object, but *
 *    may be any other object in the tree.  Please note that it is    *
 *    not necessary to specify the object's name, pds_class, and          *
 *    position at the same time.  In fact, if you specify only one    *
 *    of them and pass in zero for the others, the routine will work  *
 *    just fine.  The parameter may be specified by name, position,   *
 *    or by both.  A string list is allocated for all the values      *
 *    found.                                                          *
 *                                                                    *
 *    If the use_quotes input is TRUE, then quotes will be included   *
 *    in the value returned if it represents a symbol or character    *
 *    string.                                                         *
 *                                                                    *
 *    See also: lab_get_value                                         *
 *$Error_Handling                                                     *
 *    1) If the object or parameter cannot be found, then the         *
 *       label_status variable is set to PDS_ERROR and a NULL value   *
 *       is returned.                                                 *
 *    2) If more than one object is found that matches the            *
 *       specifications passed in for the object, then the            *
 *       label_status is set to PDS_MULTIPLE_OBJECTS and a NULL       *
 *       value is returned.                                           *
 *    3) If more than one parameter is found that matches the         *
 *       specifications passed in for the parameter, then the         *
 *       label_status is set to PDS_MULTIPLE_PARMS and a NULL value   *
 *       is returned.                                                 *
 *    4) If everything went according to plan, then the label_status  *
 *       is set to PDS_SUCCESS and a pointer to the value is returned.*
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
 *    MDD   04-23-91   Original Code.                                 *
 *    KLM   06-17-91   Changed call from lab_fetch_value to           *
 *                     lu_fetch_value.                                *
 *    MDD   10-21-91   Fixed free statement.                          *
 **********************************************************************/


STRING_LIST *lab_get_all_values (label_ptr, object_class, object_name,
                                 object_position, parameter_name, 
                                 parameter_position, use_quotes, 
                                 label_status)
   AGGREGATE label_ptr;
   char *object_class;
   char *object_name;
   int object_position;
   char *parameter_name;
   int parameter_position;
   LOGICAL use_quotes;
   int *label_status;
{
   STRING_LIST *value_list = NULL;
   PARAMETER parameter_ptr = {NULL};
   char *data_value = NULL;
   VALUE value_ptr;


   /*----------------------------------------------------------------*/
   /** find the specified parameter                                 **/
   /*----------------------------------------------------------------*/

   *label_status = PDS_ERROR;
   parameter_ptr = lab_find_parameter (label_ptr, object_class, 
                                        object_name, object_position, 
                                        parameter_name, parameter_position, 
                                                   label_status);
   /*----------------------------------------------------------------*/
   /** IF the parameter was found THEN                              **/
   /**   LOOP through the values of the parameter                   **/
   /**      fetch the value                                         **/
   /**      add it to the string list                               **/
   /**   ENDLOOP    
   /*----------------------------------------------------------------*/

   if (parameter_ptr != NULL && *label_status == PDS_SUCCESS)
      {
      for (value_ptr = FirstValue (parameter_ptr); value_ptr != NULL;
              value_ptr = NextValue (value_ptr)) 
      {
         data_value = lu_fetch_value (value_ptr, use_quotes);
         if (data_value != NULL)
         {
            value_list = util_append_string_list (value_list, data_value, 
                                                     STRING_TYPE);
            Lemme_Go(data_value);
	 }
      }
   }
   /*----------------------------------------------------------------*/
   /** ENDIF the parameter was found...                             **/
   /*----------------------------------------------------------------*/

   return (value_list);

   /** END lab_get_all_values **/
}




/**********************************************************************
 *$Component                                                          *
 *   POINTER_INFO *lab_get_pointer (label_ptr, pointer_name,          *
 *                                  pointer_position, label_status)   *
 *$Abstract                                                           *
 *    Gets a pointer value from a PDS label.                          *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    POINTER                                                         *
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
 *    pointer_name:                                                   *
 *        The pointer_name variable is a character string             *
 *        which contains the name of a pointer in a PDS label         *
 *        (e.g., the line "^TABLE = "TABLE.DAT" implies that "^TABLE" *
 *        is the pointer_name.  The caret must NOT be included.       *
 *    pointer_position:                                               *
 *        The pointer_position variable is an integer which           *
 *        represents the relative position of a pointer in an         *
 *        object in a PDS label.  If this variable is used in         *
 *        conjunction with the pointer_name variable, then it         *
 *        represents a particular occurrence of that pointer          *
 *        in the object (e.g., if pointer_name is "^TABLE" and        *
 *        pointer_position   is 2, then this represents the second    *
 *        "^TABLE" pointer in the object).  On the other hand, if     *
 *        this variable is used by itself, it represents the absolute *
 *        position of the parameter within the object, starting with  *
 *        first parameter in the object (position = 1), counting all  *
 *        parameters, not just pointers.                              *
 *$Outputs                                                            *
 *    label_status:                                                   *
 *        The label_status variable is an integer which is used to    *
 *        indicate the success or failure of a PDS label routine,     *
 *        and whether or not the object or parameter specifications   *
 *        passed in were ambiguous.  Valid values are:  PDS_SUCCESS,  *
 *        PDS_ERROR, PDS_MULTIPLE_OBJECTS, PDS_MULTIPLE_PARMS, and    *
 *        PDS_WARNING.                                                *
 *$Returns                                                            *
 *    pointer_info:                                                   *
 *        The pointer_info variable is the address of a POINTER_INFO  *
 *        structure, which contains information about a label pointer,*
 *        indicating the file it points to, the location of an object *
 *        within the file, the type of the location (record or bytes) *
 *        and whether the data file is attached or detached from the  *
 *        label.                                                      *
 *$Detailed_Description                                               *
 *    The lab_get_pointer routine searches the ROOT object in the     *
 *    given label tree for the specified pointer (according to        *
 *    the search rules used by the PDS label library).  If the ROOT   *
 *    object contains the pointer, then the pointer_info output       *
 *    structure is assigned the appropriate location, file_name, and  *
 *    flag values.  If the pointer is found, this routine returns     *
 *    TRUE. If the pointer is not found, then this routine returns    *
 *    FALSE, and the pointer_info structure is set to the default:    *
 *    empty string as file_name, location = 1, has_byte_loc = FALSE,  *
 *    and is_attached = TRUE.                                         *
 *$Error_Handling:                                                    *
 *    For label_status values, see lab_get_value.                     *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore/JPL                                             *
 *$Version_and_Date                                                   *
 *    1.0   October 10, 1991                                          *
 *$Change_History                                                     *
 *    MDD   10-10-91   Original Code.                                 *
 **********************************************************************/


POINTER_INFO *lab_get_pointer (label_ptr, pointer_name, pointer_position,
                               label_status)

AGGREGATE label_ptr;
char *pointer_name;
int pointer_position;
int *label_status;

{
   char *temp = NULL;
   PARAMETER keyword_ptr = NULL;
   VALUE value_ptr;
   POINTER_INFO *pointer_info = NULL;

/** BEGIN **/

   /*------------------------------------------------------------------*/
   /** Look for the pointer in the ROOT object                        **/
   /** IF it was found THEN                                           **/
   /*------------------------------------------------------------------*/

   keyword_ptr = lab_find_parameter (label_ptr, "ROOT", NULL, 1,
                                     pointer_name, pointer_position, label_status);

   if (keyword_ptr != NULL && *label_status == PDS_SUCCESS)
   {

      /*------------------------------------------------------------------*/
      /** Initialize the pointer structure to default values             **/
      /*------------------------------------------------------------------*/

      pointer_info = (POINTER_INFO *) malloc (sizeof (POINTER_INFO));
      Check_Malloc(pointer_info);
      strcpy (pointer_info -> name, pointer_name);
      pointer_info -> has_byte_loc = FALSE;
      pointer_info -> is_attached = TRUE;
      pointer_info -> location = 1;
      strcpy (pointer_info -> file_name, "");

      /*---------------------------------------------------------------*/
      /** IF the pointer has at least one value THEN                  **/
      /*---------------------------------------------------------------*/

      value_ptr = FirstValue (keyword_ptr);
      if (value_ptr != NULL)
      {
          /*-----------------------------------------------------------*/
          /** set the is_attached flag if the value is an integer     **/
          /** get the actual value                                    **/
          /*-----------------------------------------------------------*/

          pointer_info -> is_attached =
                           (value_ptr -> item.type == TV_INTEGER);
          temp = lu_fetch_value (value_ptr, FALSE);

          /*-----------------------------------------------------------*/
          /** IF the pointer is to an attached data file THEN         **/
          /**    the first value is a file name, so store it, and get **/
          /**    the next value                                       **/
          /** ENDIF                                                   **/
          /*-----------------------------------------------------------*/

          if (!pointer_info -> is_attached)
          {
             strcpy (pointer_info -> file_name, temp);
             Lemme_Go(temp);
             value_ptr = NextValue (value_ptr);
             temp = lu_fetch_value (value_ptr, FALSE);
          }
          /*-----------------------------------------------------------*/
          /** Next value is the location of the object, so store it   **/
          /** Get the units for the value                             **/
          /** set the has_byte_loc flag according to the units        **/
          /*-----------------------------------------------------------*/

          if (temp != NULL)
          {
             pointer_info -> location = Make_Long(temp);
             Lemme_Go(temp);
             temp = lu_format_units (value_ptr -> item.value.integer.units);
             if (temp != NULL)
                pointer_info -> has_byte_loc =
                                    (strcmp (temp, "<BYTES>") == 0 ||
                                     strcmp (temp, "<BYTE>") == 0);
          }
          Lemme_Go(temp);
      }
      /*---------------------------------------------------------------*/
      /** ELSE                                                        **/
      /*---------------------------------------------------------------*/

      else
      {
         *label_status = PDS_ERROR;
         Lemme_Go(pointer_info);
      }

      /*---------------------------------------------------------------*/
      /** ENDIF the pointer has at least one value...                 **/
      /*---------------------------------------------------------------*/
   }
   /*------------------------------------------------------------------*/
   /** ENDIF it was found...                                          **/
   /*------------------------------------------------------------------*/

   return (pointer_info);

/** END **/
}



/**********************************************************************
 *$Component                                                          *
 *   char *lab_get_value (label_ptr, object_class, object_name,       *
 *                        object_position, parameter_name,            *
 *                        parameter_position, value_position,         *
 *                        use_quotes, label_status)                   *
 *$Abstract                                                           *
 *    Gets a value of a keyword in a PDS label.                       *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    VALUE                                                           *
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
 *    object_class:                                                   *
 *        The object_class variable is a character string             *
 *        which contains the pds_class of an object in a PDS label        *
 *        (e.g., "OBJECT = IMAGE" implies that "IMAGE" is the pds_class   *
 *        of the object).                                             *
 *    object_name:                                                    *
 *        The object_name variable is a character string              *
 *        which contains the name of an object in a PDS label.        *
 *        This is assumed to be the value of the "NAME" parameter     *
 *        in the object (e.g., "NAME = VOYAGER" implies that          *
 *        "VOYAGER" is the name of the object).                       *
 *    object_position:                                                *
 *        The object_position variable is an integer which            *
 *        represents the relative position of an object in a PDS      *
 *        label.  If this variable is used in conjunction with either *
 *        the object_class or object_name variables, then it          *
 *        represents a particular occurrence of that thing in the     *
 *        label (e.g., if object_class is "TABLE" and object_position *
 *        is 3, then this represents the third "TABLE" object in the  *
 *        label).  On the other hand, if this variable is used by     *
 *        itself, it represents the absolute position of the object,  *
 *        starting from the "ROOT" object (position = 1).  The        *
 *        sequence follows the structure of the label as it looks     *
 *        in a flat file.                                             *
 *    parameter_name:                                                 *
 *        The parameter_name variable is a character string           *
 *        which contains the name of a parameter in a PDS label       *
 *        (e.g., the line "SCID = VG1" implies that "SCID" is the     *
 *        parameter name).                                            *
 *    parameter_position:                                             *
 *        The parameter_position variable is an integer which         *
 *        represents the relative position of a parameter in an       *
 *        object in a PDS label.  If this variable is used in         *
 *        conjunction with the parameter_name variable, then it       *
 *        represents a particular occurrence of that parameter        *
 *        in the object (e.g., if parameter_name is "SCID" and        *
 *        parameter_position is 2, then this represents the second    *
 *        "SCID" parameter in the object).  On the other hand, if     *
 *        this variable is used by itself, it represents the absolute *
 *        position of the parameter within the object, starting with  *
 *        first parameter in the object (position = 1).               *
 *   value_position:                                                  *
 *        The value_position variable is an integer which represents  *
 *        the position of a value in a keyword/value statement in a   *
 *        PDS label.  For example, the statement ``SPACECRAFT_ID =    *
 *        (VG1, MGN)" has two values with positions 1 and 2. If only  *
 *        a single value is present, then it has position 1.          *
 *    use_quotes:                                                     *
 *        The use_quotes variable is a true/false flag which indicates*
 *        whether quotes should be included in a string or not.       *
 *$Outputs                                                            *
 *    label_status:                                                   *
 *        The label_status variable is an integer which is used to    *
 *        indicate the success or failure of a PDS label routine,     *
 *        and whether or not the object or parameter specifications   *
 *        passed in were ambiguous.  Valid values are:  PDS_SUCCESS,  *
 *        PDS_ERROR, PDS_MULTIPLE_OBJECTS, PDS_MULTIPLE_PARMS, and    *
 *        PDS_WARNING.                                                *
 *$Returns                                                            *
 *    label_value_ptr:                                                *
 *        The label_value_ptr variable is a character string list     *
 *        containing the value of a keyword in a PDS label.           *
 *$Detailed_Description                                               *
 *    The lab_get_value routine returns the value of a keyword in a   *
 *    PDS label.  It searches for the parent object of a parameter,   *
 *    then searches for the parameter, and then finally searches for  *
 *    the value with the given position. The object may be specified  *
 *    by pds_class, name, or position, or by any combination of these.    *
 *    The search for the parent begins at the object pointed to by    *
 *    the label_ptr variable.  This is usually the "ROOT" object, but *
 *    may be any other object in the tree.  Please note that it is    *
 *    not necessary to specify the object's name, pds_class, and          *
 *    position at the same time.  In fact, if you specify only one    *
 *    of them and pass in zero for the others, the routine will work  *
 *    just fine.  The parameter may be specified by name, position,   *
 *    or by both.                                                     *
 *                                                                    *
 *    If the use_quotes input is TRUE, then quotes will be included   *
 *    in the value returned if it represents a symbol or character    *
 *    string.                                                         *
 *                                                                    *
 *    Note that string values which originally contained the PDS      *
 *    escape characters \n and \t will be returned with these         *
 *    sequences stored as TWO characters: the backslash and then the  *
 *    n or t.  Any REAL tabs or newlines in the original label have   *
 *    been removed.  To replace the two character sequences with real *
 *    C escape sequences for TAB and NEWLINE, see the routine         *
 *    util_replace_formatters.                                        *
 *$Error_Handling                                                     *
 *    1) If the object or parameter cannot be found, then the         *
 *       label_status variable is set to PDS_ERROR and a NULL value   *
 *       is returned.                                                 *
 *    2) If more than one object is found that matches the            *
 *       specifications passed in for the object, then the            *
 *       label_status is set to PDS_MULTIPLE_OBJECTS and a NULL       *
 *       value is returned.                                           *
 *    3) If more than one parameter is found that matches the         *
 *       specifications passed in for the parameter, then the         *
 *       label_status is set to PDS_MULTIPLE_PARMS and a NULL value   *
 *       is returned.                                                 *
 *    4) If everything went according to plan, then the label_status  *
 *       is set to PDS_SUCCESS and a pointer to the value is returned.*
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
 *    KLM   06-17-91   Changed call from lab_fetch_value to           *
 *                     lu_fetch_value.                                *
 **********************************************************************/

char *lab_get_value (label_ptr, object_class, object_name,
                     object_position, parameter_name, parameter_position,
                     value_position, use_quotes, label_status)

AGGREGATE label_ptr;
char *object_class;
char *object_name;
int object_position;
char *parameter_name;
int parameter_position;
int value_position;
LOGICAL use_quotes;
int *label_status;

{
    PARAMETER parameter_ptr = {NULL};
    char *data_value = NULL;
    VALUE value_ptr = NULL;
    int i;

/** BEGIN **/
    /*----------------------------------------------------------------*/
    /** find the specified parameter                                 **/
    /*----------------------------------------------------------------*/

    *label_status = PDS_ERROR;
    parameter_ptr = lab_find_parameter (label_ptr, object_class, 
                                         object_name, object_position, 
                                         parameter_name, parameter_position, 
                                                   label_status);

    /*----------------------------------------------------------------*/
    /** IF the parameter was found THEN                              **/
    /**   find the value with the given position                     **/
    /**   fetch that value                                           **/
    /*----------------------------------------------------------------*/

    if (parameter_ptr != NULL && *label_status == PDS_SUCCESS)
    {
       value_ptr = FirstValue (parameter_ptr);
       for (i = 1; i < value_position; i++) 
       {
          value_ptr = NextValue (value_ptr);
       }
       data_value = lu_fetch_value (value_ptr, use_quotes);
    }

    /*----------------------------------------------------------------*/
    /** ENDIF the parameter was found...                             **/
    /** reset the label_status to error if no value was found        **/
    /*----------------------------------------------------------------*/

    if (value_ptr == NULL || data_value == NULL)
    {
       *label_status = PDS_ERROR;
    }
    return (data_value);

/** END lab_get_value **/
}


/**********************************************************************
 *$Component                                                          *
 *    LOGICAL lab_match_classes (primary_object_class,                *
 *                               compared_object_class)               *
 *$Abstract                                                           *
 *    Compares two object classes                                     *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    OBJECT                                                          *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    primary_object_class:                                           *
 *        The primary_object_class variable is a character string     *
 *        which contains the primary object pds_class, e.g. the object    *
 *        pds_class against which all other classes are to be compared.   *
 *    compared_object_class:                                          *
 *        The compared_object_class variable is a character string    *
 *        which contains the object pds_class to be compare against       *
 *        the primary pds_class.  This pds_class may be compared whole, or    *
 *        it may be compared piece by piece, depending on whether     *
 *        or not the global generic pds_class flag is set.                *
 *$Outputs                                                            *
 *    NONE                                                            *
 *$Returns                                                            *
 *    success_flag:                                                   *
 *        The success_flag variable is a general purpose variable     *
 *        which indicates success (a value of TRUE) or failure        *
 *        (a value of FALSE).                                         *
 *$Detailed_Description                                               *
 *    This routine takes the primary pds_class passed in and compares it  *
 *    against the compared pds_class.  If they match then TRUE is         *
 *    returned.  A match is determined in the following way:          *
 *    First, all trailing numbers and underscores are stripped from   *
 *    the compared pds_class.   Then, the remaining string is compared    *
 *    against the primary pds_class.  If the global generic pds_class flag is *
 *    not set, then this comparison is performed on the entire string.*
 *    Otherwise, the primary pds_class is treated as a generic pds_class and  *
 *    it is compared against the other string piece by piece.         *
 *$Error_Handling                                                     *
 *    None.                                                           *
 *$Side_Effects                                                       *
 *    This routine causes memory to be allocated which must be freed  *
 *    by the calling routine.                                         *
 *$External_References                                                *
 *    Item                     Shared-Data           Access           *
 *    ------------------------------------------------------------    *
 *    pds_generic_class        pdsglob.h             read             *
 *$Author_and_Institution                                             *
 *    David P. Bernath / J. P. L.                                     *
 *$Version_and_Date                                                   *
 *    1.1   October 21, 1991                                          *
 *$Change_History                                                     *
 *    DPB   07-24-91   Original Code.                                 *
 *    MDD   10-21-91   Fixe malloc, free, and sys_exit_system calls   *
 **********************************************************************/

LOGICAL lab_match_classes (primary_object_class, compared_object_class)

char *primary_object_class;
char *compared_object_class;

{
    char *temp_class= {NULL};
    char *c = {NULL};
    LOGICAL found = {FALSE};

/** BEGIN **/

    /*-----------------------------------------------------------------------*/
    /** IF both pieces of information were passed in THEN                   **/
    /*-----------------------------------------------------------------------*/

    if ((primary_object_class != NULL) && (compared_object_class != NULL))
    {
        /*-------------------------------------------------------------------*/
        /** Prepare a temporary string.                                     **/
        /*-------------------------------------------------------------------*/

        Malloc_String(temp_class, (int) String_Size(compared_object_class));
        strcpy (temp_class, compared_object_class);

	/*-------------------------------------------------------------------*/
	/** IF the global generic pds_class flag is not set THEN                **/
	/**     The classes match if a direct comparison succeeds.          **/
	/** ELSE                                                            **/
	/*-------------------------------------------------------------------*/

        if (! pds_generic_class)
            found = (strcmp (primary_object_class, temp_class) == 0);
        else
	{
	   /*-------------------------------------------------------------------*/
	   /** Remove any trailing numbers and underscores.                    **/
	   /*-------------------------------------------------------------------*/

	   for (c = String_End(temp_class);
		((c >= temp_class) && (isdigit(*c) || (*c == '_'))); --c) ;
	   *(++c) = EOS;


	    /*---------------------------------------------------------------*/
	    /** The classes match when the primary pds_class matches a piece    **/
	    /**     of the compared pds_class.  These pieces start from the     **/
	    /**     left and go to the end of the string.                   **/
            /*---------------------------------------------------------------*/

            for (c = temp_class; 
                    ((*c != EOS) && (strcmp (primary_object_class, c) != 0)); )
            {
                c = strchr (c, (int) ('_'));

                if (c == NULL)
                    c = String_End(temp_class);

                ++c;
    
            }  /*  End:  "for (c = temp_class, ..."  */

            found = (*c != EOS);

        /*-------------------------------------------------------------------*/
        /** ENDIF                                                           **/
        /*-------------------------------------------------------------------*/

        }  /*  End:  "if (! pds_temp_class) ... else ..."  */

        /*-------------------------------------------------------------------*/
        /** Free local storage.                                             **/
        /*-------------------------------------------------------------------*/

        Lemme_Go(temp_class);
        
    /*-----------------------------------------------------------------------*/
    /** ENDIF                                                               **/
    /*-----------------------------------------------------------------------*/

    }  /*  End:  "if (object_class == NULL) ... else ..."  */

    /*-----------------------------------------------------------------------*/
    /** RETURN the success flag.                                            **/
    /*-----------------------------------------------------------------------*/

    return (found);

/** END **/

}  /*  "lab_match_classes"  */


/**********************************************************************
 *$Component                                                          *
 *    PARAMETER lab_move_parameter (label_ptr, object_class,          *
 *                         object_name, object_position,              *
 *                         parameter_name, parameter_position,        *
 *                         new_position, label_status)                *
 *$Abstract                                                           *
 *    Moves a parameter in a PDS Label.                               *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    PARAMETER                                                       *
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
 *    object_class:                                                   *
 *        The object_class variable is a character string             *
 *        which contains the pds_class of an object in a PDS label        *
 *        (e.g., "OBJECT = IMAGE" implies that "IMAGE" is the pds_class   *
 *        of the object).                                             *
 *    object_name:                                                    *
 *        The object_name variable is a character string              *
 *        which contains the name of an object in a PDS label.        *
 *        This is assumed to be the value of the "NAME" parameter     *
 *        in the object (e.g., "NAME = VOYAGER" implies that          *
 *        "VOYAGER" is the name of the object).                       *
 *    object_position:                                                *
 *        The object_position variable is an integer which            *
 *        represents the relative position of an object in a PDS      *
 *        label.  If this variable is used in conjunction with either *
 *        the object_class or object_name variables, then it          *
 *        represents a particular occurrence of that thing in the     *
 *        label (e.g., if object_class is "TABLE" and object_position *
 *        is 3, then this represents the third "TABLE" object in the  *
 *        label).  On the other hand, if this variable is used by     *
 *        itself, it represents the absolute position of the object,  *
 *        starting from the "ROOT" object (position = 1).  The        *
 *        sequence follows the structure of the label as it looks     *
 *        in a flat file.                                             *
 *    parameter_name:                                                 *
 *        The parameter_name variable is a character string           *
 *        which contains the name of a parameter in a PDS label       *
 *        (e.g., the line "SCID = VG1" implies that "SCID" is the     *
 *        parameter name).                                            *
 *    parameter_position:                                             *
 *        The parameter_position variable is an integer which         *
 *        represents the relative position of a parameter in an       *
 *        object in a PDS label.  If this variable is used in         *
 *        conjunction with the parameter_name variable, then it       *
 *        represents a particular occurrence of that parameter        *
 *        in the object (e.g., if parameter_name is "SCID" and        *
 *        parameter_position is 2, then this represents the second    *
 *        "SCID" parameter in the object).  On the other hand, if     *
 *        this variable is used by itself, it represents the absolute *
 *        position of the parameter within the object, starting with  *
 *        first parameter in the object (position = 1).               *
 *    new_position:                                                   *
 *        The new_position variable is the same as either             *
 *        object_position or parameter_position, depending upon       *
 *        context, and is used when either an object or parameter is  *
 *        to be moved to a new location within the parent object      *
 *        an a PDS label.                                             *
 *$Outputs                                                            *
 *    label_status:                                                   *
 *        The label_status variable is an integer which is used to    *
 *        indicate the success or failure of a PDS label routine,     *
 *        and whether or not the object or parameter specifications   *
 *        passed in were ambiguous.  Valid values are:  PDS_SUCCESS,  *
 *        PDS_ERROR, PDS_MULTIPLE_OBJECTS, PDS_MULTIPLE_PARMS, and    *
 *        PDS_WARNING.                                                *
 *$Returns                                                            *
 *    parameter_ptr:                                                  *
 *        The parameter_ptr variable is a pointer to the structure    *
 *        used to represent a parameter in a PDS label.               *
 *$Detailed_Description                                               *
 *    The lab_move_parameter routine will move the specified          *
 *    parameter to a new location in its parent object.  The parent   *
 *    object and the parameter are located using the standard search  *
 *    rules for the PDS Label Library.  The parameter is then         *
 *    relocated to the absolute position specified by new_position.   *
 *$Error_Handling                                                     *
 *    1) If the object or parameter cannot be found, or memory cannot *
 *       be allocated for the new value, then the label_status        *
 *       is set to PDS_ERROR and a NULL value is returned.            *
 *    2) If more than one object is found that matches the            *
 *       specifications passed in for the object, then the            *
 *       label_status is set to PDS_MULTIPLE_OBJECTS and a NULL       *
 *       value is returned.                                           *
 *    3) If more than one parameter is found that matches the         *
 *       specifications passed in for the parameter, then the         *
 *       label_status is set to PDS_MULTIPLE_PARMS and a NULL         *
 *       value is returned.                                           *
 *    4) If everything went according to plan, then the label_status  *
 *       is set to PDS_SUCCESS and a pointer to the new parameter is  *
 *       returned.                                                    *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore/ J.P.L                                          *
 *$Version_and_Date                                                   *
 *    1.0   November 4, 1991                                          *
 *$Change_History                                                     *
 *    MDD   11-04-91   Original Code.                                 *
 **********************************************************************/

PARAMETER lab_move_parameter (label_ptr, object_class, object_name, object_position,
                              parameter_name, parameter_position, new_position,
                              label_status)

AGGREGATE label_ptr;
char *object_class;
char *object_name;
int object_position;
char *parameter_name;
int parameter_position;
int new_position;
int *label_status;
{
   AGGREGATE object_ptr;
   PARAMETER parameter_ptr;

/** BEGIN **/

   /*----------------------------------------------------------------------*/
   /** IF the parameter to be moved can be found THEN                     **/
   /**    cut it from its former position                                 **/
   /**    paste it at the new location                                    **/
   /** ENDIF                                                              **/
   /*----------------------------------------------------------------------*/

   parameter_ptr = lab_find_parameter (label_ptr, object_class, object_name,
                   object_position, parameter_name, parameter_position,
                   label_status);
   if (*label_status == PDS_SUCCESS && parameter_ptr != NULL)
   {
      object_ptr = parameter_ptr -> owner;
      parameter_ptr = CutParameter(parameter_ptr);
      lu_paste_parm_at_pos(object_ptr, parameter_ptr, new_position);
   }
   return (parameter_ptr);

/** END **/
}



/**********************************************************************
 *$Component                                                          *
 *    void lab_print_messages ()                                      *
 *$Abstract                                                           *
 *    Prints the contents of the global message list.                 *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    ERROR1                                                          *
 *    PRINT                                                           *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    None                                                            *
 *$Outputs                                                            *
 *    None                                                            *
 *$Returns                                                            *
 *    None                                                            *
 *$Detailed_Description                                               *
 *    The lab_print_messages routine displays the contents of the     *
 *    global message list.                                            *
 *$Error_Handling                                                     *
 *    None                                                            *
 *$Side_Effects                                                       *
 *    None                                                            *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    David P. Bernath / J.P.L.                                       *
 *$Version_and_Date                                                   *
 *    1.0   April 10, 1991                                            *
 *$Change_History                                                     *
 *    DPB   04-10-91   Original Code.                                 *
 **********************************************************************/

void lab_print_messages ()

{
    err_write_to_file (NULL, TRUE);

}  /*  lab_print_messages"  */



/**********************************************************************
 *$Component                                                          *
 *    LOGICAL lab_print_label (label_ptr, label_status)               *
 *$Abstract                                                           *
 *    Prints out a PDS Label to the screen.                           *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    PRINT                                                           *
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
 *$Outputs                                                            *
 *    label_status:                                                   *
 *        The label_status variable is an integer which is used to    *
 *        indicate the success or failure of a PDS label routine,     *
 *        and whether or not the object or parameter specifications   *
 *        passed in were ambiguous.  Valid values are:  PDS_SUCCESS,  *
 *        PDS_ERROR, PDS_MULTIPLE_OBJECTS, PDS_MULTIPLE_PARMS, and    *
 *        PDS_WARNING.                                                *
 *$Returns                                                            *
 *    success_flag:                                                   *
 *        The success_flag variable is a general purpose variable     *
 *        which indicates success (a value of TRUE) or failure        *
 *        (a value of FALSE).                                         *
 *$Detailed_Description                                               *
 *    The lab_print_label routine displays the contents of an         *
 *    ODL tree in the format of a PDS label.                          *
 *$Error_Handling                                                     *
 *    If the input label_ptr is NULL, label_status is set to          *
 *    PDS_ERROR. Otherwise, it is set to PDS_SUCCESS.                 *
 *$Side_Effects                                                       *
 *    None                                                            *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Herbert C. Gamble / J.P.L.                                      *
 *    David P. Bernath / J.P.L.                                       *
 *$Version_and_Date                                                   *
 *    2.0   June 19, 1992                                             *
 *$Change_History                                                     *
 *    HCG   03-08-91   Original Code.                                 *
 *    DPB   04-10-91   Restructured routine.                          *
 *    MDD   06-19-92   Changed name and added label status output     *
 **********************************************************************/

LOGICAL lab_print_label (label_ptr, label_status)

AGGREGATE label_ptr;
int *label_status;

{
    if (label_ptr != NULL)
        PrintLabel(label_ptr);

    *label_status = (label_ptr != NULL) ? PDS_SUCCESS : PDS_ERROR;

    return (label_ptr != NULL);

}  /*  "lab_print_label"  */


/**********************************************************************
 *$Component                                                          *
 *    AGGREGATE lab_read_label (input_fname, label_status)            *
 *$Abstract                                                           *
 *    Builds an ODL tree from a label file.                           *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    OBJECT                                                          *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    input_fname:                                                    *
 *        The input_fname variable is the name of the file that       *
 *        contains the input to be processed by a routine.            *
 *$Outputs                                                            *
 *    label_status:                                                   *
 *        The label_status variable is an integer which is used to    *
 *        indicate the success or failure of a PDS label routine,     *
 *        and whether or not the object or parameter specifications   *
 *        passed in were ambiguous.  Valid values are:  PDS_SUCCESS,  *
 *        PDS_ERROR, PDS_MULTIPLE_OBJECTS, PDS_MULTIPLE_PARMS, and    *
 *        PDS_WARNING.                                                *
 *$Returns                                                            *
 *    label_ptr:                                                      *
 *        The label_ptr variable is a pointer to the root of an       *
 *        ODL tree.  Usually it points to the actual "ROOT" object,   *
 *        but it may point to any other object in the tree.  If it    *
 *        does point to something other than the "ROOT" object, then  *
 *        this object will be treated as the root of a sub-tree, and  *
 *        processing will not be allowed to move above the level of   *
 *        this object.                                                *
 *$Detailed_Description                                               *
 *    The lab_read_label routine reads a file containing              *
 *    a PDS label and returns a pointer to an ODL tree which          *
 *    represents the label.                                           *
 *$Error_Handling                                                     *
 *    If the input file cannot be opened, or if the tree cannot be    *
 *    created, or if there is nothing in the input file, then a       *
 *    NULL pointer is returned, and the label_status is set to        *
 *    PDS_ERROR. If something was read from the file and a tree was   *
 *    created, but errors have been appended to the message list,     *
 *    then the label_status is set to PDS_WARNING.                    *
 *$Side_Effects                                                       *
 *    This routine causes memory to be allocated which must be freed  *
 *    by the calling routine.                                         *
 *$External_References                                                *
 *    Item                     Shared-Data           Access           *
 *    ------------------------------------------------------------    *
 *    ODLroot_node             odldef.h              update           *
 *    pds_last_message         pdsglob.h             read             *
 *    pds_use_sfdus            pdsglob.h             read             *
 *$Author_and_Institution                                             *
 *    David P. Bernath / J.P.L.                                       *
 *$Version_and_Date                                                   *
 *    2.1   September 10, 1992                                        *
 *$Change_History                                                     *
 *    DPB   04-10-91   Original Code.                                 *
 *    MDD   04-20-92   Added code to look for file in label root if   *
 *                     it's not found right away.                     *
 *    MDD   06-01-92   Added code to locate and skip over SFDUs       *
 *                     changed name to lab_read_label, and added the  *
 *                     label_status output.                           *
 *    MDD   09-10-92   Added code to look for lower case version of   *
 *                     label file name if original is not found.      *
 **********************************************************************/

AGGREGATE lab_read_label (input_fname, label_status) 

char *input_fname;
int *label_status;
{
    AGGREGATE label_ptr = {NULL};
    FILE *file_ptr = {NULL};
    char *temp_name;
    char buffer [PDS_MAXLINE + 1];
    ERROR_LIST *temp_msg = pds_last_message;
    int local_status = PDS_ERROR;
    char lower_name [PDS_MAXLINE + 1];

/** BEGIN **/

    /*-----------------------------------------------------------------------*/
    /** Initialize the global ODL root pointer                              **/
    /*-----------------------------------------------------------------------*/

    ODLroot_node = NULL;

    /*-----------------------------------------------------------------------*/
    /** Try to open the input file as specified.                            **/
    /** IF it wasn't found THEN try to open it in the label root directory  **/
    /*-----------------------------------------------------------------------*/
    file_ptr = fopen (input_fname, "r");
    if (file_ptr == NULL)
    {
       strcpy (lower_name, input_fname);
       util_lower_case (lower_name);
       file_ptr = fopen (lower_name, "r");
       if (file_ptr != NULL)
           err_append_message (CONTINUE, 
             "Label file was found only after name was converted to lower case");
    }
    if (file_ptr == NULL)
    {
       temp_name = util_create_file_spec (PDS_LABEL_ROOT, input_fname);
       file_ptr = fopen (temp_name, "r");
       if (file_ptr == NULL)
       {
          strcpy (lower_name, temp_name);
          util_lower_case (lower_name);
          file_ptr = fopen (lower_name, "r");
          if (file_ptr != NULL)
              err_append_message (CONTINUE, 
                "Label file was found only after name was converted to lower case");
          Lemme_Go(temp_name);
       }
    }

    /*-----------------------------------------------------------------------*/
    /** IF the file could be opened THEN                                    **/
    /*-----------------------------------------------------------------------*/

    if (file_ptr != NULL)
    {
	/*-------------------------------------------------------------------*/
	/** Try to create a ROOT node for the new tree.                     **/
	/*-------------------------------------------------------------------*/

	label_ptr = NewAggregate (NULL, KA_OBJECT, "ROOT", "");

	/*-------------------------------------------------------------------*/
	/** IF the root was created THEN                                    **/
	/*-------------------------------------------------------------------*/

	if (label_ptr != NULL)
	{
	    /*---------------------------------------------------------------*/
	    /** Locate and skip over the SFDU labels in the file.           **/
	    /*---------------------------------------------------------------*/

            if (pds_use_sfdus)
              {
	          label_ptr -> sfdu_list = lu_locate_sfdus (file_ptr);
	          if (label_ptr -> sfdu_list != NULL)
		        fread (buffer, 1, label_ptr -> sfdu_list -> begin_offset,
		                                                       file_ptr);
	          }

	    /*---------------------------------------------------------------*/
	    /** Try to read and parse the label and build the ODL tree.     **/
	    /*---------------------------------------------------------------*/

	    ReadLabel (file_ptr, label_ptr);

	    /*---------------------------------------------------------------*/
	    /** Check to see if a tree was created.  We can tell if this    **/
	    /**     happened by seeing if the root object has any children  **/
	    /**     or parameters.                                          **/
	    /** IF a tree was not created THEN                              **/
	    /**     Remove the root object and re-set the global pointer.   **/
	    /** ENDIF                                                       **/
	    /*---------------------------------------------------------------*/

	    if ((label_ptr -> first_child == NULL) &&
		    (label_ptr -> first_parameter == NULL))
	    {
		label_ptr = lab_remove_label (label_ptr, &local_status);
		ODLroot_node = NULL;
	    }

	/*-------------------------------------------------------------------*/
	/** ENDIF                                                           **/
	/*-------------------------------------------------------------------*/

	}  /*  End:  "if (label_ptr == NULL) ... else ..."  */

	/*-------------------------------------------------------------------*/
	/** Close the input file                                            **/
	/*-------------------------------------------------------------------*/

	fclose (file_ptr);

    /*-----------------------------------------------------------------------*/
    /** ENDIF                                                               **/
    /*-----------------------------------------------------------------------*/

    }  /*  End:  "if (file_ptr != NULL) ..."  */

    /*-----------------------------------------------------------------------*/
    /** set label_status based on pointer existence and message list contents**/
    /** RETURN a pointer to the root object in the ODL tree.                **/
    /*-----------------------------------------------------------------------*/

    *label_status = (label_ptr == NULL) ? PDS_ERROR : PDS_SUCCESS;
    if (label_ptr != NULL && temp_msg != pds_last_message) 
       *label_status = PDS_WARNING;
    return (label_ptr);

/** END **/

}  /*  "lab_read_label"  */

/**********************************************************************
 *$Component                                                          *
 *    AGGREGATE lab_remove_label (label_ptr, label_status)            *
 *$Abstract                                                           *
 *    Deallocates all objects in an ODL tree.                         *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    REMOVE                                                          *
 *    ODL                                                             *
 *    TREE                                                            *
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
 *$Outputs                                                            *
 *    label_status:                                                   *
 *        The label_status variable is an integer which is used to    *
 *        indicate the success or failure of a PDS label routine,     *
 *        and whether or not the object or parameter specifications   *
 *        passed in were ambiguous.  Valid values are:  PDS_SUCCESS,  *
 *        PDS_ERROR, PDS_MULTIPLE_OBJECTS, PDS_MULTIPLE_PARMS, and    *
 *        PDS_WARNING.                                                *
 *$Returns                                                            *
 *    label_ptr:                                                      *
 *        The label_ptr variable is a pointer to the root of an       *
 *        ODL tree.  Usually it points to the actual "ROOT" object,   *
 *        but it may point to any other object in the tree.  If it    *
 *        does point to something other than the "ROOT" object, then  *
 *        this object will be treated as the root of a sub-tree, and  *
 *        processing will not be allowed to move above the level of   *
 *        this object.                                                *
 *$Detailed_Description                                               *
 *    The lab_remove_label routine deallocates all                    *
 *    storage used by an ODL tree pointed to by label_ptr. If it is   *
 *    successful, then this routine should always return NULL.        *
 *$Error_Handling                                                     *
 *    If the resulting label tree is NOT NULL, or the SFDU labels     *
 *    cannot be deallocated, the the label_status is set to           *
 *    PDS_ERROR. Otherwise, it is set to PDS_SUCCESS.                 *
 *$Side_Effects                                                       *
 *    This routine causes memory to be deallocated.                   *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Herbert C. Gamble / J.P.L.                                      *
 *    David P. Bernath / J.P.L.                                       *
 *$Version_and_Date                                                   *
 *    2.0   June 19. 1992                                             *
 *$Change_History                                                     *
 *    HCG   03-08-91   Original Code.                                 *
 *    DPB   04-10-91   Restructured routine.                          *
 *    MDD   06-19-92   Changed name and added label_status output.    *
 **********************************************************************/

AGGREGATE lab_remove_label (label_ptr, label_status) 

AGGREGATE label_ptr; 
int *label_status;
{
    while (label_ptr != NULL)
        label_ptr = RemoveAggregate (label_ptr);
    *label_status = (label_ptr != NULL) ? PDS_ERROR : PDS_SUCCESS;

    return (label_ptr);

}  /*  "lab_remove_label"  */ 


/**********************************************************************
 *$Component                                                          *
 *    AGGREGATE lab_remove_object (label_ptr, object_class,           *
 *                                 object_name, object_position,      *
 *                                 label_status)                      *
 *$Abstract                                                           *
 *    Removes an object from a PDS Label.                             *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    REMOVE                                                          *
 *    OBJECT                                                          *
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
 *    object_class:                                                   *
 *        The object_class variable is a character string             *
 *        which contains the pds_class of an object in a PDS label        *
 *        (e.g., "OBJECT = IMAGE" implies that "IMAGE" is the pds_class   *
 *        of the object).                                             *
 *    object_name:                                                    *
 *        The object_name variable is a character string              *
 *        which contains the name of an object in a PDS label.        *
 *        This is assumed to be the value of the "NAME" parameter     *
 *        in the object (e.g., "NAME = VOYAGER" implies that          *
 *        "VOYAGER" is the name of the object).                       *
 *    object_position:                                                *
 *        The object_position variable is an integer which            *
 *        represents the relative position of an object in a PDS      *
 *        label.  If this variable is used in conjunction with either *
 *        the object_class or object_name variables, then it          *
 *        represents a particular occurrence of that thing in the     *
 *        label (e.g., if object_class is "TABLE" and object_position *
 *        is 3, then this represents the third "TABLE" object in the  *
 *        label).  On the other hand, if this variable is used by     *
 *        itself, it represents the absolute position of the object,  *
 *        starting from the "ROOT" object (position = 1).  The        *
 *        sequence follows the structure of the label as it looks     *
 *        in a flat file.                                             *
 *$Outputs                                                            *
 *    label_status:                                                   *
 *        The label_status variable is an integer which is used to    *
 *        indicate the success or failure of a PDS label routine,     *
 *        and whether or not the object or parameter specifications   *
 *        passed in were ambiguous.  Valid values are:  PDS_SUCCESS,  *
 *        PDS_ERROR, PDS_MULTIPLE_OBJECTS, PDS_MULTIPLE_PARMS, and    *
 *        PDS_WARNING.                                                *
 *$Returns                                                            *
 *    parent_object_ptr:                                              *
 *        The parent_object_ptr variable is a pointer to the          *
 *        structure used to represent the parent of an object in a    *
 *        PDS label.                                                  *
 *$Detailed_Description                                               *
 *    The lab_remove_object routine removes an object from a PDS      *
 *    label.  It searches for the object, deallocates its storage,    *
 *    and returns a pointer to its parent.  The object may by         *
 *    specified by pds_class, name, or position, or by any combination    *
 *    of these.  The search for the object begins at the object       *
 *    pointed to by the label_ptr variable.  This is usually the      *
 *    "ROOT" object, but may be any other object in the tree.         *
 *    Please note that it is not necessary to specify the object's    *
 *    name, pds_class, and position at the same time.  In fact, if you    *
 *    specify only one of them and pass in zero for the others, the   *
 *    routine will work just fine.                                    *
 *$Error_Handling                                                     *
 *    1) If the object cannot be found then the label_status          *
 *       is set to PDS_ERROR and a NULL value is returned.            *
 *    2) If more than one object is found that matches the            *
 *       specifications passed in for the parent object, then the     *
 *       label_status is set to PDS_MULTIPLE_OBJECTS and a NULL       *
 *       value is returned.                                           *
 *    3) If everything went according to plan, then the label_status  *
 *       is set to PDS_SUCCESS and a pointer to the parent of the     *
 *       object is <returned.                                          *
 *    4) If the object is the "ROOT" object, then the entire tree     *
 *       is deallocated, the label_status is set to PDS_SUCCESS,      *
 *       and a NULL pointer is returned.                              *
 *$Side_Effects                                                       *
 *    This routine causes memory to be deallocated.                   *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Herbert C. Gamble / J.P.L.                                      *
 *    David P. Bernath / J.P.L.                                       *
 *$Version_and_Date                                                   *
 *    1.1   April 26, 1991                                            *
 *$Change_History                                                     *
 *    HCG   03-08-91   Original Code.                                 *
 *    DPB   04-10-91   Restructured routine.                          *
 *    DPB   04-26-91   Added code to strip blanks from inputs.        *
 **********************************************************************/

AGGREGATE lab_remove_object (label_ptr, object_class, 
                             object_name, object_position, label_status)

AGGREGATE label_ptr;
char *object_class;
char *object_name;
int object_position;
int *label_status;

{
    AGGREGATE object_ptr = {NULL};
    AGGREGATE parent_object_ptr = {NULL};

/** BEGIN **/

    /*-----------------------------------------------------------------------*/
    /** Initialize the label_status variable.                               **/
    /*-----------------------------------------------------------------------*/

    *label_status = PDS_ERROR;

    /*-----------------------------------------------------------------------*/
    /** Remove leading and trailing blanks from character inputs.           **/
    /*-----------------------------------------------------------------------*/

    util_strip_lead_and_trail (object_class, ' ');
    util_strip_lead_and_trail (object_name, ' ');

    /*-----------------------------------------------------------------------*/
    /** Try to find an object which meets the specifications passed in.     **/
    /*-----------------------------------------------------------------------*/

    object_ptr = lab_find_object (label_ptr, object_class, 
                                  object_name, object_position, label_status);

    /*-----------------------------------------------------------------------*/
    /** IF an object was found THEN                                         **/
    /**     Save a pointer to the object's parent and deallocate the        **/
    /**         object and all of its children.                             **/
    /** ENDIF                                                               **/
    /*-----------------------------------------------------------------------*/

    if ((object_ptr != NULL) && (*label_status == PDS_SUCCESS))
    {
        parent_object_ptr = object_ptr -> parent;
        RemoveAggregate (object_ptr);
    }

    /*-----------------------------------------------------------------------*/
    /** RETURN a pointer to the object's parent.                            **/
    /*-----------------------------------------------------------------------*/

    return (parent_object_ptr);

/** END **/

}  /*  "lab_remove_object"  */


/**********************************************************************
 *$Component                                                          *
 *    AGGREGATE lab_remove_parameter (label_ptr, object_class,        *
 *                                object_name, object_position,       *
 *                                parameter_name, parameter_position, *
 *                                label_status)                       *
 *$Abstract                                                           *
 *    Removes a parameter from a PDS Label.                           *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    REMOVE                                                          *
 *    PARAMETER                                                       *
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
 *    object_class:                                                   *
 *        The object_class variable is a character string             *
 *        which contains the pds_class of an object in a PDS label        *
 *        (e.g., "OBJECT = IMAGE" implies that "IMAGE" is the pds_class   *
 *        of the object).                                             *
 *    object_name:                                                    *
 *        The object_name variable is a character string              *
 *        which contains the name of an object in a PDS label.        *
 *        This is assumed to be the value of the "NAME" parameter     *
 *        in the object (e.g., "NAME = VOYAGER" implies that          *
 *        "VOYAGER" is the name of the object).                       *
 *    object_position:                                                *
 *        The object_position variable is an integer which            *
 *        represents the relative position of an object in a PDS      *
 *        label.  If this variable is used in conjunction with either *
 *        the object_class or object_name variables, then it          *
 *        represents a particular occurrence of that thing in the     *
 *        label (e.g., if object_class is "TABLE" and object_position *
 *        is 3, then this represents the third "TABLE" object in the  *
 *        label).  On the other hand, if this variable is used by     *
 *        itself, it represents the absolute position of the object,  *
 *        starting from the "ROOT" object (position = 1).  The        *
 *        sequence follows the structure of the label as it looks     *
 *        in a flat file.                                             *
 *    parameter_name:                                                 *
 *        The parameter_name variable is a character string           *
 *        which contains the name of a parameter in a PDS label       *
 *        (e.g., the line "SCID = VG1" implies that "SCID" is the     *
 *        parameter name).                                            *
 *    parameter_position:                                             *
 *        The parameter_position variable is an integer which         *
 *        represents the relative position of a parameter in an       *
 *        object in a PDS label.  If this variable is used in         *
 *        conjunction with the parameter_name variable, then it       *
 *        represents a particular occurrence of that parameter        *
 *        in the object (e.g., if parameter_name is "SCID" and        *
 *        parameter_position is 2, then this represents the second    *
 *        "SCID" parameter in the object).  On the other hand, if     *
 *        this variable is used by itself, it represents the absolute *
 *        position of the parameter within the object, starting with  *
 *        first parameter in the object (position = 1).               *
 *$Outputs                                                            *
 *    label_status:                                                   *
 *        The label_status variable is an integer which is used to    *
 *        indicate the success or failure of a PDS label routine,     *
 *        and whether or not the object or parameter specifications   *
 *        passed in were ambiguous.  Valid values are:  PDS_SUCCESS,  *
 *        PDS_ERROR, PDS_MULTIPLE_OBJECTS, PDS_MULTIPLE_PARMS, and    *
 *        PDS_WARNING.                                                *
 *$Returns                                                            *
 *    object_ptr:                                                     *
 *        The object_ptr variable is a pointer to the structure       *
 *        used to represent an object in a PDS label.                 *
 *$Detailed_Description                                               *
 *    The lab_remove_parameter routine removes a parameter from a PDS *
 *    label.  The routine searches the ODL tree for the object        *
 *    specified, searches for the parameter specified, and removes    *
 *    the parameter from the list.  The object may by specified by    *
 *    pds_class, name, or position, or by any combination of these, and   *
 *    the parameter may be specified by name or position, or both.    *
 *    The search for the object begins at the object pointed to by    *
 *    the label_ptr variable.  This is usually the "ROOT" object, but *
 *    may be any other object in the tree.  Please note that it is    *
 *    not necessary to specify the object's name, pds_class, and position *
 *    at the same time.  In fact, if you specify only one of them     *
 *    and pass in zero for the others, the routine will work just     *
 *    fine.  This also applies to the parameter name and position.    *
 *$Error_Handling                                                     *
 *    1) If the object cannot be found then the label_status          *
 *       is set to PDS_ERROR and a NULL value is returned.            *
 *    2) If more than one object is found that matches the            *
 *       specifications passed in for the object, then the            *
 *       label_status is set to PDS_MULTIPLE_OBJECTS and a NULL       *
 *       value is returned.                                           *
 *    3) If more than one parameter is found that matches the         *
 *       specifications passed in for the parameter, then the         *
 *       label_status is set to PDS_MULTIPLE_PARMS and a NULL         *
 *       value is returned.                                           *
 *    4) If everything went according to plan, then the label_status  *
 *       is set to PDS_SUCCESS, and a pointer to the object which     *
 *       owns the parameter is returned.                              *
 *$Side_Effects                                                       *
 *    This routine causes memory to be deallocated.                   *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Herbert C. Gamble / J.P.L.                                      *
 *    David P. Bernath / J.P.L.                                       *
 *$Version_and_Date                                                   *
 *    1.1   April 26, 1991                                            *
 *$Change_History                                                     *
 *    HCG   03-08-91   Original Code.                                 *
 *    DPB   04-10-91   Restructured routine.                          *
 *    DPB   04-26-91   Added code to strip blanks from inputs.        *
 **********************************************************************/

AGGREGATE lab_remove_parameter (label_ptr, object_class, object_name, 
                                object_position, parameter_name, 
                                parameter_position, label_status)

AGGREGATE label_ptr;
char *object_class;
char *object_name;
int object_position;
char *parameter_name;
int parameter_position;
int *label_status;

{
    AGGREGATE object_ptr = {NULL};
    PARAMETER parameter_ptr = {NULL};

/** BEGIN **/

    /*-----------------------------------------------------------------------*/
    /** Initialize the label_status variable.                               **/
    /*-----------------------------------------------------------------------*/

    *label_status = PDS_ERROR;

    /*-----------------------------------------------------------------------*/
    /** Remove leading and trailing blanks from character inputs.           **/
    /*-----------------------------------------------------------------------*/

    util_strip_lead_and_trail (object_class, ' ');
    util_strip_lead_and_trail (object_name, ' ');
    util_strip_lead_and_trail (parameter_name, ' ');

    /*-----------------------------------------------------------------------*/
    /** Try to locate the parameter to be removed.                          **/
    /*-----------------------------------------------------------------------*/

    parameter_ptr = lab_find_parameter (label_ptr, object_class, 
                                        object_name, object_position, 
                                        parameter_name, parameter_position, 
                                        label_status);

    /*-----------------------------------------------------------------------*/
    /** IF the parameter was found THEN                                     **/
    /**     Save a pointer to the object which owns it and deallocate the   **/
    /**         storage used by it and all of its values.                   **/
    /** ENDIF                                                               **/
    /*-----------------------------------------------------------------------*/

    if ((parameter_ptr != NULL) && (*label_status == PDS_SUCCESS))
    {
        object_ptr = parameter_ptr -> owner;
        RemoveParameter (parameter_ptr);
    }

    /*-----------------------------------------------------------------------*/
    /** RETURN a pointer to the object which owned the parameter            **/
    /*-----------------------------------------------------------------------*/

    return (object_ptr);

/** END **/

}  /*  "lab_remove_parameter"  */



/**********************************************************************
 *$Component                                                          *
 *    LOGICAL lab_remove_sfdu_labels (label_ptr, label_status)        *
 *$Abstract                                                           *
 *    Removes the SFDU labels from a label tree.                      *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    SFDU                                                            *
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
 *$Outputs                                                            *
 *    label_status:                                                   *
 *        The label_status variable is an integer which is used to    *
 *        indicate the success or failure of a PDS label routine,     *
 *        and whether or not the object or parameter specifications   *
 *        passed in were ambiguous.  Valid values are:  PDS_SUCCESS,  *
 *        PDS_ERROR, PDS_MULTIPLE_OBJECTS, PDS_MULTIPLE_PARMS, and    *
 *        PDS_WARNING.                                                *
 *$Returns                                                            *
 *    None                                                            *
 *$Detailed_Description                                               *
 *    The lab_remove_sfdu_labels routine removes the SFDU labels from *
 *    the ODL tree pointed to by label_ptr. These SFDU labels were    *
 *    those found and saved by the lab_read_label routine.            *
 *$Error_Handling                                                     *
 *    The label_status is set to PDS_SUCCESS is label_ptr is not      *
 *    NULL. Otherwise, the status is set to PDS_ERROR.                *
 *$Side_Effects                                                       *
 *    The memory used by the SFDU list structure is deallocated.      *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore/ JPL                                            *
 *$Version_and_Date                                                   *
 *    1.0   June 18, 1992                                             *
 *$Change_History                                                     *
 *    MDD   06-18-92   Original Code.                                 *
 **********************************************************************/

void lab_remove_sfdu_labels (label_ptr, label_status)

AGGREGATE label_ptr;
int *label_status;
{

    SFDU_INFO *temp;
    SFDU_INFO *sfdu_list;

    if (label_ptr != NULL)
    {
        *label_status = PDS_SUCCESS;
	sfdu_list = label_ptr -> sfdu_list;
	while (sfdu_list != NULL)
	{
	   temp = sfdu_list -> next;
	   Lemme_Go(sfdu_list -> sfdu_label);
	   Lemme_Go(sfdu_list);
	   sfdu_list = temp;
       }
       label_ptr -> sfdu_list = NULL;
    }
    else
        *label_status = PDS_ERROR;
    return;
}


 
/**********************************************************************
 *$Component                                                          *
 *   LOGICAL lt_add_pointer (label_ptr, pointer_info, label_status)   *
 *$Abstract                                                           *
 *    Adds a pointer value to a PDS label.                            *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    POINTER                                                         *
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
 *    pointer_info:                                                   *
 *        The pointer_info variable is the address of a POINTER_INFO  *
 *        structure, which contains information about a label pointer,*
 *        indicating the file it points to, the location of an object *
 *        within the file, the type of the location (record or bytes) *
 *        and whether the data file is attached or detached from the  *
 *        label.                                                      *
 *$Outputs                                                            *
 *    label_status:                                                   *
 *        The label_status variable is an integer which is used to    *
 *        indicate the success or failure of a PDS label routine,     *
 *        and whether or not the object or parameter specifications   *
 *        passed in were ambiguous.  Valid values are:  PDS_SUCCESS,  *
 *        PDS_ERROR, PDS_MULTIPLE_OBJECTS, PDS_MULTIPLE_PARMS, and    *
 *        PDS_WARNING.                                                *
 *$Returns                                                            *
 *    success_flag:                                                   *
 *        The success_flag variable is a general purpose variable     *
 *        which indicates success (a value of TRUE) or failure        *
 *        (a value of FALSE).                                         *
 *$Detailed_Description                                               *
 *    The lab_add_pointer replaces or adds the pointer stored in the  *
 *    pointer_info structure in the ROOT object of the label pointed  *
 *    to by label_ptr. It handles four types of pointers:             *
 *           Record location, attached (^TABLE = 3)                   *
 *           Byte location, attached (^TABLE = 512<BYTES>)            *
 *           Record location, detached (^TABLE = ("TABLE.DAT",3))     *
 *           Byte location, detached (^TABLE = ("TABLE.DAT",512<BYTES>))*
 *    If the pointer cannot be added, this routine returns FALSE.     *
 *    The name of the pointer to be added, WITHOUT THE CARET, must be *
 *    stored in the pointer_info structure.                           *
 *$Error_Handling                                                     *
 *    For label_status_values, see lab_add_parameter.                 *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore/JPL                                             *
 *$Version_and_Date                                                   *
 *    1.0   October 10, 1991                                          *
 *$Change_History                                                     *
 *    MDD   10-10-91   Original Code.                                 *
 **********************************************************************/

LOGICAL lab_replace_pointer (label_ptr, pointer_info, label_status)

AGGREGATE label_ptr;
POINTER_INFO *pointer_info;
int *label_status;

{
   char temp_str [PDS_MAXLINE + 1];
   char temp_name [PDS_MAXLINE + 1];

/** BEGIN **/

   /*---------------------------------------------------------------*/
   /** create pointer string as attached with byte location        **/
   /*---------------------------------------------------------------*/

   *label_status = PDS_ERROR;
   if (pointer_info -> is_attached && pointer_info -> has_byte_loc)
      sprintf (temp_str, "%ld<bytes>", pointer_info -> location);

   /*---------------------------------------------------------------*/
   /** create pointer string as attached with record location      **/
   /*---------------------------------------------------------------*/

   else if (pointer_info -> is_attached)
      sprintf (temp_str, "%ld", pointer_info -> location);

   /*---------------------------------------------------------------*/
   /** create pointer string as detached with byte location        **/
   /*---------------------------------------------------------------*/

   else if (pointer_info -> has_byte_loc)
      sprintf (temp_str, "(\"%s\", %ld<bytes>)", pointer_info -> file_name,
               pointer_info -> location);

   /*---------------------------------------------------------------*/
   /** create pointer string as detached with record location      **/
   /*---------------------------------------------------------------*/

   else
      sprintf (temp_str, "(\"%s\", %ld)", pointer_info -> file_name,
               pointer_info -> location);

   /*---------------------------------------------------------------*/
   /** Add the pointer to the label                                **/
   /*---------------------------------------------------------------*/

   lab_change_value (label_ptr, "ROOT", NULL, 1, pointer_info -> name, 1, temp_str, 
                      label_status);
   if (*label_status != PDS_SUCCESS)
   {
      sprintf (temp_name, "^%s", pointer_info -> name);
      lab_add_parameter (label_ptr, "ROOT", NULL, 1, temp_name, temp_str, 
                         label_status);
   }
   return (*label_status == PDS_SUCCESS);

/** END **/
}


/**********************************************************************
 *$Component                                                          *
 *    LOGICAL lab_setup ()                                            *
 *$Abstract                                                           *
 *    Sets up the Label Library                                       *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    None.                                                           *
 *$Outputs                                                            *
 *    None.                                                           *
 *$Returns                                                            *
 *    success_flag:                                                   *
 *        The success_flag variable is a general purpose variable     *
 *        which indicates success (a value of TRUE) or failure        *
 *        (a value of FALSE).                                         *
 *$Detailed_Description                                               *
 *    The lab_setup routine clears the global message list, calls     *
 *    fio_setup to create temporary files used by the label library,  *
 *    and returns a success or failure flag. If errors occurred, then *
 *    specific errors will be added to the message list, and can be   *
 *    checked after this routine returns.                             *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore/ JPL                                            *
 *$Version_and_Date                                                   *
 *    1.0   June 24, 1992                                             *
 *$Change_History                                                     *
 *    MDD   06-24-92   Original Code.                                 *
 **********************************************************************/

LOGICAL lab_setup () 
{
   lab_clear_messages (); 
   fio_setup (); 
   return (!lab_has_messages ());
}


/**********************************************************************
 *$Component                                                          *
 *    LOGICAL lab_start_label (label_status)                          *
 *$Abstract                                                           *
 *    Starts a new label tree.                                        *
 *$Keywords                                                           *
 *    LABEL                                                           *
 *    NON_UI_COMMON                                                   *
 *$Inputs                                                             *
 *    None.                                                           *
 *$Outputs                                                            *
 *    label_status:                                                   *
 *        The label_status variable is an integer which is used to    *
 *        indicate the success or failure of a PDS label routine,     *
 *        and whether or not the object or parameter specifications   *
 *        passed in were ambiguous.  Valid values are:  PDS_SUCCESS,  *
 *        PDS_ERROR, PDS_MULTIPLE_OBJECTS, PDS_MULTIPLE_PARMS, and    *
 *        PDS_WARNING.                                                *
 *$Returns                                                            *
 *    success_flag:                                                   *
 *        The success_flag variable is a general purpose variable     *
 *        which indicates success (a value of TRUE) or failure        *
 *        (a value of FALSE).                                         *
 *$Detailed_Description                                               *
 *    The lab_start_label routine starts a new label tree by          *
 *    creating a ROOT object and returning a pointer to it.           *
 *$Error_Handling                                                     *
 *    If a ROOT is created, label_status is set to PDS_SUCCESS.       *
 *    Otherwise, it is set to PDS_ERROR.                              *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore/ JPL                                            *
 *$Version_and_Date                                                   *
 *    1.0   June 24, 1992                                             *
 *$Change_History                                                     *
 *    MDD   06-24-92   Original Code.                                 *
 **********************************************************************/

AGGREGATE lab_start_label (label_status)

int *label_status;
{
   AGGREGATE temp;
   temp = lu_append_object (NULL, "ROOT");
   *label_status = (temp == NULL) ? PDS_ERROR : PDS_SUCCESS;
   return temp;
}

/**********************************************************************
 *$Component                                                          *
 * LOGICAL lab_write__attached_label (label_ptr,                      *
 *             product_file_name, data_file_name, data_skip,          *
 *             label_status)                                          *
 *$Abstract                                                           *
 *    Writes a label to a file                                        *
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
 *    product_file_name:                                              *
 *        The product_file_name variable is a character               *
 *        string containing the file specification of a file that     *
 *        contains a PDS label and attached data.                     *
 *    data_file_name:                                                 *
 *        The file_name variable is a general purpose character       *
 *        string containing the file specification of a file of data. *
 *    data_skip:                                                      *
 *        The data_skip variable is an integer indicating how many    *
 *        bytes of data at the beginning of a data file should be     *
 *        skipped when attaching the data to a PDS label.             *
 *$Outputs                                                            *
 *    label_status:                                                   *
 *        The label_status variable is an integer which is used to    *
 *        indicate the success or failure of a PDS label routine,     *
 *        and whether or not the object or parameter specifications   *
 *        passed in were ambiguous.  Valid values are:  PDS_SUCCESS,  *
 *        PDS_ERROR, PDS_MULTIPLE_OBJECTS, PDS_MULTIPLE_PARMS, and    *
 *        PDS_WARNING.                                                *
 *$Returns                                                            *
 *    success_flag:                                                   *
 *        The success_flag variable is a general purpose variable     *
 *        which indicates success (a value of TRUE) or failure        *
 *        (a value of FALSE).                                         *
 *$Detailed_Description                                               *
 *    The lab_write_attached_label routine writes the ODL label       *
 *    pointed to by label_ptr to the output file product_file_name    *
 *    by calling lab_write_label and using the record type indicated  *
 *    by the label itself. Then the data in the given data file is    *
 *    appended to the product file, and, if the record type is        *
 *    fixed length, the file is padded to meet record length          *
 *    requirements.                                                   *
 *                                                                    *
 *    NOTE that this routine only supports the following record types:*
 *                                                                    *
 *       PDS_RF_STREAM_CRLF                                           *
 *       PDS_RF_STREAM_CR                                             *
 *       PDS_RF_STREAM_LF                                             *
 *       PDS_RF_BINARY                                                *
 *       PDS_RF_RMS_VAR                                               *
 *       PDS_RF_RMS_STREAM                                            *
 *       PDS_RF_ASCII                                                 *
 *                                                                    *
 *    because lab_write_label only supports these. If the label       *
 *    indicates an UNKNOWN or UNDEFINED record type, then the         *
 *    record type specified by pds_default_rectype is used.           *
 *                                                                    *
 *    Any "attached" pointers in the label will be adjusted by the    *
 *    lab_adjust_pointers routine, which is called by this routine.   *
 *    Before calling that routine, this routine will check for the    *
 *    presence of the LABEL_RECORDS keyword in the label, and will    *
 *    assume it is 0 if not there.  Note that all changes made to the *
 *    pointers and keywords in the label are made to a COPY of it,    *
 *    and do therefore not affect the label pointed to by label_ptr.  *
 *                                                                    *
 *    NOTE to programmers: the behavior of this routine is HIGHLY     *
 *    dependent upon the behavior of lab_write_label. Be cautious     *
 *    when making changes.                                            *
 *$External_References                                                *
 *    Item                     Shared-Data           Access           *
 *    ------------------------------------------------------------    *
 *    pds_last_message       pdsglob.h               read             *
 *    pds_default_rectype    pdsglob.h               read             *
 *    pds_records_written    pdsglob.h               read             *
 *$Error_Handling                                                     *
 *    If the file cannot be opened, written, or the record type       *
 *    is invalid, then   label_status will be set to PDS_ERROR and    *
 *    FALSE will be returned.  Otherwise, TRUE will be returned. The  *
 *    label_status will be PDS_ERROR  if  messages were placed onto   *
 *    the message list, and PDS_SUCCESS if not.                       *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore/JPL                                             *
 *$Version_and_Date                                                   *
 *    1.0   June 30, 1992                                             *
 *$Change_History                                                     *
 *    MDD   06-30-92   Original Code.                                 *
 *    DWS   05-01-03   Changed "FIXED LENGTH" to "FIXED_LENGTH"       *
 **********************************************************************/

LOGICAL lab_write_attached_label (label_ptr, product_file_name, data_file_name, 
                                  data_skip, label_status)

AGGREGATE label_ptr;
char *product_file_name;
char *data_file_name;
long data_skip;
int *label_status;
{
   
   LOGICAL success = TRUE;
   FILE *product_file_ptr;
   FILE *data_file_ptr;
   ERROR_LIST *temp_msg = pds_last_message;
   char buffer [PDS_BUFFLEN + 1];
   int chars;
   long i;
   char *temp_str;
   int local_status;
   long file_size;
   long space_left;
   long record_length = 0;
   AGGREGATE label_copy;

/** BEGIN **/

   label_copy = CopyAggregate (label_ptr);
   
   /*------------------------------------------------------------------------*/
   /** IF a label, product file name, and data file name were passed in THEN**/
   /*------------------------------------------------------------------------*/

   if (label_copy != NULL && product_file_name != NULL && data_file_name != NULL)
   {
      /*---------------------------------------------------------------------*/
      /** strip blanks from input fields                                    **/
      /** add the LABEL_RECORDS keyword to the label if not present         **/
      /** write the label to the product file                               **/
      /** adjust pointers according to the label size                       **/
      /** IF that was successful THEN                                       **/
      /*---------------------------------------------------------------------*/

      util_strip_lead_and_trail (product_file_name, ' ');
      util_strip_lead_and_trail (data_file_name, ' ');
      if (!lab_find_parameter (label_copy, "ROOT", NULL, 1, "LABEL_RECORDS", 1,
                              &local_status))
      {
          err_append_message (WARNING, 
                              "LABEL_RECORDS has been added to output label.");
          err_append_message (WARNING, 
                              "Previous value of LABEL_RECORDS assumed to be 0.");
          lab_add_parameter (label_copy, "ROOT", NULL, 1, "LABEL_RECORDS", "0",
                             &local_status);
      }

      success = lab_write_label (label_copy, product_file_name, TRUE, 0, 
                                 &local_status);
      if (success)
      {
         lab_adjust_pointers (label_copy, pds_records_written, &local_status);
         success = success && lab_write_label (label_copy, product_file_name, TRUE, 
                                               pds_records_written, &local_status);
      }
      if (success && local_status == PDS_SUCCESS)
      {
         /*---------------------------------------------------------------*/
         /** get the value of RECORD_TYPE from the label                 **/
         /** IF it existed THEN                                          **/
         /**    IF the format is fixed THEN                              **/
         /**       get the value of RECORD_BYTES and assign it to        **/
         /**       record_length                                         **/
         /**    ELSE IF format is unknown but default was fixed THEN     **/
         /**       assign default length to record_length                **/
         /**    ENDIF                                                    **/
         /** ENDIF                                                       **/
         /*---------------------------------------------------------------*/

         temp_str = lab_get_value (label_copy, NULL, NULL, 1, "RECORD_TYPE",
                                   1, 1, FALSE, &local_status);
         if (temp_str != NULL)
         {
/*             util_replace_char (temp_str, '_', ' ');*/
             util_upper_case (temp_str);
             if ((strcmp (temp_str, "FIXED") == 0 ) ||
                 (strcmp (temp_str, "FIXED_LENGTH")  == 0))
             {
                 Lemme_Go(temp_str);
                 temp_str = lab_get_value (label_copy, NULL, NULL, 1, 
                                           "RECORD_BYTES", 1, 1, FALSE, 
                                           &local_status);
                 if (temp_str != NULL) record_length = atol (temp_str);
             }
             else if (pds_default_rectype == PDS_RF_BINARY &&
                      (strcmp (temp_str, "UNKNOWN") == 0 ||
                       strcmp (temp_str, "UNDEFINED") == 0))
                 record_length = PDS_DEFAULT_RECLEN;
         }    

         /*------------------------------------------------------------------*/
         /** open the product file in append mode                           **/
         /** open the data file for reading                                 **/
         /** IF both could be opened THEN                                   **/
         /*------------------------------------------------------------------*/

         product_file_ptr = fopen (product_file_name, "ab+");
         data_file_ptr = fopen (data_file_name, "rb");
         if (product_file_ptr && data_file_ptr)
         {
            /*---------------------------------------------------------------*/
            /** skip data_skip bytes in the data file                       **/
            /** append remaining data to the product file                   **/
            /** close files                                                 **/
            /*---------------------------------------------------------------*/

            for (i = 1; i <= data_skip && !feof (data_file_ptr); i++) 
                 fgetc (data_file_ptr);
            while (!feof (data_file_ptr))
            {
                chars = (int) fread (buffer, 1, PDS_BUFFLEN, data_file_ptr);
                fwrite (buffer, chars, 1, product_file_ptr);
            }      
            Close_Me(product_file_ptr);
            Close_Me(data_file_ptr);

#ifndef VAX
            /*---------------------------------------------------------------*/
            /** IF the file is fixed (has a required record length) THEN    **/
            /**    get the size of the product file                         **/
            /**    open the file in append mode                             **/
            /**    pad the file to the required record length               **/
            /** ENDIF                                                       **/
            /*---------------------------------------------------------------*/

            if (record_length != 0)
            {
                file_size = fio_size (product_file_name);
                product_file_ptr = fopen (product_file_name, "ab+");
                space_left = record_length - (file_size % record_length);
                if (space_left == record_length) space_left = 0;
                for (i = 1; i <= space_left; i++) fputc (' ', product_file_ptr);
                Close_Me(product_file_ptr);
	    }
#endif
         }
         /*------------------------------------------------------------------*/
         /** ELSE                                                           **/
         /**    files could not be opened so warn the user                  **/
         /*------------------------------------------------------------------*/

         else
         {
            err_append_message (ERROR1, 
                 "Could not open label and/or data file to attach data.");
            success = FALSE;
	 }
         /*------------------------------------------------------------------*/
         /** ENDIF                                                          **/
         /*------------------------------------------------------------------*/
      }
      /*---------------------------------------------------------------------*/
      /** ENDIF                                                             **/
      /*---------------------------------------------------------------------*/

      RemoveAggregate (label_copy);
   }
   /*------------------------------------------------------------------------*/
   /** ELSE                                                                 **/
   /**    arguments are bad, so set error flag                              **/
   /*------------------------------------------------------------------------*/

   else
   {
      success = FALSE;
      if (label_copy == NULL && label_ptr != NULL)
         err_append_message (ERROR1, "Ran out of memory when copying label.");
   }

   /*------------------------------------------------------------------------*/
   /** ENDIF                                                                **/
   /** set status based on success flag and message list contents           **/
   /*------------------------------------------------------------------------*/

   *label_status = (success) ? PDS_SUCCESS : PDS_ERROR;
   if (success && temp_msg != pds_last_message) 
      *label_status = PDS_WARNING;
   return (success);

/** END lab_write_attached_label **/
}

/**********************************************************************
 *$Component                                                          *
 * LOGICAL lab_write_label (label_ptr, file_name, use_label_info,     *
 *                          label_status)                             *
 *$Abstract                                                           *
 *    Writes a label to a file                                        *
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
 *    file_name:                                                      *
 *        The file_name variable is a general purpose character       *
 *        string containing a file specification.                     *
 *    use_label_info:                                                 *
 *        The use_label_info variable is a flag indicating whether    *
 *        a label is to be written using the record type and record   *
 *        length indicated in the label itself, or using the default  *
 *        record format.                                              *
 *    pad_length:                                                     *
 *        The pad_length variable is an integer containing the number *
 *        of records that a label file should be padded to.           *
 *$Outputs                                                            *
 *    label_status:                                                   *
 *        The label_status variable is an integer which is used to    *
 *        indicate the success or failure of a PDS label routine,     *
 *        and whether or not the object or parameter specifications   *
 *        passed in were ambiguous.  Valid values are:  PDS_SUCCESS,  *
 *        PDS_ERROR, PDS_MULTIPLE_OBJECTS, PDS_MULTIPLE_PARMS, and    *
 *        PDS_WARNING.                                                *
 *$Returns                                                            *
 *    success_flag:                                                   *
 *        The success_flag variable is a general purpose variable     *
 *        which indicates success (a value of TRUE) or failure        *
 *        (a value of FALSE).                                         *
 *$Detailed_Description                                               *
 *    The lab_write_label routine writes the ODL label                *
 *    pointed to by label_ptr to the given output file. If the        *
 *    use_label_info flag is set, then the record type and length     *
 *    of the label will be determined by examining the RECORD_TYPE    *
 *    and RECORD_BYTES keywords in the label itself. Otherwise, the   *
 *    record type used will be the one stored in the global variable  *
 *    pds_default_rectype.                                            *
 *                                                                    *
 *    NOTE that this routine only supports the following record types:*
 *                                                                    *
 *       PDS_RF_STREAM_CRLF                                           *
 *       PDS_RF_STREAM_CR                                             *
 *       PDS_RF_STREAM_LF                                             *
 *       PDS_RF_BINARY                                                *
 *       PDS_RF_RMS_VAR                                               *
 *       PDS_RF_RMS_STREAM                                            *
 *       PDS_RF_ASCII                                                 *
 *                                                                    *
 *    The PDS_RF_BINARY record type is used if use_label_info is set  *
 *    and RECORD_TYPE is fixed length. The record terminators used    *
 *    between label records in this case can be controlled by setting *
 *    the global pds_record_term to the desired terminator string     *
 *    before invoking this routine. The DEFAULT_REC_TYPE format       *
 *    is used if record type is stream. Variable length records can   *
 *    only be produced on VMS systems. If use_label_info is not set   *
 *    then the record type specified by pds_default_rectype is used.  *
 *    PDS_RF_STREAM_CRLF is used if the use_label_info flag is not set*
 *    and the default record type variable is PDS_RF_ASCII or         *
 *    PDS_RF_RMS_STREAM.  The record_type specified by the variable   *
 *    pds_default_rectype is also used if the use_label_info flag is  *
 *    set and the value of RECORD_TYPE is UNKNOWN, NONE, or UNDEFINED.*
 *                                                                    *
 *    The pad_length input is used to force padding of the label to   *
 *    the given number of records.  If the default length of the label*
 *    exceeds this number, then no padding is performed.              *
 *                                                                    *
 *    After this routine has completed, then the number of label      *
 *    records written may be found in the global pds_records_written. *
 *                                                                    *
 *    NOTE to programmers: lab_write_attached_label is HIGHLY         *
 *    dependent upon the behavior of this routine. Be cautious when   *
 *    making changes.                                                 *
 *$External_References                                                *
 *    Item                     Shared-Data           Access           *
 *    ------------------------------------------------------------    *
 *    pds_last_message       pdsglob.h               read             *
 *    pds_default_rectype    pdsglob.h               read             *
 *    pds_default_reclen     pdsglob.h               read             *
 *    pds_record_term        pdsglob.h               update           *
 *    pds_finish_label       pdsglob.h               update           *
 *    pds_records_needed     pdsglob.h               update           *
 *    pds_records_written    pdsglob.h               update           *
 *$Error_Handling                                                     *
 *    If the file cannot be opened, written, or the record type       *
 *    is invalid, then   label_status will be set to PDS_ERROR and    *
 *    FALSE will be returned.  Otherwise, TRUE will be returned. The  *
 *    label_status will be PDS_ERROR  if  messages were placed onto   *
 *    the message list, and PDS_SUCCESS if not.                       *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore/JPL                                             *
 *$Version_and_Date                                                   *
 *    1.0   June 30, 1992                                             *
 *$Change_History                                                     *
 *    MDD   06-30-92   Original Code.                                 *
 *    DWS   05-01-03   Changed "FIXED LENGTH" to "FIXED_LENGTH"       *
**********************************************************************/

LOGICAL lab_write_label (label_ptr, file_name, use_label_info,
			 pad_length, label_status)

AGGREGATE label_ptr;
char *file_name;
LOGICAL use_label_info;
long pad_length;
int *label_status;

{
   FILE *file_ptr;
   LOGICAL save = pds_finish_label;
   ERROR_LIST *temp_msg = pds_last_message;
   long space_needed;
   SFDU_INFO *end_sfdu = NULL;
   char *term = NULL;
   int record_type = pds_default_rectype;
   long record_length = pds_default_reclen;
   char *temp_str = NULL;
   int local_status;
   long i;
   LOGICAL success = TRUE;
   char save_term [3];
   char mrs [10]={0};
   long save_reclen = pds_default_rectype;
   char *temp_ptr = NULL;
   char *last_record = NULL;
   LOGICAL unknown_type = FALSE;
   
/** BEGIN **/

   strcpy (save_term, pds_record_term);

   /*-------------------------------------------------------------------*/
   /** IF a label and file name were passed in THEN                    **/
   /*-------------------------------------------------------------------*/

   if (label_ptr != NULL && file_name != NULL)
   {
      /*----------------------------------------------------------------*/
      /** IF the label information is to be used to specify record     **/
      /**    format THEN                                               **/
      /*----------------------------------------------------------------*/

      if (use_label_info)
      {
	 /*-------------------------------------------------------------*/
	 /** Get the value of the RECORD_TYPE keyword                  **/
	 /** set the record_type variable based upon its value         **/
	 /*-------------------------------------------------------------*/

	 temp_str = lab_get_value (label_ptr, NULL, NULL, 1, "RECORD_TYPE", 1, 1,
				   FALSE,  &local_status);
	 if (temp_str != NULL)
	 {
/*	    util_replace_char (temp_str, '_', ' ');*/
	    util_upper_case (temp_str);
	    if((strcmp (temp_str, "FIXED") == 0 ) ||
					(strcmp (temp_str, "FIXED_LENGTH") == 0))
			record_type = PDS_RF_BINARY;
	    else if (strcmp (temp_str, "STREAM") == 0)
			record_type = DEFAULT_REC_TYPE;
	    else if (strcmp (temp_str, "VARIABLE") == 0 ||
		     strcmp (temp_str, "VARIABLE_LENGTH") == 0)
	       record_type = PDS_RF_RMS_VAR;
	    else if (strcmp (temp_str, "UNKNOWN") == 0 ||
		     strcmp (temp_str, "UNDEFINED") == 0 ||
		     strcmp (temp_str, "NONE") == 0) 
	    {
	       record_type = DEFAULT_REC_TYPE;
               unknown_type = TRUE;
	    }
	    else
	    {
	       err_append_message (ERROR1,
		  "Label value for RECORD_TYPE is unrecognized or missing.");
	       success = FALSE;
	    }
            Lemme_Go(temp_str);
	 }

	 /*-------------------------------------------------------------*/
	 /** IF record_type is a fixed length type THEN                **/
	 /**    set record_length based on RECORD_BYTES in the label   **/
	 /** ENDIF                                                     **/
	 /*-------------------------------------------------------------*/

	 if (success && record_type == PDS_RF_BINARY)
	 {
	    Lemme_Go(temp_str);
	    temp_str = lab_get_value (label_ptr, NULL, NULL, 1, "RECORD_BYTES",
					  1, 1, FALSE, &local_status);
	    if (temp_str != NULL)
		pds_default_reclen = record_length = atol (temp_str);
	    if (temp_str == NULL || record_length <= 0)
	    {
	       err_append_message (ERROR1,
		   "Label value for RECORD_BYTES is invalid or missing.");
	       success = FALSE;
	    }
            Lemme_Go(temp_str);
	 }
      }

      /*----------------------------------------------------------------*/
      /** ENDIF                                                        **/
      /** Make sure record_type is a supported one                     **/
      /*----------------------------------------------------------------*/

      switch (record_type)
      {
	 case PDS_RF_ASCII:
	 case PDS_RF_RMS_STREAM: record_type = PDS_RF_STREAM_CRLF;
				 break;

	 case PDS_RF_FIXED_CRLF:
	 case PDS_RF_FIXED_CR:
	 case PDS_RF_FIXED_LF:
	 case PDS_RF_UNKNOWN:    err_append_message (ERROR1,
			"Label cannot be written using current default record type.");
				 success = FALSE;
				 break;

#ifndef VAX
	 case PDS_RF_RMS_VAR:    err_append_message (ERROR1,
		    "Variable length records cannot be written on non-VMS systems.");
				 success = FALSE;
				 break;
#endif
      }

      /*----------------------------------------------------------------*/
      /** IF this is not a fixed format THEN                           **/
      /**    assign the correct record terminator to a global          **/
      /** ENDIF                                                        **/
      /*----------------------------------------------------------------*/

      if (success && record_type != PDS_RF_BINARY)
      {
         pds_default_reclen = 0;
	 term = fio_get_term (NULL, record_type);
	 strcpy (pds_record_term, term);
	 Lemme_Go(term);
      }

      /*----------------------------------------------------------------*/
      /** IF everything is okay THEN                                   **/
      /*----------------------------------------------------------------*/

      if (success)
      {

	 /*-------------------------------------------------------------*/
	 /** warn about odd record lengths in VMS                      **/
	 /*-------------------------------------------------------------*/

#ifdef VAX
         if (pds_default_reclen % 2 != 0)
         {
            err_append_message (WARNING, 
                "VMS does not allow for odd record lengths.");
            err_append_message (WARNING,
                "Each physical label record will be padded with a 0 byte by VMS.");
	 }
#endif
	 /*-------------------------------------------------------------*/
	 /** strip blanks from input fields                            **/
	 /** open the label file (use appropriate open statement if    **/
         /**   this is VMS.                                            **/
	 /*-------------------------------------------------------------*/

	 util_strip_lead_and_trail (file_name, ' ');
#ifndef VAX
	 file_ptr = fopen (file_name, "wb");
#else
         switch (record_type)
         {
 	    case PDS_RF_STREAM_CR   : file_ptr = fopen (file_name,"w","RFM=STMCR");
		   		      break;

	    case PDS_RF_STREAM_LF   : file_ptr = fopen (file_name,"w","RFM=STMLF");
				      break;

	    case PDS_RF_STREAM_CRLF : file_ptr = fopen (file_name,"w","RFM=STM");
				      break;

	    case PDS_RF_BINARY      : sprintf (mrs, "MRS=%ld", record_length);
				      file_ptr = fopen (file_name,"w", "RFM=FIX", 
                                                        mrs);
				      break;

	    case PDS_RF_RMS_VAR     : file_ptr = fopen (file_name,"w", "RFM=VAR",
                                                    "RAT=CR");
				      break;

	    default                 : file_ptr = fopen (file_name,"w");
				      break;
	 }
#endif

	 /*-------------------------------------------------------------*/
	 /** IF the label file cannot be opened THEN                   **/
	 /**    warn the user                                          **/
	 /*-------------------------------------------------------------*/

	 if (file_ptr == NULL)
	 {
	    err_append_message (ERROR1,
		 "Could not open output file in order to write label.");
	    success = FALSE;
	 }

	 /*----------------------------------------------------------------*/
	 /** ELSE                                                         **/
	 /*----------------------------------------------------------------*/

	 else
	 {
	    /*-------------------------------------------------------------*/
	    /** set the globals for label records needed and written      **/
	    /*-------------------------------------------------------------*/

            pds_records_written = 0;
            pds_records_needed = pad_length;

	    /*-------------------------------------------------------------*/
	    /** IF there are SFDU labels THEN                             **/
	    /**   write the first one                                     **/
	    /**   IF there is a second one THEN turn off the writing of   **/
	    /**      the ODL END statement, or if the records are fixed,  **/
            /**      or if the record type was unknown                    **/
	    /** ENDIF                                                     **/
	    /*-------------------------------------------------------------*/
         
	    if (label_ptr -> sfdu_list != NULL)
	    {
	       ODLWriteStmt (file_ptr, label_ptr -> sfdu_list -> sfdu_label);
	       ODLWriteStmt (file_ptr, pds_record_term);
	       end_sfdu = label_ptr -> sfdu_list -> next;
               pds_finish_label = (end_sfdu == NULL || record_type == PDS_RF_BINARY
                                   || unknown_type == TRUE);
	    }
 
	    /*-------------------------------------------------------------*/
	    /** IF these are fixed length records and the last will       **/
            /**   contains an SFDU then decrement the number of label     **/
            /**   records needed.                                         **/
	    /*-------------------------------------------------------------*/

            if (end_sfdu && record_type == PDS_RF_BINARY && pds_records_needed != 0)
               pds_records_needed--;

	    /*-------------------------------------------------------------*/
	    /** Write the ODL label                                       **/
	    /*-------------------------------------------------------------*/

	    WriteLabel (file_ptr, label_ptr);

	    /*-------------------------------------------------------------*/
	    /** IF the records are not fixed and there is a second SFDU   **/
	    /**    append the ODL END, the SFDU label, and the record     **/
	    /**       terminator to the label file                        **/
            /**    pad the label to number of records needed              **/
	    /*-------------------------------------------------------------*/

	    if (record_type != PDS_RF_BINARY && unknown_type == FALSE)
	    {
               if (end_sfdu != NULL)
               {
	          ODLWriteStmt (file_ptr, "END ");
	          ODLWriteStmt (file_ptr, end_sfdu -> sfdu_label);
  	          ODLWriteStmt (file_ptr, pds_record_term);
	       }
               while (pds_records_needed > pds_records_written)
     	            ODLWriteStmt (file_ptr, pds_record_term);
	       fclose (file_ptr);
	    }

	    /*-------------------------------------------------------------*/
	    /** IF the records are fixed THEN                             **/
            /**    create a pad record                                    **/
            /**    IF there is a second SFDU THEN                         **/
            /**       pad to number of records needed                     **/
            /**       add a last padded record containing the SFDU label  **/
            /**    ELSE                                                   **/
            /**       pad to number of records needed                     **/
            /**    ENDIF                                                  **/
            /** ENDIF                                                     **/
	    /*-------------------------------------------------------------*/

	    if (record_type == PDS_RF_BINARY)
	    { 
	       Malloc_String (last_record, record_length + 1);
	       for (i = 1, temp_ptr = last_record; 
                    i <= record_length; i++, temp_ptr++) 
                    *temp_ptr = ' ';
               if (end_sfdu != NULL)
               {
		   for (;pds_records_needed > pds_records_written; 
                         pds_records_written++)
        	        fputs (last_record, file_ptr);
	           space_needed = (int) strlen (end_sfdu -> sfdu_label);
	           for (i = 1, temp_ptr = last_record; 
                       i <= record_length - space_needed; i++, temp_ptr++) 
                       *temp_ptr = ' ';
                   strcpy (temp_ptr, end_sfdu -> sfdu_label);
	           fputs (last_record, file_ptr);
                   pds_records_written++;
	       }
               else
               {
		   for (; pds_records_needed > pds_records_written; 
                        pds_records_written++)
        	        fputs (last_record, file_ptr);
               }
	       fclose (file_ptr);
               Lemme_Go(last_record);
	    }
	    /*-------------------------------------------------------------*/
	    /** IF the records are not fixed but the type was unknown     **/
            /**    and there is a second SFDU                             **/
	    /**    pad to number of label records needed                  **/
	    /**    append the SFDU label to the label file                **/
	    /*-------------------------------------------------------------*/

	    if (unknown_type == TRUE)
 	    {
               while (pds_records_needed > pds_records_written)
     	            ODLWriteStmt (file_ptr, pds_record_term);
	       if (end_sfdu != NULL) ODLWriteStmt (file_ptr, end_sfdu -> sfdu_label);
	       fclose (file_ptr);
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
      /** ENDIF                                                           **/
      /*-------------------------------------------------------------------*/
   }
   /*----------------------------------------------------------------------*/
   /** ELSE                                                               **/
   /**    inputs are bad, so set failure flag                             **/
   /*----------------------------------------------------------------------*/

   else
      success = FALSE;

   /*---------------------------------------------------------------------*/
   /** ENDIF                                                             **/
   /** set status based on message list contents                         **/ 
   /** restore modified global values                                    **/ 
   /*---------------------------------------------------------------------*/

   *label_status = (success && temp_msg == pds_last_message) ? 
                    PDS_SUCCESS : PDS_ERROR;

   strcpy (pds_record_term, save_term);
   pds_finish_label = save;
   pds_default_reclen = save_reclen;
   return (*label_status == PDS_SUCCESS);

/** END lab_write_label **/
}

/**********************************************************************
 *$Component                                                          *
 *    LOGICAL lab_write_product_label (label_ptr, top_ddid,           *
 *                     bottom_ddid, version_id, label_type,           *
 *                     new_record_type, record_length,                *
 *                     file_name)                                     *
 *$Abstract                                                           *
 *    Writes out a product label to an output label file.             *
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
 *    version_id:                                                     *
 *        The version_id variable is an integer that holds the        *
 *        version ID of an SFDU label (1 or 3).                       * 
 *    label_type:                                                     *
 *        The label_type variable is a pointer to a character         *
 *        string that contains the type of an SFDU label (ZK, ZI, or  *
 *        NO_SFDUS)                                                   *
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
 *        This routine is a router.  It merely calls the appropriate  *
 *        label write routine  based on the input arguments.          *
 *        This routine handles version 1 and version 3 "ZI" label     *
 *        structures, version 3 "ZK" strcutures, and "NO_SFDUS".      *
 *$External_References                                                *
 *    None                                                            *
 *$Author_and_Institution                                             *
 *    Marti D. DeMore/JPL                                             *
 *$Version_and_Date                                                   *
 *    1.2   March 17, 1992                                            *
 *$Change_History                                                     *
 *    MDD   05-13-91   Original Code.                                 *
      KLM   06-17-91   Changed calls from lab_write_z.._label to      *
 *                     lu_write_z.._label.                            *
 *    MDD   03-17-92   The great int -> long conversion               *
 **********************************************************************/

LOGICAL lab_write_product_label (label_ptr, top_ddid, bottom_ddid, version_id,
                                 label_type, new_record_type, record_length, 
                                 file_name)
AGGREGATE label_ptr;
char *top_ddid;
char *bottom_ddid;
int version_id;
char *label_type;
int new_record_type;
long record_length;
char *file_name;
{
   LOGICAL success = FALSE;

   util_strip_lead_and_trail (label_type, ' ');
   util_upper_case (label_type);

   if (strcmp (label_type, "NO SFDUS") == 0) 
      success = lab_write_label_or_template 
           (label_ptr, new_record_type, record_length, file_name);
   else if (strcmp (label_type, "ZK") == 0 && version_id == 3)
      success = lu_write_zk3_label (label_ptr, top_ddid, bottom_ddid,
                                    new_record_type, record_length, file_name);
   else if (strcmp (label_type, "ZI") == 0 && version_id == 1)
      success = lu_write_zi1_label (label_ptr, top_ddid, new_record_type,
                                     record_length, file_name);
   else if (strcmp (label_type, "ZI") == 0 && version_id == 3)
      success = lu_write_zi3_label (label_ptr, top_ddid, new_record_type, 
                                     record_length, file_name);
   else
   {
      err_append_message (ERROR1,
         "A label file could not be written to your specifications");
   }       
   return (success);
}
