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
 
#ifndef __HISTOGRAMCONTROL_H__
#define __HISTOGRAMCONTROL_H__

#include "Types.h"
#include "BaseControl.h"
#include "Environment.h"
#include "MachOFrameworkSupport.h"

namespace FitsLiberator {
	namespace Mac {
	
/**
 * Control parts constants for histogram slider
 */
enum {
	kControlSliderNoPart 				= kControlNoPart,
	kControlSliderWhiteLevelThumbPart 	= 3,
	kControlSliderBlackLevelThumbPart	= 4
};

/**
 * Control parts constants for histogram
 */
enum {
	kControlHistogramNoPart = kControlNoPart,
	kControlHistogramPart 	= 5
};

/**
 * Slider control with two thumbs. A white level and black level thumb.
 */	
class HistogramSliderControl : public BaseControl {
	public:
		HistogramSliderControl( BaseDialog*, Int id );
		~HistogramSliderControl();
		
		Void drawBackground( CGContextRef );
		Void drawWhiteLevelThumb( CGContextRef, Int );
		Void drawBlackLevelThumb( CGContextRef, Int );
		
		ControlPartCode getControlPart( const ::Point* );
		
	protected:
		static Void loadImage( CGImageRef*, CFStringRef );
		static Void loadIcon( IconRef*, CFStringRef, OSType );
	
	private:
		const FitsLiberator::Size iconSize;	///< Size of icon - implies that both icons have same size
		const Byte				  margin;	///< Margin on each side of control - defined by icon size.
		Int			ctrlWidth;				///< Width of control.
	
		IconRef whiteLevelThumbIcon;		///< Icon for white level thumb.
		IconRef	blackLevelThumbIcon;		///< Icon for black level thumb.
		const OSType whiteLevelIconType;	///< Icon type of white level thumb.
		const OSType blackLevelIconType;	///< Icon type of black level thumb.
		CGRect	whiteLevelBounds;			///< Placement of white level thumb.
		CGRect	blackLevelBounds;			///< Placement of black level thumb.
		Rect	backgroundBounds;			///< Bounds of background.
};

/**
 * Histogram control.
 */
class HistogramControl : public BaseControl {
	public:
		HistogramControl( BaseDialog *dialog, Int id );
		
		Void drawBackground( CGContextRef );
		Void drawHistogramBins( CGContextRef, Vector<Int>& );
		Void drawWhiteLevelMarker( CGContextRef, Int );
		Void drawBlackLevelMarker( CGContextRef, Int );
		Void drawPickerMarker( CGContextRef, Int );
		Void drawSelectArea( CGContextRef, Int, Int );
		Void drawMarker( CGContextRef, Int );
		/** Draws a marker at the specified location and with the specified color.
			@param context Drawing context.
			@param location Location to draw the marker.
			@param color Color of the marker. */
		Void drawMarker( CGContextRef context, Int location, const FitsLiberator::Color& color);

		
		ControlPartCode 	getControlPart( const ::Point* );
		
		Void 				setContextualMenu( BaseMenu *menu );
		BaseMenu*			getContextualMenu();
		
	private:
		const Byte					borderSize;			///< Size of broder around histogram.
		Rect						histogramRect;		///< For use with getControlPart().
		CGRect						histogramBounds;	///< Size/position of histogram.
		CGRect						selectAreaBounds;	///< Size/position of select area.
		BaseMenu* 					contextualMenu;		///< Contextual menu of control.
};



	} // namespace Mac end
} // namespace FitsLiberator end

#endif //__HISTOGRAMCONTROL_H__