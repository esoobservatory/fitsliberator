/*****************************************************************************

  File:  OAL_IDL_interface.c

  Description: This file contains the C routine OaIDLCall, which makes up the
               C part of the IDL interface for the PDS Object Access Library
               and L3.  This routine, the OA library, and L3 are compiled into
               a shareable, dynamically-linked library.  OaIDLCall is then
               called by IDL wrapper routines in OAL_IDL_interface.pro,
               using IDL's CALL_EXTERNAL function.

  Author:  Steve Monk, University of Colorado LASP

  Creation Date:   7  Feb  1996
  Last Modified:   7  Feb  1996

  History:

    Creation - This routine was part of the Release 1.1 of the OA library.

*****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "oal.h"

typedef struct {
  unsigned short length;
  short reserved;
  char *s;
  } STRING;

#if defined(IBM_PC) || defined(MAC)
typedef long int4;
#else
typedef int int4;
#endif


/*****************************************************************************

  Routine: OaIDLCall

  Description:  This routine makes most OAL and L3 functions, as well as C
                functions specific to this interface, callable from IDL.
                It is called via a CALL_EXTERNAL call within the IDL function
                'OaIDLCall' in OAL_IDL_interface.pro.

  Author:  Steve Monk, University of Colorado LASP

  Creation Date:   7  Feb  1996
  Last Modified:   7  Feb  1996

  History:

    Creation - This routine was part of the Release 1.1 of the OA library.

  Input and Output:

    This routine adheres to the UNIX/Macintosh/PC calling sequence of a
    routine called by CALL_EXTERNAL, passing arguments in argv[] and the
    number of arguments in argc.  Modifications for VAX/VMS and Alpha/VMS
    are in progress.

    - The first argument, argv[0], is a pointer to an IDL STRING structure
      whose "s" field points to a C string containing the name of the OAL, L3
      or interface function being called, e.g. "OaParseLabelFile".

    - Following the first argument are the IDL versions of arguments normally
      used to call the corresponding C function.

    - For C functions which return a (char *), OaIDLCall returns the same
      (char *), which CALL_EXTERNAL converts to an IDL string variable when it
      returns.  WARNING!!! THERE IS A MEMORY LEAK HERE, BECAUSE THE C STRING
      ISN'T FREED.  POSSIBLE FIX IS TO ADD ANOTHER PARAMETER CONTAINING THE
      C STRING POINTER IN A DOUBLE VARIABLE, AND HAVE THE WRAPPER CALL OA_FREE
      WITH IT.  (WATCH OUT AND DON'T FREE THE STATIC RETURN STRING "OK"!!!)

    - For functions which do not return strings, an additional argument (the
      last argument) is passed, which is a pointer to the IDL variable in
      which to pass back the result;  the code copies the C function's return
      value there.

  Notes:

  1) The calling sequence from the IDL wrapper function (e.g. the IDL function
     OaReadObject) to the C function (e.g. the C function OaReadObject) is
     diagrammed below:

                       IDL wrapper function
                                |
                		V
                            OaIDLCall
                		|
                		V
                          CALL_EXTERNAL
            IDL code    	|
            ---------------------------------------
            C code    		|
                                V
                            OaIDLCall
                		|
                		V
                          OAL/L3 function

  2) Pointers:
     The code passes pointers between OAL and IDL as doubles;  IDL stores
     OA_OBJECT and ODLTREE pointers in IDL 'double' variables.  On most
     platforms with 4-byte pointers, only the first 4 bytes of the 8-byte
     double are used.  On the Dec Alpha, which has 8-byte pointers, all 8
     bytes of the double are used.  A NULL pointer is represented by 0.0.
     These pointers are inscrutable within IDL;  they can only be stored,
     perhaps compared, and become useful only when passed back to C functions.

  3) General Review of IDL Parameter Passing:
     This interface uses CALL_EXTERNAL to call C routines;  C routines cannot 
     make calls back to IDL.  The following are the types in argv[] when using
     CALL_EXTERNAL's default parameter passing mechanism:
     - An IDL integer gets passed as a pointer to a 2-byte integer (short *).
     - An IDL long is passed as a pointer to a 4-byte integer.  On most
       platforms, this is a (int *), but on the IBM-PC it's a (long *).
     - An IDL double is passed as a pointer to a double (double *).  A double
       is 8 bytes on all platforms.  A double is the only primitive data type
       which can be passed to and from IDL which is big enough to hold a
       pointer on all platforms.
     - An IDL string is passed as a pointer to the STRING structure (STRING *).
     - An IDL array is passed as a pointer to the first element in the array.
       For an array of strings, this means a pointer to a STRING structure gets
       passed.  There's no way of knowing in the C code how long an array is
       unless an additional parameter is passed giving the length of the array;
       this parameter is added by the code in OAL_IDL_interface.pro whenever
       necessary.

  4) OAL-Specific Parameter Passing:
     - Passing integer types:
       The IDL wrappers cast all integer types into IDL long's before
       passing them to OaIDLCall and CALL_EXTERNAL.  Thus the C code is
       simplified by only having to deal with 4-byte integers across the
       interface.

     - Return values:
       The CALL_EXTERNAL call is made from the IDL function OaIDLCall with
       the /S_VALUE keyword argument;  thus the C function OaIDLCall always
       returns a (char *), which CALL_EXTERNAL converts to an IDL string and 
       returns to the IDL function OaIDLCall, and OaIDLCall returns the string
       to the wrapper function.  The wrapper function decides whether to return
       this string to the caller, or to toss it out and return the last
       result argument.   If the wrapper function is supposed to return some
       other type, then the wrapper initialized the last input argument to
       the OaIDLCall with an IDL variable of the expected return type, and the
       C function OaIDLCall copied the return value there.  
       Sometimes a C function which is supposed to set a string and return an
       integer is implemented by returning the string and passing the integer
       as an additional argument. 
       Sometimes a C function is supposed to set multiple strings;  this is
       implemented by passing an additional argument after the first argument
       which tells which string to return on this call, and multiple calls are
       made to OalIDLCall to get all the strings.

  5) Memory:
     IDL uses a custom memory management scheme which is incompatible with
     the OAL/L3 scheme (malloc and free by default), and IDL's memory
     routines are inaccessible to OAL (at least in this interface which uses
     CALL_EXTERNAL to be portable to all IDL platforms).
     Thus the code must copy object data into IDL variables whose memory was
     allocated by IDL, rather than using the same memory space.  As in a
     C program, the user is responsible for freeing OAL objects once they've
     been copied into IDL variables.

*****************************************************************************/

#ifdef MAC
#pragma export on
#endif

#if defined( VAX) || defined( ALPHA_VMS)

/* VAX/VMS and Alpha/VMS don't use the argv, argc calling convention. */

char *OaIDLCall( void *arg_count, void *p0, void *p1, void *p2, void *p3,
                                  void *p4, void *p5, void *p6, void *p7,
                                  void *p8, void *p9)

#else

#ifdef _NO_PROTO

char *OaIDLCall( argc, argv)
int argc;
void *argv[];

#else

char *OaIDLCall( int argc, void *argv[])

#endif
#endif
{
char *proc_name, *str, *str1, *str2, *str3, *str4, *str5, *str6;
char **ptr_array, return_string[128], *ptr, *ptr_result;
static char *static_str, **static_str_arr;
ODLTREE odltree, odltree1, odltree2, odltree3;
OA_OBJECT oa_object, oa_object1, oa_object2;
KEYWORD *kwd1, *kwd2;
PTR data_ptr;
unsigned short ushort1, ushort2;
int int1, int2, int3, i;
static int static_int;
int4 my_int4, *int4_ptr;
long long1, long2, long3, long4, long5, long6, size;
static long *static_long_arr;
unsigned long ulong1, ulong2, ulong3;
FILE *fd, *fd1, *fd2;
static FILE *debug_fd = NULL;

/* HISTOGRAM attributes */
long items, item_bytes;
int data_type, interchange_format;

/* IMAGE attributes */
long lines, line_samples, sample_bits, bands, prefix_bytes, suffix_bytes;
int encoding_type, band_storage_type;
char *sample_type_str;

/* TABLE attributes */
long rows, row_bytes;
int table_storage_type;

/* ARRAY attributes */
long *axis_items, axes;
int sequence_items;

#if defined( VAX) || defined( ALPHA_VMS)

/* VAX/VMS and Alpha/VMS don't use the same argv, argc calling convention as
   all the other platforms, so take care of this now so don't have to worry
   about it later.  */

void *argv[10];
int argc;

/* Copy arguments into argv. */

argc = (int) *((int4 *) arg_count);
argv[0] = p0;  /* arg_count is always at least 1. */
if (argc > 1)  argv[1] = p1;
if (argc > 2)  argv[2] = p2;
if (argc > 3)  argv[3] = p3;
if (argc > 4)  argv[4] = p4;
if (argc > 5)  argv[5] = p5;
if (argc > 6)  argv[6] = p6;
if (argc > 7)  argv[7] = p7;
if (argc > 8)  argv[8] = p8;
if (argc > 9)  argv[9] = p9;

#endif

/*if (debug_fd == NULL) debug_fd = fopen( "tmp.tmp", "w");*/

/* Dereference the C string from inside the STRING structure in the first
   argument, then switch on it.  This selection is blocked into 4 parts:
   1) Interface functions, which start with "OaIDL".
   2) OAL functions, which start with "Oa".
   3) L3 function, which start with "Odl".
   4) Other functions, e.g. implementations of macros like "LeftmostChild".
*/

proc_name = ((STRING *) argv[0])->s;


if (strncmp( proc_name, "OaIDL", 5) == 0) {

  /***************************************************************************
                             Interface Functions
  ***************************************************************************/

  if        (strcmp( proc_name, "OaIDLSize") == 0) {

    /* The returned array differs from an IDL 'size' array in two ways:
       1) The type code is the last element;  the total number of elements is
          omitted.
       2) An extra dimension is added to string arrays as the first (fastest
          changing) dimension, giving the string length; this is always a
          constant for OA_OBJECTS.  */
   
    oa_object = *((OA_OBJECT *) argv[1]);
    int4_ptr = (int4 *) argv[2];

    int4_ptr[0] = 0;  /* Initialize for unknown object type */
    int4_ptr[1] = 0;
    int4_ptr[2] = 1;

    if (oa_object->is_in_memory == FALSE)
      return( "OaIDLSize: Object must be in-memory!");
    odltree = oa_object->odltree;
    if ((interchange_format = OaGetObjectInterchangeFormat( odltree)) ==
         OA_UNKNOWN_INTERCHANGE_FORMAT)
      return( "OaIDLSize: Unknown interchange_format");

    switch (OaGetObjectClass( odltree)) {

      case OA_HISTOGRAM:
        if ((OaKwdValuetoStr( "DATA_TYPE", odltree, &str) != 0) ||
            (OaKwdValuetoLong( "ITEM_BYTES", odltree, &item_bytes) != 0) ||
            (OaKwdValuetoLong( "ITEMS", odltree, &items) != 0))
          return( "OaIDLSize: Missing/bad keyword");
        int4_ptr[0] = 1;                  /* One-dimensional array */
        int4_ptr[1] = items;              /* Number of array elements */
        if ((interchange_format == OA_BINARY_INTERCHANGE_FORMAT) &&
            (strstr( str, "ASCII") == NULL)) {
          if (strstr( str, "REAL") != NULL) {
            switch (item_bytes) {
              case 4:  int4_ptr[2] = 4;  break;  /* float */
              case 8:  int4_ptr[2] = 5;  break;  /* double */
              default: int4_ptr[2] = 0;  break;  /* Unknown float type */
            }
          } else {
            switch (item_bytes) {
              case 1:  int4_ptr[2] = 1;  break;  /* byte */
              case 2:  int4_ptr[2] = 2;  break;  /* short int (2 bytes) */
              case 4:  int4_ptr[2] = 3;  break;  /* long int (4 bytes)  */
              default: int4_ptr[2] = 0;  break;  /* Unknown integer type */
            }
          }
        } else {        /* Strings */
          int4_ptr[0] = 2;
          int4_ptr[1] = item_bytes;
          int4_ptr[2] = items;
          int4_ptr[3] = 7;  
        }
      break;  /* end case HISTOGRAM */

      case OA_IMAGE:
        if (OaGetImageKeywords( odltree, &lines, &line_samples, &sample_bits,
                                &sample_type_str, &bands, &band_storage_type,
                                &prefix_bytes, &suffix_bytes,
                                &encoding_type) != 0)
          return( "OaIDLSize: Missing/bad keyword in IMAGE node");
        if (oa_object->size != (lines * line_samples * sample_bits/8))
          return( "OaIDLSize: oa_object->size inconsistent with keywords");

        int4_ptr[0] = 2;
        int4_ptr[1] = line_samples;
        int4_ptr[2] = lines;
        int4_ptr[4] = lines * line_samples;
        if (strstr( sample_type_str, "REAL") != NULL) {
          switch (sample_bits/8) {
            case 4:  int4_ptr[3] = 4;  break;  /* float */
            case 8:  int4_ptr[3] = 5;  break;  /* double */
            default: int4_ptr[3] = 0;  break;  /* Unknown float type */
          }
        } else {
          switch (sample_bits/8) {
            case 1:  int4_ptr[3] = 1;  break;  /* byte */
            case 2:  int4_ptr[3] = 2;  break;  /* short int (2 bytes) */
            case 4:  int4_ptr[3] = 3;  break;  /* long int (4 bytes)  */
            default: int4_ptr[3] = 0;  break;  /* Unknown integer type */
  	  }
        }
      break;  /* end case IMAGE */

      case OA_TABLE:
      case OA_SPECTRUM:
      case OA_SERIES:
      case OA_PALETTE:
      case OA_GAZETTEER:

        if ((OdlGetObjDescChildCount( odltree) > 1) ||
            (OaGetObjectClass( LeftmostChild( odltree)) != OA_COLUMN)) {
          int4_ptr[0] = 1;  /* return SIZ array for an IDL structure */
          int4_ptr[1] = 1;
          int4_ptr[2] = 8;
          int4_ptr[3] = 1;
          return( "OK");

        } else {            /* TABLE has a single COLUMN object. */
          if (OaGetTableKeywords( odltree, &rows, &row_bytes, &prefix_bytes,
                                  &suffix_bytes, &interchange_format,
                                  &table_storage_type) != 0)
            return( "OaIDLSize: Missing/bad keyword");
          odltree1 = LeftmostChild( odltree);
          OaKwdValuetoLong( "BYTES", odltree1, &item_bytes);
          items = 1;
          OaKwdValuetoLong( "ITEMS", odltree1, &items);
          if (items > 1)
            OaKwdValuetoLong( "ITEM_BYTES", odltree1, &item_bytes);
          OaKwdValuetoStr( "DATA_TYPE", odltree1, &str);

          if ((strstr( str, "ASCII") == str) ||
              (strcmp( str, "CHARACTER") == 0)) {  /* ASCII data */
            if (items == 1) {
              int4_ptr[0] = 2;
              int4_ptr[1] = item_bytes;
              int4_ptr[2] = rows;
              int4_ptr[3] = 7;                     /* Type code for a string */
            } else {
              int4_ptr[0] = 3;
              int4_ptr[1] = item_bytes;
              int4_ptr[2] = items;
              int4_ptr[3] = rows;
              int4_ptr[4] = 7;                     /* Type code for a string */
            }

          } else {  /* Binary, numeric data */

            if (items > 1) {
              int4_ptr[0] = 2;  /* Two-dimensional array. */
              int4_ptr[1] = items;
              int4_ptr[2] = rows;
            } else {
              int4_ptr[0] = 1;  /* One-dimensional array. */
              int4_ptr[1] = rows * items;
            }
            i = int4_ptr[0] + 1;

            if (strstr( str, "REAL") != NULL) {
              switch (item_bytes) {
                case 4:  int4_ptr[i] = 4;  break;  /* float */
                case 8:  int4_ptr[i] = 5;  break;  /* double */
                default: int4_ptr[i] = 0;  break;  /* Unknown float type */
              }
            } else {
              switch (item_bytes) {
                case 1:  int4_ptr[i] = 1;  break;  /* byte */
                case 2:  int4_ptr[i] = 2;  break;  /* short int (2 bytes) */
                case 4:  int4_ptr[i] = 3;  break;  /* long int (4 bytes)  */
                default: int4_ptr[i] = 0;  break;  /* Unknown integer type */
              }
            }
          }  /* end else DATA_TYPE is numeric */
        } /* end else TABLE has single COLUMN object */
      break;  /* end case TABLE-like object */

      case OA_COLLECTION:
        int4_ptr[0] = 1;  /* return SIZ array for an IDL structure */
        int4_ptr[1] = 1;
        int4_ptr[2] = 8;
        int4_ptr[3] = 1;
        return( "OK");
      break;  /* end case COLLECTION */

      case OA_ARRAY:
        if (OdlFindObjDesc( odltree, "*COLLECTION", NULL, NULL, 0L,
                            (unsigned short) ODL_RECURSIVE_DOWN) != NULL) {
          int4_ptr[0] = 1;  /* ARRAY has a COLLECTION somewhere under it,  */
          int4_ptr[1] = 1;  /* so return SIZ array for an IDL structure.   */
          int4_ptr[2] = 8;
          int4_ptr[3] = 1;
          return( "OK");

        } else {            /* ARRAY is 'simple': no embedded COLLECTIONS. */
          if (OdlFindObjDesc( odltree, "*BIT_ELEMENT", NULL, NULL, 0L,
                            (unsigned short) ODL_RECURSIVE_DOWN) != NULL)
            return( "Cannot handle BIT_ELEMENT");

          /* Find the ELEMENT object below the ARRAY; there may be more ARRAY
             objects between the ELEMENT and the root ARRAY, so loop up through
             them, starting at the parent of the ELEMENT, adding each ARRAY's
             dimensions to the SIZ array until get back to the root ARRAY.  */

          odltree1 = OdlFindObjDesc( odltree, "*ELEMENT", NULL, NULL, 0L,
                                    (unsigned short) ODL_RECURSIVE_DOWN);
          if (odltree1 == NULL)
            return( "OaIDLSize: Cannot find ELEMENT under ARRAY");
          odltree2 = Parent( odltree1);
          while (odltree2 != NULL) {
            if ((OaGetObjectClass( odltree2) != OA_ARRAY) ||
                (OaKwdValuetoLong( "AXES", odltree2, &axes) != 0) ||
                (OaSequencetoLongArray( "AXIS_ITEMS", odltree2, &axis_items,
                                        &sequence_items) != 0)) {
              int4_ptr[0] = 0;  /* Return unknown object type */
              int4_ptr[1] = 0;
              int4_ptr[2] = 1;
              return(  
           "OaIDLSize: Bad object class under ARRAY or bad/missing keywords");
            }
            if (axes != sequence_items) {
              int4_ptr[0] = 0;  /* Return unknown object type */
              int4_ptr[1] = 0;
              int4_ptr[2] = 1;
              return( "AXES inconsistent with AXIS_ITEMS");
            }
            for (i=0; i<axes; i++)
              int4_ptr[ int4_ptr[0] + 1 + i] = axis_items[i];
            int4_ptr[0] += axes;
            odltree2 = Parent( odltree2);
          }  /* end while loop through ARRAY nodes above ELEMENT node */

          /* Now find the data type of the ELEMENT node. */

          OaKwdValuetoLong( "BYTES", odltree1, &item_bytes);
          OaKwdValuetoStr( "DATA_TYPE", odltree1, &str);

          i = int4_ptr[0] + 1;
          if ((strstr( str, "ASCII") == str) ||
              (strcmp( str, "CHARACTER") == 0) ||
              (strcmp( str, "DATE") == 0) ||
              (strcmp( str, "TIME") == 0) ||
              (interchange_format == OA_ASCII_INTERCHANGE_FORMAT)) {
            int4_ptr[i] = 7;                       /* Type code for a string */
            for (i=int4_ptr[0]+1; i>0; i--)        /* Shift array right by 1 */
              int4_ptr[i+1] = int4_ptr[i];
            int4_ptr[1] = item_bytes;              /* Insert new dimension   */
            (int4_ptr[0])++;                       /* Update # of dimensions */

          } else {  /* Binary, numeric data */
            if (strstr( str, "REAL") != NULL) {
              switch (item_bytes) {
                case 4:  int4_ptr[i] = 4;  break;  /* float */
                case 8:  int4_ptr[i] = 5;  break;  /* double */
                default: int4_ptr[i] = 0;  break;  /* Unknown float type */
              }
            } else {
              switch (item_bytes) {
                case 1:  int4_ptr[i] = 1;  break;  /* byte */
                case 2:  int4_ptr[i] = 2;  break;  /* short int (2 bytes) */
                case 4:  int4_ptr[i] = 3;  break;  /* long int (4 bytes)  */
                default: int4_ptr[i] = 0;  break;  /* Unknown integer type */
              }
            }
          }  /* end else DATA_TYPE is numeric */
        }  /* end else have a 'simple' ARRAY */
      break;  /* end case ARRAY */

      default:
        return( "OaIDLSize: Unknown object type");
      break;
    }  /* end switch through all the object classes */
    return( "OK");

  } else if (strcmp( proc_name, "OaIDLCopyObjectDatatoIDLVariable") == 0) {
    oa_object = *((OA_OBJECT *) argv[1]);
    memcpy( argv[2], oa_object->data_ptr, oa_object->size);
    return( "OK");

  } else if (strcmp( proc_name, "OaIDLGetOaObjectStruct") == 0) {
    oa_object = *((OA_OBJECT *) argv[1]);
    memcpy( argv[2], &(oa_object->odltree), sizeof( char *));
    memcpy( argv[3], &(oa_object->data_ptr), sizeof( char *));
    my_int4 = (int4) oa_object->size;
    memcpy( argv[4], &my_int4, 4);
    return( "OK");

  } else if (strcmp( proc_name, "OaIDLGetODLTreeNodeStruct") == 0) {
    odltree = *((ODLTREE *) argv[1]);
    if        (strcmp( ((STRING *) argv[2])->s, "class_name") == 0) {
      return( odltree->class_name);
    } else if (strcmp( ((STRING *) argv[2])->s, "pre_comment") == 0) {
      return( odltree->pre_comment);
    } else if (strcmp( ((STRING *) argv[2])->s, "line_comment") == 0) {
      return( odltree->line_comment);
    } else if (strcmp( ((STRING *) argv[2])->s, "post_comment") == 0) {
      return( odltree->post_comment);
    } else if (strcmp( ((STRING *) argv[2])->s, "end_comment") == 0) {
      return( odltree->end_comment);
    } else if (strcmp( ((STRING *) argv[2])->s, "file_name") == 0) {
      return( odltree->file_name);
    } else {
      my_int4 = (int4) odltree->child_count;
      memcpy( argv[3], &my_int4, 4);
      memcpy( argv[4], &(odltree->parent), sizeof( char *));
      memcpy( argv[5], &(odltree->left_sibling), sizeof( char *));
      memcpy( argv[6], &(odltree->right_sibling), sizeof( char *));
      memcpy( argv[7], &(odltree->first_child), sizeof( char *));
      memcpy( argv[8], &(odltree->last_child), sizeof( char *));
      memcpy( argv[9], &(odltree->first_keyword), sizeof( char *));
      memcpy( argv[10], &(odltree->last_keyword), sizeof( char *));
      return( "OK");
    }

  } else if (strcmp( proc_name, "OaIDLGetProfile") == 0) {
    int4_ptr = (int4 *) argv[1];                /* dst_format_for_ASCII_src */
    *int4_ptr = (int4) Oa_profile.dst_format_for_ASCII_src;
    int4_ptr = (int4 *) argv[2];                /* dst_format_for_binary_src */
    *int4_ptr = (int4) Oa_profile.dst_format_for_binary_src;
    int4_ptr = (int4 *) argv[3];                /* dst_alignment_type */
    *int4_ptr = (int4) Oa_profile.dst_alignment_type;
    int4_ptr = (int4 *) argv[4];                /* data_translation_profile */
    *int4_ptr = (int4) Oa_profile.data_translation_profile;
    int4_ptr = (int4 *) argv[5];                /* check_ASCII_writes */
    *int4_ptr = (int4) Oa_profile.check_ASCII_writes;
    return( "OK");

  } else if (strcmp( proc_name, "OaIDLSetProfile") == 0) {
    my_int4 = *((int4 *) argv[1]);              /* dst_format_for_ASCII_src */
    Oa_profile.dst_format_for_ASCII_src = (char) my_int4;
    my_int4 = *((int4 *) argv[2]);              /* dst_format_for_binary_src */
    Oa_profile.dst_format_for_binary_src = (char) my_int4;
    my_int4 = *((int4 *) argv[3]);              /* dst_alignment_type */
    Oa_profile.dst_alignment_type = (char) my_int4;
    my_int4 = *((int4 *) argv[4]);              /* data_translation_profile */
    Oa_profile.data_translation_profile = (char) my_int4;
    my_int4 = *((int4 *) argv[5]);              /* check_ASCII_writes */
    Oa_profile.check_ASCII_writes = (char) my_int4;
    return( "OK");
  }
} else if (strncmp( proc_name, "Oa", 2) == 0) {

  /***************************************************************************
                             OAL Functions
  ***************************************************************************/

  if        (strcmp( proc_name, "OaParseLabelFile") == 0) {
    str1 = ((STRING *) argv[1])->s;                  /* filespec */
    str2 = ((STRING *) argv[2])->s;                  /* errfilespec */
    ushort1 = (unsigned short) *((int4 *) argv[3]);  /* expand */
    ushort2 = (unsigned short) *((int4 *) argv[4]);  /* nomsgs */
    ptr_result = (char *) argv[5];                   /* return value */
    ptr = (char *) OaParseLabelFile( str1, str2, ushort1, ushort2);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OaGetObjectClass") == 0) {
    odltree = *((ODLTREE *) argv[1]);           /* odltreenode */
    int4_ptr = (int4 *) argv[2];                /* return value */
    *int4_ptr = (int4) OaGetObjectClass( odltree);
    return( "OK");

  } else if (strcmp( proc_name, "OaGetSubCollection") == 0) {
    oa_object = *((OA_OBJECT *) argv[1]);        /* oa_object */
    odltree = *((ODLTREE *) argv[2]);            /* sub_object_node */
    ptr_result = (char *) argv[3];               /* return value */
    ptr = (char *) OaGetSubCollection( oa_object, odltree);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OaGetSubTable") == 0) {
    oa_object = *((OA_OBJECT *) argv[1]);        /* table_object */
    long1 = (long) *((int4 *) argv[2]);          /* start_row */
    long2 = (long) *((int4 *) argv[3]);          /* stop_row */
    ptr = (char *) argv[4];                      /* subobject_nodes */
    int1 = (int) *((int4 *) argv[5]);            /* n_subobject_nodes */
    ptr_result = (char *) argv[6];               /* return value */
    ptr_array = (char **) OaMalloc( (long) (int1 * sizeof( char *)));
    if (ptr_array == NULL) {
      memset( ptr_result, 0, sizeof( char *));
      return( "OaMalloc failed!");
    }
    for (i=0; i<int1; i++,ptr+=8)
      memcpy( &(ptr_array[i]), ptr, sizeof( char *));
    ptr = (char *) OaGetSubTable( oa_object, long1, long2,
                                  (ODLTREE *) ptr_array, int1);
    memcpy( ptr_result, &ptr, sizeof( char *));
    OaFree( (char *) ptr_array);
    return( "OK");

  } else if (strcmp( proc_name, "OaReadSubTable") == 0) {
    odltree = *((ODLTREE *) argv[1]);            /* table_node */
    long1 = (long) *((int4 *) argv[2]);          /* start_row */
    long2 = (long) *((int4 *) argv[3]);          /* stop_row */
    ptr = (char *) argv[4];                      /* subobject_nodes */
    int1 = (int) *((int4 *) argv[5]);            /* n_subobject_nodes */
    ptr_result = (char *) argv[6];               /* return value */
    ptr_array = (char **) OaMalloc( (long) (int1 * sizeof( char *)));
    if (ptr_array == NULL) {
      memset( ptr_result, 0, sizeof( char *));
      return( "OaMalloc failed!");
    }
    for (i=0; i<int1; i++,ptr+=8)
      memcpy( &(ptr_array[i]), ptr, sizeof( char *));
    ptr = (char *) OaReadSubTable( odltree, long1, long2,
                                  (ODLTREE *) ptr_array, int1);
    memcpy( ptr_result, &ptr, sizeof( char *));
    OaFree( (char *) ptr_array);
    return( "OK");

  } else if (strcmp( proc_name, "OaKwdValuetoLong") == 0) {
    str = ((STRING *) argv[1])->s;            /* kwd_name */
    odltree = *((ODLTREE *) argv[2]);         /* odltreenode */
    int4_ptr = (int4 *) argv[4];              /* return value */
    *int4_ptr = (int4) OaKwdValuetoLong( str, odltree, &long1);
    int4_ptr = (int4 *) argv[3];              /* value */
    *int4_ptr = (int4) long1;
    return( "OK");

  } else if (strcmp( proc_name, "OaKwdValuetoStr") == 0) {
    str1 = ((STRING *) argv[1])->s;          /* kwd_name */
    odltree = *((ODLTREE *) argv[2]);        /* odltreenode */
    int4_ptr = (int4 *) argv[3];             /* return value, watch out! */
    *int4_ptr = (int4) OaKwdValuetoStr( str1, odltree, &str2);
    return( str2);

  } else if (strcmp( proc_name, "OaReadObject") == 0) {
    odltree1 = *((ODLTREE *) argv[1]);        /* object_node */
    ptr_result = (char *) argv[2];            /* return value */
    ptr = (char *) OaReadObject( odltree1);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OaRouteErrorMessages") == 0) {
    str = ((STRING *) argv[1])->s;            /* message_fname */
    fd = *((FILE **) argv[2]);                /* message_fptr */
    int4_ptr = (int4 *) argv[3];              /* return value */
    *int4_ptr = (int4) OaRouteErrorMessages( str, fd);
    return( "OK");

  } else if (strcmp( proc_name, "OaObjectClasstoStr") == 0) {
    int1 = (int) *((int4 *) argv[1]);            /* oa_object_class */
    str = OaObjectClasstoStr( int1);
    return( str);                                /* return value */

  } else if (strcmp( proc_name, "OaUnravelContainer") == 0) {
    oa_object = *((OA_OBJECT *) argv[1]);        /* table_object */
    ptr_result = (char *) argv[2];               /* return value */
    ptr = (char *) OaUnravelContainer( oa_object);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OaAddContainerAroundTable") == 0) {
    oa_object = *((OA_OBJECT *) argv[1]);        /* table_object */
    ptr_result = (char *) argv[2];               /* return value */
    ptr = (char *) OaAddContainerAroundTable( oa_object);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OaAddLineTerminatorstoTable") == 0) {
    oa_object = *((OA_OBJECT *) argv[1]);        /* table_object */
    ptr_result = (char *) argv[2];               /* return value */
    ptr = (char *) OaAddLineTerminatorstoTable( oa_object );
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OaCloseImage") == 0) {
    oa_object = *((OA_OBJECT *) argv[1]);        /* image_handle_object */
    my_int4 = (int4) OaCloseImage( oa_object);
    memcpy( argv[2], &my_int4, 4);               /* return value */
    return( "OK");

  } else if (strcmp( proc_name, "OaCloseOutputFile") == 0) {
    oa_object = *((OA_OBJECT *) argv[1]);        /* file_object */
    str1 = ((STRING *) argv[2])->s;              /* label_filespec */
    my_int4 = (int4) OaCloseOutputFile( oa_object, str1);
    memcpy( argv[3], &my_int4, 4);               /* return value */
    return( "OK");

  } else if (strcmp( proc_name, "OaConvertImagetoArray") == 0) {
    oa_object = *((OA_OBJECT *) argv[1]);        /* simple_image */
    ptr_result = (char *) argv[2];               /* return value */
    ptr = (char *) OaConvertImagetoArray( oa_object);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OaConvertObject") == 0) {
    oa_object = *((OA_OBJECT *) argv[1]);        /* object */
    ptr_result = (char *) argv[2];               /* return value */
    ptr = (char *) OaConvertObject( oa_object);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OaConvertObjecttoOneType") == 0) {
    oa_object = *((OA_OBJECT *) argv[1]);        /* object */
    str1 = ((STRING *) argv[2])->s;              /* data_type */
    int1 = (int) *((int4 *) argv[3]);            /* bytes */
    int2 = (int) *((int4 *) argv[4]);            /* rescale */
    ptr_result = (char *) argv[5];               /* return value */
    ptr = (char *) OaConvertObjecttoOneType( oa_object, str1, int1, int2);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OaCopyObject") == 0) {
    oa_object = *((OA_OBJECT *) argv[1]);        /* object */
    ptr_result = (char *) argv[2];               /* return value */
    ptr = (char *) OaCopyObject( oa_object);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OaCreateAttachedLabel") == 0) {
    odltree = *((ODLTREE *) argv[1]);           /* odltree */
    str1 = ((STRING *) argv[2])->s;             /* label_filespec */
    str2 = ((STRING *) argv[3])->s;             /* attached_label_filespec */
    my_int4 = (int4) OaCreateAttachedLabel( odltree, str1, str2);
    memcpy( argv[4], &my_int4, 4);
    return( "OK");

  } else if (strcmp( proc_name, "OaDeleteColumn") == 0) {
    oa_object = *((OA_OBJECT *) argv[1]);        /* oa_object */
    odltree = *((ODLTREE *) argv[2]);            /* input_node */
    ptr_result = (char *) argv[3];               /* return value */
    ptr = (char *) OaDeleteColumn( oa_object, odltree);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OaDeleteObject") == 0) {
    oa_object = *((OA_OBJECT *) argv[1]);        /* object */
    my_int4 = (int4) OaDeleteObject( oa_object);
    memcpy( argv[2], &my_int4, 4);               /* return value */
    return( "OK");

  } else if (strcmp( proc_name, "OaDeleteRow") == 0) {
    oa_object = *((OA_OBJECT *) argv[1]);        /* table_object */
    long1 = (long) *((int4 *) argv[2]);          /* row */
    ptr_result = (char *) argv[3];               /* return value */
    ptr = (char *) OaDeleteRow( oa_object, long1);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OaGetFileKeywords") == 0) {

    /* This call has to return two strings and some integers, so the wrapper
       calls it twice: the first call passes "label_filename" in argv[1] and
       returns everything except data_filename.  The second call passes
       "data_filename" in argv[1] and returns data_filename, which has been
       stored in a static variable from the first call; the actual C function
       OaGetFileKeywords() is only called once.  */

    if (strcmp( ((STRING *) argv[1])->s, "label_filename") == 0) {
      odltree = *((ODLTREE *) argv[2]);
      my_int4 = (int4) OaGetFileKeywords( odltree, &str1, &static_str, &int1,
                                          &long1, &long2, &long3, &int2);
      memcpy( argv[8], &my_int4, 4);             /* return value */
      my_int4 = (int4) int1;
      memcpy( argv[3], &my_int4, 4);             /* record_type */
      my_int4 = (int4) long1;
      memcpy( argv[4], &my_int4, 4);             /* record_bytes */
      my_int4 = (int4) long2;
      memcpy( argv[5], &my_int4, 4);             /* file_records */
      my_int4 = (int4) long3;
      memcpy( argv[6], &my_int4, 4);             /* file_offset */
      my_int4 = (int4) int2;
      memcpy( argv[7], &my_int4, 4);             /* object_interchange_format*/
      return( str1);                             /* label_filename */
    } else {
      return( static_str);                       /* data_filename */
    }

  } else if (strcmp( proc_name, "OaGetPartialImage") == 0) {
    oa_object = *((OA_OBJECT *) argv[1]);        /* simple_image */
    long1 = (long) *((int4 *) argv[2]);          /* start_line */
    long2 = (long) *((int4 *) argv[3]);          /* stop_line */
    long3 = (long) *((int4 *) argv[4]);          /* start_sample */
    long4 = (long) *((int4 *) argv[5]);          /* stop_sample */
    ptr_result = (char *) argv[6];               /* return value */
    ptr = (char *) OaGetPartialImage( oa_object, long1, long2, long3, long4);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OaImportColumn") == 0) {
    long1 = (long) *((int4 *) argv[2]);         /* rows */
    long2 = (long) *((int4 *) argv[3]);         /* items */
    long3 = (long) *((int4 *) argv[4]);         /* item_bytes */
    str1 = ((STRING *) argv[5])->s;             /* data_type */
    int1 = (int) *((int4 *) argv[6]);           /* interchange_format */
    str2 = ((STRING *) argv[7])->s;             /* name */
    ptr_result = (char *) argv[8];              /* return value */
    size = long1 * long2 * long3;
    data_ptr = (PTR) OaMalloc( size);
    if (data_ptr == NULL) {
      memset( ptr_result, 0, sizeof( char *));
      return( "OaMalloc failed!");
    }
    memcpy( data_ptr, argv[1], size);
    ptr = (char *) OaImportColumn( data_ptr, long1, long2, long3, str1, int1,
                                   str2);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OaImportHistogram") == 0) {
    long1 = (long) *((int4 *) argv[2]);         /* items */
    long2 = (long) *((int4 *) argv[3]);         /* item_bytes */
    str1 = ((STRING *) argv[4])->s;             /* data_type */
    int1 = (int) *((int4 *) argv[5]);           /* interchange_format */
    ptr_result = (char *) argv[6];              /* return value */
    size = long1 * long2;
    data_ptr = (PTR) OaMalloc( size);
    if (data_ptr == NULL) {
      memset( ptr_result, 0, sizeof( char *));
      return( "OaMalloc failed!");
    }
    memcpy( data_ptr, argv[1], size);
    ptr = (char *) OaImportHistogram( data_ptr, long1, long2, str1, int1);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OaImportImage") == 0) {
    long1 = (long) *((int4 *) argv[2]);         /* lines */
    long2 = (long) *((int4 *) argv[3]);         /* line_samples */
    str1 = ((STRING *) argv[4])->s;             /* sample_type */
    int1 = (int) *((int4 *) argv[5]);           /* sample_bits */
    ptr_result = (char *) argv[6];              /* return value */
    size = long1 * long2 * int1/8;
    data_ptr = (PTR) OaMalloc( size);
    if (data_ptr == NULL) {
      memset( ptr_result, 0, sizeof( char *));
      return( "OaMalloc failed!");
    }
    memcpy( data_ptr, argv[1], size);
    ptr = (char *) OaImportImage( data_ptr, long1, long2, str1, int1);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OaJoinTables") == 0) {
    oa_object1 = *((OA_OBJECT *) argv[1]);        /* table_A */
    oa_object2 = *((OA_OBJECT *) argv[2]);        /* table_B */
    int1 = (int) *((int4 *) argv[3]);             /* option */
    ptr_result = (char *) argv[4];                /* return value */
    ptr = (char *) OaJoinTables( oa_object1, oa_object2, int1);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OaOpenImage") == 0) {
    odltree = *((ODLTREE *) argv[1]);           /* input_node */
    int1 = (int) *((int4 *) argv[2]);           /* band */
    ptr_result = (char *) argv[3];              /* return value */
    ptr = (char *) OaOpenImage( odltree, int1);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OaOpenOutputFile") == 0) {
    str1 = ((STRING *) argv[1])->s;             /* data_filename */
    int1 = (int) *((int4 *) argv[2]);           /* record_type */
    long1 = (long) *((int4 *) argv[3]);         /* record_bytes */
    ptr_result = (char *) argv[4];              /* return value */
    ptr = (char *) OaOpenOutputFile( str1, int1, long1);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OaReadImage") == 0) {
    odltree = *((ODLTREE *) argv[1]);           /* image_node */
    int1 = (int) *((int4 *) argv[2]);           /* band */
    ptr_result = (char *) argv[3];              /* return value */
    ptr = (char *) OaReadImage( odltree, int1);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OaReadImageFromQube") == 0) {
    odltree = *((ODLTREE *) argv[1]);           /* qube_node */
    int1 = (int) *((int4 *) argv[2]);           /* band */
    ptr_result = (char *) argv[3];              /* return value */
    ptr = (char *) OaReadImageFromQube( odltree, int1);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OaReadSpectrumFromImage") == 0) {
    odltree = *((ODLTREE *) argv[1]);           /* image_node */
    int1 = (int) *((int4 *) argv[2]);           /* line */
    int2 = (int) *((int4 *) argv[3]);           /* sample */
    ptr_result = (char *) argv[4];              /* return value */
    ptr = (char *) OaReadSpectrumFromImage( odltree, int1, int2);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OaReadSpectrumFromQube") == 0) {
    odltree = *((ODLTREE *) argv[1]);           /* qube_node */
    int1 = (int) *((int4 *) argv[2]);           /* line */
    int2 = (int) *((int4 *) argv[3]);           /* sample */
    ptr_result = (char *) argv[4];              /* return value */
    ptr = (char *) OaReadSpectrumFromQube( odltree, int1, int2);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OaReadPartialImage") == 0) {
    oa_object = *((OA_OBJECT *) argv[1]);       /* oa_object */
    long1 = (long) *((int4 *) argv[2]);         /* start_line */
    long2 = (long) *((int4 *) argv[3]);         /* stop_line */
    long3 = (long) *((int4 *) argv[4]);         /* start_sample */
    long4 = (long) *((int4 *) argv[5]);         /* stop_sample */
    ptr_result = (char *) argv[6];              /* return value */
    ptr = (char *) OaReadPartialImage( oa_object, long1, long2, long3, long4);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OaReportError") == 0) {
    str1 = ((STRING *) argv[1])->s;             /* input_error_string */
    my_int4 = (int4) OaReportError( str1);
    memcpy( argv[2], &my_int4, 4);              /* return value */
    return( "OK");

  } else if (strcmp( proc_name, "OaTransposeTable") == 0) {
    oa_object = *((OA_OBJECT *) argv[1]);       /* table_object */
    ptr_result = (char *) argv[2];              /* return value */
    ptr = (char *) OaTransposeTable( oa_object);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OaWriteObject") == 0) {
    oa_object1 = *((OA_OBJECT *) argv[1]);      /* file_object */
    oa_object2 = *((OA_OBJECT *) argv[2]);      /* object */
    my_int4 = (int4) OaWriteObject( oa_object1, oa_object2);
    memcpy( argv[3], &my_int4, 4);              /* return value */
    return( "OK");

  } else if (strcmp( proc_name, "OaCopyTree") == 0) {
    odltree = *((ODLTREE *) argv[1]);           /* input_node */
    int1 = (int) *((int4 *) argv[2]);           /* options */
    ptr_result = (char *) argv[3];              /* return value */
    ptr = (char *) OaCopyTree( odltree, int1);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OaDeleteKwd") == 0) {
    str1 = ((STRING *) argv[1])->s;             /* kwd_name */
    odltree = *((ODLTREE *) argv[2]);           /* odltreenode */
    my_int4 = (int4) OaDeleteKwd( str1, odltree);
    memcpy( argv[3], &my_int4, 4);              /* return value */
    return( "OK");

  } else if (strcmp( proc_name, "OaGetObjectInterchangeFormat") == 0) {
    odltree = *((ODLTREE *) argv[1]);           /* TLO_object_node */
    my_int4 = (int4) OaGetObjectInterchangeFormat( odltree);
    memcpy( argv[2], &my_int4, 4);              /* return value */
    return( "OK");

  } else if (strcmp( proc_name, "OaGetImageKeywords") == 0) {
    odltree = *((ODLTREE *) argv[1]);           /* image_node */
    my_int4 = (int4) OaGetImageKeywords( odltree, &long1, &long2, &long3,
                                         &str1, &long4, &int1, &long5, &long6,
                                         &int2);
    memcpy( argv[9], &my_int4, 4);              /* return value */
    my_int4 = (int4) long1;
    memcpy( argv[2], &my_int4, 4);              /* lines */
    my_int4 = (int4) long2;
    memcpy( argv[3], &my_int4, 4);              /* line_samples */
    my_int4 = (int4) long3;
    memcpy( argv[4], &my_int4, 4);              /* sample_bits */
    my_int4 = (int4) long4;
    memcpy( argv[5], &my_int4, 4);              /* bands */
    my_int4 = (int4) int1;
    memcpy( argv[6], &my_int4, 4);              /* encoding_type */
    my_int4 = (int4) long5;
    memcpy( argv[7], &my_int4, 4);              /* line_prefix_bytes */
    my_int4 = (int4) long6;
    memcpy( argv[8], &my_int4, 4);              /* line_suffix_bytes */
    my_int4 = (int4) int2;
    memcpy( argv[9], &my_int4, 4);              /* encoding_type */
    return( str1);                              /* sample_type_str */

  } else if (strcmp( proc_name, "OaGetTableKeywords") == 0) {
    odltree = *((ODLTREE *) argv[1]);           /* odltreenode */
    my_int4 = (int4) OaGetTableKeywords( odltree, &long1, &long2, &long3,
                                         &long4, &int1, &int2);
    memcpy( argv[8], &my_int4, 4);              /* return value */
    my_int4 = (int4) long1;
    memcpy( argv[2], &my_int4, 4);              /* rows */
    my_int4 = (int4) long2;
    memcpy( argv[3], &my_int4, 4);              /* row_bytes */
    my_int4 = (int4) long3;
    memcpy( argv[4], &my_int4, 4);              /* row_prefix_bytes */
    my_int4 = (int4) long4;
    memcpy( argv[5], &my_int4, 4);              /* row_suffix_bytes */
    my_int4 = (int4) int1;
    memcpy( argv[6], &my_int4, 4);              /* interchange_format */
    my_int4 = (int4) int1;
    memcpy( argv[7], &my_int4, 4);              /* table_storage_type */
    return( "OK");

  } else if (strcmp( proc_name, "OaLongtoKwdValue") == 0) {
    str1 = ((STRING *) argv[1])->s;             /* kwd_name */
    odltree = *((ODLTREE *) argv[2]);           /* odltreenode */
    long1 = (long) *((int4 *) argv[3]);         /* keyword_value */
    my_int4 = (int4) OaLongtoKwdValue( str1, odltree, long1);
    memcpy( argv[4], &my_int4, 4);              /* return value */
    return( "OK");

  } else if (strcmp( proc_name, "OaPDSDataTypetoStr") == 0) {
    int1 = (int) *((int4 *) argv[1]);           /* PDS_data_type */
    return( OaPDSDataTypetoStr( int1));         /* return value */

  } else if (strcmp( proc_name, "OaSequencetoLongArray") == 0) {

    /* The wrapper makes two calls to this;  passing "first call" and "second
       call" in argv[1].  The first call returns the function return value
       and sequence_items.  The second call returns array, in the appropriately
       sized lonarr passed in.  The pointer to the array and sequence_items are
       stored in a static variables between calls.  */

    str1 = ((STRING *) argv[1])->s;         /* "first call" or "second call"*/

    if (strcmp( str1, "first call") == 0) {
      str1 = ((STRING *) argv[2])->s;         /* kwd_name */
      odltree = *((ODLTREE *) argv[3]);       /* odltreenode */
      my_int4 = (int4) OaSequencetoLongArray( str1, odltree, &static_long_arr,
                                              &static_int);
      memcpy( argv[5], &my_int4, 4);          /* return value */
      my_int4 = (int4) static_int;
      memcpy( argv[4], &my_int4, 4);          /* sequence_items */
      return( "OK");
    } else {
      int4_ptr = (int4 *) argv[2];            /* array */
      for (i=0; i<static_int; i++)
        int4_ptr[i] = (int4) static_long_arr[i];
      OaFree( (char *) static_long_arr);
      return( "OK");
    }

  } else if (strcmp( proc_name, "OaSequencetoStrArray") == 0) {

    /* This is called once to return the first string and sequence_items,
       then (sequence_items-1) more times to get all the strings.  The
       argv[1] parameter is a pointer to an int4 containing 0,1,2...  */

    my_int4 = *((int4 *) argv[1]);              /* # of call */
    if (my_int4 == 0) {
      str1 = ((STRING *) argv[2])->s;           /* kwd_name */
      odltree = *((ODLTREE *) argv[3]);         /* odltreenode */
      my_int4 = (int4) OaSequencetoStrArray( str1, odltree, &static_str_arr,
                                             &int1);
      memcpy( argv[5], &my_int4, 4);            /* return value */
      my_int4 = (int4) int1;
      memcpy( argv[4], &my_int4, 4);            /* sequence_items */
      if (int1 > 0) {
        return( static_str_arr[0]);
      } else {
        return( "see return value");
      }
    } else {
      return( static_str_arr[ my_int4]);
    }

  } else if (strcmp( proc_name, "OaStrtoKwdValue") == 0) {
    str1 = ((STRING *) argv[1])->s;             /* kwd_name */
    odltree = *((ODLTREE *) argv[2]);           /* odltreenode */
    str2 = ((STRING *) argv[3])->s;             /* str */
    my_int4 = (int4) OaStrtoKwdValue( str1, odltree, str2 );
    memcpy( argv[4], &my_int4, 4);              /* return value */
    return( "OK");

  } else if (strcmp( proc_name, "OaStrtoPDSDataType") == 0) {
    str1 = ((STRING *) argv[1])->s;             /* str */
    my_int4 = *((int4 *) argv[2]);              /* interchange_format */
    my_int4 = (int4) OaStrtoPDSDataType( str1, my_int4);
    memcpy( argv[3], &my_int4, 4);              /* return value */
    return( "OK");
  }
} else if (strncmp( proc_name, "Odl", 3) == 0) {

  /***************************************************************************
                             L3 Functions
  ***************************************************************************/

  if        (strcmp( proc_name, "OdlFindObjDesc") == 0) {
    odltree1 = *((ODLTREE *) argv[1]);               /* start_object */
    str1 = ((STRING *) argv[2])->s;                  /* object_class */
    str2 = ((STRING *) argv[3])->s;                  /* keyword_name */
    str3 = ((STRING *) argv[4])->s;                  /* keyword_value */
    ulong1 = (unsigned long) *((int4 *) argv[5]);    /* object_position */
    ushort1 = (unsigned short) *((int4 *) argv[6]);  /* search_scope */
    ptr_result = (char *) argv[7];                   /* return value */
    ptr = (char *) OdlFindObjDesc( odltree1, str1, str2, str3, ulong1, 
                                   ushort1);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OdlGetObjDescClassName") == 0) {
    odltree = *((ODLTREE *) argv[1]);        /* object */
    ptr = OdlGetObjDescClassName( odltree);
    return( ptr);                            /* return value */

  } else if (strcmp( proc_name, "OdlNextObjDesc") == 0) {
    odltree1 = *((ODLTREE *) argv[1]);               /* object */
    ulong1 = (unsigned long) *((int4 *) argv[2]);    /* root_level */
    ushort1 = (unsigned short) *((int4 *) argv[3]);  /* search_scope */
    ptr_result = (char *) argv[4];                   /* return value */
    ptr = (char *) OdlNextObjDesc( odltree1, ulong1, &ushort1);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OdlCutObjDesc") == 0) {
    odltree1 = *((ODLTREE *) argv[1]);               /* object */
    ptr_result = (char *) argv[2];                   /* return value */
    ptr = (char *) OdlCutObjDesc( odltree1);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OdlPasteObjDesc") == 0) {
    odltree1 = *((ODLTREE *) argv[1]);               /* new_object */
    odltree2 = *((ODLTREE *) argv[2]);               /* parent_object */
    ptr_result = (char *) argv[3];                   /* return value */
    ptr = (char *) OdlPasteObjDesc( odltree1, odltree2);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OdlPasteObjDescBefore") == 0) {
    odltree1 = *((ODLTREE *) argv[1]);               /* new_object */
    odltree2 = *((ODLTREE *) argv[2]);               /* old_object */
    ptr_result = (char *) argv[3];                   /* return value */
    ptr = (char *) OdlPasteObjDescBefore( odltree1, odltree2);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OdlPasteObjDescAfter") == 0) {
    odltree1 = *((ODLTREE *) argv[1]);               /* new_object */
    odltree2 = *((ODLTREE *) argv[2]);               /* old_object */
    ptr_result = (char *) argv[3];                   /* return value */
    ptr = (char *) OdlPasteObjDescAfter( odltree1, odltree2);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OdlCopyObjDesc") == 0) {
    odltree1 = *((ODLTREE *) argv[1]);               /* object */
    ptr_result = (char *) argv[2];                   /* return value */
    ptr = (char *) OdlCopyObjDesc( odltree1);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OdlNewObjDesc") == 0) {
    str1 = ((STRING *) argv[1])->s;                  /* object_class */
    str2 = ((STRING *) argv[2])->s;                  /* pre_comment */
    str3 = ((STRING *) argv[3])->s;                  /* line_comment */
    str4 = ((STRING *) argv[4])->s;                  /* post_comment */
    str5 = ((STRING *) argv[5])->s;                  /* end_comment */
    str6 = ((STRING *) argv[6])->s;                  /* file_name */
    ushort1 = (unsigned short) *((int4 *) argv[7]);  /* is_a_group */
    ulong1 = (unsigned long) *((int4 *) argv[8]);    /* line_number */
    ptr_result = (char *) argv[9];                   /* return value */
    ptr = (char *) OdlNewObjDesc( str1, str2, str3, str4, str5, str6,
                                  ushort1, ulong1);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OdlGetObjDescChildCount") == 0) {
    odltree1 = *((ODLTREE *) argv[1]);               /* object */
    my_int4 = (int4) OdlGetObjDescChildCount( odltree1);
    memcpy( argv[2], &my_int4, 4);                   /* return value */
    return( "OK");

  } else if (strcmp( proc_name, "OdlFindKwd") == 0) {
    odltree1 = *((ODLTREE *) argv[1]);               /* start_object */
    str1 = ((STRING *) argv[2])->s;                  /* keyword_name */
    str2 = ((STRING *) argv[3])->s;                  /* keyword_value */
    ulong1 = (unsigned long) *((int4 *) argv[4]);    /* keyword_position */
    ushort1 = (unsigned short) *((int4 *) argv[5]);  /* search_scope */
    ptr_result = (char *) argv[6];                   /* return value */
    ptr = (char *) OdlFindKwd( odltree1, str1, str2, ulong1, ushort1);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OdlNextKwd") == 0) {
    kwd1 = *((KEYWORD **) argv[1]);                  /* start_keyword */
    str1 = ((STRING *) argv[2])->s;                  /* keyword_name */
    str2 = ((STRING *) argv[3])->s;                  /* keyword_value */
    ulong1 = (unsigned long) *((int4 *) argv[4]);    /* keyword_position */
    ushort1 = (unsigned short) *((int4 *) argv[5]);  /* search_scope */
    ptr_result = (char *) argv[6];                   /* return value */
    ptr = (char *) OdlNextKwd( kwd1, str1, str2, ulong1, ushort1);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OdlCutKwd") == 0) {
    kwd1 = *((KEYWORD **) argv[1]);                  /* keyword */
    ptr_result = (char *) argv[2];                   /* return value */
    ptr = (char *) OdlCutKwd( kwd1);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OdlPasteKwd") == 0) {
    kwd1 = *((KEYWORD **) argv[1]);                  /* keyword */
    odltree1 = *((ODLTREE *) argv[2]);               /* object */
    ptr_result = (char *) argv[3];                   /* return value */
    ptr = (char *) OdlPasteKwd( kwd1, odltree1);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OdlPasteKwdBefore") == 0) {
    kwd1 = *((KEYWORD **) argv[1]);                  /* new_keyword */
    kwd2 = *((KEYWORD **) argv[2]);                  /* old_keyword */
    ptr_result = (char *) argv[3];                   /* return value */
    ptr = (char *) OdlPasteKwdBefore( kwd1, kwd2);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OdlPasteKwdAfter") == 0) {
    kwd1 = *((KEYWORD **) argv[1]);                  /* new_keyword */
    kwd2 = *((KEYWORD **) argv[2]);                  /* old_keyword */
    ptr_result = (char *) argv[3];                   /* return value */
    ptr = (char *) OdlPasteKwdAfter( kwd1, kwd2);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OdlCopyKwd") == 0) {
    kwd1 = *((KEYWORD **) argv[1]);                  /* keyword */
    ptr_result = (char *) argv[2];                   /* return value */
    ptr = (char *) OdlCopyKwd( kwd1);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OdlNewKwd") == 0) {
    str1 = ((STRING *) argv[1])->s;                  /* keyword_name */
    str2 = ((STRING *) argv[2])->s;                  /* value_text */
    str3 = ((STRING *) argv[3])->s;                  /* pre_comment */
    str4 = ((STRING *) argv[4])->s;                  /* line_comment */
    str5 = ((STRING *) argv[5])->s;                  /* file_name */
    long1 = (long) *((int4 *) argv[6]);              /* line_number */
    ptr_result = (char *) argv[7];                   /* return value */
    ptr = (char *) OdlNewKwd( str1, str2, str3, str4, str5, long1);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OdlGetFirstKwd") == 0) {
    odltree1 = *((ODLTREE *) argv[1]);               /* object */
    ptr_result = (char *) argv[2];                   /* return value */
    ptr = (char *) OdlGetFirstKwd( odltree1);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OdlGetNextKwd") == 0) {
    kwd1 = *((KEYWORD **) argv[1]);                  /* keyword */
    ptr_result = (char *) argv[2];                   /* return value */
    ptr = (char *) OdlGetNextKwd( kwd1);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OdlCopyKwd") == 0) {
    kwd1 = *((KEYWORD **) argv[1]);                  /* keyword */
    ptr_result = (char *) argv[2];                   /* return value */
    ptr = (char *) OdlCopyKwd( kwd1);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OdlFreeTree") == 0) {
    odltree1 = *((ODLTREE *) argv[1]);               /* object */
    ptr_result = (char *) argv[2];                   /* return value */
    ptr = (char *) OdlFreeTree( odltree1);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OdlFreeKwd") == 0) {
    kwd1 = *((KEYWORD **) argv[1]);                  /* keyword */
    ptr_result = (char *) argv[2];                   /* return value */
    ptr = (char *) OdlFreeKwd( kwd1);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OdlGetNextKwd") == 0) {
    kwd1 = *((KEYWORD **) argv[1]);                  /* keyword */
    ptr_result = (char *) argv[2];                   /* return value */
    ptr = (char *) OdlGetNextKwd( kwd1);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "OdlPrintLabel") == 0) {
    odltree1 = *((ODLTREE *) argv[1]);               /* object */
    str1 = ((STRING *) argv[2])->s;                  /* message_fname */
    fd = *((FILE **) argv[3]);                       /* message_fptr */
    ulong1 = (unsigned long) *((int4 *) argv[4]);    /* root_level */
    ushort1 = (unsigned short) *((int4 *) argv[5]);  /* options */
    OdlPrintLabel( odltree1, str1, fd, ulong1, ushort1);
    return( "OK");                                   /* no return value */

  } else if (strcmp( proc_name, "OdlGetKwdValue") == 0) {
    kwd1 = *((KEYWORD **) argv[1]);                  /* keyword */
    return( OdlGetKwdValue( kwd1));                   /* return value */

  } else if (strcmp( proc_name, "OdlGetKwdValueType") == 0) {
    kwd1 = *((KEYWORD **) argv[1]);                  /* keyword */
    my_int4 = (int4) OdlGetKwdValueType( kwd1);
    memcpy( argv[2], &my_int4, 4);                   /* return value */
    return( "OK");

  } else if (strcmp( proc_name, "OdlGetKwdUnit") == 0) {
    kwd1 = *((KEYWORD **) argv[1]);                  /* keyword */
    return( OdlGetKwdUnit( kwd1));                   /* return value */

  } else if (strcmp( proc_name, "OdlGetKwdName") == 0) {
    kwd1 = *((KEYWORD **) argv[1]);                  /* keyword */
    return( OdlGetKwdName( kwd1));                   /* return value */

  } else if (strcmp( proc_name, "OdlFreeAllKwds") == 0) {
    odltree1 = *((ODLTREE *) argv[1]);               /* object */
    ptr_result = (char *) argv[2];                   /* return value */
    ptr = (char *) OdlFreeAllKwds( odltree1);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");
  }


} else {

  /***************************************************************************
                             Other Functions
  ***************************************************************************/

  if         (strcmp( proc_name, "LeftmostChild") == 0) {
    odltree1 = *((ODLTREE *) argv[1]);        /* ODL tree node */
    ptr_result = (char *) argv[2];            /* return value */
    ptr = (char *) LeftmostChild( odltree1);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "LeftSibling") == 0) {
    odltree1 = *((ODLTREE *) argv[1]);        /* ODL tree node */
    ptr_result = (char *) argv[2];            /* return value */
    ptr = (char *) LeftSibling( odltree1);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "Parent") == 0) {
    odltree1 = *((ODLTREE *) argv[1]);        /* ODL tree node */
    ptr_result = (char *) argv[2];            /* return value */
    ptr = (char *) Parent( odltree1);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "RightmostChild") == 0) {
    odltree1 = *((ODLTREE *) argv[1]);        /* ODL tree node */
    ptr_result = (char *) argv[2];            /* return value */
    ptr = (char *) RightmostChild( odltree1); 
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  } else if (strcmp( proc_name, "RightSibling") == 0) {
    odltree1 = *((ODLTREE *) argv[1]);        /* ODL tree node */
    ptr_result = (char *) argv[2];            /* return value */
    ptr = (char *) RightSibling( odltree1);
    memcpy( ptr_result, &ptr, sizeof( char *));
    return( "OK");

  }
}
sprintf( return_string, "Could not find function: %s", proc_name);
return( return_string);
}

#ifdef MAC
#pragma export off
#endif
