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

#include "MacGlobalSettingsView.h"

using namespace FitsLiberator::Mac;

/**
 * Consutrctor
 *
 * @param dlog	Dialog in which the view's gui components resides.	
 * @param m		Model of the view.
 * @param c		Controller of the view.
 */
MacGlobalSettingsView::MacGlobalSettingsView( BaseDialog *dlog, FitsLiberator::Modelling::GlobalSettingsModel& m , FitsLiberator::Modelling::GlobalSettingsController& c ) 
: GlobalSettingsView( m, c ) {
	static const ::EventTypeSpec eventTypes[] = { 
		{ kEventClassCommand, kEventCommandProcess }		
	};
	
	previewCheckbox 	= new BaseControl( dlog, kFITSUICheckboxPreview );
	freezeCheckbox		= new BaseControl( dlog, kFITSUICheckboxFreeze );
	undefinedCheckbox	= new BaseControl( dlog, kFITSUICheckboxUndefined );
	whiteCheckbox		= new BaseControl( dlog, kFITSUICheckboxWhite );
	blackCheckbox		= new BaseControl( dlog, kFITSUICheckboxBlack );
	imageInformationRadio = new RadioGroupControl( dlog, kFITSUIRadioImageInformation );
	
	previewCheckbox->installEventHandler( eventTypes, GetEventTypeCount( eventTypes ), this );
	freezeCheckbox->installEventHandler( eventTypes, GetEventTypeCount( eventTypes ), this );
	undefinedCheckbox->installEventHandler( eventTypes, GetEventTypeCount( eventTypes ), this );
	whiteCheckbox->installEventHandler( eventTypes, GetEventTypeCount( eventTypes ), this );
	blackCheckbox->installEventHandler( eventTypes, GetEventTypeCount( eventTypes ), this );
	imageInformationRadio->installEventHandler( eventTypes, GetEventTypeCount( eventTypes ), this );	
}

/**
 * Desctructor deallocates controls.
 */
MacGlobalSettingsView::~MacGlobalSettingsView() {
	if( previewCheckbox != NULL ) 	{ delete previewCheckbox; }
	if( freezeCheckbox != NULL ) 	{ delete freezeCheckbox; }
	if( undefinedCheckbox != NULL )	{ delete undefinedCheckbox; }
	if( whiteCheckbox != NULL ) 	{ delete whiteCheckbox; }
	if( blackCheckbox != NULL ) 	{ delete blackCheckbox; }
	if( imageInformationRadio != NULL) { delete imageInformationRadio; }
}

/**
 * Event handler for GlobalSettingsView. Dispatches events to
 * GlobalSettingsController.
 */
OSStatus MacGlobalSettingsView::onCommand( HICommand *command ) {
	OSStatus result = eventNotHandledErr;

	switch( command->commandID ) {
		case kFITSUICommandTogglePreview:
			controller.togglePreviewEnabled();
			result = noErr;
			break;
			
		case kFITSUICommandToggleFreeze:
			controller.toggleFreezeSettings();
			result = noErr;
			break;
		
		case kFITSUICommandToggleMarkUndefined:
			controller.toggleMarkUndefined();
			result = noErr;
			break;
			
		case kFITSUICommandToggleMarkWhite:
			controller.toggleMarkWhiteClipping();
			result = noErr;
			break;
			
		case kFITSUICommandToggleMarkBlack:
			controller.toggleMarkBlackClipping();
			result = noErr;
			break;
		
		case kFITSUICommandToggleImageInformation:
			Int selectedIndex = imageInformationRadio->getValue() - 1;
		
			switch( selectedIndex ) {
				case FitsLiberator::Modelling::valuesStretched:
					controller.setImageInformation( FitsLiberator::Modelling::valuesStretched );
					break;
				case FitsLiberator::Modelling::valuesScaled:
					controller.setImageInformation( FitsLiberator::Modelling::valuesScaled );
					break;
			}
			result = noErr;
			break;			
	}
	
	return result;
}

/**
 * Update preview check box
 *
 * @param b True preview check box should be check marked, false if not.
 */
Void MacGlobalSettingsView::updatePreviewEnabled( Bool b ) {
	previewCheckbox->setValue( (b ? kControlCheckBoxCheckedValue : kControlCheckBoxUncheckedValue ) );
}

/**
 * Update freeze settings check box
 *
 * @param b True if freeze settings check box should be check marked, false if not.
 */
Void MacGlobalSettingsView::updateFreezeSettings( Bool b ) {
	freezeCheckbox->setValue( (b ? kControlCheckBoxCheckedValue : kControlCheckBoxUncheckedValue ) );
}

/**
 * Update mark undefined check box
 *
 * @param b True if mark undefined check box should be check marked, false if not.
 */
Void MacGlobalSettingsView::updateMarkUndefined( Bool b ) {
	undefinedCheckbox->setValue( (b ? kControlCheckBoxCheckedValue : kControlCheckBoxUncheckedValue ) );
}

/**
 * Update mark white clipping check box
 *
 * @param b True if mark white clipping check box should be check marked, false if not.
 */
Void MacGlobalSettingsView::updateMarkWhiteClipping( Bool b ) {
	whiteCheckbox->setValue( (b ? kControlCheckBoxCheckedValue : kControlCheckBoxUncheckedValue ) );
}

/**
 * Update mark black clipping check box
 *
 * @param b True if mark black clipping check box should be check marked, false if not.
 */
Void MacGlobalSettingsView::updateMarkBlackClipping( Bool b ) {
	blackCheckbox->setValue( (b ? kControlCheckBoxCheckedValue : kControlCheckBoxUncheckedValue ) );
}

/**
 * Update image information radio control.
 *
 * @param setting Which setting that is the choosen one.
 */
Void MacGlobalSettingsView::updateImageInformation( FitsLiberator::Modelling::ImageInformationSettings setting ) {
	imageInformationRadio->setValue( setting + 1 );
}