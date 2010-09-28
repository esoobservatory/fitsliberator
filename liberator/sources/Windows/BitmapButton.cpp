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
#include "BitmapButton.h"
#include <math.h>

using namespace Gdiplus;
using namespace FitsLiberator;
using namespace FitsLiberator::Windows;

/**
 * Creates a bitmap button from an existing button. Note that the life-time of this object is
 * tied to the button window. The object destroys itself when the window is destroyed.
 * @param bitmap Bitmap to paint on the button.
 * @param buttonWindow Existing button to modify.
 */
BitmapButton::BitmapButton( Bitmap* bitmap, HWND buttonWindow ) : super( buttonWindow ) {
    this->bitmap = bitmap;
    this->oldWindowProc = NULL;
    if( NULL != bitmap && ::IsWindow( buttonWindow ) ) {
        // Subclass the button to display a bitmap
        ::SetWindowLong( buttonWindow, GWL_USERDATA, (LONG)this );
        oldWindowProc = (WNDPROC)::SetWindowLong( buttonWindow, GWL_WNDPROC, (LONG)&BitmapButton::WindowProc );
    }
}

/**
 * Paints the button content. This paints the bitmap centered on the button.
 * @param hDC Device context to paint on.
 */
Void BitmapButton::PaintBitmap( HDC hDC ) {
    if( NULL != bitmap ) {
        Graphics g( hDC );
        FitsLiberator::Rectangle bounds = getClient();

        Gdiplus::SizeF size;
        bitmap->GetPhysicalDimension( &size );
        
        Gdiplus::PointF location(
            (bounds.getWidth() - size.Width) / 2,
            (bounds.getHeight() - size.Height) / 2
        );

        g.DrawImage( bitmap, (int)floor(location.X), (int)floor(location.Y), (int)size.Width, (int)size.Height );
    }
}

/**
 * Custom window procedure, used to override painting.
 * @see Window::WindowProc.
 */
LRESULT CALLBACK BitmapButton::WindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) {
    BitmapButton* owner = (BitmapButton*)::GetWindowLong( hWnd, GWL_USERDATA );

    LRESULT result = 0;
    HDC hDC = NULL;
    switch( uMsg ) {
        case WM_PAINT:
            result = ::CallWindowProc( owner->oldWindowProc, hWnd, uMsg, wParam, lParam );            
            
            hDC = ::GetDC( hWnd );
            owner->PaintBitmap( hDC );
            ::ReleaseDC( hWnd, hDC );

            break;
        case WM_NCDESTROY:
            ::SetWindowLong( hWnd, GWL_WNDPROC, (LONG)owner->oldWindowProc );
            result = ::CallWindowProc( owner->oldWindowProc, hWnd, uMsg, wParam, lParam );
            delete owner;
            break;
        default:
            result = ::CallWindowProc( owner->oldWindowProc, hWnd, uMsg, wParam, lParam );
    }

    return result;
}