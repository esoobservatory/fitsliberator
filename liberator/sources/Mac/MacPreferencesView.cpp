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


#include "MacPreferencesView.h"
#include "FitsDialog.h"
#include "Environment.h"

using FitsLiberator::Environment;
using namespace FitsLiberator::Mac;

#define deleteControl(c) { if(c != NULL) { delete c; c = NULL; } }

class PreferencesDialog : public PhotoshopDialog, public EventHandler {
	bool exitState;
public:
	PreferencesDialog(BundleFactory* bundleFactory) : PhotoshopDialog(), exitState(false) {
		create( bundleFactory, CFSTR( kFITSNibFile ), CFSTR( kFITSNibWindowPreferences ) );
		
		static const ::EventTypeSpec eventTypes[] = {
			{ kEventClassCommand, kEventCommandProcess },
		};
		installEventHandler( eventTypes, GetEventTypeCount( eventTypes ), this );	
	}
	
	OSStatus onCommand( ::HICommand *command ) {
		OSStatus result = eventNotHandledErr;
		
		switch( command->commandID ) {
			case kFITSUICommandOptionsOk:
				isCancelled(false);
			case kFITSUICommandOptionsCancel:
				quitModalEventLoop();
				result = noErr;
				break;
		}
		
		return result;
	}
	
	/** Sets whether the user pressed cancel. */
	void isCancelled(bool pressedCancel) {
		exitState = !pressedCancel;
	}
	
	/** Gets whether the user pressed cancel.
		@return True if the user presed cancel, false otherwise. */
	bool isCancelled() {
		return !exitState;
	}
};

MacPreferencesView::MacPreferencesView(BaseDialog* dialog, const OptionsModel& model, OptionsController& controller) 
  : OptionsView(model, controller), mainDialog(dialog) {
	  
	method = blackLevel = whiteLevel = NULL;
	function = scaledPeakLevel = NULL;
    showZero = showBackground = showPeak = NULL;
    showMean = showStretchedMean = showScaledMean = NULL;
    showMedian = showStretchedMedian = showScaledMedian = NULL;
	applyStrechDirectly = restoreDefaults = clearHistory = NULL; 
	  
	static const EventTypeSpec buttonEventTypes[] = {
		{ kEventClassCommand, kEventCommandProcess }
	};
	
    preferencesButton = new BaseControl(dialog, kFITSUIButtonPreferences);
	preferencesButton->installEventHandler(buttonEventTypes, GetEventTypeCount( buttonEventTypes ), this );
}

MacPreferencesView::~MacPreferencesView() {
	deleteControl(preferencesButton);
	destroyPreferencesControls();
}

OSStatus MacPreferencesView::onCommand( HICommand* command ) {
	OSStatus result = noErr;
	
	switch (command->commandID) {
		case kFITSUICommandPreferences:
			showPreferencesDialog();
			break;
		case kFITSUICommandMenuOptionsPercentages:
			controller.setGuessMethod(0);
			break;
		case kFITSUICommandMenuOptionsMedian:
			controller.setGuessMethod(1);
			break;
		case kFITSUICommandMenuOptionsMean:
			controller.setGuessMethod(2);
			break;
		case kFITSUICommandOptionsClearHistory:
			controller.ClearHistory();
			break;
		case kFITSUICommandOptionsRestoreDefault:
			controller.Defaults();
			break;
		case kFITSUICommandOptionsToggleGrid:
			controller.toggleMarker(OptionsModel::MarkerGrid);
			break;
		case kFITSUICommandOptionsToggleZero:
			controller.toggleMarker(OptionsModel::MarkerZero);
			break;
		case kFITSUICommandOptionsToggleBack:
			controller.toggleMarker(OptionsModel::MarkerBackground);
			break;
		case kFITSUICommandOptionsTogglePeak:
			controller.toggleMarker(OptionsModel::MarkerPeak);
			break;
        case kFITSUICommandOptionsToggleMean:
            controller.toggleMarker(OptionsModel::MarkerMean);
            break;
        case kFITSUICommandOptionsToggleStretchedMean:
            controller.toggleMarker(OptionsModel::MarkerStretchedMean);
            break;
        case kFITSUICommandOptionsToggleScaledMean:
            controller.toggleMarker(OptionsModel::MarkerScaledMean);
            break;
        case kFITSUICommandOptionsToggleMedian:
            controller.toggleMarker(OptionsModel::MarkerMedian);
            break;
        case kFITSUICommandOptionsToggleStretchedMedian:
            controller.toggleMarker(OptionsModel::MarkerStretchedMedian);
            break;
        case kFITSUICommandOptionsToggleScaledMedian:
            controller.toggleMarker(OptionsModel::MarkerScaledMedian);
            break;
		case kFITSUICommandOptionsToggleApplyValues:
            controller.toggleApplyStretchDirectly();
            break;
		default:
			if(command->commandID >= kFITSUICommandOptionsStretchFirst && command->commandID <= kFITSUICommandOptionsStretchLast) {
				OnStretchSelected(command->commandID - kFITSUICommandOptionsStretchFirst);
			} else {
				result = eventNotHandledErr;
			}
			break;
	}
		
	return result;
}

OSStatus MacPreferencesView::onSetFocusPart( EventRef event ) {
	if( method == 0 ) return eventNotHandledErr;	// No controls created...

	// Apply the new values when a textbox loses focus.
	
	ControlRef ctrl;
	ControlRef focusCtrl;
	
	::GetKeyboardFocus( blackLevel->getDialog()->getWindow(), &focusCtrl );
	::GetEventParameter( event, ::kEventParamDirectObject, typeControlRef, NULL, sizeof (ControlRef), NULL, &ctrl );
	
	if( ctrl == focusCtrl ) {
		if( ctrl == blackLevel->getControl() ) {
			controller.setBlackLevelPercentage(blackLevel->getDouble() / 100.0);
		} else if( ctrl == whiteLevel->getControl() ) {
			controller.setWhiteLevelPercentage(whiteLevel->getDouble() / 100.0);
		} else if( ctrl == scaledPeakLevel->getControl() ) {
			controller.setScaledPeak(scaledPeakLevel->getDouble());
		} 
	} 
	
	// Let the standard event handler, handle the setting of focus.
	return eventNotHandledErr;
}

OSStatus MacPreferencesView::onMouseEntered( EventRef event ) {
	::SetThemeCursor( kThemeIBeamCursor  );
	
	return noErr;
}

OSStatus MacPreferencesView::onMouseExited( EventRef event ) {
	::SetThemeCursor( kThemeArrowCursor  );
	
	return noErr;
}

#define CHK(ctl, value) (ctl)->setValue((value) ? kControlCheckBoxCheckedValue : kControlCheckBoxUncheckedValue)

void MacPreferencesView::Update() {
	if( method == 0 ) // Test one of the controls to see if the dialog has been created.
		return;
	
	method->setValue(model.GuessMethod() + 1);
	blackLevel->setDouble(model.BlackLevelPercentage() * 100.0);
	whiteLevel->setDouble(model.WhiteLevelPercentage() * 100.0);
	if(model.EditableLevels()) {
		blackLevel->enable();
		whiteLevel->enable();
	} else {
		blackLevel->disable();
		whiteLevel->disable();
	}
	
	function->setValue(getStretchFunctionIndex() + 1);
	scaledPeakLevel->setDouble(model.ScaledPeak());
	
	unsigned int markers = model.HistogramMarkers();
    CHK(showZero,               markers & OptionsModel::MarkerZero);
	CHK(showBackground,         markers & OptionsModel::MarkerBackground);
    CHK(showPeak,               markers & OptionsModel::MarkerPeak);
    CHK(showMean,               markers & OptionsModel::MarkerMean);
    CHK(showStretchedMean,      markers & OptionsModel::MarkerStretchedMean);
    CHK(showScaledMean,         markers & OptionsModel::MarkerScaledMean);
    CHK(showMedian,             markers & OptionsModel::MarkerMedian);
    CHK(showStretchedMedian,    markers & OptionsModel::MarkerStretchedMedian);
    CHK(showScaledMedian,       markers & OptionsModel::MarkerScaledMedian);
	CHK(applyStrechDirectly,    model.ApplyStretchDirectly() );
	
	if(model.ClearHistory()) {
		clearHistory->disable();
	} else {
		clearHistory->enable();
	}
}

void MacPreferencesView::createPreferencesControls(BaseDialog* dialog) {
	method			= new BaseControl(dialog, kFITSUIPopupButtonGuessMethod);
	blackLevel		= new NumericTextControl(dialog, kFITSUIEditTextBlackPercentage);
	whiteLevel		= new NumericTextControl(dialog, kFITSUIEditTextWhitePercentage);
	
	function		= new BaseControl(dialog, kFITSUIPopupButtonDefaultFunction);
	scaledPeakLevel	= new NumericTextControl(dialog, kFITSUIEditTextAutoScaleLevel);
	
	showZero            = new BaseControl(dialog, kFITSUICheckboxShowZero);
	showBackground      = new BaseControl(dialog, kFITSUICheckboxShowBackground);
	showPeak            = new BaseControl(dialog, kFITSUICheckboxShowPeak);
    showMean            = new BaseControl(dialog, kFITSUICheckboxShowMean);
    showStretchedMean   = new BaseControl(dialog, kFITSUICheckboxShowStretchedMean);
    showScaledMean      = new BaseControl(dialog, kFITSUICheckboxShowScaledMean);
    showMedian          = new BaseControl(dialog, kFITSUICheckboxShowMedian);
    showStretchedMedian = new BaseControl(dialog, kFITSUICheckboxShowStretchedMedian);
    showScaledMedian    = new BaseControl(dialog, kFITSUICheckboxShowScaledMedian);
	applyStrechDirectly = new BaseControl(dialog, kFITSUICheckboxApplyValues);
	
	
	restoreDefaults = new BaseControl(dialog, kFITSUIButtonRestoreDefaults);
	clearHistory	= new BaseControl(dialog, kFITSUIButtonClearHistory);
	
	static const EventTypeSpec buttonEventTypes[] = {
		{ kEventClassCommand, kEventCommandProcess }
	};
	
	dialog->installEventHandler(buttonEventTypes, GetEventTypeCount(buttonEventTypes), this);
	
	static const ::EventTypeSpec editTextEventTypes[] = {
		{ kEventClassControl, kEventControlSetFocusPart },
		{ kEventClassMouse, kEventMouseEntered },
		{ kEventClassMouse, kEventMouseExited }
	};	
	
	blackLevel->setPrecision(0);
	whiteLevel->setPrecision(0);
	blackLevel->installEventHandler(editTextEventTypes, GetEventTypeCount(editTextEventTypes), this);
	whiteLevel->installEventHandler(editTextEventTypes, GetEventTypeCount(editTextEventTypes), this);
	scaledPeakLevel->installEventHandler(editTextEventTypes, GetEventTypeCount(editTextEventTypes), this);	
}

void MacPreferencesView::destroyPreferencesControls() {
	deleteControl(method);
	deleteControl(blackLevel);
	deleteControl(whiteLevel);
	deleteControl(function);
	deleteControl(scaledPeakLevel);
	deleteControl(showZero);
	deleteControl(showBackground);
	deleteControl(showPeak);
    deleteControl(showMean);
    deleteControl(showStretchedMean);
    deleteControl(showScaledMean);
    deleteControl(showMedian);
    deleteControl(showStretchedMedian);
    deleteControl(showScaledMedian);
	deleteControl(applyStrechDirectly);
	deleteControl(restoreDefaults);
	deleteControl(clearHistory);
}

void MacPreferencesView::showPreferencesDialog() {
	PreferencesDialog dialog(Environment::getBundleFactory());
	
	createPreferencesControls(&dialog);
	Update();
	controller.MarkBaseline();	// In case the user decides to throw away the changes revert to the current state
	
	dialog.show();
	dialog.runModalEventLoop();
	destroyPreferencesControls();
	dialog.hide();
	
	if(dialog.isCancelled()) {
		controller.Revert();
	}
	
	dialog.dispose();
}
