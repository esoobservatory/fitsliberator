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
#include "SelectionTrackerSink.h"

using namespace FitsLiberator::Windows;

/**
 * Default constructor.
 * @param hWnd Window handle of the owner control.
 * @param activeRegion The active region.
 */
SelectionTrackerSink::SelectionTrackerSink( HWND hWnd, const RECT& activeRegion ) {
    this->hWnd = hWnd;
    this->activeRegion = activeRegion;
}

/**
 * Event handler, called by the mouse tracker while dragging.
 * @param location Current mouse location.
 */
Void SelectionTrackerSink::OnTrackingBegin( MouseTracker& tracker, const POINT& location ) {
    selectionAnchor = location;
    selectionRegion.left = selectionRegion.right = location.x;
    selectionRegion.top = selectionRegion.bottom = location.y;

    isActive = true;
}

/**
 * Event handler, called by the mouse tracker while dragging.
 * @param location Current mouse location.
 */
Void SelectionTrackerSink::OnTrackingMove( MouseTracker& tracker, const POINT& location ) {
    ::InvalidateRect( hWnd, NULL, FALSE );
    if( abs( location.x - selectionAnchor.x ) > kFITSDragMarginX
        || abs( location.y - selectionAnchor.y) > kFITSDragMarginY ) {

        // Determine the new selection rectangle
        selectionRegion.left = min(selectionAnchor.x, location.x);
        selectionRegion.top  = min(selectionAnchor.y, location.y);
        selectionRegion.right = max(selectionAnchor.x, location.x);
        selectionRegion.bottom = max(selectionAnchor.y, location.y);
        adjustSelection();
    }
}

/**
 * Event handler, called by the mouse tracker when the dragging is over.
 * @param location Current mouse location.
 */
Void SelectionTrackerSink::OnTrackingEnd( MouseTracker& tracker, const POINT& location ) {
    // Clear the old selection
    ::InvalidateRect( hWnd, NULL, FALSE );
    isActive = false;
}

/**
 * Crops the selection to active region.
 */
Void SelectionTrackerSink::adjustSelection() {
    if( selectionRegion.left < activeRegion.left )     selectionRegion.left = activeRegion.left;
    if( selectionRegion.top < activeRegion.top )       selectionRegion.top = activeRegion.top;
    if( selectionRegion.right > activeRegion.right )   selectionRegion.right = activeRegion.right;
    if( selectionRegion.bottom > activeRegion.bottom ) selectionRegion.bottom = activeRegion.bottom;
}

/**
 * Returns the selected area.
 */
RECT SelectionTrackerSink::getSelection() const {
    return this->selectionRegion;
}

Bool SelectionTrackerSink::hasSelection() const  {
    return this->isActive;
}

/**
 * Updates the active region.
 * @param value The new active region.
 */
Void SelectionTrackerSink::setActiveRegion( const RECT& value ) {
    this->activeRegion = value;
}