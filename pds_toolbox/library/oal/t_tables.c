/* This program demonstrates use of the OAL library's TABLE functions.
   The two files SGC_0031.LBL and SGC_0031.DAT must be present.

   Platforms this has been tested on are:
   1) SGI/Irix                     (miranda)
   2) VAX/VMS                      (aries)
   3) Sun Sparc/SunOS and Solaris  (mirza and rescha)
   4) Dec Alpha/OSF/1              (maverick)
   5) Macintosh                    (Mac T01, Power PC, emulation mode)
   6) Dec 3100/Ultrix              (syrtis)
   7) Dec Alpha/OpenVMS            (ringside)
   8) IBM PC/DOS                   (VESTA2)
*/

#include "oal.h"
#include <string.h>
#include <stdio.h>

main() {

OA_OBJECT oa_object, table_A, table_B, file_object, tmp_object;
ODLTREE root_node, odltreenode, table_node, column_node, column_nodes[10];
char *label_filename, *errfilespec;
float *column_values1, *column_values2;
int i, *table_values;
char c, *ptr, *data_type;
long row_bytes;

/* Read in the label from SGC_0031.LBL.  This contains two binary TABLES
   with VAX data types - orbit/pointing info for PV/OUVS orbit #31.  */

label_filename = "SGC_0031.LBL";
sprintf( error_string, "Reading %s...", label_filename);
OaReportError( error_string);
if ((root_node = OaParseLabelFile( label_filename, NULL, 0, 0)) == NULL) {
  OaReportError( "Error from OaParseLabelFile");
  return(0);
}

/* Find the SGC_TABLE node.  */

if ((table_node = OdlFindObjDesc( root_node, "SGC_TABLE", NULL, NULL, 0,
                                  ODL_RECURSIVE_DOWN)) == NULL) {
  OaReportError( "Couldn't find SGC_TABLE node!");
  return(0);
}

/* Get the VENUS_SC_VECTOR column of the TABLE into memory two different ways.
   #1: Read in the entire table, then extract the VENUS_SC_VECTOR column. */

if ((table_A = OaReadObject( table_node)) == NULL) {
  OaReportError( "OaReadObject failed!");
  return(1);
}

/* Find the VENUS_SC_VECTOR column node (in the ODL tree of the in-memory
   table).  */

if ((column_node = OdlFindObjDesc( table_A->odltree, "COLUMN",
                                  "NAME", "VENUS_SC_VECTOR", 0,
                                   ODL_RECURSIVE_DOWN)) == NULL) {
  OaReportError( "Couldn't find VENUS_SC_VECTOR column!");
  return(1);
}

column_nodes[0] = column_node;

OaReportError( "OaGetSubTable extracting VENUS_SC_VECTOR column...");

if ((table_B = OaGetSubTable( table_A, (long) 1, (long) 256,
                                    column_nodes, 1)) == NULL) {
  OaReportError( "OaGetSubTable failed!");
  return(1);
}

/* Free table_A.  */

OaDeleteObject( table_A);

/* Method #2: read in only the VENUS_SC_VECTOR with OaReadSubTable.  */

if ((column_node = OdlFindObjDesc( root_node, "COLUMN",
                                  "NAME", "VENUS_SC_VECTOR", 0,
                                   ODL_RECURSIVE_DOWN)) == NULL) {
  OaReportError( "Couldn't find VENUS_SC_VECTOR column!");
  return(1);
}

column_nodes[0] = column_node;

OaReportError( "OaReadSubTable reading in VENUS_SC_VECTOR...");

if ((table_A = OaReadSubTable( table_node, (long) 1, (long) 256,
                                     column_nodes, 1)) == NULL) {
  OaReportError( "OaReadSubTable failed!");
  return(1);
}

/* Print out a few values to see if it worked.  */

column_values1 = (float *) table_A->data_ptr;
column_values2 = (float *) table_B->data_ptr;
OaReportError( "First 3 values should be:  5940.21    -729.04   10895.34");
sprintf( error_string, "First 3 values are:     %10.2f %10.2f %10.2f",
         column_values1[0], column_values1[1], column_values1[2]);
OaReportError( error_string);


/* Compare every element in the two tables. */

for (i=0; i<(3*256); i++) {
  if (column_values1[i] != column_values2[i]) {
    sprintf( error_string, "columns don't match: i = %d\n", i);
    OaReportError( error_string);
    return(1);
  }
}
OaReportError( "column data matches.");

OaDeleteObject( table_B);

/* Convert the object data to ASCII.  */

Oa_profile.dst_format_for_binary_src = OA_ASCII_INTERCHANGE_FORMAT;
printf( "Converting column data to ASCII...\n");
if ((table_B = OaConvertObject( table_A)) == NULL) {
  OaReportError( "OaConvertObject failed!");
  return(1);
}

/* Print out first 40 chars of the ASCII data.  */

ptr = (char *) table_B->data_ptr;
c = ptr[40];
ptr[40] = '\0';
sprintf( error_string, "First 40 chars of ASCII tables are: %s", ptr);
OaReportError( error_string);
ptr[40] = c;

/* Append CR/LF to each row of the ASCII table.  */

if (OaAddLineTerminatorstoTable( table_B) == NULL) {
  printf( "OaAddLineTerminatorstoTable failed!\n");
  return(1);
}

/* Open output file. */

if ((file_object = OaOpenOutputFile( "TMP_TABLE.DAT", OA_STREAM,
                                      0)) == NULL) {
  OaReportError( "OaOpenOutputFile failed!");
  return(1);
}

/* Write the ASCII table to the output file.  */

if (OaWriteObject( file_object, table_B) != 0) {
  OaReportError( "OaWriteObject failed!");
  return(1);
}

/* Close the output file and write the label.  */

if (OaCloseOutputFile( file_object, "TMP_TABLE.LBL") != 0) {
  OaReportError( "OaCloseOutputFile failed!");
  return(1);
}
OaReportError( 
        "Type out TMP_TABLE.DAT and TMP_TABLE.LBL and verify contents.");

/* Import two columns and slice and dice them.  */

table_values = (int *) OaMalloc( sizeof(int)*3);
table_values[0] = 11;
table_values[1] = 21;
table_values[2] = 31;

#if (defined(ALPHA_VMS) || defined(ALPHA_OSF) || defined(VAX) || defined(IBM_PC) || defined(ULTRIX))
data_type = "LSB_INTEGER";
#else
data_type = "MSB_INTEGER";
#endif


OaReportError( "Calling OaImportColumn...");
table_A = OaImportColumn( (char *) table_values, (long) 3, (long) 1,
                          (long) sizeof( int), data_type, 
                          OA_BINARY_INTERCHANGE_FORMAT, "FIRST_COLUMN");
if (table_A == NULL) {
  OaReportError( "OaImportColumn failed!");
  return(1);
}

table_values = (int *) OaMalloc( sizeof(int)*3);
table_values[0] = 12;
table_values[1] = 22;
table_values[2] = 32;
OaReportError( "Calling OaImportColumn...");
table_B = OaImportColumn( (char *) table_values, (long) 3, (long) 1,
                          (long) sizeof( int), data_type, 
                          OA_BINARY_INTERCHANGE_FORMAT, "SECOND_COLUMN");
if (table_B == NULL) {
  OaReportError( "OaImportColumn failed!");
  return(1);
}

/* Join the two TABLES, appending COLUMNS, and print out the new table's
   data.  */

OaReportError( "Calling OaJoinTables...");
if (OaJoinTables( table_A, table_B, OA_ADD_COLUMNS) == NULL) {
  OaReportError( "OaJoinTables failed!");
  return(1);
}

OaDeleteObject( table_B);

OaReportError( "TABLE should be row-major, with three rows and two columns:");
table_values = (int *) table_A->data_ptr;
sprintf( error_string, 
        "%d %d\n%d %d\n%d %d\n\n", table_values[0], table_values[1],
        table_values[2], table_values[3], table_values[4], table_values[5]);
OaReportError( error_string);

/* Transpose the TABLE to COLUMN_MAJOR and print out its data.  */

OaReportError( "Calling OaTransposeTable...");

if (OaTransposeTable( table_A) == NULL) {
  OaReportError( "OaTransposeTable failed!");
  return(1);
}

OaReportError( "TABLE should now be column-major:");
table_values = (int *) table_A->data_ptr;
sprintf( error_string,
        "%d %d %d\n%d %d %d\n\n", table_values[0], table_values[1],
        table_values[2], table_values[3], table_values[4], table_values[5]);
OaReportError( error_string);

/* Transpose the TABLE back to ROW_MAJOR and print out its data.  */

OaReportError( "Calling OaTransposeTable again...");

if (OaTransposeTable( table_A) == NULL) {
  OaReportError( "OaTransposeTable failed!");
  return(1);
}

OaReportError( "TABLE should be row-major, with three rows and two columns:");
table_values = (int *) table_A->data_ptr;
sprintf( error_string, 
         "%d %d\n%d %d\n%d %d\n\n", table_values[0], table_values[1],
        table_values[2], table_values[3], table_values[4], table_values[5]);
OaReportError( error_string);

/* Delete the middle row and print out the data.  */

OaReportError( "Calling OaDeleteRow to delete second row...");
if (OaDeleteRow( table_A, (long) 2) == NULL) {
  OaReportError( "OaDeleteRow failed!");
  return(1);
}

OaReportError( "TABLE should have first and third rows and two columns:");
table_values = (int *) table_A->data_ptr;
sprintf( error_string, "%d %d\n%d %d\n\n", table_values[0], table_values[1],
         table_values[2], table_values[3]);
OaReportError( error_string);

/* Delete the first column.  */

if ((column_node = OdlFindObjDesc( table_A->odltree, "COLUMN",
                                  "NAME", "FIRST_COLUMN", 0,
                                   ODL_RECURSIVE_DOWN)) == NULL) {
  OaReportError( "Couldn't find FIRST_COLUMN column!");
  return(1);
}

OaReportError( "Calling OaDeleteColumn to delete first column...");
if (OaDeleteColumn( table_A, column_node) == NULL) {
  OaReportError( "OaDeleteColumn failed!");
  return(1);
}

OaReportError( "TABLE should have first and third rows and second column:");
table_values = (int *) table_A->data_ptr;
sprintf( error_string, "%d\n%d\n\n", table_values[0], table_values[1]);
OaReportError( error_string);

OaReportError( "All tests worked!");
return(0);
}



