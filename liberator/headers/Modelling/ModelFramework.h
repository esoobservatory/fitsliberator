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

#ifndef __MODELFRAMEWORK_H__
#define __MODELFRAMEWORK_H__

//header for the super-struct containing all models and controllers.

#include "FitsSession.h"
#include "PlaneModel.h"
#include "PlaneController.h"
#include "GlobalSettingsModel.h"
#include "GlobalSettingsController.h"
#include "PreviewModel.h"
#include "PreviewController.h"
#include "StretchModel.h"
#include "StretchController.h"
#include "ToolModel.h"
#include "ToolController.h"
#include "PixelValueModel.h"
#include "StatisticsModel.h"
#include "Statisticscontroller.h"
#include "HistogramModel.h"
#include "HistogramController.h"
#include "PreviewListener.h"
#include "HeaderModel.h"
#include "RepositoryModel.h"
#include "RepositoryController.h"
#include "TaxonomyEditorModel.h"
#include "TaxonomyEditorController.h"
#include "FlowController.h"
#include "TileControl.h"

#include "OptionsModel.h"
#include "OptionsController.h"

#include "ProgressModel.h"
#include "MainModel.h"


#include "AccumulatingChangeManager.h"

namespace FitsLiberator
{
    namespace Modelling
    {
		/**
		Contains the entire modeling framework and a few
		utility functions. In the future it should probably be
		merged with FlowController
		Furthermore, all the models and the controllers are public
		members which is really not a good way to do it.
		Some of them are called from the respective views in this way.
		*/
        class ModelFramework
        {
        public:
            ModelFramework(
                AccumulatingChangeManager* changeManager,                
                FitsLiberator::FitsSession*,
				const String& inputFile );
            virtual ~ModelFramework();
            
            Void saveStateToSession( Bool );
            Void updateViews();
            
            
            void updateModels();
            void applySession(const FitsSession& session);
			
            Void openNewFile( String fileName );
			Void saveEdit( String fileName );
			Void saveFile( String fileName );
			String getFileName();

			FitsLiberator::Engine::TileControl*		tileControl;

            PlaneModel*                 planeModel;
            PlaneController*            planeController;
           
            GlobalSettingsModel*        globalSettingsModel;
            GlobalSettingsController*   globalSettingsController;

            PreviewModel*               previewModel;
            PreviewController*          previewController;

            StretchModel*               stretchModel;
            StretchController*          stretchController;

            ToolModel*                  toolModel;
            ToolController*             toolController;
            
            PixelValueModel*			pixelValueModel;
            
            StatisticsModel*            statisticsModel;
            StatisticsController*       statisticsController;

            HistogramModel*             histogramModel;
            HistogramController*        histogramController;
            
            PreviewListener*			previewListener;
            
            HeaderModel*				headerModel;

            RepositoryModel*            repositoryModel;
            RepositoryController*       repositoryController;

			TaxonomyEditorModel*        taxonomyEditorModel;
			TaxonomyEditorController*   taxonomyEditorController;
            
            AccumulatingChangeManager*  changeManager;

			FlowController*				flowController;

            OptionsModel*               optionsModel;
            OptionsController*          optionsController;

            ProgressModel*              progressModel;

			MainModel*					mainModel;
            
			
			
            private:
            	Bool doGuess;				
				Bool loadFile( String file );
				Void initializeFramework();
				Void makeDefaultSession( FitsLiberator::FitsSession& session );
				Void applyDefaultPrefs( const FitsLiberator::Preferences::Preferences& );

				
				FitsLiberator::FitsSession *			session;
				FitsLiberator::Engine::ImageReader*		reader;  
			
			
        };
    }
}


#endif


