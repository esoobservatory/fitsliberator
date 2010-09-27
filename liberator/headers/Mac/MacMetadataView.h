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
 
#ifndef __MACMETADATAVIEW_H__
#define __MACMETADATAVIEW_H__

#include "Types.h"
#include "DataBrowser.h"
#include "RepositoryModel.h"
#include "FitsMacTerminology.h"
#include "FitsDialog.h"
#include "RepositoryView.h"
#include "MacTaxonomyEditorView.h"
//#include "MachOFrameworkSupport.h"

namespace FitsLiberator {
	namespace Mac {
	
	pascal OSStatus itemDataCallback( ControlRef, DataBrowserItemID, DataBrowserPropertyID, DataBrowserItemDataRef, Boolean );
	pascal Void 	itemNotificationCallback( ControlRef, DataBrowserItemID, DataBrowserItemNotification );
	pascal Void		itemHelpContentCallback( ControlRef, DataBrowserItemID, DataBrowserPropertyID, HMContentRequest, HMContentProvidedType *, HMHelpContentPtr );
	pascal Void 	drawItemCallback( ControlRef, DataBrowserItemID, DataBrowserPropertyID, DataBrowserItemState, const Rect *, SInt16, Boolean );

	/**
	 *
	 *
	 */
	class MacMetadataView : public FitsLiberator::Modelling::RepositoryView, public ControlEventHandler {
		public:
			MacMetadataView( FitsDialog& dlog,  FitsLiberator::Modelling::TaxonomyEditorModel&, FitsLiberator::Modelling::TaxonomyEditorController&, FitsLiberator::Modelling::RepositoryModel&, FitsLiberator::Modelling::RepositoryController&, FitsLiberator::Modelling::PlaneModel&, FitsLiberator::Modelling::StretchModel&, FitsLiberator::Modelling::HistogramModel& );
			~MacMetadataView();
			
			OSStatus 	itemDataCallback( ControlRef browser, DataBrowserItemID itemID, DataBrowserPropertyID property, DataBrowserItemDataRef itemData, Boolean changeValue );
			Void 		itemNotificationCallback( ControlRef, DataBrowserItemID, DataBrowserItemNotification );
			Void		itemHelpContentCallback( ControlRef, DataBrowserItemID, DataBrowserPropertyID, HMContentRequest, HMContentProvidedType *, HMHelpContentPtr );
			Void 		drawItemCallback( ControlRef, DataBrowserItemID, DataBrowserPropertyID, DataBrowserItemState, const Rect *, SInt16, Boolean );
			OSStatus 	onUnicodeForKeyEvent( EventRef event );
			OSStatus 	onSetFocusPart( EventRef event );
			OSStatus 	onCommand( HICommand * );

		
		protected:
			virtual Void updateListing();
		
		private:
			DataBrowserItemID 	firstSelectableItem();
			DataBrowserItemID 	lastSelectableItem();
			DataBrowserItemID 	nextSelectableItem( DataBrowserItemID itemID );
			DataBrowserItemID 	prevSelectableItem( DataBrowserItemID itemID );
			DataBrowserItemID 	keywordIDToItemID( UInt );
			UInt				itemIDToKeywordID( DataBrowserItemID );
			DataBrowserItemID 	categoryIDToItemID( UInt );
			UInt 				itemIDToCategoryID( DataBrowserItemID );
			Bool 				isCategoryID( DataBrowserItemID );

			DataBrowserControl* 		control;
			FitsDialog& 				dialog;
			DataBrowserCallbacks 		dbCallbacks;
			DataBrowserCustomCallbacks	dbCustomCallbacks;
			UInt						categoryCount;
			UInt						keywordCount;
			
			DataBrowserItemID			selectedItemID;
			TextControl *				comboBox;
			TextControl *				editTextBox;
			TextControl * 				popupBox;
			TextControl * 				categoryButton;
			TextControl *				activeControl;
			HIViewRef					dataBrowserView;
			BaseControl *				verticalScroller;
			DataBrowserItemID			doRevealID;
			Bool 						doingReveal;
			Bool						keepText;
			
			MacTaxonomyEditorView*		editor;
			TaxonomyEditorModel&		taxonomyModel;
			TaxonomyEditorController&	taxonomyController;
	};
		
	} // end namespace Mac
} // end namespace FitsLiberator


#endif //__MACMETADATAVIEW_H__