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

#ifndef __FITSDIALOG_H__
#define __FITSDIALOG_H__

#include "BaseGui.h"
#include "FitsMacTerminology.h"
#include "NSCursorWrappers.h"
#include "Version.h"
#include <ext/hash_map>

namespace std {
	template<> struct equal_to<MouseTrackingRef> {
		 bool operator()( MouseTrackingRef s1, MouseTrackingRef s2 ) const { return ( s1 == s2 ); }
	};
}

namespace __gnu_cxx {
	template<> struct hash<MouseTrackingRef> {
		size_t operator()( const MouseTrackingRef& x ) const { return size_t( x ); }
	};	
}

namespace FitsLiberator {
	namespace Mac {


typedef __gnu_cxx::hash_map<MouseTrackingRef, Int> MouseTrackingTabs;

	
/**
 * FitsDialog specific implementation of TabControl, to enable/disable mouse
 * tracking.
 */
class FitsTabControl : public TabControl {
	public:
		FitsTabControl( BaseDialog *dlog, Int id, const TabPaneID* panes ) : TabControl( dlog, id, panes ) {};
		Void addTrackingRegion( MouseTrackingRef, Int );
		
		Void switchTab( TabPaneID switchToIndex );
	
	private:
		MouseTrackingTabs trackingRegions; ///< List of mouse tracking regions.
};

/**
 * Class for a Photoshop Dialog. Implements modal application event loop, which also
 * dispatch events to the host.
 */
class PhotoshopDialog : public BaseDialog {
	public:
		PhotoshopDialog() : BaseDialog(), continueEventLoop( true ) {};
	
		virtual Void		runModalEventLoop();
		virtual Void		quitModalEventLoop();
		virtual Void 		setContinueEventLoop( Bool );
		virtual Bool 		getContinueEventLoop();
	
	private:
		Bool 	continueEventLoop;
		
};

/**
 * Class for the main dialog of the FITS Liberator.
 */
class FitsDialog : public PhotoshopDialog, public EventHandler {
	public:
		FitsDialog( BundleFactory *factory );
		~FitsDialog();
			
		OSStatus			onCommand( HICommand *command );
		
		Bool				isCancelled();		
		Void 				globalToLocal( ::Point *p );
		Void 				translateWindowToTab( ::Point *p );
		Void 				translateWindowToTab( Rect *r );
		Void 				translateTabToWindow( Rect *r );
		Void 				translateTabToWindow( ::Point *p );
		Void 				translateWindowToScreen( ::Point *r );
		Void 				translateScreenToWindow( ::Point *p );
		Void 				translateControlToScreen( ::Point*, BaseControl* );
		Void 				translateScreenToControl( ::Point*, BaseControl* );
		MouseTrackingRef	createMouseTrackingRegion( const Rect*, MouseTrackingRegionID, BaseControl* );
		Void 				enteredRegion( SInt32 );
		Bool 				exitedRegion( SInt32 );
		FitsTabControl&		getTabControl();
		
		
		virtual Void		setCursor( FitsCursorID );
		
	protected:
		Void 				setExitState( Bool isContinue );
		
	private:
		void                loadHelpGuide();
		FitsTabControl *	tabControl;		///< Reference to tab control.
		Bool				exitState;		///< On exit, whether OK or Cancel was pressed.
		Int 				xTranslation;	///< X translation factor for tab control.
		Int 				yTranslation;	///< Y translation factor for tab control.
		
		/**
		 * Container to keep track of which regions was entered/exited, to prevent 
		 * calls like Entered Rgn1, Entered Rgn2, Exited Rgn1 when Rgn1 og Rgn2 are
		 * to distinct regions.
		 */
		 Bool				histogramRegion;
		 Bool				previewRegion;
	
		CursorRef			zoomInCursor;		///< NSCursor reference for Zoom In
		CursorRef			zoomOutCursor;		///< NSCursor reference for Zoom Out
		CursorRef			zoomLimitCursor;	///< NSCursor reference for Zoom Limit
	
		static const float	HOTSPOTX = 6.0f;
		static const float	HOTSPOTY = 6.0f;
};

/**
 * About dialog.
 */
class FitsAboutDialog : public PhotoshopDialog, public ControlEventHandler {
	public:
		FitsAboutDialog( BundleFactory *factory );
		~FitsAboutDialog();
		
		virtual OSStatus	processMouseEvent( EventHandlerCallRef, EventRef );	
		virtual OSStatus	onDraw( EventRef );
	
	private:
		CGImageRef 			image;					///< Image of about control. 
		TextControl* 		versionTextControl;		///< Version text for the about dialog. Note: Deprecated - version information is entered in the image.
		BaseControl* 		userPane;				///< User pane to draw the image in.
};


	} // namespace Mac end
} // namespace FitsLiberator end
#endif //__FITSDIALOG_H__