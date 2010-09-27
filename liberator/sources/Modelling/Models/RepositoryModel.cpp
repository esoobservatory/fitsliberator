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
#include "TextUtils.h"
#include "RepositoryModel.h"

#include <sstream>

#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

// ------------------------------------------------------------------------------------------------
// Implementation
// ------------------------------------------------------------------------------------------------

using namespace FitsLiberator::Modelling;
using namespace FitsLiberator::Engine;

using namespace boost::algorithm;

using namespace std;

// ------------------------------------------------------------------------------------------------
// Implementation of FitsLiberator::Modelling::Keyword
// ------------------------------------------------------------------------------------------------

Keyword::Keyword( String name, Keyword::Type type, Keyword::Flags flags, Keyword::Category category ) {
    this->name = name;
    this->type = type;
    this->flags = flags;
    this->category = category;
}

// ------------------------------------------------------------------------------------------------
// Implementation of FitsLiberator::Modelling::RepositoryModel
// ------------------------------------------------------------------------------------------------

RepositoryModel::RepositoryModel( ChangeManager* c ) : Model(c) {
    String text = Environment::readResource( "MetadataDefinition.xml" );
    readDefinition( text );
}

RepositoryModel::~RepositoryModel() {
    UInt index, count;

    count = categories.size();
    for( index = 0; index < count; index++ ) {
        delete categories[index];
    }

    count = keywords.size();
    for( index = 0; index < count; index++ ) {
        delete keywords[index];
    }

    for(int i = 0; i < sizeof(rules)/sizeof(rules[0]); ++i) { 
        RuleList& list = rules[i];

        count = list.size();
        for( index = 0; index < count; index++ ) {
            delete list[index];
        }
    }
}

Bool RepositoryModel::readDefinition( const String& xml ) {
    TiXmlDocument document;
	document.Parse( xml.c_str() );
	
    if( !document.Error() ) {
        TiXmlElement* rootElement = document.RootElement();

        String elementName    = rootElement->Value();
        String elementVersion = rootElement->Attribute( "Version" );
        if( elementName != "Metadata" || elementVersion != "1.1" )
            return false;

        vrSchema    = rootElement->Attribute( "Schema" );
        vrNamespace = rootElement->Attribute( "Namespace" );

        const TiXmlNode* node;
        for( node = rootElement->FirstChildElement(); node; node = node->NextSiblingElement() ) {
            elementName = node->Value();
            if( elementName == "Category" ) {
                if( !readCategory( node->ToElement() ) ) return false;
            }
            else if( elementName == "Ruleset" ) {
                if( !readRules( node->ToElement() ) ) return false;
            } 
        }
        return true;
    }
    return false;
}

Bool RepositoryModel::readCategory( const TiXmlElement* parent ) {
    assert( parent != NULL );

    const Char* nameAttribute = parent->Attribute( "Name" );
    if( nameAttribute == NULL )
        return false;

    categories.push_back( new String( nameAttribute ) );

    const TiXmlNode* node;
    for( node = parent->FirstChildElement(); node; node = node->NextSiblingElement() ) {
        String elementName = node->Value();
        if( elementName == "Keyword" )
            if( !readKeyword( categories.size() - 1, node->ToElement() ) ) return false;
    }
    return true;
}

Bool RepositoryModel::readRules( const TiXmlElement* parent ) {
    assert( parent != NULL );

    int ruleSet = -1;
    const char* formatAttribute = parent->Attribute( "Format" );
    if( formatAttribute == NULL ) { // Global ruleset
        ruleSet = 0;
    } else if(strcmp(formatAttribute, "FITS") == 0) {
        ruleSet = ImageReader::FITS;
    } else if(strcmp(formatAttribute, "PDS") == 0) {
        ruleSet = ImageReader::PDS;
    }

    if( ruleSet >= 0 && ruleSet <= sizeof(rules)/sizeof(rules[0]) ) {
        RuleList& list = rules[ruleSet];

	    const TiXmlNode* node;
	    for( node = parent->FirstChildElement(); node; node = node->NextSiblingElement() ) {
		    Rule* rule = Rule::Create( node->ToElement() );
		    if( rule != NULL )
			    list.push_back( rule );
	    }
    }
	return true;
}

Bool RepositoryModel::readKeyword( Keyword::Category category, const TiXmlElement* parent ) {
    assert( parent != NULL );

    // The format is <Keyword ... Name=".."></Keyword>, without a name
    // the keyword is invalid.
    const Char* name = parent->Attribute( "Name" );
    if( NULL == name ) return false;

    // Determine type and flags
    const Char* typeString = parent->Attribute( "Type" );
    const Char* flagsString = parent->Attribute( "Flags" );
    const Char* descriptionString = parent->Attribute( "Description" );
	const Char* elementsString = parent->Attribute( "Elements" );
	const Char* idString = parent->Attribute( "Id" );

    Keyword::Type type = (typeString == NULL)
        ? Keyword::TypeString
        : (Keyword::Type)TextUtils::stringToInt( typeString );
    Keyword::Flags flags = (flagsString == NULL)
        ? Keyword::FlagsNone
        : (Keyword::Flags)TextUtils::stringToInt( flagsString );

    Keyword* keyword = new Keyword( name, type, flags, category );
    if( descriptionString != NULL ) keyword->description = descriptionString;
	if( idString != NULL ) keyword->id = idString;
	keyword->elements = (elementsString == NULL)
		? 1 : TextUtils::stringToInt( elementsString );

    // See if any completions exist for this keyword
    const TiXmlNode* node;
    for( node = parent->FirstChildElement(); node; node = node->NextSiblingElement() ) {
        String elementName = node->Value();
        if( elementName == "Completion" ) {
            const TiXmlNode* content = node->FirstChild();
            if( content != NULL && content->Type() == TiXmlNode::TEXT ) {
                keyword->completions.push_back( content->Value() );
            }
		}
    }

    keywords.push_back( keyword );

    return true;
}

String RepositoryModel::serializeAsXmp() const {
    //
    // Generate the XML
    TiXmlDocument document;
    TiXmlElement* xmp = new TiXmlElement( "x:xmpmeta" );
    xmp->SetAttribute( "xmlns:x", "adobe:ns:meta/" );
    document.LinkEndChild( xmp );

    serializeAsRdf( xmp );

    //
    // Write to the output, as per the spec some padding is added to the XMP packet.
	std::ostringstream s;
    s << "<?xpacket begin='﻿' id='W5M0MpCehiHzreSzNTczkc9d'?>";
	s << document;
    for( UInt i = 0; i < 30; i++ )
        s << "                                                                                                    \n";
    s << "<?xpacket end='w'?>";

    return s.str();
}

Void RepositoryModel::serializeAsRdf( TiXmlNode* parent ) const {
    assert( parent != NULL );

    // <rdf:RDF>
    TiXmlElement* rdf = new TiXmlElement( "rdf:RDF" );
    rdf->SetAttribute( "xmlns:rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#" );

	serializeVrSchema( rdf );
	serializeIptcSchema( rdf );
	serializeDcSchema( rdf );
	serializePhotoshopSchema( rdf );
	serializeXmpSchema( rdf );

    // </rdf:RDF>
    parent->LinkEndChild( rdf );
}

Void RepositoryModel::serializeVrSchema( TiXmlNode* rdf ) const {
    // <rdf:Description>
    TiXmlElement* description = new TiXmlElement( "rdf:Description" );
    String        schemaName = "xmlns:" + vrNamespace;
    description->SetAttribute( "rdf:about", "" );
    description->SetAttribute( schemaName, vrSchema );

    for( Vector<Keyword*>::const_iterator i = keywords.begin(); i != keywords.end(); i++ ) {
		if( (*i)->flags & Keyword::FlagsDontSerialize )
			continue;

        serializeKeywordAsRdf( (*i), description );
    }

    // </rdf:Description>
    rdf->LinkEndChild( description );
}

Void RepositoryModel::serializeIptcSchema( TiXmlNode* rdf ) const {
    // <rdf:Description>
    TiXmlElement* description = new TiXmlElement( "rdf:Description" );
    description->SetAttribute( "rdf:about", "" );
	description->SetAttribute( "xmlns:Iptc4xmpCore", "http://iptc.org/std/Iptc4xmpCore/1.0/xmlns/" );

		// <Iptc4xmpCore:CreatorContactInfo rdf:parseType="Resource">
		TiXmlElement* creatorContactInfo = new TiXmlElement( "Iptc4xmpCore:CreatorContactInfo" );
		creatorContactInfo->SetAttribute( "rdf:parseType", "Resource" );

			// Because of a mismatch in the definition of Contact.Email and Iptc4xmpCore:CiEmailWork
			// it is necessary to emit these manually.
			TiXmlElement* keyword;
			
			keyword = new TiXmlElement( "Iptc4xmpCore:CiEmailWork" );
			serializeText( getKeyword( "Contact.Email" )->value, keyword );
			creatorContactInfo->LinkEndChild( keyword );

			keyword = new TiXmlElement( "Iptc4xmpCore:CiTelWork" );
			serializeText( getKeyword( "Contact.Telephone" )->value, keyword );
			creatorContactInfo->LinkEndChild( keyword );

			keyword = new TiXmlElement( "Iptc4xmpCore:CiUrlWork" );
			serializeText( getKeyword( "CreatorURL" )->value, keyword );
			creatorContactInfo->LinkEndChild( keyword );

			serializeKeywordAsRdf( "Contact.Address", "Iptc4xmpCore:CiAdrExtadr", creatorContactInfo );
			serializeKeywordAsRdf( "Contact.City", "Iptc4xmpCore:CiAdrCity", creatorContactInfo );
			serializeKeywordAsRdf( "Contact.StateProvince", "Iptc4xmpCore:CiAdrRegion", creatorContactInfo );
			serializeKeywordAsRdf( "Contact.PostalCode", "Iptc4xmpCore:CiAdrPcode", creatorContactInfo );
			serializeKeywordAsRdf( "Contact.Country", "Iptc4xmpCore:CiAdrCtry", creatorContactInfo );

		// </Iptc4xmpCore:CreatorContactInfo>
		description->LinkEndChild( creatorContactInfo );


    // </rdf:Description>
    rdf->LinkEndChild( description );	
}

Void RepositoryModel::serializeDcSchema( TiXmlNode* rdf ) const {
    // <rdf:Description>
    TiXmlElement* description = new TiXmlElement( "rdf:Description" );
    description->SetAttribute( "rdf:about", "" );
	description->SetAttribute( "xmlns:dc", "http://purl.org/dc/elements/1.1/" );

		serializeKeywordAsRdf( "Contact.Name", "dc:creator", description );
		serializeKeywordAsRdf( "Description", "dc:description", description );
		serializeKeywordAsRdf( "Subject.Name", "dc:subject", description );
		serializeKeywordAsRdf( "Title", "dc:title", description );

    // </rdf:Description>
    rdf->LinkEndChild( description );		
}

Void RepositoryModel::serializePhotoshopSchema( TiXmlNode* rdf ) const {
    // <rdf:Description>
    TiXmlElement* description = new TiXmlElement( "rdf:Description" );
    description->SetAttribute( "rdf:about", "" );
	description->SetAttribute( "xmlns:photoshop", "http://ns.adobe.com/photoshop/1.0/" );

		serializeKeywordAsRdf( "Creator", "photoshop:Source", description );
		serializeKeywordAsRdf( "Headline", "photoshop:Headline", description );
		serializeKeywordAsRdf( "Credit", "photoshop:Credit", description );
		serializeKeywordAsRdf( "Date", "photoshop:DateCreated", description );

    // </rdf:Description>
    rdf->LinkEndChild( description );	
}

Void RepositoryModel::serializeKeywordAsRdf( const string& name, const string& path, TiXmlNode* parent ) const {
	const Keyword* keyword = getKeyword( name );
	if( keyword->value.length() > 0 ) {
		TiXmlElement* node = new TiXmlElement( path );
		serializeKeywordValue( keyword, node );
		parent->LinkEndChild( node );
	}
}

Void RepositoryModel::serializeXmpSchema( TiXmlNode* rdf ) const {
    TiXmlElement* description = new TiXmlElement( "rdf:Description" );
    description->SetAttribute( "rdf:about", "" );
	description->SetAttribute( "xmlns:xmpRights", "http://ns.adobe.com/xap/1.0/rights/" );
		serializeKeywordAsRdf( "Rights", "xmpRights:UsageTerms", description );
    rdf->LinkEndChild( description );
}

Void RepositoryModel::serializeKeywordAsRdf( Keyword* keyword, TiXmlNode* parent ) const {
    assert( keyword != NULL );
    assert( parent != NULL );

    if( keyword->value.length() > 0 ) {
        String name = vrNamespace + ":" + keyword->name;

        TiXmlElement* element = new TiXmlElement( name );	// <aoi:Keyword>
		serializeKeywordValue( keyword, element );					//   Value
        parent->LinkEndChild( element );							// </aoi:Keyword>
    }
}

Void RepositoryModel::serializeKeywordValue( const Keyword* keyword, TiXmlNode* parent ) const {
	assert( keyword != NULL );
	assert( parent != NULL );

	if( keyword->flags & (Keyword::FlagsList | Keyword::FlagsOrdered) ) {
		serializeListAsRdf( keyword, parent );
	}
	else if( keyword->flags & (Keyword::FlagsAlt ) ) {
		serializeAltText( keyword->value, parent );
	}
	else {

		serializeText( keyword->value, parent );
	}
}

Void RepositoryModel::serializeListAsRdf( const Keyword* keyword, TiXmlNode* parent ) const {
    string        outer  = (keyword->flags & Keyword::FlagsOrdered) ? "rdf:Seq" : "rdf:Bag";
	TiXmlElement* rdfSeq = new TiXmlElement( outer );			// <rdf:Seq>

    // Lists need special handling
    vector<string> items;
	split(items, keyword->value, is_any_of(";"));

    vector<string>::const_iterator i = items.begin();
    while( i != items.end() ) {
        TiXmlElement* rdfLi = new TiXmlElement( "rdf:li" );		//   <rdf:li>
		serializeText( (*i++), rdfLi );
        rdfSeq->LinkEndChild( rdfLi );							//   </rdf:li>
    }
	
	items.clear();

    parent->LinkEndChild( rdfSeq );								// </rdf:Seq>
}

Void RepositoryModel::serializeText( const string& text, TiXmlNode* element ) const {
	TiXmlText* value = new TiXmlText( text );
	element->LinkEndChild( value );
}

Void RepositoryModel::serializeAltText( const string& text, TiXmlNode* element ) const {
	TiXmlElement* rdfAlt = new TiXmlElement( "rdf:Alt" );		// <rdf:Alt>
		TiXmlElement* rdfLi = new TiXmlElement( "rdf:li" );		//   <rdf:li
		rdfLi->SetAttribute( "xml:lang", "x-default" );			//     xml:lang="x-default">
			serializeText( text, rdfLi );						//   Text here
		rdfAlt->LinkEndChild( rdfLi );							//   </rdf:li>
	element->LinkEndChild( rdfAlt );							// </rdf:Alt>
}

const Keyword* RepositoryModel::getKeyword( UInt index ) const {
	if( index < 0 || index >= keywords.size() )
		return NULL;
    return keywords[index];
}

const Keyword* RepositoryModel::getKeyword( String name ) const {
	return getKeyword( findKeyword( name ) );
}

UInt RepositoryModel::getCount() const {
    return keywords.size();
}

String RepositoryModel::getCategory( UInt index ) const {
	if( index < 0 || index >= categories.size() )
		return "";
    return *(categories[index]);
}

UInt RepositoryModel::getCategoryCount() const {
    return categories.size();
}

Int RepositoryModel::findKeyword( String name ) const {
    UInt count = getCount();
    for( UInt i = 0; i < count; i++ ) {
        if( keywords[i]->name == name )
            return i;
    }
    return -1;
}

Void RepositoryModel::setValue( UInt keywordIndex, String value ) {
    assert( keywordIndex >= 0 );
    assert( keywordIndex < getCount() );

    Keyword* keyword = keywords[keywordIndex];
    keyword->value = value;
    super::Notify();
}

Void RepositoryModel::flushValues()
{
	for ( UInt i = 0; i < keywords.size(); i++ )
	{
		String cat = *(categories[keywords[i]->category]);
		
		if(0 == (keywords[i]->flags & Keyword::FlagsRetained))
			keywords[i]->value = "";
	}
	
}

Void RepositoryModel::addCompletion( UInt keywordIndex, String value ) {
    assert( keywordIndex >= 0 );
    assert( keywordIndex < getCount() );

    Keyword* keyword = keywords[keywordIndex];
    keyword->completions.push_back( value );
}

const RuleList&
RepositoryModel::getRules() const {
    return getRules(static_cast<ImageReader::ImageFormat>(0));
}

const RuleList&
RepositoryModel::getRules(ImageReader::ImageFormat format) const {
    assert(format >= 0);
    assert(format <= ImageReader::PDS);

    return rules[format];
}