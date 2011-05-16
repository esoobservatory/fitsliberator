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
#ifndef __WINDOWSPLANEVIEW_H__
#define __WINDOWSPLANEVIEW_H__

#include "Dialog.h"
#include "PlaneView.h"
#include "EventSink.h"
#include "ComboBox.h"

using namespace FitsLiberator::Modelling;
namespace FitsLiberator {
    namespace Windows {
        /**
         * Implements the interface between the Windows GUI and ToolView.
         */ 
        class WindowsPlaneView : public FitsLiberator::Modelling::PlaneView, public EventSink {
            public:

                //ObsoleteWindowsPlaneView( FitsLiberator::Modelling::PlaneModel& model, FitsLiberator::Modelling::PlaneController& controller );
				WindowsPlaneView( PlaneModel&, FlowController&, PlaneController& planeController  );

				virtual Void updateBitDepth( ChannelSettings, Bool, Bool );
				virtual Void updateUndefinedOption( UndefinedSettings, Bool, Bool );
                virtual Void OnInit( Dialog*, Int );
				virtual Void updateFlip( Bool );

                virtual Void OnCommand( WPARAM, LPARAM );
                virtual Void OnNotify( Int, LPNMHDR );
                virtual Void OnScroll( WPARAM, LPARAM );
			protected:
                virtual Void updateSelectorIndex( Int );
                virtual Void updateSize( Int, Int );
            private:
                Dialog* owner;                      ///< Dialog that owns this view.
                ComboBox selector;                  ///< Window handle of the plane dropdown.
                Window   widthLabel;                ///< Window handle of the width label.
                Window   heightLabel;               ///< Window handle of the height label.
                HWND     channel8Button;             ///< Window handle of the 8-bit channel radio button.
                HWND     channel16Button;            ///< Window handle of the 16-bit channel radio button.
                HWND     channel32Button;           ///< Window handle of the 32-bit channel radio button.
                HWND     undefinedBlackButton;       ///< Window handle of the undefined-black radio button.
                HWND     undefinedTransparentButton; ///< Window handle of the undefined-transparent radio button.
				HWND	 flipButton;
        };
    }
}

#endif //__WINDOWSPLANEVIEW_H__
