/*****************************************************************************

  File:  oal_sys.h

  Description: This is the include file for system-specific typedefs used in
               OAL.  So far there is just one: the typedef for PTR is a huge
               pointer on the IBM-PC to allow access to objects greater than
               64K in size.  This is necessary because when pointer arithmetic
               is done on far pointers, only the segment offset portion is
               effected, and the pointer will "wrap" after 64K.  With huge
               pointers, the compiler correctly adjusts the segment portion.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   20 March  1995
  Last Modified:   20 March  1995

  History:

    Creation - This include file was part of Beta Release of the OA
               library.

*****************************************************************************/

#ifndef OAL_SYS_INCLUDED
#define OAL_SYS_INCLUDED 1

#if (defined( IBM_PC) && defined( MS_DOS))
typedef char huge *PTR;
#else
typedef char *PTR;
#endif

#endif  /* #ifndef OAL_SYS_INCLUDED  */

