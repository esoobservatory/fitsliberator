/*****************************************************************************

  File:  binrep.c

  Description:  This file contains the data structures and functions used in
                binary-to-binary conversions of integer and floating-point
                numeric types.  The routines in this file are:

                OalResetBinrepErrors
                OalReportBinrepErrors
                OalFindBinrepDescrip
                BinrepConvert
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:  16 Apr   1997

  History:

    Creation - These routines were part of the Alpha Release of the OA library.
    12/6/95  - Replaced malloc() by OaMalloc() throughout.  SM
    07/24/96 - Added detection of the following errors:  SM
               - loss of precision in real-to-real conversions
               - exponent underflow and overflow in real-to-real conversions
               - converting an integer to an integer too small to represent
                 the source integer; this includes converting an unsigned
                 to a signed of the same size, when the MSB bit of the
                 unsigned is set.
               - converting a negative integer to a unsigned integer
    04/16/97 - Modified OalFindBinrepDescrip to create on-the-fly n-byte
               binrep_desc structures to support BIT_STRINGS.  SM

  Notes:  

*****************************************************************************/

#include "binrep.h"
#include "oamalloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

#ifdef _NO_PROTO
extern int       OaReportError();
#else
extern int       OaReportError( char *input_error_string);
#endif
extern int oa_errno;

/****************************************************************************
Global variable binrep_descriptions contains binrep specifications, which
are used by BinrepConvert in converting from one binary representation to 
another.  See the binrep.h for complete documentation on this structure.
****************************************************************************/
struct binrep_desc binrep_descriptions[ N_BINREP_DESCRIPTIONS] = {
        "UINT1", 1, BINREP_INTEGER, -1, '0', "0..0", 0, NULL, NULL, 0,
         "INT1", 1, BINREP_INTEGER, -1, '2', "0..0", 0, NULL, NULL, 0,

    "MSB_UINT2", 2, BINREP_INTEGER, -1, '0', "0..1", 0, NULL, NULL, 0,
    "MSB_UINT4", 4, BINREP_INTEGER, -1, '0', "0..3", 0, NULL, NULL, 0,
    "MSB_UINT8", 8, BINREP_INTEGER, -1, '0', "0..7", 0, NULL, NULL, 0,

    "MSB_INT2", 2, BINREP_INTEGER, -1, '2', "0..1", 0, NULL, NULL, 0,
    "MSB_INT4", 4, BINREP_INTEGER, -1, '2', "0..3", 0, NULL, NULL, 0,
    "MSB_INT8", 8, BINREP_INTEGER, -1, '2', "0..7", 0, NULL, NULL, 0,

    "LSB_UINT2", 2, BINREP_INTEGER, -1, '0', "1..0", 0, NULL, NULL, 0,
    "LSB_UINT4", 4, BINREP_INTEGER, -1, '0', "3..0", 0, NULL, NULL, 0,
    "LSB_UINT8", 8, BINREP_INTEGER, -1, '0', "7..0", 0, NULL, NULL, 0,

    "LSB_INT2", 2, BINREP_INTEGER, -1, '2', "1..0", 0, NULL, NULL, 0,
    "LSB_INT4", 4, BINREP_INTEGER, -1, '2', "3..0", 0, NULL, NULL, 0,
    "LSB_INT8", 8, BINREP_INTEGER, -1, '2', "7..0", 0, NULL, NULL, 0,

    "IEEE_REAL4", 4, BINREP_FLOAT, 0, '0', NULL, 127,   "1..8",  "'1.',9..31", 0,
    "IEEE_REAL8", 8, BINREP_FLOAT, 0, '0', NULL, 1023,  "1..11", "'1.',12..63", 0,
    "IEEE_REAL10", 10, BINREP_FLOAT, 0, '0', NULL, 16383, "1..15", "16..16,'.',17..79", 0,

    "VAXF_REAL4", 4, BINREP_FLOAT, 8, '0', NULL, 129,  "9..15,0..0", "'1.',1..7,24..31,16..23", 0,
    "VAXD_REAL8", 8, BINREP_FLOAT, 8, '0', NULL, 129,  "9..15,0..0", "'1.',1..7,24..31,16..23,40..47,32..39,56..63,48..55", 0,
    "VAXG_REAL8", 8, BINREP_FLOAT, 8, '0', NULL, 1025, "9..15,0..3", "'1.',4..7,24..31,16..23,40..47,32..39,56..63,48..55", 0,
    "VAXH_REAL16", 16, BINREP_FLOAT, 8, '0', NULL, 16385, "9..15,0..7", "'1.',24..31,16..23,40..47,32..39,56..63,48..55,72..79,64..71,88..95,80..87,104..111,96..103,120..127,112..119", 0,

    "PC_REAL4", 4, BINREP_FLOAT, 24, '0', NULL, 127, "25..31,16..16", "'1.',17..23,8..15,0..7", 0,
    "PC_REAL8", 8, BINREP_FLOAT, 56, '0', NULL, 1023, "57..63,48..51", "'1.',52..55,40..47,32..39,24..31,16..23,8..15,0..7", 0,
    "PC_REAL10", 10, BINREP_FLOAT, 72, '0', NULL, 16383, "73..79,64..71", "56..56,'.',57..63,48..55,40..47,32..39,24..31,16..23,8..15,0..7", 0
};


/****************************************************************************
Global variable binrep_errors contains counters for the number of errors
occurred of various types.
****************************************************************************/

struct binrep_errors_struct {
  unsigned long int_truncations;
  unsigned long neg_to_unsigned_int_conversions;
  unsigned long exponent_underflows;
  unsigned long exponent_overflows;
  unsigned long precision_losses;
} binrep_errors = {0L,0L,0L,0L,0L};



/*****************************************************************************

  Routine:  OalResetBinrepErrors

  Description:  This function sets all the binrep error counters to zeros.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  24 July  1996
  Last Modified:  24 July  1996

  History:

    Creation - This routine was part of the Version 1.1 Release of the OA
               library.

  Input:  
         
  Output:  The function always returns zero.
 
  Notes:  

*****************************************************************************/

int OalResetBinrepErrors() {

memset( &binrep_errors, 0, sizeof( binrep_errors));
return(0);
}



/*****************************************************************************

  Routine:  OalReportBinrepErrors

  Description:  This function checks binrep_errors for non-zero values, and
                calls OaReportError if any are set.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  24 July  1996
  Last Modified:  24 July  1996

  History:

    Creation - This routine was part of the Version 1.1 Release of the OA
               library.

  Input:  
         
  Output:  The function always returns zero.  If one or more binrep_errors
           counters was non-zero, an error message is issued with
           OaReportError.
 
  Notes:  

*****************************************************************************/

#ifdef _NO_PROTO

int OalReportBinrepErrors( proc_name)
char *proc_name;

#else

int OalReportBinrepErrors( char *proc_name)

#endif
{
char message[128];
int len;

if (proc_name != NULL)
  sprintf( message, "%s: ", proc_name);
else
  message[0] = '\0';
len = strlen( message);

if (binrep_errors.int_truncations > 0)
  sprintf( message + strlen( message), "int_truncations = %lu  ", 
           binrep_errors.int_truncations);

if (binrep_errors.neg_to_unsigned_int_conversions > 0)
  sprintf( message + strlen( message),
           "neg_to_unsigned_int_conversions = %lu  ", 
           binrep_errors.neg_to_unsigned_int_conversions);

if (binrep_errors.precision_losses > 0)
  sprintf( message + strlen( message),
           "precision_losses = %lu  ", 
           binrep_errors.precision_losses);

if (binrep_errors.exponent_overflows > 0)
  sprintf( message + strlen( message),
           "exponent_overflows = %lu  ", 
           binrep_errors.exponent_overflows);

if (binrep_errors.exponent_underflows > 0)
  sprintf( message + strlen( message),
           "exponent_underflows = %lu  ", 
           binrep_errors.exponent_underflows);

if ((int) strlen( message) > len) {
  oa_errno = 904;
  OaReportError( message);
}
return(0);
}



/*****************************************************************************

  Routine:  OalFindBinrepDescrip

  Description:  This function searches the global array of structures 
                binrep_descriptions for the structure with the binrep_q_code
                specified in the input parameter, and returns a pointer to
                this structure.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:  16 Apr   1997

  History:

    Creation - This routine was part of the Alpha Release of the OA library.
    04/16/97 - Modified OalFindBinrepDescrip to create on-the-fly n-byte
               binrep_desc structures to support BIT_STRINGS.  SM

  Input:  
          binrep_q_code - A string specifying a length-qualified binrep
                          "q-code", like "MSB_INT4" or "LSB_INT13".
         
  Output:  If successful, the function returns a pointer to a (initialized)
           binrep_desc structure.  If the pointer points directly into the
           global binrep_descriptions array, then the is_dynamic field of the
           structure is 0.  If a binrep_desc was created on-the-fly, then
           is_dynamic is set to 1.  If no binrep_desc could be found or
           created on-the-fly, the function returns NULL.
 
  Notes:  

*****************************************************************************/

#ifdef _NO_PROTO

struct binrep_desc *OalFindBinrepDescrip( binrep_q_code)
char *binrep_q_code;

#else

struct binrep_desc *OalFindBinrepDescrip( char *binrep_q_code)

#endif
{
int i;
struct binrep_desc *binrep_desc;
char *ptr;
short *short_arr, n_bytes;

if (binrep_q_code == NULL) return(NULL);
if (strlen( binrep_q_code) < (size_t) 1) return(NULL);
for (i=0; i<N_BINREP_DESCRIPTIONS; i++)
  if (strcmp( binrep_descriptions[i].q_code, binrep_q_code) == 0) 
    return( &(binrep_descriptions[i]));

/* Couldn't find the binrep_q_code in the global array; if it's an integer
   q_code, create a binrep description on-the-fly - it's probably for a
   BIT_STRING.  Parse the q_code, which should look something like:
   "MSB_UINT32" or "LSB_INT12" */

if (strstr( binrep_q_code, "INT") != NULL) {
  binrep_desc = (struct binrep_desc *) OaMalloc( (long) sizeof( 
                                                  struct binrep_desc));
  binrep_desc->q_code = (char *) OaMalloc( (long) strlen( binrep_q_code)+1);
  strcpy( binrep_desc->q_code, binrep_q_code);
  ptr = strstr( binrep_q_code, "INT") + 3;
  if (sscanf( ptr, "%hd", &n_bytes) != 1)
    return( NULL);
  binrep_desc->bytes = n_bytes;
  binrep_desc->type = BINREP_INTEGER;
  binrep_desc->sign_bit = 0;
  if        (strstr( binrep_q_code, "_UINT") != NULL) {
    binrep_desc->complement = '0';
  } else if (strstr( binrep_q_code, "_INT") != NULL) {
    binrep_desc->complement = '2';
  } else {
    return( NULL);
  }  
  short_arr = (short *) OaMalloc( 2 * sizeof( short));
  binrep_desc->integer_order = (char *) short_arr;
  if        (strncmp( binrep_q_code, "MSB_", 4) == 0) {
    short_arr[0] = 0;
    short_arr[1] = n_bytes-1;
  } else if (strncmp( binrep_q_code, "LSB_", 4) == 0) {
    short_arr[0] = n_bytes-1;
    short_arr[1] = 0;
  } else {
    return( NULL);
  }
  binrep_desc->bias = 0;
  binrep_desc->exponent_order = NULL;
  binrep_desc->mantissa_order = NULL;
  binrep_desc->is_dynamic = 1;
  return( binrep_desc);
}

return(NULL);
}



/*****************************************************************************

  Routine:  BinrepConvert

  Description: BinrepConvert is the binary-to-binary conversion routine used
               by the PDS Object Access Library.  It converts a single
               integer or floating point number from one type to another, e.g.
               a VAX F-float to an IEEE float.  The algorithm is based on 
               the binrep ("Binary Representation") scheme of describing binary
               data types, from the draft document:  "ANSI Transfer Syntax 
               Description Notation";  binrep descriptions for most PDS
               numeric data types are defined in the binrep_descriptions
               global variable.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:    1 Sept   1994
  Last Modified:   15 Apr    1997

  History:

    Creation - This routine was part of the Alpha Release of the OA library.
    10/18/94 - Fixed bug in float conversion section: dst bytes are now zeroed 
               out before converted data is OR'd into dst.  SM
    11/21/96 - Fixed several bugs in int conversion section: sign extension
               and int truncation detection.  SM
    04/15/97 - Changed size of src_val buffer to 512 to handle BIT_STRINGS
               of up to 512 bytes in length.  SM

  Input:  
          src_descrip - A pointer to a binrep_desc structure, which describes
                        the source data type.
 
          dst_descrip - A pointer to a binrep_desc structure, which describes
                        the destination data type.
  
          srcp        - A pointer to the first byte of the source data.
  
          dstp        - A pointer to the first byte of the destination.

  Output:  If successful the function returns 0, otherwise non-zero.
           The source data type has been converted to the destination data
           type and placed in memory starting at dstp.
    
  Notes:

  1) This algorithm currently works for the most common floating-point
     types, which includes all the floating-point types defined in the PDS
     Standards Document EXCEPT the IBM_REAL types.
  2) The source data pointer and destination pointer may overlap.
  3) Floating point types may be converted to other floating point types, and 
     integer types may be converted to other integer types;  float-to-integer
     and integer-to-float are NOT supported.  These are supported in 
     OalConvert.
  4) When integer truncation, exponent overflow, exponent underflow or mantissa
     precision loss is detected, the appropriate counter in global structure
     variable binrep_errors is incremented, and the function returns 1 (except
     for mantissa precision loss, where it returns 0).
  5) Integers may be of any number of bytes, but currently only 1,2,4 and 8 
     byte integers are defined in binrep_descriptions.  For integers, a
     modification of the binrep description is used for better performance - 
     the integer_order field refers to bytes, not bits.
  6) Integer types may be uncomplemented (unsigned) or 2's complement (signed);
     1's complement and sign-magnitude integers are NOT supported.
  7) The exponent and mantissa of floats are always assumed to be base 2,
     uncomplemented; the mantissa has a sign bit, and the exponent has a bias.
  8) Floating point types: one of the following MUST be present at the 
     beginning of the binrep mantissa format description to describe the 
     location of the decimal point, and if there is a hidden bit:
     '0.' - no hidden bit, first mantissa bit is counted as the first bit after
            the decimal point, with an implicit '0.' in front.  The floating
            point value is  0.(mantissa) * 2**(exponent-bias).
     '1.' - hidden bit, first mantissa bit is counted as the first bit after
            the decimal point, with an implicit '1.' in front.  The floating
            point value is  1.(mantissa) * 2**(exponent-bias).
     <x..x>,'.' - a single bit, weighted as the most-significant bit, is
                  physically present in the float at bit location x; this bit
                  may be a 1 or a 0.  The implicit decimal point follows,  
                  followed by the next mantissa bit, which is weighted as the
                  first bit after the decimal point.   The floating
                  point value is  x.(mantissa) * 2**(exponent-bias) where x
                  is 1 or 0.
     Note: to convert a '.1' to a '1.', add one to the bias.
  9) The strings in the binrep_descriptions variable are pre-processed on
     the first call to BinrepConvert for performance reasons.
 10) Algorithm:
     a) The main idea of the binrep representation is to list the numeric
        type's bit (or byte) positions in most-significant to
        least-significant order.  A bit or byte position is an offset from
        the start of the number; in increasing memory order for bytes, and
        in most-significant bit to least-significant bit order for bits within
        a byte.  On all platforms, a 128 masks the most-significant bit of a
        byte, and 1 masks the least-significant bit.   Bit and byte positions
        start at 0, and grow as one moves away from the starting location in
        memory.  For integers, the binrep description lists the position of 
        the most-significant byte first, followed by the position of the next
        most-significant byte etc.  For floats, the binrep description lists
        the  mantissa bit positions and exponent bit positions separately,
        each in most-significant to least-significant bit order.
     b) For integers, the algorithm copies bytes from the source to the
        destination starting with the least-significant byte.  It finds each
        byte using the position specified by the source description, then 
        copies the byte to the position specified for it by the destination
        description.  The loop walks backwards through the source and
        destination descriptions in parallel.  After exhausting one or both
        of the descriptions, it does sign extension, if necessary.
     c) For floating-point types, the algorithm extracts the exponent bits
        in a manner similar to the integer algorithm, except it deals in bits
        instead of bytes, and puts it into a native 2-byte integer.  It then
        copies the mantissa bits into a string, where a set bit is represented
        by a '1' and a cleared bit by a '0'.  The first character of the
        string is set according to the hidden bit specification.
        Then it adds the bias difference to the exponent, and compares the
        source and destination hidden bit specifications, adjusting the
        exponent value as needed, and checks for underflow and overflow of the
        exponent.  The next step transfers the exponent bits from the native
        2-byte integer to the dst;  the exponent bit locations are known from
        the integer byte order of the platform, and the destination bits from
        the destination exponent description.  The last step moves through the
        mantissa string and the destination mantissa description together in
        most-significant bit to least-significant bit order, setting the
        appropriate bit in the destination for each '1' in the mantissa string.

******************************************************************************/

#define ONE_POINT   0x7FFD  /* Replaces mantissa description's '1.'  */
#define ZERO_POINT  0x7FFE  /* Replaces mantissa description's '0.'  */
#define POINT       0x7FFF  /* Replaces mantissa description's '.'   */

#ifdef _NO_PROTO

int BinrepConvert( src_descrip, dst_descrip, srcp, dstp)
struct binrep_desc *src_descrip;
struct binrep_desc *dst_descrip;
PTR srcp;
PTR dstp;

#else

int BinrepConvert( struct binrep_desc *src_descrip,
                   struct binrep_desc *dst_descrip,
                   PTR srcp, PTR dstp)

#endif
{

typedef unsigned short uint2;  /* A short int is 2 bytes on ALL machines/ */
                               /* compilers supported by OAL.             */
short endian_indicator=1;      /* Initialized to 1 to determine the value */
                               /* of *platform_is_little_endian below.    */
char *platform_is_little_endian = (char *) &endian_indicator;
/* If platform is little-endian (lsb first), then the 1 initialized in
   endian_indicator will be in the first byte, and *platform_is_little_endian
   will have the value TRUE (1), otherwise it will be FALSE (0).  A test is
   done on *platform_is_little_endian when reading the exponent bits of a 
   float into the 2-byte exponent variable, in order to get a valid integer
   for the platform being run on.  */


/* All local variables are static for better performance. 
   Variables used in float and integer conversions:  */

static int binrep_initialized = FALSE;  

static char *src;
static char src_val[512], error_string[132];
static short *src_fmt, *dst_fmt, *src_mantissa_fmt, *dst_mantissa_fmt;
static int i, j, n;

/* Variables used only in float conversions:  */

static uint2 exponent;
static int bits_copied;
static long dst_exponent;

static char *msb, *lsb, *current_byte;
static char str[128], *str_ptr;
static short start_bit, stop_bit;

/* The bits variable is used throughout the BINREP_FLOAT code to mask bits; 
   bits[0] is a byte whose most significant bit is a 1, and other bits 0's,
   thus when AND'ed to a byte, it extracts the most significant bit of the
   byte;  similarily, bits[7] masks/extracts the least-sig bit.  This is works
   the same on all platforms.  */

static unsigned char bits[8] = {128,64,32,16,8,4,2,1}; 

/* 'exponent_overflow_masks' lets the exponent overflow checking code look up
    a mask to extract the remaining bits in the 'exponent' variable and check
    that they are all 0's;  e.g. if 'bits_copied' is 11, then the code AND's
    exponent_overflow_masks[11] with 'exponent' to get the 5 most-sig. bits
    from 'exponent'.  */

static unsigned short exponent_overflow_masks[17];

/* Variables used only in integer conversions.  The xxx_byte variables are used
   as integers which hold a byte offset from a pointer.  */

static int src_start_byte, dst_start_byte, src_stop_byte, dst_stop_byte;
static int current_src_byte, current_dst_byte, src_inc, dst_inc;
static char c;

/* If the binrep structures haven't been initialized, initialize them.  This
   is only done once, on the first call to OalBinrepConvert.  Initialization
   consists of translating the binrep_descriptions' integer_order string or
   exponent_order and mantissa_order strings into arrays of shorts containing
   the bit/byte ranges specified in the strings;  the expensive sscanf calls
   are only done once during initialization.  */

if (binrep_initialized == FALSE) {
  for (n=0; n<N_BINREP_DESCRIPTIONS; n++) {

    if (binrep_descriptions[n].type == BINREP_INTEGER) {

      /* All PDS integer data types are described with a single byte range,
         so allocate space for 2 shorts.  */

      src_fmt = (short *) OaMalloc( (long) (2 * sizeof( short)));
      sscanf( binrep_descriptions[n].integer_order, "%hd%*2c%hd", 
              &(src_fmt[0]), &(src_fmt[1]));
      binrep_descriptions[n].integer_order = (char *) src_fmt;      

    } else { /* BINREP_FLOAT */

      /* Initialize exponent_order; the number of bit ranges is equal to the
         number of commas in the exponent_order string plus one, and each
         range needs two shorts in the array; one extra short at the start of
         the array gives the number of shorts in the array which follow. */

      src = (char *) binrep_descriptions[n].exponent_order;
      if (src == NULL) {
        printf("Error: exponent_order is NULL for %s\n", 
                binrep_descriptions[n].q_code);
        return(1);
      }
      j = 0;
      for (i=0; i < (int) strlen( src); i++)
	  {
        if (src[i] == ',') 
			j++;
	  }
      j++; 
	  
      src_fmt = (short *) OaMalloc( (long) (2*j + 1) * sizeof( short)); 
      src_fmt[0] = (short) (2 * j);   /* Number of shorts which follow. */
      for (i=0; i<j; i++) {           /* Loop through the bit ranges.   */
        sscanf( src, "%hd%*2c%hd", &start_bit, &stop_bit);
        src_fmt[ i*2 + 1] = start_bit;
        src_fmt[ i*2 + 2] = stop_bit;
        while((*src != ',') && (*src != '\0')) src++;
        if (*src == '\0') break;
        else src++;                   /* Move past comma.               */
      }
    binrep_descriptions[n].exponent_order = (char *) src_fmt; 
	
	  
	  
      /* Initialize mantissa_order.  The number of bit ranges is equal to the
         number of commas, and each range needs two shorts in the array; one 
         extra short is needed for the hidden bit description, and an extra
         short at the start of the array gives the number of shorts which
         follow in the array.  */

      src = (char *) binrep_descriptions[n].mantissa_order;
      if (src == NULL) {
        printf("Error: mantissa_order is NULL for %s\n", 
                binrep_descriptions[n].q_code);
        return(1);
      }
      j = 0;
	  /* 10-28-02 MDC Added brackets around the for statment */
      for (i=0; i < (int) strlen( src); i++)
	  {
        if (src[i] == ',') 
			j++;
	  }
     
	  src_fmt = (short *) OaMalloc( (long) (2*j + 2) * sizeof( short));  
      src_fmt[0] = (short) (2 * j + 1);  /* Number of shorts which follow. */
      src = (char *) binrep_descriptions[n].mantissa_order;
      i = 1;
      for (;;) {
        if        (strstr( src, "'1.'") == src) {
          src_fmt[i++] = (short) ONE_POINT;
        } else if (strstr( src, "'0.'") == src) {
          src_fmt[i++] = (short) ZERO_POINT;
        } else if (strstr( src, "'.'") == src) {
          src_fmt[i++] = (short) POINT;
        } else {
          sscanf( src, "%hd%*2c%hd", &start_bit, &stop_bit);
          src_fmt[i]   = start_bit;
          src_fmt[i+1] = stop_bit;
          i += 2;
        }
        while ((*src != ',') && (*src != '\0')) src++;
        if (*src == '\0') break;
        else src++;  /* Move past comma. */
      }
    binrep_descriptions[n].mantissa_order = (char *) src_fmt; 

	}   /* end else BINREP_FLOAT */
  }     /* end for loop through binrep_descriptions  */

  for (i=1; i<=16; i++)
    exponent_overflow_masks[i] = (0xFFFF << i);

  binrep_initialized = TRUE; 
   
}

if (src_descrip->type != dst_descrip->type) {
  return(1);
}

#ifdef OA_DEBUG2
printf("src_descrip->bytes = %d, dst_descrip->bytes = %d\n",
        src_descrip->bytes, dst_descrip->bytes);
#endif

/* Copy src to local buffer src_val (necessary if src and dst overlap). */

for (i=0; i<src_descrip->bytes; i++)
  src_val[i] = srcp[i];
src = (char *) src_val;

/* If the src and dst have the same binrep_description, copy src to dst and
   return. */

if (src_descrip == dst_descrip) {
  for (i=0; i<dst_descrip->bytes; i++) *(dstp+i) = *(src+i);
  return(0);
} 


/* Switch on the conversion type of src and dst - BINREP_INTEGER or 
   BINREP_FLOAT.  */

switch (src_descrip->type) {

  /***************************************************************************
     Integer types
  ***************************************************************************/

  case BINREP_INTEGER:

  /* Integer conversions are done by byte-swapping and sign extension only.
     If a destination integer is longer than a signed source integer, then the
     source's sign bit is extended leftwards into the extra destination bytes.
  */
 
	src_fmt = (short *) src_descrip->integer_order;
    dst_fmt = (short *) dst_descrip->integer_order;   
    src_start_byte = (int) src_fmt[0];
    src_stop_byte  = (int) src_fmt[1];
    dst_start_byte = (int) dst_fmt[0];
    dst_stop_byte  = (int) dst_fmt[1];

    /* Byte copies always start at the least-significant byte and proceed
       leftwards towards the most-significant byte.  */

    current_src_byte = src_stop_byte;
    current_dst_byte = dst_stop_byte;
    if (src_stop_byte > src_start_byte) src_inc = -1; else src_inc = 1;
    if (dst_stop_byte > dst_start_byte) dst_inc = -1; else dst_inc = 1;

    if (src_descrip->bytes == dst_descrip->bytes) {

      /* Same number of bytes in source integer and destination integer. */

      for (i=0; i<dst_descrip->bytes; i++) {
#ifdef OA_DEBUG2
        printf( "copying *(src+%d) to *(dst+%d)\n",
                 current_src_byte, current_dst_byte); 
#endif
        *(dstp + current_dst_byte) = *(src + current_src_byte);
        current_src_byte += src_inc;        
        current_dst_byte += dst_inc;        
      }

      /* Check if the source integer was too large for the destination.
         If src is unsigned and has it's MSB bit set, then it can't be
         represented in a signed integer of the same size, because the MSB
         bit would indicate a negative number; an additional 0 bit, which
         we don't have, is needed in front of the 1 bit.  */

      if ((src_descrip->complement == '0') &&
          (dst_descrip->complement == '2') &&
          (*(src + src_start_byte) & 0x80) != 0) {
        binrep_errors.int_truncations++;
        return(1);
      }

    } else if (src_descrip->bytes > dst_descrip->bytes) {  

      /* Source integer is longer than destination integer. */

      for (i=0; i<dst_descrip->bytes; i++) {
#ifdef OA_DEBUG2
        printf( "copying *(src+%d) to *(dst+%d)\n",
                 current_src_byte, current_dst_byte); 
#endif
        *(dstp + current_dst_byte) = *(src + current_src_byte);
        current_src_byte += src_inc;        
        current_dst_byte += dst_inc;        
      }

      /* Check if the source integer was too large for the destination.  */

      if (src_descrip->complement == '2') {

        /* If src is negative, then dst better be negative, and all the
           uncopied src bytes better be FF's.  If src is positive, then if
           dst is signed, it better be positive.  Regardless of whether
           dst is signed or unsigned, if src is positive, all uncopied
           src bytes should be 0's.  */

        if ((*(src + src_start_byte) & 0x80) != 0) {
          c = (char) 0xFF;                                 /* src is neg */
          if ((*(dstp + dst_start_byte) & 0x80) == 0) {
            binrep_errors.int_truncations++;               /* dst is pos */
            return(1);
          }
        } else {                                           /* src is pos */
          c = 0x00;
          if (dst_descrip->complement == '2') {            /* dst is signed */
            if ((*(dstp + dst_start_byte) & 0x80) != 0) {
              binrep_errors.int_truncations++;             /* dst is neg */ 
              return(1);
            }
          }
        }
          
        for (j=i; j<src_descrip->bytes; j++) {
          if (*(src + current_src_byte) != c) {
            binrep_errors.int_truncations++;
            return(1);
          }
          current_src_byte += src_inc;
        }

      } else if (src_descrip->complement == '0') {

        /* The src is unsigned: 1) if the dst is signed, check that dst is
           positive, because if it's negative, we would need one more 0 bit in
           front of it (which we don't have) to represent the positive src
           value.  2) all the src bytes not copied better be 0's.  */

        if (dst_descrip->complement == '2')                /* dst is signed */
          if ((*(src + src_start_byte) & 0x80) != 0) {
            binrep_errors.int_truncations++;               /* src is neg    */
            return(1);
          }
        for (j=i; j<src_descrip->bytes; j++) {
          if (*(src + current_src_byte) != 0) {
            binrep_errors.int_truncations++;
            return(1);
          }
          current_src_byte += src_inc;
        }
      } else {
        ;
      }

    } else {                             /* Source integer is shorter than   */
                                         /* destination integer.             */
      for (i=0; i<src_descrip->bytes; i++) {
#ifdef OA_DEBUG2
        printf( "copying *(src+%d) to *(dst+%d)\n",
                 current_src_byte, current_dst_byte); 
#endif
        *(dstp + current_dst_byte) = *(src + current_src_byte);
        current_src_byte += src_inc;        
        current_dst_byte += dst_inc;        
      }
      if ((src_descrip->complement == '2') && 
         ((*(src + src_start_byte) & 0x80) != 0)) { /* Signed integer and    */
                                                    /* sign bit is set, so   */
                                                    /* extend sign bit.      */
        for (i=0; i<(dst_descrip->bytes - src_descrip->bytes); i++) {
          *(dstp + current_dst_byte) = (char) 0xFF;
          current_dst_byte += dst_inc;        
        }
      } else {                     /* Copy 0's into untouched bytes of dst. */
        for (i=0; i<(dst_descrip->bytes - src_descrip->bytes); i++) {
          *(dstp + current_dst_byte) = 0x00;
          current_dst_byte += dst_inc;        
        }
      }
    }

    /* If dst is unsigned and src is negative (if so, MSB is negative), flag an
       error.  */

    if ((dst_descrip->complement == '0') &&
        (src_descrip->complement == '2') &&
        ((*(src + src_start_byte) & 0x80) != 0)) {
      binrep_errors.neg_to_unsigned_int_conversions++;
      return(1);
    }

    return(0);
    /*NOTREACHED*/
    break;

  /***************************************************************************
     Floating point types
  ***************************************************************************/

  case BINREP_FLOAT:

    /* Zero out the destination, because the bit-wise ORing done in the
       destination doesn't always touch all the bits, and the code assumes
       all the destination bits are initially cleared.  */

    for (i=0; i<dst_descrip->bytes; i++)
      dstp[i] = 0;
    
    /* Get the exponent bits into exponent variable, which is 2 bytes long.
       (No PDS floating point type has an exponent longer than 2 bytes.)
       The exponent thus only has 2 possible byte orders - MSB or LSB.  
       Set msb and lsb to point to the most- and least-significant bytes.  */

    msb = (char *) &exponent;
    lsb = (char *) &exponent;
    exponent = 0;

    if (*platform_is_little_endian == TRUE)
      msb++;
    else
      lsb++;

	
    src_fmt = (short *) src_descrip->exponent_order;
    dst_fmt = (short *) dst_descrip->exponent_order;
 
	current_byte = lsb;
    bits_copied = 0;

    /* Loop through all the bit ranges in the source exponent description, 
       starting with the least-significant bit range and stepping leftwards in
       both the binrep description and in the binary exponent variable towards
       the most-significant bit range.  src_fmt[0] gives the number of shorts
       in the array after src_fmt[0], and the shorts always occur in pairs: 
       start_bit followed by stop_bit. */

    for (i=src_fmt[0]; i>0; i=i-2) {
      start_bit = src_fmt[i-1];
      stop_bit  = src_fmt[i];

      /* Loop through the bits in the current bit range. Once we've copied 8 
         bits to the lsb of exponent, switch to copying to the msb of 
         exponent.  If the bit in the src is set, set the corresponding bit
         in the current byte of exponent.  */

#ifdef OA_DEBUG2
      printf("exponent start_bit = %hd, stop_bit = %hd\n",
              start_bit, stop_bit);
#endif
      for (j=stop_bit; j>=start_bit; j--,bits_copied++) {
        if (bits_copied == 8) {
#ifdef OA_DEBUG2
          printf("switching current exponent byte to msb\n");
#endif
          current_byte = msb;
        }
        if ((*(src + j/8) & bits[ j%8]) != 0) {        /* if src bit is 1 */
          *current_byte |= bits[ 7-bits_copied%8];
        }
      }
    }      /* end loop through all the bit ranges */

#ifdef OA_DEBUG2
    printf("source exponent = %hu\n", exponent);
#endif

    /* If the exponent is zero, then check if all the bytes are zero; if so,
       the number is a zero, so take a short-cut by leaving all the bytes in 
       the dst zeros, and return.  */

    if (exponent == 0) {
      for (i=0; i<src_descrip->bytes; i++)
        if (src[i] != 0) break;
      if (i == src_descrip->bytes) 
        return(0);
    }

    /* Copy/translate the source mantissa bits into the str string variable.  
       Str contains '1's and '0's representing the set and unset bits in the
       mantissa, and includes the hidden bit explicitely in the first char, 
       followed an implicit decimal point. E.g. if the mantissa hidden bit
       specification is '1.', then a '1' will be put in str[0].  */

	
    src_mantissa_fmt = (short *) src_descrip->mantissa_order;
    dst_mantissa_fmt = (short *) dst_descrip->mantissa_order;

	str_ptr = str;

    /* Loop through the mantissa format array. */
  
    i = 1;
    while (i <= src_mantissa_fmt[0]) {
      if        (src_mantissa_fmt[i] == ONE_POINT) {
        *str_ptr++ = '1';
        i++;
      } else if (src_mantissa_fmt[i] == ZERO_POINT) {
        *str_ptr++ = '0';
        i++;
      } else if (src_mantissa_fmt[i] == POINT) {
        i++;
      } else {
        start_bit = src_mantissa_fmt[i];
        stop_bit  = src_mantissa_fmt[i+1];
        i += 2;

        /* Loop through the bits in this bit range.  Since the bits are in 
           MSB to LSB order in the binrep description, start copying to the 
           first character in str, and proceed sequentially.  If the bit in
           the src is set, write a '1' to str; else write a '0'.  */

        for (j=start_bit; j<=stop_bit; j++,bits_copied++,str_ptr++) {
          if ((*(src + j/8) & bits[ j%8]) != 0) *str_ptr = '1';
          else *str_ptr = '0';
        }
      } /* end else process bit range */
    }   /* end while loop through mantissa format array  */
    *str_ptr = '\0';

#ifdef OA_DEBUG2
    printf("mantissa str = %s\n", str);
#endif

    /* Copy the exponent to dst_exponent, and adjust it to reflect the
       difference in bias between the source and destination floating
       point types.  The exponent may be adjusted again later if we're
       converting between floats with different hidden bit specifications.  
       After all adjustments have been made, dst_exponent is tested for
       underflow and overflow.  */

    dst_exponent = (long) (exponent + dst_descrip->bias - src_descrip->bias);

    /* Deal with the hidden bit.  This is the trickiest part...
       The goal is to set str_ptr to point to the first "bit" in
       the string at which to start the translation to binary to the dst.
       We may need to skip over the first bit in str if the dst represents
       it implicitly as the hidden bit, or may need to step through to the 
       first '1' bit, essentially left-justifying the string, if the dst
       specifies a first bit or hidden bit of 1.  */

    str_ptr = str;
    if (dst_mantissa_fmt[1] == ZERO_POINT) {
        
      /* The dst float has no hidden bit, and counts the first mantissa bit
         as the first bit to the right of the decimal point.  If the source
         float is likewise, advance str_ptr by one over the leading 0 in str.
         Otherwise subtract 1 from exponent so that the first mantissa "bit" 
         in str[0] is weighed as the first bit to the right of the decimal
         point.  */

      if (src_mantissa_fmt[1] == ZERO_POINT)
        str_ptr++;
      else {
        dst_exponent--;
      }
    } else if (dst_mantissa_fmt[1] == ONE_POINT) {
        
      /* Dst float specifies a hidden bit of 1, and weighs the first mantissa
         bit as the first bit to the right of the decimal point.  If the 
         source float has the same specification, advance str_ptr by one over
         the leading '1' in str[0].  If the source float has an explicit bit 
         instead of a hidden bit left of the decimal point (POINT), then if 
         the bit is one (str[0] == '1'), treat it the same as above.  If the 
         explicit bit is cleared, or if the source spec is ZERO_POINT, then
         advance str_ptr and decrement exponent until a 1 is right of the 
         decimal point.  If reach end of string, have an all zero's mantissa,
         so return all zeros.  */

      if ((src_mantissa_fmt[1] == ONE_POINT) ||
         ((src_mantissa_fmt[3] == POINT) && (*str_ptr == '1'))) {
        str_ptr++;
      } else {
                    /* Advance until point to first '1'.  */
        while ((*str_ptr != '1') && (*str_ptr != '\0')) {
          str_ptr++;
          dst_exponent--;
        }
        if (*str_ptr != '\0') {  /* Advance one beyond the first '1'.  */
          str_ptr++;
          dst_exponent--;
        } else {  

          /* Mantissa is all 0's, so leave all the bytes in dst zeros the
             way they were initialized, and return.  */

          return(0);
        }
      }
    } else ;  /* Dst format is xx..xx,'.'  so can start at str[0], left of the
                 decimal point.  */

    /* Detect any underflow of dst_exponent.  */

    if (dst_exponent < 0) {
      binrep_errors.exponent_underflows++;
      return(1);
    }

    exponent = (uint2) dst_exponent;

#ifdef OA_DEBUG2
    printf("dst exponent = %hu\n", exponent);
#endif

    /* Transfer the bits from the exponent variable to the exponent bits of
       dst.  Start at the right end of the destination exponent format array
       and work back left, least-sig bit towards most-sig bit.  */
          
    current_byte = lsb;
    bits_copied = 0;

    for (i=dst_fmt[0]; i>1; i=i-2) {
      start_bit = dst_fmt[i-1];
      stop_bit  = dst_fmt[i];
      for (j=stop_bit; j>=start_bit; j--,bits_copied++) {
        if (bits_copied == 8) current_byte = msb;
        if ((*current_byte & bits[ 7-bits_copied%8]) != 0)
          *(dstp + j/8) |= bits[ j%8];
      }
    }   /* end of for loop through all the bit ranges */

    /* Detect overflow by examining the remaining bit locations in the
       'exponent' variable which weren't copied to the dst exponent: if any
       are non-zero, then the dst exponent doesn't have enough bits to 
       represent 'exponent', i.e. have overflow.  */

    if ((exponent & exponent_overflow_masks[ bits_copied]) != 0) {
      binrep_errors.exponent_overflows++;
      return(1);
    }

    /* Translate from the mantissa string to the mantissa dst bits.
       Loop through the bit ranges in the dst's mantissa order array,
       from left to right (MSB to LSB);  step over ONE_POINT, ZERO_POINT or
       POINT, and with each bit location, translate the '1' or '0' in str to a
       bit, bit-wise OR this bit mask with the current dst byte, and advance
       str_ptr to the next '0' or '1' in the string.  */

    i = 1;
    while (i <= dst_mantissa_fmt[0]) {
      if ((dst_mantissa_fmt[i] == ONE_POINT)  || 
          (dst_mantissa_fmt[i] == ZERO_POINT) ||
          (dst_mantissa_fmt[i] == POINT)) i++;
      else {
        start_bit = dst_mantissa_fmt[i];
        stop_bit  = dst_mantissa_fmt[i+1];

        /* Loop through the bits in this bit range. If the bit in str is set,
           bitwise OR the corresponding bit mask with the correct byte of dst.
           Since the dst may have more mantissa bits than the src, break if
           reach end of src mantissa string.  */

        for (j=start_bit; j<=stop_bit; j++,str_ptr++) {
          if (*str_ptr == '1')
            *(dstp + j/8) |= bits[ j%8];
          else if (*str_ptr == '\0')
            break;
        }
        i += 2;
      }
    }   /* end while loop processing format array */

    /* Check if there are '1' bits remaining in the src mantissa string;
       if so, the dst mantissa must be shorter than the src mantissa, and
       we've lost precision.  */

    while (*str_ptr != '\0') {
      if (*str_ptr == '1') {
        binrep_errors.precision_losses++;
        break;
      }
      str_ptr++;
    }
    /* If the src sign bit is set, copy it from the src to dst.  */

    if ((*(src + src_descrip->sign_bit/8) & 
         bits[ src_descrip->sign_bit%8]) != 0)
      *(dstp + dst_descrip->sign_bit/8) |= bits[ dst_descrip->sign_bit%8];

    return(0);
    /*NOTREACHED*/
    break;             /* end of case BINREP_FLOAT */

  default: 
    sprintf( error_string, "binrep: binrep type %d is not implemented",
             src_descrip->type);
    break;
}  /* end switch */
return(1);
}
