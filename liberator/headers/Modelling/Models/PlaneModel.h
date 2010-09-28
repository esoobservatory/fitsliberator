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
#ifndef __PLANEMODEL_H__	
#define __PLANEMODEL_H__

#include "FitsLiberator.h"
#include "Observer.h"
#include "Plane.h"
#include "ImageReader.hpp"
#include "ImportSettings.h"
#include "FitsMath.h"
#include "Flip.h"
using namespace FitsLiberator::Engine;

namespace FitsLiberator {
    namespace Modelling {
        class PlaneController;
        
        /** 
         * Models the plane selector
         */
        class PlaneModel : public Model {
            public:
                PlaneModel( ChangeManager*, ImageReader* );

                Int getImageIndex() const;
                Int getPlaneIndex() const;
                Int getWidth() const;
                Int getHeight() const;
                
				Void setFlipped( Bool );
				const Flip& getFlipped();
                
				Void setImportBitDepth( ChannelSettings );
				Void setImportUndefinedOption( UndefinedSettings );
				
				ChannelSettings	getImportBitDepth();
				UndefinedSettings getImportUndefinedOption();
				
				Vector<Plane> getEntries();
				Int getEntryIndex();
								
				Plane& getPlane();
				
				Void resetFlip();
				/** Determines if the image should be flipped about the 
					horizontal axis to match the convention used in 
					astronomy. */
				Bool shouldFlip() const;

                Bool updateModel( Int imageIndex, Int planeIndex );

				Void updateReader( FitsLiberator::Engine::ImageReader* r );
			protected:  
                friend class PlaneController;
            private:
                Int imageIndex;    ///< Current image index
                Int planeIndex;    ///< Current plane index
				
				Flip flip;

                Int width;         ///< Width of teh current plane.
                Int height;        ///< Height of the current plane.

                ChannelSettings 	bitDepth;			///< Bit depth to import in.
		        UndefinedSettings	undefinedOption;	///< How to import undefined values
				
				Void updateFlip();

				FitsLiberator::Engine::ImageReader* reader;
				Plane plane;
				Void doUpdateModel();
        };
    } // end namespace Modelling
} // end namespace FitsLiberator

#endif // __PLANEMODEL_H__