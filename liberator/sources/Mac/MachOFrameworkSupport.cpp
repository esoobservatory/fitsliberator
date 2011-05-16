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

#include "MachOFrameworkSupport.h"
#include "Folders.h"

using namespace FitsLiberator::Mac::MachO;

namespace FitsLiberator{
	namespace Mac {
		namespace MachO {

CFBundleRef carbonFrameworkBundle 		= NULL;
Bool		unloadCarbonFramework		= false;
CFBundleRef appServicesFrameworkBundle 	= NULL;
Bool		unloadAppServicesFramework	= false;
CFBundleRef securityFrameworkBundle 	= NULL;
Bool		unloadSecurityFramework		= false;

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//	CARBON
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
CreateMouseTrackingRegionPtr CreateMouseTrackingRegion = NULL;
SetWindowMouseTrackingRegionsEnabledPtr SetWindowMouseTrackingRegionsEnabled = NULL;
SetMouseTrackingRegionEnabledPtr SetMouseTrackingRegionEnabled = NULL;
HIViewFindByIDPtr HIViewFindByID = NULL;
HIViewGetRootPtr HIViewGetRoot = NULL;
HITextViewGetTXNObjectPtr HITextViewGetTXNObject = NULL; 
GetCurrentEventKeyModifiersPtr GetCurrentEventKeyModifiers = NULL;
HIViewAddSubviewPtr HIViewAddSubview = NULL;
HIViewGetSuperviewPtr HIViewGetSuperview = NULL;
HIViewGetBoundsPtr HIViewGetBounds = NULL;
HIComboBoxCreatePtr HIComboBoxCreate = NULL;
HIComboBoxGetItemCountPtr HIComboBoxGetItemCount = NULL;
HIComboBoxInsertTextItemAtIndexPtr HIComboBoxInsertTextItemAtIndex = NULL;
HIComboBoxAppendTextItemPtr HIComboBoxAppendTextItem = NULL;
HIComboBoxCopyTextItemAtIndexPtr HIComboBoxCopyTextItemAtIndex = NULL;
HIComboBoxRemoveItemAtIndexPtr HIComboBoxRemoveItemAtIndex = NULL;
HIComboBoxChangeAttributesPtr HIComboBoxChangeAttributes = NULL;
HIComboBoxGetAttributesPtr HIComboBoxGetAttributes = NULL;
HIViewSetVisiblePtr HIViewSetVisible = NULL;
HIViewIsVisiblePtr HIViewIsVisible = NULL;
HIViewPlaceInSuperviewAtPtr HIViewPlaceInSuperviewAt = NULL;
HIViewMoveByPtr HIViewMoveBy = NULL;
HIViewRemoveFromSuperviewPtr HIViewRemoveFromSuperview = NULL;
HIThemeDrawTextBoxPtr HIThemeDrawTextBox = NULL;
HIViewSetNeedsDisplayPtr HIViewSetNeedsDisplay = NULL;
HIViewSetNextFocusPtr HIViewSetNextFocus = NULL;
HIViewAdvanceFocusPtr HIViewAdvanceFocus = NULL;
HIViewSetFirstSubViewFocusPtr HIViewSetFirstSubViewFocus = NULL;
HIWindowChangeFeaturesPtr HIWindowChangeFeatures = NULL;
HIObjectRegisterSubclassPtr HIObjectRegisterSubclass = NULL;
HIObjectCreatePtr HIObjectCreate = NULL;

DataBrowserChangeAttributesPtr DataBrowserChangeAttributesNULL = NULL;
DataBrowserGetAttributesPtr DataBrowserGetAttributesNULL = NULL;
			
//-------------------------------------------------------------------------------
//	Initialize global variables.
//-------------------------------------------------------------------------------
/**
 * Initialize all function pointers.
 */
Void InitializeCarbonMachOPointes() {
	CreateMouseTrackingRegion = (CreateMouseTrackingRegionPtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("CreateMouseTrackingRegion") );
	SetWindowMouseTrackingRegionsEnabled = (SetWindowMouseTrackingRegionsEnabledPtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("SetWindowMouseTrackingRegionsEnabled") );
	SetMouseTrackingRegionEnabled = (SetMouseTrackingRegionEnabledPtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("SetMouseTrackingRegionEnabled") );
	HIViewFindByID = (HIViewFindByIDPtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("HIViewFindByID") );
	HIViewGetRoot = (HIViewGetRootPtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("HIViewGetRoot") );
	HITextViewGetTXNObject = (HITextViewGetTXNObjectPtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("HITextViewGetTXNObject") );
	GetCurrentEventKeyModifiers = (GetCurrentEventKeyModifiersPtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("GetCurrentEventKeyModifiers") );
	HIViewAddSubview = (HIViewAddSubviewPtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("HIViewAddSubview") );
	HIViewGetSuperview = (HIViewGetSuperviewPtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("HIViewGetSuperview") );
	HIViewGetBounds = (HIViewGetBoundsPtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("HIViewGetBounds") );
	HIComboBoxCreate = (HIComboBoxCreatePtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("HIComboBoxCreate") );
	HIComboBoxGetItemCount = (HIComboBoxGetItemCountPtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("HIComboBoxGetItemCount") );
	HIComboBoxInsertTextItemAtIndex = (HIComboBoxInsertTextItemAtIndexPtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("HIComboBoxInsertTextItemAtIndex") );
	HIComboBoxAppendTextItem = (HIComboBoxAppendTextItemPtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("HIComboBoxAppendTextItem") );
	HIComboBoxCopyTextItemAtIndex = (HIComboBoxCopyTextItemAtIndexPtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("HIComboBoxCopyTextItemAtIndex") );
	HIComboBoxRemoveItemAtIndex = (HIComboBoxRemoveItemAtIndexPtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("HIComboBoxRemoveItemAtIndex") );
	HIComboBoxChangeAttributes = (HIComboBoxChangeAttributesPtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("HIComboBoxChangeAttributes") );
	HIComboBoxGetAttributes = (HIComboBoxGetAttributesPtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("HIComboBoxGetAttributes") );
	HIViewSetVisible = (HIViewSetVisiblePtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("HIViewSetVisible") );
	HIViewIsVisible = (HIViewIsVisiblePtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("HIViewIsVisible") );
	HIViewPlaceInSuperviewAt = (HIViewPlaceInSuperviewAtPtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("HIViewPlaceInSuperviewAt") );
	HIViewMoveBy = (HIViewMoveByPtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("HIViewMoveBy") );
	HIViewRemoveFromSuperview = (HIViewRemoveFromSuperviewPtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("HIViewRemoveFromSuperview") );
	HIThemeDrawTextBox = (HIThemeDrawTextBoxPtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("HIThemeDrawTextBox") );
	HIViewSetNeedsDisplay = (HIViewSetNeedsDisplayPtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("HIViewSetNeedsDisplay") );
	HIViewSetNextFocus = (HIViewSetNextFocusPtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("HIViewSetNextFocus") );
	HIViewAdvanceFocus = (HIViewAdvanceFocusPtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("HIViewAdvanceFocus") );
	HIViewSetFirstSubViewFocus = (HIViewSetFirstSubViewFocusPtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("HIViewSetFirstSubViewFocus") );
	HIWindowChangeFeatures = (HIWindowChangeFeaturesPtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("HIWindowChangeFeatures") );
	HIObjectRegisterSubclass = (HIObjectRegisterSubclassPtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("HIObjectRegisterSubclass") );
	HIObjectCreate = (HIObjectCreatePtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("HIObjectCreate") );
	
	// Warn: Exception is not thrown if function pointer is null. You must check 
	// the pointer for null before use.
	DataBrowserChangeAttributesNULL = (DataBrowserChangeAttributesPtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("DataBrowserChangeAttributes"), false );
	DataBrowserGetAttributesNULL = (DataBrowserGetAttributesPtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("DataBrowserGetAttributes"), false );
	
	// = (Ptr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("") );
}

//-------------------------------------------------------------------------------
//	Terminate global variables.
//-------------------------------------------------------------------------------
/**
 * Make sure all MachO function pointers can't be used any more.
 */
Void TerminateCarbonMachOPointes() {
	CreateMouseTrackingRegion = NULL;
	SetWindowMouseTrackingRegionsEnabled = NULL;
	SetMouseTrackingRegionEnabled = NULL;
	HIViewFindByID = NULL;
	HIViewGetRoot = NULL;
	HITextViewGetTXNObject = NULL;
	GetCurrentEventKeyModifiers = NULL;
	HIViewAddSubview = NULL;
	HIViewGetSuperview = NULL;
	HIViewGetBounds = NULL;
	HIComboBoxCreate = NULL;
	HIComboBoxGetItemCount = NULL;
	HIComboBoxInsertTextItemAtIndex = NULL;
	HIComboBoxAppendTextItem = NULL;
	HIComboBoxCopyTextItemAtIndex = NULL;
	HIComboBoxRemoveItemAtIndex = NULL;
	HIComboBoxChangeAttributes = NULL;
	HIComboBoxGetAttributes = NULL;
	HIViewSetVisible = NULL;
	HIViewIsVisible = NULL;
	HIViewPlaceInSuperviewAt = NULL;
	HIViewMoveBy = NULL;
	HIViewRemoveFromSuperview = NULL;
	DataBrowserChangeAttributesNULL = NULL;
	DataBrowserGetAttributesNULL = NULL;
	HIThemeDrawTextBox = NULL;
	HIViewSetNeedsDisplay = NULL;
	HIViewSetNextFocus = NULL;
	HIViewAdvanceFocus = NULL;
	HIViewSetFirstSubViewFocus = NULL;
	HIWindowChangeFeatures = NULL;
	HIObjectRegisterSubclass = NULL;
	HIObjectCreate = NULL;
}


//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//	APPLICATION SERVICES
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
QDBeginCGContextPtr QDBeginCGContext = NULL;
QDEndCGContextPtr QDEndCGContext = NULL;
CGContextStrokeRectWithWidthPtr CGContextStrokeRectWithWidth = NULL;
CGContextTranslateCTMPtr CGContextTranslateCTM = NULL;
CGContextScaleCTMPtr CGContextScaleCTM = NULL;
CGContextSetRGBStrokeColorPtr CGContextSetRGBStrokeColor = NULL;
CGContextSetRGBFillColorPtr CGContextSetRGBFillColor = NULL;
CGContextFillRectPtr CGContextFillRect = NULL;
CGContextAddLineToPointPtr CGContextAddLineToPoint = NULL;
CGContextMoveToPointPtr CGContextMoveToPoint = NULL;
CGContextFillPathPtr CGContextFillPath = NULL;
CGContextStrokePathPtr CGContextStrokePath = NULL;
CGContextSetShouldAntialiasPtr CGContextSetShouldAntialias = NULL;
CGContextSetLineDashPtr CGContextSetLineDash = NULL;
CGContextSetLineJoinPtr CGContextSetLineJoin = NULL;
CGContextSetLineWidthPtr CGContextSetLineWidth = NULL;
CGContextSetLineCapPtr CGContextSetLineCap = NULL;
CGContextSetInterpolationQualityPtr CGContextSetInterpolationQuality = NULL;
CGContextFlushPtr CGContextFlush = NULL;
CGContextSynchronizePtr CGContextSynchronize = NULL;
CGContextBeginPathPtr CGContextBeginPath = NULL;
CGContextClosePathPtr CGContextClosePath = NULL;
CGContextDrawImagePtr CGContextDrawImage = NULL;
CGContextClearRectPtr CGContextClearRect = NULL;
CGImageReleasePtr CGImageRelease = NULL;
CGImageCreateWithPNGDataProviderPtr CGImageCreateWithPNGDataProvider = NULL;
CGImageMaskCreatePtr CGImageMaskCreate = NULL;
CGImageCreatePtr CGImageCreate = NULL;
CGDataProviderCreateWithURLPtr CGDataProviderCreateWithURL = NULL;
CGDataProviderReleasePtr CGDataProviderRelease = NULL;
CGDataProviderCreateWithDataPtr CGDataProviderCreateWithData = NULL;
PlotIconRefInContextPtr PlotIconRefInContext = NULL;
CGColorSpaceCreateDeviceGrayPtr CGColorSpaceCreateDeviceGray = NULL;
CGColorSpaceCreateDeviceRGBPtr CGColorSpaceCreateDeviceRGB = NULL;
CGColorSpaceReleasePtr CGColorSpaceRelease = NULL;
CGRectContainsPointPtr CGRectContainsPoint = NULL;

			
//-------------------------------------------------------------------------------
//	Initialize global variables.
//-------------------------------------------------------------------------------
/**
 * Initialize all function pointers.
 */
Void InitializeApplicationServicesMachOPointes() {
	QDBeginCGContext = (QDBeginCGContextPtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("QDBeginCGContext") );
	QDEndCGContext = (QDEndCGContextPtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("QDEndCGContext") );
	CGContextStrokeRectWithWidth = (CGContextStrokeRectWithWidthPtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("CGContextStrokeRectWithWidth") );
	CGContextTranslateCTM = (CGContextTranslateCTMPtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("CGContextTranslateCTM") );
	CGContextScaleCTM = (CGContextScaleCTMPtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("CGContextScaleCTM") );
	CGContextSetRGBStrokeColor = (CGContextSetRGBStrokeColorPtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("CGContextSetRGBStrokeColor") );
	CGContextSetRGBFillColor = (CGContextSetRGBFillColorPtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("CGContextSetRGBFillColor") );
	CGContextFillRect = (CGContextFillRectPtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("CGContextFillRect") );
	CGContextAddLineToPoint = (CGContextAddLineToPointPtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("CGContextAddLineToPoint") );
	CGContextMoveToPoint = (CGContextMoveToPointPtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("CGContextMoveToPoint") );
	CGContextFillPath = (CGContextFillPathPtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("CGContextFillPath") );
	CGContextStrokePath = (CGContextStrokePathPtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("CGContextStrokePath") );
	CGContextSetShouldAntialias  = (CGContextSetShouldAntialiasPtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("CGContextSetShouldAntialias") );
	CGContextSetLineDash = (CGContextSetLineDashPtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("CGContextSetLineDash") );
	CGContextSetLineJoin = (CGContextSetLineJoinPtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("CGContextSetLineJoin") );
	CGContextSetLineWidth = (CGContextSetLineWidthPtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("CGContextSetLineWidth") );
	CGContextSetLineCap = (CGContextSetLineCapPtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("CGContextSetLineCap") );
	CGContextSetInterpolationQuality = (CGContextSetInterpolationQualityPtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("CGContextSetInterpolationQuality") );
	CGContextFlush = (CGContextFlushPtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("CGContextFlush") );
	CGContextSynchronize = (CGContextSynchronizePtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("CGContextSynchronize") );
	CGContextBeginPath = (CGContextBeginPathPtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("CGContextBeginPath") );
	CGContextClosePath = (CGContextClosePathPtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("CGContextClosePath") );
	CGContextDrawImage = (CGContextDrawImagePtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("CGContextDrawImage") );
	CGImageRelease = (CGImageReleasePtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("CGImageRelease") );
	CGImageCreateWithPNGDataProvider = (CGImageCreateWithPNGDataProviderPtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("CGImageCreateWithPNGDataProvider") );
	CGImageMaskCreate = (CGImageMaskCreatePtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("CGImageMaskCreate") );
	CGImageCreate = (CGImageCreatePtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("CGImageCreate") );	
	CGDataProviderCreateWithURL = (CGDataProviderCreateWithURLPtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("CGDataProviderCreateWithURL") );
	CGDataProviderRelease = (CGDataProviderReleasePtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("CGDataProviderRelease") );
	CGDataProviderCreateWithData = (CGDataProviderCreateWithDataPtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("CGDataProviderCreateWithData") );
	PlotIconRefInContext  = (PlotIconRefInContextPtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("PlotIconRefInContext") );
	CGColorSpaceCreateDeviceGray = (CGColorSpaceCreateDeviceGrayPtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("CGColorSpaceCreateDeviceGray") );
	CGColorSpaceCreateDeviceRGB = (CGColorSpaceCreateDeviceRGBPtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("CGColorSpaceCreateDeviceRGB") );
	CGColorSpaceRelease = (CGColorSpaceReleasePtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("CGColorSpaceRelease") );
	CGContextClearRect = (CGContextClearRectPtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("CGContextClearRect") );
	CGRectContainsPoint = (CGRectContainsPointPtr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("CGRectContainsPoint") );
	// = (Ptr) GetFunctionPointerForName( appServicesFrameworkBundle, CFSTR("") );
}

//-------------------------------------------------------------------------------
//	Terminate global variables.
//-------------------------------------------------------------------------------
/**
 * Make sure all MachO function pointers can't be used any more.
 */
Void TerminateApplicationServicesMachOPointes() {
	QDBeginCGContext = NULL;
	QDEndCGContext = NULL;
	CGContextStrokeRectWithWidth = NULL;
	CGContextTranslateCTM = NULL;
	CGContextScaleCTM = NULL;
	CGContextSetRGBStrokeColor = NULL;
	CGContextSetRGBFillColor = NULL;
	CGContextFillRect = NULL;
	CGContextAddLineToPoint = NULL;
	CGContextMoveToPoint = NULL;
	CGContextFillPath = NULL;
	CGContextStrokePath = NULL;
	CGContextSetShouldAntialias = NULL;
	CGContextSetLineDash = NULL;
	CGContextSetLineJoin = NULL;
	CGContextSetLineWidth = NULL;
	CGContextSetLineCap = NULL;
	CGContextSetInterpolationQuality = NULL;
	CGContextFlush = NULL;
	CGContextSynchronize = NULL;
	CGContextBeginPath = NULL;
	CGContextClosePath = NULL;
	CGContextDrawImage = NULL;
	CGContextClearRect = NULL;
	CGImageRelease = NULL;
	CGImageCreateWithPNGDataProvider = NULL;
	CGImageMaskCreate = NULL;
	CGImageCreate = NULL;
	CGDataProviderCreateWithURL = NULL;
	CGDataProviderRelease = NULL;
	CGDataProviderCreateWithData = NULL;
	PlotIconRefInContext = NULL;
	CGColorSpaceCreateDeviceGray = NULL;
	CGColorSpaceCreateDeviceRGB = NULL;
	CGColorSpaceRelease = NULL;
	CGRectContainsPoint = NULL;
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//	SECURITY
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
AuthorizationCreatePtr AuthorizationCreate = NULL;
AuthorizationFreePtr AuthorizationFree = NULL;
AuthorizationCopyRightsPtr AuthorizationCopyRights = NULL;
AuthorizationCopyInfoPtr AuthorizationCopyInfo = NULL;
AuthorizationMakeExternalFormPtr AuthorizationMakeExternalForm = NULL;
AuthorizationCreateFromExternalFormPtr AuthorizationCreateFromExternalForm = NULL;
AuthorizationFreeItemSetPtr AuthorizationFreeItemSet = NULL;
AuthorizationExecuteWithPrivilegesPtr AuthorizationExecuteWithPrivileges = NULL;
AuthorizationCopyPrivilegedReferencePtr AuthorizationCopyPrivilegedReference = NULL;
			
//-------------------------------------------------------------------------------
//	Initialize global variables.
//-------------------------------------------------------------------------------
/**
 * Initialize all function pointers.
 */
Void InitializeSecurityMachOPointes() {
	AuthorizationCreate = (AuthorizationCreatePtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("AuthorizationCreate") );
	AuthorizationFree = (AuthorizationFreePtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("AuthorizationFree") );
	AuthorizationCopyRights = (AuthorizationCopyRightsPtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("AuthorizationCopyRights") );
	AuthorizationCopyInfo = (AuthorizationCopyInfoPtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("AuthorizationCopyInfo") );
	AuthorizationMakeExternalForm = (AuthorizationMakeExternalFormPtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("AuthorizationMakeExternalForm") );
	AuthorizationCreateFromExternalForm = (AuthorizationCreateFromExternalFormPtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("AuthorizationCreateFromExternalForm") );
	AuthorizationFreeItemSet = (AuthorizationFreeItemSetPtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("AuthorizationFreeItemSet") );
	AuthorizationExecuteWithPrivileges = (AuthorizationExecuteWithPrivilegesPtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("AuthorizationExecuteWithPrivileges") );
	AuthorizationCopyPrivilegedReference = (AuthorizationCopyPrivilegedReferencePtr) GetFunctionPointerForName( carbonFrameworkBundle, CFSTR("AuthorizationCopyPrivilegedReference") );
}

//-------------------------------------------------------------------------------
//	Terminate global variables.
//-------------------------------------------------------------------------------
/**
 * Make sure all MachO function pointers can't be used any more.
 */
Void TerminateSecurityMachOPointes() {
	AuthorizationCreate = NULL;
	AuthorizationFree = NULL;
	AuthorizationCopyRights = NULL;
	AuthorizationCopyInfo = NULL;
	AuthorizationMakeExternalForm = NULL;
	AuthorizationCreateFromExternalForm = NULL;
	AuthorizationFreeItemSet = NULL;
	AuthorizationExecuteWithPrivileges = NULL;
	AuthorizationCopyPrivilegedReference = NULL;
}

//-------------------------------------------------------------------------------
//	Mach-O from CFM support functions.
//-------------------------------------------------------------------------------

/**
 * Load Carbon.framework. First try if framework is already loaded, else load it.
 *
 * May be com.apple.Carbon need to be used instead of com.apple.HIToolbox
 */
Void LoadCarbonFrameworkBundle() {
	if( carbonFrameworkBundle == NULL ) {
		carbonFrameworkBundle = CFBundleGetBundleWithIdentifier( CFSTR("com.apple.HIToolbox") );
		
		if( carbonFrameworkBundle == NULL ){
			if( noErr != LoadFrameworkBundle( CFSTR("Carbon.framework"), &carbonFrameworkBundle ) ) {
				carbonFrameworkBundle = NULL;
				throw FitsLiberator::Exception("Coulnd't load Carbon framework.");
			}
			
			#ifdef _DEBUG
				DebugStr("\pLoading Carbon.framework...");
			#endif
		}
		
		unloadCarbonFramework = true;
		InitializeCarbonMachOPointes();
	}
}

/**
 * Unload Carbon.framework.
 */
Void UnloadCarbonFrameworkBundle() {
	if( carbonFrameworkBundle != NULL ) {
		TerminateCarbonMachOPointes();
	
		if( unloadCarbonFramework ) {
			unloadCarbonFramework = false;
			UnloadFrameworkBundle( carbonFrameworkBundle );
			carbonFrameworkBundle = NULL;
		}
	}
}

/**
 * Load ApplicationServices.framework.
 */
Void LoadApplicationServicesFrameworkBundle() {
	if( appServicesFrameworkBundle == NULL ) {
		appServicesFrameworkBundle = CFBundleGetBundleWithIdentifier( CFSTR("com.apple.ApplicationServices") );
	
		if( appServicesFrameworkBundle == NULL ) {
			if( noErr != LoadFrameworkBundle( CFSTR("ApplicationServices.framework/"), &appServicesFrameworkBundle ) ) {
				appServicesFrameworkBundle = NULL;
				throw FitsLiberator::Exception("Coulnd't load ApplicationServices framework.");
			}
			
			#ifdef _DEBUG
				DebugStr("\pLoading ApplicationServices.framework...");
			#endif
		}
		
		unloadAppServicesFramework = true;	
		InitializeApplicationServicesMachOPointes();
	}
}

/**
 * Unload ApplicationServices.framework.
 */
Void UnloadApplicationServicesFrameworkBundle() {
	if( appServicesFrameworkBundle != NULL ) {
		TerminateApplicationServicesMachOPointes();
		
		if( unloadAppServicesFramework ) {
			unloadAppServicesFramework = false;
			UnloadFrameworkBundle( appServicesFrameworkBundle );	
			appServicesFrameworkBundle = NULL;
		}
	}
}

/**
 * Load Security.framework.
 */
Void LoadSecurityFrameworkBundle() {
	if( securityFrameworkBundle == NULL ) {
		securityFrameworkBundle = CFBundleGetBundleWithIdentifier( CFSTR("com.apple.Security") );
	
		if( securityFrameworkBundle == NULL ) {
			if( noErr != LoadFrameworkBundle( CFSTR("Security.framework/"), &securityFrameworkBundle ) ) {
				securityFrameworkBundle = NULL;
				throw FitsLiberator::Exception("Couldn't load Security framework.");
			}
			
			#ifdef _DEBUG
				DebugStr("\pLoading Security.framework...");
			#endif
		}
			
		unloadSecurityFramework = true;
		InitializeSecurityMachOPointes();
	}
}

/**
 * Unload Security.framework.
 */
Void UnloadSecurityFrameworkBundle() {
	if( securityFrameworkBundle != NULL ) {
		TerminateSecurityMachOPointes();
		
		if( unloadSecurityFramework ) {
			unloadSecurityFramework = false;
			UnloadFrameworkBundle( securityFrameworkBundle );	
			securityFrameworkBundle = NULL;
		}
	}
}


/**
 * Loads a function pointer from a framework and ensure that's not NULL.
 * 
 * @param	bundle					Bundle reference.
 * @param	functionName			Name of function in bundle to get.
 * @param 	throwExceptionOnNull	Pass false if you don't want to be thrown an exception 
 *									if the functions isn't available. Usefull for e.g.
 *									10.4 only functionality.  
 *
 * @return	A function pointer on succes, NULL if bundle or functioName is NULL.
 * @throws	Exception if there was a problem getting the function pointer.
 */
static Void* GetFunctionPointerForName( CFBundleRef bundle, CFStringRef functionName, Bool throwExceptionOnNull ) {
	Void* functionPointer = NULL;
	
	if( bundle != NULL && functionName != NULL ) {
		functionPointer = ::CFBundleGetFunctionPointerForName( bundle, functionName );
	
		if( functionPointer == NULL ) {
			#ifdef _DEBUG
				DebugStr("\pCould not load function pointer for: ");
				::CFShow( bundle );
				::CFShow( functionName );
			#endif
			
			if( throwExceptionOnNull ) {
				throw FitsLiberator::Exception("Couldn't get function pointer from framework.");	
			}
		} 
	}
	
	return functionPointer;
}


/**
 * Loads a data pointer from a framework and ensure that's not NULL.
 * 
 * @param	bundle			Bundle reference.
 * @param	symbolName		Name of symbol in bundle to get.
 *
 * @return	A symbol pointer on succes, NULL if bundle or symbolName is NULL.
 * @throws	Exception if there was a problem getting the data pointer.
 */
static Void* GetDataPointerForName( CFBundleRef bundle, CFStringRef symbolName ) {
	Void* symbolPointer = NULL;
	
	if( bundle != NULL && symbolName != NULL ) {
		symbolPointer = ::CFBundleGetDataPointerForName( bundle, symbolName );
	
		if( symbolPointer != NULL ) {
			throw FitsLiberator::Exception("Couldn't get data pointer from framework.");
		} 
	}
	
	return symbolPointer;
}

/**
 * Unloads a framework bundle loaded with LoadFrameworkBundle(). The bundle
 * will get unloaded and released.
 *
 * @param bundleRef Reference to bundle previously loaded with LoadFrameworkBundle().
 * @return 			\c noErr on success, and \c coreFoundationUnknownErr on failure.
 */
static OSStatus UnloadFrameworkBundle( CFBundleRef bundleRef ) {
	if( bundleRef != NULL ) {
		::CFBundleUnloadExecutable( bundleRef );
		::CFRelease( bundleRef );
		
		return noErr;
	} else {
		return 	coreFoundationUnknownErr;
	}
}

/**
 * This routine finds a the named framework and creates a CFBundle 
 * object for it.  It looks for the framework in the frameworks folder, 
 * as defined by the Folder Manager.  Currently this is 
 * "/System/Library/Frameworks", but we recommend that you avoid hard coded 
 * paths to ensure future compatibility.
 * 
 * You might think that you could use CFBundleGetBundleWithIdentifier but 
 * that only finds bundles that are already loaded into your context. 
 * That would work in the case of the System framework but it wouldn't 
 * work if you're using some other, less-obvious, framework.
 *
 * @param 	framework	Framework name, e.g. <tt>CFSTR("Carbon.framework")</tt>
 * @param 	bundlePtr	A bundle reference to the framework, if success.
 * @return				\c noErr on success, \c coreFoundationUnknownErr on error.
 *
 * @see MoreIsBetter sample code from ADC Reference Library.
 * @see UnloadFrameworkBundle().
 */
static OSStatus LoadFrameworkBundle( CFStringRef framework, CFBundleRef *bundlePtr ) {
	OSStatus 	err;
	FSRef 		frameworksFolderRef;
	CFURLRef	baseURL;
	CFURLRef	bundleURL;
	
	if( bundlePtr == NULL ) {
		return coreFoundationUnknownErr;
	}
	
	*bundlePtr 	= NULL;
	baseURL 	= NULL;
	bundleURL 	= NULL;
	
	// Find the frameworks folder and create a URL for it.
	err = ::FSFindFolder( kOnAppropriateDisk, kFrameworksFolderType, true, &frameworksFolderRef );
	
	if( err == noErr ) {
		
		baseURL = ::CFURLCreateFromFSRef( kCFAllocatorSystemDefault, &frameworksFolderRef );
		
		if( baseURL == NULL ) {
			err = coreFoundationUnknownErr;
		}
	}
	
	// Append the name of the framework to the URL.
	if( err == noErr ) {
		bundleURL = ::CFURLCreateCopyAppendingPathComponent( kCFAllocatorSystemDefault, baseURL, framework, false );
		
		if( bundleURL == NULL ) {
			err = coreFoundationUnknownErr;
		}
	}
	
	// Create a bundle based on that URL and load the bundle into memory.
	// We never unload the bundle, which is reasonable in this case because 
	// the sample assumes that you'll be calling functions from this 
	// framework throughout the life of your application.
	if( err == noErr ) {
		*bundlePtr = ::CFBundleCreate(kCFAllocatorSystemDefault, bundleURL);
		
		if( *bundlePtr == NULL ) {
			err = coreFoundationUnknownErr;
		}
	}
	
	if( err == noErr ) {
	    if( ! ::CFBundleLoadExecutable( *bundlePtr ) ) {
			err = coreFoundationUnknownErr;
	    }
	}

	// Clean up.
	if( err != noErr && *bundlePtr != NULL ) {
		::CFRelease( *bundlePtr );
		*bundlePtr = NULL;
	}
	if( bundleURL != NULL ) {
		::CFRelease( bundleURL );
	}	
	if( baseURL != NULL ) {
		::CFRelease( baseURL );
	}	
	
	return err;
}

		} // end namespace MachO
	} // end namespace Mac
} // end namespace FitsLiberator
