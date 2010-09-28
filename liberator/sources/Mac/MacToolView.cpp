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

#include "MacToolView.h"

using namespace FitsLiberator::Mac;
using namespace FitsLiberator::Modelling;

/**
 *
 */
MacToolView::MacToolView( FitsDialog * dlog, ToolModel& m, ToolController& c ) : ToolView( m, c ), dialog( dlog ) {
	static const EventTypeSpec eventTypes[] = { 
		{ kEventClassCommand, kEventCommandProcess }, 
	};
	
	static const Int eventCount = GetEventTypeCount( eventTypes );
	
	handButton 			= new BevelButtonControl( dlog, kFITSUIBevelButtonHand, 'IcHT' );
	zoomButton 			= new BevelButtonControl( dlog, kFITSUIBevelButtonZoom, 'IcZT' );
	backgroundButton 	= new BevelButtonControl( dlog, kFITSUIBevelButtonBackground, 'IcBG' );
	peakButton 			= new BevelButtonControl( dlog, kFITSUIBevelButtonPeak, 'IcPL' );
	whiteLevelButton 	= new BevelButtonControl( dlog, kFITSUIBevelButtonWhiteLevel, 'IcWT' );
	blackLevelButton 	= new BevelButtonControl( dlog, kFITSUIBevelButtonBlackLevel, 'IcBT' );
	
	BundleFactory *factory = FitsLiberator::Environment::getBundleFactory();
	
	handButton->setIcon( CFSTR("toolMove.icns"), factory );
	zoomButton->setIcon( CFSTR("toolZoom.icns"), factory );
	backgroundButton->setIcon( CFSTR("toolPickerBackground.icns"), factory );
	peakButton->setIcon( CFSTR("toolPickerPeak.icns"), factory );
	whiteLevelButton->setIcon( CFSTR("toolPickerWhite.icns"), factory );
	blackLevelButton->setIcon( CFSTR("toolPickerBlack.icns"), factory );
	
	handButton->installEventHandler( eventTypes, eventCount, this );
	zoomButton->installEventHandler( eventTypes, eventCount, this );
	backgroundButton->installEventHandler( eventTypes, eventCount, this );
	peakButton->installEventHandler( eventTypes, eventCount, this );
	whiteLevelButton->installEventHandler( eventTypes, eventCount, this );
	blackLevelButton->installEventHandler( eventTypes, eventCount, this );
	
	// Install event handler on the window for modifier keys changed.
	static const EventTypeSpec windowEventTypes[] = {
		{ kEventClassKeyboard, kEventRawKeyModifiersChanged }, 
		{ kEventClassTextInput, kEventTextInputUnicodeForKeyEvent },
		{ kEventClassKeyboard, kEventRawKeyUp }
	};
	
	dlog->installEventHandler( windowEventTypes, GetEventTypeCount( windowEventTypes ), this );
}

/**
 *
 */
MacToolView::~MacToolView() {
	delete handButton;
	delete zoomButton;
	delete backgroundButton;
	delete peakButton;
	delete whiteLevelButton;
	delete blackLevelButton;
}

/**
 * Handle only certain keyboard events
 *
 * We only use raw keyboard events for space bar and modifier changes since we also need
 * key up events for space bar.
 */
OSStatus MacToolView::processKeyboardEvent( EventHandlerCallRef handler, EventRef event ) {
	OSStatus result = eventNotHandledErr;
	// 1 means kFITSUITabPanePreview
	if(  dialog->getTabControl().getEnabledTab() == 1 ) {
			Char c;

			switch( ::GetEventKind( event ) ) {
				case kEventRawKeyModifiersChanged: 
					UInt32 		modifiers;
			
					::GetEventParameter( event, ::kEventParamKeyModifiers, typeUInt32, NULL, sizeof (UInt32), NULL, &modifiers );
					
					controller.changeToolFunction( (modifiers & optionKey) && !(modifiers & cmdKey) );
					break;
					
				case kEventRawKeyDown:
					::GetEventParameter( event, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof (Char), NULL, &c );

					// Use only raw keyboard events for space bar					
					if( c == kSpaceCharCode ) {
						if( controller.keyboardShortCut( kSpaceCharCode ) ) result = noErr;
					}
					break;
				
				case kEventRawKeyUp:
					::GetEventParameter( event, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof (Char), NULL, &c );
					
					// Use only raw keyboard events for space bar
					if( c == kSpaceCharCode ) {
						if( controller.keyboardShortCutUp( kSpaceCharCode ) ) result = noErr;
					}
					break;
			}
	}
	
	return result; // We want the event to dispatch to other also.	
}

/**
 * Handle keydown/repeat events.
 */
OSStatus MacToolView::onUnicodeForKeyEvent( EventRef unicodeEvent ) {
	EventRef event = NULL;
	::GetEventParameter( unicodeEvent, kEventParamTextInputSendKeyboardEvent, typeEventRef, NULL, sizeof(EventRef), NULL, &event );

	// 1 means kFITSUITabPanePreview
	if(  dialog->getTabControl().getEnabledTab() == 1 ) {
		Char c;
		
		::GetEventParameter( event, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof (Char), NULL, &c );

		if( ::GetEventKind( event ) == kEventRawKeyDown) {
			if( controller.keyboardShortCut( c ) ) return noErr;
		}
	}
	
	return eventNotHandledErr; // We want the event to dispatch to other also.	
}

/**
 *
 */
OSStatus MacToolView::onCommand( HICommand *command ) {
	switch( command->commandID ) {
		case kFITSUICommandUseHandTool:
			controller.toolHandSelected();
			return noErr;
			break;
			
		case kFITSUICommandUseZoomTool:
			controller.toolZoomSelected();
			return noErr;
			break;
			
		case kFITSUICommandUseWhiteLevelTool:
			controller.toolWhiteLevelSelected();
			return noErr;
			break;
		
		case kFITSUICommandUseBlackLevelTool:
			controller.toolBlackLevelSelected();
			return noErr;
			break;
		
		case kFITSUICommandUseBackgroundTool:
			controller.toolBackgroundSelected();
			return noErr;
			break;
			
		case kFITSUICommandUsePeakTool:
			controller.toolPeakSelected();
			return noErr;
			break;
		
	}

	return eventNotHandledErr;
}

/**
 *
 */
Void MacToolView::setHandStatus( Bool status ) {
	handButton->setValue( (status ? 1 : 0) );
}

/**
 *
 */
Void MacToolView::setPickerStatus( Bool status ) {
	//pickerButton->setValue( (status ? 1 : 0) );
}

/**
 *
 */
Void MacToolView::setEyedropperBackgroundStatus( Bool status ) {
	backgroundButton->setValue( (status ? 1 : 0) );
}

/**
 *
 */
Void MacToolView::setEyedropperPeakStatus( Bool status ) {
	peakButton->setValue( (status ? 1 : 0) );
}

/**
 *
 */
Void MacToolView::setEyedropperBlacklevelStatus( Bool status ) {
	blackLevelButton->setValue( (status ? 1 : 0) );
}

/**
 *
 */
Void MacToolView::setEyedropperWhitelevelStatus( Bool status ) {
	whiteLevelButton->setValue( (status ? 1 : 0) );
}

/**
 *
 */
Void MacToolView::setMagnifierZoomStatus( Bool status ) {
	zoomButton->setValue( (status ? 1 : 0) );
}

