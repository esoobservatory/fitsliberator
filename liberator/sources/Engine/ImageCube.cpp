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
    CFITSIO and pds_toolbox. This file defines the classes Engine::ImageFile
	and Engine::ImageCube.

	@version       $Revision: 1.6 $
	@date          $Date: 2010/09/25 15:32:59 $

	@author        Teis Johansen <teis@siet.dk>
	@author        Kaspar Kirstein Nielsen <kaspar@barmave.dk>
	@author        Lars Holm Nielsen <lars@hankat.dk> */

#include "ImageCube.hpp"
#include "TextUtils.h"

using std::string;

using FitsLiberator::Engine::ImageCube;
using FitsLiberator::Engine::ImageReader;

ImageCube::ImageCube(ImageCube::size_type width, ImageCube::size_type height, 
					 ImageCube::size_type planes, ImageCube::PixelFormat format,
					 ImageReader* owner
	) {
	this->width		= width;
	this->height	= height;
	this->planes	= planes;
	this->format		= format;
	this->owner     = owner;
}

ImageCube::~ImageCube() {

}

ImageCube::size_type 
ImageCube::Width() const {
	return this->width;
}

ImageCube::size_type 
ImageCube::Height() const {
	return this->height;
}

ImageCube::size_type 
ImageCube::Planes() const {
	return this->planes;
}

ImageCube::PixelFormat
ImageCube::Format() const {
	return this->format;
}

ImageCube::size_type
ImageCube::Pixels() const {
	return this->planes * this->width * this->height;
}

ImageCube::size_type
ImageCube::PixelsPerPlane() const {
	return this->width * this->height;
}

ImageCube::size_type
ImageCube::SizeOf(ImageCube::size_type width, 
                  ImageCube::size_type height) const {
	return SizeOf(this->format, width, height);
}

ImageCube::size_type
ImageCube::SizeOf(ImageCube::size_type plane) const {
	return SizeOf(width, height);
}

ImageCube::size_type
ImageCube::SizeOf() const {
	return planes * SizeOf(0);
}

ImageCube::size_type
ImageCube::SizeOf(ImageCube::PixelFormat format, 
				  ImageCube::size_type width, 
				  ImageCube::size_type height) {

	int bytes = width * height;
	switch(format) {
		case Float64:
		case Signed64:
			return 8 * bytes;
		case Float32:
		case Signed32:
		case Unsigned32:
			return 4 * bytes;
		case Unsigned8:
		case Signed8:
			return bytes;
		case Signed16:
		case Unsigned16:
			return 2 * bytes;
		default:
			return 0;
	}
}



ImageReader*
ImageCube::Owner() const {
	return this->owner;
}

bool
ImageCube::NumericProperty(const string& name, double *out) const {
	if(out != 0) {
		string value = this->Property(name);
		if(value.size() != 0) {
			*out = TextUtils::stringToDouble(value);
			return true;
		}
	}
	return false;
}