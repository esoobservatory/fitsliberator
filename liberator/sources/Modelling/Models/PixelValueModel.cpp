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
#include "PixelValueModel.h"

using namespace FitsLiberator::Modelling;

/**
 * Default constructor
 */
PixelValueModel::PixelValueModel( ChangeManager* chman ) : 
Model( chman ), x( 0 ), y( 0 ), value( 0 ), stretchedValue( 0 ), scaledValue( 0 ), ra( 0 ), dec( 0 ) {
}

/**
 *	Called when the current position of the 
 *  mouse cursor is changed and is over the
 *  preview area.
 *  @param p the x,y coordinates in preview space
 *  @rValue the real pixel value
 *  @sValue the stretched pixel value
 *  @scValue the scaled pixel value
 *  @raIn the right ascension value
 *  @decIn the declination value
 */
Void PixelValueModel::setPosition( const FitsLiberator::Point& p, const Double rValue, 
								  const Double sValue, const Double scValue, 
								  const Double raIn, const Double decIn ) {
	this->x = p.x;
	this->y = p.y;
	this->value = rValue;
	this->stretchedValue = sValue;
	this->scaledValue = scValue;
	this->ra = raIn;
	this->dec = decIn;
	Notify();
}

/**
 *
 */
Int PixelValueModel::getY() {
	return this->y;
}

/**
 *
 */
Int PixelValueModel::getX() {
    return this->x;
}

/**
 *
 */
Double PixelValueModel::getValue() {
	return this->value;
}

/**
 *
 */
Double PixelValueModel::getStretchedValue() {
	return this->stretchedValue;
}

/**
 *
 */
Double PixelValueModel::getScaledValue() {
	return this->scaledValue;
}

Double PixelValueModel::getRaValue()
{
	return this->ra;
}

Double PixelValueModel::getDecValue()
{
	return this->dec;
}