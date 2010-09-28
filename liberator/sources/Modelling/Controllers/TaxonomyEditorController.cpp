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
#include "TaxonomyEditorController.h"

using namespace FitsLiberator::Modelling;

struct TaxonomyEditorController::Unchecker {
private:
	TaxonomyEditorController& controller;
public:
	Unchecker( TaxonomyEditorController& c ) : controller( c ) {}
	Void operator()( const TaxonomyEditorModel::Category* category ) {
		controller.UpdateCategory( category, false );
		if( category->CategoryCount() > 0 ) {
			category->EnumerateCategories( *this );
		}
	}
};

TaxonomyEditorController::TaxonomyEditorController( TaxonomyEditorModel& m, AccumulatingChangeManager *manager ) 
  : super( manager ), model( m ) {
	this->activeCategory = NULL;
}

Void TaxonomyEditorController::SetScaleIndex( Int newIndex ) {
	model.SetScaleIndex( newIndex );
	super::SendNotifications();
}

Void TaxonomyEditorController::ToggleCategory( const TaxonomyEditorModel::Category* category ) {
	if( category != NULL && category->Editable() ) {
		if( category->Checked() ) {
			Unchecker f( *this );
			f( category );
			// The unchecker removes the checkmark for the specified category and all child categories,
			// but if the category parent is non-editable, we also need to consider the parents of the category.
			const TaxonomyEditorModel::Category* parent = category->Parent();
			while( parent != NULL && !parent->Editable() && parent->IsLeaf() ) {
				UpdateCategory( parent, false );
				parent = parent->Parent();
			}
		}
		else {
			while( category != NULL ) {
				UpdateCategory( category, true );
				category = category->Parent();
			}
		}
	}
}

Void TaxonomyEditorController::UpdateCategory( const TaxonomyEditorModel::Category* category, Bool checked ) {
	activeCategory = category;
	model.CheckCategory( category, checked );
	super::SendNotifications();
	activeCategory = NULL;
}

const TaxonomyEditorModel::Category* TaxonomyEditorController::ActiveCategory() const {
	return activeCategory;
}