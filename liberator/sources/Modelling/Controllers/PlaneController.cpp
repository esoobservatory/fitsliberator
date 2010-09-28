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
#include "PlaneController.h"

using namespace FitsLiberator::Engine;
using namespace FitsLiberator::Modelling;

/**
 * Default constructor.
*/
PlaneController::PlaneController( PlaneModel& m, StatisticsController& sc, HistogramController& hc, PreviewController& pc, AccumulatingChangeManager* chman )
  : ACMController( chman ), model( m ), statisticsController( sc ), histogramController( hc ), previewController( pc ) {
}



/**
Toggles the flipping of the image.
*/
Void PlaneController::toggleFlip()
{
	if ( model.getFlipped().flipped )
		model.setFlipped( false );
	else
		model.setFlipped( true );
}


/**
Returns the current plane
*/
FitsLiberator::Engine::Plane& PlaneController::getPlane()
{
	return this->model.getPlane();
}

/**
 *
 */
Void PlaneController::toggleBitDepth() {
	switch( model.getImportBitDepth() ) {
		case channel8:
			setBitDepth( channel16 );
			break;
			
		case channel16:
			setBitDepth( channel8 );
			break;
	}
}

Void PlaneController::resetFlip()
{
	model.resetFlip();	
}

/** 
 * Sets the selected bit depth option.
 * @param setting Selected bit depth.
 */
Void PlaneController::setBitDepth( ChannelSettings setting ) {
    model.setImportBitDepth( setting );
    SendNotifications();
}

/**
 * Sets the undefined option.
 * @param setting Selected undefined option.
 */
Void PlaneController::setUndefined( UndefinedSettings setting ) {
    model.setImportUndefinedOption( setting );
    SendNotifications();
}

/**
 *
 */
Void PlaneController::toggleUndefinedOption() {
	switch( model.getImportUndefinedOption() ) {
		case undefinedBlack:
			setUndefined( undefinedTransparent );
			break;
			
		case undefinedTransparent:
            setUndefined( undefinedBlack );
			break;
	}
}