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

#ifndef __FITSENGINE_H__
#define __FITSENGINE_H__

#include "FitsLiberator.h"
#include "Stretch.h"
#include "ImageCube.hpp"

namespace FitsLiberator {
    namespace Engine {
        /**
         * FitsEngine is used to apply stretches to a range of pixels. It serves as a hub for pixel processing.
         */
        class FitsEngine {
        public:			
			static Void stretchRealValues(const Stretch&, Double* rawPixels, Double* out, Int count);
            /** Stretches an array of pixels. While it may be called by anyone, it is designed to work with caches.
                @param stretch Stretch to apply
                @param bitDepth Bit depth of the pixels
                @param rawPixels Pixel data
                @param nullPixels Null map, used to bypass processing when it is not needed.
                @param out The output array
                @param count Number of pixels to process; rawPixels, nullPixels and out must contain atleast this number of elements.
				@param nCpus the number of cpus to use when processing in parallel
				*/
            static Void stretch(const Stretch& stretch, ImageCube::PixelFormat bitDepth,
				Void* rawPixels, Byte* nullPixels, Double* out, UInt count, Int nCpus );
            /** Scales an array of pixels inplace.
                @param stretch This method uses only blackLevel, whiteLevel and outputMax.
                @param pixels Pixel array.
                @param count Number of elements in the pixel array. */
            static Void scale( const Stretch&, Double*, UInt );
			/** Scales an array of pixels inplace in parallel
                @param stretch This method uses only blackLevel, whiteLevel and outputMax.
                @param pixels Pixel array.
                @param count Number of elements in the pixel array. 
				@param nCpus the number of cpus to use during the processing*/
			static Void scale_par( const Stretch&, Double*, UInt, UInt );
            /** Returns the linear value of the given Double. 
				@param stretch the given stretch to use in the process
				@param val the value to be stretched
				@returns the stretched value*/
			static Double getLinearVal(const Stretch&, Double);
			static Double getLinearValWithoutStretch(const Stretch&, Double);
            static Double linearValStretch( const Stretch&, Double val, Bool doStretch );

            static inline Double applyPreStretch( Double pixel, Double scale, Double offset, Double scaleBackground ) {
                return (scale * ( pixel - offset ) + scaleBackground);
            };

            static inline Double unapplyPreStretch( Double pixel, Double scale, Double offset, Double scaleBackground ) {
                return ((pixel - scaleBackground) / scale + offset );
            };

        private:
            /** Internal method, which does the actual stretching. This is a template function to allow the
                compiler to optimize the code for each datatype.
                @param stretch Stretch to apply
                @param rawPixels Pixel data
                @param nullPixels Null map, used to bypass processing when it is not needed.
                @param out The output array
                @param count Number of pixels to process; rawPixels, nullPixels and out must contain
                atleast this number of elements. */
            template<typename I>
            static Void _stretch(const Stretch& stretch, I* rawPixels, Byte* nullPixels, 
				Double* out, Int count, Int nCpus );
        };
    }
}

#endif