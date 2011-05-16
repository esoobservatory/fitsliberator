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

#include "FitsDialog.h"

using namespace FitsLiberator::Mac;

//-------------------------------------------------------------------------------
//	PhotoshopDialog implementation
//-------------------------------------------------------------------------------

/**
 * Overrides the systems default event handling, since we also need
 * to update the host.
 *
 * @todo 	Do not convert the EventRef to EventRecord before we know it is a update
 *			or activate event.
 * @todo	Do not send events to Photoshop, that is for our window.
 * @todo	Verify that Apple Events are processed properly.
 * @todo	Contains code restricted by Photoshop licence agreement.
 */
Void PhotoshopDialog::runModalEventLoop() {
	::BeginAppModalStateForWindow( getWindow() );
	
	::InstallStandardEventHandler( ::GetWindowEventTarget( getWindow() ) );
	
	::EventRef 			event;
	::EventRecord 		psEvent;
	::EventTargetRef 	target = ::GetEventDispatcherTarget();
	
	OSStatus status = ::ReceiveNextEvent( 0, NULL, kEventDurationForever, true, &event);
    while( ( status == noErr ) || ( continueEventLoop )  ) {
    	if( status != eventLoopQuitErr ) {
			continueEventLoop = false;
		}
    
    	::ConvertEventRefToEventRecord( event, &psEvent );
    	
    	if( ::GetEventClass( event ) ==  kEventClassAppleEvent ) {
    		::AEProcessAppleEvent(  &psEvent );
    	} else {  	
    		::SendEventToEventTarget( event, target );
    	
    		// Send events to the host.	
    		if( psEvent.what == updateEvt || psEvent.what == activateEvt ) {
				// TODO-NEW: sPSUIHooks->processEvent( &psEvent );
			}
    	}
		
		::ReleaseEvent( event );
		
		status = ::ReceiveNextEvent( 0, NULL, kEventDurationForever, true, &event);
	}
}

Void PhotoshopDialog::setContinueEventLoop( Bool value ) {
	continueEventLoop = value;
}

Bool PhotoshopDialog::getContinueEventLoop() {
	return continueEventLoop;
}

/**
 * Exit the event loop run with FitsDialog::runModalEventLoop();
 */
Void PhotoshopDialog::quitModalEventLoop() {
	::QuitEventLoop( ::GetCurrentEventLoop() );
	::EndAppModalStateForWindow( getWindow() );
}

//-------------------------------------------------------------------------------
//	FitsDialog implementation
//-------------------------------------------------------------------------------

/**
 * Constructor for FitsDialog.
 */
FitsDialog::FitsDialog( BundleFactory *bundleFactory ) 
: exitState( false ),
  histogramRegion( false ),
  previewRegion( false ),
  PhotoshopDialog() {
	static const ::EventTypeSpec eventTypes[] = {
		{ kEventClassCommand, kEventCommandProcess },
	}; 
		
	create( bundleFactory, CFSTR( kFITSNibFile ), CFSTR( kFITSNibWindow ) );
	installEventHandler( eventTypes, GetEventTypeCount( eventTypes ), this );	
	
	// TabControl
	static const TabPaneID panes[] = { 4, kFITSUITabPanePreview, kFITSUITabPaneVR, kFITSUITabPaneHeader, kFITSUITabPaneHelpGuide };
	tabControl = new FitsTabControl( this, kFITSUITabControl, panes );
	
	static const ::EventTypeSpec tabControlEvents = { kEventClassControl, kEventControlHit };	
	tabControl->installEventHandler( &tabControlEvents, 1, tabControl );
	
	  
	loadHelpGuide();	  
	  
	// We get the translation at this point, since we don't
	// change it later on. Therefore it's faster to store it
	// here. 
	::Point p = tabControl->getTranslation();
	xTranslation = p.h;
	yTranslation = p.v;
	  
	//
	// Create cursors
	zoomInCursor    = ::CreateCocoaCursorFromUrl(Environment::getBundleResourceURL("cursorZoomIn.tiff"), HOTSPOTX, HOTSPOTY);
	zoomOutCursor   = ::CreateCocoaCursorFromUrl(Environment::getBundleResourceURL("cursorZoomOut.tiff"), HOTSPOTX, HOTSPOTY);
	zoomLimitCursor = ::CreateCocoaCursorFromUrl(Environment::getBundleResourceURL("cursorZoomLimit.tiff"), HOTSPOTX, HOTSPOTY);
}

/**
 * Destructor disposes tab control.
 */
FitsDialog::~FitsDialog() {
	if( tabControl != NULL ) {
		delete tabControl;		
	}
	
	::ReleaseCocoaCursor(zoomInCursor);
	::ReleaseCocoaCursor(zoomOutCursor);
	::ReleaseCocoaCursor(zoomLimitCursor);
}


/**
 * Convert global coordinates to local coordinates for the window.
 */
Void FitsDialog::globalToLocal( ::Point *p ) {
	BaseDialog::globalToLocal( p );
	
	translateWindowToTab( p );
	
}

/**
 * Translate coordinates relative to window, to coordinates relative to tab.
 */
Void FitsDialog::translateWindowToTab( ::Point *p ) {
	p->h -= xTranslation;
	p->v -= yTranslation;
}

/**
 * Translate coordinates relative to window, to coordinates relative to tab.
 */
Void FitsDialog::translateWindowToTab( Rect *r ) {
	r->top		-= yTranslation;
	r->left 	-= xTranslation;
	r->bottom	-= yTranslation;
	r->right 	-= xTranslation;
}

/**
 * Translate coordinates relative to tab, to coordinates relative to window.
 */
Void FitsDialog::translateTabToWindow( Rect *r ) {
	r->top		+= yTranslation;
	r->left 	+= xTranslation;
	r->bottom	+= yTranslation;
	r->right 	+= xTranslation;
}

/**
 * Translate coordinates relative to tab, to coordinates relative to tab.
 */
Void FitsDialog::translateTabToWindow( ::Point *p ) {
	p->h += xTranslation;
	p->v += yTranslation;
}

/**
 * Translate coordinates relative to window, to coordinates relative to screen.
 */
Void FitsDialog::translateWindowToScreen( ::Point *p ) {
	Rect r;
	getBounds( &r );

	p->h += r.left;
	p->v += r.top;
}

/**
 * Translate coordinates relative to screen, to coordinates relative to window.
 */
Void FitsDialog::translateScreenToWindow( ::Point *p ) {
	Rect r;
	getBounds( &r );

	p->h -= r.left;
	p->v -= r.top;
}

/**
 * Translates coordinates relative to control to coordinates relative to screen, if
 * control is in tab control.
 */
Void FitsDialog::translateControlToScreen( ::Point* p, BaseControl* ctrl ) {
	Rect bounds;
	ctrl->getBounds( &bounds );
	
	p->h += bounds.left;
	p->v += bounds.top; 
	
	translateTabToWindow( p );
	translateWindowToScreen( p );
}

/**
 * Translates coordinates relative to control to coordinates relative to screen, if
 * control is in tab control.
 */
FitsTabControl& FitsDialog::getTabControl() {
	return *tabControl;
}


/**
 * Translates coordinates relative to screen to coordinates relative to control, if
 * control is in tab control.
 */
Void FitsDialog::translateScreenToControl( ::Point* p, BaseControl* ctrl ) {
	translateScreenToWindow( p );
	translateWindowToTab( p );
	
	Rect bounds;
	ctrl->getBounds( &bounds );
	
	p->h -= bounds.left;
	p->v -= bounds.top; 
}

/**
 * Command handler and propagater.  
 *
 * @param	command		A reference to the command to handle.
 */
OSStatus FitsDialog::onCommand( ::HICommand *command ) {
	OSStatus result = eventNotHandledErr;

	switch( command->commandID ) {
		case kFITSUICommandOk:
			setExitState( true );
		case kFITSUICommandCancel:
			quitModalEventLoop();
			result = noErr;
			break;
	}
	
	return result;
}

/**
 * Set whether the user press OK or Cancel
 * 
 * @param isContinue	Set to true to continue, false to cancel.
 */
Void FitsDialog::setExitState( Bool isContinue ) {
	exitState = isContinue;
}

/**
 * If user wants to cancel
 *
 * @return True if user wants to cancel, false otherwise.
 */
Bool FitsDialog::isCancelled() {
	return !exitState;
}

/**
 * Ensures that bounds bounds are translated correctly due to tab, before
 * creating the mouse tracking region.
 *
 * @see BaseDialog::createMouseTrackingRegion()
 */
MouseTrackingRef FitsDialog::createMouseTrackingRegion( const Rect* bounds, MouseTrackingRegionID trackID, BaseControl* ctrl ) {
	Rect translatedBounds = *bounds;
	
	translateTabToWindow( &translatedBounds );
	
	MouseTrackingRef trackRef = BaseDialog::createMouseTrackingRegion( &translatedBounds, trackID, ctrl );
	
	tabControl->addTrackingRegion(trackRef, ctrl->getID() );
	
	return trackRef;
}

/**
 * Set the cursor for FitsDialog. 
 * 
 * @param	A cursor ID. In contrast to BaseDialog::setCursor() this is NOT a
 *			a ThemeCursor ID. It's an id defined in FitsMacTerminology.h
 *			This ensure that we can also use resource based cursors.
 */
Void FitsDialog::setCursor( FitsCursorID curs ) {
	switch( curs ) {
		case kFITSUICursorNormal:
			::SetThemeCursor( kThemeArrowCursor );
			break;
		
		case kFITSUICursorMoveInactive:
			::SetThemeCursor( kThemeOpenHandCursor );
			break;
		
		case kFITSUICursorMoveActive:
			::SetThemeCursor( kThemeClosedHandCursor );
			break;
			
		case kFITSUICursorZoomIn:
			::SetCocoaCursor(zoomInCursor);
			break;
			
		case kFITSUICursorZoomOut:
			::SetCocoaCursor(zoomOutCursor); 
			break;
			
		case kFITSUICursorZoomLimit:
			::SetCocoaCursor(zoomLimitCursor);
			break;
		
		case kFITSUICursorPickerBlack:
		case kFITSUICursorPickerBackground:	
		case kFITSUICursorPickerPeak:	
		case kFITSUICursorPickerWhite:
			::SetThemeCursor( kThemeCrossCursor );
			break;			
	}
}

/**
 * Report that mouse has entering a region.
 *
 * @param regionId	Use mouse tracking region id.
 */
Void FitsDialog::enteredRegion( SInt32 regionId ) {
	switch( regionId ) {
		case kFITSUIHistogramTrackingRegion:
			histogramRegion = true;
			previewRegion 	= false;
			break;
		
		case kFITSUIPreviewTrackingRegion:
			histogramRegion = false;
			previewRegion 	= true;
			break;
			
	}
}

/**
 * Report that the mouse exited a region.
 *
 * @param regionId	Use mouse tracking region id.
 * @return 			True if the mouse hasn't entered another region, false if it has entered another region.
 */
Bool FitsDialog::exitedRegion( SInt32 regionId ) {
	switch( regionId ) {
		case kFITSUIHistogramTrackingRegion:
			// Note if mouse has entered another region, histogramRegion will be false.
			if( histogramRegion ) {
				histogramRegion = false;
			} else {
				return false;
			}
			break;
		
		case kFITSUIPreviewTrackingRegion:
			if( previewRegion ) {
				previewRegion = false;
			} else {
				return false;
			}
			break;
	}
	
	return true;
}


void FitsDialog::loadHelpGuide() {
	HIViewID	textViewID;
	HIViewRef 	textView;
	
	// Text Control
	textViewID.signature 	= kFITSApplicationSignature;
	textViewID.id 			= kFITSUITextViewHelpGuide;
	
	MachO::HIViewFindByID( MachO::HIViewGetRoot( getWindow() ), textViewID, &textView );
	
	TXNObject textObject = MachO::HITextViewGetTXNObject( textView );
	::TXNReadFromCFURL( textObject, kTXNStartOffset, kTXNEndOffset, NULL, Environment::getBundleResourceURL( "HelpGuide.rtf" ), NULL );
}


//-------------------------------------------------------------------------------
//	FitsTabControl implementation
//-------------------------------------------------------------------------------

/**
 * Specific implementation of switchTab in TabControl for FitsDialog. This is to 
 * disable/enable onMouseEntered/onMouseExited events dispatching only in Preview tab.
 *
 * @see TabControl::switchTab
 */
Void FitsTabControl::switchTab( TabPaneID index ) {
	TabControl::switchTab( index );
	
	getDialog()->clearKeyboardFocus();
	
	// Note the numbers 100,200,300,400 indicates controls that lies with this area of ideas.
	// That means that tab 1 has from 100 to 200. All controls creating mouse tracking regions
	// will have to have an id within this range.
	if( index >= 1 && index <= 4 ) {
		for( MouseTrackingTabs::iterator it = trackingRegions.begin(); it != trackingRegions.end(); it++ ) {
			MachO::SetMouseTrackingRegionEnabled( (*it).first, ( (*it).second > 100*index && (*it).second < 100*(index+1) ) );
		}	
	}	
}

/**
 *
 *
 */
Void FitsTabControl::addTrackingRegion( MouseTrackingRef trackRef, Int controlID ) {
	trackingRegions[ trackRef ] = controlID;
	
	for( MouseTrackingTabs::iterator it = trackingRegions.begin(); it != trackingRegions.end(); it++ ) {
		MachO::SetMouseTrackingRegionEnabled( (*it).first, ( (*it).second > 100*lastTabIndex && (*it).second < 100*(lastTabIndex+1) ) );
	}
}

//-------------------------------------------------------------------------------
//	FitsAboutDialog implementation
//-------------------------------------------------------------------------------

/**
 * Constructor for FitsDialog.
 */
FitsAboutDialog::FitsAboutDialog( BundleFactory *bundleFactory ) 
:  PhotoshopDialog(), 
   image( NULL ),
   versionTextControl( NULL ),
   userPane( NULL ) {
	
	static const ::EventTypeSpec eventTypes[] = {
		{ kEventClassMouse, kEventMouseUp },
		{ kEventClassWindow, kEventWindowGetRegion }
	}; 
		
	create( bundleFactory, CFSTR( kFITSNibFile ), CFSTR( kFITSNibWindowAbout ) );
	installEventHandler( eventTypes, GetEventTypeCount( eventTypes ), this );
	
	
	static const ::EventTypeSpec imageEventTypes[] = {
		{ kEventClassControl, kEventControlDraw }
	}; 

	HIViewRef rootControl = MachO::HIViewGetRoot( getWindow() );
	installBaseEventHandler( ::GetControlEventTarget(rootControl), imageEventTypes, GetEventTypeCount( imageEventTypes), this );
	
	// telling the HIToolbox that our window is not opaque so that we will be asked for the opaque region
	UInt32 features;
	::GetWindowFeatures( getWindow(), &features);

	if ( ( features & kWindowIsOpaque ) != 0 ) {
		MachO::HIWindowChangeFeatures( getWindow(), 0, kWindowIsOpaque );
	}
		
	// force opaque shape to be recalculated
	::ReshapeCustomWindow( getWindow() );

	// ensure that HIToolbox doesn't use standard shadow style, which defeats custom opaque shape
	// this is only needed before Mac OS X v10.4
	long response;
	OSStatus status = ::Gestalt( gestaltSystemVersion, &response );
	if( !((status == noErr) && (response < 0x00001040)) ) {
		setAlpha( 0.999 );
	}
	
	userPane = new BaseControl( this, kFITSUIPictAbout );
	userPane->installEventHandler( imageEventTypes, GetEventTypeCount( imageEventTypes), this );
	
	// Load image
	CFBundleRef	bundle 	= Environment::getBundleFactory()->getCFBundleRef();
	CFURLRef 	url		= ::CFBundleCopyResourceURL( bundle, CFSTR("About.png"), NULL, NULL );
	CGDataProviderRef provider = MachO::CGDataProviderCreateWithURL( url );
	image = MachO::CGImageCreateWithPNGDataProvider( provider, NULL, true, kCGRenderingIntentDefault );
	::CFRelease( url );
	MachO::CGDataProviderRelease( provider );	
	
	// Set version text
	//String text = "Version " FITSVERSION;
	//versionTextControl = new TextControl( this, kFITSUITextVersion );
	//versionTextControl->setText( text );
}

/**
 * Destructor disposes tab control.
 */
FitsAboutDialog::~FitsAboutDialog() {
	if( image != NULL ) {
		MachO::CGImageRelease( image );	
	}
	
	//if( versionTextControl != NULL ) {
	//	delete versionTextControl;
	//}
	
	if( userPane != NULL ) {
		delete userPane;
	}
}

/**
 * Window event handler for about dialog.
 */
OSStatus FitsAboutDialog::processMouseEvent( EventHandlerCallRef handler, EventRef event ) {
	switch( GetEventKind(event) ) {
		case kEventMouseUp:
			quitModalEventLoop();
			return noErr;
			break;
		
		default:
			return eventNotHandledErr;
	}
}

/**
 * Draw event handler for user pane dialog.
 */
OSStatus FitsAboutDialog::onDraw( EventRef event ) {
	ControlRef ctrl;
	CGContextRef context;
	Rect r;
	
	::GetEventParameter( event, kEventParamDirectObject, typeControlRef, NULL, sizeof (ControlRef), NULL, &ctrl );
	::GetEventParameter( event, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof (CGContextRef), NULL, &context );
	
	::GetControlBounds( ctrl, &r );
	MachO::CGContextClearRect(context, CGRectMake( 0, 0, r.right - r.left, r.bottom - r.top) );
	
	// We want to use quartz coordinates instead of QuickDraw
	MachO::CGContextScaleCTM( context, 1.0, -1.0);
	MachO::CGContextTranslateCTM( context, 0, -(r.bottom - r.top) );
	if( image != NULL ) {
		MachO::CGContextDrawImage( context, CGRectMake( 0, 0, r.right - r.left, r.bottom - r.top ), image );
	} else {
		BaseDebugStr("Couldn't draw image!");	
	}
	
	
	//versionTextControl->drawControl();
	
	return noErr;
}