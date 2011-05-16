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
#ifndef __PreviewController_H__
#define __PreviewController_H__

#include "FitsLiberator.h"
#include "AccumulatingChangeManager.h"
#include "PreviewModel.h"
#include "PlaneModel.h"
#include "FitsEngine.h"
#include "FitsMath.h"
#include "GlobalSettingsModel.h"
#include "ImageTile.h"

namespace FitsLiberator
{
	namespace Modelling
	{
		/**
		*	Implements the handling of the preview
		*	like zooming, panning and so on.
		*/
		class PreviewController
		{
		public:			
			
			PreviewController( PreviewModel&, const GlobalSettingsModel& );
			
			~PreviewController();

			
			Void movePreview( MovementVector&, const FitsLiberator::Size& rawSize );//> Moves the preview in the direction and by the amount specified by the argument
			Void incrementZoom(const FitsLiberator::Size&, 
				FitsLiberator::Point&, Bool flip );		//> increments the zoom if doable
			Void decrementZoom(const FitsLiberator::Size&, 
				FitsLiberator::Point&, Bool flip );		//> decrements the zoom if doable
			Void fitToPreview( const FitsLiberator::Size& rawSize, Bool flip );	//> Scales the raw image so it fits the preview frame
			Void zoomRectangle( FitsLiberator::Rectangle&, 
				const FitsLiberator::Size& rawSize, Bool flip );//> Zooms the image to the rectange specified up to the best matching zoomfactor
			Void scaleDynamicRange( Double bl, Double wl);///> rescales the image in the current dynamic range [blacklevel:whitelevel]
			Double getPixelGuess( const FitsLiberator::Point&, FitsLiberator::Size& rawSize, 
				const FitsLiberator::Engine::Stretch& stretch, const Bool flip );

			Void setZoomIndex( Int f, const FitsLiberator::Size& rawSize, Bool flip );
			Void setUnityZoom( const FitsLiberator::Size&, Bool flip );
            Void centerPreview( const FitsLiberator::Size& );
			

			//Methods for dealing with the tiling
			//Prepares a tile, i.e. defines if it is in range or not
			Void prepareTile( ImageTile& tile, Bool flipped );
			//zooms the part of the preview that corresponds to the given tile
			Void zoomTile( ImageTile& tile, Bool flipped );
			Void zoomTile_par( ImageTile& tile, const Bool flip, UInt nThreads );
			//sets the currently covered part of the preview
			Void setImageArea( const FitsLiberator::Size& rawSize );

			Double getRealValueAtPos( const FitsLiberator::Point&, FitsLiberator::Size& rawSize, 
				const FitsLiberator::Engine::Stretch& stretch, const Bool flip );
			Double getStretchedValueAtPos( const FitsLiberator::Point& );
			Double getScaledValueAtPos( const FitsLiberator::Point& p, FitsLiberator::Size& s,
				const FitsLiberator::Engine::Stretch& str, const Bool flip );
			
			Void makeBorder(FitsLiberator::Rectangle&, Double bl );
		private:

			Void doZoom(const FitsLiberator::Size&, FitsLiberator::Point&, Bool incr, Bool flip );

			
			PreviewModel&						previewModel;
			
			const GlobalSettingsModel&				globalSettingsModel;
			
			Int startX;
			Int startY;
			Int endX;
			Int endY;
			Bool moved;
		};
	}
}

#endif 