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
 
#include "MacMainView.h"
#include "MacGlobalSettingsView.h"
#include "MacStretchView.h"
#include "MacPixelValueView.h"
#include "MacStatisticsView.h"
#include "MacToolView.h"
#include "MacPlaneView.h"
#include "MacHistogramView.h"
#include "MacPreviewView.h"
#include "MacHeaderView.h"
#include "MacTaxonomyEditorView.h"
#include "MacChangeManager.h"
#include "NavDialog.h"

using namespace FitsLiberator::Mac;
using namespace FitsLiberator::Engine;

using namespace std;

MacMainView::MacMainView( FitsDialog& dlog, ModelFramework& mf ) 
  : dialog( dlog ), modelFramework( mf ), cmdPressed(false),
    customHandlerProc(NULL), customHandler(NULL), MainView(*mf.mainModel)
{
	// Install event handler on the window for modifier keys changed.
	static const EventTypeSpec windowEventTypes[] = {
		{ kEventClassKeyboard, kEventRawKeyModifiersChanged }, 
		{ kEventClassTextInput, kEventTextInputUnicodeForKeyEvent },
		{ kEventClassKeyboard, kEventRawKeyUp }
	};
	
	dlog.installEventHandler( windowEventTypes, GetEventTypeCount( windowEventTypes ), this );
	installCustomHandler();
	
	globalSettingsView	= new MacGlobalSettingsView( &dialog, *modelFramework.globalSettingsModel, *modelFramework.globalSettingsController );
	stretchView			= new MacStretchView( &dialog, *modelFramework.toolController, *modelFramework.stretchModel, *modelFramework.flowController, *modelFramework.optionsModel );
	pixelValueView		= new MacPixelValueView( &dialog, *modelFramework.pixelValueModel, *modelFramework.globalSettingsModel );
	statisticsView		= new MacStatisticsView( &dialog, *modelFramework.statisticsModel, *modelFramework.globalSettingsModel );
	toolView			= new MacToolView( &dialog, *modelFramework.toolModel, *modelFramework.toolController );
	
	histogramView		= new MacHistogramView( &dialog, *modelFramework.histogramModel,  *modelFramework.toolModel, *modelFramework.statisticsModel, 
											    *modelFramework.stretchModel, *modelFramework.optionsModel, *modelFramework.toolController, 
											    *modelFramework.histogramController, *modelFramework.flowController );
	previewView			= new MacPreviewView( &dialog, *modelFramework.toolController, *modelFramework.previewModel, *modelFramework.toolModel, 
											  *modelFramework.globalSettingsModel, *modelFramework.planeModel, *modelFramework.progressModel,
											  *modelFramework.previewController, *modelFramework.flowController );
	planeView			= new MacPlaneView( &dialog, *modelFramework.planeModel, *modelFramework.flowController, *modelFramework.planeController );
	headerView			= new MacHeaderView( &dialog, *modelFramework.headerModel );
	metadataView		= new MacMetadataView( dialog, *modelFramework.taxonomyEditorModel, *modelFramework.taxonomyEditorController, *modelFramework.repositoryModel, *modelFramework.repositoryController, *modelFramework.planeModel, *modelFramework.stretchModel, *modelFramework.histogramModel );
	preferencesView     = new MacPreferencesView( &dialog, *modelFramework.optionsModel, *modelFramework.optionsController );
	
	saveButton			= new BaseControl( &dlog, kFITSUIButtonSaveFile );
	openButton			= new BaseControl( &dlog, kFITSUIButtonOpenFile );
	editButton			= new BaseControl( &dlog, kFITSUIButtonSaveAndEdit );
	aboutButton			= new BaseControl( &dlog, kFITSUIButtonAbout );
	
  	static const EventTypeSpec menuEventTypes[] = { 
		{ kEventClassCommand, kEventCommandProcess }
  	};	
	
	saveButton->installEventHandler( menuEventTypes, GetEventTypeCount( menuEventTypes ), this );
	openButton->installEventHandler( menuEventTypes, GetEventTypeCount( menuEventTypes ), this );
	editButton->installEventHandler( menuEventTypes, GetEventTypeCount( menuEventTypes ), this );
	aboutButton->installEventHandler( menuEventTypes, GetEventTypeCount( menuEventTypes ), this );
	
	logoControl			= new LogoControl( &dlog, kFITSUICustomLogo );
	
	progressModel = modelFramework.progressModel;
	progressModel->Attach(this);
}

MacMainView::~MacMainView() {
	progressModel->Detach(this);
	removeCustomHandler();
	delete preferencesView;
	delete metadataView;
	delete headerView;
	delete planeView;
	delete previewView;			
	delete histogramView;		
	delete toolView;			
	delete statisticsView;		
	delete pixelValueView;		
	delete stretchView;			
	delete globalSettingsView;
	delete logoControl;
}

/**
 * Handle only tab switch events
 */
OSStatus MacMainView::processKeyboardEvent( EventHandlerCallRef handler, EventRef event ) {
	OSStatus result = eventNotHandledErr;

	switch( ::GetEventKind( event ) ) {
		case kEventRawKeyModifiersChanged: 
			UInt32 		modifiers;
	
			::GetEventParameter( event, ::kEventParamKeyModifiers, typeUInt32, NULL, sizeof (UInt32), NULL, &modifiers );
			
			cmdPressed =  !(modifiers & optionKey) && (modifiers & cmdKey) ;
			break;
	}
	
	return result; // We want the event to dispatch to other also.	
}

/**
 * Handle only tab switch events
 */
OSStatus MacMainView::onUnicodeForKeyEvent( EventRef unicodeEvent ) {
	EventRef event = NULL;
	::GetEventParameter( unicodeEvent, kEventParamTextInputSendKeyboardEvent, typeEventRef, NULL, sizeof(EventRef), NULL, &event );

	Char c;
		
	::GetEventParameter( event, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof (Char), NULL, &c );

	if( ::GetEventKind( event ) == kEventRawKeyDown ) {
		if( cmdPressed ) {
			switch( c ) {
				case '1': dialog.getTabControl().switchTab( 1 ); break;
				case '2': dialog.getTabControl().switchTab( 2 ); break;
				case '3': dialog.getTabControl().switchTab( 3 ); break;
				case '4': dialog.getTabControl().switchTab( 4 ); break;
			}
		} else if(c == kFITSCancelOperationShortcut) {
            progressModel->Cancel();
        }
	}
	
	return eventNotHandledErr; // We want the event to dispatch to other also.	
}

OSStatus MacMainView::onCommand( HICommand* command ) {
	OSStatus status = noErr;
	
	switch( command->commandID ) {
		case kFITSUICommandSaveFile:
			onSave();
			break;
			
		case kFITSUICommandOpenFile:
			onOpen();
			break;
			
		case kFITSUICommandSaveAndEdit:
			onEdit();
			break;
			
		case kFITSUICommandAbout:
			{
				FitsAboutDialog about( Environment::getBundleFactory() );
				
				about.show();
				about.runModalEventLoop();
				about.hide();
				about.dispose();
			}
			break;
			
		default:
			status = eventNotHandledErr;
	}
	
	return status;	
}

void MacMainView::onOpen() {
	dialog.getTabControl().switchTab( 1 );
	navDialog.showOpenSheet(dialog.getWindow(), &MacMainView::openFileCallback, this);	
}

void MacMainView::onSave() {
	dialog.getTabControl().switchTab( 1 );
	navDialog.showSaveSheet(dialog.getWindow(), 
							Environment::getFilePart(MainView::model.getSaveFileName()) ,
							&MacMainView::saveFileCallback, this);	
}

void MacMainView::onEdit() {
	dialog.getTabControl().switchTab( 1 );
	navDialog.showSaveSheet(dialog.getWindow(), 
							Environment::getFilePart(MainView::model.getSaveFileName()),
							&MacMainView::editFileCallback, this);
}

void MacMainView::onPreferences() {
	preferencesView->showPreferencesDialog();
}

void MacMainView::openFileCallback(void* userData) {
	MacMainView* self = reinterpret_cast<MacMainView*> (userData);

	string path = self->navDialog.getPath();
	if( ImageReader::IsSupported(path) ) {
		self->modelFramework.openNewFile(path);
	}
}

void MacMainView::saveFileCallback(void* userData) {
	MacMainView* self = reinterpret_cast<MacMainView*> (userData);
	self->modelFramework.saveFile(self->navDialog.getPath());
}

void MacMainView::editFileCallback(void* userData) {
	MacMainView* self = reinterpret_cast<MacMainView*> (userData);
	self->modelFramework.saveEdit(self->navDialog.getPath());
}

pascal OSStatus
MacMainView::marshall(EventHandlerCallRef handler, EventRef event, void* userData) {
	MacMainView* self = reinterpret_cast<MacMainView*>(userData);
	MacChangeManager* cm = reinterpret_cast<MacChangeManager*>(self->modelFramework.changeManager);
	
	cm->Invoke();
	return noErr;
}

void
MacMainView::installCustomHandler() {
	static const EventTypeSpec customEventTypes[] = {
		{ MacChangeManager::kEventClassCustom, MacChangeManager::kEventKindCustomUpdate }
	};

	customHandlerProc = ::NewEventHandlerUPP(marshall);
	OSStatus res = ::InstallApplicationEventHandler(customHandlerProc,
		GetEventTypeCount(customEventTypes), customEventTypes, (void*)this, &customHandler);
		
	if(noErr != res) {
		throw Exception("Couldn't install custom event handler.");
	}
}

void
MacMainView::removeCustomHandler() {
	::RemoveEventHandler(customHandler);
	::DisposeEventHandlerUPP(customHandlerProc);
}

void
MacMainView::Update(Models* models) {
	MainView::Update(models);
	
	Models::iterator it;
	for(it = models->begin(); it != models->end(); ++it) {
		if(progressModel == *it) {
			UpdateProgress();
		}
	}
}

void
MacMainView::UpdateProgress() {
	if(progressModel->QueryCancel() && progressModel->QueryInitializing()) {
        dialog.quitModalEventLoop();
    }
}

void
MacMainView::updateTitle(const String& title) {
	CFStringRef titleRef = ::CFStringCreateWithCString(kCFAllocatorDefault, title.c_str(), kCFStringEncodingMacRoman);
	dialog.setTitle(titleRef);
	::CFRelease(titleRef);
}

void
MacMainView::updateNetworkWarning(bool showNetworkWarning) {
	BaseControl networkWarning(&dialog, kFITSUIIconNetworkWarning);
	networkWarning.setVisibility(showNetworkWarning, true);	
}
