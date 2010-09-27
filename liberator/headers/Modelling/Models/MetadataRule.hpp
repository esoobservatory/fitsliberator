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
#ifndef __METADATARULE_HPP__
#define __METADATARULE_HPP__ 

#include "FitsLiberator.h"
#include "tinyxml.h"

using std::string;

namespace FitsLiberator {
	namespace Modelling {
		/** Source of header values. */
		class HeaderSource {
		public:
			virtual ~HeaderSource() {}
			/** Returns the value of a FITS header.
				@param name Name of the header.
				@return The value of the header. If the value doesn't exist and empty string is returned. */
			virtual string getHeader( const string& name ) const = 0;
		};

		/** Rules are used to map FITS header values to metadata keywords. */
		class Rule {
		protected:
			/** Metadata keyword this rule applies to. */
			string keyword;
			/** Default constructor.
				@param keyword Metadata keyword this rule applies to. */
			Rule( const string& keyword );
		public:
			virtual ~Rule() {}
			/** Applies the rule.
				@param source Source of FITS header values.
				@return The new value for the keyword. */
			virtual string getValue( const HeaderSource& source ) const = 0;
			/** @return Gets the keyword this rules applies to.*/
			string getKeyword() const;
			/** Constructs a rule from an XML element.
				@param definition XML element to read the definition from.
				@return A valid rule, or NULL if the XML definition was invalid. */
			static Rule* Create( const TiXmlElement* definition );
		};

		class SetRule : public Rule {
			typedef Rule super;
		protected:
			/** Metadata keyword value. */
			string value;
		public:
			/** Default constructor.
				@param keyword Metadata keyword this rule applies to.
				@param value Value to set. */
			SetRule( const string& keyword, const string& value );
			virtual ~SetRule() {}
			virtual string getValue( const HeaderSource& source ) const;
			/** Constructs a rule from an XML element.
				@param definition XML element to read the definition from.
				@return A valid rule, or NULL if the XML definition was invalid. */
			static SetRule* Create( const TiXmlElement* definition );
		};

		/** This rule type is a simple mapping between FITS header and keyword. */
		class MapRule : public Rule {
			typedef Rule super;
		protected:
			/** Header to read from. */
			std::vector<std::string> headers;
		public:
			/** Default constructor.
				@param keyword Metadata keyword this rule applies to.
				@param header Header to read from. */
			MapRule( const string& keyword, const string& header );
			virtual ~MapRule() {}
			virtual string getValue( const HeaderSource& source ) const;
			/** Constructs a rule from an XML element.
				@param definition XML element to read the definition from.
				@return A valid rule, or NULL if the XML definition was invalid. */
			static MapRule* Create( const TiXmlElement* definition );
		};

		class VectorRule : public Rule {
			typedef Rule super;
		protected:
			/** Header prefix to read from. */
			std::vector<std::string> headers;
		public:
			/** Default constructor.
				@param keyword Metadata keyword this rule applies to.
				@param header Header to read from. */
			VectorRule( const string& keyword, const string& header );
			virtual ~VectorRule() {}
			virtual string getValue( const HeaderSource& source ) const;
			/** Constructs a rule from an XML element.
				@param definition XML element to read the definition from.
				@return A valid rule, or NULL if the XML definition was invalid. */
			static VectorRule* Create( const TiXmlElement* definition );
		};

		class TransformRule : public MapRule {
			typedef MapRule super;
		protected:
			string match;
			string format;
		public:
			/** Default constructor.
				@param keyword Metadata keyword this rule applies to.
				@param header Header to read from.
				@param match A regular expression to match the header value with. 
				@param format A regular expression used for format the ouput. */
			TransformRule( const string& keyword, const string& header, const string& match, const string& format );
			virtual ~TransformRule() {}
			virtual string getValue( const HeaderSource& source ) const;
			/** Constructs a rule from an XML element.
				@param definition XML element to read the definition from.
				@return A valid rule, or NULL if the XML definition was invalid. */
			static TransformRule* Create( const TiXmlElement* definition );
		};

        typedef Vector<String*>     CategoryList;
        typedef Vector<const Rule*>	RuleList;

	} // end namespace Modelling
} // end namespace FitsLiberator

#endif // __METADATARULE_HPP__