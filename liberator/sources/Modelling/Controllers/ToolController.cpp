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
#include "ToolController.h"

using namespace FitsLiberator::Modelling;

ToolController::ToolController( ToolModel& tool, AccumulatingChangeManager* chman ): ACMController( chman ), toolModel(tool), lastUsedTool( kFITSToolHand )
{
}


ToolController::~ToolController()
{
}

/**
	This method is called when the user selects the zoom-button
	Should update the ToolModel and...?
*/
Void ToolController::toolZoomSelected()
{
	toolModel.setCurrentState(  kFITSToolMagnifier );
	SendNotifications();
}

Void ToolController::toolBackgroundSelected()
{
	toolModel.setCurrentState( kFITSToolEyedropperBackground );
	SendNotifications();
}

Void ToolController::toolPeakSelected()
{
	toolModel.setCurrentState( kFITSToolEyedropperPeak );
	SendNotifications();
}

Void ToolController::toolBlackLevelSelected()
{
	toolModel.setCurrentState( kFITSToolEyedropperBlackLevel );
	SendNotifications();
}
	
Void ToolController::toolWhiteLevelSelected()
{
	toolModel.setCurrentState( kFITSToolEyedropperWhiteLevel );
	SendNotifications();
}

/**
 * Change the function of the current tool.
 *
 * @param value Set to true if option is pressed, false if not.
 */
Void ToolController::changeToolFunction( Bool value ) {
	toolModel.setOptionState( value );
	SendNotifications();
}

/**
 * Change the function of the current tool.
 */
Bool ToolController::keyboardShortCut( Char key ) {
	Bool res = false;

	switch( key ) {
		case kFITSHandToolShortcut:
		case kFITSHandToolShortcut + 32:
			toolModel.setCurrentState( kFITSToolHand );
			res = true;
			break;
		
		case kFITSMagnifierToolShortcut:
		case kFITSMagnifierToolShortcut + 32:
			toolModel.setCurrentState( kFITSToolMagnifier );
			res = true;
			break;
		
		case kFITSBackgroundToolShortcut:
		case kFITSBackgroundToolShortcut + 32:
			toolModel.setCurrentState( kFITSToolEyedropperBackground );
			res = true;
			break;
		
		case kFITSWhiteLevelToolShortcut:	
		case kFITSWhiteLevelToolShortcut + 32:
			toolModel.setCurrentState( kFITSToolEyedropperWhiteLevel );
			res = true;
			break;
		
		case kFITSBlackLevelToolShortcut:	
		case kFITSBlackLevelToolShortcut + 32:
			toolModel.setCurrentState( kFITSToolEyedropperBlackLevel );
			res = true;
			break;
		
		case kFITSPeakToolShortcut:	
		case kFITSPeakToolShortcut + 32:
			toolModel.setCurrentState( kFITSToolEyedropperPeak );
			res = true;
			break;
		
		case kFITSSpacebar:
			lastUsedTool = toolModel.getCurrentState();
			toolModel.setCurrentState( kFITSToolHand );
			res = true;
			break;
	}

	if( res == true ) {
		SendNotifications();	
	}
	
	return res;
}

/**
 * Change the function of the current tool.
 */
Bool ToolController::keyboardShortCutUp( Char key ) {
	Bool res = false;

	switch( key ) {
		case kFITSSpacebar:
			toolModel.setCurrentState( lastUsedTool );
			res = true;
			break;
	}

	if( res == true ) {
		SendNotifications();	
	}
	
	return res;
}

/**
 This method is called when the user selects the hand button 

*/
Void ToolController::toolHandSelected()
{
	toolModel.setCurrentState( kFITSToolHand );
	SendNotifications();
}

