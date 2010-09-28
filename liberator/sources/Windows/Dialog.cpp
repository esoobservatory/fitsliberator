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
#include "Dialog.h"

using namespace FitsLiberator::Windows;

/**
 * Common constructor.
 * @param ID Dialog resource ID
 */
Dialog::Dialog(Int ID)
  : super(), ID(ID) {

}

/**
 * Destructor
 */
Dialog::~Dialog() {
    Close();
}

/**
 * Shows the dialog box modally
 * @param hWndParent Window handle of the parent window
 * @return Result depends on the dialog, but typical values are
 * IDOK and IDCANCEL.
 */
Int Dialog::Modal( HWND hWndParent ) {
    return (Int) ::DialogBoxParam(g_hInstance, MAKEINTRESOURCE(ID), hWndParent, &DialogProc, (LPARAM)this);
}

/**
 * Shows the dialog box in a modeless fashion
 * @param hWndParent Window handle of the parent window
 */
Void Dialog::Modeless( HWND hWndParent ) {
    this->hWnd = ::CreateDialogParam( g_hInstance, MAKEINTRESOURCE(ID), hWndParent, &DialogProc, (LPARAM)this );
}

/**
 * Returns the window handle of a child item. This signaure is by design,
 * allowing users of the method to wrap the window handle by a more 
 * sophisticated class than Window.
 * @param itemID Resource ID of the item.
 * @return Window handle of the item or NULL if the item doesn't exist.
 */
HWND Dialog::getItem(Int itemID) const {
    return ::GetDlgItem(hWnd, itemID);
}

/**
 * Event handler, called after all the controls on the dialog have been created. 
 * Corresponds to WM_INITDIALOG.
 * @param hWnd Window handle of the dialog box.
 * @return True to continue creation, false otherwise.
 * @remark After this method is called, the hWnd member is initialized.
 */
Bool Dialog::OnInit(HWND hWnd) {
    this->hWnd = hWnd;
    Center();
    return true;
}

/**
 * Takes over the job for Window::OnMessage.
 */
Bool Dialog::OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, Int& returnValue) {
    returnValue = TRUE; // We handled the message
    switch(uMsg) {
        case WM_INITDIALOG:
            if( !OnInit(hWnd) ) {
                Close( IDCANCEL );
                returnValue = FALSE;
            }
            break;
        default:
            return super::OnMessage(hWnd, uMsg, wParam, lParam, returnValue);
    }
    return true;
}

/**
 * @see Window::DefaultHandler
 */
Int Dialog::DefaultHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    // Default action for dialog procedures is to return FALSE, when the message
    // is not handled.
    return FALSE;
}

/**
 * Closes the dialog, usually called by the dialog itself.
 * @param result The result to report to the creator.
 */
Void Dialog::Close(Int result) {
    ::EndDialog(hWnd, result);
}


/**
 * @see Dialog::Close(Int)
 */
Void Dialog::Close() {
    Close(IDOK);
}


/**
 * Main dialog procedure - delegates all work to a Dialog instance.
 * @param hDlg Window handle of the dialog
 * @param uMsg Message
 * @param wParam Message parameter 1
 * @param lParam Message parameter 2, this is a Dialog* when uMsg = WM_INITDIALOG
 * @return Depends on uMsg, see Platform SDK for more information.
 */
BOOL CALLBACK Dialog::DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) { 
    // Because the FitsLiberator::Windows namespace implements a UI framework
    // many dialog instances may use this function simultaneously. To ensure
    // we notify the correct Dialog instance, we use the Windows property system
    // to associate a hDlg to a Dialog*. For this purpose WM_INITDIALOG is very useful
    // because Windows lets us pass a Dialog* as the lParam for WM_INITDIALOG.
    if( WM_INITDIALOG == uMsg ) {
        ::SetWindowLong( hDlg, GWL_USERDATA, (LONG)lParam );
    }

    // Now we can just retrieve the Dialog* and pass the message along.
    Dialog* owner = (Dialog*) ::GetWindowLong( hDlg, GWL_USERDATA );
    if( NULL != owner )
        return owner->ProcessMessage(hDlg, uMsg, wParam, lParam);

    // We shouldn't get here, but to play nice we return FALSE => we didn't
    // handle the message.
    return FALSE;
}
