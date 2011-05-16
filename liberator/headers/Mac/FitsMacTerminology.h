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

#ifndef __FITSMACTERMINOLOGY_H__
#define __FITSMACTERMINOLOGY_H__

//-------------------------------------------------------------------------------
//	Application definitions
//-------------------------------------------------------------------------------
#define kFITSApplicationSignature	 '8BIF'
#define kFITSNibFile				 "FITSLiberator2GUI"
#define kFITSNibWindow				 "MainWindow"
#define kFITSNibMainMenu             "MainMenu"
#define kFITSNibWindowProgress		 "ProgressWindow"
#define kFITSNibWindowAbout			 "AboutWindow"
#define kFITSNibWindowCategoryEditor "CategoryEditor"
#define kFITSNibPreviewMenu			 "PreviewPopupMenu"
#define kFITSNibHistogramMenu		 "HistogramPopupMenu"
#define kFITSNibHeaderMenu			 "HeaderPopupMenu"
#define kFITSNibWindowPreferences	 "PreferencesWindow"

//-------------------------------------------------------------------------------
//	GUI components definitions
//-------------------------------------------------------------------------------
/**
 * Control IDs for main dialog. MUST corrosponds with the IDs in the NIB file. 
 */
enum {
	kFITSUIButtonOk					= 1,
	kFITSUIButtonCancel				= 2,
	kFITSUIButtonReset				= 3,
	kFITSUICheckboxPreview			= 4,
	kFITSUICheckboxFreeze			= 5,
	kFITSUICheckboxUndefined		= 6,
	kFITSUICheckboxWhite			= 7,
	kFITSUICheckboxBlack			= 8,
	kFITSUITabControl				= 9,
	kFITSUICheckboxFlip				= 10,
    kFITSUIIconNetworkWarning       = 11,
	kFITSUIButtonOpenFile			= 12,
	kFITSUIButtonSaveFile			= 13,
	kFITSUIButtonSaveAndEdit		= 14,
	kFITSUIButtonAbout				= 15,
	
	// Preview tab pane
	kFITSUITabPanePreview			= 100,
	kFITSUIBevelButtonHand			= 101,
	kFITSUIBevelButtonZoom			= 102,
	kFITSUIBevelButtonBackground	= 103,
	
	
	kFITSUICustomPreview			= 104,
	kFITSUIPopupButtonZoom			= 105,
	kFITSUIProgress					= 106,
	kFITSUICustomHistogram			= 107,
	kFITSUIEditTextBlack			= 108,
	kFITSUIEditTextWhite			= 109,
	kFITSUIPopupButtonPlane			= 110,
	kFITSUIFrameStretchFunction		= 113,
	kFITSUITextX					= 116,
	kFITSUITextY					= 117,
	kFITSUITextStretched			= 118,
	kFITSUITextWidth				= 119,
	kFITSUITextHeight				= 120,
	kFITSUITextMinReal				= 121,
	kFITSUITextMaxReal				= 122,
	kFITSUITextMeanReal				= 123,
	kFITSUITextMedianReal			= 124,
	kFITSUITextSTDEVReal			= 125,
	kFITSUITextMinStretched			= 126,
	kFITSUITextMaxStretched			= 127,
	kFITSUITextReal					= 128,
	kFITSUITextMeanStretched		= 129,
	kFITSUITextMedianStretched		= 130,
	kFITSUITextSTDEVStretched		= 131,
	kFITSUIPopupButtonFunction		= 132,
	kFITSUIEditTextBackground		= 133,
	kFITSUIButtonAutoScaling		= 134,
	kFITSUIEditTextPeak				= 135,
	kFITSUIButtonAutoPeak			= 136,
	kFITSUIRadioChannels			= 137,
	kFITSUIRadioTransparent			= 138,
	
	kFITSUIBevelButtonWhiteLevel	= 139,
	kFITSUIBevelButtonBlackLevel	= 140,
	kFITSUIArrowsBlackLevel			= 141,
	kFITSUIArrowsWhiteLevel			= 142,
	kFITSUITextHistogramMin			= 143,
	kFITSUITextHistogramMax			= 144,
	kFITSUICustomHistogramSlider	= 145,
	kFITSUICustomLogo				= 146,
	kFITSUIBevelButtonMinus			= 147,
	kFITSUIBevelButtonPlus			= 148,
	
	kFITSUIComboScaledPeakLevel		= 150,
	kFITSUIBevelButtonPeak			= 151,
	kFITSUIRadioImageInformation	= 152,
	kFITSUITextStretchedScaled		= 153,
	kFITSUITextPixelStretchedScaled	= 154,
	kFITSUIBevelButtonAutoBackground = 155,
	kFITSUIBevelButtonAutoPeak		= 156,
	kFITSUIButtonPreferences		= 157,
	kFITSUITextRa					= 158,
	kFITSUITextDec					= 159,
	
	kFITSUIButtonApplyValues		= 160,
	
	// Virtual Repository tab pane
	kFITSUITabPaneVR				= 200,
	kFITSUIDataBrowser				= 201,
	kFITSUIComboBoxValueEdit		= 202,
	kFITSUIEditTextValueEdit		= 203,
	kFITSUIPopupButtonValueEdit		= 204,
	kFITSUIButtonCategory			= 205,
	
	// FITS Header tab pane
	kFITSUITabPaneHeader			= 300,
	kFITSUITextViewHeader			= 301,
	kFITSUIScrollViewHeader			= 302,
	
	// Help Guide tab pane
	kFITSUITabPaneHelpGuide			= 400,
	kFITSUITextViewHelpGuide		= 401,
	kFITSUIScrollViewHelpGuide		= 402,
	
	// Preferences dialog
	kFITSUIButtonSaveOptions		= 501,
	kFITSUIButtonCancelOptions		= 502,
	kFITSUIPopupButtonGuessMethod	= 503,
	kFITSUIEditTextBlackPercentage	= 504,
	kFITSUIEditTextWhitePercentage	= 505,
	kFITSUIPopupButtonDefaultFunction = 506,
	kFITSUIEditTextAutoScaleLevel	= 507,
	kFITSUICheckboxShowGrid			= 508,
	kFITSUICheckboxShowZero			= 509,
	kFITSUICheckboxShowBackground	= 510,
	kFITSUICheckboxShowPeak         = 511,
    kFITSUICheckboxShowMean         = 512,
    kFITSUICheckboxShowStretchedMean    = 513,
    kFITSUICheckboxShowScaledMean   = 514,
    kFITSUICheckboxShowMedian       = 515,
    kFITSUICheckboxShowStretchedMedian  = 516,
    kFITSUICheckboxShowScaledMedian = 517,
	kFITSUIButtonRestoreDefaults	= 518,
	kFITSUIButtonClearHistory		= 519,
	kFITSUICheckboxApplyValues		= 520
};

/**
 * Enum for about dialog.
 */
enum {
	kFITSUIPictAbout = 1,
	kFITSUITextVersion = 3
};

/**
 * Enum for progress window
 */
enum {
	kFITSUIProgressBar = 1
};

/**
 * Enum for category editor window
 */
enum {
	kFITSUICategorySelector	= 1,
	kFITSUICategoryBrowser 	= 2,
	kFITSUIButtonDone 		= 3
};


//-------------------------------------------------------------------------------
//	Databrowser definitions
//-------------------------------------------------------------------------------

/**
 * DataBrowser column descriptions as defined in the NIB file.
 */
enum {
	kFITSDataKeyword		= 'KEYW',
	kFITSDataDescription	= 'DESC',
	kFITSDataCategoryCheck	= 'CHBX',
	kFITSDataCategory		= 'CATE'
};

//-------------------------------------------------------------------------------
//	Cursor definitions
//-------------------------------------------------------------------------------

/**
 * Definitions of IDs for cursors.
 */
enum {
	kFITSUICursorNormal,
	kFITSUICursorMoveInactive,
	kFITSUICursorMoveActive,
	kFITSUICursorZoomIn,
	kFITSUICursorZoomOut,
	kFITSUICursorZoomLimit,
	kFITSUICursorPickerWhite,
	kFITSUICursorPickerBlack,
	kFITSUICursorPickerBackground,
	kFITSUICursorPickerPeak,
	kFITSUICursorCrossHair
};


#define CursorArrow 		kPICursorArrow				
#define CursorHand 			kPICursorHand
#define CursorGrab 			kPICursorGrab
#define CursorZoomIn 		kPICursorZoomIn
#define CursorZoomOut 		kPICursorZoomOut
#define CursorZoomLimit 	kPICursorZoomLimit
#define CursorEyedropper 	kPICursorEyedropper
#define CursorCrosshair 	kPICursorCrosshair

//-------------------------------------------------------------------------------
//	MouseTrackingRegion definitions
//-------------------------------------------------------------------------------
/**
 * Definition IDs for mouse tracking regions.
 */
enum {
	kFITSUIHistogramTrackingRegion 		= 1,
	kFITSUIPreviewTrackingRegion 		= 2,
	kFITSUIEditTextWhiteRegion 			= 3,
	kFITSUIEditTextBlackRegion 			= 4,
	kFITSUIEditTextBackgroundRegion		= 5,
	kFITSUIEditTextScaleRegion 			= 6,
	kFITSUIEditTextViewHeaderRegion		= 7,
	kFITSUIEditTextRescaleRegion		= 8
};

//-------------------------------------------------------------------------------
// Command definitions
//-------------------------------------------------------------------------------
#define kFITSUICommandOk					'FIok'
#define kFITSUICommandCancel				'FIno'
#define kFITSUICommandOpenFile				'BtOp'
#define kFITSUICommandSaveFile				'BtSv'
#define kFITSUICommandSaveAndEdit			'BtEd'
#define kFITSUICommandAbout					'BtAb'
#define kFITSUICommandReset					'BtRs'
#define kFITSUICommandPreferences			'BtPr'
#define kFITSUICommandTogglePreview			'TgPr'
#define kFITSUICommandToggleFlip			'TgFp'
#define kFITSUICommandToggleFreeze			'TgFr'
#define kFITSUICommandToggleMarkUndefined	'TgMU'
#define kFITSUICommandToggleMarkWhite		'TgMW'
#define kFITSUICommandToggleMarkBlack		'TgMB'
#define kFITSUICommandSelectStretchFunction	'SlSF'
#define kFITSUICommandSelectPlane			'SlPL'
#define kFITSUICommandUseHandTool			'SlHT'
#define kFITSUICommandUseZoomTool			'SlZT'
#define kFITSUICommandUsePickerTool			'SlPT'
#define kFITSUICommandUseWhiteLevelTool		'SlWT'
#define kFITSUICommandUseBlackLevelTool		'SlBT'
#define kFITSUICommandUseBackgroundTool		'SlBG'
#define kFITSUICommandUsePeakTool			'SlPL'
#define kFITSUICommandToggleImageInformation 'TgII'
#define kFITSUICommandToggleBitDepth		'TgRB'
#define kFITSUICommandToggleUndefined		'TgRU'
#define kFITSUICommandMenuZoom				'MzZo'
#define kFITSUICommandMenuZoomFit			'MzFt'
#define kFITSUICommandMenuZoom100			'MzAc'
#define kFITSUICommandMenuZoomIn			'MzIn'
#define kFITSUICommandMenuZoomOut			'MzOu'
#define kFITSUICommandMenuCentrePreview		'MzCe'
#define kFITSUICommandMenuShowAll			'MhSh'
#define kFITSUICommandMenuHistogramZoomIn	'MhZi'
#define kFITSUICommandMenuHistogramZoomOut	'MhZo'
#define kFITSUICommandMenuOptionsPercentages 'MoPe'
#define kFITSUICommandMenuOptionsMedian		'MoMd'
#define kFITSUICommandMenuOptionsMean		'MoMe'
#define kFITSUICommandOptionsOk				'BtOO'
#define kFITSUICommandOptionsCancel			'BtOC'
#define kFITSUICommandOptionsClearHistory	'BtCH'
#define kFITSUICommandOptionsRestoreDefault 'BtRD'
#define kFITSUICommandOptionsToggleGrid		'TgHG'
#define kFITSUICommandOptionsToggleZero		'TgHZ'
#define kFITSUICommandOptionsToggleBack		'TgHB'
#define kFITSUICommandOptionsTogglePeak     'TgHP'
#define kFITSUICommandOptionsToggleMean     'TgHM'
#define kFITSUICommandOptionsToggleStretchedMean 'TgSM'
#define kFITSUICommandOptionsToggleScaledMean  'TgZM'
#define kFITSUICommandOptionsToggleMedian   'TgHE'
#define kFITSUICommandOptionsToggleStretchedMedian 'TgSE'
#define kFITSUICommandOptionsToggleScaledMedian 'TgZE'
#define kFITSUICommandOptionsStretchFirst	'Fn00'
#define kFITSUICommandOptionsStretchLast	'Fn13'
#define kFITSUICommandMenuCopy				'FkCp'
#define kFITSUICommandMenuSelectAll			'FkSA'
#define kFITSUICommandMenuPaste				'FkPs'
#define kFITSUICommandMenuCut				'FkCu'
#define kFITSUICommandMenuClear				'FkCl'
#define kFITSUICommandAutomaticBackground	'StAB'
#define kFITSUICommandAutomaticPeak			'StAP'
#define kFITSUICommandAutomaticScaling		'StAS'
#define kFITSUICommandSelectMetaDataValue	'SlMD'
#define kFITSUICommandLoadCategoryEditor	'LdCE'
#define kFITSUICommandCategoryEditorDone	'CEdo'
#define kFITSUICommandSelectCategory		'SlCa'
#define kFITSUICommandApplyValues			'StAp'
#define kFITSUICommandOptionsToggleApplyValues 'TgAp'

//-------------------------------------------------------------------------------
//	Misc definitions
//-------------------------------------------------------------------------------
// Note: Unused! See FitsBehaviour instead.
#define kFITSUIUnitsPixels 					"px"
#define kFITSUIMenuIndexZoomIn				4
#define kFITSUIMenuIndexZoomOut				5

#endif //__FITSMACTERMINOLOGY_H__