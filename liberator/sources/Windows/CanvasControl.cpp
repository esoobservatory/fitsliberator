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

#include "CanvasControl.h"
#include "Resources.h"
#include "Environment.h"

using namespace FitsLiberator::Modelling;
using namespace FitsLiberator::Windows;

//---------------------------------------------------------------------------------------
// Constructors and destructors
//---------------------------------------------------------------------------------------

CanvasControl::CanvasControl() {
	cursor = NULL;
}

CanvasControl::~CanvasControl() {

}

//---------------------------------------------------------------------------------------
// Event handlers
//---------------------------------------------------------------------------------------

/**
 * @see Window::OnPaint
 */
Void CanvasControl::OnPaint( HWND, HDC hDC, RECT* paintRegion, PAINTSTRUCT* ) {
    Refresh( hDC );
}

/**
 * Repaints the control using double buffering.
 */
Void CanvasControl::Refresh() {
    HDC hDC = ::GetDC( hWnd );
    Refresh( hDC );
    ::ReleaseDC( hWnd, hDC );
}

/**
 * Repaints the control using double buffering.
 * @param hDC Canvas to paint on
 */
Void CanvasControl::Refresh( HDC hDC ) {
    // Get the size of the window
    RECT rc;
    ::GetClientRect( hWnd, &rc );

    // Create the offscreen buffer
    HDC memoryCanvas = ::CreateCompatibleDC( hDC );
    HBITMAP memoryBitmap = ::CreateCompatibleBitmap( hDC, rc.right, rc.bottom );
    HGDIOBJ oldBitmap = ::SelectObject( memoryCanvas, (HGDIOBJ)memoryBitmap );

    // Paint the control to the offscreen buffer
    OnRefresh( memoryCanvas );
    
    // Transfer from the offscreen buffer to the screen.
    ::BitBlt( hDC, 0, 0, rc.right, rc.bottom, memoryCanvas, 0, 0, SRCCOPY );

    // Clean up
    ::DeleteObject( ::SelectObject( memoryCanvas, oldBitmap ) );
    ::DeleteDC( memoryCanvas );
}

/**
 * Event handler, called when the control needs to be redrawn.
 * @param hDC Canvas to paint on.
 */
Void CanvasControl::OnRefresh( HDC hDC ) {
    // Default is to do nothing
}

/**
 * @see Window::OnSetCursor
 */
Bool CanvasControl::OnSetCursor( HWND hWnd, Int, Int ) {
    if( cursor != NULL && hWnd == this->hWnd ) {
        RECT rc; POINT pt;

        ::GetCursorPos( &pt );
        ::ScreenToClient( hWnd, &pt );
        ::GetClientRect( hWnd, &rc );

        if( ::PtInRect( &rc, pt ) ) {
			::SetCursor(cursor);
            return true;
        }
    }
    return false;
}

/**
 * Called when the tool has been changed, this effectively changes the cursor.
 * @param tool The new tool.
 */
Void CanvasControl::OnToolChanged( ToolTypeFunction tool ) {
    switch( tool ) {
        case kFITSToolFunctionPeakPicker:
        case kFITSToolFunctionBackgroundPicker:
        case kFITSToolFunctionBlackLevelPicker:
        case kFITSToolFunctionWhiteLevelPicker:
			Cursor(IDC_CROSS);
            break;
        case kFITSToolFunctionMove:
			Cursor(IDC_CURSOR_HAND);
            break;
        case kFITSToolFunctionZoomIn:
			Cursor(canZoomIn() ? IDC_CURSOR_ZOOMIN : IDC_CURSOR_ZOOMLIMIT);
            break;
        case kFITSToolFunctionZoomOut:
			Cursor(canZoomIn() ? IDC_CURSOR_ZOOMOUT : IDC_CURSOR_ZOOMLIMIT);
            break;
        default:
			Cursor(IDC_ARROW);
    }

    OnSetCursor( hWnd, 0, 0 );
}

Void CanvasControl::Cursor(LPCSTR name) {
	if(cursor != NULL) {
		::DestroyCursor(cursor);
	}

	cursor = LoadCursor(NULL, name);
}

Void CanvasControl::Cursor(unsigned int id) {
	if(cursor != NULL) {
		::DestroyCursor(cursor);
	}

	cursor = LoadCursor(g_hInstance, MAKEINTRESOURCE(id));
}
