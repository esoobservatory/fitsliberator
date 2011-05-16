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
 
#include "BaseMenu.h"

using namespace FitsLiberator::Mac;

/**
 * Constructor
 */
BaseMenu::BaseMenu() : menu( NULL ) {}

/**
 * Constructor
 */
BaseMenu::BaseMenu( MenuRef m ) : menu( m ) {
	
}

/**
 * Destructor
 */
BaseMenu::~BaseMenu() {
	if( menu != NULL ) {
		// Important to not fuck up Photoshops Image Size menu.
		::DeleteMenu( ::GetMenuID( menu ) );
	}	
}


/**
 * Create a menu from a nibs
 *
 * @param bundleFactory	A bundle factory. 
 * @param nibFileName	
 * @param menuName	
 *
 * @see BundleFactory.
 */
Void BaseMenu::create( BundleFactory* bundleFactory, ::CFStringRef nibFileName, ::CFStringRef menuName ) {
	::IBNibRef	menuNib;
	
	::CFBundleRef menuBundle = bundleFactory->getCFBundleRef();
	
	if( noErr != ::CreateNibReferenceWithCFBundle( menuBundle, nibFileName, &menuNib ) ) {
		throw Exception( "Can't create reference to NIB file" );
	} 
	
    if( noErr != ::CreateMenuFromNib( menuNib, menuName, &menu ) ) {
    	throw Exception( "Can't create menu" );
    }
    
    ::DisposeNibReference( menuNib );
    bundleFactory->release( menuBundle );
    
   	insert(-1);
}

/**
 * Installs an event handler for the menu. The user data for the event handler, will be 
 * a reference to this object.
 *
 * @param 	inList		A pointer to an array of EventTypeSpec entries representing the events you are interested in.
 * @throws 	Exception	On error.
 */
EventHandlerRef BaseMenu::installEventHandler( const ::EventTypeSpec * inList, UInt32 inNumTypes, EventHandler *handler ) 
{
	if( menu != NULL ) {
		return BaseComponent::installBaseEventHandler( ::GetMenuEventTarget( menu ), inList, inNumTypes, handler );
	} else {
		throw Exception( "Menu not loaded! Can't install event handler for menu." );	
	}
}

/**
 * Get the menu reference
 *
 * @return Menu reference.
 */
MenuRef BaseMenu::getMenu() {
	return menu;
}

/**
 * Inserts the menu into the current menu list.
 */
Void BaseMenu::insert( MenuID beforeID ) {
	if( menu != NULL ) {
		::InsertMenu( menu, beforeID );	
	}
}

/**
 * 
 *
 */
Void BaseMenu::deleteAllItems( ) {
	UInt16 itemCount = ::CountMenuItems( menu );
	
	::DeleteMenuItems( menu, 1, itemCount );
}


/**
 * Appends a menu item to the menu.
 */
MenuItemIndex BaseMenu::addSeparator() {
	String txt = "";
	
	return addItem(txt, kMenuItemAttrSeparator, NULL );
}

/**
 * Appends a menu item to the menu.
 */
MenuItemIndex BaseMenu::addItem( String& text, MenuAttributes attributes, MenuCommand command ) {
	CFStringRef menuText = ::CFStringCreateWithCString( NULL, text.c_str(), kCFStringEncodingMacRoman );
	
	MenuItemIndex newItem;
		
	::AppendMenuItemTextWithCFString( menu, menuText, attributes, command, &newItem );	
		
	::CFRelease( menuText );
	
	return newItem;
}

/**
 * Enables or disables a menu item
 *
 * @param enable	True to enable, false to disable.
 * @param i			Index of menu item
 */
Void BaseMenu::enableItem( Bool enable, MenuItemIndex i ) {
	if( enable ) {
		EnableMenuItem( menu, i );
	} else {
		DisableMenuItem( menu, i );
	}
}