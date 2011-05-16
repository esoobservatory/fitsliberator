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

#define kFITSDefaultDecimalSeparator    "."     // Default decimal separator
#define kFITSDefaultThousandsSeparator  ","     // Default thousands separator
#define kFITSDefaultNegativeSign        "-"     // Default negative sign
#define kFITSDefaultPositiveSign        "+"     // Default posisitve sign
#define kFITSDefaultPrecision           2       // Default numeric precision
#define kFITSDefaultUnfixedPrecision    14      // Default unfixed numeric precision
#define kFITSMaxPrecision               14      // Maximum numeric precision
#define kFITSUnitSeparator              " "     // Separator between number and unit
#define kFITSSmallestRegularNumber      0.01    // Numbers smaller than this are displayed in scientific notation
#define kFITSLargestRegularNumber       99999   // Numbers larger than this are displayed in scientific notation

#define kFITSBackgroundGuessMedian		true
#define kFITSDefaultShowPreview         true    // Is preview on by default
#define kFITSDefaultFreezeSettings      false   // Are setting freezed by default
#define kFITSDefaultShowUndefined       false    // Is the undefined mask shown by default
#define kFITSDefaultShowWhiteClipped    false
#define kFITSDefaultShowBlackClipped    false
#define kFITSDefaultImageInformation	FitsLiberator::Modelling::valuesStretched
#define kFITSDefaultHistMarkers         2       // Show zero in histogram
#define kFITSDefaultStretch             stretchLinear
#define kFITSDefaultGuess               guessPercentage
#define kFITSDefaultAutoScaling         10
#define kFITSDefaultShow
#define kFITSDefaultBitDepth            channel16
#define kFITSDefaultUndefined           undefinedBlack
#define kFITSDefaultTool                kFITSToolHand
#define kFITSBigIncrementFactor         10.0    // When shift+clicking the up/down arrows on a textbox, the increment is multiplied by this factor
#define kFITSHandToolShortcut           'H'     //
#define kFITSMagnifierToolShortcut      'Z'
#define kFITSBackgroundToolShortcut     'B'
#define kFITSPeakToolShortcut     		'P'
#define kFITSWhiteLevelToolShortcut     'W'
#define kFITSBlackLevelToolShortcut     'S'
#define kFITSFitInPreviewShortcut       'F'
#define kFITSActualPixelsShortcut       'A'
#define kFITSCenterPreviewShortcut      'C'
#define kFITSCancelOperationShortcut	27
#define kFITSIncrementZoomShortcut      '+'
#define kFITSDecrementZoomShortcut      '-'
#define kFITSSpacebar					32

#define kFITSDragMarginX                4      // Horizontal distance the mouse needs to be dragged for the drag to be recognised
#define kFITSDragMarginY                4      // Vertical distance the mouse needs to be dragged for the drag to be recognised
#define kFITSHistogramCursorColor       RGB( 0x56, 0x74, 0xB9 ) // Light blue
#define kFITSHistogramColor             RGB( 0x81, 0x81, 0x81 ) // 50% Gray
#define kFITSHistogramSliderColor       RGB( 0, 0, 0 )          // Black

#define KFITSNumberOfLoadedFiles		100		// Defines the maximum number of loaded files to store in the settings file.

#define kFITSBackgroundGuessWidth       10      // Defines the size of the area used for guessing the background level
#define kFITSBackgroundGuessHeight      10

#define kFITSInitialGuessMinPercent		0.05	
#define kFITSInitialGuessMaxPercent		0.90

/*
#define kFITSInitialGuessMedianPMStdev	false	// defines the initial guess to be median +/- stdev
#define kFITSInitialGuessMeanPMStdev	false	// defines the initial guess to be mean +/- stdev
#define kFITSInitialGuessHisThreePer	true	// defines the initial guess to be min=kFITSInitialGuessMinPercent% of pixel vals and max = kFITSInitialGuessMaxPercent% of pixel vals.
*/
#define kFITSHistogramBins				1000000	//defines the number of bins in the raw histogram

#define kFITSDefaulBackgroundScale		0		//defines the value of the default background scale.

//#define kFITSCacheMaxMemUsage		   1500000000
//#define kFITSCacheMaxMemUsage			524288000
//#define kFITSCacheMaxMemUsage		   1073741824
//#define kFITSCacheMaxMemUsage			104857600.
//#define kFITSCacheMaxMemUsage			209715200 // defines the highest amount of memory the cache is allowed to use
//#define kFITSCacheMaxMemUsage			1000000
#define kFITSDefaultRescaleFactor		10.0	//defines the default rescale factor, so the image will lie in the range [1:10]
#ifdef WINDOWS
#define kFITSProgressDelay              1000    // The number of milliseconds an operation needs to take before the progress bar is shown.=======
#else
#define kFITSProgressDelay              60    	// The number of 1/60 an operation needs to take before the progress bar is shown.=======
#endif
#define kFITSStretchNumberOfFunctions	15

#define kFITSDoCache					true	//	 defines whether we should use caching or not.
