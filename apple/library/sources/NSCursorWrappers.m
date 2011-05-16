/*
 File:		NSCursorWrappers.m
 
 Abstract:	Provide Carbon wrapper functions around the Cocoa NSCursor methods we
 use to handle our cursor support.
 
 
 Version:	1.0
 
 Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
 ("Apple") in consideration of your agreement to the following terms, and your
 use, installation, modification or redistribution of this Apple software
 constitutes acceptance of these terms.  If you do not agree with these terms,
 please do not use, install, modify or redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject
 to these terms, Apple grants you a personal, non-exclusive license, under Apple's
 copyrights in this original Apple software (the "Apple Software"), to use,
 reproduce, modify and redistribute the Apple Software, with or without
 modifications, in source and/or binary forms; provided that if you redistribute
 the Apple Software in its entirety and without modifications, you must retain
 this notice and the following text and disclaimers in all such redistributions of
 the Apple Software.  Neither the name, trademarks, service marks or logos of
 Apple Computer, Inc. may be used to endorse or promote products derived from the
 Apple Software without specific prior written permission from Apple.  Except as
 expressly stated in this notice, no other rights or licenses, express or implied,
 are granted by Apple herein, including but not limited to any patent rights that
 may be infringed by your derivative works or by other works in which the Apple
 Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
 WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
 WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
 COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
 OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
 (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
 ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 Copyright © 2006 Apple Computer, Inc., All Rights Reserved
 */

/*
 
 This code is based on the code example Carbon Cocoa PictureCursor by Apple Computer, Inc.
 
 */

#import <Cocoa/Cocoa.h>
#import "NSCursorWrappers.h"

CGImageRef CGImageCreateFromURL( CFURLRef urlToImage ) {
	CGImageSourceRef	imageSource = CGImageSourceCreateWithURL( urlToImage, NULL );
	CGImageRef			theImage = nil;
	
	require( imageSource != nil, CantCreateImageSource );
	
	theImage = CGImageSourceCreateImageAtIndex( imageSource, 0, NULL );
	require( theImage != nil, CantGetImage );
	
	CFRelease( imageSource );
	
CantCreateImageSource:
CantGetImage:
	return( theImage );
}

NSImage *CreateNSImageFromCGImage( CGImageRef image ) {
	NSRect			imageRect		= NSMakeRect(0.0, 0.0, 0.0, 0.0);
	
	// Get the image dimensions.
	imageRect.size.height = CGImageGetHeight(image);
	imageRect.size.width = CGImageGetWidth(image);
	
	// Create a new image to receive the Quartz image data.
	NSImage	*newImage = [[NSImage alloc] initWithSize:imageRect.size]; 
	[newImage lockFocus];
	
	// Get the Quartz context and draw.
	CGContextRef	imageContext = (CGContextRef) [[NSGraphicsContext currentContext] graphicsPort];
	CGContextDrawImage( imageContext, *(CGRect*)&imageRect, image );
	[newImage unlockFocus];
	
	return( newImage );
}

CursorRef CreateCocoaCursor( CGImageRef cgImageRef, float hotSpotX, float hotSpotY ) {
	static BOOL		firstTime	= YES;
	
	if ( firstTime ) {
		//	Must first call [[[NSWindow alloc] init] release] to get the NSWindow machinery set up so that NSCursor can use a window to cache the cursor image
		[[[NSWindow alloc] init] release];
		firstTime = NO;
	}
	
	NSImage		*nsImage	= CreateNSImageFromCGImage( cgImageRef );
	NSCursor	*cursor		= [[NSCursor alloc] initWithImage:nsImage hotSpot:NSMakePoint( hotSpotX, hotSpotY )];
	
	[nsImage release];
	
	return( (CursorRef)cursor );
}

void ReleaseCocoaCursor( CursorRef cursor ) {
	[(NSCursor *)cursor release];
}

void SetCocoaCursor( CursorRef cursor ) {
	[(NSCursor *)cursor set];
}

CursorRef CreateCocoaCursorFromUrl( CFURLRef urlToImage, float hotSpotX, float hotSpotY ) {
	CGImageRef cgImageRef = CGImageCreateFromURL( urlToImage );
	CursorRef newCursor = NULL;
	
	require( cgImageRef != NULL, CantCreateImage );
	newCursor = CreateCocoaCursor( cgImageRef, hotSpotX, hotSpotY );
	if ( cgImageRef )
		CFRelease( cgImageRef );
	
CantCreateImage:		
	return( (CursorRef)newCursor );
}
