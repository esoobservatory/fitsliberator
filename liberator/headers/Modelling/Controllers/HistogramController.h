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
#ifndef __HistogramController_h__
#define __HistogramController_h__

#include "FitsLiberator.h"
#include "HistogramModel.h"
#include "StatisticsModel.h"
#include "StretchModel.h"
#include "StretchController.h"
#include "AccumulatingChangeManager.h"
	
namespace FitsLiberator
{
	namespace Modelling
	{
		/**
			Implements the HistogramController which in collaboration with the
			HistograModel takes care of all the logic, interaction and storage
			concerned with the histogram
		*/
		class HistogramController  : public ACMController {
            
		public:
			HistogramController( HistogramModel&, AccumulatingChangeManager* chman );
           /** Centers the histogram around a given point. 
                @param coord Coordinate of the value to center on relative to the 
                    left-most pixel in the histogram widget. */
            Void centerHistogram( UInt coord, Double stretchMin, Double stretchMax );
			
			/** Increases the zoom factor and centers around a specific coordinate.
                @param coord Coordinate of the value to center on relative to the 
                    left-most pixel in the histogram widget. */
            Void incrementZoom( UInt coord, Double stretchMin, Double stretchMax );
            /** Decreases the zoom factor and centers around a specific coordinate.
                @param coord Coordinate of the value to center on relative to the
                    left-most pixel in the histogram widget. */
			Void decrementZoom( UInt coord, Double stretchMin, Double stretchMax );
            /** Zooms the histogram to display the entire range. */
            Void showAll( Double stretchMin, Double stretchMax );
            /** Zooms the histogram to display a specific range.
                @param left Coordinate of the left edge of the range relative to the 
                    left-most pixel in the histogram widget.
                @param right Coordinate of the right edge of the range relative to the 
                    left-most pixel in the histogram widget. */
            void showRange(unsigned int left, unsigned int right, Double min, Double max );
            /** Moves the histogram by a specific amount.
                @param delta Number of pixels to move the histogram. Negative 
                    values move the histogram to the left, positive values move
                    the histogram to the right. */
            Void move( Int delta, Double stretchMin, Double stretchMax );
            /** Moves the black level slider.
                @param posistion New position of the black level slider 
                    relative to the left-most pixel in the histogram widget. */
            Void moveBlackLevelSlider( Int position, Double stretchMin, Double stretchMax );
            /** Moves the white level slider.
                @param posistion New position of the white level slider 
                    relative to the left-most pixel in the histogram widget. */
			Void moveWhiteLevelSlider( Int position, Double stretchMin, Double stretchMax );
			
            
            Void setMaxBin(Double);			///> sets the max bin of the histoModel
			Double getMaxBin();
			
			Void updateHistogram( Double rangeMin, Double rangeMax );			///> Updates the histogram (call when e.g. plane change).

			Void refreshHistogram( Double strMin, Double strMax );
			/**This function refreshes the histogram in order to keep
			it in the previous state but with a new stretch*/
			Void refreshHistogram( Double rangeMin, Double rangeMax,
								   Double stretchMin, Double stretchMax );

			//interaction with the black level
            Void    incrementBlackLevel(Bool, Bool);
			Void	setBlackLevel(Double);

			//interaction with the white level
            Void    incrementWhiteLevel(Bool, Bool);
			Void	setWhiteLevel(Double);

		private:
            /** Generates the view of the histogram to display.
                @param min Minimum value of the view.
                @param max Maximum value of the view. */
            void generateView(double min, double max);

			Void pushHistogram( Int, Int, Double, Double );
			Void calculateIncrementFactor();
			Void generateHistogramBins( Double strMin, Double strMax );	///> Updates the array of Int vals representing the histogram
			UInt getRawBin( Double val, Double hMin, Double hMax );			///> Returns the raw bin number corresponding to the given coord in the shown histogram

			HistogramModel& histoModel;
		};
	}
}

#endif