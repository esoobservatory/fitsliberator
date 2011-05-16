/*****************************************************************************

  File:  stream_l.h

  Description: This file is the include file for stream_l.c.
               It defines data structures and enumerated types, and declares
               all the functions making up the Stream Layer.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   6 Dec   1994
  Last Modified:  16 Mar   1998

  History:

    Creation - This include file was part of the Beta Release of the
               OA library.
    11/27/95 - Added OA_UNDEFINED_RECORD_TYPE to oa_record_type_enum.  SM
    04/09/96 - Added macros FOPEN, FREAD, FGETS, FCLOSE, GETC, UNGETC. SM
    03/16/98 - Added OA_GIF_FILE to oa_record_type_enum.  SM

*****************************************************************************/

#ifndef OA_STREAM_L_INCLUDED
#define OA_STREAM_L_INCLUDED 1

/* Globally visible variable: */

extern char error_string[];    

/* This structure is the stream descriptor used by the Stream Layer for each 
   open file. */
struct OaStreamStruct {
  char *filename;         /* Copy of file name in dynamic memory.           */
  int  record_type;       /* OA_FIXED_LENGTH, OA_VARIABLE_LENGTH, OA_STREAM,*/
                          /* OA_UNDEFINED_RECORD_TYPE, OA_GIF_FILE,         */
                          /* OA_UNKNOWN_RECORD_TYPE                         */
  int  VMS_record_type;   /* OA_FIXED_LENGTH, OA_VARIABLE_LENGTH, OA_STREAM */
  long record_bytes;      /* Longest record if OA_VARIABLE_LENGTH.          */
  FILE *fp;               /* File pointer for I/O.                          */
  char *buf;              /* Pointer to a buffer.                           */
  long buf_siz;           /* Size of the buffer.                            */
  long current_position;  /* The record number (OA_VARIABLE_LENGTH) or byte */
                          /* offset (OA_STREAM, OA_FIXED_LENGTH and         */
                          /* OA_UNDEFINED_RECORD_TYPE) where next read or   */
                          /* write will start; starts with 0.               */
  int  flags;             /* Zero or more bits may be set; the bits are:    */
                          /* OA_IS_SEEKABLE, OA_MISSING_LAST_RECORD_BYTE    */
};

/* These are the file record types OAL knows about.  */
enum oa_record_type_enum { OA_UNKNOWN_RECORD_TYPE = 0,  OA_FIXED_LENGTH, 
                           OA_STREAM, OA_VARIABLE_LENGTH,
                           OA_UNDEFINED_RECORD_TYPE, OA_GIF_FILE};

/* This enumeration masks bits in OaStreamStruct's flags field.  When new
   values are added, they must be multiples of 2, i.e. 4,8,16 etc. */
enum oa_stream_flags_enum { OA_IS_SEEKABLE               = 1,
                            OA_MISSING_LAST_RECORD_BYTE  = 2};
                            

/* Macros for low-level I/O.  These are normally set to their system routine
   equivalents, but can be changed by users to hook application-specific
   I/O routines below the OA library stream layer.  These macros are used only
   in the stream layer, and in code directly related to object data (e.g. 
   clmdcmp2.c).  The macros are NOT used for HISTORY write/read and label
   writes.  */

#define FOPEN( filename, type)             fopen( filename, type)
#define FREAD( ptr, size, n_items, stream) fread( ptr, size, n_items, stream)
#define FGETS( s, n, stream)               fgets( s, n, stream)
#define FGETC( stream)                     fgetc( stream)
#define UNGETC( c, stream)                 ungetc( c, stream)
#define FCLOSE( stream)                    fclose( stream)


/* Stream Layer functions:  */

#ifdef _NO_PROTO

extern int       OalCloseStream();
extern struct OaStreamStruct *
                 OalNewStreamDescriptor();
extern struct OaStreamStruct *
                 OalOpenStream();
extern int       OalReadStream();
extern int       OalSeek();
extern int       OalWriteStream();

#else

extern int       OalCloseStream( struct OaStreamStruct *stream_id);
extern struct OaStreamStruct *
                 OalNewStreamDescriptor( void);
extern struct OaStreamStruct *
                 OalOpenStream( char *filename,
                                int record_type,
                                long record_bytes,
                                long file_records,
                                char *read_write_mode);
extern int       OalReadStream( struct OaStreamStruct *stream_id,
                                long bytes_to_read,
                                char **buf_ptr,
                                long file_offset,
                                long *bytes_read);
extern int       OalSeek( struct OaStreamStruct *stream_id, long file_offset);
extern int       OalWriteStream( struct OaStreamStruct *stream_id,
                                 long bytes_to_write,
                                 char *buf,
                                 long *bytes_written);
#endif

#endif  /* #ifndef OA_STREAM_L_INCLUDED */
