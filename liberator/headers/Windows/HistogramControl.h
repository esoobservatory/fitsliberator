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
#ifndef __HISTOGRAMCONTROL_H__
#define __HISTOGRAMCONTROL_H__

#include "CanvasControl.h"
#include "MouseTracker.h"
#include "SelectionTrackerSink.h"
#include "MoveTrackerSink.h"
#include "HistogramModel.h"
#include "HistogramController.h"
#include "ToolModel.h"
#include "FlowController.h"
#include "HistogramView.h"

namespace FitsLiberator {
    namespace Windows {
        /**
         * Implements the histogram control.
         */
        class HistogramControl : public CanvasControl {
            class HistogramSelectionTrackerSink : public SelectionTrackerSink {
                public:
                    HistogramSelectionTrackerSink( HWND, const RECT& );
                protected:
                    virtual Void adjustSelection();
            };

            public:
                HistogramControl(
                    FitsLiberator::Modelling::HistogramModel&, 
                    FitsLiberator::Modelling::HistogramView&,             
					FitsLiberator::Modelling::ToolModel&,
					FitsLiberator::Modelling::FlowController& );
                ~HistogramControl();

                Bool Create( HWND, const LPRECT );
                virtual Void OnToolChanged( FitsLiberator::Modelling::ToolTypeFunction );
            protected:
                virtual Void OnCreate( HWND );
                virtual Void OnMouseDown( Int, Int, Int );
                virtual Void OnMouseUp( Int, Int, Int );
                virtual Void OnMouseMove( Int, Int, Int );
                virtual Void OnRefresh( HDC );
                virtual Bool canZoomIn();
                virtual Bool canZoomOut();
            private:
                typedef CanvasControl super;

                HGDIOBJ histogramPen;                                       ///< Used to paint the histogram
                HGDIOBJ sliderPen;                                          ///< Used the paint the level indicators
                
                FitsLiberator::Modelling::HistogramModel&      model;       ///< Model to get data from.
                FitsLiberator::Modelling::HistogramView&        view;
                FitsLiberator::Modelling::ToolModel&           toolModel;   ///< Model to use for mode information.
				FitsLiberator::Modelling::FlowController&		flowController;

                MouseTracker* histogramTracker;                             ///< The mouse tracker
                MoveTrackerSink* histogramMoveTrackerSink;                  ///< Move event sink
                HistogramSelectionTrackerSink* histogramSelectionTrackerSink;   ///< Selection event sink

                void PaintBackground(HDC hDC, const FitsLiberator::Rectangle& bounds);
                void PaintContent(HDC hDC, const FitsLiberator::Rectangle& bounds);
                void PaintSliders(HDC hDC, const FitsLiberator::Rectangle& bounds);
                void PaintSelection(HDC hDC, const FitsLiberator::Rectangle& bounds);
                void PaintMarker(HDC hDC, const FitsLiberator::Rectangle& bounds, int index);
        };
    }
}

#endif //__HISTOGRAMCONTROL_H__