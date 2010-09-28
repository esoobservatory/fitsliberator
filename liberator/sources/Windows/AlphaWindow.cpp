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
#include "AlphaWindow.h"
#include "ResourceManager.h"
#include "Environment.h"

using namespace Gdiplus;
using namespace FitsLiberator;
using namespace FitsLiberator::Windows;

ATOM AlphaWindow::classAtom = 0;
const Char* AlphaWindow::windowClass = "FitsLiberator::Windows::AlphaWindow";

/**
 * Default constructor.
 * @param b Bitmap to display. The bitmap is expected be a 32argb image.
 */
AlphaWindow::AlphaWindow( Bitmap& b ) : bitmap( b ) {

}

/**
 * Show the bitmap as a dialog.
 * @param parent Parent window.
 * @return True on success, false on failure.
 */
Bool AlphaWindow::Show( HWND parent ) {
    if( !Create( parent ) )
        return false;

    setVisible( true );
    if( !ApplyBitmap( this->bitmap ) )
        return false;
    
    ModalLoop( parent );

    return true;
}

/**
 * Implements a modal message pump.
 * @param parent Parent window.
 */
Void AlphaWindow::ModalLoop( HWND parent ) {
    Window host( parent );
    host.setEnabled( false );
    
    keepAlive = true;
    
    MSG m;
    while( keepAlive && ::GetMessage( &m, NULL, 0, 0 ) ) {
        ::TranslateMessage( &m );
        ::DispatchMessage( &m );
    }

    host.setEnabled( true );
    ::SetActiveWindow( parent );
}

/**
 * Creates the window, the window is initially hidden.
 * @param parent Parent window.
 * @return True on success, false on failure.
 */
Bool AlphaWindow::Create( HWND parent ) {
    if( Register() ) {
        this->hWnd = ::CreateWindowEx( WS_EX_LAYERED, windowClass, "", WS_POPUP, 0, 0, 0, 0, parent, NULL, g_hInstance, this );
        if( NULL != this->hWnd ) {
            return true;
        }
    }
    return false;
}

/**
 * Registers the window class.
 * @return True on success, false on failure.
 */
Bool AlphaWindow::Register() {
    if( classAtom == 0 ) {
        WNDCLASSEX wcx; 
        
        // Fill in the window class structure with parameters 
        // that describe the control window. 
        wcx.cbSize = sizeof(wcx);               // Size of structure 
        wcx.style = CS_HREDRAW | CS_VREDRAW;    // Redraw if size changes 
        wcx.lpfnWndProc = super::WindowProc;
        wcx.cbClsExtra = 0;                     // No extra class memory 
        wcx.cbWndExtra = 0;                     // No extra window memory 
        wcx.hInstance = g_hInstance;            // Instance
        wcx.hIcon = NULL;                       // No icon
        wcx.hCursor = NULL;                     // No cursor
        wcx.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);  // Button face background color
        wcx.lpszMenuName =  NULL;               // No menu
        wcx.lpszClassName = windowClass;        // Window class
        wcx.hIconSm = NULL;                     // No small icon
        
        // Register the window class.
        classAtom = ::RegisterClassEx(&wcx);
    }

    return (classAtom != NULL);
}

Bool AlphaWindow::ApplyBitmap( Bitmap& bitmap ) {
    Bool success = false;

    SizeF bitmapSize;
    bitmap.GetPhysicalDimension( &bitmapSize );

    // Size and move our window
    FitsLiberator::Rectangle windowBounds( 0, 0, (Int)bitmapSize.Width, (Int)bitmapSize.Height );
    setBounds( windowBounds );
    Center();
    windowBounds = getBounds();

    HDC screenDC = ::GetDC( NULL );
    HDC memDC    = ::CreateCompatibleDC( screenDC );
    HBITMAP hBitmap = NULL;
    HBITMAP hOldBitmap = NULL;

    if( Ok == bitmap.GetHBITMAP( Gdiplus::Color(0), &hBitmap ) ) {
        hOldBitmap = (HBITMAP)::SelectObject( memDC, hBitmap );
        
        SIZE newSize = { (Int)bitmapSize.Width, (Int)bitmapSize.Height };
        POINT sourceLocation = { 0, 0 };
        POINT newLocation = { windowBounds.left, windowBounds.top };
        BLENDFUNCTION blend;
        blend.BlendOp = AC_SRC_OVER;
        blend.BlendFlags = 0;
        blend.SourceConstantAlpha = 255;
        blend.AlphaFormat = AC_SRC_ALPHA;

        ::UpdateLayeredWindow( hWnd, screenDC, &newLocation, &newSize, memDC, &sourceLocation, 0, &blend, ULW_ALPHA );
        success = true;
    }

    ::ReleaseDC( NULL, screenDC );
    if( hBitmap != NULL ) {
        ::SelectObject( memDC, hOldBitmap );
        ::DeleteObject( hBitmap );
    }
    ::DeleteDC( memDC );

    return success;
}

Void AlphaWindow::OnMouseDown( Int button, Int x, Int y ) {
    super::OnMouseDown( button, x, y );

    Destroy();
}

Void AlphaWindow::OnKeyPress( Int character, Int flags ) {
    super::OnKeyPress( character, flags );

    Destroy();
}

Void AlphaWindow::OnNcDestroy() {
    keepAlive = false;  // Instruct the ModalLoop to exit
}
