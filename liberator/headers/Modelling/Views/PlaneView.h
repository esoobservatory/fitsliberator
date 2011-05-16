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
#ifndef __PLANEVIEW_H__
#define __PLANEVIEW_H__

#include "FitsLiberator.h"
#include "Plane.h"
#include "PlaneModel.h"
#include "FlowController.h"
#include "PlaneController.h"

namespace FitsLiberator {
    namespace Modelling {
    
        class PlaneModel;
        class PlaneController;

		/**
		*	Super class for the plane view.
		*   Should be extended by any platform or GUI specific 
		*   view.
		*/
        class PlaneView : public Observer {
            public:
				PlaneView( PlaneModel&, FlowController&, PlaneController& );
               	~PlaneView();

                virtual Void Update( Models* m );
            protected:
                String getEntry( Int ) const;
                Int    getEntryCount() const;
                Void   selectorChanged( Int );

                virtual Void updateSelectorIndex( Int index ) = 0;
                virtual Void updateSize( Int width, Int height ) = 0;
                virtual Void updateBitDepth( ChannelSettings, Bool, Bool ) = 0;
				virtual Void updateUndefinedOption( UndefinedSettings, Bool, Bool ) = 0;
				virtual Void updateFlip( Bool ) = 0;

                PlaneModel&		 model;              ///< Model
                FlowController&  flowController;
				PlaneController& planeController;
			
                Vector<FitsLiberator::Engine::Plane> entries;          ///< Items in the selector dropdown
        };
    } // end namespace Modelling
} // end namespace FitsLiberator
#endif //__PLANEVIEW_H__