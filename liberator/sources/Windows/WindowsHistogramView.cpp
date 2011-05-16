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
#include "WindowsHistogramView.h"
#include "Resources.h"
#include "MainDialog.h"

using namespace FitsLiberator::Modelling;
using namespace FitsLiberator::Windows;

const RECT WindowsHistogramView::histogramBounds = { 36, 252, 36+312, 330 };

//---------------------------------------------------------------------------------------
// Constructors and destructors
//---------------------------------------------------------------------------------------
/*obsolete
WindowsHistogramView::WindowsHistogramView( HistogramModel& histogramModel, HistogramController& histogramController, ToolModel& toolModel )
  : HistogramView( histogramModel, histogramController, toolModel ),
    histogramControl( histogramModel, histogramController, toolModel ),
    sliderControl( histogramModel, histogramController ) {

}
*/
WindowsHistogramView::WindowsHistogramView( HistogramModel& hm, ToolModel& tm, StatisticsModel& sm, StretchModel& sf, OptionsModel& om, HistogramController& hc, FlowController& fc )
: HistogramView( hm, tm, sm, sf, om, hc, fc ),
  histogramControl( model, *this, toolModel, fc ),
  sliderControl( model, histoController, fc )
{

}
//---------------------------------------------------------------------------------------
// EventSink interface
//---------------------------------------------------------------------------------------
Void WindowsHistogramView::OnInit( Dialog* owner, Int ID ) {
    if( IDD_MAIN == ID ) {
        mainDialog = (MainDialog*)owner;
    }
    else if( IDD_PREVIEW == ID ) {
        hWndWhiteSpin = owner->getItem( IDC_HISTOGRAM_WHITESPIN );
        hWndBlackSpin = owner->getItem( IDC_HISTOGRAM_BLACKSPIN );
		
        whiteLevel.Attach( owner->getItem( IDC_HISTOGRAM_WHITE ) );
        blackLevel.Attach( owner->getItem( IDC_HISTOGRAM_BLACK ) );

		UDACCEL accelaration;
        accelaration.nInc = 1;
        accelaration.nSec = 0;

		::SendMessage( hWndWhiteSpin, UDM_SETRANGE, 0, MAKELONG(2, 0) );
        ::SendMessage( hWndWhiteSpin, UDM_SETPOS, 0, MAKELONG(1, 0) );
        ::SendMessage( hWndWhiteSpin, UDM_SETACCEL, 1, (LPARAM)&accelaration );

        ::SendMessage( hWndBlackSpin, UDM_SETRANGE, 0, MAKELONG(2, 0) );
        ::SendMessage( hWndBlackSpin, UDM_SETPOS, 0, MAKELONG(1, 0) );
        ::SendMessage( hWndBlackSpin, UDM_SETACCEL, 1, (LPARAM)&accelaration );

		whiteLevel.setValue( model.getWhiteLevel() );
        blackLevel.setValue( model.getBlackLevel() );


        //
        // Create the controls
        RECT bounds;
        
        bounds = histogramBounds;
        ::MapDialogRect( owner->getHandle(), &bounds );
        
        bounds.bottom -= SliderControl::preferredHeight;
        histogramControl.Create( owner->getHandle(), &bounds );
        
        bounds.top    = bounds.bottom;
        bounds.bottom = bounds.top + SliderControl::preferredHeight;
        sliderControl.Create( owner->getHandle(), &bounds );
        
        minLabel.setHandle( owner->getItem( IDC_HISTOGRAM_MIN ) );
        maxLabel.setHandle( owner->getItem( IDC_HISTOGRAM_MAX ) );
    }
}

Void WindowsHistogramView::OnCommand( WPARAM wParam, LPARAM lParam ) 
{
    UInt code = HIWORD(wParam);
    UInt command = LOWORD(wParam);

    FitsLiberator::Rectangle bounds = histogramControl.getClient();
    switch( code ) {
        case NTN_ENTERPRESSED:
            OnNumericEnter( command );
            break;
        case NTN_UP:
        case NTN_DOWN:
            OnNumericArrow( command, code );
            break;
        case NTN_GOTFOCUS:
            OnNumericGotFocus( command );
            break;
        case NTN_LOSTFOCUS:
            OnNumericLostFocus( command );
            break;
		default:
            switch( command ) {
                case IDC_HISTOGRAM_SHOWALL:
                    flowController.showAllHistogram();
                    break;
                case IDC_HISTOGRAM_ZOOMIN:
                    flowController.incrementHistogramZoom( bounds.getWidth() / 2 ) ;
                    break;
                case IDC_HISTOGRAM_ZOOMOUT:
                    flowController.decrementHistogramZoom( bounds.getWidth() / 2 ) ;
            }
            break;
    }
}

Void WindowsHistogramView::OnNumericArrow( UInt command, UInt code ) {
	Bool shift = ( (Short)::GetKeyState( VK_SHIFT ) < 0 );
    switch( command ) {
        case IDC_HISTOGRAM_WHITE:
            flowController.incrementWhiteLevel( code == NTN_UP, shift );
            break;
        case IDC_HISTOGRAM_BLACK:
            flowController.incrementBlackLevel( code == NTN_UP, shift );
            break;
    }
}

Void WindowsHistogramView::OnNumericEnter( UInt command ) {
    switch( command ) {
        case IDC_HISTOGRAM_WHITE:
            //histoController.setWhiteLevel( whiteLevel.getValue() );
			flowController.setWhiteLevel( whiteLevel.getValue() );
            break;
        case IDC_HISTOGRAM_BLACK:
            //histoController.setBlackLevel( blackLevel.getValue() );			
			flowController.setBlackLevel( blackLevel.getValue() );
			break;
    }
}

Void WindowsHistogramView::OnNumericGotFocus( UInt ) {
    mainDialog->disableShortcuts();
}

Void WindowsHistogramView::OnNumericLostFocus( UInt command ) {
    mainDialog->enableShortcuts();
    switch( command ) {
        case IDC_HISTOGRAM_WHITE:
            if( whiteLevel.getValue() != model.getWhiteLevel() ) {
               // histoController.setWhiteLevel( whiteLevel.getValue() );				
				flowController.setWhiteLevel( whiteLevel.getValue() );
            }
            break;
        case IDC_HISTOGRAM_BLACK:
            if( blackLevel.getValue() != model.getBlackLevel() ) {
                //histoController.setBlackLevel( blackLevel.getValue() );
				flowController.setBlackLevel( blackLevel.getValue() );
            }
            break;
    }
}

Void WindowsHistogramView::OnNotify( Int idCtrl, LPNMHDR pnmh ) {
    // No notify events for this view
}

Void WindowsHistogramView::OnScroll( WPARAM wParam, LPARAM lParam ) {
    // Validate the control
    HWND hWndSpin = (HWND)lParam;
    if( !(hWndSpin == hWndWhiteSpin || hWndSpin == hWndBlackSpin) )
        return;

    Int pos = ::SendMessage( hWndSpin, UDM_GETPOS, 0, 0 ) & 0xFFFF; // Mask out any error
    
    // pos == 1 is the center position, and we only care about changes.
    if( 1 != pos ) {
        Bool up = (pos == 2);
        Bool shift = ( (Short)::GetKeyState( VK_SHIFT ) < 0 );

        //
        // Dispatch the event
        if( hWndSpin == hWndWhiteSpin )
            flowController.incrementWhiteLevel( up, shift );
        if( hWndSpin == hWndBlackSpin )
            flowController.incrementBlackLevel( up, shift );
        //
        // Reset the position
        ::SendMessage( hWndSpin, UDM_SETPOS, 0, MAKELONG(1, 0) );
    }
}

//---------------------------------------------------------------------------------------
// HistogramView implementation
//---------------------------------------------------------------------------------------

Void WindowsHistogramView::drawHistogram() {
    histogramControl.Refresh();
    sliderControl.Refresh();
}

Void WindowsHistogramView::setHistogramMin(Double min) {
    minLabel.setValue( min );
}

Void WindowsHistogramView::setHistogramMax(Double max) {
    maxLabel.setValue( max );
}

Void WindowsHistogramView::setWhiteLevel( Double value ) {
    whiteLevel.setValue( value );
}


Void WindowsHistogramView::setBlackLevel( Double value ) {
    blackLevel.setValue( value );
}


Void WindowsHistogramView::setCurrentToolFunction( ToolTypeFunction tool ) {
    histogramControl.OnToolChanged( tool );
}
