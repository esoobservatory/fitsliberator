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
#include "MouseTracker.h"
#include "FitsBehavior.h"

using namespace FitsLiberator;
using namespace FitsLiberator::Windows;

/**
 * Default constructor.
 * @param sink Event sink that recieves tracking events.
 * @param activeRegion The region in which tracking is enabled.
 */
MouseTracker::MouseTracker( IMouseTrackerSink* sink, const RECT& activeRegion ) : trackerSink( sink ) {
    this->activeRegion = activeRegion;
    this->tracking = false;
}

/**
 * Event handler, forwarded by a control when a mouse button is pressed.
 * @see Window::OnMouseDown for parameters.
 * @return MouseActionTracking if the user initiated a drag or MouseActionNone otherwise.
 */
MouseAction MouseTracker::OnMouseDown( HWND hWnd, Int button, Int x, Int y ) {
    anchor.x = x;
    anchor.y = y;

    this->button = 0;
    if( ::PtInRect( &activeRegion, anchor ) ) {
        this->button = button;
        if( button & MK_LBUTTON) {
            // User pressed the left key down
            tracking = true;
            trackerSink->OnTrackingBegin( *this, anchor );
            ::SetCapture( hWnd );
            return MouseActionTracking;
        }
    }
    return MouseActionNone;
}

/**
 * Event handler, forwarded by a control when the mouse moves.
 * @see Window::OnMouseMove for parameters.
 * @return MouseActionTracking if the user initiated is still dragging or MouseActionNone otherwise.
 */
MouseAction MouseTracker::OnMouseMove( HWND hWnd, Int, Int, Int ) {
    if( tracking ) {
        // We can't rely on the x and y passed to the function.
        POINT pt;
        ::GetCursorPos( &pt );
        ::ScreenToClient( hWnd, &pt );

        trackerSink->OnTrackingMove( *this, pt );

        return MouseActionTracking;
    }
    return MouseActionNone;
}

/**
 * Event handler, forwarded by a control when a mouse button is released.
 * @see Window::OnMouseUp for parameters.
 * @return A MouseAction indicating whether the user pressed the left or right button, or if the user selected an area.
 */
MouseAction MouseTracker::OnMouseUp( HWND hWnd, Int, Int, Int ) {
    if( tracking ) {
        ::ReleaseCapture();
        tracking = false;

        // We can't rely on the x and y passed to the function.
        POINT pt;
        ::GetCursorPos( &pt );
        ::ScreenToClient( hWnd, &pt );

        trackerSink->OnTrackingEnd( *this, pt );

        // If the user dragged the mouse long enough report a selection
        if( abs( pt.x - anchor.x ) > kFITSDragMarginX || abs( pt.y - anchor.y) > kFITSDragMarginY )
            return MouseActionTracking;
    }
    if( button & MK_LBUTTON )
        return MouseActionLeftClick;
    if( button & MK_RBUTTON )
        return MouseActionRightClick;
    return MouseActionNone;
}

/**
 * Updates the active region.
 * @param value The new active region.
 */
Void MouseTracker::setActiveRegion( const RECT& value ) {
    this->activeRegion = value;
}

/**
 * Changes the event sink.
 * @param value The new event sink.
 */
Void MouseTracker::setTrackerSink( IMouseTrackerSink* value ) {
    this->trackerSink = value;
}
