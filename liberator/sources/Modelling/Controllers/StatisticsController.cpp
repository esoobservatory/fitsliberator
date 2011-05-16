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
#include "StatisticsController.h"

using namespace FitsLiberator::Modelling;
using namespace FitsLiberator::Engine;


StatisticsController::StatisticsController( StatisticsModel& m ) : 
model( m )
{

}


Void StatisticsController::defaultValues()
{
	this->model.clearCache();
}

/**
	Called when the statistics should be refreshed.

	This method can make the right calls for calculating the statistics
	and it can use the cache if enabled
	and it will cheat in linear mode if possible.

	@returns true if it was able to use cached values, false otherwise.
*/
Bool StatisticsController::performStatistics( Bool stretched, Stretch& stretch, Plane& plane,
											 Double* min, Double* max, Double* mean, Double* stdev,
											 Double* median, Vector<Double>& tmpBins, Double* maxBin )
{
	if ( !kFITSDoCache )
	{
		return false;
	}
	else
	{
		if ( this->model.useCache( stretch, plane, min, max, mean, stdev, median, tmpBins, maxBin ) )
		{
			return true;
		}
	}
	return false;
}
StatisticsModel& StatisticsController::getModel()
{
	return this->model;
}

/**
 *
 */
Void StatisticsController::setRealValues( Double min, Double max, Double mean, 
										 Double median, Double stdev, Double scale,
										 Double background, Double scaleBackground )
{
	this->model.realMin 	= min;
	this->model.realMax		= max;
	this->model.realMean	= mean;
	this->model.realMedian	= median;
	this->model.realSTDEV 	= stdev;
	
	this->setScaledValues( scale, background, scaleBackground );
}

Void StatisticsController::setScaledValues( Double scaleFactor, Double background, Double scaleBackground )
{
	Double min				= this->model.getRealMin();
	Double max				= this->model.getRealMax();
	Double mean				= this->model.getRealMean();
	Double median			= this->model.getRealMedian();
	Double stdev			= this->model.getRealSTDEV();

	this->model.scaledMin	= scaleFactor * ( min - background ) + scaleBackground;
	this->model.scaledMax	= scaleFactor * ( max - background ) + scaleBackground;
	this->model.scaledMean	= scaleFactor * ( mean - background ) + scaleBackground;
	this->model.scaledMedian= scaleFactor * ( median - background ) + scaleBackground;
	this->model.scaledSTDEV	= scaleFactor * stdev;

	this->model.Notify();
	
}
/*
Used if the stretched values are the same as the scaled values
*/
Void StatisticsController::useScaledAsStretchedValues()
{
	setStretchValues( model.getScaledMin(), model.getScaledMax(), model.getScaledMean(), 
					  model.getScaledMedian(), model.getScaledSTDEV() );
}

/**
 *
 */
Void StatisticsController::setStretchValues( Double min, Double max, Double mean, 
											Double median, Double stdev )
{
	
	this->model.stretchMin 		= min;
	this->model.stretchMax		= max;
	this->model.stretchMean		= mean;
	this->model.stretchMedian	= median;
	this->model.stretchSTDEV 	= stdev;
	this->model.Notify();
	
}




Void StatisticsController::setStretchMin(Double d)
{
	this->model.stretchMin = d;
	this->model.Notify();
}
Void StatisticsController::setStretchMax(Double d)
{
	this->model.stretchMax = d;
	this->model.Notify();
}
Void StatisticsController::setStretchMean(Double d)
{
	this->model.stretchMean = d;
	this->model.Notify();
}
Void StatisticsController::setStretchMedian(Double d)
{
	this->model.stretchMedian = d;
	this->model.Notify();
}
Void StatisticsController::setStretchSTDEV(Double d)
{
	this->model.stretchSTDEV = d;
	this->model.Notify();
}
