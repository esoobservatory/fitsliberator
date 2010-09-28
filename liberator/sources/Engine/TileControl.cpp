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
#include "TileControl.h"

#include "omp.h"
#include <time.h>

using namespace FitsLiberator::Engine;
using namespace std;

/**
*	The TileControl constructor
*
*/
TileControl::TileControl( UInt mmUsg )
{
	tiles = NULL;
	nTiles = -1;
	nAllocTiles = -1;
	allocatedTiles = NULL;
	nQueues = -1;
	oldCubeWidth = -1;
	oldCubeHeight = -1;
	oldTileSize = -1;
	oldTiles = NULL;
	maxThreads = 1;
	maxMemUsage = mmUsg;


	oldMaxMemUsage = 0;
	nMaxAllocTiles = -1;
}

TileControl::~TileControl()
{
	if ( tiles != NULL ) delete[] tiles;
	if ( allocatedTiles != NULL ) delete[] allocatedTiles;
	if ( oldTiles != NULL ) delete[] oldTiles;
	
	
}

/**
*	Decreases maxMemUsage by 10 %
*/
Void TileControl::decreaseMaxMem()
{
	maxMemUsage = (UInt)( 0.9 * maxMemUsage );

}

/**
Returns a reference to the specified tile without allocating or anything
This is used only for coordinate stuff, i.e. no direct pixel-manipulation
*/
ImageTile* TileControl::getLightTile( const Int tile )
{
	if ( tile >= 0 && tile < nTiles )
	{
		return &tiles[tile];
	}
	return NULL;
}

const Int TileControl::getNumberOfTiles()
{
	return this->nTiles;
}

/**
	This method allocates a given tile and makes sure that 
	either an already allocated tile is de-allocated (if it is not in use
	by another thread, i.e. its locked field is false) or the 
	new tile is simply flagged in the allocatedTiles array
	@param tile the index of the tile that is asked for
	@param cube the image cube containing the current image
	@param plane the image plane currently used
	@param lock if the returned tile should be locked
	@returns -1 if somehow it cannot be done otherwise it returns 0 if everything went ok
*/
ImageTile* TileControl::getTile( const Int tile, const ImageCube* cube, 
								const Plane& plane, const Bool lock )
{
	Int threadNum = 0;
#ifdef USE_OPENMP
	threadNum = omp_get_thread_num();
#endif
	if ( tile >= 0 && tile < nTiles && allocatedTiles != NULL )
	{
		//first we check if the tile is already allocated and is not in use
		if ( tiles[tile].locked == false && tiles[tile].isAllocated() == true )
		{
			return &tiles[tile];
		}
		else if ( tiles[tile].locked == false && tiles[tile].isAllocated() == false )
		{
			//allocate and return
			
			//first we should check if the queue has reached its limit
			if ( allocatedTiles[0].size() >= (UInt)nAllocTiles ) //threadNum].size() >= (UInt)nAllocTiles )
			{
				//then we need to push through the queue until we reach an object
				//which is not locked
				Bool found = false;
				//this is a potentially unsafe loop that might be dead-locked
				//if there are more processors than tiles or just equally many
				//We therefore introduce a counter so the process can be aborted
				Int cnt = 0;
				while ( found == false )
				{
					Int cTile = allocatedTiles[0].front();//[threadNum].front();
					if ( tiles[cTile].locked == false  )
					{
						found = true;
					}
					else
					{
						//add the oldest element to the back since it is still in use
						allocatedTiles[0].push( cTile );//[threadNum].push( cTile );
						//pop the oldest element since we have just saved it as the newest.
						allocatedTiles[0].pop();//[threadNum].pop();
					}
					//the tile could not be allocated. Bad, but it would be worse if
					// the loop was dead-locked
					if ( cnt > nAllocTiles && found == false ) throw Exception("Could not free up memory. This should not be allowed to happen.");
					cnt++;
				}
				//deallocate the oldest tile
				Int cTile = allocatedTiles[0].front();				
				
				tiles[cTile].deallocatePixels();
				//remove the old tile from the queue
				allocatedTiles[0].pop();
			}
			//allocate the new tile and load the pixels
			tiles[tile].allocatePixels( cube->SizeOf(1,1) );
			
			//add the new tile to the queue
			allocatedTiles[0].push( tile );

#ifdef USE_OPENMP
#pragma omp critical//this is done critically since cfitsio is not thread safe
			{
#endif
			if ( lock ) tiles[tile].locked = true;
			//load the pixels from the file
			cube->Read( plane.planeIndex, tiles[tile].getBounds(),
				tiles[tile].rawPixels, tiles[tile].nullPixels);
			
#ifdef USE_OPENMP
			}
#endif
			return &tiles[tile];
		}
		else
		{
			//is locked and cannot be accessed
			return NULL;
		}
	}
	else
	{
		return NULL;
	}
}

/**
Returns true if the two tiles overlap, false if not
*/
Bool TileControl::tilesOverlap( ImageTile& tile1, ImageTile& tile2 )
{
	const Rectangle& rect1 = tile1.getBounds();
	const Rectangle& rect2 = tile2.getBounds();

	if ( rect1.top <= rect2.top && rect1.bottom >= rect2.bottom && 
		rect1.left <= rect2.left && rect1.right >= rect2.right )
	{
		return true;
	}
	return false;

}


/**
Private method for distributing the tiles based on the
preferred tile size, number of cpu's, available memory etc.

	either the image can be fully loaded into memory or
	it has to be tiled up

	determine the maximum available amount of memory for the image
	the global constant kFITSCacheMaxMemUsage defines the maximum
	amount of bytes available (for the time being)

	calculate how much memory the current image will use
	
	we need the following
	- A copy of the raw pixels (size equal to the bitdepth of the image
	- A local representation of the image as Double. This will contain the current
		pixels no matter if they are stretched or whatever, i.e. they are directly used
		for preview generation, statistics etc
	- A nullmap containing a map of which pixels are defined as null in the image. This is a
		byte array

	Calculate the tiling of the image based on how much memory there is available.
	We define a minimum tile size through the local constants tile_min_width and
	tile_min_height. The tiling will then make as many as these tiles as possible. Obviously,
	if the entire image can be represented in memory at all times then we only need one tile		
	@param imgWidth the width of the image
	@param imgHeight the height of the image
	@param minWidth the requested minimum width of the tile
	@param minHeight the requested minimum height of the tile
	@param bytesPrPixel the total number of bytes per pixel (=8+1+bitDepth)
	@param totalBytes the total amount of bytes allocatable
	@param *nTls pointer to be filled out by this function with the total number of tiles
	@param *nAlcTls pointer to be filled out by this function with the number of allocatable tiles
	@param cube the image cube containing the current image
	@param plane the current plane
*/
Bool TileControl::distributeTiles( const Int imgWidth, const Int imgHeight, 
								   const Int minWidth, const Int minHeight, const Int bytesPrPixel,
								   const Int64 totalBytes, Int* nTls, Int* nAlcTls,
								   const ImageCube* cube, const Plane& plane )
{
	Int nTilesXDirection = -1;
	Int nTilesYDirection = -1;
	Int bitDepth = cube->SizeOf(1,1);

	//determine the number of tiles in the x-direction
	nTilesXDirection = imgWidth / minWidth;
	
	//if the division was not clean
	if ( imgWidth % minWidth != 0 )
		nTilesXDirection++;

	//determine the number of tiles in the y-direction
	nTilesYDirection = imgHeight / minHeight;
	
	//if the division was not clean
	if ( imgHeight % minHeight != 0 )
		nTilesYDirection++;
	
	//the number of tiles
	*nTls = nTilesXDirection * nTilesYDirection;

	//the number of allocatable tiles at a time
	//if this was not set to one then it means that
	//more than one tile may be allocated at the same time
	//otherwise we should have only one currently allocated
	if ( *nAlcTls != 1 )
	{
		//in the case when we do not import this is fine since the tiles are large
		*nAlcTls = maxMemUsage / ( bytesPrPixel * minWidth * minHeight );
	}

	nMaxAllocTiles = *nAlcTls;


	if ( *nAlcTls < 1 )
		*nAlcTls = 1;	

	//init the new tiles
	if ( tiles == NULL && *nTls > 0 )
	{
		/*The tiles should be distributed once and for all for the given image.
		  This means that the tiles are pre-determined and whether they contain
		  pixels or just null pointers is determined by which parts of the image are
		  in use currently
		*/
		tiles = new ImageTile[*nTls];

		if ( *nTls > 1 && nTilesXDirection > 0 && nTilesYDirection > 0 )
		{			
			//loop over each tile
			for ( Int j = 0; j < nTilesYDirection; j++ )
			{		
				for ( Int i = 0; i < nTilesXDirection; i++ )
				{			
					Int curWidth = 0;
					Int curHeight = 0;
					
					curWidth = imgWidth / nTilesXDirection;					
										
					curHeight = imgHeight / nTilesYDirection;					
					
					//set the upper left corner of the current tile
					tiles[j * nTilesXDirection + i].setX( i * curWidth );
					tiles[j * nTilesXDirection + i].setY( j * curHeight );
					
					//Should be corrected if the division is not clean
					if ( i == nTilesXDirection - 1 )
						curWidth += imgWidth % nTilesXDirection;
					if ( j == nTilesYDirection - 1 )								
						curHeight += imgHeight % nTilesYDirection;

					//set the width and height of the current tile
					tiles[j * nTilesXDirection + i].setWidth( curWidth );
					tiles[j * nTilesXDirection + i].setHeight( curHeight );
										
				}								
			}
		}
		else
		{
			tiles[0].x = 0;
			tiles[0].y = 0;
			tiles[0].width = imgWidth;
			tiles[0].height = imgHeight;
			//try to actually allocate the tile.
			//if not possible then return false
			if ( tiles[0].allocatePixels( bitDepth ) == ImageTile::AllocOk )
			{
				//load the pixels from the file
				cube->Read( plane.planeIndex, tiles[0].getBounds(),
					tiles[0].rawPixels, tiles[0].nullPixels);
				return true;
			}
			else
				return false;
		}

	}
	return true;
	
}

/**
	This method performs the tiling of the image.
	If the image is small enough to fit in memory the
	tiling is either one tile or the number of logic processors
	on the system depending on whether omp is enabled.
	If the image is too large then the tiling is done according
	to the min width and min height defined in FlowController.h
	@param cube the current imagecube
	@param tileSize the requested scheme should be either TileControl::tileSizeSmall, Large or Import
*/
Void TileControl::reTile( const ImageCube* cube, const Int tileSize, const Plane& plane )
{

	//first check if the tiling has already been done based on the width and height of the cube
	if ( ( cube->Width() != this->oldCubeWidth || cube->Height() != this->oldCubeHeight ) ||
		tileSize != this->oldTileSize  || maxMemUsage != this->oldMaxMemUsage )
	{
		//special case. If there is only one tile, however big, we should never go
		//to small tiles!
		if ( cube->Width() == this->oldCubeWidth && cube->Height() == this->oldCubeHeight &&
			 tileSize == TileControl::tileSizeSmall && maxMemUsage == this->oldMaxMemUsage &&
			 getNumberOfTiles() == 1 )
			return;

		this->oldCubeWidth = cube->Width();
		this->oldCubeHeight = cube->Height();
		this->oldTileSize = tileSize;
		this->oldMaxMemUsage = maxMemUsage;
		
		
		Bool tilingOK = false;

		while ( !tilingOK )
		{
			/*we need the following
			- A copy of the raw pixels (size equal to the bitdepth of the image)
			- A local representation of the image as Double. This will contain the current
				pixels no matter if they are stretched or whatever, i.e. they are directly used
				for preview generation, statistics etc
			- A nullmap containing a map of which pixels are defined as null in the image. This is a
				byte array
			*/
			Int bytesPrPixel = cube->SizeOf(1,1) + sizeof( Double ) + sizeof( Byte );

			/*
				Calculate the tiling of the image based on how much memory there is available.
				We define a minimum tile size through the local constants tile_min_width and
				tile_min_height. The tiling will then make as many as these tiles as possible. Obviously,
				if the entire image can be represented in memory at all times then we only need one tile		
			*/
			//how much is needed
			//determine the maximum available amount of memory for the image
			//the global constant kFITSCacheMaxMemUsage defines the maximum
			//amount of bytes available (for the time being)

			//calculate how much memory the current image will use
			//It is _crucial_ that the calculation is done using a int_64 since
			//the number easily can get above max range of UInt (namly 2^32)
			Int64 totalBytes = (Int64)(cube->Width()) * (Int64)(cube->Height()) * (Int64)(bytesPrPixel);
			
			//reset the tiles
			nTiles = -1;
			nAllocTiles = -1;

			if ( tiles != NULL ) delete[] tiles;
			tiles = NULL;
			
			//the array of tile queues should have the length of the number of cpus
			//default is one if OMP is not enabled
			nQueues = 1;

			//it is safe to delete the queues since we have already deleted the tiles and therefore
			// cannot end up in a situation where we leak memory
			if ( allocatedTiles != NULL ) delete[] allocatedTiles;
			allocatedTiles = NULL;

			UInt tile_min_width = 0;
			UInt tile_min_height = 0;
			//default for the number of threads
			maxThreads = 1;
	#ifdef USE_OPENMP
			maxThreads = omp_get_num_procs();
	#endif
			switch( tileSize )
			{
			case TileControl::tileSizeSmall:
				tile_min_width = ImageTile::tile_small_min_width;
				tile_min_height = ImageTile::tile_small_min_height;
				//important to set this to be != 1 since distributeTiles() then
				// makes the number of currently allocated tiles increase
				nAllocTiles = 0;
				break;
			case TileControl::tileSizeLarge:		
				tile_min_width = cube->Width();			

				tile_min_height = (Int)(FitsMath::round( (Double)maxMemUsage /
										( (Double)( bytesPrPixel ) * tile_min_width ) ) );
					

				//simple reduncancy checking just in case
				if ( tile_min_width > cube->Width() )
					tile_min_width = cube->Width();

				if ( tile_min_height > cube->Height() )
					tile_min_height = cube->Height();

				//set this to one so that no more than one tile is allocated
				//at a time when the tiles are large
				nAllocTiles = 1;

				break;
			case TileControl::tileSizeImport:
				//We cannot use the full tile size which was used internally
				//in FL when operating on the image.
				//We have to reduce our resource usage when exporting the 
				//tiff file since some double buffering will occur
				tile_min_width = cube->Width();
				tile_min_height = TileControl::maxHeightImport;
				
				//set this to one so that no more than one tile is allocated
				//at a time when the tiles are large
				nAllocTiles = 1;
				
				maxThreads = 1;
				#ifdef USE_OPENMP
				maxThreads = omp_get_num_procs();
				#endif
				if ( tile_min_width > cube->Width() )
					tile_min_width = cube->Width();

				if ( tile_min_height > cube->Height() )
					tile_min_height = cube->Height();
				break;
			default:
				tile_min_width = ImageTile::tile_small_min_width;
				tile_min_height = ImageTile::tile_small_min_height;
				//important to set this to be != 1 since distributeTiles() then
				// makes the number of currently allocated tiles increase
				nAllocTiles = 0;
				break;
			}
			//either the image can be fully loaded into memory or
			//it has to be tiled up		
			
			tilingOK = distributeTiles( cube->Width(), cube->Height(), tile_min_width, tile_min_height,
				bytesPrPixel, totalBytes, &nTiles, &nAllocTiles, cube, plane );
			if ( !tilingOK )
				decreaseMaxMem();			
		}

#ifdef USE_OPENMP
		nQueues = maxThreads;
#endif
		allocatedTiles = new queue<Int>[1];//[nQueues];		
	}
}

/**
*	Does the statistics using the tiles
*	@param cube current ImageCube
*	@param stretched whether the statistics is stretched
*	@param *globalMin pointer to the resulting min value
*	@param *globalMax pointer to the resulting max value
*	@param *globalMean pointer to the resulting mean value
*	@param *globalMedian pointer to the resulting median value
*	@param *globalStdev pointer to the resulting stdev value
*	@param histogram the resulting histogram
*	@param *maxBinCount pointer to the resulting maximum bin value
*	@param stretch the current stretch
*	@param plane the current plane
*	@param previewController reference to the previewController passed so that
*	the preview may be generated at the same time as doing the statistics to save
*	time
*	@param doPreview flags whether the preview should be done on the flyw
*	@param flipped determines whether the preview should be flipped
*	@param *progressModel pointer to the progressModel. This is not a pretty way to do it
*	@return ImageTile::AllocOk if the function was able to handle the memory, ImageTile::AllocErr if not
*
*/

Int TileControl::doStatistics3( const ImageCube* cube, Bool stretched, Double* globalMin, Double* globalMax,
							   Double* globalMean, Double* globalMedian, Double* globalStdev,
							   Vector<Double>& histogram, Double* maxBinCount, Stretch& stretch,
							   const Plane plane, FitsLiberator::Modelling::PreviewController* previewController, 
							   Bool doPreview, Bool flipped, FitsLiberator::Modelling::ProgressModel* progressModel )
{
	*globalMin = DoubleMax;
	*globalMax = DoubleMin;
	*globalMean = 0;
	*globalMedian = 0;
	*globalStdev = 0;
	Int globalPixelCount = 0;	

	UInt width = 0;
	UInt height = 0;
	UInt bitDepth = cube->SizeOf(1,1);
	//temporary pointers for the pixels
	//this is an ugly hack and it should probably
	//be re-flowed in a new version
	Double* stretchedPixels = NULL;
	Void* rawPixels = NULL;
	char* nullPixels = NULL;
	//if the number of tiles is greater than 1 then we
	//should try and allocate the pixels locally
	if ( getNumberOfTiles() > 1 )
	{
		//Find the biggest tile
		for ( Int i = 0; i < getNumberOfTiles(); i++ )
		{
			ImageTile* t = getLightTile(i);
			if ( t->width > width ) width = t->width;
			if ( t->height > height ) height = t->height;
		}
		
		try
		{
			stretchedPixels = new Double [width * height];
			rawPixels = (Void*)new Byte[width*height*bitDepth];
			nullPixels = new char [width * height];		
		}
		catch ( std::bad_alloc ba )
		{
			if ( rawPixels != NULL ) delete[] reinterpret_cast<Byte*>(rawPixels);
			rawPixels = NULL;
			
			if ( nullPixels != NULL ) delete[] nullPixels;
			nullPixels = NULL;
			
			if ( stretchedPixels != NULL ) delete[] stretchedPixels;
			stretchedPixels = NULL;
			//if the allocation went bad then simply exit after cleaning
			return ImageTile::AllocErr;
		}
	}
	for ( Int i = 0; i < getNumberOfTiles(); i++ )
	{
		ImageTile* tile = NULL;

		//set the pointers
		if ( getNumberOfTiles() > 1 )
		{
			tile = &tiles[i];
			tile->rawPixels = rawPixels;
			tile->stretchedPixels = stretchedPixels;
			tile->nullPixels = nullPixels;
			//load the pixels
			cube->Read( plane.planeIndex, tile->getBounds(), tile->rawPixels, tile->nullPixels );
		}
		else
		{
			//if there is only one tile then we simply get the reference
			//to it since it was already allocated an loaded.
			tile = getTile( i, cube, plane, true );
		}
	

		//stretch the tile in parallel
		stretchTile_par( *tile, stretch, cube );
		//accumulate the range
		FitsStatisticsTools::getRange_par( tile->stretchedPixels, tile->width*tile->height,
			&globalPixelCount, globalMin, globalMax, globalMean, this->getNumberOfThreads() );

		//generate preview
		if ( doPreview )
		{
			previewController->prepareTile( *tile, flipped );
			if ( tile->isCurrent() )
			{
				previewController->zoomTile_par( *tile, flipped, this->getNumberOfThreads() );
			}
		}
		if ( progressModel != NULL )
		{
			progressModel->Increment();
			//if the user has hit cancel
			if ( progressModel->QueryCancel() )
			{							
				if ( rawPixels != NULL ) delete[] reinterpret_cast<Byte*>(rawPixels);
				rawPixels = NULL;
				
				if ( nullPixels != NULL ) delete[] nullPixels;
				nullPixels = NULL;
				
				if ( stretchedPixels != NULL ) delete[] stretchedPixels;
				stretchedPixels = NULL;
				return ImageTile::OperationCanceled;				
			}
		}
		//set the tile's pointers to null to make sure they are not deallocated
		if ( getNumberOfTiles() > 1 )
		{
			tile->rawPixels = NULL;
			tile->stretchedPixels = NULL;
			tile->nullPixels = NULL;
		}
		tile->locked = false;

	}

	//now only one thread exists and we can calculate the mean
	*globalMean = *globalMean / globalPixelCount;
	
	//reset histogram
	for ( Int i = 0; i < histogram.size(); i++ )	
		histogram[i] = 0.;
	
	//the number of bins
	Int binCount = histogram.size();
	//the width of each bin in the histogram
	Double invBinSize =  (binCount - 1.) / (*globalMax - *globalMin);

	*maxBinCount = 0.;

	//do the histogram and mean
	for ( Int i = 0; i < getNumberOfTiles(); i++ )
	{
		//first get the tile
		ImageTile* tile = NULL;

		//set the pointers
		if ( getNumberOfTiles() > 1 )
		{
			tile = &tiles[i];
			tile->rawPixels = rawPixels;
			tile->stretchedPixels = stretchedPixels;
			tile->nullPixels = nullPixels;		
			//load the pixels
			cube->Read( plane.planeIndex, tile->getBounds(), tile->rawPixels, tile->nullPixels );
		}
		else
		{
			tile = getTile( i, cube, plane, true );
		}
		
		
		//stretch the tile in parallel
		stretchTile_par( *tile, stretch, cube );
		
		//get the histogram
		FitsStatisticsTools::getHistogram_par( tile->stretchedPixels, tile->width * tile->height,
											globalStdev, *globalMean, *globalMin, 
											invBinSize, histogram, this->getNumberOfThreads() );
		
		if ( progressModel != NULL )
		{
			progressModel->Increment();
			if ( progressModel->QueryCancel() )
			{
				//remember to clean up!
				if ( rawPixels != NULL ) delete[] reinterpret_cast<Byte*>(rawPixels);
				rawPixels = NULL;
				
				if ( nullPixels != NULL ) delete[] nullPixels;
				nullPixels = NULL;
				
				if ( stretchedPixels != NULL ) delete[] stretchedPixels;
				stretchedPixels = NULL;
				
				return ImageTile::OperationCanceled;				
			}
		}

		//set the tile's pointers to null to ensure the main buffers are not deallocated
		if ( getNumberOfTiles() > 1 )
		{
			tile->rawPixels = NULL;
			tile->stretchedPixels = NULL;
			tile->nullPixels = NULL;		
		}
		tile->locked = false;
	}

	*globalStdev = FitsMath::squareroot( 1./((Double)globalPixelCount) * (*globalStdev) );


	FitsStatisticsTools::scaleHistogram( histogram, globalMedian, *globalMin, *globalMax, 
		maxBinCount, globalPixelCount );

	//clean up
	if ( rawPixels != NULL ) delete[] reinterpret_cast<Byte*>(rawPixels);
	if ( nullPixels != NULL ) delete[] nullPixels;
	if ( stretchedPixels != NULL ) delete[] stretchedPixels;

	return ImageTile::AllocOk;
	
}

/**
Returns true if the image is small enough to make a thumb. False if not
Actually obsolete since v. 3.0 since Fl is not a PS plugin anymore.
However, the functionality may be re-used if FL is made into
a thumbnail creator in, e.g., Windows.
*/
Bool TileControl::canProduceThumb( const ImageCube* cube )
{	 
	assert(cube != 0);
	
	// Figure out how many bytes we will need:
	// total = sizeof(pixels) + sizeof(stretched) + sizeof(null_map)
	UInt bytesNeeded	= cube->SizeOf(0) + 
		cube->PixelsPerPlane() * (sizeof(double) + sizeof(unsigned char));
	
	if ( bytesNeeded > 100000000 ) //maxMemUsage )
	{
		return false;
	}
	return true;	
}

/**
Stretches the raw pixels on a single thread
*/
Void TileControl::stretchTile( ImageTile& tile, Stretch& stretch, const ImageCube* cube )
{
	if ( tile.stretched == false || tile.stretch != stretch )
	{
		if ( tile.isAllocated() )
		{
			FitsEngine::stretch( stretch, cube->Format(), (Void*)(tile.rawPixels),
				(Byte*)tile.nullPixels, tile.stretchedPixels, tile.width*tile.height, 1 );
			tile.stretched = true;
			tile.stretch = stretch;
		}
		else
		{
			throw Exception("Tried to stretch no allocated pixels");
		}
	}
}

/**
Stretches the raw pixels in parallel
*/
Void TileControl::stretchTile_par( ImageTile& tile, Stretch& stretch, const ImageCube* cube )
{
	if ( tile.stretched == false || tile.stretch != stretch )
	{
		if ( tile.isAllocated() )
		{
			FitsEngine::stretch( stretch, cube->Format(), (Void*)(tile.rawPixels),
				(Byte*)tile.nullPixels, tile.stretchedPixels, tile.width*tile.height, this->getNumberOfThreads() );
			tile.stretched = true;
			tile.stretch = stretch;
		}
		else
		{
			throw Exception("Tried to stretch no allocated pixels");
		}
	}
}

Int TileControl::getNumberOfThreads()
{
	return maxThreads;
}

Int TileControl::getTileStrategy()
{
	return this->oldTileSize;
}

ImageTile* TileControl::getTiles()
{
	return this->tiles;
}

/**
Used to flush the tiles when a new image is selected
*/
Void TileControl::deallocateTiles()
{
	if ( nTiles > 0 && tiles != NULL )
	{
		for ( Int i = 0; i < nTiles; i++ )
			tiles[i].deallocatePixels();
	}
}
