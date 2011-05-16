/*****************************************************************************

  File:  oa_gif.c

  Description: This file contains the routines OalCreateODLTreeFromGif,
               OalReadImageFromGif, and associated helper routines.  
               OalCreateODLTreeFromGif is called by OaParseLabelFile when 
               OalOpenStream detects a Gif file and returns OA_GIF_FILE in
               stream_id->record_type.  A label is created for an UNDEFINED
               record type file, with image and color table pointer offsets 
               specified in bytes.  Each image's offset is the start of the
               compressed image data, not the image header which preceeds it.
               OalReadImageFromGif is called by OaReadImage when it finds 
               the keyword ENCODING_TYPE = "GIF", which was put there by
               OalCreateODLTreeFromGif.  The byte offsets of the color
               table(s) are specified in the ODL tree, and can be read in
               like any other table by OAL, since they aren't compressed.

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  16 Mar   1998
  Last Modified:  16 Mar   1998

  History:

    Creation - This file was added for OAL Version 1.3.  SM

*****************************************************************************/

/* The GIF-reading routines in this file were modified from netpbm       */
/* freeware, whose copyright notice appears below.                       */
/* +-------------------------------------------------------------------+ */
/* | Copyright 1990, 1991, 1993, David Koblas.  (koblas@netcom.com)    | */
/* |   Permission to use, copy, modify, and distribute this software   | */
/* |   and its documentation for any purpose and without fee is hereby | */
/* |   granted, provided that the above copyright notice appear in all | */
/* |   copies and that both that copyright notice and this permission  | */
/* |   notice appear in supporting documentation.  This software is    | */
/* |   provided "as is" without express or implied warranty.           | */
/* +-------------------------------------------------------------------+ */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "oal.h"

#ifndef SEEK_SET
#define SEEK_SET 0
#endif

#define MAXCOLORMAPSIZE  256
#define CM_RED         0
#define CM_GREEN       1
#define CM_BLUE        2

#define MAX_LWZ_BITS  12

#define INTERLACE      0x40
#define LOCALCOLORMAP  0x80

#define BitSet(byte, bit)        (((byte) & (bit)) == (bit))
#define LM_to_uint(a,b)          (((b)<<8)|(a))

static struct {
       unsigned int    Width;
       unsigned int    Height;
       unsigned char   ColorMap[3][MAXCOLORMAPSIZE];
       unsigned int    BitPixel;
       unsigned int    ColorResolution;
       unsigned int    Background;
       unsigned int    AspectRatio;
       int             GrayScale;
} GifScreen;

static struct {
       int     transparent;
       int     delayTime;
       int     inputFlag;
       int     disposal;
} Gif89 = { -1, -1, -1, 0 };

unsigned long total_bytes_read=0;

#ifdef _NO_PROTO
#define ARGS(alist) ()
#else
#define ARGS(alist) alist
#endif

/* These are declared in oal.h.

ODLTREE OalCreateODLTreeFromGif ARGS(( FILE *fd, char *filename ));
OA_OBJECT OalReadImageFromGif ARGS(( char *data_filename, long file_offset,
                                     int lines, int line_samples,
                                     ODLTREE image_node ));
*/

static int OalGifReadColorMap ARGS(( FILE *fd, int number,
                                   unsigned char buffer[3][MAXCOLORMAPSIZE]));
static int OalGifDoExtension ARGS(( FILE *fd, int label ));
static int OalGifGetDataBlock ARGS(( FILE *fd, unsigned char  *buf ));
static int OalGifGetCode ARGS(( FILE *fd, int code_size, int flag ));
static int OalGifLWZReadByte ARGS(( FILE *fd, int flag, int input_code_size ));
static unsigned char * OalGifReadImage ARGS(( FILE *fd, int len, int height,
                                              int interlace, int ignore ));
static int OalGifReadOK ARGS(( FILE *fd, unsigned char *buf, int len ));
void OalGifError ARGS(( char *error_string));
void OalGifMsg ARGS(( char *error_string));
void OalGifAddTableObj ARGS (( ODLTREE root_node, char *table_name,
                               char *filename, unsigned long byte_offset,
                               int rows ));
void OalGifAddImageObj ARGS (( ODLTREE root_node, char *image_name,
                               char *filename, unsigned long byte_offset,
                               int lines, int line_samples, int interlace ));
int OaReportError ARGS (( char *input_error_string ));

extern int oa_errno;
extern char error_string[];


/*****************************/
/*  OalCreateODLTreeFromGif  */
/*****************************/

ODLTREE OalCreateODLTreeFromGif( FILE *fd, char *filename)
/*FILE *fd;
char *filename;
commented out for solaris compiler   DWS 09-26-02*/
{
unsigned char   buf[16];
unsigned char   c;
unsigned char   localColorMap[3][MAXCOLORMAPSIZE];
int             grayScale = 0;
int             useGlobalColormap;
int             bitPixel;
int             imageCount = 0;
int             local_color_table_count = 0;
char            version[4];
long            rows, lines, line_samples, interlace;
ODLTREE         root_node;
char            kwd_name[64];
char *          label_filename = NULL, data_filename = NULL;
int             record_type = 0, record_bytes = 0, file_records = 0, file_offset = 0;
int             object_interchange_format = 0;
char *          proc_name="OalCreateODLTreeFromGif";

total_bytes_read = 0;   

root_node = OdlNewObjDesc( "ROOT", NULL, NULL, NULL, NULL, filename,
                           (short) 0, (long) 0);
OaStrtoKwdValue(  "PDS_VERSION_ID", root_node, "PDS3");
OaStrtoKwdValue(  "RECORD_TYPE", root_node, "UNDEFINED");

if (! OalGifReadOK(fd,buf,6)) {
  sprintf( error_string, "%s: error reading magic number", proc_name );
  OalGifError( error_string);
  return( NULL);
}
if (strncmp((char *)buf,"GIF",3) != 0) {
  sprintf( error_string, "%s: not a GIF file", proc_name);
  OalGifError( error_string);
  return( NULL);
}
strncpy(version, (char *)buf + 3, 3);
version[3] = '\0';

if ((strcmp(version, "87a") != 0) && (strcmp(version, "89a") != 0)) {
  sprintf( error_string, "%s: bad version number, not '87a' or '89a'",
           proc_name);
  OalGifError( error_string);
}

if (! OalGifReadOK(fd,buf,7)) {
  sprintf( error_string, "%s: failed to read screen descriptor", proc_name);
  OalGifError( error_string);
  return( NULL);
}

GifScreen.Width           = LM_to_uint(buf[0],buf[1]);
GifScreen.Height          = LM_to_uint(buf[2],buf[3]);
GifScreen.BitPixel        = 2<<(buf[4]&0x07);

rows = GifScreen.BitPixel;  /* rows in GLOBAL_COLOR_TABLE */

GifScreen.ColorResolution = (((unsigned char) (buf[4] & 0x70) >> 3) + 1);
GifScreen.Background      = buf[5];
GifScreen.AspectRatio     = buf[6];

if (BitSet(buf[4], LOCALCOLORMAP)) { /* Global Colormap follows */

  OalGifAddTableObj( root_node, "GLOBAL_COLOR_TABLE",
                     filename, total_bytes_read+1, rows);

  if (OalGifReadColorMap(fd,GifScreen.BitPixel,GifScreen.ColorMap)) {
    sprintf( error_string, "%s: error reading global colormap", proc_name);
    OalGifError( error_string);
    return( NULL);
  }
}

if (GifScreen.AspectRatio != 0 && GifScreen.AspectRatio != 49) {
  sprintf( error_string, "%s: warning - non-square pixels", proc_name);
  OalGifMsg( error_string);
}

for (;;) {
  if (! OalGifReadOK(fd,&c,1)) {
    sprintf( error_string, "%s: EOF or read error on image data", proc_name);
    OalGifError( error_string);
    return( NULL);
  }

  if (c == ';') {         /* GIF terminator */
    return( root_node);
  }

  if (c == '!') {         /* Extension */
    if (! OalGifReadOK(fd,&c,1)) {
      sprintf( error_string, 
               "%s: EOF or read error on extension function code",
               proc_name);
      OalGifError( error_string);
      return( NULL);
    }
    OalGifDoExtension(fd, c);
    continue;
  }

  if (c != ',') {         /* Not a valid start character */
    sprintf( error_string, "%s: bogus character 0x%02x, ignoring", proc_name,
             (int) c );
    OalGifMsg( error_string);
    continue;
  }

  ++imageCount;

  if (! OalGifReadOK(fd,buf,9)) {
    sprintf( error_string, "%s: couldn't read left/top/width/height",
             proc_name);
    OalGifError( error_string);
    return( NULL);
  }
  useGlobalColormap = ! BitSet(buf[8], LOCALCOLORMAP);

  bitPixel = 1<<((buf[8]&0x07)+1);
  rows = bitPixel;  /* rows in local color table */

  if (! useGlobalColormap) {
    local_color_table_count++;
    if (local_color_table_count == 1)
      sprintf( kwd_name, "LOCAL_%-d_COLOR_TABLE",
               local_color_table_count);
    else
      strcpy( kwd_name, "LOCAL_COLOR_TABLE");

     OalGifAddTableObj( root_node, kwd_name,
                        filename, total_bytes_read+1, rows);

    if (OalGifReadColorMap(fd, bitPixel, localColorMap)) {
      sprintf( error_string, "%s: error reading local colormap",
               proc_name);
      OalGifError( error_string);
      return( NULL);
    }
  }

  lines = LM_to_uint(buf[6],buf[7]);
  line_samples = LM_to_uint(buf[4],buf[5]);
  interlace = BitSet( buf[8], INTERLACE);

  if (imageCount == 1)
    strcpy( kwd_name, "GIF_IMAGE");
  else
    sprintf( kwd_name, "GIF_%-d_IMAGE", imageCount);
  OalGifAddImageObj( root_node, kwd_name, filename, total_bytes_read+1, 
                     lines, line_samples, (int) interlace);
  OalGifReadImage(fd, line_samples, lines, (int) interlace, TRUE);

}
}


/*************************/
/*  OalReadImageFromGif  */
/*************************/

OA_OBJECT OalReadImageFromGif(char *data_filename, long file_offset, int lines,
										int line_samples, ODLTREE image_node)


/*
char *data_filename;
long file_offset;
int lines;
int line_samples;
ODLTREE image_node;
 commented out for solaris compiler  DWS 09-26-02*/
{

long interlace = 0, remaining_bytes_to_read = 0;
int bytes_to_read = 0, bytes_read = 0;
FILE *fd;
unsigned char *image = NULL, buf[512] = {0};
char *str, *proc_name="OalReadImageFromGif";
OA_OBJECT oa_object;

OaKwdValuetoLong( "INTERLACE", image_node, &interlace);
if (OaKwdValuetoStr( "ENCODING_TYPE", image_node, &str) != 0) {
  sprintf( error_string, "%s: Couldn't find ENCODING_TYPE = GIF!",
           proc_name);
  oa_errno = 531;
  OaReportError( error_string);
  return( NULL);
}
if (strcmp( str, "GIF") != 0) {
  sprintf( error_string, "%s: ENCODING_TYPE must be GIF!",
           proc_name);
  oa_errno = 520;
  OaReportError( error_string);
  return( NULL);
}

if ((fd = fopen( data_filename, "rb")) == NULL) {
  sprintf( error_string, "%s: couldn't open %s ", proc_name,
           data_filename);
  oa_errno = 700;
  OaReportError( error_string);
  return( NULL);
}


#if (defined( VAX) || defined( ALPHA_VMS))

/* Seeking is problematic under VMS, so use of fread().  */

/* Call fread() in increments of 512 (standard VMS fixed-length
   record size for binary ftp'ed files) until have less than 512 
   to go, then read the remainder with the last fread().  On VMS, this
   reads across records for all supported record types (fixed-length, 
   variable-length and stream_LF).  */

remaining_bytes_to_read = file_offset;
while (remaining_bytes_to_read > 0) {
  if (remaining_bytes_to_read >= 512)
    bytes_to_read = 512;
  else
    bytes_to_read = remaining_bytes_to_read;
  bytes_read = fread( buf, (size_t) 1, (size_t) bytes_to_read, fd);
  if (bytes_read != (size_t) bytes_to_read) {
    sprintf( error_string, 
             "%s: fread error; didn't read %d bytes.", proc_name,
             bytes_to_read);
    oa_errno = 700;
    OaReportError( error_string);
    fclose( fd);
    return(NULL);
  }
  remaining_bytes_to_read -= bytes_read;
}  /* end while (remaining_bytes_to_read > 0) */

#else

if (fseek( fd, file_offset, SEEK_SET) != 0) {
  sprintf( error_string, "%s: fseek returned error!", proc_name);
  oa_errno = 700;
  OaReportError( error_string);
  fclose( fd);
  return( NULL);
}

#endif


image = OalGifReadImage(fd, line_samples, lines, (int) interlace, FALSE);
fclose( fd);

if (image != NULL) {

  /* Create an OA_OBJECT to encapsulate the image.  */

  oa_object = OaNewOaObject();
  oa_object->odltree = OaCopyTree( image_node, 0);
  OaDeleteKwd( "INTERLACE", oa_object->odltree);
  OaDeleteKwd( "ENCODING_TYPE", oa_object->odltree);
  oa_object->data_ptr = (PTR) image;
  oa_object->size = lines * line_samples;
  oa_object->is_in_memory = TRUE;
  oa_object->profile = Oa_profile;
  oa_object->stream_id = NULL;
  return( oa_object);

} else {
  return( NULL);
}
}



static int
OalGifReadColorMap(FILE *fd, int number,unsigned char buffer[3][MAXCOLORMAPSIZE])
/*FILE           *fd;
int            number;
unsigned char  buffer[3][MAXCOLORMAPSIZE];
 commented out for solaris compiler  DWS  09-26-02*/
{
char           *proc_name="OalGifReadColorMap";

       int             i;
       unsigned char   rgb[3];

       for (i = 0; i < number; ++i) {
               if (! OalGifReadOK(fd, rgb, sizeof(rgb))) {
                       sprintf( error_string, "%s: bad colormap", proc_name);
                       OalGifError( error_string);
 	       }
               buffer[CM_RED][i] = rgb[0] ;
               buffer[CM_GREEN][i] = rgb[1] ;
               buffer[CM_BLUE][i] = rgb[2] ;

       }
       return 0;
}

static int
OalGifDoExtension(FILE *fd, int label)
/*FILE   *fd;
int    label;
 commented out for solaris compiler  DWS  09-26-02*/
{
       static char     buf[256];
       char            *str = NULL;
       int             showComment;

       showComment = FALSE;
       switch (label) {
       case 0x01:              /* Plain Text Extension */
               /*str = "Plain Text Extension";*/
#ifdef notdef
               if (OalGifGetDataBlock(fd, (unsigned char*) buf) == 0)
                       ;

               lpos   = LM_to_uint(buf[0], buf[1]);
               tpos   = LM_to_uint(buf[2], buf[3]);
               width  = LM_to_uint(buf[4], buf[5]);
               height = LM_to_uint(buf[6], buf[7]);
               cellw  = buf[8];
               cellh  = buf[9];
               foreground = buf[10];
               background = buf[11];

               while (OalGifGetDataBlock(fd, (unsigned char*) buf) != 0) {
                       image[ypos][xpos] = (unsigned char) v;
                       ++index;
               }

               return FALSE;
#else
               break;
#endif
       case 0xff:              /* Application Extension */
               /*str = "Application Extension";*/
               break;
       case 0xfe:              /* Comment Extension */
               /*str = "Comment Extension";*/
               while (OalGifGetDataBlock(fd, (unsigned char*) buf) != 0) {
                       if (showComment) {
                               sprintf( error_string, "gif comment: %s", buf );
                               OalGifMsg( error_string);
		       }
               }
               return FALSE;
       case 0xf9:              /* Graphic Control Extension */
               /*str = "Graphic Control Extension";*/
               (void) OalGifGetDataBlock(fd, (unsigned char*) buf);
               Gif89.disposal    = (buf[0] >> 2) & 0x7;
               Gif89.inputFlag   = (buf[0] >> 1) & 0x1;
               Gif89.delayTime   = LM_to_uint(buf[1],buf[2]);
               if ((buf[0] & 0x1) != 0)
                       Gif89.transparent = buf[3];

               while (OalGifGetDataBlock(fd, (unsigned char*) buf) != 0)
                       ;
               return FALSE;
       default:
               /*str = buf;*/
               sprintf(buf, "UNKNOWN (0x%02x)", label);
               break;
       }

       while (OalGifGetDataBlock(fd, (unsigned char*) buf) != 0)
               ;

       return FALSE;
}

int    ZeroDataBlock = FALSE;

static int
OalGifGetDataBlock(FILE *fd, unsigned char *buf)
/*FILE           *fd;
unsigned char  *buf;
 commented out for solaris compiler  DWS  09-26-02*/

{
       char           *proc_name="OalGifGetDataBlock";
       unsigned char   count;

       if (! OalGifReadOK(fd,&count,1)) {
               sprintf( error_string, "%s: error in getting DataBlock size",
                        proc_name);
               OalGifError( error_string);
               return -1;
       }

       ZeroDataBlock = count == 0;

       if ((count != 0) && (! OalGifReadOK(fd, buf, count))) {
               sprintf( error_string, "%s: error in reading DataBlock",
                        proc_name);
               OalGifMsg( error_string);
               return -1;
       }

       return count;
}

static int
OalGifGetCode(FILE *fd, int code_size, int flag)
/*FILE   *fd;
int    code_size;
int    flag;
 commented out for solaris compiler  DWS  09-26-02*/

{
       static unsigned char    buf[280];
       static int              curbit, lastbit, done, last_byte;
       int                     i, j, ret;
       unsigned char           count;
       char                    *proc_name="OalGifGetCode";
       if (flag) {
               curbit = 0;
               lastbit = 0;
               done = FALSE;
               return 0;
       }

       if ( (curbit+code_size) >= lastbit) {
               if (done) {
                       if (curbit >= lastbit) {
                               sprintf( error_string, 
                                        "%s: ran off the end of my bits",
                                        proc_name);
                               OalGifError( error_string);
		        }
                        return -1;
               }
               buf[0] = buf[last_byte-2];
               buf[1] = buf[last_byte-1];

               if ((count = OalGifGetDataBlock(fd, &buf[2])) == 0)
                       done = TRUE;

               last_byte = 2 + count;
               curbit = (curbit - lastbit) + 16;
               lastbit = (2+count)*8 ;
       }

       ret = 0;
       for (i = curbit, j = 0; j < code_size; ++i, ++j)
               ret |= ((buf[ i / 8 ] & (1 << (i % 8))) != 0) << j;

       curbit += code_size;

       return ret;
}

static int
OalGifLWZReadByte(FILE *fd, int flag, int input_code_size)
/*FILE   *fd;
int    flag;
int    input_code_size;
 commented out for solaris compiler  DWS  09-26-02*/
{
       char            *proc_name="OalGifLWZReadByte";
       static int      fresh = FALSE;
       int             code, incode;
       static int      code_size, set_code_size;
       static int      max_code, max_code_size;
       static int      firstcode, oldcode;
       static int      clear_code, end_code;
       static int      table[2][(1<< MAX_LWZ_BITS)];
       static int      stack[(1<<(MAX_LWZ_BITS))*2], *sp;
       register int    i;

       if (flag) {
               set_code_size = input_code_size;
               code_size = set_code_size+1;
               clear_code = 1 << set_code_size ;
               end_code = clear_code + 1;
               max_code_size = 2*clear_code;
               max_code = clear_code+2;

               OalGifGetCode(fd, 0, TRUE);
               
               fresh = TRUE;

               for (i = 0; i < clear_code; ++i) {
                       table[0][i] = 0;
                       table[1][i] = i;
               }
               for (; i < (1<<MAX_LWZ_BITS); ++i)
                       table[0][i] = table[1][0] = 0;

               sp = stack;

               return 0;
       } else if (fresh) {
               fresh = FALSE;
               do {
                       firstcode = oldcode =
                               OalGifGetCode(fd, code_size, FALSE);
               } while (firstcode == clear_code);
               return firstcode;
       }

       if (sp > stack)
               return *--sp;

       while ((code = OalGifGetCode(fd, code_size, FALSE)) >= 0) {
               if (code == clear_code) {
                       for (i = 0; i < clear_code; ++i) {
                               table[0][i] = 0;
                               table[1][i] = i;
                       }
                       for (; i < (1<<MAX_LWZ_BITS); ++i)
                               table[0][i] = table[1][i] = 0;
                       code_size = set_code_size+1;
                       max_code_size = 2*clear_code;
                       max_code = clear_code+2;
                       sp = stack;
                       firstcode = oldcode =
                                       OalGifGetCode(fd, code_size, FALSE);
                       return firstcode;
               } else if (code == end_code) {
                       int             count;
                       unsigned char   buf[260];

                       if (ZeroDataBlock)
                               return -2;

                       while ((count = OalGifGetDataBlock(fd, buf)) > 0)
                               ;

                       if (count != 0) {
                         sprintf( error_string,
                         "%s: missing EOD in data stream (common occurence)",
                                  proc_name);
                         OalGifError( error_string);
		       }
                       return -2;
               }

               incode = code;

               if (code >= max_code) {
                       *sp++ = firstcode;
                       code = oldcode;
               }

               while (code >= clear_code) {
                       *sp++ = table[1][code];
                       if (code == table[0][code]) {
                               sprintf( error_string,
                                        "%s: circular table entry BIG ERROR",
                                        proc_name);
                               OalGifError( error_string);
 		       }
                       code = table[0][code];
               }

               *sp++ = firstcode = table[1][code];

               if ((code = max_code) <(1<<MAX_LWZ_BITS)) {
                       table[0][code] = oldcode;
                       table[1][code] = firstcode;
                       ++max_code;
                       if ((max_code >= max_code_size) &&
                               (max_code_size < (1<<MAX_LWZ_BITS))) {
                               max_code_size *= 2;
                               ++code_size;
                       }
               }

               oldcode = incode;

               if (sp > stack)
                       return *--sp;
       }
       return code;
}

static unsigned char *
OalGifReadImage(FILE *fd, int len, int height, int interlace, int ignore)
/*FILE   *fd;
int    len, height;
int    interlace, ignore;
 commented out for solaris compiler  DWS  09-26-02*/

{
       char            *proc_name="OalGifReadImage";
       unsigned char   c;      
       int             v;
       int             xpos = 0, ypos = 0, pass = 0;
       unsigned char * image;

       /*
       **  Initialize the Compression routines
       */
       if (! OalGifReadOK(fd,&c,1)) {
               sprintf( error_string, "%s: EOF or read error on image data",
                        proc_name);
               OalGifError( error_string);
               return( NULL);
       }
       if (OalGifLWZReadByte(fd, TRUE, c) < 0) {
               sprintf( error_string, "%s: error reading image", proc_name);
               OalGifError( error_string);
               return( NULL);
       }
       /*
       **  If this is an "uninteresting picture" ignore it.
       */
       if (ignore) {
               /*OalGifMsg("skipping image..." );*/

               while (OalGifLWZReadByte(fd, FALSE, c) >= 0)
                       ;
               return( NULL);
       }

       if ((image = (unsigned char *) OaMalloc( len * height)) == NULL) {
               sprintf( error_string, "%s: couldn't alloc space for image",
                        proc_name);
               OalGifError( error_string);
               return( NULL);
       }
       sprintf( error_string, "%s: reading %d by %d%s GIF image", proc_name, 
                len, height, interlace ? " interlaced" : "" );
       /* OalGifMsg( error_string); */

       while ((v = OalGifLWZReadByte(fd,FALSE,c)) >= 0 ) {
               image[ypos*len + xpos] = (unsigned char) v;

               ++xpos;
               if (xpos == len) {
                       xpos = 0;
                       if (interlace) {
                               switch (pass) {
                               case 0:
                               case 1:
                                       ypos += 8; break;
                               case 2:
                                       ypos += 4; break;
                               case 3:
                                       ypos += 2; break;
                               }

                               if (ypos >= height) {
                                       ++pass;
                                       switch (pass) {
                                       case 1:
                                               ypos = 4; break;
                                       case 2:
                                               ypos = 2; break;
                                       case 3:
                                               ypos = 1; break;
                                       default:
                                               goto fini;
                                       }
                               }
                       } else {
                               ++ypos;
                       }
               }
               if (ypos >= height)
                       break;
       }

fini:
       if (OalGifLWZReadByte(fd,FALSE,c)>=0) {
         sprintf( error_string, "%s: too much input data, ignoring extra...", 
                  proc_name);
         OalGifMsg( error_string);
       }
       return( image);
}


void OalGifError(char *error_string)
/*char *error_string;
commented out for solaris compiler  DWS  09-26-02*/
{
oa_errno = 700;
OaReportError( error_string);
}


void OalGifMsg(char *error_string)
/*char *error_string;
 commented out for solaris compiler  DWS  09-26-02*/
{
oa_errno = 950;
OaReportError( error_string);
}


static int OalGifReadOK(FILE *fd, unsigned char *buf, int len)
/*FILE *fd;
unsigned char *buf;
int len;
 commented out for solaris compiler  DWS  09-26-02*/
{

int bytes_read;

bytes_read = (int) fread( buf, 1, len, fd);
total_bytes_read += bytes_read;
return( bytes_read);
}


void OalGifAddTableObj(ODLTREE root_node, char *table_name, char *filename, unsigned long byte_offset, int rows)
/*ODLTREE root_node;
char *table_name;
char *filename;
unsigned long byte_offset;
int rows;
 commented out for solaris compiler  DWS  09-26-02*/
{

ODLTREE table_node, column_node;
char kwd_name[128], kwd_value[256];

/* Add a ^POINTER keyword for the table to the root node. */

sprintf( kwd_name, "^%s", table_name);
UpperCase( kwd_name);
sprintf( kwd_value, "(\"%s\",%-ld<BYTES>)", filename, byte_offset);
OaStrtoKwdValue( kwd_name, root_node, kwd_value);

/* Create the TABLE node and put in keywords.  */

table_node = OdlNewObjDesc( kwd_name+1, NULL, NULL, NULL, NULL, NULL, 
                            (short) 0, (long) 0);
OaLongtoKwdValue( "ROWS", table_node, (long) rows);
OaStrtoKwdValue(  "COLUMNS", table_node, "3");
OaStrtoKwdValue(  "ROW_BYTES", table_node, "3");
OaStrtoKwdValue(  "INTERCHANGE_FORMAT", table_node, "BINARY");
OdlPasteObjDesc( table_node, root_node);

/* Create the "RED" COLUMN node and put in keywords.  */

column_node = OdlNewObjDesc( "COLUMN", NULL, NULL, NULL, NULL, NULL, 
                             (short) 0, (long) 0);
OaStrtoKwdValue(  "NAME", column_node, "RED");
OaStrtoKwdValue(  "DATA_TYPE", column_node, "UNSIGNED_INTEGER");
OaStrtoKwdValue(  "START_BYTE", column_node, "1");
OaStrtoKwdValue(  "BYTES", column_node, "1");
OdlPasteObjDesc( column_node, table_node);

column_node = OaCopyTree( column_node, 0);
OaStrtoKwdValue(  "NAME", column_node, "GREEN");
OaStrtoKwdValue(  "START_BYTE", column_node, "2");
OdlPasteObjDesc( column_node, table_node);

column_node = OaCopyTree( column_node, 0);
OaStrtoKwdValue(  "NAME", column_node, "BLUE");
OaStrtoKwdValue(  "START_BYTE", column_node, "3");
OdlPasteObjDesc( column_node, table_node);

return;
}


void OalGifAddImageObj( ODLTREE root_node, char *image_name, char *filename, unsigned long byte_offset,
					   int lines, int line_samples, int interlace)
/*ODLTREE root_node;
char *image_name;
char *filename;
unsigned long byte_offset;
int lines;
int line_samples;
int interlace;
 commented out for solaris compiler  DWS  09-26-02*/
{

ODLTREE image_node;
char kwd_name[128], kwd_value[256];

/* Add a ^POINTER keyword for the image to the root node. */

sprintf( kwd_name, "^%s", image_name);
UpperCase( kwd_name);
sprintf( kwd_value, "(\"%s\",%-ld<BYTES>)", filename, byte_offset);
OaStrtoKwdValue( kwd_name, root_node, kwd_value);

/* Create the IMAGE node and put in keywords.  */

image_node = OdlNewObjDesc( kwd_name+1, NULL, NULL, NULL, NULL, NULL, 
                            (short) 0, (long) 0);
OaLongtoKwdValue( "LINES", image_node, (long) lines);
OaLongtoKwdValue( "LINE_SAMPLES", image_node, (long) line_samples);
OaStrtoKwdValue(  "SAMPLE_BITS", image_node, "8");
OaStrtoKwdValue(  "SAMPLE_TYPE", image_node, "UNSIGNED_INTEGER");
OaStrtoKwdValue(  "ENCODING_TYPE", image_node, "GIF");
if (interlace)
  OaLongtoKwdValue( "INTERLACE", image_node, (long) interlace);
OdlPasteObjDesc( image_node, root_node);

return;
}
