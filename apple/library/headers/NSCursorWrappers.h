/*
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

#include <Carbon/Carbon.h>	// for CFURLRef

#ifdef __cplusplus
extern "C" {
#endif
	
typedef struct OpaquePoolRef* PoolRef;
	
/**	@brief	Initialize Cocoa memory pool.
	@return	A reference to the memory pool.
 
	Call this to ensure that it is possible to call Cocoa APIs before calling RunApplicationEventLoop */
PoolRef CreateCocoaPool(void);
/**	@brief	Release a Cocoa memory pool.
	@param	Reference of the pool to release. */
void ReleaseCocoaPool(PoolRef pool);
/**	A CursorRef is just a pointer to an opaque data structure. */
typedef struct OpaqueCursorRef* CursorRef;

/**	@brief	Creates an NSCursor from an image URL.
	@param	urlToImage	URL of the cursor image.
	@param	hotSpotX	x-coordinate of the hotspot.
	@param	hotSpotY	y-coordinate of the hotspot.
	@return	A reference to the newly created cursor or NULL if the cursor could not be created.
			When the cursor is no longer needed it can be disposed by calling @see SetCocoaCursor. */
CursorRef CreateCocoaCursorFromUrl( CFURLRef urlToImage, float hotSpotX, float hotSpotY );
/**	@brief	Releases an NSCursor.
	@param	cursor	Reference to the cursor to release. */
void ReleaseCocoaCursor( CursorRef cursor );
/**	@brief	Uses an NSCursor as the current cursor for this application.
	@param	cursor	Reference to the cursor to use. */
void SetCocoaCursor( CursorRef cursor );
	
#ifdef __cplusplus
}
#endif
