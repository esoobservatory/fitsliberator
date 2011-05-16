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

#ifndef __FitsMath_H__
#define __FitsMath_H__

#include <math.h>
#include "FitsLiberator.h"

namespace FitsLiberator {
    namespace Engine {
        //-----------------------------------------------------------------------------
        // Type constants
        //-----------------------------------------------------------------------------
        const Float FloatMin = -3.402823466e+38F;
        const Float FloatMax = 3.402823466e+38F;
        const Double DoubleMin = -1.7976931348623158e+308;
        const Double DoubleMax = 1.7976931348623158e+308;

        /**
        * Defines useful math routines
        */
        class FitsMath {
            public:
				/** Rounds the double d to a double representation of an integer.
					@param d The number to round.
					@return The rounded number. */                
				static inline Double round(Double d) {					
					return Int( d > 0.0 ? d + 0.5 : d - 0.5);
					//return ::Int(d + 0.5);
				}
				/** Returns the logarithm to base 10 of a number
					@param d The number to apply the function on.
					@return log_10(d) */
				static inline Double log10(Double d) {
					return ::log10(d);
				}
				/** Returns the absolute value of the given argument.
					@param number Number to test. */
				template<typename T>
				static T absolute(T number) {
					return (number < 0) ? -number : number;
				}
				/** Returns the squareroot of the a number. 
					@param x Number to take the root of.
					@return sqrt(x). */
				static inline Double squareroot(Double x) {
					return ::sqrt(x);
				}
				/** Calculates x^y. */
				static inline Double power(Double x, Double y) {
					return ::pow(x, y);
				}
				/** Returns the smaller of two numbers. */
				template<typename T>
				static T minimum(T x, T y) {
					return (x < y) ? x : y;
				}
				/** Returns the bigger of two numbers. */
				template<typename T>
				static T maximum(T x, T y) {
					return (x > y) ? x : y;
				}
				static Double maxval(Double* vals, Int count);
//				static Bool isFinite(Double d);
                static const Double NaN;
				template<typename T>
				static inline T signof( T d )
				{
					if ( d < 0 ) return -1;
					else return 1;
				}

				static inline Bool isFinite( Double x)
				{
					return ( x <= DoubleMax && x >= -DoubleMax ); 
				}    

                
        };
    }
}
#endif