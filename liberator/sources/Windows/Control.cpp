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
#include "Control.h"
#include "Environment.h"
#include "Resources.h"

using namespace FitsLiberator;
using namespace FitsLiberator::Windows;

const TCHAR Control::className[] = TEXT("FitsLiberator::Windows::Control");
ATOM Control::classAtom = NULL;

/**
 * Default constructor, registers the custom window class.
 */ 
Control::Control() {
    if( NULL == classAtom ) {
        WNDCLASSEX wcx; 
     
        // Fill in the window class structure with parameters 
        // that describe the control window. 
        wcx.cbSize = sizeof(wcx);                           // Size of structure 
        wcx.style = CS_HREDRAW | CS_VREDRAW;                // Redraw if size changes 
        wcx.lpfnWndProc = super::WindowProc;
        wcx.cbClsExtra = 0;                                 // No extra class memory 
        wcx.cbWndExtra = 0;                                 // No extra window memory 
        wcx.hInstance = g_hInstance;                        // Instance
        wcx.hIcon = NULL;                                   // No icon
        wcx.hCursor = ::LoadCursor(NULL, IDC_ARROW);        // Arrow cursor is the default
        wcx.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);    // Button face background color
        wcx.lpszMenuName =  NULL;                           // No menu
        wcx.lpszClassName = className;                      // Window class
        wcx.hIconSm = NULL;                                 // No small icon
     
        // Register the window class.
        classAtom = ::RegisterClassEx(&wcx);
    }
}

/**
 * Destructor, destroys the custom window.
 */
Control::~Control() {
    Destroy();
}

/**
 * Creates the custom control, can be used by superclasses to simplify creation.
 */
Bool Control::Create( HWND hWndParent, const LPRECT bounds, DWORD style, DWORD exStyle ) {
    if( NULL == classAtom )
        throw Exception( Environment::getString( IDS_FAILEDTOCREATEWINDOW ) );

    this->hWnd = ::CreateWindowEx( exStyle, className, "", style, bounds->left, bounds->top, bounds->right - bounds->left, bounds->bottom - bounds->top, hWndParent, NULL, g_hInstance, this );
    return ( this->hWnd != NULL);
}
