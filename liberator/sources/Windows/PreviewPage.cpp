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
#include "PreviewPage.h"
#include "TooltipManager.h"
#include "ResourceManager.h"
#include "Environment.h"
#include "BitmapButton.h"

using namespace Gdiplus;
using namespace FitsLiberator::Modelling;
using namespace FitsLiberator::Windows;

/**
 * Default contructor.
 * @param view The event dispatcher to use.
 */
PreviewPage::PreviewPage( DispatchView& view )
  : PropertyPage( IDD_PREVIEW ),
    dispatchView( view ), ti(NULL) {
}

/**
*  Destructor
*/
PreviewPage::~PreviewPage()
{
	if ( this->ti != NULL )
		delete ti;
}

/**
 * @see Dialog::OnInit
 */
Bool PreviewPage::OnInit( HWND hWnd ) {
    if( !super::OnInit( hWnd ) )
        return false;

    // Equip the two auto buttons with icons
    BitmapButton* button;
    button = new BitmapButton(
        ResourceManager::LoadImage( IDB_USEBLACKLEVEL ),
        getItem( IDC_STRETCH_AUTOBACKGROUND )
    );
    
    button = new BitmapButton(
        ResourceManager::LoadImage( IDB_USEWHITELEVEL ),
        getItem( IDC_STRETCH_AUTOPEAK )
    );
	
    
    dispatchView.OnInit( this, IDD_PREVIEW );

    if( Environment::isShowToolTips() )
        TooltipManager::OnInit( this );

	setupWarningIcon();

    return true;
}

Void PreviewPage::showWarningIcon( Bool show )
{
	HWND hwndWarning = getItem(IDC_WARNING_NETWORK);
	if ( show && ti->lpszText == "" )
	{
		String toolTip = ResourceManager::LoadString(IDC_WARNING_NETWORK);
		ti->lpszText = const_cast<char*>(toolTip.c_str());
		
		::SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) ti);
		::ShowWindow( hwndWarning, SW_SHOW );	
		::EnableWindow( hwndTT, true );		
	}
	else if ( !show && ti->lpszText != "" )
	{
		ti->lpszText = "";
		::ShowWindow(hwndWarning, SW_HIDE);
		::EnableWindow( hwndTT, false );
		::SendMessage(hwndTT, TTM_DELTOOL, 0, (LPARAM) (LPTOOLINFO) ti);
	}
	
}

Void PreviewPage::setupWarningIcon( )
{
	HWND hwndWarning = getItem(IDC_WARNING_NETWORK);
	HICON hiconWarning = reinterpret_cast<HICON>(::LoadImageA(NULL, IDI_WARNING, 
		IMAGE_ICON, 16, 16, LR_SHARED));
	if(hiconWarning) {
		::SendMessage(hwndWarning, STM_SETICON, 
			reinterpret_cast<WPARAM>(hiconWarning), 0);
	}

	hwndTT = CreateWindowEx(WS_EX_TOPMOST,
	TOOLTIPS_CLASS, NULL,
	WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,		
	CW_USEDEFAULT, CW_USEDEFAULT,
	CW_USEDEFAULT, CW_USEDEFAULT,
	hWnd, NULL, g_hInstance,NULL);

	SetWindowPos(hwndTT, HWND_TOPMOST,
		0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	
	// Set up "tool" information.
	// In this case, the "tool" is the entire parent window.
	ti = new TOOLINFO();
	ti->cbSize = sizeof(TOOLINFO);
	ti->uFlags = TTF_SUBCLASS;
	ti->hwnd = hWnd;
	ti->hinst = g_hInstance;
	ti->lpszText = "";
	
	
	RECT rectWar;
	RECT rectDia;
	
	::GetWindowRect(hwndWarning, &rectWar);
	::GetWindowRect(hWnd, &rectDia);

	ti->rect.bottom = rectWar.bottom - rectDia.top;
	ti->rect.left = rectWar.left - rectDia.left;
	ti->rect.right = rectWar.right - rectDia.left;
	ti->rect.top = rectWar.top - rectDia.top;

	::ShowWindow(hwndWarning, SW_HIDE);
	::EnableWindow( hwndTT, false );

	
		
}

/**
 * @see Dialog::OnCommand
 */
Void PreviewPage::OnCommand( WPARAM wParam, LPARAM lParam ) {
    dispatchView.OnCommand( wParam, lParam );
}

/**
 * @see Window::OnNotify
 */
LRESULT PreviewPage::OnNotify( Int control, NMHDR* e ) {
    dispatchView.OnNotify( control, e );
    TooltipManager::OnNotify( e );

	switch( control ) {
		case IDC_STRETCH_FUNCTION:
		case IDC_PREVIEW_ZOOMLIST:
			if( e->code == NM_CUSTOMDRAW ) {
				LPNMCUSTOMDRAW pncd = reinterpret_cast<LPNMCUSTOMDRAW>(e);
				switch( pncd->dwDrawStage ) {
					case CDDS_PREPAINT:
						::SetWindowLong(hWnd, DWL_MSGRESULT, CDRF_NOTIFYPOSTPAINT);
						return TRUE;
					case CDDS_POSTPAINT:
						::SendMessage(getItem(control), BM_DRAWARROW, 0, reinterpret_cast<LPARAM>(pncd));
						::SetWindowLong(hWnd, DWL_MSGRESULT, CDRF_NOTIFYPOSTPAINT);
						return TRUE;						
				}
			}
	}

    return super::OnNotify( control, e );
}

/**
 * @see Window::OnVerticalScroll.
 */
Void PreviewPage::OnVerticalScroll( WPARAM wParam, LPARAM lParam ) {
    dispatchView.OnScroll( wParam, lParam );
}
