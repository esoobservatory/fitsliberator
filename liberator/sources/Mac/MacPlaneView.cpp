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

#include "MacPlaneView.h"

using namespace FitsLiberator::Mac;
using namespace FitsLiberator::Modelling;

/**
 *
 */
MacPlaneView::MacPlaneView( BaseDialog * dlog, PlaneModel & m, FlowController & c, PlaneController& p )
: PlaneView( m, c, p ) {
	static const EventTypeSpec eventTypes[] = { 
		{ kEventClassCommand, kEventCommandProcess }
	};
	
	static const Int eventCount = GetEventTypeCount( eventTypes );
	
	radioBitDepth			= new RadioGroupControl( dlog, kFITSUIRadioChannels );
	radioUndefined			= new RadioGroupControl( dlog, kFITSUIRadioTransparent );
	widthControl			= new UnitsNumericTextControl( dlog, kFITSUITextWidth );
	heightControl			= new UnitsNumericTextControl( dlog, kFITSUITextHeight );
	planeSelectorControl	= new BaseControl( dlog, kFITSUIPopupButtonPlane );
	flipCheckbox			= new BaseControl( dlog, kFITSUICheckboxFlip );
	
	widthControl->setUnits( kFITSUIUnitsPixels );
	widthControl->setPrecision( 0 );
	heightControl->setUnits( kFITSUIUnitsPixels );
	heightControl->setPrecision( 0 );
	
	radioBitDepth->installEventHandler( eventTypes, eventCount, this );
	radioUndefined->installEventHandler( eventTypes, eventCount, this );
	planeSelectorControl->installEventHandler( eventTypes, eventCount, this );
	flipCheckbox->installEventHandler( eventTypes, eventCount, this );
	
	//Get menu
	MenuRef	menu;

	planeSelectorControl->getData( kControlMenuPart, kControlPopupButtonMenuRefTag, &menu, sizeof( menu ), NULL );
	
	if( menu == NULL ) {
		throw Exception( "Couldn't obtain reference to plane menu." );
	}
	
	// Assumes we have only one menu item, with defined
	// modifiers and command id. We obtain its settings, and
	// use it as a template for the other menu items.
	MenuCommand menuCommand;
	UInt8 		menuModifiers;
	
	::GetMenuItemCommandID( menu, 1, &menuCommand );
	::GetMenuItemModifiers( menu, 1, &menuModifiers );
	::DeleteMenuItem( menu, 1 );
	
    CFStringRef		menuText;
    MenuItemIndex	itemIndex;
    
    Int entryCount = getEntryCount();
    
    for( Int i = 0; i < entryCount; i++ ) {
		String s = getEntry( i );
        
		menuText = CFStringCreateWithCString( NULL, s.c_str(),  kCFStringEncodingMacRoman );
        
		if( noErr != ::AppendMenuItemTextWithCFString( menu, menuText, (1 << 14), menuCommand, &itemIndex ) ) {
			throw Exception( "Couldn't add menu items to plane selector." );
		}
        
		::SetMenuItemModifiers( menu, itemIndex, menuModifiers );
		//::SetMenuItemCommandKey( menu, itemIndex, false, '2' );
		::CFRelease( menuText );
    }
    
    // Make sure we can select the menu items.
    planeSelectorControl->setMaximumValue( entryCount );
    
    

}

/**
 *
 */
MacPlaneView::~MacPlaneView() {
	delete radioBitDepth;
	delete radioUndefined;
	delete widthControl;
	delete heightControl;
	delete planeSelectorControl;
	delete flipCheckbox;
}

/**
 *
 */
OSStatus MacPlaneView::onCommand( HICommand *command ) {
	Int selectedIndex = radioBitDepth->getValue() - 1;	
	switch( command->commandID ) {
		case kFITSUICommandToggleBitDepth:
			switch( selectedIndex ) {
				case channel8:
					planeController.setBitDepth( channel8 );
					break;
				case channel16:
					planeController.setBitDepth( channel16 );
					break;
				case channel32:
					planeController.setBitDepth( channel32 );
					break;
			}
			
			return noErr;
			break;
		
		case kFITSUICommandToggleUndefined:
			planeController.toggleUndefinedOption();
			return noErr;
			break;
			
		case kFITSUICommandToggleFlip:
			flowController.toggleFlip();
			return noErr;
			break;
			
		case kFITSUICommandSelectPlane:
			this->selectorChanged( command->menu.menuItemIndex - 1 );
			return noErr;
			break;
	}
	
	return eventNotHandledErr;
}

/**
 * @TODO must disable only one of the options. not the entire control.
 */
Void MacPlaneView::updateBitDepth( ChannelSettings depth, Bool isChosen, Bool isValid ) {
	if( isValid ) {
		radioBitDepth->enableItem( depth + 1 );
	} else {
		radioBitDepth->disableItem( depth + 1 );
	}
	
	if( isChosen ) {
		radioBitDepth->setValue( depth + 1 );
	}	
}

/**
 *
 */
Void MacPlaneView::updateUndefinedOption( UndefinedSettings opt, Bool isChosen, Bool isValid ) {
	if( isValid ) {
		radioUndefined->enableItem( opt + 1 );
	} else {
		radioUndefined->disableItem( opt + 1 );
	}
	
	if( isChosen ) {
		radioUndefined->setValue( opt + 1 );
	}
}

/**
 *
 */
Void MacPlaneView::updateSelectorIndex( Int index ) {
	planeSelectorControl->set16BitValue( index + 1 );
	planeSelectorControl->drawControl();
}

/**
 *
 */
Void MacPlaneView::updateSize( Int width, Int height ) {
	widthControl->setInt( width );
	heightControl->setInt( height );
}

Void MacPlaneView::updateFlip( Bool b ) {
	flipCheckbox->setValue( (b ? kControlCheckBoxCheckedValue : kControlCheckBoxUncheckedValue ) );
}
