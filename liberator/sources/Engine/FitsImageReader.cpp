// The ESA/ESO/NASA FITS Liberator - http://code.google.com/p/fitsliberator
//
// Copyright (c) 2004-2010, ESA/ESO/NASA.
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the names of the European Space Agency (ESA), the European 
//       Southern Observatory (ESO) and the National Aeronautics and Space 
//       Administration (NASA) nor the names of its contributors may be used to
//       endorse or promote products derived from this software without specific
//       prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
// ARE DISCLAIMED. IN NO EVENT SHALL ESA/ESO/NASA BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// =============================================================================
//
// The ESA/ESO/NASA FITS Liberator uses NASA's CFITSIO library, libtiff, 
// TinyXML, Boost C++ Libraries, Object Access Library and Intel Threading 
// Building Blocks.
//
// =============================================================================
//
// Project Executive:
//   Lars Lindberg Christensen
//
// Technical Project Manager:
//   Lars Holm Nielsen
//
// Developers:
//   Kaspar Kirstein Nielsen & Teis Johansen
// 
// Technical, scientific support and testing: 
//   Robert Hurt
//   Davide De Martin
//
// =============================================================================

/** @file
    Contains definitions for the wrapper layer above the 3rd party I/O libraries
    CFITSIO and pds_toolbox. This file implements the class 
	Engine::FitsImageReader.
	
	@version       $Revision: 1.16 $
	@date          $Date: 2010/09/25 15:32:59 $

	@author        Teis Johansen <teis@siet.dk>
	@author        Kaspar Kirstein Nielsen <kaspar@barmave.dk>
	@author        Lars Holm Nielsen <lars@hankat.dk> */

#include <iostream>
#include "FitsImageReader.hpp"
#include "Text.hpp"

using std::string;
using std::ostream;

using FitsLiberator::newline;
using FitsLiberator::Rectangle;

using FitsLiberator::Engine::ImageReaderException;
using FitsLiberator::Engine::FitsImageReaderException;

using FitsLiberator::Engine::ImageReader;
using FitsLiberator::Engine::ImageCube;
using FitsLiberator::Engine::FitsImageReader;
using FitsLiberator::Engine::FitsImageCube;

FitsImageReaderException::FitsImageReaderException(fitsfile *fileHandle, 
												   int status)
  : super( fileHandle->Fptr->filename ) {
	this->fileHandle = fileHandle;
	this->status = status;
}

FitsImageReader::FitsImageReader(const string& filename)
  : super(filename) {
	
    int status = 0;

    int nAxis;
    long nAxes[4];
    int bitDepth;

    int hduCount;
    int hduType;

    if( fits_open_diskfile(&fileHandle, filename.c_str(), READONLY, &status) )
        throw ImageReaderException(filename);

    // Get the number of HDUs in the file
    if( fits_get_num_hdus(fileHandle, &hduCount, &status) )
        throw FitsImageReaderException(fileHandle, status);

    for( unsigned int i = 1; i <= hduCount; i++ ) {
        hduType = ANY_HDU;
        if( fits_movabs_hdu(fileHandle, i, &hduType, &status) )
			throw FitsImageReaderException(fileHandle, status);

        if( IMAGE_HDU == hduType && !fits_get_img_dim(fileHandle, &nAxis, &status) ) {
            // Make sure we don't do an address violation.
            if( nAxis <= 4 ) {
                // Get the size of the image
                if( fits_get_img_size(fileHandle, 4, nAxes, &status) )
                    throw FitsImageReaderException(fileHandle, status);

                // Only accept images with NAXIS < 4 or the special case NAXIS = 4 and NAXIS4 = 1
                if( nAxis > 1 && ((nAxis == 4 && nAxes[3] == 1) || nAxis < 4) ) {
                    // Get the bit depth, using this method we don't have to check the 
                    // values of BSCALE & BZERO
                    if( fits_get_img_equivtype(fileHandle, &bitDepth, &status) )
                        throw FitsImageReaderException(fileHandle, status);
                    
                    insert(new FitsImageCube(i, nAxis, nAxes, bitDepth, this));
                }
            }
        }
    }
}

FitsImageReader::~FitsImageReader() {
	int status = 0;
	if( NULL != fileHandle )
		fits_close_file(fileHandle, &status);
}

void
FitsImageReader::SelectHDU(const FitsImageCube* image) const {
	FitsImageReader* self = dynamic_cast<FitsImageReader*>(image->Owner());
	assert(self != NULL && self == this);
	assert(image != 0);

	int dummy  = ANY_HDU;
	int status = 0;

	if( fits_movabs_hdu(fileHandle, image->Index(), &dummy, &status) )
		throw FitsImageReaderException(fileHandle, status);
}

int
FitsImageReader::Map(ImageCube::PixelFormat format) {
    switch(format) {
		case ImageCube::Float64:
			return TDOUBLE;
		case ImageCube::Float32:
			return TFLOAT;
		case ImageCube::Unsigned8:
			return TBYTE;
		case ImageCube::Signed8:
			return TSBYTE;
		case ImageCube::Signed16:
			return TSHORT;
		case ImageCube::Unsigned16:
			return TUSHORT;
		case ImageCube::Signed32:
			return TLONG;
		case ImageCube::Unsigned32:
			return TULONG;
		case ImageCube::Signed64:
			return TLONGLONG;
		default:
			assert(false);
			return 0;
    }
}

void
FitsImageReader::Read(const FitsImageCube* image, ImageCube::size_type plane, 
					  const Rectangle& bounds, void* buffer) {
    if(bounds.getArea() == image->PixelsPerPlane()) {
        Read(image, plane, buffer);
    } else {
        assert(buffer != 0);
        assert(plane < image->Planes());
        assert(bounds.left >= 0 && bounds.top >= 0 
            && bounds.right <= image->Width() && bounds.bottom <= image->Height());

        long first[4] = {bounds.left + 1, bounds.top + 1, plane + 1, 1};
        long last[4]  = {bounds.right, bounds.bottom, plane + 1, 1};
	    int dataType  = Map(image->Format());
        long inc      = 0;
        int status    = 0;

	    SelectHDU(image);
        if( fits_read_subset(fileHandle, dataType, first, last, &inc, 
            NULL, buffer, NULL, &status) )
		    throw FitsImageReaderException(fileHandle, status);
    }
}

void 
FitsImageReader::Read(const FitsImageCube* image, ImageCube::size_type plane, 
					  const Rectangle& bounds, void* buffer, char* valid) {
    if( bounds.getArea() == image->PixelsPerPlane() ) {
        Read(image, plane, buffer, valid);
    } else {
        assert(buffer != 0);
        assert(valid != 0);
        assert(plane < image->Planes());
        assert(bounds.left >= 0 && bounds.top >= 0 
            && bounds.right <= image->Width() && bounds.bottom <= image->Height());


	    int dataType   = Map(image->Format());
	    int anyNull    = 0;

	    int status     = 0;

	    int increment  = 0;
		long long n = 0;
		//if the asked for width is equal to the image's width then
		//we use the height of the bounds as increment
		if ( image->Width() == bounds.getWidth() )
		{
			increment = image->SizeOf( bounds.getWidth(), bounds.getHeight() );
			n = bounds.getArea();
		}
		else
		{
			increment = image->SizeOf(bounds.getWidth(), 1);
			n = bounds.getWidth();
		}
	    char* begin    = reinterpret_cast<char*>(buffer);
	    char* end      = begin + image->SizeOf(bounds.getWidth(), bounds.getHeight());

            

	    SelectHDU(image);

	    long topLeft[4] = {bounds.left + 1, bounds.top + 1, plane + 1, 1};
        while(begin != end) {
            if( fits_read_pixnull(fileHandle, dataType, topLeft, n, begin, valid, &anyNull, &status) )
                throw FitsImageReaderException(fileHandle, status);
            begin      += increment;
            valid      += n;
            topLeft[1] += 1;
        }
    }
}

void 
FitsImageReader::Read(const FitsImageCube* image, ImageCube::size_type plane, 
					  void* buffer) {
	assert(buffer != 0);
    assert(plane < image->Planes());

	long topLeft[4] = {1, 1, plane+1, 1};
	long long maxPixels  = image->Width() * image->Height();
	
	int dataType   = Map(image->Format());
	int anyNull;

	int status     = 0;

	SelectHDU(image);
	if( fits_read_pix(fileHandle, dataType, topLeft, maxPixels, 0, buffer, &anyNull, &status) )
		throw FitsImageReaderException(fileHandle, status);	
}

void 
FitsImageReader::Read(const FitsImageCube* image, ImageCube::size_type plane, 
					  void* buffer, char* valid) {
	assert(buffer != 0);
    assert(valid != 0);
    assert(plane < image->Planes());

	long topLeft[4] = {1, 1, plane+1, 1};
	long long maxPixels  = image->Width() * image->Height();
	
	int dataType   = Map(image->Format());
	int anyNull;
	
	int status     = 0;

	SelectHDU(image);
	if( fits_read_pixnull(fileHandle, dataType, topLeft, maxPixels, buffer, valid, &anyNull, &status) )
		throw FitsImageReaderException(fileHandle, status);
}

bool
FitsImageReader::ReadWCS(const FitsImageCube* image, 
                         double* xrefval, double* yrefval, 
                         double* xrefpix, double* yrefpix, 
                         double* xinc,    double* yinc,
                         double* rot,
                         char*   type) const {

	int status = 0;

    SelectHDU(image);
	fits_read_img_coord(fileHandle, 
        xrefval, yrefval, 
        xrefpix, yrefpix, 
        xinc, yinc, 
        rot, 
        type,
        &status);

    return !(*xrefval == 0.0 && *yrefval == 0.0 
        && *xrefpix == 0.0 && *yrefpix == 0.0
        && *xinc == 1.0 && *yinc == 1.0 && *rot == 0.0 );
}

bool
FitsImageReader::CanRead(const string& filename) {
    int status = 0;
    fitsfile* handle = 0;

    if(fits_open_diskfile(&handle, filename.c_str(), READONLY, &status))
        return false;   // cfitsio returns 1 on failure
    return true;
}

string
FitsImageReader::Property(const FitsImageCube* image, const string& name) const {
	string header;

	int status = 0;
	char value[81] = {0};

	SelectHDU(image);
	if(VALUE_UNDEFINED != fits_read_key(fileHandle, TSTRING, const_cast<char*>(name.c_str()), value, NULL, &status)) {
		header = value;
	}

	return header;
}

void
FitsImageReader::Properties(const FitsImageCube* image, ostream& stream, const string& prefix) const {
	int          status		 = 0;
	int          recordCount = 0;
	char         record[81]	 = {0};

	SelectHDU(image);
	if(fits_get_hdrspace(fileHandle, &recordCount, NULL, &status))
		throw FitsImageReaderException(fileHandle, status);

	for(int i = 1; i <= recordCount; ++i) {
		if(fits_read_record(fileHandle, i, record, &status))
			throw FitsImageReaderException(fileHandle, status);
		stream << prefix << record << newline;
	}
}

void
FitsImageReader::header(std::ostream& stream) const {
    int image = 1;
	for(const_iterator i = begin(); i != end(); ++i) {
        stream << "Image #" << image << newline;
        (*i)->Properties(stream, INDENT);
		stream << newline;
        image += 1;
	}    
}

ImageReader::ImageFormat
FitsImageReader::format() const {
    return ImageReader::FITS;
}