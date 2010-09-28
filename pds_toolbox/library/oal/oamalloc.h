/*****************************************************************************

  File:  oamalloc.h

  Description: This file is gives prototypes for the memory managment functions
               OaMalloc, OaFree and OaRealloc.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   6 Dec   1995
  Last Modified:  21 Mar   1997

  History:

    Creation - This include file was part of the first post-Beta Release of the
               OA library.  These prototypes were previously located in oal.h.
    03/21/97 - Added old_size argument to OaRealloc. SM

*****************************************************************************/

#ifndef OA_MALLOC_INCLUDED
#define OA_MALLOC_INCLUDED 1

#include <stdlib.h>
#if (defined( IBM_PC) && defined( MS_DOS))
#include <alloc.h>
#endif

#ifdef _NO_PROTO

extern char *    OaMalloc();
extern void      OaFree();
extern char *    OaRealloc();

#else

extern char *    OaMalloc( long bytes);
extern void      OaFree( char *ptr);
extern char *    OaRealloc( char *old_ptr, long old_size, long bytes);

#endif  /* ifdef _NO_PROTO  */
#endif  /* ifndef OA_MALLOC_INCLUDED  */
