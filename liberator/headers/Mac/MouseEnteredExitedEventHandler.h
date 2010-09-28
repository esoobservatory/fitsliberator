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

#ifndef __MOUSEENTEREDEXITEDEVENTHANDLER_H__
#define __MOUSEENTEREDEXITEDEVENTHANDLER_H__

#include "Types.h"
#include "EventHandler.h"
#include "BaseControl.h"
#include <ext/hash_map>

namespace __gnu_cxx {
	template<> struct hash<FitsLiberator::Mac::ControlEventHandler *> {
		size_t operator()( FitsLiberator::Mac::ControlEventHandler * const & x) const { return size_t( x ); }
	};
}

namespace FitsLiberator {
	namespace Mac {

/**
 * Structure to hold information needed in mouseEnteredExitedList.
 */
struct ControlState {
	BaseControl * 	control;	///< Reference to the control which we're tracking.
	Boolean 		state;		///< True if we are in control, false if not.
};

/**
 * Needed for MouseEventList hash_map.
 */
//struct eqMouseList { bool operator()(ControlEventHandler * s1, ControlEventHandler * s2) const { return (s1 == s2); } };

/**
 * Hash map definition to hold control event handlers and which control they are tracking including state.
 */
typedef __gnu_cxx::hash_map< ControlEventHandler *, ControlState *> MouseEventList;

/**
 * MouseEventHandler that dispatches onMouseEntered and onMouseExited events.
 */
class MouseEnteredExitedEventHandler : public MouseEventHandler {
	public:
		MouseEnteredExitedEventHandler( BaseDialog * dlog ) : active ( true ), dialog( dlog ) {};
		~MouseEnteredExitedEventHandler();
	
		virtual OSStatus		onMouseMoved( EventRef event );	

		Void					installMouseEnteredExited( ControlEventHandler * handler, BaseControl * control );
		Void					removeMouseEnteredExited( ControlEventHandler * handler );
		
		Void 					disable();
		Void 					enable();
		
	protected:
		MouseEventList			mouseEnteredExitedList;	///< List of EventHandlers that must be notified.
		Boolean					active;					///< Whether the mouse tracking is enabled/disabled.
		BaseDialog *			dialog;					///< Reference to dialog to track mouse in.
};

	} // namespace Mac end
} // namespace FitsLiberator end

#endif //__MOUSEENTEREDEXITEDEVENTHANDLER_H__