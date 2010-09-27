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
#ifndef __STRETCHVIEW_H__
#define __STRETCHVIEW_H__

#include "Observer.h"
#include "StretchModel.h"
#include "FitsLiberator.h"
#include "FlowController.h"
#include "OptionsModel.h"

namespace FitsLiberator
{
	namespace Modelling
	{
		/**
		*	Super class for the stretch view
		*/
		class StretchView : public Observer
		{
        public:
			Void Update( Models * m );
			static const StretchFunction stretchFunctions[21];
		protected:
			StretchView( StretchModel&, FlowController&, OptionsModel& optMod );		
			~StretchView();
			
			/** Enumerates all the defined stretch functions.
				@param func A function object to invoke for each stretch function. 
				This parameter must evaluate to a function object f( StretchFunction function, Int string ).
				In this function f, function is the stretch function, string is the associated string ID,
				param is a user defined parameter. */
			template<class F>
			Void enumerateFunctions( F& func ) {
				for( Int i = 0; i < sizeof(stretchFunctions)/sizeof(stretchFunctions[0]); i++ )
					func( stretchFunctions[i], stretchFunctionStrings[i] );
			}
			
			/** Must be overwritten to fill out the dropdown box with the 
				correct stretchfunction corresponding to the @param 
				StretchFunction defined in StretchModel.h. */
            virtual Void setStretchFunction( Int ) = 0;
			
			/** Returns the current view stretch index. */
			Int getStretchFunctionIndex() const;
            /** Called by a derived class when the user selects reset. */
            void onReset();
            void onAutoBackground();
            void onAutoPeak();
            void onAutoScale();
            void onStretch(int index);
            void onBackground(double value );
            void onPeak(double value );
            void onScale(double value );
			void onBackgroundScalePeak( Double bg, Double pl, Double sPl );
			/** Must be overwritten to fill out the textfield with the 
				backgroundlevel. */
			virtual Void setBackgroundLevel(Double) = 0;
			
			/** Must be overwritten to fill out the scale textfield */
			virtual Void setScale(Double) = 0;
			virtual Void setRescaleFactor( Double ) = 0;
			virtual Void setPeakLevel( Double ) = 0;
			/**Must be overwritten. Sets whether the button for
			applying the stretch values all at once is enabled.*/
			virtual Void setApplyStretchEnabled( Bool ) = 0;
			FlowController& controller;

			StretchModel& model;
			OptionsModel& optionsModel;
            static const Double scaledPeakCompletions[13];

		private:
			/** Called by a derived class, when the user changes the stretch 
				view.
				@param index The logical index of the stretch function. For 
				legal values see StretchIDMap. */
			Void selectStretchFunction( Int index );

			static const Int stretchFunctionStrings[21];
		};
	}
}

#endif