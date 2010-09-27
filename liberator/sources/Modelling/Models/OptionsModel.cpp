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
#include "OptionsModel.h"

using FitsLiberator::Modelling::OptionsModel;
using FitsLiberator::Modelling::ChangeManager;

using namespace FitsLiberator::Engine;

OptionsModel::OptionsModel(ChangeManager* manager)
  : super(manager) {
    clearHistory = false;
}

void
OptionsModel::Defaults() {
    GuessMethod((int)kFITSDefaultGuess);
    BlackLevelPercentage(kFITSInitialGuessMinPercent);
    WhiteLevelPercentage(kFITSInitialGuessMaxPercent);
    DefaultStretch((int)kFITSDefaultStretch);
    ScaledPeak(kFITSDefaultRescaleFactor);
    HistogramMarkers(kFITSDefaultHistMarkers);
	ApplyStretchDirectly(true);

}

void
OptionsModel::MarkBaseline() {
    initialState = currentState;
}

void OptionsModel::Revert() {
    currentState = initialState;
    Notify();
}

int
OptionsModel::GuessMethod() const {
    return currentState.guess;
}

void
OptionsModel::GuessMethod(int index) {
    currentState.guess = index;
    Notify();
}

bool
OptionsModel::EditableLevels() const {
    return currentState.guess == (int)guessPercentage;
}

double
OptionsModel::BlackLevelPercentage() const {
    return currentState.black;
}

void
OptionsModel::BlackLevelPercentage(double percentage) {
    currentState.black = percentage;
    Notify();
}

double
OptionsModel::WhiteLevelPercentage() const {
    return currentState.white;
}

void
OptionsModel::WhiteLevelPercentage(double percentage) {
    currentState.white = percentage;
    Notify();
}

int
OptionsModel::DefaultStretch() const {
    return currentState.stretch;
}

void
OptionsModel::DefaultStretch(int index) {
    currentState.stretch = index;
    Notify();
}

double
OptionsModel::ScaledPeak() const {
    return currentState.scaledPeak;
}

void
OptionsModel::ScaledPeak(double value) {
    currentState.scaledPeak = value;
    Notify();
}



unsigned int
OptionsModel::HistogramMarkers() const {
    return currentState.markers;
}

void
OptionsModel::HistogramMarkers(unsigned int value) {
    currentState.markers = value;
    Notify();
}

bool
OptionsModel::ClearHistory() const {
    return clearHistory;
}

void 
OptionsModel::ClearHistory(bool) {
    clearHistory = true;
    Notify();
}

bool OptionsModel::ApplyStretchDirectly() const
{
	return currentState.applyStretchDirectly;
}

void OptionsModel::ApplyStretchDirectly(bool app)
{
	currentState.applyStretchDirectly = app;
	Notify();
}