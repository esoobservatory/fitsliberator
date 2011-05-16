/*****************************************************************************

  File:  oamalloc.c

  Description: This file contains the OA Library's memory management routines.
               All memory management performed by the OA Library and L3 is
               done through these functions.  The function are meant to be
               easily customized by application developers to suit their
               memory management scheme.
               The routines in this file are:

               OaMalloc
               OaFree
               OaRealloc

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   6  Dec  1995
  Last Modified:  21  Mar  1997

  History:

    Creation - These routines were part of the Alpha Release of the OA
               library, and were located in obj_l1.c and obj_l2.c.
    12/6/95  - Moved to oamalloc.c and changed return pointer types from PTR
               to (char *).  SM
    03/21/97 - Added old_size argument to OaRealloc. SM
	08/23/00 - changed code so that XVT_DEF must be defined for xvt code to 
			   be compiled.   DWS
*****************************************************************************/

#include <stdio.h>
#include "oamalloc.h"
#if defined XVT_DEF
#include "xvt.h"
#endif


/*****************************************************************************

  Routine:  OaMalloc

  Description:  OaMalloc allocates memory for object data.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  28 Feb  1995
  Last Modified:   2 Dec  1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.
    12/2/95  - Changed return value from PTR to char *.  SM

  Input:  bytes - The number of bytes of memory to allocate.

  Output:  If successful, the function returns a pointer to the allocated
           memory, otherwise NULL. 

  Notes:  The OA Library uses OaMalloc, OaFree and OaRealloc to allocate,
          reallocate and free memory, both for small amounts of memory and
          for the object data pointed to by the Oa_object's data_ptr, which
          is typically large.
          Application developers may wish to re-write OaMalloc, OaFree and
          OaRealloc to suit their particular memory management requirements.

*****************************************************************************/

#ifdef _NO_PROTO

char *OaMalloc( bytes)
long bytes;

#else

char *OaMalloc( long bytes)

#endif
{

if (bytes <= 0)
  return( NULL);

#if (defined( IBM_PC) && defined( MS_DOS))
	#if defined XVT_DEF
		return((char *)xvt_mem_zalloc((unsigned long) bytes));
	#else
		return( (char *) farmalloc( (unsigned long) bytes));
	#endif
#else
	#if defined XVT_DEF
		return( (char *) xvt_mem_zalloc( (size_t) bytes));
	#else
		return( (char *) malloc( (size_t) bytes));
	#endif
#endif
}



/*****************************************************************************

  Routine:  OaFree

  Description:  OaFree frees memory allocated by OaMalloc.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  28 Feb  1995
  Last Modified:   2 Dec  1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.
    12/2/95  - Changed input parameter from PTR to char *.  SM

  Input:  
         ptr - A pointer to data in dynamic memory, originally allocated by
               OaMalloc.
         
  Output:  If the input pointer was non-NULL, then the memory it points to
           has been freed.

  Notes:  The OA Library uses OaMalloc, OaFree and OaRealloc to allocate,
          reallocate and free memory, both for small amounts of memory and
          for the object data pointed to by the Oa_object's data_ptr, which
          is typically large.
          Application developers may wish to re-write OaMalloc, OaFree and
          OaRealloc to suit their particular memory management requirements.

*****************************************************************************/

#ifdef _NO_PROTO

void OaFree( ptr)
char *ptr;

#else

void OaFree( char *ptr)

#endif
{

if (ptr != NULL) {

#if (defined( IBM_PC) && defined( MS_DOS))
	#if defined XVT_DEF
			xvt_mem_free (ptr);
	#else
		    farfree( ptr);
	#endif
#else
	#if defined XVT_DEF
			xvt_mem_free (ptr);
	#else
			free( ptr);
	#endif
#endif

/*
#if (defined( IBM_PC) && defined( MS_DOS))
//  farfree( ptr);
	xvt_mem_free (ptr);
#else
//  free( ptr);
	xvt_mem_free(ptr);
#endif
	*/
}
}



/*****************************************************************************

  Routine:  OaRealloc

  Description:  This routine adjusts the size of memory allocated by
                OaMalloc.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  28 Feb  1995
  Last Modified:  21 Mar  1997

  History:

    Creation - This routine was part of the Beta Release of the OA library.
    12/6/95  - Changed input argument and return value from PTR to char *.  SM
    03/21/97 - Added old_size argument. SM


  Input:  old_ptr  - A pointer to dynamic memory originally allocated by
                     OaMalloc (or OaRealloc).
 
          old_size - Size in bytes pointed to be old_ptr.

          bytes    - The size to reallocate to.
                                      
         
  Output:  If successful, the function returns a pointer to the allocated
           memory, otherwise NULL. 

  Notes:  The OA Library uses OaMalloc, OaFree and OaRealloc to allocate,
          reallocate and free memory, both for small amounts of memory and
          for the object data pointed to by the Oa_object's data_ptr, which
          is typically large.
          Application developers may wish to re-write OaMalloc, OaFree and
          OaRealloc to suit their particular memory management requirements.

*****************************************************************************/
#ifdef _NO_PROTO

char *OaRealloc( old_ptr, old_size, bytes)
char *old_ptr;
long old_size;
long bytes;

#else

char *OaRealloc( char *old_ptr, long old_size, long bytes)

#endif
{

if ((old_ptr == NULL) || (bytes <= 0))
  return( NULL);

#if (defined( IBM_PC) && defined( MS_DOS))
	#if defined XVT_DEF
		return((char *)xvt_mem_realloc( old_ptr, bytes));
	#else
		return( (char *) realloc( old_ptr, bytes));
	#endif
#else
	#if defined XVT_DEF
		return( (char *) xvt_mem_realloc( old_ptr, bytes));
	#else
		return( (char *) realloc( old_ptr, bytes));
	#endif
#endif




/*#if (defined( IBM_PC) && defined( MS_DOS))

//return( (char *) farrealloc( old_ptr, bytes));
return( (char *) xvt_mem_realloc( old_ptr, bytes));
#else
//return( (char *) realloc( old_ptr, bytes));
return( (char *) xvt_mem_realloc( old_ptr, bytes));
#endif
*/

}

