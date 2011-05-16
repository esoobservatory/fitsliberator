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
 
#ifndef __BASECONTROL_H__
#define __BASECONTROL_H__

#include "Types.h"
#include "BaseComponent.h"
#include "BaseDialog.h"
#include "BaseMenu.h"

#define QD2QUARTZ( height, quickdraw ) 	height - quickdraw
#define QUARTZ2QD( height, quartz )		height - quartz 

namespace FitsLiberator {
	namespace Mac {
	
class BaseDialog;
	
/**
 * Base class for a control in a nib-based window. Encapsulates functions from the 
 * Window Manager and the Control Manager releated to windows. 
 */
class BaseControl : public BaseComponent {
	public:
		BaseControl();
		BaseControl( BaseDialog *dialog );
		BaseControl( BaseDialog *dialog, Int id );
		
		Int					    getID();
		Void 				    getByID( Int id );
		
		SInt32 				    getValue();
		Void 				    setValue( SInt32 value );
		Void 				    set16BitValue( SInt16 value );
		SInt16 				    get16BitValue( );
		SInt32 				    getMinimumValue();
		Void 				    setMinimumValue( SInt32 );
		SInt32 				    getMaximumValue();
		Void 				    setMaximumValue( SInt32 );
		
		OSErr				    setFontStyle( const ControlFontStyleRec* );
		
		Void 				    getData( ControlPartCode part, ResType tagName, Void *buffer, ::Size bufferSize, ::Size *actualSize );
		Void 				    getDataSize( ControlPartCode part, ResType tagName, ::Size *outBufferSize );
		Void 				    setData( ControlPartCode part, ResType tagName, ::Size dataSize, const Void *data);
		
		Void				    getBounds( Rect *r );
		Void 				    setVisibility( Bool inIsVisible, Bool inDoDraw );
		Void 				    deactivate();
		Void				    activate();
		Void 				    disable();
		Void				    enable();
		Void 				    drawControl();
		Void 				    keyboardFocus( ControlFocusPart part );

		Void				    getRegion( ControlPartCode part, RgnHandle outRegion );
	
		virtual EventHandlerRef installEventHandler( const EventTypeSpec * inList, UInt32 inNumTypes, EventHandler *handler );
	
		BaseDialog*			    getDialog();
		ControlRef			    getControl();

	private:
		ControlRef 	control;		///< Reference to the control.
		Int			controlID;		///< ID of control in window.
		BaseDialog* dialog;			///< Reference to the dialog in which the control resides.
}; 



	} // namespace Mac end
} // namespace FitsLiberator end

#endif //__BASECONTROL_H__