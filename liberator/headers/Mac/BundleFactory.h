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

#ifndef __BUNDLEFACTORY_H__
#define __BUNDLEFACTORY_H__

#include "Types.h"

#include <CFString.h>
#include <CFUrl.h>

namespace FitsLiberator {
	namespace Mac {

// Utility functions
const char * getBundleIdentifier( CFBundleRef bundle );
const char * getBundlePath( CFBundleRef bundle );
		
/**
 * Pure abstract base class for bundle factories. Bundle factories are responsible
 * for getting and releasing bundles.
 *
 * @see MainBundleFactory.
 * @see FilePathBundleFactory
 * @see PlugInFileURLBundleFactory
 */
class BundleFactory {
	public:
		virtual CFBundleRef	getCFBundleRef() = 0;
		virtual Void 		release( CFBundleRef bundle ) = 0;
		
		virtual 			~BundleFactory() = 0;
};

/**
 * MainBundle implementation of a bundle factory.
 *
 * @see BundleFactory.
 */
class MainBundleFactory : public BundleFactory {
	public:
		~MainBundleFactory();
		
		CFBundleRef getCFBundleRef();
		Void 		release( CFBundleRef ) {};	///< Main bundle shouldn't be released, so it does nothing.
};

/**
 * MainBundle implementation of a bundle factory.
 *
 * @see BundleFactory.
 */
class IdentifierBundleFactory : public BundleFactory {
	public:
		IdentifierBundleFactory( CFStringRef identifier );
		~IdentifierBundleFactory();
		
		CFBundleRef getCFBundleRef();
		Void 		release( CFBundleRef );
	
	
	private:
		CFStringRef identifier;	///< Identifier to bundle, eg org.spacetelescope.fits_liberator
		CFBundleRef bundle;		
};
		
/**
 * Implementation of bundle factory for bundles some where on the disk.
 *
 * @see BundleFactory.
 */
class FilePathBundleFactory : public BundleFactory {
	public:
		FilePathBundleFactory( CFStringRef path ); 
		~FilePathBundleFactory();
		
		CFBundleRef getCFBundleRef();
		Void 		release( CFBundleRef bundle );
	
	
		Void 		setFilePath( CFStringRef path );
		CFStringRef	getFilePath();
	
	private:
		CFStringRef	filePath;		///< File path to the bundle
		CFBundleRef	bundle;			
};

/**
 * Implementation of bundle factory for bundles located by URL.
 *
 * @see BundleFactory.
 */
class PlugInFileURLBundleFactory : public BundleFactory {
	public:
		PlugInFileURLBundleFactory( CFURLRef url );
		~PlugInFileURLBundleFactory();

		CFBundleRef getCFBundleRef();
		Void 		release( CFBundleRef bundle );
	
	private:
		CFBundleRef	bundle;				///< Reference to the bundle.
		CFURLRef	plugInFileURL;		///< URL to the plug-in 
};

	} // namespace Mac end
} // namespace FitsLiberator end
#endif //__BUNDLEFACTORY_H__