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
#include "StretchController.h"

using namespace FitsLiberator::Modelling;
using namespace FitsLiberator::Engine;

StretchController::StretchController( StretchModel& m ) : stretchModel( m )
{

}
StretchController::~StretchController() {
}




Void StretchController::defaultValues()
{
	this->stretchModel.setBackground ( 0.0 );
	this->stretchModel.setScale ( 1.0 );
	this->stretchModel.setFunction( stretchLinear );
	this->stretchModel.setRescaleFactor( 1.0 );
	this->stretchModel.setPeakLevel( 1.0 );
}


//==========================================================
// Methods for interaction with the background level
//==========================================================

/**
 * Increments/decrements the background level
 * @param up Direction, true to increment false to decrement.
 * @param big Size of the increment big or small.
 */
Void StretchController::incrementBackgroundLevel( Bool up, Bool big ) {  
    Double incr = this->stretchModel.getBackgroundIncrementation();
    if( big )
        incr *= kFITSBigIncrementFactor;
    if( !up )
        incr = -incr;

}

/**
*	Sets the background level.
*/
Bool StretchController::setBackgroundLevel( Double d, const Stretch& stretch )
{	
	Double peak			= this->stretchModel.getPeakLevel();

	Double oldVal		= this->stretchModel.getBackground();

	//checks the boundaries. PeakLevel - BackgroundLevel > 0
	if ( peak - d > 0.0 )
	{
		//then we should update the background
		this->stretchModel.setBackground ( d );	

		//should update the scale factor with rescaleFactor / (peakLevel - backgroundlevel)
		Double rescale	= this->stretchModel.getRescaleFactor();
		this->stretchModel.setScale( rescale / ( peak - d ) );

		return true;
	}
	this->stretchModel.setBackground( oldVal );
	return false;
}
Double StretchController::getRescaleFactor()
{
	return this->stretchModel.getRescaleFactor();
}

Double StretchController::getPeakLevel()
{
	return this->stretchModel.getPeakLevel();
}

/**
*	Transfers the input levels given the specified stretch
*/
Void StretchController::transferLevels( Double* blackLevel, Double* whiteLevel, const Stretch& stretch )
{
	//we should transfer the values of the black and white levels,
	//which are in the default linear regime.

    FitsEngine::stretchRealValues(stretch, blackLevel, blackLevel, 1);
    FitsEngine::stretchRealValues(stretch, whiteLevel, whiteLevel, 1);
}

//returns the background level
Double StretchController::getBackgroundLevel()
{
	return this->stretchModel.getBackground();
}

//========================================================
// Methods for interaction with the scale factor
//========================================================

/**
 * @see StretchControll::incrementBackgroundLevel
 */
Void StretchController::incrementScale( Bool up, Bool big ) {
    Double incr = this->stretchModel.getScaleIncrementation();
    if( big )
        incr *= kFITSBigIncrementFactor;
    if( !up )
        incr = -incr;

//    this->setScale( this->getScale() + incr );
}


//returns the scale value
Double StretchController::getScale()
{
	return this->stretchModel.getScale();
}

