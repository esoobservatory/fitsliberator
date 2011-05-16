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

#ifndef __TILECONTROL_H__
#define __TILECONTROL_H__

#include "FitsLiberator.h"
#include "Stretch.h"
#include "ImageTile.h"
#include "Plane.h"
#include "ImageReader.hpp"
#include "FitsEngine.h"
#include "FitsStatisticsTools.h"
#include "TilePusher.h"
#include <queue>
#include "TextUtils.h"
#include "Environment.h"
#include "PreviewController.h"
#include "ProgressModel.h"

namespace FitsLiberator
{
	namespace Engine
	{
		/**
		*	Implements the class for storing and handling
		*	the image tiles containing the current fits/pds
		*	image information.
		*/
		class TileControl
		{		
		public:
			TileControl( UInt mmmUsg );
			~TileControl();
		
            ImageTile* getTile( const Int tile, const ImageCube* cube, 
				const Plane& plane, const Bool lock );

			ImageTile* getLightTile( const Int tile );

			Void reTile( const ImageCube* cube, const Int, const Plane& plane );

			//stretches the pixel of a specific tile on a single thread
			Void stretchTile( ImageTile& tile, Stretch& stretch, const ImageCube* cube );

			//stretches the pixel of a specific tile in parallel
			Void stretchTile_par( ImageTile& tile, Stretch& stretch, 
				const ImageCube* cube );

			//perform statistics in a tile-parallel way

			Int doStatistics3( const ImageCube* cube, Bool stretched, Double* globalMin,
								Double* globalMax,Double* globalMean, Double* globalMedian,
								Double* globalStdev,Vector<Double>& histogram, Double* maxBinCount,
								Stretch& stretch, const Plane plane, FitsLiberator::Modelling::PreviewController* previewController,
								Bool doPreview, Bool flipped, FitsLiberator::Modelling::ProgressModel* progressModel );

			const Int getNumberOfTiles();

			/**Returns the maximum number of threads usable for OMP*/
			Int getNumberOfThreads();

			/**Get the current tile strategy */
			Int getTileStrategy();

			Void deallocateTiles();

			Void decreaseMaxMem();

			Bool canProduceThumb( const ImageCube* cube );

			ImageTile* getTiles();

			static const Int tileSizeLarge = 0;
			static const Int tileSizeSmall = 1;
			static const Int tileSizeImport = 2;
			
		private:
			//pointer to the image tiles
			ImageTile* tiles;
			//pointer to the old image tiles
			ImageTile* oldTiles;
			//the number of total tiles 
			Int nTiles;
			//the number of allocated tiles at a time (for each thread)
			Int nAllocTiles;					
			//The total number of allocatable tiles
			Int nMaxAllocTiles;

			//max number of threads
			Int maxThreads;
			
			
			//queue with the indices of the currently allocated tiles
			std::queue<Int>* allocatedTiles;

			//the number of queues of allocated tiles
			Int nQueues;
			
			//save these values so re-tiling of the same image is not necessary
			Int oldCubeWidth;
			Int oldCubeHeight;
			Int oldTileSize;			

			//test if two tiles are overlapping
			Bool tilesOverlap( ImageTile& tile1, ImageTile& tile2 );

			Bool distributeTiles( const Int imgWidth, const Int imgHeight, 
								  const Int minWidth, const Int minHeight, const Int bytesPrPixel,
								  Int64 totalBytes, Int* nTiles, Int* nAllocTiles, 
								  const ImageCube* cube, const Plane& plane ); 
			
			/**Returns the number of tiles totally allocated  currently*/
			Int getNCurrentlyAllocated( );


			static const Int tilePolicy = 2;
			//maximum amount of lines in the image imported
			//at a time when exporting to TIFF
			static const Int maxHeightImport = 1000;

			UInt maxMemUsage;
			UInt oldMaxMemUsage;


		};
	}
}

#endif