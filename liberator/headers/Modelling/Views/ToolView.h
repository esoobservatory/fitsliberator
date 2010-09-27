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
#ifndef __TOOLVIEW_H__
#define __TOOLVIEW_H__

#include "Observer.h"
#include "ToolModel.h"
#include "ToolController.h"
#include "FitsLiberator.h"

namespace FitsLiberator {
	namespace Modelling {

		class ToolView : public Observer {
			public:
				ToolView(ToolModel&, ToolController&);
				~ToolView();
			
				virtual Void Update( Models * m );
			
			protected:
				//should be overwritten. Activates/deactivates the hand tool
				virtual Void setHandStatus(Bool) = 0;
				
				//should be overwritten. Activates/deactivates the eyedropper_background tool
				virtual Void setEyedropperBackgroundStatus(Bool) = 0;
				
				//should be overwritten. Activates/deactivates the eyedropper_peak tool
				virtual Void setEyedropperPeakStatus(Bool) = 0;
				
				//should be overwritten. Activates/deactivates the eyedropper_blacklevel tool
				virtual Void setEyedropperBlacklevelStatus(Bool) = 0;

				//should be overwritten. Activates/deactivates the eyedropper_whitelevel tool
				virtual Void setEyedropperWhitelevelStatus(Bool) = 0;
				
				//should be overwritten. Activates/deactivates the last selected eyedropper tool
				virtual Void setPickerStatus(Bool) = 0;
				
				//should be overwritten. Activates/deactivates the magnifier_zoom_in tool
				virtual Void setMagnifierZoomStatus(Bool) = 0;
				
				

				ToolModel& model;
	            ToolController& controller;
		};
		
	} // end namespace Modelling
} // end namespace FitsLiberator

#endif // __TOOLVIEW_H__