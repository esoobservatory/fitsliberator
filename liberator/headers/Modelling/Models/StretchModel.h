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
#pragma once
#include "FitsLiberator.h"
#include "Observer.h"
#include "Stretch.h"

namespace FitsLiberator
{
	namespace Modelling
	{

		class StretchModel : public Model
		{
		public:
			//constructor / destructor
			StretchModel( ChangeManager * chman );
			~StretchModel();
			
			//interaction  with the stretch function
			FitsLiberator::Engine::StretchFunction	getFunction() const;
			Void								    setFunction(FitsLiberator::Engine::StretchFunction);
			FitsLiberator::Engine::StretchFunction	setDefaultFunction();					///> Sets the stretch function to the current default.
			Void									setDefaultFunction(FitsLiberator::Engine::StretchFunction);	///> Sets a new default stretch function

			//interaction with the background
			Double          getBackground() const;
			Void            setBackground(Double);
			Double			getPeakLevel() const;
			Void			setPeakLevel(Double);
			Double          setDefaultBackground();					///> Sets the offset to the current default
			Void			setDefaultBackground(Double);		///> Sets a new default offset value
			Void			setBackgroundIncrementation( Double );	///> Sets the increment/decrement value for the offset
			Double			getBackgroundIncrementation() const;

			//interaction with the scale
			Double			getScaleBackground();
			Void			setScaleBackground( Double );
			Double			getRescaleFactor() const;
			Void			setRescaleFactor(Double);
			Double          getScale() const;
			Void            setScale(Double);
			Double          setDefaultScale();						///> Sets the scale to the current default
			Void			setDefaultScale(Double);				///> Sets a new default scale value
			Void			setScaleIncrementation( Double );		///> Sets the scale increment/decrement value
			Double			getScaleIncrementation() const;
			
		private:
			/*
				Members containing default values
				The default values are inited to some very standard values, but they should
				be set to some fitting default values according to the specific image.
				Maybe we should consider using default values for each stretch function?
			*/
			FitsLiberator::Engine::StretchFunction	defaultStretchFunction;				///> The default stretch function
			Double									default_background;					///> Default offset
			Double									default_scale;						///> Default scale
			Double									default_rescaleFactor;
			Double									default_peakLevel;
			Double									default_scaleBackground;
			//members containing variable information
			FitsLiberator::Engine::StretchFunction	function;
			Double									background;
			Double									backgroundIncrementation;			///> Increment/decrement value for offset
			Double									scale;
			Double									scaleIncrementation;			///> Increment/decrement value for scale
			Double									peakLevel;
			Double									rescaleFactor;
			Double									scaleBackground;
			friend class StretchController;
		};
	}
}
