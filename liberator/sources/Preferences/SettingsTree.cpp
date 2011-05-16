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
#include "SettingsTree.h"

using namespace FitsLiberator::Preferences;

//-------------------------------------------------------------------------------
//	SettingsNode implemetation
//-------------------------------------------------------------------------------

/**
 * The destructor must be declared virutal to make sure the right
 * destructor is called when for instance deleting a ArrayNode.
 * If the destructor is not declared virtual SettingsNode destructor
 * will be called when ArrayNode is accessed as an SettingsNode. With
 * a virutal destructor for SettingsNode and inhierted classes, the ArrayNode 
 * destructor will be called instead of the SettingsNode.  
 */
SettingsNode::~SettingsNode() {
	
}

/**
 * Get the type of a node
 *
 * @return The type of a given node.
 */
const Byte SettingsNode::getType() {
	return type;
}

//-------------------------------------------------------------------------------
//	RealNode implemetation
//-------------------------------------------------------------------------------

/**
 * Constructor for real nodes.
 *
 * @param v Value to store.
 */
RealNode::RealNode( const Double v ) : value( v ), SettingsNode( SETTINGS_TREE_REAL ) {
}

/**
 * Get the value of the node
 *
 * @return Value of node
 */
Double RealNode::getValue() {
	return value;
}

/**
 * Set the value of the node
 *
 * @param v Value to store.
 */
Void RealNode::setValue( Double v ) {
	value = v;
}

/**
 * Visitor method.
 */
Void RealNode::visit( SettingsVisitor& visitor ) {
	visitor.visitRealNode( *this );
}

//-------------------------------------------------------------------------------
//	IntegerNode implemetation
//-------------------------------------------------------------------------------
/**
 * Constructor for integer nodes.
 *
 * @param v Value to store.
 */
IntegerNode::IntegerNode( const Int v ) : value( v ), SettingsNode( SETTINGS_TREE_INTEGER ) {
}

/**
 * Get the value of the node
 *
 * @return Value of node
 */
Int IntegerNode::getValue() {
	return value;
}

/**
 * Set the value of the node
 *
 * @param v Value to store.
 */
Void IntegerNode::setValue( const Int v ) {
	value = v;
}

/**
 * Visitor method.
 */
Void IntegerNode::visit( SettingsVisitor& visitor ) {
	visitor.visitIntegerNode( *this );
}

//-------------------------------------------------------------------------------
//	StringNode implemetation
//-------------------------------------------------------------------------------
/**
 * Constructor for string nodes.
 *
 * @param v Value to store.
 */
StringNode::StringNode( const String& v ) : value( v ), SettingsNode( SETTINGS_TREE_STRING ) {
}

/**
 * Get the value of the node
 *
 * @return Value of node
 */
const String& StringNode::getValue() {
	return value;
}

/**
 * Set the value of the node
 *
 * @param v Value to store.
 */
Void StringNode::setValue( const String& v ) {
	value = v;
}

/**
 * Visitor method.
 */
Void StringNode::visit( SettingsVisitor& visitor ) {
	visitor.visitStringNode( *this );
}

//-------------------------------------------------------------------------------
//	BooleanNode implemetation
//-------------------------------------------------------------------------------
/**
 * Constructor for boolean nodes.
 *
 * @param v Value to store.
 */
BooleanNode::BooleanNode( const Bool v ) : value( v ), SettingsNode( SETTINGS_TREE_BOOLEAN ) {
}

/**
 * Get the value of the node
 *
 * @return Value of node
 */
Bool BooleanNode::getValue() {
	return value;
}

/**
 * Set the value of the node
 *
 * @param v Value to store.
 */
Void BooleanNode::setValue( const Bool v ) {
	value = v;
}

/**
 * Visitor method.
 */
Void BooleanNode::visit( SettingsVisitor& visitor ) {
	visitor.visitBooleanNode( *this );
}

//-------------------------------------------------------------------------------
//	ArrayNode implemetation
//-------------------------------------------------------------------------------
/**
 * Constructor for array nodes.
 */
ArrayNode::ArrayNode() : SettingsNode( SETTINGS_TREE_ARRAY  ) {
}

/**
 * Destructor - destroys all of its elements.
 */
ArrayNode::~ArrayNode() {
	if( countChildren() > 0 ) {
		for( UInt i = 0; i < children.size(); i++ ) {
			delete children[i];
		}		
	}
}

/**
 * The datastructure takes control of added object. You
 * are no longer responsible for deleting the node you add.
 *
 * @param node A derived class of SettingsNode
 */
Void ArrayNode::addChild( SettingsNode* node ) {
	if( node != NULL ) {
		children.push_back( node );
		size = countChildren();
	}
}

/**
 * Delete a element
 *
 * @param v Position of element to delete.
 */
Void ArrayNode::deleteChild( Int pos ) {
	if( pos < countChildren() ) {
		delete children[pos];
		children.erase( (children.begin() + pos) );
		size = countChildren();
	}
}

/**
 * Count the number of elements.
 *
 * @return Number of elements in array.
 */
Int ArrayNode::countChildren() {
	return (Int) children.size();
}

/**
 * Get element in list
 *
 * @param index Position of element. First element is returned if index is out of bound.
 */
SettingsNode& ArrayNode::operator[]( const Int index ) {
	if( index >= 0 && index < size ) {
		return *(children[index]);	
	} else {
		return *(children[0]);
	}
	
}

/**
 * Visitor method.
 */
Void ArrayNode::visit( SettingsVisitor& visitor ) {
	visitor.visitArrayNode( *this );
	
	for( UInt i = 0; i < size; i++ ){
		((*this)[i]).visit( visitor );
	}
}


//-------------------------------------------------------------------------------
//	DictionaryNode implemetation
//-------------------------------------------------------------------------------

/**
 * Constructor for Dictioary nodes.
 */
DictionaryNode::DictionaryNode() : SettingsNode( SETTINGS_TREE_DICTIONARY ) {
}

/**
 * Destructor - destroys all key value pairs.
 */
DictionaryNode::~DictionaryNode() {
	for( beginChild(); hasMoreChildren(); nextChild() ) {
		delete &nextChildValue();
	}	
}

/**
 * Associates the given key with a node. If the key already exists,
 * it is overwritten with the new value.
 */
Void DictionaryNode::addKey( const String& key, SettingsNode* node ) {
	if( node != NULL ) {
		children[ key ] = node;
	}
}

/**
 * Removes a given key and its value from the dictionary.
 */
Void DictionaryNode::removeKey( const String& key ) {
	SettingsNode* tmp = children[ key ];
	
	if( tmp != NULL ) {
		delete tmp;
	}
	
	children.erase( key );
}

/**
 * Return first element
 */
Void DictionaryNode::beginChild() {
	it = children.begin();
}

/**
 * Returns true if structure has more childrens, false otherwise.
 *
 * DictionaryNode::beginChild() must be called before call to this function.
 */
Bool DictionaryNode::hasMoreChildren() {
	return ( it != children.end() );
}

/**
 * Returns next key element in structure.
 */
const String& DictionaryNode::nextChildKey() {
	return (*it).first;
}

/**
 * Returns next value element in structure.
 */
SettingsNode& DictionaryNode::nextChildValue() {
	return *(*it).second;
}

/**
 * Increments the internal position to next element.
 */
Void DictionaryNode::nextChild() {
	it++;
}

/**
 * Count the number of key/value pairs.
 */
Int DictionaryNode::countKeys() {
	return (Int) children.size();
}

/**
 * Returns true if key exists in dictionary
 */
Bool DictionaryNode::containsKey( const String& key ) {
	StringSettingsNodeMap::iterator iter = children.find( key );

	if( iter != children.end() ) {
		return true;
	} else {
		return false;
	}
}

/**
 * Get a specific element from the list.
 */
SettingsNode& DictionaryNode::getKey( const String& key ) {
	StringSettingsNodeMap::iterator iter = children.find( key );

	return *(*iter).second;
}

/**
 * Visit method.
 */
Void DictionaryNode::visit( SettingsVisitor& visitor ) {
	visitor.visitDictionaryNode( *this );
	
	for( StringSettingsNodeMap::iterator i = children.begin(); i != children.end(); i++ ) {
		((*i).second)->visit( visitor );
	}
}

/*
//-------------------------------------------------------------------------------
//	PrintingSettingsVisitor implemetation
//-------------------------------------------------------------------------------

Void PrintingSettingsVisitor::visitRealNode( RealNode& ) {
	DebugStr("\pIn RealNode");	
}

Void PrintingSettingsVisitor::visitIntegerNode( IntegerNode& ) {
	DebugStr("\pIn IntegerNode");
}

Void PrintingSettingsVisitor::visitStringNode( StringNode& ) {
	DebugStr("\pIn StringNode");
}

Void PrintingSettingsVisitor::visitBooleanNode( BooleanNode& ) {
	DebugStr("\pIn BooleanNode");
}

Void PrintingSettingsVisitor::visitArrayNode( ArrayNode& ) {
	DebugStr("\pIn ArrayNode");
}

Void PrintingSettingsVisitor::visitDictionaryNode( DictionaryNode& ) {
	DebugStr("\pInDictionaryNode");
}*/