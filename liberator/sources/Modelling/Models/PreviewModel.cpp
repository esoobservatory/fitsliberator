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
#include "PreviewModel.h"
#include <math.h>

using namespace FitsLiberator::Modelling;
using namespace FitsLiberator::Engine;
using namespace FitsLiberator::Caching;

//-------------------------------------------------------------------------------
//	PreviewImage implementation
//-------------------------------------------------------------------------------

PreviewImage::PreviewImage(UInt w, UInt h)
{
	rawPixels		= new Double[w * h];
	previewPixels	= new Byte[w * h];
	
	whiteClippedMap	= new Byte[w * h];
	blackClippedMap = new Byte[w * h];
	nullMap			= new Byte[w * h];
	this->size = FitsLiberator::Size(w, h);

	for ( Int i = 0; i < w * h; i++ )
	{
		rawPixels[i] = 0.;
		previewPixels[i] = 0;
		whiteClippedMap[i] = 0;
		blackClippedMap[i] = 0;
		nullMap[i] = 0;
	}

}

PreviewImage::~PreviewImage()
{
	delete[] this->rawPixels;
	delete[] this->previewPixels;
	delete[] this->whiteClippedMap;
	delete[] this->blackClippedMap;
	delete[] this->nullMap;
}

//-------------------------------------------------------------------------------
//	PreviewModel implementation
//-------------------------------------------------------------------------------

PreviewModel::PreviewModel( ChangeManager * chman, GlobalSettingsModel& gsm)
: Model( chman ), globalSettingsModel( gsm )
{
	//we initialize the zoom values.
	this->zoomIndex	= 0;
	this->zooms = 24;
	
	zoomFactors = new Double[this->zooms];
	std::fill( zoomFactors, zoomFactors + this->zooms, 0. );
	
	//contains the zoom factors except for the "fit in preview" factor.
	staticZoomFactors = new Double[this->zooms-1];

	staticZoomFactors[0] = 16.0;
	staticZoomFactors[1] = 12.0;
	staticZoomFactors[2] = 8.0;
	staticZoomFactors[3] = 7.0;
	staticZoomFactors[4] = 6.0;
	staticZoomFactors[5] = 5.0;
	staticZoomFactors[6] = 4.0;
	staticZoomFactors[7] = 3.0;
	staticZoomFactors[8] = 2.0;
	staticZoomFactors[9] = 1.0;
	staticZoomFactors[10] = 2.0 / 3.0;	//66%
	staticZoomFactors[11] = 1.0 / 2.0;	//50%
	staticZoomFactors[12] = 1.0 / 3.0;	//33%
	staticZoomFactors[13] = 1.0 / 4.0;	//25%
	staticZoomFactors[14] = 1.0 / 6.0;	//16.6%
	staticZoomFactors[15] = 1.0 / 8.0;	//12.5%
	staticZoomFactors[16] = 1.0 / 12.0;	//8%
	staticZoomFactors[17] = 1.0 / 16.0;	//6.5%
	staticZoomFactors[18] = 1.0 / 20.0;	//5%
	staticZoomFactors[19] = 1.0 / 25.0;	//4%
	staticZoomFactors[20] = 3.0 / 100.0; //3%
	staticZoomFactors[21] = 2.0 / 100.0;//2%
	staticZoomFactors[22] = 1.0 / 100.0;//1%

	this->previewImage = NULL;
	currentAnchor	= new FitsLiberator::Point(0,0);


	if ( kFITSDoCache )
		this->cacheHandler = new PreviewCacheHandler();
	else
		this->cacheHandler = NULL;

    this->busy = false;
}

PreviewModel::~PreviewModel()
{
	if ( previewImage != NULL )
		delete this->previewImage;

	delete this->currentAnchor;
	delete[] this->zoomFactors;
	delete[] this->staticZoomFactors;
	if ( this->cacheHandler != NULL )
		delete this->cacheHandler;
}

//cache implementation

Bool PreviewModel::useCache( Stretch& stretch, Plane& plane, FitsLiberator::Point& anch,
							Double zoom, Double* pixels, Int size, Bool flip )
{
	if ( kFITSDoCache )
		return this->cacheHandler->useCache( stretch, plane, anch, zoom, pixels, size, flip );
	else
		return false;
}

Void PreviewModel::storeCache( Stretch& stretch, Plane& plane, FitsLiberator::Point&  anch,
							  Double zoom, Double* pixels, Int size, Bool flip )
{
	if ( kFITSDoCache )
		this->cacheHandler->storeCache( stretch, plane, anch, zoom, pixels, size, flip );	
}

Void PreviewModel::clearCache()
{
	this->cacheHandler->clearCache();
}


/*
|---------------------------------------------------------------|
|Following code is for the dimension and location of the image	|
|segment to be showed.											|
|---------------------------------------------------------------|
*/

FitsLiberator::Rectangle& PreviewModel::getImageArea()
{
	return this->imageOfPreview;
}

Int PreviewModel::getZoomIndex()
{
	return this->zoomIndex;
}

Void PreviewModel::setPreviewSize(FitsLiberator::Size& pSize)
{
//	if ( globalSettingsModel.getPreviewEnabled() )
//	{
		if (previewImage != NULL)
		{
			delete previewImage;
		}
		
		if( currentAnchor != NULL ) {
			delete currentAnchor;
		}
		
		previewImage	= new PreviewImage(pSize.width,pSize.height);
		this->zoomIndex	= 0;
		currentAnchor	= new FitsLiberator::Point(0,0);
		imageSegment.left = 0;
		imageSegment.top = 0;			
		imageSegment.right = getPreviewImage().size.width;
		imageSegment.bottom = getPreviewImage().size.height;
	
//	}
	
}

Void PreviewModel::generateZoomFit(FitsLiberator::Size& rS)
{
	Int i = 0;
	this->rawSize = rS;
	//calculates the zoomfactors.
	
	//determines the zoom fit factor.
	Double zoomMin = FitsMath::minimum((Double)this->previewImage->size.width / (Double)rawSize.width,
									   (Double)this->previewImage->size.height / (Double)rawSize.height);
									   
	Int incre = 0;
	Int spIncr = 0;
	for ( i = 0; i < zooms-1; i++)
	{
		if ( zoomMin >= staticZoomFactors[i+1] && zoomMin <= staticZoomFactors[i] )
		{
			zoomFactors[i] = staticZoomFactors[i];
			zoomFactors[i+1] = zoomMin;
			zoomFitPreviewIndex = i+1;

			if ( abs( zoomMin - staticZoomFactors[i+1] ) < abs( zoomMin - staticZoomFactors[i] ) )
			{
				this->zoomFitIndex = i + 1;
				spIncr = -1;
			}
			else
			{
				this->zoomFitIndex = i;
			}
			incre = 1;
		}
		else
		{
			zoomFactors[i + incre] = staticZoomFactors[i];
		}
		if ( staticZoomFactors[i] == 1.0 )
		{
			unityIndex = i + incre + spIncr;
		}
		else
		{
			spIncr = 0;
		}
	}
	zoomFactors[zooms-1] = staticZoomFactors[zooms-2];	
	this->zoomIndex = zoomFitPreviewIndex;
	Notify();
}

Double* PreviewModel::getZoomFactors()
{
	return this->zoomFactors;
}

Int PreviewModel::getZoomFactorCount() {
    return this->zooms;
}

Int PreviewModel::getNumberOfZoomFactors()
{
	return zooms;
}

FitsLiberator::Size& PreviewModel::getRawSize()
{
	return rawSize;
}

Int PreviewModel::getUnityZoomIndex() {
    return this->unityIndex;
}

Void PreviewModel::setUnityZoomIndex()
{
	this->zoomIndex = getUnityZoomIndex();
}

Void PreviewModel::setZoomFitIndex()
{
	//if ( globalSettingsModel.getPreviewEnabled() )
	//{
		this->zoomIndex = this->zoomFitIndex;
		Notify();
	//}
}

Void PreviewModel::setZoomFitFactor()
{
	//if ( globalSettingsModel.getPreviewEnabled() )
	//{
		this->zoomIndex = this->zooms - 1;
//	}
}


/**
* Returns the size of the preview area
*/
FitsLiberator::Size& PreviewModel::getPreviewSize()
{
	return this->previewImage->size;
}

FitsLiberator::Point* PreviewModel::getAnchorPoint()
{
	return this->currentAnchor;
}


/**
* Returns a Rectangle describing the segment of the actual image
* to be drawn. That is, it contains the upper left corner (x,y)
* and the width and height.
*/
FitsLiberator::Rectangle& PreviewModel::getImageSegment()
{
	return this->imageSegment;
}

/**
* Returns an Image, which contains the pixels of the actual
* image without the mask of undefined and null pixels.
*/
PreviewImage& PreviewModel::getPreviewImage()
{
	return *(this->previewImage);
}


/*
|---------------------------------------------------------------|
|Following code is for the zooming part of the preview model	|
|---------------------------------------------------------------|
*/

//sets the zoom factor closest to the argument
Void PreviewModel::setZoomFactor(Double f)
{
	//if ( globalSettingsModel.getPreviewEnabled() )
	//{
		//updates the index of the static zoom factor closest to the value passed in the argument.
		for (Int i = 0; i < zooms-1; i++)
		{
			Double f1 = zoomFactors[i+1];
			Double f2 = zoomFactors[i];
			if (f >= f1 && f <= f2)
			{
				this->zoomIndex = i;
			}
		}
	//}
}

Int PreviewModel::getZoomFitPreviewIndex()
{
	return this->zoomFitPreviewIndex;
}

/**
	this is a tricky function. We should either return the currently selected zoom value
	based on the index or the special factor, namely the zoomFit value.
	We choose only to return the index-based value. The zoomfit value should be optained from
	the appropriate method.
*/
Double PreviewModel::getZoomFactor()
{	
	//if the current zoomIndex is equal to the
	//last index, then the zoomFit value has been chosen. We should then use the 
	//zoomIndex closes to the static value corresponding to the zoom fit value
	if (zoomIndex == zooms )
	{
		zoomIndex = zoomFitIndex;
	}
    if (zoomIndex < zooms)
	{
		return this->zoomFactors[this->zoomIndex];
	}
	
	return -1;
}

Double PreviewModel::getZoomFitFactor()
{
	//if ( globalSettingsModel.getPreviewEnabled() )
	//{
		//sets the zoom index to be the last one
		zoomIndex = zooms - 1;
		return this->zoomFactors[zoomIndex];
	/*}
	else
	{
		return -1;
	}*/
}

/*
	Returns the maximum zoom factor possible
*/
Double PreviewModel::getMaxZoomFactor()
{
	//if ( globalSettingsModel.getPreviewEnabled() )
	//{
		return this->zoomFactors[0];
	/*}
	else
	{
		return -1;
	}*/
}

/*
	Returns true if the zoom factor can go higher, and false if not
*/
Bool PreviewModel::canIncrement()
{
    return ((zoomIndex > 0) && (!busy));
}

/*
	Returns true if the zoom factor can go lower, false if not.
*/
Bool PreviewModel::canDecrement()
{
    return ((zoomIndex < (UInt)zooms - 1) && (!busy));
}

Void PreviewModel::incrementZoomFactor()
{
	//if ( globalSettingsModel.getPreviewEnabled() )
	//{
		if (this->canIncrement())
		{
			this->zoomIndex--;
		}
	//}
}

Void PreviewModel::decrementZoomFactor()
{
	//if ( globalSettingsModel.getPreviewEnabled() )
	//{
		if (this->canDecrement())
		{
			this->zoomIndex++;
		}
	//}
}


FitsLiberator::Point PreviewModel::getImageCoordinates( const FitsLiberator::Point& p, Bool flip )
{
	FitsLiberator::Point retP(0,0);
	//if ( globalSettingsModel.getPreviewEnabled() )
	//{
		Double zoomFactor				= 0.0;
		
		//we have to check if the zoom fit value has been chosen
		if (zoomIndex == zooms - 1)
		{
			zoomFactor = getZoomFitFactor();
		}
		else
		{
			zoomFactor = this->getZoomFactor();
		}
		
		FitsLiberator::Point* anchor	= this->getAnchorPoint();
		
		FitsLiberator::Rectangle& imgR	= this->getImageArea();


		Int rawX = (Int)FitsLiberator::Engine::FitsMath::round(
			anchor->x + (p.x - imgR.left) / zoomFactor );
		Int rawY = 0;
		if ( !flip )
		{
			rawY = (Int)FitsLiberator::Engine::FitsMath::round(
				anchor->y + (p.y - imgR.top) / zoomFactor );
		}
		else
		{
			rawY = (Int)FitsLiberator::Engine::FitsMath::round(
				anchor->y + ( imgR.getHeight() - ( p.y - imgR.top ) ) / zoomFactor );
		}

		if (rawX < 0)
		{
			rawX = 0;
		}
		if (rawX > (Int)(this->rawSize.width))
		{
			rawX = (Int)(this->rawSize.width);
		}

		if (rawY < 0)
		{
			rawY = 0;
		}
		if ((UInt)rawY > this->rawSize.height)
		{
			rawY = this->rawSize.height;
		}
		
		retP.x = rawX;
		retP.y = rawY;
	//}
	return retP;
}

//-------------------------------------------------------------------------------
//	MovementVector implementation
//-------------------------------------------------------------------------------
MovementVector::MovementVector( Int x, Int y ) {
    this->x = x;
    this->y = y;
}