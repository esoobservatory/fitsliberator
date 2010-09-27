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
#include "TaxonomyEditor.h"
#include "ResourceManager.h"

using namespace FitsLiberator::Modelling;
using namespace FitsLiberator::Windows;

struct TaxonomyEditor::CategoryInitializer {
private:
	HWND treeView;
	HTREEITEM root;
	TaxonomyEditor& owner;
public:
	CategoryInitializer( TaxonomyEditor& owner ) 
	  : owner( owner ) {
		this->treeView = owner.getItem( IDC_TAXONOMY_CATEGORIES );
		this->root = NULL;
	}

	Void operator()( const TaxonomyEditorModel::Category* category ) {
		CategoryInitializer initializer = *this;	// Clones

		TVINSERTSTRUCT tvi = {0};
		tvi.hParent = root;
		tvi.hInsertAfter = TVI_LAST;
		tvi.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE;
		tvi.item.pszText = (LPSTR)category->Name().c_str();
		tvi.item.lParam = reinterpret_cast<LPARAM>(category);
		tvi.item.stateMask = TVIS_STATEIMAGEMASK | TVIS_EXPANDED;
		if( category->Editable() ) {
			tvi.item.state = INDEXTOSTATEIMAGEMASK( category->Checked() ? 2 : 1 );
		} else {
			tvi.item.state = INDEXTOSTATEIMAGEMASK( 0 );
		}
		if( category->Checked() ) {
			tvi.item.state |= TVIS_EXPANDED;
		}

		initializer.root = TreeView_InsertItem( treeView, &tvi );
		owner.Map( category, initializer.root );

		category->EnumerateCategories( initializer );
	}
};

struct TaxonomyEditor::ScaleInitializer {
private:
	ComboBox& dropDown;
public:
	ScaleInitializer( ComboBox& d ) : dropDown( d ) {}
	Void operator()( const TaxonomyEditorModel::Scale* scale ) {
		dropDown.AddItem( scale->Name() );
	}
};

TaxonomyEditor::TaxonomyEditor( TaxonomyEditorModel& model, TaxonomyEditorController& controller ) 
  : super( IDD_TAXONOMYEDITOR ), MappedTaxonomyEditorView( model, controller ) {

	checkBoxes = ImageList_Create( 17, 17, ILC_MASK, 3, 0 );
	if( checkBoxes != NULL ) {
		HBITMAP bmp;
		
		bmp = LoadBitmap( g_hInstance, MAKEINTRESOURCE( IDB_CHECKBOX_EMPTY ) );
		ImageList_AddMasked( checkBoxes, bmp, RGB(0xFF, 0xFF, 0xFF) );
		::DeleteObject( bmp );

		bmp = LoadBitmap( g_hInstance, MAKEINTRESOURCE( IDB_CHECKBOX_UNCHECKED ) );
		ImageList_AddMasked( checkBoxes, bmp, RGB(0xFF, 0xFF, 0xFF) );
		::DeleteObject( bmp );

		bmp = LoadBitmap( g_hInstance, MAKEINTRESOURCE( IDB_CHECKBOX_CHECKED ) );
		ImageList_AddMasked( checkBoxes, bmp, RGB(0xFF, 0xFF, 0xFF) );
		::DeleteObject( bmp );

		if( ImageList_GetImageCount( checkBoxes ) != 3 ) {
			ImageList_Destroy( checkBoxes );
			checkBoxes = NULL;
		}
	}
}

Bool TaxonomyEditor::OnInit( HWND hWnd ) {
	if( !super::OnInit( hWnd ) )
		return false;

	//
	// Populate the combobox
	ComboBox scale;
	scale.setHandle( getItem( IDC_TAXONOMY_SCALE ) );
	ScaleInitializer scaleInitializer( scale );
	model.EnumerateScales( scaleInitializer );
	scale.setSelectedItem( model.ScaleIndex() );

	HWND treeView = getItem( IDC_TAXONOMY_CATEGORIES );
	SetWindowLong( treeView, GWL_STYLE, GetWindowLong( treeView, GWL_STYLE ) | TVS_NOHSCROLL );
	TreeView_SetImageList( treeView, checkBoxes, TVSIL_STATE );

	CategoryInitializer categoryInitializer( *this );
	model.EnumerateCategories( categoryInitializer );

	return true;
}

Void TaxonomyEditor::OnDestroy( HWND ) {
	if( checkBoxes != NULL )
		ImageList_Destroy( checkBoxes );
}

Void TaxonomyEditor::OnCommand( WPARAM wParam, LPARAM ) {
    Int command = LOWORD(wParam);
	switch( command ) {
		case IDOK:
		case IDCANCEL:
			Close( command );
			break;
		case IDC_TAXONOMY_SCALE:
			if( HIWORD(wParam) == CBN_SELCHANGE ) {
				ComboBox scale;
				scale.setHandle( getItem( IDC_TAXONOMY_SCALE ) );
				OnScaleSelectionChanged( scale.getSelectedItem() );
			}
			break;
	}
}

LRESULT TaxonomyEditor::OnNotify( Int idCtrl, LPNMHDR pnmh ) {
	if( idCtrl == IDC_TAXONOMY_CATEGORIES ) {
		if( pnmh->code == NM_CLICK ) {
			TVHITTESTINFO ht;

			::GetCursorPos( &ht.pt );
			::ScreenToClient( getItem( IDC_TAXONOMY_CATEGORIES ), &ht.pt );

			TreeView_HitTest( pnmh->hwndFrom, &ht );
			if( ht.hItem != NULL && (ht.flags & TVHT_ONITEMSTATEICON) ) {
				TVITEM item;
				item.hItem = ht.hItem;
				item.mask = TVIF_PARAM | TVIF_STATE | TVIF_HANDLE;
				item.stateMask = TVIS_STATEIMAGEMASK;
				TreeView_GetItem( pnmh->hwndFrom, &item );

				const TaxonomyEditorModel::Category* category = reinterpret_cast<const TaxonomyEditorModel::Category*>( item.lParam );
				OnCategoryClicked( category );
			}
		}
	}
	return super::OnNotify( idCtrl, pnmh );
}

HBRUSH TaxonomyEditor::OnCtlColor( HWND hWnd, HDC hDC, UINT ctlType ) {
	if( ctlType == CTLCOLOR_STATIC ) 
		return ::GetSysColorBrush( COLOR_WINDOW );
	return super::OnCtlColor( hWnd, hDC, ctlType );
}

Void TaxonomyEditor::UpdateScale( Int ) {
	// The ComboBox handles this automatically
	//scale.setSelectedItem( newIndex );
}

Void TaxonomyEditor::UpdateCategory( const TaxonomyEditorModel::Category* category ) {
	if( category->Editable() )
		TreeView_SetCheckState( getItem( IDC_TAXONOMY_CATEGORIES ), Map( category ), category->Checked() ? TRUE : FALSE );
}
