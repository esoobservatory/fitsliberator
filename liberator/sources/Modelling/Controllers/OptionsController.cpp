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
#include "OptionsController.h"

using FitsLiberator::Modelling::OptionsController;
using FitsLiberator::Modelling::AccumulatingChangeManager;
using FitsLiberator::Modelling::OptionsModel;

using namespace FitsLiberator::Engine;

OptionsController::OptionsController(OptionsModel& m, 
                                     AccumulatingChangeManager* manager)
  : super(manager), model(m) {
}

void
OptionsController::Defaults() {
    model.Defaults();
    SendNotifications();
}


void
OptionsController::ClearHistory() {
    model.ClearHistory(true);
    SendNotifications();
}

void
OptionsController::MarkBaseline() {
	model.MarkBaseline();
}

void
OptionsController::Revert() {
	model.Revert();
	SendNotifications();
}

void
OptionsController::setGuessMethod(int index) {
    if(index >= 0 && index < 3) {
        model.GuessMethod(index);
        SendNotifications();
    }
}

void
OptionsController::setBlackLevelPercentage(double percentage) {
    if(percentage >= 0.0 && percentage <= 1.0) {
        model.BlackLevelPercentage(percentage);
        SendNotifications();
    }
}

void
OptionsController::setWhiteLevelPercentage(double percentage) {
    if(percentage >= 0.0 && percentage <= 1.0) {
        model.WhiteLevelPercentage(percentage);
        SendNotifications();
    }
}

void
OptionsController::setDefaultStretch(int index) {
    if(index >= 0 && index < 14) {
        model.DefaultStretch(index);
        SendNotifications();
    }
}

void
OptionsController::setScaledPeak(double value) {
    model.ScaledPeak(value);
    SendNotifications();
}

void OptionsController::toggleApplyStretchDirectly()
{
	if ( model.ApplyStretchDirectly() )
		model.ApplyStretchDirectly(false);
	else
		model.ApplyStretchDirectly(true);
	SendNotifications();
}

void
OptionsController::toggleMarker(unsigned int marker) {
    unsigned int setting = model.HistogramMarkers();
    if(setting & marker) {
        // Set, so clear it
        setting = setting & ~marker;
    } else {
        // Not set, so set it
        setting = setting | marker;
    }
    model.HistogramMarkers(setting);
    SendNotifications();
}
