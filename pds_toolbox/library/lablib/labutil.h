/**********************************************************************
 * Component                                                          *
 *    Include file labutil.h                                          *
 * Used By                                                            *
 *    PDS label software.                                             *
 * Detailed Description                                               *
 *    Defines symbols, macros, flags, typedefs, and structures        *
 *    used by PDS label software.                                     *
 * Author and Institution                                             *
 *    David P. Bernath / J.P.L.                                       *
 * Version and Date                                                   *
 *    1.3 June 2, 1992                                                *
 * Change History                                                     *
 *    KLM   06-12-91   Low level utility routines from label.c        *
 *    DPB   12-04-91   Added lu_keyword_value prototype.              *
 *    MDD   03-23-92   Modified function prototypes.                  *
 *    MDD   06-02-92   Added SFDU symbols                             *
 *    MDC   12-22-03   Added function prototype for lu_get_units      *
 **********************************************************************/

/*--------------------------------------------------------------------*/
/*                              SFDU symbols                          */
/*--------------------------------------------------------------------*/

#define PDS_SFDU_BUFSIZE      		512
#define PDS_SFDU_MAXBUFFERS             500
#define PDS_SFDU_LEN           		20
#define PDS_SFDU_CAID_START		0
#define PDS_SFDU_CAID_LEN		4
#define PDS_SFDU_VERSION		4
#define PDS_SFDU_CLASS              	5
#define PDS_SFDU_DELIM		        6
#define PDS_SFDU_SPARE_1		6
#define PDS_SFDU_SPARE_2		7
#define PDS_SFDU_DDID_START	   	8
#define PDS_SFDU_DDID_LEN		4
#define PDS_SFDU_VALUE_START		12
#define PDS_SFDU_VALUE_LEN		8


/*--------------------------------------------------------------------*/
/*                     LABUTIL Function Prototypes                    */
/*--------------------------------------------------------------------*/

#ifndef SUN_UNIX

AGGREGATE lu_append_object (AGGREGATE, char *);

PARAMETER lu_append_parameter (AGGREGATE, char *, char *);

VALUE lu_append_value (PARAMETER, char *);

char *lu_fetch_value (VALUE, LOGICAL);

char *lu_get_units(VALUE);

STRING_LIST *lu_fetch_all_values (PARAMETER, LOGICAL, LOGICAL);

int lu_find_object_level (AGGREGATE);

char *lu_format_date (VALUE_DATA *);

char *lu_format_date_time (VALUE_DATA *);

char *lu_format_integer (VALUE_DATA *);

char *lu_format_real (VALUE_DATA *);

char *lu_format_string (VALUE_DATA *, LOGICAL);

char *lu_format_symbol (VALUE_DATA *, LOGICAL);

char *lu_format_time (VALUE_DATA *);

char *lu_format_string (VALUE_DATA *, LOGICAL);

char *lu_format_null_data (VALUE_DATA *, LOGICAL);

char *lu_format_units (struct ODLUnits *);

char *lu_keyword_value (AGGREGATE, char *, int, LOGICAL);

SFDU_INFO *lu_locate_sfdus (FILE *);

VALUE lu_new_value (PARAMETER, VALUE_DATA *);

LOGICAL lu_parse_string (AGGREGATE, char *);

PARAMETER lu_paste_parm_at_pos (AGGREGATE, PARAMETER, int);

LOGICAL lu_write_zi1_label (AGGREGATE, char *, int, long, char *);

LOGICAL lu_write_zi3_label (AGGREGATE, char *, int, long, char *);

LOGICAL lu_write_zk3_label (AGGREGATE, char *, char *, int, long, char *);

#else

AGGREGATE lu_append_object ();
PARAMETER lu_append_parameter ();
VALUE lu_append_value ();
STRING_LIST *lu_fetch_all_values ();
char *lu_fetch_value ();
char *lu_get_units();
int lu_find_object_level ();
char *lu_format_date ();
char *lu_format_date_time ();
char *lu_format_integer ();
char *lu_format_real ();
char *lu_format_string ();
char *lu_format_symbol ();
char *lu_format_time ();
char *lu_format_null_data ();
char *lu_format_units ();
char *lu_keyword_value ();
SFDU_INFO *lu_locate_sfdus ();
VALUE lu_new_value ();
LOGICAL lu_parse_string ();
PARAMETER lu_paste_parm_at_pos ();
LOGICAL lu_write_zi1_label ();
LOGICAL lu_write_zi3_label ();
LOGICAL lu_write_zk3_label ();

#endif

