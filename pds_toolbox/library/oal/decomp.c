/*****************************************************************************

  File:  decomp.c

  Description: This file contains routines used in Huffman First Difference
               decompression of images.

               The routines in this file are:

               new_node
               sort_freq
               OalCreateHFDTree
               OalHFDDecompress
               OalFreeHFDTree

  Author: Kris Becker/USGS Flagstaff
  Packaged and tweeked for the OA Library: Steve Monk/LASP

  Creation Date:   17 Feb   1988
  Last Modified:   18 Mar   1996

  History:

    Creation - Most of this code was hastily incorporated into the Alpha 
               Release of the OA library with different function names and
               code structure than it does now.
    03/02/95 - Cleaned up for Beta Release: renamed functions to the OAL
               naming convention, removed wrappers for Fortran callers, 
               changed documentation header format, changed function return 
               values so as to hide the NODE structure, added #ifdefs for
               the IBM-PC, and made a few other minor changes while porting
               to all the platforms OAL runs on.  SM
    12/11/95 - Replaced printf's by OaReportError.
             - Included "oamalloc.h" and replaced 'malloc' by 'OaMalloc' and
               'free' by 'OaFree'.  SM
             - Added error codes.  SM
    03/18/96 - Fixed bug in OalCreateHFDTree. SM/Mike Martin

*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "oamalloc.h"

extern char error_string[];    
extern int oa_errno;

/****************************************************************************
node_def defines the basic element used to build the Huffman tree for data
decompression.  The *right and *left pointers point to another NODE
structure. The dn field will contain a -1 if the node is not a leaf (or end
node) otherwise it will contain the pixel difference value.  This value is
then subtracted from the preceding pixel and 256 is added to normalize the
actual first difference value.  The left and right pointers are undefined
(NULL) if the node is a leaf, otherwise they will point to the next adjacent
node in the tree. 
****************************************************************************/

typedef struct leaf {
  struct leaf *right;
  short int dn;
  struct leaf *left;
} NODE;


/* These are prototypes for functions in this file.  */

#ifdef _NO_PROTO
int OaReportError();
NODE *new_node();
void sort_freq();
void *OalCreateHFDTree();

/*void OalHFDDecompress();*/
int OalHFDDecompress();

void OalFreeHFDTree();
#else
int OaReportError( char *input_error_string);
NODE *new_node( short int value);
#if (defined( __MSDOS__) || defined( MSDOS) || defined( IBM_PC))
void sort_freq( long *freq_list, NODE **node_list, int num_freq);
#else
void sort_freq( int *freq_list, NODE **node_list, int num_freq);
#endif
void *OalCreateHFDTree( void *input_histogram);


/*void OalHFDDecompress( char  *ibuf, char  *obuf, long  *nin, long  *nout,
                       void  *input_root);*/
int OalHFDDecompress( char  *ibuf, char  *obuf, long  *nin, long  *nout,
                       void  *input_root);


void OalFreeHFDTree( void *input_root);
#endif


/*****************************************************************************

  Routine:  new_node

  Description:  This routine allocates a NODE structure and returns a pointer
                 to it.
 
  Author:  Kris Becker/USGC Flagstaff
 
  Creation Date:  17 Feb  1988
  Last Modified:   2 Mar  1995

  History:

    Creation - See header at top of file.

  Input:  
          value - The value to put in the dn field of the node.

  Output: A new HFD tree node.

  Notes:  

*****************************************************************************/

#ifdef _NO_PROTO
NODE *new_node( value)
short int value;

#else
NODE *new_node( short int value)

#endif
{
    NODE *temp;         /* Pointer to the memory block */

/***************************************************************************
  Allocate the memory and initialize the fields.
****************************************************************************/

  temp = (NODE *) OaMalloc((long) sizeof(NODE));

  if (temp != NULL) 
    {
      temp->right = NULL;
      temp->dn = value;
      temp->left = NULL;
    }
  else
    {
       oa_errno = 720;
       OaReportError("OalCreateHFDTree: new_node out of memory!");
       exit(1);
    }

   return( temp);
  }



/*****************************************************************************

  Routine:  sort_freq

  Description:  Sorts frequency list (a step in building the HFD tree).
 
  Author:  Kris Becker/USGC Flagstaff
 
  Creation Date:  17 Feb  1988
  Last Modified:   2 Mar  1995

  History:

    Creation - See header at top of file.

  Input:  
          freq_list - Pointer to frequency list (HFD encoding histogram).

          node_list - Pointer to array of node pointers.

          num_freq  - Number of values in freq list.

  Output: 

  Notes:  
  Algorithm:
  This routine uses an insertion sort to reorder a frequency list in order of 
  increasing frequency.  The corresponding elements of the node list are
  reordered to maintain correspondence.  The node list is actually a pointer to
  an array of pointers to tree nodes.

*****************************************************************************/

#ifdef _NO_PROTO
void sort_freq( freq_list, node_list, num_freq)
#if (defined( __MSDOS__) || defined( MSDOS) || defined( IBM_PC))
long *freq_list;
#else
int *freq_list;
#endif
NODE **node_list;
int num_freq;

#else
#if (defined( __MSDOS__) || defined( MSDOS) || defined( IBM_PC))
void sort_freq( long *freq_list, NODE **node_list, int num_freq)
#else
void sort_freq( int *freq_list, NODE **node_list, int num_freq)
#endif

#endif
{

    /* Local Variables */

#if (defined( __MSDOS__) || defined( MSDOS) || defined( IBM_PC))
    register long     *i;       /* primary pointer into freq_list */
    register long     *j;       /* secondary pointer into freq_list */
    
    register NODE **k;          /* primary pointer to node_list */
    register NODE **l;          /* secondary pointer into node_list */

    long     temp1;             /* temporary storage for freq_list */
    NODE *temp2;                /* temporary storage for node_list */

    register long     cnt;      /* count of list elements */
#else
    register int      *i;       /* primary pointer into freq_list */
    register int      *j;       /* secondary pointer into freq_list */
    
    register NODE **k;          /* primary pointer to node_list */
    register NODE **l;          /* secondary pointer into node_list */

    int      temp1;             /* temporary storage for freq_list */
    NODE *temp2;                /* temporary storage for node_list */

    register int      cnt;      /* count of list elements */
#endif

/************************************************************************
  Save the current element - starting with the second - in temporary
  storage.  Compare with all elements in first part of list moving 
  each up one element until the element is larger.  Insert current 
  element at this point in list.
*************************************************************************/

   if (num_freq <= 0) return;      /* If no elements or invalid, return */

   for (i=freq_list, k=node_list, cnt=num_freq ; --cnt ; *j=temp1, *l=temp2)
     {
        temp1 = *(++i);
        temp2 = *(++k);

        for (j = i, l = k ;  *(j-1) > temp1 ; )
          {
            *j = *(j-1);
            *l = *(l-1);
            j--;
            l--;
            if ( j <= freq_list) break;
          }
 
     }
  return;
  }



/*****************************************************************************

  Routine:  OalCreateHFDTree

  Description:  OalCreateHFDTree constructs the Huffman tree and returns
                a pointer to the root of the tree.
 
  Author:  Kris Becker/USGC Flagstaff
 
  Creation Date:  17 Feb  1988
  Last Modified:  18 Mar  1996

  History:

    Creation - See header at top of file.
    03/18/96 - Fixed bug in array assignment loop which was accessing 512'th
               item in the input histogram, which only has 511 elements. SM/MM

  Input:  

  Output: 

  Notes:  The algorithm uses an array of node pointers allocated for all
           possible values.  This array is then initialized by assigning all
           leafs to the array.  Each leaf has a cooresponding frequency 
           assigned to it and the frequencies are sorted in ascending order. 
           All zero frequencies are ignored and tree construction begins.  
           The tree is built by combining the two least occuring frequencies
           into one node.  This new node is treated as one by adding together
           the two frequencies forming a cummulative frequency of the
           combining nodes.  The second smallest node now contains the newly
           combined node and the smallest node is deleted from the list.  The
           frequency list is then resorted to determine the next two node
           combinations until one node is left.  This node will be the root
           of the tree.  This pointer is then returned to the calling routine.
           The histogram integers are assumed to be in native format 4-byte
           integer format on entry.

*****************************************************************************/

#ifdef _NO_PROTO
void *OalCreateHFDTree( input_histogram)
void *input_histogram;

#else
void *OalCreateHFDTree( void *input_histogram)

#endif
{

  /*  Local variables used */
#if (defined( __MSDOS__) || defined( MSDOS) || defined( IBM_PC))
    long     *hist = (long *) input_histogram;
    long     freq_list[512];     /* Histogram frequency list, 4 bytes */
    register long    *fp;        /* Frequency list pointer */
#else
    int      *hist = (int *) input_histogram;
    int      freq_list[512];      /* Histogram frequency list, 4 bytes */
    register int      *fp;        /* Frequency list pointer */
#endif

    register int    num_freq;   /* Number non-zero frequencies in histogram */

    NODE **node_list;             /* DN pointer array list */
    
    register NODE **np;           /* Node list pointer */

    register short int num_nodes; /* Counter for DN initialization */
    register short int cnt;       /* Miscellaneous counter */

    short int znull = -1;         /* Null node value */
    
    register NODE *temp;          /* Temporary node pointer */

/***************************************************************************
  Allocate the array of nodes from memory and initialize these with numbers
  corresponding with the frequency list.  There are only 511 possible
  permutations of first difference histograms.  There are 512 allocated 
  here to adhere to the FORTRAN version.
****************************************************************************/

   fp = freq_list;
   node_list = (NODE **) OaMalloc((long) sizeof(temp)*512);
   if (node_list == NULL)
    {
      oa_errno = 720;
      OaReportError("OalCreateHFDTree: out of memory!");
      exit(1);
    }
   np = node_list;

   for (num_nodes=1, cnt=512 ; cnt-- ; num_nodes++)
     {

/* Now make the assignment */
        if (cnt > 0) 
          *fp++ = *hist++;
        temp = new_node(num_nodes);
        *np++ = temp;
     }

     *fp = 0;         /* Ensure the last element is zeroed out.  */

/***************************************************************************
  Now, sort the frequency list and eliminate all frequencies of zero.
****************************************************************************/

  num_freq = 512;
  sort_freq(freq_list,node_list,num_freq);

  fp = freq_list;
  np = node_list;

  for (num_freq=512 ; (*fp) == 0 && (num_freq) ; fp++, num_freq--) np++;


/***************************************************************************
  Now create the tree.  Note that if there is only one difference value,
  it is returned as the root.  On each interation, a new node is created
  and the least frequently occurring difference is assigned to the right
  pointer and the next least frequency to the left pointer.  The node
  assigned to the left pointer now becomes the combination of the two
  nodes and it's frequency is the sum of the two combining nodes.
****************************************************************************/

  for (temp=(*np) ; (num_freq--) > 1 ; )
    {
        temp = new_node(znull);
        temp->right = (*np++);
        temp->left = (*np);
        *np = temp;
        *(fp+1) = *(fp+1) + *fp;
        *fp++ = 0;
        sort_freq(fp,np,num_freq);
    }
  
  return( (void *) temp);
 }



/*****************************************************************************

  Routine:  OalHFDDecompress

  Description:  This routine decompresses a Huffman coded compressed image
                 line.
 
  Author:  Kris Becker/USGC Flagstaff
 
  Creation Date:  17 Feb  1988
  Last Modified:   2 Mar  1995

  History:

    Creation - See header at top of file.

  Input:  
          ibuf       - Pointer to compressed data buffer.

          obuf       - Pointer to output buffer for decompressed image line.

          nin        - Number of bytes in input buffer.

          nout       - Number of bytes in output buffer (unused input).

          input_root - Huffman coded tree used to decompress image lines.

  Output: The output buffer contains the decompressed image line.

  Returns 1 if successfull, or a 0 if not successfull

  Notes:
  Algorithm: It follows a path from the root of the Huffman tree to one of
  it's leaves.  The choice at each branch is decided by the successive bits
  of the compressed input stream.  Left for 1, right  for 0.  Only leaf nodes
  have a value other than -1.  The routine traces a path through the tree
  until it finds a node with a value not equal to -1 (a leaf node).  The
  value at the leaf node is subtracted from the preceeding pixel value plus
  256 to restore the uncompressed pixel.  This is then repeated until the
  entire line has been processed.

  Changes:
  13-04-00  DWS changed void to int in function definition and added
  return values, also changed prototypes at beginning of file and in oal.h
*****************************************************************************/

#ifdef _NO_PROTO
/*void OalHFDDecompress( ibuf, obuf, nin, nout, input_root)*/
int OalHFDDecompress( ibuf, obuf, nin, nout, input_root)
char  *ibuf;
char  *obuf;
long  *nin;
long  *nout;
void  *input_root;

#else
/*void OalHFDDecompress( char  *ibuf, char  *obuf, long  *nin, long  *nout,
                       void  *input_root)*/

int OalHFDDecompress( char  *ibuf, char  *obuf, long  *nin, long  *nout,
                       void  *input_root)
#endif
{


    /* Local Variables */
    NODE *root = (NODE *) input_root;
    register NODE *ptr = (NODE *) input_root;

    register unsigned char test;      /* test byte for bit set */
    register unsigned char idn;       /* input compressed byte */

    register char odn;                /* last dn value decompressed */

    char *ilim = ibuf + *nin;         /* end of compressed bytes */
    char *olim = obuf + *nout;        /* end of output buffer */


/**************************************************************************
  Check for valid input values for nin, nout and make initial assignments.
***************************************************************************/

    if (ilim > ibuf && olim > obuf)
       odn = *obuf++ = *ibuf++;
    else
       {
           oa_errno = 502;
           OaReportError("OalHFDDecompress: invalid inputs!");
/*           exit(1);*/
		   return 0;
       }

/**************************************************************************
  Decompress the input buffer.  Assign the first byte to the working
  variable, idn.  An arithmatic and (&) is performed using the variable
  'test' that is bit shifted to the right.  If the result is 0, then
  go to right else go to left.
***************************************************************************/

    for (idn=(*ibuf) ; ibuf < ilim  ; idn =(*++ibuf))
     {
        for (test=0x80 ; test ; test >>= 1)
           {
            ptr = (test & idn) ? ptr->left : ptr->right;

            if (ptr->dn != -1) 
              {
                if (obuf >= olim) return 1;
                odn -= ptr->dn + 256;
                *obuf++ = odn;
                ptr = root;
              }
          }
     }
   return 1;
  }



/*****************************************************************************

  Routine:  OalFreeHFDTree

  Description:  This routine frees the HFD encoding tree.
 
  Author:  Steve Monk, University of Colorado LASP
 
  Creation Date:   2 Mar  1995
  Last Modified:   2 Mar  1995

  History:

    Creation - This routine was part of the Beta release of the OA Library.

  Input:  

  Output: 

  Notes:  

  This routine frees each node of the HFD decoding tree, which were allocated
  in dynamic memory by OalCreateHFDTree.  It uses a post-order traverse to
  visit and free each node.  An example of a post-order traverse is:

                    5 (root, input node)
                   / \                                                    
                  3   4
                 / \ 
                1   2

*****************************************************************************/

#ifdef _NO_PROTO
void OalFreeHFDTree( input_root)
void *input_root;

#else
void OalFreeHFDTree( void *input_root)

#endif
{

NODE *root = (NODE *) input_root;
NODE *next_node, *right_sibling;

if (root != NULL) {
  next_node = root->left;
  while (next_node != NULL) {

    /* Find the right sibling of next_node now and save it, because next_node
       will soon be freed.  */

    right_sibling = next_node->right;
    OalFreeHFDTree( next_node);
    next_node = right_sibling;
  }
  OaFree( (char *) root);
}
}
