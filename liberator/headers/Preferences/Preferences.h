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

#ifndef __PREFERENCES_H__
#define __PREFERENCES_H__

#include <deque>
#include "XMLSettingsTree.h"
#include "Types.h"
#include "FitsSession.h"
#include "GlobalSettingsModel.h"

#define PREFERENCES_MAJOR_VERSION 1
#define PREFERENCES_MINOR_VERSION 1

using namespace std;

namespace FitsLiberator {
	namespace Preferences {
		
		/**
		 * Represents a fits file with an associated FitsSession.
		 *
		 * Used for storing a session together with a filename.
		 */	
		struct FileSession {
				String filename;
				FitsLiberator::FitsSession session;
		};

		#ifdef __GNUC__
            typedef __gnu_cxx::hash_map<String, String> StringDictionary;
        #else
            typedef stdext::hash_map< String, String > StringDictionary;
        #endif

        using FitsLiberator::Engine::StretchFunction;
        using FitsLiberator::Engine::InitialGuess;
        using FitsLiberator::Modelling::ImageInformationSettings;

		/** Class used for storing and retrieving the preferences
		*	files (provides high-level access to the SettingsTree).
		*   Use functions store() and load() to save/read the preference file.
		*/
		class Preferences {
            // Preview options --------------------------------------------------------------------
			Bool    preview;        ///< State of preview (enabled/disabled)
			Bool 	markUndefined;	///< State of mark undefined options (enabled/disabled)
			Bool 	markWhite;		///< State of mark white clipping (enabled/disabled)
			Bool 	markBlack;		///< State of mark black clipping (enabled/disabled)

            // Statistics options -----------------------------------------------------------------
			ImageInformationSettings imageInformation; ///< Whether to show scaled or stretched values

            // History options --------------------------------------------------------------------
			Bool 	                    freeze;			///< State of freeze settings (enabled/disabled)
            FitsLiberator::FitsSession	freezeSession;	///< The session data for the latest opned file.
			deque<FileSession *>  		loadedFiles;	///< A deque of the loaded files. Deque is used to beause it supports constant time insertion and removal of elements at the beginning of the sequence.
            StringDictionary            metaData;		///< Metadata saved values and tempaltes. 

            // Installation options ---------------------------------------------------------------
            Bool    xmpInstalled;		    ///< Whether XMP panels has been installed - affects OS X only.

            // House keeping ----------------------------------------------------------------------
            String  libMajorVersion;	    ///< Major version number of Liberator used to save preference file.
			String  libMinorVersion;	    ///< Minor version number of Liberator used to save preference file.
			Int		prefVersionMajor;	    ///< Major version number of preference file.
			Int		prefVersionMinor;	    ///< Minor version number of preference file.
			const String& filename;			///< Filename of the preference file.
			XmlSettingsTree* settingsTree;	///< The SettingsTree representing the preference file.
		public:
            /** Constructor for the Preferences class. Uses values from FitsBehaviour.h to 
                initialize local variabels in case the preference file can't be loaded.
                @param f Path to preference file. E.g. 
                /Users/lars/Library/Preferences/org.spacetelescope.fits_liberator.plist. */
			Preferences( const String& f );
            /** Destructor of Preferences class. */
			~Preferences();
			
			Void store();
			Void load();
		
			Void setGlobalSettings( Bool, Bool, Bool, Bool, Bool, FitsLiberator::Modelling::ImageInformationSettings );
			Void addLoadedFile( String&, FitsLiberator::FitsSession& );
			Void setFreezeSettings( FitsLiberator::FitsSession& );
            Void addMetaField( const String&, const String& );
			Bool getPreviewSetting();
			Bool getFreezeSetting();
			Bool getMarkUndefinedSetting();
			Bool getMarkBlackClippingSetting();
			Bool getMarkWhiteClippingSetting();
			Bool getXMPPanelsInstalled();
			Void setXMPPanelsInstalled( Bool );
			
			String getLiberatorMajorVersion();
			String getLiberatorMinorVersion();
			String getLiberatorVersion();
			Int getPreferencesVersionMajor();
			Int getPreferencesVersionMinor();
			
			FitsLiberator::Modelling::ImageInformationSettings getImageInformationSetting();
			FitsLiberator::FitsSession& getFreezeSession();
			Bool getSession( String& file, FitsSession* );
            StringDictionary& getMetaData();

            Void cleanHistory();
			
			Bool flipped;
			
			Bool applyStretchValues;

            // Stretch options --------------------------------------------------------------------
            Int             defaultStretch;                 ///< Default stretch function
            Int             defaultGuess;                   ///< Default initial guess
            Double          defaultScaledPeak;              ///< Default scaled peak level
            Double          defaultBlackLevelPercentage;    ///< Default scaled background level
            Double          defaultWhiteLevelPercentage;    ///< Default auto scaling range

            // Histogram options ------------------------------------------------------------------
            UInt    histMarkers;    ///< Histogram markers
		private:
			Bool requirePrefsVersion(const Int, const Int);
		
			DictionaryNode*	storeGlobalSettings();
			ArrayNode* 		storeLoadedFiles();
			DictionaryNode* storeFile( String& filename, FitsLiberator::FitsSession& session );
			DictionaryNode* storeFitsSession( FitsLiberator::FitsSession& session );
            DictionaryNode* storeMetaData();

			Void 		loadGlobalSettings( DictionaryNode& );
			Void 		loadFile( DictionaryNode& );
			Void 		loadLoadedFiles( ArrayNode& );
            Void        loadMetaData( DictionaryNode& );
			Void 		loadBooleanNode( SettingsNode&, Bool* );
			Bool 		loadFitsSession( DictionaryNode&, FitsLiberator::FitsSession& );
			Bool 		loadIntegerNode( SettingsNode&, Int* );
			Bool 		loadRealNode( SettingsNode&, Double* );
            /** Load a stretch function node. 
                @param node Node representing the setting.
                @param property An output pointer to the variable represented by the node.
                @return True if sucessfully read, false otherwise. If the return value is false
                the value pointed to by property is not changed. */
            Bool loadStretchFunctionNode( SettingsNode& node, StretchFunction* property );
		};

	} // end namespace Preferences
} // end namespace FitsLiberator

#endif// __PREFERENCES_H__