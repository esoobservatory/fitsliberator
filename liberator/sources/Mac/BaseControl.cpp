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
 
#include "BaseControl.h"

using namespace FitsLiberator::Mac;

//-------------------------------------------------------------------------------
//	BaseControl implementation
//-------------------------------------------------------------------------------

/**
 * Constructor for a base control.
 *
 * @param 	dlog		Dialog in which control resides.
 */
BaseControl::BaseControl( BaseDialog *dlog ) : dialog( dlog ), control( NULL ), controlID( NULL ) {
}

/**
 * Constructor for a base control.
 */
BaseControl::BaseControl( ) : dialog( NULL ), control( NULL ), controlID( NULL ) {
}

/**
 * Constructor for a base control.
 *
 * @param 	id			ID of control
 * @param 	dlog		Dialog in which control resides.
 * @throws 	Exception	If control couldn't be found.
 */
BaseControl::BaseControl( BaseDialog *dlog, Int id ) : dialog ( dlog ) {
	getByID( id );
}

Void BaseControl::getByID( Int id ) {
	if( dialog != NULL ) {
		control = dialog->getControlByID( id );
		controlID = id;
	} else {
		throw Exception("BaseControl not initialized.");
	}
}

/**
 * Get the dialog in which the control resides.
 */
BaseDialog* BaseControl::getDialog() {
	return dialog;
}

/**
 * Set a 32 bit value for the control.
 *
 * @param value		Value to set.
 */
Void BaseControl::setValue( SInt32 value ) {
	if( control != NULL ) {
		::SetControl32BitValue( control, value );	
	} else {
		throw Exception( "Couldn't set value of control - control not loaded!" );	
	}
}

/**
 * Set a value for the control.
 *
 * @param value		Value to set.
 */
Void BaseControl::set16BitValue( SInt16 value ) {
	if( control != NULL ) {
		::SetControlValue( control, value );	
	} else {
		throw Exception( "Couldn't set value of control - control not loaded!" );	
	}
}

/**
 * Get a value for the control.
 *
 * @param value		Value to set.
 */
SInt16 BaseControl::get16BitValue() {
	if( control != NULL ) {
		return ::GetControlValue( control );
	} else {
		throw Exception( "Couldn't get value of control - control not loaded!" );	
	}
}

/**
 * Get the 32 bit value for the control.
 */
SInt32 BaseControl::getValue() {
	if( control != NULL ) {
		return ::GetControl32BitValue( control );
	} else {
		throw Exception( "Couldn't get value of control - control not loaded!" );	
	}	
}

/**
 * Move control to Point in local coordiantes.
 */
//Void BaseControl::moveControl( ::Point p ) {
//	if( control != NULL ) {
//		::MoveControl( control, p.h, p.v );
//	} else {
//		throw Exception( "Couldn't move control - control not loaded!" );	
//	}
//}

/**
 * Move control to x,y in local coordiantes.
 */
//Void BaseControl::moveControl( Int x, Int y ) {
//	if( control != NULL ) {
//		::MoveControl( control, x, y );
//	} else {
//		throw Exception( "Couldn't move control - control not loaded!" );	
//	}
//}


/**
 * Get Control Data
 *
 * @see GetControlData in Control Manager.
 */
Void BaseControl::getData( ControlPartCode part, ResType tagName, Void *buffer, ::Size bufferSize, ::Size *actualSize ) {
	if( control != NULL ) {
		OSStatus res;
	
		res = ::GetControlData( control, part, tagName, bufferSize, buffer, actualSize );

		if( res != noErr  ) {
			if( res == errDataNotSupported ) {
				throw Exception( "Invalid data tag used to get control data. This is a bug, please file it a http://www.spacetelescpoe.org/projects/fits_liberator/." );
			} else {
				throw Exception( "Couldn't get control data." );
			}
		}
	} else {
		throw Exception( "Couldn't get control data - control not loaded!" );
	}
}


/**
 * Get Control Data.
 *
 */
Void BaseControl::getDataSize( ControlPartCode part, ResType tagName, ::Size *outBufferSize ) {
	if( control != NULL ) {
		OSStatus res;
	
		res = ::GetControlDataSize( control, part, tagName, outBufferSize );
		
		if( res != noErr ) {
			if( res == errDataNotSupported ) {
				throw Exception( "Invalid data tag use to query control. This is a bug, please file it a http://www.spacetelescpoe.org/projects/fits_liberator/." );
			} else {
				throw Exception( "Couldn't get size of data in control." );
			}
		}
	} else {
		throw Exception( "Couldn't get control data - control not loaded!" );
	}
}

/**
 * Set Control Data
 *
 */
Void BaseControl::setData( ControlPartCode part, ResType tagName, ::Size dataSize, const Void *data) {
	if( control != NULL ) {
		OSStatus res;
	
		res = ::SetControlData( control, part, tagName, dataSize, data );

		if( res != noErr  ) {
			if( res == errDataNotSupported ) {
				throw Exception( "Invalid data tag used to set control data. This is a bug, please file it a http://www.spacetelescpoe.org/projects/fits_liberator/." );
			} else {
				throw Exception( "Couldn't set control data." );
			}
		}
	} else {
		throw Exception( "Couldn't set control data - control not loaded!" );
	}
}

/**
 * Get the bounds of a control.
 *
 * @param r	On input, a pointer to a Rect, on output the bounds of the control.
 */
Void BaseControl::getBounds( Rect *r ) {
	if( control != NULL ) {
		::GetControlBounds( control, r );
	} else {
		throw Exception( "Couldn't get bounds of control - control not loaded!" );	
	}
}


/**
 * Set the visibility of a control.
 *
 * @see http://developer.apple.com/documentation/Carbon/Reference/Control_Manager/controlman_ref/function_group_3.html#//apple_ref/doc/uid/TP30000171/F06782
 */
Void BaseControl::setVisibility( Bool inIsVisible, Bool inDoDraw ) {
	if( control != NULL ) {
		if( noErr != ::SetControlVisibility( control, inIsVisible, inDoDraw ) ){
			throw Exception( "Couldn't set visibility of control." );
		}
	} else {
		throw Exception( "Couldn't set visibility of control - control not loaded!" );	
	}
}

/**
 * Activate control.
 */
Void BaseControl::activate() {
	if( control != NULL ) {
		::ActivateControl( control );	
	} else {
		throw Exception( "Couldn't activate control - control not loaded!" );	
	}
}

/**
 * Deactivate control.
 */
Void BaseControl::deactivate() {
	if( control != NULL ) {
		::DeactivateControl( control );	
	} else {
		throw Exception( "Couldn't deactivate control - control not loaded!" );	
	}
}

/**
 * Enable control.
 */
Void BaseControl::enable() {
	if( control != NULL ) {
		::EnableControl( control );	
	} else {
		throw Exception( "Couldn't enable control - control not loaded!" );	
	}
}

/**
 * Disable control.
 */
Void BaseControl::disable() {
	if( control != NULL ) {
		::DisableControl( control );	
	} else {
		throw Exception( "Couldn't disable control - control not loaded!" );	
	}
}

/**
 * Get the control of this class.
 *
 * @return Reference to the control.
 */
ControlRef BaseControl::getControl() {
	if( control != NULL ) { 
		return control;			
	}  else {
		throw Exception( "Couldn't get control reference - control not loaded!" );	
	}
}


/**
 * Draw this control.
 */
Void BaseControl::drawControl() {
	if( control != NULL ) {
		::Draw1Control( control);	
	} else {
		throw Exception( "Couldn't draw control - control not loaded!" );	
	}
}


/**
 * Installs an event handler for the control. The user data for the event handler, will be 
 * a reference to this object.
 *
 * @param 	inList		A pointer to an array of EventTypeSpec entries representing the events you are interested in.
 * @throws 	Exception	On error.
 */
EventHandlerRef BaseControl::installEventHandler( const ::EventTypeSpec * inList, UInt32 inNumTypes, EventHandler *handler ) {
	if( control != NULL ) {
		return BaseComponent::installBaseEventHandler( ::GetControlEventTarget( control ), inList, inNumTypes, handler );
	} else {
		throw Exception( "Couldn't install event handler for control - control not loaded!" );	
	}
}

/**
 * Get a region of the control
 */
Void BaseControl::getRegion( ::ControlPartCode part, ::RgnHandle outRegion ) {
    if( control != NULL ) {
		::GetControlRegion( control, part, outRegion );
    } else {
    	throw Exception("Couldn't get control region - control not loaded!");
    }
}

/**
 * Get the ID of the control.
 *
 * @return ID of control.
 */
Int BaseControl::getID() {
	return controlID;
}

/**
 * Set minimum value of control
 *
 * @param value Minimum value.
 */
Void BaseControl::setMinimumValue( SInt32 value ) {
	if( control != NULL ) {
		::SetControl32BitMinimum( control, value );
    } else {
    	throw Exception("Couldn't set control minimum value - control not loaded!");
    }
}

/**
 * Set minimum value of control
 *
 * @return Minimum value of control.
 */
SInt32 BaseControl::getMinimumValue() {
	if( control != NULL ) {
		return ::GetControl32BitMinimum( control );
    } else {
    	throw Exception("Couldn't get control minimum value - control not loaded!");
    }
}

/**
 * Set maximum value of control
 *
 * @param value Maximum value.
 */
Void BaseControl::setMaximumValue( SInt32 value ) {
	if( control != NULL ) {
		::SetControl32BitMaximum( control, value );
    } else {
    	throw Exception("Couldn't set control maximum value - control not loaded!");
    }
}

/**
 * Set maximum value of control
 *
 * @return Maximum value of control.
 */
SInt32 BaseControl::getMaximumValue() {
	if( control != NULL ) {
		return ::GetControl32BitMaximum( control );
    } else {
    	throw Exception("Couldn't get control minimum value - control not loaded!");
    }
}

/**
 * Set font style of control.
 */
OSErr BaseControl::setFontStyle( const ControlFontStyleRec* fontStyle ) {
	if( control != NULL ) {
		return ::SetControlFontStyle( control, fontStyle );
	} else {
		throw Exception( "Couldn't set font style for control - control not loaded!" );	
	}
}

/**
 * Set font style of control.
 */
Void BaseControl::keyboardFocus( ControlFocusPart part ) {
	if( control != NULL ) {
		::SetKeyboardFocus( getDialog()->getWindow(), control, part );
	} else {
		throw Exception( "Couldn't set keyboard focus for control - control not loaded!" );	
	}
}

/**
 * Get the title of the control.
 */
//Void BaseControl::getTitleCFString( ::CFStringRef *outString ) {
//	if( control != NULL ) {
//		if( ::CopyControlTitleAsCFString( control, outString ) != noErr ) {
//			throw Exception( "Couldn't get title of control" );
//		}
//	} else {
//		throw Exception( "Couldn't get control title - control not loaded!" );
//	}
//}

/**
 * Set title of the control.
 */
//Void BaseControl::setTitleCFString( ::CFStringRef inString ) {
//	if( control != NULL ) {
//		if( ::SetControlTitleWithCFString( control, inString ) != noErr ) {
//			throw Exception( "Couldn't set title of control" );
//		}
//	} else {
//		throw Exception( "Couldn't set control title - control not loaded!" );
//	}
//}