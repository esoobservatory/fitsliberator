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

#include "Environment.h"
#include "FitsStatistics.h"
#ifdef USE_OPENMP
#include <omp.h>
#endif

using namespace FitsLiberator::Engine;
using namespace FitsLiberator;


FitsStatistics::FitsStatistics(FitsEngine& owner, const Stretch& stretch, const Plane& plane, UInt planeSize)
  : owner(owner),
    stretch(stretch),
    plane(plane),
    planeSize(planeSize)
{
}

FitsStatistics::~FitsStatistics()
{
}

/**
	Method that performs the initial guess based on whatever solution we want.
	Obsolete
*/
/*
Void FitsStatistics::initialGuess( Double* blackLevel, Double* whiteLevel, Double min, Double max, Double mean, Double stdev, Double median, Vector<Double>& histoBins )
{
	if ( kFITSInitialGuessMeanPMStdev )
	{
		//the initial guess is set to mean +/- stdev
		Double b = mean - stdev;
		Double w = mean + stdev;

		if ( b < min || !FitsMath::isFinite( b ) )
		{
			b = min;
		}
		if ( w > max || !FitsMath::isFinite( w ))
		{
			w = max;
		}
		*blackLevel = b;
		*whiteLevel = w;
	}
	else if ( kFITSInitialGuessHisThreePer )
	{
		Double rangeMin = 0;
		Double rangeMax = 0;
		Int minBin		= 0;
		Int maxBin		= 0;
		Double sum = 0;
		UInt i = 0;
		//the initial guess is set to min=3% and max=97%
		for ( i = 0; i < histoBins.size(); i++)
		{
			//sum += pow(10.0,histoBins[i]);
			sum += histoBins[i];
		}
		for ( i = 0; i < histoBins.size(); i++)
		{
			if ( rangeMin / sum <= kFITSInitialGuessMinPercent )
			{
				//rangeMin += pow( 10.0, histoBins[i] );
				rangeMin += histoBins[i];
				minBin = i;
			}
			if ( rangeMax / sum <= kFITSInitialGuessMaxPercent )
			{
				//rangeMax += pow( 10.0, histoBins[i] );
				rangeMax += histoBins[i];
				maxBin = i;
			}
		}
		*blackLevel = ((Double)minBin / (Double)histoBins.size()) * (max - min) + min;	
		
		*whiteLevel = ((Double)maxBin / (Double)histoBins.size()) * (max - min) + min;	
	}
	else if ( kFITSInitialGuessMedianPMStdev )
	{
		//the initial guess is set to median +/- stdev
		Double b = median - 2.0 * stdev;
		Double w = median + 2.0 * stdev;

		if ( b < min || !FitsMath::isFinite( b ) )
		{
			b = min;
		}
		if ( w > max || !FitsMath::isFinite( w ))
		{
			w = max;
		}
		*blackLevel = b;
		*whiteLevel = w;	
	}
}
*/
Void FitsStatistics::scaleHistogram(Vector<Double>& histogram, Int binCount, UInt pixelCount, Double* maxBinCount, Double* median, Double max, Double min)
{
	// Calculate median while scaling the histogram
    *maxBinCount = 0.0;
    Double medianCount = 0;

	UInt cnt = 0;
	for( UInt i = 0; i < (UInt)binCount; i++ ) {
		Double bin = histogram[i];
		// Count pixels for the median
		if( medianCount < (Double)pixelCount / 2.0 )
		{
			medianCount += histogram[cnt];	
			cnt++;
		}
		if (histogram[i] > 0.0)
		{
			histogram[i] = FitsMath::log10( bin );
		}
		if( histogram[i] > *maxBinCount )
		{
			*maxBinCount = histogram[i];
		}
	}
	*median = ((Double)cnt / (Double)binCount) * (max - min) + min;
}

Void FitsStatistics::accumulateHistogram( Double* pixels, UInt length, Double* stdev, Double mean, Double min, Double invBinSize, Vector<Double>& histogram, UInt cChunk, UInt nChunks)
{
	Int nCallBacks = 4;
//	CallbackSink* sink = Environment::getCallbacks();
	UInt width = length / nCallBacks;

	const Vector<Double>::size_type size = histogram.size();
#ifdef USE_OPENMP
	Int nProcs = omp_get_num_procs();
#endif

	Double stdev_tmp = 0.;
	for ( Int j = 0; j < nCallBacks; j++)
	{		 			
#ifdef USE_OPENMP
		#pragma omp parallel num_threads( nProcs ) firstprivate(stdev_tmp)	
#endif
		{
#ifdef USE_OPENMP
		#pragma omp for
#endif
			for ( Int i = 0; i < width; i++)
			{
				Double pixel = pixels[ j * width + i];
				if ( pixel != FitsMath::NaN && FitsMath::isFinite( pixel ) )
				{
					stdev_tmp += (mean - pixel) * (mean - pixel);

					Int binIndex = (Int)FitsMath::round( invBinSize * (pixel - min) );
					// Make sure we don't crash.
					if( binIndex < 0 ) {
						binIndex = 0;
					} else if( binIndex >= size ) {
						binIndex = size - 1;
					}
					histogram[binIndex]++;
				}
			}
#ifdef USE_OPENMP
			#pragma omp critical
#endif
			{				
				*stdev = *stdev + stdev_tmp;
				stdev_tmp = 0.;
			}			
		}	
//		sink->Progress( nCallBacks * cChunk + j, nCallBacks * nChunks );
	}
#ifdef USE_OPENMP
	#pragma omp parallel num_threads( nProcs ) firstprivate(stdev_tmp)
#endif
	{ 
#ifdef USE_OPENMP
		#pragma omp for
#endif
		for ( Int k = width * nCallBacks; k < length; k++)
		{
			Double pixel = pixels[k];
			if ( pixel != FitsMath::NaN && FitsMath::isFinite( pixel ) )			
			{
				stdev_tmp += (mean - pixel) * (mean - pixel);

				Int binIndex = (Int)FitsMath::round( invBinSize * (pixel - min) );
				// Make sure we don't crash.
				if( binIndex < 0 ) {
					binIndex = 0;
				} else if( binIndex >= size ) {
					binIndex = size - 1;
				}
				histogram[binIndex]++;
			}
		}
#ifdef USE_OPENMP
		#pragma omp critical
#endif
		{				
			*stdev = *stdev + stdev_tmp;
			stdev_tmp = 0.;
		}
	}
	//sink->Progress( nCallBacks * cChunk + nCallBacks, nCallBacks * nChunks );
}

Void FitsStatistics::accumulateRange(Double* pixels, UInt length, Double* mean, UInt* pixelCount, Double* max, Double* min, UInt cChunk, UInt nChunks)
{
//	CallbackSink* sink = Environment::getCallbacks();
	
	UInt nCallBacks = 4;

	UInt width	= length / nCallBacks;
	Int pxCnt = 0;
	Double mean_tmp = 0;
	Double max_tmp = *max;
	Double min_tmp = *min;

#ifdef USE_OPENMP
	Int nProcs = omp_get_num_procs();
#endif	

	for ( Int j = 0; j < nCallBacks; j++)
	{
#ifdef USE_OPENMP
		#pragma omp parallel num_threads( nProcs ) firstprivate(pxCnt,mean_tmp,max_tmp,min_tmp)
#endif
		{ 	
#ifdef USE_OPENMP
			#pragma omp for 
#endif
			for ( Int i = 0; i < width; i++ )
			{
				Double pixel = pixels[ j * width + i];
				if ( pixel != FitsMath::NaN && FitsMath::isFinite( pixel ) )
				{
					if ( pixel > max_tmp )
					{
						max_tmp = pixel;
					}
					if ( pixel < min_tmp )
					{
						min_tmp = pixel;
					}
					mean_tmp += pixel;
					pxCnt += 1;
				}	
			}
#ifdef USE_OPENMP
			#pragma omp critical
#endif
			{
				if ( max_tmp > *max )
				{
					*max = max_tmp;
				}
				if ( min_tmp < *min )
				{
					*min = min_tmp;
				}
				*mean = *mean + mean_tmp;
				mean_tmp = 0.;
				*pixelCount = *pixelCount + pxCnt;
			}
		}
//		sink->Progress( nCallBacks * cChunk + j, nCallBacks * nChunks );
	}
#ifdef USE_OPENMP
	#pragma omp parallel num_threads( nProcs ) firstprivate(pxCnt,mean_tmp,max_tmp,min_tmp)
#endif
	{ 	
#ifdef USE_OPENMP
		#pragma omp for
#endif
		for ( Int k = width * nCallBacks; k < length; k++ )
		{
			Double pixel = pixels[k];
			if ( pixel != FitsMath::NaN && FitsMath::isFinite( pixel ) )
			{
				if ( pixel > max_tmp )
				{
					max_tmp = pixel;
				}
				if ( pixel < min_tmp )
				{
					min_tmp = pixel;
				}
				mean_tmp += pixel;
				pxCnt += 1;
			}
		}
#ifdef USE_OPENMP
		#pragma omp critical
#endif
		{
			if ( max_tmp > *max )
			{
				*max = max_tmp;
			}
			if ( min_tmp < *min )
			{
				*min = min_tmp;
			}
			*mean = *mean + mean_tmp;
			*pixelCount = *pixelCount + pxCnt;
		}
	}


	//sink->Progress( nCallBacks * cChunk + nCallBacks, nCallBacks * nChunks );
}

/**
 * Calculates all the statistics for the image. This has been lumped together to only iterate twice over the pixels.
 * @param stretched Calculate strected or regular statistics.
 * @param min [out] Recieves the minimum value.
 * @param max [out] Recieves the maximim value.
 * @param mean [out] Recieves the mean value.
 * @param stddev [out] Recieves the stddev.
 * @param median [out] Recieves the median.
 * @param histogram [out] Recieves the histogram.
 * @param maxBinCount [out] Recieves the maximum number of pixels in one histogram bin.
 */
Void FitsStatistics::getStatistics( Bool stretched, Double* min, Double* max, 
								   Double* mean, Double* stddev, Double* median, 
								   Vector<Double>& histogram, Double* maxBinCount,
								   const Flip& flip)
{
	
	//CallbackSink* sink = Environment::getCallbacks();
	
	UInt i = 0;

	//resets the bins
	for (i = 0; i < histogram.size(); i++)
	{
		histogram[i] = 0.0;
	}
    // Initialization
    Stretch stretch;    // Initialize a neutral stretch

    // If we need the strected version copy the stretch data
    // recieved in the constructor.
    if( stretched )
        stretch = this->stretch;
	
	FitsPlane plane = owner.getPlane( this->plane, stretch, flip );

	//counts the number of pixels != NaN && != isFinite
	UInt pixelCount = 0;

	//sink->Begin( Environment::getString( this->getRangeStringID( stretched ) ) );
	
	getRange( plane, min, max, mean, true, stretched, &pixelCount );

    // Actual calculations
    UInt binCount = histogram.capacity();
    Double binSize = (*max - *min) / (binCount - 1);
	
	UInt nChunks = plane.getNumberOfChunks();	
	
	//accumulate both histogram, stdev and median
	if ( binCount > 0 )
	{	
		Double invBinSize = 1.0 / binSize;
		//runs through the available chunks
		for ( i = 0; i < nChunks; i++ )
		{
			UInt size = 0;
			Double* pixelBuffer = plane.getChunk( i, &size, stretch );
		//	sink->Status( Environment::getString( this->getStatsStringID( stretched ) ) );
			accumulateHistogram( pixelBuffer, size, stddev, *mean, *min, invBinSize, histogram, i, nChunks );
		}
	}//only find stddev
	else
	{
		for ( i = 0; i < nChunks; i++ )
		{
			UInt size = 0;
			Double* pixelBuffer = plane.getChunk( i, &size, stretch );
	
			Double pixel = pixelBuffer[i];

			if( pixel != FitsMath::NaN && FitsMath::isFinite( pixel ))
			{
				// StdDev
				*stddev += (*mean - pixel) * (*mean - pixel);
			}
		}
	}
	*stddev = FitsMath::squareroot( 1.0 / (Double)pixelCount * (*stddev) );

	scaleHistogram( histogram, binCount, pixelCount, maxBinCount, median, *max, *min);

   // sink->End();
}

/**
 * Determines the range of values in the plane.
 * @param plane Plane to read from.
 * @param min [out] Recieves the minimum value.
 * @param max [out] Recieves the maximim value.
 * @param mean [out] Recieves the mean value.
 */
Void FitsStatistics::getRange( FitsPlane& plane, Double* min, Double* max, Double* mean,
							  Bool usePixelBuffer, Bool stretched, UInt* pixelCount ) {

	//CallbackSink* sink = Environment::getCallbacks();

	*min = DoubleMax;
    *max = DoubleMin;
    *mean = 0;
	UInt i = 0;
	UInt nChunks = plane.getNumberOfChunks();	

	//init default stretch
	Stretch stretch;

	//if the pixels should be stretched we should use the
	//stretch given in the constructor.
	if ( stretched )
		stretch = this->stretch;

	
	for ( i = 0; i < nChunks; i++)
	{
		UInt size = 0;

		Double* pixelBuffer = plane.getChunk( i, &size , stretch );
	//	sink->Status( Environment::getString( this->getRangeStringID( stretched ) ) );
		accumulateRange( pixelBuffer, size, mean, pixelCount, max, min,i, nChunks );

	}
	if ( *pixelCount > 0 )
	{
		*mean = *mean / (Double)(*pixelCount);
	}
	//sink->End();
}

Int FitsStatistics::getRangeStringID( Bool stretched )
{
	if ( stretched )
		return IDS_STATS_RANGE_PROGRESS_STRETCH;
	else
		return IDS_STATS_RANGE_PROGRESS_REAL;
}

Int FitsStatistics::getStatsStringID( Bool stretched )
{
	if ( stretched )
		return IDS_STATS_PROGRESS_STRETCH;
	else
		return IDS_STATS_PROGRESS_REAL;
}
