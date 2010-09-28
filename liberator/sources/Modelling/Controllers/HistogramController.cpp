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
#include "HistogramController.h"

using namespace FitsLiberator::Modelling;


HistogramController::HistogramController( HistogramModel& h, AccumulatingChangeManager* chman ) 
: histoModel( h ), ACMController( chman )
{

}




//=================================================
//Methods for interaction with the levels
//=================================================

Void HistogramController::incrementBlackLevel( Bool up, Bool big )
{
    Double incr = histoModel.getLevelIncrementation();
    if( big )
        incr *= kFITSBigIncrementFactor;
    if( !up )
        incr = -incr;

    this->setBlackLevel( histoModel.getBlackLevel() + incr );

}

Void HistogramController::setBlackLevel(Double d)
{
	histoModel.setBlackLevel( d );
}

Void HistogramController::moveBlackLevelSlider( Int pos, Double stretchMin, Double stretchMax )
{
	pushHistogram( pos, pos - histoModel.getBlackSliderPos(), stretchMin, stretchMax );
	
	histoModel.setBlackSliderPos( pos );
	
}

Void HistogramController::incrementWhiteLevel( Bool up, Bool big ) {
    Double incr = histoModel.getLevelIncrementation();
    if( big )
        incr *= kFITSBigIncrementFactor;
    if( !up )
        incr = -incr;

    this->setWhiteLevel( histoModel.getWhiteLevel() + incr );

}

Void HistogramController::setWhiteLevel(Double d)
{
	histoModel.setWhiteLevel( d );
}

/**
	Moves the white level slider to @param pos
*/
Void HistogramController::moveWhiteLevelSlider( Int pos, Double stretchMin, Double stretchMax )
{
	pushHistogram( pos, pos - histoModel.getWhiteSliderPos(), stretchMin, stretchMax );

	histoModel.setWhiteSliderPos( pos );
}


//==================================================
//Methods for interaction with the actual histogram
//==================================================

Void HistogramController::pushHistogram( Int pos, Int delta, Double stretchMin, Double stretchMax )
{
	Int hisWidth = histoModel.getHistogramSize().width;

	if ( ( pos > hisWidth - 20 && delta > 0) || ( pos < 20 && delta < 0 ))
		move( -delta *2, stretchMin, stretchMax );
}

Void HistogramController::showAll( Double stretchMin, Double stretchMax )
{
	this->updateHistogram( stretchMin, stretchMax );
}


/**
 * Update the histogram - this will show the entire histogram,
 * but not send any notifications. For use from when e.g. another plane
 * is selected.
 */
Void HistogramController::updateHistogram( Double rangeMin, Double rangeMax )
{
	
	histoModel.histoRangeMax	= rangeMax;
	histoModel.histoRangeMin	= rangeMin;
	
	histoModel.setMinZoomValue();
	this->generateHistogramBins( rangeMin, rangeMax );	
	this->calculateIncrementFactor();
	
}

Void HistogramController::move( Int delta, Double stretchMin, Double stretchMax )
{
	
	Double off		= (Double)(histoModel.getOffset());
	Double invZoom	= 1.0 / histoModel.getCurrentZoom();

	Int newOffset	= (Int)FitsLiberator::Engine::FitsMath::round(off - delta * invZoom);

	Double hisWidth	= (Double)(histoModel.getHistogramSize().width);
	Double rangeMax = histoModel.getRangeMax();
	Double rangeMin = histoModel.getRangeMin();

	Double rD	= (Double)delta * (rangeMax - rangeMin) / (hisWidth);
	
	if (rangeMax - rD <= stretchMax && rangeMin - rD >= stretchMin)
	{
		histoModel.histoRangeMax -= rD;
		histoModel.histoRangeMin -= rD;

		histoModel.setOffset( newOffset );
		this->generateHistogramBins( stretchMin, stretchMax );		
	}
	
}

/**
Should be called with transfered values
When a stretch alteres the range this function is used
*/
Void HistogramController::refreshHistogram( Double rangeMin, Double rangeMax, 
											Double strMin, Double strMax )
{
	if ( rangeMin < strMin )
		rangeMin = strMin;

	if ( rangeMax > strMax )
		rangeMax = strMax;

	this->histoModel.histoRangeMin = rangeMin;
	this->histoModel.histoRangeMax = rangeMax;
	this->refreshHistogram( strMin, strMax );
}

/**
Redraws the histogram when e.g. a stretch function has altered the
range
*/
Void HistogramController::refreshHistogram( Double strMin, Double strMax )
{
	this->generateHistogramBins( strMin, strMax );	
	this->calculateIncrementFactor();	
}

/**
	Scales the histogram to the given range without considering the zoom factor...
*/
Void HistogramController::showRange(UInt left, UInt right, Double min, Double max )
{
	Double hisWidth					= (Double)(histoModel.getHistogramSize().width);
	
	Double nMax						= histoModel.getVal(right);
	Double nMin						= histoModel.getVal(left);

	Double leftRawBin				= getRawBin(nMin, min, max );
	Double rightRawBin				= getRawBin(nMax, min, max );

	//makes sure that we don't zoom a range that is too small
	if (rightRawBin - leftRawBin >= hisWidth)
	{
		histoModel.histoRangeMax	= nMax;
		histoModel.histoRangeMin	= nMin;

		//sets the zoom factor
		histoModel.setZoomValue( hisWidth / (rightRawBin - leftRawBin) );
		//set the new offset
		histoModel.offset			= (UInt)leftRawBin;

		this->generateHistogramBins( min, max );
		this->calculateIncrementFactor();	
	}
}

Void HistogramController::incrementZoom( UInt coord, Double stretchMin, Double stretchMax )
{

    if(histoModel.incrementZoom())
        centerHistogram( coord, stretchMin, stretchMax );
}

Void HistogramController::decrementZoom( UInt coord, Double stretchMin, Double stretchMax )
{
	if(histoModel.decrementZoom())
        centerHistogram( coord, stretchMin, stretchMax );
}

Void HistogramController::centerHistogram( UInt coord, Double stretchMin, Double stretchMax )
{
	//the old range
	Double rangeMax		= histoModel.getRangeMax();
	Double rangeMin		= histoModel.getRangeMin();

	
	//width of the histogram
	Double histoWidth	= histoModel.getHistogramSize().width;

	//the pixel value of the new centre
	Double newCentre	= (Double)coord / histoWidth * (rangeMax - rangeMin) + rangeMin;

	//zoom factor
	//Double zoom			= model.getReverseZoom();
	Double zoom			= histoModel.getCurrentZoom();

//	Int binHalfRange	= (Int)FitsLiberator::Engine::FitsMath::round(
//						0.5 * zoom * model.rawBins.size()
//							);
	Int binHalfRange	= (Int)FitsLiberator::Engine::FitsMath::round(
						0.5 * 1.0 / zoom * histoModel.histogramSize.width
							);

	Int rawCentre		= getRawBin( newCentre, stretchMin, stretchMax );

	Int leftBin			= rawCentre - binHalfRange;
	Int rightBin		= rawCentre + binHalfRange;

	if ( leftBin < 0 )
	{
		//rightBin	+= -1 * leftBin;
		leftBin		= 0;
		//rightBin	= rawCentre * 2;
	}
	if ( rightBin > (Int)( histoModel.rawBins.size() ) )
	{
		//leftBin		-= (rightBin - histoModel.rawBins.size());
		rightBin	= histoModel.rawBins.size();
		//leftBin		= rightBin - 2 * rawCentre;
	}

	histoModel.histoRangeMax	= (Double)(rightBin) / (Double)histoModel.rawBins.size() * (stretchMax - stretchMin) + stretchMin;
	histoModel.histoRangeMin	= (Double)(leftBin) / (Double)histoModel.rawBins.size() * (stretchMax - stretchMin) + stretchMin;

	histoModel.offset			= (UInt)(leftBin);

	this->generateHistogramBins( stretchMin, stretchMax );
	this->calculateIncrementFactor();
}

Double HistogramController::getMaxBin()
{
	return histoModel.maxBin;
}

Void HistogramController::setMaxBin(Double d)
{
	histoModel.maxBin = d;
}

/**
 returns the raw bin corresponding to the given val
*/
UInt HistogramController::getRawBin( Double val, Double hMin, Double hMax )
{
	Double nR	= histoModel.getRawBins().size() - 1.; // histoModel.getEndBins().size() - 1.;

	return (UInt)FitsLiberator::Engine::FitsMath::round(((val - hMin) * nR) / (hMax - hMin));
}

void
HistogramController::generateView(double viewMin, double viewMax) {
    
    
}


/**
	Generates a part of the histogram based on the offset according to the raw array and 
	the zoomfactor, with which the histogram should be scaled.
*/
Void HistogramController::generateHistogramBins( Double stretchMin, Double stretchMax )
{
	
	//the output bins
	Vector<Int>& output				= histoModel.getEndBins();
	//the input (raw) bins
	Vector<Double>& input			= histoModel.getRawBins();

	//the range shown at the moment in the histogram
	Double rangeMax					= histoModel.getRangeMax();
	Double rangeMin					= histoModel.getRangeMin();

	//the maximum value any bin takes.
	Double maxVal					= histoModel.getMaxBin();

	//fetches the size of the output window.
	FitsLiberator::Size& histoSize	= histoModel.getHistogramSize();

	//counters
	UInt i = 0;
	UInt j = 0;

	//defines the interval of raw bins to be averaged
	UInt from		= 0;
	UInt to			= 0;

	//the average value for each bin
	Double avVal	= 0;

	//determines the scaleheight of the histogram, so the bin 
	//with the maximum size corresponds to the height of the output window
	Double scale	= (Double)histoSize.height / maxVal;

	Int leftBin		= getRawBin( rangeMin, stretchMin, stretchMax );
	Int rightBin	= getRawBin( rangeMax, stretchMin, stretchMax );

	if ( leftBin >= 0 && rightBin < input.size() && leftBin < rightBin )
	{
		//outer loop runs through each output bin
		for (j = 0; j < output.size(); j++)
		{
			from	= (Int)FitsLiberator::Engine::FitsMath::round((Double)j / (Double)output.size() * (Double)(rightBin-leftBin) + leftBin);
			to		= (Int)FitsLiberator::Engine::FitsMath::round((Double)(j+1) / (Double)output.size() * (Double)(rightBin-leftBin) + leftBin);
			Int outVal = 0;
			
			avVal = input[from];
			for (i = from; i < to; i++)
			{
				//scales the current bin and sums the values
				if ( avVal < input[i] )
				{
				//	avVal += input[i];
					avVal = input[i];
				}
			}
			
			outVal = (Int)FitsLiberator::Engine::FitsMath::round(scale * avVal);
			output[j] = outVal;
		}
	}
	histoModel.Notify();
	
}

/**
 *
 */
Void HistogramController::calculateIncrementFactor() {
	FitsLiberator::Size histSize = histoModel.getHistogramSize();
	
	histoModel.setLevelIncrementation( ( histoModel.getRangeMax() - histoModel.getRangeMin() ) / histSize.width / 10. );
}