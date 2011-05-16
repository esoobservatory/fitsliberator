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

#include "MacStretchView.h"
#include "Resources.h"

using namespace FitsLiberator::Mac;
using namespace FitsLiberator::Modelling;

struct AddStretchFunction {
	MenuRef menu;
	Int index;
	MenuCommand menuCommand;
	UInt8  menuModifiers;

	AddStretchFunction( MenuRef d )
	  : menu( d ) {
		this->index = 0;
		
		// Assumes we have only one menu item, with defined
		// modifiers and command id. We obtain its settings, and
		// use it as a template for the other menu items.
		
	
		::GetMenuItemCommandID( menu, 1, &menuCommand );
		::GetMenuItemModifiers( menu, 1, &menuModifiers );
		::DeleteMenuItem( menu, 1 );
	}

	Void operator()( StretchFunction stretchFunction, Int str ) {
		String s = FitsLiberator::Environment::getString( str );
        		
       	MenuItemIndex itemIndex;
		CFStringRef menuText = ::CFStringCreateWithCString( NULL, s.c_str(),  kCFStringEncodingMacRoman );
        
		if( noErr != ::AppendMenuItemTextWithCFString( menu, menuText, (1 << 14), menuCommand, &itemIndex ) ) {
			throw FitsLiberator::Exception( "Couldn't add menu items to stretch function popup menu." );
		}
        
   		::SetMenuItemModifiers( menu, itemIndex, menuModifiers );
		::CFRelease( menuText );
		
		index++;
	}
};

/**
 *
 *
 */
MacStretchView::MacStretchView( FitsDialog * dlog, ToolController& tctrl, StretchModel& m, FlowController& c, OptionsModel& opt  ) 
: StretchView( m, c, opt ), toolController( tctrl ) {	
	BundleFactory *factory = FitsLiberator::Environment::getBundleFactory();

	//-----------------------
	// Controls
	//-----------------------
	static const ::EventTypeSpec popupEventTypes[] = {
			{ kEventClassCommand, kEventCommandProcess },
	};

	guessBackgroundBevel 	= new BevelButtonControl( dlog, kFITSUIBevelButtonAutoBackground, 'IcAB' );
	guessPeakBevel 			= new BevelButtonControl( dlog, kFITSUIBevelButtonAutoPeak, 'IcAP' );
	stretchFunctionPopup 	= new BaseControl( dlog, kFITSUIPopupButtonFunction );
	resetButton				= new BaseControl( dlog, kFITSUIButtonReset );
	automaticScalingButton	= new BaseControl( dlog, kFITSUIButtonAutoScaling );
	applyValuesButton		= new BaseControl( dlog, kFITSUIButtonApplyValues );
	
	guessBackgroundBevel->installEventHandler( popupEventTypes, GetEventTypeCount(popupEventTypes), this );
	guessPeakBevel->installEventHandler( popupEventTypes, GetEventTypeCount(popupEventTypes), this );
	stretchFunctionPopup->installEventHandler( popupEventTypes, GetEventTypeCount( popupEventTypes ), this );
	resetButton->installEventHandler( popupEventTypes, GetEventTypeCount( popupEventTypes ), this );
	automaticScalingButton->installEventHandler( popupEventTypes, GetEventTypeCount( popupEventTypes ), this );
	applyValuesButton->installEventHandler( popupEventTypes, GetEventTypeCount( popupEventTypes ), this );
	
	if( opt.ApplyStretchDirectly() ) {
		applyValuesButton->disable();
	} else {
		applyValuesButton->enable();
	}
	
	guessBackgroundBevel->setIcon( CFSTR("iconUseBlackLevel.icns"), factory );
	guessPeakBevel->setIcon( CFSTR("iconUseWhiteLevel.icns"), factory );
	
	static const ::EventTypeSpec editTextEventTypes[] = {
			{ kEventClassControl, kEventControlSetFocusPart },
			{ kEventClassTextInput, kEventTextInputUnicodeForKeyEvent },
			{ kEventClassMouse, kEventMouseEntered },
			{ kEventClassMouse, kEventMouseExited }
	};

	static const Int editEventCount = GetEventTypeCount( editTextEventTypes );

	backgroundLevelEdit		= new NumericTextControl( dlog, kFITSUIEditTextBackground );
	scaleEdit				= new NumericTextControl( dlog, kFITSUIEditTextPeak );
	rescaleFactorEdit		= new NumericTextControl( dlog, kFITSUIComboScaledPeakLevel );
	
	backgroundLevelEdit->setPrecision( kFITSDefaultPrecision );
	scaleEdit->setPrecision( kFITSDefaultPrecision );
	rescaleFactorEdit->setPrecision( kFITSDefaultPrecision );
	
	backgroundLevelEdit->installEventHandler( editTextEventTypes, editEventCount, this );
	scaleEdit->installEventHandler( editTextEventTypes, editEventCount, this );
	rescaleFactorEdit->installEventHandler( editTextEventTypes, editEventCount, this );
	
	//-------------------------------------------
	// Insert stretch functions in popup control.
	//-------------------------------------------
		
	//Get menu
	MenuRef	menu;

	stretchFunctionPopup->getData( kControlMenuPart, kControlPopupButtonMenuRefTag, &menu, sizeof( menu ), NULL );
	
	if( menu == NULL ) {
		throw Exception( "Couldn't obtain reference to stretch function popup menu." );
	}
	
	// Initialize the function list
	AddStretchFunction f( menu );
	enumerateFunctions( f );
	
	stretchFunctionPopup->setMaximumValue( kFITSStretchNumberOfFunctions );
		
	//-----------------------
	// Mouse tracking regions
	//-----------------------
	// Install mouse tracking regions - this is to make the cursor change to IBeam when
	// hovering over the edit text fields. Carbon sometimes really sukcs ;-)
	MouseTrackingRegionID trackBackgroundID = { kFITSApplicationSignature, kFITSUIEditTextBackgroundRegion };
	MouseTrackingRegionID trackScaleID = { kFITSApplicationSignature, kFITSUIEditTextScaleRegion };
	
	Rect backgroundBounds, scaleBounds, addBounds, rescaleFactorBounds;
	
	backgroundLevelEdit->getBounds( &backgroundBounds );
	scaleEdit->getBounds( &scaleBounds );
	rescaleFactorEdit->getBounds( &rescaleFactorBounds );
	
	BaseControl tmpControl( dlog, kFITSUIFrameStretchFunction );
	tmpControl.getBounds( &addBounds );
	
	// Since these to edit text is in a frame we need move their rects
	::OffsetRect( &backgroundBounds, addBounds.left, addBounds.top );
	::OffsetRect( &scaleBounds, addBounds.left, addBounds.top );
	::OffsetRect( &rescaleFactorBounds, addBounds.left, addBounds.top );
	
	dlog->createMouseTrackingRegion( &backgroundBounds, trackBackgroundID, backgroundLevelEdit );
	dlog->createMouseTrackingRegion( &scaleBounds, trackScaleID, scaleEdit );
	dlog->createMouseTrackingRegion( &rescaleFactorBounds, trackScaleID, rescaleFactorEdit );
	
	//-------------------------
	// Setup allowed characters
	//-------------------------
	// Remember to change HistogramView also
	allowedCharacters.append( "0123456789eE" );
	allowedCharacters.append( Environment::getDecimalSeparator() );
	allowedCharacters.append( Environment::getNegativeSign() );
	allowedCharacters.append( Environment::getPositiveSign() );
	allowedCharacters.append( "\x08" ); // backspace
	allowedCharacters.append( "\x7F" ); // delete
	allowedCharacters.append( "\x1C" ); // left arrow
	allowedCharacters.append( "\x1D" ); // right arrow
	
	dialog = dlog;
}

/**
 * Destructor
 */
MacStretchView::~MacStretchView() {
	delete stretchFunctionPopup;
	delete backgroundLevelEdit;
	delete scaleEdit;
	delete resetButton;
	delete rescaleFactorEdit;
	delete guessBackgroundBevel;
	delete guessPeakBevel;
	delete automaticScalingButton;
	delete applyValuesButton;
}

/**
 *
 *
 */
OSStatus MacStretchView::onMouseEntered( EventRef event ) {
	::SetThemeCursor( kThemeIBeamCursor  );
	
	return noErr;
}

/**
 *
 *
 */
OSStatus MacStretchView::onMouseExited( EventRef event ) {
	::SetThemeCursor( kThemeArrowCursor  );
	
	return noErr;
}

/**
 *
 *
 */
Void MacStretchView::setStretchFunction( Int func ) {
	stretchFunctionPopup->setValue( func + 1 );
}

/**
 *
 *
 */
Void MacStretchView::setBackgroundLevel( Double value ) {
	backgroundLevelEdit->setDouble( value );
}

/**
 *
 *
 */
Void MacStretchView::setScale( Double value ) {
	// not used anymore. 
}


/**
 *
 *
 */
Void MacStretchView::setRescaleFactor( Double value ) {
	rescaleFactorEdit->setDouble( value );
}

/**
 *
 *
 */
Void MacStretchView::setPeakLevel( Double value ) {
	scaleEdit->setDouble( value );
}

/**
 *
 */
Void MacStretchView::setApplyStretchEnabled( Bool value ) {
	if( value ) {
		applyValuesButton->enable();
	} else {
		applyValuesButton->disable();
	}
}


/**
 * Command event handling
 *
 */
OSStatus MacStretchView::onCommand( HICommand *command ) {
	OSStatus result = eventNotHandledErr;

	switch( command->commandID ) {
		case kFITSUICommandSelectStretchFunction:
			onStretch( stretchFunctionPopup->getValue() - 1 );
			result = noErr;
			break;
			
		case kFITSUICommandReset:
			onReset();
			result = noErr;
			break;
		
		case kFITSUICommandAutomaticBackground:
			onAutoBackground();
			result = noErr;
			break;
			
		case kFITSUICommandAutomaticPeak:
			onAutoPeak();
			result = noErr;
			break;
			
		case kFITSUICommandAutomaticScaling:
			onAutoScale();
			result = noErr;
			break;
		case kFITSUICommandApplyValues:
			onBackgroundScalePeak( backgroundLevelEdit->getDouble(), scaleEdit->getDouble(), rescaleFactorEdit->getDouble() );
			result = noErr;
			break;
	}
	
	return result;
}

/**
 * Apply text fields values when loosing focus.
 */
OSStatus MacStretchView::onSetFocusPart( EventRef event ) {
        ControlRef ctrl;
        ControlRef focusCtrl;
        
        ::GetKeyboardFocus( backgroundLevelEdit->getDialog()->getWindow(), &focusCtrl );
        ::GetEventParameter( event, ::kEventParamDirectObject, typeControlRef, NULL, sizeof (ControlRef), NULL, &ctrl );
		
        if( ctrl == focusCtrl ) {
            if( ctrl == backgroundLevelEdit->getControl() ) {
				if( backgroundLevelEdit->getDouble() != model.getBackground() ) {
					onBackground( backgroundLevelEdit->getDouble() );
				}
				//backgroundLevelEdit->setDouble( model.getBackground() );
            } else if( ctrl == scaleEdit->getControl() ) {
				if( scaleEdit->getDouble() != model.getPeakLevel() ) {
					onPeak( scaleEdit->getDouble() );
				}
                //scaleEdit->setDouble(model.getPeakLevel());
            } else if( ctrl == rescaleFactorEdit->getControl() ) {
				if( rescaleFactorEdit->getDouble() != model.getRescaleFactor() ){
					onScale( rescaleFactorEdit->getDouble() );
				}
                //rescaleFactorEdit->setDouble(model.getRescaleFactor());
            }
        }
	
	// Let the standard event handler, handle the setting of focus.
	return eventNotHandledErr;
}

/**
 * Handle input in edit text controls. 
 * 
 * @WARN 	Special care must before editing this function. Be aware of keyboard 
 *			handling in HistogramView and ToolView and Cmd+menu command.
 *			Remember also to update code in StretchView which is nearly identical
 *			to this function.
 *	
 */
OSStatus MacStretchView::onUnicodeForKeyEvent( EventRef unicodeEvent ) {
	OSStatus result = eventNotHandledErr;

	EventRef event = NULL;
	::GetEventParameter( unicodeEvent, kEventParamTextInputSendKeyboardEvent, typeEventRef, NULL, sizeof(EventRef), NULL, &event );

	EventKind kind = ::GetEventKind( event );

	if( ( kind == kEventRawKeyDown ) || ( kind == kEventRawKeyRepeat ) ) {
		// We get events from two edit text fields so wee need
		// to find the control with the keyboard focus
		ControlRef ctrl;
		NumericTextControl* textControl;
	
		::GetKeyboardFocus( backgroundLevelEdit->getDialog()->getWindow(), &ctrl );
		
		if( ctrl == backgroundLevelEdit->getControl() ) {
			textControl = backgroundLevelEdit;
		} else if( ctrl == scaleEdit->getControl() ) {
			textControl = scaleEdit;
		} else if( ctrl == rescaleFactorEdit->getControl() ) {
			textControl = rescaleFactorEdit;
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
	
		switch( c ) {
			case kTabCharCode:	
			case kEnterCharCode:
			case kReturnCharCode:
				if( textControl == backgroundLevelEdit ) {
					if( backgroundLevelEdit->getDouble() != model.getBackground() ) {
						onBackground( backgroundLevelEdit->getDouble() );
					}
					result = (c == kTabCharCode) ? eventNotHandledErr : noErr;
				} else if( textControl == scaleEdit ) {
					if( scaleEdit->getDouble() != model.getPeakLevel() ) {
						onPeak( scaleEdit->getDouble() );
					}
					result = (c == kTabCharCode) ? eventNotHandledErr : noErr;
				} else if( textControl == rescaleFactorEdit ) {
					if( rescaleFactorEdit->getDouble() != model.getRescaleFactor() ){
						onScale( rescaleFactorEdit->getDouble() );
					}
					result = (c == kTabCharCode) ? eventNotHandledErr : noErr;
				}				
				break;
					
			case kUpArrowCharCode:
			case kDownArrowCharCode:
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