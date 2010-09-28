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

#include "BaseDialog.h"

using namespace FitsLiberator::Mac;

//-------------------------------------------------------------------------------
//	BaseDialog implementation
//-------------------------------------------------------------------------------

/**
 * Constructor for BaseDialog.
 */
BaseDialog::BaseDialog() : window( NULL ) {}

/**
 * Create a window from a nib-
 *
 * @param bundleFactory	A bundle factory. 
 * @param nibFileName	
 * @param windowName	
 *
 * @see BundleFactory.
 */
Void BaseDialog::create( BundleFactory* bundleFactory, ::CFStringRef nibFileName, ::CFStringRef windowName ) {
	::IBNibRef	windowNib;
	
	::CFBundleRef windowBundle = bundleFactory->getCFBundleRef();
	
	if( noErr != ::CreateNibReferenceWithCFBundle( windowBundle, nibFileName, &windowNib ) ) {
		throw Exception( "BaseDialog - Can't create reference to NIB file" );
	} 
	
    if( noErr != ::CreateWindowFromNib( windowNib, windowName, &window ) ) {
    	throw Exception( "Can't create window" );
    }
    
    ::DisposeNibReference( windowNib );
    bundleFactory->release( windowBundle );
}

/**
 * Dispose the window.
 */
Void BaseDialog::dispose() {
	if( window != NULL ) {
		::DisposeWindow( window );
		window = NULL;
	}
}

/**
 * Shows the window and initialize the cursor.
 */
Void BaseDialog::show() {
	if( window != NULL ) {
		::ShowWindow( window );
		::InitCursor();		// Changes the cursor from clock to pointer.
		::HiliteMenu( 0 ); 	// Removes the highlighting of the "File" menu.
	}
}

/**
 * Hides the window.
 */
Void BaseDialog::hide() {
	if( window != NULL ) {
		::HideWindow( window );
	}
}

/**
 * Activate the window.
 */
Void BaseDialog::activate( Bool b) {
	if( window != NULL ) {
		::ActivateWindow( window, b );
	}
}

/**
 * Bring window to front.
 */
Void BaseDialog::bringToFront() {
	if( window != NULL ) {
		::BringToFront( window );
	}
}

/**
 * Runs the application modal event loop for the dialog.
 */
Void BaseDialog::runModalEventLoop() {
	if( window != NULL ) {
		::RunAppModalLoopForWindow( window );
	}
}

/**
 * Runs the application modal event loop for the dialog.
 */
Void BaseDialog::quitModalEventLoop() {
	if( window != NULL ) {
		::QuitAppModalLoopForWindow( window );
	}
}

/**
 * Returns the windows graphics port.
 */
::CGrafPtr BaseDialog::getWindowPort() {
	if( window != NULL ) {
		return ::GetWindowPort( window );
	} else {
		return NULL;
	}
}

/**
 * Installs an event handler for the window. The user data for the event handler, will be 
 * a reference to this object.
 *
 * @param 	inList		A pointer to an array of EventTypeSpec entries representing the events you are interested in.
 * @throws 	Exception	On error.
 */
EventHandlerRef BaseDialog::installEventHandler( const ::EventTypeSpec * inList, UInt32 inNumTypes, EventHandler *handler ) {
	return BaseComponent::installBaseEventHandler( ::GetWindowEventTarget( window ), inList, inNumTypes, handler );
}


/**
 * Get a control by ID.
 *
 * @param id		...
 * @param ctrlRef	...
 */
ControlRef BaseDialog::getControlByID( Int id ) {
	::ControlRef 		ctrlRef	= NULL;
	const ::ControlID	ctrlID	= { kFITSApplicationSignature, id };
	
	getControlByID( (::ControlID *) &ctrlID, &ctrlRef );
	
	return ctrlRef;
}


/**
 * Get a control by ID.
 *
 * @param id		...
 * @param ctrlRef	...
 */
Void BaseDialog::getControlByID( Int id, ::ControlRef *ctrlRef ) {
	const ::ControlID ctrlID = { kFITSApplicationSignature, id };
	
	getControlByID( (::ControlID *) &ctrlID, ctrlRef );
}

/**
 * Get a control by ID.
 *
 * @param 	ctrlID		...
 * @param 	ctrlRef		...
 * @throws	Exception	On error.
 *
 * @see BaseDialog::getControlByID( Int id, ControlRef *ctrlRef )
 */
Void BaseDialog::getControlByID( ::ControlID *ctrlID, ::ControlRef *ctrlRef ) {	
	if( noErr != ::GetControlByID( window, ctrlID, ctrlRef ) ) {
		throw Exception( "Couldn't get control." );
	}
}

/**
 * Associates an arbitrary piece of data with a window.
 *
 * \code
 * SomeObject *buffer = new SomeObject(); 
 * dialog.setProperty( 'WIND', sizeof(buffer), (void *) buffer);
 * \endcode
 *
 * @param 	propertyTag	A value identifying the data to be associated. You define the tag your application uses to identify the data; this code is not to be confused with the file type for the data.
 * @param 	bufferSize	The size of the data to be associated.
 * @param 	buffer		A pointer to the data to be associated.
 * @throws	Exception	On error.
 *
 * @see BaseDialog::getProperty( PropertyTag propertyTag, void * buffer )
 */
//Void BaseDialog::setProperty( ::PropertyTag propertyTag, UInt32 bufferSize, const Void * buffer ) {
//	if( noErr != ::SetWindowProperty( window, kFITSApplicationSignature, propertyTag, bufferSize, buffer ) ) {
//		throw Exception( "Couldn't set window property" );
//	}	
//}


/**
 * Obtains a piece of data that is associated with a window. 
 *
 * @param	propertyTag	The application-defined code identifying the associated data to be obtained.
 * @param	buffer		On input, a pointer to a buffer. On return, this buffer contains a copy of 
 * 						the data that is associated with the specified window.
 * @throws	Exception	If can't get window property buffer size or can't get buffer.
 *
 * @see  BaseDialog::setProperty( PropertyTag propertyTag, UInt32 bufferSize, const void * buffer )
 */
//Void BaseDialog::getProperty( ::PropertyTag propertyTag, Void * buffer ) {
//	UInt32 		bufferSize;
//	OSStatus 	err;
//	
//	if( noErr == ::GetWindowPropertySize( window, kFITSApplicationSignature, propertyTag, &bufferSize ) ) {
//		if( noErr != ::GetWindowProperty( window, kFITSApplicationSignature, propertyTag, bufferSize, NULL, buffer) ) {
//			throw Exception( "Can't get window property" );
//		}
//	} else {
//		throw Exception( "Couldn't get size of window property" );
//	}
//}

/**
 * Get the windows alpha value.
 *
 * @param 	alpha		On input a pointer to a float, on return the alpha value of the window.
 * @throws	Exception	If window hasn't been created.
 */
Void BaseDialog::getAlpha( Float *alpha ) {
	if( window != NULL ) {
		::GetWindowAlpha( window, alpha );
	} else {
		throw Exception( "Window not created" );
	}
}

/**
 * Set the windows alpha value.
 *
 * @param 	alpha		On input a pointer to a float, on return the alpha value of the window.
 * @throws	Exception	If window hasn't been created or couldn't set alpha value.
 */
Void BaseDialog::setAlpha( Float alpha ) {
	if( window != NULL ) {
		if( noErr != ::SetWindowAlpha( window, alpha ) ) {
			throw Exception( "Couldn't set alpha value of the window!" );
		}
	} else {
		throw Exception( "Window not created!" );
	}
}

/**
 * Obtains the size and position of the bounding rectangle of the specified window region.
 * 
 * @param globalBounds	A pointer to a structure of type Rect. On return, the rectangle contains the dimensions and position, in global coordinates, of the window region specified in the regionCode parameter.	
 */
Void BaseDialog::getBounds( ::Rect *globalBounds ) {
	if( noErr != ::GetWindowBounds( window, kWindowContentRgn, globalBounds) ) {
		throw Exception( "Couldn't get bounds of window." );
	}
}

/**
 * Sets the title of the window.
 *
 * @param 	title 		Title of window.
 * @throws	Exception	On error.
 */
Void BaseDialog::setTitle( ::CFStringRef title ) {
	if( noErr != ::SetWindowTitleWithCFString( window, title ) ) {
		throw Exception( "Couldn't set title of window" );
	}
}

/**
 * Gets the title of the window.
 *
 * @param 	title 		On input a pointer to a CFStrignRef, on return the title of the window.
 * @throws	Exception	On error.
 */
Void BaseDialog::getTitle( ::CFStringRef *title ) {
	if( noErr != ::CopyWindowTitleAsCFString( window, title ) ){
		throw Exception( "Couldn't get title of window" );
	}
}

/**
 * Appends a title to the window title.
 *
 * @param	appendTitle		String to append to window title.
 * 
 * @see BaseDialog::getTitle( CFStringRef *title )
 * @see BaseDialog::setTitle( CFStringRef title )
 * @todo Can be implemented better with CFMutableString.
 * @todo Should be Moved to FitsDialog.
 */
Void BaseDialog::appendTitle( ::CFStringRef appendTitle ){
	::CFStringRef tmpTitle;
	::CFMutableStringRef newTitle; 
	
	getTitle( &tmpTitle );
	newTitle = ::CFStringCreateMutableCopy( NULL, 0, tmpTitle );
	
	CFRelease( tmpTitle );
	
	::CFStringAppend( newTitle, appendTitle );
	tmpTitle = ::CFStringCreateCopy( NULL, newTitle );
	CFRelease( newTitle );
		
	if( tmpTitle != NULL ) {
		setTitle( tmpTitle );	
		::CFRelease( tmpTitle );
	}
}

/**
 * Clear the keyboard focus. 
 *
 * @throws Exception	On error.
 */
Void BaseDialog::clearKeyboardFocus() {
	if( noErr != ::ClearKeyboardFocus( window ) ) {
		throw Exception( "Couldn't clear keyborad focus." );
	}
}

/**
 * Select the window
 *
 * @throws Exception	On error.
 */
Void BaseDialog::select() {
	::SelectWindow( window );
}

/**
 * Draw all controls in the window.
 *
 * @throws Exception	On error.
 */
Void BaseDialog::drawControls() {
	::DrawControls( window );
}

/**
 * Get the window reference
 *
 * @throws Exception	On error.
 */
WindowRef BaseDialog::getWindow() {
	return window;
}

/**
 * Convert global coordinates to local coordinates for the window.
 */
Void BaseDialog::globalToLocal( ::Point *p ) {
	::CGrafPtr port;

	::GetPort( &port );
	::SetPort( getWindowPort() );
	
	::GlobalToLocal( p );
	
	::SetPort( port );
}

/**
 * Creates a mouse tracking region.
 *
 * @param bounds	The bounds for the mouse tracking region.
 * @param trackID	ID of mouse tracking region.
 * @param ctrl 		Pointer to BaseControl which should be used as event target. 
 *					May be NULL to use window as event target.
 * @return 			A mouse tracking reference.
 */
MouseTrackingRef BaseDialog::createMouseTrackingRegion( const Rect* bounds, MouseTrackingRegionID trackID, BaseControl* ctrl ) {
	RgnHandle 			trackRgn = ::NewRgn();
	MouseTrackingRef	trackRef = NULL;
	
	::SetRectRgn( trackRgn, bounds->left, bounds->top, bounds->right, bounds->bottom );
	
	if( ctrl == NULL ) {
		MachO::CreateMouseTrackingRegion( window, trackRgn, NULL, kMouseTrackingOptionsLocalClip, trackID,  NULL, NULL, &trackRef );
	} else {
		MachO::CreateMouseTrackingRegion( window, trackRgn, NULL, kMouseTrackingOptionsLocalClip, trackID,  NULL, ::GetControlEventTarget( ctrl->getControl() ), &trackRef );	
	}
		
	::DisposeRgn( trackRgn );
	
	return trackRef;
}

/**
 * Set the cursor
 *
 * @param curs	A theme cursor ID. FitsCursorID is of same type as ThemeCursor.
 */
Void BaseDialog::setCursor( FitsCursorID curs ) {
	::SetThemeCursor( curs );
}