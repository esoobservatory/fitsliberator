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
#include "WindowsPixelValueView.h"
#include "Resources.h"
#include "TextUtils.h"

using namespace FitsLiberator::Modelling;
using namespace FitsLiberator::Windows;

//---------------------------------------------------------------------------------------
// Constructors and destructors
//---------------------------------------------------------------------------------------

WindowsPixelValueView::WindowsPixelValueView( PixelValueModel& m, GlobalSettingsModel& g ) : PixelValueView( m, g ) {

}

//---------------------------------------------------------------------------------------
// EventSink interface
//---------------------------------------------------------------------------------------
Void WindowsPixelValueView::OnInit( Dialog* owner, Int ID ) {
    if( IDD_PREVIEW == ID ) {
        this->owner = owner;

        realValue = owner->getItem( IDC_PICKER_REAL );
        stretchedValue = owner->getItem( IDC_PICKER_STRETCHED );
        locationX = owner->getItem( IDC_PICKER_X );
        locationY = owner->getItem( IDC_PICKER_Y );
		raValue = owner->getItem( IDC_PICKER_RA );
		decValue = owner->getItem( IDC_PICKER_DEC );
    }
}

Void WindowsPixelValueView::OnCommand( WPARAM wParam, LPARAM lParam ) {
    // No command events for this view
}

Void WindowsPixelValueView::OnNotify( Int idCtrl, LPNMHDR pnmh ) {
    // No notify events for this view
}

Void WindowsPixelValueView::OnScroll( WPARAM wParam, LPARAM lParam ) {
    // No scroll events for this view
}

//---------------------------------------------------------------------------------------
// PixelValueView implementation
//---------------------------------------------------------------------------------------
Void WindowsPixelValueView::updateXValue( Int x ) {
    String text = TextUtils::doubleToString( (Double)x, 0 );
    TextUtils::appendUnitText( text, String("px") );
    Window::setText( locationX, text );
}

Void WindowsPixelValueView::updateYValue( Int y ) {
    String text = TextUtils::doubleToString( (Double)y, 0 );
    TextUtils::appendUnitText( text, String("px") );
    Window::setText( locationY, text );
}

Void WindowsPixelValueView::updateRealValue( Double value ) {
    Window::setText( realValue, TextUtils::doubleToString( value ) );
}

Void WindowsPixelValueView::updateStretchValue( Double value ) {
    Window::setText( stretchedValue, TextUtils::doubleToString( value ) );
}

Void WindowsPixelValueView::updateScaledValue( Double value ) {
    Window::setText( stretchedValue, TextUtils::doubleToString( value ) );
}
Void WindowsPixelValueView::updateText( String& s) {
    Window::setText( owner->getItem( IDC_PICKER_MODE ), s );
}

Void WindowsPixelValueView::updateRa( Double d )
{
	Window::setText( raValue, TextUtils::doubleToString( d ) );
}

Void WindowsPixelValueView::updateDec( Double d )
{
	Window::setText( decValue, TextUtils::doubleToString( d ) );
}

Void WindowsPixelValueView::updateShowValues( Bool show ) {
    Int sw = show ? SW_SHOW : SW_HIDE;
    ::ShowWindow( locationX, sw );
    ::ShowWindow( locationY, sw );
    ::ShowWindow( realValue, sw );
    ::ShowWindow( stretchedValue, sw );
	::ShowWindow( raValue, sw );
	::ShowWindow( decValue, sw );
}