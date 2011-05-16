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
#ifndef __REPOSITORYMODEL_H__
#define __REPOSITORYMODEL_H__ 

#include "FitsLiberator.h"
#include "Observer.h"
#include "tinyxml.h"

#include "MetadataRule.hpp"
#include "ImageReader.hpp"

namespace FitsLiberator {
	namespace Modelling {
        /** Represents a keyword for the repository/metadata part of the plugin. */
        struct Keyword {
        public:
            enum Flags {
                FlagsNone           =   0,  ///< Simple keyword
                FlagsReadonly       =   1,  ///< The keyword is to be considered readonly (GUI hint)
                FlagsRequired       =   2,  ///< The keyword must be filled out (GUI hint)
		        FlagsList		    =   4,	///< If the keyword has several elements, serialize it as an unordered list.
		        FlagsOrdered        =   8,	///< If the keyword has several elements, serialize it as an ordered list.
                FlagsHidden         =  16,  ///< The keyword should be hidden (GUI hint)
                FlagsRetained       =  32,  ///< The keyword should be retained as a static value
                FlagsRetainedHint   =  64,  ///< The keyword should be retained, but the retained value should only be shown as a hint (GUI hint)
				FlagsDontSerialize  = 128,	///< The keyword should not be serialized to XMP in the regular AOI schema.
				FlagsAlt			= 256	///< The keyword should be stored in a rdf:Alt element
            };

            enum Type {
                TypeString,				    ///< Default type
                TypeStringCv,			    ///< String with predefined values.
                TypeReal,				    ///< Generic real (validation hint) stored as a numeral
                TypeDate,				    ///< Date (validation hint)
                TypeDateTime,			    ///< Date time (validaion hint)
                TypeTime,				    ///< Time (validation hint)
                TypeUrlID,				    ///< URL host (validation hint)
                TypeUrlResource,		    ///< URL resource (validation hint)
                TypeIvoID,				    ///< IVO ID (validation hint)
                TypeIvoIdentifier,		    ///< IVO identifier
                TypeIvoResource			    ///< IVO resource (validation hint)
            };

            typedef UInt Category;

            Keyword( String, Type, Flags, Category );

            String          name;           ///< Name of the keyword
			String			id;				///< Keyword ID (number)
            String          value;          ///< Value
            String          description;    ///< Description of the keyword
            Type            type;           ///< Type of the keyword, used for validation
            Flags           flags;          ///< Keyword flags
			UInt			elements;		///< List element count
            Category        category;       ///< Keyword category
            Vector<String>  completions;    ///< Available completions, for most keywords this contains 0 elements
        };

        typedef Vector<Keyword*>    KeywordList;
        class RepositoryController;

        /** Models the repository/metadata part of the component */
        class RepositoryModel : public Model {
        public:
            RepositoryModel( ChangeManager* );
            ~RepositoryModel();
			/** Returns the number of keywords. */
            UInt getCount() const;
			/** Returns a single keyword.
				@param index Index of the keyword, must be in the interval [0;getCount()[.
				@return A const pointer to the keyword or NULL if the keyword is not found. */
            const Keyword* getKeyword( UInt index ) const;
			/** Returns a single keyword.
				@param name Name of the keyword.
				@return A const pointer to the keyword or NULL if the keyword is not found. */
			const Keyword* getKeyword( String name ) const;
			/** Finds the index of a keyword given its name.
				@param name Name of the keyword.
				@return The index of the keyword or -1 if not find. */
            Int findKeyword( String name ) const;
			/** Returns the name of a category.
				@param index Index of the category, must be in the interval [0;getCategoryCount()[.
				@return The name of the category or an empty string if the category is not found. */
            String getCategory( UInt index ) const;
			/** Returns the number of categories. */
            UInt getCategoryCount() const;
            /** Returns the rules from a specific set.
                @param format Retrieve the rules associated with this format. */
            const RuleList& getRules(FitsLiberator::Engine::ImageReader::ImageFormat format) const;
            /** Retuns the rules from the global set. */
            const RuleList& getRules() const;
			/** Serializes the metadata as Adobe XMP.
				@return A string containing the XMP. */
            String serializeAsXmp() const;
			Void flushValues();
        protected:
            friend class RepositoryController;
			/** Sets the values of a keyword, note that ";" is used as a separator for keywords 
				with the KeywordFlagsList flag.
				@param keywordIndex Index of the keyword, must be in the interval [0;getCount()[.
				@param value New keyword value. */
            Void setValue( UInt keywordIndex, String value );
            Void addCompletion( UInt, String );
			
        private:
            typedef Model super;
			/** Reads the metadata definition from an XML string.
				@param xml The XML definition.
				@return True on success, false on failure. */
            Bool readDefinition( const String& xml );
			/** Reads a single keyword category.
				@param parent XML node holding the category.
				@return True on success, false on failure. */
            Bool readCategory( const TiXmlElement* parent );
			/** Reads a FITS Header <=> VR Metadata mapping rule.
				@param parent XML node holding the rule.
				@return True on success, false on failure. */
            Bool readRules( const TiXmlElement* parent );
			/** Reads a single metadata keyword definition.
				@param category Keyword category this keyword belongs to.
				@param parent XML node holding the keyword.
				@return True on success, false on failure. */
            Bool readKeyword( Keyword::Category category, const TiXmlElement* parent );
            Bool readCompletion( Keyword* keyword, const TiXmlElement* source );
			/** Serializes the metadata as RDF. 
				@param parent XML node to append to. */
            Void serializeAsRdf( TiXmlNode* parent ) const;
			/** Serializes the keywords to the VR schema.
				@param parent XML node to append to. */
			Void serializeVrSchema( TiXmlNode* parent ) const;
			/** Serializes the keywords according to the IPTC schema.
				@param parent XML node to append to. */
			Void serializeIptcSchema( TiXmlNode* parent ) const;
			/** Serializes the keywords according to the Dublin Core schema.
				@param parent XML node to append to. */
			Void serializeDcSchema( TiXmlNode* parent ) const;
			/** Serializes the keywords according to the Photoshop schema.
				@param parent XML node to append to. */
			Void serializePhotoshopSchema( TiXmlNode* parent ) const;
			/** Serializes the keywords according to the XMP schema.
				@param parent XML node to append to. */
			Void serializeXmpSchema( TiXmlNode* parent ) const;
			/** Serializes a single keyword as RDF.
				@param keyword Keyword to serialize.
				@param parent XML node to append to. */
            Void serializeKeywordAsRdf( Keyword* keyword, TiXmlNode* parent ) const;
			/** Serializes a single keyword as RDF.
				@param name Name of the keyword to serialize.
				@param path Output XML element name.
				@param parent XML node to append to. */
			Void serializeKeywordAsRdf( const std::string& name, const std::string& path, TiXmlNode* parent ) const;
			/** Serializes the keyword value.
				@param keyword Keyword to serialize.
				@param parent XML node to append to. */
			Void serializeKeywordValue( const Keyword* keyword, TiXmlNode* parent ) const;
			/** Serializes a list of strings as RDF using a RDF sequence element.
				@param keyword Keyword being serialized.
				@param parent XML node to append to. */
            Void serializeListAsRdf( const Keyword* keyword, TiXmlNode* parent ) const;
			/** Assigns a string value to a XML node.
				@param value Value to set.
				@param node Node to append to. */
			Void serializeText( const std::string& value, TiXmlNode* node ) const;
			/** Stores a simple string value in a rdf:Alt element.
				@param value Text to store.
				@param node Node to append to. */
			Void serializeAltText( const std::string& value, TiXmlNode* node ) const;

            KeywordList     keywords;               ///< Defined keywords
            CategoryList    categories;             ///< Defined categories
            RuleList        rules[3];               ///< Array<List<Rule>>; one list for each format
            String          vrSchema;               ///< Schema to use for serialization
            String          vrNamespace;            ///< XML namespace prefix to use for serialization
		};
	
		
	} // end namespace Modelling
} // end namespace FitsLiberator

#endif // __REPOSITORYMODEL_H__