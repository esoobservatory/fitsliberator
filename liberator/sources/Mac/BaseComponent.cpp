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
 
#include "BaseComponent.h"

using namespace FitsLiberator::Mac;

//-------------------------------------------------------------------------------
//	Event Dispatcher
//-------------------------------------------------------------------------------

/**
 * Replacement for TrackMouseLocation which also takes key up and down events into account.
 */
Void FitsLiberator::Mac::TrackMouseLocationAndKey( ::Point* p, MouseTrackingResult* outResult, Char* outKeyPressed )
{
	static const EventTypeSpec events[] = {
		{ kEventClassMouse, kEventMouseUp },
		{ kEventClassMouse, kEventMouseDragged },
		{ kEventClassKeyboard, kEventRawKeyDown },
		{ kEventClassKeyboard, kEventRawKeyUp },
		{ kEventClassKeyboard, kEventRawKeyModifiersChanged }
	};

	*outResult = 0;

	while ( *outResult == 0 )
	{
		EventRef event;

		if ( ::ReceiveNextEvent( GetEventTypeCount( events ), events, kEventDurationForever, true, &event ) == noErr ) {
			switch ( ::GetEventClass( event ) ) {
				case kEventClassMouse:
					::GetGlobalMouse( p );
				
					switch ( GetEventKind( event ) ) {
						case kEventMouseUp:
							*outResult = kMouseTrackingMouseUp;
							break;
						case kEventMouseDragged:
							*outResult = kMouseTrackingMouseDragged;
							break;
					}
					break;

				case kEventClassKeyboard:
					switch ( ::GetEventKind( event ) ) {
						case kEventRawKeyDown:  {
							char ch;
							::GetEventParameter( event, kEventParamKeyMacCharCodes, typeChar, NULL, 1, NULL, &ch );
							*outKeyPressed 	= ch;
							*outResult 		= kMouseTrackingKeyDown;
							}
							break;
						
						case kEventRawKeyUp:  {
							char ch;
							::GetEventParameter( event, kEventParamKeyMacCharCodes, typeChar, NULL, 1, NULL, &ch );
							*outKeyPressed 	= ch;
							*outResult 		= kMouseTrackingKeyUp;
							}
							break;
						
						case kEventRawKeyModifiersChanged:
							*outResult = kMouseTrackingKeyModifiersChanged;
							break;
					}
					break;
			}
		}
	}
}

/**
 * Dispatch events to the class which registred for the event.
 *
 */
pascal OSStatus FitsLiberator::Mac::eventDispatcher( ::EventHandlerCallRef handler, ::EventRef event, void *userData ) {
	return ((EventHandler *) userData)->processEvent( handler, event );
}

//-------------------------------------------------------------------------------
//	BaseComponent implementation
//-------------------------------------------------------------------------------

/**
 * Installs an event handler.
 */
EventHandlerRef BaseComponent::installBaseEventHandler( ::EventTargetRef target, const ::EventTypeSpec * inList, UInt32 inNumTypes, EventHandler *handler ) {
	OSStatus res = noErr;
	EventHandlerRef handlerRef;
	
	res = ::InstallEventHandler( target, ::NewEventHandlerUPP( eventDispatcher ), inNumTypes, inList, (void *) handler, &handlerRef );

	if( noErr != res && eventHandlerAlreadyInstalledErr != res  ) {
		throw Exception( "Couldn't install event handler." );	
	}
	
	return handlerRef;
}