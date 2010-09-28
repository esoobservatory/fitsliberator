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

/** @file
 * Class implementation of the MainBundleFactory and FilePathBundleFactory classes
 * which implements 
 */

#include "BundleFactory.h"

using namespace FitsLiberator::Mac;

//-------------------------------------------------------------------------------
//	MainBundleFactory implementation
//-------------------------------------------------------------------------------

/**
 * Returns a reference to the main bundle
 */
BundleFactory::~BundleFactory() {
}

//-------------------------------------------------------------------------------
//	MainBundleFactory implementation
//-------------------------------------------------------------------------------

/**
 * Returns a reference to the main bundle
 */
CFBundleRef MainBundleFactory::getCFBundleRef() {
	return ::CFBundleGetMainBundle();
}

//-------------------------------------------------------------------------------
//	IdentifierhBundleFactory implementation
//-------------------------------------------------------------------------------

/**
 * Constructor for IdentifierBundleFactory. Takes a bundle identifier as input.
 *
 * @param id	Identifier to Adobe plugin. IdentifierBundleFactory takes control
 *				of the CFStringRef, i.e. you're no longer responsiple for releasing it.
 */
IdentifierBundleFactory::IdentifierBundleFactory( ::CFStringRef id ) : identifier( id ), bundle( NULL ) {
}

/**
 * Destructor for IdentifierBundleFactory.
 */
IdentifierBundleFactory::~IdentifierBundleFactory( ) {
	if( identifier != NULL ) {
		::CFRelease( identifier );
		identifier = NULL;
	}
	
	// Do not release the bundle.
}

/**
 * Returns a reference to the bundle identified by a pre-specified identifier.
 */
CFBundleRef IdentifierBundleFactory::getCFBundleRef() {
	if( bundle == NULL ) {
		bundle = CFBundleGetBundleWithIdentifier( identifier );
    }
    
    return bundle;
}

/**
 * Not implemented. Desctructor takes care of disposing used resources.
 */
Void IdentifierBundleFactory::release( ::CFBundleRef b ) {
	//::CFRelease( b );
}

//-------------------------------------------------------------------------------
//	FilePathBundleFactory implementation
//-------------------------------------------------------------------------------

/**
 * Constructor for FilePathBundleFactory. Takes a file path as input.
 *
 * @param path	Path to the bundle.
 */
FilePathBundleFactory::FilePathBundleFactory( ::CFStringRef path ) : filePath( path ), bundle( NULL ) {
}

/**
 * Destructor for FilePathBundleFactory. 
 *
 * @param path	Path to the bundle.
 */
FilePathBundleFactory::~FilePathBundleFactory( ) {
	if( bundle != NULL ) {
		::CFRelease( bundle );
	}
}

/**
 * Returns a reference to the bundle located pre-specified path.
 *
 * @todo Must take UFS pahts into account
 */
CFBundleRef FilePathBundleFactory::getCFBundleRef() {
	if( bundle == NULL ) {
		
	
		::CFURLRef	bundleURL 	= ::CFURLCreateWithFileSystemPath( kCFAllocatorDefault, filePath, kCFURLPOSIXPathStyle, TRUE );
    				bundle 		= ::CFBundleCreate( kCFAllocatorDefault, bundleURL );
    
	    ::CFRelease( bundleURL );
    }
    
    return bundle;
}

/**
 * Not implemented. Desctructor takes care of disposing used resources.
 */
Void FilePathBundleFactory::release( ::CFBundleRef b ) {
	//::CFRelease( b );
}

/**
 * Set the file paths
 *
 * @param path File path of bundle.
 */
Void FilePathBundleFactory::setFilePath( ::CFStringRef path ) {
	if( bundle != NULL ) {
		::CFRelease( bundle );
	}
	
	filePath = path;
}

/**
 * Returns the file path to the bundle.
 */
::CFStringRef FilePathBundleFactory::getFilePath() {
	return filePath;
}

//-------------------------------------------------------------------------------
//	PlugInFileURLBundleFactory implementation
//-------------------------------------------------------------------------------

/**
 * Constructor for PlugInFileURLBundleFactory. Takes a URL to a plugInFile as input.
 *
 * @param path	URL to Adobe plugin. PlugInFileURLBundleFactory takes control
 *				of the CFURLRef, i.e. you're no longer responsiple for releasing it.
 */
PlugInFileURLBundleFactory::PlugInFileURLBundleFactory( ::CFURLRef url ) : plugInFileURL( url ), bundle( NULL ) {
}

/**
 * Destructor for FilePathBundleFactory. Takes a file path as input.
 *
 * @param path	Path to the bundle.
 */
PlugInFileURLBundleFactory::~PlugInFileURLBundleFactory() {
	if( plugInFileURL != NULL ) {
		::CFRelease( plugInFileURL );	
	}
	
	if( bundle != NULL ) {
		::CFRelease( bundle );	
	}
}

/**
 * Returns a reference to the bundle located pre-specified path.
 */
CFBundleRef PlugInFileURLBundleFactory::getCFBundleRef() {
	if( bundle == NULL ) {
		CFURLRef	bundleURL 	= ::CFURLCreateCopyDeletingLastPathComponent( NULL, plugInFileURL );
   		bundle 		= ::CFBundleCreate( kCFAllocatorDefault, bundleURL );
    
		::CFRelease( bundleURL );
	}
		    
    return bundle;
}

/**
 * Not implemented - you shouldn't call this function. The object uses the desctructor to
 * dipose the bundle reference.
 */
Void PlugInFileURLBundleFactory::release( ::CFBundleRef bundle ) {
	//::CFRelease( bundle );
}

//-------------------------------------------------------------------------------
//	Utility functions
//-------------------------------------------------------------------------------


/**
 * Read the identifier of a bundle.
 */
const char * FitsLiberator::Mac::getBundleIdentifier( ::CFBundleRef bundle ) {
	return ::CFStringGetCStringPtr( CFBundleGetIdentifier( bundle ), kCFStringEncodingMacRoman );
}

/**
 * Read the path of a bundle.
 */
const char * FitsLiberator::Mac::getBundlePath( CFBundleRef bundle ) {
	return ::CFStringGetCStringPtr( ::CFURLGetString( ::CFBundleCopyBundleURL( bundle ) ), kCFStringEncodingMacRoman );	
}