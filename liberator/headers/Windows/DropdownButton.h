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
#ifndef __DROPDOWNBUTTON_H__
#define __DROPDOWNBUTTON_H__

#include "Button.h"

#define BM_DRAWARROW	WM_USER+1

namespace FitsLiberator {
    namespace Windows {
        /** A button with a drop-down arrow, which shows a menu when clicked. */
        class DropdownButton : public Button {
            typedef Button super;

            WNDPROC oldWindowProc;     ///< Old window procedure.
			HMENU   menu;			   ///< Menu handle to show as the dropdown.
			int     resource;	       ///< Menu resource ID.
			/** Custom window procedure, used to override painting.
				@see Window::WindowProc. */
            static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
			/** Paint the arrow. */
			void Paint(LPNMCUSTOMDRAW pncd) const;
		protected:
			virtual void ShowMenu();
		public:
			/** Creates a dropdown button from an existing button. Note that 
			    the life-time of this object is tied to the button window. The 
				object destroys itself when the window is destroyed. 
				@param bitmap Bitmap to paint on the button.
				@param buttonWindow Existing button to modify. */
			DropdownButton( HWND hWnd );
            virtual ~DropdownButton();
			/** Gets the menu to display when the button is clicked. */
			HMENU Menu() const;
			/** Sets the menu to display when the button is clicked.
				@param menu New menu to use. */
			void Menu(HMENU menu);
			/** Gets the menu resource id. */
			int Resource() const;
			/** Sets the menu resource id.
				@param id New menu resource ID. */
			void Resource(int id);
        };
    }
}

#endif //__DROPDOWNBUTTON_H__