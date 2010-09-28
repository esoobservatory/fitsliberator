/*****************************************************************************

  File:  oal.c

  Description:  This file defines and initializes the global variables declared
                in oal.h:

                  oa_errno
                  Oa_alias_translations
                  Oa_profile
                  Oa_type_conversion_info  (part of the profile)

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:  25 Apr   1996

  History:

    Creation - This was part of the Alpha Release of the OA library.
    10/07/94 - Added data types profile for Dec Alpha/OSF; the same one is
               used for ULTRIX and IBM-PC.  SM
    12/11/95 - Added oa_errno.  SM
    03/21/96 - Added data types profile for Dec Alpha/OpenVMS, which differs
               from the VMS profile by using G_FLOATs instead of D_FLOATs.  SM
    03/21/96 - Changed the binary data type an ASCII_REAL is converted to in
               data types profile: was converted to 4-byte float, now
               converted to 8-byte double.  SM
    04/25/96 - Added LSB_INT8, LSB_UINT8 and VAX(D)_COMPLEX to data types 
               profile.  SM
    04/16/97 - Added MSB_BIT_STRING, LSB_BIT_STRING and ASCII_BIT_STRING to
               profile.  SM
    06/24/97 - Added OA_BCD to profile.  SM
	09/19/00 - Changed length spec of strings from 10 to 12 characters to allow
			   for the pad char and - on negative numbers.  The pad and the -
			   are added by OalConvert.  DWS

  Notes:  

*****************************************************************************/

#include "oal.h"

/****************************************************************************
Global variable oa_errno is used by the OA Library to pass error codes.
It is set when an error occurs in a function, but not cleared when a non-
erroneous call is made (like its UNIX counterpart, errno).  All OA functions
set oa_errno before calling OaReportError, allowing OaReportError to filter
messages based on oa_errno value.
****************************************************************************/
int oa_errno = 0;

/****************************************************************************
Global variable Oa_alias_translations contains aliases and their de-aliased
values for keyword names.  (Not currently used.)
****************************************************************************/
struct oa_alias_translations Oa_alias_translations[ OA_ALIAS_TRANSLATIONS] = {
  {"IMAGE_LINES",    "LINES"},
  {"IMAGE_RECORDS",  "LINES"}
};


/****************************************************************************
Global variable Oa_profile specifies the format to convert ASCII and binary
sources to, how to align the data (only applicable for binary numeric
destination data), and which data types profile to use when creating an SDT.
The #define values, e.g. ALPHA_OSF, should be defined when compiling (in your
Makefile).
****************************************************************************/
#if defined(ALPHA_OSF)
struct oa_profile Oa_profile = { OA_BINARY_INTERCHANGE_FORMAT,
                                 OA_BINARY_INTERCHANGE_FORMAT,
                                 OA_ALIGN_RISC, OA_ALPHA_OSF, TRUE};
#endif
#if defined(ALPHA_VMS)
struct oa_profile Oa_profile = { OA_BINARY_INTERCHANGE_FORMAT,
                                 OA_BINARY_INTERCHANGE_FORMAT,
                                 OA_ALIGN_RISC, OA_ALPHA_VMS, TRUE};
#endif
#if defined(MAC)
struct oa_profile Oa_profile = { OA_BINARY_INTERCHANGE_FORMAT,
                                 OA_BINARY_INTERCHANGE_FORMAT,
                                 OA_ALIGN_EVEN, OA_MAC_IEEE, TRUE};
#endif
#if defined(IBM_PC)
struct oa_profile Oa_profile = { OA_BINARY_INTERCHANGE_FORMAT,
                                 OA_BINARY_INTERCHANGE_FORMAT,
                                 OA_NO_ALIGN, OA_IBM_PC, TRUE};
								 
#endif
#if defined(OSX)
struct oa_profile Oa_profile = { OA_BINARY_INTERCHANGE_FORMAT,
								 OA_BINARY_INTERCHANGE_FORMAT,
								 OA_NO_ALIGN, OA_IBM_PC, TRUE};
#endif
#if defined(SUN3)
struct oa_profile Oa_profile = { OA_BINARY_INTERCHANGE_FORMAT,
                                 OA_BINARY_INTERCHANGE_FORMAT,
                                 OA_ALIGN_EVEN, OA_SUN3, TRUE};
#endif
#if defined(SUN4)
struct oa_profile Oa_profile = { OA_BINARY_INTERCHANGE_FORMAT,
                                 OA_BINARY_INTERCHANGE_FORMAT,
                                 OA_ALIGN_RISC, OA_SUN4, TRUE};
#endif
#if defined(ULTRIX)
struct oa_profile Oa_profile = { OA_BINARY_INTERCHANGE_FORMAT,
                                 OA_BINARY_INTERCHANGE_FORMAT,
                                 OA_ALIGN_RISC, OA_ULTRIX, TRUE};
#endif
#if defined(VAX)
struct oa_profile Oa_profile = { OA_BINARY_INTERCHANGE_FORMAT,
                                 OA_BINARY_INTERCHANGE_FORMAT,
                                 OA_NO_ALIGN, OA_VAX, TRUE};
#endif
#if defined(SGI)
struct oa_profile Oa_profile = { OA_BINARY_INTERCHANGE_FORMAT,
                                 OA_BINARY_INTERCHANGE_FORMAT,
                                 OA_ALIGN_RISC, OA_SGI, TRUE};
#endif


/****************************************************************************
Global variable Oa_type_conversion_info tells OAL how to convert every PDS
numeric type to its equivalent native PDS numeric type, on every platform.  
It's a two-dimensional array, with each platform's conversions grouped in the
sub-array indexed by the first array index.  The second array index addresses
the individual conversions.  The sub-array to use for a given platform is 
specified in the initialization of global variable Oa_profile above, by an
enumerated type like OA_SUN3, OA_VAX etc. (defined in oal.h).
  For example, if OAL is running on a Sun3, Oa_profile.data_translation_profile
specifies OA_SUN3 (= 1) as subarray;  this sub-array contains
Oa_type_conversion_info structures which specify converting LSB_INTEGER's to
MSB_INTEGER's, VAX floats to IEEE floats, etc.
  Since Oa_type_conversion_info is globally accessible, the user can change
values if desired, but this should be rare;  users should normally use the
defaults given.

Notes: 
  1) Binreps for IBM mainframe data types are still missing, so their source
     binrep q-code is left NULL.
  2) The BIT_STRING types are a special case, in that they don't specify a
     size, neither as the suffix of the binrep q-codes nor in the binary or
     ASCII size fields;  the actual sizes are determined in
     OalDetermineConvertParameters, according to the number of bytes in the
     BIT_STRING.

****************************************************************************/

/* Currently this includes 4 profiles:
   1) Profile for VAX (default double is D_FLOAT)
   2) Profile for SGI, Sun and Mac (CodeWarrier defaults or ThinkC with IEEE
      floats selected).
   3) Profile for Dec Alpha, ULTRIX and IBM-PC.
   4) Profile for Alpha/OpenVMS (identical to VAX profile, except the default
      double is G_FLOAT).  */

struct oa_type_conversion_info 
       Oa_type_conversion_info[ OA_PROFILES][ OA_PROFILED_DATA_TYPES] = {{
{NULL,          OA_ASCII_INTEGER,         0, 
 "LSB_INT4",    OA_LSB_INTEGER,           4, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{NULL,          OA_ASCII_REAL,            0, 
 "VAXD_REAL8",  OA_VAX_REAL,              8, 
                OA_ASCII_REAL,           15},
{NULL,          OA_IBM_COMPLEX,           8, 
 "VAXF_REAL4",  OA_VAX_REAL,              4, 
                OA_ASCII_REAL,           15},
{NULL,          OA_IBM_COMPLEX,          16, 
 "VAXD_REAL8",  OA_VAX_REAL,              8, 
                OA_ASCII_REAL,           15},
{NULL,          OA_IBM_INTEGER,           4, 
 "LSB_INT4",    OA_LSB_INTEGER,           4, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{NULL,          OA_IBM_REAL,              4, 
 "VAXF_REAL4",  OA_VAX_REAL,              4, 
                OA_ASCII_REAL,           15},
{NULL,          OA_IBM_REAL,              8, 
 "VAXD_REAL8",  OA_VAX_REAL,              8, 
                OA_ASCII_REAL,           15},
{NULL,          OA_IBM_UNSIGNED_INTEGER,  1, 
 "UINT1",       OA_LSB_UNSIGNED_INTEGER,  1, 
                OA_ASCII_INTEGER,         5},
{NULL,          OA_IBM_UNSIGNED_INTEGER,  2, 
 "LSB_UINT2",   OA_LSB_UNSIGNED_INTEGER,  2, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{NULL,          OA_IBM_UNSIGNED_INTEGER,  4, 
 "LSB_UINT4",   OA_LSB_UNSIGNED_INTEGER,  4, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"IEEE_REAL4",  OA_IEEE_COMPLEX,          8, 
 "VAXF_REAL4",  OA_VAX_REAL,              4, 
                OA_ASCII_REAL,           15},
{"IEEE_REAL8",  OA_IEEE_COMPLEX,         16, 
 "VAXD_REAL8",  OA_VAX_REAL,              8, 
                OA_ASCII_REAL,           15},
{"IEEE_REAL10", OA_IEEE_COMPLEX,         20, 
 "VAXD_REAL8",  OA_VAX_REAL,              8, 
                OA_ASCII_REAL,           15},
{"IEEE_REAL4",  OA_IEEE_REAL,             4,
 "VAXF_REAL4",  OA_VAX_REAL,              4,
                OA_ASCII_REAL,           15},
{"IEEE_REAL8",  OA_IEEE_REAL,             8, 
 "VAXD_REAL8",  OA_VAX_REAL,              8, 
                OA_ASCII_REAL,           15},
{"IEEE_REAL10", OA_IEEE_REAL,            10, 
 "VAXD_REAL8",  OA_VAX_REAL,              8, 
                OA_ASCII_REAL,           15},
{"INT1",        OA_LSB_INTEGER,           1, 
 "INT1",        OA_LSB_INTEGER,           1, 
                OA_ASCII_INTEGER,         5},
{"LSB_INT2",    OA_LSB_INTEGER,           2, 
 "LSB_INT2",    OA_LSB_INTEGER,           2, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"LSB_INT4",    OA_LSB_INTEGER,           4, 
 "LSB_INT4",    OA_LSB_INTEGER,           4, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"LSB_INT8",    OA_LSB_INTEGER,           8, 
 "LSB_INT4",    OA_LSB_INTEGER,           4, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"UINT1",       OA_LSB_UNSIGNED_INTEGER,  1, 
 "UINT1",       OA_LSB_UNSIGNED_INTEGER,  1, 
                OA_ASCII_INTEGER,         5},
{"LSB_UINT2",   OA_LSB_UNSIGNED_INTEGER,  2, 
 "LSB_UINT2",   OA_LSB_UNSIGNED_INTEGER,  2, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"LSB_UINT4",   OA_LSB_UNSIGNED_INTEGER,  4, 
 "LSB_UINT4",   OA_LSB_UNSIGNED_INTEGER,  4, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"LSB_UINT8",   OA_LSB_UNSIGNED_INTEGER,  8, 
 "LSB_UINT4",   OA_LSB_UNSIGNED_INTEGER,  4, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"INT1",        OA_MSB_INTEGER,           1, 
 "INT1",        OA_LSB_INTEGER,           1, 
                OA_ASCII_INTEGER,         5},
{"MSB_INT2",    OA_MSB_INTEGER,           2, 
 "LSB_INT2",    OA_LSB_INTEGER,           2, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"MSB_INT4",    OA_MSB_INTEGER,           4, 
 "LSB_INT4",    OA_LSB_INTEGER,           4, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"UINT1",       OA_MSB_UNSIGNED_INTEGER,  1, 
 "UINT1",       OA_LSB_UNSIGNED_INTEGER,  1, 
                OA_ASCII_INTEGER,         5},
{"MSB_UINT2",   OA_MSB_UNSIGNED_INTEGER,  2, 
 "LSB_UINT2",   OA_LSB_UNSIGNED_INTEGER,  2, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"MSB_UINT4",   OA_MSB_UNSIGNED_INTEGER,  4, 
 "LSB_UINT4",   OA_LSB_UNSIGNED_INTEGER,  4, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"PC_REAL4",    OA_PC_COMPLEX,            8, 
 "VAXF_REAL4",  OA_VAX_REAL,              4, 
                OA_ASCII_REAL,           15},
{"PC_REAL8",    OA_PC_COMPLEX,           16, 
 "VAXD_REAL8",  OA_VAX_REAL,              8, 
                OA_ASCII_REAL,           15},
{"PC_REAL10",   OA_PC_COMPLEX,           20, 
 "VAXD_REAL8",  OA_VAX_REAL,              8, 
                OA_ASCII_REAL,           15},
{"PC_REAL4",    OA_PC_REAL,               4, 
 "VAXF_REAL4",  OA_VAX_REAL,              4, 
                OA_ASCII_REAL,           15},
{"PC_REAL8",    OA_PC_REAL,               8, 
 "VAXD_REAL8",  OA_VAX_REAL,              8, 
                OA_ASCII_REAL,           15},
{"PC_REAL10",   OA_PC_REAL,              10, 
 "VAXD_REAL8",  OA_VAX_REAL,              8, 
                OA_ASCII_REAL,           15},
{"VAXF_REAL4",  OA_VAX_COMPLEX,           8, 
 "VAXF_REAL4",  OA_VAX_REAL,              4, 
                OA_ASCII_REAL,           15},
{"VAXD_REAL8",  OA_VAX_COMPLEX,          16, 
 "VAXD_REAL8",  OA_VAX_REAL,              8, 
                OA_ASCII_REAL,           15},
{"VAXG_REAL8",  OA_VAXG_COMPLEX,         16, 
 "VAXD_REAL8",  OA_VAX_REAL,              8, 
                OA_ASCII_REAL,           15},
{"VAXF_REAL4",  OA_VAX_REAL,              4, 
 "VAXF_REAL4",  OA_VAX_REAL,              4, 
                OA_ASCII_REAL,           15},
{"VAXD_REAL8",  OA_VAX_REAL,              8, 
 "VAXD_REAL8",  OA_VAX_REAL,              8, 
                OA_ASCII_REAL,           15},
{"VAXG_REAL8",  OA_VAXG_REAL,             8, 
 "VAXD_REAL8",  OA_VAX_REAL,              8, 
                OA_ASCII_REAL,           15},
{"VAXH_REAL16", OA_VAX_COMPLEX,          32, 
 "VAXH_REAL16", OA_VAX_REAL,             16, 
                OA_ASCII_REAL,           15},
{"VAXH_REAL16", OA_VAX_REAL,             16, 
 "VAXH_REAL16", OA_VAX_REAL,             16, 
                OA_ASCII_REAL,           15},
{"LSB_UINT",    OA_LSB_BIT_STRING,        0, 
 "LSB_UINT",    OA_LSB_BIT_STRING,        0, 
                OA_ASCII_BIT_STRING,      0},
{"MSB_UINT",    OA_MSB_BIT_STRING,        0, 
 "MSB_UINT",    OA_MSB_BIT_STRING,        0, 
                OA_ASCII_BIT_STRING,      0},
{NULL,          OA_ASCII_BIT_STRING,      0, 
 "LSB_UINT",    OA_LSB_BIT_STRING,        0, 
                OA_ASCII_BIT_STRING,      0},
{"MSB_UINT",    OA_BCD,                   0, 
 "MSB_UINT",    OA_BCD,                   0, 
                OA_ASCII_BIT_STRING,      0},
{NULL,          OA_UNKNOWN_DATA_TYPE,     0,
 NULL,          OA_UNKNOWN_DATA_TYPE,     0,
                OA_UNKNOWN_DATA_TYPE,     0}},



{
{NULL,          OA_ASCII_INTEGER,         0, 
 "MSB_INT4",    OA_MSB_INTEGER,           4, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{NULL,          OA_ASCII_REAL,            0, 
 "IEEE_REAL8",  OA_IEEE_REAL,             8, 
                OA_ASCII_REAL,           15},
{NULL,          OA_IBM_COMPLEX,           8, 
 "IEEE_REAL4",  OA_IEEE_REAL,             4, 
                OA_ASCII_REAL,           15},
{NULL,          OA_IBM_COMPLEX,          16, 
 "IEEE_REAL8",  OA_IEEE_REAL,             8, 
                OA_ASCII_REAL,           15},
{NULL,          OA_IBM_INTEGER,           4, 
 "MSB_INT4",    OA_MSB_INTEGER,           4, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{NULL,          OA_IBM_REAL,              4, 
 "IEEE_REAL4",  OA_IEEE_REAL,             4, 
                OA_ASCII_REAL,           15},
{NULL,          OA_IBM_REAL,              8, 
 "IEEE_REAL8",  OA_IEEE_REAL,             8, 
                OA_ASCII_REAL,           15},
{NULL,          OA_IBM_UNSIGNED_INTEGER,  1, 
 "UINT1",       OA_MSB_UNSIGNED_INTEGER,  1, 
                OA_ASCII_INTEGER,         5},
{NULL,          OA_IBM_UNSIGNED_INTEGER,  2, 
 "MSB_UINT2",   OA_MSB_UNSIGNED_INTEGER,  2, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{NULL,          OA_IBM_UNSIGNED_INTEGER,  4, 
 "MSB_UINT4",   OA_MSB_UNSIGNED_INTEGER,  4, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"IEEE_REAL4",  OA_IEEE_COMPLEX,          8, 
 "IEEE_REAL4",  OA_IEEE_REAL,             4, 
                OA_ASCII_REAL,           15},
{"IEEE_REAL8",  OA_IEEE_COMPLEX,         16, 
 "IEEE_REAL8",  OA_IEEE_REAL,             8, 
                OA_ASCII_REAL,           15},
{"IEEE_REAL10", OA_IEEE_COMPLEX,         20, 
 "IEEE_REAL10", OA_IEEE_REAL,            10, 
                OA_ASCII_REAL,           15},
{"IEEE_REAL4",  OA_IEEE_REAL,             4,
 "IEEE_REAL4",  OA_IEEE_REAL,             4,
                OA_ASCII_REAL,           15},
{"IEEE_REAL8",  OA_IEEE_REAL,             8, 
 "IEEE_REAL8",  OA_IEEE_REAL,             8, 
                OA_ASCII_REAL,           15},
{"IEEE_REAL10", OA_IEEE_REAL,            10, 
 "IEEE_REAL10", OA_IEEE_REAL,            10, 
                OA_ASCII_REAL,           15},
{"INT1",        OA_LSB_INTEGER,           1, 
 "INT1",        OA_MSB_INTEGER,           1, 
                OA_ASCII_INTEGER,         5},
{"LSB_INT2",    OA_LSB_INTEGER,           2, 
 "MSB_INT2",    OA_MSB_INTEGER,           2, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"LSB_INT4",    OA_LSB_INTEGER,           4, 
 "MSB_INT4",    OA_MSB_INTEGER,           4, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"LSB_INT8",    OA_LSB_INTEGER,           8, 
 "MSB_INT4",    OA_MSB_INTEGER,           4, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"UINT1",       OA_LSB_UNSIGNED_INTEGER,  1, 
 "UINT1",       OA_MSB_UNSIGNED_INTEGER,  1, 
                OA_ASCII_INTEGER,         5},
{"LSB_UINT2",   OA_LSB_UNSIGNED_INTEGER,  2, 
 "MSB_UINT2",   OA_MSB_UNSIGNED_INTEGER,  2, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"LSB_UINT4",   OA_LSB_UNSIGNED_INTEGER,  4, 
 "MSB_UINT4",   OA_MSB_UNSIGNED_INTEGER,  4, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"LSB_UINT8",   OA_LSB_UNSIGNED_INTEGER,  8, 
 "MSB_UINT4",   OA_MSB_UNSIGNED_INTEGER,  4, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"INT1",        OA_MSB_INTEGER,           1, 
 "INT1",        OA_MSB_INTEGER,           1, 
                OA_ASCII_INTEGER,         5},
{"MSB_INT2",    OA_MSB_INTEGER,           2, 
 "MSB_INT2",    OA_MSB_INTEGER,           2, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"MSB_INT4",    OA_MSB_INTEGER,           4, 
 "MSB_INT4",    OA_MSB_INTEGER,           4, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"UINT1",       OA_MSB_UNSIGNED_INTEGER,  1, 
 "UINT1",       OA_MSB_UNSIGNED_INTEGER,  1, 
                OA_ASCII_INTEGER,         5},
{"MSB_UINT2",   OA_MSB_UNSIGNED_INTEGER,  2, 
 "MSB_UINT2",   OA_MSB_UNSIGNED_INTEGER,  2, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"MSB_UINT4",   OA_MSB_UNSIGNED_INTEGER,  4, 
 "MSB_UINT4",   OA_MSB_UNSIGNED_INTEGER,  4, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"PC_REAL4",    OA_PC_COMPLEX,            8, 
 "IEEE_REAL4",  OA_IEEE_REAL,             4, 
                OA_ASCII_REAL,           15},
{"PC_REAL8",    OA_PC_COMPLEX,           16, 
 "IEEE_REAL8",  OA_IEEE_REAL,             8, 
                OA_ASCII_REAL,           15},
{"PC_REAL10",   OA_PC_COMPLEX,           20, 
 "IEEE_REAL10", OA_IEEE_REAL,            10, 
                OA_ASCII_REAL,           15},
{"PC_REAL4",    OA_PC_REAL,               4, 
 "IEEE_REAL4",  OA_IEEE_REAL,             4, 
                OA_ASCII_REAL,           15},
{"PC_REAL8",    OA_PC_REAL,               8, 
 "IEEE_REAL8",  OA_IEEE_REAL,             8, 
                OA_ASCII_REAL,           15},
{"PC_REAL10",   OA_PC_REAL,              10, 
 "IEEE_REAL10", OA_IEEE_REAL,            10, 
                OA_ASCII_REAL,           15},
{"VAXF_REAL4",  OA_VAX_COMPLEX,           8, 
 "IEEE_REAL4",  OA_IEEE_REAL,             4, 
                OA_ASCII_REAL,           15},
{"VAXD_REAL8",  OA_VAX_COMPLEX,          16, 
 "IEEE_REAL8",  OA_IEEE_REAL,             8, 
                OA_ASCII_REAL,           15},
{"VAXG_REAL8",  OA_VAXG_COMPLEX,         16, 
 "IEEE_REAL8",  OA_IEEE_REAL,             8, 
                OA_ASCII_REAL,           15},
{"VAXF_REAL4",  OA_VAX_REAL,              4, 
 "IEEE_REAL4",  OA_IEEE_REAL,             4, 
                OA_ASCII_REAL,           15},
{"VAXD_REAL8",  OA_VAX_REAL,              8, 
 "IEEE_REAL8",  OA_IEEE_REAL,             8, 
                OA_ASCII_REAL,           15},
{"VAXG_REAL8",  OA_VAXG_REAL,             8, 
 "IEEE_REAL8",  OA_IEEE_REAL,             8, 
                OA_ASCII_REAL,           15},
{"VAXH_REAL16", OA_VAX_COMPLEX,          32, 
 "IEEE_REAL8",  OA_IEEE_REAL,             8, 
                OA_ASCII_REAL,           15},
{"VAXH_REAL16", OA_VAX_REAL,             16, 
 "IEEE_REAL8",  OA_IEEE_REAL,             8, 
                OA_ASCII_REAL,           15},
{"LSB_UINT",    OA_LSB_BIT_STRING,        0, 
 "LSB_UINT",    OA_LSB_BIT_STRING,        0, 
                OA_ASCII_BIT_STRING,      0},
{"MSB_UINT",    OA_MSB_BIT_STRING,        0, 
 "MSB_UINT",    OA_MSB_BIT_STRING,        0, 
                OA_ASCII_BIT_STRING,      0},
{NULL,          OA_ASCII_BIT_STRING,      0, 
 "MSB_UINT",    OA_MSB_BIT_STRING,        0, 
                OA_ASCII_BIT_STRING,      0},
{"MSB_UINT",    OA_BCD,                   0, 
 "MSB_UINT",    OA_BCD,                   0, 
                OA_ASCII_BIT_STRING,      0},
{NULL,          OA_UNKNOWN_DATA_TYPE,     0,
 NULL,          OA_UNKNOWN_DATA_TYPE,     0,
                OA_UNKNOWN_DATA_TYPE,     0}},



{
{NULL,          OA_ASCII_INTEGER,         0, 
 "LSB_INT4",    OA_LSB_INTEGER,           4, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{NULL,          OA_ASCII_REAL,            0, 
 "PC_REAL8",    OA_PC_REAL,               8, 
                OA_ASCII_REAL,           15},
{NULL,          OA_IBM_COMPLEX,           8, 
 "PC_REAL4",    OA_PC_REAL,               4, 
                OA_ASCII_REAL,           15},
{NULL,          OA_IBM_COMPLEX,          16, 
 "PC_REAL8",    OA_PC_REAL,               8, 
                OA_ASCII_REAL,           15},
{NULL,          OA_IBM_INTEGER,           4, 
 "LSB_INT4",    OA_LSB_INTEGER,           4, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{NULL,          OA_IBM_REAL,              4, 
 "PC_REAL4",    OA_PC_REAL,               4, 
                OA_ASCII_REAL,           15},
{NULL,          OA_IBM_REAL,              8, 
 "PC_REAL8",    OA_PC_REAL,               8, 
                OA_ASCII_REAL,           15},
{NULL,          OA_IBM_UNSIGNED_INTEGER,  1, 
 "UINT1",       OA_LSB_UNSIGNED_INTEGER,  1, 
                OA_ASCII_INTEGER,         5},
{NULL,          OA_IBM_UNSIGNED_INTEGER,  2, 
 "LSB_UINT2",   OA_LSB_UNSIGNED_INTEGER,  2, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{NULL,          OA_IBM_UNSIGNED_INTEGER,  4, 
 "LSB_UINT4",   OA_LSB_UNSIGNED_INTEGER,  4, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"IEEE_REAL4",  OA_IEEE_COMPLEX,          8, 
 "PC_REAL4",    OA_PC_REAL,               4, 
                OA_ASCII_REAL,           15},
{"IEEE_REAL8",  OA_IEEE_COMPLEX,         16, 
 "PC_REAL8",    OA_PC_REAL,               8, 
                OA_ASCII_REAL,           15},
{"IEEE_REAL10", OA_IEEE_COMPLEX,         20, 
 "PC_REAL10",   OA_PC_REAL,              10, 
                OA_ASCII_REAL,           15},
{"IEEE_REAL4",  OA_IEEE_REAL,             4,
 "PC_REAL4",    OA_PC_REAL,               4,
                OA_ASCII_REAL,           15},
{"IEEE_REAL8",  OA_IEEE_REAL,             8, 
 "PC_REAL8",    OA_PC_REAL,               8, 
                OA_ASCII_REAL,           15},
{"IEEE_REAL10", OA_IEEE_REAL,            10, 
 "PC_REAL10",   OA_PC_REAL,              10, 
                OA_ASCII_REAL,           15},
{"INT1",        OA_LSB_INTEGER,           1, 
 "INT1",        OA_LSB_INTEGER,           1, 
                OA_ASCII_INTEGER,         5},
{"LSB_INT2",    OA_LSB_INTEGER,           2, 
 "LSB_INT2",    OA_LSB_INTEGER,           2, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"LSB_INT4",    OA_LSB_INTEGER,           4, 
 "LSB_INT4",    OA_LSB_INTEGER,           4, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"LSB_INT8",    OA_LSB_INTEGER,           8, 
 "LSB_INT8",    OA_LSB_INTEGER,           8, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"UINT1",       OA_LSB_UNSIGNED_INTEGER,  1, 
 "UINT1",       OA_LSB_UNSIGNED_INTEGER,  1, 
                OA_ASCII_INTEGER,         5},
{"LSB_UINT2",   OA_LSB_UNSIGNED_INTEGER,  2, 
 "LSB_UINT2",   OA_LSB_UNSIGNED_INTEGER,  2, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"LSB_UINT4",   OA_LSB_UNSIGNED_INTEGER,  4, 
 "LSB_UINT4",   OA_LSB_UNSIGNED_INTEGER,  4, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"LSB_UINT8",   OA_LSB_UNSIGNED_INTEGER,  8, 
 "LSB_UINT8",   OA_LSB_UNSIGNED_INTEGER,  8, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"INT1",        OA_MSB_INTEGER,           1, 
 "INT1",        OA_LSB_INTEGER,           1, 
                OA_ASCII_INTEGER,         5},
{"MSB_INT2",    OA_MSB_INTEGER,           2, 
 "LSB_INT2",    OA_LSB_INTEGER,           2, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"MSB_INT4",    OA_MSB_INTEGER,           4, 
 "LSB_INT4",    OA_LSB_INTEGER,           4, 
                OA_ASCII_INTEGER,        12},/*changed from 10  DWS 09-18-00*/
{"UINT1",       OA_MSB_UNSIGNED_INTEGER,  1, 
 "UINT1",       OA_LSB_UNSIGNED_INTEGER,  1, 
                OA_ASCII_INTEGER,         5},
{"MSB_UINT2",   OA_MSB_UNSIGNED_INTEGER,  2, 
 "LSB_UINT2",   OA_LSB_UNSIGNED_INTEGER,  2, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"MSB_UINT4",   OA_MSB_UNSIGNED_INTEGER,  4, 
 "LSB_UINT4",   OA_LSB_UNSIGNED_INTEGER,  4, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"PC_REAL4",    OA_PC_COMPLEX,            8, 
 "PC_REAL4",    OA_PC_REAL,               4, 
                OA_ASCII_REAL,           15},
{"PC_REAL8",    OA_PC_COMPLEX,           16, 
 "PC_REAL8",    OA_PC_REAL,               8, 
                OA_ASCII_REAL,           15},
{"PC_REAL10",   OA_PC_COMPLEX,           20, 
 "PC_REAL10",   OA_PC_REAL,              10, 
                OA_ASCII_REAL,           15},
{"PC_REAL4",    OA_PC_REAL,               4, 
 "PC_REAL4",    OA_PC_REAL,               4, 
                OA_ASCII_REAL,           15},
{"PC_REAL8",    OA_PC_REAL,               8, 
 "PC_REAL8",    OA_PC_REAL,               8, 
                OA_ASCII_REAL,           15},
{"PC_REAL10",   OA_PC_REAL,              10, 
 "PC_REAL10",   OA_PC_REAL,              10, 
                OA_ASCII_REAL,           15},
{"VAXF_REAL4",  OA_VAX_COMPLEX,           8, 
 "PC_REAL4",    OA_PC_REAL,               4, 
                OA_ASCII_REAL,           15},
{"VAXD_REAL8",  OA_VAX_COMPLEX,          16, 
 "PC_REAL8",    OA_PC_REAL,               8, 
                OA_ASCII_REAL,           15},
{"VAXG_REAL8",  OA_VAXG_COMPLEX,          16, 
 "PC_REAL8",    OA_PC_REAL,               8, 
                OA_ASCII_REAL,           15},
{"VAXF_REAL4",  OA_VAX_REAL,              4, 
 "PC_REAL4",    OA_PC_REAL,               4, 
                OA_ASCII_REAL,           15},
{"VAXD_REAL8",  OA_VAX_REAL,              8, 
 "PC_REAL8",    OA_PC_REAL,               8, 
                OA_ASCII_REAL,           15},
{"VAXG_REAL8",  OA_VAXG_REAL,             8, 
 "PC_REAL8",    OA_PC_REAL,               8, 
                OA_ASCII_REAL,           15},
{"VAXH_REAL16", OA_VAX_COMPLEX,          32, 
 "PC_REAL8",    OA_PC_REAL,               8, 
                OA_ASCII_REAL,           15},
{"VAXH_REAL16", OA_VAX_REAL,             16, 
 "PC_REAL8",    OA_PC_REAL,               8, 
                OA_ASCII_REAL,           15},
{"LSB_UINT",    OA_LSB_BIT_STRING,        0, 
 "LSB_UINT",    OA_LSB_BIT_STRING,        0, 
                OA_ASCII_BIT_STRING,      0},
{"MSB_UINT",    OA_MSB_BIT_STRING,        0, 
 "MSB_UINT",    OA_MSB_BIT_STRING,        0, 
                OA_ASCII_BIT_STRING,      0},
{NULL,          OA_ASCII_BIT_STRING,      0, 
 "LSB_UINT",    OA_LSB_BIT_STRING,        0, 
                OA_ASCII_BIT_STRING,      0},
{"MSB_UINT",    OA_BCD,                   0, 
 "MSB_UINT",    OA_BCD,                   0, 
                OA_ASCII_BIT_STRING,      0},
{NULL,          OA_UNKNOWN_DATA_TYPE,     0,
 NULL,          OA_UNKNOWN_DATA_TYPE,     0,
                OA_UNKNOWN_DATA_TYPE,     0}},

{
{NULL,          OA_ASCII_INTEGER,         0, 
 "LSB_INT4",    OA_LSB_INTEGER,           4, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{NULL,          OA_ASCII_REAL,            0, 
 "VAXG_REAL8",  OA_VAXG_REAL,             8, 
                OA_ASCII_REAL,           15},
{NULL,          OA_IBM_COMPLEX,           8, 
 "VAXF_REAL4",  OA_VAX_REAL,              4, 
                OA_ASCII_REAL,           15},
{NULL,          OA_IBM_COMPLEX,          16, 
 "VAXG_REAL8",  OA_VAXG_REAL,             8, 
                OA_ASCII_REAL,           15},
{NULL,          OA_IBM_INTEGER,           4, 
 "LSB_INT4",    OA_LSB_INTEGER,           4, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{NULL,          OA_IBM_REAL,              4, 
 "VAXF_REAL4",  OA_VAX_REAL,              4, 
                OA_ASCII_REAL,           15},
{NULL,          OA_IBM_REAL,              8, 
 "VAXG_REAL8",  OA_VAXG_REAL,             8, 
                OA_ASCII_REAL,           15},
{NULL,          OA_IBM_UNSIGNED_INTEGER,  1, 
 "UINT1",       OA_LSB_UNSIGNED_INTEGER,  1, 
                OA_ASCII_INTEGER,         5},
{NULL,          OA_IBM_UNSIGNED_INTEGER,  2, 
 "LSB_UINT2",   OA_LSB_UNSIGNED_INTEGER,  2, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{NULL,          OA_IBM_UNSIGNED_INTEGER,  4, 
 "LSB_UINT4",   OA_LSB_UNSIGNED_INTEGER,  4, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"IEEE_REAL4",  OA_IEEE_COMPLEX,          8, 
 "VAXF_REAL4",  OA_VAX_REAL,              4, 
                OA_ASCII_REAL,           15},
{"IEEE_REAL8",  OA_IEEE_COMPLEX,         16, 
 "VAXG_REAL8",  OA_VAXG_REAL,             8, 
                OA_ASCII_REAL,           15},
{"IEEE_REAL10", OA_IEEE_COMPLEX,         20, 
 "VAXG_REAL8",  OA_VAXG_REAL,             8, 
                OA_ASCII_REAL,           15},
{"IEEE_REAL4",  OA_IEEE_REAL,             4,
 "VAXF_REAL4",  OA_VAX_REAL,              4,
                OA_ASCII_REAL,           15},
{"IEEE_REAL8",  OA_IEEE_REAL,             8, 
 "VAXG_REAL8",  OA_VAXG_REAL,             8, 
                OA_ASCII_REAL,           15},
{"IEEE_REAL10", OA_IEEE_REAL,            10, 
 "VAXG_REAL8",  OA_VAXG_REAL,             8, 
                OA_ASCII_REAL,           15},
{"INT1",        OA_LSB_INTEGER,           1, 
 "INT1",        OA_LSB_INTEGER,           1, 
                OA_ASCII_INTEGER,         5},
{"LSB_INT2",    OA_LSB_INTEGER,           2, 
 "LSB_INT2",    OA_LSB_INTEGER,           2, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"LSB_INT4",    OA_LSB_INTEGER,           4, 
 "LSB_INT4",    OA_LSB_INTEGER,           4, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"LSB_INT8",    OA_LSB_INTEGER,           8, 
 "LSB_INT8",    OA_LSB_INTEGER,           8, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"UINT1",       OA_LSB_UNSIGNED_INTEGER,  1, 
 "UINT1",       OA_LSB_UNSIGNED_INTEGER,  1, 
                OA_ASCII_INTEGER,         5},
{"LSB_UINT2",   OA_LSB_UNSIGNED_INTEGER,  2, 
 "LSB_UINT2",   OA_LSB_UNSIGNED_INTEGER,  2, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"LSB_UINT4",   OA_LSB_UNSIGNED_INTEGER,  4, 
 "LSB_UINT4",   OA_LSB_UNSIGNED_INTEGER,  4, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"LSB_UINT8",   OA_LSB_UNSIGNED_INTEGER,  8, 
 "LSB_UINT8",   OA_LSB_UNSIGNED_INTEGER,  8, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"INT1",        OA_MSB_INTEGER,           1, 
 "INT1",        OA_LSB_INTEGER,           1, 
                OA_ASCII_INTEGER,         5},
{"MSB_INT2",    OA_MSB_INTEGER,           2, 
 "LSB_INT2",    OA_LSB_INTEGER,           2, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"MSB_INT4",    OA_MSB_INTEGER,           4, 
 "LSB_INT4",    OA_LSB_INTEGER,           4, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"UINT1",       OA_MSB_UNSIGNED_INTEGER,  1, 
 "UINT1",       OA_LSB_UNSIGNED_INTEGER,  1, 
                OA_ASCII_INTEGER,         5},
{"MSB_UINT2",   OA_MSB_UNSIGNED_INTEGER,  2, 
 "LSB_UINT2",   OA_LSB_UNSIGNED_INTEGER,  2, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"MSB_UINT4",   OA_MSB_UNSIGNED_INTEGER,  4, 
 "LSB_UINT4",   OA_LSB_UNSIGNED_INTEGER,  4, 
                OA_ASCII_INTEGER,        12},  /*DWS changed 09-18-00*/
{"PC_REAL4",    OA_PC_COMPLEX,            8, 
 "VAXF_REAL4",  OA_VAX_REAL,              4, 
                OA_ASCII_REAL,           15},
{"PC_REAL8",    OA_PC_COMPLEX,           16, 
 "VAXG_REAL8",  OA_VAXG_REAL,             8, 
                OA_ASCII_REAL,           15},
{"PC_REAL10",   OA_PC_COMPLEX,           20, 
 "VAXG_REAL8",  OA_VAXG_REAL,             8, 
                OA_ASCII_REAL,           15},
{"PC_REAL4",    OA_PC_REAL,               4, 
 "VAXF_REAL4",  OA_VAX_REAL,              4, 
                OA_ASCII_REAL,           15},
{"PC_REAL8",    OA_PC_REAL,               8, 
 "VAXG_REAL8",  OA_VAXG_REAL,             8, 
                OA_ASCII_REAL,           15},
{"PC_REAL10",   OA_PC_REAL,              10, 
 "VAXG_REAL8",  OA_VAXG_REAL,             8, 
                OA_ASCII_REAL,           15},
{"VAXF_REAL4",  OA_VAX_COMPLEX,           8, 
 "VAXF_REAL4",  OA_VAX_REAL,              4, 
                OA_ASCII_REAL,           15},
{"VAXD_REAL8",  OA_VAX_COMPLEX,          16, 
 "VAXG_REAL8",  OA_VAXG_REAL,             8, 
                OA_ASCII_REAL,           15},
{"VAXG_REAL8",  OA_VAXG_COMPLEX,         16, 
 "VAXG_REAL8",  OA_VAXG_REAL,             8, 
                OA_ASCII_REAL,           15},
{"VAXF_REAL4",  OA_VAX_REAL,              4, 
 "VAXF_REAL4",  OA_VAX_REAL,              4, 
                OA_ASCII_REAL,           15},
{"VAXD_REAL8",  OA_VAX_REAL,              8, 
 "VAXG_REAL8",  OA_VAXG_REAL,             8, 
                OA_ASCII_REAL,           15},
{"VAXG_REAL8",  OA_VAXG_REAL,             8, 
 "VAXG_REAL8",  OA_VAXG_REAL,             8, 
                OA_ASCII_REAL,           15},
{"VAXH_REAL16", OA_VAX_COMPLEX,          32, 
 "VAXH_REAL16", OA_VAX_REAL,             16, 
                OA_ASCII_REAL,           15},
{"VAXH_REAL16", OA_VAX_REAL,             16, 
 "VAXH_REAL16", OA_VAX_REAL,             16, 
                OA_ASCII_REAL,           15},
{"LSB_UINT",    OA_LSB_BIT_STRING,        0, 
 "LSB_UINT",    OA_LSB_BIT_STRING,        0, 
                OA_ASCII_BIT_STRING,      0},
{"MSB_UINT",    OA_MSB_BIT_STRING,        0, 
 "MSB_UINT",    OA_MSB_BIT_STRING,        0, 
                OA_ASCII_BIT_STRING,      0},
{NULL,          OA_ASCII_BIT_STRING,      0, 
 "LSB_UINT",    OA_LSB_BIT_STRING,        0, 
                OA_ASCII_BIT_STRING,      0},
{"MSB_UINT",    OA_BCD,                   0, 
 "MSB_UINT",    OA_BCD,                   0, 
                OA_ASCII_BIT_STRING,      0},
{NULL,          OA_UNKNOWN_DATA_TYPE,     0,
 NULL,          OA_UNKNOWN_DATA_TYPE,     0,
                OA_UNKNOWN_DATA_TYPE,     0}}
};





