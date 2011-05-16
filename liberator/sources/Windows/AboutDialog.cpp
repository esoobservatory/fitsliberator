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

#include "AboutDialog.h"
#include "Version.h"
#include "Resources.h"

using namespace FitsLiberator::Windows;

/**
 * Default constructor
 */
AboutDialog::AboutDialog() 
  : Dialog(IDD_ABOUT) {
    //
    // Create a hollow (transparent) brush to use for filling the version text with
    hollowBrush = (HBRUSH)::GetStockObject(HOLLOW_BRUSH);
}

/**
 * Destructor
 */
AboutDialog::~AboutDialog() {
    ::DeleteObject(hollowBrush);
}

/**
 * Event handler, called upon creation.
 * @see Dialog::OnInit
 */
Void AboutDialog::OnInit(HWND hWnd) {
    Dialog::OnInit(hWnd);

    //
    // Make sure the dialog has the correct size,
    // remember that Windows uses dialog units, not pixels
    // when specifying the size of dialogs in resource files
    FitsLiberator::Rectangle rect = getBounds();
    Int dx = 0, dy = 0;
    
    // First move and resize the dialog itself
	dx = (600 - (rect.right - rect.left)) / 2;
	dy = (400 - (rect.bottom - rect.top)) / 2;
	rect.left -= dx;
	rect.top -= dy;
	rect.right += dx;
	rect.bottom += dy;
    setBounds(rect);

    // Next modify the size of the image
	rect.left = 0;
	rect.top = 0;
	rect.right = 600;
	rect.bottom = 400;
    Window imageControl(getItem(IDC_ABOUT));
    imageControl.setBounds(rect);

    //
    // Modify the version text to actually include the version
    // number
    Window versionControl(getItem(IDC_VERSION));
    versionControl.appendText(String(FITSVERSION));
}

/**
 * @see Dialog::OnCommand
 */
Void AboutDialog::OnCommand(WPARAM wParam, LPARAM lParam) {
    Close();
}

/**
 * @see Dialog::OnParentNotify
 */
Void AboutDialog::OnParentNotify(UINT uMsg, LPARAM lParam) {
    Close();
}

/**
 * @see Window::OnCtlColor
 */
HBRUSH AboutDialog::OnCtlColor(HWND hWnd, HDC hDC, UINT ctlType) {
    if( hWnd == getItem( IDC_VERSION ) ) {
        ::SetBkMode(hDC, TRANSPARENT);
        return hollowBrush;
    }
    return Dialog::OnCtlColor( hWnd, hDC, ctlType );
}