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
#include "WindowsGlobalSettingsView.h"
#include "Resources.h"

using namespace FitsLiberator::Modelling;
using namespace FitsLiberator::Windows;

//---------------------------------------------------------------------------------------
// Constructors and destructors
//---------------------------------------------------------------------------------------

WindowsGlobalSettingsView::WindowsGlobalSettingsView( GlobalSettingsModel& model, GlobalSettingsController& controller ) 
  : GlobalSettingsView( model, controller ) {
    
}

//---------------------------------------------------------------------------------------
// EventSink interface
//---------------------------------------------------------------------------------------

/**
 * Called when a dialog is created, hooks up the relevant control events.
 * @param owner Dialog being created.
 * @param ID Template ID of the dialog being created.
 */
Void WindowsGlobalSettingsView::OnInit( Dialog* owner, Int ID ) {
    if( IDD_MAIN == ID ) {
        //
        // Get handles to the controls
        this->owner = owner;

        hWndPreview = owner->getItem( IDC_GLOBAL_PREVIEW );
        hWndMarkUndefined = owner->getItem( IDC_GLOBAL_UNDEFINED );
        hWndMarkWhiteClipping = owner->getItem( IDC_GLOBAL_WHITE );
        hWndMarkBlackClipping = owner->getItem( IDC_GLOBAL_BLACK );
        hWndFreeze = owner->getItem( IDC_GLOBAL_FREEZE ); 

        imageInformationScaled = owner->getItem( IDC_GLOBAL_SCALED );
        imageInformationStretched = owner->getItem( IDC_GLOBAL_STRETCHED );

        //
        // Init the controls
        updatePreviewEnabled( model.getPreviewEnabled() );
        updateFreezeSettings( model.getFreezeSettings() );
        updateMarkUndefined( model.getMarkUndefined() );
        updateMarkWhiteClipping( model.getMarkWhiteClipping() );
        updateMarkBlackClipping( model.getMarkBlackClipping() );
        updateImageInformation( model.getImageInformation() );
    }
}

/**
 * WM_COMMAND handler
 * @see Window::OnCommand
 */
Void WindowsGlobalSettingsView::OnCommand( WPARAM wParam, LPARAM lParam ) {
    Int control = LOWORD( wParam ); // ID of the control that sent the notification
    switch( control ) {
        case IDC_GLOBAL_FREEZE:
            controller.toggleFreezeSettings();
            break;
        case IDC_GLOBAL_PREVIEW:
            controller.togglePreviewEnabled();
            break;
        case IDC_GLOBAL_UNDEFINED:
            controller.toggleMarkUndefined();
            break;
        case IDC_GLOBAL_WHITE:
            controller.toggleMarkWhiteClipping();
            break;
        case IDC_GLOBAL_BLACK:
            controller.toggleMarkBlackClipping();
            break;
        case IDC_GLOBAL_SCALED:
            controller.setImageInformation( valuesScaled );
            break;
        case IDC_GLOBAL_STRETCHED:
            controller.setImageInformation( valuesStretched );
            break;
    }
}

/**
 * WM_NOTIFY handler
 * @see Window::OnNotify
 */
Void WindowsGlobalSettingsView::OnNotify( Int idCtrl, LPNMHDR pnmh ) {
    // No notify events for this view
}

Void WindowsGlobalSettingsView::OnScroll( WPARAM wParam, LPARAM lParam ) {
    // No scroll events for this view
}

//---------------------------------------------------------------------------------------
// GlobalSettingsView implementation
//---------------------------------------------------------------------------------------
Void WindowsGlobalSettingsView::updatePreviewEnabled( Bool value ) {
    ::SendMessage( hWndPreview, BM_SETCHECK, value ? BST_CHECKED : BST_UNCHECKED, 0 );
}

Void WindowsGlobalSettingsView::updateFreezeSettings( Bool value ) {
    ::SendMessage( hWndFreeze, BM_SETCHECK, value ? BST_CHECKED : BST_UNCHECKED, 0 );
}

Void WindowsGlobalSettingsView::updateMarkUndefined( Bool value ) {
    ::SendMessage( hWndMarkUndefined, BM_SETCHECK, value ? BST_CHECKED : BST_UNCHECKED, 0 );
}

Void WindowsGlobalSettingsView::updateMarkWhiteClipping( Bool value ) {
    ::SendMessage( hWndMarkWhiteClipping, BM_SETCHECK, value ? BST_CHECKED : BST_UNCHECKED, 0 );
}

Void WindowsGlobalSettingsView::updateImageInformation( FitsLiberator::Modelling::ImageInformationSettings s) {
    ::SendMessage( imageInformationScaled, BM_SETCHECK, (s == valuesScaled) ? BST_CHECKED : BST_UNCHECKED, 0 );
    ::SendMessage( imageInformationStretched, BM_SETCHECK, (s == valuesStretched) ? BST_CHECKED : BST_UNCHECKED, 0 );
}

Void WindowsGlobalSettingsView::updateMarkBlackClipping( Bool value ) {
    ::SendMessage( hWndMarkBlackClipping, BM_SETCHECK, value ? BST_CHECKED : BST_UNCHECKED, 0 );
}