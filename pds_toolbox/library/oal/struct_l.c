/*****************************************************************************

  File:  struct_l.c

  Description: This file contains the C routines which make up the Structure
               Layer of the PDS Object Access Library.  The routines are:

               LeftmostSDTChild
               LeftSDTSibling
               RightmostSDTChild
               RightSDTSibling

               OalAdjustKwdstoMatchSDT
               OalAttachItemOffsetNodes
               OalBuildSDTNode
               OalCheckForGapInSrcData
               OalCompressSDT
               OalConvert
               OalCreateSDT
               OalDetermineConvertParameters
               OalDetermineRepNodeAlignment
               OalFreeSDT
               OalFreeSDTStruct
               OalGetNativeCTypeInfo
               OalGetTypeConversionFromProfile
               OalNewSDTNode
               OalInitializeSDT
               OalPositionToNextDataNode
               OalPostOrderTraverse
               OalProcessSDT
               OalSDTtoODLTree

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:  21 Apr   1997

  History:

    Creation - This set of routines was part of the Alpha Release of the
               OA library.
    10/06/94 - Added OalCompressSDT. SM
    12/03/94 - Added OalCheckForGapInSrcData, OalAttachItemOffsetNodes and
               OalDetermineRepNodeAlignment.  SM
    12/06/95 - Replaced malloc() by OaMalloc() throughout.  SM
    12/11/95 - Added error codes.  SM
    04/25/96 - Added OalGetNativeCTypeInfo.  SM
    04/16/97 - Added OalFreeSDTStruct.  SM
    04/21/97 - Various code changes to allow SDT's which preserve unused ODL
               tree nodes which don't have SDT nodes attached (BIT_COLUMNS,
               ALIAS etc), 4 new SDT navigation functions.  SM
  

*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include "oal.h"
#include "binrep.h"
#ifdef IBM_PC
#pragma extref _floatconvert
#endif


/*****************************************************************************

  Routines:  LeftmostSDTChild,  LeftSDTSibling,
             RightmostSDTChild, RightSDTSibling

  Description: These routines are like the macros LeftmostChild, LeftSibling,
               RightmostChild and RightSibling, except nodes which don't have
               SDT nodes attached are skipped over.

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  21 Apr   1997
  Last Modified:  21 Apr   1997

  History:

    Creation - These routines were part of the Version 1.2 Release of the OA
               library.

  Input:
         
  Output:

  Notes:  

*****************************************************************************/
  
#ifdef _NO_PROTO

ODLTREE LeftmostSDTChild( current_node)
ODLTREE current_node;

#else

ODLTREE LeftmostSDTChild( ODLTREE current_node)

#endif
{
ODLTREE next_node;

next_node = LeftmostChild( current_node);
while (next_node != NULL)
  if (next_node->appl1 != NULL)
    return( next_node);
  else
    next_node = RightSibling( next_node);
return( next_node);
}


#ifdef _NO_PROTO

ODLTREE LeftSDTSibling( current_node)
ODLTREE current_node;

#else

ODLTREE LeftSDTSibling( ODLTREE current_node)

#endif
{
ODLTREE next_node;

next_node = LeftSibling( current_node);
while (next_node != NULL)
  if (next_node->appl1 != NULL)
    return( next_node);
  else
    next_node = LeftSibling( next_node);
return( next_node);
}


#ifdef _NO_PROTO

ODLTREE RightmostSDTChild( current_node)
ODLTREE current_node;

#else

ODLTREE RightmostSDTChild( ODLTREE current_node)

#endif
{
ODLTREE next_node;

next_node = RightmostChild( current_node);
while (next_node != NULL)
  if (next_node->appl1 != NULL)
    return( next_node);
  else
    next_node = LeftSibling( next_node);
return( next_node);
}


#ifdef _NO_PROTO

ODLTREE RightSDTSibling( current_node)
ODLTREE current_node;

#else

ODLTREE RightSDTSibling( ODLTREE current_node)

#endif
{
ODLTREE next_node;

next_node = RightSibling( current_node);
while (next_node != NULL)
  if (next_node->appl1 != NULL)
    return( next_node);
  else
    next_node = RightSibling( next_node);
return( next_node);
}



/*****************************************************************************

  Routine:  OalAdjustKwdstoMatchSDT

  Description: Translates an SDT's destination parameters into ODL keywords.

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:   1 Apr   1997

  History:

    Creation - This routine was part of the Alpha Release of the OA library.
    10/12/94 - Added code to not delete ALIAS object whose dst.size=0. SM
    02/14/95 - Changed input parameters to src_interchange_format and
               dst_interchange_format. SM
    03/06/96 - Refined code for ARRAY, ELEMENT and COLLECTION.  SM
    04/01/96 - Preserve ALIAS and/or BIT_COLUMN nodes under the COLUMN node in
               the ITEM_OFFSET special case.  SM

  Input:
         current_node - An ODL tree node which usually has an SDT node 
                        attached; if so, the SDT nodes's dst.start_offset, 
                        dst.size and dst.PDS_data_type are set.

         src_interchange_format - Ignored, necessary because called generically
                                  from OalPostOrderTraverse.
         dst_interchange_format - Set to either
                                  OA_ASCII_INTERCHANGE_FORMAT or 
                                  OA_BINARY_INTERCHANGE_FORMAT.
         
  Output:  The function always returns 0.
           If current_node didn't have an SDT node attached, the function
           does nothing and returns 0.  Otherwise keywords in the ODL tree
           node corresponding to the SDT node's dst.start_offset, dst.size and
           dst.PDS_data_type and interchange_format have been updated, and the
           SDT node is deleted.  If SDT_node->dst.size=0 then the SDT node and
           the ODL tree node are deleted from the tree.

  Notes:  
    1) This function is called from PostOrderTraverse, from within
       OalSDTtoODLTree.
    2) ITEM_OFFSET special case:  if the ITEM_OFFSET keyword is detected in a
       COLUMN-class node, then the setup done by OalAttachItemOffsetNodes is
       undone by deleting all the COLUMN-class nodes under the ITEM_OFFSET
       node, and removing the ITEM_OFFSET keyword, since SDT processing 
       removed the implicit spares between ITEM_BYTES and ITEM_OFFSET.
       Any other class nodes below the COLUMN are preserved (BIT_COLUMN or
       ALIAS).

*****************************************************************************/
  
#ifdef _NO_PROTO

int OalAdjustKwdstoMatchSDT( current_node, src_interchange_format,
                             dst_interchange_format)
ODLTREE current_node;
int src_interchange_format;
int dst_interchange_format;

#else

int OalAdjustKwdstoMatchSDT( ODLTREE current_node, int src_interchange_format,
                             int dst_interchange_format)

#endif
{

/*static char *proc_name = "OalAdjustKwdstoMatchSDT";*/
ODLTREE tmp_node, next_node;
KEYWORD *kwdptr;
SDT_node *sdt_node_ptr;
int object_class;
long items, item_bytes;
char *ptr;

object_class = src_interchange_format; /* src_interchange_format input arg   */
                                       /* has to be used somewhere to prevent*/
                                       /* compiler warning. Not actually used*/
                                       /* anywhere.                          */
/* If there's no SDT node attached, then return to caller. */

if (current_node->appl1 == NULL) {
  return(0);
} else sdt_node_ptr = (SDT_node *) current_node->appl1;

/* If dst.size=0 then delete the SDT node and ODL tree node from the tree and
   return.  */

if (sdt_node_ptr->dst.size == 0) {
  OalFreeSDT( OdlCutObjDesc( current_node));
  return(0);
}

/* Get the object class, and do the translation from the SDT parameters to
   object-specific keywords.  */

object_class = OaGetObjectClass( current_node);
switch (object_class) {

  case OA_COLLECTION:
    OaLongtoKwdValue( "BYTES", current_node, 
                      (long) sdt_node_ptr->dst.size);
  case OA_ARRAY:  /* and OA_COLLECTION */
    OaLongtoKwdValue( "START_BYTE", current_node, 
                      (long) sdt_node_ptr->dst.start_offset+1);
    if (dst_interchange_format == OA_ASCII_INTERCHANGE_FORMAT)
      OaStrtoKwdValue(  "INTERCHANGE_FORMAT", current_node, "ASCII");
    else 
      OaStrtoKwdValue(  "INTERCHANGE_FORMAT", current_node, "BINARY");
  break;

  case OA_BIT_COLUMN:
  case OA_BIT_ELEMENT:
  break;

  case OA_COLUMN:

    /* If the ITEM_OFFSET keyword is present, then undo the setup done by
       OalAttachItemOffsetNodes: delete the ITEM_OFFSET keyword, adjust
       DATA_TYPE, ITEM_BYTES and BYTES keywords, and delete the added COLUMN
       nodes.  */
 
    if ((kwdptr = OdlFindKwd( current_node, "ITEM_OFFSET", "*", 
                              (unsigned long) 0,  
                              (unsigned short) ODL_THIS_OBJECT)) != NULL) {
      OdlFreeKwd( OdlCutKwd( kwdptr));
      OaLongtoKwdValue( "START_BYTE", current_node, 
                        (long) sdt_node_ptr->dst.start_offset+1);

      /* Get the keyword values for DATA_TYPE, ITEM_BYTES and BYTES from the
         COLUMN2 node added by OdlAttachItemOffsetNodes, and copy/overwrite
         the same keywords in current_node;  leave the ITEMS keyword in 
         current_node as it is, since it hasn't changed;  note that the SDT
         nodes of these COLUMN nodes have already been deleted, since they've
         already been visited.  */

      tmp_node = LeftSibling( RightmostChild( current_node));
      OaKwdValuetoStr( "DATA_TYPE", tmp_node, &ptr);
      OaStrtoKwdValue( "DATA_TYPE", current_node, ptr);
      OaKwdValuetoStr( "BYTES", tmp_node, &ptr);
      OaStrtoKwdValue( "ITEM_BYTES", current_node, ptr);
      OaKwdValuetoLong( "ITEMS", current_node, &items);
      OaKwdValuetoLong( "ITEM_BYTES", current_node, &item_bytes);
      OaLongtoKwdValue( "BYTES", current_node, items * item_bytes);

      /* Delete all the COLUMN-class nodes under the ITEM_OFFSET node, leaving
         any other nodes (ALIAS, BIT_COLUMN).  Since this function processes
         nodes in a post-order traverse, these COLUMN nodes have already been
         visited, and their deletion will not foul up the traversal.  */

      tmp_node = LeftmostChild( current_node);
      while (tmp_node != NULL) {
        next_node = RightSibling( tmp_node);
        if (OaGetObjectClass( tmp_node) == OA_COLUMN)
          OdlFreeTree( OdlCutObjDesc( tmp_node));
        tmp_node = next_node;
      }
      break;
    }

    /* Cludge for complex data types: OalCreateSDT left the src.PDS_data_type
       as a complex, but the rest of the SDT node represents 2 adjacent reals,
       so change the SDT node back to represent a complex before going further.
    */

    switch (sdt_node_ptr->src.PDS_data_type) {
      case OA_COMPLEX:
      case OA_IBM_COMPLEX:
      case OA_PC_COMPLEX:
      case OA_VAX_COMPLEX:
      case OA_VAXG_COMPLEX:
        sdt_node_ptr->total_repetitions /= 2;
        sdt_node_ptr->src.size *= 2;
        sdt_node_ptr->dst.size *= 2;
        switch (sdt_node_ptr->dst.PDS_data_type) {
          case OA_REAL:
            sdt_node_ptr->dst.PDS_data_type = OA_COMPLEX;
          break;
          case OA_IBM_REAL:
            sdt_node_ptr->dst.PDS_data_type = OA_IBM_COMPLEX;
          break;
          case OA_PC_REAL:
            sdt_node_ptr->dst.PDS_data_type = OA_PC_COMPLEX;
          break;
          case OA_VAX_REAL:
            sdt_node_ptr->dst.PDS_data_type = OA_VAX_COMPLEX;
          break;
          case OA_VAXG_REAL:
            sdt_node_ptr->dst.PDS_data_type = OA_VAXG_COMPLEX;
          break;
        }
      break;
      default: /* src.PDS_data_type is not complex, so nothing to do.  */
      break;
    }

    ptr = OaPDSDataTypetoStr( sdt_node_ptr->dst.PDS_data_type);
    OaStrtoKwdValue(  "DATA_TYPE", current_node, ptr);
    OaLongtoKwdValue( "START_BYTE", current_node, 
                      (long) sdt_node_ptr->dst.start_offset+1);
    if (OdlFindKwd( current_node, "ITEM_BYTES", "*", (unsigned long) 0, 
                    (unsigned short) ODL_THIS_OBJECT) != NULL)
      OaLongtoKwdValue( "ITEM_BYTES", current_node, 
                        (long) sdt_node_ptr->dst.size);
    OaLongtoKwdValue( "BYTES", current_node, 
                      (long) sdt_node_ptr->dst.size *
                             sdt_node_ptr->total_repetitions);
  break;

  case OA_CONTAINER:
    OaLongtoKwdValue( "START_BYTE", current_node, 
                      (long) sdt_node_ptr->dst.start_offset+1);
    OaLongtoKwdValue( "BYTES", current_node, 
                      (long) sdt_node_ptr->dst.size);
    OaLongtoKwdValue( "REPETITIONS", current_node, 
                      (long) sdt_node_ptr->total_repetitions);
  break;

  case OA_ELEMENT:

    /* Cludge for complex data types: OalCreateSDT left the src.PDS_data_type
       as a complex, but the rest of the SDT node represents 2 adjacent reals,
       so change the SDT node back to represent a complex before going further.
    */

    switch (sdt_node_ptr->src.PDS_data_type) {
      case OA_COMPLEX:
      case OA_IBM_COMPLEX:
      case OA_PC_COMPLEX:
      case OA_VAX_COMPLEX:
      case OA_VAXG_COMPLEX:
        sdt_node_ptr->total_repetitions /= 2;
        sdt_node_ptr->src.size *= 2;
        sdt_node_ptr->dst.size *= 2;
        switch (sdt_node_ptr->dst.PDS_data_type) {
          case OA_REAL:
            sdt_node_ptr->dst.PDS_data_type = OA_COMPLEX;
          break;
          case OA_IBM_REAL:
            sdt_node_ptr->dst.PDS_data_type = OA_IBM_COMPLEX;
          break;
          case OA_PC_REAL:
            sdt_node_ptr->dst.PDS_data_type = OA_PC_COMPLEX;
          break;
          case OA_VAX_REAL:
            sdt_node_ptr->dst.PDS_data_type = OA_VAX_COMPLEX;
          break;
          case OA_VAXG_REAL:
            sdt_node_ptr->dst.PDS_data_type = OA_VAXG_COMPLEX;
          break;
          default:
          break;
        }
      break;
      default: /* src.PDS_data_type is not complex, so nothing to do.  */
      break;
    }
    OaLongtoKwdValue( "START_BYTE", current_node, 
                      (long) sdt_node_ptr->dst.start_offset+1);
    ptr = OaPDSDataTypetoStr( sdt_node_ptr->dst.PDS_data_type);
    OaStrtoKwdValue( "DATA_TYPE", current_node, ptr);
    OaLongtoKwdValue( "BYTES", current_node, (long) sdt_node_ptr->dst.size);
  break;

  case OA_HISTOGRAM:
    OaLongtoKwdValue( "ITEMS", current_node, sdt_node_ptr->total_repetitions);
    ptr = OaPDSDataTypetoStr( sdt_node_ptr->dst.PDS_data_type);
    OaStrtoKwdValue( "DATA_TYPE", current_node, ptr);
    OaLongtoKwdValue( "ITEM_BYTES", current_node, 
                      (long) sdt_node_ptr->dst.size);
    if (dst_interchange_format == OA_ASCII_INTERCHANGE_FORMAT)
      OaStrtoKwdValue(  "INTERCHANGE_FORMAT", current_node, "ASCII");
    else 
      OaStrtoKwdValue(  "INTERCHANGE_FORMAT", current_node, "BINARY");
  break;

  case OA_IMAGE:  
    ptr = OaPDSDataTypetoStr( sdt_node_ptr->dst.PDS_data_type);
    OaStrtoKwdValue( "SAMPLE_TYPE", current_node, ptr);
    OaLongtoKwdValue( "SAMPLE_BITS", current_node, sdt_node_ptr->dst.size*8);
  break;

  case OA_GAZETTEER:
  case OA_PALETTE:
  case OA_SERIES:
  case OA_SPECTRUM:
  case OA_TABLE:
    if (dst_interchange_format == OA_ASCII_INTERCHANGE_FORMAT)
      OaStrtoKwdValue(  "INTERCHANGE_FORMAT", current_node, "ASCII");
    else 
      OaStrtoKwdValue(  "INTERCHANGE_FORMAT", current_node, "BINARY");
    OaLongtoKwdValue( "ROWS", current_node, sdt_node_ptr->total_repetitions);
    OaLongtoKwdValue( "ROW_BYTES", current_node, sdt_node_ptr->dst.size);
    OaLongtoKwdValue( "COLUMNS", current_node, 
                      (long) OdlGetObjDescChildCount( current_node));
  break; 

  default: 
  break;
}
OalFreeSDTStruct( (SDT_node *) current_node->appl1);
current_node->appl1 = NULL;
return(0);
}



/*****************************************************************************

  Routine:  OalAttachItemOffsetNodes

  Description:  This routine is called by OalBuildSDTNode to make tree
                modifications to handle ITEM_OFFSET:
                Example from an ASCII TABLE:
                OBJECT        = COLUMN
                  ITEMS       = 3
                  ITEM_BYTES  = 2
                  ITEM_OFFSET = 5   The number of bytes from start of one item
                  BYTES       = 6   to the start of the next item.
                  START_BYTE  = 2
                END_OBJECT    = COLUMN
                Data:  "AA","BB","CC"<CR><LF>
             
                The following SDT subtree is added by this function:

                COLUMN0-----COLUMN1-----COLUMN2
                         |           |
                         |           ---COLUMN3
                         |
                         ---COLUMN4

                COLUMN0 is the original input node, kept so that the tree
                        traversal doesn't mess up on the added tree nodes;
                        a repetitions node with total_repetitions = 1.
                COLUMN1 is a repetitions node looping through ITEMS-1 items.
                COLUMN2 represents one data item.
                COLUMN3 represents the delimitors between data items.
                COLUMN4 represents the last data item; it can't be included in
                        COLUMN1's repetitions because it doesn't necessarily 
                        have the same delimitors following it.
                These nodes are added below the original input node COLUMN0,
                and to the right (after) any ALIAS or BIT_COLUMN nodes which
                may be present under COLUMN0.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   3 Dec  1994
  Last Modified:  21 Apr  1997

  History:

    Creation - This routine was part of the Beta Release of the OA library.
    02/14/95 - Changed input parameters to src_interchange_format and
               dst_interchange_format.
    04/21/97 - Code change to allow SDT's which preserve unused ODL tree
               nodes, which don't have SDT nodes attached (BIT_COLUMNS,
               ALIAS etc).  SM

  Input:  
         current_node           - An ODL tree node with an SDT node attached.
                                  The sdt node's src.PDS_data_type must be set.

         items                  - The ITEMS keyword value.

         item_bytes             - The ITEM_BYTES keyword value.

         item_offset            - The ITEM_OFFSET keyword value.

         src_interchange_format - Set to either
                                  OA_ASCII_INTERCHANGE_FORMAT or 
                                  OA_BINARY_INTERCHANGE_FORMAT.

         dst_interchange_format - Set to either
                                  OA_ASCII_INTERCHANGE_FORMAT or 
                                  OA_BINARY_INTERCHANGE_FORMAT.
         
  Output:  The function always returns 0.

  Notes:  This function was originally part of OalCreateSDT, but was
          broken out because OalCreateSDT was getting too big.

*****************************************************************************/

#ifdef _NO_PROTO

int OalAttachItemOffsetNodes( input_node, items, item_bytes, item_offset, 
                              src_interchange_format, dst_interchange_format)
ODLTREE input_node;
long items, item_bytes, item_offset;
int src_interchange_format, dst_interchange_format;
#else

int OalAttachItemOffsetNodes( ODLTREE input_node, long items, 
                              long item_bytes, long item_offset, 
                              int src_interchange_format, 
                              int dst_interchange_format)
#endif
{

ODLTREE columns[5], left_sibling;
SDT_node *sdt_node_ptrs[5], *tmp_sdt_node_ptr;
int PDS_data_type, i;

columns[0] = input_node;
sdt_node_ptrs[0] = (SDT_node *) input_node->appl1;
PDS_data_type = sdt_node_ptrs[0]->src.PDS_data_type;

/* Create all the new ODL tree nodes, attach SDT nodes to them and link
   them up.  */

for (i=1; i<5; i++) {
  columns[i] = OdlNewObjDesc( "COLUMN", NULL, NULL, NULL, NULL, NULL, 
                              (short) 0, (long) 0);
  sdt_node_ptrs[i] = OalNewSDTNode();
  columns[i]->appl1 = (char *) sdt_node_ptrs[i];
}
OdlPasteObjDesc( columns[1], columns[0]);
OdlPasteObjDesc( columns[2], columns[1]);
OdlPasteObjDescAfter( columns[3], columns[2]);
OdlPasteObjDescAfter( columns[4], columns[1]);

/* Set up COLUMN2, the data node for all but the last item.  */

sdt_node_ptrs[2]->total_repetitions = 1;
sdt_node_ptrs[2]->src.PDS_data_type = PDS_data_type;
sdt_node_ptrs[2]->src.start_offset = 0;
sdt_node_ptrs[2]->src.size = item_bytes;
OalDetermineConvertParameters( columns[2], src_interchange_format,
                               dst_interchange_format);

/* Set up COLUMN4, representing the last, lone data item (most of it is a copy
   of COLUMN2).  */

*sdt_node_ptrs[4] = *sdt_node_ptrs[2];
sdt_node_ptrs[4]->src.start_offset = (items-1) * item_offset;
sdt_node_ptrs[4]->dst.start_offset = sdt_node_ptrs[2]->dst.size * (items-1);

/* Set up COLUMN3 as a SPARE node which tosses out the data between the
   end of an item and the start of the next item.  */

sdt_node_ptrs[3]->total_repetitions = 1;
sdt_node_ptrs[3]->conversion_type = OA_NOT_APPLICABLE;
sdt_node_ptrs[3]->src.start_offset = item_bytes;
sdt_node_ptrs[3]->src.size = item_offset - item_bytes;
sdt_node_ptrs[3]->dst.size = 0;
sdt_node_ptrs[3]->dst.alignment_req = 1;

/* Set up COLUMN1, the repetitions node for all but the last item.  */

sdt_node_ptrs[1]->total_repetitions = items-1;
sdt_node_ptrs[1]->src.PDS_data_type = (char) -1;
sdt_node_ptrs[1]->dst.PDS_data_type = (char) -1;
sdt_node_ptrs[1]->src.start_offset = 0;
sdt_node_ptrs[1]->src.size = item_offset;
sdt_node_ptrs[1]->dst.start_offset = 0;
sdt_node_ptrs[1]->dst.size = sdt_node_ptrs[2]->dst.size;
sdt_node_ptrs[1]->dst.alignment_req = sdt_node_ptrs[2]->dst.alignment_req;

/* Finish setting up COLUMN0, the original COLUMN node.  Leave its
   src.start_offset the same as it was on entry.  */

sdt_node_ptrs[0]->total_repetitions = 1;
sdt_node_ptrs[0]->src.size = (items-1) * item_offset + item_bytes;
sdt_node_ptrs[0]->src.PDS_data_type = (char) -1;
sdt_node_ptrs[0]->dst.PDS_data_type = (char) -1;
sdt_node_ptrs[0]->dst.size = sdt_node_ptrs[2]->dst.size * items;
sdt_node_ptrs[0]->dst.alignment_req = sdt_node_ptrs[2]->dst.alignment_req;

/* Check if an alignment pad is needed before COLUMN0;  if so, add it.  */

OalDetermineRepNodeAlignment( columns[0]);

/* Set COLUMN0's dst.start_offset to be its left_sibling's end, if is has a
   left sibling, otherwise 0. */

if ((left_sibling = LeftSDTSibling( columns[0])) != NULL) {
  tmp_sdt_node_ptr = (SDT_node *) left_sibling->appl1;
  sdt_node_ptrs[0]->dst.start_offset = tmp_sdt_node_ptr->dst.start_offset +
    tmp_sdt_node_ptr->dst.size * tmp_sdt_node_ptr->total_repetitions;
} else {  /* No left sibling, so start at start_offset = 0.  */
  sdt_node_ptrs[0]->dst.start_offset = 0;
}
OalCheckForGapInSrcData( columns[0]);
return(0);
}



/*****************************************************************************

  Routine:  OalBuildSDTNode

  Description:  This routine attaches an SDT node to the input ODL tree node
                and initializes all its parameters:  the size, data type
                and offset of the source data, according to keywords in the ODL
                tree node, then the size, data type, alignment, in-memory
                offset and conversion type of the destination data, according
                to Oa_profile.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:  21 Apr   1997

  History:

    Creation - This routine was part of the Alpha Release of the OA library.
    10/12/94 - Added code to delete ALIAS nodes. SM
    02/14/95 - Changed input parameters to src_interchange_format and
               dst_interchange_format. SM
    03/07/96 - Added call to OalCheckForGapInSrcData for repetition nodes. SM
    04/21/97 - Various code changes to handle SDT's with unused ODL tree nodes which
               don't have SDT nodes attached (BIT_COLUMNS, ALIAS etc).  SM

  Input:  
         current_node - An ODL tree node.  All left siblings and children of
                        current_node must have already been visited by this
                        function.  The order in which nodes are visited within
                        PostOrderTraverse, which calls this function, 
                        guarantees this.

         src_interchange_format - Set to either
                                  OA_ASCII_INTERCHANGE_FORMAT or 
                                  OA_BINARY_INTERCHANGE_FORMAT.

         dst_interchange_format - Set to either
                                  OA_ASCII_INTERCHANGE_FORMAT or 
                                  OA_BINARY_INTERCHANGE_FORMAT.
         
  Output:  If successful, the function returns 0, otherwise a non-zero value.
           If current_node already had an SDT node attached, OalBuildSDTNode 
           returns 0 immediately, otherwise it attaches and initializes an
           SDT node as described below.

  Notes:  

  1) Constraint Checking:
     This function extensively uses PDS's object-class specific standards
     for keyword names, keyword semantics, and ODL tree structure rules;  thus it
     is effectively the main label-verifier of the OA Library.  (Less extensive 
     label verification is done in OaConvertLabel.)  The function flags an
     error if:
     a) A required keyword is missing, or the keyword value isn't the right
        type.
     b) A keyword value or combination of keyword values is inconsistent with
        other keyword values, or inconsistent with the structure of the tree.
     c) An ODL tree node with a particular object class requires the presence
        of one or more sub-objects, but they aren't there.
     d) An ODL tree node with a particular object class is only allowed as a
        sub-object of another object, isn't allowed to have sub-objects, 
        is required to have sub-object, etc, and one of these conditions is
        violated.

  2) Algorithm:

   - If current_node already had an SDT node attached, OalBuildSDTNode returns
     immediately.  This will occur for SPARE column nodes representing prefix
     or suffix tables, which were set up by OalCreateSDT before calling
     PostOrderTraverse,  which calls OalBuildSDTNode.
   - OalBuildSDTNode visits the top-level object node last, because of the
     order in which it is called by PostOrderTraverse.
   - Data nodes and pure repetitions nodes are treated completely separately
     in the code.  A few definitions to help to understand the algorithm:
     1) A "Data node" is a node which describes individual data atoms, 
        normally doesn't have children, and may have repetitions associated
        with it.  Examples are a COLUMN, ELEMENT, HISTOGRAM or IMAGE.
     2) A "Repetitions node" is a node which doesn't describe data, but has
        children which do. Examples are a TABLE, CONTAINER, COLLECTION, ARRAY.
     3) The SDT may have ODL tree nodes without SDT nodes attached.  These
        include ALIAS, GROUP and BIT_COLUMN.  They are preserved, but ignored
        during SDT construction.  They must be removed by OalCompressSDT before
        processing the SDT, because OalProcessSDT cannot handle them.  After
        SDT processing, the uncompressed SDT is returned to the object layer
        with the BIT_COLUMNS etc. intact.

     Algorithm for data-nodes:
     a) Determine from keyword values the SDT node parameters which describe 
        the source data:  src.PDS_data_type, src.start_offset and src.size of
        the first repetition of the object, and total_repetitions.
     b) Call OalDetermineConvertParameters to determine the SDT node
        parameters which describe the destination data:  dst.PDS_data_type, 
        dst.start_offset from its parent, dst.size of the first repetition,
        the object's in-memory alignment requirement and dst.start_offset.  
        Based on source and destination data types, set conversion_type to 
        OA_BINREP, OA_ASCII_TO_BINARY, OA_BINARY_TO_ASCII or OA_MEMCPY.
        In some cases, dst.size will be set to 0, in which case conversion_type
        is not applicable.
     c) Call OalCheckForGapInSrcData to check for a gap in the src data
        description before the current node; if one is found it adds an
        explicit SPARE before the current node.

     Algorithm for repetition nodes:
     a) Determine src.size, src.start_offset and total_repetitions from
        keyword values.
     b) Call OalDetermineRepNodeAlignment to determine the dst alignment 
        requirement, based on the most restrictive alignment requirement of 
        all the node's children, and add a SPARE node to the left if needed.
     c) Set dst.size to be the rightmost child's dst.start_offset plus its
        dst.size multiplied by its total_repetitions.  The dst.size of a
        repetitions node is the total size of one repetition of all its
        children.  The total dst size encompassed by a repetitions node is its
        dst.size * total_repetitions.
     d) Set dst.start_offset to be just past the node's left_sibling's end, if
        it has a left sibling, otherwise 0.
     e) Call OalCheckForGapInSrcData to check for a gap in the src data
        description before the repetition node, and if one is found, to add an
        explicit SPARE before the current node.
     f) Check if the src size of the repetition node's subobjects is the same
        as the src size calculated in 1).  If not, add a SPARE node as a 
        rightmost child to make up the difference.

  3) In-Memory Object Size Falls Out At The End:
     After OalBuildSDTNode processes the last node (the "top-level object" 
     node), that node's total_repetitions * dst.size is the final in-memory
     size of the object.  Object Layer routines use this size in a call to
     allocate memory for the object. The pointer returned by OaMalloc is the
     starting address from which all offsets are added when processing a slice
     and actually storing the destination data in memory (see OalProcessSDT).

  4) Special Cases For BIT_COLUMN, BIT_ELEMENT and ARRAY:
     These involve creating and/or deleting ODL tree nodes, and are documented
     in the code.

  5) Gaps In Source Data Description:
     The ODL tree up to the input node describes the source data completely;
     wherever implicit spares (gaps in the source description) were detected,
     SPARE nodes have been added.  OalProcessSDT requires these to keep its
     position in the input stream when the stream is broken in the middle
     of a gap.  Source data not mapped to the destination (prefix bytes, 
     suffix bytes and SPARE node data for binary->ASCII conversions) will have
     dst.size=0.

  6) Gaps In Destination Data Description:
     The destination offsets and sizes given by the SDT nodes dst structure 
     MAY have gaps and holes in it.  This is to add pad bytes in order to 
     align the next data atom.  Only if the alignment requirement is 
     NO_ALIGN will there not be any gaps in the destination data.  If the
     source data had gaps or implicit spares (like the double quote column
     delimiters in an ASCII table), the dst data will be compacted by
     removal of these.

*****************************************************************************/

#ifdef _NO_PROTO

int OalBuildSDTNode( current_node, src_interchange_format, 
                     dst_interchange_format)
ODLTREE current_node;
int src_interchange_format;
int dst_interchange_format;
#else

int OalBuildSDTNode( ODLTREE current_node, int src_interchange_format,
                     int dst_interchange_format)
#endif
{

static char *proc_name = "OalBuildSDTNode";
int alignment_type;
SDT_node *sdt_node_ptr, *tmp_sdt_node_ptr;
ODLTREE next_node, left_sibling, rightmost_child, spare_node;
char *str, *text;
int n_dims, object_class, parent_object_class, pad_bytes, i;
int sequence_items, tmpint1, tmpint2;
long first_start_bit, last_start_bit, total_bits, tmp_long, *axis_items;
long bytes, items, item_offset, item_bytes, rows, row_bytes;
long prefix_bytes=0, suffix_bytes=0;

alignment_type = Oa_profile.dst_alignment_type;

/* Check input parameters. */

if ((src_interchange_format != OA_ASCII_INTERCHANGE_FORMAT) &&
    (src_interchange_format != OA_BINARY_INTERCHANGE_FORMAT)) {
  sprintf( error_string, "%s: invalid src_interchange_format: %d",
           proc_name, src_interchange_format);
  oa_errno = 531;
  OaReportError( error_string);
  return(1);
}

if ((dst_interchange_format != OA_ASCII_INTERCHANGE_FORMAT) &&
    (dst_interchange_format != OA_BINARY_INTERCHANGE_FORMAT)) {
  sprintf( error_string, "%s: invalid dst_interchange_format: %d.",
           proc_name, dst_interchange_format);
  oa_errno = 531;
  OaReportError( error_string);
  return(1);
}

if ((alignment_type != OA_NO_ALIGN) &&
    (alignment_type != OA_ALIGN_EVEN) &&
    (alignment_type != OA_ALIGN_RISC)) {
  sprintf( error_string, "%s: invalid Oa_profile.dst_alignment_type: %d.",
           proc_name, alignment_type);
  oa_errno = 730;
  OaReportError( error_string);
  return(1);
}


/* Check if an SDT node has already been attached (previous special setup by
   the caller, not common).  If so, return; if not, call OalNewSDTNode to
   malloc a new SDT node and initialize with 0's, and attach it to the ODL
   tree node.  */

if (current_node->appl1 != NULL)
  return(0);
else {
  sdt_node_ptr = OalNewSDTNode();
  current_node->appl1 = (char *) sdt_node_ptr;
}

/* Get the object class. */

object_class = OaGetObjectClass( current_node);

/* If the node has no children with an SDT node attached), do the following
   block of code for data nodes;  otherwise do the block of code for
   repetitions nodes much further down.  */

if ((next_node = LeftmostSDTChild( current_node)) == NULL) {

  /***************************************************************************
                          DATA NODES
  ***************************************************************************/

  /* No children with an SDT node attached, so this is a data node; determine
     src.PDS_data_type. */

  switch( object_class) {

    case OA_ALIAS:
    case OA_BIT_COLUMN:

      /* ALIAS and BIT_COLUMN nodes need to be preserved for informational 
         purposes (in the final destination tree returned to the Object Layer),
         so leave them in the tree, but delete the SDT node. 
         A BIT_COLUMN just describes a subset of the data already described by
         the obligatory COLUMN node above it, so isn't needed for the SDT. */

      OalFreeSDTStruct( (SDT_node *) current_node->appl1);
      current_node->appl1 = NULL;
      return(0);
    break;

    case OA_BIT_ELEMENT:

    /* Unlike a BIT_COLUMN, a BIT_ELEMENT is only found below a COLLECTION or
       ARRAY;  thus if its bits aren't a multiple of 8, and it needs to be 
       combined with adjacent BIT_ELEMENTS to make a byte-sized affair that
       can be represented in the SDT.  Since PDS hasn't yet finalized the
       BIT_ELEMENT definition (especially as regards to LSB/MSB), use
       OA_MSB_BIT_STRING for now.  */

       sdt_node_ptr->src.PDS_data_type = OA_MSB_BIT_STRING;

    break;

    case OA_IMAGE:
      if (OaKwdValuetoStr( "SAMPLE_TYPE", current_node, &str) == 0)
          sdt_node_ptr->src.PDS_data_type = OaStrtoPDSDataType( str,
                                              src_interchange_format);
      else {
        sprintf( error_string, "%s: SAMPLE_TYPE keyword value not found", 
                 proc_name);
        strcat( error_string, " for IMAGE object.");
        oa_errno = 531;
        OaReportError( error_string);
        return(1);
      }
    break;

    default:
      if (OaKwdValuetoStr( "DATA_TYPE", current_node, &str) == 0)
        sdt_node_ptr->src.PDS_data_type = OaStrtoPDSDataType( str, 
                                            src_interchange_format);
      else {
        sprintf( error_string, 
                 "%s: DATA_TYPE keyword not found for %s object",
                 proc_name, OaObjectClasstoStr( object_class));
        if (OaKwdValuetoStr( "NAME", current_node, &str) == 0) {
          sprintf( error_string + strlen( error_string),
                   "\nwith NAME = %s.", str);
        }
        oa_errno = 531;
        OaReportError( error_string);
        return(1);
      }
    break;
  }  /* end switch on object_class for DATA_TYPE or equivalent keyword */

  /* If DATA_TYPE is unknown, then only if it's a SPARE is it OK (probably
     has "DATA_TYPE = N/A"); otherwise report error and return.  */

  if (sdt_node_ptr->src.PDS_data_type == OA_UNKNOWN_DATA_TYPE) {
    if (OaKwdValuetoStr( "NAME", current_node, &str) == 0) {
      if (strcmp( str, "SPARE") != 0) {
        OaKwdValuetoStr( "DATA_TYPE", current_node, &text);
        sprintf( error_string, 
                 "%s: DATA_TYPE keyword value: %s in\n", proc_name, text);
        sprintf( error_string + strlen( error_string),
                 "%s is an unknown data type", current_node->class_name);
        if (OaKwdValuetoStr( "NAME", current_node, &str) == 0) {
          sprintf( error_string + strlen( error_string),
                   ", NAME = %s.", str);
        }
        oa_errno = 531;
        OaReportError( error_string);
        return(1);
      } /* it's not a SPARE */
    } else {  /* NAME keyword not found */
      OaKwdValuetoStr( "DATA_TYPE", current_node, &text);
      sprintf( error_string, "%s: DATA_TYPE keyword value: %s in\n",
               proc_name, text);
      sprintf( error_string + strlen( error_string),
               "%s node is an unknown data type.", current_node->class_name);
      oa_errno = 531;
      OaReportError( error_string);
      return(1);
    }
  }

  /* Determine src.size and src.start_offset from object-specific keywords. */

  switch (object_class) {

    case OA_ARRAY:
    case OA_COLLECTION:
    case OA_CONTAINER:
    case OA_GAZETTEER:
    case OA_PALETTE:
    case OA_SERIES:
    case OA_SPECTRUM:
    case OA_TABLE:
      sprintf( error_string, "%s: error: %s object has no sub-object.",
               proc_name, OaObjectClasstoStr( object_class));
      oa_errno = 530;
      OaReportError( error_string);
      return(1);

    case OA_BIT_ELEMENT:

      /* Special case for BIT_ELEMENTS:
         1) If below an ARRAY, can't handle it; report error and return.
         2) If below a COLLECTION, combine adjacent BIT_ELEMENTs into an SDT
            node describing an integral number of bytes, and delete the SDT
            nodes from all but the right-most BIT_ELEMENT. */

      parent_object_class = OaGetObjectClass( Parent( current_node));
      switch (parent_object_class) {

        case OA_ARRAY:   /* Can't handle this case; report an error. */
          sprintf( error_string, 
         "%s: can't handle a BIT_ELEMENT node directly under an ARRAY node.",
                   proc_name);
          oa_errno = 520;
          OaReportError( error_string);
          return(1);
          /*NOTREACHED*/
        break;

        case OA_COLLECTION:

          /* If current_node has a right sibling which is also a BIT_ELEMENT,
             then delete current_node's SDT node;  all the BIT_ELEMENT nodes
             will have their SDT nodes removed by the time we get to the
             right-most one.  */

          if (RightSibling( current_node) != NULL) {
            if ((OaGetObjectClass( RightSibling( current_node))) ==
                OA_BIT_ELEMENT) {
              OalFreeSDTStruct( (SDT_node *) current_node->appl1);
              current_node->appl1 = NULL;
              return(0);
              /*NOTREACHED*/
            }
          }

          /* If we got to here, then current_node is the right-most of a series
             of BIT_ELEMENTs; examine all its left siblings until get to one
             which isn't a BIT_ELEMENT; then add an SDT node to the rightmost
             BIT_ELEMENT which describes all the BIT_ELEMENTS.  */

          left_sibling = LeftSibling( current_node);
          while (left_sibling != NULL) {

            if (OaGetObjectClass( left_sibling) == OA_BIT_ELEMENT) {

              /* Always save the leftmost node's start bit for later use.  */

              first_start_bit = 1;
              if ((OaKwdValuetoLong( "START_BIT", left_sibling, 
                                     &first_start_bit)) != 0) {
                if ((OaKwdValuetoLong( "START_BYTE", left_sibling, 
                                       &first_start_bit)) != 0)
                  first_start_bit = first_start_bit * 8;
              }
              left_sibling = LeftSDTSibling( left_sibling);
            } else break;
          }   /* end while */
       
          /* Initialize the SDT node.  */          

          last_start_bit = 1;
          if ((OaKwdValuetoLong( "START_BIT", current_node,
                                 &last_start_bit)) != 0) {
            if ((OaKwdValuetoLong( "START_BYTE", current_node, 
                                   &last_start_bit)) != 0)
              last_start_bit = last_start_bit * 8;
          }
          if ((OaKwdValuetoLong( "BITS", current_node, &total_bits)) != 0) {
            sprintf( error_string,
                     "%s: couldn't find BITS keyword in %s object.",
                     proc_name, OaObjectClasstoStr( object_class));
            OaReportError( error_string);
            return(1);
          }
        
          total_bits = last_start_bit - first_start_bit + 1 + total_bits;
          if ((total_bits % 8) > 0) {
            sprintf( error_string,
  "%s: Total number of bits in adjacent BIT_ELEMENTS must be a multiple of 8.",
                     proc_name);
            oa_errno = 530;
            OaReportError( error_string);
            return(1);
          }
          sdt_node_ptr->total_repetitions = 1;
          sdt_node_ptr->src.start_offset = (first_start_bit-1)/8;
          sdt_node_ptr->src.size = total_bits/8;
        break;

        default:
          sprintf( error_string,
                   "%s: can't handle a BIT_ELEMENT under a %s.",
                   proc_name, OaObjectClasstoStr( parent_object_class));
          oa_errno = 530;
          OaReportError( error_string);
          return(1);
          /*NOTREACHED*/
        break;
      }
    break;

    case OA_COLUMN:

      if (OaKwdValuetoLong( "START_BYTE", current_node, &tmp_long) == 0)
        sdt_node_ptr->src.start_offset = tmp_long-1;
      else {
        sprintf( error_string, "%s: START_BYTE keyword value not found", 
                 proc_name);
        strcat( error_string, " for COLUMN object.");
        oa_errno = 531;
        OaReportError( error_string);
        return(1);
      }

      if (OaKwdValuetoLong( "ITEMS", current_node, &items) == 0) {
        if (OaKwdValuetoLong( "BYTES", current_node, &bytes) != 0) {
          sprintf( error_string, "%s: BYTES keyword value not found", 
                   proc_name);
          strcat( error_string, " for COLUMN object.");
          oa_errno = 531;
          OaReportError( error_string);
          return(1);
        }
        if (OaKwdValuetoLong( "ITEM_OFFSET", current_node,  
                               &item_offset) != 0) {
          sdt_node_ptr->total_repetitions = items;
          sdt_node_ptr->src.size = bytes/items;
        } else {
          if (OaKwdValuetoLong( "ITEM_BYTES", current_node, 
                                 &item_bytes) != 0) {
            sprintf( error_string, "%s: ITEM_BYTES keyword value not found", 
                     proc_name);
            strcat( error_string, " for COLUMN object.");
            oa_errno = 531;
            OaReportError( error_string);
            return(1);
          }
          OalAttachItemOffsetNodes( current_node, items, item_bytes, 
                                    item_offset, src_interchange_format,
                                    dst_interchange_format);
          return(0);
        }
      } else {
        sdt_node_ptr->total_repetitions = 1;
        if (OaKwdValuetoLong( "BYTES", current_node, &tmp_long) == 0)
          sdt_node_ptr->src.size = tmp_long;
        else {
          sprintf( error_string, "%s: BYTES keyword value not found", 
                   proc_name);
          strcat( error_string, " for COLUMN object.");
          oa_errno = 531;
          OaReportError( error_string);
          return(1);
        }
      }
    break;

    case OA_ELEMENT:
      if (OaKwdValuetoLong( "START_BYTE", current_node, &tmp_long) == 0)
        sdt_node_ptr->src.start_offset = tmp_long-1;
      else {
        sdt_node_ptr->src.start_offset = 0;
      }
      sdt_node_ptr->total_repetitions = 1;
      if (OaKwdValuetoLong( "BYTES", current_node, &tmp_long) == 0)
        sdt_node_ptr->src.size = tmp_long;
      else {
        sprintf( error_string, "%s: BYTES keyword value not found", 
                 proc_name);
        strcat( error_string, " for ELEMENT object.");
        oa_errno = 531;
        OaReportError( error_string);
        return(1);
      }
    break;

    case OA_HISTOGRAM:
      if (OaKwdValuetoLong( "ITEM_BYTES", current_node, &tmp_long) == 0)
        sdt_node_ptr->src.size = tmp_long;
      else {
        sprintf( error_string, "%s: ITEM_BYTES keyword value not found", 
                 proc_name);
        strcat( error_string, " for HISTOGRAM object.");
        oa_errno = 531;
        OaReportError( error_string);
        return(1);
      }

      if (OaKwdValuetoLong( "ITEMS", current_node, &tmp_long) == 0)
        sdt_node_ptr->total_repetitions = tmp_long;
      else {
        sprintf( error_string, "%s: ITEMS keyword value not found", 
                 proc_name);
        strcat( error_string, " for HISTOGRAM object.");
        oa_errno = 531;
        OaReportError( error_string);
        return(1);
      }
    break;

    case OA_IMAGE:  
      if (OaKwdValuetoLong( "SAMPLE_BITS", current_node, &tmp_long) == 0)
        sdt_node_ptr->src.size = tmp_long/8;
      else {
        sprintf( error_string, "%s: SAMPLE_BITS keyword value not found", 
                 proc_name);
        strcat( error_string, " for IMAGE object.");
        oa_errno = 531;
        OaReportError( error_string);
        return(1);
      }

      if (OaKwdValuetoLong( "LINES", current_node, &tmp_long) == 0)
        sdt_node_ptr->total_repetitions = tmp_long;
      else {
        sprintf( error_string, "%s: LINES keyword value not found", 
                 proc_name);
        strcat( error_string, " for IMAGE object.");
        oa_errno = 531;
        OaReportError( error_string);
        return(1);
      }

      if (OaKwdValuetoLong( "LINE_SAMPLES", current_node, &tmp_long) == 0)
        sdt_node_ptr->total_repetitions *= tmp_long;
      else {
        sprintf( error_string, "%s: LINE_SAMPLES keyword value not found", 
                 proc_name);
        strcat( error_string, " for IMAGE object.");
        oa_errno = 531;
        OaReportError( error_string);
        return(1);
      }
      break;

    default: 
      sprintf( error_string, "%s: Unknown object class.", proc_name);
      oa_errno = 530;
      OaReportError( error_string);
      return(1);
  }

  OalDetermineConvertParameters( current_node, src_interchange_format,
                                 dst_interchange_format);

  /* A cludge for COMPLEX data types is distributed between this function and 
     the data types profile, oa_type_conversion_info.  The profile maps a
     complex (2 adjacent reals) to a single real, so the function called above,
     OalDetermineConvertParameters, has set the dst as if it were a single
     real;  now multiply repetitions by two, and divide src.size by two.  */

  switch (sdt_node_ptr->src.PDS_data_type) {
    case OA_COMPLEX:
    case OA_IBM_COMPLEX:
    case OA_PC_COMPLEX:
    case OA_VAX_COMPLEX:
    case OA_VAXG_COMPLEX:
      sdt_node_ptr->total_repetitions *= 2;
      sdt_node_ptr->src.size /= 2;
    break;
    default:
    break;
  }

  /* Determine the dst.start_offset, which depends on where the previous object
     ended, and the alignment just determined.  If the previous object didn't
     end on the boundary required, pad the difference by adding bytes to 
     dst.start_offset.  */

  if ((left_sibling = LeftSDTSibling( current_node)) != NULL) {
    tmp_sdt_node_ptr = (SDT_node *) left_sibling->appl1;
    pad_bytes = (tmp_sdt_node_ptr->dst.start_offset + 
                 tmp_sdt_node_ptr->dst.size * 
                 tmp_sdt_node_ptr->total_repetitions) % 
                 sdt_node_ptr->dst.alignment_req;
    if (pad_bytes > 0) 
      pad_bytes = sdt_node_ptr->dst.alignment_req - pad_bytes;
    sdt_node_ptr->dst.start_offset = tmp_sdt_node_ptr->dst.start_offset +
                                     tmp_sdt_node_ptr->dst.size *
                                     tmp_sdt_node_ptr->total_repetitions +
                                     pad_bytes;
  } else {
    sdt_node_ptr->dst.start_offset = 0;
  }

  OalCheckForGapInSrcData( current_node);
  return(0);


} else {  /* Current node is a repetitions node enclosing sub-nodes. */
  
  /***************************************************************************
                          REPETITIONS NODES
  ***************************************************************************/

  /*  Since the node has children with SDT nodes attached, it's not a data
      node, so src and dst PDS_data_type are not applicable.  */

  sdt_node_ptr->src.PDS_data_type = (char) -1;
  sdt_node_ptr->dst.PDS_data_type = (char) -1;

  /* Determine src.size, src.start_offset and total_repetitions from object-
     specific keyword values.  */

  switch (object_class) {

    case OA_BIT_COLUMN:
    case OA_BIT_ELEMENT: 
    case OA_HISTOGRAM:
    case OA_IMAGE:  
      sprintf( error_string, "%s: a %s object cannot contain sub-objects.", 
               proc_name, OaObjectClasstoStr( object_class));
      oa_errno = 530;
      OaReportError( error_string);
      return(1);
      /*NOTREACHED*/
    break;

    case OA_ARRAY:

      if (OaKwdValuetoLong( "START_BYTE", current_node, &tmp_long) == 0)
        sdt_node_ptr->src.start_offset = tmp_long-1;
      else
        sdt_node_ptr->src.start_offset = 0;
      if (OaKwdValuetoLong( "AXES", current_node, &tmp_long) == 0)
        n_dims = tmp_long;
      else {
        sprintf( error_string, "%s: AXES keyword value not found", 
                 proc_name);
        strcat( error_string, " for ARRAY object.");
        oa_errno = 531;
        OaReportError( error_string);
        return(1);
      }
      if (OdlFindKwd( current_node, "AXIS_ITEMS", "*", (unsigned long) 0, 
                     (unsigned short) ODL_THIS_OBJECT) == NULL) {
        sprintf( error_string, "%s: AXIS_ITEMS keyword value not found", 
                 proc_name);
        strcat( error_string, " for ARRAY object.");
        oa_errno = 531;
        OaReportError( error_string);
        return(1);
      }
      if (OaSequencetoLongArray( "AXIS_ITEMS", current_node,
                                  &axis_items, &sequence_items) == 0) {
        if (sequence_items == n_dims) {
          sdt_node_ptr->total_repetitions = 1;
          for (i=0; i<n_dims; i++)
            sdt_node_ptr->total_repetitions *= axis_items[i];
          LemmeGo( axis_items);
        } else {
          sprintf( error_string,
           "%s: number of sequence elements in AXIS_ITEMS: %d does ",
            proc_name, sequence_items);
          sprintf( error_string + strlen( error_string), 
            "not agree with AXES keyword value: %d.", n_dims);
          oa_errno = 532;
          OaReportError( error_string);
          LemmeGo( axis_items);
          return(1);
        }
      } else {
        return(1);  /* Error parsing AXIS_ITEMS value, error msg issued.  */
      }

      /* Now that we know how many elements there are in the array, find
         out how big each element is from the child of the ARRAY.  Since
         the child has already been visited by OalPostOrderTraverse, the
         information has already been stored in an SDT node by a previous 
         call to BuildSDTNode.  */

      if ((next_node = LeftmostSDTChild( current_node)) == NULL) {
        sprintf( error_string, "%s: ARRAY must contain a sub-object.", 
                 proc_name);
        oa_errno = 530;
        OaReportError( error_string);
        return(1);
      }
      tmp_sdt_node_ptr = (SDT_node *) next_node->appl1;
      sdt_node_ptr->src.size = tmp_sdt_node_ptr->src.size *
                               tmp_sdt_node_ptr->total_repetitions;

      /* Check that the ARRAY has only one child, per PDS Standards. */
	  /* 12-07-04 MDC - TEMPORARY CHANGE */
/*      if (LeftmostSDTChild( current_node) != 
          RightmostSDTChild( current_node)) {
        sprintf( error_string, 
                 "%s: ARRAY cannot contain more than one sub-object.", 
                 proc_name);
        oa_errno = 530;
        OaReportError( error_string);
        return(1);
      }*/
    break;

    case OA_COLLECTION:
      if (OaKwdValuetoLong( "START_BYTE", current_node, &tmp_long) == 0)
        sdt_node_ptr->src.start_offset = tmp_long-1;
      else
        sdt_node_ptr->src.start_offset = 0;
      sdt_node_ptr->total_repetitions = 1;
      if (OaKwdValuetoLong( "BYTES", current_node, &tmp_long) == 0)
        sdt_node_ptr->src.size = tmp_long;
      else {
        sprintf( error_string, "%s: BYTES keyword value not found", 
                 proc_name);
        strcat( error_string, " for COLLECTION object.");
        oa_errno = 531;
        OaReportError( error_string);
        return(1);
        /*NOTREACHED*/
      }

      /* Check that the COLLECTION has at least one child.  */

      if (LeftmostSDTChild( current_node) == NULL) {
        sprintf( error_string, 
                 "%s: COLLECTION must contain at least one sub-object.", 
                 proc_name);
        oa_errno = 530;
        OaReportError( error_string);
        return(1);
      }
    break;

    case OA_COLUMN:

      if (OaKwdValuetoLong( "START_BYTE", current_node, &tmp_long) == 0)
        sdt_node_ptr->src.start_offset = tmp_long-1;
      else {
        sprintf( error_string, "%s: START_BYTE keyword value not found", 
                 proc_name);
        strcat( error_string, " for COLUMN object.");
        oa_errno = 531;
        OaReportError( error_string);
        return(1);
      }

      if (OaKwdValuetoLong( "ITEMS", current_node, &tmp_long) == 0) {
        sdt_node_ptr->total_repetitions = tmp_long;
        if (OaKwdValuetoLong( "BYTES", current_node, &tmp_long) == 0)
          sdt_node_ptr->src.size = tmp_long/ sdt_node_ptr->total_repetitions;
        else {
          sprintf( error_string, "%s: BYTES keyword value not found", 
                   proc_name);
          strcat( error_string, " for COLUMN object.");
          oa_errno = 531;
          OaReportError( error_string);
          return(1);
          /*NOTREACHED*/
        }
      } else {
        sdt_node_ptr->total_repetitions = 1;
        if (OaKwdValuetoLong( "BYTES", current_node, &tmp_long) == 0)
          sdt_node_ptr->src.size = tmp_long;
        else {
          sprintf( error_string, "%s: BYTES keyword value not found", 
                   proc_name);
          strcat( error_string, " for COLUMN object.");
          oa_errno = 531;
          OaReportError( error_string);
          return(1);
          /*NOTREACHED*/
        }
      }
      break;

    case OA_CONTAINER:
      if (OaKwdValuetoLong( "START_BYTE", current_node, &tmp_long) == 0)
        sdt_node_ptr->src.start_offset = tmp_long-1;
      else {
        sprintf( error_string, "%s: START_BYTE keyword value not found", 
                 proc_name);
        strcat( error_string, " for CONTAINER object.");
        oa_errno = 531;
        OaReportError( error_string);
        return(1);
      }
      if (OaKwdValuetoLong( "REPETITIONS", current_node, &tmp_long) == 0)
        sdt_node_ptr->total_repetitions = tmp_long;
      else {
        sprintf( error_string, "%s: REPETITIONS keyword value not found", 
                 proc_name);
        strcat( error_string, " for CONTAINER object.");
        oa_errno = 531;
        OaReportError( error_string);
        return(1);
      }
      if (OaKwdValuetoLong( "BYTES", current_node, &tmp_long) == 0)
        sdt_node_ptr->src.size = tmp_long;
      else {
        sprintf( error_string, "%s: BYTES keyword value not found", 
                 proc_name);
        strcat( error_string, " for ELEMENT object.");
        oa_errno = 531;
        OaReportError( error_string);
        return(1);
      }
    break;

    case OA_ELEMENT:
      if (OaKwdValuetoLong( "START_BYTE", current_node, &tmp_long) == 0)
        sdt_node_ptr->src.start_offset = tmp_long-1;
      else {
        sdt_node_ptr->src.start_offset = 0;
      }
      sdt_node_ptr->total_repetitions = 1;
      if (OaKwdValuetoLong( "BYTES", current_node, &tmp_long) == 0)
        sdt_node_ptr->src.size = tmp_long;
      else {
        sprintf( error_string, "%s: BYTES keyword value not found", 
                 proc_name);
        strcat( error_string, " for ELEMENT object.");
        oa_errno = 531;
        OaReportError( error_string);
        return(1);
      }
    break;

    case OA_GAZETTEER:
    case OA_PALETTE:
    case OA_SERIES:
    case OA_SPECTRUM:
    case OA_TABLE:
      if (OaGetTableKeywords( current_node, &rows, &row_bytes,
                              &prefix_bytes, &suffix_bytes, 
                              &tmpint1, &tmpint2) != 0)
        return(-1);  /* Error message already written and oa_errno set.  */

      sdt_node_ptr->src.size = row_bytes + prefix_bytes + suffix_bytes;
      sdt_node_ptr->total_repetitions = rows;
      sdt_node_ptr->src.start_offset = 0;
    break;

    default: 
      sprintf( error_string, "%s: Unknown object class.", proc_name);
      oa_errno = 530;
      OaReportError( error_string);
      return(1);
  }

  OalDetermineRepNodeAlignment( current_node);

  OalCheckForGapInSrcData( current_node);

  /* Check if a gap (implicit spare) in the src description exists at the end:
     detectable when end of the rightmost child is less than the src size
     determined from the repetition node's keywords.  If so, add a SPARE as
     the new rightmost child of the repetitions node to make up the
     difference.  */

  rightmost_child = RightmostSDTChild( current_node);
  tmp_sdt_node_ptr = (SDT_node *) rightmost_child->appl1;
  pad_bytes = sdt_node_ptr->src.size - (tmp_sdt_node_ptr->src.start_offset + 
                tmp_sdt_node_ptr->src.size * 
                tmp_sdt_node_ptr->total_repetitions);
  if (pad_bytes > 0) {
    sdt_node_ptr = tmp_sdt_node_ptr;
    spare_node = OdlNewObjDesc( "COLUMN", NULL, NULL, NULL, NULL, NULL, 
                                (short) 0, (long) 0);
    OaStrtoKwdValue( "NAME", spare_node, "SPARE");
    OdlPasteObjDescAfter( spare_node, rightmost_child);
    tmp_sdt_node_ptr = OalNewSDTNode();
    spare_node->appl1 = (char *) tmp_sdt_node_ptr;
    tmp_sdt_node_ptr->total_repetitions = 1;
    tmp_sdt_node_ptr->src.start_offset = 
      sdt_node_ptr->src.start_offset + sdt_node_ptr->src.size *
      sdt_node_ptr->total_repetitions;
    tmp_sdt_node_ptr->src.size = pad_bytes;
    tmp_sdt_node_ptr->conversion_type = OA_NOT_APPLICABLE;
    tmp_sdt_node_ptr->dst.size = 0;
    tmp_sdt_node_ptr->dst.alignment_req = 1;

    sprintf( error_string, 
     "%s: implicit SPARE detected after RightmostChild of repetitions node; ", 
             proc_name);
    sprintf( error_string + strlen( error_string), 
             "inserted a SPARE COLUMN node after %s node, with src.size: %d", 
             rightmost_child->class_name, (int) pad_bytes);
    sprintf( error_string + strlen( error_string), 
             " and src.start_offset = %ld.", 
             tmp_sdt_node_ptr->src.start_offset);
    oa_errno = 900;
    OaReportError( error_string);
  }  /* end if needed to add spare */

  /* Set dst.size of the repetitions node to be the offset plus size of its
     rightmost child (if its rightmost child is a SPARE with dst.size=0, then
     use the next node left of the rightmost child).  Dst.size is always the 
     size of a single repetition.  */

  sdt_node_ptr = (SDT_node *) current_node->appl1;
  next_node = RightmostSDTChild( current_node);
  tmp_sdt_node_ptr = (SDT_node *) next_node->appl1;
  if (tmp_sdt_node_ptr->dst.size == 0) {
    next_node = LeftSDTSibling( next_node);
    tmp_sdt_node_ptr = (SDT_node *) next_node->appl1;
  }
  sdt_node_ptr->dst.size = tmp_sdt_node_ptr->dst.start_offset +
     tmp_sdt_node_ptr->dst.size * tmp_sdt_node_ptr->total_repetitions;

  /* Set the repetition node's dst.start_offset to be its left_sibling's end, 
     if is has a left sibling, otherwise 0. */

  if ((next_node = LeftSDTSibling( current_node)) != NULL) {
    tmp_sdt_node_ptr = (SDT_node *) next_node->appl1;
    sdt_node_ptr->dst.start_offset = 
      tmp_sdt_node_ptr->dst.start_offset +
      tmp_sdt_node_ptr->dst.size * tmp_sdt_node_ptr->total_repetitions;
  } else {  /* No left sibling, so start at start_offset = 0.  */
    sdt_node_ptr->dst.start_offset = 0;
  }
  return(0);
}   /* end else repetitions node */
}



/*****************************************************************************

  Routine:  OalCheckForGapInSrcData

  Description:   This function checks if there's a gap (implicit spare) in 
                 the src description between the input_node's start_offset 
                 and its left sibling's end, or if it has no left sibling,
                 checks if it's src.start_offset is greater than 0.  If a gap
                 is detected, the function adds a new SPARE COLUMN node, 
                 pad_bytes in size, just left of current_node.  
                 (If the input_node is the top-most repetition node, it won't
                 have a parent, thus adding a left sibling is impossible; 
                 however a top-most repetition node should always have 
                 src.start_offset=0, so won't ever happen.)

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   2 Dec  1994
  Last Modified:   2 Dec  1994

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:
         input_node - An ODL tree node in an SDT with and SDT node attached,
                      describing a data node (not a repetitions node).
         
  Output:  If a gap was detected in the source data description, then a new
           ODL tree node with attached SDT node is added as a left_sibling of
           the input_node.  The function always returns 0.

  Notes:  This function was originally part of OalBuildSDT, but was
          broken out because OalBuildSDT was too big.

*****************************************************************************/

#ifdef _NO_PROTO

int OalCheckForGapInSrcData( input_node)
ODLTREE input_node;

#else

int OalCheckForGapInSrcData( ODLTREE input_node)

#endif
{

static char *proc_name = "OalCheckForGapInSrcData";
long pad_bytes;
ODLTREE left_sibling, spare_node;
SDT_node *input_sdt_node_ptr, *tmp_sdt_node_ptr;

input_sdt_node_ptr = (SDT_node *) input_node->appl1;
pad_bytes = 0;

if ((left_sibling = LeftSDTSibling( input_node)) == NULL)
  pad_bytes = input_sdt_node_ptr->src.start_offset;
else {
  tmp_sdt_node_ptr = (SDT_node *) left_sibling->appl1;
  pad_bytes = input_sdt_node_ptr->src.start_offset -
              (tmp_sdt_node_ptr->src.start_offset + 
               tmp_sdt_node_ptr->src.size *
               tmp_sdt_node_ptr->total_repetitions);
}
if (pad_bytes > 0) {
  spare_node = OdlNewObjDesc( "COLUMN", NULL, NULL, NULL, NULL, NULL, 
                              (short) 0, (long) 0);
  OaStrtoKwdValue( "NAME", spare_node, "SPARE");
  OdlPasteObjDescBefore( spare_node, input_node);
  tmp_sdt_node_ptr = OalNewSDTNode();
  spare_node->appl1 = (char *) tmp_sdt_node_ptr;

  tmp_sdt_node_ptr->total_repetitions = 1;
  tmp_sdt_node_ptr->src.start_offset = input_sdt_node_ptr->src.start_offset - 
                                       pad_bytes;
  tmp_sdt_node_ptr->src.size = pad_bytes;
  tmp_sdt_node_ptr->conversion_type = OA_NOT_APPLICABLE;
  tmp_sdt_node_ptr->dst.size = 0;
  tmp_sdt_node_ptr->dst.alignment_req = 1;

  /* All other fields of the SDT node were initialized the way they should be
     by OalNewSDTNode().  This SPARE node is used as a place-holder by the
     SDT processing routine OalProcessSDT, and will be removed before the
     Object Layer ever sees it.  */

#ifdef OA_DEBUG
  sprintf( error_string, "%s: warning - implicit SPARE detected; ", 
           proc_name);
  sprintf( error_string + strlen( error_string), 
           "inserted a SPARE COLUMN node before %s node, with src.size: %d", 
           input_node->class_name, (int) pad_bytes);
  sprintf( error_string + strlen( error_string), 
           " and src.start_offset: %ld.", 
           tmp_sdt_node_ptr->src.start_offset);
  oa_errno = 900;
  OaReportError( error_string);
#endif
}

if (pad_bytes < 0) {
  sprintf( error_string, "%s: error: overlapping source data. ", proc_name);
  sprintf( error_string + strlen( error_string),
           "src.start_offset = %ld, previous src.start_offset = %ld ",
            input_sdt_node_ptr->src.start_offset, 
            tmp_sdt_node_ptr->src.start_offset);
  sprintf( error_string + strlen( error_string),
           "previous src.size = %ld", tmp_sdt_node_ptr->src.size);
  oa_errno = 534;
  OaReportError( error_string);
  return(1);
}
return(0);
}



/*****************************************************************************

  Routine:  OalCompressSDT

  Description: Compresses an SDT to improve performance in OalProcessSDT.
               So far the only compression done applies to end nodes (data
               nodes):
               1) If src.size=0 and dst.size=0, no data is described by the
                  node, so it is deleted.
               2) An SDT node which has a conversion_type of OA_BINREP and
                  whose src.binrep_desc and dst.binrep_desc are the same
                  will have its conversion_type changed to OA_MEMCPY.
               3) An SDT node which has a conversion_type of OA_MEMCPY will
                  have its total_repetitions changed to 1, and its src.size 
                  and dst.size multiplied by the old total_repetitions.
                  This way a single memcpy call is performed, instead of as
                  many memcpy calls as there were old repetitions.
               4) OA_MEMCPY nodes whose dst data are adjacent (i.e. no gap
                  in dst data between them) are combined into one node.

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   6 Oct  1994
  Last Modified:   6 Oct  1994

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:
         sdt - The root node of an SDT as created by OalCreateSDT.
         
  Output:  The root of the compressed SDT.  The input SDT is unchanged.

  Notes:
  1) The caller must save the uncompressed SDT, and pass it to OalSDTtoODLTree;
     the compressed SDT should NOT be passed to OalSDTtoODLTree; it should be
     used only by OalProcessSDT.
  2) This function traverses the SDT in a pre-order traversal order.
     An example of a pre-order traversal order is:
                     1 (root)
                    / \
                   2   5
                  / \
                 3   4
*****************************************************************************/

#ifdef _NO_PROTO

ODLTREE OalCompressSDT( sdt)
ODLTREE sdt;

#else

ODLTREE OalCompressSDT( ODLTREE sdt)

#endif
{

ODLTREE current_node, save_node, compressed_SDT;
SDT_node *sdt_node_ptr;
/*
SDT_node *left_sibling_sdt_node_ptr;
ODLTREE left_sibling;
*/

/* The contents of current_node (*current_node) are saved in save_node, so that
   if the code in the loop deletes current_node from the tree, the positioning
   code at the end of the loop can look in save_node to find current_node's
   parent and right_sibling. */

save_node = OdlNewObjDesc( NULL, NULL, NULL, NULL, NULL, NULL, 
                           (short) 0, (long) 0);
compressed_SDT = OaCopyTree( sdt, OA_STRIP_KEYWORDS | OA_STRIP_COMMENTS);

/* Loop through all the nodes in the tree.  */

current_node = compressed_SDT;
while( current_node != NULL) {

  if (current_node->appl1 == NULL) {
    *save_node = *current_node;
    OdlFreeTree( OdlCutObjDesc( current_node));
    current_node = save_node;

  } else {

    sdt_node_ptr = (SDT_node *) current_node->appl1;

    if ((sdt_node_ptr->src.size == 0) && (sdt_node_ptr->dst.size == 0) &&
        (LeftmostChild( current_node) == NULL)) {

      /* Delete current_node from the tree, but first save a copy of its
         sibling and parent pointers so can position to the next node. */

      OalFreeSDTStruct( (SDT_node *) current_node->appl1);
      current_node->appl1 = NULL;
      *save_node = *current_node;
      OdlFreeTree( OdlCutObjDesc( current_node));
      current_node = save_node;
      
    } else {

      if ((sdt_node_ptr->conversion_type == OA_BINREP) &&
          (sdt_node_ptr->src.binrep_descrip == 
           sdt_node_ptr->dst.binrep_descrip)) {

        /* Redundant binrep node; convert to memcpy node.  */

        sdt_node_ptr->conversion_type = OA_MEMCPY;
        sdt_node_ptr->src.binrep_descrip = NULL;
        sdt_node_ptr->dst.binrep_descrip = NULL;
      }

      if (sdt_node_ptr->conversion_type == OA_MEMCPY) {

        /* Set total_repetitions to 1 by adjusting src.size and dst.size.  */

        if (sdt_node_ptr->total_repetitions > 1) {
          sdt_node_ptr->src.size *= sdt_node_ptr->total_repetitions;
          sdt_node_ptr->dst.size = sdt_node_ptr->src.size;
          sdt_node_ptr->total_repetitions = 1;
        }

        /* If the current node's left_sibling is also a OA_MEMCPY node, and 
           the left_sibling's end is the start of the current node (no gap
           in dst), then incorporate the left_sibling's data into the current
           node and delete the left_sibling.  total_repetitions of both nodes
           are 1, since already dealt with above.  */

#ifdef COMMENTED_OUT
        if ((left_sibling = LeftSDTSibling( current_node)) != NULL) {
          left_sibling_sdt_node_ptr = (SDT_node *) left_sibling->appl1;
          if ((left_sibling_sdt_node_ptr->conversion_type == OA_MEMCPY) &&
              (sdt_node_ptr->dst.start_offset ==
                (left_sibling_sdt_node_ptr->dst.start_offset +
                 left_sibling_sdt_node_ptr->dst.size))) {
            sdt_node_ptr->src.size += left_sibling_sdt_node_ptr->src.size;
            sdt_node_ptr->dst.size = sdt_node_ptr->src.size;
            sdt_node_ptr->src.start_offset = 
              left_sibling_sdt_node_ptr->src.start_offset;
            sdt_node_ptr->dst.start_offset = 
              left_sibling_sdt_node_ptr->dst.start_offset;
            OalFreeSDT( OdlCutObjDesc( left_sibling));
          }  /* end if the two nodes describe adjacent dst data     */
        }  /* end if current_node has a left sibling              */
#endif
      }  /* end if current_node's conversion_type is OA_MEMCPY  */
    }  /* end else not an ALIAS node                          */
  }  /* end else current_node has SDT node attached           */

  /* Position current_node to the next node;  if current_node has children,
     then the leftmost child is the new current_node;  else if current_node
     has a right sibling, then the right sibling is the new current_node;  
     else search upwards in the tree until get to a node which has a right
     sibling.  */

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
LemmeGo( save_node);
return( compressed_SDT);
}



/*****************************************************************************

  Routine:  OalConvert

  Description:  This routine performs the data conversion specified in the 
                input SDT node, on the data pointed to by the source and
                destination pointers in the SDT node.  The conversions are
                ASCII-to-binary, binary-to-ASCII, binary-to-binary and
                straight copy.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:  24 July  1996

  History:

    Creation - This routine was part of the Alpha Release of the OA library.
    12/16/94 - Binary-to-ASCII conversions now use the format specification in
               sdt_node_ptr->dst.format_spec, which is set by the function
               OalDetermineConvertParameters.  SM
    03/22/96 - Changed format string in sscanf call in ASCII_TO_BINARY
               conversion section for IBM-PC from "%f" to "%lf".  SM
    04/25/96 - Added support for binary-to-binary cross-conversions between
               integers and reals;  reals are truncated (NOT rounded) when
               converted to ints;  changed code for binary-to-ASCII and
               ASCII-to-binary conversions to handle non-native binary src and
               dst respectively.  SM
    07/24/96 - Added support for binary-to-ASCII cross-conversions i.e. binary
               real to ASCII integer, and binary integer to ASCII real.  Added
               similar support for ASCII-to-binary cross-conversions.
               ASCII-to-ASCII cross-conversions are still not supported;
               a straight copy is done.  SM

  Input:  sdt_node_ptr - Points to an SDT node, previously set up by
                         OalBuildSDTNode.  sdt_node_ptr->src.ptr must point to
                         the beginning of the data to be converted, and
                         sdt_node_ptr->dst.ptr must point to the location to
                         store the converted data; these are set by
                         OalProcessSDT and may overlap.
         
  Output:  If the function successfully converts the data, it returns 0, 
           otherwise a non-zero value.  For ASCII-to-binary conversions,
           it always returns 0.

  Notes:  
  

*****************************************************************************/

typedef short int2;
#if defined(IBM_PC) || defined(MAC)
typedef long int4;
#else
typedef int int4;
#endif

#ifdef _NO_PROTO

int OalConvert( sdt_node_ptr)
SDT_node *sdt_node_ptr;

#else

int OalConvert( SDT_node *sdt_node_ptr)

#endif
{

static char *proc_name = "OalConvert";
static char initialized = FALSE;
static struct binrep_desc *long_binrep_descrip = NULL;
static struct binrep_desc *ulong_binrep_descrip = NULL;
static struct binrep_desc *double_binrep_descrip = NULL;
struct binrep_desc *src_binrep_descrip = NULL;
struct binrep_desc *dst_binrep_descrip = NULL;
int i, dummy1, dummy2, len;
long l;
unsigned long ul;
double d;
PTR src, dst;


static unsigned char buf[512]; /* For ASCII-to-binary conversions, src is copied here 
                         so sscanf won't read past end of allocated memory if
                         no number is present.  For binary-to-ASCII
                         conversions, ASCII value is written here, then
                         dst.size characters are copied to the dst ptr, also
                         to prevent writing over the end of allocated memory.
                      */

/* If this is the first call to this function, get the binrep descriptions for
   native 'long', 'unsigned long' and 'double' types.  */

if (initialized == FALSE) {
  OalGetNativeCTypeInfo( "long", &dummy1, &dummy2, &long_binrep_descrip);
  OalGetNativeCTypeInfo( "unsigned long", &dummy1, &dummy2, 
                          &ulong_binrep_descrip);
  OalGetNativeCTypeInfo( "double", &dummy1, &dummy2, &double_binrep_descrip);
  if ((long_binrep_descrip == NULL) || (ulong_binrep_descrip == NULL) ||
      (double_binrep_descrip == NULL)) {
    sprintf( error_string, 
             "%s: couldn't find binrep descrip for native long or double.",
             proc_name);
    oa_errno = 730;
    OaReportError( error_string);
    return(1);
  } else {
    initialized = TRUE;
  }
}

src_binrep_descrip = sdt_node_ptr->src.binrep_descrip;
dst_binrep_descrip = sdt_node_ptr->dst.binrep_descrip;
src = sdt_node_ptr->src.ptr;
dst = sdt_node_ptr->dst.ptr;

switch( sdt_node_ptr->conversion_type) {

  case OA_BINREP:

    /* If the src and dst types are the same (both integer types or both
       floating-point types), call BinrepConvert to do the conversion.  */

    if (src_binrep_descrip->type == dst_binrep_descrip->type) {

      BinrepConvert( src_binrep_descrip, dst_binrep_descrip, src, dst);

    } else {

      /* Types aren't the same, so we're cross-converting a floating-point 
         type to an integer type or visa-versa, an operation not directly
         supported by BinrepConvert.
         1) Call BinrepConvert to convert the source to a native long,
            unsigned long or double.  (If it's already in that format, 
            BinrepConvert just copies.)
         2) If the source is an integer type, cast the long or unsigned long
            to a double.
         3) Call BinrepConvert again to convert the long, unsigned long or
            double to the dst type. (If the source is already in dst format, 
            BinrepConvert just copies.)  */

      if (src_binrep_descrip->type == BINREP_INTEGER) {
        if (src_binrep_descrip->complement == '0') {
          BinrepConvert( src_binrep_descrip, ulong_binrep_descrip, src,
                         (PTR) buf);
          d = (double) *((unsigned long *) buf);
        } else {
          BinrepConvert( src_binrep_descrip, long_binrep_descrip, src,
                         (PTR) buf);
          d = (double) *((long *) buf);
        }
        BinrepConvert( double_binrep_descrip, dst_binrep_descrip, (PTR) &d, 
                       dst);

      } else {             /* type == BINREP_FLOAT */
        BinrepConvert( src_binrep_descrip, double_binrep_descrip, src,
                       (PTR) buf);
        d = *((double *) buf);
        if (dst_binrep_descrip->complement == '0') {
          ul = (unsigned long) d;
          BinrepConvert( ulong_binrep_descrip, dst_binrep_descrip,
                         (PTR) &ul, dst);
        } else {
          l = (long) d;
          BinrepConvert( long_binrep_descrip, dst_binrep_descrip,
                         (PTR) &l, dst);
        }
      }
    }
  break;


  case OA_ASCII_TO_BINARY:

    /* On entry, sdt_node_ptr->src.PDS_data_type must be OA_ASCII_INTEGER or
       OA_ASCII_REAL;  OA_ASCII_COMPLEX is NOT allowed and should be pre-
       processed away by the caller.  The ASCII value is first read into a
       native long or double, then BinrepConvert converts the long or double
       to the dst format. (If the long or double is already in dst format,
       BinrepConvert just copies.)  */

    /* Copy src to buf and append a '\0' at the end before converting.  
       This unfortunate (for performance) buffering is necessary when the
       ASCII data in src isn't an integer or float like it's supposed to be:
       for example, all blanks.  In this case atol or atof will step forward
       through the blanks until it finds non-white space, which may be after
       the end of the src data (a malloc'ed stream buffer or object data), 
       and result in a segmentation violation (crash).  */

    for (i=0; i<sdt_node_ptr->src.size; i++) 
      buf[i] = src[i];
    buf[i] = '\0';

    switch (sdt_node_ptr->src.PDS_data_type) {
    
      case OA_ASCII_INTEGER:  /* Read the ASCII value into a long (PDS does */
        l = atol( (char *)buf);       /* not have UNSIGNED_ASCII_INTEGER, so OK).   cast added   DWS 09-26-02*/
        if (dst_binrep_descrip->type == BINREP_INTEGER) {
          BinrepConvert( long_binrep_descrip, dst_binrep_descrip,
                         (PTR) &l, dst);
        } else {                     /* BINREP_FLOAT cross-conversion */
          d = (double) l;
          BinrepConvert( double_binrep_descrip, dst_binrep_descrip,
                         (PTR) &d, dst);
        }
      break;  /* end case OA_ASCII_INTEGER */

      case OA_ASCII_REAL:     /* Read the ASCII value into a double.  */
#ifdef IBM_PC
        sscanf( buf, "%lf", &d);
#else
        d = atof( (char *)buf);   /*cast added   DWS 09-26-02*/
#endif
        if (dst_binrep_descrip->type == BINREP_FLOAT) {
          BinrepConvert( double_binrep_descrip, dst_binrep_descrip,
                         (PTR) &d, dst);
        } else {                     /* BINREP_INTEGER cross-conversion */
          l = (long) d;
          BinrepConvert( long_binrep_descrip, dst_binrep_descrip,
                         (PTR) &l, dst);
        }
      break;  /* end case OA_ASCII_REAL  */

      default:
        sprintf( error_string, 
                 "%s: can't do ASCII_TO_BINARY conversion on %s!",
                 proc_name, 
                 OaPDSDataTypetoStr( sdt_node_ptr->src.PDS_data_type));
        oa_errno = 535;
        OaReportError( error_string);
        return(1);
    }  /* end switch (sdt_node_ptr->src.PDS_data_type)  */


  break;  /* end case conversion_type OA_ASCII_TO_BINARY  */

  case OA_BINARY_TO_ASCII:

    switch (src_binrep_descrip->type) {

      case BINREP_INTEGER:

        if (sdt_node_ptr->dst.PDS_data_type == OA_ASCII_BIT_STRING) {

          /* Convert to MSB with destination buf, then use the format string
             to convert each byte to ASCII (either hex or octal), directly
             to the final destination .  */

          BinrepConvert( src_binrep_descrip, dst_binrep_descrip, 
                         src, (PTR) buf);
          *dst = ' ';
          if (strchr( sdt_node_ptr->dst.format_spec, 'o') != NULL)
            len = 3;  /* octal */
          else
            len = 2;  /* hex */
          for (i=0; i<(dst_binrep_descrip->bytes); i++)
            sprintf( dst+1+i*len, sdt_node_ptr->dst.format_spec, buf[i]);
       
        } else {  /* Converting to a numeric type, not to a ASCII_BIT_STRING */

        /* The source has to be a native binary type for the C library function
           sprintf to convert it to ASCII, so call BinrepConvert to convert the
           source to a native long, unsigned long or double.  (If the source
           format is already the same as the native (destination) format, then 
           BinrepConvert just copies.)  The destination of sprintf is buf, 
           which is big enough for any number sprintf will ever write to it.  
           The sprintf call uses the format specification in
           sdt_node_ptr->dst.format_spec, which was set by the function
           OalDetermineConvertParameters.  Since the PDS format specification
           "Ix" implicitly specifies signed, don't have to worry about unsigned
           integers as a separate case.  */

          BinrepConvert( src_binrep_descrip, long_binrep_descrip, 
                         src, (PTR) &l);
          if (sdt_node_ptr->dst.PDS_data_type == OA_ASCII_INTEGER) {
            sprintf( (char *)buf, sdt_node_ptr->dst.format_spec, l);/*   cast added   DWS 09-26-02*/
          } else {   /* cross conversion */
            d = (double) l;
            sprintf( (char *)buf, sdt_node_ptr->dst.format_spec, d);/*   cast added   DWS 09-26-02*/
          }

          /* Copy dst.size characters from buf to dst, preceeded by a space; 
             this prevents ever running over the end of allocated memory, 
             which might occur without buffering.  */

          *dst = ' ';
          for (i=0; i<sdt_node_ptr->dst.size-1; i++)
            dst[1+i] = buf[i];
        }  /* end else dst.PDS_data_type not OA_ASCII_BIT_STRING */

      break;

      case BINREP_FLOAT:

        if (sdt_node_ptr->dst.PDS_data_type == OA_ASCII_BIT_STRING) {

          /* Use the format string to convert each byte to ASCII (either hex
             or octal), directly to the final destination .  */

          *dst = ' ';
          if (strchr( sdt_node_ptr->dst.format_spec, 'o') != NULL)
            len = 3;  /* octal */
          else
            len = 2;  /* hex */
          for (i=0; i<(src_binrep_descrip->bytes); i++)
            sprintf( dst+1+i*len, sdt_node_ptr->dst.format_spec, src[i]);
       
        } else {  /* Convert to numeric real type, not to ASCII_BIT_STRING */

          BinrepConvert( src_binrep_descrip, double_binrep_descrip, src,  
                         (PTR) &d);
          if (sdt_node_ptr->dst.PDS_data_type == OA_ASCII_REAL) {
            sprintf((char *) buf, sdt_node_ptr->dst.format_spec, d);/*   cast added   DWS 09-26-02*/
          } else {   /* cross conversion */
            l = (long) d;
            sprintf( (char *)buf, sdt_node_ptr->dst.format_spec, l);/*   cast added   DWS 09-26-02*/
          }

          /* Copy dst.size characters from buf to dst, preceeded by a space; 
             this prevents ever running over the end of allocated memory, 
             which might occur without buffering.  */

          *dst = ' ';
          for (i=0; i<sdt_node_ptr->dst.size-1; i++)
            dst[1+i] = buf[i];
        }
      break;
    }

  break;  /* end case conversion_type OA_BINARY_TO_ASCII  */

  default: 
    sprintf( error_string, "%s: invalid conversion type: %d.",
             proc_name, sdt_node_ptr->conversion_type);
    oa_errno = 535;
    OaReportError( error_string);
    return(1);
}  /* end of switch on conversion_type  */
return(0);
}



/*****************************************************************************

  Routine:  OalCreateSDT

  Description:  OalCreateSDT creates a SDT (Stream Decomposition Tree) from an
                ODL tree.  This function is called by Object Layer functions to
                set up an SDT filter for reading a PDS data object from a file
                into memory, or for doing an in-memory conversion.  The SDT
                directs the conversion and translation from source data to 
                destination data, which is performed by OalProcessSDT.
                On return, every node of the ODL tree which describes data or
                describes a number of repetitions of its children, has an SDT
                node attached to it, with all the constant SDT parameters set.
                Other nodes (ALIAS, GROUP, BIT_COLUMN, BIT_ELEMENT etc.) are
                preserved in the tree, but without SDT nodes attached.  
                This work is done by OalBuildSDTNode.
                The tree structure itself may be modified (nodes added or
                deleted) to accomodate prefixes, suffixes, ITEM_OFFSETS, 
                BIT_ELEMENTS etc. as described in detail in the notes below,
                and in OalBuildSDTNode.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:  11 Dec   1996

  History:

    Creation - This routine was part of the Alpha Release of the OA library.
    12/13/94 - Modified column-major COLUMN keyword values so that
               OalBuildSDTNode doesn't need special cases for column-major. SM
    01/09/96 - Fixed bug for START_BYTE value in image_line_node; wasn't 
               adding prefix bytes. SM
    12/11/96 - Fixed bug with prefix/suffix bytes in TABLE-like objects.  SM

  Input:  
         TLO_node               - A pointer to a top-level object node in an
                                  ODL tree, i.e. a TABLE, IMAGE, HISTOGRAM etc.

         src_interchange_format - Set to either OA_ASCII_INTERCHANGE_FORMAT or 
                                  OA_BINARY_INTERCHANGE_FORMAT.

  Output:  If successful, the function returns a pointer to the root of the
           ODL tree, and most nodes have an SDT node attached, with all the
           SDT parameters initialized.  The tree structure may be modified, 
           as described in the notes below. 
           If unsuccessful, it returns NULL.

  Notes:  

  1) OalCreateSDT makes structural modifications to the tree, if necessary,
     then traverses the tree, calling OalBuildSDTNode at every node.
     OalBuildSDTNode does the actual work of adding the SDT nodes; detailed
     documentation of the algorithm can be found there.

  2) For TABLE and IMAGE objects with prefixes and/or suffixes, ODL tree
     structure modifications are done here, instead of deeper in
     OalBuildSDTNode.  In general, such cludges should be isolated here, (and
     in OalSDTtoODLTree for the reverse process), instead of deeper in
     OalBuildSDTNode (or in OalAdjustKwdstoMatchSDT for the reverse process).

  3) Prefix and suffix bytes are always thrown away; 
     OalCreateSDT adds a SPARE nodes to represent the prefix and suffix bytes
     in the source data, setting SDT_node->dst.size=0.  These modifications
     are shown below:
     For an IMAGE:                          For a TABLE:
     IMAGE----COLUMN (prefix bytes)         TABLE------COLUMN (prefix bytes)
           ---COLUMN (image line)                   ---COLUMN (table columns
           ---COLUMN (suffix bytes)                     ....   go here)
                                                    ---COLUMN (suffix bytes)
     For an IMAGE with prefix and/or suffix bytes, an extra image_line_node is
     added, which describes a single image line;  this is necessary so that
     OalProcessSDT can loop through the prefix bytes, suffix bytes and the
     image line data together.

  4) If a future Object Layer method wants to separate the prefix and/or 
     suffix bytes into separate objects, it can simply create a SDT for each
     one, and feed each buffer full of input stream data to each SDT: e.g. a
     prefix table SDT, a suffix table SDT, and a main table or image SDT.

  5) A COLUMN_MAJOR table description is converted to a ROW_MAJOR description
     before passing the tree on to OalBuildSDTNode.  The keywords in all the
     columns for BYTES, ITEMS etc. are multiplied by ROWS, and START_OFFSETS 
     adjusted accordingly.  In the table node, ROWS is changed to 1, and
     ROW_BYTES to old ROWS * old ROW_BYTES.  OalSDTtoODLTree undoes this
     setup.

  6) Prefixes and suffixes are not allowed in COLUMN-major tables.

*****************************************************************************/

#ifdef _NO_PROTO

ODLTREE OalCreateSDT( TLO_node, src_interchange_format)
ODLTREE TLO_node;
int src_interchange_format;

#else

ODLTREE OalCreateSDT( ODLTREE TLO_node, int src_interchange_format)

#endif
{

static char *proc_name = "OalCreateSDT";
ODLTREE image_line_node, tmp_node, odltreenode;
KEYWORD *kwdptr;
long rows, row_bytes, bytes, start_byte;
long item_bytes = 0, items = 0, prefix_bytes = 0, suffix_bytes = 0;
SDT_node *tmp_sdt_node_ptr, *TLO_sdt_node_ptr;
int object_class, result, dst_interchange_format;

/* Determine the destination interchange format from the global variable
   Oa_profile.  */

if (src_interchange_format == OA_ASCII_INTERCHANGE_FORMAT)
  dst_interchange_format = Oa_profile.dst_format_for_ASCII_src;
else
  dst_interchange_format = Oa_profile.dst_format_for_binary_src;

if ((Oa_profile.dst_alignment_type != OA_NO_ALIGN) &&
    (Oa_profile.dst_alignment_type != OA_ALIGN_EVEN) &&
    (Oa_profile.dst_alignment_type != OA_ALIGN_RISC)) {
  sprintf( error_string, "%s: invalid Oa_profile.dst_alignment_type: %d.",
           proc_name, Oa_profile.dst_alignment_type);
  oa_errno = 535;
  OaReportError( error_string);
  return( NULL);
}

/* Input node shouldn't have a parent;  if it does, issue a warning message
   and cut it out.  */

if (Parent( TLO_node) != NULL) {
  OdlCutObjDesc( TLO_node);
  sprintf( error_string, "%s: Warning: input node shouldn't have a parent.\n",
           proc_name);
  strcat( error_string, " Input node has now been cut off from parent node.");
  oa_errno = 530;
  OaReportError( error_string);
}

object_class = OaGetObjectClass( TLO_node);

/* Find values for prefix and suffix bytes, if any. */

OaKwdValuetoLong( "*PREFIX_BYTES", TLO_node, &prefix_bytes);
OaKwdValuetoLong( "*SUFFIX_BYTES", TLO_node, &suffix_bytes);

if ((prefix_bytes > 0) || (suffix_bytes > 0)) {

  switch( object_class) {

    case OA_IMAGE:

      /* Create a new COLUMN node which represents a single image line, and
         set its keywords appropriate for a COLUMN; later OalBuildSDTNode will
         be used to create the SDT node for this COLUMN node.  */

      image_line_node = OdlNewObjDesc( "COLUMN", NULL, NULL, NULL, NULL, NULL, 
                                        (short) 0, (long) 0);
      OdlPasteObjDesc( image_line_node, TLO_node);
      OaStrtoKwdValue( "NAME", image_line_node, "IMAGE_LINE");
      OaLongtoKwdValue( "START_BYTE", image_line_node,
                        (long) (1 + prefix_bytes));
      kwdptr = OdlFindKwd( TLO_node, "SAMPLE_TYPE", "*", (unsigned long) 0,
                           (unsigned short) ODL_THIS_OBJECT);
      OaStrtoKwdValue( "DATA_TYPE", image_line_node, kwdptr->value);

      OaKwdValuetoLong( "SAMPLE_BITS", TLO_node, &item_bytes);
      item_bytes = item_bytes/8;
      OaKwdValuetoLong( "LINE_SAMPLES", TLO_node, &items);

      OaLongtoKwdValue( "BYTES", image_line_node, items * item_bytes);
      OaLongtoKwdValue( "ITEMS", image_line_node, items);
      OaLongtoKwdValue( "ITEM_BYTES", image_line_node, item_bytes);

      /* Initialize and attach an SDT node to the TLO node; OalBuildSDTNode 
         will ignore this node since it already has an SDT node attached.  
         Later, after OalBuildSDTNode has determined the dst.size of the
         image line node, we'll set dst.size of the TLO_node.  */
     
      TLO_sdt_node_ptr = OalNewSDTNode();
      TLO_node->appl1 = (char *) TLO_sdt_node_ptr;
      TLO_sdt_node_ptr->src.PDS_data_type = (char) -1;  
      TLO_sdt_node_ptr->dst.PDS_data_type = (char) -1;  
      OaKwdValuetoLong( "LINES", TLO_node, 
                        &(TLO_sdt_node_ptr->total_repetitions));

      /* Create the SPARE COLUMN node for the prefix bytes, if any, then add
         it as the left sibling of the image line node.  */

      if (prefix_bytes > 0) {
        tmp_node = OdlNewObjDesc( "COLUMN", NULL, NULL, NULL, NULL, NULL, 
                                   (short) 0, (long) 0);
        OdlPasteObjDescBefore( tmp_node, image_line_node);
        OaStrtoKwdValue( "NAME", tmp_node, "SPARE");

        tmp_sdt_node_ptr = OalNewSDTNode();
        tmp_node->appl1 = (char *) tmp_sdt_node_ptr;
        tmp_sdt_node_ptr->total_repetitions = 1;
        tmp_sdt_node_ptr->conversion_type = OA_NOT_APPLICABLE;
        tmp_sdt_node_ptr->src.size = prefix_bytes;
        tmp_sdt_node_ptr->src.start_offset = 0;
        tmp_sdt_node_ptr->dst.size = 0;
        tmp_sdt_node_ptr->src.PDS_data_type = (char) -1;  
        tmp_sdt_node_ptr->dst.PDS_data_type = (char) -1;  
      }

      /* Create the SPARE COLUMN node for the suffix bytes, if any, then add
         it as the right sibling of the image line node.  */

      if (suffix_bytes > 0) {
        tmp_node = OdlNewObjDesc( "COLUMN", NULL, NULL, NULL, NULL, NULL,
                                   (short) 0, (long) 0);
        OdlPasteObjDescAfter( tmp_node, image_line_node);
        OaStrtoKwdValue( "NAME", tmp_node, "SPARE");

        tmp_sdt_node_ptr = OalNewSDTNode();
        tmp_node->appl1 = (char *) tmp_sdt_node_ptr;
        tmp_sdt_node_ptr->total_repetitions = 1;
        tmp_sdt_node_ptr->conversion_type = OA_NOT_APPLICABLE;
        tmp_sdt_node_ptr->src.size = suffix_bytes;
        tmp_sdt_node_ptr->src.start_offset = prefix_bytes + items * item_bytes;
        tmp_sdt_node_ptr->dst.size = 0;
        tmp_sdt_node_ptr->src.PDS_data_type = (char) -1;  
        tmp_sdt_node_ptr->dst.PDS_data_type = (char) -1;  
      }
      TLO_sdt_node_ptr->src.start_offset = 0;
      TLO_sdt_node_ptr->src.size = prefix_bytes + items*item_bytes + 
                                   suffix_bytes;
    break;   /* end case IMAGE object                     */

    case OA_GAZETTEER:
    case OA_PALETTE:
    case OA_SERIES:
    case OA_SPECTRUM:
    case OA_TABLE:

      if (OaKwdValuetoLong( "ROW_BYTES", TLO_node, &row_bytes) != 0) {
        sprintf( error_string, 
                 "%s: ROW_BYTES keyword value not found for %s object.",
                 proc_name, OdlGetObjDescClassName( TLO_node));
        oa_errno = 531;
        OaReportError( error_string);
        return(NULL);
      }        

      /* For a TABLE-like object, add the SPARE nodes on the far left and 
         right of the TLO_node's children, and leave it to OalBuildSDTNode to
         create the TLO_node's SDT node.  */

      if (prefix_bytes > 0) {

        /* Create the SPARE COLUMN node for the prefix bytes.  */

        tmp_node = OdlNewObjDesc( "COLUMN", NULL, NULL, NULL, NULL, NULL, 
                                   (short) 0, (long) 0);
        OaStrtoKwdValue( "NAME", tmp_node, "SPARE");
        if ((odltreenode = LeftmostChild( TLO_node)) != NULL) {
          OdlPasteObjDescBefore( tmp_node, odltreenode);
        } else {
          sprintf( error_string, "%s: TABLE-like object without any COLUMNs!",
                   proc_name);
          oa_errno = 530;
          OaReportError( error_string);
          return( NULL);
        }

        tmp_sdt_node_ptr = OalNewSDTNode();
        tmp_node->appl1 = (char *) tmp_sdt_node_ptr;
        tmp_sdt_node_ptr->total_repetitions = 1;
        tmp_sdt_node_ptr->conversion_type = OA_NOT_APPLICABLE;
        tmp_sdt_node_ptr->src.size = prefix_bytes;
        tmp_sdt_node_ptr->src.start_offset = 0;
        tmp_sdt_node_ptr->dst.size = 0;
        tmp_sdt_node_ptr->src.PDS_data_type = (char) -1;  
        tmp_sdt_node_ptr->dst.PDS_data_type = (char) -1;  

        /* Add prefix_bytes to the START_BYTE's of TLO_nodes's children. */

        odltreenode = RightSibling( tmp_node);
        while (odltreenode != NULL) {
          switch( OaGetObjectClass( odltreenode)) {
            case OA_COLUMN:
            case OA_CONTAINER:
              OaKwdValuetoLong( "START_BYTE", odltreenode, &bytes);
              bytes += prefix_bytes;
              OaLongtoKwdValue( "START_BYTE", odltreenode, bytes);
            break;
          }
          odltreenode = RightSibling( odltreenode);
        }
      }

      if (suffix_bytes > 0) {

        /* Create the SPARE COLUMN node for the suffix bytes.  */

        tmp_node = OdlNewObjDesc( "COLUMN", NULL, NULL, NULL, NULL, NULL, 
                                   (short) 0, (long) 0);
        OaStrtoKwdValue( "NAME", tmp_node, "SPARE");
        if ((odltreenode = RightmostChild( TLO_node)) != NULL) {
          OdlPasteObjDescAfter( tmp_node, odltreenode);
        } else {
          sprintf( error_string, "%s: TABLE-like object without any COLUMNs!",
                   proc_name);
          oa_errno = 530;
          OaReportError( error_string);
          return( NULL);
        }

        tmp_sdt_node_ptr = OalNewSDTNode();
        tmp_node->appl1 = (char *) tmp_sdt_node_ptr;
        tmp_sdt_node_ptr->total_repetitions = 1;
        tmp_sdt_node_ptr->conversion_type = OA_NOT_APPLICABLE;
        tmp_sdt_node_ptr->src.size = suffix_bytes;
        tmp_sdt_node_ptr->src.start_offset = prefix_bytes + row_bytes;
        tmp_sdt_node_ptr->dst.size = 0;
        tmp_sdt_node_ptr->src.PDS_data_type = (char) -1;  
        tmp_sdt_node_ptr->dst.PDS_data_type = (char) -1;  
      }
    break;  /* End prefix/suffix section for TABLE-like object. */

    default:
      sprintf( error_string, "%s: prefix and/or suffix bytes are not allowed",
               proc_name);
      sprintf( error_string + strlen( error_string), " for %s object.",
               OdlGetObjDescClassName( TLO_node));
      oa_errno = 530;
      OaReportError( error_string);
      return( NULL);
    break;

  }       /* End switch on object_class.  */
}       /* end if there are prefix or suffix bytes */


/* If it's a COLUMN_MAJOR TABLE, then adjust the ITEMS, BYTES and START_OFFSET
   keywords in each column, so that each column description includes all the
   rows;  then change the table node's ROWS to 1, and ROW_BYTES to the old
   ROW_BYTES * ROWS; add a new keyword OLD_ROWS to save the old rows for
   OalSDTtoODLTree, since this information would otherwise be lost.  */

if ((object_class == OA_TABLE) && 
    (OdlFindKwd( TLO_node, "TABLE_STORAGE_TYPE", "COLUMN_MAJOR",
                (unsigned long) 0, (unsigned short) ODL_THIS_OBJECT) 
    != NULL)) {
  if (OaKwdValuetoLong( "ROWS", TLO_node, &rows) != 0) {
    sprintf( error_string, "%s: couldn't find ROWS keyword", proc_name);
    oa_errno = 531;
    OaReportError( error_string);
    return( NULL);
  }
  OaLongtoKwdValue( "OLD_ROWS", TLO_node, rows);
  tmp_node = LeftmostChild( TLO_node);
  while (tmp_node != NULL) {

    if (OaGetObjectClass( tmp_node) != OA_COLUMN) {
      sprintf( error_string, 
               "%s: object class: %s not allowed in a column-major TABLE",
               proc_name, OdlGetObjDescClassName( tmp_node));
      oa_errno = 530;
      OaReportError( error_string);
      return(NULL);
    }
    if (OaKwdValuetoLong( "START_BYTE", tmp_node, &start_byte) != 0) {
      sprintf( error_string, 
               "%s: START_BYTE keyword value not found for COLUMN object.",
               proc_name);
      oa_errno = 531;
      OaReportError( error_string);
      return(NULL);
    }
    if (LeftSibling( tmp_node) != NULL) {
      start_byte = (start_byte - 1) * rows + 1;
      OaLongtoKwdValue( "START_BYTE", tmp_node, start_byte);
    }

    if (OaKwdValuetoLong( "BYTES", tmp_node, &bytes) != 0) {
      sprintf( error_string,
               "%s: BYTES keyword value not found for COLUMN object.",
               proc_name);
      oa_errno = 531;
      OaReportError( error_string);
      return(NULL);
    }
    if (OaKwdValuetoLong( "ITEMS", tmp_node, &items) != 0) {
      items = 1;
      item_bytes = bytes;
      OaLongtoKwdValue( "ITEM_BYTES", tmp_node, item_bytes);
    }
    items *= rows;
    OaLongtoKwdValue( "ITEMS", tmp_node, items);
    bytes *= rows;
    OaLongtoKwdValue( "BYTES", tmp_node, bytes);

    tmp_node = RightSibling( tmp_node);
  }  /* end while loop through all columns */

  row_bytes *= rows;
  OaLongtoKwdValue( "ROW_BYTES", TLO_node, row_bytes);
  rows = 1;
  OaLongtoKwdValue( "ROWS", TLO_node, rows);

}  /* end if column-major TABLE */

/* ODL tree structure modifications are now completed, so go through the
   entire tree and build SDT nodes.  */

result = OalPostOrderTraverse( TLO_node, OA_BUILD_SDT_NODE,
                               src_interchange_format, dst_interchange_format);
if (result != 0)  /* OalBuildSDTNode has already issued an error message. */
  return( NULL);

TLO_sdt_node_ptr = (SDT_node *) TLO_node->appl1;

if ((object_class == OA_IMAGE) && (LeftmostSDTChild( TLO_node) != NULL)) {

  /* Set dst.size of the TLO SDT node to the dst.size of the image_line node.
     This is here in case the profile specified a different size data type for
     the dst.  */

  tmp_sdt_node_ptr = (SDT_node *) image_line_node->appl1;
  TLO_sdt_node_ptr->dst.size = tmp_sdt_node_ptr->dst.size *
                               tmp_sdt_node_ptr->total_repetitions;
}

return( TLO_node);
}



/*****************************************************************************

  Routine:  OalDetermineConvertParameters

  Description:  This routine determines the SDT destination parameters for an
                end-node: data type, size, alignment, conversion type and if
                applicable, src.binrep_descrip, dst.binrep_descrip and 
                dst.format_spec.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:  16 Apr   1997

  History:

    Creation - This routine was part of the Alpha Release of the OA library.
    12/02/94 - Moved code to determine src.binrep_descrip from OaBuildSDTNode
               to here. SM
    02/14/95 - Changed input parameters src_interchange_format and 
               dst_interchange_format.  SM
    12/19/95 - Don't align ASCII dst data.  SM
    04/25/96 - Changed format string for compatibility with changes in
               OalConvert, cleaned up code throughout.  SM
    07/24/96 - Added support for binary-to-ASCII cross-conversions i.e. binary
               real to ASCII integer, and binary integer to ASCII real.  
               ASCII-to-binary cross-conversions didn't need any additions
               here.  SM
    04/16/97 - Added support for BIT_STRINGS.  SM
    06/24/97 - Added support for BCD.  SM
    11/04/97 - Added support for "odd-sized" INTEGER types, by changing their
               src.PDS_data_type to MSB or LSB BIT_STRING.
    12/16/97 - Fixed bug in dst.format_spec by removing 'el' in front of "lf". SM  

  Input:  
         current_node           - An end-node (data node) with no children.
                                  An SDT node is attached via appl1, with
                                  src.PDS_data_type and src.size set.

         src_interchange_format - Set to either
                                  OA_ASCII_INTERCHANGE_FORMAT or 
                                  OA_BINARY_INTERCHANGE_FORMAT.

         dst_interchange_format - Set to either
                                  OA_ASCII_INTERCHANGE_FORMAT or 
                                  OA_BINARY_INTERCHANGE_FORMAT.
         
  Output:  The following fields of the SDT node are set:
           dst.PDS_data_type
           dst.size
           src.binrep_descrip
           dst.binrep_descrip
           dst.alignment_req
           conversion_type

  Notes:  
  
  1) The DATA_TYPE keyword value in the ODL tree is often set incorrectly
     for ASCII files, e.g. LSB_INTEGER instead of ASCII_INTEGER.
     OalBuildSDTNode determines the correct src.PDS_data_type based on 
     DATA_TYPE and src_interchange_format.

  2) For "odd-sized" integers, i.e. binary INTEGER types whose size isn't 1, 2 or
     4 bytes, src.PDS_data_type is changed to a BIT_STRING type, and treated as
     such from here on.
     
  3) If doing a binary-to-ASCII conversion and src.PDS_data_type is OA_UNKNOWN
     DATA_TYPE, then dst.size will be set to 0 (had a bug when memcpy was used:
     binary 0's from SPARE COLUMNs with DATA_TYPE = "N/A" could be written to 
     ASCII stream files, which couldn't later be read correctly with fgets).

  4) For binary-to-ASCII conversion of BIT_STRINGS, binrep descriptions of the
     source BIT_STRING and it's MSB counterpart are created on-the-fly by
     OalFindBinrepDescription, and added to src.binrep_descrip and
     dst.binrep_descrip respectively.  OalConvert uses these to convert to
     MSB order, then uses the format_spec to print out each byte in hex or
     octal (default is hex if FORMAT keyword doesn't say octal).

*****************************************************************************/

#define ALIGN_1      1      /* One of these values is assigned to the SDT   */
#define ALIGN_2      2      /* node's dst.alignment_req.                    */
#define ALIGN_4      4      /* This is the only function where these        */
#define ALIGN_8      8      /* #define symbols are used.                    */

#ifdef _NO_PROTO

int OalDetermineConvertParameters( current_node, src_interchange_format,
                                   dst_interchange_format)
ODLTREE current_node;
int src_interchange_format, dst_interchange_format;
#else

int OalDetermineConvertParameters( ODLTREE current_node, 
                                   int src_interchange_format,  
                                   int dst_interchange_format)

#endif
{

static char *proc_name = "OalDetermineConvertParameters";
long in_memory_size;
int is_numeric, field_width=0, precision=0, format_error=FALSE;
int cross_conversion=FALSE, len;
char *format_string, *str, c, binrep_q_code[32], buf[64];
SDT_node *sdt_node_ptr;
struct binrep_desc *binrep_descrip;
struct oa_type_conversion_info *conversion_info;

sdt_node_ptr = (SDT_node *) current_node->appl1;

/* Check to see if the source data type is an "odd-sized" integer, and if so,
   change it to a BIT_STRING type.  */

if ((sdt_node_ptr->src.PDS_data_type == OA_MSB_INTEGER) &&
    (sdt_node_ptr->src.size != 1) &&
    (sdt_node_ptr->src.size != 2) &&
    (sdt_node_ptr->src.size != 4)) {
  sdt_node_ptr->src.PDS_data_type = OA_MSB_BIT_STRING;
}
if ((sdt_node_ptr->src.PDS_data_type == OA_LSB_INTEGER) &&
    (sdt_node_ptr->src.size != 1) &&
    (sdt_node_ptr->src.size != 2) &&
    (sdt_node_ptr->src.size != 4)) {
  sdt_node_ptr->src.PDS_data_type = OA_LSB_BIT_STRING;
}

/* Get the conversion record for the source data type from the data types
   profile.  The data types profile specifies if and how every numeric PDS data
   type is to be converted;  this depends on the platform being run on, and on
   the current settings of Oa_profile.dst_format_for_ASCII_src and
   Oa_profile.dst_format_for_binary_src.  */

conversion_info = OalGetTypeConversionFromProfile( 
                    (int) sdt_node_ptr->src.PDS_data_type,
                    sdt_node_ptr->src.size);

/* Can't currently handle ASCII_TO_BINARY conversions on ASCII_BIT_STRING,
   so filter it out.  */

if ((sdt_node_ptr->src.PDS_data_type == OA_ASCII_BIT_STRING) &&
    (dst_interchange_format == OA_BINARY_INTERCHANGE_FORMAT)) {
  conversion_info = NULL;
  oa_errno = 520;
  sprintf( error_string, "Can't convert ASCII_BIT_STRING to binary!");
  OaReportError( error_string);
}

/*  If src isn't numeric (e.g. CHARACTER, DATE, TIME, BOOLEAN), then there is
    no entry in Oa_type_conversion_info, and OalGetTypeConversionFromProfile 
    returns NULL;  src.binrep_descrip is left NULL.  */

if (conversion_info != NULL) {  /* Data type is numeric. */

  /* If conversion_info->binrep_q_code (the source's q-code) is non-NULL, then
     the source must be binary;  otherwise the source is ASCII
     (e.g. ASCII_INTEGER, ASCII_BIT_STRING, ASCII_REAL etc.)  */

  if (conversion_info->binrep_q_code != NULL) {  /* Source is binary. */

    /* Determine src.binrep_descrip.  First copy the binrep q-code to a buffer
       and check that its length is specified; if not, it's a q-code of
       unspecified length used for BIT_STRINGS and wacko-length INTEGERS.
       Specify its length by appending src.size to the end of the q-code, so 
       that OalFindBinrepDescrip will create an appropriate binrep descriptor
       on-the-fly.  */

    strcpy( binrep_q_code, conversion_info->binrep_q_code);
    len = (int) strlen( binrep_q_code);
    if (!isdigit( binrep_q_code[ len-1]))
      sprintf( binrep_q_code + len, "%-d", (int) sdt_node_ptr->src.size);
    sdt_node_ptr->src.binrep_descrip = OalFindBinrepDescrip( binrep_q_code);
    if (sdt_node_ptr->src.binrep_descrip == NULL) {
      sprintf( error_string,
               "%s: couldn't find q_code: %s in binrep_descriptions",
               proc_name, binrep_q_code);
      oa_errno = 730;
      OaReportError( error_string);
      return(1);
    }
  }
  if (conversion_info->native_binrep_q_code != NULL)
    is_numeric = TRUE;
  else
    is_numeric = FALSE; /* Error, doesn't have q_code, so no conversion. */
} else {
  is_numeric = FALSE;
}

/* If source is ASCII and destination is ASCII, set up for a straight copy
   and return.  If the profile specifies a different data type for the dst,
   flag an error and continue, since we don't yet support any ASCII-to-ASCII
   conversions (such as ASCII_REAL to ASCII_INTEGER). */
    
if ((src_interchange_format == OA_ASCII_INTERCHANGE_FORMAT) &&
     (dst_interchange_format == OA_ASCII_INTERCHANGE_FORMAT)) {

  if (conversion_info != NULL) {
    if (sdt_node_ptr->src.PDS_data_type != 
        conversion_info->ASCII_PDS_data_type) {
      sprintf( error_string, "%s: Can't convert %s to %s!", proc_name,
               OaPDSDataTypetoStr( sdt_node_ptr->src.PDS_data_type),
               OaPDSDataTypetoStr( conversion_info->ASCII_PDS_data_type));
      sprintf( error_string + strlen( error_string), " Leaving it as %s.",
               OaPDSDataTypetoStr( sdt_node_ptr->src.PDS_data_type));
      oa_errno = 520;
      OaReportError( error_string);
    }
  }
  sdt_node_ptr->conversion_type = OA_MEMCPY;
  sdt_node_ptr->dst.PDS_data_type = sdt_node_ptr->src.PDS_data_type;
  sdt_node_ptr->dst.size = sdt_node_ptr->src.size;
  sdt_node_ptr->dst.alignment_req = ALIGN_1;
  return(0);
}


/* If source is binary and destination is ASCII, set up for a binary-to-ASCII
   conversion and return.  */

if ((src_interchange_format == OA_BINARY_INTERCHANGE_FORMAT) &&
    (dst_interchange_format == OA_ASCII_INTERCHANGE_FORMAT)) {

  sdt_node_ptr->dst.alignment_req = ALIGN_1;

  if (sdt_node_ptr->src.PDS_data_type == OA_UNKNOWN_DATA_TYPE) {
    sdt_node_ptr->conversion_type = OA_NOT_APPLICABLE;
    sdt_node_ptr->dst.PDS_data_type = OA_UNKNOWN_DATA_TYPE;
    sdt_node_ptr->dst.size = 0;
    return(0);
  }

  if (is_numeric == TRUE) {
    sdt_node_ptr->conversion_type = OA_BINARY_TO_ASCII;
    sdt_node_ptr->dst.PDS_data_type = conversion_info->ASCII_PDS_data_type;

    /* If this is a conversion from a binary INTEGER or BIT_STRING type to
       ASCII_BIT_STRING, then set dst.binrep_descrip to be the MSB version of
       the source BIT_STRING, so that it will be converted to MSB order before
       being printed out in hex.  If the source is any other type (a binary
       real type) then do nothing.  */

    if ((sdt_node_ptr->src.binrep_descrip->type == BINREP_INTEGER) &&
        (sdt_node_ptr->dst.PDS_data_type == OA_ASCII_BIT_STRING)) {
      binrep_q_code[0] = 'M';  /* E.g. "LSB_UINT24" -> "MSB_UINT24" */
      sdt_node_ptr->dst.binrep_descrip = OalFindBinrepDescrip( binrep_q_code);
    }

    /* If this is a cross-conversion, i.e. integer to ASCII real, or real to
       ASCII integer, delete the FORMAT keyword, if any, since it won't be
       applicable after the conversion.  */

    if (((sdt_node_ptr->src.binrep_descrip->type == BINREP_FLOAT) &&
         (sdt_node_ptr->dst.PDS_data_type == OA_ASCII_INTEGER)) ||
        ((sdt_node_ptr->src.binrep_descrip->type == BINREP_INTEGER) &&
         (sdt_node_ptr->dst.PDS_data_type == OA_ASCII_REAL))) {
      cross_conversion = TRUE;
      OaDeleteKwd( "FORMAT", current_node);
    }

    /* Determine dst.size and the C format specification string to be used by
       OalConvert.  If the FORMAT keyword exists then doctor it's field width
       into C form and use it;  otherwise use the default field width from the
       profile.  OalConvert passes a long or double to sprintf when converting
       to ASCII, so the C format string specifies  "%ld", "%f", or %g".  
       For BIT_STRINGS however, the FORMAT keyword must specify hex or octal;
       anything else is ignored and the default hex is used.  The resulting
       format_spec string specifies hex or octal for one byte, since the
       conversion is done byte-wise in a loop by OalConvert. */

    if (OaKwdValuetoStr( "FORMAT", current_node, &format_string) == 0) {
      CopyString( str, format_string);
      UpperCase( str);
      StripLeading( str, '"');
      StripLeading( str, '\'');
      StripLeading( str, ' ');
      StripTrailing( str, '"');
      StripTrailing( str, '\'');
      StripTrailing( str, ' ');

      if (sdt_node_ptr->dst.PDS_data_type == OA_ASCII_BIT_STRING) {
        if (strchr( str, 'O') != NULL) {  /* octal */
          field_width = sdt_node_ptr->src.size * 3;
          sdt_node_ptr->dst.size = sdt_node_ptr->src.size * 3 + 1;
          strcpy( sdt_node_ptr->dst.format_spec, "%-3.3o");
        } else {
          field_width = sdt_node_ptr->src.size * 2;
          sdt_node_ptr->dst.size = sdt_node_ptr->src.size * 2 + 1;
          strcpy( sdt_node_ptr->dst.format_spec, "%-2.2X");
          sprintf( buf, "\"X%d\"", sdt_node_ptr->dst.size-1);
          OaStrtoKwdValue( "FORMAT", current_node, buf);        
        }
      } else {
        if (sdt_node_ptr->src.binrep_descrip->type == BINREP_INTEGER) {
          if (((int) strlen( str) > 1) && (str[0] == 'I')) {
            if (sscanf( str+1, "%d", &field_width) != 1)
              format_error = TRUE;
          } else {
            format_error = TRUE;
          }
        } else {                                   /* BINREP_FLOAT */
          if (((int) strlen( str) >= 4) && ((str[0] == 'E') ||
              (str[0] == 'F'))) {
            if (sscanf( str+1, "%d.%d", &field_width, &precision) != 2)
              format_error = TRUE;
          } else {
            format_error = TRUE;
          }
        }
      }

      if ((format_error == TRUE) || (field_width == 0)) {
        sprintf( error_string, "%s: error parsing FORMAT keyword value",
                 proc_name, str);
        if (OaKwdValuetoStr( "NAME", current_node, &str) == 0)
          sprintf( error_string + strlen( error_string), 
                   ", NAME = %s.", str);
        strcat( error_string, " Using default from profile.");
        oa_errno = 601;
        OaReportError( error_string);
      } else {
        sdt_node_ptr->dst.size = field_width + 1;
        c = tolower(str[0]);  /* c should be 'i','f' or 'e' */
      }
    }  /* end if found FORMAT keyword */

    if ((format_error == TRUE) || (field_width == 0)) {

      /* FORMAT keyword was absent, or didn't work out, so use the default
         ASCII field width from profile.  If dst is ASCII_BIT_STRING, then
         then make the format hex.  */

      if (sdt_node_ptr->dst.PDS_data_type != OA_ASCII_BIT_STRING) {
        sdt_node_ptr->dst.size = conversion_info->ASCII_size;
        field_width = sdt_node_ptr->dst.size-1;  /* First dst char is a space*/
        c = 'g';                                 /* Only used for floats     */
        precision = sdt_node_ptr->dst.size - 7;  /* Ditto above, 7 is ad-hoc */

      } else {  /* print value byte-by-byte in hex (2 digits per byte) */
        sdt_node_ptr->dst.size = sdt_node_ptr->src.size * 2 + 1;
        strcpy( sdt_node_ptr->dst.format_spec, "%-2.2X");
        sprintf( buf, "\"X%d\"", sdt_node_ptr->dst.size-1);
        OaStrtoKwdValue( "FORMAT", current_node, buf);        
      }
    }

    /* If the format specification wasn't written above (it was only if we
       have a BIT_STRING), then write it.  The number is to be right-justified,
       with the calculated field width (and precision for floats).  */

    if (strlen( sdt_node_ptr->dst.format_spec) == 0) {

      if (cross_conversion == FALSE) {        
        if (sdt_node_ptr->src.binrep_descrip->type == BINREP_INTEGER)
          sprintf( sdt_node_ptr->dst.format_spec, "%s%dld", "%", field_width);
        else                                       /* BINREP_FLOAT */
          sprintf( sdt_node_ptr->dst.format_spec, "%s%d.%d%c", "%", 
                   field_width, precision, c);
      } else {

        /* For cross conversions, OalConvert converts the src to a long or
           double, casts to a double or long respectively, then converts to
           ASCII using dst.format_spec; thus format_spec must be set for the
           dst type, not the src type.  */

        if (sdt_node_ptr->dst.PDS_data_type == OA_ASCII_INTEGER)
          sprintf( sdt_node_ptr->dst.format_spec, "%s%dld", "%", field_width);
        else                                /* OA_ASCII_REAL */
          sprintf( sdt_node_ptr->dst.format_spec, "%s%d.%d%c", "%", 
                   field_width, precision, c);
      }
    }  /* end if format_spec hasn't been written yet */

  } else {  /* is_numeric is FALSE */

    sdt_node_ptr->conversion_type = OA_MEMCPY;
    sdt_node_ptr->dst.PDS_data_type = sdt_node_ptr->src.PDS_data_type;
    sdt_node_ptr->dst.size = sdt_node_ptr->src.size;
  }

  return(0);

}   /* end src BINARY, dst ASCII  */


/* If source is ASCII and destination is binary, set up for a 
   ASCII-to-binary conversion, then drop down to the last section of code
   to set dst.alignment_req. */

if ((src_interchange_format == OA_ASCII_INTERCHANGE_FORMAT) &&
    (dst_interchange_format == OA_BINARY_INTERCHANGE_FORMAT)) {

  /* If native_binrep_q_code is non-NULL, then this means dst is numeric,
     so set up for an ASCII-to-binary conversion.  Otherwise it's not numeric,
     or OA doesn't know about it, so set up for a straight copy.  */

  if (is_numeric == TRUE) {
    sdt_node_ptr->conversion_type = OA_ASCII_TO_BINARY;
    sdt_node_ptr->dst.PDS_data_type = conversion_info->native_PDS_data_type;
    sdt_node_ptr->dst.size = conversion_info->native_size;

    /* Determine dst.binrep_descrip.  First copy the dst binrep q-code to a
       buffer and check that its length is defined; if not, assume the ASCII
       source is in hex format (two hex digits/letters per byte), and append
       src.size/2 as the binary length to the end of the q-code so that
       OalFindBinrepDescrip will create an appropriate binrep descriptor 
       on-the-fly (for BIT_STRINGS).  */

    strcpy( binrep_q_code, conversion_info->native_binrep_q_code);
    len = (int) strlen( binrep_q_code);
    if (!isdigit( binrep_q_code[ len-1]))
      sprintf( binrep_q_code + len, "%-d", (int) sdt_node_ptr->src.size/2);

    if ((binrep_descrip = OalFindBinrepDescrip( binrep_q_code)) != NULL) {
      sdt_node_ptr->dst.binrep_descrip = binrep_descrip;
    } else {
      sprintf( error_string, "%s: couldn't find q_code: %s in ",
               proc_name, conversion_info->native_binrep_q_code);
      sprintf( error_string + strlen( error_string), 
              "binrep_descriptions for PDS_data_type: %s.",
              OaPDSDataTypetoStr( sdt_node_ptr->dst.PDS_data_type));
      oa_errno = 730;
      OaReportError( error_string);
      return(1);
    }
    if (((sdt_node_ptr->dst.binrep_descrip->type == BINREP_FLOAT) &&
         (sdt_node_ptr->src.PDS_data_type == OA_ASCII_INTEGER)) ||
        ((sdt_node_ptr->dst.binrep_descrip->type == BINREP_INTEGER) &&
         (sdt_node_ptr->src.PDS_data_type == OA_ASCII_REAL))) {
      OaDeleteKwd( "FORMAT", current_node);
    }

  } else {  /* is_numeric == FALSE */
      sdt_node_ptr->conversion_type = OA_MEMCPY;
      sdt_node_ptr->dst.PDS_data_type = sdt_node_ptr->src.PDS_data_type;
      sdt_node_ptr->dst.size = sdt_node_ptr->src.size;
      sdt_node_ptr->dst.alignment_req = ALIGN_1;
  }  /* end else not numeric      */
}    /* end src ASCII, dst BINARY */



/* If source is binary and destination is binary, set up for a 
   binary-to-binary conversion, then drop down to the last section of code
   to set dst.alignment_req. */

if ((src_interchange_format == OA_BINARY_INTERCHANGE_FORMAT) &&
    (dst_interchange_format == OA_BINARY_INTERCHANGE_FORMAT)) {

  if (is_numeric == TRUE) {

    sdt_node_ptr->conversion_type = OA_BINREP;
    sdt_node_ptr->dst.PDS_data_type = conversion_info->native_PDS_data_type;
    sdt_node_ptr->dst.size = conversion_info->native_size;

    /* Determine dst.binrep_descrip.  First copy the dst binrep q-code to a
       buffer and check that its length is defined; if not, assume the dst
       size is the same as the source size: append src.size as the length to
       the end of the q-code so that OalFindBinrepDescrip will create an
       appropriate binrep descriptor on-the-fly (for BIT_STRINGS).  */

    strcpy( binrep_q_code, conversion_info->native_binrep_q_code);
    len = (int) strlen( binrep_q_code);
    if (!isdigit( binrep_q_code[ len-1])) {
      sprintf( binrep_q_code + len, "%-d", (int) sdt_node_ptr->src.size);
      sdt_node_ptr->dst.size = sdt_node_ptr->src.size;
    }

    if ((binrep_descrip = OalFindBinrepDescrip( binrep_q_code)) != NULL) {
      sdt_node_ptr->dst.binrep_descrip = binrep_descrip;
    } else {
      sprintf( error_string, "%s: couldn't find q_code: %s in ",
               proc_name, conversion_info->native_binrep_q_code);
      sprintf( error_string + strlen( error_string), 
              "binrep_descriptions for PDS_data_type: %s.",
              OaPDSDataTypetoStr( sdt_node_ptr->dst.PDS_data_type));
      oa_errno = 730;
      OaReportError( error_string);
      return(1);
    }
    if (sdt_node_ptr->src.binrep_descrip->type !=
        sdt_node_ptr->dst.binrep_descrip->type) {
      OaDeleteKwd( "FORMAT", current_node);
    }

  } else {  /* is_numeric == FALSE */

    sdt_node_ptr->conversion_type = OA_MEMCPY;
    sdt_node_ptr->dst.PDS_data_type = sdt_node_ptr->src.PDS_data_type;
    sdt_node_ptr->dst.size = sdt_node_ptr->src.size;
    sdt_node_ptr->dst.alignment_req = ALIGN_1;
  }
}

/* Determine the alignment_req for the data node from its dst size and type.
   (dst_interchange_format is BINARY if we got this far).  */

sdt_node_ptr = (SDT_node *) current_node->appl1;
in_memory_size = sdt_node_ptr->dst.size;
sdt_node_ptr->dst.alignment_req = ALIGN_1;
if ((is_numeric == TRUE) && (in_memory_size > 1)) {
  switch ( in_memory_size) {
 
    case 2:  /* integers */
      switch ( Oa_profile.dst_alignment_type) {
        case OA_NO_ALIGN:   sdt_node_ptr->dst.alignment_req = ALIGN_1;
        break;
        case OA_ALIGN_EVEN: sdt_node_ptr->dst.alignment_req = ALIGN_2;  
        break;
        case OA_ALIGN_RISC: sdt_node_ptr->dst.alignment_req = ALIGN_2;  
        break;
      }
    break;

    case 4:  /* integers and reals */
      switch ( Oa_profile.dst_alignment_type) {
        case OA_NO_ALIGN:   sdt_node_ptr->dst.alignment_req = ALIGN_1;
        break;
        case OA_ALIGN_EVEN: sdt_node_ptr->dst.alignment_req = ALIGN_2;
        break;
        case OA_ALIGN_RISC: sdt_node_ptr->dst.alignment_req = ALIGN_4;
        break;
      }
    break;

    case 8:  /* reals */
    case 16: /* VAXH_REAL */
      switch ( Oa_profile.dst_alignment_type) {
        case OA_NO_ALIGN:   sdt_node_ptr->dst.alignment_req = ALIGN_1;
        break;
        case OA_ALIGN_EVEN: sdt_node_ptr->dst.alignment_req = ALIGN_2;
        break;
        case OA_ALIGN_RISC: sdt_node_ptr->dst.alignment_req = ALIGN_8;
        break;
      }
    break;

    default:  /* Not a normal numeric type, because size is not 1,2,4,8, or 16
                 so doesn't need to be aligned.  This means 10-byte temporary
                 floats are NOT aligned!  */
      sdt_node_ptr->dst.alignment_req = ALIGN_1;  break;
  }  /* end of switch on in_memory_size */
}    /* end of if ((is_numeric == TRUE) && (in_memory_size > 1))  */


return(0);
}



/*****************************************************************************

  Routine:  OaFindEncodingHistogram

  Description:  This routine finds the ODLTREE node describing the encoding
                histogram for a Huffman First Difference (HFD) compressed
                image.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:   1 Sept  1994

  History:

    Creation - This routine was part of the Alpha Release of the OA library.

  Input:   
          input_node - Pointer to an ODL tree node of class IMAGE with an 
                       ENCODING_TYPE keyword value of HUFFMAN_FIRST_DIFFERENCE.
         
  Output:  The function returns a pointer to the encoding histogram node if it
           was found, otherwise NULL. 

  Notes:  

  The logic for finding the encoding histogram is as follows:
  1) Loop through all the siblings of the input image node, checking for a
     histogram node;  the first one which matches the criteria is chosen.
     Criteria:
       object class == OA_HISTOGRAM AND
       "ITEMS" keyword value greater or equal to 511 AND
       ("OBJECT" keyword value contains "ENCOD" OR
        "NAME" keyword value contains "ENCOD")

*****************************************************************************/

#ifdef _NO_PROTO

ODLTREE OaFindEncodingHistogram( input_node)
ODLTREE input_node;

#else

ODLTREE OaFindEncodingHistogram( ODLTREE input_node)

#endif
{

static char *proc_name = "OaFindEncodingHistogram";
ODLTREE current_node, histogram_node = NULL;
long l;

/* Check input parameters.  */

if (OaGetObjectClass( input_node) != OA_IMAGE) {
  sprintf( error_string, "%s: input node must be an IMAGE.", proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return( NULL);
}

if (Parent( input_node) == NULL) {
  sprintf( error_string, "%s: couldn't find HFD decoding HISTOGRAM.", 
           proc_name);
  oa_errno = 530;
  OaReportError( error_string);
  return( NULL);
}

/* Loop through all siblings of the input node.  */

current_node = LeftmostChild( Parent( input_node));
while (current_node != NULL) {
  if (OaGetObjectClass( current_node) == OA_HISTOGRAM) {
    if (OaKwdValuetoLong( "ITEMS", current_node, &l) == 0) {
      if (l >= 511) {
        if ((OdlFindKwd( current_node, "NAME", "*ENCOD*", (unsigned long) 0,
                         (unsigned short) ODL_THIS_OBJECT) != NULL) ||
          (strstr( OdlGetObjDescClassName( current_node), "ENCOD") != NULL)) {
          histogram_node = current_node;
          break;
        }
      }
    }
  }
  current_node = RightSibling( current_node);
}
return( histogram_node);
}



/*****************************************************************************

  Routine:  OalDetermineRepNodeAlignment

  Description:  This funtion determines the alignment requirement of a
                repetitions node, based on the most restrictive alignment
                requirement of all the node's children.  If alignment bytes
                are needed, it adds a SPARE node to the left of the input
                node, with src.size=0.

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   3 Dec   1994
  Last Modified:   3 Dec   1994

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:
         input_node - An ODL tree node with an SDT node attached;  the 
                      alignment_req fields of all the SDT node's children
                      should be set.
         
  Output:  The function always returns 0.

  Notes:  Explicit SPARES are added to represent alignment bytes for 
          repetitions nodes, but for data nodes, they're left implicit
          (i.e. a gap is left in the SDT's dst description).
*****************************************************************************/
  
#ifdef _NO_PROTO

int OalDetermineRepNodeAlignment( input_node)
ODLTREE input_node;

#else

int OalDetermineRepNodeAlignment( ODLTREE input_node)

#endif
{

static char *proc_name = "OalDetermineRepNodeAlignment";
ODLTREE next_node, left_sibling, spare_node;
SDT_node *tmp_sdt_node_ptr, *sdt_node_ptr;
long pad_bytes;

sdt_node_ptr = (SDT_node *) input_node->appl1;

/* Determine alignment requirement, based on the most stringent alignment_req
   of all the node's children.  */

next_node = LeftmostSDTChild( input_node);
do {
  tmp_sdt_node_ptr = (SDT_node *) next_node->appl1;
  if (tmp_sdt_node_ptr->dst.alignment_req > sdt_node_ptr->dst.alignment_req)
    sdt_node_ptr->dst.alignment_req = tmp_sdt_node_ptr->dst.alignment_req;
} while ((next_node = RightSDTSibling( next_node)) != NULL);

/* If the input node has left sibling, see if its left sibling ends on a 
   boundary that satisfies the alignment requirement found above.  
   If not, determine the number of needed pad bytes, and add a SPARE to the
   left of the input node with src.size=0,  dst.size=pad_bytes and 
   dst.start_offset set appropriately.  */

if ((left_sibling = LeftSDTSibling( input_node)) != NULL) {
  tmp_sdt_node_ptr = (SDT_node *) left_sibling->appl1;
  pad_bytes =  (tmp_sdt_node_ptr->dst.start_offset + 
                tmp_sdt_node_ptr->dst.size) %
                sdt_node_ptr->dst.alignment_req;
  if (pad_bytes > 0) pad_bytes = sdt_node_ptr->dst.alignment_req - pad_bytes;
  if (pad_bytes > 0) {
    spare_node = OdlNewObjDesc( "COLUMN", NULL, NULL, NULL, NULL, NULL, 
                                 (short) 0, (long) 0);
    OaStrtoKwdValue( "NAME", spare_node, "SPARE");
    OdlPasteObjDescBefore( spare_node, input_node);
    tmp_sdt_node_ptr = OalNewSDTNode();
    spare_node->appl1 = (char *) tmp_sdt_node_ptr;

    tmp_sdt_node_ptr->total_repetitions = 1;
    tmp_sdt_node_ptr->dst.start_offset = 
       tmp_sdt_node_ptr->dst.start_offset + 
       tmp_sdt_node_ptr->dst.size * tmp_sdt_node_ptr->total_repetitions;

    tmp_sdt_node_ptr->src.size = 0;
    tmp_sdt_node_ptr->dst.size = pad_bytes;
    tmp_sdt_node_ptr->dst.alignment_req = 1;

    sprintf( error_string, "%s: SPARE needed for alignment; ", 
             proc_name);
    sprintf( error_string + strlen( error_string), 
           "inserted a SPARE COLUMN node before %s node, with dst.size: %d", 
            input_node->class_name, (int) pad_bytes);
    sprintf( error_string + strlen( error_string), 
             " and dst.start_offset: %ld.", 
             tmp_sdt_node_ptr->dst.start_offset);
    oa_errno = 950;
    OaReportError( error_string);

    /* All other fields of the SPARE SDT node were initialized the way we want
       them to be by OalNewSDTNode().  This node is exclusively for use as a 
       place-holder by the SDT processing software, and will be removed
       before the Object Layer ever sees it.  */

    } /* end if need to add a SPARE node for pad_bytes.  */
  } /* if there's a left_sibling of input_node.        */
return(0);
}



/*****************************************************************************

  Routine:  OalFreeSDT

  Description:  This routine frees the entire ODL tree whose root is passed in,
                and frees the SDT nodes attached to the ODL tree nodes.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  14 Feb  1995
  Last Modified:  16 Apr  1997

  History:

    Creation - This routine was part of the Beta Release of the OA library.
    04/16/97 - Added calling of OalFreeSDTStruct.  SM

  Input:  
          root_node - An ODL tree which usually has SDT nodes attached; 

  Output:  The ODL tree and attached SDT nodes have all been freed.  An SDT
           node is assumed to be attached if appl1 is non-NULL.
           The function always returns 0.

  Notes:  

*****************************************************************************/

#ifdef _NO_PROTO

int OalFreeSDT( root_node)
ODLTREE root_node;

#else

int OalFreeSDT( ODLTREE root_node)

#endif
{
if (root_node != NULL) {
  OalFreeSDT( root_node->first_child);
  OalFreeSDT( root_node->right_sibling);
  OdlFreeAllKwds( root_node);
  if (root_node->appl1 != NULL)
    OalFreeSDTStruct( (SDT_node *) root_node->appl1);
  LemmeGo(root_node->class_name)      
  LemmeGo(root_node->pre_comment)
  LemmeGo(root_node->line_comment)
  LemmeGo(root_node->post_comment)
  LemmeGo(root_node->end_comment)
  LemmeGo(root_node->file_name)
  LemmeGo(root_node)
}               
return(0);
}



/*****************************************************************************

  Routine:  OalFreeSDTStruct

  Description:  This routine frees the SDT structure and its dynamically
                allocated binrep structures, if any.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  16 Apr  1997
  Last Modified:  16 Apr  1997

  History:

    Creation - This routine was part of the Version 1.2 Release of the OA
               library.

  Input:  
          sdt_node_ptr - A pointer to a SDT_node structure. 

  Output:  The function always returns 0.

  Notes:  

*****************************************************************************/

#ifdef _NO_PROTO

int OalFreeSDTStruct( sdt_node_ptr)
SDT_node *sdt_node_ptr;

#else

int OalFreeSDTStruct( SDT_node *sdt_node_ptr)

#endif
{
if (sdt_node_ptr != NULL) {
  if (sdt_node_ptr->src.binrep_descrip != NULL) {
    if (sdt_node_ptr->src.binrep_descrip->is_dynamic == 1) {
      LemmeGo( sdt_node_ptr->src.binrep_descrip->q_code)
      LemmeGo( sdt_node_ptr->src.binrep_descrip->integer_order)
      LemmeGo( sdt_node_ptr->src.binrep_descrip);
    }
  }
  if (sdt_node_ptr->dst.binrep_descrip != NULL) {
    if (sdt_node_ptr->dst.binrep_descrip->is_dynamic == 1) {
      LemmeGo( sdt_node_ptr->dst.binrep_descrip->q_code)
      LemmeGo( sdt_node_ptr->dst.binrep_descrip->integer_order)
      LemmeGo( sdt_node_ptr->dst.binrep_descrip);
    }
  }
  LemmeGo( sdt_node_ptr);
}  /* end if sdt_node_ptr != NULL */
return(0);
}



/*****************************************************************************

  Routine:  OalGetNativeCTypeInfo

  Description:  This routine returns the PDS data type enum, size in bytes,
                and a pointer to a binrep_descrip structure corresponding to
                the input C type string, according to the platform being run
                on.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  25 Apr  1996
  Last Modified:  25 Apr  1996

  History:

    Creation - This routine was part of the Version 1.1 Release of the OA
               library.

  Input:  
          c_type         - A C numeric type, in lower-case.  Allowed values
                           are: "char", "unsigned char", "short",
                           "unsigned short", "int", "unsigned int", "long",
                           "unsigned long", "float", "double".

  Output:  If successful, the function returns 0, otherwise 1.

          PDS_data_type  - The enumerated value of the PDS data type.
 
          size           - The size in bytes of the PDS data type.

          binrep_descrip - A pointer to one of the structures in the 
                           global binrep_descriptions array.

  Notes:  

*****************************************************************************/

#ifdef _NO_PROTO

int OalGetNativeCTypeInfo( c_type, PDS_data_type, size, binrep_descrip)
char *c_type;
int *PDS_data_type;
int *size;
struct binrep_desc **binrep_descrip;

#else

int OalGetNativeCTypeInfo( char *c_type, int *PDS_data_type, int *size,
                           struct binrep_desc **binrep_descrip)

#endif
{

char *proc_name = "OalGetNativeCTypeInfo";
char *q_code;
int sizeof_int = sizeof( int);
int sizeof_long = sizeof( long);
short endian_indicator=1;      /* Initialized to 1 to determine the value */
                               /* of *platform_is_little_endian below.    */
char *platform_is_little_endian = (char *) &endian_indicator;
/* If platform is little-endian (lsb first), then the 1 initialized in
   endian_indicator will be in the first byte, and *platform_is_little_endian
   will have the value TRUE (1), otherwise it will be FALSE (0).  Using this
   saves a lot of #ifdefs.  */

if ((c_type == NULL) || (PDS_data_type == NULL) || (size == NULL) ||
    (binrep_descrip == NULL)) {
  sprintf( error_string, "%s: one or more inputs is NULL.", proc_name);
  oa_errno = 501;
  OaReportError( error_string);
  return(1);
}

if        (strcmp( c_type, "char") == 0) {
  if (*platform_is_little_endian == TRUE)
    *PDS_data_type = OA_LSB_INTEGER;
  else
    *PDS_data_type = OA_MSB_INTEGER;
  *size = 1;
  q_code = "INT1";

} else if (strcmp( c_type, "unsigned char") == 0) {
  if (*platform_is_little_endian == TRUE)
    *PDS_data_type = OA_LSB_UNSIGNED_INTEGER;
  else
    *PDS_data_type = OA_MSB_UNSIGNED_INTEGER;
  *size = 1;
  q_code = "UINT1";

} else if (strcmp( c_type, "short") == 0) {
  if (*platform_is_little_endian == TRUE) {
    *PDS_data_type = OA_LSB_INTEGER;
    q_code = "LSB_INT2";
  } else {
    *PDS_data_type = OA_MSB_INTEGER;
    q_code = "MSB_INT2";
  }
  *size = 2;

} else if (strcmp( c_type, "unsigned short") == 0) {
  if (*platform_is_little_endian == TRUE) {
    *PDS_data_type = OA_LSB_UNSIGNED_INTEGER;
    q_code = "LSB_UINT2";
  } else {
    *PDS_data_type = OA_MSB_UNSIGNED_INTEGER;
    q_code = "MSB_UINT2";
  }
  *size = 2;

} else if (strcmp( c_type, "int") == 0) {
  *size = sizeof_int;
  if (*platform_is_little_endian == TRUE) {
    *PDS_data_type = OA_LSB_INTEGER;
    if (sizeof_int == 2)
      q_code = "LSB_INT2";
    else
      q_code = "LSB_INT4";
  } else {
    *PDS_data_type = OA_MSB_INTEGER;
    if (sizeof_int == 2)
      q_code = "MSB_INT2";
    else
      q_code = "MSB_INT4";
  }

} else if (strcmp( c_type, "unsigned int") == 0) {
  *size = sizeof_int;
  if (*platform_is_little_endian == TRUE) {
    *PDS_data_type = OA_LSB_UNSIGNED_INTEGER;
    if (sizeof_int == 2)
      q_code = "LSB_UINT2";
    else
      q_code = "LSB_UINT4";
  } else {
    *PDS_data_type = OA_MSB_UNSIGNED_INTEGER;
    if (sizeof_int == 2)
      q_code = "MSB_UINT2";
    else
      q_code = "MSB_UINT4";
  }

} else if (strcmp( c_type, "long") == 0) {
  *size = sizeof_long;
  if (*platform_is_little_endian == TRUE) {
    *PDS_data_type = OA_LSB_INTEGER;
    if (sizeof_long == 8)  
      q_code = "LSB_INT8";  /* Dec/Alpha */
    else
      q_code = "LSB_INT4";
  } else {
    *PDS_data_type = OA_MSB_INTEGER;
    if (sizeof_long == 8)   
      q_code = "MSB_INT8";  /* No such platform...yet. */
    else
      q_code = "MSB_INT4";
  }

} else if (strcmp( c_type, "unsigned long") == 0) {
  *size = sizeof_long;
  if (*platform_is_little_endian == TRUE) {
    *PDS_data_type = OA_LSB_UNSIGNED_INTEGER;
    if (sizeof_long == 8)
      q_code = "LSB_UINT8";
    else
      q_code = "LSB_UINT4";
  } else {
    *PDS_data_type = OA_MSB_UNSIGNED_INTEGER;
    if (sizeof_long == 8)
      q_code = "MSB_UINT8";
    else
      q_code = "MSB_UINT4";
  }

} else if (strcmp( c_type, "float") == 0) {
  *size = 4;
#if defined( VAX) || defined( ALPHA_VMS)
  *PDS_data_type = OA_VAX_REAL;
  q_code = "VAXF_REAL4";
#else
#if defined( IBM_PC) || defined( ULTRIX) || defined( ALPHA_OSF)
  *PDS_data_type = OA_PC_REAL;
  q_code = "PC_REAL4";
#else
  *PDS_data_type = OA_IEEE_REAL;
  q_code = "IEEE_REAL4";
#endif
#endif

} else if (strcmp( c_type, "double") == 0) {
  *size = 8;
#if defined( VAX)
  *PDS_data_type = OA_VAX_REAL;
  q_code = "VAXD_REAL8";
#else
#if defined( ALPHA_VMS)
  *PDS_data_type = OA_VAX_REAL;
  q_code = "VAXG_REAL8";
#else
#if defined( IBM_PC) || defined( ULTRIX) || defined( ALPHA_OSF)
  *PDS_data_type = OA_PC_REAL;
  q_code = "PC_REAL8";
#else
  *PDS_data_type = OA_IEEE_REAL;
  q_code = "IEEE_REAL8";
#endif
#endif
#endif

} else {
  sprintf( error_string, "%s: c_type input is invalid: %s.", proc_name, 
           c_type);
  oa_errno = 501;
  OaReportError( error_string);
  return(1);
}
*binrep_descrip = OalFindBinrepDescrip( q_code);
if (*binrep_descrip == NULL) {
  sprintf( error_string, "%s: couldn't find binrep descrip for q_code = %s",
           proc_name, q_code);
  oa_errno = 730;
  OaReportError( error_string);
  return(1);
}
return(0);
}



/*****************************************************************************

  Routine:  OalGetTypeConversionFromProfile

  Description:  This function searches the global array of structures 
                Oa_type_conversion_info for an entry with the PDS_data_type
                and size specified in the input parameters.  It uses the
                global variable Oa_profile to find the appropriate sub-array of
                Oa_type_conversion_info to search in.  
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:  21 Apr   1997

  History:

    Creation - This routine was part of the Alpha Release of the OA library.
    04/27/97 - Modified output specification as commented below.  SM

  Input:  
          PDS_data_type - The enumerated type of the data.

          size          - The size in bytes of the data.

          Oa_profile    - The data_translation_profile field of global
                          variable Oa_profile must be set, and must be within
                          the sub-array bounds of the Oa_profile array.
         
  Output:  If PDS_data_type and size match a structure in global variable
           Oa_type_conversion_info, then the function returns a pointer to the
           structure;  if PDS_data_type matches and
           Oa_type_conversion_info[i][j].size is 0, then the function ignores
           the 'size' input and returns a pointer to the structure.
           Otherwise it returns NULL.

  Notes:  

  1) This function expects the last structure in each sub-array of 
     Oa_type_conversion_info to have its PDS_data_type set to 
     OA_UNKNOWN_DATA_TYPE, as this is the way the function determines the end
     of the array.

*****************************************************************************/

#ifdef _NO_PROTO

struct oa_type_conversion_info *OalGetTypeConversionFromProfile( 
                                PDS_data_type, size)
int  PDS_data_type;
long size;

#else

struct oa_type_conversion_info *OalGetTypeConversionFromProfile( 
                                int PDS_data_type, long size)

#endif
{

char *proc_name = "OalGetTypeConversionFromProfile";
int i = Oa_profile.data_translation_profile;
int j;

if ((i<0) || (i>=OA_PROFILES)) {
  sprintf( error_string, 
           "%s: internal error:\nOa_profile.data_translation_profile = %d\n",
           proc_name, Oa_profile.data_translation_profile);
  strcat( error_string, 
          "is out of the range of the Oa_type_conversion_info array.");
  oa_errno = 730;
  OaReportError( error_string);
  return(NULL);
}

for (j=0; Oa_type_conversion_info[i][j].PDS_data_type !=
          (char) OA_UNKNOWN_DATA_TYPE; j++) {
  if (Oa_type_conversion_info[i][j].PDS_data_type == (char) PDS_data_type) {
    if (Oa_type_conversion_info[i][j].size > 0) {
      if (Oa_type_conversion_info[i][j].size == (char) size)
        return( &(Oa_type_conversion_info[i][j]));
    } else {
      return( &(Oa_type_conversion_info[i][j]));
    }
  }
}

return(NULL);
}



/*****************************************************************************

  Routine:  OalNewSDTNode

  Description:  This routine allocates an SDT node structure and initializes
                it.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:   1 Sept  1994

  History:

    Creation - This routine was part of the Alpha Release of the OA library.

  Input:  None.
         
  Output:  The function returns a pointer to a SDT node in dynamic memory,
           initialized to 0's.

  Notes:  

*****************************************************************************/

SDT_node *OalNewSDTNode() 
{

static char *proc_name = "OalNewSDTNode";
char *ptr;
int i;

if (( ptr = (char *) OaMalloc( sizeof( SDT_node))) == NULL) {
  sprintf( error_string, "%s: OaMalloc returned error", proc_name);
  oa_errno = 720;
  OaReportError( error_string);
  exit(1);
  /*NOTREACHED*/
} else {                   /* Initialize every byte in the structure to 0. */
                           /* WARNING! May not work for pointers on IBM-PC */
                           /* Fix this when SDT_node definition is stable. */
  for (i=0; i<sizeof( SDT_node); i++) ptr[i] = 0;
  return( (SDT_node *) ptr);
}
/*NOTREACHED*/
return( NULL);
}



/*****************************************************************************

  Routine:  OalInitializeSDT

  Description:  This routine finds the first data node in the SDT, and stores 
                the pointer in that node, and in the TLO (root) node.  It
                returns a pointer to the first data node.  This setup is
                always needed before calling OalProcessSDT.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:  17 Oct  1994
  Last Modified:  17 Oct  1994

  History:

    Creation - This routine was part of the Beta Release of the OA library.

  Input:  sdt      - The root node of the SDT.

          data_ptr - A pointer to the start of the storage for the destination
                     data.

  Output:  The function returns a pointer to the first data node.

  Notes:  This function can be used to re-initialize an SDT which has been
          used by OalProcessSDT.  The current_reps parameter in every SDT
          node is set to 0.  It does NOT reset any other SDT parameters which 
          might be non-zero from a previous call to OalProcessSDT.  If 
          OalProcessSDT didn't finish processing the SDT, i.e. didn't return
          OA_REACHED_END_OF_SDT, then this might be the case; 
          SDT_node->buf_bytes, SDT_node->src.bytes_processed etc. might still 
          be non-zero, but it would be an error to use an SDT which hasn't
          been fully traversed, so these are NOT reset by this function.

*****************************************************************************/

#ifdef _NO_PROTO

ODLTREE OalInitializeSDT( sdt, data_ptr)
ODLTREE sdt;
PTR data_ptr;

#else

ODLTREE OalInitializeSDT( ODLTREE sdt, PTR data_ptr)

#endif
{

ODLTREE current_node;
SDT_node *sdt_node_ptr;

if (data_ptr == NULL)   return( NULL);
if (sdt == NULL)        return( NULL);
if (sdt->appl1 == NULL) return( NULL);

/* Set the dst.ptr of the TLO (root) node to data_ptr.  */

sdt_node_ptr = (SDT_node *) sdt->appl1;
sdt_node_ptr->dst.ptr = data_ptr;

/* Traverse the tree, setting current_reps to 0 in each SDT node.  */

current_node = sdt;
while( current_node != NULL) {

  if (current_node->appl1 != NULL) {
    sdt_node_ptr = (SDT_node *) current_node->appl1;
    sdt_node_ptr->current_reps = 0;
  }

  /* Position to the next node. */

  if (LeftmostChild( current_node) != NULL)
    current_node = LeftmostChild( current_node);
  else if (RightSibling( current_node) != NULL)
    current_node = RightSibling( current_node);
  else {
    while ((current_node = Parent( current_node)) != NULL) {
      if (RightSibling( current_node) != NULL) {
        current_node = RightSibling( current_node);
        break;
      }
    }
  }
}

/* Position to the first data node and set its dst.ptr to data_ptr. */

current_node = sdt;
while (LeftmostChild( current_node) != NULL)
  current_node = LeftmostChild( current_node);
sdt_node_ptr = (SDT_node *) current_node->appl1;
sdt_node_ptr->dst.ptr = data_ptr;
return( current_node);

}



/*****************************************************************************

  Routine:  OalPositionToNextDataNode

  Description:  This routine positions to the next SDT node during the 
                processing of a buffer of data read by the Stream Layer.
                It is called by OalProcessSDT.   It determines the "next" data
                node according to a modified post-order tree traversal order.
                In addition to the actual traversal, when traversing through a
                repetitions node, it increments the node's repetitions count, 
                and when the "next" data node is reached, it sets the node's
                dst.ptr.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:   1 Sept  1994

  History:

    Creation - This routine was part of the Alpha Release of the OA library.

  Input: current_node - An ODL tree node with an SDT node attached, whose SDT 
                        node's current_reps count is 0.  The node must be a
                        "data node"; see definitions below.  Every node in
                        the SDT MUST have an SDT node attached with either
                        src.size > 0 or dst.size > 0.
         
  Output:  The function returns the next data node, if there is one; if it
           reaches the end of the tree (the root) before finding the one, it 
           returns NULL.  Once it finds the next data node, it computes the
           current destination address, dst.ptr, for the new node, taking as
           the starting address the pointer in the top-level node's dst.ptr. 

  Notes:  
  1) Classes of SDT nodes:  
     "data node": an ODL tree node with an SDT node attached, with either
        src.size > 0, dst.size > 0, or both > 0;  it may also have 
        repetitions, but has no children (at least, no children with SDT
        nodes attached - an ALIAS node with no attached SDT node is allowed);
        analogous to a COLUMN with ITEMS and ITEM_BYTES.
     "repetitions node": an ODL tree node with an SDT node attached, with 
        children which also have SDT nodes attached.  A repetitions node is
        used to loop through children.  Analogous to a TABLE with ROWS and
        ROW_BYTES, where the actual data is described by the COLUMN nodes.

     A data node has its repetition count incremented by OalProcessSDT each
     time a complete loop through it's data is finished.
     A repetitions node has its repetition count incremented by 
     OalPositionToNextDataNode each time a complete loop through its children 
     is finished. 

  2) The root node's SDT node should have dst.ptr set to the beginning of the
     dst memory, e.g. the pointer returned by OaMalloc for file reads into
     memory. 

*****************************************************************************/

#ifdef _NO_PROTO

ODLTREE OalPositionToNextDataNode( input_node)
ODLTREE input_node;

#else

ODLTREE OalPositionToNextDataNode( ODLTREE input_node)

#endif
{

ODLTREE current_node, next_node;
SDT_node *sdt_node_ptr, *next_sdt_node_ptr;
long offset_sum;

current_node = input_node;

if (RightSibling( current_node) != NULL) {  /* There's a right sibling, so  */
                                            /* follow its branch down to    */
                                            /* the lowest, leftmost child.  */
  current_node = RightSibling( current_node);
  while ((next_node = LeftmostChild( current_node)) != NULL)
    current_node = next_node;

} else {  /* No right sibling, so go up to the parent repetitions node and
             see if it has repetitions left to go.  If so, go to it's leftmost
             lowest data node.  If not, see if it has a right sibling.  If so,
             follow it down to it's leftmost, lowest data node.  If not, go
             up to the next parent.  */
             
  for (;;) {
    current_node = Parent( current_node);
    if (current_node == NULL) return( NULL);     /* No parent, so we've done
                                                    the whole tree.          */
    sdt_node_ptr = (SDT_node *) current_node->appl1;
    sdt_node_ptr->current_reps++;
    if (sdt_node_ptr->current_reps == sdt_node_ptr->total_repetitions) {  

      /* Repetitions are exhausted, so reset the current repetitions count.*/

      sdt_node_ptr->current_reps = 0;

      /* If node has a right sibling, position to its lowest, leftmost child;
         else drop through and repeat the for loop with the parent.  */

      if ((next_node = RightSibling( current_node)) != NULL) {
        while (next_node != NULL) {
          current_node = next_node;
          next_node = LeftmostChild( next_node);
        }
        break;  /* Break out of for loop. */
      }

    } else {  /* Position for another repetition through parent's children,
                 starting at the lowest, leftmost child.  */
      while ((next_node = LeftmostChild( current_node)) != NULL) {
        current_node = next_node;
      }
      break;  /* Break out of for loop. */
    }
  }  /* end for loop upwards in tree */
}  /* end else input node had no right sibling, so searched upwards. */

/* If the current node (a data node) has a non-zero dst.size, then it describes
   actual data to be converted from src to dst, as opposed to a place-holder 
   for src data which will be tossed out.  Determine the current pointer 
   value (memory address) of the destination:  start summing up  
   start_offset + current_reps * dst.size for all nodes above the current 
   node, moving upwards to the root.  When pass the TLO node, which by 
   definition contains the base pointer to the beginning of the data object 
   in memory, save the base pointer in the current node's dst.ptr.  When have
   reached the root, add the total byte offset which was summed up from all 
   the previous nodes to the base pointer value.  */

sdt_node_ptr = (SDT_node *) current_node->appl1;
if (sdt_node_ptr->dst.size > 0) {
  offset_sum = 0;
  next_node = current_node;
  do {
    next_sdt_node_ptr = (SDT_node *) next_node->appl1;
    if (Parent( next_node) == NULL)
      sdt_node_ptr->dst.ptr = next_sdt_node_ptr->dst.ptr;
    offset_sum += ((next_sdt_node_ptr->current_reps *
                  next_sdt_node_ptr->dst.size) +
                  next_sdt_node_ptr->dst.start_offset);
  } while ((next_node = Parent( next_node)) != NULL);

  sdt_node_ptr->dst.ptr += offset_sum;

} else {
  sdt_node_ptr->dst.ptr = NULL;
}
return( current_node);
}



/*****************************************************************************

  Routine:  OalPostOrderTraverse

  Description:  This function does a postorder traversal of an ODL tree, 
                calling the specified function at each node.  An example of a
                post-order traverse order is:

                     5 (root)
                    / \
                   3   4
                  / \
                 1   2

                Postorder traversals are used in OAL because they visit ODL
                tree nodes in the same order in which the data the nodes 
                describe is stored.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:  11 Dec   1997

  History:

    Creation - This routine was part of the Alpha Release of the OA library.
    12/11/97 - Changed input parameter 'operation' to be an enumerated type
               instead of a function pointer because caused too many compile
               problems.

  Input:  
          current_node -  An ODL tree node.

          operation              - Enumerated type specifying which function
                                   to call: OA_BUILD_SDT_NODE or 
                                   OA_ADJUST_KWDS_TO_MATCH_SDT.

          src_interchange_format - Set to either
                                   OA_ASCII_INTERCHANGE_FORMAT or 
                                   OA_BINARY_INTERCHANGE_FORMAT.

          dst_interchange_format - Set to either
                                   OA_ASCII_INTERCHANGE_FORMAT or 
                                   OA_BINARY_INTERCHANGE_FORMAT.
         
  Output:  OalPostOrderTraverse returns 0 after it has traversed the entire
           tree.  If the function it calls returns anything besides 0, 
           OalPostOrderTraverse aborts the traverse and returns the value.

  Notes:  

   - PostOrderTraverse will traverse all the nodes below the first node passed
     in, and visit the passed-in node last.
   - The post-order traverse order assures that for a given node, the 
     passed-in function has already been called for all nodes directly below 
     the given node in the tree; the function has already been called
     for all nodes before it in the post-order traverse order.
   - PostOrderTraverse is recursive; there are as many calls to it as there
     are tree nodes, but the number of nested calls on the system call stack 
     at any time is never greater than the depth of the tree.  
   - If the operation function deletes node(s) from the tree, it should be 
     very, very careful;  currently functions which delete node(s) are
     OalBuildSDTNode and OalAdjustKwdstoMatchSDT: in some special cases they 
     delete the current node and/or some of its left siblings.
     OalPostOrderTraverse will not work correctly if the operation function
     deletes the right sibling of the current node!

*****************************************************************************/

#ifdef _NO_PROTO

int OalPostOrderTraverse( current_node, operation, src_interchange_format,
                          dst_interchange_format)
ODLTREE current_node;
int operation;
int src_interchange_format, dst_interchange_format;
#else

int OalPostOrderTraverse( ODLTREE current_node, 
                          int operation,
                          int src_interchange_format, 
                          int dst_interchange_format)

#endif
{

int result;
ODLTREE next_node, right_sibling;

if (current_node != NULL) {
  next_node = LeftmostChild( current_node);
  while (next_node != NULL) {

    /* Find the right sibling of next_node now, and save it, in case the
       function deletes next_node.  */

    right_sibling = RightSibling( next_node);
    if ((result = OalPostOrderTraverse( next_node, operation, 
                                        src_interchange_format,
                                        dst_interchange_format)) != 0) 
      return( result);
    next_node = right_sibling;
  }
  switch( operation) {
    case OA_BUILD_SDT_NODE:
      result = OalBuildSDTNode( current_node, src_interchange_format,
                                dst_interchange_format);
    break;
    case OA_ADJUST_KWDS_TO_MATCH_SDT:
      result = OalAdjustKwdstoMatchSDT( current_node, src_interchange_format,
                                        dst_interchange_format);
    break;
  }
  return( result);
}
return(0);
}



/*****************************************************************************

  Routine:  OalProcessSDT

  Description:  This routine controls the actual transfer and translation of
                a buffer full of data read by the Stream Layer.  It advances
                a pointer through the buffer, while simultaneously traversing
                the SDT, and calls the transfer/conversion functions specified
                at each node until it reaches the end of the buffer.
                Upon exit, the current_node argument points to the ODL tree
                node (which has an SDT node attached) where the processing of 
                the next buffer of source data begins.  This may be the same as
                the previous current node or it may be a different node.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:  15 Apr   1997

  History:

    Creation - This routine was part of the Alpha Release of the OA library.
    11/29/95 - Fixed bug whereby source data was being stored in buf when
               conversion_type was OA_NOT_APPLICABLE.  Now being tossed out. SM
    04/15/97 - Changed size of internal buffer to 512 so that BIT_STRINGS of
               up to 512 bytes can be processed. SM

  Input:  
          source       - Points to the buffer of source data.

          source_bytes - The number of bytes of source data.

          current_node - The address of a pointer to the current ODL tree
                         node in the SDT.  Every node in the SDT MUST have an
                         SDT node attached.

  Output:  The function returns when it has processed source_bytes.
           *current_node points to the new current node, where the next source
           will start processing.  This may be the same as the previous current
           node, or it may be a new one.

  Notes:  

  1) The exact values in an SDT node and how OalProcessSDT uses them are
     detailed below.

     The first four SDT parameters are constants which were determined when
     the SDT was built:
    *current_node->appl1->src.start_offset gives the offset, in bytes, of the 
      start of this data atom from its parent in the input stream. 
    *current_node->appl1->src.size gives the number of bytes in a single
      repetition of the data atom in the input stream.  (The total size of the
      contiguous source data described by a SDT node is SDT_node->src.size * 
      SDT_node->total_repetitions.)  If src.size is zero, OalProcessSDT 
      advances the dst pointer dst.size bytes, but doesn't advance the source
      pointer.
    *current_node->appl1->conversion_type specifies the type of conversion to 
      be used in converting src bytes to dst bytes: OA_BINREP, 
      OA_ASCII_TO_BINARY, OA_BINARY_TO_ASCII or OA_MEMCPY.  OalProcessSDT 
      stores up bytes until it has src.size bytes before calling the conversion
      function, which then writes dst.size bytes.  The only exception to this 
      is OA_MEMCPY, which will copy however many bytes are available in the
      source (less than or equal to src.size).
    *current_node->appl1->src.PDS_data_type and dst.PDS_data_type are 
      enumerated data types, e.g. OA_IEEE_REAL
    *current_node->appl1->dst.ptr points to memory where the bytes at the 
      start of the current repetition of the current node go, or NULL if
      dst.size=0.  This is usually set by OalPositionToNextDataNode, but
      must be set by OalInitializeSDT for the first data node before the
      first call to OalProcessSDT.

  buf_bytes and buf are only used when the source ends in the middle of a data 
    atom (e.g. middle of an integer);  if current_node->appl1->buf_bytes > 0, 
    then current_node->appl1->buf contains bytes from the previous call which
    are be pre-pended to the current source.

  *current_node->appl1->src.bytes_processed gives the number of bytes in the
    current repetition that have been processed (i.e. sent to a conversion 
    function; it does NOT include buf_bytes).  This is dynamic;  it is zeroed 
    after each call to the conversion function, unless the conversion function 
    is OA_MEMCPY.  If the conversion function is OA_MEMCPY, bytes_processed is 
    the offset from dst.ptr in memory for the next dst bytes;  
    src.bytes_processed always wraps around to 0 after reaching src.size, in 
    order to set up for the next repetition.  dst.bytes_processed is never 
    used.

  2) Recommended uses for OalProcessSDT in Object Layer routines:
   - For file reads, the source is an I/O buffer full of data, and the 
     destination(s) are in dynamic memory.  
   - For in-memory conversions, both the source and destination are in dynamic 
     memory.  
   - For file writes, don't use an SDT; do an in-memory conversion first so the
     data in memory is exactly the way you want it in the file, then write the
     object to the file using OaWriteObject.

  3) OalProcessSDT deals with explicit spares in the source (src.size > 0,
     dst.size = 0, dst.ptr = NULL) by skipping over data in the source without 
     routing it anywhere.  Such explicit spares can be used to cause input data
     to be ignored (e.g. prefix or suffix bytes).

  4) OalProcessSDT may leave gaps in the destination data for alignment 
     padding, if specified in the SDT.  It deals with explicit spares where
     src.size = 0 and dst.size > 0 by skipping over bytes in the destination.

*****************************************************************************/

#ifdef _NO_PROTO

int OalProcessSDT( source, source_bytes, current_node)
PTR source;
long source_bytes;
ODLTREE *current_node;

#else

int OalProcessSDT( PTR source, long source_bytes, ODLTREE *current_node)

#endif
{

static char buf[512];  /* This buffer is pointed to by sdt_node->buf.  */
long remaining_source_bytes, remaining_bytes, i;
PTR source_ptr;
SDT_node *sdt_node_ptr;

sdt_node_ptr = (SDT_node *) (*current_node)->appl1;
remaining_source_bytes = source_bytes;
remaining_bytes = sdt_node_ptr->src.size - sdt_node_ptr->src.bytes_processed;
source_ptr = source;  
         
while (remaining_bytes == 0) {
  
  /* This happens whenever there's an explicit gap in the output data:
     src.size = 0, dst.size >= 0.  SDT's created by OA software never have
     such nodes; this code is here for robustness.  */

  if ((*current_node = OalPositionToNextDataNode( *current_node)) != NULL) {
    sdt_node_ptr = (SDT_node *) (*current_node)->appl1;
    remaining_bytes = sdt_node_ptr->src.size;
  } else return( OA_REACHED_END_OF_SDT);  /* No next node, so must have just 
                                             processed last node on tree 
                                             (root), so done! */
}

/* Note:  both control variables for the following while loop are changed
   within the loop.  */

while (remaining_source_bytes >= remaining_bytes) { 

  /* The slice contains as much or more data than remains to be processed for 
     a single repetition of the current SDT node, so call the conversion 
     function.  The conversion function can change the contents of the SDT 
     node if it wants (so far none of the conversion functions provided by
     OA do) but it can't change which node is the current node.
     Note: buf_bytes is used only for non-MEMCPY storage.  src.bytes_processed
     is used only for MEMCPY dst positioning; it is not used in non-MEMCPY
     conversion nodes; only src.bytes_processed is used, dst.bytes_processed
     is never used for anything.  */

  if (sdt_node_ptr->dst.size > 0) {
    if (sdt_node_ptr->conversion_type == OA_MEMCPY) {

#ifdef IBM_PC

      /* If we're on an IBM-PC, use a for loop to do the copy, since memcpy
         may not work for copies greater than 64K and/or over segment
         boundaries;  otherwise use memcpy since it's usually optimized.  */

      for (i=0; i<remaining_bytes; i++)
        sdt_node_ptr->dst.ptr[ sdt_node_ptr->src.bytes_processed + i] =
          source_ptr[i];
#else
      memcpy( sdt_node_ptr->dst.ptr + sdt_node_ptr->src.bytes_processed, 
              source_ptr, remaining_bytes);
#endif
    } else {
      if (sdt_node_ptr->buf_bytes > 0) {

        /* Data was in the local buffer from the previous call, so add the
           new data to it, then call the conversion function.  */

        sdt_node_ptr->buf = &(buf[0]);
        for (i=0; i<remaining_bytes; i++)
          sdt_node_ptr->buf[ sdt_node_ptr->buf_bytes + i] = source_ptr[i];
        sdt_node_ptr->src.ptr = (PTR) sdt_node_ptr->buf;
        OalConvert( sdt_node_ptr);
        sdt_node_ptr->buf_bytes = 0;

      } else {

        /* No data in local buffer, so call the conversion function.  */

        sdt_node_ptr->src.ptr = source_ptr;
        OalConvert( sdt_node_ptr);

      }  /* End else no data in buffer from previous call. */
    }    /* End else not MEMCPY.                           */
  }      /* End if dst specifies data (dst.size > 0).      */

  /* We've just finished processing a single repetition of the current
     SDT node, so advance the slice pointer and set up for the next
     repetition.  */

  source_ptr += remaining_bytes;              /* Advance source pointer by   */
                                              /* bytes just processed.       */
  remaining_source_bytes -= remaining_bytes;  /* Decrement bytes left to     */
                                              /* process in source.          */
  remaining_bytes = sdt_node_ptr->src.size;   /* Reset for next pass.        */
  sdt_node_ptr->src.bytes_processed = 0;      /* Set up for next repetition. */
  sdt_node_ptr->current_reps++;               /* Increment repetition count  */

  if (sdt_node_ptr->current_reps < sdt_node_ptr->total_repetitions) {

    /* Have repetitions to go in this node; advance the destination pointer 
       over the bytes just written, by dst.size bytes, the size of a single 
       repetition. */

    if (sdt_node_ptr->dst.ptr != NULL) 
      sdt_node_ptr->dst.ptr += sdt_node_ptr->dst.size;

    /* If there's more data in the source, continue back to start of the
       while loop; else return.  */

    if (remaining_source_bytes == 0) return( OA_READY_FOR_NEXT_SLICE);

  } else {

    /* No more repetitions in this node; reset it and go to the next node. */

    sdt_node_ptr->current_reps = 0;

    if ((*current_node = OalPositionToNextDataNode( *current_node)) != NULL) {

      sdt_node_ptr = (SDT_node *) (*current_node)->appl1;
      remaining_bytes = sdt_node_ptr->src.size;
      while (remaining_bytes == 0) {
  
        /* This happens whenever there's an explicit gap in the output data:
           src.size = 0, dst.size >= 0.  SDT's created by the OA Structure
           Layer never have such nodes; this code is here in case someone
           inputs their own custom-built SDT.  */

        if ((*current_node = OalPositionToNextDataNode( *current_node)) 
            != NULL) {
          sdt_node_ptr = (SDT_node *) (*current_node)->appl1;
          remaining_bytes = sdt_node_ptr->src.size;
        } else return( OA_REACHED_END_OF_SDT);  /* No next node, so must have 
                                                   processed last node on tree 
                                                   (root), so done! */
      }
    } else {

      return( OA_REACHED_END_OF_SDT);  /* No next node, so must have just 
                                          processed last node on tree 
                                          (root), so done! */
    }
  }  /* End of else block for no more repetitions in this node. */
}    /* End of while slice_size >= remaining_bytes loop.        */

if (remaining_source_bytes > 0) {

  /* Remaining_source_bytes is now less than remaining_bytes needed to finish a
     repetition of the current node.  
     - If conversion type is OA_MEMCPY, do the copy to dst.ptr.
     - If conversion type is OA_NOT_APPLICABLE, then the source data is being
       tossed out, so just update src.bytes_processed.
     - If conversion_type is anything else, save the remaining bytes in the
       local buffer.
  */

  switch( sdt_node_ptr->conversion_type) {

    case OA_MEMCPY: 

#ifdef IBM_PC

      /* If we're on an IBM-PC, use a for loop to do the copy, since memcpy
         may not work for copies greater than 64K and/or over segment
         boundaries;  otherwise use memcpy since it's usually optimized.  */

      for (i=0; i<remaining_source_bytes; i++)
        sdt_node_ptr->dst.ptr[ sdt_node_ptr->src.bytes_processed + i] =
          source_ptr[i];
#else
      memcpy( sdt_node_ptr->dst.ptr + sdt_node_ptr->src.bytes_processed, 
              source_ptr, remaining_source_bytes);
#endif
    break;

    case OA_NOT_APPLICABLE:
    break;

    default: /* Store in buf until have enough bytes for conversion funct. */
      sdt_node_ptr->buf = &(buf[0]);
      for (i=0; i<remaining_source_bytes; i++)
        sdt_node_ptr->buf[ sdt_node_ptr->buf_bytes + i] = source_ptr[i];
      sdt_node_ptr->buf_bytes += remaining_source_bytes;
  }
  sdt_node_ptr->src.bytes_processed += remaining_source_bytes;
}
return( OA_READY_FOR_NEXT_SLICE);
}




/*****************************************************************************

  Routine:  OalSDTtoODLTree

  Description:  This routine updates the ODL tree nodes' keywords to match 
                the destination data described by the SDT nodes, and strips
                off the SDT nodes.

  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   1 Sept  1994
  Last Modified:  13 Dec   1994

  History:

    Creation - This routine was part of the Alpha Release of the OA library.
    12/13/94 - Added code to undo setup done by OalCreateSDT for column-major
               TABLES. SM

  Input:  
          sdt                    - An ODL tree, with SDT nodes attached to
                                   some or all of its nodes.
                  
          dst_interchange_format - Set to OA_ASCII_INTERCHANGE_FORMAT or
                                   OA_BINARY_INTERCHANGE_FORMAT.

  Output:  If successful, the function returns 0, otherwise non-zero.
           If successful, sdt is an ODL tree without any SDT nodes, and its 
           keywords describe the destination data converted by SDT processing,
           as was described by the SDT nodes.  
           Example:  if the SDT was used to read data from a file into memory,
           then the keywords now describe the converted, in-memory data - if 
           the data file had VAX_INTEGERs, and we're running on a Sun, then
           the integers were converted to MSB_INTEGERs, and the DATA_TYPE
           keywords now say MSB_INTEGER.
           The keyword INTERCHANGE_FORMAT in the root node is set as indicated
           by dst_interchange_format.

  Notes:  

  1) OalSDTtoODLTree essentially does the reverse of OalCreateSDT, and is
     designed analogously:  the cludges for prefixes and suffixes are isolated 
     in the first section of code, so that the following code is clean.  
  2) The call to OalPostOrderTraverse with function OalAdjustKwdstoMatchSDT
     does object class specific keyword manipulation on each individual node, 
     and deletes nodes with no destination data (SDT_node->dst.size=0).

*****************************************************************************/

#ifdef _NO_PROTO

int OalSDTtoODLTree( sdt, dst_interchange_format)
ODLTREE sdt;
int dst_interchange_format;

#else

int OalSDTtoODLTree( ODLTREE sdt, int dst_interchange_format)

#endif
{

ODLTREE current_node;
SDT_node *sdt_node_ptr;
long rows, row_bytes, bytes, items, start_byte;
char *str;

/* Set the INTERCHANGE_FORMAT keyword in the root node according to the
   dst_interchange_format argument;  this is the interchange format of the
   data created by the most recent SDT processing.  If the keyword isn't
   already present, add it as the first keyword. */

if (dst_interchange_format == OA_ASCII_INTERCHANGE_FORMAT)
  str = "ASCII";
else
  str = "BINARY";
if (OdlFindKwd( sdt, "INTERCHANGE_FORMAT", "*", (unsigned long) 0,
               (unsigned short) ODL_THIS_OBJECT) == NULL) {
  OdlPasteKwdBefore( OdlNewKwd( "INTERCHANGE_FORMAT", str, NULL, NULL, NULL, 
                                (long) 0), OdlGetFirstKwd( sdt));
} else {
  OaStrtoKwdValue( "INTERCHANGE_FORMAT", sdt, str);
}
if ((OaGetObjectClass( sdt) == OA_IMAGE) && (LeftmostChild( sdt) != NULL)) {

  /* Undo the special prefix/suffix setup which was done by OalCreateSDT, by
     changing the SAMPLE_TYPE keyword in the IMAGE node to the image line
     node's SDT_node->dst.PDS_data_type, and deleting the image line node.
     The SDT looks like this:
     IMAGE------COLUMN (prefix SPARE)
             ---COLUMN (image line)
             ---COLUMN (suffix SPARE)
     OalAdjustKwdstoMatchSDT will delete the prefix and suffix nodes, since it
     deletes all nodes whose sdt_node_ptr->dst.size = 0.  */

  current_node = OdlFindObjDesc( sdt, "COLUMN", "NAME", "IMAGE_LINE",
                                 (unsigned long) 0, 
                                 (unsigned short) ODL_CHILDREN_ONLY);
  if (current_node != NULL) {
    sdt_node_ptr = (SDT_node *) current_node->appl1;
    if (sdt_node_ptr != NULL)
      OaStrtoKwdValue( "SAMPLE_TYPE", sdt,
                        OaPDSDataTypetoStr( sdt_node_ptr->dst.PDS_data_type));
    OalFreeSDT( OdlCutObjDesc( current_node));
    OalFreeSDTStruct( (SDT_node *) sdt->appl1);
    sdt->appl1 = NULL;
  }
}

/* Call OalPostOrderTraverse, specifying -1 for src_interchange_format, since
   OalAdjustKwdstoMatchSDT only cares about dst_interchange_format.  */

OalPostOrderTraverse( sdt, OA_ADJUST_KWDS_TO_MATCH_SDT, -1, 
                      dst_interchange_format);

/* If have a column-major TABLE, then undo the setup done by OalCreateSDT. */

if ((OaGetObjectClass( sdt) == OA_TABLE) && 
    (OdlFindKwd( sdt, "TABLE_STORAGE_TYPE", "COLUMN_MAJOR",
                (unsigned long) 0, (unsigned short) ODL_THIS_OBJECT) 
    != NULL)) {
  OaKwdValuetoLong( "OLD_ROWS", sdt, &rows);
  OaDeleteKwd( "OLD_ROWS", sdt);
  OaLongtoKwdValue( "ROWS", sdt, rows);
  OaKwdValuetoLong( "ROW_BYTES", sdt, &row_bytes);
  OaLongtoKwdValue( "ROW_BYTES", sdt, row_bytes / rows);
  current_node = LeftmostChild( sdt);
  while (current_node != NULL) {

    if (LeftSibling( current_node) != NULL) {
      OaKwdValuetoLong( "START_BYTE", current_node, &start_byte);
      start_byte = (start_byte - 1) / rows + 1;
      OaLongtoKwdValue( "START_BYTE", current_node, start_byte);
    }

    OaKwdValuetoLong( "BYTES", current_node, &bytes);
    OaKwdValuetoLong( "ITEMS", current_node, &items);
    bytes /= rows;
    items /= rows;
    OaLongtoKwdValue( "BYTES", current_node, bytes);
    OaLongtoKwdValue( "ITEMS", current_node, items);

    current_node = RightSibling( current_node);
  }  /* end while loop through all columns */
}  /* end if column-major TABLE */

return(0);
}

