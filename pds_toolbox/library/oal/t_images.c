/* This program reads the IMAGE and IMAGE_HISTOGRAM objects from the attached
   label file V1.LBL, calculates the histogram of the image, and compares it
   with the IMAGE_HISTOGRAM read from the file - the two should be identical.
   The file V1.LBL must be present in the default directory the program is
   run from.

   Platforms this has been tested on are:
   1) SGI/Irix
   2) VAX/VMS
   3) Sun Sparc/SunOS and Solaris
   4) Dec Alpha/OSF/1
   5) Macintosh
   6) Dec 3100/Ultrix
   7) Dec Alpha/OpenVMS
   8) IBM PC/DOS
*/

#include "oal.h"
#include <string.h>
#include <stdio.h>

main() {

#ifdef IBM_PC
typedef long int4;
#else
typedef int int4;
#endif

OA_OBJECT oa_object, image_object1, image_object2, histogram_object;
OA_OBJECT image_handle;
ODLTREE root_node, odltreenode, image_node, histogram_node;
KEYWORD *kwdptr;
char *label_filename, *errfilespec;
char fmt[80];
PTR image_values;              /* For compatibility with the IBM-PC, use the
                                  PTR typedef whenever accessing data greater
                                  than 64K in size, so the compiler will use
                                  "huge" pointers.  */
int4 histogram_values[256];
int4 *image_histogram_values;  /* image_histogram_values points to an object
                                  1024 bytes in size, so don't need to use the
                                  PTR typedef.  */
int i, j, lines_per_call, err;
long line, sample;
unsigned char u;
long lines;

/* Read in the label from V1.LBL.  This is an attatched label with variable-
   length records, containing a HFD compressed Voyager image of miranda,
   an engineering table and two histograms. */

OaReportError( "Reading V1.LBL...");
label_filename = "V1.LBL";
if ((root_node = OaParseLabelFile( label_filename, NULL, ODL_EXPAND_STRUCTURE, 
                                   TRUE)) == NULL) {
  OaReportError( "Error from OaParseLabelFile!");
  return(0);
}

/* Find the IMAGE node.  */

if ((image_node = OdlFindObjDesc( root_node, "IMAGE", NULL, NULL, 0,
                                  ODL_RECURSIVE_DOWN)) == NULL) {
  OaReportError( "Couldn't find IMAGE node!");
  return(0);
}

/* Calculate the histogram from the image; use OaReadPartialImage to
   read in the image piece by piece, lines_per_call lines at a time.  */

for (i=0; i<256; i++)
  histogram_values[i] = 0;


if ((image_handle = OaOpenImage( image_node, 1)) == NULL) {
  OaReportError( "Error from OaOpenImage!");
  return(0);
}

lines_per_call = 200;

sprintf( error_string, 
         "Calculating histogram; %d calls to OaReadPartialImage...",
         800/lines_per_call);
oa_errno = 950;  /* Error code for informational messages.  */
OaReportError( error_string);

for (i=0; i<(800/lines_per_call); i++) {
  OaReportError( "Calling OaReadPartialImage...");
  if ((image_object1 = OaReadPartialImage( image_handle, 
                       (long) (i*lines_per_call+1),  
                       (long) (i*lines_per_call+lines_per_call),
                       (long) 1, (long) 800)) == NULL) {
    OaReportError( "Error from OaReadPartialImage!");
    return(0);
  }
  image_values = image_object1->data_ptr;

  for (line=0; line<lines_per_call; line++) {
    for (sample=0; sample<800; sample++) {

      /* WARNING! When accessing the values of a large block of memory as
         done below, be sure to use large enough data types to index it:
         the expression "line*800 + sample" will overflow and wrap around if
         line and sample are declared as integers, and the expression
         evaluates to greater than the size of an integer.  This happened on
         the IBM-PC with Borland C++ when line and sample were declared as 
         integers (2-bytes in size, 32K max value); declaring them as long's
         fixed it!  */

      u = (unsigned char) image_values[ line*800 + sample];
      histogram_values[u] += 1;
    }
  }
  OaDeleteObject( image_object1);
}
OaCloseImage( image_handle);


/* Find the IMAGE_HISTOGRAM node.  */

if ((histogram_node = OdlFindObjDesc( root_node, "IMAGE_HISTOGRAM", 
                                      NULL, NULL, 0, ODL_RECURSIVE_DOWN)) 
                                      == NULL) {
  OaReportError( "Couldn't find IMAGE_HISTOGRAM node!");
  return(0);
}

/* Read the IMAGE_HISTOGRAM into memory.  */

if ((histogram_object = OaReadHistogram( histogram_node)) == NULL) {
  OaReportError( "Error from OaReadHistogram!");
  return(0);
}

/* Strip the HISTOGRAM object of everything but the data.  */

image_histogram_values = (int4 *) OaExportObject( histogram_object);

/* Print out a few values to see if it worked.  */

oa_errno = 950;  /* Error code for informational messages.  */
OaReportError( "First 5 values should be:  529   182   139   284   315");
#ifdef IBM_PC
strcpy( fmt, "First 5 values are:       %ld  %ld  %ld  %ld  %ld\n");
#else
strcpy( fmt, "First 5 values are:        %d   %d   %d   %d   %d\n");
#endif
sprintf( error_string, fmt, image_histogram_values[0], 
         image_histogram_values[1],  image_histogram_values[2], 
         image_histogram_values[3],  image_histogram_values[4]);
OaReportError( error_string);

/* Compare the calculated histogram with the histogram read from the file.  */

for (i=0; i<256; i++)
  if (histogram_values[i] != image_histogram_values[i]) {
    sprintf( error_string, "Error: histograms do not match; i = %d\n", i);
    sprintf( error_string + strlen( error_string), fmt,
             histogram_values[0], histogram_values[1], histogram_values[2],
             histogram_values[3], histogram_values[4]);
    oa_errno = 950;  /* Error code for informational messages.  */
    OaReportError( error_string);
    return(1);
  }
oa_errno = 950;  /* Error code for informational messages.  */
OaReportError( "histograms match");

OaFree( (char *) image_histogram_values);

return(0);
}



