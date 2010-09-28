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


#ifndef ___MACPREFERENCESVIEW_H__
#define ___MACPREFERENCESVIEW_H__

#include "Types.h"
#include "BaseGui.h"
#include "OptionsView.h"

using namespace FitsLiberator::Modelling;

namespace FitsLiberator {
	namespace Mac {
		class MacPreferencesView : public OptionsView, public ControlEventHandler {
			BaseDialog*         mainDialog;
			BaseControl*        preferencesButton;
						
			BaseControl*        method;
			NumericTextControl* blackLevel;
			NumericTextControl* whiteLevel;
			
			BaseControl*        function;
			NumericTextControl* scaledPeakLevel;
			
			BaseControl*		showZero;
			BaseControl*		showBackground;
			BaseControl*		showPeak;
			BaseControl*		showMean;
			BaseControl*		showStretchedMean;
			BaseControl*		showScaledMean;
			BaseControl*		showMedian;
			BaseControl*		showStretchedMedian;
			BaseControl*		showScaledMedian;
			BaseControl*		applyStrechDirectly;
			
			BaseControl*		restoreDefaults;
			BaseControl*		clearHistory;
			
			void createPreferencesControls(BaseDialog*);
			void destroyPreferencesControls();
		public:
			MacPreferencesView( BaseDialog*, const OptionsModel&, OptionsController& );
			~MacPreferencesView();
			
			void showPreferencesDialog();
			
			// ControlEventHandler
			
			virtual OSStatus onCommand( HICommand * );
			virtual OSStatus onSetFocusPart( EventRef event );
			virtual OSStatus onMouseEntered( EventRef event );
			virtual OSStatus onMouseExited( EventRef event );
			
			// OptionsView
			
			virtual void Update();
		};
	}  // namespace Mac end
} // namespace FitsLiberator end
#endif // ___MACPREFERENCESVIEW_H__