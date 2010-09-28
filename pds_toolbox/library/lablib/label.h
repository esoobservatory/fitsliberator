
/**********************************************************************
 * Component                                                          *
 *    Include file label.h                                            *
 * Used By                                                            *
 *    PDS label software.                                             *
 * Detailed Description                                               *
 *    Defines symbols, macros, flags, typedefs, and structures        *
 *    used by PDS label software.                                     *
 * Author and Institution                                             *
 *    David P. Bernath / J.P.L.                                       *
 * Version and Date                                                   *
 *    2.0   June 19, 1992                                             *
 * Change History                                                     *
 *    DPB   04-10-91   Original code.                                 *
 *    MDD   04-25-91   Added get and format value routines            *
 *    DPB   05-30-91   Added lab_fetch_all_values                     *
 *    KLM   06-25-91   Added lab_skip_sfdus                           *
 *    DPB   07-25-91   Added lab_match_classes.                       *
 *    MDD   01-24-92   Added lab_exit and lab_setup macros            *
 *    MDD   03-23-92   Modified function prototypes.                  *
 *    MDD   06-19-92   Added new prototypes and notice of obsolete    *
 *                     functions.                                     *
 **********************************************************************/ 

/*---------------------------------------------------------------------------*/
/*                         LABEL Define Statements                           */
/*---------------------------------------------------------------------------*/

#ifdef SUN_UNIX
#define PDS_LABEL_ROOT           "/cdrom/label"
#endif

#ifdef VAX
#define PDS_LABEL_ROOT          "[label]"
#endif

#ifdef MSDOS_TC
#define PDS_LABEL_ROOT          "\\label"
#endif

#ifdef MAC_THINK
#define PDS_LABEL_ROOT          ""
#endif

#ifdef OSX
#define PDS_LABEL_ROOT			"/label"
#endif

/*---------------------------------------------------------------------------*/
/*                        LABEL Function Prototypes                          */
/*---------------------------------------------------------------------------*/

#ifndef SUN_UNIX
                                              
LOGICAL lab_adjust_pointers (AGGREGATE, long, int *);

AGGREGATE lab_add_object (AGGREGATE, char *, char *, int, char *, int *);

PARAMETER lab_add_parameter (AGGREGATE, char *, char *, int, char *, char *, 
                             int *);

LOGICAL lab_change_object_class (AGGREGATE, char *, char *, int, char *, int *);

LOGICAL lab_change_parameter_name (AGGREGATE, char *, char *, int, char *, int,
                                   char *, int *);

VALUE lab_change_value (AGGREGATE, char *, char *, int, char *, int, char *, 
                        int *);

void lab_clear_messages (void);

LOGICAL lab_exit (void);

AGGREGATE lab_find_object (AGGREGATE, char *, char *, int, int *);

PARAMETER lab_find_parameter (AGGREGATE, char *, char *, int, char *, int, int *);

STRING_LIST *lab_get_all_values (AGGREGATE, char *, char *, int, char *, int,
                                 LOGICAL, int *);

char *lab_get_value (AGGREGATE, char *, char *, int, char *, int, int,
                     LOGICAL, int *);

LOGICAL lab_has_messages (void);

LOGICAL lab_match_classes (char *, char *);

PARAMETER lab_move_parameter (AGGREGATE, char *, char *, int, char *, int,
                              int, int *);

LOGICAL lab_print_label (AGGREGATE, int *);

void lab_print_messages (void);

void lab_remove_sfdu_labels (AGGREGATE, int *);

AGGREGATE lab_read_label (char *, int *);

AGGREGATE lab_remove_label (AGGREGATE, int *);

AGGREGATE lab_remove_object (AGGREGATE, char *, char *, int, int *);

AGGREGATE lab_remove_parameter (AGGREGATE, char *, char *, int, char *, int,
                                int *);

LOGICAL lab_setup (void);

AGGREGATE lab_start_label (int *);

LOGICAL lab_write_attached_label (AGGREGATE, char *, char *, long, int *);

LOGICAL lab_write_label (AGGREGATE, char *, LOGICAL, long, int *);

LOGICAL lab_write_product_label (AGGREGATE, char *, char *, int, char *, int,
                                 long, char *);

LOGICAL lab_replace_pointer (AGGREGATE, POINTER_INFO *, int *);

POINTER_INFO *lab_get_pointer (AGGREGATE, char *, int, int *);

/*------------------------------------------------------------------------*/
/* The following functions are obsolete, and should not be used by new    */
/* programs.                                                              */
/*------------------------------------------------------------------------*/


VALUE lab_add_value (AGGREGATE, char *, char *, int, char *, int, char *, int *);

LOGICAL lab_print_pds_label (AGGREGATE);

AGGREGATE lab_read_label_or_template (char *);

AGGREGATE lab_remove_label_or_template (AGGREGATE);

LOGICAL lab_write_label_or_template (AGGREGATE, int, long, char *);


#else

AGGREGATE lab_add_object ();
PARAMETER lab_add_parameter ();
LOGICAL lab_adjust_pointers ();
LOGICAL lab_change_object_class ();
LOGICAL lab_change_parameter_name ();
VALUE lab_change_value ();
void lab_clear_messages ();
LOGICAL lab_exit ();
AGGREGATE lab_find_object ();
PARAMETER lab_find_parameter ();
STRING_LIST *lab_get_all_values ();
POINTER_INFO *lab_get_pointer ();
char *lab_get_value ();
LOGICAL lab_has_messages ();
PARAMETER lab_move_parameter ();
LOGICAL lab_match_classes ();
void lab_print_messages ();
LOGICAL lab_print_label ();
AGGREGATE lab_read_label ();
AGGREGATE lab_remove_label ();
AGGREGATE lab_remove_object ();
AGGREGATE lab_remove_parameter ();
void lab_remove_sfdu_labels ();
LOGICAL lab_replace_pointer (); 
LOGICAL lab_setup ();
AGGREGATE lab_start_label ();
LOGICAL lab_write_attached_label ();
LOGICAL lab_write_label ();
LOGICAL lab_write_product_label ();

/*------------------------------------------------------------------------*/
/* The following functions are obsolete, and should not be used by new    */
/* programs.                                                              */
/*------------------------------------------------------------------------*/

VALUE lab_add_value ();
LOGICAL lab_print_pds_label ();
AGGREGATE lab_read_label_or_template ();
AGGREGATE lab_remove_label_or_template ();
LOGICAL lab_write_label_or_template ();

#endif


