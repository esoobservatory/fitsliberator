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

#ifndef __IMAGETILE_H__
#define __IMAGETILE_H__

#include "FitsLiberator.h"

#include "Stretch.h"

namespace FitsLiberator
{
	namespace Engine
	{

		/**
			Data storage low-level logic of the ImageTile 
			which is the back-bone of the pixel handling.
		*/
		struct ImageTile
		{		
			ImageTile();
			~ImageTile();
		
			friend Bool operator==( const ImageTile& lhs, const ImageTile& rhs );

			UInt x;
			UInt y;
			UInt width;
			UInt height;
			Int effLeft;
			Int effTop;
			Int effRight;
			Int effBottom;

			Bool locked;
			Bool stretched;
			FitsLiberator::Engine::Stretch stretch;
			Double* stretchedPixels;
			Void* rawPixels;
			char* nullPixels;

		
			FitsLiberator::Rectangle bounds;
			FitsLiberator::Rectangle effBounds;
			Bool isAllocated();
			const FitsLiberator::Rectangle getBounds();
			const FitsLiberator::Rectangle& getEffBounds();
			Int allocatePixels( Int bitDepth );
			Void deallocatePixels();

			Bool isCurrent();
			Void setX( Int x );
			Void setY( Int y );
			Void setWidth( Int w );
			Void setHeight( Int h );

			//these two constants might be changed somewhat
			//The optimal size is currently unknown.
			static const Int tile_large_min_width = 8192;
			static const Int tile_large_min_height = 8192;

			//static const Int tile_max_pixels = 30000000;

			static const Int tile_small_min_width = 256;
			static const Int tile_small_min_height = 256;

			static const Int AllocErr = -1;
			static const Int AllocOk = 0;
			static const Int OperationCanceled = 1;

		};
	}
}

#endif