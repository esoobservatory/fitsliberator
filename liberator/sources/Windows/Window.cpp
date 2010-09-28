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
//---------------------------------------------------------------------------------------
// Includes and namespace imports
//---------------------------------------------------------------------------------------
#include "Window.h"
#include "TextUtils.h"

using namespace FitsLiberator::Windows;

//---------------------------------------------------------------------------------------
// Constructors and destructors
//---------------------------------------------------------------------------------------

/**
 * Default constructor, builds a uninialized window. Do not
 * use the methods until the hWnd field is set.
 */
Window::Window() 
  : hWnd(NULL) {
}

/**
 * Common constructor, initializes the Window object to use a
 * specific window handle. Typical usage is something like:
 * @code 
 * Window control(hWndControl);
 * control.setText(String("Hello World"));
 * @endcode
 *
 * @param hWnd Window handle of the window to represent.
 */
Window::Window(HWND hWnd) {
    Window();
    this->hWnd = hWnd;
}

/**
 * Destructor, currently does nothing
 */
Window::~Window() {
}

//---------------------------------------------------------------------------------------
// Public methods
//---------------------------------------------------------------------------------------

/**
 * This is where all messages pass through. It is used to ensure that windows get unsubclassed. 
 * For parameter information see the Windows Platform SDK.
 */
Int Window::ProcessMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    Int returnValue;

	if( OnMessage( hWnd, uMsg, wParam, lParam, returnValue ) )
        return returnValue;
    else
        return DefaultHandler( hWnd, uMsg, wParam, lParam );
}

/**
 * Destroys the window, the window is no longer valid after this
 */
Void Window::Destroy()  {
    ::DestroyWindow( hWnd );
    hWnd = NULL;
}

/**
 * Sends a message to the window.
 */
Int Window::SendMessage( UINT uMsg, WPARAM wParam, LPARAM lParam ) {
	if( hWnd == NULL )
		return NULL;
    return ::SendMessage( hWnd, uMsg, wParam, lParam );
}

/**
 * Prevents the window from repainting
 */
Void Window::LockUpdate() {
	SendMessage( WM_SETREDRAW, TRUE, NULL );
}

/**
 * Re-enables repainting.
 */
Void Window::UnlockUpdate() {
	SendMessage( WM_SETREDRAW, FALSE, NULL );
}

/**
 * Centers the dialog 1/3 of the way down on the main screen.
 * @remark This code is adapted from a Microsoft Platform SDK sample.
 */
Void Window::Center( HWND hWndOwner ) {
    if( !::IsWindow( hWndOwner ) )
        return;

    RECT rc, rcDlg, rcOwner;

    ::GetWindowRect(hWndOwner, &rcOwner); 
    ::GetWindowRect(hWnd, &rcDlg); 
    ::CopyRect(&rc, &rcOwner); 

    // Offset the owner and dialog box rectangles so that 
    // right and bottom values represent the width and 
    // height, and then offset the owner again to discard 
    // space taken up by the dialog box. 

    ::OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top); 
    ::OffsetRect(&rc, -rc.left, -rc.top); 
    ::OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom); 

    // The new position is the sum of half the remaining 
    // space and the owner's original position. 

    RECT bounds, desktop;    // New rectangle of the window
    bounds.left   = rcOwner.left + (rc.right / 2);
    bounds.top    = rcOwner.top + (rc.bottom / 3);
    bounds.right  = bounds.left + rcDlg.right;
    bounds.bottom = bounds.top + rcDlg.bottom;
    
    HWND hWndDesktop = ::GetDesktopWindow();
    ::GetClientRect( hWndDesktop, &desktop );

    if( bounds.left < 0 || bounds.top < 0 || bounds.right > desktop.right || bounds.bottom > desktop.bottom ) {
        if( hWndOwner != hWndDesktop ) {
            // Make sure we don't recurse if the user has a tiny resolution
            Center( hWndDesktop );
        }
        else {
            ::SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE); 
        }
    }
    else {
        ::SetWindowPos(hWnd, HWND_TOP, bounds.left, bounds.top, 0, 0, SWP_NOSIZE); 
    }
}

Void Window::Center() {
    HWND hWndOwner;
    if ((hWndOwner = ::GetParent(hWnd)) == NULL) {
        hWndOwner = ::GetDesktopWindow(); 
    }
    Center( hWndOwner );
}


//---------------------------------------------------------------------------------------
// Property accessors
//---------------------------------------------------------------------------------------

/**
 * Returns the text of a window, for top level windows this is the caption, for
 * child windows it is the content.
 * @return The window text.
 */
String Window::getText() const {
    String text;
    Int length = ::GetWindowTextLength(hWnd) + 1;

    if( length > 0 ) {
        Char* buffer = new Char[length];
        ::GetWindowText(hWnd, buffer, length);
        text = buffer;
        delete[] buffer;
    }
    return text;
}

/** 
 * Sets the window text.
 * @param text New text to use.
 */
Void Window::setText(const String& text) {
    Window::setText( this->hWnd, text );
}

/**
 * Sets the window text.
 * @param hWnd Window handle of the window whose text is to be changed.
 * @param text New text.
 */
Void Window::setText( HWND hWnd, const String& text ) {
    ::SetWindowText(hWnd, text.c_str());
}

Void Window::setValue( HWND hWnd, Double value ) {
	setText( hWnd, TextUtils::doubleToString( value ) );
}

Void Window::setValue( Double value ) {
    Window::setValue( this->hWnd, value );
}

/**
 * Utility function to append som text to a window.
 * @param text The text to append.
 */
Void Window::appendText(const String& text) {
    String newText = getText();
    newText.append(text);
    setText(newText);
}

/**
 * Returns the boundaries of the window including 
 * title bars and borders if present.
 * @return The window boundaries.
 */
FitsLiberator::Rectangle Window::getBounds() const {
    FitsLiberator::Rectangle rect;
    ::GetWindowRect(hWnd, (LPRECT)&rect);
    return rect;
}

/**
 * Returns the boundaries of the window client area 
 * @return The window boundaries.
 */
FitsLiberator::Rectangle Window::getClient() const {
    FitsLiberator::Rectangle rect;
    ::GetClientRect(hWnd, (LPRECT)&rect);
    return rect;
}

/**
 * Returns whether the window is visible
 * @return True if the window is visible, false otherwise.
 */
Bool Window::getVisible() const {
    return ((::GetWindowLong( hWnd, GWL_STYLE ) & WS_VISIBLE) != 0);
}

/**
 * Set the window visibility.
 * @param visible True if the window should be visible, false otherwise.
 */
Void Window::setVisible( Bool visible ) {
    if( visible )
        ShowWindow( hWnd, SW_SHOW );
    else
        ShowWindow( hWnd, SW_HIDE );
}

Void Window::setEnabled( Bool enabled ) {
    ::EnableWindow( hWnd, enabled ? TRUE : FALSE );
}

/**
 * Makes sure the window has input focus
 */
Void Window::SetFocus() {
    ::SetFocus( hWnd );
}

/**
 * Sets the boundaries of the window.
 * @param bounds New boundaries of the window.
 */
Void Window::setBounds(const FitsLiberator::Rectangle& bounds) {
    ::SetWindowPos(hWnd, NULL, bounds.left, bounds.top, bounds.right - bounds.left, bounds.bottom - bounds.top, SWP_NOZORDER);
}

Void Window::setBounds(const RECT* bounds) {
    ::SetWindowPos(hWnd, NULL, bounds->left, bounds->top, bounds->right - bounds->left, bounds->bottom - bounds->top, SWP_NOZORDER);
}

/**
 * Modifies the style bits for the window.
 * @param type Type of style, this can be either GWL_STYLE or GWL_EXSTYLE.
 * @param removeStyle Style bits to remove.
 * @param addStyle Style bits to add.
 */
Void Window::modifyStyle( Int type, DWORD removeStyle, DWORD addStyle ) {
    DWORD style = ::GetWindowLong( hWnd, type );
    DWORD newStyle = (style & ~removeStyle) | addStyle;

    if( style != newStyle ) {
        ::SetWindowLong( hWnd, type, newStyle );
		::SetWindowPos( hWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
    }
}

HWND Window::getHandle() {
    return this->hWnd;
}

Void Window::setHandle( HWND hWnd ) {
    this->hWnd = hWnd;
}

//---------------------------------------------------------------------------------------
// Protected methods
//---------------------------------------------------------------------------------------

/**
 * Event handler, called when the window is first created.
 * @param hWnd Window handle of the window.
 */
Void Window::OnCreate(HWND hWnd) {

}

/**
 * Event handler, called when the window is destroyed.
* @param hWnd Window handle of the window.
 */
Void Window::OnDestroy(HWND hWnd) {
//    this->hWnd = NULL;
}

Void Window::OnNcDestroy() {

}

/**
 * Event handler, called when the window needs to be painted.
 * @param hWnd Window handle of the window.
 * @param hDC Device context.
 * @param rc Boundaries of the area that needs painting.
 * @param ps Painting information.
 * @remark For more information see the Windows Platform SDK, WM_PAINT.
 */
Void Window::OnPaint(HWND hWnd, HDC hDC, RECT* rc, PAINTSTRUCT* ps) {
}

/**
 * Event handler, called when a child window is about to be painted.
 * @param hWnd Window handle of the child window.
 * @param hDC Device context of the child window.
 * @param ctlType Type of control who raised this event.
 * @return Return a valid brush handle to handle this event.
 */
HBRUSH Window::OnCtlColor(HWND hWnd, HDC hDC, UINT ctlType) {
    return NULL;
}

/**
 * Event handler, called when something happens to a child window.
 * @param uMsg Event that has occured.
 * @param lParam Event specific parameters.
 * @remark For more information see the Windows Platform SDK, WM_PARENTNOTIFY.
 */
Void Window::OnParentNotify(UINT uMsg, LPARAM lParam) {
}

/**
 * Event handler, called by Windows Common Controls when
 * things happen in a child window.
 * @param idCtrl Id of the control which sent this notification.
 * @param pnmh Notification information.
 * @remark For more information see the Windows Platform SDK, WM_NOTIFY.
 */
LRESULT Window::OnNotify(Int idCtrl, LPNMHDR pnmh) {
    return false;
}


/**
 * Event handler, called when a key has been depressed.
 * @param virtualKey Virtual keycode
 * @param flags Event flags.
 * @remark For more information see the Windows Platform SDK, WM_KEYDOWN and WM_SYSKEYDOWN
 */
Void Window::OnKeyDown( Int virtualKey, Int flags ) {
    DefaultHandler( hWnd, WM_KEYDOWN, (WPARAM)virtualKey, (LPARAM)flags );
}

/**
 * Event handler, called when a key has been pressed and released.
 * @param character Character code
 * @param flags Event flags.
 * @remark For more information see the Windows Platform SDK, WM_CHAR
 */
Void Window::OnKeyPress( Int character, Int flags ) {
    DefaultHandler( hWnd, WM_CHAR, (WPARAM)character, (LPARAM)flags );
}

/**
 * Event handler, called when a key has been released.
 * @param virtualKey Virtual keycode
 * @param flags Event flags.
 * @remark For more information see the Windows Platform SDK, WM_KEYUP and WM_SYSKEYUP
 */
Void Window::OnKeyUp( Int virtualKey, Int flags ) {
    DefaultHandler( hWnd, WM_KEYUP, (WPARAM)virtualKey, (LPARAM)flags );
}

/**
 * Event handler, called when a window is requested to scroll. Up/Down controls
 * send this message when the user clicks on the arrows.
 * @remark For more information see the Windows Platform SDK, WM_VSCROLL
 */
Void Window::OnVerticalScroll( WPARAM wParam, LPARAM lParam ) {
    DefaultHandler( hWnd, WM_VSCROLL, wParam, lParam );
}

/**
 * Event handler, called when a window is requested to scroll. Up/Down controls
 * send this message when the user clicks on the arrows.
 * @remark For more information see the Windows Platform SDK, WM_HSCROLL
 */
Void Window::OnHorizontalScroll( WPARAM wParam, LPARAM lParam ) {
    DefaultHandler( hWnd, WM_VSCROLL, wParam, lParam );
}

/**
 * Event handler, called when the mouse is pressed inside a window.
 * @remark For more information see the Windows Platform SDK, WM_LBUTTONDOWN, WM_MBUTTONDOWN and WM_RBUTTONDOWN 
 */
Void Window::OnMouseDown( Int button, Int x, Int y ) {
    SetFocus();
}

/**
 * Event handler, called when the mouse is released inside a window.
 * @remark For more information see the Windows Platform SDK, WM_LBUTTONUP, WM_MBUTTONUP and WM_RBUTTONUP
 */
Void Window::OnMouseUp( Int button, Int x, Int y ) {

}

/**
 * Event handler, called when the mouse is moved inside a window.
 * @remark For more information see the Windows Platform SDK, WM_MOUSEMOVE
 */
Void Window::OnMouseMove( Int button, Int x, Int y ) {

}

/**
 * Event handler, called when the mouse wheel is scrolled inside a window.
 * @remark For more information see the Windows Platform SDK, WM_MOUSEWHEEL.
 */
bool Window::OnMouseWheel(SHORT delta, USHORT flags, USHORT screenX, USHORT screenY) {
    return false;
}

/**
 * Event handler, called when something happens to a control, corresponds to WM_COMMAND.
 * @param wParam Event parameter.
 * @param lParam Event parameter.
 * @remark See the Windows Platform SDK for more information.
 */
Void Window::OnCommand(WPARAM wParam, LPARAM lParam) {

}

/**
 * Event handler, called when Windows needs a cursor.
 * @param hWnd Window handle of the window that needs a cursor.
 * @param hitTest Hit test code (what is the cursor over).
 * @param message Mouse message that triggered this.
 * @return true if the message was handled.
 */
Bool Window::OnSetCursor( HWND hWnd, Int hitTest, Int message) {
    return false;
}

/**
 * Event dispatcher, called by ProcessMessage().
 * @param hWnd Window handle.
 * @param uMsg Message ID.
 * @param wParam Message parameter 1.
 * @param lParam Message parameter 2.
 * @param returnValue Return value of the message processing.
 * @return Returns true to indicate that the message was processed, false to perform
 * default processing.
 */
Bool Window::OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, Int& returnValue) {
    RECT rc;
    PAINTSTRUCT ps;
    HDC hDC;

    returnValue = 0;    // We handled the message
    switch(uMsg) {
        case WM_CREATE:
            OnCreate(hWnd);
            break;
        case WM_DESTROY:
            OnDestroy(hWnd);
            break;
        case WM_NCDESTROY:
            OnNcDestroy();
            break;
        case WM_COMMAND:
            OnCommand( wParam, lParam );
            break;
        case WM_PAINT:
            if( ::GetUpdateRect(hWnd, &rc, FALSE) ) {
                hDC = ::BeginPaint(hWnd, &ps);
                OnPaint(hWnd, hDC, &rc, &ps);
                ::EndPaint(hWnd, &ps);
            }
            break;
        case WM_CTLCOLORBTN:
            returnValue = (Int)OnCtlColor((HWND)lParam, (HDC)wParam, CTLCOLOR_BTN);
            break;
        case WM_CTLCOLORDLG:
            returnValue = (Int)OnCtlColor((HWND)lParam, (HDC)wParam, CTLCOLOR_DLG);
            break;
        case WM_CTLCOLOREDIT:
            returnValue = (Int)OnCtlColor((HWND)lParam, (HDC)wParam, CTLCOLOR_EDIT);
            break;
        case WM_CTLCOLORLISTBOX:
            returnValue = (Int)OnCtlColor((HWND)lParam, (HDC)wParam, CTLCOLOR_LISTBOX);
            break;
        case WM_CTLCOLORMSGBOX:
            returnValue = (Int)OnCtlColor((HWND)lParam, (HDC)wParam, CTLCOLOR_MSGBOX);
            break;
        case WM_CTLCOLORSCROLLBAR:
            returnValue = (Int)OnCtlColor((HWND)lParam, (HDC)wParam, CTLCOLOR_SCROLLBAR);
            break;
        case WM_CTLCOLORSTATIC:
            returnValue = (Int)OnCtlColor((HWND)lParam, (HDC)wParam, CTLCOLOR_STATIC);
            break;
        case WM_PARENTNOTIFY:
            OnParentNotify((UINT)wParam, lParam);
            break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            OnKeyDown( (Int)wParam, (Int)lParam );
            break;
        case WM_CHAR:
            OnKeyPress( (Int)wParam, (Int)lParam );
            break;
        case WM_KEYUP:
        case WM_SYSKEYUP:
            OnKeyUp( (Int)wParam, (Int)lParam );
            break;
        case WM_VSCROLL:
            OnVerticalScroll( wParam, lParam );
            break;
        case WM_HSCROLL:
            OnHorizontalScroll( wParam, lParam );
            break;
        case WM_NOTIFY:
            returnValue = (Int)OnNotify( (Int)wParam, (LPNMHDR)lParam );
            break;
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
            OnMouseDown( (Int)wParam, LOWORD(lParam), HIWORD(lParam));
            break;
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
            OnMouseUp( (Int)wParam, LOWORD(lParam), HIWORD(lParam) );
            break;
        case WM_MOUSEMOVE:
            OnMouseMove( (Int)wParam, LOWORD(lParam), HIWORD(lParam) );
            break;
        case WM_MOUSEWHEEL:
            if(!OnMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam), GET_KEYSTATE_WPARAM(wParam), LOWORD(lParam), HIWORD(lParam)))
                returnValue = -1;
            break;
        case WM_SETCURSOR:
            return OnSetCursor( (HWND)wParam, LOWORD(lParam), HIWORD(lParam) );
        default:
            return false;
    }
    return true;
}

/**
 * Default message handler.
 */
Int Window::DefaultHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    return ::DefWindowProc( hWnd, uMsg, wParam, lParam );
}

//---------------------------------------------------------------------------------------
// Static functions
//---------------------------------------------------------------------------------------

/** 
 * Shared WndProc, redirects all messages to a Window instance. Expects
 * the ::GetWindowLong( hWnd, GWL_USERDATA ) to return a valid.
 * Window instance pointer.
 * For parameter information see the Windows Platform SDK.
 */
LRESULT CALLBACK Window::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    // Custom windows pass a this pointer in the call to CreateWindowEx, which
    // calls this function with uMsg = WM_CREATE. So now we can hook up
    // this function to a Window instance.
    if( WM_CREATE == uMsg ) {
        LPCREATESTRUCT cs = (LPCREATESTRUCT)lParam;
        ::SetWindowLong( hWnd, GWL_USERDATA, (LONG)cs->lpCreateParams );
    }

    // Retrieve the Window instance and call start the message processing.
    Window* owner = (Window*)::GetWindowLong( hWnd, GWL_USERDATA );
    if( NULL != owner )
        return owner->ProcessMessage( hWnd, uMsg, wParam, lParam );

    return ::DefWindowProc( hWnd, uMsg, wParam, lParam );
}