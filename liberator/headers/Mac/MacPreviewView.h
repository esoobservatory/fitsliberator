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

#ifndef __MACPREVIEWVIEW_H__
#define __MACPREVIEWVIEW_H__

#include "Types.h"
#include "BaseGui.h"
#include "FitsDialog.h"
#include "PreviewView.h"
#include "PreviewControl.h"

using namespace FitsLiberator::Modelling;

namespace FitsLiberator {
	namespace Mac {

		/**
		 *
		 *
		 */
		class MacPreviewView : public PreviewView, public ControlEventHandler {
			public:
			MacPreviewView( FitsDialog *, ToolController&, PreviewModel&, ToolModel&, GlobalSettingsModel&, PlaneModel&, ProgressModel&,
						   PreviewController&, FlowController& );
				~MacPreviewView();
								
				OSStatus onContextualMenuClick( EventRef );
				OSStatus onMouseEntered( EventRef );
				OSStatus onMouseExited( EventRef );
				OSStatus onDraw( EventRef );
				OSStatus onTrack( EventRef );
				OSStatus onHitTest( EventRef );
				OSStatus onMouseMoved( EventRef event );
				OSStatus onCommand( HICommand* );
			
			protected:
				virtual Void updateZooms( String*, Int );
				virtual Void setZoomCombo( Int );
				virtual Void drawPreview();
				virtual Void setCurrentTool( ToolTypeFunction );
				virtual Void updateZoomComboState( Bool );
				virtual void gotBusy();
				virtual void gotDone();
				
			private:
				/** Timer callback. For more information look at
					EventLoopTimerProcPtr in the SDK. */
				static void TimerProc(EventLoopTimerRef inTimer, 
					void* inUserData);
			
				Bool				showZoom;
				CGRect				zoomArea;
				
				EventLoopTimerRef	timer;
				int					progress;
				
				PreviewControl * 	previewControl;
				BaseControl*		zoomPopup;
				BaseControl*		plusButton;
				BaseControl*		minusButton;
				BaseMenu*			zoomPopupMenu;
				Rect				previewBounds;
				BaseMenu*			contextualMenu;
				Bool				inPreview;
				FitsDialog*			dialog;
				FitsCursorID		currentCursorActive;
				FitsCursorID		currentCursorInactive;
				FitsLiberator::Point centerPoint;
				ToolController&		toolController;
			
				ControlRef			tempFocusControl;
			
				MenuRef				mainMenu;	///< Main menu reference
			                                    ///< - for disabling the main menu when busy
		};
  
	
	}  // namespace Mac end
} // namespace FitsLiberator end
#endif // __MACPREVIEWVIEW_H__