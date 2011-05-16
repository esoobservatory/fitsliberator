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

#ifndef __MACMAINVIEW_H__
#define __MACMAINVIEW_H__

#include "Types.h"
#include "FitsDialog.h"
#include "ModelFramework.h"
#include "MainView.h"
#include "MacGlobalSettingsView.h"
#include "MacStretchView.h"
#include "MacPixelValueView.h"
#include "MacStatisticsView.h"
#include "MacToolView.h"
#include "MacPlaneView.h"
#include "MacHistogramView.h"
#include "MacPreviewView.h"
#include "MacHeaderView.h"
#include "MacMetadataView.h"
#include "MacPreferencesView.h"
#include "LogoControl.h"
#include "NavDialog.h"

using namespace FitsLiberator::Modelling;

namespace FitsLiberator {
	namespace Mac {
		
		class MacMainView : public ControlEventHandler, public FitsLiberator::Modelling::MainView  {
			public:
				MacMainView( FitsDialog& dlog, FitsLiberator::Modelling::ModelFramework& framework );
				virtual ~MacMainView();
				
				void onSave();
				void onOpen();
				void onEdit();
				void onPreferences();
			
				// ControlEventHandler
			
				OSStatus processKeyboardEvent( EventHandlerCallRef, EventRef );
				OSStatus onUnicodeForKeyEvent( EventRef event );
				OSStatus onCommand( HICommand *command );
				
				// MainView
			
				virtual Void updateTitle( const String& title );
				virtual Void updateNetworkWarning( Bool showNetworkWarning );
				virtual void Update(Models* m);
			private:
				/** This function is called by the Carbon Event Manager when a background thread
					has to update the user interface. User interface updates MUST happen on the main
					thread, so the MacChangeManager and MacMainView work in tandem to marshall the
					call to the main thread using the Carbon Event Manager.
					
					This function is a Carbon event handler. For details on the parameters, please 
					refer to the Carbon Event Manager documentation. */
				static pascal OSStatus marshall(EventHandlerCallRef handler, EventRef event, 
					void* userData);
			
				static void openFileCallback(void* userData);
				static void saveFileCallback(void* userData);
				static void editFileCallback(void* userData);			
			
				/** Installs the custom event handler for background thread updates. */
				void installCustomHandler();
				/** Removes the custom event handler for background thread updates. */
				void removeCustomHandler();
				/** Handles updates from the progress model. */
				void UpdateProgress();
			
				FitsLiberator::Modelling::ModelFramework& modelFramework;
				FitsLiberator::Modelling::ProgressModel*  progressModel;

				FitsDialog& 			dialog;
				
				MacGlobalSettingsView* 	globalSettingsView;
				MacStretchView*	 		stretchView;
				MacPixelValueView* 		pixelValueView;
				MacStatisticsView*		statisticsView;
				MacToolView*			toolView;
				MacHistogramView*		histogramView;
				MacPreviewView*			previewView;
				MacPlaneView*			planeView;
				MacHeaderView*			headerView;
				MacMetadataView*		metadataView;
				MacPreferencesView*		preferencesView;
				
				LogoControl*			logoControl;
				BaseControl*			textBox;
				
				BaseControl*			saveButton;
				BaseControl*			openButton;
				BaseControl*			editButton;
				BaseControl*			aboutButton;
				
				Bool					cmdPressed;
			
				NavDialog               navDialog;

				EventHandlerUPP			customHandlerProc;
				EventHandlerRef			customHandler;
		};
	} //end namespace Mac
} // end namespace FitsLiberator

#endif __MACMAINVIEW_H__
