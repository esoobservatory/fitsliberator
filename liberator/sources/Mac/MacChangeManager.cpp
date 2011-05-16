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

#include "MacChangeManager.h"

using FitsLiberator::Mac::MacChangeManager;

MacChangeManager::MacChangeManager() : super() {
	mainThread = currentThread();
}

MacChangeManager::~MacChangeManager() {}

void MacChangeManager::SendNotifications() {
	if(currentThread() != mainThread) {
		// We have to do marshalling
		EventRef event;
		OSStatus error;
	
		error = ::CreateEvent(kCFAllocatorDefault, 
			kEventClassCustom, kEventKindCustomUpdate, 
			0, kEventAttributeNone, 
			&event);
		if(noErr == error) {
			error = ::PostEventToQueue(::GetMainEventQueue(), event, kEventPriorityHigh);
			if(noErr != error) {
				throw Exception("Failed to post event");
			}
			::ReleaseEvent(event);
		} else {
			throw Exception("Failed to create event");
		}
	} else {
		// We are already on the main thread
		Invoke();
	}
}

void MacChangeManager::Invoke() {
    super::SendNotifications();    
}

ThreadID MacChangeManager::currentThread() {
	ThreadID thread;
	if(::GetCurrentThread(&thread) != noErr)
		throw Exception("Apparently I am running threadless");
	return thread;
}

