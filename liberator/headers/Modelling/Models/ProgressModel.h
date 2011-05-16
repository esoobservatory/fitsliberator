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

/** @file       Declares the ProgressModel class.
    @version	$Revision: 1.11 $
    @date		$Date: 2010/09/25 15:32:49 $
    @author		Teis Johansen <teis@siet.dk>
    @author		Kaspar Kirstein Nielsen <kaspar@barmave.dk>
    @author		Lars Holm Nielsen <lars@hankat.dk>  */

#pragma once

#include "FitsLiberator.h"
#include "Observer.h"

namespace FitsLiberator {
	namespace Modelling {
        /** Acts as a progress notification link. */
		class ProgressModel : public Model {
            typedef Model super;
            /** True if there is a heavy task going on. */
            volatile bool isBusy;
            /** True if there the user has opted to end the heavy task. */
            volatile bool shouldCancel;
			/** True if the user has the possibility of canceling; false if not.*/
			volatile bool canCancel;
			/** Progress value in percent.
				IMPORTANT: Since variable is a regular integer and there is 
				only one thread that updates this value we can get away with 
				not using synchronization primitives. */
			volatile unsigned int progress;
			/** How many times has End been called? This value is only allowed
				to get up to 2. */
			volatile int endedCount;
			/** Progress increment. */
			double increment;
			/** Progress max. */
			double max;
			/** Progress value used by the writer. */
			double writerProgress;
			/** Sets the progress value.
				@param	value	New progress value. */
			void Progress(unsigned int value);
        public:
            ProgressModel(ChangeManager* manager);
            /** Starts a new task. */
            void Begin();
            /** Ends a task. */
            void End();
			/**Set whether the progressModel can cancel */
			inline void CanCancel( bool ca ) { canCancel = ca; }			
            /** Indicate that the current task should be cancelled. */
            inline void Cancel(){ if ( canCancel ) shouldCancel = true; }
            /** Query the cancel state. */
            inline bool QueryCancel() const { return shouldCancel; }
            /** Query the busy state. */
            inline bool QueryBusy() const { return isBusy; }
			/** Query the initializing state. */
			inline bool QueryInitializing() const { return (endedCount < 2); }
			/** Resets the progress counter. */
			inline void Reset() { writerProgress = 0.0; Progress(0); }
			/** Increments the progress counter. */
			void Increment();
			/** Sets the amount the progress is incremented by a call to 
				Increment. */
			void SetIncrement(unsigned int inc);
			/** Sets the maximum progress value. */
			void SetMax(unsigned int max);
			/** Gets the progress value. */
			inline unsigned int Progress() { return progress; }  
		};
	} // namespace Modelling
} // namespace FitsLiberator
