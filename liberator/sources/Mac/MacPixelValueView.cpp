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
 
#include "MacPixelValueView.h"

using namespace FitsLiberator::Mac;
using namespace FitsLiberator::Modelling;

/**
 *
 */
MacPixelValueView::MacPixelValueView( BaseDialog * dialog, FitsLiberator::Modelling::PixelValueModel& m , FitsLiberator::Modelling::GlobalSettingsModel& gm ) : PixelValueView( m, gm ) {
	xItem = new UnitsNumericTextControl( dialog, kFITSUITextX );
	xItem->setUnits( kFITSUIUnitsPixels );
	xItem->setPrecision( 0 );
			
	yItem = new UnitsNumericTextControl( dialog, kFITSUITextY );
	yItem->setUnits( kFITSUIUnitsPixels );
	yItem->setPrecision( 0 );
	
	valueItem = new NumericTextControl( dialog, kFITSUITextReal );
	valueItem->setPrecision( kFITSDefaultPrecision );
	
	stretchItem = new NumericTextControl( dialog, kFITSUITextStretched );
	stretchItem->setPrecision( kFITSDefaultPrecision );
	
	textItem = new TextControl( dialog, kFITSUITextPixelStretchedScaled );
	
	raItem = new NumericTextControl( dialog, kFITSUITextRa );
	decItem = new NumericTextControl( dialog, kFITSUITextDec );
	
	updateXValue( 0 );
	updateYValue( 0 );
	updateRealValue( 0 );
	updateStretchValue( 0 );
		
	updateShowValues( globalSettingsModel.getPreviewEnabled() );
}

/**
 *
 */
MacPixelValueView::~MacPixelValueView()  {
	if( xItem != NULL ) 		{ delete xItem; }
	if( yItem != NULL ) 		{ delete yItem; }
	if( valueItem != NULL ) 	{ delete valueItem; }
	if( stretchItem != NULL ) 	{ delete stretchItem; }
	if( textItem != NULL )		{ delete textItem; }
	if( raItem != NULL )		{ delete raItem; }
	if( decItem != NULL)		{ delete decItem; }
}

Void MacPixelValueView::updateRa( Double value ) {
	raItem->setDouble( value );
}

Void MacPixelValueView::updateDec( Double value ) {
	decItem->setDouble( value );
}

/**
 * 
 *
 * @param value 
 */
Void MacPixelValueView::updateXValue( Int value ) {
	xItem->setInt( value );
}

/**
 * 
 *
 * @param value 
 */
Void MacPixelValueView::updateYValue( Int value ) {
	yItem->setInt( value );
}

/**
 * 
 *
 * @param value 
 */
Void MacPixelValueView::updateRealValue( Double value ) {
	valueItem->setDouble( value );
}

/**
 * 
 *
 * @param value 
 */
Void MacPixelValueView::updateStretchValue( Double value ) {
	stretchItem->setDouble( value );
}

/**
 * 
 *
 * @param value 
 */
Void MacPixelValueView::updateScaledValue( Double value ) {
	stretchItem->setDouble( value );
}

/**
 * 
 *
 * @param value 
 */
Void MacPixelValueView::updateText( String& value ) {
	textItem->setText( value );
}

/**
 * 
 *
 * @param value 
 */
Void MacPixelValueView::updateShowValues( Bool value ) {
	raItem->setVisibility( value, true );
	decItem->setVisibility( value, true );
	xItem->setVisibility( value, true );
	yItem->setVisibility( value, true );
	valueItem->setVisibility( value, true );
	stretchItem->setVisibility( value, true );
}