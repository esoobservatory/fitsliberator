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
 
#include "TextControl.h"
 
using namespace FitsLiberator::Mac;

//-------------------------------------------------------------------------------
//	BevelButton implementation
//-------------------------------------------------------------------------------

BevelButtonControl::BevelButtonControl( BaseDialog *dlog, Int Id, OSType t ) 
: iconType( t ), icon( NULL ), BaseControl( dlog, Id ) {
	
}

BevelButtonControl::~BevelButtonControl(  ) {
	if( icon != NULL ) {
		//::UnregisterIconRef( kFITSApplicationSignature, iconType );		
	}
}


Void BevelButtonControl::setIcon( CFStringRef filename, BundleFactory* factory ) {
	CFURLRef 	url;
	FSRef 		fileRef;
	CFBundleRef	bundle = factory->getCFBundleRef();
	
	// Find icon file in resource directory.
	url = ::CFBundleCopyResourceURL( bundle, filename, NULL, NULL );
	
	if( ::CFURLGetFSRef( url, &fileRef ) != true ) {
		throw Exception( "Couldn't read icon file for bevel button." );
	}

	::CFRelease( url );
	
	IconFamilyHandle iconFamily;
	::ReadIconFromFSRef( &fileRef, &iconFamily );
	IconSuiteRef iconSuite;
	::IconFamilyToIconSuite( iconFamily, kSelectorAllAvailableData, &iconSuite );
	
	// Register the icon
	//if( noErr != ::RegisterIconRefFromFSRef( kFITSApplicationSignature, iconType, &fileRef, &icon ) ) {
	//	throw Exception( "Couldn't register icon file for bevel button." );
	//}
	
	ControlButtonContentInfo contentInfo;
	
	//contentInfo.u.iconRef = icon;
	//contentInfo.contentType = kControlContentIconRef;
	contentInfo.u.iconSuite = iconSuite;
	contentInfo.contentType = kControlContentIconSuiteHandle;
	
	Boolean scale = true;
	
	// Add the icon  to the bevel button.
	setData( kControlButtonPart, kControlBevelButtonContentTag, sizeof( ControlButtonContentInfo ), &contentInfo );
	setData( kControlEntireControl, kControlBevelButtonScaleIconTag, sizeof( Boolean ), &scale );
	
	// Redraw the button.
	drawControl();
}


//-------------------------------------------------------------------------------
//	TextControl implementation
//-------------------------------------------------------------------------------

TextControl::TextControl( BaseDialog *dlog, Int Id ) : BaseControl( dlog, Id ) {
	
}

/**
 * Set the text of a StaticText or EditText control
 */
Void TextControl::setText( String &text, ControlPartCode part ) {	
	setData( part, kControlEditTextTextTag, text.length(), text.c_str() );
}

/**
 * Get the text of a StaticText or EditText control
 */
String TextControl::getText(ControlPartCode part) {
	::Size len;
	
	// Note: You are supposed to use kControlStaticTextTextTag to get the text
	// from a StaticText control, but it's equal to kControlEditTextTextTag ('text')
	// so it works. 
	getDataSize( part, kControlEditTextTextTag, &len );
	
	Char *buffer = new Char[len+1];
	getData( kControlEntireControl, kControlEditTextTextTag, buffer, len, NULL );
	buffer[len] = '\0';
	
	String text;
	
	if( buffer != NULL ) {	
		text = (Char *) buffer;
		delete buffer;
	}
	
	return text;
}

//-------------------------------------------------------------------------------
//	NumericTextControl implementation
//-------------------------------------------------------------------------------

/**
 * Constructor for NumericTextControl. 
 */
NumericTextControl::NumericTextControl( BaseDialog *dlog, Int Id ) : TextControl( dlog, Id ) {
	setPrecision( 2 );
}

/**
 * Set the used precision.
 */
Void NumericTextControl::setPrecision( Int p ) {
	precision = p;
}

/**
 * Get the used precision.
 */
Int NumericTextControl::getPrecision() {
	return precision;
}

/**
 * Set an int value. Just a wrapper around setDouble().
 */
Void NumericTextControl::setInt( Int value ) {
	setDouble( (Double) value );
}

/**
 * Get the text as an integer value.
 */
Int NumericTextControl::getInt() {
	return FitsLiberator::TextUtils::stringToInt( getText() );
}

/**
 * Set the text of the control as an doble value.
 */
Void NumericTextControl::setDouble( Double value ) {
	String text = FitsLiberator::TextUtils::doubleToString( value, precision );
	setText( text );
}

/**
 * Get the text as an double value.
 */
Double NumericTextControl::getDouble() {
	return FitsLiberator::TextUtils::stringToDouble( getText() );
}

//-------------------------------------------------------------------------------
//	UnitsNumericTextControl implementation
//-------------------------------------------------------------------------------

/**
 * Constructor for units text control.
 */
UnitsNumericTextControl::UnitsNumericTextControl( BaseDialog *dlog, Int Id ) : NumericTextControl( dlog, Id )  {
}


/**
 * Overrides TextControl 's setText, and appends a unit add the text (e.g. px, cm etc.).
 */
Void UnitsNumericTextControl::setText( String &text, ControlPartCode part ) {
	FitsLiberator::TextUtils::appendUnitText( text, unitText );
	NumericTextControl::setText( text, part );
}

/**
 * Overrides TextControl 's getText, and removes any units from the string.
 */
String UnitsNumericTextControl::getText( ControlPartCode part ) {
	String text = NumericTextControl::getText( part );
	
	FitsLiberator::TextUtils::removeUnitText( text, unitText );

	return text;
}

/**
 * Set the used units.
 */
Void UnitsNumericTextControl::setUnits( const Char *text ) {
	unitText = text;
}

/**
 * Get the used units.
 */
String UnitsNumericTextControl::getUnits() {
	return unitText;
}