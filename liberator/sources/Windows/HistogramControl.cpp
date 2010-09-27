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
#include "HistogramControl.h"
#include "Resources.h"
#include "TextUtils.h"
#include "Menu.h"
#include "Environment.h"

using namespace Gdiplus;
using namespace FitsLiberator;
using namespace FitsLiberator::Modelling;
using namespace FitsLiberator::Windows;

// ------------------------------------------------------------------------------------------------
// Implementation of HistogramControl
// ------------------------------------------------------------------------------------------------

/**
 * Default constructor
 */
HistogramControl::HistogramControl(HistogramModel& model, HistogramView& view, 
								   ToolModel& toolModel, FlowController& fc )
  : model( model ),
    view( view ),
    toolModel( toolModel ),
	flowController( fc )
{
    histogramPen = (HGDIOBJ)::CreatePen( PS_SOLID, 1, kFITSHistogramColor );
    sliderPen    = (HGDIOBJ)::CreatePen( PS_SOLID, 1, kFITSHistogramSliderColor );
}

/**
 * Destructor, frees the system cursors, pens and brushes used.
 */
HistogramControl::~HistogramControl() {
    delete histogramTracker;
    delete histogramMoveTrackerSink;
    delete histogramSelectionTrackerSink;

    ::DeleteObject( histogramPen );
    ::DeleteObject( sliderPen );
}

/**
 * Creates the control.
 * @param hWndParent Parent window.
 * @param bounds Boundaries of the control.
 * @return True on success, false on failure.
 */
Bool HistogramControl::Create( HWND hWndParent, const LPRECT bounds ) {
    return super::Create( hWndParent, bounds, WS_VISIBLE | WS_CHILD, 0 );
}

/**
 * Creates the control.
 * @param hWnd Parent window.
 */
Void HistogramControl::OnCreate( HWND hWnd ) {
    super::OnCreate( hWnd );

    RECT rc;
    ::GetClientRect( hWnd, &rc );
    model.setHistogramSize( Size( rc.right - rc.left, rc.bottom - rc.top ) );

    //
    // Create the mouse trackers
    histogramMoveTrackerSink = new MoveTrackerSink();
    histogramSelectionTrackerSink = new HistogramSelectionTrackerSink( hWnd, rc );
    histogramTracker = new MouseTracker( histogramSelectionTrackerSink, rc );
}

/**
 * Implements custom painting.
 * @see CanvasControl::OnRefresh
 */
Void HistogramControl::OnRefresh( HDC hDC ) {
    FitsLiberator::Rectangle bounds = getClient();
    PaintBackground(hDC, bounds);
    PaintContent(hDC, bounds);
    for(int i = 0; i < HistogramView::MarkerCount; ++i)
        PaintMarker(hDC, bounds, i);
    PaintSliders(hDC, bounds);
    PaintSelection(hDC, bounds);
}

void
HistogramControl::PaintBackground(HDC hDC, const FitsLiberator::Rectangle &bounds) {
    HGDIOBJ oldPen = ::SelectObject( hDC, histogramPen );
    HGDIOBJ oldBrush = ::SelectObject( hDC, (HGDIOBJ)::GetSysColorBrush( COLOR_WINDOW ) );
    ::Rectangle( hDC, bounds.left, bounds.top, bounds.right, bounds.bottom + 2);
    ::SelectObject( hDC, oldPen );
    ::SelectObject( hDC, oldBrush );    
}

void
HistogramControl::PaintContent(HDC hDC, const FitsLiberator::Rectangle &bounds) {
    HGDIOBJ oldPen = ::SelectObject( hDC, histogramPen );

    Vector<Int>& histogram = model.getEndBins();
    UInt count = histogram.size();
    for( UInt x = 1; x <= count; x++ ) {  // start from 1 to account for the frame
        ::MoveToEx( hDC, x, bounds.bottom, NULL );
		::LineTo( hDC, x, bounds.bottom - histogram[x-1] + 1);
    }

    ::SelectObject( hDC, oldPen );
}

void
HistogramControl::PaintSliders(HDC hDC, const FitsLiberator::Rectangle &bounds) {
    HGDIOBJ oldPen = ::SelectObject( hDC, sliderPen );
    int position = model.getBlackSliderPos();
    ::MoveToEx( hDC, position, bounds.bottom, NULL );
    ::LineTo( hDC, position, bounds.top + 1 );

    position = model.getWhiteSliderPos();
    ::MoveToEx( hDC, position, bounds.bottom, NULL );
    ::LineTo( hDC, position, bounds.top + 1 );

    ::SelectObject( hDC, oldPen );
}

void
HistogramControl::PaintSelection(HDC hDC, const FitsLiberator::Rectangle &bounds) {
    if( histogramSelectionTrackerSink->hasSelection() ) {
        Gdiplus::Graphics g( hDC );

        RECT selection = histogramSelectionTrackerSink->getSelection();
        Gdiplus::Rect rect( selection.left, selection.top, selection.right - selection.left, selection.bottom - selection.top);
        COLORREF highlightColor = ::GetSysColor( COLOR_HIGHLIGHT );
        Gdiplus::Pen pen( Gdiplus::Color( 0xFF, GetRValue( highlightColor ), GetGValue( highlightColor ), GetBValue( highlightColor ) ) );
        Gdiplus::SolidBrush brush( Gdiplus::Color( 0x7F, GetRValue( highlightColor ), GetGValue( highlightColor ), GetBValue( highlightColor ) ) );
        g.FillRectangle( &brush, rect );
        g.DrawRectangle( &pen, rect );
    }
}

void
HistogramControl::PaintMarker(HDC hDC, const FitsLiberator::Rectangle& bounds, int index) {
    COLORREF color      = RGB(view.markerColors[index].red, view.markerColors[index].green, 
                            view.markerColors[index].blue);
    HPEN     markerPen  = ::CreatePen( PS_SOLID, 1, color );
    int      position   = view.markerPositions[index];
    
    HGDIOBJ oldPen = ::SelectObject( hDC, markerPen );
    ::MoveToEx( hDC, position, bounds.bottom, NULL );
    ::LineTo( hDC, position, bounds.top );
    ::SelectObject( hDC, oldPen );
}

/**
 * @see Window::OnMouseDown
 */
Void HistogramControl::OnMouseDown( Int button, Int x, Int y ) {
    super::OnMouseDown( button, x, y );

    histogramTracker->OnMouseDown( this->hWnd, button, x, y );
}

/**
 * @see Window::OnMouseMove
 */
Void HistogramControl::OnMouseMove( Int button, Int x, Int y ) {
    super::OnMouseMove( button, x, y );
    SendMessage( WM_SETCURSOR, (WPARAM)this->hWnd, HTCLIENT );

    if( MouseActionTracking == histogramTracker->OnMouseMove( hWnd, button, x, y ) ) {
        switch( toolModel.getCurrentFunction() ) {
            case kFITSToolFunctionMove:
                flowController.moveHistogram( histogramMoveTrackerSink->getDistance().cx );
                histogramMoveTrackerSink->Reset();
                break;
            case kFITSToolEyedropperBlackLevel:
                flowController.moveBlackLevelSlider( histogramMoveTrackerSink->getLocation().x );
                histogramMoveTrackerSink->Reset();
                break;
            case kFITSToolEyedropperWhiteLevel:
                flowController.moveWhiteLevelSlider( histogramMoveTrackerSink->getLocation().x );
                histogramMoveTrackerSink->Reset();
                break;
        }
    }
}

/**
 * @see Window::OnMouseUp
 */
Void HistogramControl::OnMouseUp( Int button, Int x, Int y ) {
    super::OnMouseUp( button, x, y );

    POINT pt;
    ::GetCursorPos( &pt );
    ::ScreenToClient( hWnd, &pt );
    ToolTypeFunction function = toolModel.getCurrentFunction();

    switch( histogramTracker->OnMouseUp( this->hWnd, button, x, y ) ) {
        case MouseActionTracking:
            switch( function ) {
                case kFITSToolEyedropperBlackLevel:
                case kFITSToolEyedropperWhiteLevel:
                    flowController.applyLevels();
                    break;
                case kFITSToolFunctionZoomIn:
                case kFITSToolFunctionZoomOut:
                    RECT& selection = histogramSelectionTrackerSink->getSelection();
                    flowController.showRange( selection.left, selection.right );
                    break;
            }
            break;
        case MouseActionLeftClick:
            switch( function ) {
                case kFITSToolFunctionZoomIn:
                    flowController.incrementHistogramZoom( x );
                    break;
                case kFITSToolFunctionZoomOut:
                    flowController.decrementHistogramZoom( x );
                    break;
                case kFITSToolEyedropperBlackLevel:
                    flowController.moveBlackLevelSlider( x );
                    flowController.applyLevels();
                    break;
                case kFITSToolEyedropperWhiteLevel:
                    flowController.moveWhiteLevelSlider( x );
                    flowController.applyLevels();
                    break;
            }
            break;
        case MouseActionRightClick:
            ::ClientToScreen( hWnd, &pt );
            
//            Environment::setCursor( kPICursorArrow );
            Menu::Popup( IDM_HISTOGRAM_MENU, ::GetParent(hWnd), pt );
            super::OnSetCursor( hWnd, 0, 0 );

            break;
    }

    OnToolChanged( function );  // To make sure the cursor is updated
}

/**
 * @return True if the control can zoom in, false otherwise.
 */
Bool HistogramControl::canZoomIn() {
    return true;
}

/**
 * @return True if the control can zoom out, false otherwise.
 */
Bool HistogramControl::canZoomOut() {
    return true;
}

/**
 * @see CanvasControl::OnToolChanged
 */
Void HistogramControl::OnToolChanged( FitsLiberator::Modelling::ToolTypeFunction tool ) {
    super::OnToolChanged( tool );

    switch( tool ) {
        case kFITSToolFunctionZoomIn:
        case kFITSToolFunctionZoomOut:
            histogramTracker->setTrackerSink( histogramSelectionTrackerSink );
            break;
        default:
            histogramTracker->setTrackerSink( histogramMoveTrackerSink );
    }       
}

// ------------------------------------------------------------------------------------------------
// Implementation of HistogramSelectionTrackerSink
// ------------------------------------------------------------------------------------------------
HistogramControl::HistogramSelectionTrackerSink::HistogramSelectionTrackerSink( HWND hWnd, const RECT& activeRegion )
  : SelectionTrackerSink( hWnd, activeRegion ) {

}

/**
 * @see SelectionTrackerSink::adjustSelection
 */
Void HistogramControl::HistogramSelectionTrackerSink::adjustSelection() {
    SelectionTrackerSink::adjustSelection();
    selectionRegion.top = activeRegion.top;
    selectionRegion.bottom = activeRegion.bottom;
}
