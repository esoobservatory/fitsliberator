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
#ifndef __TAXONOMYEDITORMODEL_H__
#define __TAXONOMYEDITORMODEL_H__ 

#include "FitsLiberator.h"
#include "Observer.h"
#include "tinyxml.h"

namespace FitsLiberator {
	namespace Modelling {
		class TaxonomyEditorController;

		/** Models the editor for the VR keyword Subject.Category. The model
			consists of a list of astronomical scales and a tree of categories. */
        class TaxonomyEditorModel : public Model {
		public:
			class Category;
			class Scale;

            /** Represents an astronomical scale for the category editor. */
            class Scale {
				String definition;
			protected:
				friend class TaxonomyEditorModel;
				Scale( const TiXmlElement* element );
            public:
                Char Identifier() const;
                const String& Name() const;
            };

            /** Represents a category node in the category editor tree. */
            class Category {
				/** String defining the category. This contains both the 
					identifier as well name and editable state. */
				String definition;
				Bool checked;
				Vector<Category*> subCategories;
				Category* parent;
			protected:
				friend class TaxonomyEditorModel;
				/** Constructs category from an XML element.
					@param parent Parent category, may be NULL if this category is at the root.
					@param element XML element to construct this category from. The XML is expected
					to have the form:
						<Category Definition="...">
							<Category Definition="..."/>
							<Category Definition="..."/>
						</Category> */
				Category( Category* parent, const TiXmlElement* element );
				~Category();
				/** Changes the checked state of this category.
					@param checked Is the category checked or not. */
				Void Checked( Bool checked );
            public:
				/** Enumerates the child categories.
					@param func A function object that gets invoked for each child category. 
					This parameter must evaluate to a function object f( const Category* ). */			
				template<class F>
				Void EnumerateCategories( F& func ) const {
					Vector<Category*>::const_iterator i;
					for( i = subCategories.begin(); i != subCategories.end(); i++ )
						func( *i );
				}
				/** Gets the number of child categories. */
				Int CategoryCount() const;
				/** Returns the parent category for this category.
					@return The parent category or NULL if the category has no parent. */
				const Category* Parent() const;
				/** Identifier for this category. */
                String Identifier() const;
				/** Full name of this category including its identifier. */
                const String& Name() const;
				/** Is this category currently checked. */
                Bool Checked() const;
				/** Is this category editable, ie. is it a real category rather
					than a logical level. */
                Bool Editable() const;
				/** Returns whether this category has any checked children. */
				Bool IsLeaf() const;
            };
		private:
			typedef Model super;
			
			Vector<Category*> categories;
            Vector<Scale*> scales;
            Int scaleIndex;
		protected:
			friend class TaxonomyEditorController;

			Void SetScaleIndex( Int newIndex );
			Void CheckCategory( const Category* category, Bool checked );
        public:
			/** Builds the TaxonomyEditorModel.
				@param manager ChangeManager to use. */
            TaxonomyEditorModel( const String& xml, ChangeManager* manager );
			~TaxonomyEditorModel();
			/**  Index of the currently selected scale. */
			Int ScaleIndex() const;
			/** Enumerates the defined scales.
				@param func A function object that gets invoked for each scale. 
				This parameter must evaluate to a function object f( const Scale* ). */
			template<class F>
			Void EnumerateScales( F& func ) const {
				Vector<Scale*>::const_iterator i;
				for( i = scales.begin(); i != scales.end(); i++ )
					func( *i );
			}
			/** Enumerates the root categories.
				@param func A function object that gets invoked for each root category. 
				This parameter must evaluate to a function object f( const Category* ). */
			template<class F>
			Void EnumerateCategories( F& func ) const {
				Vector<Category*>::const_iterator i;
				for( i = categories.begin(); i != categories.end(); i++ )
					func( *i );
			}
			/** Converts the state of the model to a string. */
			String Serialize() const;
        };
    }
}

#endif // __TAXONOMYEDITORMODEL_H__