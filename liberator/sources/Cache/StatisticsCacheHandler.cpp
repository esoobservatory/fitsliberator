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
#include "StatisticsCacheHandler.h"

using namespace FitsLiberator::Caching;


StatisticsCacheHandler::StatisticsCacheHandler()
{
	realCache = NULL;
	for( UInt i = 0; i < kFITSStretchNumberOfFunctions; i++ )
		caches[i] = new FitsStatisticsCache();
}

StatisticsCacheHandler::~StatisticsCacheHandler()
{
	if ( realCache != NULL )
		delete realCache;
	for( UInt i = 0; i < kFITSStretchNumberOfFunctions; i++ )
		delete caches[i];
}

Bool StatisticsCacheHandler::isRealData(Stretch& stretch, Plane& plane)
{
	return ( stretch.function == stretchLinear );
}
/*
	compares the stored caches with the given stretch and plane, and if
	stretch.offset == [SomeCache].background &&
	stretch.scale == [SomeCache].scale &&
	plane.imageIndex == [SomeCache].imageIndex &&
	plane.planeIndex == [SomeCache].planeIndex
	if will return true
	false otherwise.

*/
Bool StatisticsCacheHandler::useCache( Stretch& stretch, Plane& plane, Double* min, Double* max, Double* mean,
							   Double* stdev, Double* median, Vector<Double>& histogram, Double* maxBinCount  )
{

	FitsStatisticsCache* cache = NULL;


	Int index = getStretchIndex( stretch.function );

	if (index >= 0 && index < kFITSStretchNumberOfFunctions )
	{
		if ( cache == NULL )
		{
			cache = (FitsStatisticsCache*)caches[index];
		}
		if ( stretch.offset		== cache->background	&&
			 stretch.scale		== cache->scale		&&
			 plane.imageIndex	== cache->imageIndex &&
			 plane.planeIndex	== cache->planeIndex )
		{
			
			*max			= cache->max;
			*min			= cache->min;
			*mean			= cache->mean;
			*median			= cache->median;
			*stdev			= cache->stdev;
			*maxBinCount	= cache->maxBinCount;
		
			histogram.assign( cache->histogram.begin(), cache->histogram.end() );

			return true;
		}
	}
	return false;
}

Void StatisticsCacheHandler::storeCache( Stretch& stretch, Vector<Double>& histo, Double maxBinCount, Plane& plane, Double background, Double scale,
										Double stretchMax, Double stretchMin, Double stretchMean, Double stretchMedian, Double stretchSTDEV)
{
	Int index = getStretchIndex( stretch.function );
	if ( index >= 0 && index < kFITSStretchNumberOfFunctions )
	{
		FitsStatisticsCache* cache = NULL;

		cache = (FitsStatisticsCache*)caches[index];
		cache->planeIndex	= plane.planeIndex;
		cache->imageIndex	= plane.imageIndex;
		cache->background	= background;
		cache->scale		= scale;
		
		
		cache->max			= stretchMax;
		cache->min			= stretchMin;
		cache->mean			= stretchMean;
		cache->median		= stretchMedian;
		cache->stdev		= stretchSTDEV;
		
		cache->maxBinCount	= maxBinCount;
		
		cache->histogram.assign( histo.begin(), histo.end() );
	}
}