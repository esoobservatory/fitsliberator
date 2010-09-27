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
#include "TooltipManager.h"

using namespace FitsLiberator::Windows;

/**
 * Event handler, should be called by the owner dialogs OnInit method.
 * @param dialog Owner dialog
 */
Void TooltipManager::OnInit( Dialog* dialog ) {
    HWND hWnd = dialog->getHandle();

    // Create the Tooltip Window
    HWND tooltipWindow = ::CreateWindowEx( NULL, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hWnd, NULL, g_hInstance, NULL );
    ::SetWindowPos( tooltipWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );    

    OnCreate( tooltipWindow, hWnd );

    ::SendMessage( tooltipWindow, TTM_ACTIVATE, TRUE, 0 );
}

/**
 * Event handler, should be called by the owner dialogs OnNotify method.
 * @see Window::OnNotify
 */
Void TooltipManager::OnNotify( NMHDR* e ) {
    if( e->code == TTN_GETDISPINFO )
        OnToolGetDispInfo( (NMTTDISPINFO*)e );
}

/**
 * Event handler, called when the tooltip control needs text to display in a tooltip
 * param e Event parameters, see the TTN_GETDISPINFO notification in the Platform SDK
 */
Void TooltipManager::OnToolGetDispInfo( NMTTDISPINFO* e ) {
    Int control = (e->uFlags & TTF_IDISHWND)
        ? ::GetDlgCtrlID( reinterpret_cast<HWND>(e->hdr.idFrom) )
        : e->hdr.idFrom;

    e->hinst = g_hInstance;
    e->lpszText = MAKEINTRESOURCE( control );
}

/**
 * Registers all child windows in the owner dialog with the tooltip control, unless they
 * have an invalid ID.
 * @param tooltipWindow Window handle of the tooltip control
 * @param hWnd Window handle of the dialog.
 */
Void TooltipManager::OnCreate( HWND tooltipWindow,  HWND hWnd ) {
    TOOLINFO toolInfo; ::ZeroMemory( &toolInfo, sizeof( toolInfo ) );
    toolInfo.cbSize   = sizeof( toolInfo );
    toolInfo.uFlags   = TTF_SUBCLASS | TTF_IDISHWND;
    toolInfo.hwnd     = hWnd;
    toolInfo.lpszText = LPSTR_TEXTCALLBACK;


    HWND childWindow = ::GetWindow( hWnd, GW_CHILD );
    while( childWindow ) {
        toolInfo.uId = (UINT_PTR)childWindow;
        ::SendMessage( tooltipWindow, TTM_ADDTOOL, 0, (LPARAM)&toolInfo );
        childWindow = ::GetWindow( childWindow, GW_HWNDNEXT );
    }
}