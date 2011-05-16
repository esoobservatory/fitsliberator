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
#include "TilePusher.h"

using namespace FitsLiberator::Engine;

/**
Inits the tilepusher instance with the tiles array
*/
TilePusher::TilePusher( ImageTile* tls, UInt nTls )
{
	this->tiles = tls;
	this->nTiles = nTls;

	if ( nTiles > 0 )
	{
		//initialize and default to false
		tilesReturned = new Bool[nTiles];
		std::fill( tilesReturned, tilesReturned+nTiles, false );
	}
	else
		tilesReturned = NULL;
}

TilePusher::~TilePusher()
{
	//clean up
	if ( tilesReturned != NULL )
		delete[] tilesReturned;
}


/**
Inits the session meening that the tilesReturned array is reset
*/
Void TilePusher::initSession()
{
	if ( nTiles > 0 && tilesReturned != NULL )
		std::fill( tilesReturned, tilesReturned+nTiles, false );
	else
		throw Exception("Could not init session since the object was not correctly initialized");
}

/**
Returns the next tile in line
*/
ImageTile* TilePusher::getNextTile()
{
	//first loop over the tilesReturned array
	for ( UInt i = 0; i < nTiles; i++ )
	{
		//if the tile is allocated already but not returned yet.
		if ( !(tilesReturned[i]) && tiles[i].isAllocated() )
		{
			tilesReturned[i] = true;
			return &tiles[i];
		}
	}

	/*if a tile could not be returned the first tile allocated and returned 
	is deallocated and the next comming tile not returned is returned for allocation and usage
	*/
	for ( UInt i = 0; i < nTiles; i++ )
	{
		if ( tiles[i].isAllocated() && tilesReturned[i] )
		{
			tiles[i].deallocatePixels();
			//escape the loop
			break;
		}
	}

	for ( UInt i = 0; i < nTiles; i++ )
	{
		//the check for isAllocated is probably not necessary but doesn't hurt us
		if ( !(tilesReturned[i]) && !(tiles[i].isAllocated()) )
		{
			tilesReturned[i] = true;
			return &tiles[i];
		}
	}
	return NULL;
}

/**
@returns true if there are not-returned tiles, false if not
*/
Bool TilePusher::sessionHasMoreTiles()
{
	for ( UInt i = 0; i < nTiles; i++ )
	{
		if ( !(tilesReturned[i]) ) return true;
	}
	return false;
}