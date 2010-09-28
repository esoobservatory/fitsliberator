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
#include "WindowsToolView.h"
#include "Resources.h"
#include "Menu.h"

using namespace FitsLiberator::Modelling;
using namespace FitsLiberator::Windows;

const RECT WindowsToolView::toolbarBounds = { 8, 8, 200+8, 30+8 };

//---------------------------------------------------------------------------------------
// Constructors and destructors
//---------------------------------------------------------------------------------------

WindowsToolView::WindowsToolView( ToolModel& model, ToolController& controller )
  : ToolView( model, controller ){

}

//---------------------------------------------------------------------------------------
// EventSink interface
//---------------------------------------------------------------------------------------
Void WindowsToolView::OnInit( Dialog* owner, Int ID ) {
    if( IDD_PREVIEW == ID ) {
        this->owner = owner;

        //
        // Create the toolbar
        RECT bounds = toolbarBounds;
        ::MapDialogRect( owner->getHandle(), &bounds );
        toolbarControl.Create( owner->getHandle(), &bounds );

        setMagnifierZoomStatus( true );
    }
}

Void WindowsToolView::OnCommand( WPARAM wParam, LPARAM lParam ) {
    Int command = LOWORD( wParam );
    switch( command ) {
        case IDC_TOOL_BLACK:
            controller.toolBlackLevelSelected();
            break;
        case IDC_TOOL_MOVE:
            controller.toolHandSelected();
            break;
        case IDC_TOOL_BACKGROUND:
            controller.toolBackgroundSelected();
            break;
        case IDC_TOOL_PEAK:
            controller.toolPeakSelected();
            break;
        case IDC_TOOL_WHITE:
            controller.toolWhiteLevelSelected();
            break;
        case IDC_TOOL_ZOOM:
            controller.toolZoomSelected();
            break;
    }
}

Void WindowsToolView::OnNotify( Int idCtrl, LPNMHDR pnmh ) {
    // No notify events for this view
}

Void WindowsToolView::OnScroll( WPARAM wParam, LPARAM lParam ) {
    // No scroll events for this view
}


//---------------------------------------------------------------------------------------
// ToolView implementation
//---------------------------------------------------------------------------------------
Void WindowsToolView::setHandStatus( Bool selected ) {
    toolbarControl.updateButton( ToolbarControl::ButtonMove, selected );
}

Void WindowsToolView::setEyedropperBackgroundStatus( Bool selected ) {
    toolbarControl.updateButton( ToolbarControl::ButtonBackground, selected );
}

Void WindowsToolView::setEyedropperPeakStatus( Bool selected ) {
    toolbarControl.updateButton( ToolbarControl::ButtonPeak, selected );
}

Void WindowsToolView::setEyedropperBlacklevelStatus( Bool selected ) {
    toolbarControl.updateButton( ToolbarControl::ButtonBlack, selected );
}

Void WindowsToolView::setEyedropperWhitelevelStatus( Bool selected ) {
    toolbarControl.updateButton( ToolbarControl::ButtonWhite, selected );
}

Void WindowsToolView::setPickerStatus( Bool selected ) {

}

Void WindowsToolView::setMagnifierZoomStatus( Bool selected ) {
    toolbarControl.updateButton( ToolbarControl::ButtonZoom, selected );
}