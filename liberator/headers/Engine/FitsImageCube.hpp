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
    CFITSIO. This file defines the class Engine::FitsImageCube.
*/

#ifndef __FITSIMAGECUBE_H__
#define __FITSIMAGECUBE_H__

#include "ImageCube.hpp"

namespace FitsLiberator {
    namespace Engine {
		/** Represents a FITS image. Note that not all images in a FITS file will be
			compatible with FITS Liberator. The ImageReader object has a logical (only
			valid images) view of the FITS file. The FitsImageCube uses the index member
			variable to map the logical index to the FITS HDU index. */
		class FitsImageCube : public ImageCube {
			typedef ImageCube super;

			int dimensions;		//< Number of dimensions in the HDU. Needed to
								//    ensure coordinates given to CFITSIO are
								//    given properly.
			unsigned int index;	//< FITS HDU index.
		public:
			/** Constructs a FitsImageCube object from the data extracted from the FITS header.
				@param index Index of the HDU.
				@param nAxis Number of dimensions in the image.
				@param nAxes Size of each dimension.
				@param bitDepth Bitdepth as reported by CFITSIO.
				@param owner Owner object. */
			FitsImageCube(unsigned int index, unsigned int nAxis, long* nAxes, int bitDepth, ImageReader* owner);
			/** Maps a CFITSIO bit depth to a PixelFormat value.
				@param bitDepth CFITSIO bit depth. 
				@returns A PixelFormat value. */
			static PixelFormat Map(int bitDepth);
			/** @see FitsLiberator::Engine::ImageCube::NeedsNullMap */
			bool NeedsNullMap() const;
			/** Returns the HDU index of this image. */
			unsigned int Index() const;
			/** @see FitsLiberator::Engine::ImageCube::Property */
			std::string Property(const std::string& name) const;
			/** @see FitsLiberator::Engine::ImageCube::Properties */
            void Properties(std::ostream& stream, const std::string& prefix) const;
			/** @see FitsLiberator::Engine::ImageCube::RowOrder. */
			ImageCube::RowOrdering RowOrder() const;
			/** @see FitsLiberator::Engine::ImageCube::Read. */
			void Read(ImageCube::size_type plane, const FitsLiberator::Rectangle& bounds, void* buffer) const;
			/** @see FitsLiberator::Engine::ImageCube::Read. */
			void Read(ImageCube::size_type plane, const FitsLiberator::Rectangle& bounds, void* buffer, char* valid) const;
			/** @see FitsLiberator::Engine::ImageCube::Read. */
			void Read(ImageCube::size_type plane, void* buffer) const;
			/** @see FitsLiberator::Engine::ImageCube::Read. */
			void Read(ImageCube::size_type plane, void* buffer, char* valid) const;
        };
    }
}

#endif	// __FITSIMAGECUBE_H__