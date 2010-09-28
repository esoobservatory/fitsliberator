/*****************************************************************************

  File:  L3_fortran_interface.c

  Description: This file contains wrappers which are directly callable from
               Fortran for most of the routines in lablib3, plus a few
               utilities for moving around in ODL trees.  For complete
               documentation about L3 routines see lablib3.h.
               The routines in this file are:

               OdlFortLeftSibling
               OdlFortRightSibling
               OdlFortLeftmostChild
               OdlFortRightmostChild
               OdlFortParent
               OdlFortLemmeGo

               OdlFortCopyKwd
               OdlFortCopyObjDesc
               OdlFortCutKwd
               OdlFortCutObjDesc
               OdlFortExpandLabelFile
               OdlFortFindKwd
               OdlFortFindObjDesc
               OdlFortFreeAllKwds
               OdlFortFreeKwd
               OdlFortFreeTree
               OdlFortGetFileName
               OdlFortGetFirstKwd
               OdlFortGetKwdName
               OdlFortGetKwdUnit
               OdlFortGetKwdValue
               OdlFortGetKwdValueType
               OdlFortGetLabelVersion
               OdlFortGetNextKwd
               OdlFortGetObjDescChildCount
               OdlFortGetObjDescClassName
               OdlFortNewKwd
               OdlFortNewObjDesc
               OdlFortNextKwd
               OdlFortNextObjDesc
               OdlFortParseLabelFile
               OdlFortPasteKwd
               OdlFortPasteKwdBefore
               OdlFortPasteKwdAfter
               OdlFortPasteObjDesc
               OdlFortPasteObjDescAfter
               OdlFortPasteObjDescBefore
               OdlFortPrintLabel

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Jan  1995
  Last Modified:  26 Jan  1995

  History:

    Creation - These routines were part of the Beta Release of 
               the OA library.

  Notes:
  1) By default Fortran always passes function arguments by reference 
     (except for strings, explained below).  To get the actual value of
     scalars, the C code dereferences the pointer passed in from the Fortran
     caller.  A few OAL functions expect a pointer to data (the "Import"
     functions) or an array (the "Get/Read SubTable" functions) - since
     arrays are passed by reference, the C wrapper passes the pointer
     unchanged to the OAL function.

     Strings are passed from Fortran into C differently on VMS and UNIX:
     VMS:  A pointer to a VMS string descriptor (a C structure) is passed in;
           the structure contains a pointer to the beginning of the string, 
           and the length of the string.
     UNIX: A pointer to the beginning of the string is passed in, and an
           additional long parameter containing the string's length is
           appended to the end of the argument list.  Thus the C routine
           is declared with one more argument than the FORTRAN call has.

     Under both VMS and UNIX, the FORTRAN input string has blanks filled out
     to its length, and the string is not NULL terminated.  The code in these
     wrappers copies the passed-in string to a NULL-terminated C string, 
     setting its length as the number of characters up to the last non-blank
     character of the input string, which is usually less than the FORTRAN
     length.

  2) Function return values are returned by value as in C, except for strings,
     which are handled differently for VMS and UNIX.
     VMS:  The first argument of the C function is declared to be a pointer
           to a VMS string descriptor structure.  

     UNIX: The first two arguments of the C function are declared to be the
           string pointer and the string length.

     Under both VMS and UNIX, the code copies the C string to be returned to
     the input string pointer, and pads it with blanks out to the FORTRAN 
     string length, which was also passed in.  Neither the FORTRAN string 
     pointer nor the FORTRAN string length are ever changed.
           
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lablib3.h"
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

typedef OBJDESC *ODLTREE;
          
typedef long PTR;  /* C pointers are passed to and from Fortran as long's,  */
                   /* since a long is the same size as a pointer on all the */
                   /* platforms this code was developed for.                */

/*****************************************************************************

  Routine: OdlFortLeftSibling

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

        INTEGER*4  FUNCTION OdlFortLeftSibling( ODLTREENODE)
        INTEGER*4  ODLTREENODE

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
PTR OdlFortLeftSibling( PTR *odltreenode_ptr)
{
#else
#ifdef _NO_PROTO
PTR odlfortleftsibling_( odltreenode_ptr)
PTR *odltreenode_ptr;
{
#else
PTR odlfortleftsibling_( PTR *odltreenode_ptr)
{
#endif
#endif

ODLTREE odltreenode = (ODLTREE) *odltreenode_ptr;
odltreenode = odltreenode->left_sibling;
return( (PTR) odltreenode);
}


/*****************************************************************************

  Routine: OdlFortRightSibling

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

        INTEGER*4  FUNCTION OdlFortRightSibling( ODLTREENODE)
        INTEGER*4  ODLTREENODE

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
PTR OdlFortRightSibling( PTR *odltreenode_ptr)
{
#else
#ifdef _NO_PROTO
PTR odlfortrightsibling_( odltreenode_ptr)
PTR *odltreenode_ptr;
{
#else
PTR odlfortrightsibling_( PTR *odltreenode_ptr)
{
#endif
#endif

ODLTREE odltreenode = (ODLTREE) *odltreenode_ptr;
odltreenode = odltreenode->right_sibling;
return( (PTR) odltreenode);
}


/*****************************************************************************

  Routine: OdlFortLeftmostChild

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

        INTEGER*4  FUNCTION OdlFortLeftmostChild( ODLTREENODE)
        INTEGER*4  ODLTREENODE

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
PTR OdlFortLeftmostChild( PTR *odltreenode_ptr)
{
#else
#ifdef _NO_PROTO
PTR odlfortleftmostchild_( odltreenode_ptr)
PTR *odltreenode_ptr;
{
#else
PTR odlfortleftmostchild_( PTR *odltreenode_ptr)
{
#endif
#endif

ODLTREE odltreenode = (ODLTREE) *odltreenode_ptr;
odltreenode = odltreenode->first_child;
return( (PTR) odltreenode);
}


/*****************************************************************************

  Routine: OdlFortRightmostChild

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

        INTEGER*4  FUNCTION OdlFortRightmostChild( ODLTREENODE)
        INTEGER*4  ODLTREENODE

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
PTR OdlFortRightmostChild( PTR *odltreenode_ptr)
{
#else
#ifdef _NO_PROTO
PTR odlfortrightmostchild_( odltreenode_ptr)
PTR *odltreenode_ptr;
{
#else
PTR odlfortrightmostchild_( PTR *odltreenode_ptr)
{
#endif
#endif

ODLTREE odltreenode = (ODLTREE) *odltreenode_ptr;
odltreenode = odltreenode->last_child;
return( (PTR) odltreenode);
}


/*****************************************************************************

  Routine: OdlFortParent

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

        INTEGER*4  FUNCTION OdlFortParent( ODLTREENODE)
        INTEGER*4  ODLTREENODE

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
PTR OdlFortParent( PTR *odltreenode_ptr)
{
#else
#ifdef _NO_PROTO
PTR odlfortparent_( odltreenode_ptr)
PTR *odltreenode_ptr;
{
#else
PTR odlfortparent_( PTR *odltreenode_ptr)
{
#endif
#endif

ODLTREE odltreenode = (ODLTREE) *odltreenode_ptr;
odltreenode = odltreenode->parent;
return( (PTR) odltreenode);
}


/*****************************************************************************

  Routine: OdlFortLemmeGo

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

        INTEGER*4  FUNCTION OdlFortLemmeGo( MY_POINTER)
        INTEGER*4  MY_POINTER

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
PTR OdlFortLemmeGo( PTR *my_pointer)
{
#else
#ifdef _NO_PROTO
PTR odlfortlemmego_( my_pointer)
PTR *my_pointer;
{
#else
PTR odlfortlemmego_( PTR *my_pointer)
{
#endif
#endif

char *ptr = (char *) *my_pointer;
LemmeGo( ptr);
return(0);
}


/*****************************************************************************

  Routine: OdlFortCopyKwd

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

        INTEGER*4  FUNCTION OdlFortCopyKwd (KEYWORD)
        INTEGER*4  KEYWORD

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
PTR OdlFortCopyKwd (PTR *keyword)
{
#else
#ifdef _NO_PROTO
PTR odlfortcopykwd_ (PTR *keyword)
{
#else
PTR odlfortcopykwd_ (PTR *keyword)
{
#endif
#endif

return( (PTR) OdlCopyKwd( (KEYWORD *) *keyword));
}

     
/*****************************************************************************

  Routine: OdlFortCopyObjDesc

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

        INTEGER*4  FUNCTION OdlFortCopyObjDesc (OBJECT)
        INTEGER*4  OBJECT

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
PTR OdlFortCopyObjDesc (PTR *object)
{
#else
#ifdef _NO_PROTO
PTR odlfortcopyobjdesc_ (PTR *object)
{
#else
PTR odlfortcopyobjdesc_ (PTR *object)
{
#endif
#endif

return( (PTR) OdlCopyObjDesc( (ODLTREE) *object));
}


/*****************************************************************************

  Routine: OdlFortCutKwd

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

        INTEGER*4  FUNCTION OdlFortCutKwd (KEYWORD)
        INTEGER*4  KEYWORD

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
PTR OdlFortCutKwd (PTR *keyword)
{
#else
#ifdef _NO_PROTO
PTR odlfortcutkwd_ (PTR *keyword)
{
#else
PTR odlfortcutkwd_ (PTR *keyword)
{
#endif
#endif

return( (PTR) OdlCutKwd( (KEYWORD *) *keyword));
}


/*****************************************************************************

  Routine: OdlFortCutObjDesc

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

        INTEGER*4  FUNCTION OdlFortCutObjDesc (OBJECT)
        INTEGER*4  OBJECT

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
PTR OdlFortCutObjDesc (PTR *object)
{
#else
#ifdef _NO_PROTO
PTR odlfortcutobjdesc_ (PTR *object)
{
#else
PTR odlfortcutobjdesc_ (PTR *object)
{
#endif
#endif

return( (PTR) OdlCutObjDesc( (ODLTREE) *object));
}


/*****************************************************************************

  Routine: OdlFortExpandLabelFile

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

        INTEGER*4  FUNCTION OdlFortExpandLabelFile (OBJECT, MESSAGE_FNAME, 
                                                    EXPAND, SUPPRESS_MESSAGES)
        INTEGER*4      OBJECT
        CHARACTER*(*)  MESSAGE_FNAME
        INTEGER*2      EXPAND
        INTEGER*2      SUPPRESS_MESSAGES

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
PTR OdlFortExpandLabelFile (PTR *object,  
                            struct dsc$descriptor_s *message_fname_dsc, 
                            MASK *expand_ptr, 
                            unsigned short *suppress_messages_ptr)
{
char *message_fname_ptr = message_fname_dsc->dsc$a_pointer;
long message_fname_len = message_fname_dsc->dsc$w_length;

#else
#ifdef _NO_PROTO
PTR odlfortexpandlabelfile_ (object, message_fname_ptr, expand_ptr, 
                              suppress_messages_ptr, message_fname_len)
PTR *object;
char *message_fname_ptr;
MASK *expand_ptr;
unsigned short *suppress_messages_ptr;
long message_fname_len;
{
#else
PTR odlfortexpandlabelfile_ (PTR *object,
                             char *message_fname_ptr, 
                             MASK *expand_ptr, 
                             unsigned short *suppress_messages_ptr,
                             long message_fname_len)
{
#endif
#endif

char *message_fname;
PTR result;

FortrantoCString( message_fname, message_fname_ptr, message_fname_len);
result = (PTR) OdlExpandLabelFile( (ODLTREE) *object, message_fname,
                                   *expand_ptr, *suppress_messages_ptr);
LemmeGo( message_fname);
return( result);
}


/*****************************************************************************

  Routine: OdlFortFindKwd

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

        INTEGER*4  FUNCTION OdlFortFindKwd (START_OBJECT, KEYWORD_NAME,
                                            KEYWORD_VALUE, KEYWORD_POSITION,
                                            SEARCH_SCOPE)
        INTEGER*4      START_OBJECT
        CHARACTER*(*)  KEYWORD_NAME
        CHARACTER*(*)  KEYWORD_VALUE
        INTEGER*4      KEYWORD_POSITION
        INTEGER*2      SEARCH_SCOPE

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
PTR OdlFortFindKwd (PTR *start_object, 
                    struct dsc$descriptor_s *keyword_name_dsc,
                    struct dsc$descriptor_s *keyword_value_dsc,
                    unsigned long *keyword_position_ptr,
                    unsigned short *search_scope_ptr)
{
char *keyword_name_ptr = keyword_name_dsc->dsc$a_pointer;
char *keyword_value_ptr = keyword_value_dsc->dsc$a_pointer;
long keyword_name_len = keyword_name_dsc->dsc$w_length;
long keyword_value_len = keyword_value_dsc->dsc$w_length;

#else
#ifdef _NO_PROTO
PTR odlfortfindkwd_ (start_object, keyword_name_ptr, keyword_value_ptr, 
                     keyword_position_ptr, search_scope_ptr, keyword_name_len,
                     keyword_value_len)
PTR *start_object;
char *keyword_name_ptr;
char *keyword_value_ptr;
unsigned long *keyword_position_ptr;
unsigned short *search_scope_ptr;
long keyword_name_len;
long keyword_value_len;
{
#else
PTR odlfortfindkwd_ (PTR *start_object, char *keyword_name_ptr, 
                     char *keyword_value_ptr,  
                     unsigned long *keyword_position_ptr,
                     unsigned short *search_scope_ptr,
                     long keyword_name_len, long keyword_value_len)
{
#endif
#endif

char *keyword_name;
char *keyword_value;
PTR result;

FortrantoCString( keyword_name, keyword_name_ptr, keyword_name_len);
FortrantoCString( keyword_value, keyword_value_ptr, keyword_value_len);
result = (PTR) OdlFindKwd( (ODLTREE) *start_object, keyword_name, 
                           keyword_value, *keyword_position_ptr, 
                           *search_scope_ptr);
LemmeGo( keyword_name);
LemmeGo( keyword_value);
return( result);
}


/*****************************************************************************

  Routine: OdlFortFindObjDesc

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

        INTEGER*4  FUNCTION OdlFortFindObjDesc (START_OBJECT, OBJECT_CLASS, 
                                                KEYWORD_NAME, KEYWORD_VALUE,
                                                OBJECT_POSITION, SEARCH_SCOPE)
        INTEGER*4      START_OBJECT
        CHARACTER*(*)  OBJECT_CLASS
        CHARACTER*(*)  KEYWORD_NAME
        CHARACTER*(*)  KEYWORD_VALUE
        INTEGER*4      OBJECT_POSITION
        INTEGER*2      SEARCH_SCOPE

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
PTR OdlFortFindObjDesc (PTR *start_object, 
                        struct dsc$descriptor_s *object_class_dsc,
                        struct dsc$descriptor_s *keyword_name_dsc,
                        struct dsc$descriptor_s *keyword_value_dsc,
                        unsigned long *object_position_ptr,  
                        unsigned short *search_scope_ptr)
{
char *object_class_ptr = object_class_dsc->dsc$a_pointer;
char *keyword_name_ptr = keyword_name_dsc->dsc$a_pointer;
char *keyword_value_ptr = keyword_value_dsc->dsc$a_pointer;
long object_class_len = object_class_dsc->dsc$w_length;
long keyword_name_len = keyword_name_dsc->dsc$w_length;
long keyword_value_len = keyword_value_dsc->dsc$w_length;

#else
#ifdef _NO_PROTO
PTR odlfortfindobjdesc_ (start_object, object_class_ptr, keyword_name_ptr,
                         keyword_value_ptr, object_position_ptr,  
                         search_scope_ptr, object_class_len, keyword_name_len,
                         keyword_value_len)
PTR *start_object;
char *object_class_ptr;
char *keyword_name_ptr;
char *keyword_value_ptr;
unsigned long *object_position_ptr;
unsigned short *search_scope_ptr;
long object_class_len;
long keyword_name_len;
long keyword_value_len;
{
#else
PTR odlfortfindobjdesc_ (PTR *start_object, char *object_class_ptr,
                         char *keyword_name_ptr, char *keyword_value_ptr,
                         unsigned long *object_position_ptr,  
                         unsigned short *search_scope_ptr,
                         long object_class_len, long keyword_name_len,
                         long keyword_value_len)
{
#endif
#endif

char *object_class;
char *keyword_name;
char *keyword_value;
PTR result;

FortrantoCString( object_class, object_class_ptr, object_class_len);
FortrantoCString( keyword_name, keyword_name_ptr, keyword_name_len);
FortrantoCString( keyword_value, keyword_value_ptr, keyword_value_len);
result = (PTR) OdlFindObjDesc( (ODLTREE) *start_object, object_class,
                               keyword_name, keyword_value, 
                               *object_position_ptr, *search_scope_ptr);
LemmeGo( object_class);
LemmeGo( keyword_name);
LemmeGo( keyword_value);
return( result);
}


/*****************************************************************************

  Routine: OdlFortFreeAllKwds

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

        INTEGER*4  FUNCTION OdlFortFreeAllKwds (OBJECT)
        INTEGER*4  OBJECT

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
PTR OdlFortFreeAllKwds (PTR *object)
{
#else
#ifdef _NO_PROTO
PTR odlfortfreeallkwds_ ( object)
PTR *object;
{
#else
PTR odlfortfreeallkwds_ (PTR *object)
{
#endif
#endif

return( (PTR) OdlFreeAllKwds( (ODLTREE) *object));
}


/*****************************************************************************

  Routine: OdlFortFreeKwd

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

        INTEGER*4  FUNCTION OdlFortFreeKwd (KEYWORD)
        INTEGER*4  KEYWORD

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
PTR OdlFortFreeKwd (PTR *keyword)
{
#else
#ifdef _NO_PROTO
PTR odlfortfreekwd_ (keyword)
PTR *keyword;
{
#else
PTR odlfortfreekwd_ (PTR *keyword)
{
#endif
#endif

return( (PTR) OdlFreeKwd( (KEYWORD *) *keyword));
}


/*****************************************************************************

  Routine: OdlFortFreeTree

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

        INTEGER*4  FUNCTION OdlFortFreeTree (OBJECT)
        INTEGER*4  OBJECT

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
PTR OdlFortFreeTree (PTR *object)
{
#else
#ifdef _NO_PROTO
PTR odlfortfreetree_ (object)
PTR *object;
{
#else
PTR odlfortfreetree_ (PTR *object)
{
#endif
#endif

return( (PTR) OdlFreeTree( (ODLTREE) *object));
}


/*****************************************************************************

  Routine: OdlFortGetFileName

  Description:  This routine is equivalent to a FORTRAN subroutine declared as
                follows:

        subroutine OdlFortGetFileName (KEYWORD, START_LOCATION
                                       START_LOCATION_TYPE, FILENAME)
        INTEGER*4      KEYWORD
        INTEGER*4      START_LOCATION
        INTEGER*2      START_LOCATION_TYPE
        CHARACTER*(*)  FILENAME

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
void OdlFortGetFileName (PTR *keyword, unsigned long *start_location,  
                         unsigned short *start_location_type,
                         struct dsc$descriptor_s *filename_dsc)
{
char *filename_ptr = filename_dsc->dsc$a_pointer;
long filename_len = filename_dsc->dsc$w_length;
#else
#ifdef _NO_PROTO
void odlfortgetfilename_ (keyword, start_location, start_location_type,
                          filename_ptr, filename_len)
PTR *keyword;
unsigned long *start_location;
unsigned short *start_location_type;
char *filename_ptr;
long filename_len;
{
#else
void odlfortgetfilename_ (PTR *keyword, unsigned long *start_location,  
                          unsigned short *start_location_type,
                          char *filename_ptr, long filename_len)
{
#endif
#endif

char *filename;
filename = OdlGetFileName( (KEYWORD *) *keyword, start_location, 
                         start_location_type);
CtoFortranString( filename_ptr, filename, filename_len);
LemmeGo( filename);
}


/*****************************************************************************

  Routine: OdlFortGetFirstKwd

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

        INTEGER*4  FUNCTION OdlFortGetFirstKwd (OBJECT)
        INTEGER*4  OBJECT

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
PTR OdlFortGetFirstKwd (PTR *object)
{
#else
#ifdef _NO_PROTO
PTR odlfortgetfirstkwd_ (object)
PTR *object;
{
#else
PTR odlfortgetfirstkwd_ (PTR *object)
{
#endif
#endif

return( (PTR) OdlGetFirstKwd( (ODLTREE) *object));
}


/*****************************************************************************

  Routine: OdlFortGetKwdName

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

        subroutine OdlFortGetKwdName (KEYWORD, KEYWORD_NAME)
        INTEGER*4  KEYWORD
        INTEGER*4  KEYWORD_NAME

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
void OdlFortGetKwdName (PTR *keyword,
                       struct dsc$descriptor_s *keyword_name_dsc)
{
char *keyword_name_ptr = keyword_name_dsc->dsc$a_pointer;
long keyword_name_len = keyword_name_dsc->dsc$w_length;
#else
#ifdef _NO_PROTO
void odlfortgetkwdname_ (keyword, keyword_name_ptr, keyword_name_len)
PTR *keyword;
char *keyword_name_ptr;
long keyword_name_len;
{
#else
void odlfortgetkwdname_ (PTR *keyword, char *keyword_name_ptr, 
                         long keyword_name_len)
{
#endif
#endif

char *keyword_name;
keyword_name = OdlGetKwdName( (KEYWORD *) *keyword);
CtoFortranString( keyword_name_ptr, keyword_name, keyword_name_len);
LemmeGo( keyword_name);
}


/*****************************************************************************

  Routine: OdlFortGetKwdUnit

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

        subroutine  OdlFortGetKwdUnit (KEYWORD, KEYWORD_UNIT)
        INTEGER*4      KEYWORD
        CHARACTER*(*)  KEYWORD_UNIT

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
void OdlFortGetKwdUnit (PTR *keyword,
                        struct dsc$descriptor_s *keyword_unit_dsc)
{
char *keyword_unit_ptr = keyword_unit_dsc->dsc$a_pointer;
long keyword_unit_len = keyword_unit_dsc->dsc$w_length;
#else
#ifdef _NO_PROTO
void odlfortgetkwdunit_ (keyword, keyword_unit_ptr, keyword_unit_len)
PTR *keyword;
char *keyword_unit_ptr;
long keyword_unit_len;
{
#else
void odlfortgetkwdunit_ (PTR *keyword, char *keyword_unit_ptr, 
                         long keyword_unit_len)
{
#endif
#endif

char *keyword_unit;
keyword_unit = OdlGetKwdUnit( (KEYWORD *) *keyword);
CtoFortranString( keyword_unit_ptr, keyword_unit, keyword_unit_len);
LemmeGo( keyword_unit);
}


/*****************************************************************************

  Routine: OdlFortGetKwdValue

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:
 
        subroutine  OdlFortGetKwdValue (KEYWORD, KEYWORD_VALUE)
        INTEGER*4  KEYWORD
        INTEGER*4  KEYWORD_VALUE

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
void OdlFortGetKwdValue (PTR *keyword,
                         struct dsc$descriptor_s *keyword_value_dsc)
                         
{
char *keyword_value_ptr = keyword_value_dsc->dsc$a_pointer;
long keyword_value_len = keyword_value_dsc->dsc$w_length;
#else
#ifdef _NO_PROTO
void odlfortgetkwdvalue_ (keyword, keyword_value_ptr, keyword_value_len)
PTR *keyword;
char *keyword_value_ptr;
long keyword_value_len;
{
#else
void odlfortgetkwdvalue_ (PTR *keyword, char *keyword_value_ptr, 
                          long keyword_value_len)
{
#endif
#endif

char *keyword_value;
keyword_value = OdlGetKwdValue( (KEYWORD *) *keyword);
CtoFortranString( keyword_value_ptr, keyword_value, keyword_value_len);
}


/*****************************************************************************

  Routine: OdlFortGetKwdValueType

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

        INTEGER*2  FUNCTION OdlFortGetKwdValueType (KEYWORD)
        INTEGER*4  KEYWORD

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
unsigned short OdlFortGetKwdValueType (PTR *keyword)
{
#else
#ifdef _NO_PROTO
unsigned short odlfortgetkwdvaluetype_ (keyword)
PTR *keyword;
{
#else
unsigned short odlfortgetkwdvaluetype_ (PTR *keyword)
{
#endif
#endif

return( OdlGetKwdValueType( (KEYWORD *) *keyword));
}


/*****************************************************************************

  Routine: OdlFortGetLabelVersion

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

        subroutine  OdlFortGetLabelVersion (OBJECT, LABEL_VERSION
        INTEGER*4      OBJECT
        CHARACTER*(*)  LABEL_VERSION

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
void OdlFortGetLabelVersion (PTR *object,
                             struct dsc$descriptor_s *label_version_dsc)
{
char *label_version_ptr = label_version_dsc->dsc$a_pointer;
long label_version_len = label_version_dsc->dsc$w_length;
#else
#ifdef _NO_PROTO
void odlfortgetlabelversion_ (object, label_version_ptr, label_version_len)
PTR *object;
char *label_version_ptr;
long label_version_len;
{
#else
void odlfortgetlabelversion_ (PTR *object, char *label_version_ptr, 
                              long label_version_len)
{
#endif
#endif

char *label_version;
label_version = OdlGetLabelVersion( (ODLTREE) *object);
CtoFortranString( label_version_ptr, label_version, label_version_len);
LemmeGo( label_version);
}


/*****************************************************************************

  Routine: OdlFortGetNextKwd

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

        INTEGER*4  FUNCTION OdlFortGetNextKwd (KEYWORD)
        INTEGER*4  KEYWORD

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
PTR OdlFortGetNextKwd (PTR *keyword)
{
#else
#ifdef _NO_PROTO
PTR odlfortgetnextkwd_ (keyword)
PTR *keyword;
{
#else
PTR odlfortgetnextkwd_ (PTR *keyword)
{
#endif
#endif

return( (PTR) OdlGetNextKwd( (KEYWORD *) *keyword));
}


/*****************************************************************************

  Routine: OdlFortGetObjDescChildCount

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

        INTEGER*4  FUNCTION OdlFortGetObjDescChildCount (OBJECT)
        INTEGER*4  OBJECT

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
int OdlFortGetObjDescChildCount (PTR *object)
{
#else
#ifdef _NO_PROTO
int odlfortgetobjdescchildcount_ (object)
PTR *object;
{
#else
int odlfortgetobjdescchildcount_ (PTR *object)
{
#endif
#endif

return( OdlGetObjDescChildCount( (ODLTREE) *object));
}


/*****************************************************************************

  Routine: OdlFortGetObjDescClassName

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

        INTEGER*4  FUNCTION OdlFortGetObjDescClassName (OBJECT, CLASS_NAME)
        INTEGER*4  OBJECT
        CHARACTER*(*) CLASS_NAME

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
void OdlFortGetObjDescClassName (PTR *object,
                                 struct dsc$descriptor_s *class_name_dsc)
{
char *class_name_ptr = class_name_dsc->dsc$a_pointer;
long class_name_len = class_name_dsc->dsc$w_length;
#else
#ifdef _NO_PROTO
void odlfortgetobjdescclassname_ (object, class_name_ptr, class_name_len)
PTR *object;
char *class_name_ptr;
long class_name_len;
{
#else
void odlfortgetobjdescclassname_ (PTR *object,
                                  char *class_name_ptr, 
                                  long class_name_len)
{
#endif
#endif

char *result;
result = OdlGetObjDescClassName( (ODLTREE) *object);
CtoFortranString( class_name_ptr, result, class_name_len);
}


/*****************************************************************************

  Routine: OdlFortNewKwd

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

        INTEGER*4  FUNCTION OdlFortNewKwd (KEYWORD_NAME, VALUE_TEXT,
                                           PRE_COMMENT, LINE_COMMENT,
                                           FILE_NAME, LINE_NUMBER)
        CHARACTER*(*) KEYWORD_NAME
        CHARACTER*(*) VALUE_TEXT
        CHARACTER*(*) PRE_COMMENT
        CHARACTER*(*) LINE_COMMENT
        CHARACTER*(*) FILE_NAME
        INTEGER*4     LINE_NUMBER

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
PTR OdlFortNewKwd ( struct dsc$descriptor_s *keyword_name_dsc,
                    struct dsc$descriptor_s *value_text_dsc,
                    struct dsc$descriptor_s *pre_comment_dsc,
                    struct dsc$descriptor_s *line_comment_dsc,
                    struct dsc$descriptor_s *file_name_dsc,
                    int *line_number_ptr)
{
char *keyword_name_ptr = keyword_name_dsc->dsc$a_pointer;
char *value_text_ptr = value_text_dsc->dsc$a_pointer;
char *pre_comment_ptr = pre_comment_dsc->dsc$a_pointer;
char *line_comment_ptr = line_comment_dsc->dsc$a_pointer;
char *file_name_ptr = file_name_dsc->dsc$a_pointer;
long keyword_name_len = keyword_name_dsc->dsc$w_length;
long value_text_len = value_text_dsc->dsc$w_length;
long pre_comment_len = pre_comment_dsc->dsc$w_length;
long line_comment_len = line_comment_dsc->dsc$w_length;
long file_name_len = file_name_dsc->dsc$w_length;
#else
#ifdef _NO_PROTO
PTR odlfortnewkwd_ (keyword_name_ptr, value_text_ptr, pre_comment_ptr,  
                    line_comment_ptr, file_name_ptr, line_number_ptr, 
                    keyword_name_len,  value_text_len, pre_comment_len, 
                    line_comment_len, file_name_len)
char *keyword_name_ptr;
char *value_text_ptr;
char *pre_comment_ptr;
char *line_comment_ptr;
char *file_name_ptr;
int  *line_number_ptr;
long keyword_name_len;
long value_text_len;
long pre_comment_len;
long line_comment_len;
long file_name_len;
{
#else
PTR odlfortnewkwd_ (char *keyword_name_ptr, char *value_text_ptr, 
                    char *pre_comment_ptr, char *line_comment_ptr, 
                    char *file_name_ptr, int  *line_number_ptr,
                    long keyword_name_len, long value_text_len, 
                    long pre_comment_len, long line_comment_len,
                    long file_name_len)
{
#endif
#endif

char *keyword_name;
char *value_text;
char *pre_comment;
char *line_comment;
char *file_name;
PTR result;

FortrantoCString( keyword_name, keyword_name_ptr, keyword_name_len);
FortrantoCString( value_text, value_text_ptr, value_text_len);
FortrantoCString( pre_comment, pre_comment_ptr, pre_comment_len);
FortrantoCString( line_comment, line_comment_ptr, line_comment_len);
FortrantoCString( file_name, file_name_ptr, file_name_len);

result = (PTR) OdlNewKwd( keyword_name, value_text, pre_comment,
                          line_comment, file_name, *line_number_ptr);
LemmeGo( keyword_name);
LemmeGo( value_text);
LemmeGo( pre_comment);
LemmeGo( line_comment);
LemmeGo( file_name);
return( result);
}


/*****************************************************************************

  Routine: OdlFortNewObjDesc

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

        INTEGER*4  FUNCTION OdlFortNewObjDesc (OBJECT_CLASS, PRE_COMMENT, 
                                               LINE_COMMENT, POST_COMMENT,
                                               END_COMMENT, FILE_NAME, 
                                               IS_A_GROUP, LINE_NUMBER)
        CHARACTER*(*) OBJECT_CLASS
        CHARACTER*(*) PRE_COMMENT
        CHARACTER*(*) LINE_COMMENT
        CHARACTER*(*) POST_COMMENT
        CHARACTER*(*) END_COMMENT
        CHARACTER*(*) FILE_NAME
        INTEGER*2     IS_A_GROUP
        INTEGER*4     LINE_NUMBER

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
PTR OdlFortNewObjDesc ( struct dsc$descriptor_s *object_class_dsc,
                        struct dsc$descriptor_s *pre_comment_dsc,
                        struct dsc$descriptor_s *line_comment_dsc,
                        struct dsc$descriptor_s *post_comment_dsc,
                        struct dsc$descriptor_s *end_comment_dsc,
                        struct dsc$descriptor_s *file_name_dsc,
                        short *is_a_group_ptr,
                        long *line_number_ptr)
{
char *object_class_ptr = object_class_dsc->dsc$a_pointer;
char *pre_comment_ptr = pre_comment_dsc->dsc$a_pointer;
char *line_comment_ptr = line_comment_dsc->dsc$a_pointer;
char *post_comment_ptr = post_comment_dsc->dsc$a_pointer;
char *end_comment_ptr = end_comment_dsc->dsc$a_pointer;
char *file_name_ptr = file_name_dsc->dsc$a_pointer;
long object_class_len = object_class_dsc->dsc$w_length;
long pre_comment_len = pre_comment_dsc->dsc$w_length;
long line_comment_len = line_comment_dsc->dsc$w_length;
long post_comment_len = post_comment_dsc->dsc$w_length;
long end_comment_len = end_comment_dsc->dsc$w_length;
long file_name_len = file_name_dsc->dsc$w_length;

#else
#ifdef _NO_PROTO
PTR odlfortnewobjdesc_ (object_class_ptr, pre_comment_ptr, line_comment_ptr, 
                        post_comment_ptr,  end_comment_ptr, file_name_ptr, 
                        is_a_group_ptr, line_number_ptr, object_class_len,  
                        pre_comment_len, line_comment_len, post_comment_len, 
                        end_comment_len, file_name_len)
char *object_class_ptr;
char *pre_comment_ptr;
char *line_comment_ptr;
char *post_comment_ptr;
char *end_comment_ptr;
char *file_name_ptr;
short *is_a_group_ptr;
long *line_number_ptr;
long object_class_len;
long pre_comment_len;
long line_comment_len;
long post_comment_len;
long end_comment_len;
long file_name_len;
{
#else
PTR odlfortnewobjdesc_ (char *object_class_ptr, char *pre_comment_ptr, 
                        char *line_comment_ptr, char *post_comment_ptr, 
                        char *end_comment_ptr, char *file_name_ptr, 
                        short *is_a_group_ptr, long  *line_number_ptr,
                        long object_class_len, long pre_comment_len, 
                        long line_comment_len, long post_comment_len, 
                        long end_comment_len, long file_name_len)
{
#endif
#endif

char *object_class;
char *pre_comment;
char *line_comment;
char *post_comment;
char *end_comment;
char *file_name;
PTR result;

FortrantoCString( object_class, object_class_ptr, object_class_len);
FortrantoCString( pre_comment, pre_comment_ptr, pre_comment_len);
FortrantoCString( line_comment, line_comment_ptr, line_comment_len);
FortrantoCString( post_comment, post_comment_ptr, post_comment_len);
FortrantoCString( end_comment, end_comment_ptr, end_comment_len);
FortrantoCString( file_name, file_name_ptr, file_name_len);

result = (PTR) OdlNewObjDesc( object_class, pre_comment, line_comment, 
                              post_comment, end_comment, file_name,
                              *is_a_group_ptr, *line_number_ptr);
LemmeGo( object_class);
LemmeGo( pre_comment);
LemmeGo( line_comment);
LemmeGo( post_comment);
LemmeGo( end_comment);
LemmeGo( file_name);
return( result);
}


/*****************************************************************************

  Routine: OdlFortNextKwd

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

        INTEGER*4  FUNCTION OdlFortNextKwd (START_KEYWORD, KEYWORD_NAME,
                                            KEYWORD_VALUE, KEYWORD_POSITION,
                                            SEARCH_SCOPE)
        INTEGER*4     START_KEYWORD
        CHARACTER*(*) KEYWORD_NAME
        CHARACTER*(*) KEYWORD_VALUE
        INTEGER*4     KEYWORD_POSITION
        INTEGER*2     SEARCH_SCOPE

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
PTR OdlFortNextKwd ( PTR *start_keyword, 
                     struct dsc$descriptor_s *keyword_name_dsc,
                     struct dsc$descriptor_s *keyword_value_dsc,
                     unsigned long *keyword_position, 
                     unsigned short *search_scope_ptr)
{
char *keyword_name_ptr = keyword_name_dsc->dsc$a_pointer;
char *keyword_value_ptr = keyword_value_dsc->dsc$a_pointer;
long keyword_name_len = keyword_name_dsc->dsc$w_length;
long keyword_value_len = keyword_value_dsc->dsc$w_length;
#else
#ifdef _NO_PROTO
PTR odlfortnextkwd_ (start_keyword, keyword_name_ptr, keyword_value_ptr,
                     keyword_position, search_scope_ptr, 
                     keyword_name_len, keyword_value_len)
PTR *start_keyword;
char *keyword_name_ptr;
char *keyword_value_ptr;
unsigned long *keyword_position; 
unsigned short *search_scope_ptr;
{
#else
PTR odlfortnextkwd_ (PTR *start_keyword, char *keyword_name_ptr, 
                     char *keyword_value_ptr, unsigned long *keyword_position,
                     unsigned short *search_scope_ptr, long keyword_name_len,
                     long keyword_value_len)
{
#endif
#endif

char *keyword_name;
char *keyword_value;
PTR result;

FortrantoCString( keyword_name, keyword_name_ptr, keyword_name_len);
FortrantoCString( keyword_value, keyword_value_ptr, keyword_value_len);

result = (PTR) OdlNextKwd( (KEYWORD *) *start_keyword, keyword_name, 
                            keyword_value, *keyword_position, 
                            *search_scope_ptr);
LemmeGo( keyword_name);
LemmeGo( keyword_value);
return( result);
}


/*****************************************************************************

  Routine: OdlFortNextObjDesc

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

        INTEGER*4  FUNCTION OdlFortNextObjDesc (OBJECT, ROOT_LEVEL
                                                SEARCH_SCOPE)
        INTEGER*4  OBJECT
        INTEGER*4  ROOT_LEVEL
        INTEGER*2  SEARCH_SCOPE

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
PTR OdlFortNextObjDesc ( PTR *object, unsigned long *root_level_ptr,
                         PTR *search_scope_ptr)
{
#else
#ifdef _NO_PROTO
PTR odlfortnextobjdesc_ (object, root_level_ptr, search_scope_ptr)
PTR *object;
unsigned long *root_level_ptr;
PTR *search_scope_ptr;
{
#else
PTR odlfortnextobjdesc_ (PTR *object, unsigned long *root_level_ptr, 
                         PTR *search_scope_ptr)
{
#endif
#endif

return( (PTR) OdlNextObjDesc( (ODLTREE) *object, *root_level_ptr,
                              (unsigned short *) *search_scope_ptr));
}


/*****************************************************************************

  Routine: OdlFortParseLabelFile

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

        INTEGER*4  FUNCTION OdlFortParseLabelFile (FILESPEC, MESSAGE_FNAME,
                                                   EXPAND, SUPPRESS_MESSAGES)
        CHARACTER*(*) FILESPEC
        CHARACTER*(*) MESSAGE_FNAME
        INTEGER*2     EXPAND
        INTEGER*2     SUPPRESS_MESSAGES

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
PTR OdlFortParseLabelFile (struct dsc$descriptor_s *filespec_dsc,
                           struct dsc$descriptor_s *message_fname_dsc,
                           MASK *expand_ptr, 
                           unsigned short *suppress_messages_ptr)
{
char *filespec_ptr = filespec_dsc->dsc$a_pointer;
char *message_fname_ptr = message_fname_dsc->dsc$a_pointer;
long filespec_len = filespec_dsc->dsc$w_length;
long message_fname_len = message_fname_dsc->dsc$w_length;
#else
#ifdef _NO_PROTO
PTR odlfortparselabelfile_ (filespec_ptr, message_fname_ptr, expand_ptr,
                            suppress_messages_ptr, filespec_len, 
                            message_fname_len)
char *filespec_ptr;
char *message_fname_ptr;
MASK *expand_ptr;
unsigned short *suppress_messages_ptr;
long filespec_len;
long message_fname_len;
{
#else
PTR odlfortparselabelfile_ (char *filespec_ptr, char *message_fname_ptr, 
                            MASK *expand_ptr, 
                            unsigned short *suppress_messages_ptr,
                            long filespec_len, long message_fname_len)
{
#endif
#endif

PTR result;
char *filespec;
char *message_fname;

FortrantoCString( filespec, filespec_ptr, filespec_len);
FortrantoCString( message_fname, message_fname_ptr, message_fname_len);

result = (PTR) OdlParseLabelFile( filespec, message_fname, *expand_ptr,
                                  *suppress_messages_ptr);
LemmeGo( filespec);
LemmeGo( message_fname);
return( result);
}


/*****************************************************************************

  Routine: OdlFortPasteKwd

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

        INTEGER*4  FUNCTION OdlFortPasteKwd (KEYWORD, OBJECT)
        INTEGER*4  KEYWORD
        INTEGER*4  OBJECT

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
PTR OdlFortPasteKwd (PTR *keyword, PTR *object)
{
#else
#ifdef _NO_PROTO
PTR odlfortpastekwd_ ( keyword, object)
PTR *keyword;
PTR *object;
{
#else
PTR odlfortpastekwd_ (PTR *keyword, PTR *object)
{
#endif
#endif

return( (PTR) OdlPasteKwd( (KEYWORD *) *keyword, (ODLTREE) *object));
}


/*****************************************************************************

  Routine: OdlFortPasteKwdBefore

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

        INTEGER*4  FUNCTION OdlFortPasteKwdBefore (NEW_KEYWORD, OLD_KEYWORD)
        INTEGER*4  NEW_KEYWORD
        INTEGER*4  OLD_KEYWORD

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
PTR OdlFortPasteKwdBefore (PTR *new_keyword, PTR *old_keyword)
{
#else
#ifdef _NO_PROTO
PTR odlfortpastekwdbefore_ (new_keyword, old_keyword)
PTR *new_keyword;
PTR *old_keyword;
{
#else
PTR odlfortpastekwdbefore_ (PTR *new_keyword, PTR *old_keyword)
{
#endif
#endif

return( (PTR) OdlPasteKwdBefore( (KEYWORD *) *new_keyword,
                                 (KEYWORD *) *old_keyword));
}


/*****************************************************************************

  Routine: OdlFortPasteKwdAfter

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

        INTEGER*4  FUNCTION OdlFortPasteKwdAfter (NEW_KEYWORD, OLD_KEYWORD)
        INTEGER*4  NEW_KEYWORD
        INTEGER*4  OLD_KEYWORD

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
PTR OdlFortPasteKwdAfter (PTR *new_keyword, PTR *old_keyword)
{
#else
#ifdef _NO_PROTO
PTR odlfortpastekwdafter_ (new_keyword, old_keyword)
PTR *new_keyword;
PTR *old_keyword;
{
#else
PTR odlfortpastekwdafter_ (PTR *new_keyword, PTR *old_keyword)
{
#endif
#endif

return( (PTR) OdlPasteKwdBefore( (KEYWORD *) *new_keyword,
                                 (KEYWORD *) *old_keyword));
}


/*****************************************************************************

  Routine: OdlFortPasteObjDesc

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

        INTEGER*4  FUNCTION OdlFortPasteObjDesc (NEW_OBJECT, PARENT_OBJECT)
        INTEGER*4  NEW_OBJECT
        INTEGER*4  PARENT_OBJECT

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
PTR OdlFortPasteObjDesc (PTR *new_object, PTR *parent_object)
{
#else
#ifdef _NO_PROTO
PTR odlfortpasteobjdesc_ (new_object, parent_object)
PTR *new_object;
PTR *parent_object;
{
#else
PTR odlfortpasteobjdesc_ (PTR *new_object, PTR *parent_object)
{
#endif
#endif

return( (PTR) OdlPasteObjDesc( (ODLTREE) *new_object, 
                               (ODLTREE) *parent_object));
}


/*****************************************************************************

  Routine: OdlFortPasteObjDescAfter

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

        INTEGER*4  FUNCTION OdlFortPasteObjDescAfter (NEW_OBJECT, OLD_OBJECT)
        INTEGER*4  NEW_OBJECT
        INTEGER*4  OLD_OBJECT

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
PTR OdlFortPasteObjDescAfter (PTR *new_object, PTR *old_object)
{
#else
#ifdef _NO_PROTO
PTR odlfortpasteobjdescafter_ (new_object, old_object)
PTR *new_object;
PTR *old_object;
{
#else
PTR odlfortpasteobjdescafter_ (PTR *new_object, PTR *old_object)
{
#endif
#endif

return( (PTR) OdlPasteObjDescAfter( (ODLTREE) *new_object,
                                    (ODLTREE) *old_object));
}


/*****************************************************************************

  Routine: OdlFortPasteObjDescBefore

  Description:  This routine is equivalent to a FORTRAN function declared as
                follows:

        INTEGER*4  FUNCTION OdlFortPasteObjDescBefore (NEW_OBJECT, OLD_OBJECT)
        INTEGER*4  NEW_OBJECT
        INTEGER*4  OLD_OBJECT

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
PTR OdlFortPasteObjDescBefore (PTR *new_object, PTR *old_object)
{
#else
#ifdef _NO_PROTO
PTR odlfortpasteobjdescbefore_ (new_object, old_object)
PTR *new_object;
PTR *old_object;
{
#else
PTR odlfortpasteobjdescbefore_ (PTR *new_object, PTR *old_object)
{
#endif
#endif

return( (PTR) OdlPasteObjDescBefore( (ODLTREE) *new_object,
                                     (ODLTREE) *old_object));
}


/*****************************************************************************

  Routine: OdlFortPrintLabel

  Description:  This routine is equivalent to a FORTRAN subroutine declared as
                follows:

        SUBROUTINE  OdlFortPrintLabel( OBJECT, MESSAGE_FNAME, MESSAGE_FPTR,
                                       ROOT_LEVEL)
        INTEGER*4      OBJECT
        CHARACTER *(*) MESSAGE_FNAME
        INTEGER*4      MESSAGE_FPTR
        INTEGER*4      ROOT_LEVEL

        Note: INTEGER*4 should be changed to INTEGER*8 on Dec/Alpha
              or any other platform which has 8-byte pointers.
 
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
void OdlFortPrintLabel (PTR *object, 
                        struct dsc$descriptor_s *message_fname_dsc,
                        PTR *message_fptr_ptr, unsigned long *root_level_ptr)
{
char *message_fname_ptr = message_fname_dsc->dsc$a_pointer;
long message_fname_len = message_fname_dsc->dsc$w_length;
#else
#ifdef _NO_PROTO
void odlfortprintlabel_ (object, message_fname_ptr, message_fptr_ptr, 
                         root_level_ptr, message_fname_len)
PTR *object;
char *message_fname_ptr;
PTR *message_fptr_ptr;
unsigned long *root_level_ptr;
long message_fname_len;
{
#else
void odlfortprintlabel_ (PTR *object, char *message_fname_ptr, 
                         PTR *message_fptr_ptr, unsigned long *root_level_ptr,
                         long message_fname_len)
{
#endif
#endif

PTR result;
char *message_fname;

FortrantoCString( message_fname, message_fname_ptr, message_fname_len);

OdlPrintLabel( (ODLTREE) *object, message_fname, (FILE *) *message_fptr_ptr, 
               *root_level_ptr);

LemmeGo( message_fname);
}
