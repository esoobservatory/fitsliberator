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
#include "NumericTextbox.h"
#include "TextUtils.h"
#include "Environment.h"

using namespace FitsLiberator;
using namespace FitsLiberator::Windows;

String NumericTextbox::allowedCharacters;

NumericTextbox::NumericTextbox() {
    precision = 2;
    oldWindowProc = NULL;

    if( allowedCharacters.length() == 0 ) {
        allowedCharacters.append( "0123456789eE\x08%" );
        allowedCharacters.append( Environment::getDecimalSeparator() );
        allowedCharacters.append( Environment::getThousandsSeparator() );
        allowedCharacters.append( Environment::getNegativeSign() );
        allowedCharacters.append( Environment::getPositiveSign() );
    }

    embedded = false;
}

NumericTextbox::~NumericTextbox() {
    Detach();
}

Void NumericTextbox::Attach( HWND hWnd ) {
    Detach();

    if( ::IsWindow( hWnd ) ) {
        ::SetWindowLong( hWnd, GWL_USERDATA, (LONG)this );
        setHandle( hWnd );
        this->oldWindowProc = (WNDPROC)::SetWindowLong( hWnd, GWL_WNDPROC, (LONG)NumericTextbox::WindowProc );
    }
}

Void NumericTextbox::Detach() {
    if( NULL != oldWindowProc ) {
        ::SetWindowLong( hWnd, GWL_WNDPROC, (LONG)oldWindowProc );
        oldWindowProc = NULL;
    }
    setHandle(NULL);
}

Int NumericTextbox::getPrecision() const {
    return precision;
}

Void NumericTextbox::setPrecision( Int value ) {
    precision = value;
}

Double NumericTextbox::getValue() const {
    String text = getText();
    if(postfix.length() > 0) {
        String::size_type i = text.find(postfix);
        if(i < text.length()) {
            text = text.substr(0, i);
        }
    }
    return TextUtils::stringToDouble( text );
}

Void NumericTextbox::setValue( Double value ) {
    setText( TextUtils::doubleToString( value, precision ) + postfix );
}

String 
NumericTextbox::Postfix() const {
    return postfix;
}

void
NumericTextbox::Postfix(const String& value) {
    double content = getValue();
    postfix = value;
    setValue(content);
}

Void NumericTextbox::NotifyParent(USHORT command) {
    Int  id;
    HWND owner;
    
    // If we are embedded we need the parent of the parent.
    if( embedded ) {
        id    = ::GetDlgCtrlID( ::GetParent( hWnd ) );
        owner = ::GetParent( ::GetParent( hWnd ) );
    }
    else {
        id    = ::GetDlgCtrlID( hWnd );
        owner = ::GetParent( hWnd );
    }

    ::SendMessage( owner, WM_COMMAND, MAKEWPARAM( id, command ), (LPARAM)hWnd );
}

LRESULT CALLBACK NumericTextbox::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    NumericTextbox* owner = (NumericTextbox*)::GetWindowLong( hWnd, GWL_USERDATA );
    switch( uMsg ) {
        case WM_SETTEXT:
            ::CallWindowProc( owner->oldWindowProc, hWnd, uMsg, wParam, lParam );
            owner->SelectAll();
            return 0;
        case WM_KEYDOWN:
            // Handle the arrow keys
            if( wParam == VK_UP )
                owner->NotifyParent( NTN_UP );
            if( wParam == VK_DOWN )
                owner->NotifyParent( NTN_DOWN );
            break;
        case WM_CHAR:
            // Filter the keys to only allow numeric characters
            if( wParam == VK_RETURN ) {
                owner->NotifyParent( NTN_ENTERPRESSED );
            }
            else if ( (::GetAsyncKeyState( VK_CONTROL ) & 0x80000000) || allowedCharacters.find_first_of( wParam ) < allowedCharacters.length() ) {
                return ::CallWindowProc( owner->oldWindowProc, hWnd, uMsg, wParam, lParam );
            }

            return 0;   // We handled this
        case WM_KILLFOCUS:
            //if( !(owner->embedded && (::GetParent( hWnd ) == (HWND)wParam) ) )     // Don't notify when we loose focus to the parent
                owner->NotifyParent( NTN_LOSTFOCUS );
            break;
        case WM_SETFOCUS:
            //if( !(owner->embedded && (::GetParent( hWnd ) == (HWND)wParam) ) )     // Don't notify when we get focus from the parent
                owner->NotifyParent( NTN_GOTFOCUS );
            break;
        case WM_NCDESTROY:
            owner->Detach();
            break;
    }

    // Invoke default handler
    return ::CallWindowProc( owner->oldWindowProc, hWnd, uMsg, wParam, lParam );
}