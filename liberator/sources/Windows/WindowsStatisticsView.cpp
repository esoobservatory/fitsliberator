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
#include "WindowsStatisticsView.h"
#include "Resources.h"
#include "TextUtils.h"


using namespace FitsLiberator::Modelling;
using namespace FitsLiberator::Windows;

//---------------------------------------------------------------------------------------
// Constructors and destructors
//---------------------------------------------------------------------------------------

WindowsStatisticsView::WindowsStatisticsView( StatisticsModel& m, GlobalSettingsModel& gm ) : StatisticsView( m, gm ) {

}

//---------------------------------------------------------------------------------------
// EventSink interface
//---------------------------------------------------------------------------------------
Void WindowsStatisticsView::OnInit( Dialog* owner, Int ID ) {
    if( IDD_PREVIEW == ID ) {
        this->owner = owner;

		hWndRealMin = owner->getItem( IDC_STATISTICS_MIN_REAL );	
		hWndRealMax = owner->getItem( IDC_STATISTICS_MAX_REAL );
		hWndRealMean = owner->getItem( IDC_STATISTICS_MEAN_REAL );	
		hWndRealMedian = owner->getItem( IDC_STATISTICS_MEDIAN_REAL );	
		hWndRealStdDev = owner->getItem( IDC_STATISTICS_STDDEV_REAL );	
		hWndStretchedMin = owner->getItem( IDC_STATISTICS_MIN_STRETCHED );	
		hWndStretchedMax = owner->getItem( IDC_STATISTICS_MAX_STRETCHED );	
		hWndStretchedMean = owner->getItem( IDC_STATISTICS_MEAN_STRETCHED );	
		hWndStretchedMedian = owner->getItem( IDC_STATISTICS_MEDIAN_STRETCHED );	
		hWndStretchedStdDev = owner->getItem( IDC_STATISTICS_STDDEV_STRETCHED );

			updateRealValues( 	model.getRealMin(), 
								model.getRealMax(), 
								model.getRealMean(), 
								model.getRealMedian(), 
								model.getRealSTDEV()
							);
							
			updateStretchValues(	model.getStretchMin(), 
									model.getStretchMax(), 
									model.getStretchMean(), 
									model.getStretchMedian(), 
									model.getStretchSTDEV() 
								);
    }
}

Void WindowsStatisticsView::OnCommand( WPARAM wParam, LPARAM lParam ) {
    // No command events for this view
}

Void WindowsStatisticsView::OnNotify( Int idCtrl, LPNMHDR pnmh ) {
    // No notify events for this view
}

Void WindowsStatisticsView::OnScroll( WPARAM wParam, LPARAM lParam ) {
    // No scroll events for this view
}

//---------------------------------------------------------------------------------------
// StatisticsView implementation
//---------------------------------------------------------------------------------------

Void WindowsStatisticsView::updateScaledValues( Double min, Double max, Double mean, Double median, Double stddev ) 
{
	Window::setValue( hWndStretchedMin, min );
	Window::setValue( hWndStretchedMax, max );
	Window::setValue( hWndStretchedMean, mean );
	Window::setValue( hWndStretchedMedian, median );
	Window::setValue( hWndStretchedStdDev, stddev );
}

Void WindowsStatisticsView::updateText( String& s )
{
    Window::setText( owner->getItem( IDC_STATISTICS_MODE ), s );
}

Void WindowsStatisticsView::updateRealValues( Double min, Double max, Double mean, Double median, Double stddev )
{
	Window::setValue( hWndRealMin, min );
	Window::setValue( hWndRealMax, max );
	Window::setValue( hWndRealMean, mean );
	Window::setValue( hWndRealMedian, median );
	Window::setValue( hWndRealStdDev, stddev );
}

Void WindowsStatisticsView::updateStretchValues( Double min, Double max, Double mean, Double median, Double stddev )
{
	Window::setValue( hWndStretchedMin, min );
	Window::setValue( hWndStretchedMax, max );
	Window::setValue( hWndStretchedMean, mean );
	Window::setValue( hWndStretchedMedian, median );
	Window::setValue( hWndStretchedStdDev, stddev );
}

