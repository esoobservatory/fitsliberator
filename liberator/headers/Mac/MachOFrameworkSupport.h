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
 * Include this file, if you want to call a function only availble in 
 * Carbon.framework and not in CarbonLib.
 *
 * If the function you want to call doesn't exists here, you may add by
 * following this small guide:
 *
 * 1. Copy all needed information from the frameworks header file. E.g.
 * from CarbonEvents.h from Carbon.framework (prefer framework headers 
 * located in /Developer/SDKs/10.2.8). Remember all typedef's, enums etc.
 * that the function requires.
 *
 * 2. Instead of the function prototype - something like this:
 *
 * \code
 * extern OSStatus CreateMouseTrackingRegion(...)
 * \endcode
 *
 * you change it to something like this (note the Ptr on the function name):
 *
 * \code 
 * typedef OSStatus (*CreateMouseTrackingRegionPtr)(..)
 * \endcode
 *
 * 3. Add global variables in MachO namespace declartion - something like this:
 * \code
 * extern CreateMouseTrackingRegionPtr CreateMouseTrackingRegion;
 * \endcode
 *
 * 4. Add/Initialize/Terminate global variable in MachOFrameworkSupport.cpp. 
 *
 * \code 
 * CreateMouseTrackingRegionPtr CreateMouseTrackingRegion = NULL;  
 * \endcode
 *
 * Initialize it in InitializeMachOPointes():
 * \code 
 * CreateMouseTrackingRegion = (CreateMouseTrackingRegionPtr) GetFunctionPointerForName( &carbonFrameworkBundle, CFSTR("CreateMouseTrackingRegion") );
 * \endcode
 * 
 * Terminate it in TerminateMachOPointes():
 * \code 
 * CreateMouseTrackingRegion = NULL;
 * \endcode
 *
 * 5. You may now call the function by including this header file, and
 * write something like this:
 *
 * \code
 * using namespace FitsLiberator::Mac;
 *
 * MachO::CreateMouseTrackingRegion(...);
 * \endcode 
 */

#ifndef __MACHOFRAMEWORKSUPPORT_H__
#define __MACHOFRAMEWORKSUPPORT_H__

#include "Types.h"
#include "Exception.h"
#include "MacTextEditor.h"
#include "Appearance.h"
//
////-------------------------------------------------------------------------------
//// Copied from Appearance.h in SDK 10.3.9
////-------------------------------------------------------------------------------
//enum {
//  kThemeBrushAlternatePrimaryHighlightColor = -5
//};
//
////-------------------------------------------------------------------------------
//// Copied from CarbonEventsCore.h in SDK 10.2.8
////-------------------------------------------------------------------------------
typedef UInt32 (*GetCurrentEventKeyModifiersPtr)(void);
//
////-------------------------------------------------------------------------------
//// Copied from CarbonEvents.h in SDK 10.4.0
////-------------------------------------------------------------------------------
//enum {
//  kEventTextInputFilterText     = 14
//};
//
////-------------------------------------------------------------------------------
//// Copied from CarbonEvents.h in SDK 10.2.8
////-------------------------------------------------------------------------------
//
//enum {
//	typeVoidPtr                   = 'void' /* void * (used for hiobject fun)*/	
//};
//
//enum {
//	kEventMouseEntered            = 8,
//	kEventMouseExited             = 9
//};
//
//enum {
//  kMouseTrackingOptionsLocalClip = 0,
//  kMouseTrackingOptionsGlobalClip = 1,
//  kMouseTrackingOptionsStandard = kMouseTrackingOptionsLocalClip
//};
//
//enum {
//  kEventParamWindowMouseLocation = 'wmou', /* typeHIPoint (Mac OS X 10.1 or later)*/
//  kEventParamMouseTrackingRef   = 'mtrf', /* typeMouseTrackingRef*/
//  typeMouseTrackingRef          = 'mtrf' /* MouseTrackingRef*/
//};
//
//enum {
//  kEventTextInputUnicodeText    = 7
//};
//
//
//struct MouseTrackingRegionID {
//  OSType              signature;
//  SInt32              id;
//};
//
typedef struct OpaqueMouseTrackingRef*  MouseTrackingRef;
typedef UInt32 MouseTrackingOptions;
typedef struct MouseTrackingRegionID    MouseTrackingRegionID;
typedef OSStatus (*CreateMouseTrackingRegionPtr)(WindowRef inWindow, RgnHandle inRegion, RgnHandle inClip, MouseTrackingOptions inOptions, MouseTrackingRegionID inID, void * inRefCon, EventTargetRef inTargetToNotify,  MouseTrackingRef* outTrackingRef );
typedef OSStatus (*SetWindowMouseTrackingRegionsEnabledPtr)( WindowRef inWindow, OSType inSignature, Boolean inEnabled );
typedef OSStatus (*SetMouseTrackingRegionEnabledPtr)( MouseTrackingRef inMouseRef, Boolean inEnabled );
//
////-------------------------------------------------------------------------------
//// Copied from Drag.h in SDK 10.2.8
////-------------------------------------------------------------------------------
typedef CGRect			HIRect;
//
////-------------------------------------------------------------------------------
//// Copied from MacWindows.h in SDK 10.3.9
////-------------------------------------------------------------------------------
//enum {
//  kWindowDefHIView = 3
//};
//
//enum {
//	kWindowCompositingAttribute   = (1L << 19)
//};
//
typedef OSStatus (*HIWindowChangeFeaturesPtr)( WindowRef inWindow, UInt64 inSetThese, UInt64 inClearThese );
//
////-------------------------------------------------------------------------------
//// Copied from HIObject.h in SDK 10.2.8
////-------------------------------------------------------------------------------
typedef struct OpaqueHIObjectClassRef*  HIObjectClassRef;
typedef struct OpaqueHIObjectRef*       HIObjectRef;
//
//enum {
//  kEventClassHIObject           = 'hiob',
//  kEventHIObjectConstruct       = 1,
//  kEventHIObjectInitialize      = 2,
//  kEventHIObjectDestruct        = 3,
//  kEventHIObjectIsEqual         = 4,
//  kEventHIObjectPrintDebugInfo  = 5
//};
//
//enum {
//  kEventParamHIObjectInstance   = 'hioi',
//  typeHIObjectRef               = 'hiob'
//};
//
typedef OSStatus (*HIObjectRegisterSubclassPtr)( CFStringRef inClassID, CFStringRef inBaseClassID, OptionBits inOptions, EventHandlerUPP inConstructProc, UInt32 inNumEvents, const EventTypeSpec * inEventList, void * inConstructData, HIObjectClassRef * outClassRef);
typedef OSStatus (*HIObjectCreatePtr)( CFStringRef inClassID, EventRef inConstructData, HIObjectRef * outObject );
//
////-------------------------------------------------------------------------------
//// Copied from HIView.h in SDK 10.2.8
////-------------------------------------------------------------------------------
//#define kHIViewClassID                  CFSTR("com.apple.hiview")
typedef ControlRef		HIViewRef;
typedef ControlID		HIViewID;
typedef ControlPartCode	HIViewPartCode;
//extern const HIViewID kHIViewWindowContentID;
typedef OSStatus	(*HIViewFindByIDPtr)( HIViewRef inStartView, HIViewID inID, HIViewRef * outControl );
typedef HIViewRef	(*HIViewGetRootPtr)(WindowRef inWindow);
typedef OSStatus	(*HIViewAddSubviewPtr)( HIViewRef inParent, HIViewRef inNewChild );
typedef HIViewRef	(*HIViewGetSuperviewPtr)(HIViewRef inView);
typedef OSStatus	(*HIViewGetBoundsPtr)( HIViewRef inView, HIRect * outRect);
//
//enum {
//  kHIComboBoxNoAttributes       = 0L,
//  kHIComboBoxAutoCompletionAttribute = (1L << 0),
//  kHIComboBoxAutoDisclosureAttribute = (1L << 1),
//  kHIComboBoxAutoSortAttribute  = (1L << 2),
//  kHIComboBoxAutoSizeListAttribute = (1L << 3),
//  kHIComboBoxStandardAttributes = (kHIComboBoxAutoCompletionAttribute | kHIComboBoxAutoDisclosureAttribute | kHIComboBoxAutoSizeListAttribute)
//};
//
//enum {
//  kControlKindHIComboBox        = 'cbbx'
//};
//enum {
//  kHIComboBoxEditTextPart       = 5,
//  kHIComboBoxDisclosurePart     = 28
//};
//
//enum {
//  kHIComboBoxListTag            = 'cbls',
//  kHIComboBoxListPixelWidthTag  = 'cblw',
//  kHIComboBoxListPixelHeightTag = 'cblh',
//  kHIComboBoxNumVisibleItemsTag = 'cbni' 
//};
//
typedef OSStatus (*HIComboBoxCreatePtr)( const HIRect * boundsRect, CFStringRef text, const ControlFontStyleRec *  style, CFArrayRef list, OptionBits inAttributes, HIViewRef * outComboBox);
typedef ItemCount (*HIComboBoxGetItemCountPtr)(HIViewRef inComboBox);
typedef OSStatus (*HIComboBoxInsertTextItemAtIndexPtr)( HIViewRef inComboBox, CFIndex inIndex, CFStringRef inText);
typedef OSStatus (*HIComboBoxAppendTextItemPtr)(HIViewRef inComboBox, CFStringRef inText, CFIndex * outIndex);
typedef OSStatus (*HIComboBoxCopyTextItemAtIndexPtr)( HIViewRef inComboBox, CFIndex inIndex, CFStringRef * outString);
typedef OSStatus (*HIComboBoxRemoveItemAtIndexPtr)(HIViewRef inComboBox, CFIndex inIndex);
typedef OSStatus (*HIComboBoxChangeAttributesPtr)( HIViewRef inComboBox, OptionBits inAttributesToSet, OptionBits inAttributesToClear);
typedef OSStatus (*HIComboBoxGetAttributesPtr)(HIViewRef inComboBox, OptionBits * outAttributes);      
typedef OSStatus (*HIViewSetVisiblePtr)(HIViewRef inView, Boolean inVisible);
typedef Boolean (*HIViewIsVisiblePtr)(HIViewRef inView);
typedef OSStatus (*HIViewPlaceInSuperviewAtPtr)(HIViewRef inView, float inX, float inY);
typedef OSStatus (*HIViewMoveByPtr)( HIViewRef inView, float inDX, float inDY );
typedef OSStatus (*HIViewRemoveFromSuperviewPtr)( HIViewRef inView );
typedef OSStatus (*HIViewSetNeedsDisplayPtr)( HIViewRef inView, Boolean inNeedsDisplay );
typedef OSStatus (*HIViewSetNextFocusPtr)( HIViewRef inView, HIViewRef inNextFocus);
typedef OSStatus (*HIViewAdvanceFocusPtr)( HIViewRef inRootForFocus, EventModifiers inModifiers );
typedef OSStatus (*HIViewSetFirstSubViewFocusPtr)( HIViewRef inParent, HIViewRef inSubView );
//
//
////-------------------------------------------------------------------------------
//// Copied from MacTextEditor.h in SDK 10.3.0
////-------------------------------------------------------------------------------
typedef TXNObject (*HITextViewGetTXNObjectPtr)( HIViewRef inTextView );
//
////-------------------------------------------------------------------------------
//// Copied from HITheme.h in SDK 10.3.9
////-------------------------------------------------------------------------------
//enum {
//  kHIThemeOrientationNormal     = 0,
//  kHIThemeOrientationInverted   = 1
//};
//
typedef UInt32                          HIThemeOrientation;
//
//enum {
//  kHIThemeTextTruncationNone    = 0,
//  kHIThemeTextTruncationMiddle  = 1,
//  kHIThemeTextTruncationEnd     = 2
//};
//
typedef UInt32                          HIThemeTextTruncation;
//
//enum {
//  kHIThemeTextHorizontalFlushLeft = 0,
//  kHIThemeTextHorizontalFlushCenter = 1,
//  kHIThemeTextHorizontalFlushRight = 2
//};
//
typedef UInt32                          HIThemeTextHorizontalFlush;
//
//enum {
//  kHIThemeTextVerticalFlushTop  = 0,
//  kHIThemeTextVerticalFlushCenter = 1,
//  kHIThemeTextVerticalFlushBottom = 2
//};
//
typedef UInt32                          HIThemeTextVerticalFlush;
//
//enum {
//  kHIThemeTextBoxOptionNone     = 0,
//  kHIThemeTextBoxOptionStronglyVertical = (1 << 1)
//};
//
typedef OptionBits                      HIThemeTextBoxOptions;
//
//enum {
//  kHIThemeTextInfoVersionZero   = 0
//};
//
//struct HIThemeTextInfo {
//  UInt32              version;
//  ThemeDrawState      state;
//  ThemeFontID         fontID;
//  HIThemeTextHorizontalFlush  horizontalFlushness;
//  HIThemeTextVerticalFlush  verticalFlushness;
//  HIThemeTextBoxOptions  options;
//  HIThemeTextTruncation  truncationPosition;
//  UInt32              truncationMaxLines;
//  Boolean             truncationHappened;
//
//};
//
typedef struct HIThemeTextInfo          HIThemeTextInfo;
//
typedef OSStatus (*HIThemeDrawTextBoxPtr)( CFStringRef inString, const HIRect * inBounds, HIThemeTextInfo * inTextInfo, CGContextRef inContext, HIThemeOrientation inOrientation );
//
//
////-------------------------------------------------------------------------------
//// Copied from ControlDefinitions.h in SDK 10.4.0
////-------------------------------------------------------------------------------
//// WARNING: This is 10.4 only functionality. Special care must be taken on use!!!
//enum {
//  kDataBrowserAttributeNone     = 0,
//  kDataBrowserAttributeColumnViewResizeWindow = (1 << 0),
//  kDataBrowserAttributeListViewAlternatingRowColors = (1 << 1),
//  kDataBrowserAttributeListViewDrawColumnDividers = (1 << 2)
//};
//
typedef OSStatus (*DataBrowserChangeAttributesPtr)( ControlRef inDataBrowser, OptionBits inAttributesToSet, OptionBits inAttributesToClear);
typedef OSStatus (*DataBrowserGetAttributesPtr)( ControlRef inDataBrowser, OptionBits * outAttributes );
//
////-------------------------------------------------------------------------------
//// Copied from QuickDraw.h in SDK 10.2.8
////-------------------------------------------------------------------------------
typedef OSStatus (*QDBeginCGContextPtr)(CGrafPtr inPort, CGContextRef *  outContext);
typedef OSStatus (*QDEndCGContextPtr)(CGrafPtr inPort, CGContextRef *  inoutContext);
//
////-------------------------------------------------------------------------------
//// Copied from CGContext.h in SDK 10.2.8
////-------------------------------------------------------------------------------
typedef void (*CGContextStrokeRectWithWidthPtr)(CGContextRef c, CGRect rect, float width);
typedef void (*CGContextTranslateCTMPtr)(CGContextRef c, float tx, float ty);
typedef void (*CGContextScaleCTMPtr)(CGContextRef c, float sx, float sy);
typedef void (*CGContextSetRGBStrokeColorPtr)(CGContextRef c, float red, float green, float blue, float alpha);
typedef void (*CGContextSetRGBFillColorPtr)(CGContextRef c, float red, float green, float blue, float alpha);
typedef void (*CGContextFillRectPtr)(CGContextRef c, CGRect rect);
typedef void (*CGContextAddLineToPointPtr)(CGContextRef c, float x, float y);
typedef void (*CGContextMoveToPointPtr)(CGContextRef c, float x, float y);
typedef void (*CGContextFillPathPtr)(CGContextRef c);
typedef void (*CGContextStrokePathPtr)(CGContextRef c);
typedef void (*CGContextSetShouldAntialiasPtr)(CGContextRef c, bool shouldAntialias);
typedef void (*CGContextSetLineDashPtr)(CGContextRef c, float phase, const float lengths[], size_t count);
typedef void (*CGContextSetLineJoinPtr)(CGContextRef c, CGLineJoin join);
typedef void (*CGContextSetLineWidthPtr)(CGContextRef c, float width);
typedef void (*CGContextSetLineCapPtr)(CGContextRef c, CGLineCap cap);
typedef void (*CGContextSetInterpolationQualityPtr)(CGContextRef c, CGInterpolationQuality quality);
typedef void (*CGContextFlushPtr)(CGContextRef c);
typedef void (*CGContextSynchronizePtr)(CGContextRef c);
typedef void (*CGContextBeginPathPtr)(CGContextRef c);
typedef void (*CGContextClosePathPtr)(CGContextRef c);
typedef void (*CGContextDrawImagePtr)(CGContextRef c, CGRect rect, CGImageRef image);
typedef void (*CGContextClearRectPtr)(CGContextRef c, CGRect rect);
typedef int (*CGRectContainsPointPtr)(CGRect rect, CGPoint point);
//
////-------------------------------------------------------------------------------
//// Copied from CGImage.h in SDK 10.2.8
////-------------------------------------------------------------------------------
typedef void (*CGImageReleasePtr)(CGImageRef image);
typedef CGImageRef (*CGImageCreateWithPNGDataProviderPtr)(CGDataProviderRef source, const float decode[], bool shouldInterpolate, CGColorRenderingIntent intent);
typedef CGImageRef (*CGImageMaskCreatePtr)(size_t width, size_t height, size_t bitsPerComponent, size_t bitsPerPixel, size_t bytesPerRow, CGDataProviderRef provider, const float decode[], bool shouldInterpolate);
typedef CGImageRef (*CGImageCreatePtr)(size_t width, size_t height, size_t bitsPerComponent, size_t bitsPerPixel, size_t bytesPerRow, CGColorSpaceRef colorspace, CGImageAlphaInfo alphaInfo, CGDataProviderRef provider, const float decode[], bool shouldInterpolate, CGColorRenderingIntent intent);
// 
////-------------------------------------------------------------------------------
//// Copied from CGDataProvider.h in SDK 10.2.8
////-------------------------------------------------------------------------------
typedef CGDataProviderRef (*CGDataProviderCreateWithURLPtr)(CFURLRef url);
typedef void (*CGDataProviderReleasePtr)(CGDataProviderRef provider);
typedef CGDataProviderRef (*CGDataProviderCreateWithDataPtr)(void *info, const void *data, size_t size, void (*releaseData)(void *info, const void *data, size_t size));
//
////-------------------------------------------------------------------------------
//// Copied from Icons.h in SDK 10.2.8
////-------------------------------------------------------------------------------
typedef OSStatus (*PlotIconRefInContextPtr)(CGContextRef inContext, const CGRect * inRect, IconAlignmentType inAlign, IconTransformType inTransform, const RGBColor * inLabelColor, PlotIconRefFlags inFlags, IconRef inIconRef);
//
////-------------------------------------------------------------------------------
//// Copied from CGColorSpace.h in SDK 10.2.8
////-------------------------------------------------------------------------------
typedef CGColorSpaceRef (*CGColorSpaceCreateDeviceGrayPtr)(void);
typedef CGColorSpaceRef (*CGColorSpaceCreateDeviceRGBPtr)(void);
typedef void (*CGColorSpaceReleasePtr)(CGColorSpaceRef cs);
//
//
////-------------------------------------------------------------------------------
//// Copied from Authorization.h in SDK 10.3.9
////-------------------------------------------------------------------------------
//#define kAuthorizationEmptyEnvironment	NULL
//
//enum {
//	errAuthorizationSuccess					= 0,      /* The operation completed successfully. */
//    errAuthorizationInvalidSet				= -60001, /* The set parameter is invalid. */
//    errAuthorizationInvalidRef				= -60002, /* The authorization parameter is invalid. */
//    errAuthorizationInvalidTag				= -60003, /* The tag parameter is invalid. */
//    errAuthorizationInvalidPointer			= -60004, /* The authorizedRights parameter is invalid. */
//	errAuthorizationDenied					= -60005, /* The authorization was denied. */
//	errAuthorizationCanceled				= -60006, /* The authorization was cancelled by the user. */
//	errAuthorizationInteractionNotAllowed	= -60007, /* The authorization was denied since no user interaction was possible. */
//	errAuthorizationInternal                = -60008, /* something else went wrong */
//	errAuthorizationExternalizeNotAllowed	= -60009, /* authorization externalization denied */
//	errAuthorizationInternalizeNotAllowed	= -60010, /* authorization internalization denied */
//	errAuthorizationInvalidFlags            = -60011, /* invalid option flag(s) */
//	errAuthorizationToolExecuteFailure      = -60031, /* cannot execute privileged tool */
//	errAuthorizationToolEnvironmentError    = -60032, /* privileged tool environment error */
//	errAuthorizationBadAddress				= -60033 /* invalid socket address requested */
//};
//
//
//enum {
//	kAuthorizationFlagDefaults              = 0,
//	kAuthorizationFlagInteractionAllowed	= (1 << 0),
//	kAuthorizationFlagExtendRights			= (1 << 1),
//	kAuthorizationFlagPartialRights			= (1 << 2),
//	kAuthorizationFlagDestroyRights			= (1 << 3),
//	kAuthorizationFlagPreAuthorize			= (1 << 4),
//	
//	// private bits (do not use)
//	kAuthorizationFlagNoData                = (1 << 20)
//};
//
typedef UInt32 AuthorizationFlags;
//
//enum {
//	kAuthorizationFlagCanNotPreAuthorize	= (1 << 0)
//};
//
//
typedef const struct AuthorizationOpaqueRef			*AuthorizationRef;

typedef const char *AuthorizationString;

//typedef struct {
//	AuthorizationString name;
//	UInt32 valueLength;
//	void *value;
//	UInt32 flags;
//} AuthorizationItem;

//typedef struct {
//	UInt32 count;
//	AuthorizationItem *items;
//} AuthorizationItemSet;

//
//enum {
//	kAuthorizationExternalFormLength = 32
//};
////
//typedef struct {
//	char bytes[kAuthorizationExternalFormLength];
//} AuthorizationExternalForm;

typedef AuthorizationItemSet AuthorizationRights;
typedef AuthorizationItemSet AuthorizationEnvironment;
//
//
typedef OSStatus (*AuthorizationCreatePtr)(const AuthorizationRights *rights,	const AuthorizationEnvironment *environment, AuthorizationFlags flags, AuthorizationRef *authorization);
typedef OSStatus (*AuthorizationFreePtr)(AuthorizationRef authorization, AuthorizationFlags flags);
typedef OSStatus (*AuthorizationCopyRightsPtr)(AuthorizationRef authorization, const AuthorizationRights *rights, const AuthorizationEnvironment *environment, AuthorizationFlags flags, AuthorizationRights **authorizedRights);
typedef OSStatus (*AuthorizationCopyInfoPtr)(AuthorizationRef authorization, AuthorizationString tag, AuthorizationItemSet **info);
typedef OSStatus (*AuthorizationMakeExternalFormPtr)(AuthorizationRef authorization, AuthorizationExternalForm *extForm);
typedef OSStatus (*AuthorizationCreateFromExternalFormPtr)(const AuthorizationExternalForm *extForm, AuthorizationRef *authorization);
typedef OSStatus (*AuthorizationFreeItemSetPtr)(AuthorizationItemSet *set);
typedef OSStatus (*AuthorizationExecuteWithPrivilegesPtr)(AuthorizationRef authorization, const char *pathToTool, AuthorizationFlags options, char * const *arguments, FILE **communicationsPipe);
typedef OSStatus (*AuthorizationCopyPrivilegedReferencePtr)(AuthorizationRef *authorization, AuthorizationFlags flags);
//
//
////-------------------------------------------------------------------------------
//// Copied from AuthorizationTags.h in SDK 10.3.9
////-------------------------------------------------------------------------------
//#define kAuthorizationEnvironmentUsername	"username"
//#define kAuthorizationEnvironmentPassword	"password"
//#define kAuthorizationEnvironmentShared		"shared"
//#define kAuthorizationRightExecute			"system.privilege.admin"
//#define kAuthorizationEnvironmentPrompt		"prompt"
//#define kAuthorizationEnvironmentIcon		"icon"

//###############################################################################
// Mach-O from CFM support
//###############################################################################
namespace FitsLiberator {
	namespace Mac {
		namespace MachO {

//-------------------------------------------------------------------------------
// Carbon.framework: Global variables definitions
//-------------------------------------------------------------------------------		
extern CreateMouseTrackingRegionPtr	CreateMouseTrackingRegion;
extern SetWindowMouseTrackingRegionsEnabledPtr SetWindowMouseTrackingRegionsEnabled;
extern HIViewFindByIDPtr HIViewFindByID;
extern HIViewGetRootPtr HIViewGetRoot;
extern HITextViewGetTXNObjectPtr HITextViewGetTXNObject;
extern SetMouseTrackingRegionEnabledPtr SetMouseTrackingRegionEnabled;
extern GetCurrentEventKeyModifiersPtr GetCurrentEventKeyModifiers;
extern HIViewAddSubviewPtr HIViewAddSubview;
extern HIViewGetSuperviewPtr HIViewGetSuperview;
extern HIViewGetBoundsPtr HIViewGetBounds;
extern HIComboBoxCreatePtr HIComboBoxCreate;
extern HIComboBoxGetItemCountPtr HIComboBoxGetItemCount;
extern HIComboBoxInsertTextItemAtIndexPtr HIComboBoxInsertTextItemAtIndex;
extern HIComboBoxAppendTextItemPtr HIComboBoxAppendTextItem;
extern HIComboBoxCopyTextItemAtIndexPtr HIComboBoxCopyTextItemAtIndex;
extern HIComboBoxRemoveItemAtIndexPtr HIComboBoxRemoveItemAtIndex;
extern HIComboBoxChangeAttributesPtr HIComboBoxChangeAttributes;
extern HIComboBoxGetAttributesPtr HIComboBoxGetAttributes;
extern HIViewSetVisiblePtr HIViewSetVisible;
extern HIViewIsVisiblePtr HIViewIsVisible;
extern HIViewPlaceInSuperviewAtPtr HIViewPlaceInSuperviewAt;
extern HIViewMoveByPtr HIViewMoveBy;
extern HIViewRemoveFromSuperviewPtr HIViewRemoveFromSuperview;
extern HIThemeDrawTextBoxPtr HIThemeDrawTextBox;
extern HIViewSetNeedsDisplayPtr HIViewSetNeedsDisplay;
extern HIViewSetNextFocusPtr HIViewSetNextFocus;
extern HIViewAdvanceFocusPtr HIViewAdvanceFocus;
extern HIViewSetFirstSubViewFocusPtr HIViewSetFirstSubViewFocus;
extern HIWindowChangeFeaturesPtr HIWindowChangeFeatures;
extern HIObjectRegisterSubclassPtr HIObjectRegisterSubclass;
extern HIObjectCreatePtr HIObjectCreate;

extern DataBrowserChangeAttributesPtr DataBrowserChangeAttributesNULL;
extern DataBrowserGetAttributesPtr DataBrowserGetAttributesNULL;

#define DataBrowserChangeAttributes(x,y,z) if( FitsLiberator::Mac::MachO::DataBrowserChangeAttributesNULL != NULL ) FitsLiberator::Mac::MachO::DataBrowserChangeAttributesNULL(x,y,z)
#define DataBrowserGetAttributes(x,y) if( FitsLiberator::Mac::MachO::DataBrowserGetAttributesNULL != NULL ) FitsLiberator::Mac::MachO::DataBrowserGetAttributesNULL(x,y)

//-------------------------------------------------------------------------------
// ApplicationServices.framework: Global variables definitions
//-------------------------------------------------------------------------------		
extern QDBeginCGContextPtr QDBeginCGContext;
extern QDEndCGContextPtr QDEndCGContext;
extern CGContextStrokeRectWithWidthPtr CGContextStrokeRectWithWidth;
extern CGContextTranslateCTMPtr CGContextTranslateCTM;
extern CGContextScaleCTMPtr CGContextScaleCTM;
extern CGContextSetRGBStrokeColorPtr CGContextSetRGBStrokeColor;
extern CGContextSetRGBFillColorPtr CGContextSetRGBFillColor;
extern CGContextFillRectPtr CGContextFillRect;
extern CGContextAddLineToPointPtr CGContextAddLineToPoint;
extern CGContextMoveToPointPtr CGContextMoveToPoint;
extern CGContextFillPathPtr CGContextFillPath;
extern CGContextStrokePathPtr CGContextStrokePath;
extern CGContextSetShouldAntialiasPtr CGContextSetShouldAntialias;
extern CGContextSetLineDashPtr CGContextSetLineDash;
extern CGContextSetLineJoinPtr CGContextSetLineJoin;
extern CGContextSetLineWidthPtr CGContextSetLineWidth;
extern CGContextSetLineCapPtr CGContextSetLineCap;
extern CGContextSetInterpolationQualityPtr CGContextSetInterpolationQuality;
extern CGContextFlushPtr CGContextFlush;
extern CGContextSynchronizePtr CGContextSynchronize;
extern CGContextBeginPathPtr CGContextBeginPath;
extern CGContextClosePathPtr CGContextClosePath;
extern CGContextDrawImagePtr CGContextDrawImage;
extern CGContextClearRectPtr CGContextClearRect;
extern CGImageReleasePtr CGImageRelease;
extern CGImageCreateWithPNGDataProviderPtr CGImageCreateWithPNGDataProvider;
extern CGImageMaskCreatePtr CGImageMaskCreate;
extern CGImageCreatePtr CGImageCreate;
extern CGDataProviderCreateWithURLPtr CGDataProviderCreateWithURL;
extern CGDataProviderReleasePtr CGDataProviderRelease;
extern CGDataProviderCreateWithDataPtr CGDataProviderCreateWithData;
extern PlotIconRefInContextPtr PlotIconRefInContext;
extern CGColorSpaceCreateDeviceGrayPtr CGColorSpaceCreateDeviceGray;
extern CGColorSpaceCreateDeviceRGBPtr CGColorSpaceCreateDeviceRGB;
extern CGColorSpaceReleasePtr CGColorSpaceRelease;
extern CGRectContainsPointPtr CGRectContainsPoint;

//-------------------------------------------------------------------------------
// Security.framework: Global variables definitions
//-------------------------------------------------------------------------------
extern AuthorizationCreatePtr AuthorizationCreate;
extern AuthorizationFreePtr AuthorizationFree;
extern AuthorizationCopyRightsPtr AuthorizationCopyRights;
extern AuthorizationCopyInfoPtr AuthorizationCopyInfo;
extern AuthorizationMakeExternalFormPtr AuthorizationMakeExternalForm;
extern AuthorizationCreateFromExternalFormPtr AuthorizationCreateFromExternalForm;
extern AuthorizationFreeItemSetPtr AuthorizationFreeItemSet;
extern AuthorizationExecuteWithPrivilegesPtr AuthorizationExecuteWithPrivileges;
extern AuthorizationCopyPrivilegedReferencePtr AuthorizationCopyPrivilegedReference;

//-------------------------------------------------------------------------------
// Support functions and variables.
//-------------------------------------------------------------------------------
extern CFBundleRef 	carbonFrameworkBundle;
extern CFBundleRef 	appServicesFrameworkBundle;
extern CFBundleRef 	securityFrameworkBundle;

Void 			LoadCarbonFrameworkBundle();
Void 			UnloadCarbonFrameworkBundle();
Void 			LoadApplicationServicesFrameworkBundle();
Void 			UnloadApplicationServicesFrameworkBundle();
Void 			LoadSecurityFrameworkBundle();
Void 			UnloadSecurityFrameworkBundle();
Void 			InitializeCarbonMachOPointes();
Void 			TerminateCarbonMachOPointes();
Void 			InitializeApplicationServicesMachOPointes();
Void 			TerminateApplicationServicesMachOPointes();
Void 			InitializeSecurityMachOPointes();
Void 			TerminateSecurityMachOPointes();
static Void* 	GetFunctionPointerForName( CFBundleRef, CFStringRef, Bool = true );
static Void* 	GetDataPointerForName( CFBundleRef, CFStringRef );
static OSStatus	UnloadFrameworkBundle( CFBundleRef );
static OSStatus	LoadFrameworkBundle( CFStringRef, CFBundleRef * );
			
		} // end namespace MachO
	} // end namespace Mac
} // end namespace FitsLiberator

#endif // __MACHOFRAMEWORKSUPPORT_H__