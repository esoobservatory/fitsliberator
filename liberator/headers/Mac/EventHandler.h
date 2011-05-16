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

#ifndef __EVENTHANDLER_H__
#define __EVENTHANDLER_H__

#include "Types.h"
#include "Carbon.h"

namespace FitsLiberator {
	namespace Mac {

/**
 * EventHandler abstract class. All classes that whish to receive UI events, must 
 * implement this interface.
 */
class EventHandler {
	public:
		virtual ~EventHandler() {}
		virtual OSStatus 	processEvent( EventHandlerCallRef handler, EventRef event );
		virtual OSStatus 	processCommandEvent( EventHandlerCallRef handler, EventRef event );
		virtual OSStatus 	processWindowEvent( EventHandlerCallRef handler, EventRef event ){ return eventNotHandledErr; };
		virtual OSStatus 	processControlEvent( EventHandlerCallRef handler, EventRef event ){ return eventNotHandledErr; };
		virtual OSStatus 	processMouseEvent( EventHandlerCallRef handler, EventRef event ){ return eventNotHandledErr; };
		virtual OSStatus 	processKeyboardEvent( EventHandlerCallRef handler, EventRef event ){ return eventNotHandledErr; };
		virtual OSStatus 	processTextInputEvent( EventHandlerCallRef handler, EventRef event ){ return eventNotHandledErr; };
	
		virtual OSStatus	onCommand( HICommand *command ) { return eventNotHandledErr; };
		virtual OSStatus	onUpdateStatus( EventRef event ) { return eventNotHandledErr; };
};

/**
 * Event handler for controls.
 */
class ControlEventHandler : public EventHandler {
	public:
		virtual OSStatus	processControlEvent( EventHandlerCallRef handler, EventRef event );
		virtual OSStatus	processMouseEvent( EventHandlerCallRef handler, EventRef event );
		
		// Control event class handling functions		
		virtual OSStatus 	onHit( EventRef event ) { return eventNotHandledErr; };
		virtual OSStatus 	onHitTest( EventRef event ) { return eventNotHandledErr; };
		virtual OSStatus 	onSetCursor( EventRef event );// { return eventNotHandledErr; };
		virtual OSStatus 	onDraw( EventRef event ) { return eventNotHandledErr; };
		virtual OSStatus 	onTrack( EventRef event ) { return eventNotHandledErr; };
		virtual OSStatus 	onSetFocusPart( EventRef event ) { return eventNotHandledErr; };
		virtual OSStatus 	onActivate( EventRef event ) { return eventNotHandledErr; };
		virtual OSStatus 	onDeactivate( EventRef event ) { return eventNotHandledErr; };
		virtual OSStatus	onContextualMenuClick( EventRef event ) { return eventNotHandledErr; };
		virtual OSStatus	onMouseEntered( EventRef event ) { return eventNotHandledErr; };
		virtual OSStatus	onMouseExited( EventRef event ) { return eventNotHandledErr; };
		virtual OSStatus	onMouseMoved( EventRef event ) { return eventNotHandledErr; };
		
		virtual OSStatus 	processTextInputEvent( EventHandlerCallRef handler, EventRef event );
		
		// Text input event class handling functions	
		virtual OSStatus	onUpdateActiveInputArea( EventRef event ) { return eventNotHandledErr; };
		virtual OSStatus	onUnicodeForKeyEvent( EventRef event ) { return eventNotHandledErr; };
		virtual OSStatus 	onGetSelectedText( EventRef event ) { return eventNotHandledErr; };
		virtual OSStatus	onUnicodeText( EventRef event ) { return eventNotHandledErr; };
		virtual OSStatus	onFilterText( EventRef event ) { return eventNotHandledErr; };
};

/**
 * Event handler for mouse moved events.
 *
 */
class MouseEventHandler : public EventHandler {
	public:
		virtual OSStatus	processMouseEvent( EventHandlerCallRef handler, EventRef event );
		virtual OSStatus	onMouseMoved( EventRef event ) { return eventNotHandledErr; };	
};

	} // namespace Mac end
} // namespace FitsLiberator end

#endif //__EVENTHANDLER_H__