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
 
#include "EventHandler.h"
#include "MachOFrameworkSupport.h"
#include "Appearance.h"

using namespace FitsLiberator::Mac;

//-------------------------------------------------------------------------------
//	EventHandler implementation
//-------------------------------------------------------------------------------
 
 /** 
 * Process an event.
 */
OSStatus EventHandler::processEvent( ::EventHandlerCallRef handler, ::EventRef event ) {
	switch( ::GetEventClass( event ) ) {
		case kEventClassWindow:
			return processWindowEvent( handler, event );
			break;
		
		case kEventClassControl:
			return processControlEvent( handler, event );
			break;
		
		case kEventClassCommand:
			return processCommandEvent( handler, event );
			break;
		
		case kEventClassMouse:
			return processMouseEvent( handler, event );
			break;
		
		case kEventClassKeyboard:
			return processKeyboardEvent( handler, event );
			break;
			
		case kEventClassTextInput:
			return processTextInputEvent( handler, event );
			break;
	}
	
	return eventNotHandledErr;
}

/**
 * Process a command event
 */
OSStatus EventHandler::processCommandEvent( ::EventHandlerCallRef handler, ::EventRef event ) {
	switch( ::GetEventKind( event ) ) {
		case kEventCommandProcess:
			::HICommand command;
		    
		    ::GetEventParameter( event, ::kEventParamDirectObject, ::typeHICommand, NULL, sizeof (::HICommand), NULL, &command);
		     
			return onCommand( &command );
			break;
			
		case kEventCommandUpdateStatus:
			return onUpdateStatus( event );
			break;
			
		default:
			return eventNotHandledErr;
	}
}

//-------------------------------------------------------------------------------
//	ControlEventHandler implementation
//-------------------------------------------------------------------------------

/**
 * Process a control event, be sending the event to a handler function.
 */
OSStatus ControlEventHandler::processControlEvent( ::EventHandlerCallRef handler, ::EventRef event ) {
    switch( ::GetEventKind( event ) ) {
    	case kEventControlHit: 
    		return onHit( event ); 
    		break;
    		    	
    	case kEventControlHitTest: 
    		return onHitTest( event ); 
    		break;
    		
    	case kEventControlDraw:
    		return onDraw( event );
    		break;
    		
    	case kEventControlTrack:
    		return onTrack( event );
    		break;
    	
    	case kEventControlSetCursor:
    		return onSetCursor( event );
    		break;
    	
    	case kEventControlSetFocusPart:
    		return onSetFocusPart( event );
    		break;
    	
    	case kEventControlActivate:
    		return onActivate( event );
    		break;
    	
    	case kEventControlDeactivate:
    		return onDeactivate( event );
    		break;
    		    		
    	case kEventControlContextualMenuClick: 
    		return onContextualMenuClick( event ); 
    		break;
    
    	default:
    		return eventNotHandledErr;
    }
}

/**
 * Process mouse events
 */
OSStatus ControlEventHandler::processMouseEvent( ::EventHandlerCallRef handler, ::EventRef event ) {
	switch( ::GetEventKind( event ) ) {
		case kEventMouseEntered:
			return onMouseEntered( event );
			break;
		
		case kEventMouseExited:
			return onMouseExited( event );
			break;
			
		case kEventMouseMoved:
			return onMouseMoved( event );
			break;
			
		default:
			return eventNotHandledErr;
	}
}

OSStatus ControlEventHandler::onSetCursor( EventRef event ) {
	::SetThemeCursor( kThemeArrowCursor  );	
	
	return noErr;
}

/**
 * Process text input events
 */
OSStatus ControlEventHandler::processTextInputEvent( ::EventHandlerCallRef handler, ::EventRef event ) {
	switch( ::GetEventKind( event ) ) {
		case kEventTextInputUpdateActiveInputArea:
			return onUpdateActiveInputArea( event );
			break;
		
		case kEventTextInputUnicodeForKeyEvent:
			return onUnicodeForKeyEvent( event );
			break;
		
		case kEventTextInputGetSelectedText:
			return onGetSelectedText( event );
			break;
		
		case kEventTextInputUnicodeText:
			return onUnicodeText( event );
			break;
		
		case kEventTextInputFilterText:
			return onFilterText( event );
			break;
		
		default:
			return eventNotHandledErr;
	}
}

//-------------------------------------------------------------------------------
//	MouseEventHandler implementation
//-------------------------------------------------------------------------------

/**
 * Process mouse events
 */
OSStatus MouseEventHandler::processMouseEvent( ::EventHandlerCallRef handler, ::EventRef event ) {
	switch( ::GetEventKind( event ) ) {
		case kEventMouseMoved:
			return onMouseMoved( event );
			break;
		
		default:
			return eventNotHandledErr;
	}
}