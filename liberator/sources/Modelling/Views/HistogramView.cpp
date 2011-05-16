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
#include "HistogramView.h"

using FitsLiberator::Color;
using namespace FitsLiberator::Modelling;


//-----------------------------------------------------------------------------
//	Histogram markers:
//

//
//-----------------------------------------------------------------------------

HistogramView::HistogramView(
    HistogramModel& m, ToolModel& tm, StatisticsModel& sm, StretchModel& sf, OptionsModel& om,
    HistogramController& hc, FlowController& fc ) 
  : model( m ), toolModel( tm ), statistics(sm), stretch(sf), options(om),
    histoController( hc ), flowController( fc )
{
	model.Attach( this );
	toolModel.Attach( this );
    statistics.Attach( this );
    stretch.Attach( this );
    options.Attach( this );
    
    markerColors[0] = Color(0xff, 0x00, 0x00);  // Zero
    markerColors[1] = Color(0x00, 0xff, 0x00);  // Background
    markerColors[2] = Color(0xff, 0x00, 0xff);  // Peak
    markerColors[3] = Color(0x00, 0x00, 0xff);  // Mean
    markerColors[4] = Color(0x00, 0x00, 0xff);  // Stretched Mean
    markerColors[5] = Color(0x00, 0x00, 0xff);  // Scaled Mean
    markerColors[6] = Color(0x80, 0x00, 0x80);  // Median
    markerColors[7] = Color(0x80, 0x00, 0x80);  // Stretched Median
    markerColors[8] = Color(0x80, 0x00, 0x80);  // Scaled Median
    markerColors[9] = Color(0x00, 0xff, 0x00);  // Scaled Peak

    updateMarkers();
//    fill(markerPositions, markerPositions + MarkerCount, -1);
}

HistogramView::~HistogramView() {
	model.Detach( this );
	toolModel.Detach( this );
    statistics.Detach( this );
    stretch.Detach( this );
    options.Detach( this );
}
			
			
Void HistogramView::Update( Models * m )
{

	Models::iterator it;
	
	for( it = m->begin(); it != m->end(); it++ )
	{
		if( &model == *it )
		{
            updateMarkers();
			drawHistogram();
			setHistogramMax( this->model.getRangeMax() );
			setHistogramMin( this->model.getRangeMin() );
			this->setBlackLevel( this->model.getCurrentBlackLevel() );
			this->setWhiteLevel( this->model.getCurrentWhiteLevel() );
		} else if( &toolModel == *it ) {
			setCurrentToolFunction( this->toolModel.getCurrentFunction() );
        } else if( &statistics == *it ) {
            updateMarkers();
            drawHistogram();
        } else if( &options == *it ) {
            updateMarkers();
            drawHistogram();
        }
	}
}

void HistogramView::updateMarkers() {
    // Get the values to display
	double values[MarkerCount] = {
		0.0, stretch.getBackground(), stretch.getPeakLevel(),
		statistics.getRealMean(), statistics.getStretchMean(), 
		statistics.getScaledMean(), statistics.getRealMedian(), 
		statistics.getStretchMedian(), statistics.getScaledMedian(),
		stretch.getRescaleFactor()
	};
		
    unsigned int markers = options.HistogramMarkers();
    markers >>= 1;      // Skip the grid

    for(int i = 0; i < MarkerCount; ++i) {
        markerPositions[i] = (markers & 1) 
            ? model.getPositionOf(values[i])
            : -1;
        markers >>= 1;
    }
}
