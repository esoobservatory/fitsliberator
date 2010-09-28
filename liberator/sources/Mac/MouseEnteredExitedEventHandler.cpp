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

#include "MouseEnteredExitedEventHandler.h"

using namespace FitsLiberator::Mac;

//-------------------------------------------------------------------------------
//	MouseEnteredExitedEventHandler implementation
//-------------------------------------------------------------------------------

/**
 * Destructor. Makes sure all heap allocated structures gets disposed.
 */
MouseEnteredExitedEventHandler::~MouseEnteredExitedEventHandler() {
	MouseEventList::iterator it;

	for( it = mouseEnteredExitedList.begin(); it != mouseEnteredExitedList.end(); it++ ) {
		if( mouseEnteredExitedList[(*it).first] != NULL ) {
			delete mouseEnteredExitedList[(*it).first];
		}
	}
}

/**
 * Install a mouse entered/exited for BaseControl and send events to the ControlEventHandler. 
 *
 * @param handler	ControlEventHandler to receive onMouseEntered and onMouseExited events.
 * @param control	Control which should be watched.
 */
Void MouseEnteredExitedEventHandler::installMouseEnteredExited( ControlEventHandler * handler, BaseControl * control ) {
	static ControlState *temp;
	
	temp 			= new ControlState;
	temp->control	= control;
	temp->state		= false;

	mouseEnteredExitedList[handler] = temp;
}

/**
 * Removes an mouse entered/exited event handler.
 *
 * @param handler	The ControlEventHandler that have previous used installMouseEnteredExited
 *					to receive onMouseEntered/onMouseExited events.
 */
Void MouseEnteredExitedEventHandler::removeMouseEnteredExited( ControlEventHandler * handler ) {
	if( mouseEnteredExitedList[handler] != NULL ) {
		delete mouseEnteredExitedList[handler];
	}
	
	mouseEnteredExitedList.erase( handler );
}

/**
 * Enables events dispatching.
 */
Void MouseEnteredExitedEventHandler::enable() {
	active = true;
}

/**
 * Disables events dispatching, so no EventHandlers receives onMouseEntered/onMousedExited events.
 */
Void MouseEnteredExitedEventHandler::disable() {
	active = false;
}


/**
 * Mouse moved event handler. Implements similar functionality as MouseTrackingRegions
 *
 * Dispatches onMouseEntered and onMouseExited events.
 */
OSStatus MouseEnteredExitedEventHandler::onMouseMoved( EventRef event ) {
	if( active && !mouseEnteredExitedList.empty() ) {
		::Point mouseLocation;
	
    	::GetEventParameter( event, kEventParamMouseLocation, ::typeQDPoint, NULL, sizeof( ::Point ), NULL, &mouseLocation );
    	
		dialog->globalToLocal( &mouseLocation );
		
		MouseEventList::iterator it;
		static Rect controlRect;
		
		for( it = mouseEnteredExitedList.begin(); it != mouseEnteredExitedList.end(); it++ ) {
			((*it).second)->control->getBounds( &controlRect );
		
			if( ::PtInRect( mouseLocation, &controlRect ) ) {
				if( !((*it).second)->state ) {
					mouseEnteredExitedList[(*it).first]->state = true;						
					return (*(*it).first).onMouseEntered( event );	
				}
			} else {
				if( ((*it).second)->state ) {
					mouseEnteredExitedList[(*it).first]->state = false;
					return (*(*it).first).onMouseExited( event );
				}
			}	
		}
	} 
	
	return eventNotHandledErr;
}