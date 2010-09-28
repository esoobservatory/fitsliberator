/*****************************************************************************
 
  Module:  odldef.h

  Description:  This C-language include file contains declarations for
                data structures used by PDS Object Description Language
                (ODL) processing software.  No storage is allocated.
 
  Author:  Randy Davis, University of Colorado LASP
 
  Creation Date: 12 March 1989
  Last Modified: 23 March 1992

  History:

  Creation - This include file was included in the Version 1 release of
  the ODLC library.

  Version 2.0 - 30 August 1990 - R. Davis, U. of Colorado LASP
    a) Data structures and function prototypes have been modified to support
       ODL Version 2. The biggest change is that the Object_Node data
       structure has been supplanted by an Aggregate_Node that handles
       groups as well as objects, and the Attribute_Node structure by a
       Parameter_Node structure for both attributes and pointers.
  
  Version 2.1 - 13 March 1991 - R. Davis, U. of Colorado LASP
    a) Added support for comments for aggregate and parameter nodes.  Also
       added two fields to aggregates node for use by end-user applications.
    b) Eliminated the level field for aggregate structures.

  Version 2.2 - 18 May 1991- M. DeMore, Jet Propulsion Laboratory
    a) Added two fields to the parameter node for use by the PDS toolbox.
    b) Reorganized and added prototypes to create generic version for both
       Unix and VMS

  Version 2.3 - 23 March 1992 - M. DeMore, Jet Propulsion Laboratory
    a) Modified prototypes to function on all platforms.

  Version 2.4 - 04 December 2002 - DWS Changed FindObject in AO_NODES.C

  Version 2.5 - 09 January  2006 - MDC Add Yamane's changes. Some reason, it 
                                       was lost.
*****************************************************************************/
#ifndef ODLDEF
#define ODLDEF

/*------------------------------------------------------------------------*/
/*                                Includes                                */
/*------------------------------------------------------------------------*/

#ifdef SUN_UNIX

#include <stdio.h>
#include <string.h>

#endif

#ifdef MSDOS_TC

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#endif

#ifdef VAX         

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#endif

#ifdef MAC_THINK

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#endif

#ifdef OSX
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

/*------------------------------------------------------------------------*/
/*                                Defines                                 */
/*------------------------------------------------------------------------*/

/* The following value identifies the version of the Object Description
   Language to which the data structures and associated software conforms.  */
 
#define  ODLVERSION "2"

/* The following constant specifies the maximum number of characters allowed
   in an ODL statement.                                                     */

#ifdef VAX
    
#define  ODLMAXSTMT  24000

#endif

#ifdef SUN_UNIX
    
#define  ODLMAXSTMT  24000

#endif

#ifdef MSDOS_TC

#define ODLMAXSTMT 24000

#endif

#ifdef MAC_THINK

#define ODLMAXSTMT 24000

#endif

#ifdef OSX
#define ODLMAXSTMT	24000
#endif

/*------------------------------------------------------------------------*/
/*                     Enums and Enum Typedefs                            */
/*------------------------------------------------------------------------*/

enum     Aggregate_Kind          {KA_OBJECT, KA_GROUP};
typedef  enum Aggregate_Kind     AGGREGATE_KIND;

enum     Parameter_Kind          {KP_ATTRIBUTE, KP_POINTER};
typedef  enum Parameter_Kind     PARAMETER_KIND;

enum     Value_Kind              {KV_UNKNOWN, KV_SCALAR, KV_SEQUENCE, KV_SET};
typedef  enum Value_Kind         VALUE_KIND;

enum     Value_Type              {TV_NULL, TV_INTEGER, TV_REAL, TV_SYMBOL,
				  TV_STRING, TV_DATE, TV_TIME, TV_DATE_TIME,
                                  TV_NULL_VALUE };
typedef  enum Value_Type         VALUE_TYPE;


/*------------------------------------------------------------------------*/
/*                       Structure Definitions                            */
/*------------------------------------------------------------------------*/

#ifdef PDS_TOOLBOX

/* The following structure stores the SFDU label strings found in a PDS
   label, as well as the beginning and ending offsets of the data in
   the SFDU. */

typedef struct sfdu_struct
{
   long begin_offset;
   long end_offset;
   char *sfdu_label;
   struct sfdu_struct *next;
} SFDU_INFO;

#endif

/* The following structure contains information about a single object or
   group including its name and (for objects) its pds_class.  The structure
   contains pointers that thread the object/group nodes together and that
   connect an object or group with attributes.  */

struct Aggregate_Node
{
   char                  *name;            /* Name of the object or group   */
   char                  *pds_class;           /* The pds_class of object           */
   char                  *comment;         /* Annotation                    */
   AGGREGATE_KIND         kind;            /* Kind of node (object or group)*/
   long                   appl1;           /* Available for application use */
   long                  *appl2;           /* Available for application use */
   struct Aggregate_Node *parent;          /* Pointer to parent node        */
   struct Aggregate_Node *left_sibling;    /* Pointer to sibling on left    */
   struct Aggregate_Node *right_sibling;   /* Pointer to sibling on right   */
   struct Aggregate_Node *first_child;     /* Pointer to first child        */
   struct Aggregate_Node *last_child;      /* Pointer to last child         */
   struct Parameter_Node *first_parameter; /* Pointer to first parameter    */
   struct Parameter_Node *last_parameter;  /* Pointer to last parameter     */
#ifdef PDS_TOOLBOX
   SFDU_INFO             *sfdu_list;       /* The list of SFDU labels       */
#endif
};

/* The following structure contains information about a parameter -- an
   attribute of an object or a pointer.  This includes the attribute or
   pointer name and the kind of value associated with the parameter
   (scalar, sequence or set).  For sequences the number of rows and
   columns are given.  Each parameter node contains a pointer to the
   aggregate -- group or object -- node to which it belongs and a list
   of values associated with the parameter.  All of the parameter nodes
   for an aggregat are threaded together in a doubly-linked list.  */

struct Parameter_Node
{
   char                  *name;           /* The name of the parameter      */
   char                  *comment;        /* Annotation                     */
   PARAMETER_KIND         node_kind;      /* Param kind (Attribute or Ptr)  */
   VALUE_KIND             value_kind;     /* Indicates the kind of value    */
   long                   value_count;    /* Total number of values         */
   short                  columns;        /* Number of columns in sequence  */
   short                  rows;           /* Number of rows in sequence     */
   struct Aggregate_Node *owner;          /* Pointer to parameter's owner   */
   struct Parameter_Node *left_sibling;   /* Pointer to parameter to left   */
   struct Parameter_Node *right_sibling;  /* Pointer to parameter to right  */
   struct Value_Node     *first_value;    /* Pointer to first value node    */
   struct Value_Node     *last_value;     /* Pointer to last value node     */

#ifdef PDS_TOOLBOX

  long appl1;                              /* Application defined field 1    */
  long appl2;                              /* Application defined field 2    */

#endif

};

/* The following structures are used to hold information on a single value.
   The ODLReal structure holds a double precision floating point value
   and a pointer to a (possibly null) list of units designators which
   are represented through the ODLUnits structure.  Similarly, the
   ODLInteger structure holds a long integer value and units. The
   ODLDate structure holds date and time values.                            */

struct ODLUnits
{
  char             *designator;           /* Pointer to name of units       */
  long              exponent;             /* Exponent for units             */
  struct ODLUnits  *next_field;           /* Pointer to next units          */
};


struct ODLReal
{
  double            number;               /* The floating numeric value     */
  struct ODLUnits  *units;                /* Pointer to units list          */
};


struct ODLInteger
{
  long              number;               /* The integer numeric value      */
  struct ODLUnits  *units;                /* Pointer to units list          */
};


struct ODLDate
{
  short             year;                 /* Year number                    */
  short             doy;                  /* Day of year (0-366)            */
  char              month;                /* Month of year (1-12)           */
  char              day;                  /* Day of month (1-31)            */
  short             zone_hours;           /* Zone hours from GMT (-12 - +12)*/
  char              zone_minutes;         /* Zone minutes from GMT (0-59)   */
  char              hours;                /* Hours of day (0-23)            */
  char              minutes;              /* Minutes of hour (0-59)         */
  char              seconds;              /* Seconds of minute (0-59)       */
  long              nanoseconds;          /* Nanoseconds (0-999999999)      */
};

struct ODLNull
{
    char             *string;     /* Pointer to text value          */
    struct ODLUnits  *units;                /* Pointer to units list          */
};

struct Value_Data
{
   VALUE_TYPE                 type;       /* Indicator of value type        */
   char                       valid;      /* Value valid if this is > 0     */
   char                       format;     /* Value-specific format indicator*/
   short                      precision;  /* Value-specific precision       */
   short                      length;     /* Number of chars in value field */
   union {
            struct ODLInteger integer;    /* Integer value                  */
            struct ODLReal    real;       /* Real value                     */
            struct ODLDate    date_time;  /* Date and time value            */
			struct ODLNull    null_data;  /* Null value                     */
            char             *string;     /* Pointer to text value          */
         } value;
};


struct Value_Node
{
   struct Value_Data      item;           /* Data value                     */
   struct Parameter_Node *parameter;      /* Pointer to value's parameter   */
   struct Value_Node     *left_sibling;   /* Pointer to value to left       */
   struct Value_Node     *right_sibling;  /* Pointer to value to right      */
};

/*------------------------------------------------------------------------*/
/*                               Typedefs                                 */
/*------------------------------------------------------------------------*/

/* Because object, group, attribute and value nodes are created in virtual
   memory, they are almost always referred to using pointers. The following
   types are therefore defined to be equivalent to pointers to the
   aggregate, parameter and value structures */

typedef struct Aggregate_Node *AGGREGATE;
typedef struct Aggregate_Node *OBJECT;
typedef struct Aggregate_Node *GROUP;

typedef struct Parameter_Node *PARAMETER;
typedef struct Parameter_Node *ATTRIBUTE;
typedef struct Parameter_Node *POINTER;

typedef struct Value_Node     *VALUE;

/* The following provides an alternative name for the often-used structure
   that is used to hold a data value.                                       */

typedef struct Value_Data      VALUE_DATA;


/*------------------------------------------------------------------------*/
/*                                 Macros                                 */
/*------------------------------------------------------------------------*/

#define AGGREGATE_NODE_SIZE (sizeof(struct Aggregate_Node))

#define PARAMETER_NODE_SIZE (sizeof(struct Parameter_Node))

#define VALUE_DATA_SIZE (sizeof(struct Value_Data))

#define VALUE_NODE_SIZE (sizeof(struct Value_Node))


/*------------------------------------------------------------------------*/
/*                              Prototypes                                */
/*------------------------------------------------------------------------*/

#ifndef SUN_UNIX

AGGREGATE NewAggregate      (AGGREGATE, AGGREGATE_KIND, char *, char *);
AGGREGATE RemoveAggregate   (AGGREGATE);
AGGREGATE ParentAggregate   (AGGREGATE);
AGGREGATE NextAggregate     (AGGREGATE);
AGGREGATE NextSubAggregate  (AGGREGATE, AGGREGATE);
AGGREGATE FindAggregate     (AGGREGATE, char *);
AGGREGATE FindNextAggregate (AGGREGATE, AGGREGATE, char *);
AGGREGATE CutAggregate      (AGGREGATE);
AGGREGATE CopyAggregate     (AGGREGATE);
AGGREGATE PasteAggregate    (AGGREGATE, AGGREGATE);

OBJECT ParentObject   (AGGREGATE);
OBJECT NextObject     (AGGREGATE);
OBJECT NextSubObject  (AGGREGATE, AGGREGATE);
OBJECT FindObject     (AGGREGATE, char *, char *);
OBJECT FindNextObject (AGGREGATE, AGGREGATE, char *, char *);

GROUP ParentGroup   (AGGREGATE);
GROUP NextGroup     (AGGREGATE);
GROUP NextSubGroup  (AGGREGATE, AGGREGATE);
GROUP FindGroup     (AGGREGATE, char *);
GROUP FindNextGroup (AGGREGATE, AGGREGATE, char *);

PARAMETER NewParameter    (AGGREGATE, PARAMETER_KIND, char *);
PARAMETER RemoveParameter (PARAMETER);
PARAMETER FirstParameter  (AGGREGATE);
PARAMETER NextParameter   (PARAMETER);
PARAMETER FindParameter   (AGGREGATE, char *);
PARAMETER CutParameter    (PARAMETER);
PARAMETER CopyParameter   (PARAMETER);
PARAMETER PasteParameter  (AGGREGATE, PARAMETER);

VALUE NewValue    (PARAMETER, VALUE_DATA *);
VALUE RemoveValue (VALUE);
VALUE FirstValue  (PARAMETER);
VALUE NextValue   (VALUE);
VALUE CutValue    (VALUE);
VALUE CopyValue   (VALUE);
VALUE PasteValue  (PARAMETER, VALUE);

int  CommentAggregate (AGGREGATE, char *);
int  CommentParameter (PARAMETER, char *);
void StripComments    (void);

int  ReadLabel   (FILE *, AGGREGATE);
int  ReadValue   (AGGREGATE, char *, char *);
void WriteLabel  (FILE *, AGGREGATE);
void WriteSource (FILE *, FILE*);
void PrintLabel  (AGGREGATE);
void PrintSource (FILE *, long, long);

#else

AGGREGATE NewAggregate      ();
AGGREGATE RemoveAggregate   ();
AGGREGATE ParentAggregate   ();
AGGREGATE NextAggregate     ();
AGGREGATE NextSubAggregate  ();
AGGREGATE FindAggregate     ();
AGGREGATE FindNextAggregate ();
AGGREGATE CutAggregate      ();
AGGREGATE CopyAggregate     ();
AGGREGATE PasteAggregate    ();

OBJECT ParentObject   ();
OBJECT NextObject     ();
OBJECT NextSubObject  ();
OBJECT FindObject     ();
OBJECT FindNextObject ();

GROUP ParentGroup   ();
GROUP NextGroup     ();
GROUP NextSubGroup  ();
GROUP FindGroup     ();
GROUP FindNextGroup ();

PARAMETER NewParameter    ();
PARAMETER RemoveParameter ();
PARAMETER FirstParameter  ();
PARAMETER NextParameter   ();
PARAMETER FindParameter   ();
PARAMETER CutParameter    ();
PARAMETER CopyParameter   ();
PARAMETER PasteParameter  ();

VALUE NewValue    ();
VALUE RemoveValue ();
VALUE FirstValue  ();
VALUE NextValue   ();
VALUE CutValue    ();
VALUE CopyValue   ();
VALUE PasteValue  ();

int  CommentAggregate ();
int  CommentParameter ();
void StripComments    ();

int  ReadLabel   ();
int ReadValue    ();
void WriteLabel  ();
void WriteSource ();
void PrintLabel  ();
void PrintSource ();

#endif
#endif
