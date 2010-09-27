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
#ifndef __PreviewModel_H__
#define __PreviewModel_H__

#include "FitsLiberator.h"
#include "Observer.h"
#include "Image.h"
#include "FitsMath.h"
#include "GlobalSettingsModel.h"
#include "PreviewCacheHandler.h"


using namespace FitsLiberator::Caching;

namespace FitsLiberator
{
	namespace Modelling
	{
		struct MovementVector
		{
			Int x;		///> The movement in the x-direction with positive to the right
			Int y;		///> The movement in the y-direction with positive downwards

            MovementVector( Int x, Int y );
		};
		
		
		/**
		 *
		 */
		class IndexOutOfBoundsException : public FitsLiberator::Exception {
			public:
				IndexOutOfBoundsException( UInt i ) : index( i ) {};
				UInt getIndex() { return index;	};
			
			private:
				UInt index;
		};
		
		/**
		*	This struct contains all the relevant info
		*	for rendering the preview image.
		*/
		struct PreviewImage
		{
			Byte*				previewPixels;		///> the pixels (grayscale) to be rendered
			Double *			rawPixels;			///> the original pixels corresponding to the rendered pixels
			Byte*				whiteClippedMap;	///> map of which pixels should be drawn with the colour of the white clipped pixels
			Byte*				blackClippedMap;	///> map of which pixels should be drawn with the colour of the black clipped pixels
			Byte*				nullMap;			///> map of which pixels should be drawn with the colour of the null (undefined) valued pixels
			
			FitsLiberator::Size size;			///> the dimensions of the image

			PreviewImage(UInt,UInt);			///> takes the size of the image as a parameter, so the vectors can be resized to fit this
			~PreviewImage();
		};
		/**
		*	Implements the model of the preview
		*	Mainly does storage but also a few
		*	low-level logics
		*/
		class PreviewModel : public Model
		{
		public:
			PreviewModel( ChangeManager * chman, GlobalSettingsModel&);
			~PreviewModel();
			Double		getZoomFactor();
			Void		incrementZoomFactor();
			Void		decrementZoomFactor();
			Bool		canIncrement();
			Bool		canDecrement();
			
			FitsLiberator::Size&			getPreviewSize();
			
			PreviewImage&					getPreviewImage();

			FitsLiberator::Rectangle&		getImageArea();
			FitsLiberator::Rectangle&		getImageSegment();

			Double*							getZoomFactors();
            Int                             getZoomFactorCount();

			Double							getZoomFitFactor();
			Double							getMaxZoomFactor();

            Int                             getUnityZoomIndex();
			Int								getZoomIndex();
			Int								getZoomFitPreviewIndex();
			Int								getNumberOfZoomFactors();

			Void							setUnityZoomIndex();
			Void							setZoomFitIndex();
			Void							setZoomFitFactor();
			Void							generateZoomFit(FitsLiberator::Size&);		///> generates the zoom values based on the current data
			Void							setPreviewSize(FitsLiberator::Size&);
			Void							setZoomFactor(Double);
			

			FitsLiberator::Point*			getAnchorPoint();
			FitsLiberator::Point			getImageCoordinates( const FitsLiberator::Point&, Bool flip );
			

			Bool 							useCache( Stretch& stretch, Plane& plane, 
													  FitsLiberator::Point& anch, Double zoom, Double* pixels,
													  Int size, Bool b );

			Void 							storeCache( Stretch& stretch, Plane& plane, 
														FitsLiberator::Point& anch, Double zoom, 
														Double* pixels, Int size, Bool flip );

			Void							clearCache();

			FitsLiberator::Size& getRawSize();
		private:
            bool busy;                              ///< Is the preview busy?
			//misc values for the resampling
			Double* zoomFactors;					///> The span of zoom factors
			
			Double* staticZoomFactors;				///> contains all the static zoom factors
		
			UInt   zoomIndex;						///> Index of current zoom factor in zoomFactors.
			
			Int zooms;								///> defines the amount of zoom factors available
			Int unityIndex;
			Int zoomFitIndex;						///> contains the index closest to the zoom fit value in the static list.
			
			Int zoomFitPreviewIndex;				///> the index of the zoom values, which is the zoom fit value.

			FitsLiberator::Size rawSize;
			
			FitsLiberator::Point* currentAnchor;	//> the anchor point in the raw image from which the upper left corner should emerge.

			FitsLiberator::Rectangle imageOfPreview;///> the rect of the entire preview area, which is actual image data.
			FitsLiberator::Rectangle imageSegment;	///> The segment of the actual image that is showed in the preview

			PreviewImage* previewImage;				///> Representation of the image pixels.
			
			GlobalSettingsModel& globalSettingsModel;

			PreviewCacheHandler* cacheHandler;

			friend class PreviewController;
		};
	}
}

#endif