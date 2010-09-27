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
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

#include "MetadataRule.hpp"

using namespace std;
using namespace boost::algorithm;

using FitsLiberator::Modelling::HeaderSource;
using FitsLiberator::Modelling::Rule;
using FitsLiberator::Modelling::SetRule;
using FitsLiberator::Modelling::MapRule;
using FitsLiberator::Modelling::VectorRule;
using FitsLiberator::Modelling::TransformRule;

// ------------------------------------------------------------------------------------------------
// Implementation
// ------------------------------------------------------------------------------------------------

Rule::Rule( const string& keyword ) {
	this->keyword = keyword;
}

string Rule::getKeyword() const {
	return this->keyword;
}

Rule* Rule::Create( const TiXmlElement* definition ) {
	if( definition != NULL ) {
		string kind = definition->Value();
		if( kind == "Vector" )
			return VectorRule::Create( definition );
		else if( kind == "Map" )
			return MapRule::Create( definition );
		else if( kind == "Transform" )
			return TransformRule::Create( definition );
		else if( kind == "Set" )
			return SetRule::Create( definition );
	}
	return NULL;
}

SetRule::SetRule( const string& keyword, const string& value )
  : super( keyword ) {
	this->value = value;
}

string SetRule::getValue( const HeaderSource& ) const {
	return this->value;
}

SetRule* SetRule::Create( const TiXmlElement* definition ) {
	const char* value = definition->Attribute( "Value" );
	const char* keyword = definition->Attribute( "Keyword" );
	if( value != NULL && keyword != NULL )
		return new SetRule( keyword, value );
	return NULL;
}

MapRule::MapRule( const string& keyword, const string& header ) 
  : super( keyword ) {
	  
	// Split the keyword list into its parts
	split(headers, header, is_any_of("|"));
}

MapRule* MapRule::Create( const TiXmlElement* definition ) {
	const char* header = definition->Attribute( "Header" );
	const char* keyword = definition->Attribute( "Keyword" );
	if( header != NULL && keyword != NULL )
		return new MapRule( keyword, header );
	return NULL;
}

string MapRule::getValue( const HeaderSource& source ) const {
	string value;

	for( vector<string>::const_iterator i = headers.begin(); i != headers.end(); i++ ) {
		value = source.getHeader( *i );
		if( value.length() > 0 )
			break;
	}

	return value;
}

VectorRule::VectorRule( const string& keyword, const string& header )
  : super( keyword ) {
	
	// Split the KEYWORD1;KEYWORD2;... string into its elements
    split(headers, header, is_any_of(";"));
}

VectorRule* VectorRule::Create( const TiXmlElement* definition ) {
	const char* header = definition->Attribute( "Header" );
	const char* keyword = definition->Attribute( "Keyword" );
	if( header != NULL && keyword != NULL )
		return new VectorRule( keyword, header );
	return NULL;	
}

string VectorRule::getValue( const HeaderSource& source ) const {
	string value;
	
	bool first = true;
	for(Vector<string>::const_iterator i = headers.begin(); i != headers.end(); ++i) {
		string part = source.getHeader(*i);
		if(part.length() == 0) {
			// Oops, the header keyword does not exist, fail by returning an empty string.
			return "";
		}
		
		if(!first) {
			value += ";";
		} else {
			first = false;
		}
		value += part;
	}

	return value;
}

TransformRule::TransformRule(const string& keyword, const string& header, 
                             const string& match, const string& format ) 
  : super( keyword, header ) {
	this->match = match;
	this->format = format;
}

TransformRule* TransformRule::Create( const TiXmlElement* definition ) {
	const char* header = definition->Attribute( "Header" );
	const char* keyword = definition->Attribute( "Keyword" );
	const char* match = definition->Attribute( "Match" );
	const char* format = definition->Attribute( "Format" );

	if( header != NULL && keyword != NULL && match != NULL && format != NULL )
		return new TransformRule( keyword, header, match, format );
	return NULL;
}

string TransformRule::getValue( const HeaderSource& source ) const {
	boost::regex e( match );
	string value = super::getValue( source );
	if( boost::regex_match( value, e ) )
		return boost::regex_replace( value, e, format );
	return "";
}
