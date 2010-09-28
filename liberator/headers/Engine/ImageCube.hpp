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
    CFITSIO and pds_toolbox. This file defines the class Engine::ImageCube
*/

#ifndef __IMAGECUBE_H__
#define __IMAGECUBE_H__

#include <string>
#include <iostream>

#include "Image.h"

namespace FitsLiberator {
    namespace Engine {
		class ImageReader;

		/** Represents a 3D image cube. This image cube may contain one or more image
			planes. */
        class ImageCube {
		public:
			typedef unsigned int size_type;

			/** Defines the size and type of the pixels .*/
			enum PixelFormat {
                Invalid     = 0,
				Float64		= -64,
				Float32		= -32,
				Unsigned8	= 8,
				Signed8		= 10,
				Signed16	= 16,
				Unsigned16	= 20,
				Signed32	= 32,
				Unsigned32	= 40,
				Signed64	= 64
			};

			/** Defines the in-memory ordering of the image rows. TopDown 
				images have their top row at the lowest memory location. while
				BottomUp images have their bottom row at the lowest memory 
				location. */
			enum RowOrdering {
				TopDown,
				BottomUp
			};
		private:
			size_type width;
			size_type height;
			size_type planes;
			PixelFormat format;

			ImageReader* owner;
		public:
			ImageCube(size_type width, size_type height, size_type planes, 
				PixelFormat format, ImageReader* owner = 0);
			virtual ~ImageCube();
			/** Returns the width of this image cube in pixels. */
			size_type Width() const;
			/** Returns the height of this image cube in pixels. */
			size_type Height() const;
			/** Returns the number of planes in the image. */
			size_type Planes() const;
			/** Returns the pixel data type. */
			PixelFormat Format() const;
			/** Returns the total number of pixels in the cube. */
			inline size_type Pixels() const;
			/** Returns the number of pixels per plane. */
			size_type PixelsPerPlane() const;
			/** Checks whether this image needs a separate map of null pixels.
				The null map determines if a pixel value is valid or should be
				treated as a null pixel. A null pixel should not be included in
				calculations of dynamic range, mean and so on. Also the pixel 
				should be rendered as transparent if possible. */
			virtual bool NeedsNullMap() const = 0;
			/** Returns the required buffer size for a rectangular section of a plane.
				@param width Width of the area of the interest.
				@param height Height of the area of interest. */
			size_type SizeOf(size_type width, size_type height) const;
			/** Returns the required buffer size for a plane of the image.
				@param plane Plane of interest. Since image cubes are rectangular boxes
					this parameter is ignored.
				@return The required size in bytes. */
			size_type SizeOf(size_type plane) const;
			/** Returns the required buffer size for all pixels in the image.
				@return The required size in bytes. */
			size_type SizeOf() const;
			/** Returns the size of a buffer of a given size in bytes.
				@param format The number of bits per pixel selected from the 
				ImageCube::PixelFormat enumeration. Values outside those defined in this 
				enumeration results in unpredictable behavior.
				@param width Width of the area of the interest.
				@param height Height of the area of interest. */
			static size_type SizeOf(ImageCube::PixelFormat format, size_type width, size_type height);
			/** Returns a reference to the image reader this image cube belongs to.
				@returns A reference or NULL. */
			ImageReader* Owner() const;
			/** Retrieves a property.
				@param name Name of the property.
				@return The value of the property or an empty string if the property is not set. */
			virtual std::string Property(const std::string& name) const = 0;
			/** Dumps all the properties in textformat.
                @param stream Stream to dump the properties into.
                @param prefix Prefix to prepend to every property. */
            virtual void Properties(std::ostream& stream, const std::string& prefix) const = 0;
			/** Retrieves a numeric property.
				@param name Name of the property.
				@param out Buffer to receive the value.
				@return If out = 0 the return value is false. If the property cannot be found
					or is not numeric the return value is false. Otherwise the return value is true. */
			bool NumericProperty(const std::string& name, double* out) const;
			/** Returns the row order for this image. */
			virtual RowOrdering RowOrder() const = 0;
			/** Reads a block of pixels from an image.
				@param plane Plane of the image to read from. Must be in the interval 
					[0;image->Planes()[.
				@param bounds Boundaries of the block to read. The pixels include [left;right[ x [top;bottom[.
				@param buffer Buffer to write the pixels into. */
            virtual void Read(ImageCube::size_type plane, const FitsLiberator::Rectangle& bounds, void* buffer) const = 0;
			/** Reads a block of pixels from an image.
				@param plane Plane of the image to read from. Must be in the interval 
					[0;image->Planes()[.
				@param bounds Boundaries of the block to read. The pixels include [left;right[ x [top;bottom[.
				@param buffer Buffer to write the pixels into.
				@param valid  Buffer to write the null map into. One byte for 
					each pixel. */
			virtual void Read(ImageCube::size_type plane, const FitsLiberator::Rectangle& bounds, void* buffer, char* valid) const = 0;
			/** Reads the entire contents of the image.
				@param plane Plane of the image to read from. Must be in the interval 
					[0;image->Planes()[.
				@param image The contained image to read from.
				@param buffer Buffer to write the pixels into. */
			virtual void Read(ImageCube::size_type plane, void* buffer) const = 0;
			/** Reads the entire contents of the image.
				@param plane Plane of the image to read from. Must be in the interval 
					[0;image->Planes()[.
				@param image The contained image to read from.
				@param buffer Buffer to write the pixels into. The buffer is 
					expected to be of size image->SizeOf(image->Width(), 
					image->Height). 
				@param valid  Buffer to write the null map into. One byte for 
					each pixel. */
			virtual void Read(ImageCube::size_type plane, void* buffer, char* valid) const = 0;
        };
    }
}

#endif	// __IMAGECUBE_H__