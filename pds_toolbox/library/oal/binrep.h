/*****************************************************************************

  File:  binrep.h

  Description: This is the include file for the binary-to-binary conversion
               function BinrepConvert.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:    1 Sept   1994
  Last Modified:   16 Apr    1997

  History:

    Creation - This include file was part of Alpha Release of the OA
               library.
    04/16/97 - Added is_dynamic field to binrep_desc structure to support
               on-the-fly creation of n-byte BIT_STRING binreps.  SM

*****************************************************************************/

#ifndef BINREP_INCLUDED
#define BINREP_INCLUDED 1

#include <stdio.h>
#include "oal_sys.h"

#define BINREP_INTEGER          0
#define BINREP_FLOAT            1
#define N_BINREP_DESCRIPTIONS  24

struct binrep_desc {
  char *q_code;          /* A unique BINREP Q-code identifier; last 1 or 2  */
                         /* chars must specify the number of bytes.         */
  char bytes;            /* Number of bytes.                                */
  char type;             /* BINREP_INTEGER or BINREP_FLOAT                  */
  int  sign_bit;         /* For BINREP_FLOAT, bit-position of the sign bit, */
                         /* -1 if unsigned.  Ignored for BINREP_INTEGER -   */
                         /* sign bit, if any is assumed to be the leftmost  */
                         /* bit in the most-significant byte.               */
  char complement;       /* '0' for none, or '2' for two's complement.      */
                         /* Used to determine "unsigned" or "signed" for    */
                         /* integers.  Sign-magnitude integers are NOT      */
                         /* supported!                                      */
  char *integer_order;   /* Describes integer byte (not bit!) locations,    */
                         /* in most-sig-byte to least-sig-byte order.       */
                         /* Must be a single, continuous range.             */
  int  bias;             /* Actual exponent = (exponent - bias).            */
  char *exponent_order;  /* Binrep description of exponent bit locations,   */
                         /* in most-sig-bit to least-sig-bit order.         */
  char *mantissa_order;  /* Binrep description of mantissa bit locations,   */
                         /* in most-sig-bit to least-sig-bit order.         */
                         /* Must start with '0.',  '1.'  or  'x..x','.'     */
  char is_dynamic;       /* If non-zero, then the structure and char *      */
                         /* fields were OaMalloc'ed and should be freed.    */
};

extern struct binrep_desc binrep_descriptions[];

#ifdef _NO_PROTO

extern int                 BinrepConvert();
extern struct binrep_desc *OalFindBinrepDescrip();
extern int                 OalReportBinrepErrors();
extern int                 OalResetBinrepErrors();

#else

extern int BinrepConvert( struct binrep_desc *src_descrip, 
                          struct binrep_desc *dst_descrip, 
                          PTR srcp, PTR dstp);
extern struct binrep_desc *OalFindBinrepDescrip( char *binrep_q_code);
extern int                 OalReportBinrepErrors( char *proc_name);
extern int                 OalResetBinrepErrors( void);

#endif

#endif  /* #ifndef BINREP_INCLUDED  */

