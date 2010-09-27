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

#ifndef __FITSTATISTICSTOOLS_H__
#define __FITSTATISTICSTOOLS_H__

#include "FitsLiberator.h"

namespace FitsLiberator
{
	namespace Engine
	{
		/**
		Collection of static methods that handle the statistics operations
		*/
		class FitsStatisticsTools
		{
		public:
            /** Method for accumulating range and mean.
	            Designed to work on a parallel basis meaning that the mean_acc is merely a sum
	            of the pixels so it might later be converted to the actual mean value.
	            @param pixels is the array containing the pixels to work on	
	            @param nPixels is the number of pixels in the pixels array
	            @param min is a pointer to the Double-typed min value
	            @param max is a pointer to the Double-typed max value
	            @param mean_acc is a pointer to the Double-typed mean_acc value
            */
			static Void getRange( Double* pixels, Int nPixels, Int* pixelCnt,
								  Double* min, Double* max, Double* mean_acc );

            /** Method for accumulating range and mean.
	            Designed to do the job in parallel
	            @param pixels is the array containing the pixels to work on	
	            @param nPixels is the number of pixels in the pixels array
	            @param min is a pointer to the Double-typed min value
	            @param max is a pointer to the Double-typed max value
	            @param mean_acc is a pointer to the Double-typed mean_acc value
            */
			static Void getRange_par( Double* pixels, Int nPixels, Int* pixelCnt,
								  Double* min, Double* max, Double* mean_acc, Int nCpus );
            /** Method for finding stdev, median and the histogram. Retrieves 
                the histogram info on a single thread */
			static Void getHistogram(Double* pixels, Int length, Double* stdev, Double mean, 
									   Double min, Double invBinSize, Vector<Double>& histogram );
			
			/**Method for finding stdev, median and the histogram in parallel*/
			static Void getHistogram_par(Double* pixels, Int length, Double* stdev,
				Double mean, Double min, Double invBinSize, 
				Vector<Double>& histogram, Int nCpus );
			
			/**Method for scaling the histogram*/
			static Void scaleHistogram( Vector<Double>& histogram, Double* median, Double min,
				Double max, Double* maxBinCount, UInt pixelCount );
			/**Calculates the initial guess based on the user-specified algorithm
			and the statistical information about the image*/
			static Void  initialGuess( InitialGuess algorithm, Double minPercent, Double maxPercent, 
									   Double* blackLevel, Double* whiteLevel, Double min, 
									   Double max, Double mean, Double stdev, Double median, 
									   Vector<Double>& histoBins );			
			

		};
	}

}


#endif
