/*****************************************************************************

  File:  jpeg_c.h

  Description: This file was part of the original source code for the
               Clementine JPEG decompression software.

  Author:  Dr. Erick Malaret, ACT Corp.
  Packaged and tweeked for the OA Library:  Steve Monk, University of Colorado
                                            LASP  Nov 30, 1995
 
  Creation Date:  30 Nov   1995
  Last Modified:  30 Nov   1995

  History:

    Creation - This was created at ACT Corp, and modified by SM as follows
               Nov 30, 1995 for the OA Library:
               - Added #ifdef IBM_PC to existing #ifdefs.
               - Added #include for oamalloc.h
               - Replaced 'malloc' and 'free' in MALLOC and FREE macros by
                 'OaMalloc' and 'OaFree'.

*****************************************************************************/


/* Structure Definitions */
enum FmodeDef { INPUT, OUTPUT };
typedef enum FmodeDef Fmode;

struct ByteStreamDef
{
	Fmode   mode;
	short   stat;
	FILE    *file;
};
typedef struct ByteStreamDef ByteStream;

#ifdef _NO_PROTO
void cByteStream();
void dByteStream();
short ByteStream_read();
short ByteStream_write();
short ByteStream_status();
#else
void cByteStream( ByteStream *, char *FileName, Fmode FileMode );
void dByteStream(ByteStream *);
short ByteStream_read(ByteStream *);
short ByteStream_write(ByteStream *, short c);
short ByteStream_status(ByteStream *);
#endif

#define MEMORY  1
#define DISK    0

#include "oamalloc.h"        /* Added Nov 30, 1995 SM/Lasp */
#define MALLOC( bytes)  OaMalloc((long) bytes)
#define FREE( ptr)      OaFree((char *) ptr)

/*#if (defined(__BORLANDC__) || defined(IBM_PC)) && !defined(__WIN32__)*/
#if ((defined(IBM_PC)) && defined( MSDOS))
/*#define MALLOC farmalloc*/
/*#define FREE   farfree*/
#define CHARH	unsigned char huge
#define FAR		far
#else
/*#define MALLOC malloc*/
/*#define FREE	free*/
#define CHARH	unsigned char
#define FAR
#endif

struct BitStreamDef
{
	ByteStream	bytestream;
	short		BitBuffer;      /* Bit I/O buffer */
	short     	BitBuffMask;    /* Bit I/O buffer mask */
	CHARH		*outstring;
	char		mode;
	unsigned long	bytesout;
	unsigned short	bitmask[17];
};
typedef struct BitStreamDef BitStream;

#ifdef _NO_PROTO
void cBitStream();
void dBitStream();
short BitStream_write();
short BitStream_read();
#else
void cBitStream( BitStream *, char *fn, Fmode fm );
void dBitStream(BitStream *);
short BitStream_write(BitStream *, short bits, short width);
short BitStream_read(BitStream *, short bits);
#endif

/* Global Tables */
extern float	qtable[64];
extern int		zzseq[64];
extern short	dcbits[16], acbits[16];
extern char		dchuffval[12], achuffval[162];

/* Function Declarations */

#ifdef _NO_PROTO
void inithuffcode();
void encode();
void decode();
void decomp();
#else
void inithuffcode();
void encode(short *, BitStream *);
void decode(short *, BitStream *);
void decomp(BitStream *bs,CHARH *Image,long rows,long cols);
#endif
