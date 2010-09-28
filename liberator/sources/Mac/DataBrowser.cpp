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
 
#include "DataBrowser.h"
#include "MachOFrameworkSupport.h" 
 
using namespace FitsLiberator::Mac;
 
/**
 *
 *
 */
DataBrowserControl::DataBrowserControl( BaseDialog* dlog, Int id) 
: BaseControl( dlog, id ) {
	
}

/**
 *
 *
 */
OSStatus DataBrowserControl::setItemDataText( DataBrowserItemDataRef itemData, CFStringRef theData ) {
	return ::SetDataBrowserItemDataText( itemData, theData );	
}

/**
 * 
 * kThemeButtonOff, kThemeButtonOn, kThemeButtonMixed
 */
OSStatus DataBrowserControl::setItemDataButtonValue( DataBrowserItemDataRef itemData, ThemeButtonValue theData ) {
	return ::SetDataBrowserItemDataButtonValue( itemData, theData );	
}

/**
 *
 *
 */
ThemeButtonValue DataBrowserControl::getItemDataButtonValue( DataBrowserItemDataRef itemData ) {
	ThemeButtonValue theData = kThemeButtonOff;
	::GetDataBrowserItemDataButtonValue( itemData, &theData );
	return theData;
}

/**
 *
 *
 */
OSStatus DataBrowserControl::getItemDataText( DataBrowserItemDataRef itemData, CFStringRef *theData ) {
	return ::GetDataBrowserItemDataText( itemData, theData );	
}

/**
 *
 *
 */
OSStatus DataBrowserControl::setItemDataBooleanValue( DataBrowserItemDataRef itemData, Boolean theData ) {
	return ::SetDataBrowserItemDataBooleanValue( itemData, theData );	
}

/**
 *
 *
 */
OSStatus DataBrowserControl::copyEditText( CFStringRef* text ) {
	return ::CopyDataBrowserEditText( getControl(), text );
}

/**
 * @warn Horizontal scrollbars must be turned off.
 *
 */
Void DataBrowserControl::autoSizeListViewColumns() {
	if( getControl() != NULL ) {
		Boolean h, v;
	
		::GetDataBrowserHasScrollBars( getControl(), &h, &v );
		
		::SetDataBrowserHasScrollBars( getControl(), false, true );
		::AutoSizeDataBrowserListViewColumns( getControl() );
		::SetDataBrowserHasScrollBars( getControl(), h, v );
	} else {
		throw Exception("Couldn't initialize DataBrowser callbacks, because control isn't loaded.");
	}
}

/**
 *
 *
 */
Void DataBrowserControl::initCallbacks( DataBrowserCallbacks* callbacks ) {
	::InitDataBrowserCallbacks( callbacks );
}

/**
 *
 *
 */
Void DataBrowserControl::setCallbacks( DataBrowserCallbacks* callbacks ) {
	if( getControl() != NULL ) {
		::SetDataBrowserCallbacks( getControl(), callbacks );
	} else {
		throw Exception("Couldn't set DataBrowser callbacks, because control isn't loaded.");
	}
}

/**
 *
 *
 */
Void DataBrowserControl::initCustomCallbacks( DataBrowserCustomCallbacks* callbacks ) {
	::InitDataBrowserCustomCallbacks( callbacks );
}

/**
 *
 *
 */
Void DataBrowserControl::setCustomCallbacks( DataBrowserCustomCallbacks* callbacks ) {
	if( getControl() != NULL ) {
		::SetDataBrowserCustomCallbacks( getControl(), callbacks );
	} else {
		throw Exception("Couldn't set DataBrowser custom callbacks, because control isn't loaded.");
	}
}

/**
 *
 *
 */
Void DataBrowserControl::addItems( DataBrowserItemID container, UInt32 numItems, const DataBrowserItemID *items, DataBrowserPropertyID preSortProperty ) {
	if( getControl() != NULL ) {
		::AddDataBrowserItems( getControl(), container, numItems, items, preSortProperty );
	} else {
		throw Exception("Couldn't set DataBrowser callbacks, because control isn't loaded.");
	}
}

/**
 *
 *
 */
Void DataBrowserControl::setListViewDisclosureColumn( DataBrowserTableViewColumnID column, Boolean expandableRows) {
	if( getControl() != NULL ) {
		::SetDataBrowserListViewDisclosureColumn( getControl(), column, expandableRows );
	} else {
		throw Exception("Couldn't set list view disclosure column, because control isn't loaded.");
	}
}

/**
 *
 *
 */
Void DataBrowserControl::openContainer( DataBrowserItemID itemID ) {
	if( getControl() != NULL ) {
		::OpenDataBrowserContainer( getControl(), itemID );
	} else {
		throw Exception("Couldn't open data browser container, because control isn't loaded.");
	}
}

/**
 *
 *
 */
OSStatus DataBrowserControl::getPropertyFlags( DataBrowserPropertyID property, DataBrowserPropertyFlags * flags ) {
	if( getControl() != NULL ) {
		return ::GetDataBrowserPropertyFlags( getControl(), property, flags );
	} else {
		throw Exception("Couldn't get DataBrowser property flags, because control isn't loaded.");
	}
}

/**
 *
 *
 */
OSStatus DataBrowserControl::setPropertyFlags( DataBrowserPropertyID property, DataBrowserPropertyFlags flags ) {
	if( getControl() != NULL ) {
		return ::SetDataBrowserPropertyFlags( getControl(), property, flags );
	} else {
		throw Exception("Couldn't set DataBrowser property flags, because control isn't loaded.");
	}
}

/**
 *
 * @WARN This function only has effect on OS X 10.4
 */
Void DataBrowserControl::changeAttributes( OptionBits inAttributesToSet, OptionBits inAttributesToClear ) {
	if( getControl() != NULL ) {
		DataBrowserChangeAttributes( getControl(), inAttributesToSet, inAttributesToClear );
	} else {
		throw Exception("Couldn't change DataBrowser attributes, because control isn't loaded.");
	}
}

/**
 *
 * @WARN This function only has effect on OS X 10.4
 */
Void DataBrowserControl::getAttributes( OptionBits * outAttributes ) {
	if( getControl() != NULL ) {
		DataBrowserGetAttributes( getControl(), outAttributes );
	} else {
		throw Exception("Couldn't get DataBrowser attributes, because control isn't loaded.");
	}
}


/**
 *
 */
OSStatus DataBrowserControl::getItems( DataBrowserItemID container, Boolean recurse, DataBrowserItemState state, Handle items) {
	return ::GetDataBrowserItems( getControl(), container, recurse, state, items);
}