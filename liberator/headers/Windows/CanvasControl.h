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

#ifndef __CANVASCONTROL_H__
#define __CANVASCONTROL_H__

#include "Control.h"
#include "ToolModel.h"

namespace FitsLiberator {
    namespace Windows {
        /**
         * Implements double buffering on the entire control, and handles cursors
         */
        class CanvasControl : public Control {
            public:
                CanvasControl();
                virtual ~CanvasControl();

                Void Refresh();
                virtual Void OnToolChanged( FitsLiberator::Modelling::ToolTypeFunction );
            protected:
                virtual Void OnPaint( HWND, HDC, RECT*, PAINTSTRUCT* );
                virtual Void OnRefresh( HDC ) = 0;
                virtual Bool OnSetCursor( HWND, Int, Int );
                virtual Bool canZoomIn() = 0;
                virtual Bool canZoomOut() = 0;
            private:
				HCURSOR cursor;               ///< Which cursor to display

				/**	@brief	Use a new system cursor for this control.
					@param	name			Name or resource ID of the new cursor. */
				Void Cursor(LPCSTR name);
				/**	@brief	Use a new cursor for this control.
					@param	id	Resource ID of the new cursor. */
				Void Cursor(unsigned int id);

				Void Refresh( HDC );

        };
    }
}

#endif //__CANVASCONTROL_H__