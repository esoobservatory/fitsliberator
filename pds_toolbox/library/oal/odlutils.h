/*****************************************************************************

  File:  odlutils.h

  Description:  These are macros and utility function declarations for 
                manipulating ODL trees.  The code for these functions is in
                odlutils.c.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:  11 Dec   1996

  History:

    Creation - This routine was part of the Alpha Release of the OA library.
    11/27/95 - Added prototypes for OaGetImageKeywords, OaGetQubeKeywords and
               OaDeleteKwd.  SM
    12/11/96 - Added arguments to OaGetImageKeywords prototype.  SM

  Notes:  

*****************************************************************************/

#ifndef OA_ODLUTILS_INCLUDED
#define OA_ODLUTILS_INCLUDED 1

#include "lablib3.h"

/* Typedef ODLTREE to be a pointer to L3's ODL tree node structure. */
typedef OBJDESC *ODLTREE;

/* Define macros for navigating in ODL trees.                               */
#define LeftSibling(odltreenode)    (odltreenode->left_sibling)
#define RightSibling(odltreenode)   (odltreenode->right_sibling)
#define LeftmostChild(odltreenode)  (odltreenode->first_child)
#define RightmostChild(odltreenode) (odltreenode->last_child)
#define Parent(odltreenode)         (odltreenode->parent)

/* Define option bits for OaCopyTree. */
#define OA_STRIP_KEYWORDS   1
#define OA_STRIP_COMMENTS   2
#define OA_STRIP_SDT_NODES  4


#ifdef _NO_PROTO

ODLTREE OaCopyTree();
int     OaDealiasKwdName();
int     OaDeleteKwd();
int     OaGetObjectClass();
int     OaGetObjectInterchangeFormat();
int     OaGetImageKeywords();
int     OaGetQubeKeywords();
int     OaGetTableKeywords();
int     OaGetTableStorageType();
int     OaKwdValuetoDouble();
int     OaKwdValuetoLong();
int     OaKwdValuetoStr();
int     OaLongtoKwdValue();
char *  OaObjectClasstoStr();
char *  OaPDSDataTypetoStr();
int     OaSequencetoLongArray();
int     OaSequencetoStrArray();
int     OaStrtoKwdValue();
int     OaStrtoPDSDataType();

#else

ODLTREE OaCopyTree( ODLTREE input_node, int options);
int     OaDealiasKwdName( char *kwd_name, ODLTREE odltreenode);
int     OaDeleteKwd( char *kwd_name, ODLTREE odltreenode);
int     OaGetObjectClass( ODLTREE odltreenode);
int     OaGetObjectInterchangeFormat( ODLTREE TLO_object_node);
int     OaGetImageKeywords( ODLTREE image_node, long *lines,
                            long *line_samples, long *sample_bits, 
                            char **sample_type_str, long *bands,
                            int *band_storage_type, long *line_prefix_bytes,
                            long *line_suffix_bytes, int *encoding_type);
int     OaGetQubeKeywords( ODLTREE input_node,
                           long **core_items, 
                           char ***axis_names,
                           long **suffix_items,
                           long *core_item_bytes,
                           char **core_item_type);
int     OaGetTableKeywords( ODLTREE table_node,
                            long *rows,
                            long *row_bytes,
                            long *row_prefix_bytes,
                            long *row_suffix_bytes,
                            int *interchange_format,
                            int *table_storage_type);
int     OaGetTableStorageType( ODLTREE table_node);
int     OaKwdValuetoDouble( char *kwd_name, ODLTREE odltreenode, 
                            double *value);
int     OaKwdValuetoLong( char *kwd_name, ODLTREE odltreenode, long *value);
int     OaKwdValuetoStr( char *kwd_name, ODLTREE odltreenode, char **value);
int     OaLongtoKwdValue(char *kwd_name, ODLTREE odltreenode, long kwd_value);
char *  OaObjectClasstoStr( int oa_object_class);
char *  OaPDSDataTypetoStr( int PDS_data_type);
int     OaSequencetoLongArray( char *kwd_name, ODLTREE odltreenode, 
                               long **array, int *sequence_items);
int     OaSequencetoStrArray( char *kwd_name, ODLTREE odltreenode,
                              char ***array_ptr, int *sequence_items);
int     OaStrtoKwdValue( char *kwd_name, ODLTREE odltreenode, char *str);
int     OaStrtoPDSDataType( char *str, int interchange_format);

#endif

#endif  /* #ifndef OA_ODLUTILS_INCLUDED */
