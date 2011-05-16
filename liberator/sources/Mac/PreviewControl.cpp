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

#include "PreviewControl.h"

using namespace FitsLiberator::Mac;

/**
 * Constructor for preview control.
 *
 * @param dlog	Reference for dialog in which the preview control resides.
 * @param id	ID of the control.
 *
 * @see FitsMacTerminology.h
 */
PreviewControl::PreviewControl( FitsDialog* dlog, Int id ) 
: BaseControl( dlog, id ),
  contextualMenu( NULL ),
  previewImage( NULL ),
  undefinedMask( NULL ),
  blackMask( NULL ),
  whiteMask( NULL ),
  throbber( NULL ),
  dialog( dlog ),
  previewPixels( NULL ),
  undefinedBits( NULL ) {
  
  getBounds( &bounds );
  
  imageRect.origin.x 	= 0;
  imageRect.origin.y 	= 0;
  imageRect.size.width 	= bounds.right - bounds.left;
  imageRect.size.height	= bounds.bottom - bounds.top;
  
  bounds.top = 0;
  bounds.left = 0;
  bounds.right = imageRect.size.width;
  bounds.bottom = imageRect.size.height;
  
	// Load the throbber
	CFBundleRef	bundle 			= Environment::getBundleFactory()->getCFBundleRef();
	CFURLRef 	url				= ::CFBundleCopyResourceURL( bundle, CFSTR("throbber.png"), 
									NULL, NULL );
	CGDataProviderRef provider 	= MachO::CGDataProviderCreateWithURL( url );
	throbber					= MachO::CGImageCreateWithPNGDataProvider( provider, NULL, false,
									kCGRenderingIntentDefault );
	::CFRelease( url );
	MachO::CGDataProviderRelease( provider );
}

PreviewControl::~PreviewControl() {
	if( undefinedBits != NULL ) {
		delete[] previewPixels;
	}
	
	if( previewImage != NULL ) {
		::CFRelease( previewImage );
	}
	
	if( previewPixels != NULL ) {
		delete[] previewPixels;
	}
	
	if( throbber != NULL ) {
		::CFRelease( throbber );
	}
}

/**
 * Create a image mask from a PreviewImageData bitmap.
 *
 * @param mask 		On input a pointer to a CGImageRef, on output the new image mask.
 * @param bitmap	The bit map data.
 * @param size		Size of bit map.
 */
Void PreviewControl::createImageMask( CGImageRef* mask, Byte * bitmap, FitsLiberator::Size& size ) {
	CGDataProviderRef provider = MachO::CGDataProviderCreateWithData( NULL, bitmap, size.getArea(), NULL );
	
	*mask = MachO::CGImageMaskCreate( size.width, size.height, 8, 8, size.width, provider, NULL, FALSE );
	
	MachO::CGDataProviderRelease( provider );
}

/**
 * Draw background of preview.
 */
Void PreviewControl::setPreviewImage( PreviewImage& srcImage ) {
	if( srcImage.size.width > 0 && srcImage.size.height > 0 ) {
		// Create preview image. 
		CGDataProviderRef provider = MachO::CGDataProviderCreateWithData( NULL, srcImage.previewPixels, srcImage.size.getArea(), NULL );
		CGColorSpaceRef colorSpace = MachO::CGColorSpaceCreateDeviceGray();
				      
        previewImage = MachO::CGImageCreate( 	srcImage.size.width, 
        										srcImage.size.height, 
        										8, 
        										8, 
        										srcImage.size.width, 
        										colorSpace, 
        										kCGImageAlphaNone, 
        										provider, 
        										NULL, 
        										false, 
        										kCGRenderingIntentDefault
        									);
        
        MachO::CGColorSpaceRelease(colorSpace);		
		MachO::CGDataProviderRelease( provider );
		
		createImageMask( &undefinedMask, srcImage.nullMap, srcImage.size );
		createImageMask( &blackMask, srcImage.blackClippedMap, srcImage.size );
		createImageMask( &whiteMask, srcImage.whiteClippedMap, srcImage.size );
	}	
}

/**
 * Draw background of preview.
 */
Void PreviewControl::drawBackground( CGContextRef context ) {
	MachO::CGContextSetRGBFillColor( context, 0, 0, 0, 0.5); 
    MachO::CGContextFillRect( context, imageRect );
    MachO::CGContextSetRGBStrokeColor( context, 0, 0, 0, 1); 
	MachO::CGContextStrokeRectWithWidth( context, imageRect, 1.0 );
}

/**
 *
 */
Void PreviewControl::drawPreviewImage( CGContextRef context ) {
	if( previewImage != NULL ) {
		MachO::CGContextDrawImage( context, imageRect, previewImage );
	}
}

/**
 *
 */
Void PreviewControl::drawWhiteMask( CGContextRef context ) {
	MachO::CGContextSetRGBFillColor( context, 0, 1, 0, 1 );
	
	if( blackMask != NULL ) {
		MachO::CGContextDrawImage( context, imageRect, whiteMask );
	}
}

/**
 *
 */
Void PreviewControl::drawBlackMask( CGContextRef context ) {
	MachO::CGContextSetRGBFillColor( context, 0, 0, 1, 1 );
	
	if( blackMask != NULL ) {
		MachO::CGContextDrawImage( context, imageRect, blackMask );
	}
}

/**
 *
 */
Void PreviewControl::drawUndefinedMask( CGContextRef context ) {
	MachO::CGContextSetRGBFillColor( context, 1, 0, 0, 1 );
	
	if( undefinedMask != NULL ) {
		MachO::CGContextDrawImage( context, imageRect, undefinedMask );
	}
}

/**
 * 
 */
Void PreviewControl::drawZoomArea( CGContextRef context, CGRect area ) {
	MachO::CGContextSetRGBFillColor( context, 0.710, 0.835, 1, 0.5 );
	MachO::CGContextFillRect( context, area );
	
	static const float dashLengths[] = { 4, 4 };
	MachO::CGContextSetLineDash( context, 0, dashLengths, 2 );
	MachO::CGContextSetRGBStrokeColor( context, 0, 0, 0, 1 );
	MachO::CGContextStrokeRectWithWidth( context, area, 1 );
	
	MachO::CGContextSetLineDash( context, 4, dashLengths, 2 );
	MachO::CGContextSetRGBStrokeColor( context, 1, 1, 1, 1 );
	MachO::CGContextStrokeRectWithWidth( context, area, 1 );
}

Void PreviewControl::drawThrobber(
    CGContextRef context, 
    int progress,
    const String& text)
{
	MachO::CGContextSetRGBFillColor( context, 0, 0, 1, 1 );
	
	if( throbber != NULL ) {
		// Extract the proper sub-image
		unsigned int	height	= CGImageGetHeight( throbber );
		CGRect			bounds	= CGRectMake(progress * height, 0, height, height );
		CGImageRef		tile	= CGImageCreateWithImageInRect( throbber, bounds );
		CGRect			pos		= CGRectMake( 
									(imageRect.size.width - height) / 2,
									(imageRect.size.height - height) / 2,
									height, height );
		
		// Draw the sub-image
		MachO::CGContextDrawImage( context, pos, tile );
        
        // Draw the progress text
        CGPoint start, end;
        CGContextSelectFont(context, "Helvetica", 10.0f, kCGEncodingMacRoman);
        //CGContextSetRGBStrokeColor(context, 0.0f, 0.0f, 0.0f, 1.0f);
        CGContextSetRGBFillColor(context, 0.0f, 0.0f, 0.0f, 1.0f);
        
        start = CGContextGetTextPosition(context);
        CGContextSetTextDrawingMode(context, kCGTextInvisible);
        CGContextShowText(context, text.c_str(), text.size());
		end = CGContextGetTextPosition(context);
        
        CGContextSetTextPosition(context, 
                                 (imageRect.size.width - (end.x - start.x)) / 2.0f,
                                 imageRect.size.height / 2.0f - height / 4.0f);
        CGContextSetTextDrawingMode(context, kCGTextFill);
        CGContextShowText(context, text.c_str(), text.size());
        
		// Clean up
		CGImageRelease( tile );
	}
}

/**
 * 
 */
ControlPartCode PreviewControl::getControlPart( const ::Point* p ) {
	if( ::PtInRect( *p, &bounds ) ) {
		return kControlPreviewPart;	
	} else {
		return kControlPreviewNoPart;
	}
	
}

/**
 * Set a contextual menu for a control.
 *
 * @param menu 	The menu.
 */
Void PreviewControl::setContextualMenu( BaseMenu *menu ) {
	contextualMenu = menu;
}

/**
 * Get the contextual menu of this control.
 */
BaseMenu* PreviewControl::getContextualMenu() {
	return contextualMenu;
}

UInt PreviewControl::getThrobberImageCount() const {
	if( throbber != NULL ) {
		return CGImageGetWidth( throbber ) / CGImageGetHeight( throbber );
	} else {
		return 0;
	}
}

