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
    CFITSIO. This file defines the classes Engine::FitsImageReader
*/

#ifndef __FITSIMAGEREADER_H__
#define __FITSIMAGEREADER_H__

#include <iostream>
#include <fitsio.h>

#include "ImageReader.hpp"
#include "FitsImageCube.hpp"

namespace FitsLiberator {
	namespace Engine {
        class FitsImageReaderException : public ImageReaderException {
			typedef ImageReaderException super;

			int status;
			fitsfile* fileHandle;
		public:
			FitsImageReaderException(fitsfile* fileHandle, int status);
        };

        class FitsImageCube;

		class FitsImageReader : public ImageReader {
			typedef ImageReader super;

			fitsfile* fileHandle;	///< CFITSIO handle to the FITS file.

			/** Move the CFITSIO fileHandle to the HDU which contains the image
				at index. 
				@param image Image to select. */
			void SelectHDU(const FitsImageCube* image) const;
			/** Maps a ImageCube::PixelFormat to a CFITSIO datatype.
				@param format Value to map. */
			static int Map(ImageCube::PixelFormat format);
		public:
			FitsImageReader(const std::string& filename);
			virtual ~FitsImageReader();
			void Read(const FitsImageCube* image, ImageCube::size_type plane, const Rectangle& bounds, void* buffer);
			void Read(const FitsImageCube* image, ImageCube::size_type plane, const Rectangle& bounds, void* buffer, char* valid);
			void Read(const FitsImageCube* image, ImageCube::size_type plane, void* buffer);
			void Read(const FitsImageCube* image, ImageCube::size_type plane, void* buffer, char* valid);
            /** Reads WCS mapping information from a specific HDU. 
                @param image Image to read from. */
            bool ReadWCS(const FitsImageCube* image,
                         double* xrefval, double* yrefval, 
                         double* xrefpix, double* yrefpix, 
                         double* xinc,    double* yinc,
                         double* rot,
                         char*   type) const;
			/** Performs a fileformat check to see if the file can be read. 
                Even if this method returns true it does not mean the file 
                contains readable images.
                @param filename Path of the file to check.
                @return True if the file can be read, false if it cannot. */
            static bool CanRead(const std::string& filename);
			std::string Property(const FitsImageCube* image, const std::string& name) const;
            void Properties(const FitsImageCube* image, std::ostream& stream, const std::string& prefix) const;
            /** @see ImageReader::header. */
            virtual void header(std::ostream& stream) const;
            /** @see ImageReader::format. */
            virtual ImageReader::ImageFormat format() const;
		};
	}
}

#endif //__FITSIMAGEREADER_H__