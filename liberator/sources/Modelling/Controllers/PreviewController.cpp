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
#include "PreviewController.h"
#include <algorithm>
#ifdef USE_OPENMP
#include <omp.h>
#endif

using namespace FitsLiberator::Modelling;
using namespace FitsLiberator::Engine;


PreviewController::PreviewController( PreviewModel& p, const GlobalSettingsModel& g ) :
previewModel( p ), globalSettingsModel( g ),moved(false)
{

}

PreviewController::~PreviewController()
{
}

//==============================================
// Methods for the various special zoom states
//==============================================

Void PreviewController::setUnityZoom( const FitsLiberator::Size& rawSize, Bool flip )
{
	if ( !(this->previewModel.getZoomIndex() == this->previewModel.getUnityZoomIndex()) )
	{
		this->setZoomIndex( previewModel.getUnityZoomIndex(), rawSize, flip );
	}
}
/**
	Shows the entire image in the preview frame.
*/
Void PreviewController::fitToPreview( const FitsLiberator::Size& rawSize, Bool flip )
{
	setImageArea( rawSize );
	setZoomIndex( previewModel.zoomFitPreviewIndex, rawSize, flip );
}

/**
If the given tile is within range then the effective range is inserted.
If not then the eff range variables are set to -1 to indicate that the tile
is out of scope for the current zoom state
@param ImageTile& tile the tile to be prepared
@param Bool flip determines if the image is flipped (true) or not (false)
*/
Void PreviewController::prepareTile( ImageTile& tile, const Bool flip )
{
	const Point& anchor = *(previewModel.getAnchorPoint());
	Double z = previewModel.getZoomFactor();
	const Size& pSize = previewModel.getPreviewSize();
	const Rectangle& imgR = previewModel.getImageArea();
	//retrieve the bounds for the tile
	const Rectangle& bounds = tile.getBounds();
	//left,right,top,bottom determines the range
	//in the preview that the tile has
	Int left = anchor.x;
	Int right = (Int)(FitsMath::round( anchor.x + pSize.width / z ));
	Int top = anchor.y;
	Int bottom = (Int)( FitsMath::round( anchor.y + pSize.height / z ) );
	
		
	if ( left < bounds.right && right > bounds.left &&
		top < bounds.bottom && bottom > bounds.top )
	{
		tile.effLeft = FitsMath::maximum( left, bounds.left );
		tile.effTop = FitsMath::maximum( top, bounds.top );
		tile.effRight = FitsMath::minimum( right, bounds.right );
		tile.effBottom = FitsMath::minimum( bottom, bounds.bottom );
	}
	else
	{
		tile.effLeft = -1;
		tile.effRight = -1;
		tile.effTop = -1;
		tile.effBottom = -1;
	}
}

/**
This method uses nearest-neighbor interpolation to resample a part of the
preview image from the given tile
@param const ImageTile& tile the tile to be used for resampling
@param Bool flip determines if the image is flipped (true) or not (false)
*/
Void PreviewController::zoomTile( ImageTile& tile, const Bool flip )
{
	if ( tile.isAllocated() )
	{

		PreviewImage& image = previewModel.getPreviewImage();

		const Point& anchor = *(previewModel.getAnchorPoint());
		Double z = previewModel.getZoomFactor();
		
		const Rectangle& bounds = tile.getBounds();
		Int tileArea = bounds.getArea();

		const Rectangle& imageArea = previewModel.getImageArea();

		const Int imgHeight = imageArea.getHeight();

		if ( flip )
		{
			Int lowX  = (Int)( FitsMath::round( z * ( tile.effLeft - anchor.x ) ) );
			Int highX = (Int)( FitsMath::round( z * ( tile.effRight - anchor.x ) ) );
			

			Int highY = (Int)( FitsMath::round( imgHeight - z * ( tile.effTop - anchor.y ) ) );			
			Int lowY  = (Int)( FitsMath::round( imgHeight - z * ( tile.effBottom - anchor.y ) ) );

			
			for ( Int i = lowX; i < highX; i++ )
			{
				for ( Int j = lowY; j < highY; j++ )
				{
					Int prevCoord = ( j + imageArea.top ) * image.size.width + i + imageArea.left;

					Double tileX = anchor.x + i / z;

					Int tileY = (Int)( ( anchor.y + ( imgHeight - 1. - j) / z ) );				
					
					tileX = FitsMath::maximum<Int>( tileX - bounds.left, 0 );

					tileY = FitsMath::maximum<Int>( tileY - bounds.top, 0 );

					Int tileCoord = (Int)( ( tileY * bounds.getWidth() + tileX ) );
					

					if ( tileCoord < 0 || tileCoord >= tileArea || prevCoord < 0 || prevCoord >= image.size.getArea() )
					{
						if ( prevCoord >= 0 && prevCoord < image.size.getArea() )					
							image.rawPixels[prevCoord] = 0.;										
					}
					else
					{
						image.rawPixels[prevCoord] = tile.stretchedPixels[tileCoord];					
					}
				}
			}
		}
		else
		{
			Int lowX = (Int)( FitsMath::round( z * ( tile.effLeft - anchor.x ) ) );
			Int highX = (Int)( FitsMath::round( z * ( tile.effRight - anchor.x ) ) );

			Int highY = (Int)( FitsMath::round( z * ( tile.effBottom - anchor.y ) ) );			
			Int lowY = (Int)( FitsMath::round( z * ( tile.effTop - anchor.y ) ) );

			for ( Int i = lowX; i < highX; i++ )
			{
				for ( Int j = lowY; j < highY; j++ )
				{
					Int prevCoord = ( j + imageArea.top ) * image.size.width + i + imageArea.left;

					Double tileX = anchor.x + i / z;

					Int tileY = (Int)( anchor.y + j / z );
					
					tileX = FitsMath::maximum<Int>( tileX - bounds.left, 0 );

					tileY = FitsMath::maximum<Int>( tileY - bounds.top, 0 );


					Int tileCoord = (Int)( tileY * bounds.getWidth() + tileX ) ;

					if ( prevCoord < 0 || tileCoord < 0 || prevCoord >= image.size.getArea() || tileCoord >=
						bounds.getArea() )
					{
						if ( prevCoord >= 0 && prevCoord < image.size.getArea() )					
							image.rawPixels[prevCoord] = 0.;	
					}
					else
					{
						image.rawPixels[prevCoord] = tile.stretchedPixels[tileCoord];					
					}
				}
			}
		}
	}
	else
	{
		throw Exception("Tried to zoom non-allocated pixels");
	}
}



/**

*/
Void PreviewController::setZoomIndex( Int f, const FitsLiberator::Size& rawSize, Bool flip )
{
	if ( globalSettingsModel.getPreviewEnabled() )
	{
		FitsLiberator::Size& size = this->previewModel.getPreviewSize();
		FitsLiberator::Point p( size.width / 2, size.height / 2 );
		
		//boundary conditions
		if ( f >= 0 && f < this->previewModel.zooms )
		{
			//checks to see which way to go.
			if ( f > (Int)(this->previewModel.zoomIndex) )
			{
				this->previewModel.zoomIndex = f - 1;
				doZoom( rawSize, p, false, flip );
			}
			else if ( f <= (Int)(this->previewModel.zoomIndex) )
			{
				this->previewModel.zoomIndex = f + 1;
				doZoom( rawSize, p, true, flip );
			}
		}
		else if ( f >= this->previewModel.zooms )
		{
			this->fitToPreview( rawSize, flip );
		}
		else
		{
			this->previewModel.Notify();
		}
	}
	else
	{
		this->previewModel.Notify();
	}
	this->moved = false;
}




/**
This method uses nearest-neighbor interpolation to resample a part of the
preview image from the given tile. It is equivalent to zoomTile with the
only difference that this implementation is parallized.
The parallelization only works when OMP is enabled (which currently only works
on Windows; not Mac). The method works without OMP enabled as well
@param const ImageTile& tile the tile to be used for resampling
@param Bool flip determines if the image is flipped (true) or not (false)
@param nCpus the number of cpus to use for the parallelization
*/
Void PreviewController::zoomTile_par( ImageTile& tile, const Bool flip, UInt nCpus )
{
	if ( tile.isAllocated() )
	{

		PreviewImage& image = previewModel.getPreviewImage();

		const Point& anchor = *(previewModel.getAnchorPoint());
		Double z = previewModel.getZoomFactor();
		
		const Rectangle& bounds = tile.getBounds();
		Int tileArea = bounds.getArea();

		const Rectangle& imageArea = previewModel.getImageArea();

		const Int imgHeight = imageArea.getHeight();

		if ( flip )
		{
			Int lowX  = (Int)( FitsMath::round( z * ( tile.effLeft - anchor.x ) ) );
			Int highX = (Int)( FitsMath::round( z * ( tile.effRight - anchor.x ) ) );
			

			Int highY = (Int)( FitsMath::round( imgHeight - z * ( tile.effTop - anchor.y ) ) );			
			Int lowY  = (Int)( FitsMath::round( imgHeight - z * ( tile.effBottom - anchor.y ) ) );

			#ifdef USE_OPENMP	
            #pragma omp parallel num_threads( nCpus )
	        {
			#pragma omp for
			#endif // USE_OPENMP  
			for ( Int i = lowX; i < highX; i++ )
			{
				for ( Int j = lowY; j < highY; j++ )
				{
					Int prevCoord = ( j + imageArea.top ) * image.size.width + i + imageArea.left;

					Double tileX = anchor.x + i / z;

					Int tileY = (Int)( ( anchor.y + ( imgHeight - 1. - j) / z ) );				
					
					tileX = FitsMath::maximum<Int>( tileX - bounds.left, 0 );

					tileY = FitsMath::maximum<Int>( tileY - bounds.top, 0 );

					Int tileCoord = (Int)( ( tileY * bounds.getWidth() + tileX ) );
					

					if ( tileCoord < 0 || tileCoord >= tileArea || prevCoord < 0 || prevCoord >= image.size.getArea() )
					{
						if ( prevCoord >= 0 && prevCoord < image.size.getArea() )
							image.rawPixels[prevCoord] = 0.;					
					}
					else
					{
						image.rawPixels[prevCoord] = tile.stretchedPixels[tileCoord];					
					}
				}
			}
			#ifdef USE_OPENMP	
			}
			#endif
		}
		else
		{
			Int lowX = (Int)( FitsMath::round( z * ( tile.effLeft - anchor.x ) ) );
			Int highX = (Int)( FitsMath::round( z * ( tile.effRight - anchor.x ) ) );

			Int highY = (Int)( FitsMath::round( z * ( tile.effBottom - anchor.y ) ) );			
			Int lowY = (Int)( FitsMath::round( z * ( tile.effTop - anchor.y ) ) );
			#ifdef USE_OPENMP	
            #pragma omp parallel num_threads( nCpus )
	        {
			#pragma omp for
			#endif // USE_OPENMP  
			for ( Int i = lowX; i < highX; i++ )
			{
				for ( Int j = lowY; j < highY; j++ )
				{
					Int prevCoord = ( j + imageArea.top ) * image.size.width + i + imageArea.left;

					Double tileX = anchor.x + i / z;

					Int tileY = (Int)( anchor.y + j / z );
					
					tileX = FitsMath::maximum<Int>( tileX - bounds.left, 0 );

					tileY = FitsMath::maximum<Int>( tileY - bounds.top, 0 );


					Int tileCoord = (Int)( tileY * bounds.getWidth() + tileX ) ;

					if ( prevCoord < 0 || tileCoord < 0 || prevCoord >= image.size.getArea() || tileCoord >=
						bounds.getArea() )
					{
						if ( prevCoord >= 0 && prevCoord < image.size.getArea() )
						{					
							image.rawPixels[prevCoord] = 0.;	
						}
					}
					else
					{
						image.rawPixels[prevCoord] = tile.stretchedPixels[tileCoord];					
					}
				}
			}
			#ifdef USE_OPENMP	
			}
			#endif
		}
	}
	else
	{
		throw Exception("Tried to zoom non-allocated pixels");
	}
}



/**
	Performs a guess on the background based on the position p in the image
	We calculate both the mean and the median to se what wll be preferred.	
*/

Double PreviewController::getPixelGuess( const FitsLiberator::Point& p, FitsLiberator::Size& rawSize,
										const FitsLiberator::Engine::Stretch& stretch, const Bool flip )
{
	
	if ( globalSettingsModel.getPreviewEnabled() )
	{
		FitsLiberator::Size& pSize			= this->previewModel.getPreviewSize();

		//array containing each pixel value. Should be sorted afterwards to get the median.
		Double values[kFITSBackgroundGuessWidth * kFITSBackgroundGuessHeight];

		Double sum					= 0;
		Int cnt						= 0;
		
		if (p.x > kFITSBackgroundGuessWidth / 2 && p.x < (Int)pSize.width - kFITSBackgroundGuessWidth / 2 &&
			p.y > kFITSBackgroundGuessHeight / 2 && p.y < (Int)pSize.height - kFITSBackgroundGuessHeight / 2)
		{

			for (Int j = p.y - kFITSBackgroundGuessHeight / 2; j < p.y + kFITSBackgroundGuessHeight / 2; j++)
			{
				for (Int i = p.x - kFITSBackgroundGuessWidth / 2; i < p.x + kFITSBackgroundGuessWidth / 2; i++)
				{
					const FitsLiberator::Point point(i, j);
					Double val = getRealValueAtPos( point, rawSize, stretch, flip );
					if ( FitsLiberator::Engine::FitsMath::isFinite( val ) )
					{
						sum += val;
						values[cnt] = val;
						cnt++;
					}
				}
			}
		
			std::sort( values, values + cnt );
			//we now want to find the median value
			Double median = values[cnt / 2];

			if (cnt > 0)
			{
				if (kFITSBackgroundGuessMedian)
				{					
					return median;
				}
				else
				{					
					return sum / (Double)cnt;
				}
			}
		}
	}
	//should probably cast an exception here.
	return 0;
}



/**
	Decrement zoom...
*/
Void PreviewController::decrementZoom( const FitsLiberator::Size& rawSize, 
									  FitsLiberator::Point& p, Bool flip )
{
	doZoom( rawSize, p, false, flip );
}

/**
	Tries to zoom the rectangle specified.
*/
Void PreviewController::zoomRectangle( FitsLiberator::Rectangle& rect, 
									   const FitsLiberator::Size& rawSize, Bool flip )
{
	if ( globalSettingsModel.getPreviewEnabled() )
	{
		//we have to make sure, the rectangle is in the right coordinate system
		if (rect.left > rect.right)
		{
			Int l = rect.left;
			rect.left = rect.right;
			rect.right = l;
		}
		if (rect.top > rect.bottom)
		{
			Int t = rect.top;
			rect.top = rect.bottom;
			rect.bottom = t;
		}
		FitsLiberator::Rectangle& imgA		= this->previewModel.getImageArea();
		//truncates the rect if selected outside preview area. This is probably only a Mac issue
		if ( rect.left <= imgA.left && rect.top <= imgA.top && rect.right >= imgA.right && rect.bottom >= imgA.bottom )
		{
			this->fitToPreview( rawSize, flip );
			return;
		}

		FitsLiberator::Point* currentPos	= this->previewModel.getAnchorPoint();
		
		FitsLiberator::Size& pSize			= this->previewModel.getPreviewSize();

		//FitsLiberator::Point tmpPoint(rect.left, rect.top); // Lars: CodeWarrior doesn't allow the other method.
		FitsLiberator::Point tmpPoint(rect.left + rect.getWidth() / 2, rect.top + rect.getHeight() / 2);
		FitsLiberator::Point anc			= this->previewModel.getImageCoordinates( tmpPoint, flip );

		//we choose only to select a zoom factor in the static range...
		Double zoom							= this->previewModel.getZoomFactor();

		//calculates a new zoom factor
		Double z1							= zoom * pSize.width / (Double)rect.getWidth();
		Double z2							= zoom * pSize.height / (Double)rect.getHeight();

		Double nZoom						= FitsLiberator::Engine::FitsMath::minimum(z1, z2);
		Double maxZoom						= this->previewModel.getMaxZoomFactor();

		if (nZoom > maxZoom)
		{
			nZoom = maxZoom;
		}
		
		this->previewModel.setZoomFactor( nZoom );

		Double zoomFactor					= this->previewModel.getZoomFactor();
		this->setImageArea( rawSize );
		FitsLiberator::Rectangle& imgR		= this->previewModel.getImageArea();

		anc.x -= (Int)FitsLiberator::Engine::FitsMath::round( .5 / zoomFactor * imgR.getWidth() );
		anc.y -= (Int)FitsLiberator::Engine::FitsMath::round( .5 / zoomFactor * imgR.getHeight() );

		
		Double anchorX						= anc.x;
		Double anchorY						= anc.y;


		Bool doZoom = true;
		
		if (anchorX < 0)
		{
			anchorX = 0;
		}
		if (anchorX >= rawSize.width)
		{
			doZoom = false;
		}
		if (anchorX + imgR.getWidth() / zoomFactor >= rawSize.width)
		{
			anchorX = rawSize.width - imgR.getWidth() / zoomFactor;
		}

		if (anchorY < 0)
		{
			anchorY = 0;
		}
		if (anchorY >= rawSize.height)
		{
			doZoom = false;
		}
		if (anchorY + imgR.getHeight() / zoomFactor >= rawSize.height)
		{
			anchorY = rawSize.height - imgR.getHeight() / zoomFactor;
		}


		if (doZoom)
		{
			if ( this->previewModel.getZoomIndex() > this->previewModel.zoomFitPreviewIndex )
			{
				currentPos->x = 0;
				currentPos->y = 0;
			}
			else
			{
				currentPos->x = (Int)anchorX;
				currentPos->y = (Int)anchorY;
			}
		}
		this->moved = false;
	}
}

/**
 * Centers the preview
 */
Void PreviewController::centerPreview( const FitsLiberator::Size& rawSize )
{
	FitsLiberator::Point* anchor	= this->previewModel.getAnchorPoint();
	Double zoomFactor				= this->previewModel.getZoomFactor();
	FitsLiberator::Rectangle& imgR	= this->previewModel.getImageArea();

	Int x = (Int)FitsLiberator::Engine::FitsMath::round( zoomFactor * (
		anchor->x - ( 0.5 * rawSize.width - 0.5 * imgR.getWidth() / zoomFactor ))
		);
	Int y = (Int)FitsLiberator::Engine::FitsMath::round( zoomFactor * (
		anchor->y - ( 0.5 * rawSize.height - 0.5 * imgR.getHeight() / zoomFactor ))
		);

	MovementVector v( x, y );

	this->movePreview( v, rawSize );

}

/*
Increments the zoom factor and resamples the image

*/
Void PreviewController::incrementZoom( const FitsLiberator::Size& rawSize, 
									  FitsLiberator::Point& p, Bool flip )
{
	doZoom( rawSize, p,true, flip );
}

/**
Zooms with the centre in @param p, if possible.

*/
Void PreviewController::doZoom( const FitsLiberator::Size& rawSize, FitsLiberator::Point& p, 
							   Bool incr, Bool flip )
{
	if ( globalSettingsModel.getPreviewEnabled() )
	{
		FitsLiberator::Rectangle& imgR	= this->previewModel.getImageArea();
		
		//checks for out of bounds
		if ( p.x < imgR.left || p.x > imgR.right )
		{
			p.x = imgR.left + imgR.getWidth() / 2;
		}
		if ( p.y < imgR.top || p.y > imgR.bottom )
		{
			p.y = imgR.top + imgR.getHeight() / 2;
		}		

		FitsLiberator::Point rawC		= this->previewModel.getImageCoordinates( p, flip );

		if ( rawC.x < 0 || rawC.x > (Int)rawSize.width )
		{
			rawC.x = rawSize.width / 2;
		}
		if ( rawC.y < 0 || rawC.y > (Int)rawSize.height )
		{
			rawC.y = rawSize.height / 2;
		}


		FitsLiberator::Point* anchor	= this->previewModel.getAnchorPoint();
		Double zoomFactor				= this->previewModel.getZoomFactor();
		
		
		if (incr)
		{
			this->previewModel.incrementZoomFactor();
		}
		else
		{
			this->previewModel.decrementZoomFactor();
		}
		
		zoomFactor						= this->previewModel.getZoomFactor();
		this->setImageArea( rawSize );
		imgR							= this->previewModel.getImageArea();
		
		Double xHalfRange				= 0.5 / zoomFactor * imgR.getWidth();
		Double yHalfRange				= 0.5 / zoomFactor * imgR.getHeight();
		
		Int xStart						= (Int)FitsLiberator::Engine::FitsMath::round(
											rawC.x - xHalfRange);	
		Int yStart						= (Int)FitsLiberator::Engine::FitsMath::round(
											rawC.y - yHalfRange);
		
		if (xStart < 0)
		{
			xStart = 0;
		} 
		else if (xStart > rawSize.width - xHalfRange)
		{
			xStart = rawSize.width - (Int)xHalfRange;
		} 
		else if (xStart + imgR.getWidth() / zoomFactor >= rawSize.width)
		{
			xStart = rawSize.width - (Int)(imgR.getWidth() / zoomFactor);
		}

		if (yStart < 0)
		{
			yStart = 0;
		} 
		else if (yStart > rawSize.height - yHalfRange)
		{
			yStart = rawSize.height - (Int)yHalfRange;
		} 
		else if (yStart + imgR.getHeight() / zoomFactor >= rawSize.height)
		{
			yStart = rawSize.height - (Int)(imgR.getHeight() / zoomFactor);
		}

		if ( this->previewModel.getZoomIndex() >= this->previewModel.zoomFitPreviewIndex )
		{
			anchor->x = 0;
			anchor->y = 0;
		}
		else
		{
			anchor->x = xStart;
			anchor->y = yStart;
		}		
		this->moved = false;
	}
}

Void PreviewController::movePreview( MovementVector& vec, const FitsLiberator::Size& rawSize )
{
	if ( globalSettingsModel.getPreviewEnabled() )
	{
		Double zoom							= this->previewModel.getZoomFactor();
		FitsLiberator::Rectangle& imgR		= this->previewModel.getImageArea();
		FitsLiberator::Point* currentPos	= this->previewModel.getAnchorPoint();

		//updates the anchor point of the raw image

		Double anchorX						= currentPos->x - FitsMath::round(1.0 / zoom * vec.x);
		Double anchorY						= currentPos->y - FitsMath::round(1.0 / zoom * vec.y);

		
		//boundary condition on the x-axis
		if (zoom * rawSize.width <= imgR.getWidth())
		{
			vec.x = 0;
		}
		if (anchorX + imgR.getWidth() / zoom >= rawSize.width)
		{
			anchorX = rawSize.width - imgR.getWidth() / zoom;
		}
		if (anchorX < 0)
		{
			anchorX = 0;
		}


		//boundary condition on the y-axis
		if (zoom * rawSize.height <= imgR.getHeight())
		{
			vec.y = 0;
		}
		if (anchorY + imgR.getHeight() / zoom >= rawSize.height)
		{
			anchorY = rawSize.height - imgR.getHeight() / zoom;
		}
		if (anchorY < 0)
		{
			anchorY = 0;
		}

		currentPos->x = (UInt)anchorX;
		currentPos->y = (UInt)anchorY;
		
	}
}

/*
* re-scales the image in the current dynamic range [blacklevel:whitelevel]
*
*/
Void PreviewController::scaleDynamicRange(  Double bLevel, Double wLevel )
{
	//if ( globalSettingsModel.getPreviewEnabled() )
	//{
		Rectangle& previewSegment = previewModel.getImageSegment();
		//range
		Double wMinusBLevel			= wLevel - bLevel;

		//counter
		Int i = 0;
		Int j = 0;
		
		//the the pixel values to be updated
		PreviewImage& previewImage	= this->previewModel.getPreviewImage();

		//temp preview output val
		UInt previewPixelVal		= 0;

		//tmp raw val
		Double currentRawPixel		= 0;

		//flags determining special values (maps)
		Byte nullVal				= 255;
		Byte blackVal				= 255;
		Byte whiteVal				= 255;
		

		makeBorder( previewModel.getImageArea(), bLevel );

		if (previewSegment.left >= 0 && previewSegment.top >= 0 &&
			previewSegment.right <= (Int)previewImage.size.width && previewSegment.bottom <= (Int)previewImage.size.height
			)
		{
			Int pWidth = previewSegment.getWidth();
			for (j = previewSegment.top; j < previewSegment.bottom; j++)
			{
				for (i = previewSegment.left; i < previewSegment.right; i++)
				{
					//resets the special map-values
					nullVal		= 255;
					blackVal	= 255;
					whiteVal	= 255;
					
					Int coord = j * pWidth + i;

					currentRawPixel = previewImage.rawPixels[coord];

					if ( !FitsMath::isFinite(currentRawPixel) )
					{
						//if the value is undefined
						previewPixelVal = 0;
						nullVal = 0;
					}
					else if (currentRawPixel < bLevel)
					{
						//if the value is below the black level, it should be truncated to the blacklevel
						previewPixelVal = 0;
						blackVal = 0;
					}
					else if (currentRawPixel > wLevel)
					{
						//if the value is above the whitelevel, it should be truncated to the whitelevel
						previewPixelVal = 255;
						whiteVal = 0;
					}
					else
					{
						//the preview pixel value caculated from:
						// (val - lowerLevel) * (outputMax - outputMin) / (inputMax - inputMin)
						
						//previewPixelVal = 128;
						
						
						previewPixelVal = 
						(Byte)FitsLiberator::Engine::FitsMath::round( 
						(currentRawPixel - bLevel) * 255.0 / (wMinusBLevel));
						
					}
					previewImage.previewPixels[coord]	= (Byte)previewPixelVal;
					previewImage.nullMap[coord]			= nullVal;
					previewImage.blackClippedMap[coord]	= blackVal;
					previewImage.whiteClippedMap[coord]	= whiteVal;
				}
			}

			this->previewModel.Notify();
		}
	//}
}

/**
* Returns the real pixel value at the specified preview image point
*/
Double PreviewController::getRealValueAtPos( const FitsLiberator::Point& p, FitsLiberator::Size& rawSize,
											const Stretch& stretch, const Bool flip )
{
	if ( globalSettingsModel.getPreviewEnabled() )
	{
		const FitsLiberator::Point imgPoint	= this->previewModel.getImageCoordinates( p, flip );
		Int val = imgPoint.y * rawSize.width + imgPoint.x;
		if (val >= 0 && (UInt)val < rawSize.getArea())
		{
			return FitsLiberator::Engine::FitsEngine::getLinearVal( stretch, getStretchedValueAtPos( p ) );
		}

		return 0;
	}
	else
	{
		return FitsMath::NaN;
	}
}

Double PreviewController::getScaledValueAtPos( const FitsLiberator::Point& p, 
											  FitsLiberator::Size& rawSize,
											  const Stretch& stretch, const Bool flip )
{
	if ( globalSettingsModel.getPreviewEnabled() )
	{
		const FitsLiberator::Point& imgPoint	= previewModel.getImageCoordinates( p, flip );
		Int val = imgPoint.y * rawSize.width + imgPoint.x;
		if (val >= 0 && (UInt)val < rawSize.getArea())
		{
			return FitsLiberator::Engine::FitsEngine::
				getLinearValWithoutStretch( stretch, getStretchedValueAtPos( p ) );
		}

		return 0;
	}
	else
	{
		return FitsMath::NaN;
	}
}

Double PreviewController::getStretchedValueAtPos( const FitsLiberator::Point& p )
{
	if ( globalSettingsModel.getPreviewEnabled() )
	{
		FitsLiberator::Rectangle& area	= this->previewModel.getImageArea();
		PreviewImage& previewImage		= this->previewModel.getPreviewImage();
		

		if (p.x >= area.left && p.x < area.right && p.y >= area.top && p.y < area.bottom)
		{
			return previewImage.rawPixels[ p.x + p.y * previewImage.size.width ];	
		}
		return 0.;
	}
	else
	{
		return FitsMath::NaN;
	}
}

Void PreviewController::setImageArea( const FitsLiberator::Size& rawSize )
{
	//if ( globalSettingsModel.getPreviewEnabled() )
	//{
		FitsLiberator::Size& previewSize	= this->previewModel.getPreviewSize();

		Double zoomFactor					= this->previewModel.getZoomFactor();

		Double dW							= (Double)previewSize.width / zoomFactor;
		Double dH							= (Double)previewSize.height / zoomFactor;

		Int startX							= 0;
		Int endX							= previewSize.width;
		Int startY							= 0;
		Int endY							= previewSize.height;

		if (dW > rawSize.width)
		{
			startX	= (Int)FitsLiberator::Engine::FitsMath::round(
				0.5 * (previewSize.width - zoomFactor * rawSize.width)
				);
			endX	= (Int)FitsLiberator::Engine::FitsMath::round(
				startX + zoomFactor * rawSize.width
				);
		}
		if (dH > rawSize.height)
		{
			startY	= (Int)FitsLiberator::Engine::FitsMath::round(
				0.5 * (previewSize.height - zoomFactor * rawSize.height)
				);
			endY	= (Int)FitsLiberator::Engine::FitsMath::round(
				startY + zoomFactor * rawSize.height
				);
		}

		FitsLiberator::Rectangle rect(startX,startY,endX,endY);
		this->previewModel.imageOfPreview = rect;
		
	//}
}

Void PreviewController::makeBorder(FitsLiberator::Rectangle& innerRect, Double currentBlack )
{
	if ( globalSettingsModel.getPreviewEnabled() )
	{
		FitsLiberator::Size& previewSize	= this->previewModel.getPreviewSize();

		Int j								= 0;
		Int i								= 0;
		
		PreviewImage& previewImage			= this->previewModel.getPreviewImage();
		
		for (i = 0; i < innerRect.left; i++)
		{
			for (j = 0; j < (Int)previewSize.height; j++)
			{
				previewImage.rawPixels[j * previewSize.width + i] = currentBlack;
			}
		}
		for (i = innerRect.right; i < (Int)previewSize.width; i++)
		{
			for (j = 0; j < (Int)previewSize.height; j++)
			{
				previewImage.rawPixels[j * previewSize.width + i] = currentBlack;
			}
		}

		for (j = 0; j < innerRect.top; j++)
		{
			for (i = 0; i < (Int)previewSize.width; i++)
			{
				previewImage.rawPixels[j * previewSize.width + i] = currentBlack;
			}
		}
		for (j = innerRect.bottom; j < (Int)previewSize.height; j++)
		{
			for (i = 0; i < (Int)previewSize.width; i++)
			{
				previewImage.rawPixels[j * previewSize.width + i] = currentBlack;
			}
		}
	}
}
