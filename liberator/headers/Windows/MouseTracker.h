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
#ifndef __MOUSETRACKER_H__
#define __MOUSETRACKER_H__

#include "Window.h"

namespace FitsLiberator {
    namespace Windows {
        class MouseTracker;

        enum MouseAction {
            MouseActionNone,
            MouseActionLeftClick,
            MouseActionRightClick,
            MouseActionTracking
        };

        /**
         * Interface used to report tracking status to 
         */
        class IMouseTrackerSink {
            public:
                virtual Void OnTrackingBegin( MouseTracker& tracker, const POINT& location ) = 0;
                virtual Void OnTrackingMove( MouseTracker& tracker, const POINT& location ) = 0;
                virtual Void OnTrackingEnd( MouseTracker& tracker, const POINT& location ) = 0;
        };

        class NullMouseTracker : public IMouseTrackerSink {
            public:
                virtual Void OnTrackingBegin( MouseTracker& tracker, const POINT& location ) {}
                virtual Void OnTrackingMove( MouseTracker& tracker, const POINT& location ) {}
                virtual Void OnTrackingEnd( MouseTracker& tracker, const POINT& location ) {}
        };

        /**
         * This class helps the PreviewControl and HistogramControl classes
         * with tracking the mouse when the user wants to select an area or
         * drag the window content.
         */
        class MouseTracker {
            public:
                MouseTracker( IMouseTrackerSink*, const RECT& );

                MouseAction OnMouseDown( HWND, Int, Int, Int );
                MouseAction OnMouseMove( HWND, Int, Int, Int );
                MouseAction OnMouseUp( HWND, Int, Int, Int );

                Void setActiveRegion( const RECT& );
                Void setTrackerSink( IMouseTrackerSink* );
            private:
                RECT               activeRegion;    ///< The active region
                Bool               tracking;        ///< Are we tracking?
                IMouseTrackerSink* trackerSink;     ///< Event sink
                POINT              anchor;          ///< Point where the user initiated a drag
                Int                button;          ///< The mouse button pressed
        };
    }
}

#endif //__MOUSETRACKER_H__