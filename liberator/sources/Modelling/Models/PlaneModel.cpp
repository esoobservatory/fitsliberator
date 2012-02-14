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
#include "PlaneModel.h"

using namespace FitsLiberator::Modelling;
using namespace FitsLiberator::Engine;
using namespace FitsLiberator;

/**
 * Default constructor
 */
PlaneModel::PlaneModel( ChangeManager* chman, ImageReader* r )
: Model( chman ), 
  reader( r ), 
  imageIndex( -1 ), 
  planeIndex( -1 ),
  undefinedOption( kFITSDefaultUndefined ) {
  	updateModel( 0, 0 );
  	//setImportBitDepth( kFITSDefaultBitDepth );
	
}

/**
 *
 */
FitsLiberator::Engine::Plane& PlaneModel::getPlane()
{
	this->plane.imageIndex = this->getImageIndex();
	this->plane.planeIndex = this->getPlaneIndex();
	return this->plane;
}

Void PlaneModel::resetFlip()
{
	updateFlip();
	Notify();
}

/**
	Work out whether to flip or not.
	If CD-matrix is present then:
	CD = |CD1_1		CD1_2|
		 |CD2_1		CD2_2|
	if sign( CD1_1 ) != sign( CD2_2 ) AND sign(CD1_2) == sign(CD2_1) then FLIP

	if sign( CD1_1 ) == sign( CD2_2 ) AND sign(CD1_2) != sign(CD2_1) then DO NOT FLIP

	if CDELT1 and CDELT2 are present then

	if sign( CDELT1 ) != sign( CDELT2 ) then FLIP

	if sign( CDELT1 ) == sign( CDELT2 ) then DO NOT FLIP
*/
Bool PlaneModel::shouldFlip() const
{
	const ImageCube* cube = (*reader)[imageIndex];
	assert(cube != 0);

	return (cube->RowOrder() == ImageCube::BottomUp);
}

Void PlaneModel::updateFlip()
{
	const ImageCube* cube = (*reader)[imageIndex];
	assert(cube != 0);

	flip.flipped = (cube->RowOrder() == ImageCube::BottomUp);
}

Void PlaneModel::setFlipped( Bool fl )
{
	if ( fl != flip.flipped )
	{
		flip.flipped = fl;
		Notify();
	}
}

const Flip& PlaneModel::getFlipped()
{
	return flip;
}

/**
 *
 */
Int PlaneModel::getImageIndex() const {
    return this->imageIndex;
}

/**
 *
 */
Int PlaneModel::getPlaneIndex() const {
    return this->planeIndex;
}

/**
 *
 */
Int PlaneModel::getWidth() const {
    return this->width;
}

/**
 *
 */
Int PlaneModel::getHeight() const {
    return this->height;
}

/**
 *
 */
Void PlaneModel::setImportBitDepth( ChannelSettings depth ) {
	switch( depth ) {
		case channel8:
			bitDepth = depth;
			break;
			
		case channel16:
			if( Environment::supportsBitDepth( channel16 ) ) {
				bitDepth = depth;		
			} else {
				bitDepth = channel8;
			}
			break;
			
		case channel32:
			if( Environment::supportsBitDepth( channel32 ) ) {
				bitDepth = depth;
			} else {
				if( Environment::supportsBitDepth( channel16 ) ) {
					bitDepth = channel16;
				} else {
					bitDepth = channel8;
				}
			}
			break;
	}
	
    Notify();
}

/**
 *
 */
Void PlaneModel::setImportUndefinedOption( UndefinedSettings opt ) {
	undefinedOption = opt;
    Notify();
}

/**
 *
 */
ChannelSettings PlaneModel::getImportBitDepth( ) {
	return bitDepth;
}

/**
 *
 */
UndefinedSettings PlaneModel::getImportUndefinedOption( ) {
	return undefinedOption;
}

/**
 *
 */
Bool PlaneModel::updateModel( Int imageIndex, Int planeIndex ) {
	if( this->imageIndex != imageIndex || this->planeIndex != planeIndex ) {
		this->imageIndex = imageIndex;
	    this->planeIndex = planeIndex;
	    
		this->doUpdateModel();
	    
	    return true;
	} else {
		return false;
	}
}

Void PlaneModel::doUpdateModel()
{	
	const ImageCube* cube = (*reader)[plane.imageIndex];
	assert(cube != 0);
	
	this->width = cube->Width();
	this->height = cube->Height();
	
	if( ::abs(cube->Format()) <= 8 ) {
		setImportBitDepth(channel8);
	} else if( ::abs(cube->Format()) <= 16 ) {
		setImportBitDepth(channel16);
	} else {
		setImportBitDepth(channel32);
	}

	updateFlip();

	Notify();
}

Void PlaneModel::updateReader( FitsLiberator::Engine::ImageReader* r )
{
	this->reader = r;
	//important to re-initialize the settings
	this->doUpdateModel();
}

/**
 *
 */
Int PlaneModel::getEntryIndex() {
	Plane plane;
    Int index = 0;

	Int imageCount = reader->size();
    
    for( plane.imageIndex = 0; plane.imageIndex < imageCount; plane.imageIndex++ ) {
		const ImageCube* cube = (*reader)[plane.imageIndex];
		Int planeCount = cube->Planes();
		for( plane.planeIndex = 0; plane.planeIndex < planeCount; plane.planeIndex++ ) {
    		index++;
        	
    		if( plane.imageIndex == imageIndex && plane.planeIndex == planeIndex ) {
    			return index-1;	
    		}
		}
    }
    
    return 0;
}

/**
 *
 */
Vector<Plane> PlaneModel::getEntries() {
	Vector<Plane> entries;

	//
	// Enumerate the possible image/plane combinations
    Plane plane;

	Int imageCount = reader->size();
    for( plane.imageIndex = 0; plane.imageIndex < imageCount; plane.imageIndex++ ) {
		const ImageCube* cube = (*reader)[plane.imageIndex];
		Int planeCount = cube->Planes();
        for( plane.planeIndex = 0; plane.planeIndex < planeCount; plane.planeIndex++ )
            entries.push_back( plane );
    }
    
    return entries;
}