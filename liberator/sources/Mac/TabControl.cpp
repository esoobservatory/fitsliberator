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
 
#include "TabControl.h"

using namespace FitsLiberator::Mac;

//-------------------------------------------------------------------------------
//	TabControl Implementation
//-------------------------------------------------------------------------------

/**
 * Constructor - installs an event handler for the tab control,
 * and switch to the first tab control.
 *
 * When the tab is clicked, then TabControl::onHit( EventRef event ) is called. 
 */
TabControl::TabControl( BaseDialog *dialog, Int id, const TabPaneID* panes ) : tabPanes( panes ), BaseControl( dialog, id ) {	
	lastTabIndex = 1;
	
	setValue( lastTabIndex );
	switchTab( lastTabIndex );
}

/**
 * Event handler for a click in the tab control.
 */
OSStatus TabControl::onHit( EventRef event ) {
	TabPaneID newTabIndex = getValue();
		
	if( newTabIndex != lastTabIndex ){
		switchTab( newTabIndex );
		return noErr;
	} else {
		// We got somekind of error.
		return eventNotHandledErr;
	}
}

/**
 * Activates or deactivates a control.
 *
 * @param pane 		ID of tab user pane.
 * @param isEnabled	Set to true to activate, false to deactivate tab pane.
 */
Void TabControl::setTabEnabled( TabPaneID pane, Bool isEnabled ) {
	if( getControl() != NULL ) {
		for( TabPaneID i = 1; i <= (*tabPanes) ; i++) {	// Must be +1 because tab starts index at 1 and not 0.
			if( *(tabPanes+i) == pane ) {
				::SetTabEnabled( getControl(), i, isEnabled );
			} 
		}
	} else {
		throw Exception( "Control not loaded!" );
	}
}

/**
 * Get the bound of a tab pane.
 */
Void TabControl::getTabPaneBounds( Rect *r ) {
	BaseControl tabPane( getDialog(), *(tabPanes+1) );
	tabPane.getBounds( r );
}

/**
 * Get id of enabled tab.
 */
TabPaneID TabControl::getEnabledTab() {
	return lastTabIndex;
}


/**
 * Get translation of a point in the tab, to the same point in the window.
 */
::Point TabControl::getTranslation() {
	::Point	p;
	::Rect	r1,r2;
	
	getBounds( &r1 );
	getTabPaneBounds( &r2 );
	
	p.h = r1.left + r2.left;
	p.v = r1.top + r2.top;
	
	return p;
}

/**
 * Switch tab pane.
 *
 * @param switchToIndex	Index of tab to switch to.
 */
Void TabControl::switchTab( TabPaneID switchToIndex ) {
	TabPaneID			selectedTabPaneID 	= 0;
	BaseControl 		tabPane( getDialog() );

	// Hide the unselected tab panes.
	for( TabPaneID i = 1; i <= (*tabPanes); i++) {	// Must be +1 because tab starts index at 1 and not 0.
		if( i == switchToIndex ) {
			selectedTabPaneID = *(tabPanes+i);
			setValue( switchToIndex );
		} else {
			tabPane.getByID( *(tabPanes+i) );
			tabPane.setVisibility( false, false );
		} 
	}
	
	if( selectedTabPaneID != 0 ) {
		//getDialog()->clearKeyboardFocus();
		tabPane.getByID( selectedTabPaneID );
		tabPane.setVisibility( true, true );
	
		lastTabIndex = switchToIndex;
	}
	
	drawControl();
}