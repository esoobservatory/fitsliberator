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
    CFITSIO. This file implements the class Engine::FitsImageCube.
	
	@version       $Revision: 1.8 $
	@date          $Date: 2010/09/25 15:32:59 $

	@author        Teis Johansen <teis@siet.dk>
	@author        Kaspar Kirstein Nielsen <kaspar@barmave.dk>
	@author        Lars Holm Nielsen <lars@hankat.dk> */

#include "FitsImageCube.hpp"
#include "FitsImageReader.hpp"
#include "Exception.h"

using std::string;
using std::ostream;

using FitsLiberator::Rectangle;
using FitsLiberator::Engine::ImageCube;
using FitsLiberator::Engine::FitsImageCube;
using FitsLiberator::Engine::ImageReader;

template<typename Value>
static inline int sign(Value number) {
	return (number < 0) ? -1 : 1;
}

FitsImageCube::FitsImageCube(unsigned int index, unsigned int nAxis, 
							 long* nAxes, int bitDepth, ImageReader* owner
	) : super(nAxes[0], nAxes[1], (nAxis >= 3) ? nAxes[2] : 1, 
			  Map(bitDepth), owner ) {

	this->dimensions = nAxis;
	this->index      = index;
}

ImageCube::PixelFormat
FitsImageCube::Map(int bitDepth) {
	switch(bitDepth) {
        case DOUBLE_IMG:
			return ImageCube::Float64;
        case FLOAT_IMG:
			return ImageCube::Float32;
        case BYTE_IMG:
			return ImageCube::Unsigned8;
        case SBYTE_IMG:
			return ImageCube::Signed8;
        case SHORT_IMG:
			return ImageCube::Signed16;
        case USHORT_IMG:
			return ImageCube::Unsigned16;
        case LONG_IMG:
			return ImageCube::Signed32;
        case ULONG_IMG:
			return ImageCube::Unsigned32;
        case LONGLONG_IMG:
			return ImageCube::Signed64;
		default:
			assert(false);
			return ImageCube::Unsigned8;
	}
}

bool
FitsImageCube::NeedsNullMap() const {
	PixelFormat format = Format();
	return !(format == Float64 || format == Float32);
}

unsigned int
FitsImageCube::Index() const {
	return index;
}

std::string
FitsImageCube::Property(const std::string& name) const {
	FitsImageReader* reader = dynamic_cast<FitsImageReader*>(Owner());
	assert(reader != 0);

	return reader->Property(this, name);
}

void
FitsImageCube::Properties(ostream& stream, const string& prefix) const {
	FitsImageReader* reader = dynamic_cast<FitsImageReader*>(Owner());
	assert(reader != 0);

	reader->Properties(this, stream, prefix);
}

ImageCube::RowOrdering
FitsImageCube::RowOrder() const {
	ImageCube::RowOrdering order = BottomUp;

	double cd[2][2] = {0.0};
	double cdelt[2] = {0.0};

	if( NumericProperty("CD1_1", &(cd[0][0])) &&
		NumericProperty("CD1_2", &(cd[0][1])) &&
		NumericProperty("CD2_1", &(cd[1][0])) &&
		NumericProperty("CD2_2", &(cd[1][1])) )
	{
		if( sign(cd[0][0]) != sign(cd[1][1]) &&
			sign(cd[0][1]) == sign(cd[1][0]) ) 
		{
			order = BottomUp;
		} else if( sign(cd[0][0]) == sign(cd[1][1]) 
			&& sign(cd[0][1]) != sign(cd[1][0]) )
		{
			order = TopDown;
		}
	} else if( NumericProperty("CDELT1", &(cdelt[0])) &&
		NumericProperty("CDELT2", &(cdelt[1])) )
	{
		if( sign(cdelt[0]) != sign(cdelt[1]) ) {
			order = BottomUp;
		} else if( sign(cdelt[0]) == sign(cdelt[1]) ) {
			order = TopDown;
		}
	}

	return order;
}

void
FitsImageCube::Read(ImageCube::size_type plane, const Rectangle& bounds, 
					void *buffer) const {
    FitsImageReader* reader = dynamic_cast<FitsImageReader*>(Owner());
	reader->Read(this, plane, bounds, buffer);
}

void 
FitsImageCube::Read(ImageCube::size_type plane, const Rectangle& bounds, 
					void* buffer, char* valid) const {

    FitsImageReader* reader = dynamic_cast<FitsImageReader*>(Owner());
	reader->Read(this, plane, bounds, buffer, valid);
}

void 
FitsImageCube::Read(ImageCube::size_type plane, void* buffer) const {
    FitsImageReader* reader = dynamic_cast<FitsImageReader*>(Owner());
	reader->Read(this, plane, buffer);
}

void
FitsImageCube::Read(ImageCube::size_type plane, 
                    void* buffer, char* valid) const {
    FitsImageReader* reader = dynamic_cast<FitsImageReader*>(Owner());
	reader->Read(this, plane, buffer, valid);
}
