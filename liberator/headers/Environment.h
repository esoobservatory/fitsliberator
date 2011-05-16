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


#ifndef __ENVIRONMENT_H__
#define __ENVIRONMENT_H__

// Common includes
#include <time.h>
#include <stdlib.h>
#include <sstream>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include <bitset>

// Operating system includes
#ifdef WINDOWS // Windows only
	#include <limits.h>
	#include <string.h>
	#include <stdio.h>
	#include <windows.h>
	#include <windowsx.h>
	#include <errno.h>
	#include <iostream>
	#include <fstream>

	typedef HWND DialogPtr;
	typedef HWND DialogTHndl;
#else
	#include <Types.h>
	#include <Gestalt.h>
	#include <TextUtils.h>
	#include <OSUtils.h>
	#include <Menus.h>
	#include <string.h>
#endif

#include "FitsLiberator.h"
#include "ImportSettings.h"

#ifndef WINDOWS
	#include "BundleFactory.h"
//	#include "Installation.h"
#endif

namespace FitsLiberator {
    class CancelException : public Exception {
        public:
            CancelException() {};
    };
	/**
	*  Static class that implements certain general features
	*  such as system specific file handling and global information
	*/
    class Environment {
        public:
			/**	@brief	Display a message to the user.
				@param	title	Title of the message box.
				@param	message	Message to display.

				The message is displayed using a standard message box. */
			static void showMessage(const String& title, const String& message);

			static bool isNetworkPath(const String& fileName);
            static String getFilePart(const String& fileName );
		
			static String getPreferencesPath();
			
			static String readResource( String );
			static String getString( Int id );
			
			static Bool isShowToolTips();
			
			static Bool supportsBitDepth( FitsLiberator::Engine::ChannelSettings );
			static Bool supportsTransparent( FitsLiberator::Engine::ChannelSettings );
			static Bool supports32bitCoordinates();
			
			static void initPlatformResources( );
			static void releasePlatformResources();
		
			/**	@brief	Opens a file in the system specified editor.
				@param	path	Full path of the file to edit. */
			static void startSystemEditor(const String& path);

			// International Settings
			static String getDecimalSeparator();
			static String getThousandsSeparator();
			static String getNegativeSign();
			static String getPositiveSign();
			static Int    getDecimalPlaces();
			static Int	  getUnfixedDecimalPlaces();
			
			static const UInt FILEMODE_OPEN = 1;
			static const UInt FILEMODE_SAVE = 2;	
			static const Char newLine[];
			static const Char PATH_SEPARATOR[];		

			static UInt64 getMaxMemory();
			
		#ifdef DEBUG
            static Void DebugMessage( const String& file, UInt line, const String& message );
        #endif

        #ifdef WINDOWS
            static String getSettingsFile( String fileName );
			static Void InitializeUI();
            static Void DisposeUI();            
			static String LoadFileDialog( String fileName, UInt mode, HWND owner );
			typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
		#else
        	static Mac::BundleFactory* getBundleFactory();
        	static String CFStringToString( CFStringRef );
        	static CFStringRef StringToCFString( const Char* );
        	static CFURLRef getBundleResourceURL( const Char* );
        	static String getBundleResourceFilePath( const Char* name );     	
        #endif
        
		private:
            static Bool usePreciseCursor;
            static Bool showToolTips;

            static Bool hostIsElements;
            static UInt hostMajorVersion;

            #ifdef DEBUG
                static std::ofstream* debugLog;
            #endif
        
            #ifdef WINDOWS
                static ULONG_PTR gdiplusToken;      ///< GDI+ library token.
			#else
        	    static Mac::BundleFactory* bundleFactory;
            #endif
    };
}

#ifdef DEBUG
	#ifdef WINDOWS
	    #define Trace(message) FitsLiberator::Environment::DebugMessage( __FILE__, __LINE__, message )
    	#define TraceIf(condition, message) if(condition) {Trace(message);}
    #else
    	#define Trace(message) 
	    #define TraceIf(condition, message) condition	
    #endif
#else
    #define Trace(message) 
    #define TraceIf(condition, message) condition
#endif

#if _DEBUG
	#ifndef WINDOWS
		#define BaseDebugStr(x)	DebugStr("\p"x)
	#endif
#else
	#ifndef WINDOWS
		#define	BaseDebugStr(x)  
	#endif
#endif

#endif // __ENVIRONMENT_H__