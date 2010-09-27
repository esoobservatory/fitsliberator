/*****************************************************************************

  File:  clmdcmp2.c

  Description: This file contains part of the source code for the Clementine
               JPEG decompression software.  The file clmdcmp1.c contains the
               rest.  The code was originally in several separate files, and
               has been packaged into these two files with as few changes as
               possible.  This file contains the following:

               OA prototypes, #defines and #includes
               pds.c
               bitstrm.c

  Author:  Dr. Erick Malaret, ACT Corp.
  Packaged and tweeked for the OA Library:  Steve Monk, University of Colorado
                                            LASP  Nov 30, 1995
 
  Creation Date:  30 Nov   1995
  Last Modified:  26 Nov   1996

  History:

    Creation - All of these routines except OalClementineJPEGDecompress were
               created at ACT Corp, and modified by SM as follows Nov 30, 1995
               for the OA Library:
               - Replaced #ifdef sun by little/big-endian identification code.
               - Replaced 'malloc' and 'free' by 'OaMalloc' and 'OaFree'.
               - Commented out writing parameters to qparm - "paramtrs.dat".
    11/26/96 - Added return statement to ByteStream_read.  SM
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#ifndef TRUE 
#define TRUE	1
#define FALSE	0
#endif

#include "stream_l.h"
#ifdef _NO_PROTO
int OaReportError();
#else
int OaReportError( char *input_error_string);
#endif
extern char error_string[];    
extern int oa_errno;

#include "jpeg_c.h"

/* These are prototypes for functions in this file.  */

#ifdef _NO_PROTO
void init_qt();
void inithuffcode();
void readhufftbls();
void pds_decomp();
#else
void init_qt( FILE *fptr);
void inithuffcode( void);
void readhufftbls( FILE *fptr);
void pds_decomp( FILE *fptr, CHARH *p, long sizej, long sizei);
#endif
/*extern FILE *qparm;*/

/* Use of these global variable replaces #ifdef sun in the code.  SM */

short endian_indicator=1;       /* Initialized to 1 to determine the value */
                                /* of *platform_is_little_endian below.    */
char *platform_is_little_endian = (char *) &endian_indicator;
/* If platform is little-endian (lsb first), then the 1 initialized in
   endian_indicator will be in the first byte, and *platform_is_little_endian
   will have the value TRUE (1), otherwise it will be FALSE (0).  */


/***************************** File: pds.c ******************************/
/*
 *	THIS ROUTINE IS PART OF THE CLEMENTINE PDS FILE READER PROGRAM.
 *	IT WAS WRITTEN BY ACT CORP. IN DIRECT SUPPORT TO THE
 *	CLEMENTINE (DSPSE) PROGRAM.
 *
 *	IF YOU FIND A PROBLEM OR MAKE ANY CHANGES TO THIS CODE PLEASE CONTACT
 *	Dr. Erick Malaret at ACT Corp.
 *			tel: (703) 742-0294
 *			     (703) 683-7431
 *                       email:	nrlvax.nrl.navy.mil
 *
 *
 *      Oct 31, 1994 Tracie Sucharski, USGS, Flagstaff  Change so that the
 *                      carriage returns are not removed from the labels.
 *      Sep 29, 1995 Tracie Sucharski, Added a fix to init_qt as given
 *                      by Luiz Perez from ACT. 
 */
#include <stdio.h>
#include <math.h>
/*#ifdef __TURBOC__*/
/*#include <alloc.h>*/
/*#else*/
/*#include <malloc.h>*/
/*#endif*/
#include <string.h>
/*#include "jpeg_c.h"*/
/*#include "pds.h"*/

/*#define READ_PARAM "rb"*/
/*#define WRITE_PARAM "wb"*/

/*PDSINFO	pds;*/
/*FILE	*qparm;*/
extern long		*DCTHist[64];
extern float	*Rn[64];
extern float	Q[64];

#ifdef _NO_PROTO
void init_qt();
void readhufftbls();
void pds_decomp();
void PClong2SUNlongVector();
void PCshort2SUNshortVector();
void getDCTHist();
void getRn();
#else
void init_qt(FILE *fptr);
void readhufftbls(FILE *fptr);
void pds_decomp(FILE *fptr,CHARH *p,long sizej,long sizei);
void PClong2SUNlongVector(unsigned long invec[],int npts) ;
void PCshort2SUNshortVector(unsigned short invec[],int npts) ;
void getDCTHist(BitStream *bs, long rows, long cols);
void getRn(void);
#endif

#ifdef DUH  /* This function is not needed in the OA Library.  */
PDSINFO *PDSR(char *fname, long *rows, long *cols)
{
	FILE    *fptr;
	int     n;
	int     j,i;
	CHARH   *c;
	char    nstring[84],sdummy[80],buffer[50],low;
	long    sizej;                  /* number of rows in the image */
	long    sizei;                  /* number of columns in the image */
	int     bitpix;                 /* bits per pixel */
	int     record_bytes;
	int     hist_rec,brw_rec,image_rec,ns,brwsize;
	char    cval, *sptr, *ptr;
	char    record_type[20];
	int     COMPRESSED;
	long    k,hdr_size;

	if( (fptr = fopen(fname,READ_PARAM)) == NULL){  /* open disk file */
		printf("Can't open %s.\n",fname);
		return NULL;
	}

	/* initialize some basic variables */
	bitpix			= 0;
	sizej=sizei		= 0;
	pds.browse_nrows	= 0;
	pds.browse_ncols	= 0;
	pds.image_nrows	= 0;
	pds.image_ncols	= 0;
	hist_rec = brw_rec = image_rec = -1;

	/* read header */
	do{
		/* read next line of text */
		for (n=0; (cval=fgetc(fptr)); n++) {
			nstring[n]=cval;
			if(cval=='\n') {
				if( (cval=fgetc(fptr)) != '\r') ungetc(cval,fptr);
				nstring[++n]='\0';
				break;
			}
		}

		/* find line's first non-space character */
		for (ns=0; nstring[ns]==' ';ns++);
		sptr = &nstring[ns];

		if (strncmp("^IMAGE_HISTOGRAM ",sptr,17)==0) {
			n=sscanf(nstring,"%s = %d", sdummy, &hist_rec);
		}

		if (strncmp("^BROWSE_IMAGE ",sptr,14)==0) {
			n=sscanf(nstring,"%s = %d", sdummy, &brw_rec);
		}

		if (strncmp("^IMAGE ",sptr,7)==0) {
			n=sscanf(nstring,"%s = %d", sdummy, &image_rec);
		}

		if (strncmp("RECORD_TYPE",sptr,9)==0) {
			n=sscanf(nstring,"%s = %s", sdummy, &record_type);
		}

		if (strncmp("RECORD_BYTES",sptr,12)==0) {
			n=sscanf(nstring,"%s = %d", sdummy, &record_bytes);
		}

		if (strncmp("ENCODING_TYPE",sptr,13)==0) {
			n=sscanf(nstring,"%s = \"%[^\"]\"", sdummy, buffer);
			if ( strstr(buffer,"N/A") )
				COMPRESSED = 0;
			else if ( strstr(buffer,"DECOMPRESSED") )
				COMPRESSED = 0;
			else
				COMPRESSED = 1;
		}

		if (strncmp("LINES ",sptr,6)==0) {
			n=sscanf(nstring,"%s = %d", sdummy, &sizej);
		}

		if (strncmp("LINE_SAMPLES",sptr,12)==0) {
			n=sscanf(nstring,"%s = %d", sdummy, &sizei);
		}

		if (strncmp("SAMPLE_BITS",sptr,11)==0) {
			n=sscanf(nstring,"%s = %d", sdummy, &bitpix);
		}

		if (strncmp("END",sptr,3)==0) {
			if ( *(sptr+3)=='\n' || *(sptr+3)==' ' || *(sptr+3)=='\r' ) {
				hdr_size = ftell(fptr);
				break;
			}
		}
	} while(1);

	/**************   read histogram  ***************/
	if ( hist_rec != -1 ) {
		fseek(fptr,hist_rec-1,0);
		pds.hist = (long *)malloc(256*sizeof(long));
		if(pds.hist == NULL) {
			printf(" histogram memory not allocated \n");
		}
		if ( pds.hist ) {
			fread(pds.hist,sizeof(long),256,fptr);
			if (*platform_is_little_endian == FALSE)
                          PClong2SUNlongVector((unsigned long*)pds.hist,256);
		}
	}

	/**************   read browse image **********/
	if ( brw_rec != -1 ) {
		pds.browse_ncols	= sizei/8;
		pds.browse_nrows	= sizej/8;
		fseek(fptr,brw_rec-1,0);
		brwsize = (sizej/8) * (sizei/8);
		pds.brw_imag = (unsigned char *)malloc(brwsize);
		if ( pds.brw_imag )
			fread(pds.brw_imag,sizeof(char),brwsize,fptr);
	}

	/*************   read image data ***************/
	if (strncmp(record_type,"UNDEFINED",9)==0) {
		record_bytes=1;
		fseek(fptr,(image_rec-1),0);
	} else {
		fseek(fptr,(image_rec-1)*record_bytes,0);
	}

	switch (bitpix) {
	case 8:
		c = (CHARH *)MALLOC(sizej*sizei);
		if ( c == NULL ) {
			printf("Can't allocate memory for image array.\n");
			fclose(fptr);
			return NULL;
		}

		if ( COMPRESSED ) {
			qparm = fopen("paramtrs.dat","w");
			init_qt(fptr);
			readhufftbls(fptr);
			pds_decomp(fptr,c,sizej,sizei);
			fclose(qparm);
		} else {

			for (j=0, k=0; j<sizej ;j++) {
				for (i=0; i<sizei; i++) {
					if ( 1 != fread(&low,sizeof(char),1,fptr) ) {
						printf("error: possible EOF found at (%d,%d)\n",j,i);
						break;
					} else {
						c[k++] = (unsigned char)low;
					}
				}
			}
		}

		pds.image = c;
		break;
	default:
		printf("invalid number of bits per pixel\n");
		pds.image = NULL;
	}

	/************    Allocate string buffer    **************/
	rewind(fptr);

/* TLS Added one byte for string termination and do not remove CR */
	pds.text = (char *)malloc(hdr_size+1);
	if ( pds.text ) {
		for (ptr=pds.text,i=0; i<hdr_size; i++) {
			fread(ptr,sizeof(char),1,fptr);
		/*	if ( *ptr == '\r' ) {
				 do nothing 
			} else {
				ptr++;
			}
                */
		ptr++;
		}
	}
	*(ptr)='\0';

	/*****/
	fclose(fptr);


	*rows = pds.image_nrows=sizej;
	*cols = pds.image_ncols=sizei;

	return &pds;
}
#endif /* #ifdef DUH */

/******** Routines that deal with compressed images *******/
float   dfac[8] = {
0.35355339,0.35355339,0.653281482,0.27059805,
0.449988111,0.254897789,0.300672443,1.281457724
};

#ifdef _NO_PROTO
void init_qt( fptr)
FILE *fptr;
#else
void init_qt(FILE *fptr)
#endif
{
	short   i;
	unsigned short   scalef;
	unsigned short   table[64];
	float   ftable[64];

	FREAD(&scalef,sizeof(short),1,fptr);
	if (*platform_is_little_endian == FALSE)
          PCshort2SUNshortVector((unsigned short*)&scalef,1) ;
	/*fprintf(qparm,"tabf: %d\n",scalef);*/
	FREAD(table,sizeof(short),64,fptr);
	if (*platform_is_little_endian == FALSE)
  	  PCshort2SUNshortVector((unsigned short*)table,64);
	/*fprintf(qparm,"tabq:\n");*/

	for (i=0; i<64; i++) {
	        table[i] = table[i]&0x00ff;    /*  TLS 9-29-95  */
		/*fprintf(qparm,"%3d ",table[i]);*/
		/*if ( (i+1) % 8 == 0 ) fprintf(qparm,"\n");*/

		ftable[i] = ( (float)scalef*(float)table[i] )/64.0 + 0.5;
		ftable[i] = 4096.0 / (float)floor(ftable[i]);
	}

	for (i=0; i<64; i++) Q[i] = ftable[zzseq[i]];

	ftable[0] = dfac[0]*dfac[0]*ftable[0];
	ftable[32] = dfac[0]*dfac[1]*ftable[32];
	ftable[16] = dfac[0]*dfac[2]*ftable[16];
	ftable[48] = dfac[0]*dfac[3]*ftable[48];
	ftable[8] = -dfac[0]*dfac[4]*ftable[8];
	ftable[24] = -dfac[0]*dfac[5]*ftable[24];
	ftable[56] = dfac[0]*dfac[6]*ftable[56];
	ftable[40] = -dfac[0]*dfac[7]*ftable[40];

	ftable[4] = dfac[1]*dfac[0]*ftable[4];
	ftable[36] = dfac[1]*dfac[1]*ftable[36];
	ftable[20] = dfac[1]*dfac[2]*ftable[20];
	ftable[52] = dfac[1]*dfac[3]*ftable[52];
	ftable[12] = -dfac[1]*dfac[4]*ftable[12];
	ftable[28] = -dfac[1]*dfac[5]*ftable[28];
	ftable[60] = dfac[1]*dfac[6]*ftable[60];
	ftable[44] = -dfac[1]*dfac[7]*ftable[44];

	ftable[2] = dfac[2]*dfac[0]*ftable[2];
	ftable[34] = dfac[2]*dfac[1]*ftable[34];
	ftable[18] = dfac[2]*dfac[2]*ftable[18];
	ftable[50] = dfac[2]*dfac[3]*ftable[50];
	ftable[10] = -dfac[2]*dfac[4]*ftable[10];
	ftable[26] = -dfac[2]*dfac[5]*ftable[26];
	ftable[58] = dfac[2]*dfac[6]*ftable[58];
	ftable[42] = -dfac[2]*dfac[7]*ftable[42];

	ftable[6] = dfac[3]*dfac[0]*ftable[6];
	ftable[38] = dfac[3]*dfac[1]*ftable[38];
	ftable[22] = dfac[3]*dfac[2]*ftable[22];
	ftable[54] = dfac[3]*dfac[3]*ftable[54];
	ftable[14] = -dfac[3]*dfac[4]*ftable[14];
	ftable[30] = -dfac[3]*dfac[5]*ftable[30];
	ftable[62] = dfac[3]*dfac[6]*ftable[62];
	ftable[46] = -dfac[3]*dfac[7]*ftable[46];

	ftable[1] = -dfac[4]*dfac[0]*ftable[1];
	ftable[33] = -dfac[4]*dfac[1]*ftable[33];
	ftable[17] = -dfac[4]*dfac[2]*ftable[17];
	ftable[49] = -dfac[4]*dfac[3]*ftable[49];
	ftable[9] = dfac[4]*dfac[4]*ftable[9];
	ftable[25] = dfac[4]*dfac[5]*ftable[25];
	ftable[57] = -dfac[4]*dfac[6]*ftable[57];
	ftable[41] = dfac[4]*dfac[7]*ftable[41];

	ftable[3] = -dfac[5]*dfac[0]*ftable[3];
	ftable[35] = -dfac[5]*dfac[1]*ftable[35];
	ftable[19] = -dfac[5]*dfac[2]*ftable[19];
	ftable[51] = -dfac[5]*dfac[3]*ftable[51];
	ftable[11] = dfac[5]*dfac[4]*ftable[11];
	ftable[27] = dfac[5]*dfac[5]*ftable[27];
	ftable[59] = -dfac[5]*dfac[6]*ftable[59];
	ftable[43] = dfac[5]*dfac[7]*ftable[43];

	ftable[7] = dfac[6]*dfac[0]*ftable[7];
	ftable[39] = dfac[6]*dfac[1]*ftable[39];
	ftable[23] = dfac[6]*dfac[2]*ftable[23];
	ftable[55] = dfac[6]*dfac[3]*ftable[55];
	ftable[15] = -dfac[6]*dfac[4]*ftable[15];
	ftable[31] = -dfac[6]*dfac[5]*ftable[31];
	ftable[63] = dfac[6]*dfac[6]*ftable[63];
	ftable[47] = -dfac[6]*dfac[7]*ftable[47];

	ftable[5] = -dfac[7]*dfac[0]*ftable[5];
	ftable[37] = -dfac[7]*dfac[1]*ftable[37];
	ftable[21] = -dfac[7]*dfac[2]*ftable[21];
	ftable[53] = -dfac[7]*dfac[3]*ftable[53];
	ftable[13] = dfac[7]*dfac[4]*ftable[13];
	ftable[29] = dfac[7]*dfac[5]*ftable[29];
	ftable[61] = -dfac[7]*dfac[6]*ftable[61];
	ftable[45] = dfac[7]*dfac[7]*ftable[45];

	for (i=0; i<64; i++) qtable[i] = ftable[zzseq[i]];
}

#ifdef _NO_PROTO
void readhufftbls( fptr)
FILE *fptr;
#else
void readhufftbls(FILE *fptr)
#endif
{
	FREAD(dcbits,sizeof(short),16,fptr);
        if (*platform_is_little_endian == FALSE)
  	  PCshort2SUNshortVector((unsigned short*)dcbits,16);

	FREAD(dchuffval,sizeof(char),12,fptr);
	FREAD(acbits,sizeof(short),16,fptr);
        if (*platform_is_little_endian == FALSE)
	  PCshort2SUNshortVector((unsigned short*)acbits,16);

	FREAD(achuffval,sizeof(char),162,fptr);

	inithuffcode();
}


#ifdef _NO_PROTO
void pds_decomp( fptr, p, sizej, sizei)
FILE *fptr;
CHARH *p;
long sizej;
long sizei;
#else
void pds_decomp(FILE *fptr,CHARH *p,long sizej,long sizei)
#endif
{
	BitStream       ibs;
	short   i, npanels;
	long    nbytes,bytesperpanel;
	short   blocks, rem;
	unsigned short  nb;
	long    filepos1, filepos2;
	CHARH   *ptr;
	int     FLAG = 0;

	cBitStream(&ibs,NULL,INPUT);

	filepos1 = ftell(fptr);
	fseek(fptr,0,2);
	filepos2 = ftell(fptr);
	fseek(fptr,filepos1,0);

	nbytes = filepos2 - filepos1;

	ibs.outstring = (CHARH *)MALLOC(nbytes);
        if (ibs.outstring == NULL) {
          oa_errno = 720;
          OaReportError( "clmdcmp: OaMalloc failed! Out of memory!");
          exit(1);
        }
	if ( ibs.outstring ) {
		blocks = 1;
		rem = 0;
		nb = (unsigned short)nbytes;
		if ( nbytes > 60000L ) {
			blocks = (short) nbytes / 32768;
			rem = (short) nbytes % 32768;
			nb = 32768;
		};
		ptr = ibs.outstring;
		for (i=0; i < blocks; i++,ptr+=nb) {
			if ( FREAD(ptr,sizeof(char),nb,fptr) != nb ) {
				printf("Error reading data string.\n");
				FLAG = 1;
				break;
			}
		}
		if ( rem ) {
			if ( (short)FREAD(ptr,sizeof(char),rem,fptr) != rem ) {
				printf("Error reading data string.\n");
				FLAG = 1;
			}
		}
		ibs.mode = MEMORY;
	} else {
		ibs.bytestream.file = fptr;
		ibs.mode = DISK;
	}

	if ( !FLAG ) {
		npanels = (short) sizej/32;
		bytesperpanel = 32*sizei;

		/* Allocate memory for DCT coefficients histograms */
		for (i=0; i<64; i++) {
/*			DCTHist[i] = (long FAR *)MALLOC(sizeof(long)*513); */
			DCTHist[i] = (long *)MALLOC(sizeof(long)*513);
                        if (DCTHist[i] == NULL) {
                          oa_errno = 720;
                          OaReportError( 
                            "clmdcmp: OaMalloc failed!  Out of memory!");
                          exit(1);
                        }
			memset((void *)DCTHist[i],0,sizeof(long)*513);
			DCTHist[i] += 256;
		}
		/* Allocate memory for DCT coefficients table look-up */
		for (i=0; i<64; i++) {
/*			Rn[i] = (float FAR *)MALLOC(sizeof(float)*513); */
			Rn[i] = (float *)MALLOC(sizeof(float)*513);
                        if (Rn[i] == NULL) {
                          oa_errno = 720;
                          OaReportError( 
                            "clmdcmp: OaMalloc failed! Out of memory!");
                          exit(1);
                        }
			memset((void *)Rn[i],0,sizeof(float)*513);
			Rn[i] += 256;
		}

		for (i=0; i<npanels; i++)
			/* get DCT coeffcients histogram */
			getDCTHist(&ibs,32,sizei);

		/* fill in table look-up */
		getRn();

		ibs.BitBuffer = 0;
		ibs.bytesout = 0;
		ibs.BitBuffMask = 0x00;

		/* do decompression w/ DCT coefficients optimization */
		for (i=0, ptr=p; i<npanels; i++,ptr+=bytesperpanel) {
			decomp(&ibs,ptr,32,sizei);
		}

		for (i=0; i<64; i++) {
			DCTHist[i] -= 256;
			Rn[i] -= 256;
			FREE(DCTHist[i]);
			FREE(Rn[i]);
		}
	}

	if ( ibs.outstring ) FREE( ibs.outstring );
}


#ifdef _NO_PROTO
void PClong2SUNlongVector( invec, npts)
unsigned long invec[];
int npts;
#else
void PClong2SUNlongVector(unsigned long invec[],int npts)
#endif
{
	int	i;
	unsigned long 	ival,oval;

	for(i=0;i<npts;i++){
		ival	= invec[i];
		oval	= ((ival&0x000000ff)<<24) +
			  ((ival&0x0000ff00)<<8) +
			  ((ival&0x00ff0000)>>8) +
			  ((ival&0xff000000)>>24);
		invec[i]= oval;
	}
}

#ifdef _NO_PROTO
void PCshort2SUNshortVector( invec, npts)
unsigned short invec[];
int npts;
#else
void PCshort2SUNshortVector(unsigned short invec[],int npts)
#endif
{
	int	i;
	unsigned short 	ival,oval;

	for(i=0;i<npts;i++) {
		ival	= invec[i];
		oval	= (ival<<8) + ((ival>>8)&0x00ff);
		invec[i]= oval;
	}
}

/************************* End File: pds.c ******************************/


/************************* File: bitstrm.c ******************************/
/*
 *	THIS ROUTINE IS PART OF THE CLEMENTINE PDS FILE READER PROGRAM.
 *	IT WAS WRITTEN BY ACT CORP. IN DIRECT SUPPORT TO THE 
 *	CLEMENTINE (DSPSE) PROGRAM.
 *
 *	IF YOU FIND A PROBLEM OR MAKE ANY CHANGES TO THIS CODE PLEASE CONTACT
 *	Dr. Erick Malaret at ACT Corp. 
 *			tel: (703) 742-0294
 *			     (703) 683-7431
 *                       email:	nrlvax.nrl.navy.mil
 *	
 *
 */
/*#include <stdio.h>*/
/*#include "jpeg_c.h"*/

#ifdef _NO_PROTO
void cBitStream( bs, fn, fm)
BitStream *bs;
char *fn;
Fmode fm;
#else
void cBitStream( BitStream *bs, char *fn, Fmode fm )
#endif
{
	cByteStream( &bs->bytestream, fn, fm );
	bs->BitBuffer = 0;
	bs->bytesout = 0;
	bs->outstring = NULL;
	bs->BitBuffMask =  (fm==OUTPUT) ? 0x80:0x00;
	bs->bitmask[0] = 0x0000; bs->bitmask[1] = 0x0001; bs->bitmask[2] = 0x0002;
	bs->bitmask[3] = 0x0004; bs->bitmask[4] = 0x0008; bs->bitmask[5] = 0x0010;
	bs->bitmask[6] = 0x0020; bs->bitmask[7] = 0x0040; bs->bitmask[8] = 0x0080;
	bs->bitmask[9] = 0x0100; bs->bitmask[10] = 0x0200; bs->bitmask[11] = 0x0400;
	bs->bitmask[12] = 0x0800; bs->bitmask[13] = 0x1000; bs->bitmask[14] = 0x2000;
	bs->bitmask[15] = 0x4000; bs->bitmask[16] = 0x8000;
}

#ifdef _NO_PROTO
void dBitStream( bs)
BitStream *bs;
#else
void dBitStream(BitStream *bs)
#endif
{
	if ( bs->bytestream.mode == OUTPUT ) {
		if ( bs->BitBuffMask != 0x80 ) {
			while (bs->BitBuffMask) {
				bs->BitBuffer |= bs->BitBuffMask;
				bs->BitBuffMask >>= 1;
			}
			if ( bs->mode )
				bs->outstring[bs->bytesout] = (CHARH)bs->BitBuffer;
			else
				ByteStream_write( &bs->bytestream, bs->BitBuffer );
			bs->bytesout++;
		}
		if ( bs->mode ) {
			if ( fwrite(bs->outstring,sizeof(char),bs->bytesout,bs->bytestream.file) == 0 )
				printf("Error: writing output bitstream to file.\n");
		}
		if ( fseek(bs->bytestream.file,8,0) )
			printf("Error: fseek in subroutine dBitStream().\n");
		else {
			if ( fwrite(&(bs->bytesout),sizeof(long),1,bs->bytestream.file) == 0 )
				printf("Error: writing bytesout value to file.\n");
		}
	}
	dByteStream(&bs->bytestream);
}

#ifdef _NO_PROTO
short BitStream_write( bs, bits, width)
BitStream *bs;
short bits;
short width;
#else
short BitStream_write(BitStream *bs, short bits, short width)
#endif
{
	unsigned short BitMask = bs->bitmask[width];

	while ( BitMask ) {
		if ( bits & BitMask ) bs->BitBuffer |= (short)bs->BitBuffMask;
		BitMask >>= 1;
		bs->BitBuffMask >>= 1;
		if ( !bs->BitBuffMask ) {
			if ( bs->mode )
				bs->outstring[bs->bytesout] = (CHARH)bs->BitBuffer;
			else
				ByteStream_write( &bs->bytestream, bs->BitBuffer );
			bs->bytesout++;
			bs->BitBuffer = 0;
			bs->BitBuffMask = 0x80;
		}
	}
	return bs->bytestream.stat;
}

#ifdef _NO_PROTO
short BitStream_read( bs, w)
BitStream *bs;
short w;
#else
short BitStream_read( BitStream *bs, short w )
#endif
{
	unsigned short RetVal = 0, BitMask = bs->bitmask[w];

	while ( BitMask ) {
		if ( !bs->BitBuffMask ) {
			if ( bs->mode ) {
				bs->BitBuffer = ((short)bs->outstring[bs->bytesout]) & 0x00ff;
			} else
				bs->BitBuffer = ByteStream_read(&bs->bytestream);

			bs->bytesout++;
			bs->BitBuffMask = 0x80;
		}
		if ( bs->BitBuffer & bs->BitBuffMask ) RetVal |= BitMask;
		bs->BitBuffMask >>= 1;
		BitMask >>= 1;
	}
	return RetVal;
}

#ifdef _NO_PROTO
void cByteStream( Bs, FileName, FileMode)
ByteStream *Bs;
char *FileName;
Fmode FileMode;
#else
void cByteStream(ByteStream *Bs, char *FileName, Fmode FileMode)
#endif
{
	Bs->mode = FileMode;
	if ( FileName != NULL ) {
		Bs->file = fopen(FileName,(Bs->mode==INPUT) ? "rb":"wb");
		if ( Bs->file == NULL ) printf("ByteStream constructor error.\n");
		Bs->stat = 0;
	}
}

#ifdef _NO_PROTO
void dByteStream( Bs)
ByteStream *Bs;
#else
void dByteStream(ByteStream *Bs)
#endif
{
	if ( Bs->file ) {
		fclose(Bs->file);
		Bs->file = NULL;
	}
}

#ifdef _NO_PROTO
short ByteStream_read( Bs)
ByteStream *Bs;
#else
short ByteStream_read(ByteStream *Bs)
#endif
{
	short     c;
	char	cval;
        int	n;

	if ( Bs->mode == INPUT ) {
		/* c = fgetc( Bs->file ); */
		n=FREAD(&cval,sizeof(char),1,Bs->file);
                c	= cval; 
		if ( n != 1 ) Bs->stat = EOF;
		return c;
	} else
		Bs->stat = EOF;
return 0;  /* Added 11/26/96 SM/LASP to get rid of compiler warning. */
}

#ifdef _NO_PROTO
short ByteStream_write( Bs, c)
ByteStream *Bs;
short c;
#else
short ByteStream_write(ByteStream *Bs, short c)
#endif
{
	if ( (Bs->mode != OUTPUT) || (fputc(c,Bs->file) == EOF) ) Bs->stat = EOF;
	return Bs->stat;
}

#ifdef _NO_PROTO
short ByteStream_status( Bs)
ByteStream *Bs;
#else
short ByteStream_status(ByteStream *Bs)
#endif
{
	return Bs->stat;
}
/************************* End File: bitstrm.c ******************************/


