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
    Implements the WCS coordinate mapper.
*/

#include "WcsMapper.hpp"
#include "FitsImageReader.hpp"

#include <fitsio.h>
#include <limits>

using std::numeric_limits;

using FitsLiberator::Engine::ImageReader;
using FitsLiberator::Engine::ImageCube;
using FitsLiberator::Engine::WcsMapper;

WcsMapper::WcsMapper(const ImageCube* image) {
    // We only support FITS images right now
    if(image->Owner()->format() == ImageReader::FITS) {
        const FitsImageCube* cube = dynamic_cast<const FitsImageCube*>(image);
        FitsImageReader* reader = dynamic_cast<FitsImageReader*>(cube->Owner());

        assert(cube != nullptr);    // Dynamic cast MUST succeed since we check
        assert(reader != nullptr);  // As above

        valid = reader->ReadWCS(cube, 
            &xrefval, &yrefval,
            &xrefpix, &yrefpix,
            &xinc, &yinc,
            &rot,
            coordtype);
    } else {
        valid = false;
    }
}

bool
WcsMapper::Map(double x, double y, double* ra, double* dec) const {
    bool success = false;

    if(valid) {
        int status = 0;
        
        fits_pix_to_world(x, y, xrefval, yrefval, xrefpix, yrefpix, 
            xinc, yinc, rot, (char*)coordtype, ra, dec, &status);

        success = (status == 0);
    } else {
        *ra = numeric_limits<double>::signaling_NaN();
        *dec = numeric_limits<double>::signaling_NaN();
    }

    return success;
}
