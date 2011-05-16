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
#include "MoveTrackerSink.h"

using namespace FitsLiberator::Windows;

/**
 * Default constructor.
 */
MoveTrackerSink::MoveTrackerSink() {
    anchor.x = 0;
    anchor.y = 0;
    location.x = 0;
    location.y = 0;
}

/**
 * Event handler, called by the mouse tracker when dragging begins.
 * param location The poiont where the user started the dragging operation.
 */
Void MoveTrackerSink::OnTrackingBegin( MouseTracker&, const POINT& location ) {
    this->anchor = location;
    this->location = location;
}

/**
 * Event handler, called by the mouse tracker while dragging.
 * @param location Current mouse location.
 */
Void MoveTrackerSink::OnTrackingMove( MouseTracker&, const POINT& location ) {
    this->location = location;
}

/**
 * Event handler, called by the mouse tracker when the dragging is over.
 * @param location Current mouse location.
 */
Void MoveTrackerSink::OnTrackingEnd( MouseTracker&, const POINT& location ) {
    this->location = location;
}

/**
 * Returns the signed distance betweeen start and end location.
 */
SIZE MoveTrackerSink::getDistance() const {
    SIZE distance;
    distance.cx = location.x - anchor.x;
    distance.cy = location.y - anchor.y;
    return distance;
}

/**
 * Returns the current location.
 */
POINT MoveTrackerSink::getLocation() const {
    return this->location;
}

/**
 * Resets the sink so that the start location = current location.
 */
Void MoveTrackerSink::Reset() {
    anchor = location;
}