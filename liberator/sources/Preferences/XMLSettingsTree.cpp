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
#include "XMLSettingsTree.h"
#include "TextUtils.h"

#define XMLSETTINGSTREE_PLIST_VERSION		"1.0"
#define XMLSETTINGSTREE_XMLVERSION			"1.0"
#define XMLSETTINGSTREE_XMLENCODING			"UTF-8"

#define XMLSETTINGSTREE_PLIST_ELEMENT		"plist"
#define XMLSETTINGSTREE_VERSION_ATTRIBUTE	"version"
#define XMLSETTINGSTREE_DICTIONARY_ELEMENT 	"dict"
#define XMLSETTINGSTREE_KEY_ELEMENT			"key"
#define XMLSETTINGSTREE_ARRAY_ELEMENT 		"array"
#define XMLSETTINGSTREE_INTEGER_ELEMENT 	"integer"
#define XMLSETTINGSTREE_REAL_ELEMENT 		"real"
#define XMLSETTINGSTREE_STRING_ELEMENT 		"string"
#define XMLSETTINGSTREE_BOOLEANTRUE_ELEMENT "true"
#define XMLSETTINGSTREE_BOOLEANFALSE_ELEMENT "false"

using namespace FitsLiberator::Preferences;

/**
 * Creates a new settings tree backed by an xml file with file name.
 *
 * @param filename	Filename of the XML document.
 */
XmlSettingsTree::XmlSettingsTree( const String& filename ) : xmlDocument( filename.c_str() ), rootNode( NULL ) {
	
}

/**
 * Creates a new settings tree backed by an xml file with file name.
 *
 * @param filename	Filename of the XML document.
 * @param root		Root node of the XML document.
 */
XmlSettingsTree::XmlSettingsTree( const String& filename, SettingsNode* root ) : xmlDocument( filename.c_str() ), rootNode( root ) {
	
}

/**
 * Destructor for the XmlSettingsTree.
 *
 * When deleting the rootNode, the root node will make sure that all
 * of its children also get destroyed.
 */
XmlSettingsTree::~XmlSettingsTree() {
	if( rootNode != NULL ) {
		delete rootNode;
	}
}

/**
 * Read the XML document from disk.
 *
 * @return True on success, false otherwise.
 */
Bool XmlSettingsTree::loadFile() {
	if( xmlDocument.LoadFile() ) {
		if( rootNode != NULL ) {
			delete rootNode;
			rootNode = NULL;
		}
		
		TiXmlElement* tmpElement = xmlDocument.RootElement();
		
		String elementName = tmpElement->Value();
		String version = tmpElement->Attribute( XMLSETTINGSTREE_VERSION_ATTRIBUTE );
		
		if( elementName != XMLSETTINGSTREE_PLIST_ELEMENT ||  version != XMLSETTINGSTREE_PLIST_VERSION ) { return false; } 
		
		TiXmlHandle handler( tmpElement );
		
		rootNode = xmlToSettingsTree( handler.FirstChildElement().Element() );
				
		return true;
	} else {
		return false;
	}
}

/**
 * Save the XML document to disk.
 *
 * @return True on success, false otherwise.
 */
Bool XmlSettingsTree::saveFile() {
	if( rootNode != NULL ) {
		xmlDocument.Clear();
		
		TiXmlDeclaration* 	decl 	= new TiXmlDeclaration( XMLSETTINGSTREE_XMLVERSION, XMLSETTINGSTREE_XMLENCODING, "" );
		TiXmlElement* 		plist 	= new TiXmlElement( XMLSETTINGSTREE_PLIST_ELEMENT );
		
		plist->SetAttribute( XMLSETTINGSTREE_VERSION_ATTRIBUTE, XMLSETTINGSTREE_PLIST_VERSION );
		
		xmlDocument.LinkEndChild( decl );
		xmlDocument.LinkEndChild( plist );
	
		settingsTreeToXml( *rootNode, *plist );
	
		// transform settingsTree to xmlDocument.
		
		if( xmlDocument.SaveFile() ) {
			return true;
		} else {
			return false;
		}	
	} else {
		return false;
	}
}

/**
 * Get the root note of the SettingsTree
 */
SettingsNode* const XmlSettingsTree::getSettingsTree() {
	return rootNode;
}

/**
 * Set the root node of the SettingsTree.
 *
 * @param root Root node of the SettingsTree.
 */
Void XmlSettingsTree::setSettingsTree( SettingsNode* root ) {
	if( rootNode != NULL ){
		delete rootNode;
	}
	
	rootNode = root;	
}


/**
 * Transform a SettingsTree to a XML document
 *
 * @param node		A node in a SettingsTree.
 * @param parent	The parent node in of the XML document (not the SettingsTree).
 */
Void XmlSettingsTree::settingsTreeToXml( SettingsNode& node, TiXmlNode& parent ) {
	switch( node.getType() ) {
		case SETTINGS_TREE_REAL:
			settingsTreeToXml( (RealNode&) node, parent );
			break;
		
		case SETTINGS_TREE_INTEGER:
			settingsTreeToXml( (IntegerNode&) node, parent );
			break;
			
		case SETTINGS_TREE_STRING:
			settingsTreeToXml( (StringNode&) node, parent );
			break;
		
		case SETTINGS_TREE_BOOLEAN:
			settingsTreeToXml( (BooleanNode&) node, parent );
			break;
			
		case SETTINGS_TREE_ARRAY:
			settingsTreeToXml( (ArrayNode&) node, parent );
			break;
			
		case SETTINGS_TREE_DICTIONARY:
			settingsTreeToXml( (DictionaryNode&) node, parent );
			break;
	}
}

/**
 * Transform a RealNode to a XML node.
 *
 * @param node		A settings tree node representing a real node.
 * @param parent	The parent node of the XML document.
 */
Void XmlSettingsTree::settingsTreeToXml( RealNode& node, TiXmlNode& parent ) {
	TiXmlElement* 	element	= new TiXmlElement( XMLSETTINGSTREE_REAL_ELEMENT );
	TiXmlText* 		text 	= new TiXmlText( TextUtils::doubleToString( node.getValue(), 14 ).c_str() );

	element->LinkEndChild( text );
	parent.LinkEndChild( element );
}

/**
 * Transform a IntegerNode to a XML node.
 *
 * @param node		A settings tree node representing a integer node.
 * @param parent	The parent node of the XML document.
 */
 Void XmlSettingsTree::settingsTreeToXml( IntegerNode& node, TiXmlNode& parent ) {
	TiXmlElement* 	element	= new TiXmlElement( XMLSETTINGSTREE_INTEGER_ELEMENT );
	TiXmlText* 		text 	= new TiXmlText( TextUtils::doubleToString( node.getValue(), 0 ).c_str() );

	element->LinkEndChild( text );
	parent.LinkEndChild( element );
}

/**
 * Transform a StringNode to a XML node.
 *
 * @param node		A settings tree node representing a string node.
 * @param parent	The parent node of the XML document.
 */
Void XmlSettingsTree::settingsTreeToXml( StringNode& node, TiXmlNode& parent ) {
	TiXmlElement* 	element	= new TiXmlElement( XMLSETTINGSTREE_STRING_ELEMENT );
	TiXmlText* 		text 	= new TiXmlText( node.getValue().c_str() );

	element->LinkEndChild( text );
	parent.LinkEndChild( element );
}

/**
 * Transform a BooleanNode to a XML node.
 *
 * @param node		A settings tree node representing a boolean node.
 * @param parent	The parent node of the XML document.
 */
Void XmlSettingsTree::settingsTreeToXml( BooleanNode& node, TiXmlNode& parent ) {
	TiXmlElement* element;

	if( node.getValue() ) {
		element	= new TiXmlElement( XMLSETTINGSTREE_BOOLEANTRUE_ELEMENT );
	} else {
		element	= new TiXmlElement( XMLSETTINGSTREE_BOOLEANFALSE_ELEMENT );
	}

	parent.LinkEndChild( element );
}

/**
 * Transform a ArrayNode to a XML node.
 *
 * @param node		A settings tree node representing an array node.
 * @param parent	The parent node of the XML document.
 */
Void XmlSettingsTree::settingsTreeToXml( ArrayNode& node, TiXmlNode& parent ) {
	TiXmlElement* element = new TiXmlElement( XMLSETTINGSTREE_ARRAY_ELEMENT );
	
	for( Int i = 0; i < node.countChildren(); i++ ) {
		settingsTreeToXml( node[i], *element);
	}

	parent.LinkEndChild( element );
}

/**
 * Transform a DictionaryNode to a XML node.
 *
 * @param node		A settings tree node representing a dictionary node.
 * @param parent	The parent node of the XML document.
 */
Void XmlSettingsTree::settingsTreeToXml( DictionaryNode& node, TiXmlNode& parent ) {
	TiXmlElement* 	element 	= new TiXmlElement( XMLSETTINGSTREE_DICTIONARY_ELEMENT );
	TiXmlElement* 	keyElement 	= NULL;
	
	for( node.beginChild(); node.hasMoreChildren(); node.nextChild() ) {
		keyElement 		= new TiXmlElement( XMLSETTINGSTREE_KEY_ELEMENT );
		const String& key = node.nextChildKey();
		TiXmlText* text = new TiXmlText( key.c_str() );
		
		keyElement->LinkEndChild( text );
		element->LinkEndChild( keyElement );
	
		settingsTreeToXml( node.nextChildValue(), *element );
	}

	parent.LinkEndChild( element );
}

/**
 * Transform XML node to a SettingsTree.
 *
 * @param xmlElement	XML element.
 *
 * @return				A SettingsTree node.
 */
SettingsNode* XmlSettingsTree::xmlToSettingsTree( TiXmlElement* xmlElement ) {
	SettingsNode* retNode = NULL;

	if( xmlElement != NULL ) {
		String text = xmlElement->Value();
	
		if( text == XMLSETTINGSTREE_DICTIONARY_ELEMENT ) {
			retNode = xmlToDictionaryNode( xmlElement );
		} else if( text == XMLSETTINGSTREE_ARRAY_ELEMENT ) {
			retNode = xmlToArrayNode( xmlElement );
		} else if( text == XMLSETTINGSTREE_STRING_ELEMENT ) {
			retNode = xmlToStringNode( xmlElement );
		} else if( text == XMLSETTINGSTREE_REAL_ELEMENT ) {
			retNode = xmlToRealNode( xmlElement );
		} else if( text == XMLSETTINGSTREE_INTEGER_ELEMENT ) {
			retNode = xmlToIntegerNode( xmlElement );
		} else if( text == XMLSETTINGSTREE_BOOLEANTRUE_ELEMENT ) {
			retNode = xmlToBooleanTrueNode( xmlElement );
		} else if( text == XMLSETTINGSTREE_BOOLEANFALSE_ELEMENT ) {
			retNode = xmlToBooleanFalseNode( xmlElement );
		} 
	}
	
	return retNode;
}

/**
 * Transform a XML node to a DictionaryNode.
 *
 * @param xmlElement	XML element.
 *
 * @return				A DictionaryNode.
 */
DictionaryNode* XmlSettingsTree::xmlToDictionaryNode( TiXmlElement* xmlElement ) {
	DictionaryNode* retNode = new DictionaryNode();
	
	TiXmlHandle		handler( xmlElement );
	TiXmlElement* 	keyElement;
	TiXmlElement* 	valueElement; 
	
	Int i = 0;
	Bool proceed = true;
	
	do {
		keyElement	 = handler.ChildElement( i ).Element();
		valueElement = handler.ChildElement( i+1 ).Element();
		
		// If elements are NULL, then their aren't more elements
		// inside the <dict> element.
		if( keyElement != NULL && valueElement != NULL ) {				
			TiXmlText* textElement = keyElement->FirstChild()->ToText();
		
			if( textElement != NULL ) {
				const Char* key = textElement->Value();
				
				SettingsNode* valueNode = xmlToSettingsTree( valueElement );
				retNode->addKey( key, valueNode );				
			}			
		} else {
			proceed = false;
		}
		
		i += 2;
	} while( proceed );	
	
	return retNode;
}

/**
 * Transform a XML node to a ArrayNode.
 *
 * @param xmlElement	XML element.
 *
 * @return				A ArrayNode.
 */
ArrayNode* XmlSettingsTree::xmlToArrayNode( TiXmlElement* xmlElement ) {
	ArrayNode* retNode = new ArrayNode();
	
	TiXmlHandle		handler( xmlElement );
	TiXmlElement* 	child = handler.FirstChild().Element();
			
	//< @todo what if child is null?
	for( child; child; child = child->NextSiblingElement() ) {
		SettingsNode* tmp = xmlToSettingsTree( child );
		retNode->addChild( tmp );
	}
	
	return retNode;
}

/**
 * Transform a XML node to a StringNode.
 *
 * @param xmlElement	XML element.
 *
 * @return				A StringNode.
 */
StringNode* XmlSettingsTree::xmlToStringNode( TiXmlElement* xmlElement ) {
	StringNode* retNode = NULL;
	
	TiXmlHandle 	handler( xmlElement );	
	TiXmlText* 		textElement = handler.FirstChild().Text();
	
	if( textElement != NULL ) {
		String value = textElement->Value();
		retNode = new StringNode( value );
	}
	
	return retNode;
}

/**
 * Transform a XML node to a RealNode.
 *
 * @param xmlElement	XML element.
 *
 * @return				A RealNode.
 */
RealNode* XmlSettingsTree::xmlToRealNode( TiXmlElement* xmlElement ) {
	RealNode* retNode = NULL;
	
	TiXmlHandle 	handler( xmlElement );	
	TiXmlText* 		textElement = handler.FirstChild().Text();
		
	if( textElement != NULL ) {
		String value = textElement->Value();
		retNode = new RealNode( TextUtils::stringToDouble( value ) );
	}
	
	return retNode;
}

/**
 * Transform a XML node to a IntegerNode.
 *
 * @param xmlElement	XML element.
 *
 * @return				A IntegerNode.
 */
IntegerNode* XmlSettingsTree::xmlToIntegerNode( TiXmlElement* xmlElement ) {
	IntegerNode* 	retNode = NULL;
	
	TiXmlHandle 	handler( xmlElement );	
	TiXmlText* 		textElement = handler.FirstChild().Text();
		
	if( textElement != NULL ) {
		String value = textElement->Value();
		retNode = new IntegerNode( TextUtils::stringToInt( value ) );
	}
	
	return retNode;
}

/**
 * Transform a XML node to a true BooleanNode.
 *
 * @param xmlElement	XML element.
 *
 * @return				A BooleanNode representing true.
 */
BooleanNode* XmlSettingsTree::xmlToBooleanTrueNode( TiXmlElement* xmlElement ) {
	return new BooleanNode( true );
}

/**
 * Transform a XML node to a false BooleanNode
 *
 * @param xmlElement	XML element.
 *
 * @return				A BooleanNode representing false.
 */
BooleanNode* XmlSettingsTree::xmlToBooleanFalseNode( TiXmlElement* xmlElement ) {
	return new BooleanNode( false );
}