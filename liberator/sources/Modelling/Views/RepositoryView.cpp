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
#include "Environment.h"
#include "RepositoryView.h"
#include "TextUtils.h"
#include "Resources.h"

using namespace FitsLiberator;
using namespace FitsLiberator::Modelling;

/** Maps from StretchFunction to the corresponding string resource. */
const int funtionStringMap[] = {
	IDS_STRETCH_LINEAR, IDS_STRETCH_LOG, IDS_STRETCH_SQRT, IDS_STRETCH_LOGSQRT, 
	IDS_STRETCH_LOGLOG, IDS_STRETCH_CUBEROOT, IDS_STRETCH_ASINH, 
	IDS_STRETCH_ROOT4, IDS_STRETCH_ROOT5, IDS_STRETCH_ASINHASINH, 
	IDS_STRETCH_ASINHSQRT, IDS_STRETCH_POW2, IDS_STRETCH_POW3, IDS_STRETCH_POW4, IDS_STRETCH_POW5
};

RepositoryView::RepositoryView( RepositoryModel& m, RepositoryController& c, PlaneModel& p, StretchModel& s, HistogramModel& h )
  : model( m ),
    controller( c ),
    planeModel( p ),
    stretchModel( s ),
    histogramModel( h ) {
    
    model.Attach( this );
    planeModel.Attach( this );
    stretchModel.Attach( this );
    histogramModel.Attach( this );
}

RepositoryView::~RepositoryView() {
    histogramModel.Detach( this );
    stretchModel.Detach( this );
    planeModel.Detach( this );
    model.Detach( this );
}

Void RepositoryView::Update( Models* m ) {
	Models::iterator it;
	
	for( it = m->begin(); it != m->end(); it++ ) {
		if( &model == *it ) {
			updateListing();
		}
        else if( &planeModel == *it ) {
            // Parse the headers of the new image
			controller.parseHeaders( planeModel.getImageIndex(), planeModel.shouldFlip(), planeModel.getFlipped().flipped );
        }
        else if( &stretchModel == *it ) {
            // Update the stretch information
			StretchFunction function = stretchModel.getFunction();

			assert(function >= stretchLinear && function < stretchNoStretch);
            controller.parseValue( model.findKeyword( "FL.StretchFunction" ), Environment::getString( funtionStringMap[function] ) );
            controller.parseValue( model.findKeyword( "FL.BackgroundLevel" ), TextUtils::doubleToString( stretchModel.getBackground(), kFITSMaxPrecision ) );
            controller.parseValue( model.findKeyword( "FL.PeakLevel" ), TextUtils::doubleToString( stretchModel.getPeakLevel(), kFITSMaxPrecision ) );
            controller.parseValue( model.findKeyword( "FL.ScaledBackgroundLevel" ), TextUtils::doubleToString( stretchModel.getScaleBackground(), kFITSMaxPrecision ) );
            controller.parseValue( model.findKeyword( "FL.ScaledPeakLevel" ), TextUtils::doubleToString( stretchModel.getRescaleFactor(), kFITSMaxPrecision ) );

        }
        else if( &histogramModel == *it ) {
            controller.parseValue( model.findKeyword( "FL.BlackLevel" ), TextUtils::doubleToString( histogramModel.getBlackLevel(), kFITSMaxPrecision ) );
            controller.parseValue( model.findKeyword( "FL.WhiteLevel" ), TextUtils::doubleToString( histogramModel.getWhiteLevel(), kFITSMaxPrecision ) );        
        }
	}    
}