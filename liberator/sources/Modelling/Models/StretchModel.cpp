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
#include "StretchModel.h"

using namespace FitsLiberator::Modelling;

//================================================================
//	Class StretchModel
//================================================================
StretchModel::StretchModel( ChangeManager * chman ) : Model( chman )
{
	//inits somewhat arbitrary initial values..
	this->setDefaultValues();

}
StretchModel::~StretchModel()
{
}

Void StretchModel::setDefaultPrefs( const FitsLiberator::Preferences::Preferences& prefs )
{
	setDefaultValues();
	
	setFunction( (FitsLiberator::Engine::StretchFunction)(prefs.defaultStretch) );
	setRescaleFactor( prefs.defaultScaledPeak );
}

Void StretchModel::setDefaultValues()
{
	this->default_background		=	0.0;
	this->default_scale				=	1.0;
	this->default_peakLevel			=	1.0;
	this->default_rescaleFactor		=	1.0;
	this->default_scaleBackground	=	kFITSDefaulBackgroundScale;
	this->defaultStretchFunction	=	FitsLiberator::Engine::stretchLinear;

	this->backgroundIncrementation	=	1.0;
	this->scaleIncrementation		=	1.0;
	this->function					=	this->defaultStretchFunction;
	this->background				=	this->default_background;
	this->scale						=	this->default_scale;
	this->rescaleFactor				=	this->default_rescaleFactor;
	this->peakLevel					=	this->default_peakLevel;
	this->scaleBackground			=	this->default_scaleBackground;
}

//==============================================================|
//	Methods for interaction with the stretch function			|
//==============================================================|

/*
Returns an unsigned int corresponding to StretchFunction::
*/
FitsLiberator::Engine::StretchFunction StretchModel::getFunction() const {
	return this->function;
}


/*
	Sets the stretch function
*/
Void StretchModel::setFunction( FitsLiberator::Engine::StretchFunction f) {
//	if ( f != FitsLiberator::Engine::StretchFunction::stretchNoStretch )
//	{
		this->function = f;
//	}
	Notify();
}

/**
	Method sets the function to be the default stretch function and returns it.
*/
FitsLiberator::Engine::StretchFunction StretchModel::setDefaultFunction()
{
	this->setFunction( this->defaultStretchFunction );
	return this->function;
}

/**
	Method sets a new default stretch function
	Should not notify...
*/
Void StretchModel::setDefaultFunction(FitsLiberator::Engine::StretchFunction s)
{
	this->defaultStretchFunction = s;
}


//===================================================
//	Methods for interaction with the background
//===================================================

Double StretchModel::getScaleBackground()
{
	return this->scaleBackground;
}

Void StretchModel::setScaleBackground( Double d )
{
	this->scaleBackground = d;
}

Double StretchModel::getPeakLevel() const
{
	return this->peakLevel;
}

Void StretchModel::setPeakLevel(Double d)
{
	if ( d != 0.0 )
		this->peakLevel = d;

	Notify();
}

//returns the offset
Double StretchModel::getBackground() const
{
	return this->background;
}

//sets the offset
Void StretchModel::setBackground(Double o) { 
	this->background = o;
	
	Notify();
}
/**
	Method sets the offset to the default value
*/
Double StretchModel::setDefaultBackground()
{
	this->setBackground( this->default_background );	
	return this->background;
}

/**
	Method sets a new value for the default offset, but does not use it.
	It is necessary to call setDefaultOffset to use the value.
	Should not notify!
*/
Void StretchModel::setDefaultBackground(Double d)
{
	this->default_background = d;
}

//sets the incrementational value for the offset
Void StretchModel::setBackgroundIncrementation(Double d)
{
	this->backgroundIncrementation = d;
}

//returns the offset incrementational value
Double StretchModel::getBackgroundIncrementation() const
{
	return this->backgroundIncrementation;
}

//===================================================
//	Methods for interaction with the scale
//===================================================

//returns the scale factor
Double StretchModel::getScale() const
{
	return this->scale;
}

//sets the scale factor
Void StretchModel::setScale(Double s) {
	this->scale = s;
	Notify();
}

Double StretchModel::getRescaleFactor() const
{
	return this->rescaleFactor;
}

Void StretchModel::setRescaleFactor(Double d)
{
	if ( d != 0.0 )
		this->rescaleFactor = d;

	Notify();
}


/**
	Method sets the scale to the default value
*/
Double StretchModel::setDefaultScale()
{
	this->setScale( this->default_scale );
	return this->scale;
}
/**
	Method sets a new value for the default scale
*/
Void StretchModel::setDefaultScale(Double d)
{
	this->default_scale = d;
}

//sets the incrementational value for the scale
Void StretchModel::setScaleIncrementation(Double d)
{
	this->scaleIncrementation = d;
}

//returns the incrementational value for the scale
Double StretchModel::getScaleIncrementation() const
{
	return this->scaleIncrementation;
}