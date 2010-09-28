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
#include "WindowsPreviewView.h"
#include "NumericTextbox.h"
#include "Resources.h"
#include "DropdownButton.h"

using namespace FitsLiberator::Modelling;
using namespace FitsLiberator::Windows;

const RECT WindowsPreviewView::previewBounds = { 36, 6, 36+312, 6+224 };

//---------------------------------------------------------------------------------------
// Constructors and destructors
//---------------------------------------------------------------------------------------

WindowsPreviewView::WindowsPreviewView(
    PreviewModel& previewModel, ToolModel& toolModel, GlobalSettingsModel& globalSettingsModel, PlaneModel& planeModel, ProgressModel& prgModel,
    PreviewController& previewController, FlowController& flowController)
  : PreviewView( previewModel, toolModel, globalSettingsModel, planeModel, prgModel, 
    previewController, flowController ),
    previewControl( previewModel, previewController, toolModel, 
	    globalSettingsModel, flowController, prgModel ) {
	owner = NULL;
	main = NULL;
}

Void WindowsPreviewView::OnInit( Dialog* owner, Int ID ) {
	if( IDD_MAIN == ID) {
		this->main = owner;
	}

    if( IDD_PREVIEW == ID ) {
        this->owner = owner;

        // Initialize the zoom list
		zoomList = new DropdownButton(owner->getItem(IDC_PREVIEW_ZOOMLIST));
        plusButton.setHandle( owner->getItem( IDC_PREVIEW_ZOOMIN ) );
        minusButton.setHandle( owner->getItem( IDC_PREVIEW_ZOOMOUT ) );
        
        //
        // Create the preview control	
        RECT bounds = previewBounds;
        ::MapDialogRect( owner->getHandle(), &bounds );
        previewControl.Create( owner->getHandle(), &bounds );
    }
}

Void WindowsPreviewView::OnCommand( WPARAM wParam, LPARAM lParam ) {
    Int event = HIWORD(wParam);
    Int command = LOWORD(wParam);

    FitsLiberator::Rectangle bounds = previewControl.getBounds();

    switch( event ) {
        case NTN_ENTERPRESSED:  // Enter was pressed in a text box
            //previewControl.SetFocus();
            break;
        default:                // Handle menu commands
            switch( command ) {
                case IDC_PREVIEW_ACTUAL:
                    //controller.setUnityZoom();
					flowController.setUnityZoom();
                    break;
                case IDC_PREVIEW_CENTER:
                    //controller.centerPreview();
					flowController.centerPreview();
                    break;
                case IDC_PREVIEW_FIT:
                    //controller.fitToPreview();
					flowController.fitToPreview();
                    break;
                case IDC_PREVIEW_ZOOMIN:
                    //controller.incrementZoom( Point( bounds.getWidth() / 2, bounds.getHeight() / 2 ) );
					flowController.incrementZoom( Point( bounds.getWidth() / 2, bounds.getHeight() / 2 ) );
                    previewControl.SetFocus();
                    break;
                case IDC_PREVIEW_ZOOMOUT:
                    //controller.decrementZoom( Point( bounds.getWidth() / 2, bounds.getHeight() / 2 ) );
					flowController.decrementZoom( Point( bounds.getWidth() / 2, bounds.getHeight() / 2 ) );
                    previewControl.SetFocus();
                    break;
				default:
					if( command >= IDC_PREVIEW_ZOOM0 && command <= IDC_PREVIEW_LAST ) {
						//controller.setZoomIndex(command - IDC_PREVIEW_ZOOM0);
						flowController.setZoomIndex( command - IDC_PREVIEW_ZOOM0 );
					}
            }
            break;
    }
}

Void WindowsPreviewView::OnNotify( Int idCtrl, LPNMHDR pnmh ) {
    // No notify events for this view
}

Void WindowsPreviewView::OnScroll( WPARAM wParam, LPARAM lParam ) {
    // No scroll events for this view
}

Void WindowsPreviewView::ShortcutFilter( Bool down, UInt virtualKey ) {
    Char key = ::MapVirtualKey( virtualKey, 2 );    // Get a char from a virtual key.
    FitsLiberator::Rectangle bounds = previewControl.getBounds();

    if( down ) {    // We don't want to call the controller for both down and up events
        switch( key ) {
            case kFITSFitInPreviewShortcut:
                flowController.fitToPreview();
                break;
            case kFITSActualPixelsShortcut:
                flowController.setUnityZoom();
                break;
            case kFITSCenterPreviewShortcut:
                flowController.centerPreview();
                break;
            case kFITSIncrementZoomShortcut:
                flowController.incrementZoom( Point( bounds.getWidth() / 2, bounds.getHeight() / 2 ) );
                break;
            case kFITSDecrementZoomShortcut:
                flowController.decrementZoom( Point( bounds.getWidth() / 2, bounds.getHeight() / 2 ) );
                break;
        }
    }
}

//---------------------------------------------------------------------------------------
// PreviewView implementation
//---------------------------------------------------------------------------------------
Void WindowsPreviewView::drawPreview() {
    previewControl.Update();
}

Void WindowsPreviewView::setZoomCombo(Int i)
{
	String text = " " + getZoomText();
	zoomList->setText(text);
    plusButton.setEnabled( model.canIncrement() );
    minusButton.setEnabled( model.canDecrement() );

    previewControl.OnToolChanged( toolModel.getCurrentFunction() );
}

Void WindowsPreviewView::updateZooms(String* values, Int cnt)
{
	HMENU hPopup = ::CreatePopupMenu();
	int unity = model.getUnityZoomIndex();

	assert(hPopup != 0);
	
	UINT_PTR dummy = 0;
	UINT_PTR command = IDC_PREVIEW_ZOOM0;
	for( int i = 0; i < cnt; ++i ) {
		if( i == unity || i == unity + 1 || i == cnt - 1 ) {
			::AppendMenu(hPopup, MF_SEPARATOR, dummy, 0);
		}
		::AppendMenu(hPopup, MF_ENABLED | MF_STRING, command++, values[i].c_str());
	}
	zoomList->Menu(hPopup);
}	

Void WindowsPreviewView::setCurrentTool( ToolTypeFunction tool ) {
    previewControl.OnToolChanged( tool );
}

Void WindowsPreviewView::updateZoomComboState( Bool enabled ) {
    zoomList->setEnabled( enabled );
    plusButton.setEnabled( model.canIncrement() && enabled  );
    minusButton.setEnabled( model.canDecrement() && enabled );
}

void WindowsPreviewView::gotBusy() {
    previewControl.gotBusy();
}

void WindowsPreviewView::gotDone() {
    previewControl.gotDone();
	// TODO: Ugly hack!
	if(progressModel.QueryInitializing() && progressModel.QueryCancel()) {
		main->Close(IDCANCEL);
	}
}
