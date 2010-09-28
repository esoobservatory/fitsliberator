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
#include "WindowsPlaneView.h"
#include "Resources.h"
#include "TextUtils.h"

using namespace FitsLiberator;
using namespace FitsLiberator::Engine;
using namespace FitsLiberator::Modelling;
using namespace FitsLiberator::Windows;

//---------------------------------------------------------------------------------------
// Constructors and destructors
//---------------------------------------------------------------------------------------
/*
Obsolete
WindowsPlaneView::WindowsPlaneView( PlaneModel& m, PlaneController& c )
  : PlaneView( m, c )

{

}
*/
WindowsPlaneView::WindowsPlaneView( PlaneModel& m, FlowController& c, PlaneController& pc )
: PlaneView( m, c, pc )
{
}

//---------------------------------------------------------------------------------------
// EventSink interface
//---------------------------------------------------------------------------------------
Void WindowsPlaneView::OnInit( Dialog* owner, Int ID ) {
    if( IDD_PREVIEW == ID ) {
        this->owner = owner;
        
        //
        // Get handles to the controls
        selector.setHandle( owner->getItem( IDC_PLANE_SELECTOR ) );
        widthLabel.setHandle( owner->getItem( IDC_PLANE_WIDTH ) );
        heightLabel.setHandle( owner->getItem( IDC_PLANE_HEIGHT ) );
        channel8Button = owner->getItem( IDC_PLANE_CHANNEL_8 );
        channel16Button = owner->getItem( IDC_PLANE_CHANNEL_16 );
        channel32Button = owner->getItem( IDC_PLANE_CHANNEL_32 );
        undefinedBlackButton = owner->getItem( IDC_PLANE_UNDEFINED_BLACK );
        undefinedTransparentButton = owner->getItem( IDC_PLANE_UNDEFINED_TRANSPARENT );
	
        //
        // Add stuff to the plane selector gizmo
        Int entryCount = getEntryCount();
        for( Int i = 0; i < entryCount; i++ ) {
            selector.AddItem( getEntry( i ) );
        }
        selector.setSelectedItem( 0 );
    }
	else if ( ID == IDD_MAIN )
	{
			flipButton = owner->getItem( IDC_PLANE_FLIP );
			updateFlip( model.getFlipped().flipped );
	}
}

Void WindowsPlaneView::OnCommand( WPARAM wParam, LPARAM lParam ) {
    UInt code = HIWORD(wParam);
    UInt command = LOWORD(wParam);

    if( code == CBN_SELCHANGE ) {
        if( command == IDC_PLANE_SELECTOR )
            selectorChanged( selector.getSelectedItem() );
    }
    else {
        switch( LOWORD(wParam) ) {
            case IDC_PLANE_CHANNEL_32:
                planeController.setBitDepth( channel32 );
                break;
            case IDC_PLANE_CHANNEL_16:
                planeController.setBitDepth( channel16 );
                break;
            case IDC_PLANE_CHANNEL_8:
                planeController.setBitDepth( channel8 );
                break;
            case IDC_PLANE_UNDEFINED_BLACK:
                planeController.setUndefined( undefinedBlack );
                break;
            case IDC_PLANE_UNDEFINED_TRANSPARENT:
                planeController.setUndefined( undefinedTransparent );
                break;
			case IDC_PLANE_FLIP:
				flowController.toggleFlip();
				break;
        }
    }
}

Void WindowsPlaneView::OnNotify( Int idCtrl, LPNMHDR pnmh ) {
    // No notiy events for this view
}

Void WindowsPlaneView::OnScroll( WPARAM wParam, LPARAM lParam ) {
    // No scroll events for this view
}

//---------------------------------------------------------------------------------------
// PlaneView implementation
//---------------------------------------------------------------------------------------

Void WindowsPlaneView::updateSelectorIndex( Int index ) {
    selector.setSelectedItem( index );
}

Void WindowsPlaneView::updateFlip( Bool fl )
{
	HWND control = flipButton;

	::SendMessage( control, BM_SETCHECK, fl, 0 );
    ::EnableWindow( control,  TRUE );
}

Void WindowsPlaneView::updateSize( Int width, Int height ) {
    String text;
    
    text = TextUtils::doubleToString( width, 0 );
    TextUtils::appendUnitText( text, "px" );
    widthLabel.setText( text );

    text = TextUtils::doubleToString( height, 0 );
    TextUtils::appendUnitText( text, "px" );
    heightLabel.setText( text );
}

Void WindowsPlaneView::updateBitDepth( ChannelSettings bitDepth, Bool selected, Bool enabled ) {
    HWND control = NULL;
    switch( bitDepth ) {
        case channel8:
            control = channel8Button;
            break;
        case channel16:
            control = channel16Button;
            break;
        case channel32:
            control = channel32Button;
            break;
    }

    ::SendMessage( control, BM_SETCHECK, selected ? BST_CHECKED : BST_UNCHECKED, 0 );
    ::EnableWindow( control, enabled ? TRUE : FALSE );
}

Void WindowsPlaneView::updateUndefinedOption( UndefinedSettings settings, Bool selected, Bool enabled ) {
    HWND control = settings == undefinedBlack 
        ? undefinedBlackButton 
        : undefinedTransparentButton;

    ::SendMessage( control, BM_SETCHECK, selected ? BST_CHECKED : BST_UNCHECKED, 0 );
    ::EnableWindow( control, enabled ? TRUE : FALSE );
}