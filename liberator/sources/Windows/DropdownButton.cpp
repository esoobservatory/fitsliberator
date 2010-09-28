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
#include "DropdownButton.h"
#include "Menu.h"
#include "Resources.h"

using namespace FitsLiberator;
using namespace FitsLiberator::Windows;

DropdownButton::DropdownButton( HWND buttonWindow ) : super( buttonWindow ) {
    this->oldWindowProc = NULL;
	this->menu = 0;
	this->resource = 0;

    if( ::IsWindow( buttonWindow ) ) {
        ::SetWindowLong( buttonWindow, GWL_USERDATA, (LONG)this );
        oldWindowProc = (WNDPROC)::SetWindowLong( buttonWindow, GWL_WNDPROC, (LONG)&DropdownButton::WindowProc );
    }
}

DropdownButton::~DropdownButton() {
}

LRESULT CALLBACK
DropdownButton::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    DropdownButton* owner = (DropdownButton*)::GetWindowLong( hWnd, GWL_USERDATA );

    LRESULT	result  = 0;
    switch( uMsg ) {
		case BM_DRAWARROW:
			owner->Paint(reinterpret_cast<LPNMCUSTOMDRAW>(lParam));
            break;
		case WM_COMMAND:
			::SendMessage(::GetParent(owner->getHandle()), uMsg, wParam, lParam);
			break;
		case WM_LBUTTONDOWN:
			result = ::CallWindowProc(owner->oldWindowProc, hWnd, uMsg, wParam, lParam);
			owner->ShowMenu();
			break;
        case WM_NCDESTROY:
            ::SetWindowLong( hWnd, GWL_WNDPROC, (LONG)owner->oldWindowProc );
            result = ::CallWindowProc( owner->oldWindowProc, hWnd, uMsg, wParam, lParam );
            delete owner;
            break;
        default:
            result = ::CallWindowProc( owner->oldWindowProc, hWnd, uMsg, wParam, lParam );
    }

    return result;
}

void
DropdownButton::ShowMenu() {
	RECT  bounds;
	POINT location;

	::GetWindowRect(hWnd, &bounds);
	location.x = bounds.left;
	location.y = bounds.bottom;

	SendMessage(BM_SETSTATE, TRUE, 0);

	if( menu != 0 )
		Menu::Popup( menu, hWnd, location );
	else if( resource != 0 )
		Menu::Popup( resource, hWnd, location );

	SendMessage(BM_SETSTATE, FALSE, 0);
}

void
DropdownButton::Paint(LPNMCUSTOMDRAW pncd) const {
    using Gdiplus::Point;

	RECT rc              = {0};
    int  height;

	::GetClientRect(hWnd, &rc);
    height = rc.bottom - rc.top;

	Gdiplus::Graphics g(pncd->hdc);
	Gdiplus::SolidBrush b(Gdiplus::Color::Black);

    Point points[3] = {
	    Point(rc.right - (4 * height) / 6, (rc.bottom - rc.top) / 2),   // Top-Left
	    Point(rc.right - (2 * height) / 6, (rc.bottom - rc.top) / 2),   // Top-Right
	    Point(rc.right - (3 * height) / 6, rc.bottom - height / 3)     // Bottom-Middle
    };
	g.FillPolygon(&b, points, 3);
}

HMENU
DropdownButton::Menu() const {
	return this->menu;
}

void
DropdownButton::Menu(HMENU menu) {
	if( this->menu != 0 )
		::DestroyMenu(this->menu);
	this->menu = menu;
}

int
DropdownButton::Resource() const {
	return this->resource;
}

void
DropdownButton::Resource(int id) {
	this->resource = id;
}
