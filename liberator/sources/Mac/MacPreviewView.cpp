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
 
#include "MacPreviewView.h"

#include <sstream>

using namespace FitsLiberator::Modelling;
using namespace FitsLiberator::Mac;

/**
 * Initialize controls and view.
 */
MacPreviewView::MacPreviewView( FitsDialog* dlog, ToolController& tctrl, PreviewModel& model,ToolModel& tmodel, GlobalSettingsModel& gmodel, 
							   PlaneModel& pmodel, ProgressModel& prgModel,
							   PreviewController& c, FlowController& fc)
: previewControl( NULL ), 
  contextualMenu( NULL ),
  zoomPopup( NULL ),
  zoomPopupMenu( NULL ),
  showZoom( false ),
  dialog( dlog ),
  inPreview( false ),
  toolController( tctrl ),
  timer(NULL),
  progress(-1),
  mainMenu(NULL),
  tempFocusControl(NULL),
  PreviewView( model, tmodel, gmodel, pmodel, prgModel, c, fc ) {
  
  	//
  	// Contextual menu in preview
  	// 
  	static const EventTypeSpec menuEventTypes[] = { 
		{ kEventClassCommand, kEventCommandProcess }
  	};
  	
  	plusButton = new BaseControl( dialog, kFITSUIBevelButtonPlus );
  	minusButton = new BaseControl( dialog, kFITSUIBevelButtonMinus );
  	plusButton->installEventHandler( menuEventTypes, GetEventTypeCount( menuEventTypes ), this );
  	minusButton->installEventHandler( menuEventTypes, GetEventTypeCount( menuEventTypes ), this );
  
  	contextualMenu = new BaseMenu();
  	contextualMenu->create( Environment::getBundleFactory(), CFSTR( kFITSNibFile ), CFSTR( kFITSNibPreviewMenu ) );
  	contextualMenu->installEventHandler( menuEventTypes, GetEventTypeCount( menuEventTypes ), this );
	 	
  	//
  	// Popup Zoom Button
  	// 
	zoomPopup = new BaseControl( dialog, kFITSUIPopupButtonZoom );
  	zoomPopup->installEventHandler( menuEventTypes, GetEventTypeCount( menuEventTypes ), this );
  	
  	MenuRef zoomMenuRef; ::Size actualSize;	
	
	zoomPopup->getData( kControlEntireControl, kControlPopupButtonMenuRefTag, &zoomMenuRef, sizeof(MenuRef), &actualSize );
  	
  	zoomPopupMenu = new BaseMenu( zoomMenuRef );
	zoomPopupMenu->installEventHandler( menuEventTypes, GetEventTypeCount( menuEventTypes ), this );

	//
  	// Preview
  	// 
	static const EventTypeSpec previewEventTypes[] = { 
		{ kEventClassControl, kEventControlContextualMenuClick },
		{ kEventClassControl, kEventControlDraw },
		{ kEventClassControl, kEventControlHitTest },
		{ kEventClassControl, kEventControlTrack },
		{ kEventClassMouse, kEventMouseEntered },
		{ kEventClassMouse, kEventMouseExited }
	};

	previewControl = new PreviewControl( dialog, kFITSUICustomPreview );
	previewControl->getBounds( &previewBounds );
	previewControl->installEventHandler( previewEventTypes, GetEventTypeCount(previewEventTypes), this );
	previewControl->setContextualMenu( contextualMenu );
	
	MouseTrackingRegionID trackID = { kFITSApplicationSignature, kFITSUIPreviewTrackingRegion };
	dialog->createMouseTrackingRegion( &previewBounds, trackID, previewControl );
	
	// Install mouse moved event handler.
	static const EventTypeSpec windowEventTypes[] = { 
		{ kEventClassMouse, kEventMouseMoved }
	};
	
	dlog->installEventHandler( windowEventTypes, GetEventTypeCount(windowEventTypes), this );
	
	FitsLiberator::Size s( previewBounds.right - previewBounds.left, previewBounds.bottom - previewBounds.top );
	model.setPreviewSize( s );
	
	centerPoint.x = (previewBounds.right-previewBounds.left)/2;
	centerPoint.y = (previewBounds.bottom-previewBounds.top)/2;
	
	static const EventTypeSpec keyboardEventTypes[] = {
		{ kEventClassKeyboard, kEventRawKeyDown }
	};
	
	dialog->installEventHandler( keyboardEventTypes, GetEventTypeCount(keyboardEventTypes), this );
}

/**
 * Destroy controls in view.
 */
MacPreviewView::~MacPreviewView() {
	if( previewControl != NULL ) 		{ delete previewControl; }
	if( contextualMenu != NULL ) 		{ delete contextualMenu; }
	if( zoomPopup != NULL )				{ delete zoomPopup; }
	if( zoomPopupMenu != NULL )			{ delete zoomPopupMenu; }
	if( plusButton != NULL )			{ delete plusButton; }
	if( minusButton != NULL )			{ delete minusButton; }
}

/**
 * Handle a contextual menu click in the preview. Change the cursor accordingly.
 */
OSStatus MacPreviewView::onContextualMenuClick( EventRef event ) {
	if( previewControl->getContextualMenu() != NULL ) {
		::Point mouseLocation;
		
		// 4 and 5 is the index of Zoom in and zoom out menu items.
		previewControl->getContextualMenu()->enableItem( model.canIncrement(), kFITSUIMenuIndexZoomIn );
		previewControl->getContextualMenu()->enableItem( model.canDecrement(), kFITSUIMenuIndexZoomOut );
		
		dialog->setCursor( kFITSUICursorNormal );
    
    	::GetGlobalMouse( &mouseLocation );
    	::PopUpMenuSelect( previewControl->getContextualMenu()->getMenu(), mouseLocation.v, mouseLocation.h, 0 );
    	::GetGlobalMouse( &mouseLocation );

    	dialog->translateScreenToControl( &mouseLocation, previewControl );
    	
    	// onMouseExited takes care of the cursor, if it's not in the control.
    	if( kControlPreviewPart == previewControl->getControlPart( &mouseLocation ) ) {
    		dialog->setCursor( currentCursorInactive );
    	}
    	
		return noErr;
    } else {
    	return eventNotHandledErr;	
    }
}

/**
 *  
 */
OSStatus MacPreviewView::onMouseMoved( EventRef event ) {
	if( globalSettingsModel.getPreviewEnabled() ) {
		::Point mouseLocation;
		static FitsLiberator::Point p;
		
		::GetEventParameter( event, ::kEventParamMouseLocation, typeQDPoint, NULL, sizeof(::Point), NULL, &mouseLocation );
		
		dialog->translateScreenToControl( &mouseLocation, previewControl );
		
		if( previewControl->getControlPart( &mouseLocation ) == kControlPreviewPart ) {
			p.x = mouseLocation.h;
			p.y = mouseLocation.v;
			
			flowController.setCoordinates( p );
		}		
	}

	return eventNotHandledErr;
}

/**
 *  Handling of menu commands.
 */
OSStatus MacPreviewView::onCommand( HICommand* command ){
	OSStatus err = eventNotHandledErr;
	
	// Make sure we only respond to commands when we're in the right tab.
	if( dialog->getTabControl().getEnabledTab() == 1 ) {
		Int selectedIndex = command->menu.menuItemIndex - 1;
		switch( command->commandID ) {
			case kFITSUICommandMenuZoom:
				if( selectedIndex >= model.getUnityZoomIndex() ) {
					if( selectedIndex > model.getUnityZoomIndex()+1 ) {
						selectedIndex--;
					}
					selectedIndex--;
				}
				
				flowController.setZoomIndex( selectedIndex );
				err = noErr;
				break;
		
			case kFITSUICommandMenuZoomFit:
				flowController.fitToPreview();
				err = noErr;
				break;
			
			case kFITSUICommandMenuZoom100:
				flowController.setUnityZoom();
				err = noErr;
				break;
				
			case kFITSUICommandMenuZoomIn:			
				flowController.incrementZoom( centerPoint );
				err = noErr;
				break;
				
			case kFITSUICommandMenuZoomOut:
				flowController.decrementZoom( centerPoint );
				err = noErr;
				break;
				
			case kFITSUICommandMenuCentrePreview:
				flowController.centerPreview();
				err = noErr;
				break;
		}
	}
	
	return err;
}

/**
 * Change the cursor to the current tool's cursor. A cursor may only
 * enter in a inactive state (i.e. no mouse button down).
 */
OSStatus MacPreviewView::onMouseEntered( EventRef event ) {
	dialog->enteredRegion( kFITSUIPreviewTrackingRegion );
	// Cursor may only enter in inactive state.
	if( globalSettingsModel.getPreviewEnabled() ) {
		dialog->setCursor( currentCursorInactive );
	}
	inPreview = true;	
	return noErr;
}
	
/**
 * Change cursor back to normal when mouse exits preview control.
 * The cursor is only changed if mouse not already has entered another
 * region.
 */
OSStatus MacPreviewView::onMouseExited( EventRef event ) {
	if( dialog->exitedRegion( kFITSUIPreviewTrackingRegion ) ) {
		if( globalSettingsModel.getPreviewEnabled() ) {
			dialog->setCursor( kFITSUICursorNormal );
		}
	}
	
	inPreview = false;
	return noErr;
}

/**
 * Test what part of a control will be hit by a given event. This function
 * set the kEventParamControlPart parameter of the event to the part hit.
 */
OSStatus MacPreviewView::onHitTest( EventRef event ) {
	::Point 		mouseLocation;
	ControlPartCode partCode;
	
	::GetEventParameter( event, ::kEventParamMouseLocation, typeQDPoint, NULL, sizeof(::Point), NULL, &mouseLocation );

	partCode = previewControl->getControlPart( &mouseLocation);
	
	::SetEventParameter( event, kEventParamControlPart, typeControlPartCode, sizeof(ControlPartCode), &partCode );
		
	return noErr;
}

/**
 * Track the mouse in the preview. That is, a mouse button has been pressed
 * in the preview control.
 */
OSStatus MacPreviewView::onTrack( EventRef event ) {
	if( !globalSettingsModel.getPreviewEnabled() ) {
		return noErr;
	}
		
	::Point	initialMouseLocation;
	::Point	mouseLocation;
	Char 	keyChanged;
	UInt32 	modifiers = MachO::GetCurrentEventKeyModifiers();

	FitsLiberator::Point p;
	
	// QuickDraw coord
	::GetEventParameter( event, ::kEventParamMouseLocation, typeQDPoint, NULL, sizeof (::Point), NULL, &initialMouseLocation );

	ToolTypeFunction	tool 			= toolModel.getCurrentFunction();
	MouseTrackingResult	trackingResult	= kMouseTrackingMouseDown;
	
	Rect r; 
	// QuickDraw coord
	previewControl->getBounds(&r); 
	Int previewHeight = r.bottom - r.top;
	
	MovementVector vec(0,0);
	
	// QuickDraw coord
	FitsLiberator::Rectangle& imageArea = model.getImageArea();
	
	// zoomArea.origin.x in Quartz coord
	if( imageArea.left > initialMouseLocation.h ) {
		zoomArea.origin.x = imageArea.left;	
	} else if( imageArea.right < initialMouseLocation.h ) {
		zoomArea.origin.x = imageArea.right;	
	} else {
		zoomArea.origin.x = initialMouseLocation.h;
	}
	
	Int lastPosX = initialMouseLocation.h;//= zoomArea.origin.x;
	
	Int lastPosY = initialMouseLocation.v;
	
	// zoomArea.origin.y in Quartz coord
	if( imageArea.top > initialMouseLocation.v ) {
		zoomArea.origin.y = QD2QUARTZ( previewHeight, imageArea.top );	
		lastPosY = imageArea.top;
	} else if( imageArea.bottom < initialMouseLocation.v ) {
		zoomArea.origin.y = QD2QUARTZ( previewHeight, imageArea.bottom );
		lastPosY = imageArea.bottom;
	} else {
		zoomArea.origin.y = QD2QUARTZ( previewHeight, initialMouseLocation.v);
		lastPosY = initialMouseLocation.v;
	}
	
	lastPosY = initialMouseLocation.v;
	
	p.x = lastPosX;
	p.y = lastPosY;
	
	// Switch to active cursor
	dialog->setCursor( currentCursorActive );
	
	if( tool == kFITSToolFunctionZoomOut ) {
		flowController.decrementZoom( p );
	}
	
	Int spaceBarUp 		= 0;
	Int spaceBarDown 	= 0;
	Bool keyHit     	= false;
	
	// Track mouse
	while( trackingResult != kMouseTrackingMouseUp ) {
		TrackMouseLocationAndKey( &mouseLocation, &trackingResult, &keyChanged );
		
		// QuickDraw coords
		dialog->translateScreenToControl( &mouseLocation, previewControl );
		
		if( trackingResult == kMouseTrackingMouseDragged ) {
		
			switch( tool ) {	
				case kFITSToolFunctionZoomIn:
					showZoom = true;
					
					if( imageArea.left > mouseLocation.h ) {
						zoomArea.size.width 	= imageArea.left - zoomArea.origin.x;	
					} else if( imageArea.right < mouseLocation.h ) {
						zoomArea.size.width 	= imageArea.right - zoomArea.origin.x;
					} else {
						zoomArea.size.width 	= mouseLocation.h - zoomArea.origin.x;
					}
					
					if( imageArea.top > mouseLocation.v ) {
						zoomArea.size.height 	= QD2QUARTZ( previewHeight, imageArea.top ) - zoomArea.origin.y;
					} else if( imageArea.bottom < mouseLocation.v ) {
						zoomArea.size.height 	= QD2QUARTZ( previewHeight, imageArea.bottom ) - zoomArea.origin.y;
					} else {
						zoomArea.size.height 	= QD2QUARTZ( previewHeight, mouseLocation.v ) - zoomArea.origin.y;
					}
					
					drawPreview();
					break;
				
				case kFITSToolFunctionMove:
					vec.x = mouseLocation.h - lastPosX;
					vec.y = mouseLocation.v - lastPosY;
					flowController.movePreview( vec );
                    previewControl->setPreviewImage(model.getPreviewImage());
					drawPreview();
					break;

				case kFITSToolFunctionPeakPicker:
				case kFITSToolFunctionBackgroundPicker:	
				case kFITSToolFunctionBlackLevelPicker:
				case kFITSToolFunctionWhiteLevelPicker:
					p.x = mouseLocation.h;
					p.y = mouseLocation.v;
				
					flowController.setCoordinates( p );
					break;
			}
			
			lastPosX = mouseLocation.h;
			lastPosY = mouseLocation.v;
		} else if( trackingResult == kMouseTrackingKeyModifiersChanged ) {
			// We need to catch option change events, or elese they aren't handled.
			modifiers = MachO::GetCurrentEventKeyModifiers();
		} else if ( trackingResult == kMouseTrackingKeyUp ) {
			if( keyChanged == ' ' ) {
				spaceBarUp++;
				keyHit = true;
			}
		} else if ( trackingResult == kMouseTrackingKeyDown ) {
			if( keyChanged == ' ' ) {
				spaceBarDown++;
				keyHit = true;
			}
		}
	}
	
	if( keyHit ) {
		if( spaceBarUp > spaceBarDown  ) {
			toolController.keyboardShortCutUp( keyChanged );
		} else {
			if( spaceBarDown != spaceBarUp ) {
				toolController.keyboardShortCut( keyChanged );	
			}
		}	
	}
		
	// Change tool function if option key has changed state.
	toolController.changeToolFunction( modifiers & optionKey );
	
	if( showZoom ) {
		showZoom = false;
		drawPreview();	
	}	
	
	p.x = lastPosX;
	p.y = lastPosY;

	// Apply tool		
	switch( tool ) {
		// case kFITSToolFunctionZoomOut: already handle before while statement
		
		case kFITSToolFunctionZoomIn:
			if( abs(initialMouseLocation.h - p.x) <= kFITSDragMarginX  && abs( initialMouseLocation.v - p.y ) <= kFITSDragMarginY ) {
				flowController.incrementZoom( p );
			} else {
				FitsLiberator::Rectangle zoomRect( initialMouseLocation.h, initialMouseLocation.v, p.x, p.y );
				flowController.zoomRectangle( zoomRect );
			}
			break;
			
		case kFITSToolEyedropperWhiteLevel:
			flowController.pickWhiteLevel( p );
			break;
		
		case kFITSToolEyedropperBlackLevel:
			flowController.pickBlackLevel( p );
			break;
		
		case kFITSToolEyedropperBackground:
			flowController.backgroundGuess( p );
			break;
		
		case kFITSToolEyedropperPeak:
			flowController.peakGuess( p );
			break;
	}
	
	// Switch to inactive cursor according to tool and modifiers if inside
	// control. 
	// Note: Normally onMouseExited will change to Normal cursor, if mouse is 
	// outside of control, which mean that we wouldn't require the if. But if
	// mouse is outside histogram, and we first set the cursor inactive, and 
	// afterwards normal cursor, we get some nasty flickering from the mouse.
	if( previewControl->getControlPart( &mouseLocation ) == kControlPreviewPart ) {
		dialog->setCursor( currentCursorInactive );	
	}
		
	return noErr;
}

/**
 * Draws the preview according to the model. Uses Quartz through
 * MachOFrameworkSupport.h to draw.
 *
 * @warn Be aware of coordinate system.
 */
OSStatus MacPreviewView::onDraw( EventRef event ) {
	CGContextRef	context;
	Rect			r;
	
	::GetEventParameter( event, kEventParamCGContextRef, typeCGContextRef, NULL, 
		sizeof (CGContextRef), NULL, &context );
	previewControl->getBounds( &r );
	
	// We want to use quartz coordinates instead of QuickDraw
	MachO::CGContextScaleCTM( context, 1.0, -1.0);
	MachO::CGContextTranslateCTM( context, 0, -(r.bottom - r.top) );
	
	
	previewControl->drawBackground( context );
	
	if( globalSettingsModel.getPreviewEnabled() ) {
		previewControl->drawPreviewImage( context );
	
		if( globalSettingsModel.getMarkUndefined() ) {
			previewControl->drawUndefinedMask( context );	
		}
		
		if( globalSettingsModel.getMarkWhiteClipping() ) {
			previewControl->drawWhiteMask( context );	
		}
		
		if( globalSettingsModel.getMarkBlackClipping() ) {
			previewControl->drawBlackMask( context );	
		}
			
		if( showZoom && ( ::abs( zoomArea.size.width ) > kFITSDragMarginX || ::abs( zoomArea.size.height ) > kFITSDragMarginY ) ) {
			previewControl->drawZoomArea( context, zoomArea );	
		}
		
		if( progress >= 0 ) {
            std::stringstream buf;
            std::string       text;
            
            buf << progressModel.Progress();
            buf << "%";
            text = buf.str();
            
			previewControl->drawThrobber( context, progress, text );
		}
	}
	
	return noErr;
}

/**
 * Receive notifications when tool type is updated, and sets the current cursor
 * according to this.  Changes the cursor if in preview.
 */
Void MacPreviewView::setCurrentTool( ToolTypeFunction tool ) {
	switch( tool ) {
		case kFITSToolFunctionMove:
			currentCursorInactive 	= kFITSUICursorMoveInactive;
			currentCursorActive 	= kFITSUICursorMoveActive; 
			break;
		
		case kFITSToolFunctionZoomIn:
			if( model.canIncrement() ) {
				currentCursorInactive 	= kFITSUICursorZoomIn;
				currentCursorActive 	= currentCursorInactive; 	
			} else {
				currentCursorInactive 	= kFITSUICursorZoomLimit;
				currentCursorActive 	= currentCursorInactive; 
			}
			break;
						
		case kFITSToolFunctionZoomOut:
			if( model.canDecrement() ) {
				currentCursorInactive 	= kFITSUICursorZoomOut;
				currentCursorActive 	= currentCursorInactive;  	
			} else {
				currentCursorInactive 	= kFITSUICursorZoomLimit;
				currentCursorActive 	= currentCursorInactive; 
			}
			break;
		
		case kFITSToolFunctionWhiteLevelPicker:
			currentCursorInactive 	= kFITSUICursorPickerWhite;
			currentCursorActive 	= currentCursorInactive; 
			break;
		
		case kFITSToolFunctionBlackLevelPicker:
			currentCursorInactive 	= kFITSUICursorPickerBlack;
			currentCursorActive 	= currentCursorInactive; 
			break;
		
		case kFITSToolFunctionBackgroundPicker:
			currentCursorInactive 	= kFITSUICursorPickerBackground;
			currentCursorActive 	= currentCursorInactive; 
			break;
			
		case kFITSToolFunctionPeakPicker:
			currentCursorInactive 	= kFITSUICursorPickerPeak;
			currentCursorActive 	= currentCursorInactive; 
			break;
	}

	if( inPreview ) {
		if( globalSettingsModel.getPreviewEnabled() ) {
			dialog->setCursor( currentCursorInactive );	
		}
	}
}

/**
 *
 */
Void MacPreviewView::drawPreview() {
    previewControl->setPreviewImage( model.getPreviewImage() );
	previewControl->drawControl();
}

/**
 *
 */
Void MacPreviewView::updateZooms( String* values, Int zSize ) {
	zoomPopupMenu->deleteAllItems();

	for( Int i = 0; i < zSize; i++ ) {
		if( model.getUnityZoomIndex() == i ) {
			zoomPopupMenu->addSeparator();
			zoomPopupMenu->addItem( values[i], (1 << 14), kFITSUICommandMenuZoom );
			zoomPopupMenu->addSeparator();
		} else if( i == zSize-1 ) {
			zoomPopupMenu->addSeparator();
			zoomPopupMenu->addItem( values[i], (1 << 14), kFITSUICommandMenuZoom );
		} else {
			zoomPopupMenu->addItem( values[i], (1 << 14), kFITSUICommandMenuZoom );
		}
	}
	
	// Take separators into account
	zoomPopup->setMaximumValue( zSize + 10 );
}

/**
 *
 */
Void MacPreviewView::setZoomCombo( Int selected ) {
	if( selected >= model.getUnityZoomIndex() ) {
		if( selected > model.getUnityZoomIndex() ) {
			selected++;
		}
		selected++;
	}

	// Must use 16 bit value, or else it won't work.
	zoomPopup->set16BitValue( selected + 1 );
	zoomPopup->drawControl();
	
	ToolTypeFunction tool = toolModel.getCurrentFunction();
	
	if( model.canIncrement() ) {
		if( tool == kFITSToolFunctionZoomIn ) {
			currentCursorInactive 	= kFITSUICursorZoomIn;
			currentCursorActive 	= currentCursorInactive; 	
		} 
		plusButton->enable();
	} else {
		if( tool == kFITSToolFunctionZoomIn ) {
			currentCursorInactive 	= kFITSUICursorZoomLimit;
			currentCursorActive 	= currentCursorInactive; 	
		} 
		plusButton->disable();
	}
	
	if( model.canDecrement() ) {
		if( tool == kFITSToolFunctionZoomOut ) {
			currentCursorInactive 	= kFITSUICursorZoomOut;
			currentCursorActive 	= currentCursorInactive; 	
		} 
		minusButton->enable();
	} else {
		if( tool == kFITSToolFunctionZoomOut ) {
			currentCursorInactive 	= kFITSUICursorZoomLimit;
			currentCursorActive 	= currentCursorInactive; 	
		} 
		minusButton->disable();
	}
}

/**
 *
 */
Void MacPreviewView::updateZoomComboState( Bool enabled ) {
	if( enabled ) {
		zoomPopup->enable();
		if( model.canIncrement() ) {
			plusButton->enable();	
		} else {
			plusButton->disable();
		}
		
		if( model.canDecrement() ) {
			minusButton->enable();	
		} else {
			minusButton->disable();
		}
	} else {
		zoomPopup->disable();
		plusButton->disable();
		minusButton->disable();
	}
}

void MacPreviewView::gotBusy() {
	mainMenu = ::AcquireRootMenu();
	UInt16 count = ::CountMenuItems(mainMenu);
	for(int i = 1; i <= count; ++i) {
		MenuRef ref;
		::GetMenuItemHierarchicalMenu(mainMenu, i, &ref);
		::DisableMenuItem(ref, 0);
	}
	
	ControlRef focus = NULL;
	OSErr res2 = ::GetKeyboardFocus( dialog->getWindow(), &focus );
	
	
	// Store keyboard focus so it can be restored afterwards
	OSErr res = ::GetKeyboardFocus( dialog->getWindow(), &tempFocusControl );
	if( res != noErr ) {
		tempFocusControl = NULL;
	}
	
	::DisableControl(::HIViewGetRoot(dialog->getWindow()));

	::InstallEventLoopTimer(::GetMainEventLoop(), 
		0.0, 0.050, 
		&MacPreviewView::TimerProc, reinterpret_cast<void*>(this),
		&this->timer);
}

void MacPreviewView::gotDone() {
	::RemoveEventLoopTimer(timer);
	progress = -1;
	
	::EnableControl(::HIViewGetRoot(dialog->getWindow()));
	
	previewControl->setPreviewImage( model.getPreviewImage() );
	drawPreview();
	
	if(mainMenu != NULL) {
		UInt16 count = ::CountMenuItems(mainMenu);
		for(int i = 1; i <= count; ++i) {
			MenuRef ref;
			::GetMenuItemHierarchicalMenu(mainMenu, i, &ref);
			::EnableMenuItem(ref, 0);
		}
		::ReleaseMenu(mainMenu);
		mainMenu = NULL;
	}
	
	// Give back keyboard focus to control
	if( tempFocusControl != NULL ) {
		::SetKeyboardFocus( dialog->getWindow(), tempFocusControl, kControlFocusNextPart );
		tempFocusControl = NULL;
	}
}

void MacPreviewView::TimerProc(EventLoopTimerRef inTimer, void* inUserData) {
	MacPreviewView* self = reinterpret_cast<MacPreviewView*>( inUserData );
	self->progress += 1;
	if( self->progress >= self->previewControl->getThrobberImageCount() ) {
		self->progress = 0;
	}
	self->drawPreview();
}
