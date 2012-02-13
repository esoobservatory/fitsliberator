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
#include "Preferences.h"
#include "ImportSettings.h"

using namespace FitsLiberator::Preferences;
using namespace FitsLiberator::Engine;

#ifdef WINDOWS
using namespace std;
#endif

Preferences::Preferences( const String& f ) 
  : filename( f ), 
    settingsTree( NULL ), 
    libMajorVersion( FITSMAJORVERSION ), 
    libMinorVersion( FITSMINORVERSION ), 
    prefVersionMajor( PREFERENCES_MAJOR_VERSION ),
    prefVersionMinor( PREFERENCES_MINOR_VERSION ) {

    settingsTree = new XmlSettingsTree( f );
	
	preview 		= kFITSDefaultShowPreview;
	markUndefined	= kFITSDefaultShowUndefined;
	markWhite 		= kFITSDefaultShowWhiteClipped;
	markBlack 		= kFITSDefaultShowBlackClipped;

    histMarkers     = kFITSDefaultHistMarkers;

    imageInformation = kFITSDefaultImageInformation;

	flipped = false;

	applyStretchValues = true;

    defaultStretch              = kFITSDefaultStretch;
    defaultGuess                = kFITSDefaultGuess;
    defaultScaledPeak           = kFITSDefaultRescaleFactor;
    defaultBlackLevelPercentage = kFITSInitialGuessMinPercent;
    defaultWhiteLevelPercentage = kFITSInitialGuessMaxPercent;

    xmpInstalled	= false;

	freeze 			= kFITSDefaultFreezeSettings;
}

Preferences::~Preferences(){
	if( settingsTree != NULL ) { delete settingsTree; }
	
    cleanHistory();
}

/**
 * Test if loaded preference version is greather than a specified version. 
 *
 * @param major	Require at least this major version
 * @param minor	Require at least this minor version in major version.
 * 
 * @return True if loaded preference version is equal to or greather than required version, otherwise false.
 */
Bool Preferences::requirePrefsVersion(const Int major, const Int minor) 
{
	return ( prefVersionMajor > major || ( prefVersionMajor == major && prefVersionMinor >= minor) );
}

/**
 * Retrive minor version number of loaded preferences.
 * 
 * @return Minor version number
 */
Int Preferences::getPreferencesVersionMinor() {
	return prefVersionMinor;	
}

/**
 * Retrive major version number of loaded preferences.
 * 
 * @return Major version number
 */
Int Preferences::getPreferencesVersionMajor() {
	return prefVersionMajor;	
}


/**
 * Retrive Liberator minor version string used for saving Preference file.
 * 
 * @return Liberator version string. E.g ".1"
 */
String Preferences::getLiberatorMinorVersion() {
	return libMinorVersion;	
}

/**
 * Retrive Liberator major version string used for saving Preference file.
 * 
 * @return Liberator version string. E.g "3"
 */
String Preferences::getLiberatorMajorVersion() {
	return libMajorVersion;	
}

/**
 * Retrive Liberator version string used for saving Preference file.
 * 
 * @return Liberator version string. E.g "3.0"
 */
String Preferences::getLiberatorVersion() {
	return libMajorVersion + "." + libMinorVersion;	
}

/**
 * Determine if the XMP panels has been installed.
 *
 * @return True if XMP panels installed, false otherwise.
 *
 * @warn Should only be used on OS X.
 */
Bool Preferences::getXMPPanelsInstalled() {
	return xmpInstalled;
}

/**
 * Sets if the XMP panels has been installed.
 *
 * @param value True if panels installed, false otherwise.
 *
 * @warn Should only be used on OS X.
 */
Void Preferences::setXMPPanelsInstalled( Bool value ) {
	xmpInstalled = value;
}

/**
 * Retrive a fits session for a specfied file.
 *
 * @param filename	Filename of the FITS file about to load.
 * @param session	On output a pointer a session if the file previously has been loaded, otherwise unspecified.
 * @param 			True if a entry corrosponding to the file was found and the session was filled, otherwise false.  
 */
Bool Preferences::getSession( String& filename, FitsLiberator::FitsSession* session ) {
	for( deque<FileSession*>::iterator it = loadedFiles.begin(); it != loadedFiles.end(); it++ ) {
		if( *it != NULL  && (*it)->filename == filename ) {
			*session = (*it)->session;
			return true;
		}
	}

	return false;
}

/**
 * Add a FITS file to the list of loaded files.
 *
 * @param filename	Filename of the FITS file incl. path.
 * @oaram session	A reference to the session of the loaded file.
 */
Void Preferences::addLoadedFile( String& filename, FitsLiberator::FitsSession& session ) {
	FileSession* data = new FileSession;
	data->filename 	= filename;
	data->session 	= session;
	
	// Check if file is already in list, and replace data if it is.
	for( deque<FileSession*>::iterator it = loadedFiles.begin(); it != loadedFiles.end(); it++ ) {
		if( *it != NULL  && (*it)->filename == filename ) {
			delete *it;
			*it = data;
			return;
		}
	}	
	
	// file was not in list, so add it.
	if( loadedFiles.size() > KFITSNumberOfLoadedFiles ) {
		loadedFiles.pop_back();
	}
	
	loadedFiles.push_front( data );
}

/**
 * Adds metadata field to the preferences.
 *
 * @param name	Name of the field.
 * @param value Value of the field.
 */
Void Preferences::addMetaField( const String& name, const String& value ) {
    metaData[name] = value;
}

/**
 * Set the current values of the current session as the freeze settings.
 *
 * @param The freeze session
 */
Void Preferences::setFreezeSettings( FitsLiberator::FitsSession& session ) {
	freezeSession = session;	
}

/**
 * Save the preference files (build settings tree and save it as an XML document).
 */
Void Preferences::store() {
	DictionaryNode* root = new DictionaryNode();
	root->addKey( "GlobalSettings", storeGlobalSettings() );
	root->addKey( "FreezeSettings", storeFitsSession( freezeSession ) );
	root->addKey( "LoadedFiles", storeLoadedFiles() );
    root->addKey( "MetaData", storeMetaData() );
	root->addKey( "MajorVersion", new IntegerNode( PREFERENCES_MAJOR_VERSION ) );
	root->addKey( "MinorVersion", new IntegerNode( PREFERENCES_MINOR_VERSION ) );
	root->addKey( "LiberatorMajorVersion", new StringNode( FITSMAJORVERSION ) );
	root->addKey( "LiberatorMinorVersion", new StringNode( FITSMINORVERSION ) );

	// SettingsTree takes care of destroying root node.
	settingsTree->setSettingsTree( root );
	settingsTree->saveFile();
}

/**
 * Save the global settings (build settings tree).
 */
DictionaryNode* Preferences::storeGlobalSettings() {
	DictionaryNode* globalNode = new DictionaryNode();

	globalNode->addKey("Preview", new BooleanNode( preview ) );
	globalNode->addKey("Freeze", new BooleanNode( freeze ) );
	globalNode->addKey("MarkUndefined", new BooleanNode( markUndefined ) );
	globalNode->addKey("MarkWhiteClipped", new BooleanNode( markWhite ) );
	globalNode->addKey("MarkBlackClipped", new BooleanNode( markBlack ) );
	globalNode->addKey("ImageInformation", new IntegerNode( imageInformation ) );
	globalNode->addKey("XMPInstalled", new BooleanNode( xmpInstalled ) );
    globalNode->addKey("HistogramMarkers", new IntegerNode( static_cast<Int>(histMarkers) ) );
    globalNode->addKey("DefaultStretch", new IntegerNode( defaultStretch ) );
    globalNode->addKey("DefaultGuess", new IntegerNode( defaultGuess ) );
    globalNode->addKey("DefaultBlackLevelPercentage", new RealNode( defaultBlackLevelPercentage ) );
    globalNode->addKey("DefaultWhiteLevelPercentage", new RealNode( defaultWhiteLevelPercentage ) );
    globalNode->addKey("DefaultScaledPeak", new RealNode( defaultScaledPeak ) );
	//globalNode->addKey("Flip", new BooleanNode( flipped ) );
	globalNode->addKey("ApplyStretchValues", new BooleanNode( applyStretchValues ) );

    return globalNode;
}

/**
 * Save the loaded files (build settings tree).
 *
 * @return A settings tree
 */
ArrayNode* Preferences::storeLoadedFiles() {
	ArrayNode* loadedFilesNode = new ArrayNode();
	
	for( deque<FileSession*>::iterator it = loadedFiles.begin(); it != loadedFiles.end(); it++ ) {
		if( *it != NULL ) {
			loadedFilesNode->addChild( storeFile( (*it)->filename, (*it)->session ) );
		}
	}
		
	return loadedFilesNode;
}

/**
 * Save a single file (build settings tree).
 *
 * @param filename	Filename of the FITS file.
 * @param session	FITS Session for the given filename.
 *
 * @return A settings tree
 */
DictionaryNode* Preferences::storeFile( String& filename, FitsLiberator::FitsSession& session ) {
	DictionaryNode* fileNode = new DictionaryNode();
	fileNode->addKey( "File", new StringNode( filename ) );
	fileNode->addKey( "Image", new IntegerNode( session.plane.imageIndex ) );
	fileNode->addKey( "Plane", new IntegerNode( session.plane.planeIndex ) );
	fileNode->addKey( "Session", storeFitsSession( session ) );
//	fileNode->addKey( "Flip", new BooleanNode( session.flip.flipped ) );
	return fileNode;
}

/**
 * Save a FITS session (build settings tree).
 *
 * @param session 	A FITS session
 *
 * @return 			A settings tree
 */
DictionaryNode* Preferences::storeFitsSession( FitsLiberator::FitsSession& session ) {
	DictionaryNode* sessionNode = new DictionaryNode();
	sessionNode->addKey( "BackgroundLevel", new RealNode( session.stretch.offset ) );
	sessionNode->addKey( "PeakLevel", new RealNode( session.stretch.peakLevel ) );
	sessionNode->addKey( "ScaledPeakLevel", new RealNode( session.stretch.scalePeakLevel ) );
	sessionNode->addKey( "Scale", new RealNode( session.stretch.scale ) );
	sessionNode->addKey( "BlackLevel", new RealNode( session.stretch.blackLevel ) );
	sessionNode->addKey( "WhiteLevel", new RealNode( session.stretch.whiteLevel ) );
	sessionNode->addKey( "NullValues", new IntegerNode( session.importSettings.undefinedSettings ) );
	sessionNode->addKey( "Channels", new IntegerNode( session.importSettings.channelSettings ) );
	sessionNode->addKey( "StretchFunction", new IntegerNode( session.stretch.function ) );
	sessionNode->addKey( "Flip", new BooleanNode( session.flip.flipped ) );
	sessionNode->addKey( "ApplyStretchValues", new BooleanNode( session.applyStretchValues ) );
	return sessionNode;
}

/**
 * Creates the nodes required to store the metadata.
 *
 * @return A settings tree.
 */
DictionaryNode* Preferences::storeMetaData() {
    DictionaryNode* metaNode = new DictionaryNode();

    for( StringDictionary::iterator i = metaData.begin(); i != metaData.end(); i++ ) {
        metaNode->addKey( (*i).first, new StringNode( (*i).second ) );
    }

    return metaNode;
}


/**
 * Load the preferences from and XML document.
 *
 * @TODO Doesn't take malformed preference files into account.
 */
Bool Preferences::load() {
	if( settingsTree->loadFile() ) {
		if( settingsTree->getSettingsTree()->getType() == SETTINGS_TREE_DICTIONARY ) {
			DictionaryNode* node = (DictionaryNode *) settingsTree->getSettingsTree();
			
			// Version Check
			if( node->containsKey( "MajorVersion" ) ) {
				SettingsNode& versionNode = node->getKey("MajorVersion");
			
				if( versionNode.getType() == SETTINGS_TREE_INTEGER && ((IntegerNode&) versionNode).getValue() <= PREFERENCES_MAJOR_VERSION ) {			
					// Store major version used in loaded preference file.
					prefVersionMajor = ((IntegerNode&) versionNode).getValue();
					
					// Retrive minor version used in loaded preference file 
					if( node->containsKey( "MinorVersion" ) ) {
						SettingsNode& minorVersionNode = node->getKey("MinorVersion");
					
						if( minorVersionNode.getType() == SETTINGS_TREE_INTEGER ) {
							prefVersionMinor = ((IntegerNode&) minorVersionNode).getValue();
						}
					}
					
					// Retrive liberator version used to save the loaded preference file.
					if( requirePrefsVersion(1,1) && node->containsKey( "LiberatorMajorVersion" ) && node->containsKey( "LiberatorMajorVersion" ) ) {
						SettingsNode& liberatorMajVerNode = node->getKey("LiberatorMajorVersion");
						SettingsNode& liberatorMinVerNode = node->getKey("LiberatorMinorVersion");
					
						if( liberatorMajVerNode.getType() == SETTINGS_TREE_STRING && liberatorMinVerNode.getType() == SETTINGS_TREE_STRING ) {
							libMajorVersion = ((StringNode&) liberatorMajVerNode).getValue();
							libMinorVersion = ((StringNode&) liberatorMinVerNode).getValue();
						} else {
							libMajorVersion = FITSMAJORVERSION;
							libMinorVersion = FITSMINORVERSION;
						}						
					} else {
						// Since Liberator 2.0 uses pref v. 1.0 and doesn't have LiberatorVersion, 
						// we hard code this fact.
						libMajorVersion = "2";
						libMinorVersion = "0";
					}
				
					// Load all other elements.
					for( node->beginChild(); node->hasMoreChildren(); node->nextChild() ) {
						const String& key 		= node->nextChildKey();
						SettingsNode& childNode	= node->nextChildValue();
						
						if( key == "GlobalSettings" && childNode.getType() == SETTINGS_TREE_DICTIONARY ) {
							loadGlobalSettings( (DictionaryNode&) childNode );	
						} else if ( key == "FreezeSettings" && childNode.getType() == SETTINGS_TREE_DICTIONARY ) {
							loadFitsSession( (DictionaryNode&) childNode, freezeSession );
						} else if ( key == "LoadedFiles" && childNode.getType() == SETTINGS_TREE_ARRAY ) {
							loadLoadedFiles( (ArrayNode&) childNode );
                        } else if( key == "MetaData" && childNode.getType() == SETTINGS_TREE_DICTIONARY ) {
                            loadMetaData( (DictionaryNode&) childNode );
                        }
					}
				}
			}
		} else {
			return false;
			// junk in file, use default values
		}
	} else {
		return false;
		// couldn't load file, use default values.	
	}
	return true;
}

/**
 * Load a FITS session
 *
 * @param node 		Node representing a settings tree.
 * @param session	On output the sessions has been filled with the session in the node.
 *
 * @return			True if successfully read, false otherwise.
 */
Bool Preferences::loadFitsSession( DictionaryNode& node, FitsLiberator::FitsSession& session ) {
	Bool hasPeakLevel = false, hasScale = false, hasScaledPeakLevel = false, hasBgLevel = false, hasBlackLevel = false, hasWhiteLevel = false, hasChannels = false, hasFunction = false, hasNull = false;


	for( node.beginChild(); node.hasMoreChildren(); node.nextChild() ) {
		const String& key 		= node.nextChildKey();
		SettingsNode& childNode	= node.nextChildValue();
		Int tmpInt;
				
		if( key == "BackgroundLevel" ) {
			hasBgLevel = loadRealNode( childNode, &(session.stretch.offset) );
		} else if( key == "BlackLevel" ) {
			hasBlackLevel = loadRealNode( childNode, &(session.stretch.blackLevel) );
		} else if( key == "WhiteLevel" ) {
			hasWhiteLevel = loadRealNode( childNode, &(session.stretch.whiteLevel) );
		} else if( key == "ScaledPeakLevel" ) {
			hasScaledPeakLevel = loadRealNode( childNode, &(session.stretch.scalePeakLevel) );
		} else if( key == "PeakLevel" ) {
			hasPeakLevel = loadRealNode( childNode, &(session.stretch.peakLevel) );
		} else if( key == "Scale" ) {
			hasScale = loadRealNode( childNode, &(session.stretch.scale) );
		} else if( key == "NullValues" ) {
			// Don't pass the value tmpInt directly to session!!!!! It won't work on Mac.
		
			hasNull = loadIntegerNode( childNode, &tmpInt );
		
			switch( tmpInt ) {
				case undefinedBlack:
					session.importSettings.undefinedSettings = undefinedBlack;
					break;
					
				case undefinedTransparent:
					session.importSettings.undefinedSettings = undefinedTransparent;
					break;
					
				default:
					session.importSettings.undefinedSettings = kFITSDefaultUndefined;
			}
		} else if( key == "Channels" ) {
			// Don't pass the value tmpInt directly to session!!!!! It won't work on Mac.
			hasChannels = loadIntegerNode( childNode, &tmpInt );
		
			switch( tmpInt ) {
				case channel8:
					session.importSettings.channelSettings = channel8;
					break;
					
				case channel16:
					session.importSettings.channelSettings = channel16;
					break;
				
				case channel32:
					session.importSettings.channelSettings = channel32;
					break;
					
				default:
					session.importSettings.channelSettings = kFITSDefaultBitDepth;
			}
			
		} else if( key == "StretchFunction" ) {
            hasFunction = loadStretchFunctionNode( childNode, &session.stretch.function );
		} else if ( key == "Flip" ) {
			loadBooleanNode( childNode, &session.flip.flipped );
		} else if ( key == "ApplyStretchValues" ) {
			loadBooleanNode( childNode, &session.applyStretchValues );
		}

	}
	
	if( hasScale && hasBgLevel && hasBlackLevel && hasWhiteLevel && hasPeakLevel && hasChannels && hasFunction && hasNull & hasScaledPeakLevel ) {
		return true;
	} else {
		return false;
	}
}

/**
 * Load a double node
 *
 * @param node 		Node representing a settings tree.
 * @param property	On output a pointer to the double represented by the node.
 *
 * @return			True if successfully read, false otherwise.
 */
Bool Preferences::loadRealNode( SettingsNode& node, Double* property ) {
	if( node.getType() == SETTINGS_TREE_REAL && property != NULL ) {
		*property = ((RealNode&) node).getValue();
		return true;
	} else {
		return false;
	}
}

/**
 * Load a integer node
 *
 * @param node 		Node representing a settings tree.
 * @param property	On output a pointer to the integer represented by the node.
 *
 * @return			True if successfully read, false otherwise.
 */
Bool Preferences::loadIntegerNode( SettingsNode& node, Int* property ) {
	if( node.getType() == SETTINGS_TREE_INTEGER && property != NULL ) {
		*property = ((IntegerNode&) node).getValue();
		return true;
	} else {
		return false;
	}
}

Bool Preferences::loadStretchFunctionNode( SettingsNode& node, StretchFunction* property ) {
    Int tmpInt; StretchFunction function;

    if( loadIntegerNode( node, &tmpInt ) ) {
	    switch( tmpInt ) {
		    case stretchLinear:
			    function = stretchLinear;
			    break;
		    case stretchLog:
			    function = stretchLog;
			    break;
		    case stretchSqrt:
			    function = stretchSqrt;
			    break;
		    case stretchLogSqrt:
			    function = stretchLogSqrt;
			    break;
		    case stretchLogLog:
			    function = stretchLogLog;
			    break;
		    case stretchCubeR:
			    function = stretchCubeR;
			    break;
		    case stretchAsinh:
			    function = stretchAsinh;
			    break;
		    case stretchRoot4:
			    function = stretchRoot4;
			    break;
		    case stretchRoot5:
			    function = stretchRoot5;
			    break;
		    case stretchAsinhAsinh:
			    function = stretchAsinhAsinh;
			    break;
		    case stretchAsinhSqrt:
			    function = stretchAsinhSqrt;
			    break;
		    default:
			    return false;
        }

        *property = function;
        return true;
    }

    return false;
}

/**
 * Load a settings for a FITS file an put in on the deque.
 *
 * @param node 		Node representing a settings tree.
 */
Void Preferences::loadFile( DictionaryNode& node ) {
	// To make sure we read all fields.
	Bool hasFile = false, hasImage = false, hasPlane = false, hasSession = false;
	
	FileSession* data = new FileSession;
	
	for( node.beginChild(); node.hasMoreChildren(); node.nextChild() ) {
		const String& key 		= node.nextChildKey();
		SettingsNode& childNode	= node.nextChildValue();
		
		if( key == "File" && childNode.getType() == SETTINGS_TREE_STRING ) {
			data->filename = ((StringNode&) childNode).getValue();
			hasFile = true;
		} else if ( key == "Image" && childNode.getType() == SETTINGS_TREE_INTEGER ) {
			data->session.plane.imageIndex = ((IntegerNode&) childNode).getValue();
			hasImage = true;
		} else if ( key == "Plane" && childNode.getType() == SETTINGS_TREE_INTEGER ) {
			data->session.plane.planeIndex = ((IntegerNode&) childNode).getValue();
			hasPlane = true;
		} else if ( key == "Session" && childNode.getType() == SETTINGS_TREE_DICTIONARY ) {
			hasSession = loadFitsSession( (DictionaryNode&) childNode, data->session );
		} else if ( key == "Flip" && childNode.getType() == SETTINGS_TREE_BOOLEAN ) {
			data->session.flip.flipped = ((BooleanNode&) childNode).getValue();
		} else if ( key == "ApplyStretchValues" && childNode.getType() == SETTINGS_TREE_BOOLEAN ) {
			data->session.applyStretchValues = ((BooleanNode&) childNode).getValue();
		}
	}
		
	if( hasFile && hasImage && hasPlane && hasSession && loadedFiles.size() < KFITSNumberOfLoadedFiles ) {
		loadedFiles.push_back( data );
	} else {
		delete data;
	}
}

/**
 * Load settings for all files
 *
 * @param node 		Node representing a settings tree.
 */
Void Preferences::loadLoadedFiles( ArrayNode& node ) {
	for( Int i = 0; i < node.countChildren(); i++ ) {
		if( node[i].getType() == SETTINGS_TREE_DICTIONARY ) {
			loadFile( (DictionaryNode&) node[i] );
		}
	}
}

/**
 * Load the metadata from the preferences file.
 *
 * @param node 		Node representing a settings tree.
 */
Void Preferences::loadMetaData( DictionaryNode& node ) {
    for( node.beginChild(); node.hasMoreChildren(); node.nextChild() ) {
        const String& name  = node.nextChildKey();
        SettingsNode& value = node.nextChildValue();
        
        if( value.getType() == SETTINGS_TREE_STRING ) {
            addMetaField( name, ((StringNode&)value).getValue() );
        }
    }
}

/**
 * Load global settings
 *
 * @param node 		Node representing a settings tree.
 */
Void Preferences::loadGlobalSettings( DictionaryNode& node ) {
	for( node.beginChild(); node.hasMoreChildren(); node.nextChild() ) {
		const String& key 		= node.nextChildKey();
		SettingsNode& childNode	= node.nextChildValue();
		
		if( key == "Preview" ) { 
			loadBooleanNode( childNode, &preview ); 
		} else if( key == "Freeze" ) { 
			loadBooleanNode( childNode, &freeze ); 
		} else if( key == "MarkUndefined" ) { 
			loadBooleanNode( childNode, &markUndefined ); 
		} else if( key == "MarkWhiteClipped" ) { 
			loadBooleanNode( childNode, &markWhite ); 
		} else if( key == "XMPInstalled" ) { 
			loadBooleanNode( childNode, &xmpInstalled ); 
		} else if( key == "MarkBlackClipped" ) { 
			loadBooleanNode( childNode, &markBlack ); 
		} else if ( key == "ImageInformation" ) {
			Int tmpInt;
		
			loadIntegerNode( childNode, &tmpInt);
			
			switch( tmpInt ) {
				case FitsLiberator::Modelling::valuesStretched:
					imageInformation = FitsLiberator::Modelling::valuesStretched;
					break;
				case FitsLiberator::Modelling::valuesScaled:
					imageInformation = FitsLiberator::Modelling::valuesScaled;
					break;
			}
        } else if ( key == "HistogramMarkers" ) {
            loadIntegerNode( childNode, reinterpret_cast<Int*>(&histMarkers) );
        } else if ( key == "DefaultStretch" ) {
            loadIntegerNode( childNode, &defaultStretch );
        } else if ( key == "DefaultGuess" ) {
            loadIntegerNode( childNode, &defaultGuess );
        } else if ( key == "DefaultBlackLevelPercentage" ) {
            loadRealNode( childNode, &defaultBlackLevelPercentage );
        } else if ( key == "DefaultWhiteLevelPercentage" ) {
            loadRealNode( childNode, &defaultWhiteLevelPercentage );
        } else if ( key == "DefaultScaledPeak" ) {
            loadRealNode( childNode, &defaultScaledPeak );
		//} else if ( key == "Flip" ) {
		//	loadBooleanNode( childNode, &flipped );
		} else if ( key == "ApplyStretchValues" ) {
			loadBooleanNode( childNode, &applyStretchValues );
		}
	}
}

/**
 * Load a boolean node
 *
 * @param node 		Node representing a settings tree.
 * @param property	On output a pointer to the boolean represented by the node.
 *
 * @return			True if successfully read, false otherwise.
 */
Void Preferences::loadBooleanNode( SettingsNode& node, Bool* property ) {
	if( node.getType() == SETTINGS_TREE_BOOLEAN && property != NULL ) {
		*property = ((BooleanNode&) node).getValue();	
	} 
}

/**
 * Set the values of the global settings for later storing in the preference files.
 *
 * Store the values by calling Preferneces.store().
 *
 * @param p		Preview enabled/disabled
 * @param f		Freeze settings enabled/disabled
 * @param mU	Mark undefined enabled/disabled
 * @param mW	Mark white clipping enabled/disabled
 * @param mB	Mark black clipping enabled/disabled
 * @param iI	Show stretched or scaled values.
 */			
Void Preferences::setGlobalSettings( Bool p, Bool f, Bool mU, Bool mW, Bool mB, FitsLiberator::Modelling::ImageInformationSettings iI ) {
	preview 		= p;
	freeze 			= f;
	markUndefined 	= mU;
	markWhite 		= mW;
	markBlack	 	= mB;
	imageInformation = iI;
}

/**
 * Wheter to show scaled or stretched values (as of the preference file).
 */
FitsLiberator::Modelling::ImageInformationSettings Preferences::getImageInformationSetting() {
	return imageInformation;
}

/**
 * Get the session of the freeze settings option
 */			
FitsLiberator::FitsSession& Preferences::getFreezeSession() {
	return freezeSession;
}

/**
 * Get preview enabled/disabled
 */
Bool Preferences::getPreviewSetting() {
	return preview;
}

/**
 * Get freeze settings enabled/disabled
 */
Bool Preferences::getFreezeSetting() {
	return freeze;
}

/**
 * Get mark undefined enabled/disabled
 */
Bool Preferences::getMarkUndefinedSetting() {
	return markUndefined;
}

/**
 * Get mark black clipping enabled/disabled
 */
Bool Preferences::getMarkBlackClippingSetting() {
	return markBlack;
}

/**
 * Get mark white clipping enabled/disabled
 */
Bool Preferences::getMarkWhiteClippingSetting() {
	return markWhite;
}

/**
 * Get the stored metadata template. 
 */
StringDictionary& Preferences::getMetaData() {
    return metaData;
}

Void Preferences::cleanHistory() {
	// Clean up the file list
    for( deque<FileSession*>::iterator it = loadedFiles.begin(); it != loadedFiles.end(); it++ ) {
		if( *it != NULL ) {
			delete *it;
		}
	}

    loadedFiles.clear();
}