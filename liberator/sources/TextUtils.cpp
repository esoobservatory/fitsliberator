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

/** @file
 * Text utilities
 */
#include "TextUtils.h"
#include "Environment.h"
#include <cmath>
#include <cstdlib>
#include <sstream>
#include <iomanip>

using namespace FitsLiberator;


/**
 * Converts a numeric string representation to a double. 
 *
 * @param text	Numeric string.
 * @return 		Double value of string.
 */
Double TextUtils::stringToDouble( const String &text ) {
	return std::atof( text.c_str() );
}

/**
 * Converts a numeric string representation to an integer. 
 *
 * @param text	Numeric string.
 * @return 		Double value of string.
 */
Int TextUtils::stringToInt( const String &text ) {
	return (Int) FitsLiberator::Engine::FitsMath::round( TextUtils::stringToDouble(text) );
}

/**
 * Converts a double to a numeric string representation. Uses scientific notation for
 * absolute values less than kFITSSmallestRegularNumber and greather than kFITSLargestRegularNumber. 
 *
 * @param num		Double value to convert to string.
 * @param precision	Numbers of decimals.
 * @return 			String representation of num or "Undefined" if num is NaN.
 */
String TextUtils::doubleToString( Double value, Int precision ) {
	std::ostringstream buffer;
	
	if ( !FitsLiberator::Engine::FitsMath::isFinite(value) ) {
		buffer << "N/A";
	} else if ( ( abs( value ) < kFITSSmallestRegularNumber && value != 0 ) || ( abs( value ) > kFITSLargestRegularNumber && value != 0 )) {
		buffer << std::setprecision( precision ) << std::fixed << std::scientific << value;
	} else {
		buffer << std::setprecision( precision ) << std::fixed << value;
	}
	
	return buffer.str(); 
}

String TextUtils::doubleToString( Double value ) {
    return doubleToString( value, Environment::getDecimalPlaces() );
}

String TextUtils::doubleToStringUnfixed( Double value, Int precision ) {
    std::ostringstream buffer;
	
	if ( !FitsLiberator::Engine::FitsMath::isFinite(value) ) {
		buffer << "N/A";
	} else if ( ( abs( value ) < kFITSSmallestRegularNumber && value != 0 ) || ( abs( value ) > kFITSLargestRegularNumber && value != 0 )) {
		buffer << std::setprecision( precision ) << std::scientific << value;
	} else {
		buffer << std::setprecision( precision ) << value;
	}
	
	return buffer.str(); 
}

String TextUtils::doubleToStringUnfixed( Double value ) {
    return doubleToStringUnfixed( value, Environment::getUnfixedDecimalPlaces() );
}

/**
 * Converts a integer to a numeral.
 */
String TextUtils::intToString( Int num ) {
    std::ostringstream buffer;
    buffer << num;
    return buffer.str();
}

/**
 * Appends the content of unitText to text.
 */
Void TextUtils::appendUnitText( String &text, const String &unitText ) {
    text.append( kFITSUnitSeparator );
	text.append( unitText );
}

/**
 * Removes the content of unitText of text
 */
Void TextUtils::removeUnitText( String &text, const String &unitText ) {
	// Trim whitespace from right.
	const String t = kFITSUnitSeparator;
	text.erase ( text.find_last_not_of( t ) + 1 );   

	// That is, the last part of the string equal to the unitText. 
	if( text.substr( text.length() - unitText.length(), unitText.length() ) == unitText ) {
		text.erase( text.length() - unitText.length(), text.length() );
	}
}
 