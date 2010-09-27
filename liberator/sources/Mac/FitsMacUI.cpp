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

#include "FitsMacUI.h"

using namespace FitsLiberator;
using namespace FitsLiberator::Mac;

FitsMacUI::FitsMacUI() 
	: session(NULL), dialog(NULL), changeManager(NULL), modelFramework(NULL), mainView(NULL) {
		
	createMainMenu();
	mainMenu = new BaseMenu(::AcquireRootMenu());

	static const ::EventTypeSpec eventTypes[] = {
		{ kEventClassCommand, kEventCommandProcess },
	}; 
	mainMenu->installEventHandler(eventTypes, GetEventTypeCount(eventTypes), this);
}

FitsMacUI::~FitsMacUI() {
	dispose();
	
	MenuRef ref = mainMenu->getMenu();
	delete mainMenu;
	::ReleaseMenu(ref);
}

void FitsMacUI::create() {
	NavDialog openDialog;
	
	if(openDialog.showOpenDialog()) {
		dialog         = new FitsDialog(Environment::getBundleFactory());
		session        = new FitsSession();
		changeManager  = new MacChangeManager();
		modelFramework = new ModelFramework(changeManager, session, openDialog.getPath());
		mainView       = new MacMainView(*dialog, *modelFramework);
		
		static const ::EventTypeSpec eventTypes[] = {
			{ kEventClassWindow, kEventWindowClosed },
			{ kEventClassWindow, kEventWindowShown }
		}; 
		
		dialog->installEventHandler( eventTypes, GetEventTypeCount( eventTypes ), this );		
		dialog->show();
		modelFramework->updateViews();	
	}
}

void FitsMacUI::dispose() {
    if(session != NULL) {
		modelFramework->saveStateToSession( true );
		
		delete mainView;       mainView       = NULL;
		delete modelFramework; modelFramework = NULL;
		delete changeManager;  changeManager  = NULL;
		delete session;        session        = NULL;
		delete dialog;         dialog         = NULL;
	}
}

void FitsMacUI::createMainMenu() {
	::IBNibRef	  menuNib;
	::CFBundleRef menuBundle = Environment::getBundleFactory()->getCFBundleRef();
	
	if( noErr != ::CreateNibReferenceWithCFBundle( menuBundle, CFSTR( kFITSNibFile ), &menuNib ) ) {
		throw Exception( "Can't create reference to NIB file" );
	}
	
	if( noErr != ::SetMenuBarFromNib( menuNib, CFSTR( kFITSNibMainMenu ) ) ) {
		throw Exception( "Can't set main menu" );
	}
	
	::DisposeNibReference( menuNib );
	Environment::getBundleFactory()->release( menuBundle );
	disableMenuItems();
}

void FitsMacUI::enableMenuItems() {
	::EnableMenuCommand(NULL, kHICommandAbout);
	::EnableMenuCommand(NULL, kHICommandPreferences);	
	::EnableMenuCommand(NULL, kHICommandOpen);
	::EnableMenuCommand(NULL, kHICommandClose);
	::EnableMenuCommand(NULL, kHICommandSaveAs);
	::EnableMenuCommand(NULL, kFITSUICommandSaveAndEdit);	
}

void FitsMacUI::disableMenuItems() {
	::EnableMenuCommand( NULL, kHICommandAbout);
	::DisableMenuCommand(NULL, kHICommandPreferences);	
	::EnableMenuCommand( NULL, kHICommandOpen);
	::DisableMenuCommand(NULL, kHICommandClose);
	::DisableMenuCommand(NULL, kHICommandSaveAs);
	::DisableMenuCommand(NULL, kFITSUICommandSaveAndEdit);	
}

OSStatus FitsMacUI::processWindowEvent( EventHandlerCallRef handler, EventRef event ) {
	OSStatus result = ::CallNextEventHandler(handler, event);
	
	switch( ::GetEventKind( event ) ) {
		case kEventWindowShown:
			enableMenuItems();
			break;
			
		case kEventWindowClosed:
			disableMenuItems();
			dispose();
			break;
	}
	
	return result;
}

OSStatus FitsMacUI::onCommand( HICommand* command ) {
	OSStatus status = noErr;
	
	switch( command->commandID ) {
		case kHICommandOpen:
			if(mainView != NULL) {
				mainView->onOpen();
			} else {
				create();
			}
			break;
			
		case kHICommandClose:
			if(dialog != NULL) {
				dialog->dispose();
			}
			break;
			
		case kHICommandSaveAs:
			if(mainView != NULL) {
				mainView->onSave();
			}
			break;
			
		case kFITSUICommandSaveAndEdit:
			if(mainView != NULL) {
				mainView->onEdit();
			}
			break;
			
		case kHICommandAbout:
			{
				FitsAboutDialog about( Environment::getBundleFactory() );
				
				about.show();
				about.runModalEventLoop();
				about.hide();
				about.dispose();
			}
			break;
		case kHICommandPreferences:
			if(mainView != NULL) {
				mainView->onPreferences();
			}
			break;
			
		case kHICommandQuit:
			if(dialog != NULL) {
				dialog->dispose();
			}
			break;
			
		default:
			status = eventNotHandledErr;
	}
	
	return status;	
}
