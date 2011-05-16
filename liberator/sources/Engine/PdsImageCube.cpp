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
    Contains definitions for the wrapper layer above the 3rd party I/O library
    pds_toolbox. This file implements the class Engine::PdsImageCube.
	
	@version       $Revision: 1.13 $
	@date          $Date: 2010/09/25 15:32:59 $

	@author        Teis Johansen <teis@siet.dk>
	@author        Kaspar Kirstein Nielsen <kaspar@barmave.dk>
	@author        Lars Holm Nielsen <lars@hankat.dk> */

#include <sstream>
#include "PdsImageCube.hpp"
#include "ImageReader.hpp"
#include "Text.hpp"

using std::string;
using std::ostream;

using FitsLiberator::Rectangle;
using FitsLiberator::newline;
using FitsLiberator::Engine::ImageReader;
using FitsLiberator::Engine::ImageReaderException;
using FitsLiberator::Engine::ImageCube;
using FitsLiberator::Engine::PdsImageCube;

PdsImageCube::PdsImageCube(OBJDESC *imageNode, 
	unsigned int width, unsigned int height, unsigned int planes, 
	ImageCube::PixelFormat format,
	ImageReader* owner
	) : super(width, height, planes, format, owner) {
		
	  this->imageNode = imageNode;
}

PdsImageCube*
PdsImageCube::FromObjectDescription(ImageReader* owner, OBJDESC *imageNode) {
	long  lines;				// LINES = height
	long  line_samples;			// LINE_SAMPLE = width
	long  sample_bits;			// SAMPLE_BITS = bits/pixel
	char* sample_type_str;		// SAMPLE_TYPE = datatype
	long  bands;				// BANDS = planes
	int   band_storage_type;	// ??
	long  line_prefix_bytes;
	long  line_suffix_bytes;
	int   encoding_type;

	if( !OaGetImageKeywords(imageNode, &lines, &line_samples, &sample_bits,
		&sample_type_str, &bands, &band_storage_type, &line_prefix_bytes, &line_suffix_bytes,
		&encoding_type) ) {

        // PDS supports many different encodings. We only support the ones offered by the PDS library.
        if( encoding_type == OA_UNCOMPRESSED || encoding_type == OA_HUFFMAN_FIRST_DIFFERENCE || encoding_type == OA_PREVIOUS_PIXEL ) {
            int sample_format = OaGetObjectInterchangeFormat(imageNode);

		    // TODO: Verify the datatype
            ImageCube::PixelFormat format = MapDataType(sample_bits, sample_format, sample_type_str);
            if(format != ImageCube::Invalid) {
			    return new PdsImageCube(imageNode, 
				    line_samples, lines, bands, format,
				    owner);
		    }
        }
	}
	return NULL;
}

ImageCube::PixelFormat
PdsImageCube::MapDataType(long sample_bits, int sample_format, char *sample_type_str) {
    ImageCube::PixelFormat format = ImageCube::Invalid;
    int datatype = OaStrtoPDSDataType(sample_type_str, sample_format);
    switch(datatype) {
        case OA_REAL:
            if(sample_bits == 32) {
                format = ImageCube::Float32;
            } else if(sample_bits == 64) {
                format = ImageCube::Float64;
            }
            break;
        case OA_LSB_INTEGER:
            if(sample_bits == 8) {
                format = ImageCube::Signed8;
            } else if(sample_bits == 16) {
                format = ImageCube::Signed16;
            } else if(sample_bits == 32) {
                format = ImageCube::Signed32;
            } else if(sample_bits == 64) {
                format = ImageCube::Signed64;
            }
            break;
        case OA_LSB_UNSIGNED_INTEGER:
            if(sample_bits == 8) {
                format = ImageCube::Unsigned8;
            } else if(sample_bits == 16) {
                format = ImageCube::Unsigned16;
            } else if(sample_bits == 32) {
                format = ImageCube::Unsigned32;
            }
            break;
        case OA_INTEGER:
            if(sample_bits == 8) {
                format = ImageCube::Signed8;
            } else if(sample_bits == 16) {
                format = ImageCube::Signed16;
            } else if(sample_bits == 32) {
                format = ImageCube::Signed32;
            } else if(sample_bits == 64) {
                format = ImageCube::Signed64;
            }
            break;
        case OA_MSB_UNSIGNED_INTEGER:
            if(sample_bits == 8) {
                format = ImageCube::Unsigned8;
            } else if(sample_bits == 16) {
                format = ImageCube::Unsigned16;
            } else if(sample_bits == 32) {
                format = ImageCube::Unsigned32;
            }
            break;
        case OA_PC_REAL:
            if(sample_bits == 32) {
                format = ImageCube::Float32;
            } else if(sample_bits == 64) {
                format = ImageCube::Float64;
            }
            break;
    }

    return format;
}

bool
PdsImageCube::NeedsNullMap() const {
	return false;
}

ODLTREE
PdsImageCube::Node() const {
	return this->imageNode;
}

string
PdsImageCube::Property(const string& name) const {
	string value;

    OBJDESC* node = imageNode;
    while(node != 0) {
	    KEYWORD* keyword = OdlFindKwd(node, 
		    const_cast<char*>(name.c_str()), "*", 0, ODL_THIS_OBJECT);

	    if( keyword != 0 ) {
		    value = keyword->value;
            break;
	    }
        node = node->parent;
    }
	return value;
}

void
PdsImageCube::Properties(ostream& stream, const string& prefix) const {
	KEYWORD* keyword = OdlGetFirstKwd(imageNode);
	while(keyword != 0) {
        stream << prefix << keyword->name << "\t= " << keyword->value << newline;
		keyword = OdlGetNextKwd(keyword);
	}
}

ImageCube::RowOrdering
PdsImageCube::RowOrder() const {
	// The PDS standard states that all images are represented top-down.
    return ImageCube::TopDown;
}

void 
PdsImageCube::Read(ImageCube::size_type plane, 
				   const Rectangle& bounds, void* buffer) const {
    assert(buffer != 0);
	assert(plane < Planes());
	assert(bounds.left >= 0 && bounds.top >= 0 && 
	    bounds.right <= Width() && bounds.bottom <= Height() );	

	OA_OBJECT handle = OaOpenImage(imageNode, plane);
    if(handle == 0) throw ImageReaderException(Owner()->FileName());
	OA_OBJECT block  = OaReadPartialImage(handle, bounds.top + 1, 
		bounds.bottom, bounds.left + 1, bounds.right);
    if(block == 0) throw ImageReaderException(Owner()->FileName());

	unsigned char* dst = reinterpret_cast<unsigned char*>(buffer);
	unsigned char* src = reinterpret_cast<unsigned char*>(block->data_ptr);
	size_t bytes = SizeOf(bounds.right - bounds.left, 
		bounds.bottom - bounds.top);
	std::copy(src, src + bytes, dst);
    
	OaDeleteObject(block);
	OaCloseImage(handle);
}

void
PdsImageCube::Read(ImageCube::size_type plane, void* buffer) const {
    assert(buffer != 0);

	OA_OBJECT block = OaReadImage(Node(), plane);
    
    unsigned char* dst = reinterpret_cast<unsigned char*>(buffer);
    unsigned char* src = reinterpret_cast<unsigned char*>(block->data_ptr);
    size_t bytes = SizeOf(0);
    std::copy(src, src + bytes, dst);
    
    OaDeleteObject(block);
}

void
PdsImageCube::Read(ImageCube::size_type plane, 
                   const Rectangle& bounds, void* buffer, char* valid) const {
	Read(plane, bounds, buffer);
	//is used for making sure the null map is zero
    std::fill( valid, valid + bounds.getArea(), 0 );
}

void
PdsImageCube::Read(ImageCube::size_type plane, 
                   void* buffer, char* valid) const {
	Read(plane, buffer);
	//is used for making sure the null map is zero
    std::fill( valid, valid + PixelsPerPlane(), 0 );
}
