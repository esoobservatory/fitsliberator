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
#include "SliderControl.h"
#include "ResourceManager.h"

using namespace Gdiplus;
using namespace FitsLiberator;
using namespace FitsLiberator::Modelling;
using namespace FitsLiberator::Windows;

const Int SliderControl::thumbHalfWidth   = 4;
const UInt SliderControl::preferredHeight = 16;

/**
 * Default constructor
 */
SliderControl::SliderControl( HistogramModel& m, HistogramController& c, FlowController& fc ) 
  : model( m ),
    controller( c ),
	flowController( fc )
{
    blackLevelImage = ResourceManager::LoadIcon( IDI_HISTOGRAM_SLIDER_BLACK );
    whiteLevelImage = ResourceManager::LoadIcon( IDI_HISTOGRAM_SLIDER_WHITE );
    disabledImage   = ResourceManager::LoadIcon( IDI_HISTOGRAM_SLIDER_DISABLED );
    separatorPen    = (HGDIOBJ)::CreatePen( PS_SOLID, 1, kFITSHistogramSliderColor );
    borderPen       = (HGDIOBJ)::CreatePen( PS_SOLID, 1, kFITSHistogramColor );
}

/**
 * Destructor, frees the icons used
 */
SliderControl::~SliderControl() {
    ::DestroyIcon( blackLevelImage );
    ::DestroyIcon( whiteLevelImage );
    ::DestroyIcon( disabledImage );
    ::DeleteObject( separatorPen );
    ::DeleteObject( borderPen );

    delete whiteLevelTracker;
    delete blackLevelTracker;

    delete whiteLevelTrackerSink;
    delete blackLevelTrackerSink;
}

/**
 * Creates the control.
 * @param hWndParent Parent window.
 * @param bounds Boundaries of the control.
 * @return True on success, false on failure.
 */
Bool SliderControl::Create( HWND hWndParent, const LPRECT bounds ) {
    return Control::Create( hWndParent, bounds, WS_VISIBLE | WS_CHILD, 0 );
}

Void SliderControl::OnCreate( HWND hWnd ) {
    CanvasControl::OnCreate( hWnd );

    RECT rc;
    ::GetClientRect( hWnd, &rc );
    
    ::SetRect( &blackLevelBoundaries, rc.left, rc.top + 1, rc.left + 2*thumbHalfWidth + 1, rc.bottom );
    ::SetRect( &whiteLevelBoundaries, rc.right - (2*thumbHalfWidth + 1), rc.top + 1, rc.right, rc.bottom );


    blackLevelTrackerSink = new MoveTrackerSink();
    whiteLevelTrackerSink = new MoveTrackerSink();

    blackLevelTracker = new MouseTracker( blackLevelTrackerSink, blackLevelBoundaries );
    whiteLevelTracker = new MouseTracker( whiteLevelTrackerSink, whiteLevelBoundaries );
}

/**
 * @see Window::OnMouseDown.
 */
Void SliderControl::OnMouseDown( Int button, Int x, Int y ) {
    super::OnMouseDown( button, x, y );

    Int closestSlider;

    // The user pressed the mouse in 
    if( 0 == (closestSlider= model.getClosestSlider( x )) ) {
        // Black is closest
        blackLevelBoundaries.left = x - thumbHalfWidth;
        blackLevelBoundaries.right = x + thumbHalfWidth + 1;
    }
    else {
        // White is closest
        whiteLevelBoundaries.left = x - thumbHalfWidth;
        whiteLevelBoundaries.right = x + thumbHalfWidth + 1;
    }
    blackLevelTracker->setActiveRegion( blackLevelBoundaries );
    whiteLevelTracker->setActiveRegion( whiteLevelBoundaries );

    if( MouseActionTracking == blackLevelTracker->OnMouseDown( hWnd, button, x, y ) && 0 == closestSlider ) {
        flowController.moveBlackLevelSlider( x );
        flowController.applyLevels();
    }
    if( MouseActionTracking == whiteLevelTracker->OnMouseDown( hWnd, button, x, y ) && 1 == closestSlider ) {
        flowController.moveWhiteLevelSlider( x );
        flowController.applyLevels();
    }
}

/**
 * @see Window::OnMouseMove
 */
Void SliderControl::OnMouseMove( Int button, Int x, Int y ) {
    super::OnMouseUp( button, x, y );

    if( MouseActionTracking == blackLevelTracker->OnMouseMove( hWnd, button, x, y ) )
        flowController.moveBlackLevelSlider( blackLevelTrackerSink->getLocation().x );
    if( MouseActionTracking == whiteLevelTracker->OnMouseMove( hWnd, button, x, y ) )
        flowController.moveWhiteLevelSlider( whiteLevelTrackerSink->getLocation().x );
}

/**
 * @see Window::OnMouseUp
 */
Void SliderControl::OnMouseUp( Int button, Int x, Int y ) {
    /*if( MouseActionTracking == blackLevelTracker->OnMouseUp( hWnd, button, x, y ) )
        controller.applyLevels();
    if( MouseActionTracking == whiteLevelTracker->OnMouseUp( hWnd, button, x, y ) )
        controller.applyLevels();*/
	blackLevelTracker->OnMouseUp( hWnd, button, x, y );
	whiteLevelTracker->OnMouseUp( hWnd, button, x, y );
	flowController.applyLevels();
}

/**
 * Repaints the control, @see CanvasControl::OnRefresh
 */
Void SliderControl::OnRefresh( HDC hDC ) {
    FitsLiberator::Rectangle bounds = getClient();
    HGDIOBJ oldPen;
    HGDIOBJ oldBrush;

    //
    // Paint the background
    oldPen = ::SelectObject( hDC, borderPen );
    oldBrush = ::SelectObject( hDC, (HGDIOBJ)::GetSysColorBrush( COLOR_WINDOW ) );
    ::Rectangle( hDC, bounds.left, bounds.top, bounds.right, bounds.bottom );

    ::SelectObject( hDC, separatorPen );
    ::MoveToEx( hDC, bounds.left, bounds.top, NULL );
    ::LineTo( hDC, bounds.right, bounds.top );

    //
    // Paint the sliders
    Int pos = 0;
    HICON image = disabledImage;
    
    // Black slider
    pos = model.getBlackSliderPos();
    if( pos <= bounds.left ) {
        blackLevelBoundaries.left = -thumbHalfWidth;
        blackLevelBoundaries.right = thumbHalfWidth + 1;
        image = disabledImage;
    }
    else if( pos >= bounds.right ) {
        blackLevelBoundaries.left = bounds.right - thumbHalfWidth - 1;
        blackLevelBoundaries.right = bounds.right + thumbHalfWidth;
        image = disabledImage;
    }
    else {
        blackLevelBoundaries.left = pos - thumbHalfWidth;
        blackLevelBoundaries.right = pos + thumbHalfWidth + 1;
        image = blackLevelImage;
    }
    ::DrawIconEx( hDC, blackLevelBoundaries.left, blackLevelBoundaries.top, image, 0, 0, 0, NULL, DI_NORMAL );

    // white slider
    pos = model.getWhiteSliderPos();
    if( pos <= bounds.left ) {
        whiteLevelBoundaries.left = -thumbHalfWidth;
        whiteLevelBoundaries.right = thumbHalfWidth + 1;
        image = disabledImage;
    }
    else if( pos >= bounds.right - 1 ) {
        whiteLevelBoundaries.left = bounds.right - thumbHalfWidth - 1;
        whiteLevelBoundaries.right = bounds.right + thumbHalfWidth;
        image = disabledImage;
    }
    else {
        whiteLevelBoundaries.left = pos - thumbHalfWidth;
        whiteLevelBoundaries.right = pos + thumbHalfWidth + 1;
        image = whiteLevelImage;
    }
    ::DrawIconEx( hDC, whiteLevelBoundaries.left, whiteLevelBoundaries.top, image, 0, 0, 0, NULL, DI_NORMAL );
}

/**
 * Required by CanvasControl
 */
Bool SliderControl::canZoomIn() {
    return false;
}

/**
 * Required by CanvasControl
 */
Bool  SliderControl::canZoomOut() {
    return false;
}
