/*****************************************************************************

  File:  obj_l2.c

  Description: This file contains about half of the C routines making up the 
               Object Layer of the PDS Object Access Library.  The other
               half are in obj_l1.c.  The routines in this file are:

               OaOpenImage
               OaOpenOutputFile
               OaReadArray
               OaReadHistogram
               OaReadHistory
               OaReadImage
               OaReadImagePixels
               OaReadImageFromQube
               OaReadSpectrumFromImage
               OaReadSpectrumFromQube
               OaParseLabelFile
               OaReadObject
               OaReadPartialImage
               OaReadSubTable
               OaReadTable
               OaReportFileAttributes
               OaTransposeTable
               OaUnravelContainer
               OaWriteObject

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:  16 Mar   1998

  History:

    Creation - Most of these routines were part of the Alpha Release of 
               the OA library.  Several new routines were added in the Beta
               Release.
    11/27/95 - Added OaReadImageFromQube and OaReadSpectrumFromQube. SM
    12/06/95 - Moved OaRealloc to oamalloc.c.  SM
    12/06/95 - Replaced malloc() by OaMalloc() throughout.  SM
    12/11/95 - Added error codes.  SM
    11/06/96 - Redesigned OaWriteObject.  SM
    12/11/96 - Added support for multiband images; added new argument to
               OaOpenImage, OaReadImage; added new routine
               OaReadSpectrumFromImage  SM
    02/02/98 - Fixed OalSeek bug in HFD part of OaReadImagePixels.  SM
    03/09/98 - Qube routines: added BIL and suffix plane support.  SM
    03/16/98 - Added call OaCreateODLTreeFromGif in OaParseLabelFile when
               OalOpenStream detects a GIF file.  SM
	04/05/00 - added   if (encoding_type == OA_HUFFMAN_FIRST_DIFFERENCE) in front
			   of all lemmego(line_buffer) calls  DWS
    01/12/03 - Made change to OaReadImageFromQube and OaReadSpectrumFromQube 
			   for THEMIS  DWS
	11/09/04 - Made changes to OaReadImageFromQube and OaReadSpectrumFromImage
			   to repair qubes witn interleaved by pixel storage types.
    12/08/05 - Modified OaReadImageFromQube. See notes.
	01/12/06 - Modified OaReadImageFromQube. See notes. MDC/DWS
	01/13/06 - Modified OaReadImagePixels. See notes.  MDC
*****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "oal.h"

#ifdef XVT_DEF
#include "xvt.h"
#endif

/*****************************************************************************

  Routine:  OaOpenImage

  Description:  OaOpenImage sets up for reading a partial image from a file.
                OaOpenImage is the first call in the sequence: OaOpenImage,
                OaReadImagePixels (multiple calls) or OaReadPartialImage
                (multiple calls), OaCloseImage.  
                It initializes a stream descriptor, opens the file, and
                positions the file pointer to the start of the image object.  
                It creates the image handle oa_object and initializes its ODL
                tree.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  29 Sept  1994
  Last Modified:  11 Dec   1996

  History:

    Creation - This routine was part of the Beta Release of the OA
               library.
    12/11/96 - Added support for multiband images.

  Input:  
         input_node - A pointer to an ODL tree node of class IMAGE.
                      A node somewhere above the input_node must have keywords
                      specifying the data file.  If the image is HFD 
                      compressed, the input_node must have a sibling
                      HISTOGRAM node identifiable as the encoding histogram
                      for HFD decompression.

         band       - For multiband images, this is the band to return.
                      1 <= band <= BANDS  Ignored for monochrome images.

         The global variable Oa_profile is set to valid values.

  Output:  If successful, the routine returns a pointer to an OA_Object 
           structure, which contains a pointer to the image_handle in
           oa_object->appl1.  The oa_object's ODL tree describes an empty
           partial image.

  Notes:  
  1) Current limitations: 
     a) This routine can currently handle Previous Pixel and
        Huffman First Difference compressed images, and uncompressed images.
        For a Clementine-JPEG compressed image, use OaReadImage.
     b) If the image is HFD encoded, it must be in a variable-length record
        file.  
     c) Uncompressed and Previous Pixel compressed images must be in
        fixed-length record files;  variable-length record files are only
        supported for HFD images.
     d) Previous Pixel encoded images may not have prefix or suffix bytes.

  2) For multiband images, the SDT and algorithm are as follows:
     (assume 10 bands, 1 byte/pixel, with input band = 3)

     a) BAND_INTERLEAVED:
        IMAGE             (single node with total_repetitions = LINE_SAMPLES)
        OaOpenImage positions stream_id->fp to the beginning of the 3rd band.
        OaReadImagePixels positions stream_id->fp to desired line.
        OalReadStream reads a total of LINE_SAMPLES bytes.

     b) LINE_INTERLEAVED:
        IMAGE             (single node with total_repetitions = LINE_SAMPLES)
        OaOpenImage positions stream_id->fp to the beginning of the image.
        OaReadImagePixels positions stream_id->fp to desired line AND desired
          band; no prefix or suffix nodes are used.
        OalReadStream reads a total of LINE_SAMPLES bytes.
        
     c) SAMPLE_INTERLEAVED:
        IMAGE node's total_repetitions = LINE_SAMPLES
        IMAGE------COLUMN (prefix SPARE, src.size=2
                ---COLUMN (image line,   src.size=1)
                ---COLUMN (suffix SPARE, src.size=7)
        OaOpenImage positions stream_id->fp to beginning of the image.
        OaReadImagePixels positions stream_id->fp to desired line.
        OalReadStream reads a total of LINE_SAMPLES * BANDS bytes.

*****************************************************************************/

#ifdef _NO_PROTO

OA_OBJECT OaOpenImage( input_node, band)
ODLTREE input_node;
int band;

#else

OA_OBJECT OaOpenImage( ODLTREE input_node, int band)

#endif
{

static char *proc_name = "OaOpenImage";
long file_offset, record_bytes, file_records, bytes, lines;
long line_samples, sample_bits, bands, line_prefix_bytes, line_suffix_bytes;
int record_type, src_interchange_format, encoding_type, band_storage_type;
char *label_filename, *data_filename, *str;

OA_OBJECT oa_object, histogram_object;
ODLTREE sdt, histogram_node, image_node, tmp_node;
SDTNODE image_node_sdt_ptr, sdt_node_ptr;
struct OaStreamStruct *stream_id;
struct oa_image_handle *image_handle;
short endian_indicator=1;       /* Initialized to 1 to determine the value */
                                /* of *platform_is_little_endian below.    */
char *platform_is_little_endian = (char *) &endian_indicator;
/* If platform is little-endian (lsb first), then the 1 initialized in
   endian_indicator will be in the first byte, and *platform_is_little_endian
   will have the value TRUE (1), otherwise it will be FALSE (0).  A test is
   done on *platform_is_little_endian in the Previous Pixel section below to
   decide whether or not change the SAMPLE_TYPE keyword in the output
   oa_object's ODL tree to LSB_INTEGER.  */


if (OaCheckODLTree( input_node) != 0)
  return(NULL);  /* Error message already issued and oa_errno set.  */

if (OaGetImageKeywords( input_node, &lines, &line_samples, &sample_bits,
                        &str, &bands, &band_storage_type, &line_prefix_bytes,
                        &line_suffix_bytes, &encoding_type) != 0)
  return(NULL);

if ((bands > 1) && (band_storage_type == OA_UNKNOWN_BAND_STORAGE_TYPE))
  return(NULL);    /* Error message already issued by OaGetImageKeywords */
if (encoding_type == OA_UNKNOWN_ENCODING_TYPE) {
  oa_errno = 531;  /* Error message already issued by OaGetImageKeywords. */
  return(NULL);
}

if (OaGetFileKeywords( input_node, &label_filename, &data_filename,
                       &record_type, &record_bytes, &file_records,
                       &file_offset, &src_interchange_format) != 0) {
  return(NULL);  /* Error message already issued.  */
}

#ifdef OA_DEBUG
OaReportFileAttributes( label_filename, data_filename, record_type,
                        record_bytes, file_offset, src_interchange_format);
#endif

/* Reject cases not currently supported (see limitations above). */

if (((encoding_type == OA_UNCOMPRESSED) ||
    (encoding_type == OA_PREVIOUS_PIXEL)) &&
    (record_type == OA_VARIABLE_LENGTH)) {
  sprintf( error_string, "%s does not support variable-length record files ",
           proc_name);
  strcat( error_string, " except for HFD compressed images.");
  oa_errno = 520;
  OaReportError( error_string);
  return(NULL);
}
if ((encoding_type == OA_HUFFMAN_FIRST_DIFFERENCE) &&
    (record_type != OA_VARIABLE_LENGTH)) {
  sprintf( error_string, "%s: HFD compressed images are only supported",
           proc_name);
  strcat( error_string, " in variable-length record files.");
  oa_errno = 520;
  OaReportError( error_string);
  return(NULL);
}
if ((encoding_type == OA_PREVIOUS_PIXEL) || (bands > 1)) {
  if ((line_prefix_bytes != 0) || (line_suffix_bytes != 0)) {
    sprintf( error_string, 
             "%s: LINE_PREFIX_BYTES and LINE_SUFFIX_BYTES are not supported ",
               proc_name);
  strcat( error_string, "for multiband or Previous Pixel compressed images.");
    oa_errno = 520;
    OaReportError( error_string);
    return(NULL);
  }
}
if ((encoding_type != OA_UNCOMPRESSED) && (bands > 1)) {
  sprintf( error_string, "%s Compressed multi-band images not supported!",
           proc_name);
  oa_errno = 520;
  OaReportError( error_string);
  return(NULL);
}

/* Allocate an oa_image_handle structure. */

if ((image_handle = (struct oa_image_handle *) 
                     OaMalloc( sizeof( struct oa_image_handle))) == NULL) {
  sprintf( error_string, 
           "%s: OaMalloc failed to allocate space for image_handle.",
           proc_name);
  strcat( error_string, " Out of memory!");
  oa_errno = 720;
  OaReportError( error_string);
  exit(1);
  /*NOTREACHED*/
}
image_handle->image_start_offset  = file_offset;
image_handle->compression_type    = encoding_type;
image_handle->next_line           = 1;
image_handle->next_sample         = 1;
image_handle->buf_samples         = 0;
image_handle->source_lines        = lines;
image_handle->source_line_samples = line_samples;
image_handle->sample_bytes        = sample_bits/8;
image_handle->bands               = bands;
image_handle->band                = band;
image_handle->band_storage_type   = band_storage_type;

/* If image is HFD compressed, initialize the HFD decomp structure.  */

if (encoding_type == OA_HUFFMAN_FIRST_DIFFERENCE) {

  /* Call OaFindEncodingHistogram to find the node in the tree describing 
     the encoding histogram needed for HFD decompression, then call
     OaReadHistogram to read the histogram data into memory.  */

  if ((histogram_node = OaFindEncodingHistogram( input_node)) == NULL) {
    sprintf( error_string, "%s: couldn't find encoding histogram for HFD",
             proc_name);
    strcat( error_string, " compressed image.");
    oa_errno = 530;
    OaReportError( error_string);
    return( NULL);
  }

  if ((histogram_object = OaReadHistogram( histogram_node)) == NULL) {
    sprintf( error_string, "%s: couldn't read encoding histogram for HFD",
             proc_name);
    strcat( error_string, " compressed image.");
    OaReportError( error_string);
    return( NULL);
  }

  /* Build the HFD decoding tree and attach it to the image handle. Note:
     image_handle->decomp.HFD.line_buffer is initialized after building
     the SDT.  */

  if ((image_handle->decomp.HFD.decoding_tree = 
         OalCreateHFDTree( histogram_object->data_ptr)) == NULL) {
    sprintf( error_string, "%s: OalCreateHFDTree returned NULL.", 
             proc_name);
    OaReportError( error_string);
    return( NULL);
  }
  OaDeleteObject( histogram_object);
}  /* end if HFD compressed */

/* If image is PP compressed, initialize the PP decomp structure.  */

if (encoding_type == OA_PREVIOUS_PIXEL) {
  image_handle->decomp.PP.found_255 = FALSE;
  image_handle->decomp.PP.previous_byte_exists  = FALSE;
}

/* Copy the sub-tree of the input ODL tree starting at the image node.  */

image_node = OaCopyTree( input_node, OA_STRIP_COMMENTS | OA_STRIP_SDT_NODES);

/* Initialize the keywords in the ODL tree which describe where the samples
   in OaReadImagePixels' buffer came from.
   LINES               = 1
   LINE_SAMPLES        = 0
   FIRST_LINE          = 1
   FIRST_SAMPLE        = 1
   SOURCE_LINES        = <original LINES>
   SOURCE_LINE_SAMPLES = <original LINE_SAMPLES>
*/

OaLongtoKwdValue( "LINES", image_node, (long) 1);
OaLongtoKwdValue( "LINE_SAMPLES", image_node, (long) 0);
OaLongtoKwdValue( "FIRST_LINE", image_node, (long) 1);
OaLongtoKwdValue( "FIRST_SAMPLE", image_node, (long) 1);
OaLongtoKwdValue( "SOURCE_LINES", image_node,  
                   (long) image_handle->source_lines);
OaLongtoKwdValue( "SOURCE_LINE_SAMPLES", image_node, 
                   (long) image_handle->source_line_samples);

/* Unless compression_type is PREVIOUS_PIXEL, call OalCreateSDT to create a
   SDT (Stream Decomposition Tree) specifying one line's worth of samples as
   the source size.  The code temporarily changes the LINE_SAMPLES keyword
   back to image_handle->source_line_samples so OalCreateSDT will work right.
   OaReadImagePixels will read one line each call, and use the SDT to convert
   the samples to native format and/or strip off prefix and suffix bytes.  

   An SDT is not used for PREVIOUS_PIXEL compressed images because they are
   always 2-byte MSB_INTEGERs, and the decompression algorithm necessarily has
   to do the conversion to native 2-byte integers itself;  this means the
   caller gets the pixels in native format, regardless of what Oa_profile is
   set to...  */

if (encoding_type != OA_PREVIOUS_PIXEL) {
  
  OaLongtoKwdValue( "LINE_SAMPLES", image_node, 
                     (long) image_handle->source_line_samples);
 
  /* Delete keywords for BANDS, BAND_STORAGE_TYPE and BAND_SEQUENCE since 
     these will no longer be applicable for the in-memory monochromatic
     image.  */

  OaDeleteKwd( "BANDS",             image_node);
  OaDeleteKwd( "BAND_STORAGE_TYPE", image_node);
  OaDeleteKwd( "BAND_SEQUENCE",     image_node);

  if (bands > 1) {
    switch( band_storage_type) {

      case OA_LINE_INTERLEAVED:
        if (band > 1)
          OaLongtoKwdValue( "LINE_PREFIX_BYTES", image_node, 
                            line_samples * sample_bits/8 * (band - 1));
        if (band < bands)
          OaLongtoKwdValue( "LINE_SUFFIX_BYTES", image_node, 
                            line_samples * sample_bits/8 * (bands - band));
      break;

      case OA_SAMPLE_INTERLEAVED:
        if (band > 1)
          OaLongtoKwdValue( "LINE_PREFIX_BYTES", image_node, 
                            sample_bits/8 * (band - 1));
        if (band < bands)
          OaLongtoKwdValue( "LINE_SUFFIX_BYTES", image_node, 
                            sample_bits/8 * (bands - band));

        /* Trick OalCreateSDT into creating the desired SDT by making it think
           the image line is transposed into an 2-dimensional image, where
           the desired band's pixels are in a column, with undesired band data
           to the left and right of the column, represented by prefix and
           suffix bytes.  The number of lines in this image is equal to the
           number of pixels in the real image, and there's only one sample.  */

        OaLongtoKwdValue( "LINES", image_node, line_samples);
        OaLongtoKwdValue( "LINE_SAMPLES", image_node, (long) 1);
      break;
    }
  }

  if ((sdt = OalCreateSDT( image_node, src_interchange_format)) == NULL) {
    sprintf( error_string, "%s: CreateSDT returned error.", 
             proc_name);
    OaReportError( error_string);
    OalFreeSDT( image_node);
    LemmeGo( image_handle);
    return(NULL);
  }
  image_node = sdt;

  /* Undo the keyword changes made above which tricked OalCreateSDT into
     creating the right kind of SDT.  */

  OaLongtoKwdValue( "LINE_SAMPLES", image_node, (long) 0);
  if (band_storage_type == OA_SAMPLE_INTERLEAVED)
    OaLongtoKwdValue( "LINES", image_node, 1);

  image_node_sdt_ptr = (SDT_node *) image_node->appl1;

  /* Set the SAMPLE_TYPE keyword value to the dst->PDS_data_type of the pixels.
     The node with the pixels' dst->PDS_data_type is image_node, if the image
     node has no children (SPARE nodes representing line prefix or suffix
     bytes), otherwise it's the node with non-zero dst.size (the node with
     NAME = IMAGE_LINE).  */

  if ((tmp_node = LeftmostChild( image_node)) == NULL)
    OaStrtoKwdValue(  "SAMPLE_TYPE", image_node, OaPDSDataTypetoStr( 
                      image_node_sdt_ptr->dst.PDS_data_type));
  else {
    sdt_node_ptr = (SDT_node *) tmp_node->appl1;  
    if (sdt_node_ptr->dst.size == 0) {    /* prefix bytes node */
      tmp_node = RightSibling( tmp_node);
      sdt_node_ptr = (SDT_node *) tmp_node->appl1;
    }
    OaStrtoKwdValue(  "SAMPLE_TYPE", image_node, OaPDSDataTypetoStr( 
                      sdt_node_ptr->dst.PDS_data_type));
  }
} else { /* PP compressed image */

  /* If on an LSB integer platform, change SAMPLE_TYPE keyword value from
     MSB_INTEGER to LSB_INTEGER.  */

  if (*platform_is_little_endian == TRUE)
    OaStrtoKwdValue( "SAMPLE_TYPE", image_node, "LSB_INTEGER");

  /* Left out code to use the profile and convert the PP decompressed pixels;
     I assume it's very rare that the user doesn't want in them in native
     2-byte binary integer format. */
}


/* Delete keywords for ENCODING_TYPE, LINE_PREFIX_BYTES and LINE_SUFFIX_BYTES,
   since these will no longer be applicable for the in-memory image, which is
   decompressed, and has its prefix and/or suffix bytes removed.  */

OaDeleteKwd( "ENCODING_TYPE",     image_node);
OaDeleteKwd( "LINE*PREFIX*BYTES", image_node);
OaDeleteKwd( "LINE*SUFFIX*BYTES", image_node);
OaDeleteKwd( "^LINE*PREFIX*STRUCTURE", image_node);
OaDeleteKwd( "^LINE*SUFFIX*STRUCTURE", image_node);

if (encoding_type == OA_HUFFMAN_FIRST_DIFFERENCE) {

  /* Allocate a buffer big enough to store a decompressed image line.  If
     there are prefix and/or suffix bytes, they are assumed to be compressed
     along with each image line.  */

  if (LeftmostChild( image_node) == NULL) /* No prefix or suffix bytes. */
    bytes = image_node_sdt_ptr->src.size * image_handle->source_line_samples;
  else
    bytes = image_node_sdt_ptr->src.size;
  image_handle->decomp.HFD.line_buffer_bytes = bytes;
  if ((image_handle->decomp.HFD.line_buffer = (char *) OaMalloc(  
                                                  (size_t) bytes)) == NULL) {
    sprintf( error_string, 
             "%s: OaMalloc failed to allocate space for HFD line_buffer.",
             proc_name);
    strcat( error_string, " Out of memory!");
    oa_errno = 720;
    OaReportError( error_string);
    exit(1);
    /*NOTREACHED*/
  }
}

if ((stream_id = OalOpenStream( data_filename, record_type, record_bytes, 
                                file_records, "r")) == NULL) {
  sprintf( error_string, "%s: OalOpenStream returned error.",
           proc_name);
  OaReportError( error_string);
  LemmeGo( image_handle);
  OdlFreeTree( image_node);
  return(NULL);
}

/* Position the file pointer to the beginning of the image, or in the case of
   a BAND_SEQUENTIAL image, to the start of the desired band. */

if (band_storage_type == OA_BAND_SEQUENTIAL)
  file_offset += (band - 1) * sample_bits/8 * line_samples * lines;

if (OalSeek( stream_id, file_offset) != 0) {
  sprintf( error_string, "%s: OalSeek returned error, aborting.",
           proc_name);
  OaReportError( error_string);
  OalCloseStream( stream_id);
  LemmeGo( image_handle);
  return(NULL);
}

/* Create the output oa_object and initialize it.  */

oa_object = OaNewOaObject();
oa_object->odltree = image_node;
oa_object->data_ptr = NULL;
oa_object->size = 0;
oa_object->is_in_memory = FALSE;
oa_object->stream_id = stream_id;
oa_object->appl1 = image_handle;

/* Determine the size of, and allocate space for internal buffer which stores
   the latest samples which have been read, decompressed and converted.  */

if (encoding_type == OA_PREVIOUS_PIXEL)
  image_handle->buf_siz = stream_id->buf_siz * 2; /* Max compression ratio. */
else
  image_handle->buf_siz = image_node_sdt_ptr->dst.size *
                          image_node_sdt_ptr->total_repetitions;

if ((image_handle->buf = OaMalloc( (size_t) image_handle->buf_siz)) == NULL) {
  sprintf( error_string, 
           "%s: OaMalloc failed to allocate space for image_handle->buf.",
           proc_name);
  strcat( error_string, " Out of memory!");
  oa_errno = 720;
  OaReportError( error_string);
  exit(1);
  /*NOTREACHED*/
}

#ifdef OA_DEBUG
  sprintf( error_string, "%s: image_handle is:\n", proc_name);
  sprintf( error_string + strlen( error_string),
           "image_start_offset  = %ld\n", image_handle->image_start_offset);
  sprintf( error_string + strlen( error_string),
           "buf_siz             = %ld\n", image_handle->buf_siz);
  sprintf( error_string + strlen( error_string),
           "next_line           = %ld\n", image_handle->next_line);
  sprintf( error_string + strlen( error_string),
           "next_sample         = %ld\n", image_handle->next_sample);
  sprintf( error_string + strlen( error_string),
           "buf_samples         = %ld\n", image_handle->buf_samples);
  sprintf( error_string + strlen( error_string),
           "source_lines        = %ld\n", image_handle->source_lines);
  sprintf( error_string + strlen( error_string),
           "source_line_samples = %ld\n", image_handle->source_line_samples);
  sprintf( error_string + strlen( error_string),
           "bands               = %d\n", image_handle->bands);
  sprintf( error_string + strlen( error_string),
           "band                = %d\n", image_handle->band);
  sprintf( error_string + strlen( error_string),
           "sample_bytes        = %d\n", (int) image_handle->sample_bytes);
  oa_errno = 950;
  OaReportError( error_string);
#endif

return( oa_object);
}



/*****************************************************************************

  Routine:  OaOpenOutputFile

  Description:  This routine sets up for writing objects to an output file.
                It is the first call in the sequence:  OaOpenOutputFile,
                OaWriteObject (multiple calls), OaCloseOutputFile.
                It calls OalOpenStream to open the data file and initialize a
                stream descriptor, then builds a FILE-class ODL tree node to
                describe the file.  After calling OaOpenOutputFile, the caller
                can call OaWriteObject to write an object to the file and add
                new nodes and ^POINTERs to the file object's ODL tree.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:  11 Nov   1996

  History:

    Creation - This routine was part of the Alpha Release of the OA library.
    11/11/96 - Added support for UNDEFINED record type. SM

  Input:    
          data_filename - A file pathname suitable for direct use by fopen().
                          It must be valid on the platform being run on, and 
                          the process running OA must have write permission.
          record_type   - OA_FIXED_LENGTH, OA_STREAM, OA_VARIABLE_LENGTH or
                          OA_UNDEFINED_RECORD_TYPE.
          record_bytes  - Must be greater than 0 for OA_FIXED_LENGTH or 
                          OA_VARIABLE_LENGTH, and is ignored for OA_STREAM
                          and OA_UNDEFINED_RECORD_TYPE.
         
  Output:  If successful, the routine returns a pointer to an Oa_Object
           containing a pointer to a stream descriptor, which describes the
           opened data file, and the ODL tree attached to the Oa_Object has a
           single FILE node with appropriate keywords set.

  Notes:   Haven't tested what happens with bad pathnames, or if the file
           in data_filename already exists, or if don't have write permission.

*****************************************************************************/

#ifdef _NO_PROTO

OA_OBJECT OaOpenOutputFile( data_filename, record_type, record_bytes)
char *data_filename;
int  record_type;
long record_bytes;

#else

OA_OBJECT OaOpenOutputFile( char *data_filename, int record_type, 
                            long record_bytes)

#endif
{

static char *proc_name = "OaOpenOutputFile";
OA_OBJECT oa_object;

/* Check inputs for validity. */

if (data_filename == NULL) {
  sprintf( error_string, "%s: data_filename is NULL.", proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return(NULL);
}

switch( record_type) {
  case OA_FIXED_LENGTH:
  case OA_VARIABLE_LENGTH:
    if (record_bytes < 1) {
      sprintf( error_string, "%s: record bytes must be greater than 0 ",
               proc_name);
      strcat(  error_string, 
              "for a FIXED_LENGTH or VARIABLE_LENGTH record file.");
      oa_errno = 502;
      OaReportError( error_string);
      return(NULL);
    }
  break;
  case OA_STREAM:
  case OA_UNDEFINED_RECORD_TYPE:
  break;
  default:
    sprintf( error_string, "%s: invalid record type: %d.",
             proc_name, record_type);
    oa_errno = 502;
    OaReportError( error_string);
    return( NULL);
    /*NOTREACHED*/
  break;
}

/* Create a new Oa_Object structure and initialize it. */

oa_object = OaNewOaObject();
oa_object->is_in_memory = FALSE;
oa_object->size = 0;

/* Open the output file. */

if ((oa_object->stream_id = OalOpenStream( data_filename,
                                           record_type,
                                           record_bytes,
                                           (long) 0,
                                           "w")) == NULL) {
  LemmeGo( oa_object);
  return( NULL);
}

/* Create the ODL tree node to describe the file.  */

oa_object->odltree = OdlNewObjDesc( "FILE", NULL, NULL, NULL, NULL, NULL, 
                                     (short) 0, (long) 0);

/* Add PDS_VERSION_ID, RECORD_TYPE, RECORD_BYTES and FILE_RECORDS keywords to
   the file node, as appropriate for the specified record type.  */

OaStrtoKwdValue( "PDS_VERSION_ID", oa_object->odltree, "PDS3");

switch( record_type) {
  case OA_FIXED_LENGTH:
    OaStrtoKwdValue( "RECORD_TYPE", oa_object->odltree, "FIXED_LENGTH");
  break;
  case OA_VARIABLE_LENGTH:
    OaStrtoKwdValue( "RECORD_TYPE", oa_object->odltree, "VARIABLE_LENGTH");
  break;
  case OA_STREAM:
    OaStrtoKwdValue( "RECORD_TYPE", oa_object->odltree, "STREAM");
  break;
  case OA_UNDEFINED_RECORD_TYPE:
    OaStrtoKwdValue( "RECORD_TYPE", oa_object->odltree, "UNDEFINED");
  break;
}

if ((record_type != OA_STREAM) && 
    (record_type != OA_UNDEFINED_RECORD_TYPE)) {
  OaLongtoKwdValue( "RECORD_BYTES", oa_object->odltree, record_bytes);
  OaLongtoKwdValue( "FILE_RECORDS", oa_object->odltree, (long) 0);
}
return( oa_object);
}



/*****************************************************************************

  Routine:  OaReadArray

  Description:  OaReadArray opens a data file and reads an ARRAY object into 
                memory.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:   1 Sept  1994

  History:

    Creation - This routine was part of the Alpha Release of the OA library.

  Input:  
         input_node - A pointer to an ODL tree node of class ARRAY.
                      The tree above the input_node should have keywords
                      to specify the data file.
       
         The global variable Oa_profile is set to valid values.

  Output:  If successful, the routine returns a pointer to an OA_Object 
           structure, which contains an ODL tree and a pointer to the array
           data.  If unsuccessful it returns NULL.

  Notes:  

*****************************************************************************/

#ifdef _NO_PROTO

OA_OBJECT OaReadArray( input_node)
ODLTREE input_node;

#else

OA_OBJECT OaReadArray( ODLTREE input_node)

#endif
{

static char *proc_name = "OaReadArray";
long file_offset, record_bytes, file_records;
int record_type, src_interchange_format, dst_interchange_format;
char *label_filename, *data_filename;
OA_OBJECT oa_object;
ODLTREE current_node, sdt, array_node, compressed_SDT;
SDTNODE sdt_node_ptr;
int object_class;
char *buf = NULL, c;
#ifdef IBM_PC
long i;
#endif
PTR data_ptr;
struct OaStreamStruct *stream_id;
long bytes_read, object_size;

object_class = OaGetObjectClass( input_node);
switch( object_class) { 
 case OA_ARRAY:
  break;
  default:
    sprintf( error_string, "%s: input ODLTREE node is not an ARRAY object.",
             proc_name);
    oa_errno = 530;
    OaReportError( error_string);
    return(NULL);
}

if (OaGetFileKeywords( input_node, &label_filename, &data_filename,
                       &record_type, &record_bytes, &file_records,
                       &file_offset, &src_interchange_format) != 0) {
  return(NULL);  /* Error message already issued.  */
}
if (src_interchange_format == OA_ASCII_INTERCHANGE_FORMAT)
  dst_interchange_format = Oa_profile.dst_format_for_ASCII_src;
else
  dst_interchange_format = Oa_profile.dst_format_for_binary_src;

#ifdef OA_DEBUG
OaReportFileAttributes( label_filename, data_filename, record_type,
                        record_bytes, file_offset, src_interchange_format);
#endif

/* The input tree ODL tree shouldn't ever be modified, so make a copy of all
   the nodes below the input node.  */

array_node = OaCopyTree( input_node, OA_STRIP_COMMENTS | OA_STRIP_SDT_NODES);

if (OaCheckODLTree( array_node) != 0) {
  OdlFreeTree( array_node);
  return(NULL);  /* Error message already issued and oa_errno set.  */
}

/* Call OalCreateSDT to create the SDT (Stream Decomposition Tree).  */

if ((sdt = OalCreateSDT( array_node, src_interchange_format)) == NULL) {
  sprintf( error_string, "%s: CreateSDT returned error.", 
           proc_name);
  OaReportError( error_string);
  OalFreeSDT( array_node);
  return(NULL);
}

/* Note: sdt and array_node now point to the same node. */

/* Allocate memory for the array.  */

sdt_node_ptr = (SDT_node *) sdt->appl1;
object_size = sdt_node_ptr->dst.size * 
              sdt_node_ptr->total_repetitions;
if ((data_ptr = (PTR) OaMalloc( (long) object_size)) == NULL) {
  sprintf( error_string, "%s: OaMalloc failed! Out of memory!", proc_name);
  oa_errno = 720;
  OaReportError( error_string);
  exit(1);
  /*NOTREACHED*/
}
if (dst_interchange_format == OA_ASCII_INTERCHANGE_FORMAT) 
  c = ' ';
else
  c = 0;

#ifdef IBM_PC

/* If we're on an IBM-PC, use a for loop to do the set, since memset
   may not work for setting greater than 64K and/or over segment
   boundaries;  otherwise use memset since it's usually optimized.  */

for (i=0; i<object_size; i++)
  data_ptr[i] = c;
#else
memset( data_ptr, c, (size_t) object_size);
#endif

if ((stream_id = OalOpenStream( data_filename, record_type, record_bytes, 
                                file_records, "r")) == NULL) {
  sprintf( error_string, "%s: OalOpenStream returned error.",
           proc_name);
  OaReportError( error_string);
  return(NULL);
}

/* Read the first slice of data; tell OalReadStream to read as many bytes as it
   wants, store them in it's own buffer, start reading the file at the given
   byte offset, and output how many bytes it's read in bytes_read.  */

if (OalReadStream( stream_id, (long) 0, &buf, file_offset, &bytes_read) != 0) {
  sprintf( error_string, "%s: OalReadStream returned error, aborting.",
           proc_name);
  OaReportError( error_string);
  OalFreeSDT( array_node);
  OalCloseStream( stream_id);
  return(NULL);
}

/* Compress the SDT, and initialize current_node to point to the first
   end-node (data node) in the compressed SDT; initialize the node's dst.ptr
   with the pointer returned by OaMalloc.  */

compressed_SDT = OalCompressSDT( sdt);
current_node = OalInitializeSDT( compressed_SDT, data_ptr);

/* The main loop processes a buffer of data, then reads in the next buffer; 
   the SDT maps desired data to memory and throws aways the rest.  */

for (;;) {
  if (OalProcessSDT( (PTR) buf, bytes_read, &current_node) ==
      OA_REACHED_END_OF_SDT) break;

  /*  Read next slice of data, telling OalReadStream to do the same as the
      first time, except start at the current file position.  */

  buf = NULL;
  if (OalReadStream( stream_id, (long) 0, &buf, (long) -1, &bytes_read) != 0) {
    sprintf( error_string, "%s: OalReadStream returned error, aborting.",
             proc_name);
    OaReportError( error_string);
    break;
  }
}
OalCloseStream( stream_id);

/* Free the compressed SDT. */

OalFreeSDT( compressed_SDT);

/* Call OalSDTtoODLTree to modify the new ODL tree to reflect the in-memory 
   data.  This changes the DATA_TYPE keywords to reflect the in-memory data
   values which may have been converted to native types.  It also strips off 
   ODL tree nodes which have SDT_node->dst.size=0.  */

OalSDTtoODLTree( sdt, dst_interchange_format);

/* Create the output oa_object and initialize it.  */

oa_object = OaNewOaObject();
oa_object->odltree = sdt;
oa_object->data_ptr = data_ptr;
oa_object->size = object_size;
oa_object->is_in_memory = TRUE;
oa_object->profile = Oa_profile;
oa_object->stream_id = NULL;

return( oa_object);
}



/*****************************************************************************

  Routine:  OaReadHistogram

  Description:  OaReadHistogram opens a data file and reads a HISTOGRAM object
                into memory.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:   1 Sept  1994

  History:

    Creation - This routine was part of the Alpha Release of the OA library.

  Input:  
         input_node - A pointer to an ODL tree node of class HISTOGRAM.
                      The tree above the input_node should have keywords
                      to specify the data file.
       
         The global variable Oa_profile is set to valid values.

  Output:  If successful, the routine returns a pointer to an OA_Object 
           structure, which contains an ODL tree and a pointer to the histogram
           data.  If unsuccessful it returns NULL.

  Notes:  

*****************************************************************************/

#ifdef _NO_PROTO

OA_OBJECT OaReadHistogram( input_node)
ODLTREE input_node;

#else

OA_OBJECT OaReadHistogram( ODLTREE input_node)

#endif
{

static char *proc_name = "OaReadHistogram";
long file_offset, record_bytes, file_records;
int record_type, src_interchange_format, dst_interchange_format;
char *label_filename, *data_filename;
OA_OBJECT oa_object;
ODLTREE current_node, sdt, histogram_node, compressed_SDT;
SDTNODE sdt_node_ptr;
long object_size, bytes_read;
#ifdef IBM_PC
long i;
#endif
int object_class;
char *buf = NULL, c;
PTR data_ptr;
struct OaStreamStruct *stream_id;

object_class = OaGetObjectClass( input_node);
if (object_class != OA_HISTOGRAM) {
  sprintf( error_string, "%s: input ODLTREE node is not a HISTOGRAM object.",
           proc_name);
  oa_errno = 530;
  OaReportError( error_string);
  return(NULL);
}

if (OaGetFileKeywords( input_node, &label_filename, &data_filename,
                       &record_type, &record_bytes, &file_records,
                       &file_offset, &src_interchange_format) != 0) {
  return(NULL);  /* Error message already issued.  */
}
if (src_interchange_format == OA_ASCII_INTERCHANGE_FORMAT)
  dst_interchange_format = Oa_profile.dst_format_for_ASCII_src;
else
  dst_interchange_format = Oa_profile.dst_format_for_binary_src;

#ifdef OA_DEBUG
OaReportFileAttributes( label_filename, data_filename, record_type,
                        record_bytes, file_offset, src_interchange_format);
#endif

/* The input tree ODL tree shouldn't ever be modified, so make a copy of all
   the nodes below the input node.  */

histogram_node = OaCopyTree( input_node, OA_STRIP_COMMENTS | 
                                         OA_STRIP_SDT_NODES);

if (OaCheckODLTree( histogram_node) != 0) {
  OdlFreeTree( histogram_node);
  return(NULL);  /* Error message already issued and oa_errno set.  */
}

/* Call OalCreateSDT to create the SDT (Stream Decomposition Tree).  */

if ((sdt = OalCreateSDT( histogram_node, src_interchange_format)) == NULL) {
  sprintf( error_string, "%s: CreateSDT returned error.", 
           proc_name);
  OaReportError( error_string);
  OalFreeSDT( histogram_node);
  return(NULL);
}

/* Allocate memory for the histogram.  */

sdt_node_ptr = (SDT_node *) sdt->appl1;
object_size = sdt_node_ptr->dst.size * sdt_node_ptr->total_repetitions;
if ((data_ptr = (PTR) OaMalloc( (long) object_size)) == NULL){
  sprintf( error_string, "%s: OaMalloc failed! Out of memory!", proc_name);
  oa_errno = 720;
  OaReportError( error_string);
  exit(1);
  /*NOTREACHED*/
}
if (dst_interchange_format == OA_ASCII_INTERCHANGE_FORMAT) 
  c = ' ';
else
  c = 0;

#ifdef IBM_PC

/* If we're on an IBM-PC, use a for loop to do the set, since memset
   may not work for setting greater than 64K and/or over segment
   boundaries;  otherwise use memset since it's usually optimized.  */

for (i=0; i<object_size; i++)
  data_ptr[i] = c;
#else
memset( data_ptr, c, (size_t) object_size);
#endif

/* Create the output oa_object and initialize it.  */

oa_object = OaNewOaObject();
oa_object->odltree = sdt;
oa_object->data_ptr = data_ptr;
oa_object->size = object_size;
oa_object->is_in_memory = TRUE;
oa_object->profile = Oa_profile;
oa_object->stream_id = NULL;

if ((stream_id = OalOpenStream( data_filename, record_type, record_bytes, 
                                file_records, "r")) == NULL) {
  sprintf( error_string, "%s: OalOpenStream returned error.",
           proc_name);
  OaReportError( error_string);
  OalFreeSDT( histogram_node);
  return(NULL);
}

/* Read the first slice of data; tell OalReadStream to read as many bytes as it
   wants, store them in it's own buffer, start reading the file at the given
   byte offset, and output how many bytes it's read in bytes_read.  */

if (OalReadStream( stream_id, (long) 0, &buf, file_offset, &bytes_read) != 0) {
  sprintf( error_string, "%s: OalReadStream returned error, aborting.",
           proc_name);
  OaReportError( error_string);
  OalFreeSDT( histogram_node);
  return(NULL);
}

/* Compress the SDT, and initialize current_node to point to the first
   end-node (data node) in the compressed SDT; initialize the node's dst.ptr
   with the pointer returned by OaMalloc. In the case of a HISTOGRAM, there is
   only one node.  */

compressed_SDT = OalCompressSDT( sdt);
current_node = compressed_SDT;
sdt_node_ptr = (SDT_node *) current_node->appl1;
sdt_node_ptr->dst.ptr = data_ptr;

/* The main loop processes a buffer of data, then reads in the next buffer; 
   the SDT maps desired data to memory and throws aways the rest.  
   In the case of a HISTOGRAM, all the data is mapped, none is thrown away.  */

for (;;) {
  if (OalProcessSDT( (PTR) buf, bytes_read, &current_node) == 
      OA_REACHED_END_OF_SDT) break;
  
  /*  Read next slice of data, telling OalReadStream to do the same as the
      first time, except start at the current file position.  */

  buf = NULL;
  if (OalReadStream( stream_id, (long) 0, &buf, (long) -1, &bytes_read) != 0) {
    sprintf( error_string, "%s: OalReadStream returned error, aborting.",
             proc_name);
    OaReportError( error_string);
    break;
  }
}
OalCloseStream( stream_id);

/* Free the compressed SDT. */

OalFreeSDT( compressed_SDT);

/* Call OalSDTtoODLTree to modify the new ODL tree to reflect the in-memory 
   data.  This changes the DATA_TYPE keywords to reflect the in-memory data
   values which may have been converted to native types.  It also strips off 
   ODL tree nodes which have SDT_node->dst.size=0.  */

OalSDTtoODLTree( sdt, dst_interchange_format);

return( oa_object);
}



/*****************************************************************************

  Routine:  OaReadHistory

  Description:  OaReadHistory opens a data file and reads a HISTORY object
                into memory.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   9 Jan   1995
  Last Modified:   9 Jan   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:  
         input_node - A pointer to a ODL tree node which has a ^POINTER to
                      a HISTORY object;  usually the root node, but could be
                      a FILE node under the root node. 

  Output:  If successful, the routine returns a pointer to an OA_Object 
           structure, which contains an ODL tree representing the parsed
           HISTORY object.  The OA_Object's data_ptr is NULL, and size is 0,
           since the HISTORY object is all ODL tree, and no object data.
           If unsuccessful it returns NULL.

  Notes:  

*****************************************************************************/

#ifdef _NO_PROTO

OA_OBJECT OaReadHistory( input_node)
ODLTREE input_node;

#else

OA_OBJECT OaReadHistory( ODLTREE input_node)

#endif
{

static char *proc_name = "OaReadHistory";
long file_offset, record_bytes, file_records;
int record_type, src_interchange_format, seek_result;
char *label_filename, *data_filename;
OA_OBJECT oa_object;
ODLTREE history_node, root_node;
struct OaStreamStruct *stream_id;

/* Check input for validity.  */

if (input_node == NULL) {
  sprintf( error_string, "%s: input_node is NULL", proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return( NULL);
}

/* Make a temporary ODL tree node under input_node with object class
   HISTORY.  This is deleted after the call to OaGetFileKeywords.  */

history_node = OdlNewObjDesc( "HISTORY", NULL, NULL, NULL, NULL, NULL, 
                              (short) 0, (long) 0);
OdlPasteObjDesc( history_node, input_node);

if (OaGetFileKeywords( history_node, &label_filename, &data_filename,
                       &record_type, &record_bytes, &file_records,
                       &file_offset, &src_interchange_format) != 0) {
    OdlFreeTree( OdlCutObjDesc( history_node));
    return(NULL);  /* Error message already issued.  */
}
OdlFreeTree( OdlCutObjDesc( history_node));

src_interchange_format = OA_ASCII_INTERCHANGE_FORMAT;

#ifdef OA_DEBUG
OaReportFileAttributes( label_filename, data_filename, record_type,
                        record_bytes, file_offset, src_interchange_format);
#endif

/* Open the file the HISTORY object is in, and position to the start of the
   HISTORY object.  */

if ((stream_id = OalOpenStream( data_filename, record_type, record_bytes, 
                                file_records, "r")) == NULL) {
  sprintf( error_string, "%s: OalOpenStream returned error.",
           proc_name);
  OaReportError( error_string);
  return(NULL);
}
if ((seek_result = OalSeek( stream_id, file_offset)) != 0) {
  sprintf( error_string, "%s: OalSeek returned error: %d.",
           proc_name, seek_result);
  OaReportError( error_string);
  OalCloseStream( stream_id);
  return( NULL);
}

/* Pass the file pointer to OdlParseFile, and let it parse the HISTORY object
   into an ODL tree.  If L3 eventually uses the OAL Stream Layer, this will
   be changed to pass in the stream_id instead of the file pointer.  Until
   then, won't be able to read a HISTORY object from a variable-length
   record file except on a VAX.  Also suppress error messages from the parser. 
*/

root_node = OdlParseFile( NULL, stream_id->fp, NULL, NULL, NULL,  
                          1,1,1,0);

if (root_node == NULL) {
  sprintf( error_string, "%s: error parsing HISTORY object", proc_name);
  oa_errno = 1000;
  OaReportError( error_string);
  OalCloseStream( stream_id);
  return(NULL);
}
OalCloseStream( stream_id);

if (LeftmostChild( root_node) == NULL) {
  sprintf( error_string, "%s: error parsing HISTORY object", proc_name);
  oa_errno = 1000;
  OaReportError( error_string);
  return(NULL);
}

history_node = OdlCutObjDesc( LeftmostChild( root_node));
OdlFreeTree( root_node);

/* Create the output oa_object and initialize it.  */

oa_object = OaNewOaObject();
oa_object->odltree = history_node;
oa_object->data_ptr = NULL;
oa_object->size = 0;
oa_object->is_in_memory = FALSE;
oa_object->profile = Oa_profile;
oa_object->stream_id = NULL;

return( oa_object);
}



/*****************************************************************************

  Routine:  OaReadImage

  Description:  OaReadImage opens a data file and reads an entire IMAGE object
                into memory.  The image is decompressed, and any prefix and
                suffix bytes are stripped out.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:  16 Mar   1998

  History:

    Creation - This routine was part of the Alpha Release of the OA library.
    12/11/96 - Added support for multiband images; including new 'band' 
               argument.  SM
    03/16/98 - Added call to OaGIF for OA_GIF encoding type.  SM
	04/05/00 - added   if (encoding_type == OA_HUFFMAN_FIRST_DIFFERENCE) in front
			   of all lemmego(line_buffer) calls DWS

  Input:  
         input_node - A pointer to an ODL tree node of class IMAGE.
                      The tree above the input_node should have keywords
                      to specify the data file.  If the image is HFD
                      compressed, the input_node must have a sibling
                      identifiable as the encoding histogram for decompression.
       
         The global variable Oa_profile is set to valid values.

  Output:  If successful, the routine returns a pointer to an OA_Object 
           structure, which contains an ODL tree and a pointer to the image
           data.  If unsuccessful it returns NULL.

  Notes:  

*****************************************************************************/

#ifdef _NO_PROTO

OA_OBJECT OaReadImage( input_node, band)
ODLTREE input_node;
int band;

#else

OA_OBJECT OaReadImage( ODLTREE input_node, int band)

#endif
{

static char *proc_name = "OaReadImage";
long file_offset, record_bytes, file_records;
int record_type, src_interchange_format, dst_interchange_format;
char *label_filename, *data_filename;
OA_OBJECT oa_object, histogram_object, partial_image_object;
ODLTREE current_node, sdt, histogram_node, image_node;
ODLTREE compressed_SDT;
SDTNODE sdt_node_ptr;
long object_size, bands;
#ifdef IBM_PC
long i;
#endif
long lines, line_samples, sample_bits, line_prefix_bytes, line_suffix_bytes;
char *buf = NULL, c, *sample_type_str;
PTR data_ptr;
char *line_buffer = NULL;
long line_buffer_bytes, bytes_read;
struct OaStreamStruct *stream_id;
int encoding_type, band_storage_type;
void *hfd_decoding_tree = NULL;

dst_interchange_format = OA_BINARY_INTERCHANGE_FORMAT;

if (OaGetImageKeywords( input_node, &lines, &line_samples, &sample_bits,
                        &sample_type_str, &bands, &band_storage_type,
                        &line_prefix_bytes, &line_suffix_bytes,
                        &encoding_type) != 0)
  return(NULL);

if (bands > 1) {
  if ((band < 1) || (band > bands)) {
    sprintf( error_string, "%s: out of range band value.", proc_name);
    oa_errno = 520;
    OaReportError( error_string);
    return(NULL);
  }
} else {
  band = 1;
}
if (encoding_type == OA_UNKNOWN_ENCODING_TYPE) {
  oa_errno = 520;
  return(NULL);
}
if ((encoding_type == OA_PREVIOUS_PIXEL) || (bands > 1)) {
  if ((line_prefix_bytes != 0) || (line_suffix_bytes != 0)) {
    sprintf( error_string, 
             "%s: LINE_PREFIX_BYTES and LINE_SUFFIX_BYTES are not supported ",
               proc_name);
  strcat( error_string, "for multiband or Previous Pixel compressed images.");
    oa_errno = 520;
    OaReportError( error_string);
    return(NULL);
  }
}

/* If the encoding type is PREVIOUS_PIXEL, use OaReadPartialImage to read it
   in, rather than repeating a lot of previous-pixel decompression code here.
   This isn't done for the other compression types (and uncompressed), because
   for these types the code here is faster than OaReadPartialImage.  */

if (encoding_type == OA_PREVIOUS_PIXEL) {

  if ((partial_image_object = OaOpenImage( input_node, 1)) == NULL)
    return( NULL);
  if ((oa_object = OaReadPartialImage( partial_image_object, 1, lines,
                                       1, line_samples)) == NULL)
    return(NULL);
  OaCloseImage( partial_image_object);

  /* Delete keywords FIRST_LINE, FIRST_SAMPLE, SOURCE_LINES, and
     SOURCE_LINE_SAMPLES which were added by OaOpenImage, since they apply 
     to partial images, and we've just read the whole image.  */

  OaDeleteKwd( "FIRST_LINE", oa_object->odltree);
  OaDeleteKwd( "FIRST_SAMPLE", oa_object->odltree);
  OaDeleteKwd( "SOURCE_LINES", oa_object->odltree);
  OaDeleteKwd( "SOURCE_LINE_SAMPLES", oa_object->odltree);

  return( oa_object);

}  /* end if encoding_type is OA_PREVIOUS_PIXEL */


if (OaGetFileKeywords( input_node, &label_filename, &data_filename,
                       &record_type, &record_bytes, &file_records,
                       &file_offset, &src_interchange_format) != 0) {
  return(NULL);  /* Error message already issued.  */
}

#ifdef OA_DEBUG
OaReportFileAttributes( label_filename, data_filename, record_type,
                        record_bytes, file_offset, src_interchange_format);
#endif

/* Reject cases not currently supported (see limitations above). */

if (((encoding_type == OA_UNCOMPRESSED) ||
    (encoding_type == OA_PREVIOUS_PIXEL)) &&
    (record_type == OA_VARIABLE_LENGTH)) {
  sprintf( error_string, "%s does not support variable-length record files ",
           proc_name);
  strcat( error_string, " except for HFD compressed images.");
  oa_errno = 520;
  OaReportError( error_string);
  return(NULL);
}
if ((encoding_type == OA_HUFFMAN_FIRST_DIFFERENCE) &&
    (record_type != OA_VARIABLE_LENGTH)) {
  sprintf( error_string, "%s: HFD compressed images are only supported",
           proc_name);
  strcat( error_string, " in variable-length record files.");
  oa_errno = 520;
  OaReportError( error_string);
  return(NULL);
}
if ((encoding_type != OA_UNCOMPRESSED) && (bands > 1)) {
  sprintf( error_string, "%s Compressed multi-band images not supported!",
           proc_name);
  oa_errno = 520;
  OaReportError( error_string);
  return(NULL);
}

if (encoding_type == OA_GIF) {
  return( (OA_OBJECT) OalReadImageFromGif( data_filename, file_offset,
                                           lines, line_samples,
                                           input_node));
}

  
if (encoding_type == OA_CLEMENTINE_JPEG) {

  if (sample_bits != 8) {
    sprintf( error_string, 
             "%s: Clementine decompression software does not support %ld ",
             proc_name, sample_bits);
    strcat( error_string, "SAMPLE_BITS");
    oa_errno = 520;
    OaReportError( error_string);
    return( NULL);
  }

  /* Copy the input tree from the IMAGE node down.  */

  image_node = OaCopyTree( input_node, OA_STRIP_COMMENTS | OA_STRIP_SDT_NODES);

  /* Allocate space for decompression software to fill up with the image
     as it's decompressed, and initialize it with 0's.  */

  object_size = lines * line_samples;
  if ((data_ptr = (PTR) OaMalloc( (long) object_size)) == NULL) {
    sprintf( error_string, "%s: OaMalloc failed to allocate %ld bytes for ",
             proc_name, object_size);
    strcat( error_string, "image! Out of memory!");
    oa_errno = 720;
    OaReportError( error_string);
    exit(1);
    /*NOTREACHED*/
  }
  c = 0;
#ifdef IBM_PC

  /* If we're on an IBM-PC, use a for loop to do the set, since memset
     may not work for setting greater than 64K and/or over segment
     boundaries;  otherwise use memset since it's usually optimized.  */

  for (i=0; i<object_size; i++)
    data_ptr[i] = c;
#else
  memset( data_ptr, c, (size_t) object_size);
#endif

  /* Open the file, position the file pointer to the start of the image,
     and pass the file pointer and data_ptr to OalClementineJPEGDecompress. */

  if ((stream_id = OalOpenStream( data_filename, record_type, record_bytes, 
                                  file_records, "r")) == NULL) {
    sprintf( error_string, "%s: OalOpenStream returned error.",
             proc_name);
    OaReportError( error_string);
    OalFreeSDT( image_node);
    return(NULL);
  }
  OalSeek( stream_id, file_offset);
  if (OalClementineJPEGDecompress( data_ptr, lines, line_samples,
                                   stream_id->fp) != 0) {
    return( NULL);
  }
  OalCloseStream( stream_id);

  /* Create the output oa_object and initialize it.  */

  oa_object = OaNewOaObject();
  oa_object->odltree = image_node;
  oa_object->data_ptr = data_ptr;
  oa_object->size = object_size;
  oa_object->is_in_memory = TRUE;
  oa_object->profile = Oa_profile;
  oa_object->stream_id = NULL;

  OaDeleteKwd( "ENCODING_TYPE", image_node);  /* Image now decompressed. */
  OaStrtoKwdValue( "ENCODING_COMPRESSION_RATIO", image_node, "1.00");
  return( oa_object);

} /* end if encoding_type == OA_CLEMENTINE_JPEG.  */


if (encoding_type == OA_HUFFMAN_FIRST_DIFFERENCE) {

  /* Call OaFindEncodingHistogram find the node in the tree describing the 
     encoding histogram needed for HFD decompression; if found it, call
     OaReadHistogram to read the histogram data from the file into memory,
     then call OalCreateHFDTree to build the Huffman decoding tree.  */

  if ((histogram_node = OaFindEncodingHistogram( input_node)) == NULL) {
    sprintf( error_string, "%s: couldn't find encoding histogram for HFD",
             proc_name);
    strcat( error_string, " compressed image.");
    oa_errno = 530;
    OaReportError( error_string);
    return( NULL);
  } else {
    if ((histogram_object = OaReadHistogram( histogram_node)) == NULL) {
      sprintf( error_string, "%s: couldn't read encoding histogram for HFD",
               proc_name);
      strcat( error_string, " compressed image.");
      OaReportError( error_string);
      return( NULL);
    }
  }
  if ((hfd_decoding_tree = OalCreateHFDTree(  histogram_object->data_ptr)) 
                             == NULL) {
    sprintf( error_string, "%s: OalCreateHFDTree returned NULL.", 
             proc_name);
    OaReportError( error_string);
    return( NULL);
  }

  /* Once the HFD tree is created, don't need the encoding histogram
     object anymore, so free it.  */

  OaDeleteObject( histogram_object);
}  /* end if encoding_type == OA_HUFFMAN_FIRST_DIFFERENCE */


/* The input tree ODL tree shouldn't ever be modified, so make a copy of all
   the nodes below the input node.  */

image_node = OaCopyTree( input_node, OA_STRIP_COMMENTS | OA_STRIP_SDT_NODES);

if (OaCheckODLTree( image_node) != 0) {
  OdlFreeTree( image_node);
  return(NULL);  /* Error message already issued and oa_errno set.  */
}

/* For multiband images, modify some keywords to trick OalCreateSDT into
   making the kind of SDT we want.  */

if (bands > 1) {

  /* Delete keywords for BANDS, BAND_STORAGE_TYPE and BAND_SEQUENCE since 
     these will no longer be applicable for the in-memory monochromatic
     image.  */

  OaDeleteKwd( "BANDS",             image_node);
  OaDeleteKwd( "BAND_STORAGE_TYPE", image_node);
  OaDeleteKwd( "BAND_SEQUENCE",     image_node);

  switch( band_storage_type) {

    case OA_BAND_SEQUENTIAL:
      file_offset += (band - 1) * sample_bits/8 * line_samples * lines;
    break;

    case OA_LINE_INTERLEAVED:
      if (band > 1)
        OaLongtoKwdValue( "LINE_PREFIX_BYTES", image_node, 
                          line_samples * sample_bits/8 * (band - 1));
      if (band < bands)
        OaLongtoKwdValue( "LINE_SUFFIX_BYTES", image_node, 
                          line_samples * sample_bits/8 * (bands - band));
    break;

    case OA_SAMPLE_INTERLEAVED:

      /* Trick OalCreateSDT into creating the desired SDT by making it think
         each image line is transposed into an 2-dimensional image, where
         the desired band's pixels are in a column, with undesired band data
         to the left and right of the column, represented by prefix and
         suffix bytes.  Then stack all these images on top of one another to
         get the total transposed image.  The number of lines in this total
         transposed image is equal to the number of pixels in the real image 
         times the number of lines in the real image, and there's only one
         sample per line.  */

      OaLongtoKwdValue( "LINES", image_node, line_samples * lines);
      OaLongtoKwdValue( "LINE_SAMPLES", image_node, (long) 1);

      if (band > 1)
        OaLongtoKwdValue( "LINE_PREFIX_BYTES", image_node, 
                          sample_bits/8 * (band - 1));
      if (band < bands)
        OaLongtoKwdValue( "LINE_SUFFIX_BYTES", image_node, 
                          sample_bits/8 * (bands - band));
    break;
  }
}

/* Call OalCreateSDT to create the SDT (Stream Decomposition Tree).  */

if ((sdt = OalCreateSDT( image_node, src_interchange_format)) == NULL) {
  sprintf( error_string, "%s: CreateSDT returned error.", 
           proc_name);
  OaReportError( error_string);
  OalFreeSDT( image_node);
  return(NULL);
}

/* Note: sdt and image_node now point to the same node. */

/* Undo the keyword changes made above which tricked OalCreateSDT into
   creating the right kind of SDT.  */

if (band_storage_type == OA_SAMPLE_INTERLEAVED) {
  OaLongtoKwdValue( "LINES",        image_node, lines);
  OaLongtoKwdValue( "LINE_SAMPLES", image_node, line_samples);
}

/* Allocate memory for the image.  */

sdt_node_ptr = (SDT_node *) sdt->appl1;
object_size = sdt_node_ptr->dst.size * sdt_node_ptr->total_repetitions;
if ((data_ptr = (PTR) OaMalloc( (long) object_size)) == NULL) {
  sprintf( error_string, "%s: OaMalloc failed to allocate %ld bytes for ",
           proc_name, object_size);
  strcat( error_string, "image! Out of memory!");
  oa_errno = 720;
  OaReportError( error_string);
  exit(1);
  /*NOTREACHED*/
}

c = 0;
#ifdef IBM_PC

/* If we're on an IBM-PC, use a for loop to do the set, since memset
   may not work for setting greater than 64K and/or over segment
   boundaries;  otherwise use memset since it's usually optimized.  */

for (i=0; i<object_size; i++)
  data_ptr[i] = c;
#else
memset( data_ptr, c, (size_t) object_size);
#endif

/* Create the output oa_object and initialize it.  */

oa_object = OaNewOaObject();
oa_object->odltree = sdt;
oa_object->data_ptr = data_ptr;
oa_object->size = object_size;
oa_object->is_in_memory = TRUE;
oa_object->profile = Oa_profile;
oa_object->stream_id = NULL;

if (encoding_type == OA_HUFFMAN_FIRST_DIFFERENCE) {

  /* Allocate a buffer big enough to store a decompressed image line.  If
     there are prefix and/or suffix bytes, they are assumed to be part of
     each decompressed image line.  */

  if (LeftmostChild( sdt) == NULL)  /* No prefix or suffix bytes. */
    line_buffer_bytes = sdt_node_ptr->src.size * line_samples;
  else
    line_buffer_bytes = sdt_node_ptr->src.size;
  if ((line_buffer = (char *) OaMalloc( (size_t) line_buffer_bytes)) == NULL) {
    sprintf( error_string, "%s: OaMalloc failed to allocate %ld bytes ",
             proc_name, line_buffer_bytes);
    strcat( error_string, "for decompressed line buffer! Out of memory!");
    oa_errno = 720;
    OaReportError( error_string);
    exit(1);
    /*NOTREACHED*/
  }
}

if ((stream_id = OalOpenStream( data_filename, record_type, record_bytes, 
                                file_records, "r")) == NULL) {
  sprintf( error_string, "%s: OalOpenStream returned error.",
           proc_name);
  OaReportError( error_string);
  OalFreeSDT( image_node);
  return(NULL);
}

/* Read the first slice of data; tell OalReadStream to read as many bytes as it
   wants, store them in its own buffer, start reading the file at the given
   byte offset, and output how many bytes it read in bytes_read.  */

if (OalReadStream( stream_id, (long) 0, &buf, file_offset, &bytes_read) != 0) {
  sprintf( error_string, "%s: OalReadStream returned error, aborting.",
           proc_name);
  OaReportError( error_string);
  OalFreeSDT( image_node);
  OalCloseStream( stream_id);
  if (encoding_type == OA_HUFFMAN_FIRST_DIFFERENCE) LemmeGo( line_buffer); /*DWS 05-05-00*/
  return(NULL);
}

/* Compress the SDT, and initialize current_node to point to the first
   end-node (data node) in the compressed SDT; initialize the node's dst.ptr
   with the pointer returned by OaMalloc.  */

compressed_SDT = OalCompressSDT( sdt);
current_node = OalInitializeSDT( compressed_SDT, data_ptr);

/* The main loop processes a buffer of data, then reads in the next buffer; 
   the SDT maps desired buffer bytes to memory and throws aways the rest.  */

for (;;) {

  if (encoding_type == OA_HUFFMAN_FIRST_DIFFERENCE)
  {
	  int ret_val;
      ret_val = OalHFDDecompress( buf, line_buffer, &bytes_read, &line_buffer_bytes,  
                      hfd_decoding_tree);
	  if (ret_val == 0)
	  {
			OalFreeSDT( image_node);
			OalCloseStream( stream_id);
			if (encoding_type == OA_HUFFMAN_FIRST_DIFFERENCE) LemmeGo(line_buffer); /*DWS 05-05-00*/
			return(NULL);
	  }
  }
  else {
    line_buffer = buf;
    line_buffer_bytes = bytes_read;
  }

  if (OalProcessSDT( (PTR) line_buffer, line_buffer_bytes, &current_node) ==
      OA_REACHED_END_OF_SDT) break;

  /*  Read next slice of data, telling OalReadStream to do the same as the
      first time, except start at the current file position.  */

  buf = NULL;
  if (OalReadStream( stream_id, (long) 0, &buf, (long) -1, &bytes_read) != 0) {
    sprintf( error_string, "%s: OalReadStream returned error, aborting.",
             proc_name);
    OaReportError( error_string);
    break;
  }
}
OalCloseStream( stream_id);
  if (encoding_type == OA_HUFFMAN_FIRST_DIFFERENCE) LemmeGo( line_buffer); /*DWS 05-05-00*/

/* Free the compressed SDT. */

OalFreeSDT( compressed_SDT);

/* Call OalSDTtoODLTree to modify the new ODL tree to reflect the in-memory 
   data.  This changes the DATA_TYPE keywords to reflect the in-memory data
   values which may have been converted to native types.  It also strips off 
   ODL tree nodes which have SDT_node->dst.size=0.  */

OalSDTtoODLTree( sdt, dst_interchange_format);

/* Remove keywords for ENCODING_TYPE, LINE_PREFIX_BYTES and LINE_SUFFIX_BYTES,
   since these are no longer applicable for the in-memory IMAGE.  */

OaDeleteKwd( "ENCODING_TYPE", image_node);
OaDeleteKwd( "LINE*PREFIX*BYTES", image_node);
OaDeleteKwd( "LINE*SUFFIX*BYTES", image_node);
OaDeleteKwd( "^LINE*PREFIX*STRUCTURE", image_node);
OaDeleteKwd( "^LINE*SUFFIX*STRUCTURE", image_node);

if (encoding_type == OA_HUFFMAN_FIRST_DIFFERENCE)
  OalFreeHFDTree( hfd_decoding_tree);
return( oa_object);
}



/*****************************************************************************

  Routine:  OaReadImagePixels

  Description:  OaReadImagePixels is a low-level routine which returns pixels
                (samples) from an image starting at the specified location
                (line, sample number); band has already been set in
                OaOpenImage.  The number of pixels it returns varies according
                to the start location, the compression type, and the line
                length.  The high level routine OaReadPartialImage makes calls
                to OaReadImagePixels, and is the preferred routine for most
                users.  OaReadImagePixels is part of the sequence of calls:  
                OaOpenImage, OaReadImagePixels (multiple calls), OaCloseImage.
                The samples are decompressed and converted to native format
                according to Oa_profile, and the image handle object's
                data_ptr set pointing to them.  The data file is left open.
                OaOpenImage must be called before making calls to
                OaReadImagePixels.  After making the last call to 
                OaReadImagePixels, call OaCloseImage to free the image handle 
                object and its internal buffers.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  29 Sept  1994
  Last Modified:  02 Feb   1998

  History:

    Creation - This routine was part of the Beta Release of the OA library.
    12/11/96 - Added support for multiband images.  SM
    02/02/98 - Fixed OalSeek bug in HDF part.  SM

  Input:  
         image_handle_object - A pointer to an Oa_Object structure which was
                               initialized by a call to OaOpenImage.  
                               Previous calls may have been made to 
                               OaReadImagePixels.  The image_handle_object's
                               appl1 pointer points to an image_handle
                               structure, which keeps track of the position and
                               number of the last read samples (pixels), and
                               buffers the last read samples.    
                 
         start_line          - The LINE at which to start returning samples;
                               1 <= start_line <= LINES

         start_sample        - The sample number at which to start returning 
                               samples;  1 <= start_sample <= LINE_SAMPLES

         The global variable Oa_profile is set to valid values.

  Output:  If successful, the routine returns the number of pixels read,
           or 0 on error.  The input OA_Object structure contains an ODL tree 
           describing the samples as a IMAGE with 1 line and LINE_SAMPLES
           pixels.  The pixels are pointed to by image_handle_object->data_ptr,
           and image_handle_object->size is set to the total number of bytes.  
           If image_handle_object->data_ptr was non-NULL on input, then it
           has been freed and now points to the newly allocated memory.
         
  Notes:  
  1) The caller should not modify the image_handle structure in
     image_handle_object.
  2) There is a minimal buffering system underlying this routine - the data
     read, compressed and converted from the previous call is saved in a
     buffer.
  3) The image handle object's data_ptr is set to point into the image
     handle's internal buffer, and should NOT be freed, since it's not
     necessarily a base pointer returned by OaMalloc, so don't use
     OaDeleteObject to free the image handle object - use OaCloseImage instead!
  4) Current limitations: 
     a) The supported compression (encoding) types are Previous Pixel and
        Huffman First Difference.
     b) If the image is HFD encoded, it must be in a variable-length record
        file.  
     c) Uncompressed and Previous Pixel compressed images must be in
        fixed-length or undefined record files;  variable-length record files
        are only supported for HFD images.
     d) Previous Pixel encoded images may not have prefix or suffix bytes.
     e) Multi-band images may not be compressed, nor have prefix or suffix 
        bytes.

CHANGE HISTORY:
   01-13-06   MDC     Added preprocessor definition for NASAView to print 
                      error message to screen if we have an OalReadStream
                      error.
******************************************************************************/

#ifdef _NO_PROTO

int OaReadImagePixels( image_handle_object, start_line, start_sample)
OA_OBJECT image_handle_object;
long start_line, start_sample;
#else

int OaReadImagePixels( OA_OBJECT image_handle_object, long start_line, 
                       long start_sample)

#endif
{

static char *proc_name = "OaReadImagePixels";
ODLTREE image_node, current_node;
SDTNODE image_node_sdt_ptr;
char *buf = NULL, *ptr;
long bytes_to_read, bytes_read, bytes, record_number;
long byte_offset, sub_byte_offset, current_samples, buf_sample_offset;
long line_bytes_including_all_bands, requested_sample_offset;
short *decompressed_PP;
struct oa_image_handle *image_handle = NULL;
int ret_val;
short endian_indicator=1;       /* Initialized to 1 to determine the value */
                                /* of *platform_is_little_endian below.    */
char *platform_is_little_endian = (char *) &endian_indicator;
/* If platform is little-endian (lsb first), then the 1 initialized in
   endian_indicator will be in the first byte, and *platform_is_little_endian
   will have the value TRUE (1), otherwise it will be FALSE (0).  A test is
   done on *platform_is_little_endian in the Previous Pixel section below to
   decide whether or not to swap a 2-byte pixel from MSB to LSB order.  */

/* Check input arguments. */

if (image_handle_object == NULL) {
  sprintf( error_string, "%s: input image_handle_object is NULL.", proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return(0);
}

if (image_handle_object->appl1 == NULL) {
  sprintf( error_string, 
           "%s: input image_handle_object must have an image_handle",
           proc_name);
  strcat( error_string, " structure attached via appl1.");
  oa_errno = 501;
  OaReportError( error_string);
  return(0);
}

image_handle = (struct oa_image_handle *) image_handle_object->appl1;
if ((start_line < 1) || (start_line > image_handle->source_lines) || 
    (start_sample < 1) || (start_sample > image_handle->source_line_samples)) {
  sprintf( error_string,
           "%s: invalid start_line or start_sample.", proc_name);
  oa_errno = 502;
  OaReportError( error_string);
  return(0);
}

if ((image_handle->compression_type != OA_UNCOMPRESSED) &&
    (image_handle->compression_type != OA_HUFFMAN_FIRST_DIFFERENCE) &&
    (image_handle->compression_type != OA_PREVIOUS_PIXEL)) {
  sprintf( error_string, "%s: unsupported compression type: %d.", proc_name,
           (int) image_handle->compression_type);
  oa_errno = 520;
  OaReportError( error_string);
  return(0);
}

image_node = image_handle_object->odltree;
image_node_sdt_ptr = (SDT_node *) image_node->appl1;

/* Three independent code sections follow, for HUFFMAN_FIRST_DIFFERENCE,
   UNCOMPRESSED, and PREVIOUS_PIXEL.  */



if (image_handle->compression_type == OA_HUFFMAN_FIRST_DIFFERENCE) {

  /* (Re)initialize the SDT so it's ready to process a new image line.  */

  current_node = OalInitializeSDT( image_node, image_handle->buf);

  /* If the requested start line isn't in the image_handle's buffer, have to
     go to the file.  The line number of the data in the buffer is one less 
     than image_handle->next_line, since one line of data was saved from the
     last call.  */

  if ((start_line != (image_handle->next_line - 1)) || 
      (image_handle->buf_samples == 0)) {

    /* Start_line isn't buffered, so have to go to the file.  If we aren't
       already positioned to read the start_line, call OaSeek to position
       there.  HFD compressed images are always in variable-length record 
       files, with one record for each line;  however, records and lines don't
       necessarily match one-to-one because the image may not start at the
       first record.  Note: stream_id->current_position starts at 0, and 
       start_line and image_handle->next_line start at 1.  */

    if (start_line != image_handle->next_line) {

      record_number = image_handle_object->stream_id->current_position + 
                      (start_line - image_handle->next_line);
      if (OalSeek( image_handle_object->stream_id, record_number) != 0) {
        sprintf( error_string, "%s: OalSeek returned error, aborting.",
                 proc_name);
        OaReportError( error_string);
        return(0);
      }
      image_handle->next_line = start_line;
    }

    /* Read one variable-length record, which contains one compressed line;
       decompress it and filter it through the SDT.  */

    buf = NULL;
    if (OalReadStream( image_handle_object->stream_id, 0, &buf, -1, 
                       &bytes_read) != 0) {
      sprintf( error_string, "%s: OalReadStream returned error, aborting.",
               proc_name);
      OaReportError( error_string);
      return(0);
    }
    image_handle->next_line += 1;
    bytes = image_handle->decomp.HFD.line_buffer_bytes;
    ret_val = OalHFDDecompress( buf, image_handle->decomp.HFD.line_buffer, &bytes_read, 
                      &bytes, image_handle->decomp.HFD.decoding_tree);
	if(ret_val == 0) return (0);
    if (bytes != image_handle->decomp.HFD.line_buffer_bytes) {
      sprintf( error_string, "%s: SDT source size is not equal to size of HFD",
               proc_name);
      strcat( error_string, " decompressed line buffer.");
      oa_errno = 710;
      OaReportError( error_string);
      return(0);
    }
    OalProcessSDT( (PTR) image_handle->decomp.HFD.line_buffer, bytes,  
                   &current_node);
  }  /* end if requested line isn't in image_handle's buffer */

  /* Requested line is now in the image_handle's buffer, so set the 
     image handle object's data_ptr to point to the start sample in the
     buffer.  */

  bytes = (image_handle->source_line_samples - start_sample + 1) * 
           image_handle->sample_bytes;
  image_handle_object->data_ptr = (PTR) image_handle->buf + 
                                  (start_sample-1)*image_handle->sample_bytes;
  image_handle_object->size = bytes;
  image_handle_object->is_in_memory = TRUE;

  /* Update keywords describing the samples now in pointed to by
     image_handle_object->data_ptr.  */

  OaLongtoKwdValue( "LINE_SAMPLES", image_node, 
                     image_handle->source_line_samples - start_sample + 1);
  OaLongtoKwdValue( "FIRST_LINE", image_node, (long) start_line);
  OaLongtoKwdValue( "FIRST_SAMPLE", image_node, (long) start_sample);

  return( (int) image_handle->source_line_samples - start_sample + 1);

}  /* end if compression_type == OA_HUFFMAN_FIRST_DIFFERENCE  */



if (image_handle->compression_type == OA_UNCOMPRESSED) {

  /* (Re)initialize the SDT so it's ready to process a new image line.  */

  current_node = OalInitializeSDT( image_node, image_handle->buf);

  /* If start_line isn't in the image_handle's buffer, have to go to the file.
     The line number of the data in the buffer saved from the last call is one
     less than image_handle->next_line.  */

  if ((start_line != (image_handle->next_line - 1)) || 
      (image_handle->buf_samples == 0)) {

    /* Start_line isn't buffered, so go to the file.  If we aren't already
       positioned to read start_line, call OaSeek to position there.  
       Notes on OalSeek:
       1) Uncompressed images are only supported in fixed-length and undefined
          record files.  The record length may be less than, equal to, or
          greater than an image line (plus prefix and suffix bytes, if any).  
       2) Monochromatic images are read one line at a time; the current line is
          buffered, and the file pointer is left pointing to the beginning of
          the next line.
       3) Multi-band images are read one line of one band at a time, except 
          for pixel interleaved images, where one line containing all the
          bands for that line are read.  This means the file pointer is left
          pointing to the next line, except for line interleaved images,
          where it is left pointing to the next band in the line just read.
       4) stream_id->current_position starts at 0, and start_line and
          image_handle->next_line start at 1.  */

    if (start_line != image_handle->next_line) {

      if (image_handle->band_storage_type == OA_LINE_INTERLEAVED) {
        line_bytes_including_all_bands = image_handle->source_line_samples *
                                         image_handle->sample_bytes * 
                                         image_handle->bands;

        /* sub_byte_offset is the number of bytes from the current band
           back to the first band of this line.  First we get back to the
           first band in this line, then calculate the offset to the 
           desired line from there.  */

        sub_byte_offset = (image_handle_object->stream_id->current_position -
                           image_handle->image_start_offset) % 
                           line_bytes_including_all_bands;
        byte_offset = image_handle_object->stream_id->current_position -
                      sub_byte_offset +
                      (start_line - image_handle->next_line) * 
                      line_bytes_including_all_bands;
      } else {
        byte_offset = image_handle_object->stream_id->current_position + 
                      (start_line - image_handle->next_line) * 
                      image_node_sdt_ptr->src.size *
                      image_node_sdt_ptr->total_repetitions;
      }
      if (OalSeek( image_handle_object->stream_id, byte_offset) != 0) {
        sprintf( error_string, "%s: OalSeek returned error, aborting.",
                 proc_name);
        OaReportError( error_string);
        return(0);
      }
      image_handle->next_line = start_line;
    }   /* end if line not in image_handle buffer */

    /* Loop calling OalReadStream and OalProcessSDT until exactly one line has
       been processed.  Since the Stream Layer is reading a fixed-length or
       undefined record file, it can read an exact specified number of bytes
       (not so for variable-length record files, which is why they're not
       supported here).  */

    bytes_to_read = image_node_sdt_ptr->src.size * 
                    image_node_sdt_ptr->total_repetitions;
    while (bytes_to_read > 0) {
      buf = NULL;
      if (OalReadStream( image_handle_object->stream_id, bytes_to_read, 
                         &buf, -1, &bytes_read) != 0) {
        sprintf( error_string, "%s: OalReadStream returned error, aborting.",
                 proc_name);
        OaReportError( error_string);
		/* 1-13-06 MDC - Check condition and print out error message to screen if we could not read enough bytes */
#ifdef XVT_DEF
		xvt_dm_post_error("OalReadStream returned error, aborting.");
		if(bytes_to_read != bytes_read)
		{
			xvt_dm_post_error
				("Number of bytes read was %d.\nNumber of bytes attempted to be read was %d at line = %d, sample = %d of image", bytes_read, bytes_to_read, start_line, start_sample);
		}
#endif
        return(0);
      }
      bytes_to_read -= bytes_read;
      if (OalProcessSDT( (PTR) buf, bytes_read, &current_node) == 
         OA_REACHED_END_OF_SDT) break;
    }
    if (bytes_to_read != 0) {
      sprintf( error_string, 
               "%s: total bytes read from file: %d is not equal to size",
               proc_name, (int) (image_node_sdt_ptr->src.size - 
                                 bytes_to_read));
      strcat( error_string, " of source line in SDT");
      strcat( error_string, 
              "\nImage position and file position are out-of-sync.");
      oa_errno = 710;
      OaReportError( error_string);
      return(0);
    }
    image_handle->next_line += 1;
  }  /* end if requested line isn't in image_handle's buffer */

  /* Requested line is now in the image_handle's buffer, so set the 
     image handle object's data_ptr to point to the start sample in the
     buffer.  */

  bytes = (image_handle->source_line_samples - start_sample + 1) * 
           image_handle->sample_bytes;
  image_handle_object->data_ptr = (PTR) image_handle->buf +  
                                  (start_sample-1) *  
                                  image_handle->sample_bytes;
  image_handle_object->size = bytes;
  image_handle_object->is_in_memory = TRUE;

  /* Update keywords describing the samples now in pointed to by
     image_handle_object->data_ptr.  */

  OaLongtoKwdValue( "LINE_SAMPLES", image_node, 
                     image_handle->source_line_samples - start_sample + 1);
  OaLongtoKwdValue( "FIRST_LINE", image_node, (long) start_line);
  OaLongtoKwdValue( "FIRST_SAMPLE", image_node, (long) start_sample);
  return( (int) (image_handle->source_line_samples - start_sample + 1));

}  /* end if compression_type == OA_UNCOMPRESSED */



if (image_handle->compression_type == OA_PREVIOUS_PIXEL) {

  /* If the requested start_line, start_sample isn't in the image_handle's
     buffer, have to go to the file.  requested_sample_offset and 
     buf_sample_offset are the offsets in sample units of the requested
     sample and the first sample in the buffer, respectively; both start at 0.
     Note: image_handle->buf contains image_handle->buf_samples samples saved
     by the last call.  */

  requested_sample_offset  = (start_line-1) * 
                              image_handle->source_line_samples + 
                             (start_sample-1);
  buf_sample_offset = (image_handle->next_line-1) * 
                      image_handle->source_line_samples +
                      (image_handle->next_sample-1) - 
                      image_handle->buf_samples;

  if (requested_sample_offset < buf_sample_offset) {

    /* Requested start_sample isn't in the image_handle's buffer, and it
       occurs in the file before the next sample the file pointer is positioned
       at, so reset the file pointer to the start of the image and start all
       over again.  */

    if (OalSeek( image_handle_object->stream_id, 
                 image_handle->image_start_offset) != 0) {
      sprintf( error_string, "%s: OalSeek returned error, aborting.",
               proc_name);
      OaReportError( error_string);
      return(0);
    }
    image_handle->buf_samples = 0;
    image_handle->next_line = 1;
    image_handle->next_sample = 1;
    image_handle->decomp.PP.found_255 = FALSE;
    image_handle->decomp.PP.previous_byte_exists = FALSE;
    buf_sample_offset = 0;
  }

  /* Loop reading from the file until the offset of the last sample in the 
     buffer is greater than or equal to the offset of the requested start
     pixel, i.e. until the start pixel is in the buffer.  Since this may be
     the first read, the buffer might be empty, so check buf_samples too.  */

  decompressed_PP = (short *) image_handle->buf;
  while ((image_handle->buf_samples == 0) ||
         (buf_sample_offset + image_handle->buf_samples <=
          requested_sample_offset)) {

    /* Read a single stream_id->buf full of data.  */

    buf = NULL;
    if (OalReadStream( image_handle_object->stream_id, 0, &buf, -1, 
                       &bytes_read) != 0) {
      sprintf( error_string, "%s: OalReadStream returned error, aborting.",
               proc_name);
      OaReportError( error_string);
      return(0);
    }

    /* Decompress the data in stream_id->buf.  If this isn't the first buffer
       of compressed data, then the decompression state was saved as follows,
       depending on the last byte in the previous buffer:
       1) If the last byte was a 255 flagging the start of a 2-byte pixel, 
          found_255 = TRUE.
       2) If the last byte occurred after such a 255, found_255 = TRUE,
          previous_byte = <last byte>, and previous_byte_exists = TRUE.
       Note that a 255 can occur in either of the 2 bytes following the 255
       which flags the start of a pixel.  */

    current_samples = 0;

    /* If this is the beginning of the compressed data stream, check that the
       first byte is a 255.  */

    if ((image_handle->buf_samples == 0) && 
        (image_handle->decomp.PP.found_255 == FALSE) &&
        ((unsigned char) buf[0] != 255)) {
      sprintf( error_string, "%s: first byte in Previous Pixel compressed ",
               proc_name);
      strcat( error_string, "data was not a 255.");
      oa_errno = 710;
      OaReportError( error_string);
      return(0);
    }

    /* Loop through all the bytes in the buffer, decompressing them.  */

    for (byte_offset = 0; byte_offset < bytes_read; byte_offset++) {
      if (image_handle->decomp.PP.found_255 == TRUE) {
        if (image_handle->decomp.PP.previous_byte_exists == TRUE) {

          /* The last byte was the first (MSB) byte of a 2-byte pixel;
             combine it with the current (LSB) byte, store it, and 
             re-initialize the parsing variables.  */

          ptr = (char *) &(decompressed_PP[ current_samples]);
          if (*platform_is_little_endian == TRUE) {
            ptr[0] = buf[ byte_offset];
            ptr[1] = image_handle->decomp.PP.previous_byte;  /* MSB */
          } else {
            ptr[1] = buf[ byte_offset];
            ptr[0] = image_handle->decomp.PP.previous_byte;  /* MSB */
          }
          image_handle->decomp.PP.previous_pixel = 
            decompressed_PP[ current_samples];
          current_samples++;
          image_handle->decomp.PP.found_255 = FALSE;
          image_handle->decomp.PP.previous_byte_exists = FALSE;

        } else {

          /* The last byte was a 255 marking the start of a 2-byte pixel;
             the current byte is the first (MSB) byte of the pixel; save it
             in previous_byte.  */

          image_handle->decomp.PP.previous_byte = buf[ byte_offset];
          image_handle->decomp.PP.previous_byte_exists = TRUE;
        }

      } else {

        /* There is no stored-up data, so examine the next byte.  */

        if ((unsigned char) buf[ byte_offset] == 255) {
          image_handle->decomp.PP.found_255 = TRUE;
        } else {
          decompressed_PP[ current_samples] = 
            image_handle->decomp.PP.previous_pixel +
            (unsigned char) buf[byte_offset] - 127;
          image_handle->decomp.PP.previous_pixel = 
            decompressed_PP[ current_samples];
          current_samples++;
        }
      }
    }  /* end for loop processing all bytes in buf.  */

    /* Update the variables which keep track of the position of the next pixel
       to be read, and the offset (in pixels from the beginning of the
       image) of the first sample in the buffer.  */

    buf_sample_offset += image_handle->buf_samples;
    image_handle->buf_samples = current_samples;
    image_handle->next_line = (buf_sample_offset + current_samples) / 
                              image_handle->source_line_samples + 1;
    image_handle->next_sample = (buf_sample_offset + current_samples) %  
                                image_handle->source_line_samples + 1;

  }  /* end while requested start pixel isn't in image_handle's buffer */

  /* Requested start pixel is now in the image_handle's buffer, so set the
     image handle object's data_ptr to point to the start sample in the
     buffer.  */

  bytes = (image_handle->buf_samples - 
          (requested_sample_offset - buf_sample_offset)) * 2;
  image_handle_object->data_ptr = (PTR) image_handle->buf + 
                                  image_handle->buf_samples*2 - bytes;
  image_handle_object->size = bytes;
  image_handle_object->is_in_memory = TRUE;

  /* Update keywords describing the samples now in pointed to by
     image_handle_object->data_ptr.  */

  OaLongtoKwdValue( "LINE_SAMPLES", image_node, (long) (bytes/2));
  OaLongtoKwdValue( "FIRST_LINE", image_node, (long) start_line);
  OaLongtoKwdValue( "FIRST_SAMPLE", image_node, (long) start_sample);
  return( (int) bytes/2);

}  /* end if compression_type == OA_PREVIOUS_PIXEL  */
/*NOTREACHED*/
return(0);
}



/*****************************************************************************

  Routine:  OaReadImageFromQube

  Description:  OaReadImageFromQube opens a data file and reads in the IMAGE
                for the specified band from the core of the QUBE into memory.
                It throws out all suffixes, if any are present.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  27 Nov   1995
  Last Modified:  09 Mar   1998

  History:

    Creation - This routine was part of the Version 1.0 Release of the OA
               library.
    03/09/98 - Added BIL and suffix plane support.  SM
	01/12/03 - There are now THEMIS qubes which may or may not have 
			   SUFFIX_ITEMS keywords.  That has been allowed for below

  Input:  
         input_node - A pointer to an ODL tree node of class QUBE.
                      The tree above the input_node should have keywords
                      to specify the data file.

         band       - The band to read.
       
         The global variable Oa_profile is set to valid values.

  Output:  If successful, the routine returns a pointer to an OA_Object 
           structure, which contains an ODL tree of type IMAGE and a pointer
           to the image data.  If unsuccessful it returns NULL.

  Notes:  

  1) Suffixes: in standard ISIS Qubes, each item in a suffix is allocated 4
     bytes, even though the data type stored there may be less than 4 bytes;
     in fact, some Qubes have suffixes allocated, but no actual suffix data
     in them.  This is so the user can add suffix data without rewriting the
     file.

 CHANGE HISTORY:
     12-02-05   MDC     Added statements for preprocessor definition XVT_DEF
	                    to print out to the screen if there was an error in
						reading the stream
	 01-12-06  MDC/DWS  Modified routine to correctly calculate the number of
	                    bytes we should be reading from the Qube
*****************************************************************************/

#ifdef _NO_PROTO

OA_OBJECT OaReadImageFromQube( input_node, band)
ODLTREE input_node;
int     band;

#else

OA_OBJECT OaReadImageFromQube( ODLTREE input_node, int band)

#endif
{

static char *proc_name = "OaReadImageFromQube";
long file_offset, record_bytes, file_records;
int record_type, src_interchange_format, dst_interchange_format;
char *label_filename, *data_filename;
OA_OBJECT oa_object;
ODLTREE current_node, sdt, image_node;
ODLTREE compressed_SDT;
SDTNODE sdt_node_ptr;
long object_size, line_samples, lines, bands, core_item_bytes, skip_bytes;
#ifdef IBM_PC
long i;
#endif
long *suffix_items, *core_items, line_suffix_bytes, bytes_read;
char *buf = NULL, c, **axis_names, *core_item_type;
PTR data_ptr;
struct OaStreamStruct *stream_id;
int band_storage_type=OA_UNKNOWN_BAND_STORAGE_TYPE;
unsigned long total_bytes_read=0, total_bytes_to_read;
KEYWORD *i_id_kwdptr;

dst_interchange_format = OA_BINARY_INTERCHANGE_FORMAT;

if (OaGetQubeKeywords( input_node, &core_items, &axis_names, &suffix_items,
                       &core_item_bytes, &core_item_type) != 0)
  return( NULL);

if (OaGetFileKeywords( input_node, &label_filename, &data_filename,
                       &record_type, &record_bytes, &file_records,
                       &file_offset, &src_interchange_format) != 0) {
  return(NULL);  /* Error message already issued.  */
}

#ifdef OA_DEBUG
OaReportFileAttributes( label_filename, data_filename, record_type,
                        record_bytes, file_offset, src_interchange_format);
#endif

/* The storage orders this routine can currently handle are band-sequential,
   indicated by AXIS_NAME = (SAMPLE, LINE, BAND), and band-interleaved-by-
   line, indicated by AXIS_NAME = (SAMPLE, BAND, LINE).  */

if ((strcmp( axis_names[0], "SAMPLE") == 0) &&
    (strcmp( axis_names[1], "LINE") == 0) &&
    (strcmp( axis_names[2], "BAND") == 0)) {
  band_storage_type = OA_BAND_SEQUENTIAL;
  bands = core_items[2];
}
if ((strcmp( axis_names[0], "SAMPLE") == 0) &&
    (strcmp( axis_names[1], "BAND") == 0) &&
    (strcmp( axis_names[2], "LINE") == 0)) {
  band_storage_type = OA_LINE_INTERLEAVED;
  bands = core_items[1];
}
if ((strcmp( axis_names[0], "BAND") == 0) &&
    (strcmp( axis_names[1], "SAMPLE") == 0) &&
    (strcmp( axis_names[2], "LINE") == 0)) {
  band_storage_type = OA_SAMPLE_INTERLEAVED;
  bands = core_items[0];
}
if ((band_storage_type != OA_BAND_SEQUENTIAL) &&
    (band_storage_type != OA_LINE_INTERLEAVED) &&
    (band_storage_type != OA_SAMPLE_INTERLEAVED)) { /* added this choice DWS  08-15-04*/
  sprintf( error_string, "%s: Cannot handle this storage order: %s %s %s",
           proc_name, axis_names[0], axis_names[1], axis_names[2]);
  oa_errno = 520;
  OaReportError( error_string);
  return( NULL);
}

/* Check that input 'band' is within the range of existing bands.  */

if ((band < 1) || (band > bands)) {
  sprintf( error_string, "%s: input band is out-of-range", proc_name);
  oa_errno = 502;
  OaReportError( error_string);
  return( NULL);
}


/* The technique used to read in an image from the qube is trick OalCreateSDT
   into thinking it's creating an SDT for an image.  We use LINE_SUFFIX_BYTES
   to represent all the data between the end of one image line and the start
   of the next image line, so that all this data will be thrown out;  this
   includes suffixes in various dimensions, corners, and all the core data
   belonging to other bands.  Then we set file_offset to be the start of
   the qube plus all the interveaning core, suffixes, corners etc. to get
   to the start of the first line in the desired band.  In the case of the
   last line, if our LINE_SUFFIX_BYTES goes past the end of the file, then
   OalReadStream will give an error which we'll catch by keeping track of
   whether we've read all the image lines (total_bytes_read >= 
   total_bytes_to_read).  This can't happen for band-sequential, but can 
   for band-interleaved-by-line, and maybe for band_interleaved-by_pixel.  */
/*****************SAMPLE LINE BAND *******************************************/
if (band_storage_type == OA_BAND_SEQUENTIAL) {
	lines = core_items[1];
	
	line_samples = core_items[0];
	/*THEMIS QUESTION DWS 09-10-02*/
	/*If there is a SUFFIX_ITEMS keyword then we will procede as in a normal*/
	/*qube, otherwise this is probably a THEMIS cube.  A THEMIS cube may or */
	/*may not have this keyword.  If the keyword is not present then        */
	/*initialize the two variables                                          */
	if((OdlFindKwd(input_node, "SUFFIX_ITEMS", NULL, 1, ODL_TO_END) != NULL))
	{
		skip_bytes = (suffix_items[1] * 4) * line_samples;     /* bottom plane */
		skip_bytes += (suffix_items[0] * 4) * suffix_items[1]; /* corner */
		line_suffix_bytes = suffix_items[0] * 4;
	}
	else
	{
		skip_bytes = 0;
		line_suffix_bytes = 0;
	}

  /* Calculate the offset into the QUBE, in bytes, of the first pixel of the
     first line of the desired image, and add it to the offset of start of the
     QUBE.  */

  file_offset += (((line_samples * core_item_bytes + line_suffix_bytes) *
                 lines) + skip_bytes) * (band-1);
  total_bytes_to_read = (line_samples * core_item_bytes + line_suffix_bytes) *
                         lines;
}
/****SAMPLE BAND LINE*********************************************************/

if (band_storage_type == OA_LINE_INTERLEAVED) {
  line_samples = core_items[0];
  lines = core_items[2];
  line_suffix_bytes = suffix_items[0] * 4;

  /* Core and line suffix bytes between end of line (beginning of line's
     suffix bytes) and start of next line in same band. */
  skip_bytes = (line_samples * core_item_bytes + line_suffix_bytes) *
               (bands - 1) + line_suffix_bytes;

  /* Add one line's worth of backplane bytes.  */
  skip_bytes += suffix_items[1] * 4 * line_samples;

  /* Add corner bytes. */
  skip_bytes += suffix_items[1] * line_suffix_bytes;

  /* Calculate the offset into the QUBE, in bytes, of the first pixel of the
     first line of the desired image, and add it to the offset of start of the
     QUBE.  */

  file_offset += ((line_samples * core_item_bytes + line_suffix_bytes) *
                  (band-1));
  total_bytes_to_read = (line_samples * core_item_bytes + skip_bytes) *
                         lines - skip_bytes + line_suffix_bytes;
  line_suffix_bytes = skip_bytes;
}
/******BAND SAMPLE LINE *********************************new for interleaved by pixel   DWS  08-15-04*/
if (band_storage_type == OA_SAMPLE_INTERLEAVED) {
  line_samples = core_items[1];
  lines = core_items[2];
  line_suffix_bytes = suffix_items[0]* 4;
  skip_bytes = 0;
  /* Calculate the offset into the QUBE, in bytes, of the first pixel of the
     first line of the desired image, and add it to the offset of start of the
     QUBE.  */
  file_offset +=   (band-1) * core_item_bytes;
/*  total_bytes_to_read = record_bytes * line_samples *lines; */
  total_bytes_to_read = record_bytes * line_samples *lines - (band -1) * core_item_bytes; /**DWS     1/10/06**/

  line_suffix_bytes = skip_bytes;
}

/***************************************************************************/

/* Set up an ODL tree describing an image, then let OalCreateSDT do the work
   of creating the SDT.  */

image_node = OdlNewObjDesc( "IMAGE", NULL, NULL, NULL, NULL, NULL, 
                            (short) 0, (long) 0);
OaLongtoKwdValue( "LINE_SAMPLES", image_node, line_samples);
OaLongtoKwdValue( "SAMPLE_BITS", image_node, core_item_bytes * 8);
OaLongtoKwdValue( "LINES", image_node, lines);
OaStrtoKwdValue( "SAMPLE_TYPE", image_node, core_item_type);
if (line_suffix_bytes > 0)
  OaLongtoKwdValue( "LINE_SUFFIX_BYTES", image_node, line_suffix_bytes);


/* Call OalCreateSDT to create the SDT (Stream Decomposition Tree).  */

if ((sdt = OalCreateSDT( image_node, src_interchange_format)) == NULL) {
  sprintf( error_string, "%s: CreateSDT returned error.", 
           proc_name);
  OaReportError( error_string);
  OalFreeSDT( image_node);
  return(NULL);
}

/* Note: sdt and image_node now point to the same node. */

/* Allocate memory for the image.  */

sdt_node_ptr = (SDT_node *) sdt->appl1;
object_size = sdt_node_ptr->dst.size * sdt_node_ptr->total_repetitions;
if ((data_ptr = (PTR) OaMalloc( (long) object_size)) == NULL) {
  sprintf( error_string, "%s: OaMalloc failed to allocate %ld bytes for ",
           proc_name, object_size);
  strcat( error_string, "image! Out of memory!");
  oa_errno = 720;
  OaReportError( error_string);
  exit(1);
  /*NOTREACHED*/
}
if (dst_interchange_format == OA_ASCII_INTERCHANGE_FORMAT) 
  c = ' ';
else
  c = 0;

#ifdef IBM_PC

/* If we're on an IBM-PC, use a for loop to do the set, since memset
   may not work for setting greater than 64K and/or over segment
   boundaries;  otherwise use memset since it's usually optimized.  */

for (i=0; i<object_size; i++)
  data_ptr[i] = c;
#else
memset( data_ptr, c, (size_t) object_size);
#endif

/* Create the output oa_object and initialize it.  */

oa_object = OaNewOaObject();
oa_object->odltree = sdt;
oa_object->data_ptr = data_ptr;
oa_object->size = object_size;
oa_object->is_in_memory = TRUE;
oa_object->profile = Oa_profile;
oa_object->stream_id = NULL;

if ((stream_id = OalOpenStream( data_filename, record_type, record_bytes, 
                                file_records, "r")) == NULL) {
#ifdef XVT_DEF
	xvt_dm_post_note("%s: OalOpenStream returned error.", proc_name);
#else
  sprintf( error_string, "%s: OalOpenStream returned error.",
           proc_name);
  OaReportError( error_string);
#endif
  OalFreeSDT( image_node);
  return(NULL);
}

/* Read the first slice of data; tell OalReadStream to read as many bytes as it
   wants, store them in its own buffer, start reading the file at the given
   byte offset, and output how many bytes it read in bytes_read.  */
if(band_storage_type != OA_SAMPLE_INTERLEAVED)                  /*added this test for sample interleaved  DWS  11/01/04*/
{
	if (OalReadStream( stream_id, (long) 0, &buf, file_offset, &bytes_read) != 0)
	{
#ifdef XVT_DEF
		xvt_dm_post_note("%s: OalOpenStream returned error, aborting", proc_name);
#else
		sprintf( error_string, "%s: OalReadStream returned error, aborting.",
																	   proc_name);
		OaReportError( error_string);
#endif
		OalFreeSDT( image_node);
		OalCloseStream( stream_id);
		return(NULL);
	}
}
else                                                           /*added this else for sample interleaved  DWS  11/01/04*/
{
	if (OalReadStream( stream_id, (long) record_bytes, &buf, file_offset, &bytes_read) != 0) 
	{
#ifdef XVT_DEF
		xvt_dm_post_note("%s: OalReadStream returned error, aborting", proc_name);
#else
		sprintf( error_string, "%s: OalReadStream returned error, aborting.",
																       proc_name);
		OaReportError( error_string);
#endif
		OalFreeSDT( image_node);
		OalCloseStream( stream_id);
		return(NULL);
	}
}
total_bytes_read += bytes_read;

/* Compress the SDT, and initialize current_node to point to the first
   end-node (data node) in the compressed SDT; initialize the node's dst.ptr
   with the pointer returned by OaMalloc.  */

compressed_SDT = OalCompressSDT( sdt);
current_node = OalInitializeSDT( compressed_SDT, data_ptr);

/* The main loop processes a buffer of data, then reads in the next buffer; 
   the SDT maps desired buffer bytes to memory and throws aways the rest.  */

for (;;) {

	if(band_storage_type != OA_SAMPLE_INTERLEAVED)                  /*added this test for sample interleaved  DWS  11/01/04*/
	{
		if (OalProcessSDT( (PTR) buf, bytes_read, &current_node) ==
			OA_REACHED_END_OF_SDT) break;
	}
	else                                                            /*added this else for sample interleaved DWS 11/01/04*/
	{
		if (OalProcessSDT( (PTR) buf + (bands -1 ) * core_item_bytes, core_item_bytes, &current_node) ==
			OA_REACHED_END_OF_SDT) break;
	}

  /*  Read next slice of data, telling OalReadStream to do the same as the
      first time, except start at the current file position.  */

  /* 01-12-06 MDC - Add in the number of bytes read if the OalReadStream function fails. */

  buf = NULL;
  if(band_storage_type != OA_SAMPLE_INTERLEAVED)                  /*added this test for sample interleaved  DWS  11/01/04*/
  {
      if (OalReadStream( stream_id, (long) 0, &buf, (long) -1, &bytes_read) != 0) 
	  {
          total_bytes_read += bytes_read;
		  if (total_bytes_read < total_bytes_to_read) 
		  {
#ifdef XVT_DEF
			  xvt_dm_post_note
			   ("%s: OalReadStream returned error, aborting. Total bytes read (%d) is less than actual number of bytes to read (%d)", proc_name, total_bytes_read, total_bytes_to_read);
#else
			  sprintf( error_string, "%s: OalReadStream returned error, aborting.", proc_name);
			  OaReportError( error_string);
#endif
		  }
		  break;
	  }
  }
  else                                                            /*added this else for sample interleaved DWS 11/01/04*/
  {
	 if (OalReadStream( stream_id, (long) record_bytes, &buf, (long) -1, &bytes_read) != 0) 
	 {
         total_bytes_read += bytes_read;
		 if (total_bytes_read < total_bytes_to_read) 
		 {
#ifdef XVT_DEF
			xvt_dm_post_note("%s: OalReadStream returned error, aborting. Total bytes read (%d) is less than actual number of bytes to read (%d)", proc_name, total_bytes_read, total_bytes_to_read);
#else
			sprintf( error_string, "%s: OalReadStream returned error, aborting.",
                                                                    proc_name);
			OaReportError( error_string);
#endif
		 }
		 break;
	 }
  }

  total_bytes_read += bytes_read;
}
OalCloseStream( stream_id);

/* Free the compressed SDT. */

OalFreeSDT( compressed_SDT);

/* Call OalSDTtoODLTree to modify the new ODL tree to reflect the in-memory 
   data.  This changes the DATA_TYPE keywords to reflect the in-memory data
   values which may have been converted to native types.  It also strips off 
   ODL tree nodes which have SDT_node->dst.size=0.  */

OalSDTtoODLTree( sdt, dst_interchange_format);

OaDeleteKwd( "LINE*SUFFIX*BYTES", image_node);  /* No longer applicable.  */

return( oa_object);
}



/*****************************************************************************

  Routine:  OaReadSpectrumFromImage

  Description:  This routine reads a spectrum from a multi-band image, at
                the given line/sample location.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  11 Dec   1996
  Last Modified:  11 Dec   1996

  History:

    Creation - This routine was part of the Version 1.2 release of the OA
               library.

  Input:  
         input_node - A pointer to an ODL tree node of class IMAGE.
                      The tree above the input_node should have keywords
                      to specify the data file.

         line       - The line to read the spectrum from.

         sample     - The sample to read the spectrum from.

  Output:  If successful, the routine returns a pointer to an OA_Object 
           structure, which contains an ODL tree and a pointer to the spectrum
           data.  The spectrum contains all the bands of the image, at the
           given line/sample location.  If unsuccessful it returns NULL.

  Notes:  

*****************************************************************************/

#ifdef _NO_PROTO

OA_OBJECT OaReadSpectrumFromImage( input_node, line, sample)
ODLTREE input_node;
int line;
int sample;

#else

OA_OBJECT OaReadSpectrumFromImage( ODLTREE input_node, int line, int sample)

#endif
{

static char *proc_name = "OaReadSpectrumFromImage";
long file_offset, record_bytes, file_records;
int record_type, src_interchange_format, dst_interchange_format;
char *label_filename, *data_filename;
OA_OBJECT oa_object;
ODLTREE current_node, sdt, spectrum_node, column_node, compressed_SDT;
SDTNODE sdt_node_ptr;
long object_size, bands, bytes_read;
#ifdef IBM_PC
long i;
#endif
long lines, line_samples, sample_bits, line_prefix_bytes, line_suffix_bytes;
char *buf = NULL, c, *sample_type_str, *str;
PTR data_ptr;
struct OaStreamStruct *stream_id;
int encoding_type, band_storage_type;

dst_interchange_format = OA_BINARY_INTERCHANGE_FORMAT;

if (OaGetImageKeywords( input_node, &lines, &line_samples, &sample_bits,
                        &sample_type_str, &bands, &band_storage_type,
                        &line_prefix_bytes, &line_suffix_bytes,
                        &encoding_type) != 0)
  return(NULL);

if ((line < 1) || (line > lines) || (sample < 1) || (sample > line_samples)) {
  sprintf( error_string, "%s: out of range line or sample argument.", 
           proc_name);
  oa_errno = 520;
  OaReportError( error_string);
  return(NULL);
}
if (encoding_type == OA_UNKNOWN_ENCODING_TYPE) {
  oa_errno = 520;
  return(NULL);
}
if (bands < 2) {
  sprintf( error_string, "%s: image has only one band!", proc_name);
  oa_errno = 520;
  OaReportError( error_string);
  return(NULL);
}
if (encoding_type != OA_UNCOMPRESSED) {
  sprintf( error_string, "%s: compressed multi-band images not supported!", 
           proc_name);
  oa_errno = 520;
  OaReportError( error_string);
  return(NULL);
}
if ((line_prefix_bytes != 0) || (line_suffix_bytes != 0)) {
  sprintf( error_string, 
           "%s: LINE_PREFIX_BYTES and LINE_SUFFIX_BYTES are not supported ",
               proc_name);
  strcat( error_string, "for multiband images.");
  oa_errno = 520;
  OaReportError( error_string);
  return(NULL);
}
if (OaCheckODLTree( input_node) != 0) {
  OdlFreeTree( input_node);
  return(NULL);  /* Error message already issued and oa_errno set.  */
}

if (OaGetFileKeywords( input_node, &label_filename, &data_filename,
                       &record_type, &record_bytes, &file_records,
                       &file_offset, &src_interchange_format) != 0) {
  return(NULL);  /* Error message already issued.  */
}

#ifdef OA_DEBUG
OaReportFileAttributes( label_filename, data_filename, record_type,
                        record_bytes, file_offset, src_interchange_format);
#endif

if (record_type == OA_VARIABLE_LENGTH) {
  sprintf( error_string, "%s does not support variable-length record files!",
           proc_name);
  oa_errno = 520;
  OaReportError( error_string);
  return(NULL);
}

/* Create the ODL tree to represent the spectrum.  Initialize keywords to
   reflect the source data, which will be from the image.  This ODL tree
   will be used as input to OalCreateSDT.  */

spectrum_node = OdlNewObjDesc( "SPECTRUM", NULL, NULL, NULL, NULL, NULL, 
                               (short) 0, (long) 0);
OaStrtoKwdValue( "INTERCHANGE_FORMAT", spectrum_node, "BINARY");
OaLongtoKwdValue( "ROWS", spectrum_node, bands);
OaLongtoKwdValue( "ROW_BYTES", spectrum_node, sample_bits/8);
OaLongtoKwdValue( "COLUMNS", spectrum_node, (long) 1);

column_node = OdlNewObjDesc( "COLUMN", NULL, NULL, NULL, NULL, NULL, 
                             (short) 0, (long) 0);
OdlPasteObjDesc( column_node, spectrum_node);
OaStrtoKwdValue( "NAME", column_node, "SAMPLE");
OaKwdValuetoStr( "SAMPLE_TYPE", input_node, &str);
OaStrtoKwdValue( "DATA_TYPE", column_node, str);
OaLongtoKwdValue( "START_BYTE", column_node, 1);
OaLongtoKwdValue( "BYTES", column_node, sample_bits/8);

/* Modify the tree so that unwanted data will be tossed out, and add the offset
   of the first data described by the tree to file_offset.  */

switch (band_storage_type) {

  case OA_BAND_SEQUENTIAL:

    /* Each band of the multi-spectral image is represented by a row in the
       SDT.  All the samples before the desired sample are thrown out by
       representing them as ROW_PREFIX_BYTES.  Similarily, all the samples
       after the desired sample are thrown out by representing them as
       ROW_SUFFIX_BYTES.  */

    line_prefix_bytes = ((line - 1) * line_samples + (sample - 1)) * 
                        sample_bits/8;
    OaLongtoKwdValue( "ROW_PREFIX_BYTES", spectrum_node, line_prefix_bytes);
    OaLongtoKwdValue( "ROW_SUFFIX_BYTES", spectrum_node, 
                      ((lines - line) * line_samples +
                       (line_samples - sample)) * sample_bits/8);
  break;

  case OA_LINE_INTERLEAVED:

    /* The SDT describes all the bands of a single line in the multiband
       image.  Each row in the SDT represents one band's line, with prefix
       and suffix bytes representing the samples before and after the desired
       sample, which are not wanted.  The offset of the line in the multiband
       image is added to file_offset, so the first OalReadStream will start
       reading there.  */

    file_offset += (line - 1) * line_samples * bands * sample_bits/8;
    line_prefix_bytes = (sample - 1) * sample_bits/8;
    OaLongtoKwdValue( "ROW_PREFIX_BYTES", spectrum_node, line_prefix_bytes);
    OaLongtoKwdValue( "ROW_SUFFIX_BYTES", spectrum_node, 
                      (line_samples - sample) * sample_bits/8);
  break;

  case OA_SAMPLE_INTERLEAVED:

    /* The desired spectrum data is in one contiguous chunk within the
       multiband image, so calculate its offset and add it to file_offset, 
       so the first OalReadStream will start reading there.  */

    file_offset += ((line - 1) * line_samples + (sample - 1)) * bands *
                   sample_bits/8;
  break;
}

/* Call OalCreateSDT to create the SDT (Stream Decomposition Tree).  */

if ((sdt = OalCreateSDT( spectrum_node, src_interchange_format)) == NULL) {
  sprintf( error_string, "%s: CreateSDT returned error.", 
           proc_name);
  OaReportError( error_string);
  OalFreeSDT( spectrum_node);
  return(NULL);
}

/* Note: sdt and spectrum_node now point to the same node. */

/* Undo the keyword changes made above which tricked OalCreateSDT into
   creating the right kind of SDT.  */

OaDeleteKwd( "ROW_PREFIX_BYTES", spectrum_node);
OaDeleteKwd( "ROW_SUFFIX_BYTES", spectrum_node);

/* Allocate memory for the spectrum.  */

sdt_node_ptr = (SDT_node *) sdt->appl1;
object_size = sdt_node_ptr->dst.size * sdt_node_ptr->total_repetitions;
if ((data_ptr = (PTR) OaMalloc( (long) object_size)) == NULL) {
  sprintf( error_string, "%s: OaMalloc failed to allocate %ld bytes for ",
           proc_name, object_size);
  strcat( error_string, "image! Out of memory!");
  oa_errno = 720;
  OaReportError( error_string);
  exit(1);
  /*NOTREACHED*/
}

c = 0;
#ifdef IBM_PC

/* If we're on an IBM-PC, use a for loop to do the set, since memset
   may not work for setting greater than 64K and/or over segment
   boundaries;  otherwise use memset since it's usually optimized.  */

for (i=0; i<object_size; i++)
  data_ptr[i] = c;
#else
memset( data_ptr, c, (size_t) object_size);
#endif

/* Create the output oa_object and initialize it.  */

oa_object = OaNewOaObject();
oa_object->odltree = sdt;
oa_object->data_ptr = data_ptr;
oa_object->size = object_size;
oa_object->is_in_memory = TRUE;
oa_object->profile = Oa_profile;
oa_object->stream_id = NULL;

if ((stream_id = OalOpenStream( data_filename, record_type, record_bytes, 
                                file_records, "r")) == NULL) {
  sprintf( error_string, "%s: OalOpenStream returned error.",
           proc_name);
  OaReportError( error_string);
  OalFreeSDT( spectrum_node);
  return(NULL);
}

/* Read the first slice of data; tell OalReadStream to read as many bytes as it
   wants, store them in its own buffer, start reading the file at the given
   byte offset, and output how many bytes it read in bytes_read.  */

if (OalReadStream( stream_id, (long) 0, &buf, file_offset, &bytes_read) != 0) {
  sprintf( error_string, "%s: OalReadStream returned error, aborting.",
           proc_name);
  OaReportError( error_string);
  OalFreeSDT( spectrum_node);
  OalCloseStream( stream_id);
  return(NULL);
}

/* Compress the SDT, and initialize current_node to point to the first
   end-node (data node) in the compressed SDT; initialize the node's dst.ptr
   with the pointer returned by OaMalloc.  */

compressed_SDT = OalCompressSDT( sdt);
current_node = OalInitializeSDT( compressed_SDT, data_ptr);

/* The main loop processes a buffer of data, then reads in the next buffer; 
   the SDT maps desired buffer bytes to memory and throws aways the rest.  */

for (;;) {

  if (OalProcessSDT( buf, bytes_read, &current_node) ==
      OA_REACHED_END_OF_SDT) break;

  /*  Read next slice of data, telling OalReadStream to do the same as the
      first time, except start at the current file position.  */

  buf = NULL;
  if (OalReadStream( stream_id, (long) 0, &buf, (long) -1, &bytes_read) != 0) {
    sprintf( error_string, "%s: OalReadStream returned error, aborting.",
             proc_name);
    OaReportError( error_string);
    break;
  }
}
OalCloseStream( stream_id);

/* Free the compressed SDT. */

OalFreeSDT( compressed_SDT);

/* Call OalSDTtoODLTree to modify the new ODL tree to reflect the in-memory 
   data.  This changes the DATA_TYPE keywords to reflect the in-memory data
   values which may have been converted to native types.  It also strips off 
   ODL tree nodes which have SDT_node->dst.size=0.  */

OalSDTtoODLTree( sdt, dst_interchange_format);

return( oa_object);
}



/*****************************************************************************

  Routine:  OaReadSpectrumFromQube

  Description:  OaReadSpectrumFromQube opens a data file and reads in the
                SPECTRUM for the specified line/sample from the core of the
                QUBE into memory.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  27 Nov   1995
  Last Modified:  09 Mar   1998

  History:

    Creation - This routine was part of the Version 1.0 Release of the OA
               library.
    03/09/98 - Added BIL and suffix plane support.  SM
	01/12/03 - There are now THEMIS qubes which may or may not have 
			   SUFFIX_ITEMS keywords.  That has been allowed for below

  Input:  
         input_node - A pointer to an ODL tree node of class QUBE.
                      The tree above the input_node should have keywords
                      to specify the data file.

         line       - The line location of the spectrum.       

         sample     - The sample location of the spectrum.

         The global variable Oa_profile is set to valid values.

  Output:  If successful, the routine returns a pointer to an OA_Object 
           structure, which contains an ODL tree of type SPECTRUM, and a
           pointer to the spectrum data.  If unsuccessful it returns NULL.
                It throws out all suffixes, if any are present.

  Notes:  

  1) Suffixes: in standard ISIS Qubes, each item in a suffix is allocated 4
     bytes, even though the data type stored there may be less than 4 bytes;
     in fact, some Qubes have suffixes allocated, but no actual suffix data
     in them.  This is so the user can add suffix data without rewriting the
     file.

*****************************************************************************/

#ifdef _NO_PROTO

OA_OBJECT OaReadSpectrumFromQube( input_node, line, sample)
ODLTREE input_node;
int     line;
int     sample;

#else

OA_OBJECT OaReadSpectrumFromQube( ODLTREE input_node, int line, int sample)

#endif
{

static char *proc_name = "OaReadSpectrumFromQube";
long file_offset, record_bytes, file_records;
int record_type, src_interchange_format, dst_interchange_format;
int band_storage_type;
char *label_filename, *data_filename;
OA_OBJECT oa_object;
ODLTREE current_node, sdt, spectrum_node, column_node;
ODLTREE compressed_SDT;
SDTNODE sdt_node_ptr;
long object_size, line_samples, lines, bands, core_item_bytes;
#ifdef IBM_PC
long i;
#endif

/* 11-15-05 MDC - Change suffix_items to a long instead of a long pointer */


long *suffix_items, *core_items, line_suffix_bytes, bytes_read, skip_bytes;

/*long suffix_items[2]={0}, *core_items, line_suffix_bytes, bytes_read, skip_bytes;*/

char *buf = NULL, c, **axis_names, *core_item_type;
PTR data_ptr;
struct OaStreamStruct *stream_id;
unsigned long total_bytes_read=0, total_bytes_to_read;
KEYWORD *i_id_kwdptr;
dst_interchange_format = OA_BINARY_INTERCHANGE_FORMAT;

if (OaGetQubeKeywords( input_node, &core_items, &axis_names, &suffix_items,
                       &core_item_bytes, &core_item_type) != 0)
  return( NULL);

if (OaGetFileKeywords( input_node, &label_filename, &data_filename,
                       &record_type, &record_bytes, &file_records,
                       &file_offset, &src_interchange_format) != 0) {
  return(NULL);  /* Error message already issued.  */
}

#ifdef OA_DEBUG
OaReportFileAttributes( label_filename, data_filename, record_type,
                        record_bytes, file_offset, src_interchange_format);
#endif

/* The storage orders this routine can currently handle are band-sequential,
   indicated by AXIS_NAME = (SAMPLE, LINE, BAND), and band-interleaved-by-
   line, indicated by AXIS_NAME = (SAMPLE, BAND, LINE).  */

if ((strcmp( axis_names[0], "SAMPLE") == 0) &&
    (strcmp( axis_names[1], "LINE") == 0) &&
    (strcmp( axis_names[2], "BAND") == 0)) {
  band_storage_type = OA_BAND_SEQUENTIAL;
  bands = core_items[2];
  lines = core_items[1];
  line_samples = core_items[0];
}
if ((strcmp( axis_names[0], "SAMPLE") == 0) &&
    (strcmp( axis_names[1], "BAND") == 0) &&
    (strcmp( axis_names[2], "LINE") == 0)) {
  band_storage_type = OA_LINE_INTERLEAVED;
  bands = core_items[1];
  lines = core_items[2];
  line_samples = core_items[0];
}
if ((strcmp( axis_names[0], "BAND") == 0) &&  /*DWS  added this if 08-15-04*/
    (strcmp( axis_names[1], "SAMPLE") == 0) &&
    (strcmp( axis_names[2], "LINE") == 0)) {
  band_storage_type = OA_SAMPLE_INTERLEAVED;
  bands = core_items[0];
  lines = core_items[2];
  line_samples = core_items[1];
}
	/*THEMIS QUESTION DWS 09-10-02*/
	/*If there is a SUFFIX_ITEMS keyword then we will procede as in a normal*/
	/*qube, otherwise this is probably a THEMIS cube.  A THEMIS cube may or */
	/*may not have this keyword.  If the keyword is not present then        */
	/*initialize the three variables                                        */
	if((OdlFindKwd(input_node, "SUFFIX_ITEMS", NULL, 1, ODL_TO_END) != NULL))
	{
		
		line_suffix_bytes = suffix_items[0] * 4;
	}
	else 
	{
		line_suffix_bytes = 0;
		suffix_items[0] = 0;
		suffix_items[1] = 0;
	}

if ((band_storage_type != OA_BAND_SEQUENTIAL) &&
    (band_storage_type != OA_LINE_INTERLEAVED) &&
    (band_storage_type != OA_SAMPLE_INTERLEAVED)) { /* added this choice DWS  08-15-04*/
  sprintf( error_string, "%s: Cannot handle this storage order: %s %s %s",
           proc_name, axis_names[0], axis_names[1], axis_names[2]);
  oa_errno = 520;
  OaReportError( error_string);
  return( NULL);
}

/* Check that inputs 'line' and 'sample' are within the range of the QUBE.  */

if ((line < 1) || (line > lines) || (sample < 1) || (sample > line_samples)) {
  sprintf( error_string, "%s: input line or sample is out-of-range",
           proc_name);
  oa_errno = 502;
  OaReportError( error_string);
  return( NULL);
}


/* The technique used to read in a spectrum from the qube is trick OalCreateSDT
   into thinking it's creating an SDT for a table.  To get to the desired
   pixel in the first band, we set file_offset to be the start of the qube
   plus all the interveaning core, suffixes, corners etc.  Then we use
   ROW_SUFFIX_BYTES to represent all the data between a pixel in band x and
   same pixel in band x+1, so that all this data will be thrown out;  this
   includes suffixes in various dimensions, corners, and all the core data
   belonging to other pixel locations.  After we read in the pixel from the
   last band, the SDT will still specify reading ROW_SUFFIX_BYTES before
   it's been fully traversed, and this may cause an attempt to read past the
   end of the file; OalReadStream will give an error which we'll catch by
   keeping track of whether we've read the entire spectrum yet
   (total_bytes_read >= total_bytes_to_read).  */
/*********************************************************************************************/

if (band_storage_type == OA_BAND_SEQUENTIAL) {

  /* Position initial file offset past all the lines preceeding the line of
     the desired pixel, and past all pixels before the desired pixel in
     the pixel's line.  */

  file_offset += (line_samples * core_item_bytes + line_suffix_bytes) *
                 (line - 1) + (sample-1) * core_item_bytes;

  /* Skip pixels after the desired pixel in this line, and skip line suffix
     bytes.  */

  skip_bytes = (line_samples - sample) * core_item_bytes + line_suffix_bytes;

  /* Skip all lines and their line suffix bytes which occur after the desired
     pixel's line, up to the end of this band.  */

  skip_bytes += (line_samples * core_item_bytes + line_suffix_bytes) *
                (lines - line);

  /* Skip any bottom plane bytes and their suffix bytes which occur after
     the end of the band and before the start of the next band.  */

  skip_bytes += (suffix_items[1] * 4) * line_samples;    /* bottom plane */
  skip_bytes += (suffix_items[0] * 4) * suffix_items[1]; /* corner */

  /* Skip all lines and their suffix bytes in the next band which occur
     before the desired pixel's line.  */

  skip_bytes += (line_samples * core_item_bytes + line_suffix_bytes) *
                (line-1);

  /* Skip all pixels before the desired pixel in this line.  */

  skip_bytes += (sample-1) * core_item_bytes;

  total_bytes_to_read = (core_item_bytes + skip_bytes) * lines * bands - 
                        skip_bytes;
}
/*********************************************************************************************/


if (band_storage_type == OA_LINE_INTERLEAVED) {

  /* Position past all image lines before the line of the desired pixel.  */

  file_offset += (line_samples * core_item_bytes + line_suffix_bytes) *
                 bands * (line-1);

  /* Position past any back planes and their suffix bytes (corner) which
     occur before the first band of the desired pixel's line.  */

  file_offset += (suffix_items[1] * 4) * line_samples * (line-1); /*backplane*/
  file_offset += (suffix_items[0] * 4) * suffix_items[1] * (line-1); /*corner*/

  /* Position past pixels preceeding the desired pixel in the first band's
     line.  */

  file_offset += core_item_bytes * (sample-1);

  /* Skip pixels after the desired pixel in this line, and skip line suffix
     bytes.  */

  skip_bytes = (line_samples - sample) * core_item_bytes + line_suffix_bytes;

  /* Skip all pixels before the desired pixel in the next band occurrence
     of this line.  */

  skip_bytes += (sample-1) * core_item_bytes;

  total_bytes_to_read = (skip_bytes + core_item_bytes) * bands - 
                         skip_bytes;
}

/************************************************new for interleaved by pixel   DWS  08-16-04*/
/*line, sample*/
if (band_storage_type == OA_SAMPLE_INTERLEAVED) {
  line_samples = core_items[1];
  lines = core_items[2];
  line_suffix_bytes = suffix_items[0]* 4;
  skip_bytes = 0;
  /* Calculate the offset into the QUBE, in bytes, of the first pixel of the
     first line of the desired image, and add it to the offset of start of the
     QUBE.  */
  file_offset += (line - 1) * record_bytes;
  total_bytes_to_read = record_bytes;
  line_suffix_bytes = skip_bytes;
}

/***************************************************************************/

spectrum_node = OdlNewObjDesc( "SPECTRUM", NULL, NULL, NULL, NULL, NULL, 
                               (short) 0, (long) 0);
OaStrtoKwdValue( "INTERCHANGE_FORMAT", spectrum_node, "BINARY");
OaLongtoKwdValue( "ROWS", spectrum_node, bands);
OaLongtoKwdValue( "ROW_BYTES", spectrum_node, core_item_bytes);
OaLongtoKwdValue( "COLUMNS", spectrum_node, (long) 1);
OaLongtoKwdValue( "LINE_SUFFIX_BYTES", spectrum_node, skip_bytes);

column_node = OdlNewObjDesc( "COLUMN", NULL, NULL, NULL, NULL, NULL, 
                             (short) 0, (long) 0);
OdlPasteObjDesc( column_node, spectrum_node);
OaStrtoKwdValue( "NAME", column_node, "SAMPLE");
OaStrtoKwdValue( "DATA_TYPE", column_node, core_item_type);
OaLongtoKwdValue( "START_BYTE", column_node, (long) 1);
OaLongtoKwdValue( "BYTES", column_node, core_item_bytes);

/* Call OalCreateSDT to create the SDT (Stream Decomposition Tree).  */

if ((sdt = OalCreateSDT( spectrum_node, src_interchange_format)) == NULL) {
  sprintf( error_string, "%s: CreateSDT returned error.", 
           proc_name);
  OaReportError( error_string);
  OalFreeSDT( spectrum_node);
  return(NULL);
}

/* Note: sdt and spectrum_node now point to the same node. */

/* Allocate memory for the spectrum.  */

sdt_node_ptr = (SDT_node *) sdt->appl1;
object_size = sdt_node_ptr->dst.size * sdt_node_ptr->total_repetitions;
if ((data_ptr = (PTR) OaMalloc( (long) object_size)) == NULL) {
  sprintf( error_string, "%s: OaMalloc failed to allocate %ld bytes for ",
           proc_name, object_size);
  strcat( error_string, "image! Out of memory!");
  oa_errno = 720;
  OaReportError( error_string);
  exit(1);
  /*NOTREACHED*/
}
c = 0;

#ifdef IBM_PC

/* If we're on an IBM-PC, use a for loop to do the set, since memset
   may not work for setting greater than 64K and/or over segment
   boundaries;  otherwise use memset since it's usually optimized.  */

for (i=0; i<object_size; i++)
  data_ptr[i] = c;
#else
memset( data_ptr, c, (size_t) object_size);
#endif

/* Create the output oa_object and initialize it.  */

oa_object = OaNewOaObject();
oa_object->odltree = sdt;
oa_object->data_ptr = data_ptr;
oa_object->size = object_size;
oa_object->is_in_memory = TRUE;
oa_object->profile = Oa_profile;
oa_object->stream_id = NULL;

if ((stream_id = OalOpenStream( data_filename, record_type, record_bytes, 
                                file_records, "r")) == NULL) {
  sprintf( error_string, "%s: OalOpenStream returned error.",
           proc_name);
  OaReportError( error_string);
  OalFreeSDT( spectrum_node);
  return(NULL);
}

/* Read the first slice of data; tell OalReadStream to read as many bytes as it
   wants, store them in its own buffer, start reading the file at the given
   byte offset, and output how many bytes it read in bytes_read.  */


if(band_storage_type != OA_SAMPLE_INTERLEAVED)                  /*added this test for sample interleaved  DWS  11/01/04*/
{
	if (OalReadStream( stream_id, (long) 0, &buf, file_offset, &bytes_read) != 0)
	{
		sprintf( error_string, "%s: OalReadStream returned error, aborting.",
																	   proc_name);
		OaReportError( error_string);
		OalFreeSDT( spectrum_node);
		OalCloseStream( stream_id);
		return(NULL);
	}
}
else                                                           /*added this else for sample interleaved  DWS  11/01/04*/
{
	if (OalReadStream( stream_id, (long) record_bytes, &buf, file_offset, &bytes_read) != 0) 
	{
		sprintf( error_string, "%s: OalReadStream returned error, aborting.",
																       proc_name);
		OaReportError( error_string);
		OalFreeSDT( spectrum_node);
		OalCloseStream( stream_id);
		return(NULL);
	}
}total_bytes_read += bytes_read;

/* Compress the SDT, and initialize current_node to point to the first
   end-node (data node) in the compressed SDT; initialize the node's dst.ptr
   with the pointer returned by OaMalloc.  */

compressed_SDT = OalCompressSDT( sdt);
current_node = OalInitializeSDT( compressed_SDT, data_ptr);

/* The main loop processes a buffer of data, then reads in the next buffer; 
   the SDT maps desired buffer bytes to memory and throws aways the rest.  */

for (;;) {

	if(band_storage_type != OA_SAMPLE_INTERLEAVED)                  /*added this test for sample interleaved  DWS  11/01/04*/
	{
		if (OalProcessSDT( (PTR) buf, line_samples * core_item_bytes, &current_node) ==
			OA_REACHED_END_OF_SDT) break;
	}
	else                                                            /*added this else for sample interleaved DWS 11/01/04*/
	{
		if (OalProcessSDT( (PTR) buf + (bands -1 ) * core_item_bytes, core_item_bytes, &current_node) ==
			OA_REACHED_END_OF_SDT) break;
	}

  /*  Read next slice of data, telling OalReadStream to do the same as the
      first time, except start at the current file position.  */

  buf = NULL;
  if(band_storage_type != OA_SAMPLE_INTERLEAVED)                  /*added this test for sample interleaved  DWS  11/01/04*/
  {
	if (OalReadStream( stream_id, (long) 0, &buf, (long) -1, &bytes_read) != 0) 
	{
       if (total_bytes_read < total_bytes_to_read) 
	   {
          sprintf( error_string, "%s: OalReadStream returned error, aborting.",
                                                                    proc_name);
         OaReportError( error_string);
       }
       break;
    }
  }
  else                                                            /*added this else for sample interleaved DWS 11/01/04*/
  {
	 if (OalReadStream( stream_id, (long) record_bytes, &buf, (long) -1, &bytes_read) != 0) 
	 {
       if (total_bytes_read < total_bytes_to_read) 
	   {
          sprintf( error_string, "%s: OalReadStream returned error, aborting.",
                                                                    proc_name);
         OaReportError( error_string);
       }
       break;
    }
  }
  total_bytes_read += bytes_read;
}
OalCloseStream( stream_id);

/* Free the compressed SDT. */

OalFreeSDT( compressed_SDT);

/* Call OalSDTtoODLTree to modify the new ODL tree to reflect the in-memory 
   data.  This changes the DATA_TYPE keywords to reflect the in-memory data
   values which may have been converted to native types.  It also strips off 
   ODL tree nodes which have SDT_node->dst.size=0.  */

OalSDTtoODLTree( sdt, dst_interchange_format);

return( oa_object);
}



/*****************************************************************************

  Routine:  OaParseLabelFile

  Description:  This routine returns a pointer to an ODL tree that represents
                a parsed ODL label, given the name of a file that contains
                an ODL label in ASCII format.  This routine will optionally
                expand ^STRUCTURE pointers.  It is functionally identical to
                L3's OdlParseLabelFile, except that it also works with an 
                attached label in a variable-length record file, e.g. the
                Voyager images on CD-ROM.  It uses the OA Stream Layer to
                read records from variable-length record files.
                Starting with OAL Version 1.3, when given a file containing
                one of the external file formats supported by OAL, this
                routine will call external file format specific code to
                create a label for the file on-the-fly.   This is currently
                done only for GIF files.

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  26 Oct  1994
  Last Modified:  16 Mar  1998

  History:

    Creation - This routine was part of the Beta Release of the OA library.
    12/11/95 - For variable-length record files, perform ^STRUCTURE expansion
               AFTER the file_name fields have been corrected.  This is
               necessary for the code which looks for ^STRUCTURE files.  SM
    03/16/98 - Added call OalCreateODLTreeFromGif when OalOpenStream detects
               a GIF file.  SM

  Input:  
          filespec     - A character string that represents a file 
                         specification, assumed to be valid for the host 
                         system.  If not, an error message is issued.

          errfilespec  - Identifies the file into which parser errors will
                         be written.  If the file previously exists, then the
                         error messages are appended to the existing file.
                         If NULL is specified, then the error messages are
                         written to stdout.

          expand       - expand is a mask (typedef unsigned short MASK) which
                         specifies whether or not the specified ODL files are
                         to be expanded.  See lablib3.h.

                         ODL_EXPAND_STRUCTURES - expand ^STRUCTURE and
                         ^*_STRUCTURE keywords only.

                         ODL_EXPAND_CATALOG - expand ^CATALOG keywords only.

                         ODL_EXPAND_STRUCTURES | ODL_EXPAND_CATALOG - both
                         of the above.
    
          nomsgs       - A flag indicating whether or not parser error 
                         messages are to be written.  A value of TRUE or (1)
                         indicates that parser error messages will not be
                         written even if an error message file has been
                         specified.
         
  Output:  If successful, an ODL tree is returned.

  Notes:  

*****************************************************************************/

#ifdef _NO_PROTO

ODLTREE OaParseLabelFile( filespec, errfilespec, expand, nomsgs)
char *filespec;
char *errfilespec;
MASK expand;
unsigned short nomsgs;

#else

ODLTREE OaParseLabelFile( char *filespec, char *errfilespec, MASK expand, 
                          unsigned short nomsgs)

#endif
{

static char *proc_name = "OaParseLabelFile";
struct OaStreamStruct *stream_id;
ODLTREE odltree, current_node;
KEYWORD *kwdptr;
char *buf, *label_string;
int reached_end_of_label = FALSE;
long bytes_read, label_string_bytes;

/* Open the label file, specifying OA_UNKNOWN_RECORD_TYPE for the record_type.
   OalOpenStream will decide if it's a variable-length record file, a stream
   text file, or one of the external file formats supported by OAL, such as
   GIF.   */

if ((stream_id = OalOpenStream( filespec, OA_UNKNOWN_RECORD_TYPE,
                                (long) 0, (long) 0, "r")) == NULL) {
  sprintf( error_string, "%s: OalOpenStream returned error.",
           proc_name);
  OaReportError( error_string);
  return( NULL);
}

if (stream_id->record_type == OA_GIF) {
  odltree = (ODLTREE) OalCreateODLTreeFromGif( stream_id->fp, 
                                               stream_id->filename);
  OalCloseStream( stream_id);
  return( odltree);
}

if (stream_id->record_type != OA_VARIABLE_LENGTH) {

  /* Since the file doesn't have variable-length records (i.e. with explicit,
     2-byte LSB integer record counts), OdlParseLabelFile can read it.  */

  OalCloseStream( stream_id);
  return( OdlParseLabelFile( filespec, errfilespec, expand, nomsgs));

} else {

  /* File has non-transparent variable-length records, so use the OAL Stream
     Layer to read records from the file, and build up a long string
     containing the entire label for L3 to parse.  */

  if ((label_string = (char *) OaMalloc( (size_t) 1)) == NULL) {
    sprintf( error_string, "%s: OaMalloc failed! Out of memory!", proc_name);
    oa_errno = 720;
    OaReportError( error_string);
    exit(1);
    /*NOTREACHED*/
  }
  *label_string = ' ';
  label_string_bytes = 1;
  while (reached_end_of_label == FALSE) {
    buf = NULL;
    if (OalReadStream( stream_id, (long) 0, &buf, (long) -1, &bytes_read)
                       != 0) {
      sprintf( error_string, "%s: OalReadStream returned error, aborting.",
               proc_name);
      OaReportError( error_string);
      OalCloseStream( stream_id);
      return(NULL);
    }
    label_string = (char *) OaRealloc( label_string, (long) label_string_bytes,
                                       (long) label_string_bytes + bytes_read
                                        + 1);
    if (label_string == NULL) {
      sprintf( error_string, "%s: OaRealloc failed! Out of memory!",
               proc_name);
      sprintf( error_string + strlen( error_string),
               "\nlabel_string_bytes = %ld, bytes_read = %ld.",
               label_string_bytes, bytes_read);
      oa_errno = 720;
      OaReportError( error_string);
      exit(1);
      /*NOTREACHED*/
    }
    memcpy( label_string + label_string_bytes, buf, bytes_read);
    label_string_bytes += bytes_read;
    label_string[ label_string_bytes] = '\n';
    label_string_bytes++;

    buf[ bytes_read] = '\0';

    /* Detect the end of the label by finding a 3-byte record containing
       "END".  */

    if ((bytes_read == 3) && (strcmp( buf, "END") == 0)) {
      reached_end_of_label = TRUE;
      label_string[ label_string_bytes-1] = '\0';
    }
  }   /* end while reached_end_of_label == FALSE  */

  /* Close the file.  */

  OalCloseStream( stream_id);

  /* Call L3 to parse the label string, specifying NO ^STRUCTURE or
     ^CATALOG expansion.  */

  odltree = OdlParseLabelString( label_string, errfilespec, 0, nomsgs);
  if (odltree == NULL) {
     sprintf( error_string, "%s: OdlParseLabelString returned NULL!", proc_name);
     OaReportError( error_string);
  }
  LemmeGo( label_string);

  /* OdlParseLabelString no longer writes the string to a temporary file
     and OdlParseLabelFile to read the file like it used to.  Rather it calls
     OdlParseFile which now parses the string directly, and leaves the
     file_name field in all the ODL tree nodes and keywords set to NULL;
     these need to be changed to the name of the original label file.  */
  
  if (odltree != NULL) {
    current_node = odltree;

    /* Do a pre-order traverse of the entire tree, and at each node, replace
       the file_name field by the original filespec.  Do the same for the
       file_name field in all the node's keywords.  */

    while (current_node != NULL) {
      if (current_node->file_name == NULL) {
        CopyString( current_node->file_name, filespec);
      }

      kwdptr = OdlGetFirstKwd( current_node);
      while (kwdptr != NULL) {
        if (kwdptr->file_name == NULL) {
          CopyString( kwdptr->file_name, filespec);
	}
        kwdptr = OdlGetNextKwd( kwdptr);
      }

      /* Position current_node to the next node;  if current_node has children,
         then the leftmost child is the new current_node;  otherwise, if
         current_node has a right sibling, the the right sibling is the new
         current_node;  otherwise search upwards in the tree until get to a 
         node which does have a right sibling.  */

      if (LeftmostChild( current_node) != NULL)
        current_node = LeftmostChild( current_node);
      else {
        while (current_node != NULL) {
          if (RightSibling( current_node) != NULL) {
            current_node = RightSibling( current_node);
            break;
          }
          current_node = Parent( current_node);
        }
      }
    }  /* end while current_node != NULL  */
  } else {    /* end if odltree != NULL          */
    oa_errno = 1000;
    return( NULL);
  }

  /* Now expand the label.  */

  odltree = OdlExpandLabelFile( odltree, errfilespec, expand, nomsgs);
  return( odltree);

}     /* end else a variable-length record file   */
}



/*****************************************************************************

  Routine:  OaReadObject

  Description:  OaReadObject opens a data file and reads an object into 
                memory.  It determines the class of the object and calls
                the appropriate class-specific OA read function.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:   1 Sept  1994

  History:

    Creation - This routine was part of the Alpha Release of the OA library.

  Input:  
         object_node - A pointer to an ODL tree node which is a top-level
                       object, i.e. TABLE, IMAGE etc.
                       A node above the object_node should have keywords
                       to specify the data file.
       
         The global variable Oa_profile is set to valid values.

  Output:  If successful, the routine returns a pointer to an OA_Object 
           structure, which contains an ODL tree and a pointer to the object
           data.  If unsuccessful it returns NULL.

  Notes:  This routine is just a wrapper around OaReadArray, OaReadHistogram,
           OaReadImage, OaReadTable.

*****************************************************************************/

#ifdef _NO_PROTO

OA_OBJECT OaReadObject( object_node)
ODLTREE object_node;

#else

OA_OBJECT OaReadObject( ODLTREE object_node)

#endif
{

static char *proc_name = "OaReadObject";
int object_class;

object_class = OaGetObjectClass( object_node);
switch( object_class) { 
  case OA_ARRAY:
    return( OaReadArray( object_node));
    /*NOTREACHED*/
  break;
  case OA_HISTOGRAM:
    return( OaReadHistogram( object_node));
    /*NOTREACHED*/
  break;
  case OA_IMAGE:
    return( OaReadImage( object_node, 1));
    /*NOTREACHED*/
  break;
  case OA_QUBE:
    return( OaReadImageFromQube( object_node, 1));
    /*NOTREACHED*/
  break;
  case OA_COLLECTION:
  case OA_TABLE:
  case OA_SPECTRUM:
  case OA_SERIES:
  case OA_PALETTE:
  case OA_GAZETTEER:
    return( OaReadTable( object_node));
    /*NOTREACHED*/
  break;

  default:
    sprintf( error_string, "%s: input ODLTREE node is not an object class ",
             proc_name);
    strcat( error_string, "which OA can read.");
    oa_errno = 520;
    OaReportError( error_string);
  break;
}
return(NULL);
}



/*****************************************************************************

  Routine:  OaReadSubTable

  Description:  OaReadSubTable reads one or more COLUMNS and/or CONTAINERS
                of a TABLE from a file into memory.  The subobjects can be a
                mixture of COLUMNS and CONTAINERS;  all must be directly under
                a TABLE-like node, not nested under a CONTAINER, and the table
                must be row-major.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   7 Nov  1994
  Last Modified:   7 Nov  1994

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:  
         oa_object         - A pointer to an Oa_Object structure with an 
                             in-memory TABLE-like object.

         start_row         - The first row to read, 1 <= start_row <= ROWS

         stop_row          - The last row to read,  1 <= start_row <= ROWS

         subobject_nodes   - An array of ODLTREE node pointers, pointing to 
                             the nodes in the tree the user wants to read.

         n_subobject_nodes - The number of ODLTREE node pointers in the
                             subobject_nodes argument.
         
  Output:  If successful, the function returns a pointer to an Oa_Object
           structure, otherwise NULL.  The Oa_Object's ODL tree has a TABLE
           node with n_subobject_nodes children;  the ROWS keyword in the
           TABLE node is updated.  The data for the subtable is in
           oa_object->data_ptr.  Data type conversions and alignment of the
           data has been done according to oa_object->profile.

  Notes:  

*****************************************************************************/

#ifdef _NO_PROTO

OA_OBJECT OaReadSubTable( table_node, start_row, stop_row, 
                          subobject_nodes, n_subobject_nodes)
ODLTREE table_node;
long start_row, stop_row;
ODLTREE subobject_nodes[];
int n_subobject_nodes;

#else

OA_OBJECT OaReadSubTable( ODLTREE table_node, long start_row, long stop_row, 
                          ODLTREE subobject_nodes[], int n_subobject_nodes)

#endif
{

static char *proc_name = "OaReadSubTable";
OA_OBJECT output_oa_object;
ODLTREE current_node, right_sibling, sdt, sdt_node, compressed_SDT;
long new_object_size, row_bytes, rows, new_rows;
long i, file_offset, record_bytes, file_records;
long prefix_bytes, suffix_bytes, bytes_read;
int record_type, src_interchange_format, dst_interchange_format;
int table_storage_type;
char *label_filename, *data_filename, *buf = NULL, c;
PTR data_ptr;
struct OaStreamStruct *stream_id;
SDT_node *sdt_node_ptr;

/* Check for valid inputs. */

if (table_node == NULL) {
  sprintf( error_string, "%s: input table_node is NULL.", proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return( NULL);
}

if (n_subobject_nodes < 1) {
  sprintf( error_string, "%s: n_subobject_nodes must be >= 1.", proc_name);
  oa_errno = 502;
  OaReportError( error_string);
  return( NULL);
}

for (i=0; i<n_subobject_nodes; i++) {
  if (Parent( subobject_nodes[i]) != table_node) {
    sprintf( error_string, 
             "%s: subobject_nodes[%d] is not a child of the root node.",

			 proc_name, (int) i);
    oa_errno = 530;
    OaReportError( error_string);
    return( NULL);
  }
}

if (OaGetTableKeywords( table_node, &rows, &row_bytes, &prefix_bytes,
                        &suffix_bytes, &src_interchange_format,
                        &table_storage_type) != 0)
  return( NULL);  /* Error message already written and oa_errno set.  */

if (table_storage_type != OA_ROW_MAJOR) {
  sprintf( error_string, "%s: %s must be ROW_MAJOR.",
           proc_name, OdlGetObjDescClassName( table_node));
  oa_errno = 520;
  OaReportError( error_string);
  return( NULL);
}

if ((start_row < 1) || (start_row > rows) ||
    (stop_row < 1)  || (stop_row > rows)  ||
    (stop_row < start_row)) {
  sprintf( error_string, "%s: invalid start_row or stop_row.", proc_name);
  oa_errno = 502;
  OaReportError( error_string);
  return( NULL);
}
new_rows = (stop_row - start_row + 1);

/* Get the file keywords. */

if (OaGetFileKeywords( table_node, &label_filename, &data_filename,
                       &record_type, &record_bytes, &file_records,
                       &file_offset, &src_interchange_format) != 0) {
  return(NULL);  /* Error message already issued.  */
}

if (record_type == OA_VARIABLE_LENGTH) {
  sprintf( error_string, 
           "%s: currently can't read from a variable-length record file",
           proc_name);
  oa_errno = 520;
  OaReportError( error_string);
  return(NULL);
}

if (src_interchange_format == OA_ASCII_INTERCHANGE_FORMAT)
  dst_interchange_format = Oa_profile.dst_format_for_ASCII_src;
else
  dst_interchange_format = Oa_profile.dst_format_for_binary_src;

#ifdef OA_DEBUG
OaReportFileAttributes( label_filename, data_filename, record_type,
                        record_bytes, file_offset, src_interchange_format);
#endif

if (OaCheckODLTree( table_node) != 0)
  return(NULL);  /* Error message already issued and oa_errno set.  */

/* Copy the table_node ODL tree, then loop through the sub-objects, stripping
   off those in the copy which don't match one of the nodes in the
   subobject_nodes array.  The loop goes through the children of the two
   trees in parallel.  The copied/stripped tree will be the input for
   OalCreateSDT.  */

sdt = OaCopyTree( table_node, OA_STRIP_SDT_NODES | OA_STRIP_COMMENTS);
sdt_node = LeftmostChild( sdt);
current_node = LeftmostChild( table_node);

while (current_node != NULL) {      /* Loop through all columns in the table */
  for (i=0; i<n_subobject_nodes; i++)  /* Loop through subobject_nodes       */
    if (current_node == subobject_nodes[i]) break;
  right_sibling = RightSibling( sdt_node);
  if (i == n_subobject_nodes)       /* Node doesn't occur in subobject_nodes */
    OdlFreeTree( OdlCutObjDesc( sdt_node));
  current_node = RightSibling( current_node);
  sdt_node = right_sibling;  /* Note: if we stripped off sdt_node, then      */
                             /* right_sibling was updated by OdlCutObjDesc.  */
}

OaLongtoKwdValue( "ROWS", sdt, new_rows);

if ((sdt = OalCreateSDT( sdt, src_interchange_format)) == NULL) {
  sprintf( error_string, "%s: CreateSDT returned error.", 
           proc_name);
  OaReportError( error_string);
  OalFreeSDT( sdt);
  return(NULL);
}

sdt_node_ptr = (SDT_node *) sdt->appl1;
new_object_size = sdt_node_ptr->dst.size * sdt_node_ptr->total_repetitions;
OaLongtoKwdValue( "ROW_BYTES", sdt, sdt_node_ptr->dst.size);
OaLongtoKwdValue( "COLUMNS", sdt, n_subobject_nodes);

if ((data_ptr = (PTR) OaMalloc( (long) new_object_size)) == NULL) {
  sprintf( error_string, "%s: OaMalloc failed! Out of memory!", proc_name);
  oa_errno = 720;
  OaReportError( error_string);
  exit(1);
  /*NOTREACHED*/
}
if (dst_interchange_format == OA_ASCII_INTERCHANGE_FORMAT) 
  c = ' ';
else
  c = 0;

#ifdef IBM_PC

/* If we're on an IBM-PC, use a for loop to do the set, since memset
   may not work for setting greater than 64K and/or over segment
   boundaries;  otherwise use memset since it's usually optimized.  */

for (i=0; i<new_object_size; i++)
  data_ptr[i] = c;
#else
memset( data_ptr, c, (size_t) new_object_size);
#endif

/* Allocate an OA_object structure and initialize it.  */

output_oa_object = OaNewOaObject();
output_oa_object->is_in_memory = TRUE;
output_oa_object->data_ptr     = data_ptr;
output_oa_object->size         = new_object_size;
output_oa_object->profile      = Oa_profile;

if ((stream_id = OalOpenStream( data_filename, record_type, record_bytes, 
                                file_records, "r")) == NULL) {
  sprintf( error_string, "%s: OalOpenStream returned error.",
           proc_name);
  OaReportError( error_string);
  return(NULL);
}

/* Read the first slice of data; tell OalReadStream to read as many bytes as it
   wants, store them in it's own buffer, start reading the file at the given
   byte offset, and output how many bytes it's read in bytes_read.  */

if (OalReadStream( stream_id, (long) 0, &buf, file_offset +  
                  (start_row-1)*sdt_node_ptr->src.size, &bytes_read) != 0) {
  sprintf( error_string, "%s: first call to OalReadStream returned error, ",
           proc_name);
  strcat( error_string, "aborting.");
  OaReportError( error_string);
  OalFreeSDT( sdt);
  OalCloseStream( stream_id);
  return(NULL);
}

/* Compress the SDT, and initialize current_node to point to the first
   end-node (data node) in the compressed SDT; initialize the node's dst.ptr
   with the pointer returned by OaMalloc.  */

compressed_SDT = OalCompressSDT( sdt);
current_node = OalInitializeSDT( compressed_SDT, data_ptr);

/* The main loop processes a buffer of data, then reads in the next buffer; 
   the SDT maps desired data to memory and throws aways the rest.  */

for (;;) {
  if (OalProcessSDT( (PTR) buf, bytes_read, &current_node) ==
      OA_REACHED_END_OF_SDT) break;

  /*  Read next slice of data, telling OalReadStream to do the same as the
      first time, except start at the current file position.  */

  buf = NULL;
  if (OalReadStream( stream_id, (long) 0, &buf, (long) -1, &bytes_read) != 0) {
    sprintf( error_string, "%s: OalReadStream returned error, aborting.",
             proc_name);
    OaReportError( error_string);
    break;
  }
}
OalCloseStream( stream_id);

/* Free the compressed SDT. */

OalFreeSDT( compressed_SDT);

/* Call OalSDTtoODLTree to modify the new ODL tree to reflect the in-memory 
   data.  This changes the DATA_TYPE keywords to reflect the in-memory data
   values which may have been converted to native types.  It also strips off 
   ODL tree nodes which have SDT_node->dst.size=0.  */

OalSDTtoODLTree( sdt, dst_interchange_format);

/* Remove keywords for ROW_PREFIX_BYTES and ROW_SUFFIX_BYTES,
   since these have been stripped off.  */

OaDeleteKwd( "ROW*PREFIX*BYTES", sdt);
OaDeleteKwd( "ROW*SUFFIX*BYTES", sdt);
OaDeleteKwd( "^*PREFIX*STRUCTURE", sdt);
OaDeleteKwd( "^*SUFFIX*STRUCTURE", sdt);

output_oa_object->odltree = sdt;
return( output_oa_object);
}



/*****************************************************************************

  Routine:  OaReadPartialImage

  Description:  OaReadPartialImage reads a rectangular subimage from an image
                in a file into memory.  It is part of the calling sequence:
                OaOpenImage, OaReadPartialImage (multiple calls), OaCloseImage.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  29 Sept  1994
  Last Modified:  29 Sept  1994

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:  
         image_handle_object - An oa_object returned by OaOpenImage.

         start_line          - The first line samples are retrieved from.

         stop_line           - The last line samples are retrieved from.

         start_sample        - The first sample in a line to retrieve.

         stop_sample         - The last sample in a line to retrieve.

         The global variable Oa_profile is set to valid values.

  Output:  If successful, the routine returns a pointer to an OA_Object 
           structure, which contains the decompressed partial image and an
           ODL tree describing it.

  Notes:  
  1) Current limitations: 
     a) The supported compression (encoding) types are Previous Pixel and
        Huffman First Difference.
     b) If the image is HFD encoded, it must be in a variable-length record
        file.  
     c) Uncompressed and Previous Pixel compressed images must be in
        fixed-length record files;  variable-length record files are only
        supported for HFD images.
     d) Previous Pixel encoded images may not have prefix or suffix bytes.

*****************************************************************************/


#ifdef _NO_PROTO

OA_OBJECT OaReadPartialImage( image_handle_object, start_line, stop_line, 
                              start_sample, stop_sample)
OA_OBJECT image_handle_object;
long start_line, stop_line, start_sample, stop_sample;

#else

OA_OBJECT OaReadPartialImage( OA_OBJECT image_handle_object, long start_line, 
                              long stop_line, long start_sample,
                              long stop_sample)

#endif
{
static char *proc_name = "OaReadPartialImage";
struct oa_image_handle *image_handle;
OA_OBJECT output_oa_object;
long bytes, line_samples, line, sample, buf_sample, buf_samples;
long samples_from_last_to_first;
int i;
PTR src_sample_ptr, dst_sample_ptr;

/* Check inputs for validity. */

if (image_handle_object == NULL) {
  sprintf( error_string, "%s: input image_handle_object is NULL.", proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return( NULL);
}
if (image_handle_object->appl1 == NULL) {
  sprintf( error_string, 
           "%s: input image_handle_object must have image_handle attached ",
           proc_name);
  strcat( error_string, "to appl1, as\nreturned by OaOpenImage.");
  oa_errno = 501;
  OaReportError( error_string);
  return( NULL);
}

image_handle = (struct oa_image_handle *) image_handle_object->appl1;

if ((start_line < 1) || (start_line > image_handle->source_lines) ||
    (stop_line  < 1) || (stop_line  > image_handle->source_lines) ||
    (start_sample < 1) || (start_sample > image_handle->source_line_samples) ||
    (stop_sample  < 1) || (stop_sample  > image_handle->source_line_samples)) {
  sprintf( error_string, "%s: invalid inputs.", proc_name);
  sprintf( error_string + strlen( error_string),
  "start_line = %ld, stop_line = %ld\nstart_sample = %ld, stop_sample = %ld",
  start_line, stop_line, start_sample, stop_sample);
  oa_errno = 502;
  OaReportError( error_string);
  return( NULL);
}

line_samples = stop_sample - start_sample + 1;
bytes = (stop_line - start_line + 1) * line_samples * 
        image_handle->sample_bytes;

/* Create the output oa_object and allocate space for the partial image. */

output_oa_object = OaNewOaObject();
if ((output_oa_object->data_ptr = (PTR) OaMalloc( (long) bytes)) == NULL) {
  sprintf( error_string, 
           "%s: OaMalloc failed to allocate %ld bytes for output image.",
            proc_name, bytes);
  oa_errno = 720;
  OaReportError( error_string);
  return( NULL);
}
output_oa_object->size = bytes;
output_oa_object->is_in_memory = TRUE;

/* Loop through calling OaReadImagePixels to get samples, and step through
   samples, copying the desired samples to the space allocated above.  
   buf_sample is the running offset, in samples, of the "current" sample in 
   the samples returned by OaReadImagePixels;  buf_samples is the total number
   of samples.  While copying samples from within a line, buf_sample is
   incremented by one each time;  when going to a new line, buf_sample is 
   incremented by samples_from_last_to_first.  */

buf_sample = 0;
buf_samples = 0;
dst_sample_ptr = output_oa_object->data_ptr;
samples_from_last_to_first = image_handle->source_line_samples - stop_sample +
                             start_sample;
for (line=start_line; line<=stop_line; line++) {
  for (sample=start_sample; sample<=stop_sample; sample++) {
    if ((buf_samples == 0) || (buf_sample >= buf_samples)) {
      if ((buf_samples = OaReadImagePixels( image_handle_object, line,  
                                            sample)) == 0) {
        sprintf( error_string, "%s: OaReadImagePixels returned error",
                 proc_name);
        OaReportError( error_string);
        return( NULL);
      }
      buf_sample = 0;
    }
    src_sample_ptr = image_handle_object->data_ptr + 
                     buf_sample * image_handle->sample_bytes;

    for (i=0; i<image_handle->sample_bytes; i++)
      dst_sample_ptr[i] = src_sample_ptr[i];
    buf_sample++;
    dst_sample_ptr += image_handle->sample_bytes;
  } /* end for loop through samples */

  /* Reached end of a line, so back up and recalculate buf_sample, because
     simply incrementing it by one won't work in general across lines (only 
     works if start_sample=1 and stop_sample=line_samples).  */

  buf_sample += (samples_from_last_to_first - 1);
} /* end for loop through lines */

/* Copy the ODL tree (without SDT nodes attached) describing the image samples,
   strip off prefix, image_line and suffix nodes, if any, and set keywords to
   reflect the output image's dimensions.  */

output_oa_object->odltree = OaCopyTree( image_handle_object->odltree, 
                                        OA_STRIP_SDT_NODES);

while (LeftmostChild( output_oa_object->odltree) != NULL)
  OdlFreeTree( OdlCutObjDesc( LeftmostChild( output_oa_object->odltree)));
OaLongtoKwdValue( "LINES", output_oa_object->odltree, 
                  (long) (stop_line - start_line + 1));
OaLongtoKwdValue( "LINE_SAMPLES", output_oa_object->odltree, line_samples);
OaLongtoKwdValue( "FIRST_LINE", output_oa_object->odltree, start_line);
OaLongtoKwdValue( "FIRST_SAMPLE", output_oa_object->odltree, start_sample);

return( output_oa_object);
}



/*****************************************************************************

  Routine:  OaReadTable

  Description:  OaReadTable opens a data file and reads a TABLE-like object 
                into memory.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:   1 Sept  1994

  History:

    Creation - This routine was part of the Alpha Release of the OA library.

  Input:  
         input_node - A pointer to an ODL tree node of class PALETTE, 
                      TABLE, SPECTRUM, SERIES, GAZETTEER and top-level 
                      (not nested) COLLECTION.

                      The tree above the input_node should have keywords
                      to specify the data file.
       
         The global variable Oa_profile is set to valid values.

  Output:  If successful, the routine returns a pointer to an OA_Object 
           structure, which contains an ODL tree and a pointer to the table
           data.  If unsuccessful it returns NULL.

  Notes:   

*****************************************************************************/

#ifdef _NO_PROTO

OA_OBJECT OaReadTable( input_node)
ODLTREE input_node;

#else

OA_OBJECT OaReadTable( ODLTREE input_node)

#endif
{

static char *proc_name = "OaReadTable";
long file_offset, record_bytes, file_records;
int record_type, column_major;
int src_interchange_format, dst_interchange_format;
char *label_filename, *data_filename;
OA_OBJECT oa_object;
ODLTREE current_node, sdt, table_node, compressed_SDT;
SDTNODE table_node_sdt_ptr;
int object_class, parent_object_class;
char *buf = NULL, c;
#ifdef IBM_PC
long i;
#endif
PTR data_ptr;
struct OaStreamStruct *stream_id;
long bytes_read, object_size;

/* Check inputs for validity. */

if (input_node == NULL) {
  sprintf( error_string, "%s: input ODL tree node is NULL.", proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return(NULL);
}

if (Parent( input_node) == NULL) {
  sprintf( error_string, 
           "%s: input ODL tree node must have a parent with file keywords.", 
           proc_name);
  oa_errno = 530;
  OaReportError( error_string);
  return(NULL);
}

object_class = OaGetObjectClass( input_node);
switch( object_class) { 

  case OA_COLLECTION:

  /* Check that the COLLECTION is a top-level object and not nested in
     one of the objects it is allowed to be nested in.  */

    parent_object_class = OaGetObjectClass( Parent( input_node));
    if ((parent_object_class == OA_COLLECTION) ||  
        (parent_object_class == OA_ARRAY)) {
      sprintf( error_string, "%s: input_node is not a top-level object.",
               proc_name);
      oa_errno = 530;
      OaReportError( error_string);
      return(NULL);
    }
  break;

  case OA_TABLE:
  case OA_SPECTRUM:
  case OA_SERIES:
  case OA_PALETTE:
  case OA_GAZETTEER:
  break;
  default:
    sprintf( error_string, 
             "%s: input ODLTREE node: %s is not a TABLE-like object.",
             proc_name, OaObjectClasstoStr( object_class));
    oa_errno = 530;
    OaReportError( error_string);
    return(NULL);
}

/* Check if TABLE is column-major or row-major.  */

column_major = FALSE;
OaKwdValuetoStr( "TABLE_STORAGE_TYPE", input_node, &buf);
if (buf != NULL) {
  UpperCase( buf);
  if (strstr( buf, "COLUMN_MAJOR") != NULL)
    column_major = TRUE;
  buf = NULL;
}

/* If it's column-major, search for a CONTAINER anywhere under the input node,
   and if found, reject the table - can't deal with column-major tables with
   CONTAINERs.  */

if (column_major == TRUE) {
  if (OdlFindObjDesc( input_node, "*CONTAINER*", NULL, NULL, 
                      (unsigned long) 0, 
                      (unsigned short) ODL_RECURSIVE_DOWN) != NULL) {
    sprintf( error_string, 
             "%s: column-major table with nested CONTAINER not allowed.", 
             proc_name);
    oa_errno = 530;
    OaReportError( error_string);
    return(NULL);
  }
}

/* Get the file keywords. */

if (OaGetFileKeywords( input_node, &label_filename, &data_filename,
                       &record_type, &record_bytes, &file_records,
                       &file_offset, &src_interchange_format) != 0) {
  return(NULL);  /* Error message already issued.  */
}

if (src_interchange_format == OA_ASCII_INTERCHANGE_FORMAT)
  dst_interchange_format = Oa_profile.dst_format_for_ASCII_src;
else
  dst_interchange_format = Oa_profile.dst_format_for_binary_src;

#ifdef OA_DEBUG
OaReportFileAttributes( label_filename, data_filename, record_type,
                        record_bytes, file_offset, src_interchange_format);
#endif

/* The input tree ODL tree shouldn't ever be modified, so make a copy of all
   the nodes below the input node.  */

table_node = OaCopyTree( input_node, OA_STRIP_COMMENTS | OA_STRIP_SDT_NODES);

if (OaCheckODLTree( table_node) != 0) {
  OdlFreeTree( table_node);
  return(NULL);  /* Error message already issued and oa_errno set.  */
}


/* OalCreateSDT always strips out row prefix bytes and row suffix bytes; 
   check that ROW_PREFIX_BYTES and ROW_SUFFIX_BYTES aren't defined for
   a column-major TABLE.  */

if (column_major == TRUE) {
  if (OdlFindKwd( table_node, "ROW*FIX*BYTES", "*", (unsigned long) 0,
                 (unsigned short) ODL_THIS_OBJECT) != NULL) {
    sprintf( error_string, 
    "%s: column-major TABLEs can't have ROW_PREFIX_BYTES or ROW_SUFFIX_BYTES.",
             proc_name);
    oa_errno = 530;
    OaReportError( error_string);
    OdlFreeTree( table_node);
    return(NULL);
  }
}

/* Call OalCreateSDT to create the SDT (Stream Decomposition Tree).  */

if ((sdt = OalCreateSDT( table_node, src_interchange_format)) == NULL) {
  sprintf( error_string, "%s: CreateSDT returned error.", 
           proc_name);
  OaReportError( error_string);
  OalFreeSDT( table_node);
  return(NULL);
}

/* Note: sdt and table_node now point to the same node. */

/* Allocate memory for the table, and store the pointer in the root (TABLE)
   node of the SDT.  */

table_node_sdt_ptr = (SDT_node *) sdt->appl1;
object_size = table_node_sdt_ptr->dst.size * 
              table_node_sdt_ptr->total_repetitions;
if ((data_ptr = (PTR) OaMalloc( (long) object_size)) == NULL) {
  sprintf( error_string, "%s: OaMalloc failed! Out of memory!", proc_name);
  oa_errno = 720;
  OaReportError( error_string);
  exit(1);
  /*NOTREACHED*/
}

if (dst_interchange_format == OA_ASCII_INTERCHANGE_FORMAT) 
  c = ' ';
else
  c = 0;

#ifdef IBM_PC

/* If we're on an IBM-PC, use a for loop to do the set, since memset
   may not work for setting greater than 64K and/or over segment
   boundaries;  otherwise use memset since it's usually optimized.  */

for (i=0; i<object_size; i++)
  data_ptr[i] = c;
#else
memset( data_ptr, c, (size_t) object_size);
#endif

if ((stream_id = OalOpenStream( data_filename, record_type, record_bytes, 
                                file_records, "r")) == NULL) {
  sprintf( error_string, "%s: OalOpenStream returned error.",
           proc_name);
  OaReportError( error_string);
  return(NULL);
}

/* Read the first slice of data; tell OalReadStream to read as many bytes as it
   wants, store them in it's own buffer, start reading the file at the given
   byte offset, and output how many bytes it's read in bytes_read.  */

if (OalReadStream( stream_id, (long) 0, &buf, file_offset, &bytes_read) != 0) {
  sprintf( error_string, "%s: first call to OalReadStream returned error, ",
           proc_name);
  strcat( error_string, "aborting.");
  OaReportError( error_string);
  OalFreeSDT( table_node);
  OalCloseStream( stream_id);
  return(NULL);
}

/* Compress the SDT, and initialize current_node to point to the first
   end-node (data node) in the compressed SDT; initialize the node's dst.ptr
   with the pointer returned by OaMalloc.  */

compressed_SDT = OalCompressSDT( sdt);
current_node = OalInitializeSDT( compressed_SDT, data_ptr);

/* The main loop processes a buffer of data, then reads in the next buffer; 
   the SDT maps desired data to memory and throws aways the rest.  */

for (;;) {
  if (OalProcessSDT( (PTR) buf, bytes_read, &current_node) ==
      OA_REACHED_END_OF_SDT) break;

  /*  Read next slice of data, telling OalReadStream to do the same as the
      first time, except start at the current file position.  */

  buf = NULL;
  if (OalReadStream( stream_id, (long) 0, &buf, (long) -1, &bytes_read) != 0) {
    sprintf( error_string, "%s: OalReadStream returned error, aborting.",
             proc_name);
    OaReportError( error_string);
    break;
  }
}
OalCloseStream( stream_id);

/* Free the compressed SDT. */

OalFreeSDT( compressed_SDT);

/* Call OalSDTtoODLTree to modify the new ODL tree to reflect the in-memory 
   data.  This changes the DATA_TYPE keywords to reflect the in-memory data
   values which may have been converted to native types.  It also strips off 
   ODL tree nodes which have SDT_node->dst.size=0.  */

OalSDTtoODLTree( sdt, dst_interchange_format);

/* Remove keywords for ROW_PREFIX_BYTES and ROW_SUFFIX_BYTES,
   since these have been stripped off.  */

OaDeleteKwd( "ROW*PREFIX*BYTES", sdt);
OaDeleteKwd( "ROW*SUFFIX*BYTES", sdt);
OaDeleteKwd( "^*PREFIX*STRUCTURE", sdt);
OaDeleteKwd( "^*SUFFIX*STRUCTURE", sdt);

/* Create the output oa_object and initialize it.  */

oa_object = OaNewOaObject();
oa_object->odltree = sdt;
oa_object->data_ptr = data_ptr;
oa_object->size = object_size;
oa_object->is_in_memory = TRUE;
oa_object->stream_id = NULL;
oa_object->profile = Oa_profile;

return( oa_object);
}



/*****************************************************************************

  Routine:  OaReportFileAttributes

  Description:  This routine uses the OaReportError function to print the
                file attributes of an object to the error log.  It is called
                by most of the OaRead* functions.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  23 Nov  1994
  Last Modified:  23 Nov  1994

  History:

    Creation - This routine was part of the Alpha Release of the OA library.

  Input:  
          label_filename            - A NULL terminated string.

          data_filename             - A NULL terminated string.

          record_type               - The value of the RECORD_TYPE keyword,
                                      converted to a oa_record_type_enum value.

          record_bytes              - The value of the RECORD_BYTES keyword.

          file_offset               - The ^POINTER object start offset value, 
                                      converted to a byte offset for
                                      fixed-length and stream record formats.

          object_interchange_format - An oa_interchange_format_enum.
                                      
         
  Output:  OaReportError is called with a string which shows the values
           of the input arguments.  The function always returns 0.

  Notes:  

*****************************************************************************/
#ifdef _NO_PROTO

int OaReportFileAttributes( label_filename, data_filename, record_type,
                            record_bytes, file_offset, 
                            object_interchange_format)
char *label_filename;
char *data_filename;
int record_type;
long record_bytes;
long file_offset;
int object_interchange_format; 

#else

int OaReportFileAttributes( char *label_filename, char *data_filename, 
                            int record_type, long record_bytes, 
                            long file_offset, 
                            int object_interchange_format)

#endif
{

  sprintf( error_string, 
           "label_filename     = %s\n", label_filename);
  sprintf( error_string + strlen( error_string),
           "data_filename      = %s\n", data_filename);
  sprintf( error_string + strlen( error_string),
           "file_offset        = %ld\n",file_offset);
  sprintf( error_string + strlen( error_string),
           "record_bytes       = %ld\n",
           record_bytes);
  sprintf( error_string + strlen( error_string),
           "record_type        = ");
  switch (record_type) {
    case OA_FIXED_LENGTH:
      strcat( error_string + strlen( error_string), "FIXED_LENGTH\n");
      break;
    case OA_STREAM:
      strcat( error_string + strlen( error_string), "STREAM\n");
      break;
    case OA_VARIABLE_LENGTH:
      strcat( error_string + strlen( error_string), "VARIABLE_LENGTH\n");
      break;
    default:
      strcat( error_string + strlen( error_string), "UNDEFINED\n");
      break;
   }
  sprintf( error_string + strlen( error_string),
           "interchange_format = %s\n",
           (object_interchange_format == OA_ASCII_INTERCHANGE_FORMAT ? 
            "ASCII_INTERCHANGE_FORMAT" : "BINARY_INTERCHANGE_FORMAT"));
  oa_errno = 950;
  OaReportError( error_string);
  return(0);
}



/*****************************************************************************

  Routine:  OaTransposeTable

  Description:  OaTransposeTable converts the data and ODL tree for an
                in-memory TABLE from one TABLE_STORAGE_TYPE to another.
                If the table is ROW_MAJOR, it will be converted to 
                COLUMN_MAJOR;  if COLUMN_MAJOR, it will be converted to
                ROW_MAJOR.
                The table may not have any CONTAINER objects, only COLUMNS.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  15 Nov  1994
  Last Modified:  15 Nov  1994

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:  
         table_object  - A pointer to an Oa_Object structure with an 
                         in-memory TABLE object.

  Output:  If successful, the function returns a pointer to the input Oa_Object
           structure, otherwise NULL.  The Oa_Object has a new ODL tree, whose
           TABLE_STORAGE_TYPE keyword is set, and its data_ptr points to the
           transposed data.  The old data is freed.
           Note: The returned ODL tree is a new tree, and the old one has
           been freed.  Any pointers the caller may have saved which point
           to nodes in the old tree now point to freed memory!

  Notes:
  1) For COLUMN_MAJOR to ROW_MAJOR, the data is aligned according to the
     input table_object->profile.   An SDT is created, but processed manually 
     instead of by OalProcessSDT, so no data conversions are performed.
  2) A COLUMN_MAJOR output table will always have the same size in bytes as
     the ROW_MAJOR input table.
  3) A ROW_MAJOR output table may NOT always have the same size as the
     COLUMN_MAJOR input table because of the addition of alignment pad bytes
     between columns.  In this case the ROW_BYTES keyword is updated.
  
*****************************************************************************/

#ifdef _NO_PROTO

OA_OBJECT OaTransposeTable( table_object)
OA_OBJECT table_object;

#else

OA_OBJECT OaTransposeTable( OA_OBJECT table_object)

#endif
{

static char *proc_name = "OaTransposeTable";
ODLTREE table_node, sdt, current_node;
long new_object_size, row_bytes, new_row_bytes, rows, i;
#ifdef IBM_PC
long j;
#endif
long prefix_bytes, suffix_bytes;
int table_storage_type;
char pad_char;
PTR data_ptr, src_ptr, dst_ptr;
int src_interchange_format, dst_interchange_format;
SDT_node *sdt_node_ptr;
struct oa_profile save_profile;

/* Check for valid inputs. */

if (table_object == NULL) {
  sprintf( error_string, "%s: input table_object is NULL.", proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return( NULL);
}

if (table_object->odltree == NULL) {
  sprintf( error_string, "%s: input table_object's ODL tree pointer is NULL.", 
           proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return( NULL);
}

if (table_object->data_ptr == NULL) {
  sprintf( error_string, "%s: input table_object's data_ptr is NULL.", 
           proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return( NULL);
}

table_node = table_object->odltree;
if (OaGetObjectClass( table_node) != OA_TABLE) {
  sprintf( error_string, "%s: table_object->odltree must be a TABLE.",
           proc_name);
  oa_errno = 530;
  OaReportError( error_string);
  return( NULL);
}

current_node = LeftmostChild( table_node);
if (current_node == NULL) {
  sprintf( error_string, "%s: TABLE has no sub-objects.", proc_name);
  oa_errno = 530;
  OaReportError( error_string);
  return( NULL);
}
while (current_node != NULL) {
  if (OaGetObjectClass( current_node) != OA_COLUMN) {
    sprintf( error_string, 
             "%s: all children of the TABLE node must be COLUMNS.", proc_name);
    oa_errno = 530;
    OaReportError( error_string);
    return( NULL);
  }
  current_node = RightSibling( current_node);
}

if (OaGetTableKeywords( table_node, &rows, &row_bytes, &prefix_bytes,
                        &suffix_bytes, &src_interchange_format,
                        &table_storage_type) != 0)
  return( NULL);  /* Error message already written and oa_errno set.  */

if ((prefix_bytes > 0) || (suffix_bytes > 0)) {
  sprintf( error_string, 
           "%s: ROW_PREFIX_BYTES or ROW_SUFFIX_BYTES not allowed.",
           proc_name);
  oa_errno = 520;
  OaReportError( error_string);
  return( NULL);
}

dst_interchange_format = src_interchange_format;

/* Make a copy of the tree; in it change the TABLE_STORAGE_TYPE keyword
   to ROW_MAJOR, then build an SDT.
   This SDT is built solely for the convenience of having all the offsets
   and sizes converted to binary, and having the alignment figured out;
   the actual data transfer is done by a loop below, instead of by
   OaProcessSDT as is normally done.  Thus no data conversions are done.  */

sdt = OaCopyTree( table_node, OA_STRIP_SDT_NODES | OA_STRIP_COMMENTS);
OaStrtoKwdValue( "TABLE_STORAGE_TYPE", sdt, "ROW_MAJOR");

/* Set the alignment_type to be whatever the alignment_type was when the
   table was created.  If going from COLUMN_MAJOR to ROW_MAJOR, then each row
   of data will be aligned to this specification.  If going from ROW_MAJOR to
   COLUMN_MAJOR, then each column will be aligned by the addition of a whole
   new column of pads preceeding it;  this wastes space, because only a few
   pad bytes are really necessary, but there's no way to represent this in the
   ODL tree.
   After setting the global profile, set it to force keeping the same
   interchange format.  Restore the global profile after creating the SDT.  */

save_profile = Oa_profile;
Oa_profile = table_object->profile;
if (src_interchange_format == OA_ASCII_INTERCHANGE_FORMAT)
  Oa_profile.dst_format_for_ASCII_src = src_interchange_format;
else
  Oa_profile.dst_format_for_binary_src = src_interchange_format;

if ((sdt = OalCreateSDT( sdt, src_interchange_format)) == NULL) {
  sprintf( error_string, "%s: CreateSDT returned error.", 
           proc_name);
  OaReportError( error_string);
  OalFreeSDT( sdt);
  Oa_profile = save_profile;
  return(NULL);
}
Oa_profile = save_profile;

/* Update the ROW_BYTES keyword, which may be different now if the SDT
   specifies alignment pads to be added.  */

sdt_node_ptr = (SDT_node *) sdt->appl1;
new_row_bytes = sdt_node_ptr->dst.size;
OaLongtoKwdValue( "ROW_BYTES", sdt, new_row_bytes);

new_object_size = new_row_bytes * rows;

/* Allocate space for the transposed table data.  */

if ((data_ptr = (PTR) OaMalloc( (long) new_object_size)) == NULL) {
  sprintf( error_string, "%s: OaMalloc failed! Out of memory!", proc_name);
  oa_errno = 720;
  OaReportError( error_string);
  exit(1);
  /*NOTREACHED*/
}

/* Determine which character to use in alignment pad bytes - an ASCII space
   for ASCII, zero for binary.  */

if (dst_interchange_format == OA_ASCII_INTERCHANGE_FORMAT) 
  pad_char = ' ';
else
  pad_char = 0;

if (table_storage_type == OA_ROW_MAJOR) {  /* Convert to COLUMN_MAJOR */

  /* Loop through all the columns in the table;  transfer each column from the
     ROW_MAJOR table's data_ptr to the new COLUMN_MAJOR table's data_ptr,
     then go to the next column.   Add pad byte columns to the new 
     COLUMN_MAJOR table if pad bytes are specified in the SDT.  */

  current_node = LeftmostChild( sdt);
  while (current_node != NULL) {
    sdt_node_ptr = (SDT_node *) current_node->appl1;
    dst_ptr = data_ptr + rows * sdt_node_ptr->dst.start_offset;
    if (sdt_node_ptr->src.size == 0) {

      /* Src.size is 0, so OalCreateSDT put this node here to align the next
         dst column;  there is no source data in the ROW_MAJOR table.  
         Although the SDT describes a ROW_MAJOR table, some easy analysis 
         shows that putting in the alignment pads it specifies will align a
         the column of the COLUMN_MAJOR table, and so will a whole column of
         the alignment pads.  This wastes space, since only a few bytes
         are really needed to align something, but any other scheme that used
         fewer bytes wouldn't be representable in the ODL tree.    */

#ifdef IBM_PC

    /* If we're on an IBM-PC, use a for loop to do the set, since memset
       may not work for setting greater than 64K and/or over segment
       boundaries;  otherwise use memset since it's usually optimized.  */

    for (i=0; i<(rows * sdt_node_ptr->dst.size); i++)
      dst_ptr[i] = pad_char;
#else
      memset( dst_ptr, pad_char, rows * sdt_node_ptr->dst.size);
#endif

    } else {

      /* Copy all the data for the current column from the ROW_MAJOR table
         to the COLUMN_MAJOR table.  */

      for (i=0; i<rows; i++) {

#ifdef IBM_PC

        /* If we're on an IBM-PC, use a for loop to do the copy, since memcpy
           may not work for setting greater than 64K and/or over segment
           boundaries;  otherwise use memcpy since it's usually optimized.  */

        for (j=0; j<sdt_node_ptr->src.size; j++)
          dst_ptr[i * sdt_node_ptr->src.size + j] = 
            table_object->data_ptr[ i*row_bytes + 
                                    sdt_node_ptr->src.start_offset + j];
#else
        memcpy( dst_ptr + i * sdt_node_ptr->src.size,
                (char *) table_object->data_ptr + i*row_bytes + 
                sdt_node_ptr->src.start_offset, sdt_node_ptr->src.size);
#endif
      }
    }
    current_node = RightSibling( current_node);
  }  /* end while current_node != NULL */

} else {                                   /* Convert to ROW_MAJOR */

  /* Loop through all the columns in the table;  transfer each column from the
     COLUMN_MAJOR table to the ROW_MAJOR table before going to the next column.
     Add pad bytes to the ROW_MAJOR table if specified in the SDT.  */

  current_node = LeftmostChild( sdt);
  while (current_node != NULL) {
    sdt_node_ptr = (SDT_node *) current_node->appl1;
    if (sdt_node_ptr->src.size == 0) {

      /* Src.size is 0, so this node is used to align the next dst column;  
         there is no source data. */

      for (i=0; i<rows; i++) {

#ifdef IBM_PC

        /* If we're on an IBM-PC, use a for loop to do the set, since memset
           may not work for setting greater than 64K and/or over segment
           boundaries;  otherwise use memset since it's usually optimized.  */

        for (j=0; j<sdt_node_ptr->dst.size; j++)
          data_ptr[i*new_row_bytes + sdt_node_ptr->dst.start_offset + j] = 
            pad_char;
#else
        memset( data_ptr + i*new_row_bytes + sdt_node_ptr->dst.start_offset,
                pad_char, sdt_node_ptr->dst.size);
#endif
      }
    } else {

      /* Copy all the data for the current column from the COLUMN_MAJOR table
         to the ROW_MAJOR table.  */

      src_ptr = (char *) table_object->data_ptr +
                rows * sdt_node_ptr->src.start_offset;
      for (i=0; i<rows; i++) {

#ifdef IBM_PC

        /* If we're on an IBM-PC, use a for loop to do the copy, since memcpy
           may not work for setting greater than 64K and/or over segment
           boundaries;  otherwise use memcpy since it's usually optimized.  */

        for (j=0; j<sdt_node_ptr->src.size; j++)
          data_ptr[ i*new_row_bytes + sdt_node_ptr->dst.start_offset + j] = 
            src_ptr[ i*sdt_node_ptr->src.size + j];
#else
        memcpy( data_ptr + i*new_row_bytes + sdt_node_ptr->dst.start_offset,
                src_ptr + i*sdt_node_ptr->src.size,  sdt_node_ptr->src.size);
#endif
      }
    }
    current_node = RightSibling( current_node);
  }  /* end while current_node != NULL */
}  /* end else table_storage_type is COLUMN_MAJOR */

/* OalSDTtoODLTree strips off ODL tree nodes which have SDT_node->dst.size=0
   (spare nodes added for alignment).  In this case it doesn't update the 
   DATA_TYPE keywords, since there weren't any conversions.  */

OalSDTtoODLTree( sdt, dst_interchange_format);

if (table_storage_type == OA_ROW_MAJOR)  /* Now converted to COLUMN_MAJOR */
  OaStrtoKwdValue( "TABLE_STORAGE_TYPE", sdt, "COLUMN_MAJOR");
else
  OaStrtoKwdValue( "TABLE_STORAGE_TYPE", sdt, "ROW_MAJOR");

/* Replace the input table_object's ODL tree with the new ODL tree, free the
   old data_ptr, and replace it with the new data_ptr.  */

OaLongtoKwdValue( "COLUMNS", sdt, OdlGetObjDescChildCount( sdt));
OdlFreeTree( table_object->odltree);
table_object->odltree = sdt;
OaFree( (char *) table_object->data_ptr);
table_object->data_ptr = data_ptr;
table_object->size = new_object_size;
return( table_object);
}



/*****************************************************************************

  Routine: OaUnravelContainer

  Description:  This routine effects only the object's ODL tree; the object
                data is unchanged.  The routine acts on a TABLE whose only
                sub-object is a CONTAINER, removes the container and places
                all the container's sub-objects directly under the table node.
                It adjusts the ROWS and ROW_BYTES keywords of the table node
                so it looks like each CONTAINER repetition became a row of the
                table.

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  14 Feb   1995
  Last Modified:  20 Feb   1996

  History:

    Creation - This routine was part of the Beta Release of the OA library.
    02/20/96 - Copy NAME and DESCRIPTION keywords from CONTAINER node to
               TABLE node, erasing previous values of these keywords in TABLE
               node, if any. SM

  Input:  
         table_object - A pointer to an Oa_Object structure with an
                        in-memory TABLE-like object.  The table must
                        be ROW_MAJOR.

  Output:  If successful, the function returns a pointer to the input
           Oa_Object structure, otherwise NULL.  The object's ODL tree has
           been modified as described above.

  Notes:

*****************************************************************************/

#ifdef _NO_PROTO

OA_OBJECT OaUnravelContainer( table_object)
OA_OBJECT table_object;

#else

OA_OBJECT OaUnravelContainer( OA_OBJECT table_object)

#endif
{

static char *proc_name = "OaUnravelContainer";
ODLTREE table_node, container_node, odltreenode, next_node;
long rows, row_bytes, container_bytes, container_repetitions;
long prefix_bytes, suffix_bytes;
int table_storage_type, interchange_format;
char *str;

/* Check for valid inputs. */

if (table_object == NULL) {
  sprintf( error_string, "%s: input table_object is NULL.", proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return( NULL);
}

if (table_object->odltree == NULL) {
  sprintf( error_string, "%s: input table_object's ODL tree pointer is NULL.", 
           proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return( NULL);
}

table_node = table_object->odltree;

if (OaGetTableKeywords( table_node, &rows, &row_bytes, &prefix_bytes,
                        &suffix_bytes, &interchange_format,
                        &table_storage_type) != 0)
  return( NULL);  /* Error message already written and oa_errno set.  */

/* Check that TABLE_STORAGE_TYPE is ROW MAJOR.  CONTAINERS don't make sense
   for a COLUMN MAJOR table.  */

if (table_storage_type != OA_ROW_MAJOR) {
  sprintf( error_string, "%s: %s's TABLE_STORAGE_TYPE must be ROW_MAJOR.",
           proc_name, OdlGetObjDescClassName( table_node));
  oa_errno = 531;
  OaReportError( error_string);
  return( NULL);
}

/* Check that the table has a CONTAINER node as its only sub-object.  */

if (OdlGetObjDescChildCount( table_node) != 1) {
  sprintf( error_string,  
        "%s: the input table node must have CONTAINER as its only sub-object",
           proc_name);
  oa_errno = 530;
  OaReportError( error_string);
  return( NULL);
}

container_node = LeftmostChild( table_node);
if (OaGetObjectClass( container_node) != OA_CONTAINER) {
  sprintf( error_string,  
           "%s: the subobject of the input table node must be a CONTAINER",
           proc_name);
  oa_errno = 530;
  OaReportError( error_string);
  return( NULL);
}

if (OaKwdValuetoLong( "BYTES", container_node, &container_bytes) != 0) {
  sprintf( error_string, "%s: BYTES keyword not found in CONTAINER node", 
                         proc_name);
  oa_errno = 531;
  OaReportError( error_string);
  return( NULL);
}
if (OaKwdValuetoLong( "REPETITIONS", container_node,  
                      &container_repetitions) != 0) {
  sprintf( error_string, 
           "%s: REPETITIONS keyword not found in CONTAINER node", proc_name);
  oa_errno = 531;
  OaReportError( error_string);
  return( NULL);
}

/* Move all the table node's children from the container to under the table
   node.  */

odltreenode = LeftmostChild( container_node);
while (odltreenode != NULL) {
  next_node = RightSibling( odltreenode);
  OdlPasteObjDesc( OdlCutObjDesc( odltreenode), table_node);
  odltreenode = next_node;
}

/* Copy NAME and DESCRIPTION keywords from CONTAINER node to TABLE node. */

if (OaKwdValuetoStr( "NAME", container_node, &str) == 0)
  OaStrtoKwdValue( "NAME", table_node, str);
if (OaKwdValuetoStr( "DESCRIPTION", container_node, &str) == 0)
  OaStrtoKwdValue( "DESCRIPTION", table_node, str);

OdlFreeTree( OdlCutObjDesc( container_node));

OaLongtoKwdValue( "ROW_BYTES", table_node, container_bytes);
rows *= container_repetitions;
OaLongtoKwdValue( "ROWS", table_node, rows);
OaLongtoKwdValue( "COLUMNS", table_node, 
                  (long) OdlGetObjDescChildCount( table_node));

return( table_object);
}



/*****************************************************************************

  Routine:  OaWriteObject

  Description:  This routine writes the input object's data to the data file
                in file_object's stream structure, and updates the
                file_object's ODL tree.  It is part of the calling sequence:
                OaOpenOutputFile, OaWriteObject (multiple calls), 
                OaCloseOutputFile.
                - If the input object has an ODL tree, it attaches a copy of
                  the object's ODL tree as the last (rightmost) node under
                  file_object's root, and adds a ^POINTER keyword to the file
                  object's root node.
                - If the input object does not have an ODL tree, it simply
                  writes the object data to the file.  This can be used when
                  making multiple calls to write an object which is too big
                  to store in memory at one time.
                - For a HISTORY object, it writes the HISTORY's ODL tree to a
                  temporary file using L3's OdlPrintLabel, then reads it back
                  into the object data of a temporary OA_OBJECT; this data is
                  then written to the file, and a ^POINTER is added in the
                  root node.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:  11 Nov   1996

  History:

    Creation - This routine was part of the Alpha Release of the OA library.
    01/09/95 - Added writing of the HISTORY object.
    12/11/95 - Added usage of OaGetTableKeywords.
    11/06/96 - Redesigned to allow writing of partial object data.
    11/11/96 - Added support for UNDEFINED record type. SM

  Input:  
         file_object - An oa_object returned by a previous call to 
                       OaOpenOutputFile, and may have been updated by previous
                       calls to OaWriteObject.

         object      - An oa_object with data in memory (except HISTORY
                       object) and an ODL tree attached.
         
  Output:  If successful the function returns 0, otherwise non-zero.
           The input object is unchanged.
           file_object is updated and the data is written to the file.  

  Notes:  
  
  1) The data is written to the file exactly the same as it was in memory - no
     conversions are done, no line terminators are added etc.
  2) For FIXED_LENGTH record files, if the object size is not a multiple of
     RECORD_BYTES, then OalWriteStream will pad the last record out to
     RECORD_BYTES.
  3) To write out an object too big to store in memory at once, the first call
     to OaWriteObject should provide the final (full-size) object's ODL tree,
     along with the first chunk of object data.  Subsequent calls to 
     OaWriteObject should provide only object data; the input object's ODL
     tree should be set to NULL.  Care should be taken when writing to a
     FIXED_LENGTH record file that the data size of each data chunk passed to
     OaWriteObject is a multiple of RECORD_BYTES, otherwise pad data will be
     written at the end of the chunk, as in 2) above.  For complete control
     over how the object data is formatted into records, call                 
     OaOpenOutputFile and make a single call to OaWriteObject in order to get
     the ^POINTER;  then write the rest of the data using OalWriteStream 
     calls, ending with a call to OaCloseOutputFile.  This is how you would
     write an HFD compressed image to an output file, one compressed line at
     a time.

*****************************************************************************/
   
#ifdef _NO_PROTO

int OaWriteObject( file_object, object)
OA_OBJECT file_object;
OA_OBJECT object;

#else

int OaWriteObject( OA_OBJECT file_object, OA_OBJECT object)

#endif
{

static char *proc_name = "OaWriteObject";
int object_class=0, object_interchange_format;
char kwd_name[80], kwd_value[40], tmp_file_name[80], *str;
PTR ptr;
FILE *tmp_fd;
long bytes_written=0, bytes_to_write=0, bytes_remaining=0, byte_count=0;
long object_start_position, record_count=0;
ODLTREE current_node, root_node;
KEYWORD *kwd_ptr, *next_kwd_ptr;
#if (defined( VAX) || defined( ALPHA_VMS))
stat_t stat_buffer;
#else
#ifndef MAC
struct stat stat_buffer;
#endif
#endif

/* Check inputs for validity. */

if (file_object == NULL) {
  sprintf( error_string, 
           "%s: input parameter file_object is NULL.", proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return(-1);
}

if (file_object->stream_id == NULL) {
  sprintf( error_string, "%s: input parameter file_object's stream_id ",
           proc_name);
  strcat( error_string, "is NULL.");
  oa_errno = 501;
  OaReportError( error_string);
  return(-1);
}

if (file_object->odltree == NULL) {
  sprintf( error_string, "%s: input parameter file_object's ODL tree is NULL.",
           proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return(-1);
}

if (object == NULL) {
  sprintf( error_string, "%s: input parameter object is NULL.", proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return(-1);
}

if (object->odltree != NULL) {

  object_class = OaGetObjectClass( object->odltree);
  if ((object_class != OA_HISTORY) &&
       ((object->data_ptr == NULL) || (object->size < 1))) {
    sprintf( error_string,
             "%s: bad input: object's data_ptr is NULL or size < 1.",
             proc_name);
    oa_errno = 501;
    OaReportError( error_string);
    return(-1);
  }

  /* Check that the object class/name (OBJECT = <object class/name> keyword)
     doesn't match an existing ^POINTER.  Can't have two objects with the same
     name in the same data file, because their ^POINTERs names would be
     indistinguishable.  */

  sprintf( kwd_name, "^%s", object->odltree->class_name);
  if ((kwd_ptr = OdlFindKwd( file_object->odltree, kwd_name, "*", 
                             (unsigned long) 0, 
                             (unsigned short) ODL_THIS_OBJECT)) != NULL) {
    sprintf( error_string, 
             "%s: an object named %s already exists in this file.",
             proc_name, object->odltree->class_name);
    oa_errno = 520;
    OaReportError( error_string);
    return(1);
  }

  /* Get the object's INTERCHANGE_FORMAT.  */

  if ((object_interchange_format = OaGetObjectInterchangeFormat( 
         object->odltree)) == OA_UNKNOWN_INTERCHANGE_FORMAT) {
    return(-1);  /* Error message already issued.  */
  }

  /* If the object is binary and the file's record_type is OA_STREAM, then flag
     an error, because STREAM files are for ASCII text only.  */

  if ((object_interchange_format == OA_BINARY_INTERCHANGE_FORMAT) &&
      (file_object->stream_id->record_type == OA_STREAM)) {
      sprintf( error_string, "%s: binary objects not allowed in STREAM files.",
               proc_name);
      oa_errno = 520;
      OaReportError( error_string);
      return(-1);
  }

  /* If the object is ASCII and the profile flag check_ASCII_writes is TRUE,
     then check that every byte of the data is either an alpha-numeric ASCII
     character, a space or a common control character.  Note: isspace() returns
     TRUE for space, formfeed, newline, carriage return, tab, vertical tab. */

  if ((object_interchange_format == OA_ASCII_INTERCHANGE_FORMAT) &&
      (Oa_profile.check_ASCII_writes == TRUE)) {
    ptr = object->data_ptr;
    for (byte_count=0; byte_count<object->size; byte_count++) {
      if ( !isprint( *ptr) && (*ptr != '\r') && (*ptr != '\n')) {
        sprintf( error_string,
                 "%s: Oa_profile.check_ASCII_writes is TRUE and ", proc_name);
        strcat( error_string, "detected byte which\nis not an ASCII ");

        strcat( error_string, "alpha-numeric or allowed control character.\n");
        sprintf( error_string + strlen( error_string),
                 "The byte is (in hex): %x, at byte offset: %ld.", (int) *ptr,
                  byte_count);
        oa_errno = 520;
        OaReportError( error_string);
        return(-1);
      } else {
        ptr++;
      }
    }
    byte_count = 0;
  }

  /* If it's a HISTORY object, call OdlPrintLabel to write its ODL tree to a
     temporary file;  then get the file size, read the whole file into memory,
     and set object->data_ptr to point to it.  Then continue as with any other
     object.  */

  if (object_class == OA_HISTORY) {
  
    tmpnam( tmp_file_name);

    /* Open the temporary file. */

    if ((tmp_fd = fopen( tmp_file_name, "w")) == NULL) {
      sprintf( error_string, "%s: couldn't open HISTORY tmp file for write", 
               proc_name);
      oa_errno = 700;
      OaReportError( error_string);
      return(-1);
    }

    /* Write the HISTORY object's ODL tree to the temporary file.  Have to
       add a root node so that OdlPrintLabel will write the "OBJECT = HISTORY"
       and "END_OBJECT = HISTORY" lines.  */

    root_node = OdlNewObjDesc( "ROOT", NULL, NULL, NULL, NULL, NULL, 
                               (short) 0, (long) 0);
    OdlPasteObjDesc( object->odltree, root_node);

    OdlPrintLabel( root_node, NULL, tmp_fd, (unsigned long) 0, (MASK) 0);
    OdlCutObjDesc( object->odltree);
    OdlFreeTree( root_node);
    fclose( tmp_fd);

    /* Open the tmp file for read, get its size, and allocate space.  */

    if ((tmp_fd = fopen( tmp_file_name, "r")) == NULL) {
      sprintf( error_string, "%s: couldn't open HISTORY tmp file for read", 
               proc_name);
      oa_errno = 700;
      OaReportError( error_string);
      return(-1);
    }
#ifdef MAC  /* Some compilers don't have fstat(), so use seek() instead.  */
    fseek( tmp_fd, (long) 0, 2);  /* Seek to end-of-file.  */
    object->size = ftell( tmp_fd);
    fseek( tmp_fd, (long) 0, 0);      /* Seek back to beginning of file.  */
#else
    if (fstat( fileno( tmp_fd), &stat_buffer) == 0) {
      object->size = (long) stat_buffer.st_size;
    } else {
      sprintf( error_string, "%s: fstat on HISTORY tmp file returned error",
               proc_name);
      oa_errno = 700;
      OaReportError( error_string);
      return(-1);
    }
#endif
    if (object->size == 0) {
      sprintf( error_string, 
               "%s: OdlPrintLabel failed to write label!", proc_name);
      oa_errno = 1000;
      OaReportError( error_string);
      return(-1);
    }
    if ((object->data_ptr = (PTR) OaMalloc( (long) object->size)) == NULL) {
      sprintf( error_string, "%s: OaMalloc failed to allocate %ld bytes!",
               proc_name, (long) object->size);
      strcat( error_string, " Out of memory!");
      oa_errno = 720;
      OaReportError( error_string);
      object->size = 0;
      return(-1);
    }
    if (fread( object->data_ptr, (size_t) 1, (size_t) object->size, tmp_fd) < 
               (size_t) object->size) {
      sprintf( error_string, "%s: fread() failed to read HISTORY tmp file",
               proc_name);
      oa_errno = 700;
      OaReportError( error_string);
      fclose( tmp_fd);
      object->data_ptr = NULL;
      object->size = 0;
      return(-1);
    }
    fclose( tmp_fd);
    remove( tmp_file_name);
  }
}

/* Save the current file position for use later as a ^POINTER value to
   the start of the object. */

object_start_position = file_object->stream_id->current_position;

/* Now make calls to OalWriteStream to write the data to the file.
   OalWriteStream will write record_bytes at a time, until the last one,
   which may be less than record_bytes;  if record_type is OA_FIXED_LENGTH, 
   then OalWriteStream will pad it with 0's to be of size record_bytes before
   writing it.  */

bytes_written = 0;
bytes_remaining = object->size;
record_count = 0;
      
while (bytes_remaining > 0) {
  switch( file_object->stream_id->record_type) {
    case OA_FIXED_LENGTH:
    case OA_VARIABLE_LENGTH:
      if (bytes_remaining < file_object->stream_id->record_bytes)
        bytes_to_write = bytes_remaining;
      else 
        bytes_to_write = file_object->stream_id->record_bytes;
    break;
    case OA_STREAM:
    case OA_UNDEFINED_RECORD_TYPE:
      bytes_to_write = bytes_remaining;
    break;
  }
  OalWriteStream( file_object->stream_id,  bytes_to_write, 
                  (char *) object->data_ptr + bytes_written, &byte_count);
  if (byte_count >= bytes_to_write) {
    bytes_written += bytes_to_write;
    bytes_remaining -= bytes_to_write;
  } else {
    sprintf( error_string, "%s: OalWriteStream wrote %ld bytes ",
             proc_name, byte_count);
    sprintf( error_string + strlen( error_string), 
             "when told to write %ld bytes.", bytes_to_write);
    oa_errno = 700;
    OaReportError( error_string);
    return(-1);
  }
  record_count++;
}

/* Attach a copy of the new object's ODL tree to the file object tree
   (unless it's a HISTORY object). */

if (object_class != OA_HISTORY) {

  current_node = OaCopyTree( object->odltree, OA_STRIP_COMMENTS);
  OdlPasteObjDesc( current_node, file_object->odltree);

} else {  /* Undo setup done for HISTORY object */
  OaFree( (char *) object->data_ptr);
  object->data_ptr = NULL;
  object->size = 0;
}

/* If there are already ^POINTERs in the file node, find the last one, so
   the new ^POINTER keyword can be pasted after it. */

kwd_ptr = OdlFindKwd( file_object->odltree, "^*", "*", (unsigned long) 0, 
                      (unsigned short) ODL_THIS_OBJECT);
if (kwd_ptr != NULL) {
  while ((next_kwd_ptr = OdlNextKwd( kwd_ptr, "^*", "*", (unsigned long) 2,
                         (unsigned short) ODL_THIS_OBJECT)) != NULL) {
    kwd_ptr = next_kwd_ptr;
  }
}

/* Set the value of the ^POINTER keyword to ("filename",nnn) where nnn is a 
   record number starting at 1, or for STREAM files, ("filename",nnn<BYTES>) 
   where nnn is a byte offset starting at 1 (the stream_id->current_position 
   starts at 0).   If there wasn't a previous ^POINTER found above, then find 
   the last keyword OaOpenOutputFile wrote, so the new ^POINTER keyword can be
   pasted after it. */

sprintf( kwd_name, "^%s", object->odltree->class_name);
switch( file_object->stream_id->record_type) {

  case OA_STREAM:
  case OA_UNDEFINED_RECORD_TYPE:
    sprintf( kwd_value, "(\"%s\",%ld<BYTES>)", 
             file_object->stream_id->filename, object_start_position + 1);
    if (kwd_ptr == NULL)
      str = "RECORD_TYPE";
  break;

  case OA_FIXED_LENGTH:
    sprintf( kwd_value, "(\"%s\",%ld)", file_object->stream_id->filename,
             object_start_position/file_object->stream_id->record_bytes + 1);
    if (kwd_ptr == NULL)
      str = "FILE_RECORDS";
  break;

  case OA_VARIABLE_LENGTH:
    sprintf( kwd_value, "(\"%s\",%ld)", file_object->stream_id->filename, 
             object_start_position + 1);
    if (kwd_ptr == NULL)
      str = "FILE_RECORDS";
  break;
}
if (kwd_ptr == NULL) {
  kwd_ptr = OdlFindKwd( file_object->odltree, str, "*", (unsigned long) 0,
                        (unsigned short) ODL_THIS_OBJECT);
}

/* If haven't found a keyword to put the ^POINTER keyword after, then something
   is wrong, keywords are missing.  */

if (kwd_ptr == NULL) {
  sprintf( error_string, "%s: file node is missing keywords.",
           proc_name);
  oa_errno = 530;
  OaReportError( error_string);
  return(-1);
}

/* Make a new keyword for the ^POINTER, and paste it in the file node. */

next_kwd_ptr = OdlNewKwd( kwd_name, kwd_value, NULL, NULL, NULL, (long) 0);
OdlPasteKwdAfter( next_kwd_ptr, kwd_ptr);
return(0);
}



