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

#ifndef __SETTINGSTREE_H__
#define __SETTINGSTREE_H__

#include "Types.h"
#ifdef WINDOWS
	#include <hash_map>
#else
	#include <ext/hash_map>
	#include "HashFuncs.h"
#endif

namespace FitsLiberator {
	namespace Preferences {
	
		#define SETTINGS_TREE_REAL 		 1
		#define SETTINGS_TREE_INTEGER 	 2 
		#define SETTINGS_TREE_STRING 	 3 
		#define SETTINGS_TREE_BOOLEAN 	 4 
		#define SETTINGS_TREE_ARRAY 	 5 
		#define SETTINGS_TREE_DICTIONARY 6 
		
		class SettingsVisitor;
		
		/**
		 * Abstract base class for nodes in the settings tree.
		 */
		class SettingsNode {
			public:
								SettingsNode( const Byte t ) : type( t ) {};
				virtual 		~SettingsNode();
				
				const Byte 		getType();
				
				virtual void 	visit( SettingsVisitor& ) = 0;
				
			private:
				const Byte		type;		///< The type of the node.
		};
		
		/**
		 * Node for storing real numbers
		 */
		class RealNode : public SettingsNode {
			public:
								RealNode( Double );
			
				virtual void 	visit( SettingsVisitor& );
				
				Double 			getValue();
				Void 			setValue( Double );
			
			private:
				Double 			value;	
		};
		
		/**
		 * Node for storing integers
		 */
		class IntegerNode : public SettingsNode {
			public:
								IntegerNode( const Int );
				
				virtual void 	visit( SettingsVisitor& );
				
				Int 			getValue();
				Void 			setValue( Int );
			
			private:
				Int 			value;	
		};
		
		/**
		 * Node for storing strings
		 */
		class StringNode : public SettingsNode {
			public:
								StringNode( const String& );
				virtual void 	visit( SettingsVisitor& );
				
				const String& 	getValue();
				Void 			setValue( const String& );
				
			private:
				String 			value;
		};
		
		/**
		 * Node for storing booleans
		 */
		class BooleanNode : public SettingsNode {
			public:
								BooleanNode( Bool );
				virtual void 	visit( SettingsVisitor& );
				
				Bool			getValue();
				Void 			setValue( Bool );
			
			private:
				Bool 		value;
		};
		
		/**
		 * Node for storing a list of nodes.
		 */
		class ArrayNode : public SettingsNode {
			public:
								ArrayNode();
				virtual 		~ArrayNode();
				
				virtual void 	visit( SettingsVisitor& );
				
				Void			addChild( SettingsNode* );
				Void			deleteChild( Int );
				Int				countChildren();
				
				SettingsNode& 	operator[]( const Int index );
				
			private:
				Vector<SettingsNode*> 	children;
				UInt 					size;		///< Number of elements in node.
		};
		
		
		#ifdef __GNUC__
			typedef __gnu_cxx::hash_map< String, SettingsNode *> StringSettingsNodeMap;
        #else
            typedef stdext::hash_map< String, SettingsNode * > StringSettingsNodeMap;
        #endif
        
		/**
		 * Node for storing an associative list of nodes.
		 *
		 * Uses string for storing keys - found a bug when using
		 * const Char*, where same key was used for all keys. 
		 */
		class DictionaryNode : public SettingsNode {
			public:
								DictionaryNode();
				virtual			~DictionaryNode();
				
				virtual void 	visit( SettingsVisitor& );
				
				Void 			addKey( const String&, SettingsNode* );
				Void 			removeKey( const String& );
				Bool 			containsKey( const String& );
				Int 			countKeys();
				SettingsNode& 	getKey( const String& );
				
				Void 			beginChild();
				Void			nextChild();
				const String&	nextChildKey();
				SettingsNode& 	nextChildValue();
				Bool 			hasMoreChildren();
				
			private:
				StringSettingsNodeMap children;
				StringSettingsNodeMap::iterator it;	///< Children iterator
		};
		
		
		/**
		 * Visitor interface for SettingsTree traversals. Traversal order is defined in the 
		 * datastructure.
		 */
		class SettingsVisitor {
			public:
				virtual ~SettingsVisitor();
				virtual void visitRealNode( RealNode& ) = 0;
				virtual void visitIntegerNode( IntegerNode& ) = 0;
				virtual void visitStringNode( StringNode& ) = 0;
				virtual void visitBooleanNode( BooleanNode& ) = 0;
				virtual void visitArrayNode( ArrayNode& ) = 0;
				virtual void visitDictionaryNode( DictionaryNode& ) = 0;
		};
		
		
		/**
		 * Print a list of all the members in a settings tree.
		 */
		/*class PrintingSettingsVisitor : public SettingsVisitor {
			public:
				virtual void visitRealNode( RealNode& );
				virtual void visitIntegerNode( IntegerNode& );
				virtual void visitStringNode( StringNode& );
				virtual void visitBooleanNode( BooleanNode& );
				virtual void visitArrayNode( ArrayNode& );
				virtual void visitDictionaryNode( DictionaryNode& );
		};*/
	} // end namespace Preferences
} // end namespace FitsLiberator

#endif //__SETTINGSTREE_H__