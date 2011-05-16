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
#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <windows.h>
#include <commctrl.h>
#include <gdiplus.h>
#include <uxtheme.h>

#include "FitsLiberator.h"

extern HINSTANCE g_hInstance;   ///< Global instance handle

namespace FitsLiberator {
    namespace Windows {
        /**
         * Represents a window be that a top-level window or a control.
         */
        class Window {
            public:
                Window();
                Window(HWND hWnd);
                virtual ~Window();

                Int ProcessMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
                Int SendMessage( UINT uMsg, WPARAM wParam, LPARAM lParam );
                Void Destroy();
                Void SetFocus();
                Void LockUpdate();
                Void UnlockUpdate();
                Void Center();
                Void Center( HWND );

                static Void setText( HWND, const String& );
				static Void setValue( HWND, Double );

                String getText() const;
                Void setText(const String& text);
                Void setValue( Double );
                Void appendText(const String& text);

                FitsLiberator::Rectangle getBounds() const;
                FitsLiberator::Rectangle getClient() const;
                Void setBounds(const FitsLiberator::Rectangle& bounds);
                Void setBounds(const RECT* bounds);
                
                Bool getVisible() const;
                Void setVisible(Bool visible);
                Void setEnabled(Bool enabled);

                Void modifyStyle( Int type, DWORD removeStyle, DWORD addStyle );
                
                HWND getHandle();
                Void setHandle( HWND hWnd );
            protected:
                virtual Void OnCreate(HWND hWnd);
                virtual Void OnDestroy(HWND hWnd);
                virtual Void OnNcDestroy();
                virtual Void OnCommand(WPARAM wParam, LPARAM lParam);
                virtual Void OnPaint(HWND hWnd, HDC hDC, RECT* rc, PAINTSTRUCT* ps);
                virtual HBRUSH OnCtlColor(HWND hWnd, HDC hDC, UINT ctlType);
                virtual Void OnParentNotify(UINT uMsg, LPARAM lParam);
                virtual LRESULT OnNotify(Int idCtrl, LPNMHDR pnmh);
                virtual Void OnKeyDown( Int, Int );
                virtual Void OnKeyPress( Int, Int );
                virtual Void OnKeyUp( Int, Int );
                virtual Void OnVerticalScroll( WPARAM, LPARAM );
                virtual Void OnHorizontalScroll( WPARAM, LPARAM );
                virtual Void OnMouseDown( Int button, Int x, Int y );
                virtual Void OnMouseUp( Int button, Int x, Int y );
                virtual Void OnMouseMove( Int button, Int x, Int y );
                virtual bool OnMouseWheel( SHORT delta, USHORT flags, USHORT screenX, USHORT screenY);
                virtual Bool OnSetCursor( HWND, Int, Int );
                virtual Bool OnMessage( HWND, UINT, WPARAM, LPARAM, Int&);
                virtual Int DefaultHandler( HWND, UINT, WPARAM, LPARAM );
                static LRESULT CALLBACK WindowProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

                HWND hWnd;  ///< Window handle of the window this class represents
        };
    }
}
#endif