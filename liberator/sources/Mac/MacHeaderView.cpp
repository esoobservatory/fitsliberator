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

#include "MacHeaderView.h"

#include "MacTextEditor.h"
#include "ATSUnicode.h"
#include "SFNTTypes.h"
#include "TextCommon.h"

using namespace FitsLiberator::Mac;

/**
 * Consutrctor
 *
 * @param dlog	Dialog in which the view's gui components resides.	
 * @param m		Model of the view.
 */
MacHeaderView::MacHeaderView( FitsDialog *dlog, FitsLiberator::Modelling::HeaderModel& m  ) 
: HeaderView( m ), textViewControl( NULL ), contextualMenu( NULL ), dialog(dlog)  {

	// Menu
	static const EventTypeSpec menuEventTypes[] = { 
		{ kEventClassCommand, kEventCommandProcess }
  	};
  	
  	contextualMenu = new BaseMenu();
  	contextualMenu->create( Environment::getBundleFactory(), CFSTR( kFITSNibFile ), CFSTR( kFITSNibHeaderMenu ) );
  	contextualMenu->installEventHandler( menuEventTypes, GetEventTypeCount( menuEventTypes ), this );
	
	// Text Control
	textViewID.signature 	= kFITSApplicationSignature;
	textViewID.id 			= kFITSUITextViewHeader;
	
	MachO::HIViewFindByID( MachO::HIViewGetRoot( dlog->getWindow() ), textViewID, &textView );

	static const ::EventTypeSpec eventTypes[] = { 
		{ kEventClassControl, kEventControlContextualMenuClick },
		{ kEventClassMouse, kEventMouseEntered },
		{ kEventClassMouse, kEventMouseExited },
		{ kEventClassTextInput, kEventTextInputUnicodeForKeyEvent }
	};
		
	textViewControl = new BaseControl( dlog, kFITSUITextViewHeader );		
	textViewControl->installEventHandler( eventTypes, GetEventTypeCount( eventTypes ), this );
	
	// Mouse tracking region
	Rect addBounds;
	
	MouseTrackingRegionID trackID = { kFITSApplicationSignature, kFITSUIEditTextViewHeaderRegion };
	
	textViewControl->getBounds( &ctrlBounds );
		
	BaseControl tmpControl( dlog, kFITSUIScrollViewHeader );
	tmpControl.getBounds( &addBounds );
	
	::OffsetRect( &ctrlBounds, addBounds.left, addBounds.top );
	dlog->createMouseTrackingRegion( &ctrlBounds, trackID, textViewControl );
	
	// Install unicode event handler for window.
	static const ::EventTypeSpec inputEventTypes[] = {
		{ kEventClassTextInput, kEventTextInputUnicodeForKeyEvent }
	};
	
	dlog->installEventHandler( inputEventTypes, GetEventTypeCount( inputEventTypes ), this );
}

/**
 * Desctructor deallocates controls.
 */
MacHeaderView::~MacHeaderView() {
	if( textViewControl != NULL ) 	{ delete textViewControl; }
	if( contextualMenu != NULL ) 	{ delete contextualMenu; }
}

/**
 *
 *
 */
OSStatus MacHeaderView::onMouseEntered( EventRef event ) {
	::SetThemeCursor( kThemeIBeamCursor  );
	
	return noErr;
}

/**
 *
 *
 */
OSStatus MacHeaderView::onMouseExited( EventRef event ) {
	::SetThemeCursor( kThemeArrowCursor  );
	
	return noErr;
}


/**
 * Handle a contextual menu click in the text view. Change the cursor accordingly.
 */
OSStatus MacHeaderView::onContextualMenuClick( EventRef event ) {
	ControlRef ctrl;
	
	::GetEventParameter( event, ::kEventParamDirectObject, typeControlRef, NULL, sizeof (ControlRef), NULL, &ctrl );

	if( ( ctrl == textViewControl->getControl() ) && (contextualMenu != NULL ) ) {
		::Point mouseLocation;
    
		//::DisableMenuCommand( contextualMenu->getMenu(), kHICommandCut );
		//::DisableMenuCommand( contextualMenu->getMenu(), kHICommandPaste );
		//::DisableMenuCommand( contextualMenu->getMenu(), kHICommandClear );
		TXNObject textObj = MachO::HITextViewGetTXNObject( textView );
		
		if( ::TXNIsSelectionEmpty( textObj ) ) {
			::DisableMenuCommand( contextualMenu->getMenu(), kFITSUICommandMenuCopy );
		} else {
			::EnableMenuCommand( contextualMenu->getMenu(), kFITSUICommandMenuCopy );
		}
		
    
    	::GetMouse( &mouseLocation );
    	::PopUpMenuSelect( contextualMenu->getMenu(), mouseLocation.v, mouseLocation.h, 0 );
		::GetMouse( &mouseLocation );

    	dialog->translateScreenToControl( &mouseLocation, textViewControl );
    	
    	// onMouseExited takes care of the cursor, if it's not in the control.
    	if( ::PtInRect( mouseLocation, &ctrlBounds ) ){
			::SetThemeCursor( kThemeIBeamCursor  );
		} 
		    	
		return noErr;
    } else {
    	return eventNotHandledErr;	
    }
}

/**
 * Event handler for HeaderViw. 
 */
OSStatus MacHeaderView::onCommand( HICommand *command ) {
	OSStatus result = eventNotHandledErr;
	
	if(  dialog->getTabControl().getEnabledTab() == 3 ) {
		TXNObject textObj = MachO::HITextViewGetTXNObject( textView );

		switch( command->commandID ) {			
			case kHICommandCopy:
			case kFITSUICommandMenuCopy:
				if( !::TXNIsSelectionEmpty( textObj ) ) {
					::TXNCopy( textObj );
				}
				result = noErr;
				break;

			case kHICommandSelectAll:
			case kFITSUICommandMenuSelectAll:
				::TXNSelectAll( textObj );  
				result = noErr;
				break;
		}
	}
	
	return result;
}

/**
 * Update preview header box
 *
 * @param str The header as a string.
 */
Void MacHeaderView::updateHeader( String text ) {
	OSErr status;
	
	// Get HITextView
	TXNObject textObject = MachO::HITextViewGetTXNObject( textView );
	
	// Turn off read-only or else stuff cant't be changed
	TXNControlTag ctrlTag[] = { kTXNIOPrivilegesTag };
	TXNControlData ctrlData[1];
	ctrlData[0].uValue = kTXNReadWrite;
	
	status = ::TXNSetTXNObjectControls( textObject, false, 1, ctrlTag, ctrlData );	
	
	// Insert FITS header in text view
	status = ::TXNRevert( textObject );
	status = ::TXNSetData( textObject, kTXNTextData, text.c_str(), text.length(), 0, 0 );
	
	// Get mono-spaced font
	ATSUFontID theFontID;
	status = ::ATSUFindFontFromName("Monaco", strlen("Monaco"), kFontFullName, kFontNoPlatform, kFontNoScript, kFontNoLanguage, &theFontID); 

	// Set font for text view
	TXNTypeAttributes attributes[2];
	
	attributes[0].tag 				= kATSUFontTag;
	attributes[0].size 				= sizeof(ATSUFontID);
	attributes[0].data.dataValue 	= theFontID;
	
	attributes[1].tag 				= kTXNQDFontSizeAttribute;
	attributes[1].size 				= kTXNQDFontSizeAttributeSize;
	attributes[1].data.dataValue 	= 11 << 16; // Font size
	
	status = ::TXNSetTypeAttributes( textObject, 2, attributes, kTXNStartOffset,  kTXNEndOffset );		
	
	ctrlData[0].uValue = kTXNReadOnly;
	status = ::TXNSetTXNObjectControls( textObject, false, 1, ctrlTag, ctrlData );
}

/**
 * Handle keyboard events.
 *
 * @TODO	For some strange reason, the menu commands won't work even though the menu has been
 *			inserted into the menu list. So we're making a workaround!
 */
//OSStatus MacHeaderView::processKeyboardEvent( EventHandlerCallRef handler, EventRef event ) {
OSStatus MacHeaderView::onUnicodeForKeyEvent( EventRef unicodeEvent ) {
	
	OSStatus 	res = eventNotHandledErr;
	
	EventRef event = NULL;
	::GetEventParameter( unicodeEvent, kEventParamTextInputSendKeyboardEvent, typeEventRef, NULL, sizeof(EventRef), NULL, &event );
	
	// 3 means kFITSUITabPaneHeader
	if(  dialog->getTabControl().getEnabledTab() == 3 ) {	
		
		MachO::HIViewAdvanceFocus( textView, 0 );
		TXNObject textObj = MachO::HITextViewGetTXNObject( textView );
		

		switch( ::GetEventKind( event ) ) {
			case kEventRawKeyDown:
				UInt32 	modifiers;
				Char 	c;
					
				::GetEventParameter( event, kEventParamKeyModifiers, typeUInt32, NULL, sizeof (UInt32), NULL, &modifiers );
				::GetEventParameter( event, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof (Char), NULL, &c );
				
				if( (modifiers & cmdKey) && !(modifiers & shiftKey) && !(modifiers & optionKey) && !(modifiers & controlKey) && !(modifiers & rightShiftKey) && !(modifiers & rightOptionKey) && !(modifiers & rightControlKey) ) {
					switch( c ) {
						case 'C':
						case 'c':
							if( !::TXNIsSelectionEmpty( textObj ) ) {
								::TXNCopy( textObj );
							}
							res = noErr;
							break;
						case 'A':
						case 'a':
							::TXNSelectAll( textObj );
							res = noErr;
							break;
					}
				}
				break;
		}
	}
	
	return res;
}

