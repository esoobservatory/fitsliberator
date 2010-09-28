/*****************************************************************************

  File:  odlutils.c

  Description:  This file contains various utility functions used by the OA
                library for manipulating ODL trees, including ASCII-to-binary
                and binary-to-ASCII conversion functions for keyword values.
                The routines in this file are:

                OaCopyTree
                OaDealiasKwdName
                OaDeleteKwd
                OaGetObjectClass
                OaGetObjectInterchangeFormat
                OaGetImageKeywords
                OaGetQubeKeywords
                OaGetTableKeywords
                OaGetTableStorageType
                OaKwdValuetoDouble
                OaKwdValuetoLong

                OaLongtoKwdValue
                OaObjectClasstoStr
                OaPDSDataTypetoStr
                OaSequencetoLongArray
                OaSequencetoStrArray
                OaStrtoKwdValue
                OaStrtoPDSDataType
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:  16 Mar   1998

  History:

    Creation - These routines were part of the Alpha Release of the OA library.
    10/06/94 - Added "options" argument to OaCopyTree.  SM
    11/27/95 - Added QUBE to oa_class_names and oa_class_types.  SM
    11/27/95 - Added OaGetImageKeywords, OaGetQubeKeywords and
               OaGetTableKeywords.  SM
    12/06/95 - Replaced malloc() by OaMalloc() throughout.  SM
    12/11/95 - Added error codes.  SM
    07/18/96 - Added OaKwdValuetoDouble. SM
    12/11/96 - Added arguments to OaGetImageKeywords.  SM
    06/24/97 - Added OA_BCD to OaPDS_data_type_info.  SM
    01/19/98 - Added "BINARY CODED DECIMAL" and BINARY_CODED_DECIMAL as 
               aliases for BCD.  SM
    03/16/98 - Added OA_GIF encoding type to OaGetImageKeywords.  SM
	01/12/03 - changed OaGetQubeKeywords to allow for THEMIS  DWS
  Notes:  

*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include "oal.h"

/******************************************************************************
The following array of structures, OaPDS_data_type_info, is a global 
variable in this file.  It maps object class enumeration types (in oal.h)
into strings, and flags whether or not the class name is an alias.  It is
used by the functions OaPDSDataTypetoStr and OaStrtoPDSDataType.  
In a later version this may be replaced or augmented by PDS Data Dictionary
access.
******************************************************************************/

#define N_PDS_DATA_TYPE_INFO 50

struct OaPDS_data_type_info {
  int PDS_data_type;
  char *data_type_str;
  int ASCII_data_type;
  char is_an_alias;
};

static struct OaPDS_data_type_info 
  OaPDS_data_type_info[N_PDS_DATA_TYPE_INFO] = {
{OA_ASCII_REAL,           "ASCII_REAL",           OA_ASCII_REAL,        FALSE},
{OA_ASCII_INTEGER,        "ASCII_INTEGER",        OA_ASCII_INTEGER,     FALSE},
{OA_ASCII_COMPLEX,        "ASCII_COMPLEX",        OA_ASCII_COMPLEX,     FALSE},
{OA_ASCII_BIT_STRING,     "ASCII_BIT_STRING",     OA_ASCII_BIT_STRING,  FALSE},
{OA_BCD,                  "BCD",                  OA_ASCII_BIT_STRING,  FALSE},
{OA_BCD,                  "BINARY CODED DECIMAL", OA_ASCII_BIT_STRING,  TRUE},
{OA_BCD,                  "BINARY_CODED_DECIMAL", OA_ASCII_BIT_STRING,  TRUE},
{OA_BIT_STRING,           "BIT_STRING",           OA_ASCII_BIT_STRING,  TRUE},
{OA_BOOLEAN,              "BOOLEAN",              OA_ASCII_INTEGER,     FALSE},
{OA_CHARACTER,            "CHARACTER",            OA_CHARACTER,         FALSE},
{OA_COMPLEX,              "COMPLEX",              OA_ASCII_COMPLEX,     TRUE},
{OA_DATE,                 "DATE",                 OA_DATE,              FALSE},
{OA_FLOAT,                "FLOAT",                OA_ASCII_REAL,        TRUE},
{OA_IBM_COMPLEX,          "IBM_COMPLEX",          OA_ASCII_COMPLEX,     FALSE},
{OA_IBM_INTEGER,          "IBM_INTEGER",          OA_ASCII_INTEGER,     FALSE},
{OA_IBM_REAL,             "IBM_REAL",             OA_ASCII_REAL,        FALSE},
{OA_IBM_UNSIGNED_INTEGER, "IBM_UNSIGNED_INTEGER", OA_ASCII_INTEGER,     FALSE},
{OA_IEEE_COMPLEX,         "IEEE_COMPLEX",         OA_ASCII_COMPLEX,     FALSE},
{OA_IEEE_REAL,            "IEEE_REAL",            OA_ASCII_REAL,        FALSE},
{OA_INTEGER,              "INTEGER",              OA_ASCII_INTEGER,     TRUE},
{OA_LSB_BIT_STRING,       "LSB_BIT_STRING",       OA_ASCII_BIT_STRING,  FALSE},
{OA_LSB_INTEGER,          "LSB_INTEGER",          OA_ASCII_INTEGER,     FALSE},
{OA_LSB_UNSIGNED_INTEGER, "LSB_UNSIGNED_INTEGER", OA_ASCII_INTEGER,     FALSE},
{OA_MAC_COMPLEX,          "MAC_COMPLEX",          OA_ASCII_COMPLEX,     TRUE},
{OA_MAC_INTEGER,          "MAC_INTEGER",          OA_ASCII_INTEGER,     TRUE},
{OA_MAC_REAL,             "MAC_REAL",             OA_ASCII_REAL,        TRUE},
{OA_MAC_UNSIGNED_INTEGER, "MAC_UNSIGNED_INTEGER", OA_ASCII_INTEGER,     TRUE},
{OA_MSB_BIT_STRING,       "MSB_BIT_STRING",       OA_ASCII_BIT_STRING,  FALSE},
{OA_MSB_INTEGER,          "MSB_INTEGER",          OA_ASCII_INTEGER,     FALSE},
{OA_MSB_UNSIGNED_INTEGER, "MSB_UNSIGNED_INTEGER", OA_ASCII_INTEGER,     FALSE},
{OA_PC_COMPLEX,           "PC_COMPLEX",           OA_ASCII_COMPLEX,     FALSE},
{OA_PC_INTEGER,           "PC_INTEGER",           OA_ASCII_INTEGER,     TRUE},
{OA_PC_REAL,              "PC_REAL",              OA_ASCII_REAL,        FALSE},
{OA_PC_UNSIGNED_INTEGER,  "PC_UNSIGNED_INTEGER",  OA_ASCII_INTEGER,     TRUE},
{OA_REAL,                 "REAL",                 OA_ASCII_REAL,        TRUE},
{OA_SUN_COMPLEX,          "SUN_COMPLEX",          OA_ASCII_COMPLEX,     TRUE},
{OA_SUN_INTEGER,          "SUN_INTEGER",          OA_ASCII_INTEGER,     TRUE},
{OA_SUN_REAL,             "SUN_REAL",             OA_ASCII_REAL,        TRUE},
{OA_SUN_UNSIGNED_INTEGER, "SUN_UNSIGNED_INTEGER", OA_ASCII_INTEGER,     TRUE},
{OA_TIME,                 "TIME",                 OA_TIME,              FALSE},
{OA_UNSIGNED_INTEGER,     "UNSIGNED_INTEGER",     OA_ASCII_INTEGER,     TRUE},
{OA_VAX_BIT_STRING,       "VAX_BIT_STRING",       OA_ASCII_INTEGER,     TRUE},
{OA_VAX_COMPLEX,          "VAX_COMPLEX",          OA_ASCII_COMPLEX,     FALSE},
{OA_VAX_DOUBLE,           "VAX_DOUBLE",           OA_ASCII_COMPLEX,     TRUE},
{OA_VAX_INTEGER,          "VAX_INTEGER",          OA_ASCII_INTEGER,     TRUE},
{OA_VAX_REAL,             "VAX_REAL",             OA_ASCII_REAL,        FALSE},
{OA_VAX_UNSIGNED_INTEGER, "VAX_UNSIGNED_INTEGER", OA_ASCII_INTEGER,     TRUE},
{OA_VAXG_COMPLEX,         "VAXG_COMPLEX",         OA_ASCII_COMPLEX,     FALSE},
{OA_VAXG_REAL,            "VAXG_REAL",            OA_ASCII_REAL,        FALSE},
{OA_UNKNOWN_DATA_TYPE,    "\"N/A\"",              OA_UNKNOWN_DATA_TYPE, FALSE}
};


/******************************************************************************
The next two global variables, oa_class_names and oa_class_types, are used in
routines OaObjectClasstoString and OaObjectClass.  They map between class
strings and oa_object_class_enum values.
******************************************************************************/
#define N_CLASS_NAMES 17

static char *oa_class_names[ N_CLASS_NAMES] = {
"ALIAS",
"ARRAY",
"BIT_COLUMN",
"BIT_ELEMENT",
"COLLECTION",
"COLUMN",
"CONTAINER",
"ELEMENT",
"GAZETTEER",
"HISTOGRAM",
"HISTORY",
"IMAGE", 
"PALETTE",
"QUBE",
"SERIES",
"SPECTRUM",
"TABLE"
};
static int oa_class_types[ N_CLASS_NAMES] = { 
OA_ALIAS, 
OA_ARRAY, 
OA_BIT_COLUMN, 
OA_BIT_ELEMENT, 
OA_COLLECTION, 
OA_COLUMN, 
OA_CONTAINER,
OA_ELEMENT, 
OA_GAZETTEER,
OA_HISTOGRAM, 
OA_HISTORY, 
OA_IMAGE, 
OA_PALETTE, 
OA_QUBE,
OA_SERIES, 
OA_SPECTRUM, 
OA_TABLE
};



/*****************************************************************************

  Routine:  OaCopyTree

  Description:  This routine makes a copy of the ODL tree, below and
                including the input node.  By default it copies all
                keywords, comment fields and SDT nodes, if any are attatched.
                The "options" argument can be used to strip out keywords,
                comments, and/or SDT nodes from the copy.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:  16 Apr  1997

  History:

    Creation - This routine was part of the Alpha Release of the OA library.
    10/06/94 - Added options argument.  SM
    04/16/97 - Added copying of dynamically allocated binrep structures.  SM

  Input:  
          input_node - An ODL tree node.

          options    - A flag indicating whether caller wants to strip off
                       keywords, comments and/or SDT nodes.  options contains
                       set bits for each of the desired stripping operations.

  Output:  The function returns a copy of the tree below and including
           input node.  If options had the OA_STRIP_COMMENTS bit set, then
           the nodes all have NULL for the various comment fields.  If options
           had the OA_STRIP_KEYWORDS bit set, then none of the nodes have
           any keywords.  If options had the OA_STRIP_SDT_NODES bit set, then
           none of the SDT nodes were copied.

  Notes:  

*****************************************************************************/

#ifdef _NO_PROTO

ODLTREE OaCopyTree( input_node, options)
ODLTREE input_node;
int     options;

#else

ODLTREE OaCopyTree( ODLTREE input_node, int options)

#endif
{

ODLTREE new_node, next_node;
KEYWORD *kwdptr;
SDT_node *old_sdt_node_ptr, *new_sdt_node_ptr;
struct binrep_desc *old_binrep_desc, *new_binrep_desc;

if (input_node == NULL) {
  oa_errno = 530;
  return( NULL);
}

/* Make a copy of the input node. Note: OdlCopyObjDesc doesn't copy keywords,
   and zeros out child_count.  It also doesn't copy references to the
   original tree, i.e. the node's parent, siblings and children. */

new_node = OdlCopyObjDesc( input_node);
new_node->child_count = input_node->child_count;

if ((options & OA_STRIP_KEYWORDS) == 0) {  /* Not set, so copy keywords. */
  kwdptr = OdlGetFirstKwd( input_node);
  while (kwdptr != NULL) {
    OdlPasteKwd( OdlCopyKwd( kwdptr), new_node);
    kwdptr = OdlGetNextKwd( kwdptr);
  }
}

if ((options & OA_STRIP_COMMENTS) != 0) {  /* Set, so strip comments. */

  /* Strip out all comments and file_name. */

  LemmeGo( new_node->pre_comment);
  LemmeGo( new_node->line_comment);
  LemmeGo( new_node->post_comment);
  LemmeGo( new_node->end_comment);
  LemmeGo( new_node->file_name);
}

if ((options & OA_STRIP_SDT_NODES) == 0) {  /* Not set, so copy SDT node. */

  if (input_node->appl1 != NULL) {
    old_sdt_node_ptr = (SDT_node *) input_node->appl1;
    new_sdt_node_ptr = OalNewSDTNode();
    *new_sdt_node_ptr = *old_sdt_node_ptr;
    new_node->appl1 = (char *) new_sdt_node_ptr;

    if (old_sdt_node_ptr->src.binrep_descrip != NULL) {
      if (old_sdt_node_ptr->src.binrep_descrip->is_dynamic == 1) {
        old_binrep_desc = old_sdt_node_ptr->src.binrep_descrip;
        new_binrep_desc = (struct binrep_desc *) OaMalloc( (long) sizeof( 
                                                      struct binrep_desc));
        new_sdt_node_ptr->src.binrep_descrip = new_binrep_desc;
        *new_binrep_desc = *old_binrep_desc;
        new_binrep_desc->q_code = (char *) OaMalloc( (long) 
                                           strlen( old_binrep_desc->q_code)+1);
        strcpy( new_binrep_desc->q_code, old_binrep_desc->q_code);
        new_binrep_desc->integer_order = (char *) OaMalloc(  
                                                  2 * sizeof( short));
        memcpy( new_binrep_desc->integer_order, old_binrep_desc->integer_order,
                2 * sizeof( short));
      }
    }

    if (old_sdt_node_ptr->dst.binrep_descrip != NULL) {
      if (old_sdt_node_ptr->dst.binrep_descrip->is_dynamic == 1) {
        old_binrep_desc = old_sdt_node_ptr->dst.binrep_descrip;
        new_binrep_desc = (struct binrep_desc *) OaMalloc( (long) sizeof( 
                                                      struct binrep_desc));
        new_sdt_node_ptr->dst.binrep_descrip = new_binrep_desc;
        *new_binrep_desc = *old_binrep_desc;
        new_binrep_desc->q_code = (char *) OaMalloc( (long) 
                                           strlen( old_binrep_desc->q_code)+1);
        strcpy( new_binrep_desc->q_code, old_binrep_desc->q_code);
        new_binrep_desc->integer_order = (char *) OaMalloc(  
                                                  2 * sizeof( short));
        memcpy( new_binrep_desc->integer_order, old_binrep_desc->integer_order,
                2 * sizeof( short));
      }
    }
  }
} else {
  new_node->appl1 = NULL;
}

/* Attach a copy of each child of input_node to new_node, after calling
   OaCopyTree recursively to copy the tree below the child.  */

next_node = LeftmostChild( input_node);
while (next_node != NULL) {
  OdlPasteObjDesc( OaCopyTree( next_node, options), new_node);
  next_node = RightSibling( next_node);
}
OdlAdjustObjDescLevel( new_node);
return( new_node);
}



/*****************************************************************************

  Routine:  OaDeleteKwd

  Description:  This function cuts the specified keyword out of the ODL tree
                node and frees it.

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  27 Nov  1995
  Last Modified:  27 Nov  1995

  History:

    Creation - This routine was part of the Version 1.0 Release of the
               OA library.

  Input:  
          kwd_name    - The keyword name; may contain wildcards, but the
                        routine will only delete the first occurrence.

          odltreenode - The node to delete the keyword from.

  Output: 
          If the keyword was found and deleted, the function returns 0.
          If no matching keyword was found, the function returns 1.
          This function does not set oa_errno.

  Notes:

*****************************************************************************/

#ifdef _NO_PROTO

int OaDeleteKwd( kwd_name, odltreenode)
char *kwd_name;
ODLTREE odltreenode;

#else

int OaDeleteKwd( char *kwd_name, ODLTREE odltreenode)

#endif
{

KEYWORD *kwdptr;

if ((odltreenode == NULL) || (kwd_name == NULL))
  return(1);

if ((kwdptr = OdlFindKwd( odltreenode, kwd_name, "*", (unsigned long) 0,
                          (unsigned short) ODL_THIS_OBJECT)) != NULL) {
  OdlCutKwd( kwdptr);
  OdlFreeKwd( kwdptr);
  return(0);
}
return(1);
}



/*****************************************************************************

  Routine:  OaDealiasKwdName

  Description:  This function updates an ODL tree node with the un-aliased
                version of a keyword name.  The input parameter can be either
                the un-aliased name or an aliased name:
                If the input parameter is an un-aliased name, the function
                gets all its aliases, and searches for the first occurrence of
                each one in the ODL tree node;  once it finds one, it changes
                the keyword's name to the un-aliased name.  
                If the input parameter is an alias, the function searches the
                ODL tree node for a keyword with that name, and if found,
                changes its name to the un-aliased name.

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  23 Jan  1995
  Last Modified:  23 Jan  1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:  
          kwd_name - The keyword name to dealias or to find an alias for.

  Output: 
          If a keyword de-aliased, the function returns 0.
          If no matching keyword is found, or if the input parameter isn't
          in Oa_alias_conversions, then the function returns 1.
          This function does not set oa_errno.

  Notes:
  1) Since there's usually only one occurrence of the same keyword in an ODL
     tree node, only the first occurrence is dealiased.

*****************************************************************************/

#ifdef _NO_PROTO

int OaDealiasKwdName( kwd_name, odltreenode)
char *kwd_name;
ODLTREE odltreenode;

#else

int OaDealiasKwdName( char *kwd_name, ODLTREE odltreenode)

#endif
{

int i, is_an_alias;
KEYWORD *kwdptr;

/* See if the input parameter is a de-aliased keyword name.  */

is_an_alias = TRUE;
for (i=0; i<OA_ALIAS_TRANSLATIONS; i++) {
  if (strcmp( kwd_name, Oa_alias_translations[i].translated_name) == 0) {

    is_an_alias = FALSE;

    /* Input parameter is an un-aliased name, so look for the current aliased
       value in the ODL tree node.  If find it, translate it.  */

    if ((kwdptr = OdlFindKwd( odltreenode, 
                              Oa_alias_translations[i].aliased_name, 
                              "*", (unsigned long) 0, 
                              (unsigned short) ODL_THIS_OBJECT)) != NULL) {
      LemmeGo( kwdptr->name);
      CopyString( kwdptr->name, Oa_alias_translations[i].translated_name);
      return(0);
    }
  }
}

/* If input parameter is an un-aliased name and we got this far, then there
   isn't a keyword with an aliased name in the odltreenode.  */

if (is_an_alias == FALSE) 
  return(1);

/* Input parameter is an alias, so find its alias translation, then find
   the aliased keyword in the ODL tree node, and change its name to the
   un-aliased name.  */

for (i=0; i<OA_ALIAS_TRANSLATIONS; i++) {
  if (strcmp( kwd_name, Oa_alias_translations[i].aliased_name) == 0)
    break;
}
if ((i < OA_ALIAS_TRANSLATIONS) &&
    ((kwdptr = OdlFindKwd( odltreenode, kwd_name, "*", (unsigned long) 0, 
                         (unsigned short) ODL_THIS_OBJECT)) != NULL)) {
  LemmeGo( kwdptr->name);
  CopyString( kwdptr->name, Oa_alias_translations[i].translated_name);
  return(0);
} else {
  return(1);
}
}



/*****************************************************************************

  Routine:  OaGetObjectClass

  Description:  This routine determines the enumerated type object_class_enum
                for an ODL tree node, from the class_name field in the node.
                The class_name field comes from the "OBJECT = <object class>"
                line in the label.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:   1 Sept  1994

  History:

    Creation - This routine was part of the Alpha Release of the OA library.

  Input:  
          odltreenode - An ODL tree node.

  Output:  If successful, the function returns an oa_object_class_enum value,
           otherwise OA_UNKNOWN_CLASS.
           This function does not set oa_errno.

  Notes:  

  1) The basic class name string may be contained within the actual class
     name, and may be mixed with other object class names, e.g. 
     "OBJECT = IMAGE_PREFIX_TABLE". 
     Algorithm:
     1) If any basic class name exactly matches the actual class string, found
        it.
     2) If any basic class name matches the last word in the actual class
        string, use the last word as the class.  If this fails, return
        OA_UNKNOWN_CLASS.

*****************************************************************************/

#ifdef _NO_PROTO

int OaGetObjectClass( odltreenode)
ODLTREE odltreenode;

#else

int OaGetObjectClass( ODLTREE odltreenode)

#endif
{

int i,str_len,class_type_len;
char class_string[80], *ptr;

ptr = OdlGetObjDescClassName( odltreenode);
if (ptr == NULL) return( OA_UNKNOWN_CLASS);

/* If any basic class name exactly matches the actual object class string, 
   return it.  */

strcpy( class_string, ptr);
StripLeading( class_string, ' ');
StripTrailing( class_string, ' ');
UpperCase( class_string);
for (i=0; i<N_CLASS_NAMES; i++) {
  if (strcmp( oa_class_names[i], class_string) == 0)
    return( oa_class_types[i]);
}

/* If any basic class name matches the last word in the actual object class
   string, return that class.  */

str_len = (int) strlen( class_string);
for (i=0; i<N_CLASS_NAMES; i++) {
  class_type_len = (int) strlen( oa_class_names[i]);
  if (str_len > class_type_len) {
    if (strncmp( oa_class_names[i], class_string + str_len - class_type_len,
                 class_type_len) == 0) {

      return( oa_class_types[i]);
    }
  }
}

return( OA_UNKNOWN_CLASS);
}



/*****************************************************************************

  Routine:  OaGetObjectInterchangeFormat

  Description:  This function gets the value of the INTERCHANGE_FORMAT keyword
                from the input node, and translates its value into either
                OA_ASCII_INTERCHANGE_FORMAT, OA_BINARY_INTERCHANGE_FORMAT or
                OA_UNKNOWN_INTERCHANGE_FORMAT.  If the INTERCHANGE_FORMAT
                keyword doesn't exist in the input node, it returns the
                default, OA_BINARY_INTERCHANGE_FORMAT.  If the object class
                is HISTORY, it returns OA_ASCII_INTERCHANGE_FORMAT.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   6 Dec   1994
  Last Modified:   6 Dec   1994

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:  
          TLO_object_node - An ODL tree node which is a top-level object
                            (IMAGE, TABLE etc, not a COLUMN or ELEMENT).
         
  Output:  The function returns OA_ASCII_INTERCHANGE_FORMAT, 
           OA_BINARY_INTERCHANGE_FORMAT or OA_UNKNOWN_INTERCHANGE_FORMAT.

  Notes:  

*****************************************************************************/

#ifdef _NO_PROTO

int OaGetObjectInterchangeFormat( TLO_object_node)
ODLTREE TLO_object_node;

#else

int OaGetObjectInterchangeFormat( ODLTREE TLO_object_node)

#endif
{

static char *proc_name = "OaGetObjectInterchangeFormat";
char *str = NULL;

if (OaGetObjectClass( TLO_object_node) == OA_HISTORY)
  return( OA_ASCII_INTERCHANGE_FORMAT);

OaKwdValuetoStr( "INTERCHANGE*FORMAT", TLO_object_node, &str);
if (str != NULL) {
  UpperCase( str);
  if      (strstr( str, "BINARY") != NULL)
    return( OA_BINARY_INTERCHANGE_FORMAT);
  else if (strstr( str, "ASCII") != NULL)
    return( OA_ASCII_INTERCHANGE_FORMAT);
  else {
    sprintf( error_string, "%s: invalid interchange format: %s.",
             proc_name, str);
    oa_errno = 531;
    OaReportError( error_string);
    return( OA_UNKNOWN_INTERCHANGE_FORMAT);
  }
} else {
  return( OA_BINARY_INTERCHANGE_FORMAT);
}
}



/*****************************************************************************

  Routine:  OaGetImageKeywords

  Description:  This function gets and translates the keyword values of useful
                IMAGE keywords.

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  27 Nov  1995
  Last Modified:  16 Mar  1998

  History:

    Creation - This routine was part of the Version 1.0 Release of the OA
               library.
    03/16/98 - Added OA_GIF encoding type.  SM

  Input:  
           image_node - The ODL tree node for a IMAGE.

  Output:  If successful, the function returns 0.  If the input node isn't of
           class IMAGE, or if any of the required keywords aren't present, 
           an error message is written, and the function returns 1;  the output
           parameters may not all be set.

           image_node        - An ODLTREE node of class IMAGE.

           lines             - Value of required keyword LINES.

           line_samples      - Value of required keyword LINE_SAMPLES.

           sample_bits       - Value of required keyword SAMPLE_BITS. 

           sample_type_str   - Required keyword;  the pointer returned points
                               directly to the keyword's value, so must not
                               be freed!

           bands             - If the BANDS keyword isn't found, then *bands
                               is set to 1.

           band_storage_type - OA_BAND_SEQUENTIAL, OA_LINE_INTERLEAVED or
                               OA_SAMPLE_INTERLEAVED

           line_prefix_bytes - If the LINE_PREFIX_BYTES keyword isn't found,
                               then *line_prefix_bytes is set to 0.

           line_suffix_bytes - If the LINE_SUFFIX_BYTES keyword isn't found,
                               then *line_suffix_bytes is set to 0.

           encoding_type     - OA_UNCOMPRESSED, OA_HUFFMAN_FIRST_DIFFERENCE,
                               OA_PREVIOUS_PIXEL, OA_CLEMENTINE_JPEG, OA_GIF,
                               or OA_UNKNOWN_ENCODING_TYPE

  Notes:

*****************************************************************************/

#ifdef _NO_PROTO

int OaGetImageKeywords( image_node, lines, line_samples, sample_bits,
                        sample_type_str, bands, band_storage_type,
                        line_prefix_bytes, line_suffix_bytes, encoding_type)
ODLTREE image_node;
long *lines;
long *line_samples;
long *sample_bits;
char **sample_type_str;
long *bands;
int  *band_storage_type;
long *line_prefix_bytes;
long *line_suffix_bytes;
int *encoding_type;

#else

int OaGetImageKeywords( ODLTREE image_node, long *lines, long *line_samples,
                        long *sample_bits, char **sample_type_str, long *bands,
                        int *band_storage_type, long *line_prefix_bytes, 
                        long *line_suffix_bytes, int *encoding_type)
#endif
{

static char *proc_name = "OaGetImageKeywords";
char *str, buf[64];

if (OaGetObjectClass( image_node) != OA_IMAGE) {
  sprintf( error_string, "%s: input ODL tree node must have class IMAGE.",
           proc_name);
  oa_errno = 530;
  OaReportError( error_string);
  return(1);
}

if (OaKwdValuetoLong( "LINES", image_node, lines) != 0) {
  sprintf( error_string, "%s: LINES keyword missing.", proc_name);
  oa_errno = 531;
  OaReportError( error_string);
  return(1);
}
if (OaKwdValuetoLong( "LINE_SAMPLES", image_node, line_samples) != 0) {
  sprintf( error_string, "%s: LINE_SAMPLES keyword missing.", proc_name);
  oa_errno = 531;
  OaReportError( error_string);
  return(1);
}
if (OaKwdValuetoLong( "SAMPLE_BITS", image_node, sample_bits) != 0) {
  sprintf( error_string,  "%s: SAMPLE_BITS keyword missing.", proc_name);
  oa_errno = 531;
  OaReportError( error_string);
  return(1);
}
if (OaKwdValuetoStr( "SAMPLE_TYPE", image_node, sample_type_str) != 0) {
  sprintf( error_string, "%s: SAMPLE_TYPE keyword missing.", proc_name);
  oa_errno = 531;
  OaReportError( error_string);
  return(1);
}
*bands = 1;
*band_storage_type = OA_UNKNOWN_BAND_STORAGE_TYPE;
*line_prefix_bytes = 0;
*line_suffix_bytes = 0;
OaKwdValuetoLong( "BANDS", image_node, bands);
OaKwdValuetoLong( "LINE*PREFIX*BYTES", image_node, line_prefix_bytes);
OaKwdValuetoLong( "LINE*SUFFIX*BYTES", image_node, line_suffix_bytes);

if (OaKwdValuetoStr( "ENCODING_TYPE", image_node, &str) != 0) {
  *encoding_type = OA_UNCOMPRESSED;
} else {
  strcpy( buf, str);
  UpperCase( buf);
  StripLeading( buf, '"');
  StripTrailing( buf, '"');

  if         (strcmp( buf, "N/A") == 0) {
    *encoding_type = OA_UNCOMPRESSED;
  } else if ((strstr( buf, "HFD") != NULL) || 
             (strstr( buf, "HUFFMAN") != NULL)) {
    *encoding_type = OA_HUFFMAN_FIRST_DIFFERENCE;
  } else if  (strstr( buf, "CLEM-JPEG") != NULL) {
    *encoding_type = OA_CLEMENTINE_JPEG;
  } else if  (strstr( buf, "PREVIOUS_PIXEL") != NULL) {
    *encoding_type = OA_PREVIOUS_PIXEL;
  } else if  (strcmp( buf, "GIF") == 0) {
    *encoding_type = OA_GIF;
  } else {
    sprintf( error_string, "%s: unknown ENCODING_TYPE: %s", proc_name, str);
    oa_errno = 531;
    OaReportError( error_string);
    *encoding_type = OA_UNKNOWN_ENCODING_TYPE;
  }
}

if (*bands > 1) {
  if (OaKwdValuetoStr( "BAND_STORAGE_TYPE", image_node, &str) == 0) {
    strcpy( buf, str);
    UpperCase( buf);
    StripLeading( buf, '"');
    StripTrailing( buf, '"');

    if         (strcmp( buf, "N/A") == 0) {
      *band_storage_type = OA_UNKNOWN_BAND_STORAGE_TYPE;
    } else if (strstr( buf, "BAND_SEQUENTIAL") != NULL) {
      *band_storage_type = OA_BAND_SEQUENTIAL;
    } else if  (strstr( buf, "LINE_INTERLEAVED") != NULL) {
      *band_storage_type = OA_LINE_INTERLEAVED;
    } else if  (strstr( buf, "SAMPLE_INTERLEAVED") != NULL) {
      *band_storage_type = OA_SAMPLE_INTERLEAVED;
    } else {
      sprintf( error_string, "%s: unknown BAND_STORAGE_TYPE: %s",
               proc_name, str);
      oa_errno = 531;
      OaReportError( error_string);
      *band_storage_type = OA_UNKNOWN_BAND_STORAGE_TYPE;
    }
  } else {
    sprintf( error_string, "%s: BAND_STORAGE_TYPE keyword missing!",
             proc_name);
    oa_errno = 531;
    OaReportError( error_string);
  }
}  /* end if bands > 1 */

return(0);
}



/*****************************************************************************

  Routine:  OaGetQubeKeywords

  Description:  OaGetQubeKeywords gets various keyword values needed by other
                OA routines from a QUBE node.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  27 Nov   1995

  History:

    Creation - This routine was part of the Version 1.0 Release of the OA
               library.
	01/12/03   Made a change for THEMIS qubes below  DWS
  Input:  
         input_node - A pointer to an ODL tree node of class QUBE.
       
  Output:  If successful, the routine returns 0, and sets all the output
           parameters appropriately;  otherwise it returns 1.  Each output
           argument is named the same as the keyword name whose value it
           stores.

           core_items      - Address of a pointer to a long.  On output, it
                             points to a 3-element array of longs allocated
                             inside the routine.  Caller should free the
                             array when finished with it.

           axis_names      - Points to a 3-element array of strings allocated
                             inside the routine.  Caller should free each
                             string, and the array itself when finished with
                             them.

           suffix_items    - Points to a 3-element array of longs allocated
                             inside the routine.  Caller should free the
                             array when finished with it.
  
           core_item_bytes - Address of a long.  On output the long contains
                             the value of the CORE_ITEM_BYTES keyword.

           core_item_type  - Points directly into the ODL tree at the keyword
                             value of CORE_ITEM_TYPE - should NOT be freed by
                             the caller.
  Notes:  

  1) The routine reports an error and returns if the AXIS keyword value isn't
     equal to 3, or if CORE_ITEMS, AXIS_NAME and SUFFIX_ITEMS aren't all
     3-item sequences.

*****************************************************************************/

#ifdef _NO_PROTO

int OaGetQubeKeywords( input_node, core_items, axis_names, suffix_items,
                       core_item_bytes, core_item_type)
ODLTREE input_node;
ODLTREE temp_node;    /*for THEMIS   DWS 09-10-02*/
long **core_items;
char ***axis_names;
long **suffix_items;
long *core_item_bytes;
char **core_item_type;

#else

int OaGetQubeKeywords( ODLTREE input_node, long **core_items, 
                       char ***axis_names, long **suffix_items,
                       long *core_item_bytes, char **core_item_type)
#endif
{

static char *proc_name = "OaGetQubeKeywords";
long axes;
int n;
KEYWORD *i_id_kwdptr; /*THEMIS DWS 09-10-02*/

if (OaGetObjectClass( input_node) != OA_QUBE) {
  sprintf( error_string, "%s: input ODLTREE node is not an QUBE object.",
           proc_name);
  oa_errno = 530;
  OaReportError( error_string);
  return(1);
}

if (OaKwdValuetoLong( "AXES", input_node, &axes) == 0) {
  if (axes != 3) {
    sprintf( error_string, "%s: AXES not equal to 3.", proc_name);
    oa_errno = 520;
    OaReportError( error_string);
    return(1);
  }
} else {
  sprintf( error_string, "%s: couldn't find AXES keyword.", proc_name);
  oa_errno = 531;
  OaReportError( error_string);
  return(1);
}

if (OaSequencetoLongArray( "CORE_ITEMS", input_node, core_items, &n) != 0) {
  sprintf( error_string, "%s: couldn't find CORE_ITEMS keyword.", proc_name);
  oa_errno = 531;
  OaReportError( error_string);
  return(1);
} else {
  if (n != 3) {
    sprintf( error_string, "%s: CORE_ITEMS doesn't have 3 elements.", 
             proc_name);
    oa_errno = 520;
    OaReportError( error_string);
    return(1);
  }
}

if (OaSequencetoStrArray( "AXIS_NAME", input_node, axis_names, &n) != 0) {
  sprintf( error_string, "%s: couldn't find AXIS_NAME keyword.", proc_name);
  oa_errno = 531;
  OaReportError( error_string);
  return(1);
} else {
  if (n != 3) {
    sprintf( error_string, "%s: AXIS_NAME array doesn't have 3 elements.", 
             proc_name);
    oa_errno = 520;
    OaReportError( error_string);
    return(1);
  }
}
/* A THEMIS EDR qube will not have a SUFFIX ITEMS keyword, but that is ok*/
/* A THEMIS RDR qube will have a SUFFIX ITEMS keyword                   */
if (OaSequencetoLongArray( "SUFFIX_ITEMS", input_node, suffix_items, &n) != 0)
{
	if ((i_id_kwdptr = OdlFindKwd(input_node, NULL, "THEMIS", 1, ODL_TO_END)) == NULL)
	{
		sprintf( error_string, "%s: couldn't find SUFFIX_ITEMS keyword.", proc_name);
		oa_errno = 531;
		OaReportError( error_string);
		return(1);
	}

} else	{
	if (n != 3) {
		sprintf( error_string, "%s: SUFFIX_ITEMS doesn't have 3 elements.", 
			proc_name);
		oa_errno = 520;
		OaReportError( error_string);
		return(1);
	}
}

if (OaKwdValuetoLong( "CORE_ITEM_BYTES", input_node, core_item_bytes) != 0) {
  sprintf( error_string, "%s: couldn't find CORE_ITEM_BYTES keyword.", 
           proc_name);
  oa_errno = 531;
  OaReportError( error_string);
  return(1);
}
if (OaKwdValuetoStr( "CORE_ITEM_TYPE", input_node, core_item_type) != 0) {
  sprintf( error_string, "%s: couldn't find CORE_ITEM_TYPE keyword.", 
           proc_name);
  oa_errno = 531;
  OaReportError( error_string);
  return(1);
}
return(0);
}



/*****************************************************************************

  Routine:  OaGetTableKeywords

  Description:  This function gets and translates the keyword values of useful
                TABLE keywords.

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  27 Nov  1995
  Last Modified:  27 Nov  1995

  History:

    Creation - This routine was part of the Version 1.0 Release of the OA
               library.

  Input:  
           table_node - The ODL tree node for a TABLE-like object.

  Output:  If successful, the function returns 0.  If the input node isn't of
           class TABLE, SPECTRUM, SERIES, PALETTE or GAZETTEER, or if any of
           the required keywords aren't present, an error message is written,
           and the function returns 1, and the output parameters may not all
           be set.

           table_node         - An ODLTREE node with a TABLE-like class.

           rows               - Value of required keyword ROWS.

           row_bytes          - Value of required keyword ROW_BYTES.

           row_prefix_bytes   - If the ROW_PREFIX_BYTES keyword isn't found,
                                then *row_prefix_bytes is set to 0.

           row_suffix_bytes   - If the ROW_SUFFIX_BYTES keyword isn't found,
                                then *row_suffix_bytes is set to 0.

           interchange_format - Set to OA_BINARY_INTERCHANGE_FORMAT or
                                OA_ASCII_INTERCHANGE_FORMAT.

           table_storage_type - Set to OA_ROW_MAJOR or OA_COLUMN_MAJOR.

  Notes:

*****************************************************************************/

#ifdef _NO_PROTO

int OaGetTableKeywords( table_node, rows, row_bytes, row_prefix_bytes,
                        row_suffix_bytes, interchange_format, 
                        table_storage_type)
ODLTREE table_node;
long *rows;
long *row_bytes;
long *row_prefix_bytes;
long *row_suffix_bytes;
int *interchange_format;
int *table_storage_type;

#else

int OaGetTableKeywords( ODLTREE table_node, long *rows, long *row_bytes,
                        long *row_prefix_bytes, long *row_suffix_bytes,
                        int *interchange_format, int *table_storage_type)

#endif
{

static char *proc_name = "OaGetTableKeywords";

switch( OaGetObjectClass( table_node)) {
  case OA_GAZETTEER:
  case OA_PALETTE:
  case OA_SERIES:
  case OA_SPECTRUM:
  case OA_TABLE:
  case OA_ARRAY:   /*collection addition DWS 12/04/04*/
  ;
  break;
  default:
    sprintf( error_string, 
             "%s: input ODL tree node must have TABLE-like class", proc_name);
    oa_errno = 530;
    OaReportError( error_string);
    return(1);
}

if (OaKwdValuetoLong( "ROWS", table_node, rows) != 0) {
  sprintf( error_string, "%s: ROWS keyword missing.", proc_name);
  oa_errno = 531;
  OaReportError( error_string);
  return(1);
}
if (OaKwdValuetoLong( "ROW_BYTES", table_node, row_bytes) != 0) {
  sprintf( error_string, "%s: ROW_BYTES keyword missing.", proc_name);
  oa_errno = 531;
  OaReportError( error_string);
  return(1);
}
*row_prefix_bytes = 0;
*row_suffix_bytes = 0;
OaKwdValuetoLong( "ROW*PREFIX*BYTES", table_node, row_prefix_bytes);
OaKwdValuetoLong( "ROW*SUFFIX*BYTES", table_node, row_suffix_bytes);

*interchange_format = OaGetObjectInterchangeFormat( table_node);
if (*interchange_format == OA_UNKNOWN_INTERCHANGE_FORMAT)
  return(1);
*table_storage_type = OaGetTableStorageType( table_node);
if (*table_storage_type == OA_UNKNOWN_TABLE_STORAGE_TYPE)
  return(1);
return(0);
}



/*****************************************************************************

  Routine:  OaGetTableStorageType

  Description:  This function gets the TABLE_STORAGE_TYPE of a TABLE, and
                translates it into an enumeration type value.

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  15 Dec  1994
  Last Modified:  15 Dec  1994

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:  
          table_node - The ODL tree node for a TABLE.

  Output:  The function returns OA_ROW_MAJOR, OA_COLUMN_MAJOR or
           OA_UNKNOWN_TABLE_STORAGE_TYPE.  The default is OA_ROW_MAJOR if
           the TABLE_STORAGE_TYPE keyword wasn't found.  If the input node 
           isn't a TABLE-type node, an error message is issued, and
           OA_UNKNOWN_TABLE_STORAGE_TYPE is returned.

  Notes:

*****************************************************************************/

#ifdef _NO_PROTO

int OaGetTableStorageType( table_node)
ODLTREE table_node;

#else

int OaGetTableStorageType( ODLTREE table_node)

#endif
{

static char *proc_name = "OaGetTableStorageType";
char *str;

switch( OaGetObjectClass( table_node)) {
  case OA_GAZETTEER:
  case OA_PALETTE:
  case OA_SERIES:
  case OA_SPECTRUM:
  case OA_TABLE:
  ;
  break;
  default:
    sprintf( error_string, 
             "%s: input ODL tree node must have TABLE-like class", proc_name);
    oa_errno = 530;
    OaReportError( error_string);
    return( OA_UNKNOWN_TABLE_STORAGE_TYPE);
}

if (OaKwdValuetoStr( "TABLE_STORAGE_TYPE", table_node, &str) != 0)
  return( OA_ROW_MAJOR);

UpperCase( str);
StripLeading( str, '"');
StripLeading( str, '\'');
StripLeading( str, ' ');
StripTrailing( str, '"');
StripTrailing( str, '\'');
StripTrailing( str, ' ');

if        (strcmp( str, "ROW_MAJOR") == 0) {
  return( OA_ROW_MAJOR);
} else if (strcmp( str, "COLUMN_MAJOR") == 0) {
  return( OA_COLUMN_MAJOR);
} else {
  sprintf( error_string, "%s: unknown TABLE_STORAGE_TYPE: %s", proc_name,
                          str);
  oa_errno = 531;
  OaReportError( error_string);
  return( OA_UNKNOWN_TABLE_STORAGE_TYPE);
}
}



/*****************************************************************************

  Routine:  OaKwdValuetoDouble

  Description:  This routine finds the specified keyword, and converts its 
                ASCII value to a double.

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  18 July  1996
  Last Modified:  18 July  1996

  History:

    Creation - This routine was part of Release 1.1 of the OA library.

  Input:  
          kwd_name    - A string in uppercase; the name of a keyword.

          odltreenode - An ODL tree node.

          value       - A pointer to a double.

  Output:  If successful, the function returns 0 and returns the keyword value,
           converted to a double, in *value;  if unable to convert the
           keyword value, it returns 1 and reports an error; if the kwd_name
           isn't found in odltreenode, it also returns 1, but doesn't report
           any error.

  Notes:  

*****************************************************************************/

#ifdef _NO_PROTO

int OaKwdValuetoDouble( kwd_name, odltreenode, value)
char *kwd_name;
ODLTREE odltreenode;
double *value;

#else

int OaKwdValuetoDouble( char *kwd_name, ODLTREE odltreenode, double *value)

#endif
{

KEYWORD *kwdptr;
unsigned short kwd_value_type;
char *proc_name = "OaKwdValuetoLong";

if ((kwdptr = OdlFindKwd( odltreenode, kwd_name, "*", (unsigned long) 0, 
                          (unsigned short) ODL_THIS_OBJECT)) == NULL)
  return(1);

kwd_value_type = OdlGetKwdValueType( kwdptr);
if ((kwd_value_type == ODL_INTEGER) || (kwd_value_type == ODL_REAL)) {
  if (sscanf( OdlGetKwdValue( kwdptr), "%lf", value) == 1) {
     return(0);
  } else {
    sprintf( error_string, 
             "%s: sscanf failed to convert %s keyword value to double",
             proc_name, kwd_name);
  }
} else {
  sprintf( error_string, "%s: %s keyword value is not a real type.", 
           proc_name, kwd_name);
}
oa_errno = 600;
OaReportError( error_string);
return(1);
}



/*****************************************************************************

  Routine:  OaKwdValuetoLong

  Description:  This routine finds the specified keyword, and converts its 
                ASCII value to a long integer.

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:   1 Sept  1994

  History:

    Creation - This routine was part of the Alpha Release of the OA library.

  Input:  
          kwd_name    - A string in uppercase; the name of a keyword.

          odltreenode - An ODL tree node.

          value       - A pointer to a long integer.

  Output:  If successful, the function returns 0 and returns the keyword value,
           converted to a long, in *value;  if unable to convert the
           keyword value, it returns 1 and reports an error; if the kwd_name
           isn't found in odltreenode, it also returns 1, but doesn't report
           any error.

  Notes:  

*****************************************************************************/

#ifdef _NO_PROTO

int OaKwdValuetoLong( kwd_name, odltreenode, value)
char *kwd_name;
ODLTREE odltreenode;
long *value;

#else

int OaKwdValuetoLong( char *kwd_name, ODLTREE odltreenode, long *value)

#endif
{

KEYWORD *kwdptr;
unsigned short kwd_value_type;
char *proc_name = "OaKwdValuetoLong";

if ((kwdptr = OdlFindKwd( odltreenode, kwd_name, "*", (unsigned long) 0, 
                          (unsigned short) ODL_THIS_OBJECT)) == NULL)
  return(1);

kwd_value_type = OdlGetKwdValueType( kwdptr);
if (kwd_value_type == ODL_INTEGER) {
  if (sscanf( OdlGetKwdValue( kwdptr), "%ld", value) == 1) {
     return(0);
  } else {
    sprintf( error_string, 
             "%s: sscanf failed to convert %s keyword value to long",
             proc_name, kwd_name);
  }
} else {
  sprintf( error_string, "%s: %s keyword value is not an integer.", 
           proc_name, kwd_name);
}
oa_errno = 600;
OaReportError( error_string);
return(1);
}



/*****************************************************************************

  Routine:  OaKwdValuetoStr

  Description:  This function finds the specified keyword, and sets *value to
                point to the keyword's value string.  Note that it does NOT
                make a copy of the string, so the user should not modify it
                or free it;  it points directly into the ODL keyword 
                structure. 

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:   1 Sept  1994

  History:

    Creation - This routine was part of the Alpha Release of the OA library.

  Input:  
          kwd_name    - A string in uppercase; the name of a keyword.

          odltreenode - An ODL tree node.

          value       - The address of a char pointer.

  Output:  If successful, the function returns 0 and sets *value to point into
           the tree at the keyword value.  If the kwd_name isn't found in
           odltreenode, it also returns 1, but doesn't report any error.

  Notes:  

*****************************************************************************/

#ifdef _NO_PROTO

int OaKwdValuetoStr( kwd_name, odltreenode, value)
char *kwd_name;
ODLTREE odltreenode;
char **value;

#else

int OaKwdValuetoStr( char *kwd_name, ODLTREE odltreenode, char **value)

#endif
{
KEYWORD *kwdptr;
/*char *proc_name = "OaKwdValuetoStr";*/

if ((kwdptr = OdlFindKwd( odltreenode, kwd_name, "*", (unsigned long) 0, 
                          (unsigned short) ODL_THIS_OBJECT)) == NULL)
  return(1);

*value = OdlGetKwdValue( kwdptr);
return(0);
}



/*****************************************************************************

  Routine:  OaLongtoKwdValue 

  Description:  This routine converts a long integer into a string, and
                puts it in the odltreenode as a keyword value.  If there is
                already a keyword name with kwd_name, it replaces its value 
                with the new value.  If there is no keyword name of kwd_name, 
                it creates a new keyword structure and pastes it as the last
                keyword in odltreenode.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:   1 Sept  1994

  History:

    Creation - This routine was part of the Alpha Release of the OA library.

  Input:  
          kwd_name    - A string in uppercase; the name of a keyword.

          odltreenode - A pointer to an ODL tree node structure.

          kwd_value   - A long, signed integer value.

  Output:  If successful, the function returns 0 and adds the keyword as
           described above;  otherwise it returns 1.

  Notes:  

*****************************************************************************/

#ifdef _NO_PROTO

int OaLongtoKwdValue( kwd_name, odltreenode, kwd_value)
char *kwd_name;
ODLTREE odltreenode;
long kwd_value;

#else

int OaLongtoKwdValue( char *kwd_name, ODLTREE odltreenode, long kwd_value)

#endif
{

static char *proc_name = "OaLongtoKwdValue";
static char buf[40];
char *ptr = &(buf[0]);
KEYWORD *kwdptr;

buf[0] = '\0';

if ((int) sprintf( buf, "%ld", kwd_value) < 0) {
  sprintf( error_string, "%s: sprintf failed to convert long kwd_value to",
           proc_name);
  strcat( error_string, " a string\n");
  sprintf( error_string + strlen( error_string), "keyword name = %s\n",
           kwd_name);
  kwdptr = OdlFindKwd( odltreenode, "NAME", "*", (unsigned long) 0, 
                       (unsigned short) ODL_THIS_OBJECT);
  if (kwdptr != NULL)
    sprintf( error_string + strlen( error_string), "object name = %s\n",
             kwdptr->value);

  oa_errno = 600;
  OaReportError( error_string);
  return(1);
}

if ((kwdptr = OdlFindKwd( odltreenode, kwd_name, "*", (unsigned long) 0, 
              (unsigned short) ODL_THIS_OBJECT)) != NULL) {
  LemmeGo( kwdptr->value);
  CopyString( kwdptr->value, ptr);
  kwdptr->size = (unsigned long) strlen( kwdptr->value);
} else {
  kwdptr = OdlNewKwd( kwd_name, ptr, NULL, NULL, NULL, (long) 0);
  OdlPasteKwd( kwdptr, odltreenode);
}
return(0);
}



/*****************************************************************************

  Routine:  OaObjectClasstoStr

  Description:  This function takes an oa_object_class_enum value and 
                returns the corresponding string.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:   1 Sept  1994

  History:

    Creation - This routine was part of the Alpha Release of the OA library.

  Input:  
          oa_object_class - An oa_object_class_enum value.

  Output:  If successful, the function returns the object class as a string;
           otherwise it returns the string "OA_UNKNOWN_CLASS".
           The pointer returned points directly into a static array, and
           should be copied before certain uses.
          
  Notes:  

*****************************************************************************/

#ifdef _NO_PROTO

char *OaObjectClasstoStr( oa_object_class)
int oa_object_class;

#else

char *OaObjectClasstoStr( int oa_object_class)

#endif
{

int i;

for (i=0; i<N_CLASS_NAMES; i++)
  if (oa_class_types[i] == oa_object_class) return( oa_class_names[i]);
return( "OA_UNKNOWN_CLASS");
}



/*****************************************************************************

  Routine:  OaPDSDataTypetoStr

  Description:  This routine returns a data type string, given an
                oa_PDS_data_types_enum value.

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:   1 Sept  1994

  History:

    Creation - This routine was part of the Alpha Release of the OA library.

  Input:  
         PDS_data_type - An oa_PDS_data_types_enum value.

  Output:  If successful, the routine returns a pointer to a data type string 
           in the OaPDS_data_type_info array defined above, otherwise 
           "UNK".
           The pointer returned points to static data, so it should be copied
           before certain uses, and never freed.

  Notes:  

*****************************************************************************/

#ifdef _NO_PROTO

char *OaPDSDataTypetoStr( PDS_data_type)
int PDS_data_type;

#else

char *OaPDSDataTypetoStr( int PDS_data_type)

#endif
{

int i;
for (i=0; i<N_PDS_DATA_TYPE_INFO; i++) {
  if (( OaPDS_data_type_info[i].PDS_data_type == PDS_data_type) &&
      ( OaPDS_data_type_info[i].is_an_alias == FALSE))
    return( OaPDS_data_type_info[i].data_type_str);
}
return( "UNK");
}



/*****************************************************************************

  Routine:  OaSequencetoLongArray

  Description:  This function finds the specified keyword, whose value must be
                a sequence, and converts its ASCII sequence of values to long
                integers and stores them in an array.  

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:   1 Sept  1994

  History:

    Creation - This routine was part of the Alpha Release of the OA library.

  Input:  
          kwd_name    - A string in uppercase; the name of a keyword.

          odltreenode - An ODL tree node.


  Output:  If successful, the function returns 0 and sets the output parameters
           array and sequence_items appropriately.  If unable to convert any
           of the sequence values, it returns 1 and reports an error; if the
           kwd_name isn't found in odltreenode, it also returns 1, but doesn't
           report any error.

           array          - An array of long integers.

           sequence_items - The number of items in array.

  Notes:  The array must be freed by the caller.

*****************************************************************************/

#ifdef _NO_PROTO

int OaSequencetoLongArray( kwd_name, odltreenode, array, 
                           sequence_items)
char *kwd_name;
ODLTREE odltreenode;
long **array;
int *sequence_items;

#else

int OaSequencetoLongArray( char *kwd_name, ODLTREE odltreenode, 
                           long **array, int *sequence_items)

#endif
{

static char *proc_name = "OaSequencetoLongArray";
char **str_arr;
long *arr;
int result, items, i;

if ((result = OaSequencetoStrArray( kwd_name, odltreenode, &str_arr, 
                                    &items)) != 0)
  return( result);

if ((arr = (long *) OaMalloc( sizeof( long) * items)) == NULL) {
  sprintf( error_string, "%s: OaMalloc failed! Out of memory!", proc_name);
  OaReportError( error_string);
  exit(1);
  /*NOTREACHED*/
}
for (i=0; i<items; i++) {
  if (sscanf( str_arr[i], "%ld", &(arr[i])) != 1) {
    sprintf( error_string, "%s: error converting sequence value to long.",
             proc_name);
    oa_errno = 600;
    OaReportError( error_string);
    LemmeGo( arr);
    result = 1;
    break;
  }
}
for (i=0; i<items; i++) {
  LemmeGo( str_arr[i]);
}
LemmeGo( str_arr);
if (result == 0) {
  *sequence_items = items;
  *array = arr;
} else {
  *sequence_items = 0;
  *array = NULL;
}
return( result);
}



/*****************************************************************************

  Routine:  OaSequencetoStrArray

  Description:  This function finds the specified keyword, whose value should
                be a sequence, allocates memory for and copies each sequence
                item.
                It returns an array containing pointers to the sequence item
                strings.

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  20 Dec   1994
  Last Modified:  19 Mar   1996

  History:

    Creation - This routine was part of the Beta Release of the OA library.
    03/19/96 - Fixed to handle a scalar too.  SM

  Input:  
          kwd_name    - A string in uppercase; the name of a keyword.

          odltreenode - An ODL tree node.


  Output:  If successful, the function returns 0 and sets the output parameters
           array and sequence_items appropriately.  If the kwd_name isn't
           found in odltreenode, it returns 1, but doesn't report any error.
           If the keyword is there, but an error occurred parsing the
           sequence, then 0 is returned in *sequence_items.

           array          - This pointer, whose address is passed in, now
                            points to an array of strings.  The function
                            allocates memory for this array, and for each 
                            string in the array;  the array and every string
                            in the array must be freed by the caller.

           sequence_items - The number of items in array.

  Notes: 

*****************************************************************************/

#ifdef _NO_PROTO

int OaSequencetoStrArray( kwd_name, odltreenode, array_ptr, 
                          sequence_items)
char *kwd_name;
ODLTREE odltreenode;
char ***array_ptr;
int *sequence_items;

#else

int OaSequencetoStrArray( char *kwd_name, ODLTREE odltreenode, 
                          char ***array_ptr, int *sequence_items)

#endif
{

static char *proc_name = "OaSequencetoStrArray";
KEYWORD *kwdptr;
char *value_start, *value_end, *str, c;
char **array;
int i;

*sequence_items = 0;
if ((kwdptr = OdlFindKwd( odltreenode, kwd_name, "*", (unsigned long) 0, 
                          (unsigned short) ODL_THIS_OBJECT)) == NULL)
  return(1);

str = OdlGetKwdValue( kwdptr);

/* Find the number of sequence values by counting the number of commas
   separating them.  */

for (i=0; i < (int) strlen( str); i++)
  if (str[i] == ',') 
    (*sequence_items)++;
(*sequence_items)++;

/* Allocate space for the array of pointers. */

if ((*array_ptr = (char **) OaMalloc( (*sequence_items) * sizeof( char *))) 
                                    == NULL) {
  sprintf( error_string, "%s: OaMalloc failed! Out of memory!", proc_name);
  oa_errno = 720;
  OaReportError( error_string);
  exit(1);
  /*NOTREACHED*/
}
array = *array_ptr;

value_start = OdlValueStart( str);
if (value_start == str) {        /* No sequence delimitor, so assume scalar */
  if ((int) strlen( value_start) > 0) {
    CopyString( str, value_start);
    array[0] = str;    
  } else {
    array[0] = NULL;
  }
  i = 1;
} else {
  i = 0;
  while (value_start != NULL) {
    if (*value_start == '\0')
      break;
    value_end = OdlValueEnd( value_start);
    c = *(value_end+1);
    *(value_end+1) = '\0';

    if ((int) strlen( value_start) > 0) {
      CopyString( str, value_start);
      array[i] = str;    
    } else {
      array[i] = NULL;
    }
    *(value_end+1) = c;
    i++;
    value_start = OdlValueStart( value_end+1);
  }
}
if (i != *sequence_items) {
  sprintf( error_string, "%s: error parsing %s sequence", proc_name, 
           kwd_name);
  oa_errno = 600;
  OaReportError( error_string);
  *sequence_items = 0;
  return(1);
}
return(0);
}



/*****************************************************************************

  Routine:  OaStrtoKwdValue 

  Description:  This routine copies str, and puts it in the odltreenode as a 
                keyword value.  If there is already a keyword name with
                kwd_name, it replaces its value with the new value;  if there 
                is no keyword name of kwd_name, it creates a new keyword
                structure, and pastes it as the last keyword in odltreenode.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:   1 Sept  1994

  History:

    Creation - This routine was part of the Alpha Release of the OA library.

  Input:  
          kwd_name    - A string in uppercase; the name of a keyword.

          odltreenode - An ODL tree node.

          str         - A NULL-terminated string.

  Output:  If successful, the function returns 0 and adds the keyword as
           described above;  otherwise it returns 1.

  Notes:  The function makes a copy of str before assigning it to the
          keyword value.

*****************************************************************************/

#ifdef _NO_PROTO

int OaStrtoKwdValue( kwd_name, odltreenode, str)
char *kwd_name;
ODLTREE odltreenode;
char *str;

#else

int OaStrtoKwdValue( char *kwd_name, ODLTREE odltreenode, char *str)

#endif
{

KEYWORD *kwdptr;

if ((kwdptr = OdlFindKwd( odltreenode, kwd_name, "*", (unsigned long) 0, 
              (unsigned short) ODL_THIS_OBJECT)) != NULL) {
  LemmeGo( kwdptr->value);
  if (str != NULL) {
    kwdptr->size = (unsigned long) strlen( str);
    CopyString( kwdptr->value, str);
  } else {
    kwdptr->size = 0;
  }
} else {
  kwdptr = OdlNewKwd( kwd_name, str, NULL, NULL, NULL, (long) 0);
  OdlPasteKwd( kwdptr, odltreenode);
}
return(0);
}



/*****************************************************************************

  Routine:  OaStrtoPDSDataType

  Description:  This function returns an oa_PDS_data_types_enum value, given a
                data type keyword value from a PDS label, such as
                "DATA_TYPE = IEEE_REAL".
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:   1 Sept  1994

  History:

    Creation - This routine was part of the Alpha Release of the OA library.

  Input:  
          str                - A data type keyword value, e.g. "IEEE_REAL".

          interchange_format - Specifies whether the object the data type
                               keyword was found in is ASCII or binary.  This
                               is necessary because of wide-spread use of
                               the same DATA_TYPE keyword when describing 
                               ASCII and binary data - often binary DATA_TYPE
                               values such as MSB_INTEGER, REAL are used
                               instead of ASCII_INTEGER, ASCII_REAL.
         
  Output:  An oa_PDS_data_types_enum value is returned;  if the data type was
           not determined, OA_UNKNOWN_DATA_TYPE is returned.

  Notes:  
  1) Examples:
     When given "MSB_INTEGER" and OA_BINARY_INTERCHANGE_FORMAT, the function
     returns OA_MSB_INTEGER.
     When given "MSB_INTEGER" and OA_ASCII_INTERCHANGE_FORMAT, the function
     returns OA_ASCII_INTEGER.
*****************************************************************************/

#ifdef _NO_PROTO

int OaStrtoPDSDataType( str, interchange_format)
char *str;
int interchange_format;

#else

int OaStrtoPDSDataType( char *str, int interchange_format)

#endif 
{

int i;
StripLeading( str, '"');
StripTrailing( str, '"');
for (i=0; i<N_PDS_DATA_TYPE_INFO; i++) {
  if (strcmp( OaPDS_data_type_info[i].data_type_str, str) == 0)
    if (interchange_format == OA_BINARY_INTERCHANGE_FORMAT)
      return( OaPDS_data_type_info[i].PDS_data_type);
    else
      return( OaPDS_data_type_info[i].ASCII_data_type);
}
return( OA_UNKNOWN_DATA_TYPE);
}


























