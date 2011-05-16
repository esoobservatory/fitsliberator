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
#include "HistogramModel.h"

using namespace FitsLiberator::Modelling;

/**
	Constructor, where hiWidth is the width of the histogram

*/
HistogramModel::HistogramModel(ChangeManager * chman) : Model( chman ) 
{
	//the offset should be inited to zero
	this->offset = 0;
	this->rawBins.resize( kFITSHistogramBins );
	this->zoomMax	= 1.0;
	this->zoomMin	= 1.0;
	this->blackLevel = 0.0;
	this->currentBlackLevel = 0.0;
	this->whiteLevel = 0.0;
	this->currentWhiteLevel = 0.0;
	this->histoRangeMin = 0.0;
	this->histoRangeMax = 0.0;
}



//===================================================
//	Methods for interaction with the levels
//===================================================

/**
	
*/
Void HistogramModel::updateLevels(Double b, Double w)
{
	if (b < w)
	{
		this->blackLevel	= b;
		this->whiteLevel	= w;
		
		this->currentBlackLevel = b;
		this->currentWhiteLevel = w;

		Notify();
	}	
}

//returns the black level
Double HistogramModel::getBlackLevel() const
{
	return this->blackLevel;
}

//sets the black level
Void HistogramModel::setBlackLevel(Double value)
{
	if (value < this->whiteLevel )
	{
		this->blackLevel		= value;
		this->currentBlackLevel	= value;
	}
	Notify();
}

Int HistogramModel::getBlackSliderPos()
{
	return getPos( this->currentBlackLevel );
}

Void HistogramModel::setBlackSliderPos(Int p)
{	
	if( p < 0 ) {
		p = 0;	
	} else if( p >= (Int)(histogramSize.width-1) ) {
		p = histogramSize.width - 1;
	}
	
	Double val = getVal( p );
	
	if ( val >= currentWhiteLevel ) {
		currentBlackLevel = currentWhiteLevel;
	} else {
		currentBlackLevel = val;
	}
	Notify();
}
Double HistogramModel::getCurrentBlackLevel()
{
	return currentBlackLevel;
}

//returns the white level
Double HistogramModel::getWhiteLevel() const
{
	return this->whiteLevel;
}

//sets the white level
Void HistogramModel::setWhiteLevel(Double value) {
	if (this->blackLevel < value)
	{
		this->whiteLevel		= value;
		this->currentWhiteLevel	= value;
	}
	Notify();
}


Int HistogramModel::getWhiteSliderPos()
{
	return this->getPos( currentWhiteLevel );
}

Void HistogramModel::setWhiteSliderPos(Int p)
{
	if( p < 0 ) {
		p = 0;	
	} else if( p >= (Int)( histogramSize.width-1 ) ) {
		p = histogramSize.width - 1;
	}
	
	Double val = getVal( p );
	
	if( val <= currentBlackLevel ) {
		currentWhiteLevel = currentBlackLevel;
	} else {
		currentWhiteLevel = val;	
	}
	
	Notify();
}

Double HistogramModel::getCurrentWhiteLevel()
{
	return currentWhiteLevel;
}

//sets the incrementational value for the levels.
Void HistogramModel::setLevelIncrementation( Double d )
{
	this->levelIncrementation = d;
}

//returns the incrementational value for the levels.
Double HistogramModel::getLevelIncrementation() const
{
	return this->levelIncrementation;
}


Void HistogramModel::setSliders(Double b, Double w)
{
	if (b < w)
	{
		currentBlackLevel = b;
		currentWhiteLevel = w;		
		Notify();
	}
}

/**
 * Maps a value to a coordinate.
 * @param value The value to map.
 */
Int HistogramModel::getPositionOf( Double value ) {
    return getPos( value );
}

//==================================
//Utility methods
//==================================
Int HistogramModel::getPos(Double d)
{
	Double rangeMax = getRangeMax();
	Double rangeMin = getRangeMin();

	Int pos;

	if( d < rangeMin ) {
		pos = -1;
	} else if( d > rangeMax ) {
		pos = this->getHistogramSize().width;
	} else {
		pos = (Int)FitsLiberator::Engine::FitsMath::round((this->getHistogramSize().width-1) * (d - rangeMin) / (rangeMax - rangeMin));	
	}

	return pos;
}

/**
	Returns the real "pixel" value corresponding to the given coord place in the histogram
*/
Double HistogramModel::getVal(UInt coord)
{
	Double hisWidth	= (Double)(getHistogramSize().width);
	
	return (Double)coord / hisWidth * (histoRangeMax - histoRangeMin) + histoRangeMin;
}

Void HistogramModel::setNumberOfBins( UInt nBins )
{
	this->rawBins.assign( nBins, 0. );
	this->setZoom();
}

Void HistogramModel::setHistogramSize(FitsLiberator::Size s)
{
	this->histogramSize = s;
	this->endBins.resize( this->histogramSize.width );

	this->setZoom();
}

Void HistogramModel::setZoom()
{
	//determines the minimum zoom factor possible.
	this->zoomMin = (Double)this->histogramSize.width / (Double)this->rawBins.size();
	this->zoomMax = 1.0;
	currentZoomValue = zoomMin;
	this->Notify();
}

Double HistogramModel::getRangeMax()
{
	return this->histoRangeMax;
}
Double HistogramModel::getRangeMin()
{
	return this->histoRangeMin;
}

Vector<Int>& HistogramModel::getEndBins()
{
	return this->endBins;
}

Void HistogramModel::setMinZoomValue()
{
	this->currentZoomValue = this->zoomMin;
	Notify();
}

/**
	Decreases the zoom factor if possible
*/
Bool HistogramModel::incrementZoom()
{
	if ( currentZoomValue * 1.2  <= zoomMax )
		currentZoomValue = currentZoomValue * 1.2;
	else
		currentZoomValue = zoomMax;

	Notify();
	return true;
}

/**
	Increases the zoom factor if possible
*/
Bool HistogramModel::decrementZoom()
{
	if ( currentZoomValue * 0.8 >= zoomMin )
		currentZoomValue = currentZoomValue * 0.8;
	else 
		currentZoomValue = zoomMin;
	
	Notify();
	return true;
	
}

Double HistogramModel::getCurrentZoom()
{
	return this->currentZoomValue;
}

Double HistogramModel::getReverseZoom()
{
	if ( currentZoomValue <= zoomMin )
	{
		return zoomMax;
	}

	if ( currentZoomValue >= zoomMax )
	{
		return zoomMin;
	}

	return 1.0 - currentZoomValue;

}

/**
	Sets the current zoom factor to @param val if val <= 1.0 && val >= minZoom
*/
Void HistogramModel::setZoomValue( Double val )
{
	if ( val <= this->zoomMax && val >= this->zoomMin )
	{
		this->currentZoomValue = val;
		Notify();
	}
}

Void HistogramModel::setOffset(Int o)
{
	this->offset = o;
}

Int HistogramModel::getOffset()
{
	return this->offset;
}


Vector<Double>& HistogramModel::getRawBins()
{
	return this->rawBins;
}


Void HistogramModel::setMaxBin(Double d)
{
	this->maxBin = d;
}

Double HistogramModel::getMaxBin()
{
	return this->maxBin;
}

FitsLiberator::Size& HistogramModel::getHistogramSize()
{
	return this->histogramSize;
}

/**
 * Determines which slider is closest to any given position.
 * @param x Position.
 * @return Returns 1 if the white slider is the closest, 0 if the black slider is the closest.
 */
Int HistogramModel::getClosestSlider( Int x ) {
    return (
        abs( getWhiteSliderPos() - x ) < abs( getBlackSliderPos() - x )
        ) ? 1 : 0;
}