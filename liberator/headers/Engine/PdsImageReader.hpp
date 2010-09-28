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
    pds_toolbox. This file defines the class Engine::PdsImageReader
*/

#include "ImageReader.hpp"
#include "PdsImageCube.hpp"

extern "C" {
	#include <oal.h>
}

namespace FitsLiberator {
	namespace Engine {
        /** PDS file format parser. This class parses PDS files looking for 
            images. */
		class PdsImageReader : public ImageReader {
			typedef ImageReader super;
            /** Root node of the object tree contained in the PDS label file.*/
			ODLTREE root;
            /** Traverse the object tree looking for images.
                @param node Root node of the sub-tree to traverse. 
                @param action Action to perform on the node. */
            template<typename Action>
            void Traverse(OBJDESC* node, Action action, int level = 0);

            struct Include;
            struct Print;
		public:
            /** The constructor parses a PDS file by filename.
                @param filename File name of the file to parse. */
			PdsImageReader(const std::string& filename);
            /** The destructor frees all memory associated with the file. After
                calling the desctructor, pointers to the images inside the file
                are no longer valid. */
			virtual ~PdsImageReader();
            /** Performs a fileformat check to see if the file can be read. 
                Even if this method returns true it does not mean the file 
                contains readable images.
                @param filename Path of the file to check.
                @return True if the file can be read, false if it cannot. */
            static bool CanRead(const std::string& filename);
            /** @see ImageReader::header. */
            virtual void header(std::ostream& stream) const;
            /** @see ImageReader::format. */
            virtual ImageReader::ImageFormat format() const;
		};
	}
}
