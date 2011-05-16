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
    CFITSIO and pds_toolbox. This file defines the classes Engine::ImageReader
*/

#ifndef __IMAGEREADER_H__
#define __IMAGEREADER_H__

#include <string>
#include <vector>

#include "Exception.h"
#include "ImageCube.hpp"
#include "Image.h"

namespace FitsLiberator {
    namespace Engine {
		/** Exception class for the ImageReader class and its subclasses. */
		class ImageReaderException : public Exception {
			typedef Exception super;

			std::string filename;
        public:
			/** Constructs an ImageReaderException from a filename.
				@param filename Filename of the file that caused the error. */
			ImageReaderException(const std::string& filename);
            virtual ~ImageReaderException() {}
        };

        /** Represents an image file reader. An image file is a collection of 
			images and the associated metadata. Each image may contain one or more
			2-D image planes. The metadata is a dictionary of (Key,Value) 
			pairs. */
        class ImageReader {
			typedef std::vector<ImageCube*>		container;

            container   images;
            std::string	filename;
		protected:
			ImageReader(const std::string& filename);
			/** Adds an image definition to the collection of images. 
				@param image An ImageCube object to add. Note: The ImageReader
				instance takes over ownership of the memory pointed to by the image 
				parameter. */
			void insert(ImageCube* image);
		public:
			typedef const ImageCube*            value_type;

			typedef container::reference		reference;
			typedef container::const_reference	const_reference;
			typedef container::iterator			iterator;
			typedef container::const_iterator   const_iterator;
			typedef container::difference_type	difference_type;
			typedef container::size_type		size_type;

            enum ImageFormat {
                NONE = 0,
                FITS = 1,
                PDS  = 2
            };

			virtual ~ImageReader();
			/** Constructs an image reader object from a filename.
				@param filename The full path of the file to read from. */
			static ImageReader* FromFile(const std::string& filename);
			/** Returns true if the file is support (and can actually be read) */
			static Bool IsSupported( const std::string& filename );
			/** Returns the number of images contained in the image file. */
			size_type size() const;
			/** Returns true if there are no images available to read. */
			bool empty() const;
			/** Returns the file name of the image file. */
			const std::string& FileName() const;
			/** Returns information about one of the contained images.
				@param i Index of the image to retrieve information about. Must be in the 
				interval [0;ImageCount()[. 
				@returns A pointer to an ImageCube object or NULL if the requested image 
				does not exist. */
			value_type operator[](size_type i) const;
			/** Returns an iterator, which iterates over the contained image cubes. */
			const_iterator begin() const;
			/** Returns an iterator marking the end of the collection of contained image
				cubes. */
			const_iterator end() const;
            /** Retrieves the file header in human-readable format.
                @param stream Stream to write the header to. */
            virtual void header(std::ostream& stream) const = 0;
            /** Returns the type of image loaded. */
            virtual ImageFormat format() const = 0;
        };
    }
}
#endif