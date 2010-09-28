/*****************************************************************************

  File:  oal.h

  Description: This file is the main OAL include file, the only one needed
               by most user applications.  Data structures, enumerated
               types, global variables and function declarations for the
               Object Layer and Structure Layer are made.  The companion file, 
               oal.c, defines and initializes the global variables declared
               here.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:  16 Mar   1998  This is OAL Version 1.3

  History:

    Creation - This include file was part of the Alpha Release of the
               OA library.
    12/06/94 - Split off Stream Layer declarations into stream_l.h.  SM
    11/27/95 - Added OA_QUBE and prototypes for OaReadImageFromQube and
               OaReadSpectrumFromQube.  SM
    11/27/95 - Added OA_CLEMENTINE_JPEG to oa_compression_types_enum.  SM
    12/02/95 - Changed input argument and return type of OaMalloc, OaRealloc,
               OaFree from PTR to char *.  SM
    12/06/95 - Moved prototypes for OaMalloc, OaFree and OaRealloc to 
               oamalloc.c, and #included oamalloc.h below.  SM
    12/11/95 - Added declaration for oa_errno.  SM
    02/29/96 - Added prototype for OaImportHistogram. SM
    03/21/96 - Changed enum value of OA_ALPHA_VMS to 3 to access new Dec Alpha
               OpenVMS data types profile.  SM
    04/22/96 - Added native_binrep_descrip to OA_ATOM_INFO structure to support
               new multi-step conversions.  SM
    04/25/96 - Added prototype for OalGetNativeCTypeInfo, added 3 to
               OA_PROFILED_DATA_TYPES after adding 3 new types to data types
               profile (oal.c).  SM
    12/11/96 - Added support for multiband images; added new 'band' argument
               to prototypes of OaOpenImage, OaReadImage, added prototype for
               OaReadSpectrumFromImage.  SM
    04/16/97 - Added OA_ASCII_BIT_STRING to oa_PDS_data_types_enum.
               Added prototype for OalFreeSDTStruct.
               Added 3 to OA_PROFILED_DATA_TYPES after adding 3 new types to
               data types profile (oal.c).  SM
    04/21/97 - Added prototypes for LeftmostSDTChild, LeftSDTSibling,
               RightmostSDTChild, RightSDTSibling.  SM
    06/24/97 - Added OA_BCD to oa_PDS_data_types_enum.
               Added 1 to OA_PROFILED_DATA_TYPES after adding 1 new type to
               data types profile (oal.c).  SM
    03/16/98 - Added OA_GIF encoding type and prototypes for new functions
               OalCreateODLTreeFromGif and OalReadImageFromGif.  SM

*****************************************************************************/

#ifndef OA_OAL_INCLUDED
#define OA_OAL_INCLUDED 1

#include "oal_sys.h"
#include <stdio.h>
#include <stdlib.h>
#if (defined( VAX) || defined( ALPHA_VMS))
#include <stat.h>
#include <rms.h>
#include <unixio>
#include <file>
#else
#ifdef MAC
#include <unix.h>
#include <stat.h>
#else
#include <sys/stat.h>
#endif
#endif
#include "odlutils.h"
#include "stream_l.h"
#include "binrep.h"
#include "oamalloc.h"


/* This enum maps platform names into integers, which are used to index the 
   Oa_type_conversion_info array, thus determining which data type conversion
   profile is used.  One of these enum values is in the
   data_translation_profile field of global variable Oa_profile.  Some of the
   enum values are the same, for example OA_ALPHA_OSF, OA_ULTRIX, OA_IBM_PC, 
   because these platforms have identical binary data types. */
enum oa_data_type_profiles_enum {OA_ALPHA_OSF     = 2, 
                                 OA_ALPHA_VMS     = 3,
                                 OA_IBM_PC        = 2,
                                 OA_MAC_IEEE      = 1,
                                 OA_SGI           = 1,
                                 OA_SUN3          = 1,
                                 OA_SUN4          = 1,
                                 OA_ULTRIX        = 2,
                                 OA_VAX           = 0};

/* The global variable Oa_profile is this structure.  It is initialized at
   compile time for the particular platform being run on by #ifdefs in oal.c,
   and is thereafter never modified by the OA Library code.  The user can
   modify it to make OA create different kinds of SDT's.  For example, an SDT
   created to read a binary file into memory will convert the data to ASCII if 
   dst_format_for_binary_src = OA_ASCII_INTERCHANGE_FORMAT, or leave it
   binary and do binary-to-binary conversions as specified in
   data_translation_profile if dst_format_for_binary_src = 
   OA_BINARY_INTERCHANGE_FORMAT.  */
struct oa_profile {
  char dst_format_for_ASCII_src; /* OA_ASCII_INTERCHANGE_FORMAT or           */
  char dst_format_for_binary_src;/* OA_BINARY_INTERCHANGE_FORMAT             */
  char dst_alignment_type;       /* OA_NO_ALIGN, OA_ALIGN_EVEN, OA_ALIGN_RISC*/
  char data_translation_profile; /* This is used to index the oa_type_conver-*/
                                 /* sion_info array; the index is a          */
                                 /* oa_data_type_profiles_enum value.        */
                                 /* The element in oa_type_conversion_info   */
                                 /* which is indexed is itself an array of   */
                                 /* oa_type_conversion_info structures.      */
  char check_ASCII_writes;       /* If TRUE, OaWriteObject will return an    */
                                 /* error if the input object's object-      */
                                 /* interchange_format is ASCII, and the     */
                                 /* object data contains any non-alpha-      */
                                 /* numeric data (except allowed control     */
                                 /* characters).                             */
};


/* This is the "oa_object" structure used in the Object Layer, consisting of a
   pointer to the object's data and a pointer to the ODL tree describing the
   data.  */
typedef struct OA_Object {
  ODLTREE odltree;                   /* Pointer to an ODL tree whose root is */
                                     /* a byte-specified PDS object class:   */
                                     /* TABLE, IMAGE etc. NOT BIT_COLUMN.    */
  PTR     data_ptr;                  /* Pointer to the object's data and the */
                                     /* data size in bytes.  data_ptr is NULL*/
  long    size;                      /* and size is 0 if is_in_memory=FALSE. */
  struct OaStreamStruct *stream_id;  /* If NULL, no associated file.         */
  int     is_in_memory;              /* If TRUE, data object described by    */
                                     /* odltree is in memory.                */
  void    *appl1;                    /* Points to an oa_image_handle struct  */
                                     /* in OaOpenImage and OaReadImagePixels.*/
  struct oa_profile profile;         /* The profile which was used when the  */
                                     /* object data was created or last      */
                                     /* modified.                            */
} *OA_OBJECT;                        /* typedef OA_OBJECT is a pointer to    */
                                     /* this struct.                         */

/* These structures are part of the oa_image_handle structure, which stores
   information about a partial image.  It is used in Object Layer functions
   OaOpenImage and OaReadImagePixels.  */
struct oa_PP_struct {
  short previous_pixel;
  char  found_255;
  char  previous_byte_exists;
  char  previous_byte;
};
struct oa_HFD_struct {
  void  *decoding_tree;
  char  *line_buffer;
  long  line_buffer_bytes;
};
struct oa_image_handle {
  long  image_start_offset;    /* Start offset of the image in the file,     */
                               /* returned by OaGetFileKeywords.             */
  char  *buf;                  /* Points to buffer containing most recently  */
                               /* decompressed and converted image line.     */
  long  buf_siz;               /* Size of buf, determined by OaOpenImage.    */
  long  next_line;             /* next_line and next_sample give the line    */
  long  next_sample;           /* and sample the next read will return, i.e. */
                               /* the line and sample the file pointer is    */
                               /* positioned at.  Both start at 1.           */
  long  buf_samples;           /* The number of samples (pixels) in buf.     */
  long  source_lines;          /* The number of lines in the source image.   */
  long  source_line_samples;   /* The number of samples in each line of the  */
                               /* source image.                              */
  int   bands;                 /* The number of bands in a multi-band image. */
  int   band;                  /* The band currently being read.             */
  int   band_storage_type;     /* OA_BAND_SEQUENTIAL, OA_LINE_INTERLEAVED,   */
                               /* OA_SAMPLE_INTERLEAVED                      */
  char  sample_bytes;          /* The number of bytes in each sample value.  */
  char  compression_type;      /* OA_UNCOMPRESSED, OA_PREVIOUS_PIXEL,        */
                               /* OA_HUFFMAN_FIRST_DIFFERENCE                */
  union {
    struct oa_PP_struct  PP;
    struct oa_HFD_struct HFD;
  } decomp;
};

/* This structure specifies how OA will translate a given PDS data type into
   equivalent native binary and ASCII data types.  An array of these 
   structures contains translation specifications from all the numeric PDS 
   data types to the specific native data types on the particular platform 
   being run on; this array is part of the profile for the platform.  */
struct oa_type_conversion_info {
  char *binrep_q_code;       /* The first 3 fields specify the PDS data type */
  char PDS_data_type;        /* to be translated. size and binrep_q_code are */
  char size;                 /* only used for binary types.                  */
  char *native_binrep_q_code;/* The binary PDS data type to convert to; its  */
  char native_PDS_data_type; /* binrep q-code, PDS_data_type and size.       */
  char native_size;          
  char ASCII_PDS_data_type;  /* The ASCII PDS data type to convert to, and   */
  char ASCII_size;           /* its field width.  This field width can be    */
                             /* overridden by a FORMAT keyword.              */
};


/* This structure stores alias translations for PDS data types.  An array of 
   these structures contains all the data type aliases as read from the
   PDS Data Dictionary (not yet, hard-coded for now).  */
struct oa_alias_translations {
  char *aliased_name;
  char *translated_name;
};

/* This enumeration type is used in the "option" argument to OaJoinTables(). */
enum oa_join_tables_enum { OA_ADD_ROWS = 0, OA_ADD_COLUMNS = 1};

/* This enumeration type is used in many TABLE functions.  */
enum oa_table_storage_type_enum { OA_UNKNOWN_TABLE_STORAGE_TYPE = 0,
                                  OA_ROW_MAJOR = 1, OA_COLUMN_MAJOR = 2};


/* This enumeration specifies the kinds of data conversions or copies OAL 
   can do.  One of these values is stored in the conversion_type field of 
   each SDT node (Structure Layer).  */
enum oa_conversion_types_enum { OA_NOT_APPLICABLE, OA_BINREP, OA_MEMCPY,
                                OA_ASCII_TO_BINARY, OA_BINARY_TO_ASCII};

/* These are all the different alignment regimes for the platforms OAL
   runs on.  One of these values is stored in the dst_alignment_type field of
   the global variable Oa_profile, and is used in the Structure Layer.  */
enum oa_alignment_type_enum { OA_NO_ALIGN, OA_ALIGN_EVEN, OA_ALIGN_RISC};

/* These are the different interchange formats. */
enum oa_interchange_format_enum { OA_UNKNOWN_INTERCHANGE_FORMAT = 0,
                                  OA_ASCII_INTERCHANGE_FORMAT,
                                  OA_BINARY_INTERCHANGE_FORMAT};

/* These are the image compression types.  */
enum oa_compression_types_enum { OA_UNKNOWN_ENCODING_TYPE = 0,
                                 OA_UNCOMPRESSED,
                                 OA_HUFFMAN_FIRST_DIFFERENCE,
                                 OA_PREVIOUS_PIXEL,
                                 OA_CLEMENTINE_JPEG,
                                 OA_GIF};
                           
/* These are the multi-band image and qube storage types.  */
enum oa_band_storage_types_enum { OA_UNKNOWN_BAND_STORAGE_TYPE = 0,
                                  OA_BAND_SEQUENTIAL,
                                  OA_LINE_INTERLEAVED,
                                  OA_SAMPLE_INTERLEAVED};

/* These are the object classes OA knows about.  */
enum oa_object_class_enum { OA_UNKNOWN_CLASS, OA_ALIAS, OA_ARRAY, 
                            OA_BIT_COLUMN, OA_BIT_ELEMENT, 
                            OA_COLLECTION, OA_COLUMN, OA_CONTAINER,
                            OA_ELEMENT, OA_GAZETTEER, OA_HISTOGRAM,  
                            OA_HISTORY, OA_IMAGE, OA_PALETTE, OA_QUBE,
                            OA_SERIES, OA_SPECTRUM, OA_TABLE};

/* These are all the PDS data types and aliases from the November 20, 1992 
   PDS Standards Reference, Table 3.2 - PDS Standard Data Types.  Note:
   the enum values for aliased data types should be set to the same value as
   the unaliased data type. */
enum oa_PDS_data_types_enum {OA_ASCII_REAL           =  99,
                             OA_ASCII_INTEGER        =  1,
                             OA_ASCII_BIT_STRING     =  28,
                             OA_ASCII_COMPLEX        =  2,
                             OA_BCD                  =  29,
                             OA_BIT_STRING           =  3,
                             OA_BOOLEAN              =  4,
                             OA_CHARACTER            =  5,
                             OA_COMPLEX              =  6,
                             OA_DATE                 =  7,
                             OA_FLOAT                =  8,
                             OA_IBM_COMPLEX          =  9,
                             OA_IBM_INTEGER          = 10,
                             OA_IBM_REAL             = 27,
                             OA_IBM_UNSIGNED_INTEGER = 11,
                             OA_IEEE_COMPLEX         =  6,
                             OA_IEEE_REAL            =  8,
                             OA_INTEGER              = 16,
                             OA_LSB_BIT_STRING       = 13,
                             OA_LSB_INTEGER          = 14,
                             OA_LSB_UNSIGNED_INTEGER = 15,
                             OA_MAC_COMPLEX          =  6,
                             OA_MAC_INTEGER          = 16,
                             OA_MAC_REAL             =  8,
                             OA_MAC_UNSIGNED_INTEGER = 17,
                             OA_MSB_BIT_STRING       =  3,
                             OA_MSB_INTEGER          = 16,
                             OA_MSB_UNSIGNED_INTEGER = 17,
                             OA_PC_COMPLEX           = 18,
                             OA_PC_INTEGER           = 14,
                             OA_PC_REAL              = 19,
                             OA_PC_UNSIGNED_INTEGER  = 15,
                             OA_REAL                 =  8,
                             OA_SUN_COMPLEX          =  6,
                             OA_SUN_INTEGER          = 16,
                             OA_SUN_REAL             =  8,
                             OA_SUN_UNSIGNED_INTEGER = 17,
                             OA_TIME                 = 20,
                             OA_UNSIGNED_INTEGER     = 17,
                             OA_VAX_BIT_STRING       = 13,
                             OA_VAX_COMPLEX          = 21,
                             OA_VAX_DOUBLE           = 22,
                             OA_VAX_INTEGER          = 14,
                             OA_VAX_REAL             = 22,
                             OA_VAX_UNSIGNED_INTEGER = 15,
                             OA_VAXG_COMPLEX         = 25,
                             OA_VAXG_REAL            = 26,
                             OA_UNKNOWN_DATA_TYPE    = 0};

/* These are return status values from the OalProcessSDT function.  */
enum oa_process_slice_enum { OA_READY_FOR_NEXT_SLICE, OA_REACHED_END_OF_SDT};

/* These are used by the OalPostOrderTraverse function.  */
enum oa_operation_enum { OA_BUILD_SDT_NODE, OA_ADJUST_KWDS_TO_MATCH_SDT};

/* This is the part of a Stream Decomposition Tree node which describes 
   the source or destination for a data transfer.  An SDT node has two of
   these nested in it: one for the source of the transfer, the other for the
   destination of the transfer.  SDT's are used in the Structure Layer.  */
typedef struct oa_atom_info {
  long start_offset;          /* The offset in bytes of the start of the     */
                              /* first repetition of a data atom from its    */
                              /* parent; static, set when the tree is        */
                              /* created.                                    */
  long bytes_processed;       /* Bytes of the atom which have been processed */
                              /* during the current repetition; dynamic.     */
  long size;                  /* Size of a single repetition of the atom in  */
                              /* bytes; static, set when the tree is created.*/
  PTR  ptr;                   /* Points to the start of current repetition   */
                              /* of the SDT node's src or dst data; dynamic, */
                              /* set by PositiontoNextDataNode.  Exception:  */
                              /* dst.ptr (reads) or src.ptr (writes) of the  */
                              /* root of the SDT points to the beginning of  */
                              /* the object in memory and is static.         */
  char PDS_data_type;         /* An oa_PDS_data_types_enum value.            */
  struct binrep_desc *binrep_descrip;  /* Pointer into the static array of   */
                              /* binrep structs.  See binrep.h for details.  */
                              /* Only used when the atom is a binary number. */
  char format_spec[15];       /* The FORMAT keyword value, translated into   */
                              /* equivalent C format specification string.   */
                              /* Used only for binary-to-ASCII conversions.  */
  char alignment_req;         /* 1,2,4 or 8; the memory address on which the */
                              /* atom data must be aligned. 0 means not      */
                              /* applicable (repetitions nodes without data).*/
} OA_ATOM_INFO;


/* This is the structure definition for an SDT node.  */
typedef struct Stream_Decomposition_Tree_Node {
  long total_repetitions;  /* This is the number of times to loop through    */
                           /* all this node's children; static, set when the */
                           /* tree is created.                               */
  long current_reps;       /* Running count of repetitions currently done    */
                           /* during stream processing; dynamic.             */
  OA_ATOM_INFO src;        /* Describes the source data for the transfer.    */
  OA_ATOM_INFO dst;        /* Describes the destination for the transfer.    */
  char conversion_type;    /* OA_BINREP, OA_ASCII_TO_BINARY, OA_MEMCPY etc.  */
  long buf_bytes;          /* The number of bytes currently in buf.          */
  char *buf;               /* Points to the same buffer for every node, so   */
                           /* may not be needed here... used to store a      */
                           /* partial atom of data when the atom is broken   */
                           /* across a stream slice.                         */
} SDT_node;
typedef SDT_node *SDTNODE;



/* Globally visible variables: */

extern int oa_errno;
extern char error_string[];    
extern struct oa_profile Oa_profile;

#define OA_PROFILES              4
#define OA_PROFILED_DATA_TYPES  49
#define OA_ALIAS_TRANSLATIONS    2

extern struct oa_type_conversion_info 
              Oa_type_conversion_info[OA_PROFILES][OA_PROFILED_DATA_TYPES];
extern struct oa_alias_translations 
              Oa_alias_translations[ OA_ALIAS_TRANSLATIONS];



#ifdef _NO_PROTO

/* Object layer functions:  */

extern OA_OBJECT OaAddContainerAroundTable();
extern OA_OBJECT OaAddLineTerminatorstoTable();
extern int       OaCheckODLTree();
extern int       OaCloseImage();
extern int       OaCloseOutputFile();
extern OA_OBJECT OaConvertImagetoArray();
extern OA_OBJECT OaConvertObject();
extern OA_OBJECT OaConvertObjecttoOneType();
extern ODLTREE   OaConvertLabel();
extern OA_OBJECT OaCopyObject();
int              OaCreateAttachedLabel();
extern OA_OBJECT OaDeleteColumn();
extern int       OaDeleteObject();
extern OA_OBJECT OaDeleteRow();
extern PTR       OaExportObject();
extern ODLTREE   OaFindEncodingHistogram();
extern int       OaGetFileKeywords();
extern OA_OBJECT OaGetPartialImage();
extern OA_OBJECT OaGetSubCollection();
extern OA_OBJECT OaGetSubTable();
extern OA_OBJECT OaImportColumn();
extern OA_OBJECT OaImportHistogram();
extern OA_OBJECT OaImportImage();
extern OA_OBJECT OaJoinTables();
extern OA_OBJECT OaNewOaObject();
extern OA_OBJECT OaOpenImage();
extern OA_OBJECT OaOpenOutputFile();
extern OA_OBJECT OaReadArray();
extern OA_OBJECT OaReadHistory();
extern OA_OBJECT OaReadHistogram();
extern OA_OBJECT OaReadImage();
extern int       OaReadImagePixels();
extern OA_OBJECT OaReadImageFromQube();
extern OA_OBJECT OaReadSpectrumFromImage(); 
extern OA_OBJECT OaReadSpectrumFromQube(); 
extern ODLTREE   OaParseLabelFile();
extern OA_OBJECT OaReadObject();
extern OA_OBJECT OaReadPartialImage();
extern OA_OBJECT OaReadSubTable(); 
extern OA_OBJECT OaReadTable();
extern int       OaReportFileAttributes();
extern int       OaReportError();
extern int       OaRouteErrorMessages();
extern OA_OBJECT OaTransposeTable();
extern OA_OBJECT OaUnravelContainer();
extern int       OaWriteObject();

/* Structure layer functions:  */

extern ODLTREE   LeftmostSDTChild();
extern ODLTREE   LeftSDTSibling();
extern ODLTREE   RightmostSDTChild();
extern ODLTREE   RightSDTSibling();

extern int       OalAdjustKwdstoMatchSDT();
extern int       OalAttachItemOffsetNodes();
extern int       OalBuildSDTNode();
extern int       OalCheckForGapInSrcData();
extern int       OalClementineJPEGDecompress();
extern ODLTREE   OalCompressSDT();
extern int       OalConvert();
extern void *    OalCreateHFDTree();
extern ODLTREE   OalCreateODLTreeFromGif();
extern ODLTREE   OalCreateSDT();
extern int       OalDetermineConvertParameters();
extern int       OalDetermineRepNodeAlignment();
extern struct binrep_desc *
                 OalFindBinrepDescrip();
extern void      OalFreeHFDTree();
extern int       OalFreeSDT();
extern int       OalFreeSDTStruct();
extern int       OalGetNativeCTypeInfo();
extern struct oa_type_conversion_info *
                 OalGetTypeConversionFromProfile();


/*extern void      OalHFDDecompress();*/
extern int      OalHFDDecompress();



extern ODLTREE   OalInitializeSDT();
extern SDT_node *OalNewSDTNode();
extern ODLTREE   OalPositionToNextDataNode();
extern int       OalPostOrderTraverse();
extern int       OalProcessSDT();
extern OA_OBJECT OalReadImageFromGif();
extern int       OalSDTtoODLTree();

#else

/* Object Layer functions:  */

extern OA_OBJECT OaAddContainerAroundTable( OA_OBJECT table_object);
extern OA_OBJECT OaAddLineTerminatorstoTable( OA_OBJECT table_object);
extern int       OaCheckODLTree( ODLTREE TLO_node);
extern int       OaCloseImage( OA_OBJECT image_handle_object);
extern int       OaCloseOutputFile( OA_OBJECT file_object,
                                    char *label_filespec);
extern OA_OBJECT OaConvertImagetoArray( OA_OBJECT simple_image);
extern OA_OBJECT OaConvertObject( OA_OBJECT object);
extern OA_OBJECT OaConvertObjecttoOneType( OA_OBJECT object, 
                                           char *data_type, int bytes,
                                           int rescale);
extern ODLTREE   OaConvertLabel( ODLTREE root_node);
extern OA_OBJECT OaCopyObject( OA_OBJECT object);
int              OaCreateAttachedLabel( ODLTREE odltree, 
                                        char *label_filespec,
                                        char *attached_label_filespec);
extern OA_OBJECT OaDeleteColumn( OA_OBJECT oa_object,
                                 ODLTREE input_node);
extern int       OaDeleteObject( OA_OBJECT object);
extern OA_OBJECT OaDeleteRow( OA_OBJECT table_object, 
                              long row);
extern PTR       OaExportObject( OA_OBJECT object);
extern ODLTREE   OaFindEncodingHistogram( ODLTREE image_node);
extern int       OaGetFileKeywords( ODLTREE odltreenode, 
                                    char  **label_filename,
                                    char **data_filename,
                                    int *record_type,  
                                    long *record_bytes,
                                    long *file_records, 
                                    long *file_offset, 
                                    int *object_interchange_format);
extern OA_OBJECT OaGetPartialImage( OA_OBJECT simple_image, 
                                    long start_line, 
                                    long stop_line,
                                    long start_sample,
                                    long stop_sample);
extern OA_OBJECT OaGetSubCollection( OA_OBJECT oa_object, 
                                     ODLTREE subobject_node);
extern OA_OBJECT OaGetSubTable( OA_OBJECT table_object, 
                                long start_row,
                                long stop_row,
                                ODLTREE subobject_nodes[],
                                int n_subobject_nodes);
extern OA_OBJECT OaImportColumn( PTR  data_ptr,
                                 long rows,
                                 long items,
                                 long item_bytes,
                                 char *data_type,  
                                 int interchange_format,
                                 char *name);
extern OA_OBJECT OaImportHistogram( PTR  data_ptr,
                                    long items,
                                    long item_bytes,
                                    char *data_type,  
                                    int interchange_format);
extern OA_OBJECT OaImportImage( PTR  data_ptr,
                                long lines,
                                long line_samples, 
                                char *sample_type,
                                int sample_bits);
extern OA_OBJECT OaJoinTables( OA_OBJECT table_A,
                               OA_OBJECT table_B,
                               int option);
extern OA_OBJECT OaNewOaObject( void);
extern OA_OBJECT OaOpenImage( ODLTREE input_node, int band);
extern OA_OBJECT OaOpenOutputFile( char *data_filename,
                                   int record_type,
                                   long record_bytes);
extern OA_OBJECT OaReadArray( ODLTREE array_node);
extern OA_OBJECT OaReadHistory( ODLTREE input_node);
extern OA_OBJECT OaReadHistogram( ODLTREE histogram_node);
extern OA_OBJECT OaReadImage( ODLTREE image_node, int band);
extern int       OaReadImagePixels( OA_OBJECT oa_object,
                                    long start_line,
                                    long start_sample);
extern OA_OBJECT OaReadImageFromQube( ODLTREE qube_node, int band);
extern OA_OBJECT OaReadSpectrumFromImage( ODLTREE image_node, 
                                          int line,
                                          int sample);
extern OA_OBJECT OaReadSpectrumFromQube( ODLTREE qube_node, 
                                         int line,
                                         int sample);
extern ODLTREE   OaParseLabelFile( char *filespec,
                                   char *errfilespec,
                                   MASK expand,
                                   unsigned short nomsgs);
extern OA_OBJECT OaReadObject( ODLTREE object_node);
extern OA_OBJECT OaReadPartialImage( OA_OBJECT oa_object,
                                     long start_line, 
                                     long stop_line,
                                     long start_sample,
                                     long stop_sample);
extern OA_OBJECT OaReadSubTable( ODLTREE table_node, 
                                 long start_row,
                                 long stop_row,
                                 ODLTREE subobject_nodes[],
                                 int n_subobject_nodes);
extern OA_OBJECT OaReadTable( ODLTREE input_node);
extern int       OaReportFileAttributes( char *label_filename, 
                                         char *data_filename, 
                                         int record_type,
                                         long record_bytes, 
                                         long file_offset, 
                                         int object_interchange_format);
extern int       OaReportError( char *input_error_string);
extern int       OaRouteErrorMessages( char *message_fname,
                                       FILE *message_fptr);
extern OA_OBJECT OaTransposeTable( OA_OBJECT table_object);
extern OA_OBJECT OaUnravelContainer( OA_OBJECT oa_object);
extern int       OaWriteObject( OA_OBJECT file_object, OA_OBJECT object);

/* Structure layer functions:  */

extern ODLTREE   LeftmostSDTChild( ODLTREE current_node);
extern ODLTREE   LeftSDTSibling( ODLTREE current_node);
extern ODLTREE   RightmostSDTChild( ODLTREE current_node);
extern ODLTREE   RightSDTSibling( ODLTREE current_node);

extern int       OalAdjustKwdstoMatchSDT( ODLTREE current_node, 
                                          int src_interchange_format,
                                          int dst_interchange_format);
extern int       OalAttachItemOffsetNodes( ODLTREE input_node, 
                                           long items, 
                                           long item_bytes,
                                           long item_offset, 
                                           int src_interchange_format,
                                           int dst_interchange_format);
extern int       OalBuildSDTNode( ODLTREE current_node, 
                                  int src_interchange_format,
                                  int dst_interchange_format);
extern int       OalCheckForGapInSrcData( ODLTREE input_node);
extern int       OalClementineJPEGDecompress( PTR data_ptr,
                                              long lines,
                                              long samples, 
                                              FILE *fp);
extern ODLTREE   OalCompressSDT( ODLTREE sdt);
extern int       OalConvert( SDT_node *sdt_node_ptr);
extern void *    OalCreateHFDTree( void *input_histogram);
extern ODLTREE   OalCreateODLTreeFromGif( FILE *fd, char *filename);
extern ODLTREE   OalCreateSDT( ODLTREE TLO_node, int src_interchange_format);
extern int       OalDetermineConvertParameters( ODLTREE current_node, 
                                                int src_interchange_format,
                                                int dst_interchange_format);
extern int       OalDetermineRepNodeAlignment( ODLTREE input_node);
extern struct binrep_desc *
                 OalFindBinrepDescrip( char *binrep_q_code);
extern void      OalFreeHFDTree( void *input_root);
extern int       OalFreeSDT( ODLTREE root_node);
extern int       OalFreeSDTStruct( SDT_node *sdt_node_ptr);
extern int       OalGetNativeCTypeInfo( char *c_type, int *PDS_data_type,
                                        int *size,
                                        struct binrep_desc **binrep_descrip);
extern struct oa_type_conversion_info *
                 OalGetTypeConversionFromProfile( int PDS_data_type, 
                                                  long size);


/*extern void      OalHFDDecompress( char *ibuf, char*obuf, long *nin, 
                                   long *nout, void *input_root);*/
extern int      OalHFDDecompress( char *ibuf, char*obuf, long *nin, 
                                   long *nout, void *input_root);





extern ODLTREE   OalInitializeSDT( ODLTREE sdt, PTR data_ptr);
extern SDT_node *OalNewSDTNode( void);
extern ODLTREE   OalPositionToNextDataNode( ODLTREE current_node);
extern int       OalPostOrderTraverse( ODLTREE current_node, 
                                       int operation,
                                       int src_interchange_format,
                                       int dst_interchange_format);
extern int       OalProcessSDT( PTR source, 
                                long source_bytes, 
                                ODLTREE *current_node);
extern OA_OBJECT OalReadImageFromGif( char *data_filename, long file_offset,
                                     int lines, int line_samples,
                                     ODLTREE image_node);

extern int       OalSDTtoODLTree( ODLTREE sdt, 
                                  int dst_interchange_format);
#endif

#endif  /* ifndef OA_OAL_INCLUDED  */
