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
#ifndef __HistogramView_H__
#define __HistogramView_H__

#include "Observer.h"
#include "FitsLiberator.h"
#include "HistogramModel.h"
#include "ToolModel.h"
#include "StatisticsModel.h"
#include "StretchModel.h"
#include "OptionsModel.h"
#include "FlowController.h"

namespace FitsLiberator
{
	namespace Modelling
	{
		class HistogramView : public Observer {
        public:
            static const size_t MarkerCount = 10;
            /** Positions of each marker relative to the left edge of the histogram. */
            int markerPositions[MarkerCount];
            FitsLiberator::Color markerColors[MarkerCount];

			HistogramView( HistogramModel&, ToolModel&, StatisticsModel&, StretchModel&, OptionsModel&,
                HistogramController&, FlowController& );
			~HistogramView();
			
            Void Update( Models * m );
		protected:
            /** Updates the histogram markers state. */
            void updateMarkers();
            /** Draws the histogram. */
			virtual Void drawHistogram() = 0;
            /** Fills out the text field with the minimum value of the current range.
                @param value Current minimum value. */
			virtual Void setHistogramMin(Double value) = 0;
            /** Fills out the text field with the maximum value of the current range.
                @param value Current maximum value. */
			virtual Void setHistogramMax(Double) = 0;
            /** Changes the current tool function. */
			virtual Void setCurrentToolFunction( ToolTypeFunction )	= 0;
			/** Should be overwritten to fill out the white level textfield.
                @param value Current white level. */
			virtual Void setWhiteLevel(Double) = 0;
			/** Should be overwritten to fill out the black level textfield.
                @param value Current black level. */
			virtual Void setBlackLevel(Double) = 0;

			HistogramModel&			model;  			///> the model for this view
			ToolModel&				toolModel;			///> Tool model for use with tools in the histogram
            /** The markers are tied to the statistics. */
            StatisticsModel&        statistics;
            StretchModel&           stretch;
            /** The options model determines which markers to display. */
            OptionsModel&           options;
			FlowController&			flowController;     ///> 
			HistogramController&	histoController;	///> the controller for this view			StretchController&		stretchController;		



		};
	}
}

#endif