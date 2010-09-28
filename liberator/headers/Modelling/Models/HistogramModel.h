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
#ifndef __HistogramModel_H__
#define __HistogramModel_H__

#include "FitsLiberator.h"
#include "Observer.h"

namespace FitsLiberator
{
	namespace Modelling
	{
		/**
			Implements the HistogramModel which takes care of the
			low-level histogram logic and all the storage
		*/
		class HistogramModel : public Model
		{
		public:
			HistogramModel(ChangeManager * chman);
			
			
			Vector<Double>& getRawBins();			///> returns the bins
			Vector<Int>&	getEndBins();			///> returns the bins to be rendered in the window.

			Void setMaxBin(Double);					///> sets the maximum value a bin takes		
			Double getMaxBin();						///> returns the max value from a bin
			
			Void setOffset(Int);					///> sets the current offset
			Int getOffset();						///> returns the current offset
			
			Double getCurrentZoom();				///> Returns the current zoom factor
			Void setZoomValue(Double);				///> Sets the value of the zoom factor
			Void setMinZoomValue();					///> sets the minimal zoom value
			Double getReverseZoom();				///> returns 1 - currentZoom if possible

			Bool incrementZoom();					///> increases the zoom factor if possible
			Bool decrementZoom();					///> decreases the zoom factor if possible

			Int getBlackSliderPos();				///> returns the position of the black slider
			Int getWhiteSliderPos();				///> returns the position of the white slider
			Void setBlackSliderPos(Int);
			Void setWhiteSliderPos(Int);

			Double getRangeMax();					///> returns the maximum value of the range
			Double getRangeMin();					///> returns the minimum value of the range
			
	
			//interaction withe the black level
			Double          getBlackLevel() const;
			Void            setBlackLevel(Double);
			Double			setDefaultBlackLevel();					///> Sets the black level to the current default
			Void			setDefaultBlackLevel(Double);			///> Sets a new default black level value
			Double			getCurrentBlackLevel();
			//interaction with the white level
			Double          getWhiteLevel() const;
			Void            setWhiteLevel(Double);
			Double			setDefaultWhiteLevel();					///> Sets the white level to the current default
			Void			setDefaultWhiteLevel(Double);			///> Sets a new default white level value
			Double			getCurrentWhiteLevel();
			

			Void			setLevelIncrementation( Double);		///> Sets the increment/decrement value for the levels
			Double			getLevelIncrementation() const;			///> Returns the level incrementation

			Void			updateLevels(Double, Double);

			Void			setSliders(Double,Double);

            Int				getClosestSlider( Int );
            Int				getPositionOf( Double );
			
			Double			getVal(UInt);

			FitsLiberator::Size& getHistogramSize();///> returns the size of the histogram output window
			Void setHistogramSize(FitsLiberator::Size);///> sets the histogram size
			
			Void setNumberOfBins( UInt nBins );
			
			
		private:
			Int getPos(Double);
			Void setZoom();
			

			Vector<Double> rawBins;					///> the raw bins, from which the histogram is generated
			Vector<Int> endBins;					///> the length of the bins to be drawn
		
			Int offset;								///> defines the offset of the end bins according to the raw bins. Should be inited to zero
			Double currentZoomValue;				///> The actual current zoom factor
			Double maxBin;							///> Contains the maximum value a bin contains


			Double blackLevel;						///> Contains the black level (e.g. the black cut)
			Double whiteLevel;						///> Contains the white level (e.g. the white cut)
			
			//temporary values, use while dragging the sliders.
			Double currentBlackLevel;
			Double currentWhiteLevel;

			Double levelIncrementation;				///> contains the incrementational value

			Double histoRangeMax;					///> contains the maximum value of the histogram range
			Double histoRangeMin;					///> contains the minimum value of the histogram range

			Double zoomMin;							///> the smallest zoom factor possible
			Double zoomMax;							///> the greatest zoom factor possible

			FitsLiberator::Size histogramSize;		///> The size of the histogram

			friend class HistogramController;
			friend class StretchController;
			

		};
	}

}

#endif