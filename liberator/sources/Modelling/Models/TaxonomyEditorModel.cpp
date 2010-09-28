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
#include "TaxonomyEditorModel.h"
#include <sstream>

using namespace FitsLiberator::Modelling;

struct Serializer {
private:
	std::ostringstream buffer;
	Vector<String> path;
	bool first;
public:
	Serializer( Char scale ) {
		first = true;
		String s; s += scale;
		path.push_back( s );
	}

	Void Push( const TaxonomyEditorModel::Category* category ) {
		String s = category->Identifier();
		path.push_back( s );
	}

	Void Pop() {
		path.pop_back();
	}

	Void Emit() {
		if( !first )
			buffer << ';';

		for( int i = 0; i < path.size() - 1; i++ )
			buffer << path[i] << '.';
		buffer << path[path.size() - 1];

		first = false;
	}

	Void operator()( const TaxonomyEditorModel::Category* category ) {
		if( category->Checked() ) {
			Push( category );
			if( category->IsLeaf() ) {
				Emit();
			} else {
				category->EnumerateCategories( *this );
			}
			Pop();
		}
	}

	String ToString() const {
		return buffer.str();
	}
};

TaxonomyEditorModel::TaxonomyEditorModel( const String& xml, ChangeManager* manager ) 
  : super( manager ) {
    this->scaleIndex = 0;

	TiXmlDocument document;
	document.Parse( xml.c_str() );
	
	if( !document.Error()  ) {
		TiXmlElement* root = document.RootElement();
		
		for( const TiXmlNode* node = root->FirstChildElement(); node; node = node->NextSiblingElement() ) {
			String name = node->Value();
			if( name == "Scale" ) {
				this->scales.push_back( new Scale( node->ToElement() ) );
			} else if( name == "Category" ) {
				this->categories.push_back( new Category( NULL, node->ToElement() ) );
			}
		}
	}
}

TaxonomyEditorModel::~TaxonomyEditorModel() {
	for( Vector<Scale*>::const_iterator i = scales.begin(); i != scales.end(); i++ )
		delete (*i);
	for( Vector<Category*>::const_iterator i = categories.begin(); i != categories.end(); i++ )
		delete (*i);
}

Void TaxonomyEditorModel::SetScaleIndex( Int newIndex ) {
	if( newIndex != scaleIndex && newIndex >= 0 && newIndex < scales.size() ) {
		this->scaleIndex = newIndex;
		Notify();
	}
}

Void TaxonomyEditorModel::CheckCategory( const TaxonomyEditorModel::Category* category, Bool checked ) {
	if( category->Checked() != checked ) {
		((Category*)category)->Checked( checked );
		Notify();
	}
}

Int TaxonomyEditorModel::ScaleIndex() const {
	return this->scaleIndex;
}

String TaxonomyEditorModel::Serialize() const {
	Serializer s( scales[scaleIndex]->Identifier() );

	Vector<Category*>::const_iterator i;
	for( i = categories.begin(); i != categories.end(); i++ ) {
		const Category* category = *i;
		if( category->Checked() ) {
			s.Push( category );
			if( category->IsLeaf() ) {
				s.Emit();
			} else {
				category->EnumerateCategories( s );
			}
			s.Pop();
		}
	}

	return s.ToString();
}

// ------------------------------------------------------------------------------------------------
// Implementation of FitsLiberator::Modelling::TaxonomyEditorModel::Scale
// ------------------------------------------------------------------------------------------------

Char TaxonomyEditorModel::Scale::Identifier() const {
	return definition[0];
}

const String& TaxonomyEditorModel::Scale::Name() const {
	return definition;
}

TaxonomyEditorModel::Scale::Scale( const TiXmlElement* element ) {
	this->definition = element->FirstChild()->Value();
}

// ------------------------------------------------------------------------------------------------
// Implementation of FitsLiberator::Modelling::TaxonomyEditorModel::Category
// ------------------------------------------------------------------------------------------------

TaxonomyEditorModel::Category::Category( Category* parent, const TiXmlElement* element ) {
	this->checked = false;
	this->parent = parent;

	const Char* definition = element->Attribute( "Definition" );
	if( definition != NULL ) {
		this->definition = definition;
		for( const TiXmlNode* node = element->FirstChildElement(); node; node = node->NextSiblingElement() ) {
			String name = node->Value();
			if( name == "Category" ) {
				this->subCategories.push_back( new Category( this, node->ToElement() ) );
			}
		}
	}
}

TaxonomyEditorModel::Category::~Category() {
	for( Vector<Category*>::iterator i = subCategories.begin(); i != subCategories.end(); i++ )
		delete (*i);
}

const TaxonomyEditorModel::Category* TaxonomyEditorModel::Category::Parent() const {
	return this->parent;
}

String TaxonomyEditorModel::Category::Identifier() const {
	return definition.substr( 0, definition.find_first_of( '.' ) );
}

const String& TaxonomyEditorModel::Category::Name() const {
	return this->definition;
}

Bool TaxonomyEditorModel::Category::Checked() const {
	return this->checked;
}

Void TaxonomyEditorModel::Category::Checked( Bool checked ) {
	this->checked = checked;
}

Bool TaxonomyEditorModel::Category::Editable() const {
	return (this->definition.find_first_of( '[' ) == -1);
}

Int TaxonomyEditorModel::Category::CategoryCount() const {
	return this->subCategories.size();
}

Bool TaxonomyEditorModel::Category::IsLeaf() const {
	if( CategoryCount() > 0 ) {
		Vector<Category*>::const_iterator i;
		for( i = subCategories.begin(); i != subCategories.end(); i++ )
			if( (*i)->Checked() )
				return false;
	}
	return true;
}