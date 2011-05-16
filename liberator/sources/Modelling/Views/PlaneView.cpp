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
#include <sstream>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include "Environment.h"
#include "PlaneView.h"

using namespace FitsLiberator::Engine;
using namespace FitsLiberator::Modelling;

PlaneView::PlaneView( PlaneModel& m, FlowController& fc, PlaneController& c )
: model( m ), planeController(c), flowController( fc )
{
	model.Attach( this );

	this->entries = model.getEntries();
}
/**
 *
 */
PlaneView::~PlaneView() {
	model.Detach( this );
}

/**
 *
 */
Void PlaneView::Update( Models* m ) {
	Models::iterator it;
	
	for( it = m->begin(); it != m->end(); it++ ) {
		if( &model == *it ) {
			updateSelectorIndex( model.getEntryIndex() );

            // Update the bitdepth radio buttons
            FitsLiberator::Engine::ChannelSettings bitDepth = model.getImportBitDepth();
            updateBitDepth( channel8, (bitDepth == channel8), Environment::supportsBitDepth( channel8 ) );
            updateBitDepth( channel16, (bitDepth == channel16), Environment::supportsBitDepth( channel16 ) );
            updateBitDepth( channel32, (bitDepth == channel32), Environment::supportsBitDepth( channel32 ) );

            // Update the undefined radio buttons
            FitsLiberator::Engine::UndefinedSettings undefined = model.getImportUndefinedOption();
            if( undefined == undefinedTransparent && !Environment::supportsTransparent( bitDepth ) )
                undefined = undefinedBlack;

            updateUndefinedOption( undefinedBlack, (undefined == undefinedBlack), true );   // Black is always supported
            updateUndefinedOption( undefinedTransparent, (undefined == undefinedTransparent), Environment::supportsTransparent( bitDepth ) );

			updateSize( model.getWidth(), model.getHeight() );

			updateFlip( model.getFlipped().flipped );
		}
	}
}

/**
 *
 */
Int PlaneView::getEntryCount() const {
    return entries.size();
}

/**
 *
 *
 * @param index
 */
String PlaneView::getEntry( Int index ) const {
    Plane p = entries[index];

    std::ostringstream s;
    s << "Image " << p.imageIndex + 1 << ", Plane " << p.planeIndex + 1;
    
    return s.str();
}

/**
 * Map the index to imageIndex and planeIndex
 *
 * @param index
 */
Void PlaneView::selectorChanged( Int index ) {
	flowController.imageChanged( entries[index].imageIndex, entries[index].planeIndex, false );
}