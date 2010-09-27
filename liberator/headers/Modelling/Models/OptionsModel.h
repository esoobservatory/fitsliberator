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

/** @file       Declares the OptionsModel class.
    $Log: OptionsModel.h,v $
    Revision 1.8  2010/09/25 15:32:49  kaspar
    *** empty log message ***

    Revision 1.7  2010/09/21 06:35:52  kaspar
    *** empty log message ***

    Revision 1.6  2010/09/20 12:39:38  kaspar
    *** empty log message ***

    Revision 1.5  2010/09/14 13:01:52  kaspar
    *** empty log message ***

    Revision 1.4  2009/10/22 09:47:40  teis
    Changed the histogram markers.

    There are now many more histogram markers available. In the final release the UI for an undesired marker is simply removed.

    Revision 1.3  2008/09/23 15:15:43  teis
    Added histogram markers

    Revision 1.2  2008/09/18 09:04:06  teis
    Added some more options

    Revision 1.1  2008/09/15 08:09:07  teis
    Implemented the Options dialog

    @version	$Revision: 1.8 $
    @date		$Date: 2010/09/25 15:32:49 $
    @author		Teis Johansen <teis@siet.dk>
    @author		Kaspar Kirstein Nielsen <kaspar@barmave.dk>
    @author		Lars Holm Nielsen <lars@hankat.dk>  */

#pragma once

#include "FitsLiberator.h"
#include "Observer.h"

namespace FitsLiberator {
	namespace Modelling {
        /** Models the Options dialog. */
		class OptionsModel : public Model {
            typedef Model super;

            struct State {
                int          guess;
                double       black;
                double       white;
                int          stretch;
                double       scaledPeak;
                unsigned int markers;
				bool		 applyStretchDirectly;
            };

            State initialState;
            State currentState;
            bool  clearHistory;
        public:
            static const unsigned int MarkerGrid            = (1 <<  0);
            static const unsigned int MarkerZero            = (1 <<  1);
            static const unsigned int MarkerBackground      = (1 <<  2);
			static const unsigned int MarkerPeak			= (1 <<  3);
			static const unsigned int MarkerMean			= (1 <<  4);
			static const unsigned int MarkerStretchedMean	= (1 <<  5);
			static const unsigned int MarkerScaledMean		= (1 <<  6);
			static const unsigned int MarkerMedian			= (1 <<  7);
			static const unsigned int MarkerStretchedMedian	= (1 <<  8);
			static const unsigned int MarkerScaledMedian	= (1 <<  9);
			static const unsigned int MarkerScaledPeak		= (1 << 10);
			

            OptionsModel(ChangeManager* manager);
            /** Returns the options to the default. */
            void Defaults();
            /** Marks the current state as the baseline. */
            void MarkBaseline();
            /** Reverts to the baseline. */
            void Revert();
            /** Gets the index of the guess method. */
            int GuessMethod() const;
            /** Sets the new guess method
                @param index Index of the newly selected item. */
            void GuessMethod(int index);
            /** Gets whether the BlackLevelPercentage and WhiteLevelPercentage values should be editable. */
            bool EditableLevels() const;
            /** Gets the black level percentage. */
            double BlackLevelPercentage() const;
            /** Sets the new black level percentage.
                @param percentage Percentage of the black level. */
            void BlackLevelPercentage(double percentage);
            /** Gets the white level percentage. */
            double WhiteLevelPercentage() const;
            /** Sets the new white level percentage.
                @param percentage Percentage of the white level. */
            void WhiteLevelPercentage(double percentage);
            /** Gets the index of the default stretch function. */
            int DefaultStretch() const;
            /** Sets the new default stretch function. 
                @param index Index of the newly selected item. */
            void DefaultStretch(int index);
            /** Gets the default scaled peak level. */
            double ScaledPeak() const;
            /** Sets the new scaled peak level default.
                @param value New value of the default scaled peak level. */
            void ScaledPeak(double value);
            /** Gets the histogram markers to display. This value is a combination of
                the flags MarkerZero..MarkerScaledMean. */
            unsigned int HistogramMarkers() const;
            /** Sets the histogram markers to display.
                @param value new value. */
            void HistogramMarkers(unsigned int value);
            /** Gets whether the history should be cleared. */
            bool ClearHistory() const;
            /** Sets whether the history should be cleared. */
            void ClearHistory(bool);
			/** Get the ApplyStretchDirectly*/
			bool ApplyStretchDirectly() const;
			/** Set the ApplyStretchDirectly*/
			void ApplyStretchDirectly( bool );
			
		};
	} // end namespace Modelling
} // end namespace FitsLiberator
