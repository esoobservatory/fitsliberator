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
#include "StatisticsModel.h"

using namespace FitsLiberator::Modelling;
using namespace FitsLiberator::Caching;
using namespace FitsLiberator::Engine;

/**
 *
 */
StatisticsModel::StatisticsModel( ChangeManager* chman ) : Model( chman ) {
	
	this->realMax		=	0.0;
	this->realMean		=	0.0;
	this->realMedian	=	0.0;
	this->realMin		=	0.0;
	this->realSTDEV		=	0.0;
	this->stretchMax	=	0.0;
	this->stretchMean	=	0.0;
	this->stretchMedian	=	0.0;
	this->stretchMin	=	0.0;
	this->stretchSTDEV	=	0.0;
	this->scaledMax		=	0.0;
	this->scaledMean	=	0.0;
	this->scaledMedian	=	0.0;
	this->scaledMin		=	0.0;
	this->scaledSTDEV	=	0.0;
	if ( kFITSDoCache )
		this->cacheHandler = new StatisticsCacheHandler();
	else
		this->cacheHandler = NULL;
}

StatisticsModel::~StatisticsModel()
{
	if ( this->cacheHandler != NULL )
		delete this->cacheHandler;
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

Bool StatisticsModel::useCache( Stretch& stretch, Plane& plane, Double* min, Double* max, Double* mean,
							   Double* stdev, Double* median, Vector<Double>& histogram, Double* maxBinCount  )
{
	if ( kFITSDoCache )
		return this->cacheHandler->useCache( stretch, plane, min, max, mean, stdev, median, histogram, maxBinCount );
	else
		return false;
}

Void StatisticsModel::storeCache( Stretch& stretch , Vector<Double>& histo, Double maxBinCount, Plane& plane, Double background, Double scale,
								 Double max, Double min, Double mean, Double median, Double stdev)
{
	if ( kFITSDoCache )
		this->cacheHandler->storeCache( stretch, histo, maxBinCount, plane, background, scale, max, min, mean, median, stdev );
}

Void StatisticsModel::clearCache()
{
	this->cacheHandler->clearCache();
}

/**
 *
 */
Double StatisticsModel::getRealMin() {
	return realMin;
}

/**
 *
 */
Double StatisticsModel::getRealMax() {
	return realMax;
}

/**
 *
 */
Double StatisticsModel::getRealMean() {
	return realMean;
}

/**
 *
 */
Double StatisticsModel::getRealMedian() {
	return realMedian;
}

/**
 *
 */
Double StatisticsModel::getRealSTDEV() {
	return realSTDEV;
}

/**
 *
 */
Double StatisticsModel::getScaledMin() {
	return scaledMin;
}

/**
 *
 */
Double StatisticsModel::getScaledMax() {
	return scaledMax;
}

/**
 *
 */
Double StatisticsModel::getScaledMean() {
	return scaledMean;
}

/**
 *
 */
Double StatisticsModel::getScaledMedian() {
	return scaledMedian;
}

/**
 *
 */
Double StatisticsModel::getScaledSTDEV() {
	return scaledSTDEV;
}

/**
 *
 */
Double StatisticsModel::getStretchMin() {
	return stretchMin;
}

/**
 *
 */
Double StatisticsModel::getStretchMax() {
	return stretchMax;
}

/**
 *
 */
Double StatisticsModel::getStretchMean() {
	return stretchMean;
}

/**
 *
 */
Double StatisticsModel::getStretchMedian() {
	return stretchMedian;
}

/**
 *
 */
Double StatisticsModel::getStretchSTDEV() {
	return stretchSTDEV;
}