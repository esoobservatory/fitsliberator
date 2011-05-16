//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//
//    File:
//        StandardMain.cpp
//
//    Description:
//        Entry point for all standard plug ins for Photoshop. Standard plug ins
//        are all plug ins except the Automation plug in type.
//
//
//-------------------------------------------------------------------------------
#pragma once

#ifndef WINVER
#define WINVER 0x0510
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0510
#endif

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <boost/shared_ptr.hpp>

#include "Environment.h"
#include "ApplicationWindow.h"
#include "MainDialog.h"
#include "ModelFramework.h"
#include "WindowsChangeManager.h"

using namespace FitsLiberator;
using namespace FitsLiberator::Modelling;
using namespace FitsLiberator::Windows;

HINSTANCE g_hInstance;

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	int result = 0;	// Assume everything will work out

	// Store our hInstance so that the UI can load resources and stuff.
	g_hInstance = hInstance;

	//if the supplied argument is wrapped in " characters
	if ( lpCmdLine[0] == '\"' ) {
		UInt length = strlen(lpCmdLine);

		lpCmdLine[length-1] = '\0';
		lpCmdLine++;
	}

    //
    // Create a modelling framework and the dialog
	ApplicationWindow app;
	WindowsChangeManager changeManager;
	FitsSession session;

	//in order to do this properly we have to make sure that the frameWork is deleted
	//before exiting this routine. In previous versions this was not a problem
	//since the user could not cancel until everything was loaded.
	//Now this is a possibility and the modelframe work may be initialized
	//already in this case and we therefore have to make sure we clean up.
	ModelFramework* frameWork = NULL;

    try {
		Environment::initPlatformResources();
		if(!app.createWindow()) {
			return 1;
		}

		//if the supplied file input is empty we should prompt for a filename
		String fileName = lpCmdLine;
		if ( fileName == "" )
			fileName = Environment::LoadFileDialog( "", Environment::FILEMODE_OPEN, app.getHandle() );

		//create the model framework. It will throw an exception if the 
		//user hit cancel instead of opening a file.	
		if ( fileName != "" ) {
			frameWork = new ModelFramework(&changeManager,  &session, fileName );

			MainDialog dlg( *frameWork );
			dlg.Modal( app.getHandle() );
			::PostQuitMessage(0);
			app.runMessagePump();
		}
	} catch ( ImageReaderException ex ) {		
		MainDialog::ShowFileOpenError();	// Inform the user
		result = 1;
	} catch( AssertException e ) {
		Environment::showMessage("Assertion failed", e.getMessage());
		result = 1;
	} catch( CancelException ) {
		// Nothing to do really
    }

	// The user tried to open something which we could not read.
	if ( frameWork != NULL )			// Clean up
		delete frameWork;
	Environment::releasePlatformResources();
	return result;
}
