/*****************************************************************************

  File:  OAL_fortran_interface.c

  Description: This file contains wrappers which are directly callable from
               Fortran for most of the OA Library's Object Layer routines.
               For complete documentation about OAL Object Layer routines see
               obj_l1.c and obj_l2.c.
               The routines in this file are:

               OaFortAddLineTerminatorstoTable
               OaFortCheckODLTree
               OaFortCloseImage
               OaFortCloseOutputFile
               OaFortConvertObject
               OaFortConvertObjecttoOneType
               OaFortConvertLabel
               OaFortCopyObject
               OaFortCreateAttachedLabel
               OaFortDeleteColumn
               OaFortDeleteObject
               OaFortDeleteRow
               OaFortExportObject
               OaFortFindEncodingHistogram
               OaFortFree
               OaFortGetFileKeywords
               OaFortGetObjectInfo
               OaFortGetPartialImage
               OaFortGetProfileValues
               OaFortGetSubTable
               OaFortImportColumn
               OaFortImportImage
               OaFortJoinTables
               OaFortMalloc
               OaFortOpenImage
               OaFortOpenOutputFile
               OaFortReadArray
               OaFortReadHistory
               OaFortReadHistogram
               OaFortReadImage
               OaFortReadImageFromQube
               OaFortReadImagePixels
               OaFortParseLabelFile
               OaFortReadObject
               OaFortReadPartialImage
               OaFortReadSpectrumFromImage
               OaFortReadSpectrumFromQube
               OaFortReadSubTable
               OaFortReadTable
               OaFortReportFileAttributes
               OaFortReportError
               OaFortSetProfileValues
               OaFortTransposeTable
               OaFortWriteObject

               OaFortCopyTree
               OaFortGetObjectClass
               OaFortGetObjectInterchangeFmt
               OaFortGetTableStorageType
               OaFortKwdValuetoLong
               OaFortKwdValuetoStr
               OaFortLongtoKwdValue
               OaFortObjectClasstoStr
               OaFortPDSDataTypetoStr
               OaFortStrtoKwdValue
               OaFortStrtoPDSDataType

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan  1995
  Last Modified:  16 Dec  1996

  History:

    Creation - These routines were part of the Beta Release of 
               the OA library.
    12/6/95  - Changed OaFortMalloc for new OaMalloc return type.  SM
    12/16/96 - Added BAND argument to OaReadImage and OaOpenImage, added
               OaFortReadSpectrumFromQube, OaFortReadSpectrumFromImage.  SM

  Notes on implementation:
  
  1) Argument passing:
     By default Fortran always passes function arguments by reference 
     (except for strings, explained below).  To get the actual value of
     scalars, the C code dereferences the pointer passed in from the Fortran
     caller.  A few OAL functions expect a pointer to data (the "Import"
     functions) or an array (the "Get/Read SubTable" functions) - since
     arrays are passed by reference, the wrapper passes the argument
     unchanged to the OAL function.

     Strings are passed from Fortran into C differently on VMS and UNIX:
     VMS:  A pointer to a VMS string descriptor (a C structure) is passed in;
           the structure contains a pointer to the beginning of the string, 
           and the length of the string.
     UNIX: A pointer to the beginning of the string is passed in, and an
           additional long parameter containing the string's length is
           appended to the end of the argument list.  Thus the C routine
           is declared with one more argument than the FORTRAN call has.

   2) Converting a FORTRAN string to a C string:
     Under both VMS and UNIX, the FORTRAN input string has blanks filled out
     to its length, and the string is not NULL terminated.  The code in these
     wrappers copies the passed-in FORTRAN string to a NULL-terminated C
     string, setting the C string's length as the number of characters up to
     the last non-blank character of the FORTRAN string, which is usually
     less than the FORTRAN string's length.

  3) Function return values:
     Function return values are returned by value as in C.  When an OAL
     function returns a string (char *), the wrapper is declared as a 
     subroutine instead of a function, and its last argument is the return
     string.  

  4) Converting a C string to a FORTRAN string:
     The wrappers copy a C string to the FORTRAN input string, and pad it with
     blanks out to the FORTRAN string length, which was also passed in as
     in 2) above.  Neither the FORTRAN string pointer nor the FORTRAN string
     length are ever changed.

*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "oal.h"
#if (defined( VAX) || defined( ALPHA_VMS))
#include <descrip.h>
#endif

#define FortrantoCString( c_str, f_str, f_len) \
        {                                      \
          int i;                               \
          NewString( c_str, f_len+1);          \
          for (i=0; i<f_len; i++)              \
            c_str[i] = f_str[i];               \
          c_str[i] = '\0';                     \
          StripTrailing( c_str, ' ');          \
          if (strlen( c_str) == 0) {           \
            LemmeGo( c_str)                    \
	  }                                    \
        }

#define CtoFortranString( f_str, c_str, f_len) \
        {                                      \
        int c_len,i;                           \
        if (c_str != NULL) {                   \
          c_len = strlen( c_str);              \
          if (c_len < f_len) {                 \
            strcpy( f_str, c_str);             \
            for (i=c_len; i<f_len; i++)        \
              f_str[i] = ' ';                  \
          } else {                             \
            for (i=0; i<f_len; i++)            \
              f_str[i] = c_str[i];             \
          }                                    \
        } else {                               \
          for (i=0; i<f_len; i++)              \
            f_str[i] = ' ';                    \
        }                                      \
      }


/*****************************************************************************

  Routine: OaFortAddLineTerminatorstoTable

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*(PTR_SIZ)  FUNCTION OaFortAddLineTerminatorstoTable( OBJECT)
     INTEGER*(PTR_SIZ)  OBJECT

  Note: PTR_SIZ is 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
PTR OaFortAddLineTerminatorstoTable( PTR *oa_object)
{
#else
#ifdef _NO_PROTO
PTR oafortaddlineterminatorstotable_( oa_object)
PTR *oa_object;
{
#else
PTR oafortaddlineterminatorstotable_( PTR *oa_object)
{
#endif
#endif

return( (PTR) OaAddLineTerminatorstoTable( (OA_OBJECT) *oa_object));
}


/*****************************************************************************

  Routine: OaFortCheckODLTree

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*(PTR_SIZ)  FUNCTION OaFortCheckODLTree( TLO_NODE)
     INTEGER*(PTR_SIZ)  TLO_NODE

  Note: PTR_SIZ is 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
int OaFortCheckODLTree( PTR *TLO_node)
{
#else
#ifdef _NO_PROTO
int oafortcheckodltree_( TLO_node)
PTR *TLO_node;
{
#else
int oafortcheckodltree_( PTR *TLO_node)
{
#endif
#endif

return( OaCheckODLTree( (ODLTREE) *TLO_node));
}


/*****************************************************************************

  Routine: OaFortCloseImage

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*4  FUNCTION OaFortCloseImage( IMAGE_HANDLE_OBJECT)
     INTEGER*(PTR_SIZ)  IMAGE_HANDLE_OBJECT

  Note: PTR_SIZ is 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
int OaFortCloseImage( PTR *image_handle_object)
{
#else
#ifdef _NO_PROTO
int oafortcloseimage_( image_handle_object)
PTR *image_handle_object;
{
#else
int oafortcloseimage_( PTR *image_handle_object)
{
#endif
#endif

return( OaCloseImage( (OA_OBJECT) *image_handle_object));
}                           


/*****************************************************************************

  Routine: OaFortCloseOutputFile

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*4  FUNCTION OaFortCloseOutputFile( FILE_OBJECT, LABEL_FILESPEC)
     INTEGER*(PTR_SIZ)  FILE_OBJECT
     CHARACTER*(*)      LABEL_FILESPEC
 
  Note: PTR_SIZ is 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
int OaFortCloseOutputFile( PTR *file_object,
                           struct dsc$descriptor_s *label_filespec_dsc)
{
char *label_filespec_ptr = label_filespec_dsc->dsc$a_pointer;
long label_filespec_len = label_filespec_dsc->dsc$w_length;
#else
#ifdef _NO_PROTO
int oafortcloseoutputfile_( file_object, label_filespec_ptr,  
                            label_filespec_len)
PTR *file_object;
char *label_filespec_ptr;
long label_filespec_len;
{
#else
int oafortcloseoutputfile_( PTR *file_object, char *label_filespec_ptr,
                           long label_filespec_len)
{
#endif
#endif

char *label_filespec;
int result;

FortrantoCString( label_filespec, label_filespec_ptr, label_filespec_len);
result = OaCloseOutputFile( (OA_OBJECT) *file_object, label_filespec);
LemmeGo( label_filespec);
return( result);
}                           

    
/*****************************************************************************

  Routine: OaFortConvertObject

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*(PTR_SIZ)  FUNCTION OaFortConvertObject( INPUT_OBJECT)
     INTEGER*(PTR_SIZ)  INPUT_OBJECT

  Note: PTR_SIZ is 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
PTR OaFortConvertObject( PTR *input_object)
{
#else
#ifdef _NO_PROTO
PTR oafortconvertobject_( input_object)
PTR *input_object;
{
#else
PTR oafortconvertobject_( PTR *input_object)
{
#endif
#endif

return( (PTR) OaConvertObject( (OA_OBJECT) *input_object));
}


/*****************************************************************************

  Routine: OaFortConvertObjecttoOneType

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*(PTR_SIZ)  FUNCTION OaFortConvertObjecttoOneType( INPUT_OBJECT,
                                                               DATA_TYPE,
                                                               BYTES,
                                                               RESCALE)
     INTEGER*(PTR_SIZ)  INPUT_OBJECT
     CHARACTER*(*)      DATA_TYPE
     INTEGER*4          BYTES
     INTEGER*4          RESCALE

  Note: PTR_SIZ is 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  15 Nov   1996
  Last Modified:  15 Nov   1996

  History:

    Creation - This routine was part of the Version 1.1 Release of the OA
               library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
PTR OaFortConvertObjecttoOneType( PTR *input_object,
                                  struct dsc$descriptor_s *data_type_dsc,
                                  int *bytes, int *rescale)
{
char *data_type_ptr = data_type_dsc->dsc$a_pointer;
long data_type_len = data_type_dsc->dsc$w_length;
#else
#ifdef _NO_PROTO
PTR oafortconvertobjecttoonetype_( input_object, data_type_ptr,
                                   bytes, rescale, data_type_len)
PTR *input_object;
char *data_type_ptr;
int *bytes;
int *rescale;
long data_type_len;
{
#else
PTR oafortconvertobjecttoonetype_( PTR *input_object, char *data_type_ptr,
                                   int *bytes, int *rescale,
                                   long data_type_len)
{
#endif
#endif

char *data_type;
PTR result;

FortrantoCString( data_type, data_type_ptr, data_type_len);
result = (PTR) OaConvertObjecttoOneType( (OA_OBJECT) *input_object,
                                         data_type, *bytes, *rescale);
LemmeGo( data_type);
return( result);
}


/*****************************************************************************

  Routine: OaFortConvertLabel

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*(PTR_SIZ)  FUNCTION OaFortConvertLabel( ROOT_NODE)
     INTEGER*(PTR_SIZ)  ROOT_NODE

  Note: PTR_SIZ is 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
PTR OaFortConvertLabel( PTR *root_node)
{
#else
#ifdef _NO_PROTO
PTR oafortconvertlabel_( root_node)
PTR *root_node;
{
#else
PTR oafortconvertlabel_( PTR *root_node)
{
#endif
#endif

return( (PTR) OaConvertLabel( (ODLTREE) *root_node));
}


/*****************************************************************************

  Routine: OaFortCopyObject

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*(PTR_SIZ)  FUNCTION OaFortCopyObject( OA_OBJECT)
     INTEGER*(PTR_SIZ)  OA_OBJECT

  Note: PTR_SIZ is 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
PTR OaFortCopyObject( PTR *oa_object)
{
#else
#ifdef _NO_PROTO
PTR oafortcopyobject_( oa_object)
PTR *oa_object;
{
#else
PTR oafortcopyobject_( PTR *oa_object)
{
#endif
#endif

return( (PTR) OaCopyObject( (OA_OBJECT) *oa_object));
}


/*****************************************************************************

  Routine: OaFortCreateAttachedLabel

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*4  FUNCTION OaFortCreateAttachedLabel( ODLTREE, LABEL_FILESPEC,
                                                    ATTACHED_LABEL_FILESPEC)
     INTEGER*(PTR_SIZ)  ODLTREE
     CHARACTER*(*)      LABEL_FILESPEC
     CHARACTER*(*)      ATTACHED_LABEL_FILESPEC
 
  Note: PTR_SIZ is 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  25 Nov   1996
  Last Modified:  25 Nov   1996

  History:

    Creation - This routine was part of the Version 1.1 Release of the OA
               library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
int OaFortCreateAttachedLabel( PTR *odltree,
                          struct dsc$descriptor_s *label_filespec_dsc,
                          struct dsc$descriptor_s *attached_label_filespec_dsc)
{
char *label_filespec_ptr = label_filespec_dsc->dsc$a_pointer;
long label_filespec_len = label_filespec_dsc->dsc$w_length;
char *attached_label_filespec_ptr = attached_label_filespec_dsc->dsc$a_pointer;
long attached_label_filespec_len = attached_label_filespec_dsc->dsc$w_length;
#else
#ifdef _NO_PROTO
int oafortcreateattachedlabel_( odltree, label_filespec_ptr,
                                attached_label_filespec_ptr, 
                                label_filespec_len,
                                attached_label_filespec_len)
PTR *odltree;
char *label_filespec_ptr;
long label_filespec_len;
char *attached_label_filespec_ptr;
long attached_label_filespec_len;
{
#else
int oafortcreateattachedlabel_( PTR *odltree, char *label_filespec_ptr,
                                char *attached_label_filespec_ptr,
                                long label_filespec_len,
                                long attached_label_filespec_len)
{
#endif
#endif

char *label_filespec;
char *attached_label_filespec;
int result;

FortrantoCString( label_filespec, label_filespec_ptr, label_filespec_len);
FortrantoCString( attached_label_filespec, attached_label_filespec_ptr,
                  attached_label_filespec_len);
result = OaCreateAttachedLabel( (ODLTREE) *odltree, label_filespec,
                                attached_label_filespec);
LemmeGo( label_filespec);
LemmeGo( attached_label_filespec);
return( result);
}                           


/*****************************************************************************

  Routine: OaFortDeleteColumn

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*(PTR_SIZ)  FUNCTION OaFortDeleteColumn( OA_OBJECT, INPUT_NODE)
     INTEGER*(PTR_SIZ)  OA_OBJECT
     INTEGER*(PTR_SIZ)  INPUT_NODE

  Note: PTR_SIZ is 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
PTR OaFortDeleteColumn( PTR *oa_object, PTR *input_node)
{
#else
#ifdef _NO_PROTO
PTR oafortdeletecolumn_( oa_object, input_node)
PTR *oa_object;
PTR *input_node;
{
#else
PTR oafortdeletecolumn_( PTR *oa_object, PTR *input_node)
{
#endif
#endif

return( (PTR) OaDeleteColumn( (OA_OBJECT) *oa_object, (ODLTREE) *input_node));
}


/*****************************************************************************

  Routine: OaFortDeleteObject

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*4  FUNCTION OaFortDeleteObject( OA_OBJECT)
     INTEGER*(PTR_SIZ)  OA_OBJECT

  Note: PTR_SIZ is 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
int OaFortDeleteObject( PTR *oa_object)
{
#else
#ifdef _NO_PROTO
int oafortdeleteobject_( oa_object)
PTR *oa_object;
{
#else
int oafortdeleteobject_( PTR *oa_object)
{
#endif
#endif

return( OaDeleteObject( (OA_OBJECT) *oa_object));
}


/*****************************************************************************

  Routine: OaFortDeleteRow

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*(PTR_SIZ) FUNCTION OaFortDeleteRow( OA_OBJECT, ROW)
     INTEGER*(PTR_SIZ)   OA_OBJECT
     INTEGER*(LONG_SIZ)  ROW

  Note: PTR_SIZ and LONG_SIZ are 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
PTR OaFortDeleteRow( PTR *oa_object, long *row_ptr)
{
#else
#ifdef _NO_PROTO
PTR oafortdeleterow_( oa_object, row_ptr)
PTR *oa_object;
long *row_ptr;
{
#else
PTR oafortdeleterow_( PTR *oa_object, long *row_ptr)
{
#endif
#endif

return( (PTR) OaDeleteRow( (OA_OBJECT) *oa_object, *row_ptr));
}


/*****************************************************************************

  Routine: OaFortExportObject

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*(PTR_SIZ)  FUNCTION OaFortExportObject( OA_OBJECT)
     INTEGER*(PTR_SIZ)  OA_OBJECT

  Note: PTR_SIZ is 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
PTR OaFortExportObject( PTR *oa_object)
{
#else
#ifdef _NO_PROTO
PTR oafortexportobject_( oa_object)
PTR *oa_object;
{
#else
PTR oafortexportobject_( PTR *oa_object)
{
#endif
#endif

return( (PTR) OaExportObject( (OA_OBJECT) *oa_object));
}


/*****************************************************************************

  Routine: OaFortFindEncodingHistogram

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*(PTR_SIZ)  FUNCTION OaFortFindEncodingHistogram( IMAGE_NODE)
     INTEGER*(PTR_SIZ)  IMAGE_NODE

  Note: PTR_SIZ is 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
PTR OaFortFindEncodingHistogram( PTR *image_node)
{
#else
#ifdef _NO_PROTO
PTR oafortfindencodinghistogram_( image_node)
PTR *image_node;
{
#else
PTR oafortfindencodinghistogram_( PTR *image_node)
{
#endif
#endif

return( (PTR) OaFindEncodingHistogram( (ODLTREE) *image_node));
}


/*****************************************************************************

  Routine: OaFortFree

  Description:  This routine is equivalent to a FORTRAN subroutine declared as
                follows:

     subroutine OaFortFree( OA_POINTER)
     INTEGER*4  OA_POINTER

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
void OaFortFree( PTR *ptr)
{
#else
#ifdef _NO_PROTO
void oafortfree_( ptr)
PTR *ptr;
{
#else
void oafortfree_( PTR *ptr)
{
#endif
#endif

OaFree( *ptr);
}


/*****************************************************************************

  Routine: OaFortGetFileKeywords

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*4  FUNCTION OaFortGetFileKeywords( ODLTREENODE, 
                                                LABEL_FILENAME,
                                                DATA_FILENAME,
                                                RECORD_TYPE,  RECORD_BYTES,
                                                FILE_RECORDS,
                                                OBJECT_BYTE_OFFSET,
                                                OBJECT_INTERCHANGE_FORMAT)
     INTEGER*(PTR_SIZ)   ODLTREENODE
     CHARACTER*(*)       LABEL_FILENAME
     CHARACTER*(*)       DATA_FILENAME
     INTEGER*4           RECORD_TYPE
     INTEGER*(LONG_SIZ)  RECORD_BYTES
     INTEGER*(LONG_SIZ)  FILE_RECORDS
     INTEGER*(LONG_SIZ)  OBJECT_BYTE_OFFSET
     INTEGER*(LONG_SIZ)  OBJECT_INTERCHANGE_FORMAT

  Note: PTR_SIZ and LONG_SIZ are 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
int OaFortGetFileKeywords( PTR *odltreenode, 
                           struct dsc$descriptor_s *label_filename_dsc,
                           struct dsc$descriptor_s *data_filename_dsc,
                           int *record_type,  
                           long *record_bytes,
                           long *file_records, 
                           long *object_byte_offset, 
                           int *object_interchange_format)
{
char *label_filename_ptr = label_filename_dsc->dsc$a_pointer;
long label_filename_len = label_filename_dsc->dsc$w_length;
char *data_filename_ptr = data_filename_dsc->dsc$a_pointer;
long data_filename_len = data_filename_dsc->dsc$w_length;
#else
#ifdef _NO_PROTO
int oafortgetfilekeywords_( odltreenode, label_filename_ptr, data_filename_ptr,
                            record_type, record_bytes, file_records, 
                            object_byte_offset, object_interchange_format,
                            label_filename_len, data_filename_len)
PTR *odltreenode;
char *label_filename_ptr;
char *data_filename_ptr;
int *record_type;
long *record_bytes;
long *file_records;
long *object_byte_offset;
int *object_interchange_format;
long label_filename_len;
long data_filename_len;
{
#else
int oafortgetfilekeywords_( PTR *odltreenode, char *label_filename_ptr,
                            char *data_filename_ptr, int *record_type,
                            long *record_bytes, long *file_records, 
                            long *object_byte_offset, 
                            int *object_interchange_format,
                            long label_filename_len, long data_filename_len)
{
#endif
#endif

char *label_filename;
char *data_filename;
int result;

result = OaGetFileKeywords( (ODLTREE) *odltreenode, &label_filename,
                            &data_filename, record_type, record_bytes,
                            file_records, object_byte_offset,
                            object_interchange_format);
CtoFortranString( label_filename_ptr, label_filename, label_filename_len);
CtoFortranString( data_filename_ptr, data_filename, data_filename_len);
LemmeGo( label_filename);
LemmeGo( data_filename);
return( result);
}


/*****************************************************************************

  Routine: OaFortGetObjectInfo

  Description:  This routine is equivalent to a FORTRAN subroutine declared as
                follows:

     subroutine OaFortGetObjectInfo( OA_OBJECT, ODLTREE, DATA_PTR, SIZE,
                                     IS_IN_MEMORY)
                                     
     INTEGER*(PTR_SIZ)  OA_OBJECT
     INTEGER*(PTR_SIZ)  ODLTREE
     INTEGER*(PTR_SIZ)  DATA_PTR
     INTEGER*(LONG_SIZ) SIZE
     INTEGER*1          IS_IN_MEMORY

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
void OaFortGetObjectInfo( PTR  *oa_object, PTR  *odltree, PTR  *data_ptr,
                          long *size, char *is_in_memory)
{
#else
#ifdef _NO_PROTO
void oafortgetobjectinfo_( oa_object, odltree, data_ptr, size, is_in_memory)
PTR  *oa_object;
PTR  *odltree;
PTR  *data_ptr;
long *size;
char *is_in_memory;
{
#else
void oafortgetobjectinfo_( PTR  *oa_object, PTR  *odltree, PTR  *data_ptr,
                           long *size, char *is_in_memory)
{
#endif
#endif

OA_OBJECT input_oa_object = (OA_OBJECT) *oa_object;
*odltree      = (PTR) input_oa_object->odltree;
*data_ptr     = (PTR) input_oa_object->data_ptr;
*size         = (long) input_oa_object->size;
*is_in_memory = (char) input_oa_object->is_in_memory;
}


/*****************************************************************************

  Routine: OaFortGetPartialImage

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*(PTR_SIZ)  FUNCTION OaFortGetPartialImage( OA_OBJECT, START_LINE,
                                                        STOP_LINE,  
                                                        START_SAMPLE,
                                                        STOP_SAMPLE)
     INTEGER*(PTR_SIZ)  OA_OBJECT
     INTEGER*(LONG_SIZ)  START_LINE
     INTEGER*(LONG_SIZ)  STOP_LINE
     INTEGER*(LONG_SIZ)  START_SAMPLE
     INTEGER*(LONG_SIZ)  STOP_SAMPLE

  Note: PTR_SIZ and LONG_SIZ are 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
PTR OaFortGetPartialImage( PTR *oa_object, long *start_line, long *stop_line,
                           long *start_sample, long *stop_sample)
{
#else
#ifdef _NO_PROTO
PTR oafortgetpartialimage_( oa_object, start_line, stop_line, start_sample,
                           stop_sample)
PTR *oa_object;
long *start_line;
long *stop_line;
long *start_sample;
long *stop_sample;
{
#else
PTR oafortgetpartialimage_( PTR *oa_object, long *start_line, long *stop_line,
                           long *start_sample, long *stop_sample)
{
#endif
#endif

return( (PTR) OaGetPartialImage( (OA_OBJECT) *oa_object, *start_line,
                                 *stop_line, *start_sample, *stop_sample));
}


/*****************************************************************************

  Routine: OaFortGetProfileValues

  Description:  This routine is equivalent to a FORTRAN subroutine declared as
                follows:

     subroutine OaFortGetProfileValues( DST_FORMAT_FOR_ASCII_SRC,
                                        DST_FORMAT_FOR_BINARY_SRC,
                                        DST_ALIGNMENT_TYPE,
                                        DATA_TRANSLATION_PROFILE,
                                        CHECK_ASCII_WRITES)
     byte DST_FORMAT_FOR_ASCII_SRC
     byte DST_FORMAT_FOR_BINARY_SRC
     byte DST_ALIGNMENT_TYPE
     byte DATA_TRANSLATION_PROFILE
     byte CHECK_ASCII_WRITES

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
void OaFortGetProfileValues( char *dst_format_for_ASCII_src,
                             char *dst_format_for_binary_src,
                             char *dst_alignment_type,
                             char *data_translation_profile,
                             char *check_ASCII_writes)
{
#else
#ifdef _NO_PROTO
void oafortgetprofilevalues_( dst_format_for_ASCII_src, 
                              dst_format_for_binary_src,
                              dst_alignment_type, data_translation_profile,
                              check_ASCII_writes)
char *dst_format_for_ASCII_src;
char *dst_format_for_binary_src;
char *dst_alignment_type;
char *data_translation_profile;
char *check_ASCII_writes;
{
#else
void oafortgetprofilevalues_( char *dst_format_for_ASCII_src,
                              char *dst_format_for_binary_src,
                              char *dst_alignment_type,
                              char *data_translation_profile,
                              char *check_ASCII_writes)
{
#endif
#endif

*dst_format_for_ASCII_src  = (char) Oa_profile.dst_format_for_ASCII_src;
*dst_format_for_binary_src = (char) Oa_profile.dst_format_for_binary_src;
*dst_alignment_type        = (char) Oa_profile.dst_alignment_type;
*data_translation_profile  = (char) Oa_profile.data_translation_profile;
*check_ASCII_writes        = (char) Oa_profile.check_ASCII_writes;
}


/*****************************************************************************

  Routine: OaFortGetSubTable

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*(PTR_SIZ)  FUNCTION OaFortGetSubTable( OA_OBJECT, START_ROW,  
                                                    STOP_ROW,
                                                    SUBOBJECT_NODES,
                                                    N_SUBOBJECT_NODES)
     INTEGER*(PTR_SIZ)   OA_OBJECT
     INTEGER*(LONG_SIZ)  START_ROW
     INTEGER*(LONG_SIZ)  STOP_ROW
     INTEGER*(PTR_SIZ)   SUBOBJECT_NODES(*)
     INTEGER*4           N_SUBOBJECT_NODES

  Note: PTR_SIZ and LONG_SIZ are 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
PTR OaFortGetSubTable( PTR *oa_object, long *start_row, long *stop_row,
                       PTR subobject_nodes[], int *n_subobject_nodes)
{
#else
#ifdef _NO_PROTO
PTR oafortgetsubtable_( oa_object, start_row, stop_row, subobject_nodes,
                        n_subobject_nodes)
PTR *oa_object;
long *start_row;
long *stop_row;
PTR subobject_nodes[];
int *n_subobject_nodes;
{
#else
PTR oafortgetsubtable_( PTR *oa_object, long *start_row, long *stop_row,
                        PTR subobject_nodes[], int *n_subobject_nodes)
{
#endif
#endif

return( (PTR) OaGetSubTable( (OA_OBJECT) *oa_object, *start_row, *stop_row,
                             (ODLTREE *) subobject_nodes,  
                              *n_subobject_nodes));
}


/*****************************************************************************

  Routine: OaFortImportColumn

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*(PTR_SIZ)  FUNCTION OaFortImportColumn( DATA_PTR, ROWS, ITEMS,
                                                     ITEM_BYTES, DATA_TYPE,
                                                     INTERCHANGE_FORMAT, NAME)
     <any numeric scalar or array type> DATA_PTR
     INTEGER*(LONG_SIZ)  ROWS
     INTEGER*(LONG_SIZ)  ITEMS
     INTEGER*(LONG_SIZ)  ITEM_BYTES
     CHARACTER*(*)       DATA_TYPE
     INTEGER*4           INTERCHANGE_FORMAT
     CHARACTER*(*)       NAME

  Note: LONG_SIZ is 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
PTR OaFortImportColumn( char *data_ptr, long *rows, long *items, 
                        long *item_bytes, 
                        struct dsc$descriptor_s *data_type_dsc,
                        int *interchange_format,
                        struct dsc$descriptor_s *name_dsc)
{
char *data_type_ptr = data_type_dsc->dsc$a_pointer;
long data_type_len = data_type_dsc->dsc$w_length;
char *name_ptr = name_dsc->dsc$a_pointer;
long name_len = name_dsc->dsc$w_length;
#else
#ifdef _NO_PROTO
PTR oafortimportcolumn_( data_ptr, rows, items, item_bytes, data_type_ptr,  
                         interchange_format, name_ptr, data_type_len, name_len)
char *data_ptr;
long *rows;
long *items;
long *item_bytes;
char *data_type_ptr;
int *interchange_format;
char *name_ptr;
long data_type_len;
long name_len;
{
#else
PTR oafortimportcolumn_( char *data_ptr, long *rows, long *items, 
                         long *item_bytes, char *data_type_ptr, 
                         int *interchange_format, char *name_ptr, 
                         long data_type_len, long name_len)
{
#endif
#endif

char *data_type;
char *name;
PTR result;

FortrantoCString( data_type, data_type_ptr, data_type_len);
FortrantoCString( name, name_ptr, name_len);
result = (PTR) OaImportColumn( data_ptr, *rows, *items, *item_bytes,
                               data_type, (int) *interchange_format, name);
LemmeGo( data_type);
LemmeGo( name);
return( result);
}


/*****************************************************************************

  Routine: OaFortImportImage

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*4  FUNCTION OaFortImportImage( DATA_PTR, LINES, LINE_SAMPLES,
                                            SAMPLE_TYPE, SAMPLE_BITS)
     <any numeric or numeric array type> DATA_PTR
     INTEGER*(LONG_SIZ)  LINES
     INTEGER*(LONG_SIZ)  LINE_SAMPLES
     CHARACTER*(*)       SAMPLE_TYPE
     INTEGER*4           SAMPLE_BITS

  Note: LONG_SIZ is 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
PTR OaFortImportImage( char *data_ptr, long *lines, long *line_samples, 
                       struct dsc$descriptor_s *sample_type_dsc,
                       int *sample_bits)
{
char *sample_type_ptr = sample_type_dsc->dsc$a_pointer;
long sample_type_len = sample_type_dsc->dsc$w_length;
#else
#ifdef _NO_PROTO
PTR oafortimportimage_( data_ptr, lines, line_samples, sample_type_ptr,
                        sample_bits, sample_type_len)
char *data_ptr;
long *lines;
long *line_samples;
char *sample_type_ptr;
int *sample_bits;
long sample_type_len;
{
#else
PTR oafortimportimage_( char *data_ptr, long *lines, long *line_samples, 
                        char *sample_type_ptr, int *sample_bits,
                        long sample_type_len)
{
#endif
#endif

char *sample_type;
PTR result;

FortrantoCString( sample_type, sample_type_ptr, sample_type_len);
result = (PTR) OaImportImage( data_ptr, *lines, *line_samples,
                              sample_type, *sample_bits);
LemmeGo( sample_type);
return( result);
}


/*****************************************************************************

  Routine: OaFortJoinTables

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*(PTR_SIZ)  FUNCTION OaFortJoinTables( TABLE_A, TABLE_B, OPTION)

     INTEGER*(PTR_SIZ)  TABLE_A
     INTEGER*(PTR_SIZ)  TABLE_B
     INTEGER*4          OPTION

  Note: PTR_SIZ is 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
PTR OaFortJoinTables( PTR *table_A, PTR *table_B, int *option)
{
#else
#ifdef _NO_PROTO
PTR oafortjointables_( table_A, table_B, option)
PTR *table_A;
PTR *table_B;
int *option;
{
#else
PTR oafortjointables_( PTR *table_A, PTR *table_B, int *option)
{
#endif
#endif

return( (PTR) OaJoinTables( (OA_OBJECT) *table_A, (OA_OBJECT) *table_B,
                            *option));
}


/*****************************************************************************

  Routine: OaFortMalloc

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*(PTR_SIZ)  FUNCTION OaFortMalloc( BYTES)
     INTEGER*(LONG_SIZ)  BYTES

  Note: PTR_SIZ and LONG_SIZ are 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
char *OaFortMalloc( long *bytes)
{
#else
#ifdef _NO_PROTO
char *oafortmalloc_( bytes)
long *bytes;
{
#else
char *oafortmalloc_( long *bytes)
{
#endif
#endif

return( (char *) OaMalloc( *bytes));
}


/*****************************************************************************

  Routine: OaFortOpenImage

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*(PTR_SIZ)  FUNCTION OaFortOpenImage( INPUT_NODE, BAND)
     INTEGER*(PTR_SIZ)  INPUT_NODE
     INTEGER*4          BAND

  Note: PTR_SIZ is 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  16 Dec   1996

  History:

    Creation - This routine was part of the Beta Release of the OA library.
    12/16/96 - Added BAND argument.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
PTR OaFortOpenImage( PTR *input_node, int *band)
{
#else
#ifdef _NO_PROTO
PTR oafortopenimage_( input_node, band)
PTR *input_node;
int *band;
{
#else
PTR oafortopenimage_( PTR *input_node, int *band)
{
#endif
#endif

return( (PTR) OaOpenImage( (ODLTREE) *input_node, *band));
}


/*****************************************************************************

  Routine: OaFortOpenOutputFile

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*(PTR_SIZ)  FUNCTION OaFortOpenOutputFile( DATA_FILESPEC,
                                                       RECORD_TYPE,
                                                       RECORD_BYTES)
     CHARACTER*(*)      DATA_FILESPEC
     INTEGER*4          RECORD_TYPE
     INTEGER*(PTR_SIZ)  RECORD_BYTES

  Note: PTR_SIZ is 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
PTR OaFortOpenOutputFile( struct dsc$descriptor_s *data_filespec_dsc,
                          int *record_type, long *record_bytes)
{
char *data_filespec_ptr = data_filespec_dsc->dsc$a_pointer;
long data_filespec_len = data_filespec_dsc->dsc$w_length;
#else
#ifdef _NO_PROTO
PTR oafortopenoutputfile_( data_filespec_ptr, record_type, record_bytes, 
                           data_filespec_len)
char *data_filespec_ptr;
int *record_type;
long *record_bytes;
long data_filespec_len;
{
#else
PTR oafortopenoutputfile_( char *data_filespec_ptr, int *record_type,
                           long *record_bytes, long data_filespec_len)
{
#endif
#endif

char *data_filespec;
PTR result;

FortrantoCString( data_filespec, data_filespec_ptr, data_filespec_len);
result = (PTR) OaOpenOutputFile( data_filespec, *record_type, *record_bytes);
LemmeGo( data_filespec);
return( result);
}


/*****************************************************************************

  Routine: OaFortReadArray

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*(PTR_SIZ)  FUNCTION OaFortReadArray( ARRAY_NODE)
     INTEGER*(PTR_SIZ)  ARRAY_NODE

  Note: PTR_SIZ is 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
PTR OaFortReadArray( PTR *array_node)
{
#else
#ifdef _NO_PROTO
PTR oafortreadarray_( array_node)
PTR *array_node;
{
#else
PTR oafortreadarray_( PTR *array_node)
{
#endif
#endif

return( (PTR) OaReadArray( (ODLTREE) *array_node));
}


/*****************************************************************************

  Routine: OaFortReadHistory

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*(PTR_SIZ)  FUNCTION OaFortReadHistory( INPUT_NODE)
     INTEGER*(PTR_SIZ)  INPUT_NODE

  Note: PTR_SIZ is 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
PTR OaFortReadHistory( PTR *input_node)
{
#else
#ifdef _NO_PROTO
PTR oafortreadhistory_( input_node)
PTR *input_node;
{
#else
PTR oafortreadhistory_( PTR *input_node)
{
#endif
#endif

return( (PTR) OaReadHistory( (ODLTREE) *input_node));
}


/*****************************************************************************

  Routine: OaFortReadHistogram

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*(PTR_SIZ)  FUNCTION OaFortReadHistogram( HISTOGRAM_NODE)
     INTEGER*(PTR_SIZ)  HISTOGRAM_NODE

  Note: PTR_SIZ is 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
PTR OaFortReadHistogram( PTR *histogram_node)
{
#else
#ifdef _NO_PROTO
PTR oafortreadhistogram_( histogram_node)
PTR *histogram_node;
{
#else
PTR oafortreadhistogram_( PTR *histogram_node)
{
#endif
#endif

return( (PTR) OaReadHistogram( (ODLTREE) *histogram_node));
}


/*****************************************************************************

  Routine: OaFortReadImage

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*(PTR_SIZ)  FUNCTION OaFortReadImage( IMAGE_NODE, BAND)
     INTEGER*(PTR_SIZ)  IMAGE_NODE
     INTEGER*4          BAND

  Note: PTR_SIZ is 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  16 Dec   1996

  History:

    Creation - This routine was part of the Beta Release of the OA library.
    12/16/96 - Added BAND argument.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
PTR OaFortReadImage( PTR *image_node, int *band)
{
#else
#ifdef _NO_PROTO
PTR oafortreadimage_( image_node, band)
PTR *image_node;
int *band;
{
#else
PTR oafortreadimage_( PTR *image_node, int *band)
{
#endif
#endif

return( (PTR) OaReadImage( (ODLTREE) *image_node, *band));
}


/*****************************************************************************

  Routine: OaFortReadImagePixels

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*4  FUNCTION OaFortReadImagePixels( OA_OBJECT, START_LINE,
                                                START_SAMPLE)
     INTEGER*(PTR_SIZ)   OA_OBJECT
     INTEGER*(LONG_SIZ)  START_LINE
     INTEGER*(LONG_SIZ)  START_SAMPLE

  Note: PTR_SIZ and LONG_SIZ are 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
int OaFortReadImagePixels( PTR *oa_object, long *start_line,
                           long *start_sample)
{
#else
#ifdef _NO_PROTO
int oafortreadimagepixels_( oa_object, start_line, start_sample)
PTR *oa_object;
long *start_line;
long *start_sample;
{
#else
int oafortreadimagepixels_( PTR *oa_object, long *start_line,
                            long *start_sample)
{
#endif
#endif

return( OaReadImagePixels( (OA_OBJECT) *oa_object, *start_line, 
                            *start_sample));
}


/*****************************************************************************

  Routine: OaFortParseLabelFile

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*(PTR_SIZ)  FUNCTION OaFortParseLabelFile( LABEL_FILENAME,  
                                                       ERRFILESPEC,
                                                       EXPAND, NOMSGS)
     CHARACTER*(*) LABEL_FILENAME
     CHARACTER*(*) ERRFILESPEC
     INTEGER*2     EXPAND
     INTEGER*2     NOMSGS

  Note: PTR_SIZ is 4 on most platforms, 8 on Dec/Alpha.

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
PTR OaFortParseLabelFile( struct dsc$descriptor_s *label_filename_dsc, 
                          struct dsc$descriptor_s *errfilespec_dsc,
                          MASK *expand_ptr, unsigned short *nomsgs_ptr)
{
char *label_filename_ptr = label_filename_dsc->dsc$a_pointer;
long label_filename_len = label_filename_dsc->dsc$w_length;
char *errfilespec_ptr = errfilespec_dsc->dsc$a_pointer;
long errfilespec_len = errfilespec_dsc->dsc$w_length;
#else
#ifdef _NO_PROTO
PTR oafortparselabelfile_( label_filename_ptr, errfilespec_ptr,
                           expand_ptr, nomsgs_ptr, label_filename_len,
                           errfilespec_len)
char *label_filename_ptr;
char *errfilespec_ptr;
MASK *expand_ptr;
unsigned short *nomsgs_ptr;
long label_filename_len;
long errfilespec_len;
{
#else
PTR oafortparselabelfile_( char *label_filename_ptr,
                           char *errfilespec_ptr,
                           MASK *expand_ptr,
                           unsigned short *nomsgs_ptr,
                           long label_filename_len,
                           long errfilespec_len)
{
#endif
#endif

char *label_filename;
char *errfilespec;
PTR return_value;

FortrantoCString( label_filename, label_filename_ptr, label_filename_len);
FortrantoCString( errfilespec, errfilespec_ptr, errfilespec_len);

return_value = (PTR) OaParseLabelFile( label_filename, errfilespec, 
                                       *expand_ptr, *nomsgs_ptr);
LemmeGo( label_filename);
LemmeGo( errfilespec);
return( return_value);
}


/*****************************************************************************

  Routine: OaFortReadObject

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*(PTR_SIZ)  FUNCTION OaFortReadObject( OBJECT_NODE)
     INTEGER*(PTR_SIZ)  OBJECT_NODE

  Note: PTR_SIZ is 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
PTR OaFortReadObject( PTR *object_node)
{
#else
#ifdef _NO_PROTO
PTR oafortreadobject_( object_node)
PTR *object_node;
{
#else
PTR oafortreadobject_( PTR *object_node)
{
#endif
#endif

return( (PTR) OaReadObject( (ODLTREE) *object_node));
}


/*****************************************************************************

  Routine: OaFortReadImageFromQube

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*(PTR_SIZ)  FUNCTION OaFortReadImageFromQube( QUBE_NODE, BAND)
     INTEGER*(PTR_SIZ)   QUBE_NODE
     INTEGER*4           BAND

  Note: PTR_SIZ and LONG_SIZ are 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  16 Dec   1996
  Last Modified:  16 Dec   1996

  History:

    Creation - This routine was part of Release 1.2 of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
PTR OaFortReadImageFromQube( PTR *qube_node, int *band)
{
#else
#ifdef _NO_PROTO
PTR oafortreadimagefromqube_( qube_node, band)
PTR *qube_node;
int *band;
{
#else
PTR oafortreadimagefromqube_( PTR *qube_node, int *band)
{
#endif
#endif

return( (PTR) OaReadImageFromQube( (ODLTREE) *qube_node, *band));
}


/*****************************************************************************

  Routine: OaFortReadPartialImage

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*(PTR_SIZ)  FUNCTION OaFortReadPartialImage( OA_OBJECT, START_LINE,
                                                         STOP_LINE,  
                                                         START_SAMPLE,
                                                         STOP_SAMPLE)
     INTEGER*(PTR_SIZ)   OA_OBJECT
     INTEGER*(LONG_SIZ)  START_LINE
     INTEGER*(LONG_SIZ)  STOP_LINE
     INTEGER*(LONG_SIZ)  START_SAMPLE
     INTEGER*(LONG_SIZ)  STOP_SAMPLE

  Note: PTR_SIZ and LONG_SIZ are 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
PTR OaFortReadPartialImage( PTR *oa_object, long *start_line, 
                            long *stop_line, long *start_sample,
                            long *stop_sample)
{
#else
#ifdef _NO_PROTO
PTR oafortreadpartialimage_( oa_object, start_line, stop_line, start_sample,
                             stop_sample)
PTR *oa_object;
long *start_line;
long *stop_line;
long *start_sample;
long *stop_sample;
{
#else
PTR oafortreadpartialimage_( PTR *oa_object, long *start_line, 
                             long *stop_line, long *start_sample,
                             long *stop_sample)
{
#endif
#endif

return( (PTR) OaReadPartialImage( (OA_OBJECT) *oa_object, *start_line,
                                  *stop_line, *start_sample, *stop_sample));
}


/*****************************************************************************

  Routine: OaFortReadSpectrumFromImage

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*(PTR_SIZ)  FUNCTION OaFortReadSpectrumFromImage( IMAGE_NODE,
                                                              LINE,
                                                              SAMPLE)
     INTEGER*(PTR_SIZ)   IMAGE_NODE
     INTEGER*4           LINE
     INTEGER*4           SAMPLE

  Note: PTR_SIZ and LONG_SIZ are 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  16 Dec   1996
  Last Modified:  16 Dec   1996

  History:

    Creation - This routine was part of Release 1.2 of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
PTR OaFortReadSpectrumFromImage( PTR *image_node, int *line, int *sample)
{
#else
#ifdef _NO_PROTO
PTR oafortreadspectrumfromimage_( image_node, line, sample)
PTR *image_node;
int *line;
int *sample;
{
#else
PTR oafortreadspectrumfromimage_( PTR *image_node, int *line, int *sample)
{
#endif
#endif

return( (PTR) OaReadSpectrumFromImage( (ODLTREE) *image_node, *line,
                                       *sample));
}


/*****************************************************************************

  Routine: OaFortReadSpectrumFromQube

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*(PTR_SIZ)  FUNCTION OaFortReadSpectrumFromQube( QUBE_NODE,
                                                             LINE,
                                                             SAMPLE)
     INTEGER*(PTR_SIZ)   QUBE_NODE
     INTEGER*4           LINE
     INTEGER*4           SAMPLE

  Note: PTR_SIZ and LONG_SIZ are 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  16 Dec   1996
  Last Modified:  16 Dec   1996

  History:

    Creation - This routine was part of Release 1.2 of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
PTR OaFortReadSpectrumFromQube( PTR *qube_node, int *line, int *sample)
{
#else
#ifdef _NO_PROTO
PTR oafortreadspectrumfromqube_( qube_node, line, sample)
PTR *qube_node;
int *line;
int *sample;
{
#else
PTR oafortreadspectrumfromqube_( PTR *qube_node, int *line, int *sample)
{
#endif
#endif

return( (PTR) OaReadSpectrumFromQube( (ODLTREE) *qube_node, *line, *sample));
}


/*****************************************************************************

  Routine: OaFortReadSubTable

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*(PTR_SIZ)  FUNCTION OaFortReadSubTable( TABLE_NODE, START_ROW, 
                                                     STOP_ROW,
                                                     SUBOBJECT_NODES, 
                                                     N_SUBOBJECT_NODES)
     INTEGER*(PTR_SIZ)   TABLE_NODE
     INTEGER*(LONG_SIZ)  START_ROW
     INTEGER*(LONG_SIZ)  STOP_ROW
     INTEGER*(PTR_SIZ)   SUBOBJECT_NODES(*)
     INTEGER*4           N_SUBOBJECT_NODES

  Note: PTR_SIZ and LONG_SIZ are 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
PTR OaFortReadSubTable( PTR *table_node, long *start_row, long *stop_row,
                        PTR subobject_nodes[], int *n_subobject_nodes)
{
#else
#ifdef _NO_PROTO
PTR oafortreadsubtable_( table_node, start_row, stop_row, subobject_nodes,
                         n_subobject_nodes)
PTR *table_node;
long *start_row;
long *stop_row;
PTR subobject_nodes[];
int *n_subobject_nodes;
{
#else
PTR oafortreadsubtable_( PTR *table_node, long *start_row, long *stop_row,
                         PTR subobject_nodes[], int *n_subobject_nodes)
{
#endif
#endif

return( (PTR) OaReadSubTable( (ODLTREE) *table_node, *start_row, *stop_row,
                              (ODLTREE *) subobject_nodes,
                              *n_subobject_nodes));
}


/*****************************************************************************

  Routine: OaFortReadTable

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*(PTR_SIZ)  FUNCTION OaFortReadTable( INPUT_NODE)
     INTEGER*(PTR_SIZ)  INPUT_NODE

  Note: PTR_SIZ is 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
PTR OaFortReadTable( PTR *input_node)
{
#else
#ifdef _NO_PROTO
PTR oafortreadtable_( input_node)
PTR *input_node;
{
#else
PTR oafortreadtable_( PTR *input_node)
{
#endif
#endif

return( (PTR) OaReadTable( (ODLTREE) *input_node));
}


/*****************************************************************************

  Routine: OaFortReportFileAttributes

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*4  FUNCTION OaFortReportFileAttributes( LABEL_FILENAME,
                                                     DATA_FILENAME,  
                                                     RECORD_TYPE,
                                                     RECORD_BYTES,
                                                     FILE_OFFSET,
                                                     OBJECT_INTERCHANGE_FORMAT)
   CHARACTER*(*)      LABEL_FILENAME
   CHARACTER*(*)      DATA_FILENAME 
   INTEGER*4          RECORD_TYPE
   INTEGER*(PTR_SIZ)  RECORD_BYTES
   INTEGER*(PTR_SIZ)  FILE_OFFSET
   INTEGER*4          OBJECT_INTERCHANGE_FORMAT

  Note: PTR_SIZ is 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
int OaFortReportFileAttributes( struct dsc$descriptor_s *label_filename_dsc,
                                struct dsc$descriptor_s *data_filename_dsc, 
                                int *record_type, long *record_bytes, 
                                long *file_offset, 
                                int *object_interchange_format)
{
char *label_filename_ptr = label_filename_dsc->dsc$a_pointer;
long label_filename_len = label_filename_dsc->dsc$w_length;
char *data_filename_ptr = data_filename_dsc->dsc$a_pointer;
long data_filename_len = data_filename_dsc->dsc$w_length;
#else
#ifdef _NO_PROTO
int oafortreportfileattributes_( label_filename_ptr, data_filename_ptr,
                                 record_type, record_bytes, 
                                 file_offset, object_interchange_format,
                                 label_filename_len, data_filename_len)
char *label_filename_ptr;
char *data_filename_ptr;
int *record_type;
long *record_bytes;
long *file_offset;
int *object_interchange_format;
long label_filename_len;
long data_filename_len;
{
#else
int oafortreportfileattributes_( char *label_filename_ptr, 
                                 char *data_filename_ptr, 
                                 int *record_type, long *record_bytes, 
                                 long *file_offset, 
                                 int *object_interchange_format,
                                 long label_filename_len, 
                                 long data_filename_len)
{
#endif
#endif

char *label_filename;
char *data_filename;
int result;

FortrantoCString( label_filename, label_filename_ptr, label_filename_len);
FortrantoCString( data_filename, data_filename_ptr, data_filename_len);
result = OaReportFileAttributes( label_filename, data_filename, *record_type,
                                 *record_bytes, *file_offset,
                                 *object_interchange_format);
return( result);
}


/*****************************************************************************

  Routine: OaFortReportError

  Description:  This routine is equivalent to a FORTRAN subroutine declared as
                follows:

     subroutine OaFortReportError( INPUT_ERROR_STRING)
     CHARACTER*(*)  INPUT_ERROR_STRING

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
int OaFortReportError( struct dsc$descriptor_s *input_error_string_dsc)
{
char *input_error_string_ptr = input_error_string_dsc->dsc$a_pointer;
long input_error_string_len = input_error_string_dsc->dsc$w_length;
#else
#ifdef _NO_PROTO
int oafortreporterror_( input_error_string_ptr, input_error_string_len)
char *input_error_string_ptr;
long input_error_string_len;
{
#else
int oafortreporterror_( char *input_error_string_ptr,
                        long input_error_string_len)
{
#endif
#endif

char *input_error_string;

FortrantoCString( input_error_string, input_error_string_ptr, 
                  input_error_string_len);
OaReportError( input_error_string);
LemmeGo( input_error_string);
return(0);
}


/*****************************************************************************

  Routine: OaFortSetProfileValues

  Description:  This routine is equivalent to a FORTRAN subroutine declared as
                follows:

     subroutine OaFortSetProfileValues( DST_FORMAT_FOR_ASCII_SRC,
                                        DST_FORMAT_FOR_BINARY_SRC,
                                        DST_ALIGNMENT_TYPE,
                                        DATA_TRANSLATION_PROFILE,
                                        CHECK_ASCII_WRITES)
     byte DST_FORMAT_FOR_ASCII_SRC
     byte DST_FORMAT_FOR_BINARY_SRC
     byte DST_ALIGNMENT_TYPE
     byte DATA_TRANSLATION_PROFILE
     byte CHECK_ASCII_WRITES

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
void OaFortSetProfileValues( char *dst_format_for_ASCII_src,
                             char *dst_format_for_binary_src,
                             char *dst_alignment_type,
                             char *data_translation_profile,
                             char *check_ASCII_writes)
{
#else
#ifdef _NO_PROTO
void oafortsetprofilevalues_( dst_format_for_ASCII_src, 
                              dst_format_for_binary_src,
                              dst_alignment_type, data_translation_profile,
                              check_ASCII_writes)
char *dst_format_for_ASCII_src;
char *dst_format_for_binary_src;
char *dst_alignment_type;
char *data_translation_profile;
char *check_ASCII_writes;
{
#else
void oafortsetprofilevalues_( char *dst_format_for_ASCII_src,
                             char *dst_format_for_binary_src,
                             char *dst_alignment_type,
                             char *data_translation_profile,
                             char *check_ASCII_writes)
{
#endif
#endif

Oa_profile.dst_format_for_ASCII_src  = (char) *dst_format_for_ASCII_src;
Oa_profile.dst_format_for_binary_src = (char) *dst_format_for_binary_src;
Oa_profile.dst_alignment_type        = (char) *dst_alignment_type;
Oa_profile.data_translation_profile  = (char) *data_translation_profile;
Oa_profile.check_ASCII_writes        = (char) *check_ASCII_writes;
}


/*****************************************************************************

  Routine: OaFortTransposeTable

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*(PTR_SIZ)  FUNCTION OaFortTransposeTable( OA_OBJECT)
     INTEGER*(PTR_SIZ)  OA_OBJECT

  Note: PTR_SIZ is 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
PTR OaFortTransposeTable( PTR *oa_object)
{
#else
#ifdef _NO_PROTO
PTR oaforttransposetable_( oa_object)
PTR *oa_object;
{
#else
PTR oaforttransposetable_( PTR *oa_object)
{
#endif
#endif

return( (PTR) OaTransposeTable( (OA_OBJECT) *oa_object));
}


/*****************************************************************************

  Routine: OaFortWriteObject

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*4  FUNCTION OaFortWriteObject( FILE_OBJECT, OBJECT)
     INTEGER*(PTR_SIZ)  FILE_OBJECT
     INTEGER*(PTR_SIZ)  OBJECT

  Note: PTR_SIZ is 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
int OaFortWriteObject( PTR *file_object, PTR *object)
{
#else
#ifdef _NO_PROTO
int oafortwriteobject_( file_object, object)
PTR *file_object;
PTR *object;
{
#else
int oafortwriteobject_( PTR *file_object, PTR *object)
{
#endif
#endif

return( OaWriteObject( (OA_OBJECT) *file_object, (OA_OBJECT) *object));
}


/*****************************************************************************

  Routine: OaFortCopyTree

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*(PTR_SIZ)  FUNCTION OaFortCopyTree( INPUT_NODE, OPTIONS)
     INTEGER*(PTR_SIZ)  INPUT_NODE
     INTEGER*4          OPTIONS

  Note: PTR_SIZ is 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
PTR OaFortCopyTree( PTR *input_node, int *options)
{
#else
#ifdef _NO_PROTO
PTR oafortcopytree_( input_node, options)
PTR *input_node;
int *options;
{
#else
PTR oafortcopytree_( PTR *input_node, int *options)
{
#endif
#endif

return( (PTR) OaCopyTree( (ODLTREE) *input_node, *options));
}


/*****************************************************************************

  Routine: OaFortGetObjectClass

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*4  FUNCTION OaFortGetObjectClass( ODLTREENODE) 
     INTEGER*(PTR_SIZ)  ODLTREENODE

  Note: PTR_SIZ is 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
int OaFortGetObjectClass( PTR *odltreenode)
{
#else
#ifdef _NO_PROTO
int oafortgetobjectclass_( odltreenode)
PTR *odltreenode;
{
#else
int oafortgetobjectclass_( PTR *odltreenode)
{
#endif
#endif

return( OaGetObjectClass( (ODLTREE) *odltreenode));
}


/*****************************************************************************

  Routine: OaFortGetObjectInterchangeFmt

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*4  OaFortGetObjectInterchangeFmt( TLO_OBJECT_NODE)
     INTEGER*(PTR_SIZ)  TLO_OBJECT_NODE

  Note: PTR_SIZ is 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
int OaFortGetObjectInterchangeFmt( PTR *TLO_object_node)
{
#else
#ifdef _NO_PROTO
int oafortgetobjectinterchangefmt_( TLO_object_node)
PTR *TLO_object_node;
{
#else
int oafortgetobjectinterchangefmt_( PTR *TLO_object_node)
{
#endif
#endif

return( OaGetObjectInterchangeFormat( (ODLTREE) *TLO_object_node));
}


/*****************************************************************************

  Routine: OaFortGetTableStorageType

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*4  FUNCTION OaFortGetTableStorageType( TABLE_NODE)
     INTEGER*(PTR_SIZ)  TABLE_NODE

  Note: PTR_SIZ is 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
int OaFortGetTableStorageType( PTR *table_node)
{
#else
#ifdef _NO_PROTO
int oafortgettablestoragetype_( table_node)
PTR *table_node;
{
#else
int oafortgettablestoragetype_( PTR *table_node)
{
#endif
#endif

return( OaGetTableStorageType( (ODLTREE) *table_node));
}


/*****************************************************************************

  Routine: OaFortKwdValuetoLong

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*4  FUNCTION OaFortKwdValuetoLong( KWD_NAME, ODLTREENODE, VALUE)
     CHARACTER*(*)       KWD_NAME
     INTEGER*(PTR_SIZ)   ODLTREENODE
     INTEGER*(LONG_SIZ)  VALUE

  Note: PTR_SIZ and LONG_SIZ are 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
int OaFortKwdValuetoLong( struct dsc$descriptor_s *kwd_name_dsc,
                          PTR *odltreenode,
                          long *value)
{
char *kwd_name_ptr = kwd_name_dsc->dsc$a_pointer;
long kwd_name_len = kwd_name_dsc->dsc$w_length;
#else
#ifdef _NO_PROTO
int oafortkwdvaluetolong_( kwd_name_ptr, odltreenode, value,
                           kwd_name_len)
char *kwd_name_ptr;
PTR *odltreenode;
long *value;
long kwd_name_len;
{
#else
int oafortkwdvaluetolong_( char *kwd_name_ptr, PTR *odltreenode, 
                           long *value, long kwd_name_len)
{
#endif
#endif

char *kwd_name;
int result;

FortrantoCString( kwd_name, kwd_name_ptr, kwd_name_len);
result = OaKwdValuetoLong( kwd_name, (ODLTREE) *odltreenode, value);
LemmeGo( kwd_name);
return( result);
}                           


/*****************************************************************************

  Routine: OaFortKwdValuetoStr

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*4  FUNCTION OaFortKwdValuetoStr( KWD_NAME, ODLTREENODE, VALUE)
     CHARACTER*(*)      KWD_NAME
     INTEGER*(PTR_SIZ)  ODLTREENODE
     CHARACTER*(*)      VALUE

  Note: PTR_SIZ is 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
int OaFortKwdValuetoStr( struct dsc$descriptor_s *kwd_name_dsc, 
                         PTR *odltreenode, 
                         struct dsc$descriptor_s *value_dsc)
{
char *kwd_name_ptr = kwd_name_dsc->dsc$a_pointer;
long kwd_name_len = kwd_name_dsc->dsc$w_length;
char *value_ptr = value_dsc->dsc$a_pointer;
long value_len = value_dsc->dsc$w_length;
#else
#ifdef _NO_PROTO
int oafortkwdvaluetostr_( kwd_name_ptr, odltreenode, value_ptr,
                          kwd_name_len, value_len)
char *kwd_name_ptr;
PTR *odltreenode;
char *value_ptr;
long kwd_name_len;
long value_len;
{
#else
int oafortkwdvaluetostr_( char *kwd_name_ptr, PTR *odltreenode, 
                          char *value_ptr, long kwd_name_len, 
                          long value_len)
{
#endif
#endif

char *kwd_name;
char *value;
int result;

FortrantoCString( kwd_name, kwd_name_ptr, kwd_name_len);
result = OaKwdValuetoStr( kwd_name, (ODLTREE) *odltreenode, &value);
CtoFortranString( value_ptr, value, value_len);
LemmeGo( kwd_name);
return( result);
}


/*****************************************************************************

  Routine: OaFortLongtoKwdValue

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

     INTEGER*4  FUNCTION OaFortLongtoKwdValue( KWD_NAME, ODLTREENODE, 
                                               KWD_VALUE)
     CHARACTER*(*)       KWD_NAME
     INTEGER*(PTR_SIZ)   ODLTREENODE
     INTEGER*(LONG_SIZ)  KWD_VALUE

  Note: PTR_SIZ and LONG_SIZ are 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
int OaFortLongtoKwdValue( struct dsc$descriptor_s *kwd_name_dsc,  
                          PTR *odltreenode, long *kwd_value)
{
char *kwd_name_ptr = kwd_name_dsc->dsc$a_pointer;
long kwd_name_len = kwd_name_dsc->dsc$w_length;
#else
#ifdef _NO_PROTO
int oafortlongtokwdvalue_( kwd_name_ptr, odltreenode, kwd_value, kwd_name_len)
char *kwd_name_ptr;
PTR *odltreenode;
long *kwd_value;
long kwd_name_len;
{
#else
int oafortlongtokwdvalue_( char *kwd_name_ptr, PTR *odltreenode, 
                           long *kwd_value, long kwd_name_len)
{
#endif
#endif

char *kwd_name;
int result;

FortrantoCString( kwd_name, kwd_name_ptr, kwd_name_len);
result = OaLongtoKwdValue( kwd_name, (ODLTREE) *odltreenode, *kwd_value);
LemmeGo( kwd_name);
return( result);
}


/*****************************************************************************

  Routine: OaFortObjectClasstoStr

  Description:  This routine is equivalent to a FORTRAN subroutine declared as
                follows:

  subroutine OaFortObjectClasstoStr( OA_OBJECT_CLASS, STRING)
  INTEGER*4      OA_OBJECT_CLASS
  CHARACTER*(*)  STRING

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
void OaFortObjectClasstoStr( int *oa_object_class,
                             struct dsc$descriptor_s *string_dsc)
{
char *string_ptr = string_dsc->dsc$a_pointer;
long string_len = string_dsc->dsc$w_length;
#else
#ifdef _NO_PROTO
void oafortobjectclasstostr_( oa_object_class, string_ptr, string_len)
int *oa_object_class;
char *string_ptr;
long string_len;
{
#else
void oafortobjectclasstostr_( int *oa_object_class, char *string_ptr,
                              long string_len)
{
#endif
#endif

char *string;

string = OaObjectClasstoStr( *oa_object_class);
CtoFortranString( string_ptr, string, string_len);
}


/*****************************************************************************

  Routine: OaFortPDSDataTypetoStr

  Description:  This routine is equivalent to a FORTRAN subroutine declared as
                follows:

  subroutine  OaFortPDSDataTypetoStr( PDS_DATA_TYPE, STRING)
  INTEGER*4      PDS_DATA_TYPE
  CHARACTER*(*)  STRING

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
void OaFortPDSDataTypetoStr( int *PDS_data_type,
                             struct dsc$descriptor_s *string_dsc)
{
char *string_ptr = string_dsc->dsc$a_pointer;
long string_len = string_dsc->dsc$w_length;
#else
#ifdef _NO_PROTO
void oafortpdsdatatypetostr_( PDS_data_type, string_ptr, string_len)
int *PDS_data_type;
char *string_ptr;
long string_len;
{
#else
void oafortpdsdatatypetostr_( int *PDS_data_type, char *string_ptr,
                              long string_len)
{
#endif
#endif

char *string;

string = OaPDSDataTypetoStr( *PDS_data_type);
CtoFortranString( string_ptr, string, string_len);
}


/*****************************************************************************

  Routine: OaFortStrtoKwdValue

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

  INTEGER*4  FUNCTION OaFortStrtoKwdValue( KWD_NAME, ODLTREENODE, STR)
  CHARACTER*(*)      KWD_NAME
  INTEGER*(PTR_SIZ)  ODLTREENODE
  CHARACTER*(*)      STR

  Note: PTR_SIZ is 4 on most platforms, 8 on Dec/Alpha.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
int OaFortStrtoKwdValue( struct dsc$descriptor_s *kwd_name_dsc, 
                         PTR *odltreenode, 
                         struct dsc$descriptor_s *str_dsc)
{
char *kwd_name_ptr = kwd_name_dsc->dsc$a_pointer;
long kwd_name_len = kwd_name_dsc->dsc$w_length;
char *str_ptr = str_dsc->dsc$a_pointer;
long str_len = str_dsc->dsc$w_length;
#else
#ifdef _NO_PROTO
int oafortstrtokwdvalue_( kwd_name_ptr, odltreenode, str_ptr,
                          kwd_name_len, str_len)
char *kwd_name_ptr;
PTR *odltreenode;
char *str_ptr;
long kwd_name_len;
long str_len;
{
#else
int oafortstrtokwdvalue_( char *kwd_name_ptr, PTR *odltreenode, char *str_ptr,
                          long kwd_name_len, long str_len)
{
#endif
#endif

char *kwd_name;
char *str;
int result;

FortrantoCString( kwd_name, kwd_name_ptr, kwd_name_len);
FortrantoCString( str, str_ptr, str_len);
result = OaStrtoKwdValue( kwd_name, (ODLTREE) *odltreenode, str);
LemmeGo( kwd_name);
LemmeGo( str);
return( result);
}


/*****************************************************************************

  Routine: OaFortStrtoPDSDataType

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

  INTEGER*4  FUNCTION OaFortStrtoPDSDataType( STR, INTERCHANGE_FORMAT)
  CHARACTER*(*)  STR
  INTEGER*4      INTERCHANGE_FORMAT

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan   1995
  Last Modified:  26 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:   

  Output:

  Notes:

*****************************************************************************/

#if (defined( VAX) || defined( ALPHA_VMS))
int OaFortStrtoPDSDataType( struct dsc$descriptor_s *str_dsc, 
                            int *interchange_format)
{
char *str_ptr = str_dsc->dsc$a_pointer;
long str_len = str_dsc->dsc$w_length;
#else
#ifdef _NO_PROTO
int oafortstrtopdsdatatype_( str_ptr, interchange_format, str_len)
char *str_ptr;
int *interchange_format;
long str_len;
{
#else
int oafortstrtopdsdatatype_( char *str_ptr, int *interchange_format,
                             long str_len)
{
#endif
#endif

char *str;
int result;

FortrantoCString( str, str_ptr, str_len);
result = OaStrtoPDSDataType( str, *interchange_format);
LemmeGo( str);
return( result);
}
