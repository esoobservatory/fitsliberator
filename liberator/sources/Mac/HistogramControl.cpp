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
 
#include "HistogramControl.h"

using namespace FitsLiberator;
using namespace FitsLiberator::Mac;

//-------------------------------------------------------------------------------
//	HistogramSliderControl implementation
//-------------------------------------------------------------------------------

/**
 * Initialize slider control. Loads icons.
 */
HistogramSliderControl::HistogramSliderControl( BaseDialog* dlog, Int id )
: BaseControl( dlog, id ), 
  whiteLevelThumbIcon( NULL ),
  blackLevelThumbIcon( NULL ),
  whiteLevelIconType( 'IcWS' ),
  blackLevelIconType( 'IcBS' ),
  iconSize( 9,8 ),
  margin( 4 ) {
  
	loadIcon( &whiteLevelThumbIcon, CFSTR("thumbWhiteLevel.icns"), whiteLevelIconType );
	loadIcon( &blackLevelThumbIcon, CFSTR("thumbBlackLevel.icns"), blackLevelIconType );
		
	getBounds( &backgroundBounds );
	
	ctrlWidth = backgroundBounds.right - backgroundBounds.left;
	
	backgroundBounds.top		= 0;
    backgroundBounds.left		= margin;
    backgroundBounds.bottom		= 0;
    backgroundBounds.right		= ctrlWidth - margin;
    
	whiteLevelBounds.origin.x 		= ctrlWidth - 2*margin - 1;
	whiteLevelBounds.origin.y 		= 4;
	whiteLevelBounds.size.width 	= iconSize.width;
	whiteLevelBounds.size.height 	= iconSize.height;
	
	blackLevelBounds.origin.x 		= 0;
	blackLevelBounds.origin.y 		= 4;
	blackLevelBounds.size.width 	= iconSize.width;
	blackLevelBounds.size.height 	= iconSize.height;
}

/**
 * Diposes loaded icons.
 */
HistogramSliderControl::~HistogramSliderControl(){
	if( whiteLevelThumbIcon != NULL ) { ::UnregisterIconRef( kFITSApplicationSignature, whiteLevelIconType ); }
	if( blackLevelThumbIcon != NULL ) { ::UnregisterIconRef( kFITSApplicationSignature, blackLevelIconType ); }
}

/**
 * Draws the background and track of the slider.
 */
Void HistogramSliderControl::drawBackground( CGContextRef context ) {
	::DrawThemeSeparator( &backgroundBounds, kThemeStateActive );
}

/**
 * Draws the white level thumb on the track.
 * 
 * @param value	Postion of thumb on track.
 *
 * @todo Change icon if value is outside of control.
 */
Void HistogramSliderControl::drawWhiteLevelThumb( CGContextRef context, Int value ) {
	if( value >= 0 && value < ctrlWidth - 2*margin) {
		whiteLevelBounds.origin.x = value;
	} else {
		whiteLevelBounds.origin.x = ( value < 0 ) ? 0 : ctrlWidth - 2*margin - 1;
	}

	static const RGBColor labelColor = { 0, 0, 0 };
	MachO::PlotIconRefInContext( context, &whiteLevelBounds, kAlignAbsoluteCenter, kTransformNone, &labelColor, kPlotIconRefNormalFlags, whiteLevelThumbIcon );
}

/**
 * Draws the black level thumb on the track.
 * 
 * @param value	Postion of thumb on track.
 *
 * @todo Change icon if value is outside of control.
 */
Void HistogramSliderControl::drawBlackLevelThumb( CGContextRef context, Int value ) {
	if( value >= 0 && value < ctrlWidth - 2*margin) {
		blackLevelBounds.origin.x = value;
	} else {
		blackLevelBounds.origin.x = ( value < 0 ) ? 0 : ctrlWidth - 2*margin - 1;
	}
	
	static const RGBColor labelColor = { 0, 0, 0 };
	MachO::PlotIconRefInContext( context, &blackLevelBounds, kAlignAbsoluteCenter, kTransformNone, &labelColor, kPlotIconRefNormalFlags, blackLevelThumbIcon );
}


/**
 * Test which part of the control is hit by a given point in coordinates relative to control.
 *
 * @return kControlSliderWhiteLevelThumbPart, kControlSliderBlackLevelThumbPart or kControlSliderNoPart.
 */
ControlPartCode HistogramSliderControl::getControlPart( const ::Point* p ) {
	Int pos	  = p->h;
	Int small = blackLevelBounds.origin.x + margin;
	Int large = whiteLevelBounds.origin.x + margin;
		
	if( pos <= small ) {
		return kControlSliderBlackLevelThumbPart;
	} else if( pos >= large ) {
		return kControlSliderWhiteLevelThumbPart;
	} else {
		if( large - pos < pos - small ){
			return kControlSliderWhiteLevelThumbPart;
		} else if( large - pos > pos - small ) {
			return kControlSliderBlackLevelThumbPart;
		}
	}
	
//	bounds.top		= whiteLevelBounds.origin.y;
//	bounds.bottom 	= whiteLevelBounds.origin.y + whiteLevelBounds.size.height;
//	bounds.left		= whiteLevelBounds.origin.x;
//	bounds.right	= whiteLevelBounds.origin.x + whiteLevelBounds.size.width;
//	
//	if( ::PtInIconRef( p, &bounds, kAlignAbsoluteCenter, kIconServicesNormalUsageFlag, whiteLevelThumbIcon ) ) {
//		return kControlSliderWhiteLevelThumbPart;
//	} 
//	
//	bounds.top		= blackLevelBounds.origin.y;
//	bounds.bottom 	= blackLevelBounds.origin.y + blackLevelBounds.size.height;
//	bounds.left		= blackLevelBounds.origin.x;
//	bounds.right	= blackLevelBounds.origin.x + blackLevelBounds.size.width;
//	
//	if( ::PtInIconRef( p, &bounds, kAlignAbsoluteCenter, kIconServicesNormalUsageFlag, blackLevelThumbIcon ) ) {
//		return kControlSliderBlackLevelThumbPart;
//	}
	
	return kControlSliderNoPart;
}

/**
 * Loads an icon from the Resource folder of the plug-in's bundle.
 * 
 * @param icon		On output a pointer to the icon.
 * @param filename	The filename of the icon.
 * @param iconType	Application defined type of icon, e.g. 'IcHT'.
 */
Void HistogramSliderControl::loadIcon( IconRef* icon, CFStringRef filename, OSType iconType ) {
	// Obtain bundle reference
	CFBundleRef	bundle = Environment::getBundleFactory()->getCFBundleRef();
	
	// Find icon file in resource directory.
	CFURLRef url = ::CFBundleCopyResourceURL( bundle, filename, NULL, NULL );
		
	// Obtain a FSRef for icon file.
	FSRef fileRef;
	
	if( ::CFURLGetFSRef( url, &fileRef ) != true ) {
		throw Exception( "Couldn't read icon file for histogram slider control." );
	}

	::CFRelease( url );
	
	// Register the icon
	if( noErr != ::RegisterIconRefFromFSRef( kFITSApplicationSignature, iconType, &fileRef, icon ) ) {
		throw Exception( "Couldn't register icon file for histogram control." );
	}
}

/**
 * Loads an PNG image from the Resource folder of the plug-in's bundle.
 * 
 * @param image 	On output a pointer to the image.
 * @param filename	The filename of the image.
 */
Void HistogramSliderControl::loadImage( CGImageRef* image, CFStringRef filename ) {
	// Obtain bundle reference
	CFBundleRef	bundle 	= Environment::getBundleFactory()->getCFBundleRef();
	// Get URL to image
	CFURLRef 	url		= ::CFBundleCopyResourceURL( bundle, filename, NULL, NULL );
	// Create data provider from image file.
	CGDataProviderRef provider = MachO::CGDataProviderCreateWithURL( url );
	
	// Create image
	*image = MachO::CGImageCreateWithPNGDataProvider( provider, NULL, true, kCGRenderingIntentDefault );
	
	// Release acquired resources.
	::CFRelease( url );
	MachO::CGDataProviderRelease( provider );
}


//-------------------------------------------------------------------------------
//	HistogramControl implementation
//-------------------------------------------------------------------------------

/**
 * Initialize histogram control.
 */
HistogramControl::HistogramControl( BaseDialog* dlog, Int id )
: BaseControl( dlog, id ), 
  contextualMenu( NULL ),
  borderSize( 1 ) {
  	Rect bounds;

	getBounds( &bounds );
	
	histogramBounds = CGRectMake( 0, 0, bounds.right - bounds.left, bounds.bottom - bounds.top );

	histogramRect.bottom	= histogramBounds.size.height - borderSize;
	histogramRect.top		= borderSize;
	histogramRect.right		= histogramBounds.size.width - borderSize;
	histogramRect.left		= borderSize;
	
	selectAreaBounds = CGRectMake( 0, borderSize, 0, histogramBounds.size.height - 2*borderSize );
}

/**
 * Draws the background of the histogram.
 * 
 */
Void HistogramControl::drawBackground( CGContextRef context ) {
	MachO::CGContextSetRGBFillColor( context, 1, 1, 1, 1 ); 
    MachO::CGContextFillRect( context, histogramBounds );
    MachO::CGContextSetRGBStrokeColor( context, 0, 0, 0, 1 ); 
	MachO::CGContextStrokeRectWithWidth( context, histogramBounds, borderSize );
}

/**
 * Draws the bins of the histogram. 
 *
 * @todo Investigate if bins.size can be replaced by ...
 */
Void HistogramControl::drawHistogramBins( CGContextRef context, Vector<Int>& bins ) {
	MachO::CGContextSetShouldAntialias( context, false );
	MachO::CGContextSetRGBStrokeColor( context, 0.55, 0.55, 0.55, 0.8 );
	
	for( UInt x = 0; x < bins.size(); x++ ) {
		if( bins[x] > 0 ) {
			// Move one forward (x+1) so we don't paint in the border
			MachO::CGContextMoveToPoint( context, x + borderSize, borderSize );			 
			MachO::CGContextAddLineToPoint( context, x + borderSize, bins[x] );	
		}	
    }

	MachO::CGContextStrokePath( context );
}

/**
 * Draw a marker with the current stroke color of the contex.
 *
 * @param value	A postion greater than or equal to 0 and less than (<control width> - 2 x border size).
 */
Void HistogramControl::drawMarker( CGContextRef context, Int value ) {
	if( value >= 0 && value < histogramBounds.size.width - 2*borderSize ) {
		MachO::CGContextMoveToPoint( context, value+borderSize, borderSize );	
		MachO::CGContextAddLineToPoint( context, value+borderSize, histogramBounds.size.height-2*borderSize );
		MachO::CGContextStrokePath( context );	
	}
}

Void HistogramControl::drawMarker( CGContextRef context, Int value, const Color& color ) {
	if( value >= 0 && value < histogramBounds.size.width - 2*borderSize ) {
		MachO::CGContextMoveToPoint( context, value+borderSize, borderSize );
        MachO::CGContextSetRGBStrokeColor(context, 
            ((float)color.red) / 255.0, ((float)color.green) / 255.0, ((float)color.blue) / 255.0, 1.0);        
		MachO::CGContextAddLineToPoint( context, value+borderSize, histogramBounds.size.height-2*borderSize );
		MachO::CGContextStrokePath( context );
	}
}

/**
 * Draw the white level marker in the histogram. 
 */
Void HistogramControl::drawWhiteLevelMarker( CGContextRef context, Int value ) {
	MachO::CGContextSetRGBStrokeColor( context, 0, 0, 0, 1 );
	drawMarker( context, value );
}

/**
 * Draw the black level marker in the histogram. 
 */
Void HistogramControl::drawBlackLevelMarker( CGContextRef context, Int value ) {
	MachO::CGContextSetRGBStrokeColor( context, 0, 0, 0, 1 );
	drawMarker( context, value );
}

/**
 * Draw the white level marker in the histogram. 
 */
Void HistogramControl::drawPickerMarker( CGContextRef context, Int value ) {
	MachO::CGContextSetRGBStrokeColor( context, 0.65, 0.65, 0.65, 0.5);
	drawMarker( context, value );
}

/**
 * Draws a select area.
 */
Void HistogramControl::drawSelectArea( CGContextRef context, Int val1, Int val2 ) {
	if( val1 >= 0 && val1 < histogramBounds.size.width - 2*borderSize ) {
		selectAreaBounds.origin.x	= val1 + borderSize;	
	} else {
		selectAreaBounds.origin.x	= ( val1 < 0 ) ? borderSize : histogramBounds.size.width - 2*borderSize;
	}
	
	if( val2 >= 0 && val2 < histogramBounds.size.width - 2*borderSize ) {
		selectAreaBounds.size.width = val2 - val1;
	} else {
		selectAreaBounds.size.width = ( val2 < 0 ) ? -val1 : histogramBounds.size.width - 2*borderSize - val1;
	}

	MachO::CGContextSetRGBFillColor( context, 0.710, 0.835, 1, 0.5 );
	MachO::CGContextFillRect( context,  selectAreaBounds );
	
	// MachO::CGContextSetRGBStrokeColor( context, 0, 0, 0, 1 );
	// static const float dashLengths[] = { 4, 3 };
	// MachO::CGContextSetLineDash( context, 4, dashLengths, 2 );
	// No +1 on val2 in following statement.
	// MachO::CGContextStrokeRectWithWidth( context, CGRectMake( val1+1, 1, val2-val1, (r->bottom - r->top)-3 ), 1.0 );
}

/**
 * Test which part of the control is hit by a given point in coordinates relative to control.
 *
 * @return kControlHistogramPart or kControlHistogramNoPart
 */
ControlPartCode HistogramControl::getControlPart( const ::Point* p ) {
	if( ::PtInRect( *p, &histogramRect ) ){
		return kControlHistogramPart;
	} else {
		return kControlHistogramNoPart;
	}
}

/**
 * Set a contextual menu for a control.
 *
 * @param menu 	The menu.
 */
Void HistogramControl::setContextualMenu( BaseMenu *menu ) {
	contextualMenu = menu;
}

/**
 * Get the contextual menu of this control.
 */
BaseMenu* HistogramControl::getContextualMenu() {
	return contextualMenu;
}