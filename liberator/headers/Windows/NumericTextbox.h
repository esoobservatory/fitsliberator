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
#ifndef __NUMERICTEXTBOX_H__
#define __NUMERICTEXTBOX_H__

#include "Textbox.h"

#define NTN_ENTERPRESSED (EN_CHANGE + 1)
#define NTN_UP           (EN_CHANGE + 2)
#define NTN_DOWN         (EN_CHANGE + 3)
#define NTN_LOSTFOCUS    (EN_CHANGE + 4)
#define NTN_GOTFOCUS     (EN_CHANGE + 5)

namespace FitsLiberator {
    namespace Windows {
        /** A numeric text box */
        class NumericTextbox : public Textbox {
            typedef Textbox super;
            /** Custom window procedure, used for custom keyboard handling.
                @see Window::ProcessMessage for parameters. */
            static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
            /** Event handler, notifies the parent window that something special happened */
            Void NotifyParent(USHORT);

            WNDPROC oldWindowProc;              ///< Textbox window procedure
            Int     precision;                  ///< Precision
            String  postfix;                    ///< Postfix to display after the number.
            static String allowedCharacters;    ///< Character used as decimal separator
        public:
            NumericTextbox();
            virtual ~NumericTextbox();
            /** Attaches this instance to an edit window.
                @param hWnd Window handle of the window to attach. */
            Void Attach( HWND hWnd );
            /** Detaches this instance from an edit window. Called automatically by the destructor. */
            Void Detach();
            /** Gets the precision used displaying numbers. 
                @return The number of decimal places. */
            Int getPrecision() const;
            /** Sets the precision used displaying numbers. 
                @param value The new precision to use. */
            Void setPrecision( Int );
            /** Gets the value displayed in the textbox. */
            Double getValue() const;
            /** Sets the value displayed in the textbox.
                @param value New value to display. */
            Void setValue( Double );
            /** Gets the postfix displayed after the value. */
            String Postfix() const;
            /** Sets the postfix displayed after the value.
                @param value New postfix to use. */
            void Postfix(const String& value);
            /** Is this an embedded textbox like in a combobox, in that case events are delivered to the parent of the parent.*/
            Bool    embedded;
        };
    }
}

#endif //__NUMERICTEXTBOX_H__