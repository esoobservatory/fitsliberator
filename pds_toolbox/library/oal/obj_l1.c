/*****************************************************************************

  File:  obj_l1.c

  Description: This file contains about half of the C routines making up the 
               Object Layer of the PDS Object Access Library.  The other
               half are in obj_l2.c.  The routines in this file are:

               OaAddContainerAroundTable
               OaAddLineTerminatorstoTable
               OaCheckODLTree
               OaCloseImage
               OaCloseOutputFile
               OaConvertImagetoArray
               OaConvertObject
               OaConvertObjecttoOneType
               OaConvertLabel
               OaCopyObject
               OaCreateAttachedLabel
               OaDeleteColumn
               OaDeleteObject
               OaDeleteRow
               OaExportObject
               OaGetSubCollection
               OaGetSubTable
               OaGetFileKeywords
               OaGetPartialImage
               OaImportColumn
               OaImportHistogram
               OaImportImage
               OaJoinTables
               OaNewOaObject

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:  15 Nov   1996

  History:

    Creation - Most of these routines were part of the Alpha Release of 
               the OA library.  Several new routines were added for the Beta
               Release.
    12/06/95 - Moved OaMalloc and OaFree to oamalloc.c.  SM
    12/06/95 - Replaced malloc() by OaMalloc() throughout.  SM
    12/11/95 - Added error codes.  SM
    02/29/96 - Added OaImportHistogram. SM
    03/05/96 - Added OaGetSubCollection. SM
    04/25/96 - Added OaConvertObjecttoOneType.  SM
    11/15/96 - Added OaCreateAttachedLabel.  SM

*****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "oal.h"


/*****************************************************************************

  Routine: OaAddContainerAroundTable

  Description:  This routine effects only the object's ODL tree; the object
                data is not changed.  The routine takes all the columns and
                containers in a table, and adds a new CONTAINER object
                enclosing them.  The container's REPETITIONS replace the ROWS
                in the original table, so the table's ROWS keyword is set to 1,
                and ROW_BYTES is set to the entire table size.
                The result is a TABLE node which encloses a single CONTAINER
                node.  Below the CONTAINER node are all the nodes which were
                originally directly below the TABLE node.  This routine is
                useful in conjunction with OaJoinTables when building
                up a table in memory which has CONTAINER sub-object(s).

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  14 Feb   1995
  Last Modified:  14 Feb   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:  
         table_object  - A pointer to an Oa_Object structure with an in-memory
                         TABLE-like object.  The table must be ROW_MAJOR.

  Output:  If successful, the function returns a pointer to the input
           Oa_Object structure, otherwise NULL.  The object's ODL tree has
           been modified as described above.

  Notes:

*****************************************************************************/

#ifdef _NO_PROTO

OA_OBJECT OaAddContainerAroundTable( table_object)
OA_OBJECT table_object;

#else

OA_OBJECT OaAddContainerAroundTable( OA_OBJECT table_object)

#endif
{

static char *proc_name = "OaAddContainerAroundTable";
ODLTREE table_node, container_node, odltreenode, next_node;
long rows, row_bytes, prefix_bytes, suffix_bytes;
int interchange_format, table_storage_type;
char buf[80];

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

if (table_storage_type != OA_ROW_MAJOR) {
  sprintf( error_string, "%s: %s's TABLE_STORAGE_TYPE must be ROW_MAJOR.",
           proc_name, OdlGetObjDescClassName( table_node));
  oa_errno = 520;
  OaReportError( error_string);
  return( NULL);
}

/* Create a new ODL tree node for the CONTAINER, set its keywords.  */

container_node = OdlNewObjDesc( "CONTAINER", NULL, NULL, NULL, NULL, NULL,
                                (short) 0, (long) 0);
OaStrtoKwdValue( "NAME", container_node, "CONTAINER");
OaStrtoKwdValue( "START_BYTE", container_node, "1");
OaLongtoKwdValue( "BYTES", container_node, row_bytes);
OaLongtoKwdValue( "REPETITIONS", container_node, rows);
sprintf( buf, "\"This container was created from %s by %s.\"",
         OdlGetObjDescClassName( table_node), proc_name);
OaStrtoKwdValue( "DESCRIPTION", container_node, buf);

/* Move all the table node's children under the container, then paste the
   container under the table node.  */

odltreenode = LeftmostChild( table_node);
while (odltreenode != NULL) {
  next_node = RightSibling( odltreenode);
  OdlPasteObjDesc( OdlCutObjDesc( odltreenode), container_node);
  odltreenode = next_node;
}
OdlPasteObjDesc( container_node, table_node);

OaLongtoKwdValue( "ROW_BYTES", table_node, rows * row_bytes);
OaStrtoKwdValue( "ROWS", table_node, "1");
OaStrtoKwdValue( "COLUMNS", table_node, "1");

return( table_object);
}


/*****************************************************************************

  Routine: OaAddLineTerminatorstoTable

  Description:  This routine appends a CR/LF to the end of each row of an
                in-memory TABLE, and adds 2 to the ROW_BYTES keyword value.
                It copies the data from table_object->data_ptr, appends the
                CR/LF, then frees the old data.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  21 Nov   1994
  Last Modified:  21 Nov   1994

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:  
         table_object  - A pointer to an Oa_Object structure with an in-memory
                         TABLE-like object in ASCII interchange format.  
                         The table must be ROW_MAJOR.

  Output:  If successful, the function returns a pointer to the input
           Oa_Object structure, otherwise NULL.  table_object->data_ptr points
           to a new memory block containing the old data, plus CR/LFs at the
           end of each row.  The ODL tree's ROW_BYTES keyword is updated.  
           The CR/LF's are NOT described by a COLUMN node; they are implicit
           spares.

  Notes:

  1) PDS recommends the table format for ASCII tables to have FIXED_LENGTH
     RECORD_TYPE, with each record ROW_BYTES in length, and each row terminated
     with a carriage-return/line-feed <CR><LF> pair.  This function facilitates
     adding these row terminators.

*****************************************************************************/

#ifdef _NO_PROTO

OA_OBJECT OaAddLineTerminatorstoTable( table_object)
OA_OBJECT table_object;

#else

OA_OBJECT OaAddLineTerminatorstoTable( OA_OBJECT table_object)

#endif
{

static char *proc_name = "OaAddLineTerminatorstoTable";
OA_OBJECT table_with_terminators;
ODLTREE table_node, save_tree;
long i, rows, row_bytes, prefix_bytes, suffix_bytes;
int table_storage_type, interchange_format;
PTR data_ptr;


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

if (OaGetTableKeywords( table_node, &rows, &row_bytes, &prefix_bytes, 
                        &suffix_bytes, &interchange_format,
                        &table_storage_type) != 0)
  return( NULL);  /* Error message already written and oa_errno set.  */

if (table_storage_type != OA_ROW_MAJOR) {
  sprintf( error_string, "%s: %s's TABLE_STORAGE_TYPE must be ROW_MAJOR.",
           proc_name, OdlGetObjDescClassName( table_node));
  oa_errno = 530;
  OaReportError( error_string);
  return( NULL);
}

/* Allocate space for the CR/LF column, and initialize it.  */

if ((data_ptr = (PTR) OaMalloc( (long) (rows * 2))) == NULL) {
  sprintf( error_string, "%s: OaMalloc failed! Out of memory!", proc_name);
  oa_errno = 720;
  OaReportError( error_string);
  exit(1);
  /*NOTREACHED*/
}
for (i=0; i<rows*2; i+=2) {
  data_ptr[i]   = '\r';
  data_ptr[i+1] = '\n';
}

/* Package the CR/LF column into an OA_OBJECT.  */

if ((table_with_terminators = OaImportColumn( data_ptr, rows, (long) 1, 
                                              (long) 2,
                                              "CHARACTER",  
                                              OA_ASCII_INTERCHANGE_FORMAT, 
                                              "N/A")) == NULL) {
  sprintf( error_string, "%s: OaImportColumn returned NULL.", proc_name);
  OaReportError( error_string);
  return( NULL);
}

/* Use a copy of the input ODL tree, because OaJoinTables will delete the
   input tree and return a new tree;  the caller may have saved pointers
   pointing into the old tree, so it should be returned unchanged.  */

save_tree = table_object->odltree;
table_object->odltree = OaCopyTree( table_node, 0);

/* Call OaJoinTables to append the CR/LF table onto the end of the input
   table.  */

if (OaJoinTables( table_object, table_with_terminators,
                  OA_ADD_COLUMNS) == NULL) {
  sprintf( error_string, "%s: OaJoinTables returned NULL.", proc_name);
  OaReportError( error_string);
  return( NULL);
}

/* Delete the CR/LF table object.  */

OaDeleteObject( table_with_terminators);

/* Restore the original ODL tree and adjust the ROW_BYTES keyword.  */

OdlFreeTree( table_object->odltree);
table_object->odltree = save_tree;
OaLongtoKwdValue( "ROW_BYTES", table_object->odltree, row_bytes+2);

return( table_object);
}



/*****************************************************************************

  Routine:  OaCheckODLTree

  Description:  OaCheckODLTree currently only does a few keyword alias 
                conversions and gets rid of nodes under an IMAGE node.
                In the future, it will constraint check the input ODL tree
                and reject it if there are any constructs or conditions which
                OAL cannot deal with.  Currently these checks are done when
                building an SDT.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:   1 Sept  1994

  History:

    Creation - This routine was part of the Alpha Release of the OA library.

  Input:  
         TLO_node - A pointer to a top-level object node (TABLE, IMAGE etc.)
         in an ODL tree.  This may later be changed to be the root node,
         and then OaCheckODLTree will do the whole tree, instead of just one
         object in the tree.

  Output:  The ODL tree has a few aliases dealiased.  
           The function returns 0 if successful.

  Notes:  
  1) OaCheckODLTree is for checking, not converting.  The function
     OaConvertLabel should be used to convert the ODL tree to Version 3
     before calling any Object Layer functions.
  2) OaCheckODLTree currently does the following keyword manipulations and
     dealiasing:
     HISTOGRAM:
       a) Replace ITEM_TYPE keyword name with DATA_TYPE.
       b) Replace ITEM_BITS keyword name with ITEM_BYTES, and divide the value
          by 8.
     TABLE, SPECTRUM, SERIES, PALETTE, GAZETTEER:
       a) Replace BYTES keyword name with ROW_BYTES.
       b) Loop through all COLUMNS and check for ITEMS keyword; if present,
          BYTES > ITEMS, then multiply BYTES by ITEMS.  Specifying BYTES as
          the number of bytes in a single item, and leaving out ITEM_BYTES is
          a Version 2 construct which has crept into some Version 3 labels.  
          Version 3 specifies that BYTES is the total number of bytes in the
          column, including all the items,  i.e. BYTES = ITEMS * ITEM_BYTES.
          (Complications with ITEM_OFFSET are avoided by only doing this if
          ITEM_BYTES and ITEM_OFFSET are missing.)
     IMAGE:
       a) If the SAMPLE_BITS keyword is absent, then it is added with a value
          of 8.
       b) If the image has children nodes they are stripped out.
          This is the case with Voyager CD-ROM images and others which have
          the line suffix structure included via a ^STRUCTURE statement inside
          the image node.

*****************************************************************************/

#ifdef _NO_PROTO

int OaCheckODLTree( TLO_node)
ODLTREE TLO_node;

#else

int OaCheckODLTree( ODLTREE TLO_node)

#endif
{
int TLO_object_class = OA_UNKNOWN_CLASS;
ODLTREE odltreenode, current_node;
KEYWORD *kwdptr;
long items, bytes, l;

TLO_object_class = OaGetObjectClass( TLO_node);
switch( TLO_object_class) { 
  case OA_TABLE:
  case OA_SPECTRUM:
  case OA_SERIES:
  case OA_PALETTE:
  case OA_GAZETTEER:

    kwdptr = OdlFindKwd( TLO_node, "BYTES", "*", (unsigned long) 0, 
                         (unsigned short) ODL_THIS_OBJECT);
    if (kwdptr != NULL) {
      LemmeGo( kwdptr->name);
      CopyString( kwdptr->name, "ROW_BYTES");
    }

    /* Do a pre-order traverse of the entire tree below the TLO_node.  */
   
    current_node = LeftmostChild( TLO_node);
    while (current_node != NULL) {

      if (OaGetObjectClass( current_node) == OA_COLUMN) {

       /* If the ITEMS keyword is present, and its value is greater than the
          BYTES value, and neither ITEM_BYTES nor ITEM_OFFSET are present, 
          then change the BYTES value to ITEM_BYTES.  */

        if (OaKwdValuetoLong( "ITEMS", current_node, &items) == 0) {
          if ((OaKwdValuetoLong( "ITEM_BYTES", current_node, &l) != 0) &&
              (OaKwdValuetoLong( "ITEM_OFFSET", current_node, &l) != 0)) {
            if (OaKwdValuetoLong( "BYTES", current_node, &bytes) == 0) {
              if (items > bytes) {
                OaLongtoKwdValue( "ITEM_BYTES", current_node, bytes);
                OaLongtoKwdValue( "BYTES", current_node, bytes * items);
	      }
            }
          }
        }
      }

      /* Position current_node to the next node in a pre-order traverse order:
         if current_node has children, then the leftmost child is the new
         current_node;  otherwise, if current_node has a right sibling, the
         the right sibling is the new current_node;  otherwise search upwards 
         in the tree until get to a node which does have a right sibling, or
         get to the TLO_node.  */

      if (LeftmostChild( current_node) != NULL)
        current_node = LeftmostChild( current_node);
      else {
        while (current_node != NULL) {
          if (RightSibling( current_node) != NULL) {
            current_node = RightSibling( current_node);
            break;
          }
          current_node = Parent( current_node);
          if (current_node == TLO_node)
            current_node = NULL;
        }
      }
    }  /* end while current_node != NULL  */

  break;

  case OA_IMAGE:

    /* Remove all children under the image node (probably prefix or suffix
       structure nodes from a ^STRUCTURE inclusion).  Nodes nested inside
       images conflict with the PDS Standards Document's requirement for
       IMAGES not to have any sub-objects, but they can't be moved out as
       siblings of the IMAGE node in their own TABLE if the image is
       compressed, since TABLES don't have the ENCODING_TYPE keyword.  
       This can of worms needs to be cleaned up by PDS before general purpose
       code can be written.  Partly because of this, Object Layer routines
       don't read IMAGE prefixes or suffixes.  */

    while ((odltreenode = LeftmostChild( TLO_node)) != NULL)
      OdlFreeTree( OdlCutObjDesc( odltreenode));

    /* If SAMPLE_BITS keyword is absent, add it with default value of "8". */

    if (OdlFindKwd( TLO_node, "SAMPLE_BITS", "*", (unsigned long) 0, 
                   (unsigned short) ODL_THIS_OBJECT) == NULL)
      OaStrtoKwdValue( "SAMPLE_BITS", TLO_node, "8");
  break;

  case OA_HISTOGRAM:

    if ((kwdptr = OdlFindKwd( TLO_node, "ITEM_TYPE", "*", (unsigned long) 0, 
                              (unsigned short) ODL_THIS_OBJECT)) != NULL) {
      LemmeGo( kwdptr->name);
      CopyString( kwdptr->name, "DATA_TYPE");
    }

    if ((kwdptr = OdlFindKwd( TLO_node, "ITEM_BITS", "*", (unsigned long) 0, 
                              (unsigned short) ODL_THIS_OBJECT)) != NULL) {
      LemmeGo( kwdptr->name);
      CopyString( kwdptr->name, "ITEM_BYTES");
      OaKwdValuetoLong( "ITEM_BYTES", TLO_node, &l);
      l = l/8;
      OaLongtoKwdValue( "ITEM_BYTES", TLO_node, l);
    }
  break;

  default: ;
}
return(0);
}



/*****************************************************************************

  Routine:  OaCloseImage

  Description:  OaCloseImage closes the data file opened by OaOpenImage and
                frees all components of the image_handle_object.  This should
                be called after the last call to OaReadImagePixels or
                OaReadPartialImage.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  29 Sept  1994
  Last Modified:  29 Sept  1994

  History:

    Creation - This routine was part of the Beta Release of the OA
               library.

  Input:  
         image_handle_object - Pointer to an OA_Object structure created by
                               OaOpenImage.

  Output:  The routine always returns 0.

  Notes:  

*****************************************************************************/

#ifdef _NO_PROTO

int OaCloseImage( image_handle_object)
OA_OBJECT image_handle_object;

#else

int OaCloseImage( OA_OBJECT image_handle_object)

#endif
{
struct oa_image_handle *image_handle;

if (image_handle_object == NULL) return(0);
if (image_handle_object->odltree != NULL) 
  OdlFreeTree( image_handle_object->odltree);
if (image_handle_object->appl1 != NULL) {
  image_handle = (struct oa_image_handle *) image_handle_object->appl1;
  if (image_handle->compression_type == OA_HUFFMAN_FIRST_DIFFERENCE) {
    LemmeGo( image_handle->decomp.HFD.line_buffer);
    OalFreeHFDTree( image_handle->decomp.HFD.decoding_tree);
  }
  LemmeGo( image_handle->buf);
  LemmeGo( image_handle);
}
if (image_handle_object->stream_id != NULL)
  OalCloseStream( image_handle_object->stream_id);
LemmeGo( image_handle_object);
return(0);
}



/*****************************************************************************

  Routine:  OaCloseOutputFile

  Description:  This routine calls OalCloseStream to close the data file, and
                OdlPrintLabel to write the ODL tree in the file oa_object to a
                label file.  It is the last call in a sequence of calls to
                write objects to a file:  OaOpenOutputFile, OaWriteObject
                (multiple calls), OaCloseOutputFile.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:   1 Sept  1994

  History:

    Creation - This routine was part of the Alpha Release of the OA library.

  Input:
         file_object    - An oa_object which was created by OaOpenOutputFile
                          and has been written to by one or more calls to
                          OaWriteObject or other object-writing functions.

         label_filename - A file pathname suitable for direct use by fopen().
                          It must be valid on the platform being run on, and
                          the process OA is running in must have write
                          permission to it, and enough quota and/or disk space.
         
  Output:  The file object has been stripped of its stream descriptor, but is
           otherwise the same as on entry.  The ODL tree in the file object
           has been written to a detatched label file.  The data file is
           closed.  The function returns 0 if successful, non-zero otherwise.

  Notes:  

*****************************************************************************/

#ifdef _NO_PROTO

int OaCloseOutputFile( file_object, label_filename)
OA_OBJECT file_object;
char      *label_filename;

#else

int OaCloseOutputFile( OA_OBJECT file_object, char *label_filename)

#endif
{

static char *proc_name = "OaCloseOutputFile";
ODLTREE next_node;
FILE *fd;
long file_records;

/* Check inputs for validity. */

if (label_filename == NULL) {
  sprintf( error_string, "%s: label_filename is NULL.", proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return(-1);
}

if (file_object == NULL) {
  sprintf( error_string, "%s: input oa_object is NULL.", proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return(-1);
}

if (file_object->odltree == NULL) {
  sprintf( error_string, "%s: oa_object's ODL tree is NULL.", proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return(-1);
}

/* If fixed-length or variable-length, set FILE_RECORDS.  */

switch( file_object->stream_id->record_type) {
  case OA_FIXED_LENGTH:
    file_records = file_object->stream_id->current_position /
                   file_object->stream_id->record_bytes;
    OaLongtoKwdValue( "FILE_RECORDS", file_object->odltree, file_records);
  break;
  case OA_VARIABLE_LENGTH:
    file_records = file_object->stream_id->current_position;
    OaLongtoKwdValue( "FILE_RECORDS", file_object->odltree, file_records);
  break;
}

/* Close the data file and free the stream descriptor. */

if (file_object->stream_id != NULL) {
  OalCloseStream( file_object->stream_id);
  file_object->stream_id = NULL;
}

/* Open the label file. */

if ((fd = fopen( label_filename, "w")) == NULL) {
  sprintf( error_string, "%s: couldn't open %s.", proc_name, label_filename);
  oa_errno = 700;
  OaReportError( error_string);
  return(-1);
}

/* Traverse the whole tree, setting the file_name pointer in each node to be
   label_filename (or a ^STRUCTURE include file, future version). */

next_node = file_object->odltree;
while (next_node != NULL) {
  LemmeGo( next_node->file_name);
  CopyString( next_node->file_name, label_filename);
  next_node = OdlTraverseTree( next_node, (unsigned long) 0);
}

/* Write the label to the label_filename file.  */

OdlPrintLabel( file_object->odltree, NULL, fd, (unsigned long) 0, (MASK) 0);
fclose( fd);

return(0);
}



/*****************************************************************************

  Routine:  OaConvertImagetoArray

  Description:  This routine converts an IMAGE object into a 2-dimensional
                ARRAY object.  It does this by transforming the object's ODL
                tree; it doesn't change the object's data.  
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:   1 Sept  1994

  History:

    Creation - This routine was part of the Alpha Release of the OA library.

  Input:  
          simple_image - simple_image->odltree describes an IMAGE, and it
                         must be a SIMPLE_IMAGE - no prefix or suffix bytes, 
                         not compressed, not multi-banded.
         
  Output:  If successful, the input OA_Object pointer is returned; otherwise 
           NULL is returned and the input OA_Object is unchanged. 
           The object's ODL tree now describes a 2-dimensional array, 
           with the image pixel as the ELEMENT node under the ARRAY node.

  Notes:  If any of the following aren't satisfied, the function will return
          NULL and report an error message:
          1) If the ENCODING_TYPE keyword exists, then its value must be "N/A".
          2) If the BANDS keyword exists, then its value must be 1.
          3) If the LINE_PREFIX_BYTES or LINE_SUFFIX_BYTES keywords exist, 
             their values must be 0.
          4) The input IMAGE node must be a stand-alone node without parent or 
             sub-objects.

*****************************************************************************/

#ifdef _NO_PROTO

OA_OBJECT OaConvertImagetoArray( simple_image)
OA_OBJECT simple_image;

#else

OA_OBJECT OaConvertImagetoArray( OA_OBJECT simple_image)

#endif
{

static char *proc_name = "OaConvertImagetoArray";
ODLTREE image_node, array_node, element_node;
KEYWORD *kwdptr;
long lines, line_samples, sample_bits, bands;
long line_prefix_bytes, line_suffix_bytes;
char *str, buf[80];
int encoding_type, band_storage_type;

/* Check inputs for validity. */

if (simple_image == NULL) {
  sprintf( error_string, "%s: input OA_Object is NULL.", proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return(NULL);
}

if ((image_node = simple_image->odltree) == NULL) {
  sprintf( error_string, "%s: input OA_Object's odltree is NULL.", proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return(NULL);
}

if ((Parent( image_node) != NULL) || (LeftmostChild( image_node) != NULL)) {
  sprintf( error_string, 
           "%s: input OA_Object's odltree must be a stand-alone IMAGE ",
           proc_name);
  strcat( error_string, "without parent or sub-objects.");
  oa_errno = 530;
  OaReportError( error_string);
  return(NULL);
}

if (OaGetImageKeywords( image_node, &lines, &line_samples, &sample_bits,
                        &str, &bands, &band_storage_type, &line_prefix_bytes,
                        &line_suffix_bytes, &encoding_type) != 0)
  return(NULL);

/* Reject cases not currently supported (see notes above). */

if (bands > 1) {
  sprintf( error_string, "%s: multi-banded images not supported.", 
           proc_name);
  oa_errno = 520;
  OaReportError( error_string);
  return(NULL);
}
if (encoding_type != OA_UNCOMPRESSED) {
  sprintf( error_string,
           "%s: can't convert a compressed IMAGE.", proc_name);
  oa_errno = 520;
  OaReportError( error_string);
  return(NULL);
}
if ((line_prefix_bytes != 0) || (line_suffix_bytes != 0)) {
  sprintf( error_string, 
           "%s: LINE_PREFIX_BYTES and LINE_SUFFIX_BYTES not supported.",
           proc_name);
  oa_errno = 520;
  OaReportError( error_string);
  return(NULL);
}
if ((sample_bits % 8) != 0) {
  sprintf( error_string,
           "%s: SAMPLE_BITS must be a multiple of 8.", proc_name);
  oa_errno = 502;
  OaReportError( error_string);
  return(NULL);
}

/* Create a new ODL tree node for the ELEMENT and initialize its required
   keywords.  */

element_node = OdlNewObjDesc( "ELEMENT", NULL, NULL, NULL, NULL, NULL,
                              (short) 0, (long) 0);
OaLongtoKwdValue( "BYTES", element_node, sample_bits/8);
OaStrtoKwdValue( "DATA_TYPE", element_node, str);
OaStrtoKwdValue( "NAME", element_node, "IMAGE_PIXEL");

/* Create a new ODL tree node for the ARRAY and initialize its required
   keywords.  */

array_node = OdlNewObjDesc( "ARRAY", NULL, NULL, NULL, NULL, NULL,
                            (short) 0, (long) 0);
OaLongtoKwdValue( "AXES", array_node, (long) 2);
sprintf( buf, "(%ld,%ld)", lines, line_samples);
OaStrtoKwdValue( "AXIS_ITEMS", array_node, buf);
OaStrtoKwdValue( "NAME", array_node, image_node->class_name);

/* Now copy some optional keywords which IMAGEs have in common with ARRAYs
   and ELEMENTs. */

if ((kwdptr = OdlFindKwd( image_node, "CHECKSUM", "*", (unsigned long) 0, 
                          (unsigned short) ODL_THIS_OBJECT)) != NULL)
  OdlPasteKwd( OdlCutKwd( kwdptr), array_node);
if ((kwdptr = OdlFindKwd( image_node, "DESCRIPTION", "*", (unsigned long) 0, 
                          (unsigned short) ODL_THIS_OBJECT)) != NULL)
  OdlPasteKwd( OdlCutKwd( kwdptr), array_node);
if ((kwdptr = OdlFindKwd( image_node, "SAMPLE_BIT_MASK", "*", 
                          (unsigned long) 0,
                          (unsigned short) ODL_THIS_OBJECT)) != NULL)
  OaStrtoKwdValue( "BIT_MASK", element_node, kwdptr->value);

/* Free the image node. */

OdlFreeTree( image_node);

/* Attach the ELEMENT node under the ARRAY node, make the ARRAY node the
   odltree of the input OA_Object, and return.  */

OdlPasteObjDesc( element_node, array_node);
simple_image->odltree = array_node;

return( simple_image);
}



/*****************************************************************************

  Routine:  OaConvertObject

  Description:  OaConvertObject converts the data for an in-memory object to
                a (usually) different interchange format, alignment type or 
                binary data types, using the current Oa_profile settings.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:  21 Nov   1996

  History:

    Creation - This routine was part of the Alpha Release of the OA library.
    11/21/96 - Added code to initialize output object data. SM

  Input:  
          object - An Oa_Object with data in memory, and an ODL tree
                   attached; the structure fields in global variable 
                   Oa_profile are set to the values desired for the
                   conversion.

  Output:  If successful, the routine returns a pointer to a new OA_Object 
           structure, which contains an ODL tree describing the converted data
           and a pointer to the converted data.  The input object is
           unchanged.  If unsuccessful, a NULL pointer is returned.  

  Notes:  
  1) Example call to convert a binary object to ASCII:

     Oa_profile.dst_format_for_binary_src = OA_ASCII_INTERCHANGE_FORMAT;
     if (OaConvertObject( my_object) == NULL) 
       OaReportError( "OaConvertObject failed!");

*****************************************************************************/

#ifdef _NO_PROTO

OA_OBJECT OaConvertObject( object)
OA_OBJECT object;

#else

OA_OBJECT OaConvertObject( OA_OBJECT object)

#endif
{

static char *proc_name = "OaConvertObject";
char c;
int src_interchange_format, dst_interchange_format;
OA_OBJECT output_oa_object;
ODLTREE odltree, sdt, compressed_SDT, current_node;
SDTNODE TLO_sdt_node_ptr;
PTR data_ptr;
long object_size;
#ifdef IBM_PC
long i;
#endif

/* Check input parameters. */

if (object == NULL) {
  sprintf( error_string, "%s: input Oa_Object is NULL.", proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return( NULL);
}

if (object->odltree == NULL) {
  sprintf( error_string, "%s: input Oa_Object's odltree is NULL.", proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return( NULL);
}

if ((object->data_ptr == NULL) || (object->size < 1)) {
  sprintf( error_string, 
           "%s: input Oa_Object's data_ptr is NULL or size is 0.",
           proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return( NULL);
}

src_interchange_format = OaGetObjectInterchangeFormat( object->odltree);
switch( src_interchange_format) {
  case OA_BINARY_INTERCHANGE_FORMAT:
    dst_interchange_format = Oa_profile.dst_format_for_binary_src;
  break;
  case OA_ASCII_INTERCHANGE_FORMAT:
    dst_interchange_format = Oa_profile.dst_format_for_ASCII_src;
  break;
  default:
  return( NULL);  /* Error message has already been issued, oa_errno set. */
}

/* The input tree ODL tree shouldn't ever be modified, so make a copy of all
   the nodes below and including the input node.  */

odltree = OaCopyTree( object->odltree, OA_STRIP_COMMENTS |  
                                       OA_STRIP_SDT_NODES);

/* Call OalCreateSDT to create the SDT (Stream Decomposition Tree).  */

if ((sdt = OalCreateSDT( odltree, src_interchange_format)) == NULL) {
  sprintf( error_string, "%s: CreateSDT returned error.", 
           proc_name);
  OaReportError( error_string);
  OalFreeSDT( odltree);
  return(NULL);
}

/* Allocate memory for the converted object, and store the pointer in the root
   node of the SDT.  */

TLO_sdt_node_ptr = (SDT_node *) sdt->appl1;
object_size = TLO_sdt_node_ptr->dst.size * 
              TLO_sdt_node_ptr->total_repetitions;
if ((TLO_sdt_node_ptr->dst.ptr = (PTR) OaMalloc( (long) object_size)) == NULL){
  sprintf( error_string, "%s: OaMalloc failed! Out of memory!", proc_name);
  oa_errno = 720;
  OaReportError( error_string);
  exit(1);
  /*NOTREACHED*/
}
data_ptr = TLO_sdt_node_ptr->dst.ptr;
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

output_oa_object = OaNewOaObject();
output_oa_object->odltree = sdt;
output_oa_object->data_ptr = TLO_sdt_node_ptr->dst.ptr;
output_oa_object->size = object_size;
output_oa_object->is_in_memory = TRUE;
output_oa_object->stream_id = NULL;
output_oa_object->profile = Oa_profile;

/* Compress the SDT, and initialize current_node to point to the first
   end-node (data node) in the compressed SDT; initialize the node's dst.ptr
   with the pointer returned by OaMalloc.  */

compressed_SDT = OalCompressSDT( sdt);

current_node = OalInitializeSDT( compressed_SDT, TLO_sdt_node_ptr->dst.ptr);

if (OalProcessSDT( object->data_ptr, object->size, &current_node) != 
                   OA_REACHED_END_OF_SDT) {
  sprintf( error_string, "%s: OalProcessSDT returned error.", proc_name);
  OaReportError( error_string);
  OalFreeSDT( compressed_SDT);
  OalFreeSDT( sdt);
  return(NULL);
}

/* Free the compressed SDT. */

OalFreeSDT( compressed_SDT);

/* Call OalSDTtoODLTree to modify the new ODL tree to reflect the in-memory 
   data.  This changes the DATA_TYPE keywords to reflect the in-memory data
   values which may have been converted.  It also strips off ODL tree nodes 
   which have SDT_node->dst.size=0 (alignment SPARE's).  */

OalSDTtoODLTree( sdt, dst_interchange_format);

return( output_oa_object);
}



/*****************************************************************************

  Routine:  OaConvertObjecttoOneType

  Description:  OaConvertObjecttoOneType converts all the data of an
                in-memory OA_OBJECT to a single, specified numeric data type.
                The object's data must consist exclusively of numeric values of
                the same type, e.g. a single COLUMN of a TABLE containing
                binary or ASCII numeric values, or an IMAGE. 
                Heterogenous objects, such as TABLES with multiple COLUMNS
                or with CONTAINER's are not supported.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  25 Apr   1996
  Last Modified:  25 Apr   1996

  History:

    Creation - This routine was part of the Version 1.1 Release of the OA
               library.

  Input:  
          object    - Pointer to an object descriptor for the object to be
                      translated, with data in memory and an ODL tree
                      attached describing the data.

          data_type - Pointer to a string containing the name of the C type
                      or PDS data type into which the data is to be
                      translated.  If a PDS data type, data_type can be any
                      of the standard values currently allowed for DATA_TYPE
                      in the PDSDD for numeric types;  bytes must also be set.
                      If a C type, the allowed values are (case sensitive):
                      "char", "unsigned char", "short", "unsigned short",
                      "int", "unsigned int", "long", "unsigned long",
                      "float", "double";  bytes is ignored.

          bytes     - The number of bytes per data element for the
                      translated data.  Ignored if data_type is a C type.

          rescale   - TRUE (nonzero) to apply the SCALING_FACTOR and OFFSET
                      parameters to the data returned if their keywords are
                      present in the oa_object's ODL tree.  FALSE (zero) to
                      ignore SCALING_FACTOR and OFFSET.  The calculation done
                      is:  rescaled_data = (SCALING_FACTOR * data)  +  OFFSET

  Output:  If successful, the routine returns a pointer to a new OA_Object 
           structure, which contains an ODL tree describing the converted data
           and a pointer to the converted data.  The input object is
           unchanged.  If unsuccessful, a NULL pointer is returned.  
           An error message is issued if there were any integer truncation
           errors, attempts to convert a negative integer to an unsigned 
           integer, or errors converting floats, including loss of precision.

  Notes:  

  1) Example call:

         double *data;
         OA_OBJECT old_column, new_column;
    
         new_column = OaConvertObjecttoOneType( old_column, "double",
                                                0, TRUE);
         data = (double *) OaExportObject( new_column);

     Alternatively, you could accomplish the same thing by specifying the
     PDS data type equivalent to "double" on your platform.  On a Dec/Alpha
     VMS this would be:

         new_column = OaConvertObjecttoOneType( old_column, "VAXG_REAL",  
                                                8, TRUE);  

     If the number of data items in column are even, you could do this, since
     a complex is two reals next to each other:

         new_column = OaConvertObjecttoOneType( old_column, "VAXG_COMPLEX",
                                                16, TRUE);
     
  2) If bytes is zero and data_type is "ASCII_INTEGER", "ASCII_REAL", or
     "ASCII_COMPLEX", then the default field width from the profile will be
     used.  If bytes is non-zero, then it will be used as the field width, 
     overriding the profile.

  3) Specifying data_type = "char" converts to 1-byte integers, not to ASCII.
     Use data_type = "ASCII_INTEGER", "ASCII_REAL" or "ASCII_COMPLEX" to
     convert to ASCII.

  4) If the caller wants rescale, and OFFSET or SCALING_FACTOR were found, then
     the algorithm is:
     a) Convert the data to 'double'.
     b) Apply the OFFSET and/or SCALING_FACTOR.
     c) Convert to the data type the caller specified, via a recursive call to
        this function, specifying no rescale.
     Now determine whether the type the user specified is equivalent to 
     'double', by comparing q_codes.

*****************************************************************************/

#ifdef _NO_PROTO

OA_OBJECT OaConvertObjecttoOneType( object, data_type, bytes, rescale)
OA_OBJECT object;
char *data_type;
int bytes;
int rescale;

#else

OA_OBJECT OaConvertObjecttoOneType( OA_OBJECT object, char *data_type, 
                                    int bytes, int rescale)

#endif
{

static char *proc_name = "OaConvertObjecttoOneType";
OA_OBJECT output_oa_object, output_oa_object2;
ODLTREE current_node, save_node;
int PDS_data_type, dbl_PDS_data_type, size, dbl_size, i, error=FALSE;
int sizeof_type_conversion_profile, profile, user_specified_double = FALSE;
int dst_bytes=0, use_offset_and_scaling_factor = FALSE, dst_interchange_format;
int non_alias_nodes;
unsigned long l;
char *kwds[2];
struct binrep_desc *binrep_descrip, *dbl_binrep_descrip;
struct oa_type_conversion_info *conversion_info;
char *allowed_C_types = "char  unsigned char  short  unsigned short  int  unsigned int  long  unsigned long  float  double";
char dst_data_type[32], *str, *q_code = NULL;
struct oa_profile save_profile;
PTR save_type_conversion_profile;
double dbl, offset = 0.0, scaling_factor = 1.0;
#if (defined( IBM_PC) && defined( MS_DOS))
double huge *dbl_ptr;
#else
double *dbl_ptr;
#endif

kwds[0] = "OFFSET";
kwds[1] = "SCALING_FACTOR";

/* Check input parameters. */

if (object == NULL) {
  sprintf( error_string, "%s: input Oa_Object is NULL.", proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return( NULL);
}

if (object->odltree == NULL) {
  sprintf( error_string, "%s: input Oa_Object's odltree is NULL.", proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return( NULL);
}

if ((object->data_ptr == NULL) || (object->size < 1)) {
  sprintf( error_string, 
           "%s: input Oa_Object's data_ptr is NULL or size is 0.",
           proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return( NULL);
}

if (data_type == NULL) {
  sprintf( error_string, "%s: input data_type is NULL.", proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return( NULL);
}
strcpy( dst_data_type, data_type);

/* Check that the object is 'simple', i.e. a TABLE with a single COLUMN or
   an IMAGE, by checking each node for siblings; if a node has siblings, it
   must be a CONTAINER or COLLECTION node, and the object is not 'simple'. */

current_node = LeftmostChild( object->odltree);
while (current_node != NULL) {
  non_alias_nodes = 0;
  while (current_node != NULL) {
    if (OaGetObjectClass( current_node) != OA_ALIAS) {
      save_node = current_node;
      non_alias_nodes++;
    }
    current_node = RightSibling( current_node);
  }
  if (non_alias_nodes > 1) {
    error = TRUE;
    break;
  }
  if (save_node != NULL) {
    current_node = LeftmostChild( save_node);
    save_node = NULL;
  }
}
if (error == TRUE) {
  sprintf( error_string, "%s: input object must be simple.", proc_name);
  oa_errno = 530;
  OaReportError( error_string);
  return( NULL);
}

/* If rescale is desired, find OFFSET and/or SCALING_FACTOR keywords, and
   check that there is only one occurrence of each.  */

if (rescale != 0) {
  for (i=0; i<2; i++) {
    current_node = OdlFindObjDesc( object->odltree, "*", kwds[i], "*",
                                 (unsigned long) 0, 
                                 (unsigned short) ODL_RECURSIVE_DOWN);
    if (current_node != NULL) {
      use_offset_and_scaling_factor = TRUE;
      if (OdlFindObjDesc( LeftmostChild( current_node), "*", kwds[i], "*",
                          (unsigned long) 0, 
                          (unsigned short) ODL_RECURSIVE_DOWN) != NULL) {
        sprintf( error_string, "%s: found two instances of %s keyword.", 
                 proc_name, kwds[i]);
        oa_errno = 530;
        OaReportError( error_string);
        return( NULL);
      }
      if (OaKwdValuetoDouble( kwds[i], current_node, &dbl) != 0)
        return( NULL);  /* Error already reported. */
      if (i==0)
        offset = dbl;
      else
        scaling_factor = dbl;
    }
  }
}

/* Get the interchange format, PDS data type, size and binrep q-code (if
   applicable) for the destination data type.  */

if (strstr( allowed_C_types, dst_data_type) != NULL) {

  /* Desired destination data type is a native binary C type.  */

  dst_interchange_format = OA_BINARY_INTERCHANGE_FORMAT;
  if (OalGetNativeCTypeInfo( dst_data_type, &PDS_data_type, &size,
                             &binrep_descrip) != 0)
    return( NULL);  /* Error message already issued. */
  dst_bytes = size;
  q_code = binrep_descrip->q_code;

} else {  /* dst_data_type specifies a PDS data type */

  UpperCase( dst_data_type);
  if (strncmp( dst_data_type, "ASCII", 5) == 0)
    dst_interchange_format = OA_ASCII_INTERCHANGE_FORMAT;
  else
    dst_interchange_format = OA_BINARY_INTERCHANGE_FORMAT;
  dst_bytes = bytes;

  /* Pre-process away COMPLEX types by changing to REAL.  */

  if ((str = strstr( dst_data_type, "COMPLEX")) != NULL) {
    *str = '\0';
    strcat( dst_data_type, "REAL");
    dst_bytes /= 2;
  }

  PDS_data_type = OaStrtoPDSDataType( dst_data_type, dst_interchange_format);
  if (PDS_data_type == OA_UNKNOWN_DATA_TYPE) {
    sprintf( error_string, "%s: input data_type is unknown: %s.",
             proc_name, dst_data_type);
    oa_errno = 530;
    OaReportError( error_string);
    return( NULL);
  }

  if (dst_interchange_format == OA_BINARY_INTERCHANGE_FORMAT) {

    /* Check that bytes is a valid size for this PDS_data_type, and get
       its binrep q-code.  */

    conversion_info = OalGetTypeConversionFromProfile( PDS_data_type, 
                                                       dst_bytes);
    if (conversion_info == NULL) {
      sprintf( error_string, 
             "%s: can't translate to data_type: %s, bytes: %d.",
               proc_name, data_type, bytes);
      oa_errno = 530;
      OaReportError( error_string);
      return( NULL);
    }
    if ((q_code = conversion_info->binrep_q_code) == NULL) {
      sprintf( error_string, "%s: input data_type: %s, bytes: %d ",
               proc_name, data_type, bytes);
      strcat( error_string, "has no q-code in Oa_type_conversion_info.");
      oa_errno = 730;
      OaReportError( error_string);
      return( NULL);
    }
  }  /* End if destination interchange format is binary */
}  /* End else input data_type specifies a PDS data type */

/* If we're going to use OFFSET and SCALING_FACTOR, check if the data type the
   caller specified is equivalent to the native binary 'double'.  If not, 
   change it to this so the rescale can be performed on native doubles. 
   After the rescale, we'll change the rescaled doubles to the type the caller
   wants.  */

if (use_offset_and_scaling_factor == TRUE) {
  if (OalGetNativeCTypeInfo( "double", &dbl_PDS_data_type, &dbl_size,
                             &dbl_binrep_descrip) != 0)
    return( NULL);  /* Error message already issued. */
  if (q_code == dbl_binrep_descrip->q_code)
    user_specified_double = TRUE;

  if (user_specified_double == FALSE) {
    dst_interchange_format = OA_BINARY_INTERCHANGE_FORMAT;
    PDS_data_type = dbl_PDS_data_type;
    dst_bytes = dbl_size;
    q_code = dbl_binrep_descrip->q_code;
  }
}

/* Save the global Oa_profile and global Oa_type_conversion_info before
   making changes to them.  */

save_profile = Oa_profile;
sizeof_type_conversion_profile = sizeof( struct oa_type_conversion_info) *
                                 OA_PROFILED_DATA_TYPES;
save_type_conversion_profile = (PTR) OaMalloc( sizeof_type_conversion_profile);
if (save_type_conversion_profile == NULL) {
  sprintf( error_string, "%s: OaMalloc failed! Out of memory!", proc_name);
  oa_errno = 720;
  OaReportError( error_string);
  exit(1);
  /*NOTREACHED*/
}
profile = Oa_profile.data_translation_profile;
memcpy( save_type_conversion_profile, 
        &(Oa_type_conversion_info[ profile][0]),
        sizeof_type_conversion_profile);

/* Change the profile to reflect the desired interchange format. */

Oa_profile.dst_format_for_binary_src = dst_interchange_format;
Oa_profile.dst_format_for_ASCII_src = dst_interchange_format;

/* Change the data types profile so that ALL types will be converted to the
   specified data_type.  */

if (dst_interchange_format == OA_ASCII_INTERCHANGE_FORMAT) {
  for (i=0; i<OA_PROFILED_DATA_TYPES; i++) {
    Oa_type_conversion_info[ profile][i].ASCII_PDS_data_type = PDS_data_type;
    if (dst_bytes > 0) {
      Oa_type_conversion_info[ profile][i].ASCII_size = dst_bytes;
    } else {
      if (PDS_data_type == OA_ASCII_REAL) 
        Oa_type_conversion_info[ profile][i].ASCII_size = 15;
      else
        Oa_type_conversion_info[ profile][i].ASCII_size = 10;
    }
  }

} else {                   /* OA_BINARY_INTERCHANGE_FORMAT */
  for (i=0; i<OA_PROFILED_DATA_TYPES; i++) {
    Oa_type_conversion_info[ profile][i].native_binrep_q_code = q_code;
    Oa_type_conversion_info[ profile][i].native_PDS_data_type = PDS_data_type;
    Oa_type_conversion_info[ profile][i].native_size = dst_bytes;
  }
}

OalResetBinrepErrors();
output_oa_object = OaConvertObject( object);

/* Restore the original profile and data types profile.  */

Oa_profile = save_profile;
memcpy( &(Oa_type_conversion_info[ profile][0]), save_type_conversion_profile, 
        sizeof_type_conversion_profile);
OaFree( (char *) save_type_conversion_profile);

if (output_oa_object == NULL)
  return( NULL);  /* Error message already flagged by OaConvertObject */

/* If rescale was TRUE, the object is now doubles.  Apply OFFSET and
   SCALING_FACTOR.  */

if (use_offset_and_scaling_factor == TRUE) {
#if (defined( IBM_PC) && defined( MS_DOS))
  dbl_ptr = (double huge *) output_oa_object->data_ptr;
#else
  dbl_ptr = (double *) output_oa_object->data_ptr;
#endif
  for (l=0; l<(output_oa_object->size/sizeof( double)); l++) 
    dbl_ptr[l] = dbl_ptr[l] * scaling_factor + offset;

  /* Find OFFSET and SCALING_FACTOR and change them to 0.0 and 1.0
     respectively.  */

  current_node = OdlFindObjDesc( output_oa_object->odltree, "*",
                                 "OFFSET", "*", (unsigned long) 0, 
                                 (unsigned short) ODL_RECURSIVE_DOWN);
  if (current_node != NULL)
    OaStrtoKwdValue( "OFFSET", current_node, "0.0");

  current_node = OdlFindObjDesc( output_oa_object->odltree, "*",
                                 "SCALING_FACTOR", "*", (unsigned long) 0, 
                                 (unsigned short) ODL_RECURSIVE_DOWN);
  if (current_node != NULL)
    OaStrtoKwdValue( "SCALING_FACTOR", current_node, "1.0");

  /* If the caller specified something other than double, convert again via a
     recursive call, this time specifying no rescale.  */

  if (user_specified_double == FALSE) {
    output_oa_object2 = OaConvertObjecttoOneType( output_oa_object, 
                                                  data_type, bytes, 0);
    OaDeleteObject( output_oa_object);
    OalReportBinrepErrors( proc_name);  
    return( output_oa_object2);
  }
}

OalReportBinrepErrors( proc_name);  
return( output_oa_object);
}



/*****************************************************************************

  Routine:  OaConvertLabel

  Description:  This routine converts the input ODL tree from an old version
                of ODL to the latest version.  It handles Versions 0, 1 and 2.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  20 Dec   1994
  Last Modified:  11 Dec   1995

  History:

    Creation - This routine was part of the Beta Release of the OA library.
    12/11/95 - If the SFDU keyword doesn't have a value, the value "SFDU_LABEL"
               is added;  improved logic for adding PDS_VERSION_ID as first
               keyword after SFDU keyword.
    10/15/02 - Added code to move LINE_SUFFIX_BYTES and LINE_PREFIX_BYTES to
			   the new image node, if they are present.   DWS

  Input: 
          odltree  - The root node of an ODL tree.
         
  Output:  If successful, the routine returns the input odltree.  Otherwise
           it returns NULL and reports errors using OaReportError.

  Notes:  
  1) OaConvertLabel is for converting, not checking.  OaCheckODLTree and
     OalCreateSDT check for the presence of all required keywords and 
     subobjects, and dealias keywords.
  2) This currently converts labels to PDS Version 3, as described in the
     Aug 3, 1994 PDS Standards Document.
  3) OaConvertLabel assumes it has a PDS Version 0 tree if there are no
     subobjects under the root node, and the FILE_TYPE keyword is present.
     OaConvertLabel assumes it has a PDS Version 1 or 2 tree if the SFDU
     keyword has a "PDS1" or "PDS2" substring in it, and there's no
     PDS_VERSION_ID keyword.
  4) The algorithm for converting a Version 0 IMAGE is as described in the
     documentation for the PDS utility IMG0TO2.

*****************************************************************************/

#ifdef _NO_PROTO

ODLTREE OaConvertLabel( root_node)
ODLTREE root_node;

#else

ODLTREE OaConvertLabel( ODLTREE root_node)

#endif
{

static char *proc_name = "OaConvertLabel";
ODLTREE odltreenode, tmp_node, table_node, image_node, TLO_node, column_node;
ODLTREE save_node;
KEYWORD *kwdptr, *next_kwdptr;
char *str, buf[32];
char *old_pointer_keyword, *new_pointer_keyword;
char **column_names, **column_types, **column_start_bytes, **column_bytes;
int object_class, columns, i, pds_version;
long header_record_bytes=0, record_bytes=0, items, item_bytes;
long l, label_records, label_record_bytes;
long sample_bits;

if (root_node == NULL) {
  oa_errno = 501;
  return(NULL);
}
if (Parent( root_node) != NULL) {
  sprintf( error_string, "%s: input argument must be the root node",
           proc_name);
  oa_errno = 530;
  OaReportError( error_string);
}

if (OaKwdValuetoStr( "PDS_VERSION_ID", root_node, &str) == 0) {
  if (strcmp( str, "PDS3") == 0)
    return( root_node);
}

/* The ODL tree is Version 0 if there are no subobjects and the FILE_TYPE
   keyword is present.  */

if ((OdlGetObjDescChildCount( root_node) == 0) &&
    ((kwdptr = OdlFindKwd( root_node, "FILE_TYPE", "*", (unsigned long) 0, 
                          (unsigned short) ODL_THIS_OBJECT)) != NULL)) {

  /**************************************************************************
  Version 0 label
  **************************************************************************/

pds_version = 0;
#ifdef OA_DEBUG
  sprintf( error_string, "%s: converting a PDS Version 0 tree to Version 3",
           proc_name);
  oa_errno = 950;
  OaReportError( error_string);
#endif

  str = OdlGetKwdValue( kwdptr);
  if        (strcmp( str, "IMAGE") == 0) {
    object_class = OA_IMAGE;
  } else if (strcmp( str, "TABLE") == 0) {
    object_class = OA_TABLE;
  } else {
    sprintf( error_string, "%s: unknown FILE_TYPE: %s", proc_name, str);
    oa_errno = 531;
    OaReportError( error_string);
    return( NULL);
  }

  /* Make a new node for the TABLE or IMAGE and attach it below the root;
     remove the FILE_TYPE keyword and deal with the file keywords RECORD_TYPE
     and RECORD_BYTES.  */

  odltreenode = OdlNewObjDesc( str, NULL, NULL, NULL, NULL, NULL,
                              (short) 0, (long) 0);
  OdlPasteObjDesc( odltreenode, root_node);
  OdlFreeKwd( OdlCutKwd( kwdptr));

  /* Check that the RECORD_TYPE keyword is present;  if not, add it with value
     FIXED_LENGTH.  */

  if (OaKwdValuetoStr( "RECORD_TYPE", root_node, &str) != 0) {
    OaStrtoKwdValue( "RECORD_TYPE", root_node, "FIXED_LENGTH");
  }

  /* Get the RECORD_BYTES keyword value;  if not present in an IMAGE label,
     then use LINE_SAMPLES * SAMPLE_BITS/8.  */

  str = "RECORD_BYTES";
  if (OaKwdValuetoLong( str, root_node, &record_bytes) != 0) {
    if (object_class == OA_IMAGE) {
      str = "LINE_SAMPLES";
      if (OaKwdValuetoLong( str, root_node, &record_bytes) != 0) {
        sprintf( error_string, 
                 "%s: %s keyword missing from PDS Version 0 label", 
                 proc_name, str);
        oa_errno = 531;
        OaReportError( error_string);
        return( NULL);
      }
      sample_bits = 8;
      OaKwdValuetoLong( "SAMPLE_BITS", root_node, &sample_bits);
      record_bytes *= (sample_bits/8);
      OaLongtoKwdValue( "RECORD_BYTES", root_node, record_bytes);
      
    } else {
      sprintf( error_string,  
               "%s: %s keyword missing from PDS Version 0 label", 
               proc_name, str);
      oa_errno = 531;
      OaReportError( error_string);
      return( NULL);
    }
  }

  /**************************************************************************
  Version 0 TABLE
  **************************************************************************/

  if (object_class == OA_TABLE) {

    /* Move the TABLE_ROWS and ROW_COLUMNS keywords from the root node to the
       new table node and rename them ROWS and COLUMNS.  Set INTERCHANGE_FORMAT
       to ASCII.  */

    table_node = odltreenode;
    OaStrtoKwdValue( "INTERCHANGE_FORMAT", table_node, "ASCII");
    str = "TABLE_ROWS";
    if ((kwdptr = OdlFindKwd( root_node, str, "*", (unsigned long) 0, 
                             (unsigned short) ODL_THIS_OBJECT)) == NULL) {
      sprintf( error_string, 
               "%s: %s keyword missing from PDS Version 0 label", 
               proc_name, str);
      oa_errno = 531;
      OaReportError( error_string);
      return( NULL);
    }
    OaStrtoKwdValue( "ROWS", table_node, OdlGetKwdValue( kwdptr));
    OdlFreeKwd( OdlCutKwd( kwdptr));

    str = "ROW_COLUMNS";
    if ((kwdptr = OdlFindKwd( root_node, str, "*", (unsigned long) 0, 
                             (unsigned short) ODL_THIS_OBJECT)) == NULL) {
      sprintf( error_string, 
               "%s: %s keyword missing from PDS Version 0 label", 
               proc_name, str);
      oa_errno = 531;
      OaReportError( error_string);
      return( NULL);
    }
    OaStrtoKwdValue( "COLUMNS", table_node, OdlGetKwdValue( kwdptr));
    OdlFreeKwd( OdlCutKwd( kwdptr));

    OaLongtoKwdValue( "ROW_BYTES", table_node, record_bytes);

    /* Get the COLUMN_NAME, COLUMN_TYPE, COLUMN_START_BYTE and COLUMN_BYTES
       keywords, verify that they're sequences, and get their values into the
       appropriate array of strings variables.  */

    str = "COLUMN_NAME";
    if (OaSequencetoStrArray( str, root_node, &column_names, &columns) != 0) {
      sprintf( error_string, "%s: %s keyword missing from PDS Version 0 label",
               proc_name, str);
      oa_errno = 531;
      OaReportError( error_string);
      return( NULL);
    }
    if (columns == 0)   /* OaSequencetoStrArray had trouble parsing sequence,*/
      return( NULL);    /* error message already issued.                     */

    str = "COLUMN_TYPE";
    if (OaSequencetoStrArray( str, root_node, &column_types, &i) != 0) {
      sprintf( error_string, "%s: %s keyword missing from PDS Version 0 label",
               proc_name, str);
      oa_errno = 531;
      OaReportError( error_string);
      return( NULL);
    }
    if (i == 0)        /* OaSequencetoStrArray had trouble parsing sequence,*/
      return( NULL);   /* error message already issued.                     */
    if (i != columns) {
      sprintf( error_string, 
        "%s: %s must have the same number of sequence items as COLUMN_NAME",
               proc_name, str);
      oa_errno = 532;
      OaReportError( error_string);
      return( NULL);
    }

    str = "COLUMN_START_BYTE";
    if (OaSequencetoStrArray( str, root_node, &column_start_bytes, &i) != 0) {
      sprintf( error_string, "%s: %s keyword missing from PDS Version 0 label",
               proc_name, str);
      oa_errno = 531;
      OaReportError( error_string);
      return( NULL);
    }
    if (i == 0)        /* OaSequencetoStrArray had trouble parsing sequence, */
      return( NULL);   /* error message already issued.                      */
    if (i != columns) {
      sprintf( error_string, 
        "%s: %s must have the same number of sequence items as COLUMN_NAME",
               proc_name, str);
      oa_errno = 532;
      OaReportError( error_string);
      return( NULL);
    }

    str = "COLUMN_BYTES";
    if (OaSequencetoStrArray( str, root_node, &column_bytes, &i) != 0) {
      sprintf( error_string, "%s: %s keyword missing from PDS Version 0 label",
               proc_name, str);
      oa_errno = 531;
      OaReportError( error_string);
      return( NULL);
    }
    if (i == 0)        /* OaSequencetoStrArray had trouble parsing sequence,*/
      return( NULL);   /* error message already issued.                     */
    if (i != columns) {
      sprintf( error_string, 
        "%s: %s must have the same number of sequence items as COLUMN_NAME",
               proc_name, str);
      oa_errno = 532;
      OaReportError( error_string);
      return( NULL);
    }

    /* Delete these four keywords.  */

    OaDeleteKwd( "COLUMN_NAME", root_node);
    OaDeleteKwd( "COLUMN_TYPE", root_node);
    OaDeleteKwd( "COLUMN_START_BYTE", root_node);
    OaDeleteKwd( "COLUMN_BYTES", root_node);

    /* Create ODL tree nodes for each column, and initialize each column's
       required keywords.  Translate each COLUMN_TYPE value into a PDS 
       Version 3 DATA_TYPE value.  */ 

    for (i=0; i<columns; i++) {
      odltreenode = OdlNewObjDesc( "COLUMN", NULL, NULL, NULL, NULL, NULL,
                                    (short) 0, (long) 0);
      OdlPasteObjDesc( odltreenode, table_node);
      OaStrtoKwdValue( "NAME", odltreenode, column_names[i]);
      if        (strcmp( column_types[i], "LITERAL") == 0) {
        str = "CHARACTER";
      } else if (strcmp( column_types[i], "REAL") == 0) {
        str = "ASCII_REAL";
      } else if (strcmp( column_types[i], "INTEGER") == 0) {
        str = "ASCII_INTEGER";
      } else {
        str = column_types[i];
      }
      OaStrtoKwdValue( "DATA_TYPE", odltreenode, str);
      OaStrtoKwdValue( "START_BYTE", odltreenode, column_start_bytes[i]);
      OaStrtoKwdValue( "BYTES", odltreenode, column_bytes[i]);
      LemmeGo( column_names[i]);
      LemmeGo( column_types[i]);
      LemmeGo( column_start_bytes[i]);
      LemmeGo( column_bytes[i]);
    }  /* end for loop through all columns */

    /* Free the sequence pointer arrays; contents of arrays have already
       been freed in the above loop.  */

    LemmeGo( column_names);
    LemmeGo( column_types);
    LemmeGo( column_start_bytes);
    LemmeGo( column_bytes);

  }  /* end if object_class == OA_TABLE */

  /**************************************************************************
  Version 0 IMAGE
  **************************************************************************/

  if (object_class == OA_IMAGE) {

    /* Dealias the IMAGE_LINES or IMAGE_RECORDS keyword to LINES, then move
       the LINES, LINE_SAMPLES, SAMPLE_BITS and SAMPLE_BIT_MASK keywords from
       the root node to the new table node, and add SAMPLE_TYPE = 
       UNSIGNED_INTEGER.  */

    image_node = odltreenode;
    OaDealiasKwdName( "LINES", root_node);
    if ((kwdptr = OdlFindKwd( root_node, "LINES", "*", 
                             (unsigned long) 0, 
                             (unsigned short) ODL_THIS_OBJECT)) == NULL) {
      sprintf( error_string, 
               "%s: PDS Version 0 version of LINES keyword missing", 
               proc_name);
      oa_errno = 531;
      OaReportError( error_string);
      return( NULL);
    }
    OaStrtoKwdValue( "LINES", image_node, OdlGetKwdValue( kwdptr));
    OdlFreeKwd( OdlCutKwd( kwdptr));

    str = "LINE_SAMPLES";
    if ((kwdptr = OdlFindKwd( root_node, str, "*", (unsigned long) 0, 
                             (unsigned short) ODL_THIS_OBJECT)) == NULL) {
      sprintf( error_string, 
               "%s: %s keyword missing from PDS Version 0 label", 
               proc_name, str);
      oa_errno = 531;
      OaReportError( error_string);
      return( NULL);
    }
    OdlPasteKwd( OdlCutKwd( kwdptr), image_node);

    str = "SAMPLE_BITS";
    if ((kwdptr = OdlFindKwd( root_node, str, "*", (unsigned long) 0, 
                             (unsigned short) ODL_THIS_OBJECT)) == NULL)
      OaStrtoKwdValue( "SAMPLE_BITS", image_node, "8");
    else
      OdlPasteKwd( OdlCutKwd( kwdptr), image_node);

    OaStrtoKwdValue( "SAMPLE_TYPE", image_node, "UNSIGNED_INTEGER");

    str = "SAMPLE_BIT_MASK";
    if ((kwdptr = OdlFindKwd( root_node, str, "*", (unsigned long) 0, 
                             (unsigned short) ODL_THIS_OBJECT)) != NULL)
      OdlPasteKwd( OdlCutKwd( kwdptr), image_node);

    str = "LINE_SUFFIX_BYTES";
    if ((kwdptr = OdlFindKwd( root_node, str, "*", (unsigned long) 0, 
                             (unsigned short) ODL_THIS_OBJECT)) != NULL)
	{
	    OdlPasteKwd( OdlCutKwd( kwdptr), image_node);
	}
    str = "LINE_PREFIX_BYTES";
    if ((kwdptr = OdlFindKwd( root_node, str, "*", (unsigned long) 0, 
                             (unsigned short) ODL_THIS_OBJECT)) != NULL) 
	{
		OdlPasteKwd( OdlCutKwd( kwdptr), image_node);
	}


  }  /* end if IMAGE */


  /* Now convert the ^POINTER keyword from Version 0 to Version 3.  */

  if (object_class == OA_IMAGE) {
    old_pointer_keyword = "IMAGE_POINTER";
    new_pointer_keyword = "^IMAGE";
  }
  if (object_class == OA_TABLE) {
    old_pointer_keyword = "TABLE_POINTER";
    new_pointer_keyword = "^TABLE";
  }

  /* If there's no IMAGE_POINTER or TABLE_POINTER keyword in the root node,
     have an attached label;  for an attached label, a LABEL_RECORDS or
     LABEL_RECORD_BYTES keyword should be present to specify how many bytes 
     preceed the first data object.  */

  if ((kwdptr = OdlFindKwd( root_node, old_pointer_keyword, "*", 
                           (unsigned long) 0, 
                           (unsigned short) ODL_THIS_OBJECT)) == NULL) {
    label_records = 0;
    OaKwdValuetoLong( "LABEL_RECORDS", root_node, &label_records);
    label_record_bytes = 0;
    OaKwdValuetoLong( "LABEL_RECORD_BYTES", root_node, &label_record_bytes);
    
    if ((label_records == 0) && (label_record_bytes == 0)) {
      sprintf( error_string, 
    "%s: attached label missing LABEL_RECORDS and LABEL_RECORD_BYTES keywords",
               proc_name);
      oa_errno = 531;
      OaReportError( error_string);
      return( NULL);
    }

    if (label_record_bytes == 0)
      label_record_bytes = record_bytes;
    if (label_records == 0)
      label_records = 1;
    sprintf( buf, "%ld <BYTES>", label_record_bytes * label_records);
    OaStrtoKwdValue( new_pointer_keyword, root_node, buf);

  } else {  

    /* Detached label, so if there's HEADER_RECORDS or HEADER_RECORD_BYTES
       keywords, image starts after that.  Data file from the IMAGE_POINTER
       keyword is in kwdptr.  */

    if (OaKwdValuetoLong( "HEADER_RECORD_BYTES", root_node,
                          &header_record_bytes) == 0) {
      sprintf( buf, "(%s,%ld <BYTES>)", 
               OdlGetKwdValue( kwdptr), header_record_bytes+1);
      OaStrtoKwdValue( new_pointer_keyword, root_node, buf);
      OaDeleteKwd( "HEADER_RECORD_BYTES", root_node); 
        
    } else if (OaKwdValuetoLong( "HEADER_RECORDS", root_node, &l) == 0) {
      sprintf( buf, "(%s,%ld)", OdlGetKwdValue( kwdptr), l+1);
      OaStrtoKwdValue( new_pointer_keyword, root_node, buf);
      header_record_bytes = l * record_bytes;

    } else {  /* Object starts at beginning of data file.  */
      OaStrtoKwdValue( new_pointer_keyword, root_node, 
                       OdlGetKwdValue( kwdptr));
    }
  }  /* end else no POINTER keyword so attached label */

  /* Now that the new ^POINTER keyword is added, delete the old keywords. */

  OaDeleteKwd( "HEADER_RECORDS", root_node);
  OaDeleteKwd( "LABEL_RECORD_BYTES", root_node);
  OaDeleteKwd( "LABEL_RECORDS", root_node);
  OaDeleteKwd( "IMAGE_RECORDS", root_node);
  OaDeleteKwd( old_pointer_keyword, root_node);

  /* Move the ^POINTER keyword after the last of the file keywords, and set
     its file_name field to the root node's file_name field, so that
     OdlGetFileName will work correctly.  */

  kwdptr = OdlGetFirstKwd( root_node);
  while (kwdptr != NULL) {
    next_kwdptr = kwdptr;
    kwdptr = OdlNextKwd( next_kwdptr->right_sibling, "*RECORD*", "*",  
                        (unsigned long) 0, 
                        (unsigned short) ODL_THIS_OBJECT);
  }
  kwdptr = OdlFindKwd( root_node, new_pointer_keyword, "*", (unsigned long) 0, 
                      (unsigned short) ODL_THIS_OBJECT);
  CopyString( kwdptr->file_name, root_node->file_name);
  OdlPasteKwdAfter( OdlCutKwd( kwdptr), next_kwdptr);

  /* If the first keyword in the root node is the SFDU keyword, then add the
     "PDS_VERSION_ID = PDS3" keyword after the first keyword;  otherwise
     add the PDS_VERSION_ID keyword before the first keyword.  If there's an
     SFDU, make sure it has "= SFDU_LABEL" following it.  */
 
  kwdptr = OdlGetFirstKwd( root_node);
  if ((strncmp( kwdptr->name, "CCSD",4) == 0) ||
      (strncmp( kwdptr->name, "NJPL",4) == 0)) {
    OdlPasteKwdAfter( OdlNewKwd( "PDS_VERSION_ID", "PDS3", NULL, NULL, NULL, 
                                 (long) 0), kwdptr);
    if (strlen( kwdptr->value) == 0)
      OaStrtoKwdValue( kwdptr->name, root_node, "SFDU_LABEL");
  } else {
    OdlPasteKwdBefore( OdlNewKwd( "PDS_VERSION_ID", "PDS3", NULL, NULL, NULL, 
                                  (long) 0), kwdptr);
  }
  return( root_node);
}  /* end if PDS Version 0 label */


/* Check if it's a Version 1 or 2 label - it's a Version 1 or 2 label if the
   SFDU keyword name has a "PDS1" or "PDS2" in it.  */

kwdptr = OdlGetFirstKwd( root_node);
if (strstr( kwdptr->name, "PDS1") != NULL)
  pds_version = 1;
if (strstr( kwdptr->name, "PDS2") != NULL)
  pds_version = 2;

if ((pds_version == 1) || (pds_version == 2)) {

  /**************************************************************************
  Version 1 or 2 label
  **************************************************************************/

#ifdef OA_DEBUG
  sprintf( error_string, "%s: converting a PDS Version %d tree to Version 3",
           proc_name, pds_version);
  oa_errno = 950;
  OaReportError( error_string);
#endif

  /* Loop through all the top-level objects (TABLE, IMAGE etc.), and convert
     each one.  These should all be children directly under the root node, 
     i.e. there shouldn't be any interveaning FILE nodes in Version 1 and 2.  
     If the object is an IMAGE, then loop through its children, if any, 
     treating them as TABLES (the children are probably ^STRUCTURE inclusions
     for prefixes or suffixes).  */

  TLO_node = LeftmostChild( root_node);
  while (TLO_node != NULL) {
    object_class = OaGetObjectClass( TLO_node);

    if ((object_class == OA_IMAGE) && (LeftmostChild( TLO_node) != NULL)) {
      save_node = TLO_node; 
      TLO_node = LeftmostChild( TLO_node);
      object_class = OA_TABLE;
    }

    if (object_class == OA_TABLE) {
    
      /**********************************************************************
      Version 1 or 2 TABLE
      **********************************************************************/

      /* First check if a ^STRUCTURE inclusion file had an object surrounding
         all the column objects;  if so, move the extraneous node's keywords
         up to the TABLE node and remove the node.  Identify it by the word
         'STRUCTURE' at the end of the class name.  Some Mars/Viking labels
         have this 'feature'.  */

      if (OdlGetObjDescChildCount( TLO_node) == 1) {
        tmp_node = LeftmostChild( TLO_node);
        if ((int) strlen( tmp_node->class_name) >= 9) {
          if (strstr( tmp_node->class_name, "STRUCTURE") ==
              (tmp_node->class_name + strlen( tmp_node->class_name) - 9)) {

            /* Move keywords in the extraneous node to the TLO node.  */

            kwdptr = OdlFindKwd( tmp_node, NULL, NULL, (unsigned long) 0, 
                                 (unsigned short) ODL_THIS_OBJECT);
            while (kwdptr != NULL) {
              OdlPasteKwd( OdlCutKwd( kwdptr), TLO_node);
              kwdptr = OdlFindKwd( tmp_node, NULL, NULL, (unsigned long) 0, 
                                   (unsigned short) ODL_THIS_OBJECT);
            }

            /* Move children of the extraneous node to under the TLO node,
               and remove the node.  */

            odltreenode = LeftmostChild( tmp_node);
            while (odltreenode != NULL) {
              OdlPasteObjDesc( OdlCutObjDesc( odltreenode), TLO_node);
              odltreenode = LeftmostChild( tmp_node);
            }          
            OdlFreeTree( OdlCutObjDesc( tmp_node));
          }
        }
      }
          

      /* If there's no INTERCHANGE_FORMAT keyword, then check for a FORMAT
         keyword; if there is one, change it to INTERCHANGE_FORMAT, otherwise
         add "INTERCHANGE_FORMAT = ASCII" as the default.  */

      if (OaKwdValuetoStr( "INTERCHANGE_FORMAT", TLO_node, &str) != 0) {
        str = "FORMAT";
        if ((kwdptr = OdlFindKwd( TLO_node, str, "*", (unsigned long) 0, 
                                 (unsigned short) ODL_THIS_OBJECT)) == NULL) {
          OaStrtoKwdValue( "INTERCHANGE_FORMAT", TLO_node, "ASCII");        
        } else {
          LemmeGo( kwdptr->name);
          CopyString( kwdptr->name, "INTERCHANGE_FORMAT");
        }
      }

      str = "ROW_COLUMNS";
      if ((kwdptr = OdlFindKwd( TLO_node, str, "*", (unsigned long) 0, 
                               (unsigned short) ODL_THIS_OBJECT)) != NULL) {
        LemmeGo( kwdptr->name);
        CopyString( kwdptr->name, "COLUMNS");
      }

      /* Loop through all the children of the table node, doing the following:
         a) If a NAME keyword doesn't already exist, add it as the first
            keyword with the object class name as it's value.
         b) If the BYTES keyword is missing, and the ITEM_BYTES and ITEMS
            keywords are set, add BYTES keyword - it's a COLUMN.
         c) If the BYTES or BITS keyword is missing, then see if a "BYTE"
            or BIT keyword is substituting for START_BYTE and BYTES or
            START_BIT and BITS, then un-substitute it.  (See pg K-7 of
            the Oct 3, 1988 "Standards for the Preparation & Interchange
            of Data Sets" document.)
         d) Replace the object class name with "COLUMN" or "BIT_COLUMN",
            depending on whether BYTES or BITS is present.
         e) Change the TYPE or ITEM_TYPE keyword to DATA_TYPE, and leave
            the value as it is.

         Example (the way the tree would be printed):

         OBJECT       = SPACECRAFT_NAME
           START_BYTE = 1
           TYPE       = CHARACTER

           is converted to:

         OBJECT       = COLUMN
           NAME       = SPACECRAFT_NAME
           START_BYTE = 1
           DATA_TYPE  = CHARACTER
      */

      /* If a NAME keyword doesn't already exist, add it as the first
         keyword with the object class name as it's value.  */

      column_node = LeftmostChild( TLO_node);
      while (column_node != NULL) {
        if (OdlFindKwd( column_node, "NAME", "*", (unsigned long) 0, 
                       (unsigned short) ODL_THIS_OBJECT) == NULL) {
          if ((next_kwdptr = OdlGetFirstKwd( column_node)) != NULL) {
            kwdptr = OdlNewKwd( "NAME", column_node->class_name, NULL, NULL, 
                                 NULL, (long) 0);
            OdlPasteKwdBefore( kwdptr, next_kwdptr);
          } else {
            OaStrtoKwdValue( "NAME", column_node, column_node->class_name);
          }
        }

        /* If the BYTES keyword is missing, and the ITEM_BYTES and ITEMS
           keywords are set, add BYTES keyword.  */

        if (OdlFindKwd( column_node, "BYTES", "*", (unsigned long) 0, 
                       (unsigned short) ODL_THIS_OBJECT) == NULL) {
          if ((OaKwdValuetoLong( "ITEMS", column_node, &items) == 0) &&
              (OaKwdValuetoLong( "ITEM_BYTES", column_node, 
                                 &item_bytes) == 0)) {
            sprintf( buf, "%ld", items * item_bytes);
            kwdptr = OdlNewKwd( "BYTES", buf, NULL, NULL, NULL, (long) 0);
            next_kwdptr = OdlFindKwd( column_node, "ITEM_BYTES", "*", 
                                     (unsigned long) 0, 
                                     (unsigned short) ODL_THIS_OBJECT);
            OdlPasteKwdAfter( kwdptr, next_kwdptr);
          }
        }

        /* If a BYTE (or BIT) keyword is substituting for START_BYTE 
           (START_BIT) and BYTES (BITS), then un-substitute it.  */

        if ((OdlFindKwd( column_node, "BYTES", "*", (unsigned long) 0, 
                        (unsigned short) ODL_THIS_OBJECT) == NULL) &&
            (OdlFindKwd( column_node, "BITS", "*", (unsigned long) 0, 
                        (unsigned short) ODL_THIS_OBJECT) == NULL)) {
          kwdptr = OdlFindKwd( column_node, "BYTE", "*", (unsigned long) 0,  
                              (unsigned short) ODL_THIS_OBJECT);
          if (kwdptr != NULL) {
            OaStrtoKwdValue( "START_BYTE", column_node, kwdptr->value);
            OaStrtoKwdValue( "BYTES", column_node, "1");
            OdlFreeKwd( OdlCutKwd( kwdptr));
	  }
          kwdptr = OdlFindKwd( column_node, "BIT", "*", (unsigned long) 0, 
                              (unsigned short) ODL_THIS_OBJECT);
          if (kwdptr != NULL) {
            OaStrtoKwdValue( "START_BIT", column_node, kwdptr->value);
            OaStrtoKwdValue( "BITS", column_node, "1");
            OdlFreeKwd( OdlCutKwd( kwdptr));
          }
        }  /* end if need to un-substitute BYTE or BIT */

        /* Now a BYTES or BITS keyword should be present.  If so, set the
           class name to "COLUMN" or "BIT_COLUMN";  else issue an error
           message and continue with the next node.  */

        if (OaKwdValuetoLong( "BYTES", column_node, &l) == 0) {
          LemmeGo( column_node->class_name);
          CopyString( column_node->class_name, "COLUMN");
        } else if (OaKwdValuetoLong( "BITS", column_node, &l) == 0) {

          /* If there's no START_BIT keyword and BITS is divisible by 8,
             call it a COLUMN and replace BITS by BYTES;  otherwise call it
             a BIT_COLUMN.  */

          if ((OaKwdValuetoStr( "START_BIT", column_node, &str) != 0) &&
              ((l % 8) == 0)) {
            LemmeGo( column_node->class_name);
            CopyString( column_node->class_name, "COLUMN");
            OaDeleteKwd( "BITS", column_node);
            OaLongtoKwdValue( "BYTES", column_node, (l%8));
          } else {
            LemmeGo( column_node->class_name);
            CopyString( column_node->class_name, "BIT_COLUMN");
          }
        } else {
          sprintf( error_string, 
                   "%s: BYTES or BITS keyword missing from %s node", 
                   proc_name, column_node->class_name);
          OaReportError( error_string);
          column_node = RightSibling( column_node);
          continue;
        }

        /* Change the TYPE or ITEM_TYPE keyword to DATA_TYPE.  */

        if ((kwdptr = OdlFindKwd( column_node, "TYPE", "*", (unsigned long) 0, 
                                 (unsigned short) ODL_THIS_OBJECT)) != NULL) {
          LemmeGo( kwdptr->name);
          CopyString( kwdptr->name, "DATA_TYPE");
        }
        if ((kwdptr = OdlFindKwd( column_node, "ITEM_TYPE", "*", 
                                 (unsigned long) 0, 
                                 (unsigned short) ODL_THIS_OBJECT)) != NULL) {
          LemmeGo( kwdptr->name);
          CopyString( kwdptr->name, "DATA_TYPE");
        }
        column_node = RightSibling( column_node);
      }  /* end loop through all columns of TABLE */
    }  /* end if object_class == OA_TABLE */

    tmp_node = RightSibling( TLO_node);

    /* If this is the last ^STRUCTURE inclusion under an IMAGE node, then
       finished with the IMAGE node, so go to it's right sibling.  */

    if ((tmp_node == NULL) &&  
        (OaGetObjectClass( Parent( TLO_node)) == OA_IMAGE))
      TLO_node = RightSibling( save_node);
    else
      TLO_node = tmp_node;

  }  /* end loop through top-level object nodes */

  /* If the first keyword in the root node is the SFDU keyword, then add the
     "PDS_VERSION_ID = PDS3" keyword after the first keyword;  otherwise
     add the PDS_VERSION_ID keyword before the first keyword.  If there's an
     SFDU, make sure it has "= SFDU_LABEL" following it.  */
 
  kwdptr = OdlGetFirstKwd( root_node);
  if ((strncmp( kwdptr->name, "CCSD", 4) == 0) ||
      (strncmp( kwdptr->name, "NJPL", 4) == 0)) {
    OdlPasteKwdAfter( OdlNewKwd( "PDS_VERSION_ID", "PDS3", NULL, NULL, NULL, 
                                 (long) 0), kwdptr);
    if (strlen( kwdptr->value) == 0)
      OaStrtoKwdValue( kwdptr->name, root_node, "SFDU_LABEL");
  } else {
    OdlPasteKwdBefore( OdlNewKwd( "PDS_VERSION_ID", "PDS3", NULL, NULL, NULL, 
                                  (long) 0), kwdptr);
  }

  return( root_node);
} /* end if Version 1 or 2 label */

sprintf( error_string, 
         "%s: Unknown PDS Version number, couldn't convert tree",
         proc_name);
oa_errno = 532;
OaReportError( error_string);
return( NULL);
}



/*****************************************************************************

  Routine:  OaCopyObject

  Description:  This routine copies an entire oa_object - the Oa_Object 
                structure, the ODL tree, and the data, if any.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:   1 Sept  1994

  History:

    Creation - This routine was part of the Alpha Release of the OA library.

  Input: 
          oa_object - A pointer to an OA_Object structure; the object may have
                      data in memory, or it may not; the fields in oa_object
                      must be set as oa_object->is_in_memory is TRUE, 
                      oa_object->size > 0 and oa_object->data_ptr not equal
                      to NULL, or is_in_memory is FALSE, size is 0 and
                      data_ptr is NULL.  Any other combination is rejected.
         
  Output:  If successful, the routine returns a pointer to the new Oa_Object
           structure.   The input ODL tree has been copied to the new object.
           If the input oa_object's data_ptr wasn't NULL, then the data has 
           been copied to the new oa_object.  The new object's size is set the
           same as the old object's size.  The new object's stream_id is
           always set to NULL;  this part of the old object is not copied.

  Notes:  

*****************************************************************************/

#ifdef _NO_PROTO

OA_OBJECT OaCopyObject( object)
OA_OBJECT object;

#else

OA_OBJECT OaCopyObject( OA_OBJECT object)

#endif
{

static char *proc_name = "OaCopyObject";
OA_OBJECT new_object;
#ifdef IBM_PC
long i;
#endif

if (object == NULL) return(NULL);

new_object = OaNewOaObject();

if ((object->size > 0) && (object->data_ptr != NULL) &&
    (object->is_in_memory == TRUE)) {

  /* The object has data in memory, so copy it.  */

  if ((new_object->data_ptr = (PTR) OaMalloc( (long) object->size)) == NULL) {
    sprintf( error_string, "%s: OaMalloc failed! Out of memory!", proc_name);
    oa_errno = 720;
    OaReportError( error_string);
    exit(1);
    /*NOTREACHED*/
  }

#ifdef IBM_PC

  /* If we're on an IBM-PC, use a for loop to do the copy, since memcpy
     may not work for copies greater than 64K and/or over segment
     boundaries;  otherwise use memcpy since it's usually optimized.  */

  for (i=0; i<object->size; i++)
    new_object->data_ptr[i] = object->data_ptr[i];
#else
  memcpy( new_object->data_ptr, object->data_ptr,
          (size_t) object->size);
#endif
  new_object->is_in_memory = TRUE;
  new_object->size = object->size;
  new_object->stream_id = NULL;
  new_object->odltree = OaCopyTree( object->odltree, 0);
  new_object->profile = object->profile;
  return( new_object);

} else if ((object->size == 0) && (object->data_ptr == NULL) &&
           (object->is_in_memory == FALSE)) {

  /* The object does not have data in memory.  */

  new_object->data_ptr = NULL;
  new_object->is_in_memory = FALSE;
  new_object->size = 0;
  new_object->stream_id = NULL;
  new_object->odltree = OaCopyTree( object->odltree, 0);
  return( new_object);

} else {
  sprintf( error_string, "%s: invalid combination of inputs: is_in_memory, ",
           proc_name);
  strcat( error_string, " data_ptr and size.");
  oa_errno = 510;
  OaReportError( error_string);
  LemmeGo( new_object);
  return( NULL);
}
}



/*****************************************************************************

  Routine:  OaCreateAttachedLabel

  Description:  This routine takes a detached label and the data file
                described in the label, and combines them into an attached
                label file.  The attached label file will have the same
                record format as the input data file.  The label must describe
                only one data file, whose name is in every object ^POINTER
                keyword in root node of the label.

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  15 Nov  1996
  Last Modified:  15 Nov  1996

  History:

    Creation - This routine was part of the Version 1.1 Release of the OA
               library.

  Input:  
          odltree                 - An ODL tree containing the label which
                                    describes the data file.  Either odltree
                                    or label_filespec can be specified, but
                                    not both.

          label_filespec          - A character string giving the label file 
                                    name or path to it.    Either odltree
                                    or label_filespec can be specified, but
                                    not both.

          attached_label_filespec - A character string giving the name of the
                                    attached label file to be created.
         
  Output:  If successful, 0 is returned, else 1.

  Notes:  

*****************************************************************************/

#ifdef _NO_PROTO

int OaCreateAttachedLabel( odltree, label_filespec, attached_label_filespec)
ODLTREE odltree;
char *label_filespec;
char *attached_label_filespec;

#else

int OaCreateAttachedLabel( ODLTREE odltree, char *label_filespec,
                           char *attached_label_filespec)
#endif
{

static char *proc_name = "OaCreateAttachedLabel";
struct OaStreamStruct *input_stream_id, *output_stream_id;
ODLTREE root_node, TLO_object_node, odltreenode;
KEYWORD *kwdptr;
FILE *tmp_fd;
long label_bytes, file_offset, bytes_read, bytes_written, record_bytes;
long file_records, label_records;
unsigned short tmp_ushort;
char *buf, *str, *data_filename, tmp_filename[128];
int record_type, src_interchange_format, kwds=0, i, label_lines=0;
int bytes_to_add, new_bytes_to_add, unused_bytes, new_len, old_len;
int records_to_add;
struct pointer_info_struct {
  KEYWORD  *kwdptr;
  unsigned long offset;
} *pointer_info;
#if (defined( VAX) || defined( ALPHA_VMS))
stat_t stat_buffer;
#else
#ifndef MAC
struct stat stat_buffer;
#endif
#endif

if ((label_filespec == NULL) && (odltree == NULL)) {
  sprintf( error_string,  
           "%s: both label_filespec and odltree inputs are NULL.",
           proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return(1);
}
if (attached_label_filespec == NULL) {
  sprintf( error_string,  
           "%s: attached_label_filespec is NULL.",
           proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return(1);
}

/* If odltree wasn't passed in, parse the label file into an ODL tree.  
   Otherwise make a copy of the input odltree, so it doesn't get changed. */

if (odltree == NULL) {
  if ((root_node = OaParseLabelFile( label_filespec, NULL, 0, 1)) == NULL)
    return(1);
} else {
  root_node = OaCopyTree( odltree, OA_STRIP_SDT_NODES);
}

/* Loop through all the ^POINTER keywords twice:  the first time through, just
   count them up, then malloc space for them;  the second time through,
   add each one to the pointer_info array.  Verify that each keyword is an
   object ^POINTER (not a ^STRUCTURE, ^CATALOG, ^MAP_PROJECTION or
   ^DESCRIPTION) by verifying the existance of the object node corresponding
   to the ^POINTER.  */

for (i=0; i<2; i++) {
  if ((kwdptr = OdlFindKwd( root_node, "^*", "*", (unsigned long) 0, 
                           (unsigned short) ODL_THIS_OBJECT)) == NULL) {
    sprintf( error_string, "%s: no ^POINTER keywords found.", 
             proc_name);
    oa_errno = 531;
    OaReportError( error_string);
    return(1);
  }
  if (i==1) {
    if (kwds == 0) {
      sprintf( error_string, "%s: no object ^POINTER keywords found.", 
               proc_name);
      oa_errno = 531;
      OaReportError( error_string);
      return(1);
    }
    pointer_info = (struct pointer_info_struct *) 
                   OaMalloc( (long) (kwds * 
                             sizeof( struct pointer_info_struct)));
    if (pointer_info == NULL) {
      sprintf( error_string, "%s: OaMalloc failed! Out of memory!", proc_name);
      oa_errno = 720;
      OaReportError( error_string);
      exit(1);
      /*NOTREACHED*/
    }
    kwds = 0;
  }  /* end if i==1, second time through loop */

  while (kwdptr != NULL) {
    odltreenode = OdlFindObjDesc( root_node, OdlGetKwdName( kwdptr)+1, "*",
                                  "*", (unsigned long) 0, 
                                  (unsigned short) ODL_CHILDREN_ONLY);
    if (odltreenode != NULL) {
      TLO_object_node = odltreenode;
      if (i==0)
        kwds++;
      else
        pointer_info[ kwds++].kwdptr = kwdptr;
    }
    kwdptr = OdlNextKwd( OdlGetNextKwd( kwdptr), "^*", "*",
                         (unsigned long) 0, (unsigned short) ODL_THIS_OBJECT);
  }
}

/* Get the data filename, record type and record bytes.  */

if (OaGetFileKeywords( TLO_object_node, &str, &data_filename,
                       &record_type, &record_bytes, &file_records,
                       &file_offset, &src_interchange_format) != 0) {
  return(1);  /* Error message already issued. */
}
if ((record_type == OA_STREAM) || (record_type == OA_UNDEFINED_RECORD_TYPE)) 
  record_bytes = BUFSIZ;

/* Allocate the buffer for tranferring data from the data file to the
   attached label file.  */

if ((buf = (char *) OaMalloc( record_bytes)) == NULL) {
  sprintf( error_string, 
           "%s: OaMalloc failed to allocate space. Out of memory!",
           proc_name);
  oa_errno = 720;
  OaReportError( error_string);
  exit(1);
  /*NOTREACHED*/
}

/* Call L3 function OdlGetFileName to parse the ^POINTER values and save their
   offset values in the pointer_info array.  For fixed-length and variable-
   length record files, save the offsets in records units; for stream and
   undefined record formats, save in bytes units.  (OdlGetFileName makes a
   copy of the file name part of the value before returning it, so free it.)*/

for (i=0; i<kwds; i++) {
  str = OdlGetFileName( pointer_info[i].kwdptr, &(pointer_info[i].offset),
                        &tmp_ushort);
  OaFree( (char *) str);
  if ((tmp_ushort == ODL_BYTE_LOCATION) && (record_type == OA_FIXED_LENGTH))
    pointer_info[i].offset = (pointer_info[i].offset-1) / record_bytes + 1;
}

/* Update the ^POINTER keywords in the ODL tree the way we're going to when the
   final file is written, so that the number of bytes they take up doesn't
   change;  for variable-length and fixed-length record files, the object 
   location units is in records;  for all other record formats, it's in
   bytes.  */

for (i=0; i<kwds; i++) {
  sprintf( buf, "%-ld", pointer_info[i].offset);
  if ((record_type == OA_STREAM) || (record_type == OA_UNDEFINED_RECORD_TYPE))
    strcat( buf, "<BYTES>");
  OaStrtoKwdValue( OdlGetKwdName( pointer_info[i].kwdptr), 
                   (pointer_info[i].kwdptr)->parent, buf);
}
/* If file_records is non-zero, then the FILE_RECORDS keyword is in the label.
   Rewrite its value, just to make sure it's formatted OAL's way.  */

if (file_records > 0)
  OaLongtoKwdValue( "FILE_RECORDS", root_node, file_records);

/* Open a temporary file for write, and write the ODL tree to it.  If the
   input label file was originally written by anyone besides
   OdlPrintLabelFile, the temporary file size is likely to be different from
   the input label file size, because of the different formatting done by
   OdlPrintLabelFile.  Since the label part of the attached label file will
   have the same format as OdlPrintLabelFile, use the size of this temporary
   file for calculating ^POINTER offsets, not the size of the original input
   label file.  */

tmpnam( tmp_filename);
if ((tmp_fd = fopen( tmp_filename, "w")) == NULL) {
  sprintf( error_string, "%s: couldn't open temporary file for write", 
           proc_name);
  oa_errno = 700;
  OaReportError( error_string);
  OaFree( buf);
  return(1);
}
OdlPrintLabel( root_node, NULL, tmp_fd, (unsigned long) 0, (MASK) 0);
fclose( tmp_fd);

/* Open the temporary file for read and get its size in bytes.  */

if ((tmp_fd = fopen( tmp_filename, "r")) == NULL) {
  sprintf( error_string, "%s: couldn't open temporary file for read", 
           proc_name);
  oa_errno = 700;
  OaReportError( error_string);
  OaFree( buf);
  return(1);
}
#ifdef MAC  /* Some compilers don't have fstat(), so use seek() instead.  */
fseek( tmp_fd, (long) 0, 2);  /* Seek to end-of-file.  */
label_bytes = ftell( tmp_fd);
fseek( tmp_fd, (long) 0, 0);      /* Seek back to beginning of file.  */
#else
if (fstat( fileno( tmp_fd), &stat_buffer) == 0) {
  label_bytes = (long) stat_buffer.st_size;
} else {
  sprintf( error_string, "%s: fstat on tmp file returned error",
           proc_name);
  oa_errno = 700;
  OaReportError( error_string);
  OaFree( buf);
  return(1);
}
#endif
if (label_bytes == 0) {
  sprintf( error_string, 
           "%s: OdlPrintLabel failed to write label!", proc_name);
  oa_errno = 700;
  OaReportError( error_string); 
  OaFree( buf);
  return(1);
}

/* For a variable-length record file, count the number of lines in the
   temporary file, since this will be the number of records of label in the
   attached label file.
   For a fixed-length record file, make label_bytes a multiple of record_bytes,
   since the last label record written to the attached label file will be
   padded out to record_bytes.  */

switch( record_type) {

  case OA_VARIABLE_LENGTH:
    while (!feof( tmp_fd)) {
      if (fgets( buf, record_bytes, tmp_fd) != NULL)
        label_lines++;
    }
  break;

  case OA_FIXED_LENGTH:
    unused_bytes = record_bytes - (label_bytes % record_bytes);
    label_bytes += unused_bytes;
    label_records = label_bytes/record_bytes;
  break;
}

/* Delete the temporary file.  */

fclose( tmp_fd);
remove( tmp_filename);

/* Now knowing how many records or bytes the label takes up, add this
   to all the ^POINTER offsets, since the label will be written in front of
   the object(s) in the output file.  */

switch( record_type) {

  case OA_VARIABLE_LENGTH:

    /* All the ^POINTER offsets are in records units.  Since each line in the
       label file will be written as a variable-length record to the output
       file, add the number of label lines to the ^POINTER offsets.  */

    for (i=0; i<kwds; i++)
      pointer_info[i].offset += label_lines;
    if (file_records > 0)
      file_records += label_lines;
  break;

  case OA_FIXED_LENGTH:

    /* All the ^POINTER offsets are in records units, so add label_records to
       each offset, then see if doing so has made the pointer keyword value
       longer, i.e. a new decimal place has been added.  If so, add this
       number of bytes to the new total amount of bytes to be added.  At the
       end, see if the new bytes can be accomodated in unused pad bytes in
       the last record;  if not, add as many full records as necessary, and
       loop again.  If the FILE_RECORDS keyword is present, it's value grows
       too, so treat it the same as the ^POINTER keywords.  */

    records_to_add = label_records;
    while (records_to_add > 0) {
      new_bytes_to_add = 0;
      for (i=0; i<kwds; i++) {
        sprintf( buf, "%-ld", pointer_info[i].offset);
        old_len = (int) strlen( buf);
        pointer_info[i].offset += records_to_add;
        sprintf( buf, "%-ld", pointer_info[i].offset);
        new_len = (int) strlen( buf);
        new_bytes_to_add += (new_len - old_len);
      }
      if (file_records > 0) {   /* FILE_RECORDS keyword exists. */
        sprintf( buf, "%-ld", file_records);
        old_len = (int) strlen( buf);
        file_records += records_to_add;
        sprintf( buf, "%-ld", file_records);
        new_len = (int) strlen( buf);
        new_bytes_to_add += (new_len - old_len);
      }        
      if (new_bytes_to_add <= unused_bytes) {
        records_to_add = 0;
      } else {
        new_bytes_to_add -= unused_bytes;
        records_to_add = (new_bytes_to_add/record_bytes + 1);
        unused_bytes = record_bytes - (new_bytes_to_add % record_bytes);
      }
    }
  break;

  case OA_UNDEFINED_RECORD_TYPE:
  case OA_STREAM:

    /* All the ^POINTER offsets are in bytes, so add label_bytes to each
       offset, then see if adding label_bytes has made the pointer keyword
       value longer, e.g. a new decimal place has been added.  If so, add
       this to label_bytes to be the new total amount to be added.  */

    bytes_to_add = label_bytes;
    while (bytes_to_add > 0) {
      new_bytes_to_add = 0;
      for (i=0; i<kwds; i++) {
        sprintf( buf, "%-ld", pointer_info[i].offset);
        old_len = (int) strlen( buf);
        pointer_info[i].offset += bytes_to_add;
        sprintf( buf, "%-ld", pointer_info[i].offset);
        new_len = (int) strlen( buf);
        new_bytes_to_add += (new_len - old_len);
      }
      bytes_to_add = new_bytes_to_add;
    }
  break;
}


/* Update the ^POINTER keywords in the label with the new offsets.  */

for (i=0; i<kwds; i++) {
  sprintf( buf, "%-ld", pointer_info[i].offset);
  if ((record_type == OA_STREAM) || (record_type == OA_UNDEFINED_RECORD_TYPE))
    strcat( buf, "<BYTES>");
  OaStrtoKwdValue( OdlGetKwdName( pointer_info[i].kwdptr), 
                   (pointer_info[i].kwdptr)->parent, buf);
}
if (file_records > 0) 
  OaLongtoKwdValue( "FILE_RECORDS", root_node, file_records);

/* Open a temporary file for write, and write the ODL tree to it.  */

tmpnam( tmp_filename);
if ((tmp_fd = fopen( tmp_filename, "w")) == NULL) {
  sprintf( error_string, "%s: couldn't open temporary file for write", 
           proc_name);
  oa_errno = 700;
  OaReportError( error_string);
  OaFree( buf);
  return(1);
}
OdlPrintLabel( root_node, NULL, tmp_fd, (unsigned long) 0, (MASK) 0);
fclose( tmp_fd);

/* Open the temporary file for read.  */

if ((tmp_fd = fopen( tmp_filename, "r")) == NULL) {
  sprintf( error_string, "%s: couldn't open tmp file for read", 
           proc_name);
  oa_errno = 700;
  OaReportError( error_string);
  OaFree( buf);
  return(1);
}

/* Open/create the attached label file for write.  */

if ((output_stream_id = OalOpenStream( attached_label_filespec, record_type,
                                       record_bytes, 0L, "w")) == NULL) {
  OaFree( buf);
  return(1);
}

/* Read records or lines from the temporary file and write them to the
   attached label file.  For fixed-length, OalWriteStream pad the last
   record out to record_bytes, if bytes_to_write is less than
   record_bytes.  */

while (!feof( tmp_fd)) {
  switch( record_type) {

    case OA_VARIABLE_LENGTH:
      if (fgets( buf, record_bytes-2, tmp_fd) != NULL) {

        /* Get rid of <CR>, <LF> or <CR><LF> at the end of the line.  */
           
        new_len = (int) strlen( buf);
        if (buf[ new_len-1] == '\n') {
          buf[ new_len-1] = '\0';
          new_len--;
        }
        if (new_len > 0) {
          if (buf[ new_len-1] == '\r') {
            buf[ new_len-1] = '\0';
            new_len--;
          }
        }

        /* When writing ASCII to a variable-length record file under VMS,
           add <CR><LF>, which are needed as line delimitors by
           OaParseLabelFile, since on VMS OaOpenOutputFile does NOT specify
           any record attributes (like carriage-return carriage control).  */

#if (defined( VAX) || defined( ALPHA_VMS))
        if (new_len > 0) {
          strcat( buf, "\r\n");
          new_len += 2;
        }
#endif

        /* Check if the line is longer than record_bytes.  If so, the label
           won't be readable, so report an error.  */

        if (new_len > record_bytes) {
          sprintf( error_string, 
         "%s: label line is longer than maximum variable-length record size!",
                   proc_name);
          oa_errno = 520;
          OaReportError( error_string);
          fclose( tmp_fd);
          remove( tmp_filename);
          OaFree( buf);
          return(1);
        }
        OalWriteStream( output_stream_id, (long) new_len, buf,
                        &bytes_written);
      }
    break;

    case OA_FIXED_LENGTH:
    case OA_UNDEFINED_RECORD_TYPE:
    case OA_STREAM:
      bytes_read = (long) fread( buf, 1, record_bytes, tmp_fd);
      OalWriteStream( output_stream_id, bytes_read, buf, &bytes_written);
    break;
  }
}  /* end while writing the label part of the attached label file. */

/* Delete the temporary file.  */

fclose( tmp_fd);
remove( tmp_filename);

/* Open the data file for read.  */

if ((input_stream_id = OalOpenStream( data_filename, record_type,
                                      record_bytes, 0L, "r")) == NULL) {
  OaFree( buf);
  return(1);
}

/* Read data from the data file, and write to the attached label file.  */

while (1) {
  buf = NULL;
  if (OalReadStream( input_stream_id, record_bytes, &buf, (long) -1,  
                     &bytes_read) == 1)  /* Reached EOF */
    break;

  OalWriteStream( output_stream_id, bytes_read, buf, &bytes_written);
}
OalCloseStream( input_stream_id);
OalCloseStream( output_stream_id);

return(0);
}



/*****************************************************************************

  Routine:  OaDeleteColumn

  Description:  This routine removes the data described by a COLUMN or 
                CONTAINER from the object data of a TABLE-like oa_object, 
                and removes the ODL tree node describing it from the
                table_object's ODL tree.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   9 Nov   1994
  Last Modified:   9 Nov   1994

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:  
         table_object  - A pointer to an Oa_Object structure with an in-memory
                         TABLE-like object.

         input_node    - Pointer to an ODL tree node one level below the root
                         (table) node; it's class can be COLUMN or CONTAINER.
         
  Output:  If successful, the function returns a pointer to the input
           Oa_Object structure, otherwise NULL.  table_object->data_ptr points
           to a new block of memory containing the table data minus the data
           described by the COLUMN or CONTAINER, table_object->size is 
           updated, the old data freed.  The COLUMN or CONTAINER sub-tree has
           been removed from the ODL tree, and the ODL tree's COLUMNS and
           ROW_BYTES keyword values updated.
           Note: table_object->odltree points to a new tree, and the old one
           has been freed.  Any pointers the caller may have saved which point
           to nodes in the old tree are now stale.

  Notes:   
  1) The function returns NULL and issues an error message if you attempt to
     delete the only COLUMN or CONTAINER in a table.
  2) The function deletes the specified column by calling OaGetSubTable,
     telling it to extract all the data except the data described by the
     input node.

*****************************************************************************/

#ifdef _NO_PROTO

OA_OBJECT OaDeleteColumn( table_object, input_node)
OA_OBJECT table_object;
ODLTREE input_node;

#else

OA_OBJECT OaDeleteColumn( OA_OBJECT table_object, ODLTREE input_node)

#endif
{

static char *proc_name = "OaDeleteColumn";
OA_OBJECT output_oa_object;
ODLTREE table_node, current_node, *subobject_nodes;
long n_subobjects, i, rows;

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
switch (OaGetObjectClass( table_node)) {
  case OA_TABLE:
  case OA_SPECTRUM:
  case OA_SERIES:
  case OA_PALETTE:
  case OA_GAZETTEER:
  break;
  default:
    sprintf( error_string, "%s: parent of input node must be a TABLE, ",
             proc_name);
    strcat( error_string, "SERIES, SPECTRUM, PALETTE or GAZETTEER object.");
    oa_errno = 530;
    OaReportError( error_string);
    return( NULL);
}

/* Check that TABLE_STORAGE_TYPE, if it exists, is ROW MAJOR.  Eventually
   will add code to deal with a COLUMN MAJOR table, but for now, reject
   COLUMN MAJOR tables.  */

if (OaGetTableStorageType( table_node) != OA_ROW_MAJOR) {
  sprintf( error_string, "%s: %s's TABLE_STORAGE_TYPE must be ROW_MAJOR.",
           proc_name, OdlGetObjDescClassName( table_node));
  oa_errno = 520;
  OaReportError( error_string);
  return( NULL);
}

if (LeftmostChild( table_node) == RightmostChild( table_node)) {
  sprintf( error_string, 
           "%s: illegal to delete the only sub-object in a table.", 
           proc_name);
  oa_errno = 520;
  OaReportError( error_string);
  return( NULL);
}

if (Parent( input_node) != table_node) {
  sprintf( error_string, 
           "%s: input_node must be a child of table_object->odltree.", 
           proc_name);
  oa_errno = 530;
  OaReportError( error_string);
  return( NULL);
}

if (OaKwdValuetoLong( "ROWS", table_node, &rows) != 0) {
  sprintf( error_string, "%s: couldn't find ROWS keyword.", proc_name);
  oa_errno = 531;
  OaReportError( error_string);
  return( NULL);
}

/* The way to delete a column and have all subsequent columns realigned,
   is to call OaGetSubTable specifying to get all the columns except the
   one to be deleted.  Allocate memory for the subobject_nodes array, 
   then stuff it with pointers to all children of the table_node except
   the input_node.  */

n_subobjects = OdlGetObjDescChildCount( table_node);
if ((subobject_nodes = (ODLTREE *) OaMalloc( (size_t) 
                       (n_subobjects-1) * sizeof( ODLTREE))) == NULL) {
  sprintf( error_string, "%s: OaMalloc failed! Out of memory!", proc_name);
  oa_errno = 720;
  OaReportError( error_string);
  exit(1);
  /*NOTREACHED*/
}
current_node = LeftmostChild( table_node);
i = 0;
while (current_node != NULL) {
  if (current_node != input_node)
    subobject_nodes[i++] = current_node;
  current_node = RightSibling( current_node);
}

if ((output_oa_object = OaGetSubTable( table_object, 1, rows, subobject_nodes, 
                                       n_subobjects-1)) == NULL) {
  LemmeGo( subobject_nodes);
  return( NULL);  /* OaGetSubTable has already issued an error message.  */
}

/* To replace the input table_object with output_oa_object, copy the contents
   of output_oa_object's structure to table_object's structure, and free the
   components of table_object which have been replaced.  This way the pointer
   returned to the caller is the same as the input pointer, but the contents
   have changed.  */

OdlFreeTree( table_object->odltree);
OaFree( (char *) table_object->data_ptr);
*table_object = *output_oa_object;
LemmeGo( output_oa_object);
LemmeGo( subobject_nodes);
return( table_object);
}



/*****************************************************************************

  Routine:  OaDeleteObject

  Description:  This routine frees all the data structures associated with
                an oa_object.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:   1 Sept  1994

  History:

    Creation - This routine was part of the Alpha Release of the OA library.

  Input:  
           object - A pointer to an OA_Object structure.

  Output:  The routine always returns 0.  If the object's odltree pointer
           wasn't NULL, then the ODL tree has been freed.
           If the object's data_ptr wasn't NULL, then the data has been freed.
           If the object's stream_id wasn't NULL, then the stream structure,
           it's buffer and it's filename have all been freed. 

  Notes:   Callers should not allow more than one object to contain 
           pointers to the same stream structure, or for more than one
           different stream structures to point to the same filename or
           stream buffer.  Calls to this routine may attempt to free memory 
           which has already been freed.

*****************************************************************************/

#ifdef _NO_PROTO

int OaDeleteObject( object)
OA_OBJECT object;

#else

int OaDeleteObject( OA_OBJECT object)

#endif
{

if (object == NULL) 
  return(0);
if (object->odltree != NULL) 
  OdlFreeTree( object->odltree);
if (object->data_ptr != NULL)
  OaFree( (char *) object->data_ptr);
if (object->stream_id != NULL) {
  LemmeGo( object->stream_id->buf);
  LemmeGo( object->stream_id->filename);
}
LemmeGo( object->stream_id);
LemmeGo( object);
return(0);
}



/*****************************************************************************

  Routine:  OaDeleteRow

  Description:  This routine deletes a row from a TABLE.  It reallocates the
                object's data, and removes all the bytes associated with the
                row from the object data, and subtracts one from the ROWS
                keyword value in the ODL tree.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   9 Nov   1994
  Last Modified:  18 Jan   1996

  History:

    Creation - This routine was part of the Beta Release of the OA library.
    01/18/96 - Fixed indexing bug in row copying loop in IBM-PC section.  SM

  Input:  
         table_object - A pointer to an Oa_Object structure with an in-memory
                        TABLE-like object.

         row          - The row to delete; an integer in the range 1..ROWS
         
  Output:  If successful, the function returns a pointer to the input
           Oa_Object structure, otherwise NULL.  table_object->data_ptr points
           to a new block of memory in which the row has been removed,
           and table_object->size and the ROWS keyword value in the ODL tree
           have been updated.  The old data_ptr has been freed.

  Notes:
  1) OaDeleteRow will return NULL and write an error message if you attempt
     to delete the only row in a table.

*****************************************************************************/

#ifdef _NO_PROTO

OA_OBJECT OaDeleteRow( table_object, row)
OA_OBJECT table_object;
long row;

#else

OA_OBJECT OaDeleteRow( OA_OBJECT table_object, long row)

#endif
{

static char *proc_name = "OaDeleteRow";
ODLTREE table_node;
long new_object_size, row_bytes, rows, rows_to_move, i;
#ifdef IBM_PC
long j;
#endif
long prefix_bytes, suffix_bytes;
int table_storage_type, interchange_format;

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

if (OaGetTableKeywords( table_node, &rows, &row_bytes, &prefix_bytes, 
                        &suffix_bytes, &interchange_format,
                        &table_storage_type) != 0)
  return( NULL);  /* Error message already written and oa_errno set.  */

if ((row < 1) || (row > rows)) {
  sprintf( error_string, "%s: invalid row number.", proc_name);
  oa_errno = 502;
  OaReportError( error_string);
  return( NULL);
}
if (rows == 1) {
  sprintf( error_string, "%s: illegal to delete the only row in a table.", 
           proc_name);
  oa_errno = 520;
  OaReportError( error_string);
  return( NULL);
}

if (table_storage_type == OA_ROW_MAJOR) {

  /* Unless row is the last row, move all rows after row back one row.  */

  rows_to_move = (rows - row);
  if (rows_to_move > 0) {
    for (i=row; i<rows; i++) {

#ifdef IBM_PC

      /* If we're on an IBM-PC, use a for loop to do the copy, since memcpy
         may not work for copies greater than 64K and/or over segment
         boundaries;  otherwise use memcpy since it's usually optimized.  */

      for (j=0; j<row_bytes; j++)
        table_object->data_ptr[ (i - 1) * row_bytes + j] =
          table_object->data_ptr[ i * row_bytes + j];
#else
      memcpy( table_object->data_ptr + (i - 1) * row_bytes, 
              table_object->data_ptr + i * row_bytes, (size_t) row_bytes);
#endif
    }
  }

  new_object_size = (rows-1) * row_bytes;
  table_object->data_ptr = (PTR) OaRealloc( (char *) table_object->data_ptr,
                                            table_object->size,
                                            (long) new_object_size);
  if (table_object->data_ptr == NULL) {
    sprintf( error_string, "%s: OaRealloc failed! Out of memory!", proc_name);
    oa_errno =720;
    OaReportError( error_string);
    exit(1);
    /*NOTREACHED*/
  }
  table_object->size = new_object_size;
  OaLongtoKwdValue( "ROWS", table_node, rows-1);
  return( table_object);
} else { /* COLUMN_MAJOR */
  sprintf( error_string, "%s: can't handle COLUMN_MAJOR yet...", proc_name);
  oa_errno = 520;
  OaReportError( error_string);
  return( NULL);
}
}



/*****************************************************************************

  Routine:  OaExportObject

  Description:  This routine frees the Oa_Object structure and structures it
                contains, and returns the data pointer.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:   1 Sept  1994

  History:

    Creation - This routine was part of the Alpha Release of the OA library.

  Input:  
          object - A pointer to an OA_Object structure.

  Output:  If the input object's data_ptr wasn't NULL, then the routine
           returns object->data_ptr, otherwise NULL.
           If object's odltree pointer wasn't NULL, then the ODL tree has
           been freed.  
           If object's stream_id wasn't NULL, then the stream structure,
           its buffer and its filename have all been freed.

  Notes:   Callers should not allow multiple objects' stream_id pointers to
           point to the same stream structure, nor for multiple stream
           structures to point to the same filename or stream buffer, otherwise
           a call to this function routine may attempt to free memory which
           has already been freed.

*****************************************************************************/

#ifdef _NO_PROTO

PTR OaExportObject( object)
OA_OBJECT object;

#else

PTR OaExportObject( OA_OBJECT object)

#endif
{

PTR data_ptr;

if (object == NULL) return( NULL);
if (object->odltree != NULL) 
  OdlFreeTree( object->odltree);
if (object->stream_id != NULL) {
  LemmeGo( object->stream_id->buf);
  LemmeGo( object->stream_id->filename);
}
LemmeGo( object->stream_id);
data_ptr = object->data_ptr;
LemmeGo( object);
return( data_ptr);
}



/*****************************************************************************

  Routine:  OaGetSubCollection

  Description:  OaGetSubCollection extracts a subobject from an ARRAY or 
                COLLECTION.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   6 Mar  1996
  Last Modified:   6 Mar  1996

  History:

    Creation - This routine was part of Release 1.1 of the OA library.

  Input:  
         oa_object      - A pointer to an Oa_Object structure with an 
                          in-memory ARRAY or COLLECTION object.

         subobject_node - An ODLTREE node pointer, pointing to the sub-object
                          to extract.  The subobject_node can be an ARRAY,
                          COLLECTION or ELEMENT at any level under the root.
         
  Output:  If successful, the function returns a pointer to an Oa_Object
           structure, otherwise NULL.  The output Oa_Object's ODL tree has a 
           root node whose class is ARRAY.  Somewhere under the ARRAY node is
           an object of the same class as the input subobject_node, i.e. an
           ELEMENT, COLLECTION or ARRAY.  Between this node and the root are
           as many ARRAY nodes as there were in the original tree, but all
           COLLECTION nodes above the node corresponding to the input
           subobject_node have been removed.
           The data for the extracted subtable is pointed to by the returned
           Oa_Object's data_ptr.

  Notes:  

*****************************************************************************/

#ifdef _NO_PROTO

OA_OBJECT OaGetSubCollection( oa_object, subobject_node)
OA_OBJECT oa_object;
ODLTREE subobject_node;

#else

OA_OBJECT OaGetSubCollection( OA_OBJECT oa_object, ODLTREE subobject_node)

#endif
{

static char *proc_name = "OaGetSubCollection";
ODLTREE parent_node, grandparent_node, current_node;
ODLTREE new_tree, new_tree_node, old_tree_node, sdt, sdt_node, compressed_SDT;
long new_object_size;
#ifdef IBM_PC
long i;
#endif
PTR data_ptr;
char c;
OA_OBJECT output_oa_object;
int src_interchange_format, dst_interchange_format, object_class;
SDT_node *sdt_node_ptr;
struct oa_profile save_profile;

/* Check for valid inputs. */

if (oa_object == NULL) {
  sprintf( error_string, "%s: input oa_object is NULL.", proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return( NULL);
}

if (oa_object->odltree == NULL) {
  sprintf( error_string, "%s: input oa_object's ODL tree pointer is NULL.", 
           proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return( NULL);
}

object_class = OaGetObjectClass( oa_object->odltree);
if ((object_class != OA_ARRAY) && (object_class != OA_ELEMENT) &&
    (object_class != OA_COLLECTION)) {
  sprintf( error_string, "%s object_class not supported: %s: ", proc_name,
           oa_object->odltree->class_name);
  oa_errno = 520;
  OaReportError( error_string);
  return( NULL);
  /*NOTREACHED*/
}

if (oa_object->data_ptr == NULL) {
  sprintf( error_string, "%s: input oa_object's data_ptr is NULL.", 
           proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return( NULL);
}

src_interchange_format = OaGetObjectInterchangeFormat( oa_object->odltree);
if (src_interchange_format == OA_UNKNOWN_INTERCHANGE_FORMAT)
  return( NULL);

/* Copy the oa_object's ODL tree, then find the copy of subobject_node in
   the new tree.  Do a pre-order traverse of both trees in parallel, checking
   each old tree node if it's the subobject_node.  */

new_tree = OaCopyTree( oa_object->odltree, 
                       OA_STRIP_SDT_NODES | OA_STRIP_COMMENTS);
new_tree_node = new_tree;
old_tree_node = oa_object->odltree;

while (old_tree_node != subobject_node) {
  if (LeftmostChild( old_tree_node) != NULL) {
    old_tree_node = LeftmostChild( old_tree_node);
    new_tree_node = LeftmostChild( new_tree_node);
  } else {
    while (old_tree_node != NULL) {
      if (RightSibling( old_tree_node) != NULL) {
        old_tree_node = RightSibling( old_tree_node);
        new_tree_node = RightSibling( new_tree_node);
        break;
      }
      old_tree_node = Parent( old_tree_node);
      new_tree_node = Parent( new_tree_node);
    }
  }
}

/* Work upwards in the tree from new_tree_node, and strip off all nodes which
   describe data other than our subobject.  I.e. whenever current_node comes
   under a COLLECTION, strip off all siblings of current_node.  Everything we 
   strip off will be identified as implicit SPARES by OalCreateSDT, and the
   SDT will be set up to toss out this data.  */

current_node = new_tree_node;
parent_node = Parent( new_tree_node);
while( parent_node != NULL) {
  if (OaGetObjectClass( parent_node) == OA_COLLECTION) {
    while( RightSibling( current_node) != NULL)
      OdlFreeTree( OdlCutObjDesc( RightSibling( current_node)));
    while( LeftSibling( current_node) != NULL)
      OdlFreeTree( OdlCutObjDesc( LeftSibling( current_node)));
  }
  current_node = parent_node;
  parent_node = Parent( parent_node);
}

/* Create an SDT to control the extraction of the subobject data.  Set the
   destination interchange format to be the same as the source interchange
   format so no conversions are done.  */

save_profile = Oa_profile;
if (src_interchange_format == OA_ASCII_INTERCHANGE_FORMAT)
  Oa_profile.dst_format_for_ASCII_src = src_interchange_format;
else
  Oa_profile.dst_format_for_binary_src = src_interchange_format;
dst_interchange_format = src_interchange_format;

if ((sdt = OalCreateSDT( new_tree, src_interchange_format)) == NULL) {
  sprintf( error_string, "%s: CreateSDT returned error.", 
           proc_name);
  OaReportError( error_string);
  OalFreeSDT( sdt);
  Oa_profile = save_profile;
  return(NULL);
}
Oa_profile = save_profile;

sdt_node_ptr = (SDT_node *) sdt->appl1;
new_object_size = sdt_node_ptr->dst.size * sdt_node_ptr->total_repetitions;

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
output_oa_object->profile      = oa_object->profile;

/* Compress the SDT, and initialize current_node to point to the first
   end-node (data node) in the compressed SDT; initialize the node's dst.ptr
   with the pointer returned by OaMalloc.  */

compressed_SDT = OalCompressSDT( sdt);
sdt_node = OalInitializeSDT( compressed_SDT, data_ptr);

if (OalProcessSDT( (char *) oa_object->data_ptr, 
                   sdt_node_ptr->src.size * sdt_node_ptr->total_repetitions,
                   &sdt_node) != OA_REACHED_END_OF_SDT) {
  sprintf( error_string, "%s: OalProcessSDT returned error.", proc_name);
  OaReportError( error_string);
  OalFreeSDT( compressed_SDT);
  OalFreeSDT( sdt);
  return(NULL);
}

/* Free the compressed SDT. */

OalFreeSDT( compressed_SDT);

/* OalSDTtoODLTree strips off ODL tree nodes which have SDT_node->dst.size=0
   (e.g. spare nodes).  It also updates the DATA_TYPE keywords, if any 
   conversions were done.  */

OalSDTtoODLTree( sdt, dst_interchange_format);

/* Remove all COLLECTION nodes above the subobject node, since these are
   redundant (since they only contain one subobject).  */

current_node = new_tree_node;
parent_node = Parent( current_node);
while (parent_node != NULL) {
  if (OaGetObjectClass( parent_node) == OA_COLLECTION) {
    OdlCutObjDesc( current_node);
    grandparent_node = Parent( parent_node);
    if (grandparent_node != NULL) {
      OdlFreeTree( OdlCutObjDesc( parent_node));
      OdlPasteObjDesc( current_node, grandparent_node);
      parent_node = grandparent_node;
    } else {
      OdlFreeTree( parent_node);
      break;
    }
  } else {
    current_node = parent_node;
    parent_node = Parent( parent_node);
  }
}

/* If the root node is an ELEMENT object, then create a 1-dimensional ARRAY
   object and make the ELEMENT it's child.  */

if (OaGetObjectClass( current_node) == OA_ELEMENT) {
  parent_node = OdlNewObjDesc( "ARRAY", NULL, NULL, NULL, NULL, NULL, 
                               (short) 0, (long) 0);
  OdlPasteObjDesc( current_node, parent_node);
  OaStrtoKwdValue( "AXES", parent_node, "1");
  OaStrtoKwdValue( "AXIS_ITEMS", parent_node, "(1)");
  OaStrtoKwdValue( "NAME", parent_node, "ARRAY");
  current_node = parent_node;
}

output_oa_object->odltree = current_node;
return( output_oa_object);
}



/*****************************************************************************

  Routine:  OaGetSubTable

  Description:  OaGetSubTable extracts subobjects from a table in memory into
                a new table.  The subobjects can be a mixture of COLUMNS and
                CONTAINERS;  all must be directly under a TABLE-like node
                (not nested in a CONTAINER), and the table must be ROW_MAJOR.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   7 Nov  1994
  Last Modified:   7 Nov  1994

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:  
         table_object      - A pointer to an Oa_Object structure with an 
                             in-memory TABLE-like object.

         start_row         - The first row to extract, 1 <= start_row <= ROWS

         stop_row          - The last row to extract,  1 <= start_row <= ROWS

         subobject_nodes   - An array of ODLTREE node pointers, pointing to 
                             the COLUMNS or CONTAINERS to extract.

         n_subobject_nodes - The number of elements in the subobject_nodes
                             array.
         
  Output:  If successful, the function returns a pointer to an Oa_Object
           structure, otherwise NULL.  The output Oa_Object's ODL tree has a 
           TABLE root node with n_subobject_nodes children;  the ROWS keyword 
           in the TABLE node is updated.  The data for the extracted subtable
           is pointed to by the output Oa_Object's data_ptr.

  Notes:  
  1) Calling examples:
     a) Extracting a single column:
        ODLTREE column_node;
        column_object = OaGetSubTable( table_object, start_row, stop_row,
                                       &column_node, 1);
        The case of a single COLUMN-class object is optimized in the code.
        No SDT is used.
     b) Extracting two subobjects:
        ODLTREE column_node, collection_node, subobject_nodes[2];
        subobject_nodes[0] = column_node;
        subobject_nodes[1] = collection_node;
        new_table_object = OaGetSubTable( table_object, start_row, stop_row,
                                          subobject_nodes, 2);
          For this case, the code uses an SDT.

  2) Data type conversions and alignment of the data will be done according to
     the input table_object->profile, but since this profile should reflect the
     profile at the time the original table was created, no conversions and
     only alignment will normally be performed.  An exception to this may
     occur if the table was imported, and it's data types don't match
     the global profile (which is automatically inserted in it's "profile"
     field); in this case, calling this function will cause data conversions
     to be performed on the extracted subtable!  This will only happen when
     extracting multiple COLUMNS or a CONTAINER, since extracting a single
     COLUMN doesn't use an SDT as described in Note 1.
*****************************************************************************/

#ifdef _NO_PROTO

OA_OBJECT OaGetSubTable( table_object, start_row, stop_row, 
                         subobject_nodes, n_subobject_nodes)
OA_OBJECT table_object;
long start_row, stop_row;
ODLTREE subobject_nodes[];
int n_subobject_nodes;

#else

OA_OBJECT OaGetSubTable( OA_OBJECT table_object, long start_row, 
                         long stop_row, ODLTREE subobject_nodes[], 
                         int n_subobject_nodes)

#endif
{

static char *proc_name = "OaGetSubTable";
ODLTREE table_node, odltreenode, right_sibling, sdt, sdt_node, compressed_SDT;
KEYWORD *kwdptr;
long new_object_size, column_size, row_bytes, items, rows, new_rows;
long prefix_bytes, suffix_bytes, i, start_byte;
#ifdef IBM_PC
long j;
#endif
PTR data_ptr;
char c;
OA_OBJECT output_oa_object;
int src_interchange_format, dst_interchange_format, table_storage_type;
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

if (n_subobject_nodes < 1) {
  sprintf( error_string, "%s: n_subobject_nodes must be >= 1.", proc_name);
  oa_errno = 520;
  OaReportError( error_string);
  return( NULL);
}

table_node = table_object->odltree;
for (i=0; i<n_subobject_nodes; i++) {
  if (Parent( subobject_nodes[i]) != table_node) {
    sprintf( error_string, 
             "%s: subobject_nodes[%d] must be a child of the root node.",
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
  sprintf( error_string, "%s: %s's TABLE_STORAGE_TYPE must be ROW_MAJOR.",
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

/* If the desired sub-table is a single COLUMN, then have a simple algorithm, 
   since don't have to build an SDT to align multiple columns and/or 
   containers.  */

if ((n_subobject_nodes == 1) && 
    (OaGetObjectClass( subobject_nodes[0]) == OA_COLUMN)) {

  /* Find column_size, the number of bytes in each row of the column, and
     items, the number of items in each row.  */

  items = 1;
  if (OaKwdValuetoLong( "ITEMS", subobject_nodes[0], &items) == 0) {
    if (OaKwdValuetoLong( "BYTES", subobject_nodes[0], &column_size) != 0) {
      sprintf( error_string, "%s: BYTES keyword value not found", 
               proc_name);
      strcat( error_string, " for COLUMN object.");
      oa_errno = 531;
      OaReportError( error_string);
      return(NULL);
    }
  } else {
    if (OaKwdValuetoLong( "BYTES", subobject_nodes[0], &column_size) != 0) {
      sprintf( error_string, "%s: BYTES keyword value not found", 
               proc_name);
      strcat( error_string, " for COLUMN object.");
      oa_errno = 531;
      OaReportError( error_string);
      return(NULL);
    }
  }

  start_byte = 1;
  OaKwdValuetoLong( "START_BYTE", subobject_nodes[0], &start_byte);
  start_byte--;
  
  new_object_size = column_size * new_rows;
  if ((data_ptr = (PTR) OaMalloc( (long) new_object_size)) == NULL) {
    sprintf( error_string, "%s: OaMalloc failed! Out of memory!", proc_name);
    oa_errno = 720;
    OaReportError( error_string);
    exit(1);
    /*NOTREACHED*/
  }

  /* Allocate an OA_object structure and initialize it.  */

  output_oa_object = OaNewOaObject();
  output_oa_object->is_in_memory = TRUE;
  output_oa_object->data_ptr     = data_ptr;
  output_oa_object->size         = new_object_size;
  output_oa_object->profile      = table_object->profile;

  for (i=0; i<new_rows; i++) {

#ifdef IBM_PC

      /* If we're on an IBM-PC, use a for loop to do the copy, since memcpy
         may not work for copies greater than 64K and/or over segment
         boundaries;  otherwise use memcpy since it's usually optimized.  */

    for (j=0; j<column_size; j++)
      data_ptr[ i*column_size + j] =
        table_object->data_ptr[ (i+start_row-1) * row_bytes + start_byte + j];
#else
    memcpy( data_ptr + i*column_size, table_object->data_ptr +  
            (i + start_row - 1) * row_bytes + start_byte, 
            (size_t) column_size);
#endif
  }

  /* Create an ODL tree node for the root node, and set required TABLE
     keywords.  */

  odltreenode = OdlNewObjDesc( OdlGetObjDescClassName( table_node),
                               NULL, NULL, NULL, NULL, NULL, (short) 0, 
                               (long) 0);
  OaLongtoKwdValue( "ROWS", odltreenode, (long) new_rows);
  OaLongtoKwdValue( "ROW_BYTES", odltreenode, (long) column_size);
  OaLongtoKwdValue( "COLUMNS", odltreenode, (long) 1);
  if ((kwdptr = OdlFindKwd( table_node, "INTERCHANGE*FORMAT", "*",
                            (unsigned long) 0, 
                            (unsigned short) ODL_THIS_OBJECT)) != NULL)
    OdlPasteKwd( OdlCopyKwd( kwdptr), odltreenode);

  output_oa_object->odltree = odltreenode;

  /* Copy the input column node, and change its START_OFFSET keyword value. */

  odltreenode = OaCopyTree( subobject_nodes[0], OA_STRIP_COMMENTS);
  OaLongtoKwdValue( "START_BYTE", odltreenode, (long) 1);
  OdlPasteObjDesc( odltreenode, output_oa_object->odltree);

  return( output_oa_object);
}


/* Not a single COLUMN case, so copy the table_object's ODL tree, then strip
   off all the sub-objects from the copy which don't match one of the nodes in
   the subobject_nodes array.  The loop goes through the children of the two
   trees in parallel.  The copied/stripped tree will be the input for 
   OalCreateSDT.  */

sdt = OaCopyTree( table_node, OA_STRIP_SDT_NODES | OA_STRIP_COMMENTS);
sdt_node = LeftmostChild( sdt);
odltreenode = LeftmostChild( table_node);

while (odltreenode != NULL) {       /* Loop through all columns in the table */
  for (i=0; i<n_subobject_nodes; i++)  /* Loop through subobject_nodes       */
    if (odltreenode == subobject_nodes[i]) break;
  right_sibling = RightSibling( sdt_node);
  if (i == n_subobject_nodes)       /* Node doesn't occur in subobject_nodes */
    OdlFreeTree( OdlCutObjDesc( sdt_node));
  odltreenode = RightSibling( odltreenode);
  sdt_node = right_sibling;  /* Note: if we stripped off sdt_node, then      */
                             /*  right_sibling was updated by OdlCutObjDesc. */
}

/* Use the input table's profile when creating the SDT, and set the global
   profile to force the dst to be the same interchange format as the src.  */

save_profile = Oa_profile;
Oa_profile = table_object->profile;
if (src_interchange_format == OA_ASCII_INTERCHANGE_FORMAT) {
  dst_interchange_format = table_object->profile.dst_format_for_ASCII_src;
  Oa_profile.dst_format_for_ASCII_src = src_interchange_format;
} else {
  dst_interchange_format = table_object->profile.dst_format_for_binary_src;
  Oa_profile.dst_format_for_binary_src = src_interchange_format;
}

OaLongtoKwdValue( "ROWS", sdt, (long) new_rows);

if ((sdt = OalCreateSDT( sdt, src_interchange_format)) == NULL) {
  sprintf( error_string, "%s: CreateSDT returned error.", 
           proc_name);
  OaReportError( error_string);
  OalFreeSDT( sdt);
  Oa_profile = save_profile;
  return(NULL);
}
Oa_profile = save_profile;

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
output_oa_object->profile      = table_object->profile;

/* Compress the SDT, and initialize current_node to point to the first
   end-node (data node) in the compressed SDT; initialize the node's dst.ptr
   with the pointer returned by OaMalloc.  */

compressed_SDT = OalCompressSDT( sdt);
sdt_node = OalInitializeSDT( compressed_SDT, data_ptr);

if (OalProcessSDT( (char *) table_object->data_ptr + 
                   (start_row-1)*sdt_node_ptr->src.size,
                   new_rows*sdt_node_ptr->src.size, &sdt_node) !=
                   OA_REACHED_END_OF_SDT) {
  sprintf( error_string, "%s: OalProcessSDT returned error.", proc_name);
  OaReportError( error_string);
  OalFreeSDT( compressed_SDT);
  OalFreeSDT( sdt);
  return(NULL);
}

/* Free the compressed SDT. */

OalFreeSDT( compressed_SDT);

/* OalSDTtoODLTree strips off ODL tree nodes which have SDT_node->dst.size=0
   (spare nodes added for alignment).  In this case it doesn't update the 
   DATA_TYPE keywords, since there probably weren't any conversions.  */

OalSDTtoODLTree( sdt, dst_interchange_format);
output_oa_object->odltree = sdt;
return( output_oa_object);
}



/*****************************************************************************

  Routine:  OaGetFileKeywords

  Description:  This function gets various file and object position attributes
                from an ODL tree, translating numeric values from ASCII to
                binary.  It searches upwards through the tree, starting at the
                input node, for the first occurrence of the keywords.
                The object class name of the input node is used as the search
                string for the ^POINTER which gives the data file name and
                record or byte offset of the object.  E.g. if the object class
                name is "MY_IMAGE", then the function searches for the keyword
                name "^MY_IMAGE".  Once it has the data file name, it tries to
                open it to make sure the name is correct; if unsuccessful, it
                tries other variations on the name: lower-case, ";1" appended,
                no path etc.
                This function will also unzip a zip-compressed file, if it
                hasn't already been unzipped.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:  20 Feb   1996

  History:

    Creation - This routine was part of the Alpha Release of the OA library.
    10/20/94 - Added file_records output parameter. SM
    11/27/95 - Added support for OA_UNDEFINED_RECORD_TYPE. SM
    02/20/96 - Added logic for Macintosh ':' directory delimitors. SM
    06/10/97 - Added unzip capability. SM

  Input:  odltreenode - A pointer to an ODL tree node for a top-level object,
                        i.e. TABLE, IMAGE etc.
         
  Output: 
          label_filename            - Points to a string in dynamic memory
                                      containing the label file path.  This is
                                      gotton from the file_name field of the
                                      root ODL tree node, which is set by
                                      OaParseLabelFile to it's input
                                      label_filename parameter.
          data_filename             - Points to a string in dynamic memory
                                      containing the data file name from an
                                      object's ^POINTER; if this file name
                                      doesn't have a directory component, then
                                      it is prefixed by the directory of
                                      label_filename, if it has any.  (The
                                      label_filename is gotton directly from
                                      the file_name field of the ODL tree node,
                                      see above).  If the data file name or any
                                      variation on it (lower-case, ";1" appended,
                                      no label path etc.) couldn't be found, 
                                      this is returned NULL.  If the function
                                      unzipped a zip-compressed file, then this
                                      is the newly created, unzipped data file.
          record_type               - Set to OA_FIXED_LENGTH, OA_STREAM,
                                      OA_VARIABLE_LENGTH, or
                                      OA_UNDEFINED_RECORD_TYPE.
          record_bytes              - Set to the RECORD_BYTES keyword value, 
                                      or 0 for a STREAM or UNDEFINED type file.
          file_records              - Set to the FILE_RECORDS keyword value,
                                      if it exists, otherwise 0.
          file_offset               - Set to the ^POINTER value of byte offset,
                                      minus 1 (starts at 0) if in <BYTES>.
                                      If ^POINTER value was in <RECORDS> units
                                      (if there were no units, <RECORDS> is the
                                      default) and file isn't variable-length,
                                      record file, then file_offset is the
                                      ^POINTER value converted from record
                                      offset to byte offset by using
                                      record_bytes. If variable-length record
                                      file, file_offset is returned as a
                                      record offset.
          object_interchange_format - Set to OA_BINARY_INTERCHANGE_FORMAT or
                                      OA_ASCII_INTERCHANGE_FORMAT.

  Notes:  

  -  Data file name and path:
     The ODL tree data structure has a "file_name" field, which is assumed
     to contain the file name of the label, along with the path, if one
     was supplied in the call to OdlParseLabelFile;  but if the ODL tree came
     from another source (e.g. it was copied without copying the "file_name" 
     field), then the caller may need to set the "file_name" field manually.

     The pathname part of the "file_name" field, if any, is normally used as
     the path for the data file;  i.e. OAL looks for the data file in the
     same directory as the label file;  this is overridden if the ^POINTER
     specifies a directory path in front of the file name.
     The ^POINTER should specify a file (unless it's an attached label) even if
     inside a "OBJECT = FILE" node.

  -  ^POINTER syntax:
     Pointers in the root or file node can have the following forms:
     ^IMAGE = "fileA"
     ^IMAGE = ("fileA",nnn)
     ^IMAGE = ("fileA",nnn <BYTES>)
     ^IMAGE = nnn <BYTES>
     ^IMAGE = nnn

  -  Offset of object in file:
     file_offset is the byte offset in the file of the top-level
     object (from the ^POINTER keyword) for stream and fixed-length record 
     files.  File_offset is a record offset for a variable-length record
     file.  

*****************************************************************************/

#ifdef _NO_PROTO

int OaGetFileKeywords( odltreenode, label_filename, data_filename, record_type,
                       record_bytes, file_records, file_offset, 
                       object_interchange_format)
ODLTREE odltreenode;
char **label_filename;
char **data_filename;
int *record_type;
long *record_bytes;
long *file_records;
long *file_offset;
int *object_interchange_format;

#else

int OaGetFileKeywords( ODLTREE odltreenode, char **label_filename, 
                       char **data_filename, int *record_type,
                       long *record_bytes, long *file_records, 
                       long *file_offset, 
                       int *object_interchange_format)

#endif
{

static char *proc_name = "OaGetFileKeywords";
char *TLO_object_str = NULL;
int object_class = OA_UNKNOWN_CLASS;
ODLTREE current_node, next_node, TLO_node, root_node;
KEYWORD *kwdptr;
unsigned short tmp_ushort;
unsigned long tmp_ulong;
char *kwd_value_str, *ptr, compressed_filename[80];
static char search_str[80];
static char path[256], buf[512], line[80];
int found;
FILE *inp = NULL;

/* Prevent "variable not used" warnings for these variables, since they're
   not used on some platforms.  */

path[0] = '\0';
buf[0]  = '\0';
line[0] = '\0';

/* Check for valid input parameters.  */

if (odltreenode == NULL) {
  sprintf( error_string, "%s: input ODL tree is NULL.", proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return(1);
}

if ((label_filename == NULL) || (data_filename == NULL) ||
    (record_type == NULL) || (record_bytes == NULL) || 
    (file_records == NULL) || (file_offset == NULL) ||
    (object_interchange_format == NULL)) {
  sprintf( error_string, "%s: one or more input arguments was NULL",
           proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return(1);
}

/* Get the label file path from the root node of the tree.  */

root_node = odltreenode;
while (Parent( root_node) != NULL)
  root_node = Parent( root_node);
if (root_node->file_name != NULL) {
  CopyString( *label_filename, root_node->file_name);
} else {
  sprintf( error_string, 
           "%s: the file_name field in the root node cannot be NULL",
           proc_name);
  oa_errno = 530;
  OaReportError( error_string);
  return(1);
}
*data_filename = NULL;

/* Check the PDS Version number string; if it's not PDS3, then all bets are off
   for OAL running correctly, but continue anyway.  It should have already been
   converted using OaConvertLabel.  */

kwd_value_str = OdlGetLabelVersion( root_node);
if (kwd_value_str == NULL) {
  sprintf( error_string, "%s: warning - no PDS version number found in root.",
      proc_name);
  oa_errno = 900;
  OaReportError( error_string);
} else if (strstr( kwd_value_str, "PDS3") == NULL) {
  sprintf( error_string, "%s: warning: PDS version number is not PDS3",
           proc_name);
  sprintf( error_string + strlen(error_string), " but rather: %s.",
           kwd_value_str);
  oa_errno = 903;
  OaReportError( error_string);
}

object_class = OaGetObjectClass( odltreenode);
switch (object_class) {
  case OA_ARRAY:
  case OA_COLLECTION:
  case OA_GAZETTEER:
  case OA_HISTOGRAM:
  case OA_HISTORY:
  case OA_IMAGE:
  case OA_PALETTE:
  case OA_QUBE:
  case OA_SERIES:
  case OA_SPECTRUM:
  case OA_TABLE:
    TLO_node = odltreenode;
    TLO_object_str = OdlGetObjDescClassName( odltreenode);
  break;
  case OA_UNKNOWN_CLASS:
    sprintf( error_string, "%s unknown object class: ", proc_name);
    strcat( error_string, OdlGetObjDescClassName( odltreenode));
    oa_errno = 530;
    OaReportError( error_string);
    return(1);
    /*NOTREACHED*/
  break;
  default:
  break;
}

/* Search upwards from the top-level object node for the first occurrence of
   the RECORD_TYPE keyword; once find it, find RECORD_BYTES keyword in the 
   same node.  */

current_node = TLO_node;
found = FALSE;
strcpy( search_str, "RECORD_TYPE");
while (found == FALSE) {
  if ((kwdptr = OdlFindKwd( current_node, search_str, "*", (unsigned long) 0, 
                (unsigned short) ODL_THIS_OBJECT)) == NULL) {
    if ((current_node = Parent( current_node)) == NULL) break;
  } else found = TRUE;
}
if (found == FALSE) {
  sprintf( error_string, "%s: RECORD_TYPE keyword not found.", 
           proc_name);
  oa_errno = 531;
  OaReportError( error_string);
  return(1);
}

/* Get the record type from the RECORD_TYPE keyword.  */

kwd_value_str = OdlGetKwdValue( kwdptr);
if      (strstr( kwd_value_str, "FIXED") != NULL)
  *record_type = OA_FIXED_LENGTH;
else if (strstr( kwd_value_str, "VARIABLE") != NULL)
  *record_type = OA_VARIABLE_LENGTH;
else if (strstr( kwd_value_str, "STREAM") != NULL)
  *record_type = OA_STREAM;
else if (strstr( kwd_value_str, "UNDEFINED") != NULL)
  *record_type = OA_UNDEFINED_RECORD_TYPE;
else {
  sprintf( error_string, "%s: invalid record type: %s.",
           proc_name, kwd_value_str);
  oa_errno = 531;
  OaReportError( error_string);
  return(1);
}

/* If applicable, find the RECORD_BYTES keyword.  */

if ((*record_type == OA_FIXED_LENGTH) || 
    (*record_type == OA_VARIABLE_LENGTH)) {
  strcpy( search_str, "RECORD_BYTES");
  if ((kwdptr = OdlFindKwd( current_node, search_str, "*", (unsigned long) 0, 
                            (unsigned short) ODL_THIS_OBJECT)) == NULL) {
    sprintf( error_string, "%s: RECORD_BYTES keyword not found.", 
             proc_name);
    oa_errno = 531;
    OaReportError( error_string);
    return(1);
  } else {
    if (OaKwdValuetoLong( "RECORD_BYTES", current_node, record_bytes) != 0) {
      return(1);  /* Error message already sent and errno set.  */
    }
  }
} else *record_bytes = 0;

/* Find the FILE_RECORDS keyword, if it is present, and get its value.  */

current_node = TLO_node;
found = FALSE;
strcpy( search_str, "FILE_RECORDS");
*file_records = 0;
while (found == FALSE) {
  if (OaKwdValuetoLong( "FILE_RECORDS", current_node, file_records) != 0) {
    if ((current_node = Parent( current_node)) == NULL) break;
  } else {
    found = TRUE;
  }
}

/* Search upwards from the top-level object node for the first occurrence of
   a ^POINTER to the top-level object class string.  */

current_node = TLO_node;
found = FALSE;
strcpy( search_str, "^");
strcat( search_str, TLO_object_str);
while (found == FALSE) {
  if ((kwdptr = OdlFindKwd( current_node, search_str, "*", (unsigned long) 0, 
                            (unsigned short) ODL_THIS_OBJECT)) == NULL) {
    if ((current_node = Parent( current_node)) == NULL) break;
  } else found = TRUE;
}
if (found == FALSE) {
  sprintf( error_string, "%s: pointer to top-level object not found.", 
           proc_name);
  oa_errno = 530;
  OaReportError( error_string);
  return(1);
}

/* Call L3 function OdlGetFileName to parse the ^POINTER value. OdlGetFileName
   makes a copy of the file name part of the value before returning it.  */

*data_filename = OdlGetFileName( kwdptr, &tmp_ulong, &tmp_ushort);
if (*data_filename == NULL) {
  sprintf( error_string, "%s: OdlGetFileName returned NULL", proc_name);
  oa_errno = 1000;
  OaReportError( error_string);
  return(1);
}
if (tmp_ushort == ODL_RECORD_LOCATION)
  if (*record_type == OA_VARIABLE_LENGTH) {
    *file_offset = (tmp_ulong-1);
  } else {
    *file_offset = (tmp_ulong-1) * (*record_bytes);
  }
else
  *file_offset = tmp_ulong-1;

/* If data_filename is different from label_filename, then check if data_filename
   contains a directory;  if so, use it as is.  If not, then call 
   OdlExtractFilename to combine the directory from label_filename with
   data_filename, or do whatever else it needs to do to find the data file.  */

if (strcmp( *label_filename, *data_filename) != 0) {

  if (OdlExtractFilename( *data_filename) == *data_filename) {  /* no directory */

    ptr = OdlGetFileSpec( *data_filename, *label_filename, NULL);
    if (ptr != NULL) {  /* ptr is the name of the data file proven to exist.  */
      OaFree( (char *) *data_filename);
      *data_filename = ptr;
    } else {

      /* OdlGetFileSpec couldn't find the data file, so only other possibility
         is that it's in a zip file which hasn't been unzipped.  If this is the
         case, then unzip it.  */

      next_node = OdlFindObjDesc( root_node, "COMPRESSED_FILE", "FILE_NAME",
                                  "*", (unsigned long) 0, 
                                  (unsigned short) ODL_CHILDREN_ONLY);
      if (next_node == NULL) {
        sprintf( error_string, "%s: couldn't find %s ", proc_name, 
                 *data_filename);
        oa_errno = 700;
        OaReportError( error_string);
        return( 1);
      }
      OaKwdValuetoStr( "FILE_NAME", next_node, &ptr);
      strcpy( compressed_filename, ptr);
      StripLeading(  compressed_filename, '\'');
      StripLeading(  compressed_filename, '"');
      StripTrailing( compressed_filename, '\'');
      StripTrailing( compressed_filename, '"');
#if defined( MAC) || defined( IBM_PC)
      sprintf( error_string, "%s: Please unzip %s and try again", proc_name,
               compressed_filename);
      oa_errno = 551;
      OaReportError( error_string);
      return( 1);
#else

      /* Try to find the compressed file.  */

      ptr = OdlGetFileSpec( compressed_filename, *label_filename, NULL);
      if (ptr == NULL) {
        sprintf( error_string, "%s: Couldn't find %s to unzip it!", proc_name,
                 compressed_filename);
        oa_errno = 551;
        OaReportError( error_string);
        return( 1);
      }  
     
      /* Found it, so unzip it.  Use quiet mode -qq so that if successful, it won't
         say anything.  Concievably the user could override this by defining an
         alias with --qq (which negates subsequent -qq) in which case (s)he will get
         an error message.  The code still checks for the presence of the data file
         to see if unzip worked.  If unzip only worked partially, e.g. reached disk
         quota, then there may be an incomplete file present, and the user will get
         unzip's output in an error message, but the code will continue until
         OalReadStream fails...  */

      strcpy( compressed_filename, ptr);
      OaFree( (char *) ptr);
      sprintf( error_string, "%s: Performing unzip on %s", proc_name,
               compressed_filename);
      OaReportError( error_string);
      sprintf( path, "unzip -o -qq %s", compressed_filename);
      buf[0] = '\0';
#if defined( VMS) || defined( ALPHA_VMS)
      system( path);
#else
      inp = popen( path, "r");
      while (fgets( line, 80, inp) != NULL)
        if ((strlen( line) + strlen( buf)) < 512)
          strcat( buf, line);
      pclose( inp);
#endif

      /* Look for the data file again.  */

      ptr = OdlGetFileSpec( *data_filename, *label_filename, NULL);
      if (ptr != NULL) {

        /* Unzip worked, and ptr is the name of the data file now proven to exist.  */

        OaFree( (char *) *data_filename);
        *data_filename = ptr;

        /* If got any messages from unzip, show them to the user, since unzip was
           supposed work quietly, printing only error messages.  */

        if (strlen( buf) > 0) {
          oa_errno = 552;
          OaReportError( buf);
        }

      } else {

        /* Unzip failed, so show the user the command, and let them deal with it. */

        sprintf( error_string, "%s: %s failed!", proc_name, path);
        strcat( error_string, " Is unzip installed and in your path?");
        oa_errno = 551;
        OaReportError( error_string);
        return( 1);
      }
#endif
    }  /* end else check if data file needs to be unzipped  */
  }  /* end if *data_filename wasn't specified with a directory component */
}  /* end if label_filename different from data_filename */

/* Search upwards from the top-level object node for the first occurrence of
   the INTERCHANGE_FORMAT keyword.  */

*object_interchange_format = OA_BINARY_INTERCHANGE_FORMAT;
current_node = TLO_node;
found = FALSE;
strcpy( search_str, "INTERCHANGE_FORMAT");
while (found == FALSE) {
  if ((kwdptr = OdlFindKwd( current_node, search_str, "*", (unsigned long) 0, 
                           (unsigned short) ODL_THIS_OBJECT)) == NULL) {
    if ((current_node = Parent( current_node)) == NULL) break;
  } else found = TRUE;
}

/* If found INTERCHANGE_FORMAT keyword, set output accordingly, otherwise
   leave at the default of BINARY.  */

if (found == TRUE) {
  kwd_value_str = OdlGetKwdValue( kwdptr);
  if      (strstr( kwd_value_str, "BINARY") != NULL)
    *object_interchange_format = OA_BINARY_INTERCHANGE_FORMAT;
  else if (strstr( kwd_value_str, "ASCII") != NULL)
    *object_interchange_format = OA_ASCII_INTERCHANGE_FORMAT;
  else {
    sprintf( error_string, "%s: invalid interchange format: %s.",
             proc_name, kwd_value_str);
    oa_errno = 531;
    OaReportError( error_string);
    return(1);
  }
}
return(0);
}



/*****************************************************************************

  Routine:  OaGetPartialImage

  Description:  This routine extracts a rectangular subimage out of an image in
                memory and returns an oa_object encapsulating the subimage.   
                Every line between start_line and stop_line inclusive is
                included in the subimage.  On each line, every sample between
                start_sample and stop_sample inclusive is included in the
                subimage.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:  12 Oct   1994

  History:

    Creation - This routine was part of the Alpha Release of the OA library.
    10/12/94 - Removed sample_interval argument as suggested by Mark Showalter.

  Input:  simple_image    - A pointer to an Oa_Object with object class IMAGE.
                            simple_image->is_in_memory is TRUE, 
                            simple_image->data_ptr points to valid image data
                            with total bytes equal to simple_image->size, and 
                            simple_image->odltree describes the image data.
          start_line      - 1 <= start_line <= stop_line <= LINES
          stop_line       - 1 <= start_line <= stop_line <= LINES
          start_sample    - 1 <= start_sample <= stop_samples <= LINE_SAMPLES
          stop_sample     - 1 <= start_sample <= stop_samples <= LINE_SAMPLES
         
  Output:  If successful, a pointer to an OA_Object structure, which contains
           an ODL tree and a pointer to the subimage data is returned.  
           The only node in the ODL tree is the IMAGE node.  If unsuccessful, 
           the routine returns NULL.

  Notes:  In the current release, the input image must be a SIMPLE_IMAGE, 
          that is, it does not have prefix or suffix bytes attached, and 
          cannot be multi-banded.

*****************************************************************************/

#ifdef _NO_PROTO

OA_OBJECT OaGetPartialImage( simple_image, start_line, stop_line,
                             start_sample, stop_sample)
OA_OBJECT simple_image;
long      start_line;
long      stop_line;
long      start_sample;
long      stop_sample;

#else

OA_OBJECT OaGetPartialImage( OA_OBJECT simple_image, long start_line, 
                             long stop_line, long start_sample, 
                             long stop_sample)

#endif
{

static char *proc_name = "OaGetPartialImage";
OA_OBJECT oa_object;
ODLTREE image_node, subimage_node;
long lines, line_samples, subimage_lines, subimage_samples;
long line, sample, sample_bits, sample_bytes, bands, subimage_size;
long line_prefix_bytes, line_suffix_bytes, image_line_bytes;
PTR image_ptr, subimage_ptr;
char *str;
int i, encoding_type, band_storage_type;

/* Check inputs for validity. */

if (simple_image == NULL) {
  sprintf( error_string, "%s: input OA_Object is NULL.", proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return(NULL);
}

if ((image_node = simple_image->odltree) == NULL) {
  sprintf( error_string, "%s: input OA_Object's odltree is NULL.", proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return(NULL);
}

if ((Parent( image_node) != NULL) || (LeftmostChild( image_node) != NULL)) {
  sprintf( error_string, 
           "%s: input OA_Object's odltree must be a stand-alone IMAGE ",
           proc_name);
  strcat( error_string, "without parent or sub-objects.");
  oa_errno = 530;
  OaReportError( error_string);
  return(NULL);
}

if (OaGetImageKeywords( image_node, &lines, &line_samples, &sample_bits,
                        &str, &bands, &band_storage_type, &line_prefix_bytes,
                        &line_suffix_bytes, &encoding_type) != 0)
  return(NULL);

/* Reject cases not currently supported (see notes above). */

if (bands > 1) {
  sprintf( error_string, "%s: multi-banded images not supported.", 
           proc_name);
  oa_errno = 520;
  OaReportError( error_string);
  return(NULL);
}
if (encoding_type != OA_UNCOMPRESSED) {
  sprintf( error_string,
           "%s: compressed IMAGEs not supported.", proc_name);
  oa_errno = 520;
  OaReportError( error_string);
  return(NULL);
}
if ((line_prefix_bytes != 0) || (line_suffix_bytes != 0)) {
  sprintf( error_string, 
           "%s: LINE_PREFIX_BYTES and LINE_SUFFIX_BYTES not supported.",
           proc_name);
  oa_errno = 520;
  OaReportError( error_string);
  return(NULL);
}
if ((sample_bits % 8) != 0) {
  sprintf( error_string,
           "%s: SAMPLE_BITS must be a multiple of 8.", proc_name);
  oa_errno = 502;
  OaReportError( error_string);
  return(NULL);
}
sample_bytes = sample_bits/8;

if ((start_line < 1) || (start_line > stop_line) || (stop_line > lines)) {
    sprintf( error_string, "%s: invalid start_line or stop_line.", proc_name);
    oa_errno = 502;
    OaReportError( error_string);
    return(NULL);
}
if ((start_sample < 1) || (start_sample > stop_sample) || 
    (stop_sample > line_samples)) {
    sprintf( error_string, "%s: invalid start_sample or stop_sample.", 
             proc_name);
    oa_errno = 502;
    OaReportError( error_string);
    return(NULL);
}

if ((lines * line_samples * sample_bytes) != simple_image->size) {
  sprintf( error_string,
           "%s: Error! LINES*LINE_SAMPLES*SAMPLE_BITS/8 is unequal to ",
           proc_name);
  strcat( error_string, "simple_image->size.");
  oa_errno = 511;
  OaReportError( error_string);
  return(NULL);
}


/* Create the output oa_object and initialize it.  */

oa_object = OaNewOaObject();

subimage_lines = stop_line - start_line + 1;
subimage_samples = stop_sample - start_sample + 1;
subimage_size = subimage_lines * subimage_samples * sample_bytes;

if ((oa_object->data_ptr = (PTR) OaMalloc( (long) subimage_size)) == NULL) {
  sprintf( error_string, "%s: OaMalloc failed! Out of memory!", proc_name);
  oa_errno = 720;
  OaReportError( error_string);
  exit(1);
  /*NOTREACHED*/
}

/* Create a new ODL tree node for the subimage and initialize it's required
   keywords.  */

subimage_node = OdlNewObjDesc( "IMAGE", NULL, NULL, NULL, NULL, NULL,  
                               (short) 0, (long) 0);
OaLongtoKwdValue( "LINES", subimage_node, subimage_lines);
OaLongtoKwdValue( "LINE_SAMPLES", subimage_node, subimage_samples);
OaStrtoKwdValue( "SAMPLE_TYPE", subimage_node, str);
OaLongtoKwdValue( "SAMPLE_BITS", subimage_node, sample_bits);

/* Now set some optional keywords to describe how the subimage was derived
   from the input image. */

OaLongtoKwdValue( "FIRST_LINE", subimage_node, start_line);
OaLongtoKwdValue( "FIRST_LINE_SAMPLE", subimage_node, start_sample);
OaLongtoKwdValue( "SOURCE_LINES", subimage_node, subimage_lines);
OaLongtoKwdValue( "SOURCE_LINE_SAMPLES", subimage_node, subimage_samples);
OaLongtoKwdValue( "SOURCE_SAMPLE_BITS", subimage_node, sample_bits);

/* Extract the subimage from a single banded image. */

image_line_bytes = sample_bytes * line_samples;

subimage_ptr = oa_object->data_ptr;
for (line=start_line; line<=stop_line; line++) {
  image_ptr = simple_image->data_ptr + image_line_bytes * (line-1) +
              sample_bytes * (start_sample-1);
  for (sample=start_sample; sample<=stop_sample; sample += 1) {
    for (i=0; i<sample_bytes; i++)
      *subimage_ptr++ = *image_ptr++;
  }
}

oa_object->odltree = subimage_node;
oa_object->size = subimage_size;
oa_object->stream_id = NULL;
oa_object->profile = simple_image->profile;
oa_object->is_in_memory = TRUE;

return( oa_object);

}



/*****************************************************************************

  Routine:  OaImportColumn

  Description:  OaImportColumn creates an oa_object with an ODL tree
                describing a TABLE with a single COLUMN, whose data the
                caller has created outside of OAL in memory.  
                The resulting oa_object can then be manipulated by OA
                routines.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  21 Nov  1994
  Last Modified:  21 Nov  1994

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:  data_ptr           - Points to column data in dynamic memory with
                               total size in bytes equal to:
                               rows * items * item_bytes.
          rows               - row >= 1
          items              - items >= 1
          item_bytes         - items_bytes >= 1
          data_type          - A valid PDS data type; a string
          interchange_format - OA_BINARY_INTERCHANGE_FORMAT or 
                               OA_ASCII_INTERCHANGE_FORMAT.
          name               - Must be a non-NULL string.
         
  Output:  If successful the routine returns a pointer to an OA_Object 
           structure, containing an ODL tree and a pointer to the column
           data.  The ODL tree has a TABLE node as it's root, with a single
           COLUMN node beneath it.  All the required keywords are set in the
           TABLE node and the COLUMN node.
           If unsuccessful the routine returns NULL.

  Notes:

*****************************************************************************/

#ifdef _NO_PROTO

OA_OBJECT OaImportColumn( data_ptr, rows, items, item_bytes, data_type,
                          interchange_format, name)
PTR  data_ptr;
long rows;
long items;
long item_bytes;
char *data_type;
int  interchange_format;
char *name;

#else

OA_OBJECT OaImportColumn( PTR data_ptr, long rows, long items,
                          long item_bytes, char *data_type,  
                          int interchange_format, char *name)

#endif
{

static char *proc_name = "OaImportColumn";
OA_OBJECT oa_object;
ODLTREE table_node, column_node;
char *data_type_str;

/* Check inputs for validity. */

if (data_ptr == NULL) {
  sprintf( error_string, "%s: input parameter data_ptr cannot be NULL.", 
           proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return(NULL);
}

if ((rows < 1) || (items < 1) || (item_bytes < 1) || (data_type == NULL) ||
    ((interchange_format != OA_BINARY_INTERCHANGE_FORMAT) &&
     (interchange_format != OA_ASCII_INTERCHANGE_FORMAT)) || (name == NULL)) {
  sprintf( error_string, "%s: bad input parameter value.", 
           proc_name);
  oa_errno = 502;
  OaReportError( error_string);
  return(NULL);
}

CopyString( data_type_str, data_type);
UpperCase( data_type_str);
if (OaStrtoPDSDataType( data_type_str, interchange_format) ==
  OA_UNKNOWN_DATA_TYPE) {
  sprintf( error_string, 
         "%s: input parameter data_type: %s is an unknown PDS data type.", 
           proc_name, data_type);
  oa_errno = 531;
  OaReportError( error_string);
  return(NULL);
}

/* Create a new ODL tree node for the table and initialize its required
   keywords.  */

table_node = OdlNewObjDesc( "TABLE", NULL, NULL, NULL, NULL, NULL, (short) 0, 
                            (long) 0);
OaLongtoKwdValue( "ROWS", table_node, rows);
OaLongtoKwdValue( "ROW_BYTES", table_node, items * item_bytes);
OaLongtoKwdValue( "COLUMNS", table_node, (long) 1);
OaStrtoKwdValue(  "INTERCHANGE_FORMAT", table_node, 
                  ((interchange_format == OA_ASCII_INTERCHANGE_FORMAT) ?
                   "ASCII" : "BINARY"));

/* Create a new ODL tree node for the column and initialize its required
   keywords.  */

column_node = OdlNewObjDesc( "COLUMN", NULL, NULL, NULL, NULL, NULL, 
                             (short) 0, (long) 0);
OaStrtoKwdValue(  "NAME", column_node, name);
OaStrtoKwdValue(  "DATA_TYPE", column_node, data_type_str);
OaLongtoKwdValue( "START_BYTE", column_node, (long) 1);
OaLongtoKwdValue( "BYTES", column_node, items * item_bytes);
OaLongtoKwdValue( "ITEMS", column_node, items);
OaLongtoKwdValue( "ITEM_BYTES", column_node, item_bytes);

OdlPasteObjDesc( column_node, table_node);

/* Create the output oa_object and initialize it.  */

oa_object = OaNewOaObject();
oa_object->odltree = table_node;
oa_object->data_ptr = data_ptr;
oa_object->size = rows * item_bytes * items;
oa_object->stream_id = NULL;
oa_object->profile = Oa_profile;
oa_object->is_in_memory = TRUE;

LemmeGo( data_type_str);
return( oa_object);

}



/*****************************************************************************

  Routine:  OaImportHistogram

  Description:  OaImportHistogram creates an oa_object with an ODL tree
                describing a HISTOGRAM object, whose data the caller has
                created outside of OAL in memory.  The resulting oa_object
                can then be manipulated by OA routines.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  29 Feb  1996
  Last Modified:  29 Feb  1996

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:  data_ptr           - Points to histogram data in dynamic memory with
                               total size in bytes equal to items * item_bytes.
          items              - items >= 1
          item_bytes         - items_bytes >= 1
          data_type          - A valid PDS data type; a string
          interchange_format - OA_BINARY_INTERCHANGE_FORMAT or 
                               OA_ASCII_INTERCHANGE_FORMAT.
         
  Output:  If successful the routine returns a pointer to an OA_Object 
           structure, containing an ODL tree and a pointer to the histogram
           data.  The ODL tree has a HISTOGRAM node as it's root, with all
           required keywords set.
           If unsuccessful the routine returns NULL.

  Notes:

*****************************************************************************/

#ifdef _NO_PROTO

OA_OBJECT OaImportHistogram( data_ptr, items, item_bytes, data_type,
                             interchange_format)
PTR  data_ptr;
long items;
long item_bytes;
char *data_type;
int interchange_format;

#else

OA_OBJECT OaImportHistogram( PTR data_ptr, long items, long item_bytes, 
                             char *data_type, int interchange_format)

#endif
{

static char *proc_name = "OaImportHistogram";
OA_OBJECT oa_object;
ODLTREE histogram_node;
char *data_type_str;

/* Check inputs for validity. */

if (data_ptr == NULL) {
  sprintf( error_string, "%s: input parameter data_ptr cannot be NULL.", 
           proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return(NULL);
}

if ((items < 1) || (item_bytes < 1) || (data_type == NULL)) {
  sprintf( error_string, "%s: bad input parameter value.", 
           proc_name);
  oa_errno = 502;
  OaReportError( error_string);
  return(NULL);
}

CopyString( data_type_str, data_type);
UpperCase( data_type_str);
if (OaStrtoPDSDataType( data_type_str, interchange_format) ==
  OA_UNKNOWN_DATA_TYPE) {
  sprintf( error_string, 
         "%s: input parameter data_type: %s is an unknown PDS data type.", 
           proc_name, data_type);
  oa_errno = 531;
  OaReportError( error_string);
  return(NULL);
}

/* Create a new ODL tree node for the table and initialize its required
   keywords.  */

histogram_node = OdlNewObjDesc( "HISTOGRAM", NULL, NULL, NULL, NULL, NULL,
                                (short) 0, (long) 0);
OaStrtoKwdValue(  "DATA_TYPE", histogram_node, data_type_str);
OaLongtoKwdValue( "ITEMS", histogram_node, items);
OaLongtoKwdValue( "ITEM_BYTES", histogram_node, item_bytes);
OaStrtoKwdValue(  "INTERCHANGE_FORMAT", histogram_node, 
                  ((interchange_format == OA_ASCII_INTERCHANGE_FORMAT) ?
                   "ASCII" : "BINARY"));

/* Create the output oa_object and initialize it.  */

oa_object = OaNewOaObject();
oa_object->odltree = histogram_node;
oa_object->data_ptr = data_ptr;
oa_object->size = item_bytes * items;
oa_object->stream_id = NULL;
oa_object->profile = Oa_profile;
oa_object->is_in_memory = TRUE;

LemmeGo( data_type_str);
return( oa_object);
}



/*****************************************************************************

  Routine:  OaImportImage

  Description:  This routine creates an oa_object with an ODL tree describing 
                image data the caller has created outside of OAL in memory.  
                The resulting oa_object can then be manipulated by OA routines.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:   1 Sept  1994

  History:

    Creation - This routine was part of the Alpha Release of the OA library.

  Input:  data_ptr     - Points to image data in dynamic memory with total 
                         size in bytes equal to:
                         lines * line_samples * sample_bits/8.
          lines        - lines >= 1.
          line_samples - line_samples >= 1
          sample_type  - A valid PDS DATA_TYPE string.
          sample_bits  - Must be a multiple of 8.
         
  Output:  If successful the routine returns a pointer to an OA_Object 
           structure, which contains an ODL tree and a pointer to the image 
           data.  The ODL tree consists of a single IMAGE node.  
           If unsuccessful the routine returns NULL.

  Notes:  

*****************************************************************************/

#ifdef _NO_PROTO

OA_OBJECT OaImportImage( data_ptr, lines, line_samples, sample_type,
                         sample_bits)
PTR  data_ptr;
long lines;
long line_samples;
char *sample_type;
int  sample_bits;

#else

OA_OBJECT OaImportImage( PTR data_ptr, long lines, long line_samples, 
                         char *sample_type, int sample_bits)

#endif
{

static char *proc_name = "OaImportImage";
OA_OBJECT oa_object;
ODLTREE image_node;
char *sample_type_str;

/* Check inputs for validity. */

if (data_ptr == NULL) {
  sprintf( error_string, "%s: input parameter data_ptr cannot be NULL.", 
           proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return(NULL);
}

if ((lines < 1) || (line_samples < 1)) {
  sprintf( error_string, "%s: invalid input lines or line_samples.", 
           proc_name);
  oa_errno = 502;
  OaReportError( error_string);
  return(NULL);
}

if ((sample_bits % 8) != 0) {
  sprintf( error_string, 
           "%s: input parameter sample_bits must be a multiple of 8.", 
           proc_name);
  oa_errno = 502;
  OaReportError( error_string);
  return(NULL);
}

if (sample_type == NULL) {
  sprintf( error_string, 
           "%s: input parameter sample_type must not be a NULL string.", 
           proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return(NULL);
}
CopyString( sample_type_str, sample_type);
UpperCase( sample_type_str);
if (OaStrtoPDSDataType( sample_type_str, OA_BINARY_INTERCHANGE_FORMAT) ==
    OA_UNKNOWN_DATA_TYPE) {
  sprintf( error_string, 
         "%s: input parameter sample_type: %s is an unknown PDS data type.", 
           proc_name, sample_type);
  oa_errno = 531;
  OaReportError( error_string);
  return(NULL);
}

/* Warning: no check is done to insure that sample_type is a valid data type
   for an IMAGE!  */


/* Create a new ODL tree node for the image and initialize its required
   keywords.  */

image_node = OdlNewObjDesc( "IMAGE", NULL, NULL, NULL, NULL, NULL, (short) 0, 
                            (long) 0);
OaLongtoKwdValue( "LINES", image_node, lines);
OaLongtoKwdValue( "LINE_SAMPLES", image_node, line_samples);
OaStrtoKwdValue(  "SAMPLE_TYPE", image_node, sample_type_str);
OaLongtoKwdValue( "SAMPLE_BITS", image_node, (long) sample_bits);

/* Create the output oa_object and initialize it.  */

oa_object = OaNewOaObject();
oa_object->odltree = image_node;
oa_object->data_ptr = data_ptr;
oa_object->size = lines * line_samples * sample_bits/8;
oa_object->stream_id = NULL;
oa_object->profile = Oa_profile;
oa_object->is_in_memory = TRUE;

LemmeGo( sample_type_str);
return( oa_object);
}



/*****************************************************************************

  Routine:  OaJoinTables

  Description:  OaJoinTables appends table_B to the end of table_A.  
                If input argument "options" is OA_ADD_ROWS, then all of 
                table_B's rows are added after the last row of table_A;
                table_B must have the same number of columns and same ROW_BYTES
                as table_A.
                If input argument "options" is OA_ADD_COLUMNS, then table_B's
                columns are added after the last column of table_A;  the 
                number of rows in table_A and table_B must be the same.  
                In either case, table_A is returned changed and table_B is 
                unchanged.  Table_A has a new ODL tree, and a new data_ptr
                and size.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   7 Nov  1994
  Last Modified:   7 Nov  1994

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:  
         table_A - A pointer to an Oa_Object structure with an in-memory
                   TABLE-like object.

         table_B - A pointer to an Oa_Object structure with an in-memory
                   TABLE-like object.

         option  - OA_ADD_ROWS or OA_ADD_COLUMNS, defined in oal.h.

  Output:  If successful, the function returns the table_A input argument,
           otherwise NULL.  The data pointed to by table_B->data_ptr has been 
           added to the data pointed to by table_A->data_ptr, and table_A's
           ODL tree has been copied and updated.  table_A's old data_ptr has
           been freed.  Both the data and ODL tree for table_B are unchanged.

  Notes:  
  1) The two tables must have the same interchange format.
  2) If option was OA_ADD_COLUMNS, then the SDT which appends table_B to
     table_A uses table_A->profile;  if table_B's profile isn't the same as
     table_A's, then data conversions may be done to table_B's columns.
  3) WARNING! On output, table_A->odltree points to a new tree, and the old
     one has been freed.  Any saved pointers the caller may have to nodes in 
     the old tree now point to freed memory!

*****************************************************************************/

#ifdef _NO_PROTO

OA_OBJECT OaJoinTables( table_A, table_B, option)
OA_OBJECT table_A;
OA_OBJECT table_B;
int option;

#else

OA_OBJECT OaJoinTables( OA_OBJECT table_A, OA_OBJECT table_B, int option)

#endif
{

static char *proc_name = "OaJoinTables";
OA_OBJECT oa_object;
ODLTREE current_node, odltreenode, sdt, compressed_SDT;
long rows_A, rows_B, row_bytes_A, row_bytes_B, columns_A, columns_B;
long prefix_bytes, suffix_bytes, new_object_size, start_byte, i;
PTR data_ptr;
char c;
SDT_node *sdt_node_ptr;
struct oa_profile save_profile;
int src_interchange_format, dst_interchange_format, table_storage_type;
int src_interchange_format_A, src_interchange_format_B;

/* Check for valid inputs. */

if ((option != OA_ADD_ROWS) && (option != OA_ADD_COLUMNS)) {
  sprintf( error_string, "%s: \"option\" argument is invalid.", proc_name);
  oa_errno = 502;
  OaReportError( error_string);
  return( NULL);
}

/* Loop through the two tables, checking each one.  */

for (i=0; i<2; i++) {

  if (i == 0) oa_object = table_A;
  else oa_object = table_B;

  if (oa_object == NULL) {
    sprintf( error_string, "%s: an input oa_object is NULL.", proc_name);
    oa_errno = 501;
    OaReportError( error_string);
    return( NULL);
  }

  if (oa_object->odltree == NULL) {
    sprintf( error_string, 
             "%s: an input oa_object's ODL tree pointer is NULL.", 
             proc_name);
    oa_errno = 501;
    OaReportError( error_string);
    return( NULL);
  }

  if (oa_object->data_ptr == NULL) {
    sprintf( error_string, "%s: an input oa_object's data_ptr is NULL.", 
             proc_name);
    oa_errno = 501;
    OaReportError( error_string);
    return( NULL);
  }

  odltreenode = oa_object->odltree;

  if (OaGetTableKeywords( odltreenode, &rows_B, &row_bytes_B, &prefix_bytes, 
                          &suffix_bytes, &src_interchange_format_B,
                          &table_storage_type) != 0)
    return( NULL);  /* Error message already written and oa_errno set.  */

  if (OaKwdValuetoLong( "COLUMNS", odltreenode, &columns_B) != 0) {
    sprintf( error_string, "%s: couldn't find COLUMNS keyword.", proc_name);
    oa_errno = 531;
    OaReportError( error_string);
    return( NULL);
  }
  if (OdlGetObjDescChildCount( odltreenode) != columns_B) {
    sprintf( error_string, "%s: child count is not the same as COLUMNS.",
             proc_name);
    oa_errno = 530;
    OaReportError( error_string);
    return( NULL);
  }  

  if (table_storage_type != OA_ROW_MAJOR) {
    sprintf( error_string, "%s: %s's TABLE_STORAGE_TYPE must be ROW_MAJOR.",
             proc_name, OdlGetObjDescClassName( odltreenode));
    oa_errno = 520;
    OaReportError( error_string);
    return( NULL);
  }

  if (oa_object == table_A) {
    rows_A      = rows_B;
    row_bytes_A = row_bytes_B;
    columns_A   = columns_B;
    src_interchange_format_A = src_interchange_format_B;
  }

  if (src_interchange_format_A != src_interchange_format_B) {
    sprintf( error_string, 
    "%s: table_A and table_B must have the same interchange_format.",
            proc_name);
    oa_errno = 520;
    OaReportError( error_string);
    return( NULL);
  }
}

/* For OA_ADD_ROWS, realloc table_A's data and copy table_B's data after
   table_A's data, and adjust the ROWS keyword in table_A's ODL tree.  */

if (option == OA_ADD_ROWS) {

  if ((row_bytes_A != row_bytes_B) || (columns_A != columns_B)) {
    sprintf( error_string, 
             "%s: input tables must have same ROW_BYTES and COLUMNS.",
             proc_name);
    oa_errno = 520;
    OaReportError( error_string);
    return( NULL);
  }

  if ((table_A->data_ptr = (PTR) OaRealloc( (char *) table_A->data_ptr, 
                                            table_A->size,
                                    table_A->size + table_B->size)) == NULL) {
    sprintf( error_string, "%s: OaRealloc failed! Out of memory!", proc_name);
    oa_errno = 720;
    OaReportError( error_string);
    exit(1);
    /*NOTREACHED*/
  }

#ifdef IBM_PC

/* If we're on an IBM-PC, use a for loop to do the copy, since memcpy
   may not work for copies greater than 64K and/or over segment
   boundaries;  otherwise use memcpy since it's usually optimized.  */

for (i=0; i<table_B->size; i++)
  table_A->data_ptr[ table_A->size + i] = table_B->data_ptr[i];
#else

  memcpy( table_A->data_ptr + table_A->size,  
          table_B->data_ptr, table_B->size);
#endif

  table_A->size += table_B->size;
  OaLongtoKwdValue( "ROWS", table_A->odltree, rows_A + rows_B);
  return( table_A);
}


/* For OA_ADD_COLUMNS, build an SDT which describes the final table, with
   table_B's columns following table_A's columns;  OalProcessSDT will be fed
   alternately a row of table_A and a row of table_B to build up the final
   table.  */

if (rows_A != rows_B) {
  sprintf( error_string, 
           "%s: input table_A rows: %ld must equal table_B rows: %ld.",
           proc_name, rows_A, rows_B);
  oa_errno = 520;
  OaReportError( error_string);
  return( NULL);
}

sdt = OaCopyTree( table_A->odltree, OA_STRIP_SDT_NODES);

/* Concatenate table_B's columns after table_A's columns, and add row_bytes_A 
   to the START_BYTE keyword value of every table_B column.  */

current_node = LeftmostChild( table_B->odltree);
while (current_node != NULL) {
  odltreenode =  OaCopyTree( current_node, OA_STRIP_SDT_NODES);
  if (OaKwdValuetoLong( "START_BYTE", odltreenode, &start_byte) != 0) {
    sprintf( error_string, 
             "%s: table_B node: %s is missing START_BYTE keyword.", 
             proc_name, OdlGetObjDescClassName( odltreenode));
    oa_errno = 531;
    OaReportError( error_string);
    return( NULL);
  }
  start_byte += row_bytes_A;
  OaLongtoKwdValue( "START_BYTE", odltreenode, start_byte);
  OdlPasteObjDesc( odltreenode, sdt);
  current_node = RightSibling( current_node);                               
}

/* Set the profile for OalCreateSDT to be table_A's profile (the profile
   when table_A was created), and force keeping the same interchange format.
   Restore the global profile after creating the SDT.   */

src_interchange_format = src_interchange_format_A;
dst_interchange_format = src_interchange_format;
save_profile = Oa_profile;
Oa_profile = table_A->profile;
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

/* Get the size of the new, appended table (not known before SDT creation
   because of the possible addition of alignment padding), adjust a
   few keyword values, and allocate space for the appended table's data.  */

sdt_node_ptr = (SDT_node *) sdt->appl1;
new_object_size = sdt_node_ptr->dst.size * sdt_node_ptr->total_repetitions;
OaLongtoKwdValue( "ROW_BYTES", sdt, sdt_node_ptr->dst.size);
OaLongtoKwdValue( "COLUMNS", sdt, (long) OdlGetObjDescChildCount( sdt));

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

/* Compress the SDT, and initialize current_node to point to the first
   end-node (data node) in the compressed SDT; initialize the node's dst.ptr
   with the pointer returned by OaMalloc.  */

compressed_SDT = OalCompressSDT( sdt);
current_node = OalInitializeSDT( compressed_SDT, data_ptr);

/* Alternately feed the SDT a row of table_A, then a row of table_B until
   reach the end of both tables (the end of the SDT).  */

for (i=0; i<rows_A; i++) {
  if (OalProcessSDT( table_A->data_ptr + i*row_bytes_A, row_bytes_A, 
                     &current_node) != OA_READY_FOR_NEXT_SLICE) {
    if (i != (rows_A-1)) {
      sprintf( error_string, 
               "%s: OalProcessSDT reached end of SDT before end of buffer",
               proc_name);
      OaReportError( error_string);
      OaFree( (char *) data_ptr);
      OalFreeSDT( sdt);
      OalFreeSDT( compressed_SDT);
      return(NULL);
    }
  }
  if (OalProcessSDT( table_B->data_ptr + i*row_bytes_B, row_bytes_B, 
                     &current_node) != OA_READY_FOR_NEXT_SLICE) {
    if (i != (rows_A-1)) {
      sprintf( error_string, 
               "%s: OalProcessSDT reached end of SDT before end of buffer",
               proc_name);
      OaReportError( error_string);
      OaFree( (char *) data_ptr);
      OalFreeSDT( sdt);
      OalFreeSDT( compressed_SDT);
      return(NULL);
    }
  }
} /* end for loop doing SDT processing on each row */

/* Free the compressed SDT. */

OalFreeSDT( compressed_SDT);

/* OalSDTtoODLTree strips off ODL tree nodes which have SDT_node->dst.size=0
   (spare nodes added for alignment).  In this case it doesn't update the 
   DATA_TYPE keywords, since there probably weren't any conversions.  */

OalSDTtoODLTree( sdt, dst_interchange_format);

/* Replace table_A's ODL tree with the new ODL tree, free table_A's old
   data_ptr, and replace it with the new data_ptr.  Return table_A.  */

OdlFreeTree( table_A->odltree);
table_A->odltree = sdt;
OaFree( (char *) table_A->data_ptr);
table_A->data_ptr = data_ptr;
table_A->size = new_object_size;

return( table_A);
}



/*****************************************************************************

  Routine:  OaNewOaObject

  Description:  This routine allocates a new Oa_Object structure and
                initializes it.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:   1 Sept  1994

  History:

    Creation - This routine was part of the Alpha Release of the OA library.

  Input:  None.
         
  Output:  The routine returns a pointer to the Oa_Object structure.

  Notes:  

*****************************************************************************/

OA_OBJECT OaNewOaObject() 
{
static char *proc_name = "OaNewOaObject";
OA_OBJECT oa_object;

if ((oa_object = (OA_OBJECT) OaMalloc( sizeof( struct OA_Object))) == NULL) {
  sprintf( error_string, "%s: OaMalloc returned error! Out of memory!", 
           proc_name);
  oa_errno = 720;
  OaReportError( error_string);
  exit(1);
  /*NOTREACHED*/
} else {
  oa_object->odltree      = NULL;
  oa_object->data_ptr     = NULL;
  oa_object->size         = 0L;
  oa_object->stream_id    = NULL;
  oa_object->is_in_memory = FALSE;
  oa_object->appl1        = NULL;
  return( oa_object);
}
/*NOTREACHED*/
return( NULL);
}

