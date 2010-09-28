/*****************************************************************************

  File:  stream_l.c

  Description: This file contains C routines making up the Stream Layer of the
               PDS Object Access Library.  The routines are:

               OalOpenStream
               OalCloseStream
               OalNewStreamDescriptor
               OalReadStream
               OalWriteStream
               OalSeek

               These Stream Layer routines do file I/O, and handle the 
               different record formats, record sizes, and record terminators
               found in PDS files.  They also deal with a limited set of
               discrepancies which may result when a file has been transferred
               across a network from a different machine.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:  14 Nov   1996

  History:

    Creation - This set of routines was part of the Alpha Release of the
               OA library.
    11/27/95 - Removed #include of stat.h on Mac. SM
    11/27/95 - OaOpenStream now retries fopen() with a lower-case file name if
               the first open attempt failed.  SM
    11/27/95 - Added support for OA_UNDEFINED_RECORD_TYPE to OaOpenStream,
               OaReadStream and OaSeek.  SM
    12/06/95 - Replaced malloc() by OaMalloc() throughout.  SM
    12/11/95 - Added error codes.  SM
    04/09/96 - Added macros FOPEN, FREAD, FGETS, FCLOSE, GETC, UNGETC. SM
    11/14/96 - Allowed OA_UNDEFINED_RECORD_TYPE in OaWriteStream and in
               OaOpenStream with read_write_mode = 'w'. SM

*****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stream_l.h"
#include "oamalloc.h"
#include "toolbox.h"

#ifdef XVT_DEF
#include "xvt.h"
#endif


#if (defined( VAX) || defined( ALPHA_VMS))
#include <stat.h>
#include <rms.h>
#include <unixio>
#include <file>
#else
#ifdef MAC
#include <unix.h>
#else
#include <sys/stat.h>
#endif
#endif

#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif

extern int oa_errno;

#ifdef _NO_PROTO
extern int OaReportError();
#else
extern int OaReportError( char *input_error_string);
#endif

/* So far there are 2 different file I/O systems from the Stream Layer's 
   point of view - OA_UNIX_IO and OA_VAX_IO.  For example, file I/O
   on a variable-length record file is transparent under VMS, because VMS's
   RMS file system supports it;  variable-length record I/O is not transparent
   on any other system, and has to be handled explicitly by Stream Layer code.
   Different blocks of code for different file I/O systems are selected by
   #ifdef's on OA_IO_TYPE, whose value is one of the following constants.
   The symbols the ifdef's test must be defined when compiling; in general
   they are NOT predefined by the compiler.
*/

#define OA_UNIX_IO 1
#define OA_VAX_IO  2

#if defined(ALPHA_OSF) || defined(SUN3) || defined(SUN4) || defined(SGI)
#define OA_IO_TYPE OA_UNIX_IO
#endif
#if defined(ULTRIX) || defined(MAC) || defined(IBM_PC) || defined(OSX)
#define OA_IO_TYPE OA_UNIX_IO
#endif
#if (defined( VAX) || defined( ALPHA_VMS))
#define OA_IO_TYPE OA_VAX_IO
#endif


/*****************************************************************************

  Routine: OalOpenStream
 
  Description: This routine initializes a stream structure to keep track of
               single file - file pointer, name, record type, current file
               position, etc, and opens the file.  If the file is to be read,
               then OalOpenStream does some tests to verify the record type
               and size parameters, and determines what kind of I/O to use: 
               fread(), read(), fgets() or special variable-length record
               code.
   
  Input:  
         filename        - An absolute or relative pathname suitable for 
                           fopen(), valid on the platform being run on.

         record_type     - Normally OA_STREAM, OA_FIXED_LENGTH, 
                           OA_VARIABLE_LENGTH or OA_UNDEFINED_RECORD_TYPE.

                           OA_UNKNOWN_RECORD_TYPE should be used when opening
                           a file for reading which is known to start with a
                           PDS label, or with a GIF file.  OalOpenStream reads
                           the first few bytes of the file, and if it looks
                           like a GIF file, then record_type will be set to
                           OA_GIF_FILE;  if not a GIF file, then the first two
                           bytes should be ASCII, since it's a label file;
                           if they're not, and the first two bytes look like a
                           variable-length record count LSB integer, then
                           record_type will be set to OA_VARIABLE_LENGTH;  
                           otherwise it will be set to OA_STREAM.

                           OA_UNDEFINED_RECORD_TYPE is used only for binary
                           stream files and is not recommended by PDS.
          
         record_bytes    - For fixed-length record files, this is the length
                           of each record; for variable-length record files, 
                           this is the length of the longest record; ignored
                           for stream files and undefined record files.
                           If read_write_mode is "w", and record_type is
                           OA_VARIABLE_LENGTH, then record_bytes must be an
                           even number.

         file_records    - This is an optional input which helps OalOpenStream
                           verify files which were transferred over a network 
                           from another machine. If unknown, or not applicable,
                           it should be set to 0.

         read_write_mode - "r" or "w" for read or write.

   Output: The fields in the stream structure are set and the file is
           opened.

   Notes:

   1) Record_type For Reads:
      OalOpenStream determines what kind of I/O will be used to read the file.
      For "RECORD_TYPE = FIXED_LENGTH", OalReadStream will generally read
        RECORD_BYTES bytes at a time using fread().
      For "RECORD_TYPE = STREAM", OalReadStream will use fgets().
      For "RECORD_TYPE = VARIABLE_LENGTH", OalReadStream will use read() under
        VMS, and on other platforms it will use custom variable-length record
        code which uses fread().
      For "RECORD_TYPE = UNDEFINED", OalReadStream will read BUFSIZ bytes at
        a time using fread().
      For OA_GIF_FILE record_type, special GIF-reading code will be used to
        read the file.
      When running under VMS, different I/O may be used depending on the
      actual VMS record type, which may be different than the record_type in
      the PDS label.

   2) Fgets and Line Terminator Characters:
      If read_write_mode is "r" and record_type is OA_STREAM, then fgets() is
      used to read the file;  fgets() reads characters until it reaches a 
      line terminator (CR on Mac, LF all others), stores the characters and
      the terminator in the buffer, and appends a '\0' at the end.  If the file
      contains the PDS recommended <CR><LF> terminators, and the file was
      transferred with text-mode ftp, then the <CR><LF> may have turned into a
      <CR><CR> if the file was ftp'd to a Mac, or into <LF><LF> if the file was
      ftp'd from a Mac.  OalReadStream solves this by doing a fgets(), 
      followed by a getc() to see if the next byte is a <LF> (on UNIX) or a
      <LF> or <CR> (on Mac).  If the next byte is a <CR> or <LF>, then it is
      added to the buffer and char count; otherwise it is replaced on the
      stream with ungetc().

   3) File Size Check For Fixed-length Records:
      If read_write_mode is "r", record_type is FIXED_LENGTH and file_records
      is non-zero (meaning the FILE_RECORDS keyword was present), then a check
      is done to see if the actual file size is a multiple of record_bytes.  
      (This check is not done on the Mac, because not all compilers provide
      fstat().)
      If the actual file size is less by FILE_RECORDS bytes, then OalOpenStream
      sets the OA_MISSING_LAST_RECORD_BYTE bit in stream_id->flags. 
      This situation can occur when an ASCII table is transferred from a VMS
      machine to another platform with binary-mode ftp - if the VMS file was
      variable-length with carriage-return carriage control record attributes,
      then the transferred file is probably missing a LF at the end of each
      record.  The OA_MISSING_LAST_RECORD_BYTE will cause OalReadStream to
      read record_bytes-1 bytes at a time, and append a LF at the end of each
      line, and return record_bytes bytes.

   4) Finding File and Modifying File Name:
      If read_write_mode is "r", filename does NOT have a ";1" extension, and 
      opening it fails, then OalOpenStream will append a ";1" extension and 
      try to open that.  This is a typical discrepancy found on CD-ROM's: 
      the actual data filename has a ";1" extension, but the label has the
      filename without the extension.  It will also try converting the file
      name part of the path to lower-case.

   5) VMS Specifics:
      The following complications arise only if we're reading a file under VMS:
      The actual VMS record type is obtained by an fstat() call, and stored in
      stream_id->VMS_record_type.  The I/O type used by OalReadStream depends
      on whether stream_id->record_type and stream_id->VMS_record_type match.
      a) If record_type is OA_VARIABLE_LENGTH and VMS_record_type is also
         OA_VARIABLE_LENGTH, then VMS's read() is used to read the file one
         variable-length record at a time.
      b) If record_type is OA_VARIABLE_LENGTH but VMS_record_type is
         OA_FIXED_LENGTH, then specially coded variable-length record I/O is 
         used to read the file, as is done on UNIX.  The file is probably a 
         fixed-length 512 byte record file transferred with binary ftp, and 
         contains variable-length record size integers (e.g. a Voyager
         image read off of CD-ROM on a Mac and transferred to a VMS machine).
      c) If record_type is OA_FIXED_LENGTH but VMS_record_type is
         OA_VARIABLE_LENGTH, then probably have an ASCII table which was
         transferred with text-mode ftp. When going from UNIX to VMS, 
         text-mode ftp strips out LF's and makes them part of the record
         attributes, then if the record size is odd, pads each record with one
         byte to make the record size even.  OalOpenStream checks if the
         actual file size is equal to RECORD_BYTES*FILE_RECORDS + 
         FILE_RECORDS*2 (the record count bytes),  and if not, issues a
         warning message.  Regardless of this check, OalReadStream will use 
         read(), and if read() returns RECORD_BYTES-1 bytes, it will append
         a LF and return RECORD_BYTES bytes.
      d) If record_type is OA_STREAM, then it doesn't matter what kind of VMS 
         record format the file has, and fgets() is used.
      e) If record_type is OA_FIXED_LENGTH, and VMS_record_type is also
         OA_FIXED_LENGTH, then it doesn't matter what the actual VMS record 
         size is;  OalReadStream will read RECORD_BYTES bytes at a time by
         default.  Note: fread() on VMS does read across record boundaries and
         part way into records; the buffering done by RMS is transparent.  
         Files transfered to a VMS machine with binary ftp end up as fixed-
         length, 512-byte record files.

*****************************************************************************/

#ifdef _NO_PROTO

struct OaStreamStruct *OalOpenStream( filename, record_type, record_bytes,
                                      file_records, read_write_mode)
char *filename;
int   record_type;
long  record_bytes;
long  file_records;
char *read_write_mode;

#else

struct OaStreamStruct *OalOpenStream( char *filename, int record_type, 
                                      long record_bytes, long file_records,
                                      char *read_write_mode)

#endif
{

struct OaStreamStruct *stream_id;
static char *proc_name = "OalOpenStream";
char mode[10], buf[ BUFSIZ], filename_buf[256], *ptr;
#ifndef MAC
long actual_file_size;
#endif

#if (defined( VAX) || defined( ALPHA_VMS))
stat_t stat_buffer;
char rms_keyword1[20], rms_keyword2[20];
#else
#ifndef MAC
struct stat stat_buffer;
#endif
#endif

/* Check inputs for validity. */

if (filename == NULL) {
  sprintf( error_string, "%s: filename is NULL", proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return( NULL);
}

if ((record_type != OA_FIXED_LENGTH) && (record_type != OA_VARIABLE_LENGTH) &&
    (record_type != OA_STREAM) && (record_type != OA_UNKNOWN_RECORD_TYPE) &&
    (record_type != OA_UNDEFINED_RECORD_TYPE)) {
  sprintf( error_string, "%s: invalid record_type: %d.", proc_name, 
           record_type);
  oa_errno = 520;
  OaReportError( error_string);
  return( NULL);
}

if ((read_write_mode[0] != 'r') && (read_write_mode[0] != 'w')) {
  sprintf( error_string, "%s: read_write_mode must be \"r\" or \"w\".",
           proc_name);
  oa_errno = 502;
  OaReportError( error_string);
  return( NULL);
}

if ((record_type == OA_UNKNOWN_RECORD_TYPE) &&
    (read_write_mode[0] == 'w')) {
  sprintf( error_string, "%s: can't create a file for write when ",
           proc_name);
  strcat( error_string, "record_type\nis OA_UNKNOWN_RECORD_TYPE ");
  oa_errno = 520;
  OaReportError( error_string);
  return( NULL);
}

/* VMS doesn't allow creation of fixed-length record files with an odd record 
   length.  */

#if (defined( VAX) || defined( ALPHA_VMS))
if (((record_type == OA_FIXED_LENGTH) &&
     (record_bytes % 2) != 0) && (read_write_mode[0] == 'w')) {
  sprintf( error_string, "%s: record_bytes must be an even number", proc_name);
  strcat( error_string, " for a FIXED_LENGTH\noutput file this platform.");
  oa_errno = 520;
  OaReportError( error_string);
  return( NULL);
}
#endif

if (read_write_mode[0] == 'r')
#if (defined(IBM_PC) || defined(MAC))
                        /* On an IBM-PC (with Borland C) use "read binary"   */
  strcpy( mode, "rb");  /* so that fread calls don't remove carriage-returns */
                        /* or stop when finding a Ctrl-Z. On a Mac do the    */
                        /* same so 0x0D (otherwise known as carriage return, */
                        /* <CR> and '\r') doesn't get converted to 0x0A      */
                        /* (otherwise known as new line, <LF> or '\n').      */
#else
  strcpy( mode, "r");  
#endif
else                    
#ifdef MAC             /* On a Mac, use "write binary" so that the fwrite  */
  strcpy( mode, "wb"); /* calls used to write out an object don't convert  */
#else                  /* <CR><LF> into <CR><CR>.                          */ 
  strcpy( mode, "w");   

#endif

/* Allocate a new stream structure and initialize it. */

stream_id = OalNewStreamDescriptor();
CopyString( stream_id->filename, filename);
stream_id->record_type = record_type;
stream_id->record_bytes = record_bytes;
if ((record_type == OA_STREAM) || (record_type == OA_UNDEFINED_RECORD_TYPE))
  stream_id->record_bytes = BUFSIZ;
stream_id->current_position = 0;
stream_id->fp = NULL;
stream_id->flags = 0;
stream_id->flags |= OA_IS_SEEKABLE;

/* Open the file.  If we're running under VMS creating a non-stream file for
   writing, add special VMS keywords needed for the fopen() call.  */

#if (defined( VAX) || defined( ALPHA_VMS))
if (mode[0] == 'w') {
  if (record_type == OA_STREAM) {
    stream_id->VMS_record_type = OA_VARIABLE_LENGTH;
  } else {  
  
    /*  We're opening a fixed-length, variable-length or undefined record format
        file for output, so add VMS keywords to fopen() call to create the output
        file with the appropriate attributes.  */
 
    stream_id->VMS_record_type = record_type;
    switch( record_type) {
      case OA_FIXED_LENGTH:
        strcpy( rms_keyword1, "rfm = fix");
        sprintf( rms_keyword2, "mrs = %ld", (long) record_bytes);
      break;
      case OA_VARIABLE_LENGTH:
        strcpy( rms_keyword1, "rfm = var");
        sprintf( rms_keyword2, "mrs = %ld", (long) record_bytes);
      break;
      case OA_UNDEFINED_RECORD_TYPE:
        strcpy( rms_keyword1, "rfm = udf");
        strcpy( rms_keyword2, "rfm = udf");
      break;
    } /*  end switch on record_type  */

    if ((stream_id->fp = fopen( filename, mode, 
                                rms_keyword1, rms_keyword2)) == NULL) {
      sprintf( error_string, "%s: couldn't open %s for write.",
               proc_name, filename);
      oa_errno = 700;
      OaReportError( error_string);
      LemmeGo( stream_id->filename);
      LemmeGo( stream_id);
      return( NULL);
    }
  }  /* if opening non-STREAM file for write under VMS  */
}    /* if opening a file for write under VMS  */
#endif

/* If file wasn't opened above, open it.  */

if (stream_id->fp == NULL)
  stream_id->fp = FOPEN( filename, mode);


if ((stream_id->fp == NULL) && (mode[0] == 'r')) {

  /* If open for read failed, try lower-case filename, and try appending a ";1"
     extension.  (A typical discrepancy found on CD-ROM's is that the actual
     data filename has a ";1" extension, but the label has the filename without
     the extension.)  In preparation for this, set ptr to point to the start of
     the filename part of the path;  do this by detecting the last character
     in the path which matches a directory specification character.  */

  strcpy( filename_buf, filename);
  ptr = NULL;
  ptr = strrchr( filename_buf, '/');                     /* Assume UNIX.  */
#if (defined( VAX) || defined( ALPHA_VMS))
  ptr = strrchr( filename_buf, ']');                     /* Dir name?     */
  if (ptr == NULL) ptr = strrchr( filename_buf, ':');    /* Logical name? */
#endif
#ifdef IBM_PC
  ptr = strrchr( filename_buf, '\\');                    /* Dir name?     */
  if (ptr == NULL) ptr = strrchr( filename_buf, '/');    /* Also legal.   */
#endif
  if (ptr != NULL)
    ptr++;
  else
    ptr = filename_buf;

#ifdef XVT_DEF 
  stream_id->fp = FOPEN(ptr, mode);
#endif

  if (strchr( filename_buf, ';') == NULL) {            /* Try appending a   */
    strcat( filename_buf, ";1");                       /* ;1 extension      */
    if ((stream_id->fp = FOPEN( filename_buf, mode)) != NULL) {
      sprintf( error_string, 
              "%s: couldn't open %s, but opened %s",
               proc_name, filename, filename_buf);
      oa_errno = 950;
      OaReportError( error_string);
      AppendString( stream_id->filename, ";1");
    } else {                                           /* Try lower-case    */
      LowerCase( ptr);                                 /* and ;1 extension. */
      if ((stream_id->fp = FOPEN( filename_buf, mode)) != NULL) {
        sprintf( error_string, 
                "%s: couldn't open %s, but opened %s",
                proc_name, stream_id->filename, filename_buf);
        oa_errno = 950;
        OaReportError( error_string);
        LemmeGo( stream_id->filename);
        CopyString( stream_id->filename, filename_buf);
      }
    }
  }

  if (stream_id->fp == NULL) {                       /* Try lower case       */
    strcpy( filename_buf, stream_id->filename);      /* without ;1 extension.*/
    LowerCase( ptr);
    if ((stream_id->fp = FOPEN( filename_buf, mode)) != NULL) {
      sprintf( error_string, 
              "%s: couldn't open %s, but opened %s",
              proc_name, stream_id->filename, filename_buf);
      oa_errno = 950;
      OaReportError( error_string);
      LemmeGo( stream_id->filename);
      CopyString( stream_id->filename, filename_buf);
    }
  }
}

if (stream_id->fp == NULL) {
  sprintf( error_string, "%s: couldn't open %s ", proc_name, filename);
  oa_errno = 700;
  OaReportError( error_string);
  LemmeGo( stream_id->filename);
  LemmeGo( stream_id);
  return( NULL);
}

/* Now determine the true record_type for OA_UNKNOWN_RECORD_TYPE.  */

if (record_type == OA_UNKNOWN_RECORD_TYPE) {

  /* Assume the file starts with a PDS label, which might be in an attached 
     label file with variable-length records, or it's a GIF file.  As more
     external data formats are added to OAL, this check will become more
     extensive, and eliminate the reopen() call by buffering what's read.
     Read the first 6 bytes with fread(), which will work on all platforms.
     Check if the file starts with "GIF87a" or "GIF89a".
     If not, see if it's a variable-length record file, i.e. is the ASCII
     embedded in "records" which begin with a 2 byte variable-length record
     count?    If each variable-length record contains a single line of text
     of reasonable length, as would be expected in for a label, then the MSB
     byte should be zero.  If it's not zero, then assume it's some ASCII
     character, and the file is NOT a variable-length record file.  */
 
  if (FREAD( buf, (size_t) 1, (size_t) 6, stream_id->fp) != 6) {
    sprintf( error_string, "%s: FREAD() failed to read 6 bytes to determine",
             proc_name);
    strcat( error_string, " actual record type.");
    oa_errno = 700;
    OaReportError( error_string);
    FCLOSE( stream_id->fp);
    LemmeGo( stream_id->filename);
    LemmeGo( stream_id);
    return(NULL);
  }

  /* Is it a GIF file?  */

  if ((memcmp( buf, "GIF87a", 6) == 0) || (memcmp( buf, "GIF89a", 6) == 0)) {
    stream_id->record_type = OA_GIF_FILE;
    stream_id->record_bytes = BUFSIZ;
    stream_id->buf_siz = BUFSIZ;

  } else {

    /* Is it a variable-length record file?  */

    if (buf[1] == 0) {
      stream_id->record_type = OA_VARIABLE_LENGTH;
      stream_id->record_bytes = BUFSIZ - 2;
    } else {
      stream_id->record_type = OA_STREAM;
    }
    stream_id->buf_siz = BUFSIZ;
  }

  /* Allocate space for the buffer.  */

  if ((stream_id->buf = (char *) OaMalloc( (size_t) 
                                          stream_id->buf_siz + 20)) == NULL) {
    sprintf( error_string, "%s: OaMalloc failed! Out of memory!", proc_name);
    oa_errno = 720;
    OaReportError( error_string);
    exit(1);
    /*NOTREACHED*/
  }

  /* Reopen the file. */

  if ((stream_id->fp = freopen( stream_id->filename, mode,
                                stream_id->fp)) == NULL) {
    sprintf( error_string, "%s: couldn't freopen() %s.",
             proc_name, stream_id->filename);
    oa_errno = 700;
    OaReportError( error_string);
    LemmeGo( stream_id->filename);
    LemmeGo( stream_id);
    return( NULL);
  }
  return( stream_id);
}  /* end if record_type == OA_UNKNOWN_RECORD_TYPE  */


/* If running under VMS, and opening for reading, then see what kind of
   file it really is, and set VMS_record_type accordingly.  If the actual
   record type doesn't match the record type from the PDS label, issue a
   warning message.  */

#if (defined( VAX) || defined( ALPHA_VMS))
if (read_write_mode[0] == 'r') {
  if (fstat( fileno( stream_id->fp), &stat_buffer) == 0) {
    switch (stat_buffer.st_fab_rfm) {
      case FAB$C_FIX: 
        stream_id->VMS_record_type = OA_FIXED_LENGTH;
        strcpy( error_string, "actual record_type  = FIXED_LENGTH\n");
        sprintf( error_string + strlen( error_string), 
                              "record_size         = %d\n",
                 (int) stat_buffer.st_fab_mrs);
      break;
      case FAB$C_VAR: 
        stream_id->VMS_record_type = OA_VARIABLE_LENGTH;
        strcpy( error_string, "actual record_type  = VARIABLE_LENGTH\n");

        /* stat_buffer.st_fab_mrs (maximum record size) is always set to 0 for
           normal variable-length files, so max record length is not directly
           available. */

      break;
      default: 
        sprintf( error_string, "unknown stat_buffer.st_fab_rfm (record_type) = %d\n",
                 (int) stat_buffer.st_fab_rfm);
        sprintf( error_string + strlen( error_string), 
                               "stat.st_fab_mrs     = %d\n",
                 (int) stat_buffer.st_fab_mrs);
      break;
    }
    sprintf( error_string + strlen( error_string), 
                               "file bytes          = %ld\n",
             (long) stat_buffer.st_size);
    actual_file_size = (long) stat_buffer.st_size;
    sprintf( error_string + strlen( error_string),
                               "record attributes   = %d\n",
             (int) stat_buffer.st_fab_rat);
    oa_errno = 950;
#ifdef OA_DEBUG
    OaReportError( error_string);
#endif
  } else {
    sprintf( error_string, "%s: fstat returned error.", proc_name);
    LemmeGo( stream_id->filename);
    LemmeGo( stream_id);
    oa_errno = 700;
    OaReportError( error_string);
    return( NULL);
  }

  if ((record_type == OA_FIXED_LENGTH) && 
      (stream_id->VMS_record_type == OA_VARIABLE_LENGTH) &&
      (file_records > 0)) {

    /* If record_type is OA_FIXED_LENGTH but fstat says variable-length, then
       probably have an ASCII table which was transferred with text ftp.
       When going from UNIX to VMS, text ftp strips out LF's and makes
       them part of the record attributes, and then if the record size is odd,
       pads each record with one byte to make the record size even.  
       Check if the actual file size is equal to RECORD_BYTES*FILE_RECORDS + 
       FILE_RECORDS*2 (the record count bytes).  */

    if (actual_file_size == file_records * record_bytes + 2 * file_records) {

       /* The actual file size indicates that the records are in fact 
          RECORD_BYTES in length;  the question is whether the record length
          bytes say RECORD_BYTES, or if they say RECORD_BYTES-1 with the last
          byte a pad.  This is checked for in OalReadStream.  If read()
          returns RECORD_BYTES, then OalReadStream makes no modifications, and
          returns RECORD_BYTES as the number of bytes read.  If read() says
          it read RECORD_BYTES-1 bytes, then there was a pad byte, and
          OalReadStream adds a LF and returns RECORD_BYTES as the number of
          bytes read.  This situation comes about when an ASCII table with
          "RECORD_TYPE = FIXED_LENGTH" and RECORD_BYTES even, is transferred 
          with text-mode ftp to a VMS machine: ftp strips an LF off of the end of
          each "record", and adds a pad byte to make the record length even.
          The first case comes about when the file was created under VMS with
          standard I/O, which creates a variable-length record file, instead
          of specifying fixed-length attributes in the create or open call.  */

      ;  
    } else if (actual_file_size == file_records*record_bytes + file_records) {

      /* The actual file size is less by FILE_RECORDS bytes, so OalReadStream's
         read() will read RECORD_BYTES-1 bytes at a time and OalReadStream
         will append a LF and return RECORD_BYTES bytes.  */

      sprintf( error_string, "%s: Warning: file size is less than", proc_name);
      strcat( error_string, " FILE_RECORDS * RECORD_BYTES\n");
      strcat( error_string, "by exactly FILE_RECORDS bytes.");
      strcat( error_string, " OalReadStream will add an LF at the end of\n ");
      strcat( error_string, "each record;  assumed to have been stripped ");
      strcat( error_string, "off by text-mode ftp.");
      oa_errno = 901;
      OaReportError( error_string);

    } else if (actual_file_size != (file_records * record_bytes + 1)) {

      /* The actual file size doesn't make sense, (it might have been one
         byte off if platform likes even-sized files) so either the file is
         hopelessly messed up by multiple ftp transfers, or FILE_RECORDS is
         wrong, or ...?  Log an error message, and continue anyway, hoping
         for the best.  */

      sprintf( error_string, "%s: Warning: file size is not equal to ", 
               proc_name);
      strcat( error_string, " FILE_RECORDS * RECORD_BYTES.\n");
      strcat( error_string, "File may have been corrupted by network ");
      strcat( error_string, "transfer from another machine.");
      oa_errno = 901;
      OaReportError( error_string);
    }
  }
}       /* end if read_write_mode is "r" */
#endif  /* end ifdef VAX */


/* If not running under VMS, and opening for reading a fixed-length file, 
   check if the file size is record_bytes * file_records.  */

#if !defined( VAX) && !defined( ALPHA_VMS) && !defined( MAC)

if ((read_write_mode[0] == 'r') && 
    (record_type == OA_FIXED_LENGTH) &&
    (file_records != 0)) {
  if (fstat( fileno( stream_id->fp), &stat_buffer) == 0) {
    actual_file_size = (long) stat_buffer.st_size;
  } else {
    sprintf( error_string, "%s: fstat returned error.", proc_name);
    oa_errno = 700;
    OaReportError( error_string);
    LemmeGo( stream_id->filename);
    LemmeGo( stream_id);
    return( NULL);
  }

  /* When ftp'ing a VMS variable-length record file which has record attributes
     of carriage-return carriage control to UNIX, text-mode ftp adds a LF at 
     the end of each record, but binary-mode ftp doesn't.  This section of code
     tries to determine if the last byte is present or not;  it only works if
     record_bytes is even, since some platforms pad files to an even length. */

  if (actual_file_size == file_records*record_bytes - file_records) {

    /* The actual file size is less by FILE_RECORDS bytes, so OalReadStream's
       fread() will read RECORD_BYTES-1 bytes at a time, append a LF and
       return RECORD_BYTES bytes.  Issue a warning message and set the flag
       in the stream descriptor. */

    stream_id->flags |= OA_MISSING_LAST_RECORD_BYTE;
    sprintf( error_string, "%s: Warning: file size is less than", proc_name);
    strcat( error_string, " FILE_RECORDS * RECORD_BYTES\n");
    strcat( error_string, "by exactly FILE_RECORDS bytes.");
    strcat( error_string, " OalReadStream will add an LF at the end of\n ");
    strcat( error_string, "each record;  assumed to have been stripped ");
    strcat( error_string, "off by binary-mode ftp.");
    oa_errno = 901;
    OaReportError( error_string);

  } else if (actual_file_size != file_records*record_bytes) {

    /* The actual file size doesn't make sense, so either the file is
       hopelessly messed up by multiple ftp transfers, or FILE_RECORDS is
       wrong, or ...?  Log an error message, and continue anyway, hoping
       for the best.  */

    sprintf( error_string, "%s: Warning: file size is not equal to ", 
             proc_name);
    strcat( error_string, " FILE_RECORDS * RECORD_BYTES.\n");
    strcat( error_string, "File may have been corrupted by network ");
    strcat( error_string, "transfer from another machine.");
    oa_errno = 901;
    OaReportError( error_string);
  }
}       /* end if read_write_mode is "r" */
#endif  /* end if !defined( VAX) && !defined( ALPHA_VMS) && !defined( MAC)  */

/* Determine the size of stream_id->buf;  this will be used by the other
   Stream Layer functions as the number of stream bytes to deal with at a 
   time, i.e. as an argument in fgets(), fputs(), fread(), fwrite() etc.  */

switch( record_type) {
  case OA_VARIABLE_LENGTH:
    stream_id->buf_siz = record_bytes + 2;  /* Record_bytes doesn't include */
  break;                                    /* 2 byte record size count,    */
                                            /* which are explicitely stored */
                                            /* for custom variable-length   */
                                            /* record I/O.                  */
  case OA_FIXED_LENGTH:
    stream_id->buf_siz = record_bytes;
  break;
  case OA_STREAM:
  case OA_UNDEFINED_RECORD_TYPE:
    stream_id->buf_siz = BUFSIZ;
  break;
}

/* Allocate space for the buffer.  */

if ((stream_id->buf = (char *) OaMalloc( (size_t) stream_id->buf_siz + 20)) 
                      == NULL) {
  sprintf( error_string, "%s: OaMalloc failed! Out of memory!", proc_name);
  oa_errno = 720;
  OaReportError( error_string);
  exit(1);
  /*NOTREACHED*/
}

return( stream_id);
}



/*****************************************************************************

  Routine: OalReadStream
 
  Description: This routine reads bytes_to_read bytes from a file, and puts 
               them in buf.

  Input:     
         stream_id     - A pointer to a stream structure created by 
                         OalOpenStream.

         bytes_to_read - The number of bytes the caller wants read.
                         If bytes_to_read is less than 1, the Stream Layer 
                         reads stream_id->buf_siz bytes (usually record_length
                         or BUFSIZ bytes)  If stream_id->record_type is
                         VARIABLE_LENGTH, bytes_to_read is ignored and one
                         record is read in.

         buf_ptr       - If *buf_ptr is not NULL, *buf_ptr points to the
                         buffer the caller wants the data read into.

         file_offset   - The offset into the file at which to start reading; 
                         if variable-length, this is a record number; otherwise
                         it's a byte offset; both start at 0.
                         If -1, read starts at the current file position.  

   Output:
           bytes_read - The number of bytes actually read (variable-length 
                        record prefixes are not included in bytes_read, and
                        are not included in the output buffer).  
                        Reasons why bytes_read might not equal bytes_to_read:
                        1) If bytes_to_read was bigger than stream_id->buf_siz,
                           and the caller didn't supply a buffer in *buf_ptr, 
                           then bytes_read will be stream_id->buf_siz, not
                           bytes_to_read.
                        2) If stream_id->record_type is OA_FIXED_LENGTH, and
                           stream_id->current_position on entry isn't at the
                           beginning of a record (i.e. current_position mod
                           record_bytes is not equal to zero), then 
                           bytes_read will be the number of bytes from 
                           current_position to the end of the record.
                        3) If we're running under VMS, and stream_id->record_type
                           is OA_FIXED_LENGTH but stream_id->VMS_record_type is
                           OA_VARIABLE_LENGTH, then one variable-length
                           record is read.  Thus bytes_read might be less than,
                           equal to, or greater than bytes_to_read.
                        4) If stream_id->record_type is 
                           OA_UNDEFINED_RECORD_TYPE, and we're reading BUFSIZ
                           bytes near the end of the file, and get less than
                           BUFSIZ bytes.

           buf_ptr    - If *buf_ptr was NULL on input, *buf_ptr points to
                        stream_id->buf where the data was read into.  
                        The caller should NOT free this buffer!

           The returned value is 0 if more than 0 bytes were successfully
           read.  If the file pointer was at end-of-file when OalReadStream
           was called, 1 is returned.  If there was an error, -1 is returned,
           with an error message.

   Notes:  For additional information, see the notes in OalOpenStream
           describing the actions of OalReadStream.

*****************************************************************************/

#ifdef _NO_PROTO

int OalReadStream( stream_id, bytes_to_read, buf_ptr, file_offset,
                   bytes_read)
struct OaStreamStruct *stream_id;
long bytes_to_read;
char **buf_ptr;
long file_offset;
long *bytes_read;

#else

int OalReadStream( struct OaStreamStruct *stream_id, long bytes_to_read, 
                   char **buf_ptr, long file_offset, long *bytes_read)

#endif
{

static char *proc_name = "OalReadStream"; 
long pad_bytes, partial_record_bytes, file_bytes_to_read;
char *buf, *ptr;
int seek_result, next_char;
short current_record_length;    /* A short is 2 bytes on ALL platforms!    */
short endian_indicator=1;       /* Initialized to 1 to determine the value */
                                /* of *platform_is_little_endian below.    */
char *platform_is_little_endian = (char *) &endian_indicator;
/* If platform is little-endian (lsb first), then the 1 initialized in
   endian_indicator will be in the first byte, and *platform_is_little_endian
   will have the value TRUE (1), otherwise it will be FALSE (0).  A test is
   done on *platform_is_little_endian in the variable-length record section
   below to decide whether or not to swap the record count bytes.  */

/* Check inputs for validity. */

if (stream_id == NULL) {
  sprintf( error_string, "%s: stream_id is NULL.", proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return(-1);
}
if (buf_ptr == NULL) {
  sprintf( error_string, "%s: buf_ptr is NULL.", proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return(-1);
}
if (bytes_read == NULL) {
  sprintf( error_string, "%s: bytes_read is NULL.", proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return(-1);
}

/* Use either the user-defined buffer, or the stream_id's buffer. */

if (*buf_ptr != NULL) {
  buf = *buf_ptr;
} else {
  buf = stream_id->buf;
  *buf_ptr = stream_id->buf;
}

if (file_offset >= 0) {
  if ((seek_result = OalSeek( stream_id, file_offset)) != 0) {
    sprintf( error_string, "%s: OalSeek returned error: %d.",
             proc_name, seek_result);
    oa_errno = 700;
    OaReportError( error_string);
    *bytes_read = 0;
    return(-1);
  }
}

if (stream_id->record_type == OA_STREAM) {

  /* File has STREAM format, which means an ASCII file with record terminators
     suitable for fgets() on the platform being run on.  Each record should be
     delimited by a <LF> or <CR><LF> on UNIX and VMS, and by a <CR> or <CR><LF>
     on a Mac.  
     fgets() is used to read the file;  fgets() reads and transfers all chars
     up to and including a CR (Mac) or LF (all others):  if the file contains
     the <CR><LF> terminators recommended by PDS and the file was transferred
     with text-mode ftp, then the <CR><LF> may have turned into a <CR><CR> if
     the file was ftp'd to a Mac, or into <LF><LF> if the file was ftp'd from
     a Mac.  OalReadStream solves this by doing a fgets(), followed by a
     getc() to see if the next byte is a <CR> or <LF>.  If the next byte is a
     <CR> or <LF>, it is added to the buffer and the returned char count;
     otherwise it is replaced on the stream with ungetc().
     If the file doesn't have terminators fgets() can recognize, the user 
     should convert it before passing it to OA, otherwise fgets will read 
     buf_siz bytes each time, until it reaches EOF.  (The last fgets may be
     less than buf_siz bytes, and will return NULL, since it reached EOF, but
     there will still be valid data in buf.)  */

/* Get a record of data. */

  buf[0] = '\0';
  buf[ stream_id->buf_siz - 1] = '\0';
  FGETS( buf, (int) stream_id->buf_siz, stream_id->fp);
  *bytes_read = (long) strlen( buf);
  if (*bytes_read == 0) {
    if (feof( stream_id->fp))
      return(1);
    oa_errno = 902;
    return(-1);
  } else {
    stream_id->current_position += *bytes_read;
    if (*bytes_read < (stream_id->buf_siz-2))
      if ((next_char = FGETC( stream_id->fp)) != EOF) {
        if ((next_char == '\n') || (next_char == '\r')) {
          buf[ *bytes_read] = next_char;
          buf[ *bytes_read + 1] = '\0';
          *bytes_read = *bytes_read + 1;
          stream_id->current_position++;
        } else {
          UNGETC( next_char, stream_id->fp);
        }
      }      
    return(0);
  }
}    /* end if stream_id->record_type is OA_STREAM */



if (stream_id->record_type == OA_FIXED_LENGTH) {

#if (defined( VAX) || defined( ALPHA_VMS))
  if (stream_id->VMS_record_type == OA_VARIABLE_LENGTH) {

    /* We're running under VMS, file has VARIABLE_LENGTH records, but the
       RECORD_TYPE keyword in the label said FIXED_LENGTH; probably have an 
       ASCII table transferred with text-mode ftp to the VAX, which may have had
       its LF's stripped off.  Use read() to read a single record, letting VMS
       deal with the variable-length record count, and if the returned bytes_read
       is one less than stream_id->record_bytes, add a LF.  */

    *bytes_read = (long) read( fileno( stream_id->fp), stream_id->buf, 
                               (int) stream_id->record_bytes);
    if (*bytes_read > 0) {
      stream_id->current_position++;
      if (*bytes_read == stream_id->record_bytes - 1) {
        stream_id->buf[ *bytes_read] = '\n';
        *bytes_read = *bytes_read + 1;
      }
      return(0);
    } else {
      *bytes_read = 0;
      sprintf( error_string, 
               "%s: read error; failed to read variable-length record.",
               proc_name);
      oa_errno = 700;
      OaReportError( error_string);
      return(-1);
    }
  }  /* end if VMS_record_type == OA_VARIABLE_LENGTH  */
#endif

  /* File has FIXED_LENGTH format; if bytes_to_read wasn't specified, then 
     read exactly as many bytes as it takes to reach the end of a record; if
     current_position is at the beginning of a record (usual case), then 
     this will be record_bytes bytes.  But if an OalSeek() call has positioned
     partially into a record, then read only until the end of the record.
     This works the same for all platforms. (Under VMS, fread() reads across 
     record boundaries and partially into records, so doesn't matter if the
     file is actually a VMS/RMS fixed-length or variable-length file.)
     If bytes_to_read was specified, then ignore all the above and assume
     caller knows what they're doing, except if bytes_to_read is greater than
     stream_id->buf_siz and *buf was NULL on entry - in this case read only 
     stream_id->buf_siz bytes.  */

  if (bytes_to_read < 1) {
    if ((stream_id->flags & OA_MISSING_LAST_RECORD_BYTE) == 0)
      partial_record_bytes = stream_id->current_position % 
                             stream_id->record_bytes;
    else
      partial_record_bytes = stream_id->current_position % 
                             (stream_id->record_bytes - 1);
    file_bytes_to_read = stream_id->record_bytes - partial_record_bytes;
  } else {
    file_bytes_to_read = bytes_to_read;
  }

  /* If we're using the internal stream buffer and file_bytes_to_read is 
     greater than the stream buffer's size, use the stream buffer's size
     instead.  */

  if ((buf == stream_id->buf) && (file_bytes_to_read > stream_id->buf_siz))
     file_bytes_to_read = stream_id->buf_siz;

  *bytes_read = (long) FREAD( buf, (size_t) 1, (size_t) file_bytes_to_read, 
                              stream_id->fp);

  if ((ferror( stream_id->fp) != 0) || (*bytes_read != file_bytes_to_read)) {
    if (feof( stream_id->fp))
		return(1);
    sprintf( error_string, "%s: FREAD returned error\n", proc_name);
    sprintf( error_string + strlen( error_string),
             "file_bytes_to_read = %ld, bytes_read = %ld\n",
             file_bytes_to_read, *bytes_read);
    sprintf( error_string + strlen( error_string),
             "stream_id->current_position = %ld\n", 
             stream_id->current_position);
    oa_errno = 700;
    OaReportError( error_string);
    return(-1);
  }
  stream_id->current_position += *bytes_read;
  if (((stream_id->flags & OA_MISSING_LAST_RECORD_BYTE) != 0) &&
       (bytes_to_read < 0))
    buf[ stream_id->record_bytes-1] = '\n';
  return(0);

} /* end if stream_id->record_type is OA_FIXED_LENGTH */




if (stream_id->record_type == OA_UNDEFINED_RECORD_TYPE) {

  /* If bytes_to_read wasn't specified, then use fread() to read
     stream_id->buf_siz bytes, which is set to BUFSIZ in OaOpenStream. 
     (Under VMS, fread() reads across record boundaries and partially into
     records, so doesn't matter if the file is actually a VMS/RMS
     fixed-length, variable-length or undefined record format file.)
     If bytes_to_read was specified, then assume caller knows what they're
     doing, except if bytes_to_read is greater than stream_id->buf_siz and
     *buf was NULL on entry - in this case read only stream_id->buf_siz
     bytes.  */

  if (bytes_to_read < 1)
    file_bytes_to_read = stream_id->buf_siz;
  else
    file_bytes_to_read = bytes_to_read;

  /* If we're using the internal stream buffer and file_bytes_to_read is 
     greater than the stream buffer's size, use the stream buffer's size
     instead.  */

  if ((buf == stream_id->buf) && (file_bytes_to_read > stream_id->buf_siz))
     file_bytes_to_read = stream_id->buf_siz;

  *bytes_read = 0;
  *bytes_read = (long) FREAD( buf, (size_t) 1, (size_t) file_bytes_to_read, 
                              stream_id->fp);

  if (*bytes_read == 0) {
    if (feof( stream_id->fp))
      return(1);
    sprintf( error_string, "%s: FREAD returned 0 bytes\n", proc_name);
    sprintf( error_string + strlen( error_string),
             "file_bytes_to_read = %ld\n", file_bytes_to_read);
    sprintf( error_string + strlen( error_string),
             "stream_id->current_position = %ld\n", 
             stream_id->current_position);
    oa_errno = 902;
    OaReportError( error_string);
    return(-1);
  }
  stream_id->current_position += *bytes_read;
  return(0);

} /* end if stream_id->record_type is OA_UNDEFINED_RECORD_TYPE */




if (stream_id->record_type == OA_VARIABLE_LENGTH) {

#if (defined( VAX) || defined( ALPHA_VMS))
  if (stream_id->VMS_record_type == OA_VARIABLE_LENGTH) {

  /* We're running under VMS, and file has VARIABLE_LENGTH records, so can use
     read() to read a single record, letting VMS deal with the variable-length
     record count.  */

    *bytes_read = (long) read( fileno( stream_id->fp), stream_id->buf, 
                               (int) stream_id->record_bytes);
    if (*bytes_read > 0) {
      stream_id->current_position++;
      return(0);
    } else {
      *bytes_read = 0;
      if (feof( stream_id->fp)) {
        return(1);
      } else {
        sprintf( error_string, 
                 "%s: read error; failed to read variable-length record.",
                 proc_name);
        oa_errno = 700;
        OaReportError( error_string);
        return(-1);
      }
    }
  } /* end if running under VMS and VMS_record_type == OA_VARIABLE_LENGTH  */
#endif

  /* The record_type is OA_VARIABLE_LENGTH, and if we're running under VMS, then
     file is NOT a VMS variable-length record file, but probably a 512 fixed-length
     record file (this is what you get when you read a Voyager HFD compressed
     image off a CD-ROM on a Mac, then ftp it to a VMS machine).  In any case, 
     assume each record has embedded variable-length record count integers,
     with a pad byte at the end if odd length. */

  /* Read the 2 record-length bytes first. */

  *bytes_read = (long) FREAD( stream_id->buf, (size_t) 1, (size_t) 2, 
                 stream_id->fp);

  if (*bytes_read != 2) {
    if (feof( stream_id->fp))
      return(1);
    *bytes_read = 0;
    sprintf( error_string, 
             "%s: FREAD() error; didn't read 2 record length bytes.\n", 
             proc_name);
    sprintf( error_string + strlen( error_string), 
             "stream_id->current_position = %ld.",
              stream_id->current_position);
    oa_errno = 900;
    OaReportError( error_string);
    return(-1);
  }

  if (*platform_is_little_endian == FALSE) {

    /* The record count is always LSB, so swap bytes to make a MSB which can
       be used on this big-endian platform.  */

    ptr = (char *) &current_record_length;
    ptr[0] = stream_id->buf[1];
    ptr[1] = stream_id->buf[0];
  } else {
    current_record_length = *((short *) stream_id->buf);
  }

  if ((current_record_length < 0) || 
      (current_record_length > stream_id->record_bytes)) {
    sprintf( error_string, 
             "%s: invalid current variable-length record size: %d\n",
             proc_name, (int) current_record_length);
    sprintf( error_string + strlen( error_string), 
             " stream_id->record_bytes = %ld, ", stream_id->record_bytes);
    sprintf( error_string + strlen( error_string), 
             " stream_id->current_position = %ld.",
             stream_id->current_position);
    oa_errno = 700;
    OaReportError( error_string);
    return(-1);
  }

  /* If the record length is odd, there's an extra pad byte which isn't 
     counted in the record length; need to read this one too.  */

  if ((current_record_length % 2) != 0)
    pad_bytes = 1;
  else
    pad_bytes = 0;

  /* Read in the bytes. */

  *bytes_read = (long) FREAD( buf, (size_t) 1, 
                              (size_t) current_record_length + pad_bytes, 
                              stream_id->fp);

  if (*bytes_read != current_record_length + pad_bytes) {
    if (feof( stream_id->fp))
      return(1);
    *bytes_read = 0;
    sprintf( error_string, "%s: FREAD error(); didn't read %d bytes.\n", 
             proc_name, (int) (current_record_length + pad_bytes));
    sprintf( error_string + strlen( error_string), 
             "stream_id->current_position = %ld.",
              stream_id->current_position);
    oa_errno = 700;
    OaReportError( error_string);
    return(-1);
  }
  *bytes_read = *bytes_read - pad_bytes; /* Don't count the pad byte, if any.*/
  stream_id->current_position++;

  return(0);    
}  /* end if record_type == OA_VARIABLE_LENGTH  */
return(0);
}



/*****************************************************************************

  Routine: OalWriteStream
 
  Description: This routine writes to a file.  The exact action taken 
               depends on the record type of the file and the platform being
               run on.  See below.

  Input: 
         stream_id      - Points to a stream descriptor returned by
                          OaOpenStream, and stream_id->record_type and
                          stream_id->record_bytes are set.

         bytes_to_write - Can be greater than record_bytes for OA_FIXED_LENGTH,
                          but must be less than or equal to record_bytes (the
                          maximum record size) for OA_VARIABLE_LENGTH.  It can
                          be anything for OA_UNDEFINED_RECORD_TYPE and
                          OA_STREAM.

         buf            - Points to the user's buffer, and contains
                          bytes_to_write bytes.  For STREAM files, the 
                          the PDS recommended record terminators <CR><LF>
                          should be at the end of buf, and included in
                          bytes_to_write, as OalWriteStream does NOT add them.

        bytes_written   - Points to a long.


   Output:  Bytes are written to the file starting at the current file
            pointer position and bytes_written is set accordingly.

            OA_STREAM: bytes_to_write bytes are written to the file. 
                       No <CR><LF> or any other terminator are added - PDS 
                       considers these part of the object data, so they're
                       assumed to be there already.  E.g. an ASCII TABLE's 
                       ROW_BYTES must include the <CR><LF> terminators 
                       according to the PDS Standards Document).
                       bytes_written is set to bytes_to_write.

            OA_UNDEFINED_RECORD_TYPE: bytes_to_write bytes are written to the
                                      file.  bytes_written is set to
                                      bytes_to_write.

            OA_FIXED_LENGTH: at least bytes_to_write are written to the file.
                             If bytes_to_write is greater than record_bytes, 
                             multiple writes of record_bytes are performed, 
                             until the remaining bytes is less than record
                             bytes, then zeros are then appended to the
                             remaining bytes to make record_bytes, and the
                             record is written to the file.
                             If bytes_to_write is less than record_bytes, then
                             they are padded to record_bytes as above, and
                             then written.  bytes_written is set to the total
                             bytes written, and will always be divisible by
                             record_bytes.

            OA_VARIABLE_LENGTH: bytes_to_write bytes, plus a byte to make the
                                total number of bytes even, if necessary, are
                                written to the file as a variable-length
                                record.  This is transparent on VMS, and done
                                explicitly on other platforms.  On VMS, 
                                bytes_written is set to bytes_to_write. 
                                On other platforms, bytes_written is set to
                                the actual number of bytes written, which is 
                                bytes_to_write + 2 + 1 pad byte if 
                                bytes_to_write was odd.

   Notes:

*****************************************************************************/

#ifdef _NO_PROTO

int OalWriteStream( stream_id, bytes_to_write, buf, bytes_written)
struct OaStreamStruct *stream_id;
long bytes_to_write;
char *buf;
long *bytes_written;

#else

int OalWriteStream( struct OaStreamStruct *stream_id, long bytes_to_write, 
                    char *buf, long *bytes_written)

#endif
{
static char *proc_name = "OalWriteStream";
long remaining_bytes, pad_bytes, result, records_to_write;
short current_record_length;    /* A short is 2 bytes on ALL platforms!    */
char *ptr=NULL;
short endian_indicator=1;       /* Initialized to 1 to determine the value */
                                /* of *platform_is_little_endian below.    */
char *platform_is_little_endian = (char *) &endian_indicator;
/* If platform is little-endian (lsb first), then the 1 initialized in
   endian_indicator will be in the first byte, and *platform_is_little_endian
   will have the value TRUE (1), otherwise it will be FALSE (0).  A test is
   done on *platform_is_little_endian in the variable-length record section
   below to decide whether or not to swap the record count bytes.  */


if (stream_id == NULL) {
  sprintf( error_string, "%s: error: stream_id is NULL.", proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  *bytes_written = (long) 0;
  return(-1);
}

if ((buf == NULL) || (bytes_to_write < 1)) {
  sprintf( error_string, 
           "%s: error: buf is NULL and/or bytes_to_write is < 1.",
           proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  *bytes_written = (long) 0;
  return(-1);
}

if (bytes_written == NULL) {
  sprintf( error_string, "%s: bytes_written is NULL.", proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  *bytes_written = (long) 0;
  return(-1);
}

switch (stream_id->record_type) {
  case OA_STREAM:
    switch( OA_IO_TYPE) {
      case OA_UNIX_IO:
        *bytes_written = (long) fwrite( buf, (size_t) 1,
                                       (size_t) bytes_to_write,
                                        stream_id->fp);
        if (*bytes_written != bytes_to_write) {
          sprintf( error_string, "%s: fwrite error; didn't write %ld bytes.", 
                   proc_name, bytes_to_write);
          oa_errno = 700;
          OaReportError( error_string);
          return(-1);
        }
        stream_id->current_position += *bytes_written;
        return(0);
      /*NOTREACHED*/
      break;
      case OA_VAX_IO:

        /* Assumptions: we're writing to a variable-length record file with
           NO record attributes, because the <CR><LF> should be included in
           the data, and don't want it repeated when use ftp to xfer the file.  
           Loop through bytes_to_write, finding '\n' to identify lines, and 
           issuing a fwrite() to write each line.  */

        *bytes_written = 0;
        ptr = buf;
        while (*bytes_written < bytes_to_write) {
          while ((*ptr != '\n') && (ptr < (buf + bytes_to_write))) ptr++;
          if (ptr < (buf + bytes_to_write)) {
            current_record_length = (ptr - buf - *bytes_written + 1);
            result = (long) fwrite( buf + *bytes_written,
                                    current_record_length,
                                    (size_t) 1,
                                    stream_id->fp);
            if (result == 1) {
              *bytes_written = *bytes_written + current_record_length;
              stream_id->current_position += current_record_length;
              ptr++;  /* Advance one byte past the '\n'. */
            } else {
              sprintf( error_string, 
                       "%s: fwrite error; didn't write %d bytes.",
                       proc_name, (int) current_record_length);
              oa_errno = 700;
              OaReportError( error_string);
              return(-1);
            }
          } else {
            sprintf( error_string,
            "%s input buffer must be terminated by a '\n' for a STREAM file.",
              proc_name);
            oa_errno = 520;
            OaReportError( error_string);
            return(-1);
          }
        }  /* while bytes remain to be written */
        return(0);
      /*NOTREACHED*/
      break;
      default:
        sprintf( error_string, "%s: not yet implemented for this platform.", 
                 proc_name);
        oa_errno = 520;
        OaReportError( error_string);
        *bytes_written = (long) 0;
        return(-1);
      /*NOTREACHED*/
      break;
    }      /* end switch on OA_IO_TYPE */
  break; /* end case OA_STREAM */

  case OA_UNDEFINED_RECORD_TYPE:
    *bytes_written = (long) fwrite( buf, (size_t) 1,
                                   (size_t) bytes_to_write,
                                    stream_id->fp);
    if (*bytes_written != bytes_to_write) {
      sprintf( error_string, "%s: fwrite error; didn't write %ld bytes.", 
               proc_name, bytes_to_write);
      oa_errno = 700;
      OaReportError( error_string);
      return(-1);
    }
    stream_id->current_position += *bytes_written;
    return(0);
  break; /* end case OA_UNDEFINED_RECORD_TYPE */

  case OA_FIXED_LENGTH:
    switch( OA_IO_TYPE) {
      case OA_UNIX_IO:

        /* Write bytes_to_write with a single fwrite(), then write pad bytes 
           if necessary to complete a record.  */

        *bytes_written = (long) fwrite( buf, (size_t) 1,
                                       (size_t) bytes_to_write,
                                        stream_id->fp);
        if (*bytes_written != bytes_to_write) {
          sprintf( error_string, "%s: fwrite error; didn't write %ld bytes.", 
                   proc_name, bytes_to_write);
          oa_errno = 700;
          OaReportError( error_string);
          return(-1);
        }
        stream_id->current_position += *bytes_written;

        if (bytes_to_write < stream_id->record_bytes)
          pad_bytes = stream_id->record_bytes - bytes_to_write;
        else
          pad_bytes = bytes_to_write % stream_id->record_bytes;

        if (pad_bytes > 0) {
          memset( stream_id->buf, 0, (size_t) pad_bytes);
          result = (long) fwrite( stream_id->buf, (size_t) 1, 
                                  (size_t) pad_bytes, stream_id->fp);
          if (result != pad_bytes) {
            sprintf( error_string, "%s: fwrite error; didn't write %ld bytes.",
                     proc_name, (long) pad_bytes);
            oa_errno = 700;
            OaReportError( error_string);
            return(-1);
          }
          *bytes_written = *bytes_written + pad_bytes;
          stream_id->current_position += pad_bytes;
        }
        return(0);
      /*NOTREACHED*/
      break;

      case OA_VAX_IO:

        /* Assumptions: OalWriteStream is writing to a fixed-length record 
           file with NO record attributes, because don't want ftp to add
           <LF>; terminators, if ASCII, these should be included in the data.
           Loop through bytes_to_write, writing stream_id->record_bytes
           each time, until have written bytes_to_write.  If bytes_to_write
           isn't a multiple of stream_id->record_bytes, pad the last record
           with NULL bytes.  */

        *bytes_written = 0;
        ptr = buf;
        records_to_write = bytes_to_write / stream_id->record_bytes;
        
        while (records_to_write > 0) {
          result = (long) fwrite( buf + *bytes_written,
                                  stream_id->record_bytes,
                                  (size_t) 1,
                                  stream_id->fp);
          if (result == 1) {
            *bytes_written = *bytes_written + stream_id->record_bytes;
            stream_id->current_position += stream_id->record_bytes;
            records_to_write--;
          } else {
            sprintf( error_string, "%s: fwrite error; didn't write %ld bytes.",
                     proc_name, stream_id->record_bytes);
            oa_errno = 700;
            OaReportError( error_string);
            return(-1);
          }
        }

        /* If there are remaining bytes to write, copy them to the stream
           structure's buffer, and pad the rest of the buffer with 0's and
           write the record.  */

        remaining_bytes = bytes_to_write - *bytes_written;
        if (remaining_bytes > 0) {
          memcpy( stream_id->buf, buf + *bytes_written, 
                 (size_t) remaining_bytes);
          memset( stream_id->buf + remaining_bytes, 0,
                  (size_t) (stream_id->record_bytes - remaining_bytes));
          result = (long) fwrite( stream_id->buf,
                                  stream_id->record_bytes,
                                  (size_t) 1,
                                  stream_id->fp);
          if (result == 1) {
            *bytes_written = *bytes_written + stream_id->record_bytes;
            stream_id->current_position += stream_id->record_bytes;
          } else {
            sprintf( error_string, "%s: fwrite error; didn't write %ld bytes.",
                     proc_name, stream_id->record_bytes);
            oa_errno = 700;
            OaReportError( error_string);
            return(-1);
          }          
        }
        return(0);
      /*NOTREACHED*/
      break;
      default:
        sprintf( error_string, "%s: not yet implemented for this platform.", 
                 proc_name);
        oa_errno = 520;
        OaReportError( error_string);
        *bytes_written = (long) 0;
        return(-1);
      /*NOTREACHED*/
      break;
    }      /* end switch on OA_IO_TYPE */
  break; /* end case OA_FIXED_LENGTH */

  case OA_VARIABLE_LENGTH:
    if (bytes_to_write > stream_id->record_bytes) {
      sprintf( error_string, 
               "%s: bytes_to_write must be less than record_bytes.", 
               proc_name);
      oa_errno = 502;
      OaReportError( error_string);
      return(-1);
    }
    switch( OA_IO_TYPE) {
      case OA_VAX_IO:  /* Under VMS, so write a VMS variable-length record. */
        result = (long) fwrite( buf,
                                bytes_to_write,
                                (size_t) 1,
                                stream_id->fp);
        if (result == 1) {
          *bytes_written = bytes_to_write;
          stream_id->current_position += 1;
        } else {
          sprintf( error_string, "%s: fwrite error; didn't write %ld bytes.", 
                   proc_name, (long) bytes_to_write);
          oa_errno = 700;
          OaReportError( error_string);
          return(-1);
        }
      break;
      default:  /* Not under VMS, so write bytes to look like a VMS variable
                   length record. */

        /* Put the 2-byte record count at the beginning of the buffer. The
           record count must be in LSB format, and does NOT include the pad
           byte to make the record length even if bytes_to_write is odd.  */

        current_record_length = (short) bytes_to_write;
        if (*platform_is_little_endian == FALSE) {
          ptr = (char *) &current_record_length;
          stream_id->buf[0] = ptr[1];
          stream_id->buf[1] = ptr[0];
        } else {
          stream_id->buf[0] = ptr[0];
          stream_id->buf[1] = ptr[1];
        }

        /* Put the data into the buffer after the 2-byte record count. */

        memcpy( stream_id->buf + 2, buf, bytes_to_write);

        /* If bytes_to_write is odd, then add a pad byte with value 0 at the 
           end of the data.  */

        if ((bytes_to_write % 2) != 0) {
          current_record_length = (short) bytes_to_write + 1;
          stream_id->buf[2 + bytes_to_write + 1] = 0;
        } else {
          current_record_length = (short) bytes_to_write;
        }
        current_record_length += 2;  /* Include the 2 record length bytes. */
        *bytes_written = (long) fwrite( stream_id->buf, (size_t) 1,
                                       (size_t) current_record_length,
                                        stream_id->fp);
        if (*bytes_written != current_record_length) {
          sprintf( error_string, "%s: fwrite error; didn't write %d bytes.", 
                   proc_name, (int) current_record_length);
          oa_errno = 700;
          OaReportError( error_string);
          return(-1);
        }
        stream_id->current_position += 1;
      break;  /* case any other IO_TYPE besides OA_VAX_IO */
    }       /* end switch on IO_TYPE */
  break;  /* end case OA_VARIABLE_LENGTH */
}       /* end switch on record_type */
return(0);
}
                 


/*****************************************************************************

  Routine: OalCloseStream
 
  Description: This routine closes the file and deallocates the stream 
               descriptor structure.

  Input: 
         stream_id      - Points to a stream descriptor.

  Output: If stream_id->fd wasn't NULL, the file descriptor is closed.
          The stream structure is freed.

  Notes:

*****************************************************************************/

#ifdef _NO_PROTO

int OalCloseStream( stream_id)
struct OaStreamStruct *stream_id;

#else

int OalCloseStream( struct OaStreamStruct *stream_id)

#endif
{
  if (stream_id->fp != NULL) FCLOSE( stream_id->fp);
  LemmeGo( stream_id->buf);
  LemmeGo( stream_id);
  return(0);
}



/*****************************************************************************

  Routine: OalNewStreamDescriptor
 
  Description: This function allocates an OaStreamStruct structure,
               initializes it, and returns a pointer to it.

  Input: 

  Output:  The function returns a pointer to an OaStreamStruct structure.

  Notes:  

*****************************************************************************/
extern struct OaStreamStruct *
                 OalNewStreamDescriptor() {

static char *proc_name = "OalNewStreamDescriptor";
struct OaStreamStruct *stream_id;

/* Allocate a new stream structure and initialize it to all 0's. */

if ((stream_id = (struct OaStreamStruct *) 
                  OaMalloc( sizeof( struct OaStreamStruct))) == NULL) {
  sprintf( error_string, "%s: OaMalloc failed! Out of memory!", proc_name);
  oa_errno = 720;
  OaReportError( error_string);
  exit(1);
  /*NOTREACHED*/
}
memset( (char *) stream_id, 0, sizeof( struct OaStreamStruct));
return( stream_id);
}



/*****************************************************************************

  Routine: OalSeek
 
  Description: OalSeek positions the file pointer in an input file.
               It shouldn't be used to position in output files.

  Input: 

         stream_id   - A valid stream descriptor, as returned by OalOpenStream.
                       If the OA_IS_SEEKABLE bit in stream_id->flags is set, 
                       then OalSeek is allowed to seek forward and backward 
                       and call rewind();  otherwise it does reads to seek
                       forward, and returns an error if asked to seek
                       backward.
         file_offset - If variable-length, this is a record number, otherwise
                       it's a byte position; both record numbers and byte
                       positions start at 0.  

  Output:  stream_id->fp has been positioned to the desired location; the next
           call to OalReadStream will start at this location.
           OalSeek returns 0 if successful, 1 on end-of-file (no error 
           message), and -1 if there was an error (with an error message).

  Notes:  
  1) If record_type is OA_STREAM, then OalSeek will work regardless of 
     whether the record terminators (CR,LF) have been changed.  It will NOT
     work if any terminators have been stripped out;  this would happen if
     you used binary ftp to transfer a file from a VMS machine to some other
     platform and the VMS file had its LF's implicit in the VMS carriage-return 
     carriage control record attribute.  Text-mode ftp should always be used 
     for STREAM files.
  2) If on VMS, if VMS_record_type is OA_VARIABLE_LENGTH and record_type is
     OA_FIXED_LENGTH or OA_STREAM, then use OalSeek uses fread() to the
     position, since fseek() doesn't work.  The file is probably an ASCII 
     table transferred with text-mode ftp; each line has had its LF stripped 
     out and replaced by the VMS carriage-return carriage control record 
     attribute.

*****************************************************************************/

#ifdef _NO_PROTO

int OalSeek( stream_id, file_offset)
struct OaStreamStruct *stream_id;
long file_offset;

#else

int OalSeek( struct OaStreamStruct *stream_id, long file_offset)

#endif
{
static char *proc_name = "OalSeek";
long pad_bytes, seek_bytes, record_number, bytes_to_read;
size_t bytes_read;
int is_seekable;
short current_record_length;    /* A short is 2 bytes on ALL platforms!    */
char *ptr;
short endian_indicator=1;       /* Initialized to 1 to determine the value */
                                /* of *platform_is_little_endian below.     */
char *platform_is_little_endian = (char *) &endian_indicator;
/* If platform is little-endian (lsb first), then the 1 initialized in
   endian_indicator will be in the first byte, and *platform_is_little_endian
   will have the value TRUE (1), otherwise it will be FALSE (0).  A test is
   done on *platform_is_little_endian in the variable-length record section
   below to decide whether or not to swap the record count bytes.  */

if (stream_id->current_position == file_offset)
  return(0);

is_seekable = (((stream_id->flags & OA_IS_SEEKABLE) != 0) ? TRUE : FALSE);

switch( stream_id->record_type) {

  case OA_STREAM:
  case OA_FIXED_LENGTH:
  case OA_UNDEFINED_RECORD_TYPE:

    /* Seek_bytes is the number of bytes the target is away from the current 
       file position (can be positive, negative or zero).  */

    seek_bytes = file_offset - stream_id->current_position;

#if (defined( VAX) || defined( ALPHA_VMS))

/* Seeking is problematic under VMS, so force use of fread().  */

    if (seek_bytes < 0) {
      if (is_seekable == FALSE) {
        sprintf( error_string, "%s: can't seek backward because ",
                 proc_name);
        strcat( error_string, 
                "OA_IS_SEEKABLE bit in stream_id->flags isn't set");
        oa_errno = 520;
        OaReportError( error_string);
        return(-1);
      }
      if (rewind( stream_id->fp) != 0) {
        sprintf( error_string, "%s: rewind error.", proc_name);
        oa_errno = 700;
        OaReportError( error_string);
        return(-1);
      }
      seek_bytes = stream_id->current_position - seek_bytes;
      stream_id->current_position = 0;
    }
    is_seekable = FALSE;  /* Force usage of fread() instead of fseek(). */
#endif

    if (is_seekable == TRUE) {
      fseek( stream_id->fp, seek_bytes, SEEK_CUR);
      stream_id->current_position += seek_bytes;

    } else {                  /* Can't use fseek, so use fread.  */

      if (seek_bytes < 0) {
        sprintf( error_string, "%s: can't seek backwards because ",
                 proc_name);
        strcat( error_string, 
                "OA_IS_SEEKABLE bit in stream_id->flags isn't set");
        oa_errno = 520;
        OaReportError( error_string);
        return(-1);
      }

      /* Call fread() in increments of buf_siz until have less than buf_siz 
         to go, then read the remainder with the last fread().  On VMS, this
         reads across records for all supported record types (fixed-length, 
         variable-length and stream_LF).  */

      while (stream_id->current_position != file_offset) {
        if ((file_offset - stream_id->current_position) >=
             stream_id->buf_siz)
          bytes_to_read = stream_id->buf_siz;
        else
          bytes_to_read = file_offset - stream_id->current_position;
        bytes_read = FREAD( stream_id->buf, (size_t) 1,
                           (size_t) bytes_to_read,
                            stream_id->fp);
        if (bytes_read != (size_t) bytes_to_read) {
          if (feof( stream_id->fp))
            return(1);
          sprintf( error_string, 
                   "%s: FREAD error; didn't read %ld bytes.", 
                   proc_name, bytes_to_read);
          oa_errno = 700;
          OaReportError( error_string);
          return(-1);
        }
        stream_id->current_position += (long) bytes_read;
      }  /* end while (stream_id->current_position != file_offset) */
    }  /* end else is_seekable is FALSE, so use fread(). */
  break;


  case OA_VARIABLE_LENGTH:
    record_number = file_offset;
#if (defined( VAX) || defined( ALPHA_VMS))
    if (stream_id->VMS_record_type == OA_VARIABLE_LENGTH) {
      if (record_number < stream_id->current_position) {
        if (is_seekable == FALSE) {
          sprintf( error_string, "%s: can't seek backwards because ",
                   proc_name);
          strcat( error_string, 
                  "OA_IS_SEEKABLE bit in stream_id->flags isn't set");
          oa_errno = 520;
          OaReportError( error_string);
          return(-1);
        }
        if (rewind( stream_id->fp) != 0) {
          sprintf( error_string, "%s: rewind error.", proc_name);
          oa_errno = 700;
          OaReportError( error_string);
          return(-1);        
        }
        stream_id->current_position = 0;
      }
      while (stream_id->current_position < record_number) {
        bytes_read = read( fileno( stream_id->fp), stream_id->buf, 
                           (int) stream_id->record_bytes);
        if (bytes_read <= 0) {
          sprintf( error_string, 
                   "%s: read error; failed to read variable-length record.",
                   proc_name);
          oa_errno = 700;
          OaReportError( error_string);
          return(-1);
        }
        stream_id->current_position++;
      }
      return(0);
    }  /* end if VMS_record_type is OA_VARIABLE_LENGTH */
#endif

    ptr = (char *) &current_record_length;
    if (record_number < stream_id->current_position) {

      /* Desired record is before current record, so go to beginning of file,
         then read the number of records needed to get to the desired 
         record.  */

      if (is_seekable == FALSE) {
        sprintf( error_string, "%s: can't seek backwards because ",
                 proc_name);
        strcat( error_string, 
                "OA_IS_SEEKABLE bit in stream_id->flags isn't set");
        oa_errno = 520;
        OaReportError( error_string);
        return(-1);
      }
      fseek( stream_id->fp, 0, SEEK_SET);
      stream_id->current_position = 0;
    }

    while (stream_id->current_position != record_number) {

      /* Read in the 2-byte record count. */

      bytes_read = FREAD( stream_id->buf, (size_t) 1, (size_t) 2, 
                          stream_id->fp);
      if (bytes_read != 2) {
        if (feof( stream_id->fp))
          return(1);
        bytes_read = 0;
        sprintf( error_string, "%s: FREAD error; didn't read 2 bytes.", 
                 proc_name);
        oa_errno = 700;
        OaReportError( error_string);
        return(-1);
      }

      if (*platform_is_little_endian == FALSE) {

        /* The record count is always LSB, so swap bytes to make a MSB which 
           can be used on this big-endian platform.  */

        ptr = (char *) &current_record_length;
        ptr[0] = stream_id->buf[1];
        ptr[1] = stream_id->buf[0];
      } else {
        current_record_length = *((short *) stream_id->buf);
      }

      if ((current_record_length < 0) || 
          (current_record_length > stream_id->record_bytes)) {
        sprintf( error_string, 
                 "%s: invalid current variable-length record size: %d\n",
                 proc_name, (int) current_record_length);
        sprintf( error_string + strlen( error_string), 
                 " stream_id->record_bytes = %ld, ", stream_id->record_bytes);
        sprintf( error_string + strlen( error_string), 
                 " stream_id->current_position = %ld.",
                 stream_id->current_position);
        oa_errno = 700;
        OaReportError( error_string);
        return(-1);
      }

      /* If the record length is odd, there's an extra pad byte which isn't 
         counted in the record length; need to read this one too.  */

      if ((current_record_length % 2) != 0)
        pad_bytes = 1;
      else
        pad_bytes = 0;

      /* Read in the bytes which make up this variable-length record. */

      bytes_read = FREAD( stream_id->buf, (size_t) 1, 
                         (size_t) current_record_length + pad_bytes, 
                          stream_id->fp);

      if (bytes_read != (size_t) (current_record_length + pad_bytes)) {
        if (feof( stream_id->fp))
          return(1);
        sprintf( error_string, "%s: FREAD error; didn't read %d bytes.", 
                 proc_name, (int) (current_record_length + pad_bytes));
        sprintf( error_string + strlen( error_string), 
                 " stream_id->current_position = %ld, bytes_read = %ld",
                 stream_id->current_position, (long) bytes_read);
        oa_errno = 700;
        OaReportError( error_string);
        return(-1);
      }
      stream_id->current_position++;
    }  /* end while not yet at record_number */
  break;
  default:
    sprintf( error_string, "%s: unknown record_type: %d.", 
             proc_name, stream_id->record_type);
    oa_errno = 502;
    OaReportError( error_string);
    return(-1);  
    /*NOTREACHED*/
  break;
}  /* end switch on stream_id->record_type */
return(0);
}
