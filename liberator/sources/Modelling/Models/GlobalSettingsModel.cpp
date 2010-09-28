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
#include "GlobalSettingsModel.h"

using namespace FitsLiberator::Modelling;

/**
 *
 *
 */
GlobalSettingsModel::GlobalSettingsModel( ChangeManager * chman ) 
	: Model( chman ),
	  previewEnabled( kFITSDefaultShowPreview ),
	  freezeSettings( kFITSDefaultFreezeSettings ),
	  sessionLoaded( false ),
	  markUndefined( kFITSDefaultShowUndefined ),
	  markWhiteClipping( kFITSDefaultShowWhiteClipped ),
	  markBlackClipping( kFITSDefaultShowBlackClipped ),
	  imageInformation( kFITSDefaultImageInformation ) {};


/**
 *
 *
 */
Void GlobalSettingsModel::setPreviewEnabled(Bool b)
{
	previewEnabled = b;
	Notify();
}

/**
 *
 *
 */
Void GlobalSettingsModel::setFreezeSettings(Bool b)
{
	freezeSettings = b;
	Notify();
}

Void GlobalSettingsModel::setSessionLoaded( Bool b )
{
	sessionLoaded = b;
	Notify();
}

Bool GlobalSettingsModel::getSessionLoaded()
{
	return sessionLoaded;
}

/**
 *
 *
 */
Void GlobalSettingsModel::setMarkUndefined(Bool b)
{
	markUndefined = b;
	Notify();
}

/**
 *
 *
 */
Void GlobalSettingsModel::setMarkWhiteClipping(Bool b)
{
	markWhiteClipping = b;
	Notify();
}

/**
 *
 *
 */
Void GlobalSettingsModel::setMarkBlackClipping(Bool b)
{
	markBlackClipping = b;
	Notify();
}

/**
 *
 *
 */
Void GlobalSettingsModel::setImageInformation( ImageInformationSettings setting )
{
	imageInformation = setting;
	Notify();
}


/**
 *
 *
 */
ImageInformationSettings GlobalSettingsModel::getImageInformation()
{
	return imageInformation;
}

/**
 *
 *
 */	
Bool GlobalSettingsModel::getPreviewEnabled() const
{
	return previewEnabled;
}

/**
 *
 *
 */
Bool GlobalSettingsModel::getFreezeSettings()
{
	return freezeSettings;
}

/**
 *
 *
 */
Bool GlobalSettingsModel::getMarkUndefined()
{
	return markUndefined;
}

/**
 *
 *
 */
Bool GlobalSettingsModel::getMarkWhiteClipping()
{
	return markWhiteClipping;
}

/**
 *
 *
 */
Bool GlobalSettingsModel::getMarkBlackClipping()
{
	return markBlackClipping;
}