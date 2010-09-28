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
#include "RepositoryController.h"
#include "TextUtils.h"
#include "Resources.h"

#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace boost::algorithm;

using namespace FitsLiberator;
using namespace FitsLiberator::Engine;
using namespace FitsLiberator::Modelling;

const char* formats[] = {
    "^\\s*([\\-\\+]?\\d+)(\\.\\d*)?([eE][\\-\\+]\\d+)?\\s*$",       // Real
    "^\\s*(\\d{4})\\-(\\d{2})\\-(\\d{2})\\s*$",                     // Date
    "^\\s*(\\d{4})\\-(\\d{2})\\-(\\d{2})(T\\d{2}:\\d{2})?\\s*$",    // DateTime
    "^\\s*\\d{2}:\\d{2}\\s*$",                                      // Time
    "^\\s*http(s?)://.+\\s*$",                                      // Url host
    "^\\s*http(s?)://.+\\s*$",                                      // Url resource
    "^\\s*[0-9a-zA-Z\\.]+://.+\\s*$",                               // Ivo host
    "^\\s*[0-9a-zA-Z\\.]+://.+\\s*$",                               // Ivo identifier
    "^\\s*[0-9a-zA-Z\\.]+://.+\\s*$"                                // Ivo resource
};

const double PI = 4.0*atan(1.0);

double degrees(double radians) {
	return radians * 180.0 / PI;
}

double radians(double degrees) {
	return degrees * PI / 180.0;
}

RepositoryController::RepositoryController( RepositoryModel& m, AccumulatingChangeManager* chman,
										   ImageReader* r ) 
  : super( chman ), model( m ), reader( r ) {
	singleUpdate = true;
}

Void RepositoryController::parseHeaders( UInt imageIndex, Bool shouldFlip, Bool isFlipped ) {
    applyRules(imageIndex);
    applyWCS(imageIndex, shouldFlip, isFlipped);
	singleUpdate = false;
    super::SendNotifications();
	singleUpdate = true;
}

Void RepositoryController::loadValue( String keywordName, String keywordValue ) {
    Int keywordIndex = model.findKeyword( keywordName );
    if( keywordIndex >= 0 ) {
        const Keyword* keyword = model.getKeyword( keywordIndex );
        if( keyword->flags & Keyword::FlagsRetained ) {
            model.setValue( keywordIndex, keywordValue );
        }
        else if( keyword->flags & Keyword::FlagsRetainedHint ) {
            model.addCompletion( keywordIndex, keywordValue );
        }
    }
}

String RepositoryController::saveValue( const Keyword* keyword ) const {
    if( keyword->value.length() == 0 ) {
        // The user ommitted this field, see if there is an old entry to save instead
        if( keyword->completions.size() > 0 && (keyword->flags & Keyword::FlagsRetainedHint) ) {
            return keyword->completions[keyword->completions.size() - 1];
        }
        return String();
    }
    return keyword->value;
}

Bool RepositoryController::shouldSave( const Keyword* keyword ) const {
    return (0 != (keyword->flags & (Keyword::FlagsRetained | Keyword::FlagsRetainedHint)) );
}

ValidationError RepositoryController::parseValue( UInt keywordIndex, String value ) {
    // Validate input
    if( keywordIndex < 0 || keywordIndex >= model.getCount() )
        return NoError;

    // You can always set an empty value or a string value
    if( value.length() == 0 ) {
        model.setValue( keywordIndex, value );
        return NoError;
    }

    // We may need to do some validation.
    const Keyword* keyword = model.getKeyword( keywordIndex );
	if( keyword->type <= 1 ) {
		model.setValue( keywordIndex, value );
		return NoError;
	}
	else {
		boost::regex e( formats[keyword->type - 2] );
		
		if( keyword->elements > 1 ) {
			vector<string> elements;
            split(elements, value, is_any_of(";"));
			if( elements.size() == keyword->elements ) {
				for( Vector<String>::iterator i = elements.begin(); i != elements.end(); i++ ) {
					if( !boost::regex_match( *i, e ) )
						return (ValidationError)keyword->type;
				}
				model.setValue( keywordIndex, value );
				super::SendNotifications();
				return NoError;
			} else {
				return InvalidElementCount;
			}
		}
		else if( boost::regex_match( value, e ) ) {
			model.setValue( keywordIndex, value );
			super::SendNotifications();
			return NoError;
		}
		else {
			return (ValidationError)keyword->type;
		}
	}
}

String RepositoryController::serializeValue( const Keyword* keyword ) const {
    return keyword->value;
}

String RepositoryController::serializeValue( UInt keywordIndex ) const {
    const Keyword* keyword = model.getKeyword( keywordIndex );
    return serializeValue( keyword );
}

RepositoryController::FileHeaderSource::FileHeaderSource( const ImageCube* image )
  : cube( image ) {

}

String RepositoryController::FileHeaderSource::getHeader( const String& name ) const {
	return cube->Property(name);
}

Bool RepositoryController::isSingleUpdate() const {
	return singleUpdate;
}

void
RepositoryController::applyRules(UInt imageIndex) {
	FileHeaderSource source( (*reader)[imageIndex] );

    applyRules(source, model.getRules());
    applyRules(source, model.getRules(reader->format()));
}

void
RepositoryController::applyRules(const HeaderSource& source, const RuleList& rules) {
    for(RuleList::const_iterator i = rules.begin(); i != rules.end(); ++i) {
        applyRule(source, *i);
    }
}

void
RepositoryController::applyRule(const HeaderSource &source, const Rule *rule) {
    Int    keywordIndex;
    String value;

    keywordIndex = model.findKeyword( rule->getKeyword() );
    if( keywordIndex >= 0 ) {
        value = rule->getValue(source);
        if(value.size() > 0) {
            model.setValue( keywordIndex, value);
        }
    }    
}

void
RepositoryController::applyWCS(UInt imageIndex, bool shouldFlip, bool isFlipped) {
	volatile bool hasCDMatrix, hasScaleRotation, hasRef;
	double cd[4];
	double rot, scale[2];
	double ref[2];

	const ImageCube* cube = (*reader)[imageIndex];
	assert(cube != 0);

	hasCDMatrix = cube->NumericProperty("CD1_1", &(cd[0]));
	hasCDMatrix = cube->NumericProperty("CD1_2", &(cd[1])) && hasCDMatrix;
	hasCDMatrix = cube->NumericProperty("CD2_1", &(cd[2])) && hasCDMatrix;
	hasCDMatrix = cube->NumericProperty("CD2_2", &(cd[3])) && hasCDMatrix;

	hasScaleRotation = cube->NumericProperty("CDELT1", &(scale[0]));
	hasScaleRotation = cube->NumericProperty("CDELT2", &(scale[1])) && hasScaleRotation;
	
	if( !cube->NumericProperty("CROTA2", &rot) ) {
		if( !cube->NumericProperty("CROTA1", &rot) ) {
			if( !cube->NumericProperty("CROT", &rot) ) {
				hasScaleRotation = false;
			}
		}
	}
		
	hasRef = cube->NumericProperty("CRPIX1", &(ref[0]));
	hasRef = cube->NumericProperty("CRPIX2", &(ref[1])) && hasRef;

	if( hasCDMatrix && !hasScaleRotation ) {
		double det = cd[0] * cd[3] - cd[1] * cd[2];
		// NOTE: C++ math library defines ATAN2 as ATAN2(Y,X) while e.g. Excel defines it as ATAN2(X,Y)
		scale[0] = det > 0.0 ? sqrt(pow(cd[0],2) + pow(cd[2],2)) : -sqrt(pow(cd[0],2) + pow(cd[2],2));
		scale[1] = sqrt(pow(cd[1],2) + pow(cd[3],2));
		
		double rota, rotb;
		rota = (cd[0] * cd[3] > 0.0)
			? degrees(atan2(cd[2], cd[0]))
			: degrees(atan2(-cd[2], -cd[0]));
		rotb = degrees(atan2(-cd[1], cd[3]));
		rot = (rota + rotb)/2;

		hasScaleRotation = true;
	}

	if( hasScaleRotation ) {
		// Is the image positive parity, then correct using WCS Note Section 6.3.4?
		// In the table below the columns denote the parity extracted from the 
		// metadata and the rows denote the parity derived from the checkbox in
		// the user interface (which may me operated by the user).
		// 
		//                           Metadata
		// D            | Negative | Positive | isFlipped
		// i -----------|----------|----------|-----------
		// s Negative   |          |          | true
		// p -----------|----------|----------|-----------
		// l Positive   |    X     |    X     | false
		// a -----------|----------|----------|-----------
		// y shouldFlip |   true   |  false   |  
		// 
		if( !isFlipped ) {
			if( hasRef ) {
				ref[1] = cube->Height() - ref[1];
			}
			scale[0] = -scale[0];
			rot = (rot < 0) ? rot + 180.0 : rot - 180.0;
		}

		// Correct signs, so they follow AVM convention, which is that Scale[2]
		// should be positive, and Scale[1] defines the parity of the image.
		if( scale[1] < 0.0 ) {
			scale[0] = -scale[0];
			scale[1] = -scale[1];			
			rot = (rot < 0) ? rot + 180.0 : rot - 180.0;
		}
		
		model.setValue(model.findKeyword("Spatial.Scale"), TextUtils::doubleToString(scale[0],14) + ";" + TextUtils::doubleToString(scale[1],14));
		model.setValue(model.findKeyword("Spatial.Rotation"), TextUtils::doubleToStringUnfixed(rot));
	}

	if( hasRef ) {
		model.setValue(model.findKeyword("Spatial.ReferencePixel"), TextUtils::doubleToStringUnfixed(ref[0]) + ";" + TextUtils::doubleToStringUnfixed(ref[1]));
	}
}


Void RepositoryController::updateReader( FitsLiberator::Engine::ImageReader* r )
{
	this->reader = r;
}