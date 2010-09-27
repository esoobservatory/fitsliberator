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

#include "MacStatisticsView.h"

using namespace FitsLiberator::Mac;
using namespace FitsLiberator::Modelling;

/**
 *
 */
MacStatisticsView::MacStatisticsView( BaseDialog *dlog, StatisticsModel& m, GlobalSettingsModel& gm ) : FitsLiberator::Modelling::StatisticsView( m, gm ) {
	realMin 		= new NumericTextControl( dlog, kFITSUITextMinReal );
	realMin->setPrecision( kFITSDefaultPrecision );
	
	realMax 		= new NumericTextControl( dlog, kFITSUITextMaxReal );
	realMax->setPrecision( kFITSDefaultPrecision );
	
	realMean 		= new NumericTextControl( dlog, kFITSUITextMeanReal );
	realMean->setPrecision( kFITSDefaultPrecision );
	
	realMedian 		= new NumericTextControl( dlog, kFITSUITextMedianReal );
	realMedian->setPrecision( kFITSDefaultPrecision );
	
	realSTDEV 		= new NumericTextControl( dlog, kFITSUITextSTDEVReal );
	realSTDEV->setPrecision( kFITSDefaultPrecision );
	
	stretchMin 		= new NumericTextControl( dlog, kFITSUITextMinStretched);
	stretchMin->setPrecision( kFITSDefaultPrecision );
	
	stretchMax 		= new NumericTextControl( dlog, kFITSUITextMaxStretched );
	stretchMax->setPrecision( kFITSDefaultPrecision );
	
	stretchMean 	= new NumericTextControl( dlog, kFITSUITextMeanStretched );
	stretchMean->setPrecision( kFITSDefaultPrecision );
	
	stretchMedian 	= new NumericTextControl( dlog, kFITSUITextMedianStretched );
	stretchMedian->setPrecision( kFITSDefaultPrecision );
	
	stretchSTDEV 	= new NumericTextControl( dlog, kFITSUITextSTDEVStretched );
	stretchSTDEV->setPrecision( kFITSDefaultPrecision );
	
	valuesText 		= new TextControl( dlog, kFITSUITextStretchedScaled );
}

/**
 *
 */
MacStatisticsView::~MacStatisticsView() {
	delete realMin;
	delete realMax;
	delete realMean;
	delete realMedian;
	delete realSTDEV;
	delete stretchMin;
	delete stretchMax;
	delete stretchMean;
	delete stretchMedian;
	delete stretchSTDEV;
	delete valuesText;
}


/**
 *
 */
Void MacStatisticsView::updateRealValues( Double min, Double max, Double mean, Double median, Double stdev ) {
	realMin->setDouble( min );
	realMax->setDouble( max );
	realMean->setDouble( mean );
	realMedian->setDouble( median );
	realSTDEV->setDouble( stdev );
}

/**
 *
 */
Void MacStatisticsView::updateStretchValues( Double min, Double max, Double mean, Double median, Double stdev ) {
	stretchMin->setDouble( min );
	stretchMax->setDouble( max );
	stretchMean->setDouble( mean );
	stretchMedian->setDouble( median );
	stretchSTDEV->setDouble( stdev );
}

/**
 *
 */
Void MacStatisticsView::updateScaledValues( Double min, Double max, Double mean, Double median, Double stdev ) {
	stretchMin->setDouble( min );
	stretchMax->setDouble( max );
	stretchMean->setDouble( mean );
	stretchMedian->setDouble( median );
	stretchSTDEV->setDouble( stdev );
}

/**
 *
 */
Void MacStatisticsView::updateText( String& text ) {
	valuesText->setText( text );
}