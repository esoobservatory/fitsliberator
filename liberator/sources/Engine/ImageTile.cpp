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
#include "ImageTile.h"

using namespace FitsLiberator::Engine;

using namespace FitsLiberator;


/*-------------------------------------
Code for the implementation of ImageTile
-------------------------------------*/
ImageTile::ImageTile()
{
	rawPixels = NULL;
	stretchedPixels = NULL;
	nullPixels = NULL;

	locked = false;
	stretched = false;
	stretch.function = stretchNoStretch;



	effLeft = -1;
	effRight = -1;
	effTop = -1;
	effBottom = -1;
}

Bool operator==( const ImageTile& lhs, const ImageTile& rhs )
{
	return ( lhs.x == rhs.x &&
		 lhs.y == rhs.y &&
		 lhs.height == rhs.height &&
		 lhs.width == rhs.width );
}

Bool ImageTile::isAllocated()
{
	if ( rawPixels != NULL && stretchedPixels != NULL && nullPixels != NULL )
		return true;

	return false;

}

Void ImageTile::deallocatePixels()
{
	if ( isAllocated() )
	{
		if ( rawPixels != NULL ) delete[] reinterpret_cast<Byte*>(rawPixels);
		if ( stretchedPixels != NULL ) delete[] stretchedPixels;
		if ( nullPixels != NULL ) delete[] nullPixels;

		rawPixels = NULL;
		stretchedPixels = NULL;
		nullPixels = NULL;
		stretched = false;
		stretch.function = stretchNoStretch;


		if ( this->locked )
		{
			throw Exception("Tried to deallocate locked tile");
		}
	}
	
}


/**
Returns true if the current effective bounds are realistic
*/
Bool ImageTile::isCurrent()
{
	if ( effLeft > -1 && effRight > -1 && effTop > -1 && effBottom > - 1 )
		return true;

	return false;
}

/**
	Tries to allocate the current set up pixels with the specified bit dept
	If that is not possible it will return ImageTile::AllocErr and make sure that
	the contained buffers are cleaned appropriately
*/
Int ImageTile::allocatePixels( Int bitDepth)
{
	if ( isAllocated() == false && width > 0 && height > 0 && bitDepth > 0 )
	{
		try
		{
			rawPixels = (Void*)new Byte[width*height*bitDepth];
			nullPixels = new char [width * height];
			stretchedPixels = new Double [width * height];
			
		}
		catch( std::bad_alloc ba )
		{
			//if, for some reason, not enough memory could be allocated..
			
			if ( rawPixels != NULL ) delete[] reinterpret_cast<Byte*>(rawPixels);
			rawPixels = NULL;

			if ( nullPixels != NULL ) delete[] nullPixels;
			nullPixels = NULL;

			if ( stretchedPixels != NULL ) delete[] stretchedPixels;
			stretchedPixels = NULL;
						
			return ImageTile::AllocErr;
		}
	}
	
	return ImageTile::AllocOk;
}

const Rectangle ImageTile::getBounds()
{
	bounds.left = x;
	bounds.right = x + width;// - 1;
	bounds.top = y;
	bounds.bottom = y + height;// - 1;
	return bounds;
}

const Rectangle& ImageTile::getEffBounds()
{
	effBounds.left = effLeft;
	effBounds.right = effRight;
	effBounds.top = effTop;
	effBounds.bottom = effBottom;
	return effBounds;
}

Void ImageTile::setX( Int x )
{
	this->x = x;
	effLeft = x;
}
			
Void ImageTile::setY( Int y )
{
	this->y = y;
	effTop = y;
}

Void ImageTile::setWidth( Int w )
{
	this->width = w;
	effRight = x + w;
}

Void ImageTile::setHeight( Int h )
{
	this->height = h;
	effBottom = y + h;
}


ImageTile::~ImageTile()
{
	deallocatePixels();
}