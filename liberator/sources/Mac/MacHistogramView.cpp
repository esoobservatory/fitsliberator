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
 * View for histogram
 * 
 * @todo Translate documentation.
 *
 * \section noteshist Notes for implementation
 * \subsection histbehav Histogram Behaviour
 *
 * The cursor is determined by a tool and the tools function (e.g. zoom in/zoom out). A tool
 * may have more functions associated with it. A cursur can be in an \c active or \c inactive 
 * state (e.g. open hand, closed hand for move tool). A tool can be changed by bevel buttons
 * or by hot keys. After a mouse down in the histogram, a cursor's function and tool can't 
 * change until after mouse up.
 *
 * The histogram can only be moved within it's total range (min max range).  
 *
 * \subsubsection movetool Move Tool
 * \li \b Cursor: No evens = open hand.
 * \li \b Key modifiers: no effect.
 * 
 * 1. Drag event
 * 1. Drag event (mouse moved while button down): flyt histogram og ændre grænser min og max for venstre og højre kant. Cursor ændres til lukket hand. On mouseup ændres cursor til åben hånd hvis den er i control, ellers ... (hvad hvis den er i previewcontrol).
 * 
 * Flyt sliders, hvis slider kommer uden for control, lav grå inaktiv. Tryk på slider medfører at slider følger springer ind i histogram område. Se mere i beskrivelse Histogram sliders behavior.
 * 
 * Zoom tool
 * ---------
 * Key modifier: ingen
 * Cursor: Forstørrelsesglas med plus (alternativt cross, eller cross med plus)
 * 
 * 1. Mouse down: start tracking
 * 	
 * 	1. Mouse up uden drag: zoom ind på område, centreret om mouse down/up location
 * 	2. Drag: marker område fra mouse down til ny position
 * 	3. Mouse up med drag: zoom ind på om råde fra mousedown til mouse up.
 * 
 * Key modifier: alt
 * Cursor: Forstørrelsesglas med minus (alternativt cross, eller cross med minus)
 * 
 * 1. Mouse down: zoom ud centreret omkring mouse down location.   
 * 
 * 
 * Background sampler tool
 * -----------------------
 * Key modifiers: ingen betydning
 * Cursor: Normal pil cursor ? vi må se hvad der virker bedst. skal indikiere der ikke kan gøre noget andet en contextual menu click.
 * 
 * White Level sampler tool
 * ------------------------
 * Cursor: cross
 * Key modifiers: ingen
 * 
 * 1. Mouse down: tracking start
 * 	1. Mouse up: set white level to mouse up location (apply values)
 * 	2. Mouse drag: flyt sort streg markør og slider til mouse location. ændre værdi i white level text edit 
 * 
 * (alt kan evt. bruges til at skifte mellem black/white level, så den springer tilbage når alt ikke er trykket ned mere. Man skal ske globalt, så det også fungere i preview).	
 * 	
 * Black Level sampler tool
 * ------------------------
 * Som white level.
 * 
 * 
 * Histogram sliders behavior
 * ==========================
 * Cursor: normal
 * 
 * 1. HitTest - find ud af om down er i white eller black eller ingen thumb.
 * 
 * 2. Mouse down (i knap pga. hittest)
 * 	1. drag: flyt slider og sort markør til mouse location. ændre edit text boks
 * 	2. mouse up: apply values
 */
 
#include "MacHistogramView.h"

using namespace FitsLiberator::Modelling;
using namespace FitsLiberator::Engine;
using namespace FitsLiberator::Mac;

/**
 * Initialize controls and view.
 */
MacHistogramView::MacHistogramView( FitsDialog* dlog, HistogramModel& model, ToolModel& toolModel, StatisticsModel& statModel, StretchModel& stretchModel, OptionsModel& optionsModel,
								   ToolController& toolController, HistogramController& controller, FlowController& flowController)
: histogramControl( NULL ), 
  sliderControl( NULL ), 
  histogramMinControl( NULL ), 
  histogramMaxControl( NULL ),
  contextualMenu( NULL ),
  dialog( dlog ),
  inHistogram( false ),
  isOptionPressed( false ),
  toolController(toolController),
  HistogramView( model, toolModel, statModel, stretchModel, optionsModel, controller, flowController ) {
  	
  	static const ::EventTypeSpec editTextEventTypes[] = {
			{ kEventClassControl, kEventControlSetFocusPart }, 
			{ kEventClassTextInput, kEventTextInputUnicodeForKeyEvent },
			//{ kEventClassKeyboard, kEventRawKeyDown },
			//{ kEventClassKeyboard, kEventRawKeyRepeat },
			{ kEventClassMouse, kEventMouseEntered },
			{ kEventClassMouse, kEventMouseExited }
	};
	
	static const ::EventTypeSpec stepperEventTypes[] = {
			{ kEventClassControl, kEventControlHit }
	};
	
	static const Int editEventCount 	= GetEventTypeCount( editTextEventTypes );
	static const Int stepperEventCount 	= GetEventTypeCount( editTextEventTypes );
  	
  	whiteLevelEdit			= new NumericTextControl( dlog, kFITSUIEditTextWhite );
	whiteLevelStepper		= new BaseControl( dlog, kFITSUIArrowsWhiteLevel );
	blackLevelEdit			= new NumericTextControl( dlog, kFITSUIEditTextBlack );
	blackLevelStepper		= new BaseControl( dlog, kFITSUIArrowsBlackLevel );
	
	whiteLevelEdit->setPrecision( kFITSDefaultPrecision );
	blackLevelEdit->setPrecision( kFITSDefaultPrecision );
	
	whiteLevelEdit->installEventHandler( editTextEventTypes, editEventCount, this );
	blackLevelEdit->installEventHandler( editTextEventTypes, editEventCount, this );
	
	// Install mouse tracking regions - this is to make the cursor change to IBeam when
	// hovering over the edit text fields. Carbon sometimes really sukcs ;-)
	MouseTrackingRegionID trackWhiteID = { kFITSApplicationSignature, kFITSUIEditTextWhiteRegion };
	MouseTrackingRegionID trackBlackID = { kFITSApplicationSignature, kFITSUIEditTextBlackRegion };
	
	Rect whiteBounds, blackBounds;
	
	whiteLevelEdit->getBounds( &whiteBounds );
	blackLevelEdit->getBounds( &blackBounds );
	
	whiteLevelTrackingRef = dlog->createMouseTrackingRegion( &whiteBounds, trackWhiteID, whiteLevelEdit );
	blackLevelTrackingRef = dlog->createMouseTrackingRegion( &blackBounds, trackBlackID, blackLevelEdit );
	
	whiteLevelStepper->installEventHandler( stepperEventTypes, stepperEventCount, this );
	blackLevelStepper->installEventHandler( stepperEventTypes, stepperEventCount, this );
  
  	static const EventTypeSpec menuEventTypes[] = { 
		{ kEventClassCommand, kEventCommandProcess }
  	};
  
  	contextualMenu = new BaseMenu();
  	contextualMenu->create( Environment::getBundleFactory(), CFSTR( kFITSNibFile ), CFSTR( kFITSNibHistogramMenu ) );
  	contextualMenu->installEventHandler( menuEventTypes, GetEventTypeCount( menuEventTypes ), this );
	
	//-----------------------
	// Init Histogram control
	//-----------------------
	static const EventTypeSpec histogramEventTypes[] = { 
		{ kEventClassControl, kEventControlContextualMenuClick },
		{ kEventClassControl, kEventControlDraw },
		{ kEventClassControl, kEventControlHitTest },
		{ kEventClassControl, kEventControlTrack },
		{ kEventClassMouse, kEventMouseEntered },
		{ kEventClassMouse, kEventMouseExited },
	};

	histogramControl = new HistogramControl( dialog, kFITSUICustomHistogram );
	histogramControl->getBounds( &histogramBounds );
	histogramControl->installEventHandler( histogramEventTypes, GetEventTypeCount(histogramEventTypes), this );
	histogramControl->setContextualMenu( contextualMenu );
	
	MouseTrackingRegionID trackID = { kFITSApplicationSignature, kFITSUIHistogramTrackingRegion };
	histogramTrackingRef = dialog->createMouseTrackingRegion( &histogramBounds, trackID, histogramControl );

	// Subtract room for the border.	
	model.setHistogramSize( Size( histogramBounds.right - histogramBounds.left - 2, histogramBounds.bottom - histogramBounds.top - 2 - 3 ) );
	
	//-----------------------
	// Init slider control
	//-----------------------
	static const EventTypeSpec sliderEventTypes[] = { 
		{ kEventClassControl, kEventControlDraw },
		{ kEventClassControl, kEventControlHitTest },
		{ kEventClassControl, kEventControlTrack },
		{ kEventClassControl, kEventControlDeactivate },
		{ kEventClassControl, kEventControlActivate }
	};
	
	sliderControl = new HistogramSliderControl( dialog, kFITSUICustomHistogramSlider );
	sliderControl->installEventHandler( sliderEventTypes, GetEventTypeCount(sliderEventTypes), this );
	
	// Min/Max fields
	histogramMinControl = new NumericTextControl( dialog, kFITSUITextHistogramMin );
	histogramMinControl->setPrecision(kFITSDefaultPrecision);
	histogramMaxControl = new NumericTextControl( dialog, kFITSUITextHistogramMax );
	histogramMinControl->setPrecision(kFITSDefaultPrecision);
		
	maxValue 	  = histogramBounds.right - histogramBounds.left - 2;	
	minValue 	  = 0;
	blackLevelPos = minValue;
	whiteLevelPos = maxValue-1;
	zoomStart 	  = minValue;
	zoomEnd		  = zoomStart;
	showZoom 	  = false;
	
	// Remember to change StretchView also
	allowedCharacters.append( "0123456789eE" );
	allowedCharacters.append( Environment::getDecimalSeparator() );
	allowedCharacters.append( Environment::getNegativeSign() );
	allowedCharacters.append( Environment::getPositiveSign() );
	allowedCharacters.append( "\x08" ); // backspace
	allowedCharacters.append( "\x7F" ); // delete
	allowedCharacters.append( "\x1C" ); // left arrow
	allowedCharacters.append( "\x1D" ); // right arrow
}

/**
 * Destroy controls in view.
 */
MacHistogramView::~MacHistogramView() {
	if( histogramControl != NULL ) 		{ delete histogramControl; }
	if( sliderControl != NULL ) 		{ delete sliderControl; }
	if( histogramMinControl != NULL ) 	{ delete histogramMinControl; }
	if( histogramMaxControl != NULL ) 	{ delete histogramMaxControl; }
	if( contextualMenu != NULL ) 		{ delete contextualMenu; }
	delete whiteLevelEdit;
	delete whiteLevelStepper;
	delete blackLevelEdit;
	delete blackLevelStepper;
}

/**
 * Handle menu command clicks.
 */
OSStatus MacHistogramView::onCommand( HICommand* command ) {
	OSStatus err = eventNotHandledErr;
	
	switch( command->commandID ) {
		case kFITSUICommandMenuShowAll:
			flowController.showAllHistogram();
			err = noErr;
			break;
			
		case kFITSUICommandMenuHistogramZoomIn:
			flowController.incrementHistogramZoom( (histogramBounds.right - histogramBounds.left - 2)/2 );
			err = noErr;
			break;
			
		case kFITSUICommandMenuHistogramZoomOut:
			flowController.decrementHistogramZoom( (histogramBounds.right - histogramBounds.left - 2)/2 );
			err = noErr;
			break;			
	}
	
	return err;
}

/**
 * Handle a contextual menu click in the histogram. Change the cursor accordingly.
 */
OSStatus MacHistogramView::onContextualMenuClick( EventRef event ) {
	ControlRef ctrl;
	
	::GetEventParameter( event, ::kEventParamDirectObject, typeControlRef, NULL, sizeof (ControlRef), NULL, &ctrl );

	if( ( ctrl == histogramControl->getControl() ) && (histogramControl->getContextualMenu() != NULL ) ) {
		::Point mouseLocation;
		
		dialog->setCursor( kFITSUICursorNormal );
    
    	::GetGlobalMouse( &mouseLocation );
    	::PopUpMenuSelect( histogramControl->getContextualMenu()->getMenu(), mouseLocation.v, mouseLocation.h, 0 );
    	::GetGlobalMouse( &mouseLocation );

    	dialog->translateScreenToControl( &mouseLocation, histogramControl );
    	
    	// onMouseExited takes care of the cursor, if it's not in the control.
    	if( kControlHistogramPart == histogramControl->getControlPart( &mouseLocation ) ) {
    		dialog->setCursor( currentCursorInactive );
    	}
    	
		return noErr;
    } else {
    	return eventNotHandledErr;	
    }
}

/**
 * Change the cursor to the current tool's cursor. A cursor may only
 * enter in a inactive state (i.e. no mouse button down).
 */
OSStatus MacHistogramView::onMouseEntered( EventRef event ) {
	MouseTrackingRef trackRef;
	
	GetEventParameter( event, ::kEventParamMouseTrackingRef, typeMouseTrackingRef, NULL, sizeof( MouseTrackingRef ), NULL, &trackRef );
	
	if( trackRef == histogramTrackingRef ) {
		dialog->enteredRegion( kFITSUIHistogramTrackingRegion );
		// Cursor may only enter in inactive state.
		dialog->setCursor( currentCursorInactive );
		inHistogram = true;	
	} else if( trackRef == blackLevelTrackingRef || trackRef == whiteLevelTrackingRef ) {
		::SetThemeCursor( kThemeIBeamCursor  );
	}
	
	return noErr;
}

/**
 * Change cursor back to normal when mouse exits histogram control.
 * The cursor is only changed if mouse not already has entered another
 * region.
 */
OSStatus MacHistogramView::onMouseExited( EventRef event ) {
	MouseTrackingRef trackRef;
	
	GetEventParameter( event, ::kEventParamMouseTrackingRef, typeMouseTrackingRef, NULL, sizeof( MouseTrackingRef ), NULL, &trackRef );
	
	if( trackRef == histogramTrackingRef ) {
		if( dialog->exitedRegion( kFITSUIHistogramTrackingRegion ) ) {
			dialog->setCursor( kFITSUICursorNormal );
		}
	
		inHistogram = false;
	} else if( trackRef == blackLevelTrackingRef || trackRef == whiteLevelTrackingRef ) {
		::SetThemeCursor( kThemeArrowCursor  );
	}
	
	return noErr;	
}

/**
 *
 *
 */
OSStatus MacHistogramView::onHit( EventRef event ) {
	ControlRef 		ctrl;
	UInt32 			modifiers;
	ControlPartCode	part;
	
	::GetEventParameter( event, ::kEventParamDirectObject, typeControlRef, NULL, sizeof( ControlRef ), NULL, &ctrl );
	::GetEventParameter( event, ::kEventParamKeyModifiers, typeUInt32, NULL, sizeof( UInt32 ), NULL, &modifiers );
	::GetEventParameter( event, ::kEventParamControlPart, typeControlPartCode, NULL, sizeof( ControlPartCode ), NULL, &part );
	
	const Bool isUp 		= ( part == kControlUpButtonPart );
	const Bool isShiftDown 	= ( (modifiers & shiftKey) != 0 );

	if( ctrl == whiteLevelStepper->getControl() ) {
		flowController.incrementWhiteLevel( isUp, isShiftDown );
		return noErr;
	} else if( ctrl == blackLevelStepper->getControl() ) {
		flowController.incrementBlackLevel( isUp, isShiftDown );
		return noErr;
	} 
	
	return eventNotHandledErr;
}

/**
 *
 *
 */
OSStatus MacHistogramView::onSetFocusPart( EventRef event ) {
	ControlRef ctrl;
	ControlRef focusCtrl;
	
	::GetKeyboardFocus( whiteLevelEdit->getDialog()->getWindow(), &focusCtrl );
	::GetEventParameter( event, ::kEventParamDirectObject, typeControlRef, NULL, sizeof (ControlRef), NULL, &ctrl );
	
	if( ctrl == focusCtrl ) {
		if( ctrl == whiteLevelEdit->getControl() ) {
            whiteLevelEdit->setDouble(model.getWhiteLevel());
		} else if( ctrl == blackLevelEdit->getControl() ) {
            blackLevelEdit->setDouble(model.getBlackLevel());
		}
	} 
	
	// Let the standard event handler, handle the setting of focus.
	return eventNotHandledErr;
}

/**
 * Handle input in edit text controls. 
 * 
 * @WARN 	Special care must before editing this function. Be aware of keyboard 
 *			handling in StrecthView and ToolView and Cmd+menu command.
 *			Remember also to update code in StretchView which is nearly identical
 *			to this function.
 *	
 */
OSStatus MacHistogramView::onUnicodeForKeyEvent( EventRef unicodeEvent ) {
	OSStatus result = eventNotHandledErr;

	EventRef event = NULL;
	::GetEventParameter( unicodeEvent, kEventParamTextInputSendKeyboardEvent, typeEventRef, NULL, sizeof(EventRef), NULL, &event );

	EventKind kind = ::GetEventKind( event );

	if( ( kind == kEventRawKeyDown ) || ( kind == kEventRawKeyRepeat ) ) {
		// We get events from two edit text fields so wee need
		// to find the control with the keyboard focus
		ControlRef ctrl;
		NumericTextControl* textControl;
	
		::GetKeyboardFocus( whiteLevelEdit->getDialog()->getWindow(), &ctrl );
		
		// We only receive events when text fields have focus. 
		if( ctrl == whiteLevelEdit->getControl() ) {
			textControl = whiteLevelEdit;
		} else if( ctrl == blackLevelEdit->getControl() ) {
			textControl = blackLevelEdit;
		}	
	
		// Filter for keys which must invoke commands:
		// Enter/Return - apply the value.
		// Up/down - increment/decrement value and take modifiers into account.
		Char 	c;
		UInt32 	modifiers;
	
		::GetEventParameter( event, ::kEventParamKeyMacCharCodes, typeChar, NULL, sizeof (Char), NULL, &c );
		::GetEventParameter( event, ::kEventParamKeyModifiers, typeUInt32, NULL, sizeof( UInt32 ), NULL, &modifiers );
	
		// Stop right here if were about to process a command key.
		// The standard event handler will take care of this.
		if( modifiers & cmdKey ) {		
			return eventNotHandledErr;
		}
		
		const Bool isUp			= ( kUpArrowCharCode == c );
		const Bool isShiftDown 	= ( (modifiers & shiftKey) != 0 );
	
		switch( c ) {
			case kEnterCharCode:
			case kReturnCharCode:
				if( textControl == whiteLevelEdit ) {
					flowController.setWhiteLevel( whiteLevelEdit->getDouble() );
					result = noErr;
				} else if( textControl == blackLevelEdit ) {
					flowController.setBlackLevel( blackLevelEdit->getDouble() );
					result = noErr;
				}
				break;
					
			case kUpArrowCharCode:
			case kDownArrowCharCode:
				if( textControl == whiteLevelEdit ) {
					flowController.incrementWhiteLevel( isUp, isShiftDown );
					result = noErr;
				} else if( textControl == blackLevelEdit ) {
					flowController.incrementBlackLevel( isUp, isShiftDown );
					result = noErr;
				}
				break;
			
			case kTabCharCode:
			case kEscapeCharCode:
				//passthrough
				break;
				
			default:
				if( toolController.keyboardShortCut( c ) ) {
					// Speacial case: KeyUp will be swallowed by the text field
					// if not keyboard focus is cleared.
					if( c == kSpaceCharCode ) {
						dialog->clearKeyboardFocus();	
					}

					result = noErr;
				} else if ( !(allowedCharacters.find_first_of( c ) < allowedCharacters.length())  ) {
					result = noErr;					
				}
				break;
		}
	} 
	
	return result;
}

/**
 * Test what part of a control will be hit by a given event. This function
 * set the kEventParamControlPart parameter of the event to the part hit.
 */
OSStatus MacHistogramView::onHitTest( EventRef event ) {
	ControlRef		ctrl;
	::Point 		mouseLocation;
	ControlPartCode partCode;
	
	::GetEventParameter( event, ::kEventParamDirectObject, typeControlRef, NULL, sizeof(ControlRef), NULL, &ctrl );
	::GetEventParameter( event, ::kEventParamMouseLocation, typeQDPoint, NULL, sizeof(::Point), NULL, &mouseLocation );

	if( ctrl == histogramControl->getControl() ) {
		partCode = histogramControl->getControlPart( &mouseLocation);
		
		// Must be set to receive kEventControlTrack
		::SetEventParameter( event, kEventParamControlPart, typeControlPartCode, sizeof(ControlPartCode), &partCode );
		
		return noErr;
	} else if ( ctrl == sliderControl->getControl() ) {
		partCode = sliderControl->getControlPart( &mouseLocation );
		
		// Must be set to receive kEventControlTrack		
		::SetEventParameter( event, kEventParamControlPart, typeControlPartCode, sizeof(ControlPartCode), &partCode );
		
		return noErr;
	} else {
		return eventNotHandledErr;
	}
}

/**
 * Dispatch event to either onTrackHistogram() or onTrackSlider depending on
 * who sent the event.
 */
OSStatus MacHistogramView::onTrack( EventRef event ) {
	ControlRef 			ctrl;
	
	::GetEventParameter( event, ::kEventParamDirectObject, typeControlRef, NULL, sizeof (ControlRef), NULL, &ctrl );
			
	if( ctrl == histogramControl->getControl() ) {
		return onTrackHistogram( event );
	} else if ( ctrl == sliderControl->getControl() ) {
		return onTrackSlider( event );
	} else {
		return eventNotHandledErr;
	}
}

/**
 * Track the mouse in the histogram. That is, a mouse button has been pressed
 * in the histogram control.
 */
OSStatus MacHistogramView::onTrackHistogram( EventRef event ) {
	::Point	initialMouseLocation;
	::Point	mouseLocation;
	UInt32 	modifiers;
	Char 	keyChanged;
	
	::GetEventParameter( event, ::kEventParamMouseLocation, typeQDPoint, NULL, sizeof (::Point), NULL, &initialMouseLocation );
	::GetEventParameter( event, ::kEventParamKeyModifiers, typeUInt32, NULL, sizeof (UInt32), NULL, &modifiers );

	ToolTypeFunction	toolFunction	= toolModel.getCurrentFunction();
	MouseTrackingResult	trackingResult	= kMouseTrackingMouseDown;
	
	// Background eyedropper has no effect on histogram
	if( toolFunction == kFITSToolFunctionBackgroundPicker || toolFunction == kFITSToolFunctionPeakPicker ) { 
		return eventNotHandledErr;
	}

	// First move slider to cursor position.  -1 comes from borderSize in histogramControl.
	switch( toolFunction ) {
		case kFITSToolFunctionBlackLevelPicker:
			flowController.moveBlackLevelSlider( initialMouseLocation.h - 1 );
			break;
		
		case kFITSToolFunctionWhiteLevelPicker:
			flowController.moveWhiteLevelSlider( initialMouseLocation.h - 1 );
			break;
		
		case kFITSToolFunctionZoomIn:	
		case kFITSToolFunctionZoomOut:
			zoomStart 	= initialMouseLocation.h - 1;
			zoomEnd 	= zoomStart;
			break;
	}
	
	// Switch to active cursor
	dialog->setCursor( currentCursorActive );
	
	Int lastPos = initialMouseLocation.h;
	Int delta = 0;
	Int spaceBarUp 		= 0;
	Int spaceBarDown 	= 0;
	Bool keyHit     	= false;
	Int  zoomMax            = 0;
        Rect bounds;

	histogramControl->getBounds(&bounds);
	zoomMax = bounds.right - bounds.left - 2;

	// Track mouse
	while( trackingResult != kMouseTrackingMouseUp && toolFunction != kFITSToolFunctionZoomOut ) {
		TrackMouseLocationAndKey( &mouseLocation, &trackingResult, &keyChanged );
		
		dialog->translateScreenToControl( &mouseLocation, histogramControl );

		if( trackingResult == kMouseTrackingMouseDragged ) {
			delta = mouseLocation.h - lastPos;
		
			switch( toolFunction ) {
				case kFITSToolFunctionZoomIn:
					showZoom 	= true;
					zoomEnd = zoomStart + mouseLocation.h - initialMouseLocation.h;
					// Remember to do bounds checking on this
					if( zoomEnd < 0) zoomEnd = 0;
					if( zoomEnd > zoomMax) zoomEnd = zoomMax;
					histogramControl->drawControl();
					break;
				
				case kFITSToolFunctionMove:
					flowController.moveHistogram( delta );
					break;
					
				case kFITSToolFunctionBlackLevelPicker:
					flowController.moveBlackLevelSlider( mouseLocation.h - 1 );
					break;
					
				case kFITSToolFunctionWhiteLevelPicker:
					flowController.moveWhiteLevelSlider( mouseLocation.h - 1 );
					break;
			}
		} else if( trackingResult == kMouseTrackingKeyModifiersChanged ) {
			modifiers = MachO::GetCurrentEventKeyModifiers();
		}  else if ( trackingResult == kMouseTrackingKeyUp ) {
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
		
		lastPos	= mouseLocation.h;
	}
	
	if( keyHit ) {
		if( spaceBarUp > spaceBarDown ) {
			toolController.keyboardShortCutUp( keyChanged );
		} else {
			if( spaceBarDown != spaceBarUp ) {
				toolController.keyboardShortCut( keyChanged );	
			}
		}	
	}

	// Change tool function if option key has changed state.
	toolController.changeToolFunction( modifiers & optionKey );
	
	// Remove the select area from histogram
	if( showZoom ) {
		showZoom = false;
		histogramControl->drawControl();
	}
	
	
	// Switch to inactive cursor according to tool and modifiers if inside
	// control. 
	// Note: Normally onMouseExited will change to Normal cursor, if mouse is 
	// outside of control, which mean that we wouldn't require the if. But if
	// mouse is outside histogram, and we first set the cursor inactive, and 
	// afterwards normal cursor, we get some nasty flickering from the mouse.
	if( histogramControl->getControlPart( &mouseLocation) == kControlHistogramPart ) {
		dialog->setCursor( currentCursorInactive );	
	}

	// Apply tool		
	switch( toolFunction ) {
		case kFITSToolFunctionZoomOut:
			flowController.decrementHistogramZoom( zoomStart );
			break;
	
		case kFITSToolFunctionZoomIn:
			if( zoomStart == zoomEnd ) {
				flowController.incrementHistogramZoom( zoomStart );	
			} else {
				flowController.showRange( FitsMath::minimum(zoomStart, zoomEnd), FitsMath::maximum(zoomStart, zoomEnd) );
			}
			break;
			
		case kFITSToolFunctionBlackLevelPicker:	
		case kFITSToolFunctionWhiteLevelPicker:
			flowController.applyLevels();
			break;
	}
		
	return noErr;
}

/**
 *  Track the mouse in the slider. That is, a mouse button has been pressed
 * in the slider control.
 */
OSStatus MacHistogramView::onTrackSlider( EventRef event ) {
	::Point	initialMouseLocation;
	::Point	mouseLocation;
	Char    keyChanged;
	
	::GetEventParameter( event, ::kEventParamMouseLocation, typeQDPoint, NULL, sizeof (::Point), NULL, &initialMouseLocation );

	ControlPartCode		partCode		= sliderControl->getControlPart( &initialMouseLocation );
	MouseTrackingResult	trackingResult	= kMouseTrackingMouseDown;
	
	// First move slider to cursor position. -4 comes from margin in sliderControl.
	switch( partCode ) {
		case kControlSliderWhiteLevelThumbPart:
			flowController.moveWhiteLevelSlider( initialMouseLocation.h - 4 );
			break;
			
		case kControlSliderBlackLevelThumbPart:
			
			flowController.moveBlackLevelSlider( initialMouseLocation.h - 4 );
			break;
	}
	
	sliderControl->drawControl();
	histogramControl->drawControl();

	while( trackingResult != kMouseTrackingMouseUp ) {
		TrackMouseLocationAndKey( &mouseLocation, &trackingResult, &keyChanged );
		
		// Translate coordiantes 
		dialog->translateScreenToControl( &mouseLocation, sliderControl );
				
		if( trackingResult == kMouseTrackingMouseDragged ) {
			switch( partCode ) {
				case kControlSliderWhiteLevelThumbPart:
					flowController.moveWhiteLevelSlider( mouseLocation.h - 4 );					
					break;
					
				case kControlSliderBlackLevelThumbPart:
					flowController.moveBlackLevelSlider( mouseLocation.h - 4 );
					break;
			}
		}
	}
	
	// apply values, since white/black level may be invisible currently
	flowController.applyLevels();
			
	return noErr;
}

/**
 * Draws the histogram or slider according to the model. Uses Quartz through
 * MachOFrameworkSupport.h to draw.
 *
 * @warn Be aware of coordinate system.
 */
OSStatus MacHistogramView::onDraw( EventRef event ) {
	ControlRef 		ctrl;
	CGContextRef	context;
	Rect			r;
	
	::GetEventParameter( event, kEventParamDirectObject, typeControlRef, NULL, sizeof (ControlRef), NULL, &ctrl );
	::GetEventParameter( event, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof (CGContextRef), NULL, &context );
	
	::GetControlBounds( ctrl, &r );
	
	// We want to use quartz coordinates instead of QuickDraw
	MachO::CGContextScaleCTM( context, 1.0, -1.0);
	MachO::CGContextTranslateCTM( context, 0, -(r.bottom - r.top) );
	
	if( ctrl == histogramControl->getControl() ) {
		return onDrawHistogram( context );
	} else if ( ctrl == sliderControl->getControl() ) {
		return onDrawSlider( context );
	} else {
		return eventNotHandledErr;
	}
}

/**
 * Draws the histogram according to the model. Uses Quartz through
 * MachOFrameworkSupport.h to draw.
 *
 * @warn Be aware of coordinate system.
 */
OSStatus MacHistogramView::onDrawHistogram( CGContextRef context ) {
	histogramControl->drawBackground( context );

	if( showZoom ) {
		histogramControl->drawSelectArea( context, zoomStart, zoomEnd );
	}
	
	histogramControl->drawHistogramBins( context, model.getEndBins() );

	for(int i = 0; i < MarkerCount; ++i) {
		histogramControl->drawMarker( context, markerPositions[i], markerColors[i] );
	}    
    
	histogramControl->drawBlackLevelMarker( context, model.getBlackSliderPos() );
	histogramControl->drawWhiteLevelMarker( context, model.getWhiteSliderPos() );

	return noErr;
}


/**
 * Draws the slider according to the model. Uses Quartz through
 * MachOFrameworkSupport.h to draw.
 *
 * @warn Be aware of coordinate system.
 */
OSStatus MacHistogramView::onDrawSlider( CGContextRef context ) {
	sliderControl->drawBackground( context );
	sliderControl->drawBlackLevelThumb( context, model.getBlackSliderPos() );	
	sliderControl->drawWhiteLevelThumb( context, model.getWhiteSliderPos() );
	return noErr;
}

/**
 * Receive notifications when tool type is updated, and sets the current cursor
 * according to this.  Changes the cursor if in histogram.
 */
Void MacHistogramView::setCurrentToolFunction( ToolTypeFunction tool ) {
	switch( tool ) {
		case kFITSToolFunctionMove:
			currentCursorInactive 	= kFITSUICursorMoveInactive;
			currentCursorActive 	= kFITSUICursorMoveActive; 
			break;
		
		case kFITSToolFunctionZoomIn:
			currentCursorInactive 	= kFITSUICursorZoomIn;
			currentCursorActive 	= currentCursorInactive; 
			break;
			
		case kFITSToolFunctionZoomOut:
			currentCursorInactive 	= kFITSUICursorZoomOut;
			currentCursorActive 	= currentCursorInactive; 
			break;
		
		case kFITSToolFunctionWhiteLevelPicker:
			currentCursorInactive 	= kFITSUICursorPickerWhite;
			currentCursorActive 	= currentCursorInactive; 
			break;
		
		case kFITSToolFunctionBlackLevelPicker:
			currentCursorInactive 	= kFITSUICursorPickerBlack;
			currentCursorActive 	= currentCursorInactive; 
			break;
		
		// Can't operate on histogram.
		case kFITSToolFunctionBackgroundPicker:
			currentCursorInactive 	= kFITSUICursorNormal; 
			currentCursorActive 	= currentCursorInactive; 
			break;
			
		case kFITSToolFunctionPeakPicker:
			currentCursorInactive 	= kFITSUICursorNormal; 
			currentCursorActive 	= currentCursorInactive; 
			break;
	}

	if( inHistogram ) {
		dialog->setCursor( currentCursorInactive );	
	}
}


/**
 *
 */
Void MacHistogramView::drawHistogram() {
	histogramControl->drawControl();
	sliderControl->drawControl();
}

/**
 *
 */
Void MacHistogramView::setHistogramMin( Double val ) {
	histogramMinControl->setDouble( val );
}

/**
 *
 */
Void MacHistogramView::setHistogramMax( Double val ) {
	histogramMaxControl->setDouble( val );
}

/**
 *
 *
 */
Void MacHistogramView::setWhiteLevel( Double value ) {
	whiteLevelEdit->setDouble( value );
}

/**
 *
 *
 */
Void MacHistogramView::setBlackLevel( Double value ) {
	blackLevelEdit->setDouble( value );
}
