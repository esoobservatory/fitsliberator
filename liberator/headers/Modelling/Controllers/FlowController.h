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
#ifndef __FLOWCONTROLLER_H__
#define __FLOWCONTROLLER_H__


#include "FitsLiberator.h"
#include "AccumulatingChangeManager.h"
#include "Stretch.h"
#include "PreviewModel.h"
#include "StretchModel.h"
#include "StatisticsModel.h"
#include "HistogramModel.h"
#include "PixelValueModel.h"
#include "GlobalSettingsModel.h"
#include "HistogramController.h"
#include "StatisticsController.h"
#include "PreviewController.h"
#include "Image.h"
#include "Plane.h"
#include "FitsEngine.h"
#include "ImageReader.hpp"
#include "FitsStatisticsTools.h"
#include "ImportSettings.h"
#include "ImageTile.h"
#include "PlaneModel.h"
#include "PlaneController.h"
#include "TileControl.h"
#include "OptionsModel.h"
#include "ProgressModel.h"
#include "WcsMapper.hpp"
#include "FitsSession.h"
#include "FileLoader.h"
#include "RepositoryModel.h"
#include "Preferences.h"
#include "RepositoryController.h"
#include "Preferences.h"
#include <boost/scoped_ptr.hpp>

namespace FitsLiberator
{
	namespace Modelling
	{	
		/**
		Encapsulates a given state of the flow controller so that 
		it can be rolled back if the user cancels
		*/
		struct FlowControllerState
		{
		public:
			FlowControllerState( UInt, UInt );
			~FlowControllerState();
			Double* histBins;
			UInt nBins;
			PreviewImage* previewImage;
			Double realMin;
			Double realMax;
			Double realStdev;
			Double realMean;
			Double realMedian;

			Double sclMin;
			Double sclMax;
			Double sclStdev;
			Double sclMean;
			Double sclMedian;

			Double strMin;
			Double strMax;
			Double strStdev;
			Double strMean;
			Double strMedian;
			
			FitsLiberator::Engine::StretchFunction	function;
			Double									background;
			Double									backgroundIncrementation;
			Double									scale;
			Double									scaleIncrementation;
			Double									peakLevel;
			Double									rescaleFactor;
			Double									scaleBackground;

			Double maxBinCount;
			Stretch stretch;
			Plane plane;

			
		};
		/**
		Super controller that is the main gateway between the GUI
		and the engine/model framework code. All entries into this
		class takes care of the specific logic and passes on the tasks
		to the various controllers, statistics tools, engine etc.
		
		This may seem a quite monolithic implementation, however, 
		in this way a single point of taking care of cross-controller
		and cross-model talk is made. In versions before 2.3 the controllers
		and models could talk in many obscure ways with each other. 
		
		This is thus not possible anymore with the 
		FlowController and the code is thus much more transparent
		*/
		class FlowController : public ACMController
		{
		public:
			/**
			The constructor basically gets all the models and controllers
			*/
			FlowController( FitsLiberator::Engine::ImageReader* f, 
				HistogramModel& hM, StretchModel& sM, 
				StatisticsModel& stM, PreviewModel& pM, PlaneModel& plM, PixelValueModel& pxvM,
				GlobalSettingsModel& gbsM, OptionsModel& optModel, ProgressModel& prgModel, 
				RepositoryModel& repModel, HistogramController& hCtrl,
				StretchController& strCtrl, StatisticsController& stCtrl,
				PreviewController& prvCtrl, PlaneController& plCtrl, FitsLiberator::Engine::TileControl&, 
				RepositoryController& repControl, AccumulatingChangeManager* chman,
				FitsLiberator::FitsSession& session );
			~FlowController();

			/**Method to call when the user has tried to change
			the image (either the plane or image index)
			@param imageIndex the index of the current image
			@param planeIndex the index of the current plane
			@newFile is passed when the call is done due to a change in actual
			input file and not just change in the imageIndex or planeIndex.
			*/
			Void imageChanged( Int imageIndex, Int planeIndex, Bool newFile );
			
			/*--------------------------------------------------------------------
			Methods concerned with the stretching
			--------------------------------------------------------------------*/

			/**
			Method to call when the user changes the stretch function
			*/
			Void stretchFunctionSelected( const FitsLiberator::Engine::StretchFunction stretchFunction );

			/**
			Called when the user tries to change the background level
			*/
			Void setBackgroundLevel( Double level );
			
			/**
			Called when the user tries to change the peak level
			*/
			Void setPeakLevel( Double level );

			/**
			Called when the user tries to change the scaled peak level
			*/
			Void setRescaleFactor( Double level );

			/**
			Performs the background level guess based on the clicked pixel
			*/
			Void backgroundGuess( const FitsLiberator::Point& p );

			/**
			Performs the peak level guess based on the clicked pixel
			*/
			Void peakGuess( const FitsLiberator::Point& p );

			/**
			Called when the user hits the auto-background button
			*/
			Void automaticBackground();

			/**
			Called when the user hits the auto-scale button
			*/
			Void automaticScale();

			/**
			Called when the user hits the auto-background scale
			*/
			Void automaticBackgroundScale();

			/**
			Sets the default values
			*/
			Void defaultValues();

			/**
			Set both the background, peak and scaled peak levels at once			
			*/
			Void setBackgroundPeakScaledPeakLevels( Double bg, Double pl, Double sPl );

			/*--------------------------------------------------------------------
			Methods concerned with the preview
			--------------------------------------------------------------------*/
			/**
			Zooms a specific rectangle within the coordinatesystem of the preview
			image
			*/
			Void zoomRectangle( FitsLiberator::Rectangle& rect );

			/**
			Picks the black level
			*/
			Void pickBlackLevel( const FitsLiberator::Point& p );

			/**
			Picks the white level
			*/
			Void pickWhiteLevel( const FitsLiberator::Point& p );

			/**
			Sets the current coordinates of the mouse and updates the
			corresponding pixel values
			*/
			Void setCoordinates( const FitsLiberator::Point& p );

			/**
			Sets the zoom to the best-fit and centers the preview image
			*/
			Void fitToPreview();

			/**
			Set zoom to 100%
			*/
			Void setUnityZoom();

			/**
			Centers the preview
			*/
			Void centerPreview();

			/**
			Increments the zoom factor
			*/
			Void incrementZoom( FitsLiberator::Point p );

			/**
			Decrements the zoom factor
			*/
			Void decrementZoom( FitsLiberator::Point p );
			/**
			Sets a specific zoom index
			*/
			Void setZoomIndex( Int index );

			/**
			Moves the preview with the given vector
			*/
			Void movePreview( MovementVector vec );

			/**sets the bitdepth*/
			Void setBitDepth( FitsLiberator::Engine::ChannelSettings chs );

			/**sets undefined*/
			Void setUndefined( FitsLiberator::Engine::UndefinedSettings us );

			/**toggles flip*/
			Void toggleFlip();

			/**resets flip*/
			Void resetFlip();

			/**opens the save as dialog and the tiff file may be exported*/
			Void saveFile( Bool invokeEditor, String fileName );

			/**Makes a FitsSession*/
			Void makeSession( FitsSession* session );

			/**Not particularly pretty but somehow it seems necessary and easier
			than passing the pointer to any call
			This is actually rather an expression for the fact that ModelFramework
			and FlowController should be merged (since 3.0)*/
			Void updateReader( FitsLiberator::Engine::ImageReader* r );

			/**Applies the stored preferences*/
			Void applyPreferences();
			/**Store preferences into the prefs. file*/
			Void storePreferences(Bool isOkPressed);

			/**load metadata*/
			Void loadMetadata();
			
			/**Returns the preferences*/
			FitsLiberator::Preferences::Preferences& getPreferences();

			//-------------------------------------
			//Methods for the histogram
			//-------------------------------------

			Void incrementBlackLevel( Bool, Bool );
			Void setBlackLevel( Double bl );
			Void incrementWhiteLevel( Bool, Bool );
			Void setWhiteLevel( Double wl );
			Void applyLevels();
			Void showRange(UInt left, UInt right );
			Void incrementHistogramZoom( UInt coord );
			Void decrementHistogramZoom( UInt coord );
			Void showAllHistogram();
			Void moveHistogram( Int delta );
			Void moveWhiteLevelSlider( Int pos );
			Void moveBlackLevelSlider( Int pos );
			
		protected:

		private:	
            /** Called in the beginning of a long running operation. */
            Bool Begin();
            /** Called at the end of a long running operation. */
            void End();
			//Method for doing the statistics
			Int doStatistics( const FitsLiberator::Engine::ImageCube* cube, Bool stretched, Bool doPreview );
			//Method for allocating a specific tile
			Int getTile( Int tile, const FitsLiberator::Engine::ImageCube* cube, std::queue<Int>& allocatedTiles );
			//Collect the stretch object
			Stretch& getStretch();
			//do the initial guess
			Void initialGuess();
			//converts to linear values
			Void getLinearLevels( Double* blackLevel, Double* whiteLevel );
			//Sets the scale factor of the image
			Int setScale( Double scl );
			//when things have been set this method is called to control the generation the preview
			Void makePreview( const FitsLiberator::Engine::ImageCube* cube );
			Void makePreview2( const FitsLiberator::Engine::ImageCube* cube );
			//make the guess for the background or peak level
			Double makeGuess( const FitsLiberator::Point& p );
		
			//internal defaultValues function
			Void defaultValues_();
			//internal setBackgroundLevel function
			Void setBackgroundLevel_( Double level );
			//Internal function for the peakLevel
			Void setPeakLevel_( Double d );
			//Internal auto background and scale function
			Void automaticBackgroundScale_();
			//Internal scale value function
			Void setRescaleFactor_( Double d );
			//Internal stretchFunction selecter
			Void stretchFunctionSelected_( const StretchFunction f );
			//Internal imageChanged function
			Void imageChanged_( Int imageIndex, Int planeIndex, Bool newFile );
			//Internal toggleFlip
			Void toggleFlip_();
			//Internal flip resetter
			Void resetFlip_();
			//Internal zoom rectangle function
			Void zoomRectangle_( FitsLiberator::Rectangle& rect );
			//Internal fitToPreview function
			Void fitToPreview_();
			Void saveFile_( String file, Bool invokeEditor );
			//Internal unity zoom
			Void setUnityZoom_();
			Void centerPreview_();
			Void incrementZoom_( FitsLiberator::Point p );
			Void decrementZoom_( FitsLiberator::Point p );
			Void setZoomIndex_( Int index );
			Void movePreview_( MovementVector vec );
			Void setBackgroundPeakScaledPeakLevels_( Double bg, Double pl, Double sPl );
			/**The preview is flushed without loading and stretching pixels */
			Void flushPreview();
			/**Saves the current state for use when rolling back due to a cancel request */
			Void saveState();
			/**Used when the user cancels the operation to revert to the initial state before the begning of the operation*/
			Void rollBackState();

			//the current stretch
			FitsLiberator::Engine::Stretch stretch;

			//The models
			HistogramModel& histogramModel;
			StretchModel& stretchModel;
			StatisticsModel& statisticsModel;
			PreviewModel& previewModel;
			PlaneModel& planeModel;
			PixelValueModel& pixelValueModel;
			GlobalSettingsModel& globalSettingsModel;
			OptionsModel& optionsModel;
            ProgressModel& progressModel;
			RepositoryModel& repositoryModel;

			//the controllers
			HistogramController& histogramController;
			StretchController& stretchController;
			StatisticsController& statisticsController;
			PreviewController& previewController;
			PlaneController& planeController;
			RepositoryController& repositoryController;
			//reference to the fits file
			//const FitsLiberator::Engine::ImageReader& imageReader;
			FitsLiberator::Engine::ImageReader* imageReader;

			//The tiling subsystem
			FitsLiberator::Engine::TileControl& tileControl;
			
            boost::shared_ptr<WcsMapper> wcs;

			//the saved state
			FlowControllerState* currentState;
			
			//reference to the general session
			FitsLiberator::FitsSession& session;
			//storage of the preferences pointer
			FitsLiberator::Preferences::Preferences* prefs;
			
		};
	}
}


#endif