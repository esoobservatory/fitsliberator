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
#include "FlowController.h"
#include "omp.h"
#include <time.h>
#include "TextUtils.h"
#include "Environment.h"
#include "TilePusher.h"

#include <boost/thread.hpp>
#include <boost/bind.hpp>

using namespace FitsLiberator::Modelling;
using namespace FitsLiberator;
using namespace FitsLiberator::Engine;
using namespace FitsLiberator::Preferences;

/**
Implementation of the FlowControllerState struct
*/
FlowControllerState::FlowControllerState( UInt w, UInt h )
{
	previewImage = new PreviewImage( w, h );

	histBins = NULL;
	
	realMin = 0.;
	realMax = 0.;
	realMedian = 0.;
	realMean = 0.;
	realStdev = 0.;

	strMin = 0.;
	strMax = 0.;
	strMedian = 0.;
	strMean = 0.;
	strStdev = 0.;

	sclMin = 0.;
	sclMax = 0.;
	sclMedian = 0.;
	sclMean = 0.;
	sclStdev = 0.;
}

FlowControllerState::~FlowControllerState()
{
	if ( previewImage != NULL ) delete previewImage;
	if ( histBins != NULL ) delete histBins;
}



/*-------------------------------------
Code for the implementation of FlowController
-------------------------------------*/


FlowController::FlowController(
    ImageReader* f, 
    HistogramModel& hM, StretchModel& sM, 
	StatisticsModel& stM, PreviewModel& pM, PlaneModel& plM, PixelValueModel& pxM,
	GlobalSettingsModel& gbsM, OptionsModel& optModel, ProgressModel& prgModel,
	RepositoryModel& repM, HistogramController& hCtrl, StretchController& strCtrl,
	StatisticsController& stCtrl,PreviewController& prvCtrl, PlaneController& plCtrl, 
    TileControl& tileCtrl, RepositoryController& repControl,
    AccumulatingChangeManager* chman, FitsSession& sess )
  :
				imageReader( f ), histogramModel( hM ), stretchModel( sM ),
				statisticsModel( stM ), previewModel( pM ), planeModel( plM ), pixelValueModel( pxM ),
				globalSettingsModel( gbsM ), optionsModel( optModel ), progressModel( prgModel ),
                histogramController( hCtrl ),
				stretchController( strCtrl ), statisticsController( stCtrl ),
				previewController( prvCtrl ), planeController( plCtrl ), ACMController( chman ),
				tileControl( tileCtrl ), session( sess ), repositoryModel( repM ),
				repositoryController(repControl)
{
	currentState = NULL;
	prefs = new FitsLiberator::Preferences::Preferences(Environment::getPreferencesPath());
}

FlowController::~FlowController()
{
	if ( currentState != NULL ) delete currentState;
	if ( prefs != NULL ) delete prefs;
}

Void FlowController::imageChanged( Int imageIndex, Int planeIndex, Bool newFile )
{
	//first clear the cache
	if ( newFile )
	{
		previewModel.clearCache();
		statisticsModel.clearCache();
	}
	//get the FitsImage
	const ImageCube* cube = (*imageReader)[imageIndex];
	//generate the zooms
	Size size;
	size.width = cube->Width();
	size.height = cube->Height();
	previewModel.generateZoomFit( size );
	
	if ( size.getArea() >= (Double)kFITSHistogramBins )
		histogramModel.setNumberOfBins( kFITSHistogramBins );
	else
		histogramModel.setNumberOfBins( size.getArea() );

	if ( Begin() )
	{
		
		boost::thread worker(boost::bind(&FlowController::imageChanged_, this, imageIndex, 
			planeIndex, newFile ));
	}
}
Void FlowController::imageChanged_( Int imageIndex, Int planeIndex, Bool newFile )
{
	//if the user opened a new file FL cannot support the cancel operation
	//otherwise it is business as usual
	progressModel.CanCancel(!newFile);
	if ( !newFile )
		saveState();	
	//update the plane
	if ( planeModel.updateModel( imageIndex, planeIndex ) )
		tileControl.deallocateTiles();

	//get the FitsImage
	const ImageCube* cube = (*imageReader)[imageIndex];
	
    wcs.reset(new WcsMapper(cube));
	Stretch defStr;	
	
	//Generate preview
	Size size;
	size.width = cube->Width();
	size.height = cube->Height();

	previewController.fitToPreview( size, planeModel.getFlipped().flipped );
	Bool doPreview = false;
	if ( ! ( defStr != getStretch() ) ) 
	{
		doPreview = true;
		progressModel.SetMax( 100 );
	}
	else
		progressModel.SetMax( 50 );
	
	progressModel.Reset();
	
	//Perform default statistics based on the cache strategy
	Bool state = doStatistics( cube, false, doPreview );
	if (  state == ImageTile::OperationCanceled )
	{
		if ( !newFile )
			rollBackState();
		else
			End();

		return;
	}
	
	
	//Perform stretched statistics
	//if the stretch is equal to the default then don't do the stats again
	if ( defStr != getStretch() )
	{
		progressModel.SetMax( 100 );
		if ( doStatistics( cube, true, true ) == ImageTile::OperationCanceled )
		{
			if ( !newFile )
				rollBackState();
			else
				End();

			return;
		}
	}
	else
		statisticsController.useScaledAsStretchedValues();
	
	if ( ! ( globalSettingsModel.getFreezeSettings() ) && !( globalSettingsModel.getSessionLoaded() ) )
		initialGuess();

	//update the histogram
	histogramController.updateHistogram( statisticsModel.getStretchMin(), statisticsModel.getStretchMax() );


	

	tileControl.reTile( cube, TileControl::tileSizeSmall, planeModel.getPlane() );
	//makePreview( cube );
	flushPreview();

	
	End();
}
	

Void FlowController::flushPreview()
{
	Stretch& stretch = getStretch();
	Plane& plane = planeModel.getPlane();
	PreviewImage& previewImage			= previewModel.getPreviewImage();
	FitsLiberator::Point& anchor		= *(previewModel.getAnchorPoint());
	Double zoomFactor					= previewModel.getZoomFactor();
	previewModel.storeCache( stretch, plane, anchor, zoomFactor,previewImage.rawPixels,previewImage.size.getArea(),
								 planeModel.getFlipped().flipped );
	//scale the preview pixels to the 0-255 range
	previewController.scaleDynamicRange( histogramModel.getBlackLevel(), histogramModel.getWhiteLevel() );
    

	planeModel.Notify();


	SendNotifications();
}

Void FlowController::setBitDepth( FitsLiberator::Engine::ChannelSettings chs )
{

}

Void FlowController::setUndefined( FitsLiberator::Engine::UndefinedSettings us )
{

}

Void FlowController::updateReader( FitsLiberator::Engine::ImageReader* r )
{
	this->imageReader = r;
}

Void FlowController::setCoordinates( const FitsLiberator::Point& p )
{
	const Bool flipped = this->planeModel.getFlipped().flipped;

	const FitsLiberator::Point& point = previewModel.getImageCoordinates( p, flipped );

	const Plane& plane = planeModel.getPlane();

	//based on the cache strategy generate the preview
	const ImageCube* cube = (*imageReader)[plane.imageIndex];

	Double ra = 0;
	Double dec = 0;
    if(wcs)
        wcs->Map(point.x, point.y, &ra, &dec);

	Size rawSize;
	rawSize.width = cube->Width();
	rawSize.height = cube->Height();
	
	const Stretch& stretch = getStretch();

	Double real = previewController.getRealValueAtPos( p, rawSize, stretch, flipped );

	Double scl = previewController.getScaledValueAtPos( p, rawSize, stretch, flipped );

	Double str = previewController.getStretchedValueAtPos( p );

	pixelValueModel.setPosition( point, real, str, scl, ra, dec );
	SendNotifications();
}


/**
	Picks the black level 
*/
Void FlowController::pickBlackLevel( const FitsLiberator::Point& p )
{
	if ( globalSettingsModel.getPreviewEnabled() )
	{
		setBlackLevel( previewController.getStretchedValueAtPos( p ) );
	}
}
/**
	Picks the white level 
*/
Void FlowController::pickWhiteLevel( const FitsLiberator::Point& p )
{
	if ( globalSettingsModel.getPreviewEnabled() )
	{			
		setWhiteLevel( previewController.getStretchedValueAtPos( p ) );
	}
}

Void FlowController::backgroundGuess( const FitsLiberator::Point& p )
{
	setBackgroundLevel( makeGuess( p ) );
}

Void FlowController::peakGuess( const FitsLiberator::Point& p )
{
	setPeakLevel( makeGuess( p ) );
}

Double FlowController::makeGuess( const FitsLiberator::Point& p )
{
	const Bool flipped = planeModel.getFlipped().flipped;
	const Plane& plane = planeModel.getPlane();
	//based on the cache strategy generate the preview
	const ImageCube* cube = (*imageReader)[plane.imageIndex];

	Size rawSize;
	rawSize.width = cube->Width();
	rawSize.height = cube->Height();

	const Stretch& stretch = getStretch();

	Double guess = previewController.getPixelGuess( p, rawSize, stretch, flipped );

	return guess;
}

Void FlowController::toggleFlip()
{
	if ( Begin() )
	{
		boost::thread worker(boost::bind( &FlowController::toggleFlip_, this ));
	}
}

Void FlowController::toggleFlip_()
{
	const Plane& plane = planeModel.getPlane();
	const ImageCube* cube = (*imageReader)[plane.imageIndex];

	//the tilecontrol does not retile if the tiling is unchanged by this call
	tileControl.reTile( cube, TileControl::tileSizeSmall, planeModel.getPlane() );
	

	planeController.toggleFlip();
	//get the FitsImage
	
	makePreview( cube );
	End();

}

Void FlowController::resetFlip()
{
	if ( Begin() )
	{
		boost::thread worker(boost::bind( &FlowController::resetFlip_, this ));
	}
}

Void FlowController::resetFlip_()
{
	const Plane& plane = planeModel.getPlane();
	const ImageCube* cube = (*imageReader)[plane.imageIndex];
	//the tilecontrol does not retile if the tiling is unchanged by this call
	tileControl.reTile( cube, TileControl::tileSizeSmall, planeModel.getPlane() );


	planeController.resetFlip();
	//get the FitsImage
	
	makePreview( cube );
	End();
}

Void FlowController::initialGuess()
{
	Double bl = 0;
	Double wl = 0;
	Double min = statisticsModel.getStretchMin();
	Double max = statisticsModel.getStretchMax();
	Double mean = statisticsModel.getStretchMean();
	Double median = statisticsModel.getStretchMedian();
	Double stdev = statisticsModel.getStretchSTDEV();
	Vector<Double>& bins = histogramModel.getRawBins();
	FitsStatisticsTools::initialGuess( (InitialGuess)(optionsModel.GuessMethod()),
		optionsModel.BlackLevelPercentage(), 
		optionsModel.WhiteLevelPercentage(), &bl, &wl, min, max, mean, stdev, median, bins); 
	stretchModel.setRescaleFactor( wl );
	stretchModel.setPeakLevel( wl );
	stretchModel.setScale ( 1.0 );

	this->histogramModel.updateLevels( bl, wl );
}


Void FlowController::stretchFunctionSelected( const StretchFunction f )
{
	if ( f == stretchNoStretch )	
		return;

	if ( Begin() )
	{
		boost::thread worker(boost::bind(&FlowController::stretchFunctionSelected_, this, f));
	}
}

Void FlowController::stretchFunctionSelected_( const StretchFunction f )
{
	saveState();
	//if the user opened a new file FL cannot support the cancel operation
	//otherwise it is business as usual
	progressModel.CanCancel( true );
	const Plane& plane = planeModel.getPlane();
	//get the FitsImage
	const ImageCube* cube = (*imageReader)[plane.imageIndex];
	
	//the tilecontrol does not retile if the tiling is unchanged by this call
	tileControl.reTile( cube, TileControl::tileSizeLarge, planeModel.getPlane() );

	//First find the linear values based on the old stretch
	Double bl = histogramModel.getBlackLevel();
	Double wl = histogramModel.getWhiteLevel();
	Double rangeMin = histogramModel.getRangeMin();
	Double rangeMax = histogramModel.getRangeMax();

	Stretch stretch = getStretch();

	bl			= FitsEngine::getLinearVal( stretch, bl);
	wl			= FitsEngine::getLinearVal( stretch, wl);
	rangeMin	= FitsEngine::getLinearVal( stretch, rangeMin);
	rangeMax	= FitsEngine::getLinearVal( stretch, rangeMax);

	//sets the new stretch function
	stretchModel.setFunction( f );

	stretch = getStretch();
	
	Double rawPixels[4];
   Double out[4];	

	rawPixels[0] = bl;
	rawPixels[1] = wl;
	rawPixels[2] = rangeMin;
	rawPixels[3] = rangeMax;
	//stretch the values
	FitsEngine::stretchRealValues( stretch, rawPixels, out, 4 );
	
	progressModel.SetMax( 100 );
	
	progressModel.Reset();
	//perform statistics
	if ( doStatistics( cube, true, true ) == ImageTile::OperationCanceled )
	{
		rollBackState();
		return;
	}

	//update histogram
	if ( !FitsMath::isFinite( out[2] ) || !FitsMath::isFinite( out[3] ) )
	{
		this->histogramController.updateHistogram( statisticsModel.getStretchMin(), 
			statisticsModel.getStretchMax() );
	}
	else
	{
		this->histogramController.refreshHistogram( out[2], out[3], 
			statisticsModel.getStretchMin(), statisticsModel.getStretchMax() );
	}


	if ( !FitsMath::isFinite( out[0] ) || !FitsMath::isFinite( out[1] ) )
	{
		initialGuess();
	}
	else
	{
		this->histogramModel.updateLevels( out[0], out[1] );
	}
	
	//generate preview
	//makePreview( cube );
	flushPreview();
	//the tilecontrol does not retile if the tiling is unchanged by this call
	tileControl.reTile( cube, TileControl::tileSizeSmall, planeModel.getPlane() );
	

	End();
}

/**
Sets the background level (thread safe)

*/
Void FlowController::setBackgroundLevel( Double level )
{
	if ( Begin() )
	{
		boost::thread worker(boost::bind(&FlowController::setBackgroundLevel_, this, level));
	}
}

/**
Internal function for setting the background level
*/
Void FlowController::setBackgroundLevel_( Double level )
{
	saveState();
	//if the user opened a new file FL cannot support the cancel operation
	//otherwise it is business as usual
	progressModel.CanCancel( true );
//first we should back transfer the black and white levels
	Double bl = 0;
	Double wl = 0;
	getLinearLevels( &bl, &wl);

	//based on the cache strategy, perform stretched statistics
	if ( stretchController.setBackgroundLevel( level, getStretch() ) )
	{		
		const Plane& plane = planeModel.getPlane();
		//get the image
		const ImageCube* cube = (*imageReader)[plane.imageIndex];
		//the tilecontrol does not retile if the tiling is unchanged by this call
		tileControl.reTile( cube, TileControl::tileSizeLarge, planeModel.getPlane() );
		progressModel.SetMax( 100 );	
	
		progressModel.Reset();
		//need to do cancel handling
		if ( doStatistics( cube, true, true ) == ImageTile::OperationCanceled )
		{
			rollBackState();
			return;
		}

		histogramController.updateHistogram( statisticsModel.getStretchMin(), statisticsModel.getStretchMax());
		const Stretch& stretch = getStretch();
		StretchController::transferLevels( &bl, &wl, stretch );
		
		//we then set the slider positions
		this->histogramModel.updateLevels( bl, wl );		

		//generate preview
		//makePreview( cube );
		flushPreview();
		//the tilecontrol does not retile if the tiling is unchanged by this call
		tileControl.reTile( cube, TileControl::tileSizeSmall, planeModel.getPlane() );		
		
	}
	End();
}
/**
*	Called from the GUI when the user has changed more than one parameter
*   in one go (through using the temporary disabling of the direct application of new values)
*
*/
Void FlowController::setBackgroundPeakScaledPeakLevels( Double bg, Double pl, Double sPl )
{
	if ( Begin() )
	{
		boost::thread worker(boost::bind(&FlowController::setBackgroundPeakScaledPeakLevels_,
			this, bg, pl, sPl));
	}
}

Void FlowController::setBackgroundPeakScaledPeakLevels_( Double bg, Double pl, Double sPl )
{
	saveState();
	//if the user opened a new file FL cannot support the cancel operation
	//otherwise it is business as usual
	progressModel.CanCancel( true );
	Double bl = 0;
	Double wl = 0;
	getLinearLevels( &bl, &wl);
	const Plane& plane = planeModel.getPlane();
	//get the image
	const ImageCube* cube = (*imageReader)[plane.imageIndex];
	
	Double oldBg = stretchModel.getBackground();
	Double oldPeaklevel = stretchModel.getPeakLevel();
	Double oldScaledPeakLevel = stretchModel.getRescaleFactor();
	
	
	Double rescale		= sPl;
	Double bgScale		= stretchModel.getScaleBackground();

	Stretch& stretch = getStretch();

	if ( ( rescale - bgScale) > 0.0 && ( pl - bg ) > 0.0 )
	{
		stretchModel.setBackground( bg );
		stretchModel.setRescaleFactor( rescale );
		stretchModel.setPeakLevel( pl );

		Double scale		= ( rescale - bgScale) / ( pl - bg );

		stretchModel.setScale( scale );
		
		
		//the tilecontrol does not retile if the tiling is unchanged by this call
		tileControl.reTile( cube, TileControl::tileSizeLarge, planeModel.getPlane() );
		
		progressModel.SetMax( 100 );	
		
		progressModel.Reset();
		//do statistics
		if ( doStatistics( cube, true, true ) == ImageTile::OperationCanceled )
		{
			rollBackState();
			return;
		}
		
		
		//do histogram
		stretch = getStretch();
		this->histogramController.updateHistogram( statisticsModel.getStretchMin(), statisticsModel.getStretchMax() );
		StretchController::transferLevels( &bl, &wl, stretch );
		this->histogramModel.updateLevels( bl, wl );

		//do preview
		//makePreview( cube );
		flushPreview();
		//the tilecontrol does not retile if the tiling is unchanged by this call
		tileControl.reTile( cube, TileControl::tileSizeSmall, planeModel.getPlane() );
	}
	else
	{
		this->stretchModel.setRescaleFactor( oldScaledPeakLevel );
		this->stretchModel.setScale( oldScaledPeakLevel / ( oldPeaklevel - oldBg ) );					
		this->stretchModel.setBackground( oldBg );
	}

	
	
	End();
}

/**
Converts the values to their linear mode
*/
Void FlowController::getLinearLevels( Double* blackLevel, Double* whiteLevel)
{
	Double bl		= this->histogramModel.getBlackLevel();
	Double wl		= this->histogramModel.getWhiteLevel();

	const Stretch& stretch	= getStretch();
	//gets the linear values of the stretched levels
	bl				= FitsEngine::getLinearVal( stretch, bl);
	wl				= FitsEngine::getLinearVal( stretch, wl);

	*blackLevel = bl;
	*whiteLevel = wl;
	
}

Void FlowController::setPeakLevel( Double d )
{
	if ( Begin() )
	{			
		boost::thread worker(boost::bind(&FlowController::setPeakLevel_, this, d));
	}
}

Void FlowController::setPeakLevel_( Double d ) 
{
	saveState();
	if ( d != stretchModel.getPeakLevel() )
	{
		//based on the cache strategy, perform stretched statistics
		//should update the scale factor with rescaleFactor / peakLevel
		Double rescale	= stretchModel.getRescaleFactor();
		Double bg		= stretchModel.getBackground();
		Double scaleBg	= stretchModel.getScaleBackground();
		
		Double oldVal	= this->stretchModel.getPeakLevel();

		//boundaries: PeakLevel - BackgroundLevel > 0 and ScalePeakLevel - ScaleBackgroundLevel > 0
		if ( ( d - bg) > 0.0 && ( rescale - scaleBg ) > 0.0 )
		{			
			stretchModel.setPeakLevel( d );		
			if ( setScale( rescale / ( d - bg ) ) == ImageTile::OperationCanceled )
			{
				rollBackState();
				return;
			}
			/*const Plane& plane = planeModel.getPlane();
			//get the FitsImage
			const ImageCube* cube = (*imageReader)[plane.imageIndex];
			//generate preview
			makePreview( cube );
			

			//the tilecontrol does not retile if the tiling is unchanged by this call
			tileControl.reTile( cube, TileControl::tileSizeSmall, planeModel.getPlane() );				
			*/
		}
		else
		{
		//	this->showIncorrectInputAlert();
			stretchModel.setPeakLevel( oldVal );
			stretchModel.setScale( rescale / ( oldVal - bg ) );		

		}					
	}
	End();
}

Void FlowController::setRescaleFactor( Double d )
{
	if ( Begin() )
	{
		//do the tough stuff in a separate thread
		boost::thread worker(boost::bind(&FlowController::setRescaleFactor_, this, d));			
	}
}

Void FlowController::setRescaleFactor_( Double d )
{
	saveState();
	if ( d != stretchModel.getRescaleFactor() )
	{		
		//should update the scale factor with rescaleFactor / peakLevel
		Double peak		= stretchModel.getPeakLevel();
		Double bg		= stretchModel.getBackground();
		
		Double scaleBg	= stretchModel.getScaleBackground();

		Double oldVal	= stretchModel.getRescaleFactor();

		//boundaries: PeakLevel - BackgroundLevel > 0 and ScalePeakLevel - ScaleBackgroundLevel > 0
		if ( ( d - scaleBg) > 0.0 && ( peak - bg ) > 0.0 )
		{					
			stretchModel.setRescaleFactor( d );
			
			if ( setScale( d / ( peak - bg ) ) == ImageTile::OperationCanceled )
			{
				rollBackState();
				return;
			}			
		}
		else
		{			
			this->stretchModel.setRescaleFactor( oldVal );
			this->stretchModel.setScale( oldVal / ( peak - bg ) );					
		}			
	}
	
	End();
}

/**
Resets to the default values
*/
Void FlowController::defaultValues()
{
    if ( Begin() )
	{
		boost::thread worker(boost::bind( &FlowController::defaultValues_, this ));
	}
}
/**
Internal and private version of the defaultValues method
*/
Void FlowController::defaultValues_()
{
	saveState();
	//if the user opened a new file FL cannot support the cancel operation
	//otherwise it is business as usual
	progressModel.CanCancel( true );
	const Plane& plane = planeModel.getPlane();
	//get the FitsImage
	const ImageCube* cube = (*imageReader)[plane.imageIndex];
	statisticsController.defaultValues();
	//the tilecontrol does not retile if the tiling is unchanged by this call
	tileControl.reTile( cube, TileControl::tileSizeLarge, planeModel.getPlane() );
	stretchController.defaultValues();
	stretchModel.setFunction( (StretchFunction)( optionsModel.DefaultStretch() ) );
	
	progressModel.SetMax( 100 );	
	
	progressModel.Reset();
	
	if ( doStatistics( cube, true, true ) == ImageTile::OperationCanceled )
	{
		rollBackState();
		return;
	}
	
	initialGuess();
	histogramController.updateHistogram( statisticsModel.getStretchMin(), statisticsModel.getStretchMax() );
	
	//do preview
	//makePreview( cube );
	flushPreview();
	//the tilecontrol does not retile if the tiling is unchanged by this call
	tileControl.reTile( cube, TileControl::tileSizeSmall, planeModel.getPlane() );
	
	End();
}

/**
Entry for the auto background level
*/
Void FlowController::automaticBackground()
{
	//sets the background equal to the black level
	Double bg		= histogramModel.getBlackLevel();
	
	const Stretch& stretch	= getStretch();
	//gets the linear values of the stretched level
	bg = FitsEngine::getLinearVal( stretch, bg );

	setBackgroundLevel( bg );
}
/**
Entry for the auto scale

*/
Void FlowController::automaticScale()
{
	const Stretch& stretch	= getStretch();
	//gets the linear values of the stretched level
	Double wl		= histogramModel.getWhiteLevel();
	wl				= FitsEngine::getLinearVal( stretch, wl );

	setPeakLevel( wl );
}
/**
Auto scale and background
*/
Void FlowController::automaticBackgroundScale()
{
    if ( Begin() )
	{
		boost::thread worker(boost::bind( &FlowController::automaticBackgroundScale_, this ));
	}
}

/**
Internal auto background and scale function
*/
Void FlowController::automaticBackgroundScale_()
{
	saveState();
	//if the user opened a new file FL cannot support the cancel operation
	//otherwise it is business as usual
	progressModel.CanCancel( true );
	const Plane& plane = planeModel.getPlane();
	//get the image
	const ImageCube* cube = (*imageReader)[plane.imageIndex];

	Double bg			= histogramModel.getBlackLevel();
	Double top			= histogramModel.getWhiteLevel();
	
	Double rescale		= optionsModel.ScaledPeak();
	Double bgScale		= stretchModel.getScaleBackground();

	Stretch& stretch = getStretch();

	bg					= FitsEngine::getLinearVal( stretch, bg );
	top					= FitsEngine::getLinearVal( stretch, top );

	Double scale		= ( rescale - bgScale) / ( top - bg );

	stretchModel.setScale( scale );
	stretchModel.setBackground ( bg );
	stretchModel.setRescaleFactor( rescale );
	stretchModel.setPeakLevel( top );
	
	//the tilecontrol does not retile if the tiling is unchanged by this call
	tileControl.reTile( cube, TileControl::tileSizeLarge, planeModel.getPlane() );
	
	progressModel.SetMax( 100 );	
	
	progressModel.Reset();
	//do statistics
	if ( doStatistics( cube, true, true ) == ImageTile::OperationCanceled )
	{
		rollBackState();
		return;
	}
	
	
	//do histogram
	stretch = getStretch();
	this->histogramController.updateHistogram( statisticsModel.getStretchMin(), statisticsModel.getStretchMax() );
	StretchController::transferLevels( &bg, &top, stretch );
	this->histogramModel.updateLevels( bg, top );

	//do preview
	//makePreview( cube );
	flushPreview();
	//the tilecontrol does not retile if the tiling is unchanged by this call
	tileControl.reTile( cube, TileControl::tileSizeSmall, planeModel.getPlane() );
	
	End();
}

Void FlowController::zoomRectangle( FitsLiberator::Rectangle& rect )
{
	if ( Begin() )
	{
		boost::thread worker(boost::bind( &FlowController::zoomRectangle_, this, rect ));
	}
}

Void FlowController::zoomRectangle_( FitsLiberator::Rectangle& rect )
{

	const Plane& plane = planeModel.getPlane();
	//based on the cache strategy generate the preview
	const ImageCube* cube = (*imageReader)[plane.imageIndex];

	//the tilecontrol does not retile if the tiling is unchanged by this call
	tileControl.reTile( cube, TileControl::tileSizeSmall, planeModel.getPlane() );
	

	Size rawSize;
	rawSize.width = cube->Width();
	rawSize.height = cube->Height();

	previewController.zoomRectangle( rect, rawSize, planeModel.getFlipped().flipped );

	makePreview( cube );

	End();

	//tileControl.preLoad( cube, plane );

}

Void FlowController::fitToPreview()
{
	if ( Begin() )
	{
		boost::thread worker(boost::bind( &FlowController::fitToPreview_, this ));
	}
	
}

Void FlowController::fitToPreview_()
{
	const Plane& plane = planeModel.getPlane();
	//based on the cache strategy generate the preview
	//get the image
	const ImageCube* cube = (*imageReader)[plane.imageIndex];

	Size rawSize;

	rawSize.width = cube->Width();
	rawSize.height = cube->Height();

	previewController.fitToPreview( rawSize, planeModel.getFlipped().flipped );

	makePreview( cube );

	End();
}

Void FlowController::setUnityZoom()
{
	if ( Begin() )
	{
		boost::thread worker(boost::bind( &FlowController::setUnityZoom_, this ));
	}
}
Void FlowController::setUnityZoom_()
{
	//based on the cache strategy generate the preview
	const Plane& plane = planeModel.getPlane();
	//based on the cache strategy generate the preview
	//get the image
	const ImageCube* cube = (*imageReader)[plane.imageIndex];

	Size rawSize;

	rawSize.width = cube->Width();
	rawSize.height = cube->Height();

	previewController.setUnityZoom( rawSize, planeModel.getFlipped().flipped );
	
	makePreview( cube );

	End();
}

Void FlowController::centerPreview()
{
	if ( Begin() )
	{
		boost::thread worker(boost::bind( &FlowController::centerPreview_, this ));
	}
}
Void FlowController::centerPreview_()
{
	//based on the cache strategy generate the preview
	const Plane& plane = planeModel.getPlane();
	//get the image
	const ImageCube* cube = (*imageReader)[plane.imageIndex];

	Size rawSize;

	rawSize.width = cube->Width();
	rawSize.height = cube->Height();

	previewController.centerPreview( rawSize );

	makePreview( cube );

	End();
}

Void FlowController::incrementZoom( FitsLiberator::Point p )
{
	if ( Begin() )
	{
		boost::thread worker(boost::bind( &FlowController::incrementZoom_, this, p ));
	}
}
Void FlowController::incrementZoom_( FitsLiberator::Point p )
{
	const Plane& plane = planeModel.getPlane();
	//based on the cache strategy generate the preview
	//get the image
	const ImageCube* cube = (*imageReader)[plane.imageIndex];

	//the tilecontrol does not retile if the tiling is unchanged by this call
	tileControl.reTile( cube, TileControl::tileSizeSmall, planeModel.getPlane() );
	

	Size rawSize;

	rawSize.width = cube->Width();
	rawSize.height = cube->Height();

	previewController.incrementZoom( rawSize, p, planeModel.getFlipped().flipped );

	makePreview( cube );

	End();

}

Void FlowController::decrementZoom( FitsLiberator::Point p )
{
	if ( Begin() )
	{
		boost::thread worker(boost::bind( &FlowController::decrementZoom_, this, p ));
	}
}

Void FlowController::decrementZoom_( FitsLiberator::Point p )
{
	const Plane& plane = planeModel.getPlane();
	//based on the cache strategy generate the preview
	//get the image
	const ImageCube* cube = (*imageReader)[plane.imageIndex];

	//the tilecontrol does not retile if the tiling is unchanged by this call
	tileControl.reTile( cube, TileControl::tileSizeSmall, planeModel.getPlane() );
	

	Size rawSize;

	rawSize.width = cube->Width();
	rawSize.height = cube->Height();

	previewController.decrementZoom( rawSize, p, planeModel.getFlipped().flipped );

	makePreview( cube );

	End();
}

/**
Sets the zoom index
*/
Void FlowController::setZoomIndex( Int index )
{
	if ( Begin() )
	{
		boost::thread worker(boost::bind( &FlowController::setZoomIndex_, this, index ));
	}
}
Void FlowController::setZoomIndex_( Int index )
{
	const Plane& plane = planeModel.getPlane();
	//get the image
	const ImageCube* cube = (*imageReader)[plane.imageIndex];

	//the tilecontrol does not retile if the tiling is unchanged by this call
	tileControl.reTile( cube, TileControl::tileSizeSmall, planeModel.getPlane() );
	

	Size rawSize;

	rawSize.width = cube->Width();
	rawSize.height = cube->Height();

	previewController.setZoomIndex( index, rawSize, planeModel.getFlipped().flipped );

	makePreview( cube );

	End();
}

Void FlowController::movePreview( MovementVector vec )
{
/*	if ( Begin() )
	{
		boost::thread worker(boost::bind( &FlowController::movePreview_, this, vec ));
	}
*/
	movePreview_( vec );
}

Void FlowController::movePreview_( MovementVector vec )
{
	//if ( previewModel.getZoomFactor() > previewModel.getZoomFitFactor() )
	//{
		//update the vec if the image is flipped
		if ( planeModel.getFlipped().flipped )
			vec.y = -vec.y;
		const Plane& plane = planeModel.getPlane();
		//based on the cache strategy generate the preview
		//get the image
		const ImageCube* cube = (*imageReader)[plane.imageIndex];

		//the tilecontrol does not retile if the tiling is unchanged by this call
		tileControl.reTile( cube, TileControl::tileSizeSmall, planeModel.getPlane() );
		

		//get the size of the image
		Size rawSize;

		rawSize.width = cube->Width();
		rawSize.height = cube->Height();

		//make the previewController do the movement
		previewController.movePreview( vec, rawSize );

		//generate the preview
		makePreview( cube );
		//previewController.scaleDynamicRange( histogramModel.getBlackLevel(), histogramModel.getWhiteLevel() );
		//SendNotifications();
	//}
}

/**
	Applies the black/white level to the stretch
*/
Void FlowController::applyLevels()
{
	histogramModel.updateLevels( histogramModel.getCurrentBlackLevel(),histogramModel.getCurrentWhiteLevel() );	
	previewController.scaleDynamicRange( histogramModel.getBlackLevel(), histogramModel.getWhiteLevel() );
	SendNotifications();
}

/**
Increments the black level
*/
Void FlowController::incrementBlackLevel( Bool up, Bool big )
{
	histogramController.incrementBlackLevel( up, big );
	previewController.scaleDynamicRange( histogramModel.getBlackLevel(), histogramModel.getWhiteLevel() );
	SendNotifications();
}

/*
Sets the black level
*/
Void FlowController::setBlackLevel( Double bl )
{
	histogramController.setBlackLevel( bl );
	previewController.scaleDynamicRange( histogramModel.getBlackLevel(), histogramModel.getWhiteLevel() );
	SendNotifications();
}


/**
Increments the white level
*/
Void FlowController::incrementWhiteLevel( Bool up, Bool big )
{
	histogramController.incrementWhiteLevel( up, big );
	previewController.scaleDynamicRange( histogramModel.getBlackLevel(), histogramModel.getWhiteLevel() );
	SendNotifications();
}
/*
Sets the white level
*/
Void FlowController::setWhiteLevel( Double wl )
{
	histogramController.setWhiteLevel( wl );
	previewController.scaleDynamicRange( histogramModel.getBlackLevel(), histogramModel.getWhiteLevel() );
	SendNotifications();
}

/**
Saves the current state for use if the user cancels
*/
Void FlowController::saveState()
{
	if ( currentState == NULL )
	{
		currentState = new FlowControllerState( previewModel.getPreviewSize().width, previewModel.getPreviewSize().height );
		

		//make the histogram array
		currentState->nBins = histogramModel.getRawBins().size();
		currentState->histBins = new Double[currentState->nBins];
		//copy the histogram
		Vector<Double>& hist = histogramModel.getRawBins();
		for ( UInt i = 0; i < currentState->nBins; i++ )
			currentState->histBins[i] = hist[i];

		currentState->maxBinCount = histogramModel.getMaxBin();

		currentState->realMax = statisticsModel.getRealMax();
		currentState->sclMax = statisticsModel.getScaledMax();
		currentState->strMax = statisticsModel.getStretchMax();
		
			
		currentState->realMin = statisticsModel.getRealMin();
		currentState->sclMin = statisticsModel.getScaledMin();
		currentState->strMin = statisticsModel.getStretchMin();

		currentState->realMean = statisticsModel.getRealMean();
		currentState->sclMean = statisticsModel.getScaledMean();
		currentState->strMean = statisticsModel.getStretchMean();

		currentState->realMedian = statisticsModel.getRealMedian();
		currentState->sclMedian = statisticsModel.getScaledMedian();
		currentState->strMedian = statisticsModel.getStretchMedian();

		currentState->realStdev = statisticsModel.getRealSTDEV();
		currentState->sclStdev = statisticsModel.getScaledSTDEV();
		currentState->strStdev = statisticsModel.getStretchSTDEV();

		currentState->stretch = getStretch();
		currentState->plane = planeModel.getPlane();

		//copy the stretch
		currentState->function = stretchModel.getFunction();
		currentState->background = stretchModel.getBackground();
		currentState->backgroundIncrementation = stretchModel.getBackgroundIncrementation();
		currentState->scale = stretchModel.getScale();
		currentState->scaleIncrementation = stretchModel.getScaleIncrementation();
		currentState->peakLevel = stretchModel.getPeakLevel();
		currentState->rescaleFactor = stretchModel.getRescaleFactor();
		currentState->scaleBackground = stretchModel.getScaleBackground();

		PreviewImage& pImg = previewModel.getPreviewImage();
		//copy the preview image
		for ( UInt i = 0; i < currentState->previewImage->size.getArea(); i++ )
		{
			currentState->previewImage->blackClippedMap[i] = pImg.blackClippedMap[i];
			currentState->previewImage->nullMap[i] = pImg.nullMap[i];
			currentState->previewImage->previewPixels[i] = pImg.previewPixels[i];
			currentState->previewImage->rawPixels[i] = pImg.rawPixels[i];
		}
	}
}

/**
Rollbacks the to the original state before an operation that can be canceled was initiated
*/
Void FlowController::rollBackState()
{
	if ( currentState != NULL )
	{
		//copy the histogram
		Vector<Double>& hist = histogramModel.getRawBins();
		for ( UInt i = 0; i < hist.size(); i++ )
			hist[i] = currentState->histBins[i];

		//copy the preview image
		PreviewImage& pImg = previewModel.getPreviewImage();
		for ( UInt i = 0; i < pImg.size.getArea(); i++ )
		{
			pImg.blackClippedMap[i] = currentState->previewImage->blackClippedMap[i];
			pImg.nullMap[i] = currentState->previewImage->nullMap[i];
			pImg.previewPixels[i] = currentState->previewImage->previewPixels[i];
			pImg.rawPixels[i] = currentState->previewImage->rawPixels[i];
		}

		//copy the statistics
		statisticsController.setRealValues( currentState->realMin, currentState->realMax, currentState->realMean, 
			currentState->realMedian, currentState->realStdev, currentState->scale, currentState->background,
			currentState->scaleBackground );

		
		statisticsController.setStretchValues( currentState->strMin, currentState->strMax, currentState->strMean,
			currentState->strMedian, currentState->strStdev );	

		//rollback plane and stretch
		planeModel.getPlane().imageIndex = currentState->plane.imageIndex;
		planeModel.getPlane().planeIndex = currentState->plane.planeIndex;

		stretchModel.setBackground( currentState->background );
		stretchModel.setBackgroundIncrementation( currentState->backgroundIncrementation );
		stretchModel.setFunction( currentState->function );
		stretchModel.setPeakLevel( currentState->peakLevel );
		stretchModel.setRescaleFactor( currentState->rescaleFactor );
		stretchModel.setScale( currentState->scale );
		stretchModel.setScaleBackground( currentState->scaleBackground );
		stretchModel.setScaleIncrementation( currentState->scaleIncrementation );

		delete currentState;
		currentState = NULL;

		End();
	}
}

Int FlowController::doStatistics( const ImageCube* cube, Bool stretched, Bool doPreview )
{
	Double min;
	Double max;
	Double mean;
	Double median;
	Double stdev;
	Double maxBinCount;
	Vector<Double>& histogram = histogramModel.getRawBins();
	Plane& plane = planeModel.getPlane();
	Stretch stretch;
	if ( stretched )
		stretch = getStretch();
	Int err = ImageTile::AllocErr;

	
	while ( err == ImageTile::AllocErr )
	{		
		//important to set err to OK. It will be forced to be err if a problem occurs
		err = ImageTile::AllocOk;
		
		//Should only do the calculations if they were not already stored
		if ( !statisticsController.performStatistics( stretched, stretch, plane, &min, &max, &mean, &stdev, &median, histogram, &maxBinCount ) )
		{
			//Choose cache strategy for the new plane
			tileControl.reTile( cube, TileControl::tileSizeLarge, planeModel.getPlane() );
			progressModel.SetIncrement( 2*tileControl.getNumberOfTiles() );
			
			err = tileControl.doStatistics3( cube, stretched, &min, &max, &mean, &median, &stdev, histogram, 
				&maxBinCount, stretch, plane, &previewController, doPreview, planeModel.getFlipped().flipped, &progressModel );			
			
			//decrease total amount of spendable memory
			if ( err == ImageTile::AllocErr ) 
				tileControl.decreaseMaxMem();				
			else if ( err == ImageTile::OperationCanceled )
				return err;
		}
		else
		{
			previewModel.useCache( stretch, plane, *(previewModel.getAnchorPoint()), previewModel.getZoomFactor(),
				previewModel.getPreviewImage().rawPixels, previewModel.getPreviewImage().size.getArea(),
				planeModel.getFlipped().flipped );
		}
	}
	//save the statistics
	if ( !stretched )
	{
		statisticsController.setRealValues( min, max, mean, median,
			stdev, stretchModel.getScale(), stretchModel.getBackground(),
			stretchModel.getScaleBackground() );
	}
	else
	{
		statisticsController.setScaledValues( stretchModel.getScale(), stretchModel.getBackground(),
			stretchModel.getScaleBackground() );
		statisticsController.setStretchValues( min, max, mean, median, stdev );		
		//store the data in cache
		statisticsModel.storeCache( stretch, histogram, maxBinCount, plane, stretchModel.getBackground(), stretchModel.getScale(),
									max, min, mean, median, stdev);
	}	
	
	histogramModel.setMaxBin( maxBinCount );		
	
	

	return ImageTile::AllocOk;
	
}

/**
The preview is generated from the tiles and from the zoom state, the anchor and the flip state
First the tiles involved are listed.
Second there is a (possibly) multithreaded loop over these tiles
The preview image is then filled out by this loop
The previewcontroller does the preview generationg based on the tiles it gets
This method needs a lot of testing
*/
Void FlowController::makePreview( const ImageCube* cube )
{
	Stretch& stretch = getStretch();
	Plane& plane = planeModel.getPlane();
	
	PreviewImage& previewImage			= previewModel.getPreviewImage();
	FitsLiberator::Point& anchor		= *(previewModel.getAnchorPoint());
	Double zoomFactor					= previewModel.getZoomFactor();

	progressModel.SetMax( 100 );
	progressModel.Reset();
	progressModel.SetIncrement( tileControl.getNumberOfTiles() );
	if ( previewModel.useCache( stretch, plane, anchor, zoomFactor,
				previewImage.rawPixels, previewImage.size.getArea(), planeModel.getFlipped().flipped ) )
	{
	}
	else
	{
		if ( tileControl.getTileStrategy() == TileControl::tileSizeLarge )
		{
			TilePusher* tilePusher = new TilePusher( tileControl.getTiles(),
				tileControl.getNumberOfTiles() );

			tilePusher->initSession();
			//Loop through tiles
			while ( tilePusher->sessionHasMoreTiles() )
			{
				ImageTile* tile = tilePusher->getNextTile();
				previewController.prepareTile( *tile, planeModel.getFlipped().flipped );
				if ( tile->isCurrent() )
				{
					if ( !( tile->isAllocated() ) )
					{
						//allocate and load from disk
						if ( tile->allocatePixels( cube->SizeOf(1,1) ) == ImageTile::AllocOk )
						{
						
							cube->Read( plane.planeIndex, tile->getBounds(), 
								tile->rawPixels, tile->nullPixels );
						}
						else
						{
							break;
						}
					}
					tileControl.stretchTile_par( *tile, stretch, cube );
					previewController.zoomTile( *tile, planeModel.getFlipped().flipped );
				}		
				progressModel.Increment();
			}
			delete tilePusher;
		}
		else
		{

			Int nTiles = tileControl.getNumberOfTiles();

			for ( Int i = 0; i < nTiles; i++ )
			{
				//for each tile write down what to do with it (i.e. the effective range)
				ImageTile* tile = tileControl.getLightTile( i );
				previewController.prepareTile( *tile, planeModel.getFlipped().flipped );
				if ( tile->isCurrent() )
				{
					//get the tile
					tile = tileControl.getTile( i, cube, plane, true );
					if ( tile != NULL )
						tile->locked = true;		
					else
					{
						//shit
						throw Exception("Failed to aquire tile");
					}					
					//stretch the raw pixels
					tileControl.stretchTile( *tile, stretch, cube );		
					//create the part of the preview that uses this tile	
					previewController.zoomTile( *tile, planeModel.getFlipped().flipped );		
					//remember to unlock the tile
					tile->locked = false;					
				}										
				progressModel.Increment();
			}
		}
		previewModel.storeCache( stretch, plane, anchor, zoomFactor,previewImage.rawPixels,previewImage.size.getArea(),
								 planeModel.getFlipped().flipped );
	}
	//scale the preview pixels to the 0-255 range
	previewController.scaleDynamicRange( histogramModel.getBlackLevel(), histogramModel.getWhiteLevel() );
    //SendNotifications();
}


Void FlowController::makePreview2( const ImageCube* cube )
{
	Stretch& stretch = getStretch();
	Plane& plane = planeModel.getPlane();
	
	PreviewImage& previewImage = previewModel.getPreviewImage();
	FitsLiberator::Point& anchor		= *(previewModel.getAnchorPoint());
	Double zoomFactor					= previewModel.getZoomFactor();

	if ( previewModel.useCache( stretch, plane, anchor, zoomFactor,
				previewImage.rawPixels, previewImage.size.getArea(), planeModel.getFlipped().flipped ) )
	{
	}
	else
	{
		if ( tileControl.getTileStrategy() == TileControl::tileSizeLarge )
		{
			TilePusher* tilePusher = new TilePusher( tileControl.getTiles(),
				tileControl.getNumberOfTiles() );

			tilePusher->initSession();
			//Loop through tiles
			while ( tilePusher->sessionHasMoreTiles() )
			{
				ImageTile* tile = tilePusher->getNextTile();
				previewController.prepareTile( *tile, planeModel.getFlipped().flipped );
				if ( tile->isCurrent() )
				{
					if ( !( tile->isAllocated() ) )
					{
						//allocate and load from disk
						tile->allocatePixels( cube->SizeOf(1,1) );
						
						cube->Read( plane.planeIndex, tile->getBounds(), 
							tile->rawPixels, tile->nullPixels );
					}
					tileControl.stretchTile_par( *tile, stretch, cube );
					previewController.zoomTile( *tile, planeModel.getFlipped().flipped );
				}
			}
			delete tilePusher;
		}
		else
		{

			Int nTiles = tileControl.getNumberOfTiles();

			for ( Int i = 0; i < nTiles; i++ )
			{
				//for each tile write down what to do with it (i.e. the effective range)
				ImageTile* tile = tileControl.getLightTile( i );
				previewController.prepareTile( *tile, planeModel.getFlipped().flipped );
				if ( tile->isCurrent() )
				{
					//get the tile
					tile = tileControl.getTile( i, cube, plane, true );
					if ( tile != NULL )
						tile->locked = true;		
					else
					{
						//shit
						throw Exception("Failed to aquire tile");
					}					
					//stretch the raw pixels
					tileControl.stretchTile( *tile, stretch, cube );		
					//create the part of the preview that uses this tile	
					previewController.zoomTile( *tile, planeModel.getFlipped().flipped );		
					//remember to unlock the tile
					tile->locked = false;
				}		
			}
		}
		previewModel.storeCache( stretch, plane, anchor, zoomFactor,previewImage.rawPixels,previewImage.size.getArea(),
								 planeModel.getFlipped().flipped );
	}
	//scale the preview pixels to the 0-255 range
	previewController.scaleDynamicRange( histogramModel.getBlackLevel(), histogramModel.getWhiteLevel() );
    SendNotifications();
}


Int FlowController::setScale(Double d)
{
	saveState();
	//if the user opened a new file FL cannot support the cancel operation
	//otherwise it is business as usual
	progressModel.CanCancel( true );	
	Double oldVal = stretchModel.getScale();
	
	Stretch& stretch = getStretch();
	if (d != 0.0)
	{
		Double bl = 0.0;
		Double wl = 0.0;
		this->getLinearLevels( &bl, &wl );
		
		stretchModel.setScale( d );

		const Plane& plane = planeModel.getPlane();
		//get the FitsImage
		const ImageCube* cube = (*imageReader)[plane.imageIndex];

		tileControl.reTile( cube, TileControl::tileSizeLarge, planeModel.getPlane() );
		
		progressModel.SetMax( 100 );
	
		progressModel.Reset();
		if ( doStatistics( cube, true, true ) == ImageTile::OperationCanceled )
		{
			rollBackState();
			return ImageTile::OperationCanceled;
		}
		

		histogramController.updateHistogram( statisticsModel.getStretchMin(), statisticsModel.getStretchMax() );
	
		stretch = getStretch();

		StretchController::transferLevels( &bl, &wl, stretch );
		histogramModel.updateLevels( bl, wl );

		flushPreview();
	}
	else
	{
		this->stretchModel.setScale( oldVal );
	}
	return ImageTile::AllocOk;
}


/**
Makes the histogram show the specified range
*/
Void FlowController::showRange( UInt left, UInt right )
{
	Double min = statisticsModel.getStretchMin();
	Double max = statisticsModel.getStretchMax();

	histogramController.showRange( left, right, min, max );
	SendNotifications();
}

/**
Zooms the histogram
*/
Void FlowController::incrementHistogramZoom( UInt coord )
{
	Double min = statisticsModel.getStretchMin();
	Double max = statisticsModel.getStretchMax();

	histogramController.incrementZoom( coord, min, max  );
	SendNotifications();
}

/**
Zooms the histogram
*/
Void FlowController::decrementHistogramZoom( UInt coord )
{
	Double min = statisticsModel.getStretchMin();
	Double max = statisticsModel.getStretchMax();

	histogramController.decrementZoom( coord, min, max  );
	SendNotifications();
}

/**
Shows the entire range of the histogram
*/
Void FlowController::showAllHistogram()
{
	Double min = statisticsModel.getStretchMin();
	Double max = statisticsModel.getStretchMax();

	histogramController.showAll( min, max );

	SendNotifications();
}

Void FlowController::moveHistogram( Int delta )
{
	Double min = statisticsModel.getStretchMin();
	Double max = statisticsModel.getStretchMax();
	
	histogramController.move( delta, min, max );

	SendNotifications();
}

Void FlowController::moveBlackLevelSlider( Int pos )
{
	Double min = statisticsModel.getStretchMin();
	Double max = statisticsModel.getStretchMax();
	
	histogramController.moveBlackLevelSlider( pos, min, max );

	SendNotifications();
}

Void FlowController::moveWhiteLevelSlider( Int pos )
{
	Double min = statisticsModel.getStretchMin();
	Double max = statisticsModel.getStretchMax();
	
	histogramController.moveWhiteLevelSlider( pos, min, max );

	SendNotifications();
}

/**
Opens the save file dialog and subsequently saves the tiff file
@param invokeEditor if true the default editor in the system
should be invoked with the newly saved tiff file
*/
Void FlowController::saveFile( Bool invokeEditor, String fileName )
{	
	if ( fileName != "" )
	{
		if ( Begin() )
		{
			makeSession( &session );
			boost::thread worker(boost::bind(&FlowController::saveFile_, this, fileName, invokeEditor ));			
		}
	}

}

Void FlowController::saveFile_( String fileName, Bool invokeEditor )
{		
	FitsLiberator::FileLoader* loader = NULL;
	//first make sure the session is correct	

	try
	{
		progressModel.SetMax( 100 );
		progressModel.Reset();
		
		//create the loader. Catch exception if the file could not be opened.
		loader	= new FitsLiberator::FileLoader( tileControl, (*imageReader), 
			&session, fileName );
		tileControl.reTile( (*imageReader)[session.plane.imageIndex], TileControl::tileSizeImport, planeModel.getPlane() );
		
		loader->ReadStart( );
		loader->ReadContinue( progressModel );
	}
	catch ( FitsLiberator::FileLoaderException ex )
	{

	}
	
	//clean up
	if ( loader != NULL )
		delete loader;

	End();

	if ( invokeEditor )
	{
		//try to invoke the default editor with the newly saved tiff file		
		Environment::startSystemEditor(fileName);
	}
}

Void FlowController::makeSession( FitsSession* session )
{
	session->stretch.scale 						= stretchModel.getScale();
	session->stretch.function 					= stretchModel.getFunction();
	session->stretch.offset						= stretchModel.getBackground();
	session->stretch.scalePeakLevel				= stretchModel.getRescaleFactor();
	session->stretch.peakLevel					= stretchModel.getPeakLevel();
	session->stretch.blackLevel					= histogramModel.getBlackLevel();
	session->stretch.whiteLevel					= histogramModel.getWhiteLevel();

	session->plane 								= planeModel.getPlane();
	session->importSettings.channelSettings		= planeModel.getImportBitDepth();
    session->importSettings.undefinedSettings	= Environment::supportsTransparent( planeModel.getImportBitDepth() ) ? planeModel.getImportUndefinedOption() : undefinedBlack;
    session->metaData                           = repositoryModel.serializeAsXmp();
	session->flip								= planeModel.getFlipped();

	session->applyStretchValues					= optionsModel.ApplyStretchDirectly();

	//if the preferences should be stored this is the time to do it
	storePreferences( true );
	
}

Stretch& FlowController::getStretch()
{
	this->stretch.blackLevel		= this->histogramModel.getBlackLevel();
	this->stretch.function			= this->stretchModel.getFunction();
	this->stretch.offset			= this->stretchModel.getBackground();
	this->stretch.scale				= this->stretchModel.getScale();
	this->stretch.whiteLevel		= this->histogramModel.getWhiteLevel();;
	this->stretch.scalePeakLevel	= this->stretchModel.getRescaleFactor();
	this->stretch.peakLevel			= this->stretchModel.getPeakLevel();
	this->stretch.scaleBackground	= this->stretchModel.getScaleBackground();
	return this->stretch;
}

Bool FlowController::Begin() {

	if ( ! ( progressModel.QueryBusy() ) )
	{	
		SendNotifications();
		progressModel.Begin();		
		SendNotifications();	
		return true;
	}
	return false;
}

void FlowController::End() {
	if ( currentState != NULL )
	{
		delete currentState;
		currentState = NULL;
	}
	makeSession( &session );


	progressModel.End();
	
	
    SendNotifications();
	
	
}

Void FlowController::applyPreferences() {
	if ( prefs->load() )//make sure that we only set flip if the default values are not used.
		planeModel.setFlipped( prefs->flipped );
	
	globalSettingsModel.setPreviewEnabled( prefs->getPreviewSetting() );
	globalSettingsModel.setFreezeSettings( prefs->getFreezeSetting() );
	globalSettingsModel.setMarkUndefined( prefs->getMarkUndefinedSetting() );
	globalSettingsModel.setMarkWhiteClipping( prefs->getMarkWhiteClippingSetting() );
	globalSettingsModel.setMarkBlackClipping( prefs->getMarkBlackClippingSetting() );
	globalSettingsModel.setImageInformation( prefs->getImageInformationSetting() );
    
    optionsModel.GuessMethod(prefs->defaultGuess);
    optionsModel.BlackLevelPercentage(prefs->defaultBlackLevelPercentage);
    optionsModel.WhiteLevelPercentage(prefs->defaultWhiteLevelPercentage);
    optionsModel.ScaledPeak(prefs->defaultScaledPeak);
    optionsModel.DefaultStretch(prefs->defaultStretch);
    optionsModel.HistogramMarkers(prefs->histMarkers);

	
	optionsModel.ApplyStretchDirectly( prefs->applyStretchValues );
}

Void FlowController::loadMetadata()
{
	// Load the metadata
    StringDictionary& metaData = prefs->getMetaData();
    for( StringDictionary::iterator i = metaData.begin(); i != metaData.end(); i++ ) {
        repositoryController.loadValue( (*i).first, (*i).second );
    }
}

Void FlowController::storePreferences(Bool isOkPressed) {
    prefs->setGlobalSettings( 
		globalSettingsModel.getPreviewEnabled(),
		globalSettingsModel.getFreezeSettings(),
		globalSettingsModel.getMarkUndefined(),
		globalSettingsModel.getMarkWhiteClipping(),
		globalSettingsModel.getMarkBlackClipping(),
		globalSettingsModel.getImageInformation()

	);

    prefs->defaultGuess = optionsModel.GuessMethod();
    prefs->defaultBlackLevelPercentage = optionsModel.BlackLevelPercentage();
    prefs->defaultWhiteLevelPercentage = optionsModel.WhiteLevelPercentage();
    prefs->defaultScaledPeak = optionsModel.ScaledPeak();
    prefs->defaultStretch = optionsModel.DefaultStretch();
    prefs->histMarkers = optionsModel.HistogramMarkers();
	
	prefs->flipped = planeModel.getFlipped().flipped;
	prefs->applyStretchValues = optionsModel.ApplyStretchDirectly();

    if( optionsModel.ClearHistory() ) {
        prefs->cleanHistory();
    }
	
	prefs->setFreezeSettings( session );
	
	if( isOkPressed ) {
		String s = imageReader->FileName();
		prefs->addLoadedFile( s , session );
	}
	
    UInt count = repositoryModel.getCount();
    for( UInt i = 0; i < count; i++ ) {
        const Keyword* keyword = repositoryModel.getKeyword( i );
	      if( repositoryController.shouldSave( keyword ) ) {
            prefs->addMetaField( keyword->name, repositoryController.saveValue( keyword ) );
        }
    }

    prefs->store();
}


FitsLiberator::Preferences::Preferences& FlowController::getPreferences()
{
	return *prefs;
}
