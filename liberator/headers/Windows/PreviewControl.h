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
#ifndef __PREVIEWCONTROL_H__
#define __PREVIEWCONTROL_H__

#include "CanvasControl.h"
#include "MouseTracker.h"
#include "SelectionTrackerSink.h"
#include "MoveTrackerSink.h"
#include "PreviewModel.h"
#include "PreviewController.h"
#include "ToolModel.h"
#include "GlobalSettingsModel.h"
#include "FlowController.h"
#include "ProgressModel.h"

namespace FitsLiberator {
    namespace Windows {
        /**
         * Implements the preview control.
         */
        class PreviewControl : public CanvasControl {
            public:
                PreviewControl(
                    FitsLiberator::Modelling::PreviewModel&,
                    FitsLiberator::Modelling::PreviewController&,
                    FitsLiberator::Modelling::ToolModel&,
                    FitsLiberator::Modelling::GlobalSettingsModel&,
					FitsLiberator::Modelling::FlowController&,
					FitsLiberator::Modelling::ProgressModel&
                );
                virtual ~PreviewControl();

                Bool Create( HWND hWndParent, const LPRECT bounds );
                virtual Void OnToolChanged( FitsLiberator::Modelling::ToolTypeFunction );
                Void Update();
                void gotBusy();
                void gotDone();
            protected:
                virtual int OnTimer(UINT id);
                virtual bool OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, Int& result);
                virtual Void OnCreate( HWND hWnd );
                virtual Void OnMouseDown( Int, Int, Int );
                virtual Void OnMouseMove( Int, Int, Int );
                virtual Void OnMouseUp( Int, Int, Int );
                virtual bool OnMouseWheel(SHORT delta, USHORT flags, USHORT screenX, USHORT screenY);
                virtual Void OnRefresh( HDC );
                virtual Bool canZoomIn();
                virtual Bool canZoomOut();
            private:
                typedef CanvasControl super;
                
                Gdiplus::Bitmap* createPreview( const FitsLiberator::Size&, Byte*  );
                Gdiplus::Bitmap* createMask( const FitsLiberator::Size&, const Gdiplus::Color&, Byte* );
                Bool             createBitmaps();
                Void             destroyBitmaps();
                Void             destroyImage( Gdiplus::Bitmap** );
                Void             handleLeftClick( FitsLiberator::Point& );
                Void             handleRightClick( POINT* );

                FitsLiberator::Modelling::GlobalSettingsModel& options;
                FitsLiberator::Modelling::PreviewModel& model;
                FitsLiberator::Modelling::PreviewController& controller;
                FitsLiberator::Modelling::ToolModel& tool;
				FitsLiberator::Modelling::FlowController& flowController;
				FitsLiberator::Modelling::ProgressModel&  progressModel;
                
                Gdiplus::Bitmap* previewImage;                              ///< The preview image (8-bit grayscale)
                Gdiplus::Bitmap* undefinedMask;                             ///< The undefined mask (all pixels red or transparent)
                Gdiplus::Bitmap* blackMask;                                 ///< The black mask (all pixels blue or transparent)
                Gdiplus::Bitmap* whiteMask;                                 ///< The white mask (all pixels green or transparent)
                
                Gdiplus::Bitmap* throbber;
				Gdiplus::Font*	 throbberFont;
                int              progress;
                int              max_progress;

                MouseTracker*           mouseTracker;                       ///< Mousetracker
                SelectionTrackerSink*   selectionTracker;                   ///< Selection mousetracker event sink
                MoveTrackerSink*        moveTracker;                        ///< Move mousetracker event sink
                Int                     button;                             ///< Which mouse button was pressed?
        };
    }
}

#endif //__PREVIEWCONTROL_H__