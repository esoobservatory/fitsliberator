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
//---------------------------------------------------------------------------------------
// Includes and namespace imports
//---------------------------------------------------------------------------------------
#include "PropertySheet.h"

using namespace FitsLiberator::Windows;

//---------------------------------------------------------------------------------------
// Constructors and destructors
//---------------------------------------------------------------------------------------

/**
 * Default constructor.
 */
PropertySheet::PropertySheet() {
    tabHandle = NULL;
}

/**
 * Destructor.
 */
PropertySheet::~PropertySheet() {
}

//---------------------------------------------------------------------------------------
// Public methods
//---------------------------------------------------------------------------------------

/**
 * Adds a page to the property sheet. Calling this method after a call to PropertySheet::Create
 * has no effect.
 * @param page PropertyPage to add.
 */
Void PropertySheet::AddPage( PropertyPage* page ) {
    pages.push_back( page );
}

/**
 * Creates the property sheet as a control, that is as a child inside another window.
 * @param hWndParent Parent window.
 * @param bounds Boundaries of the control. Make sure all pages can fit inside this.
 */
Bool PropertySheet::Create( HWND hWndParent, const LPRECT bounds ) {
    Int pageCount = pages.size();

    PROPSHEETHEADER psh;
    PROPSHEETPAGE*  psp = new PROPSHEETPAGE[ pageCount ];
    Bool result = false;

    //
    // Initialize each page
    for( Int i = 0; i < pageCount; i++ ) {
        ::ZeroMemory( &(psp[i]), sizeof(PROPSHEETPAGE) );

        psp[i].dwSize = sizeof(PROPSHEETPAGE);
        psp[i].dwFlags = PSP_DEFAULT;
        psp[i].hInstance = g_hInstance;
        psp[i].pszTemplate = MAKEINTRESOURCE(pages[i]->getId());
        psp[i].pfnDlgProc = FitsLiberator::Windows::PropertyPageProc;
        psp[i].pszTitle = "";
        psp[i].lParam = (LPARAM)pages[i];
        psp[i].pszHeaderSubTitle = "";
        psp[i].pszHeaderTitle = "";
    }

    //
    // Initialize the header
    ::ZeroMemory( &psh, sizeof( PROPSHEETHEADER ) );

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_MODELESS | PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP | PSH_PROPSHEETPAGE | PSH_USECALLBACK;
    psh.hInstance = g_hInstance;
    psh.hwndParent = hWndParent;
    psh.pszCaption = "";
    psh.nPages = pageCount;
    psh.nStartPage = 0;
    psh.ppsp = psp;
    psh.pfnCallback = FitsLiberator::Windows::PropertySheetProc;
	
    //
    // Show the property sheet
    this->hWnd = (HWND) ::PropertySheet(&psh);
    if( this->hWnd != NULL ) {
        setBounds( bounds );
        
        modifyStyle( GWL_EXSTYLE, 0, WS_EX_CONTROLPARENT|WS_EX_WINDOWEDGE );
        modifyStyle( GWL_STYLE, WS_DLGFRAME, WS_TABSTOP );

        tabHandle = ::FindWindowEx( this->hWnd, NULL, WC_TABCONTROL, NULL );

		::ShowWindow(::GetDlgItem(this->hWnd, IDOK), SW_HIDE);
		::ShowWindow(::GetDlgItem(this->hWnd, IDCANCEL), SW_HIDE);

        result = true;
    }
    
    delete[] psp;
    return result;
}


HWND PropertySheet::getTabHandle() {
    return tabHandle;
}

//---------------------------------------------------------------------------------------
// Static functions
//---------------------------------------------------------------------------------------

/**
 * Called by the ::PropertySheet function when the property sheet is created.
 * This allows us to override its default style, so that we can embed it in
 * the main dialog. This function is loosely based on the MFC function
 * AfxPropSheetCallback.
 */
Int CALLBACK FitsLiberator::Windows::PropertySheetProc( HWND, UINT uMsg, LPARAM lParam ) {
    if( PSCB_PRECREATE == uMsg ) {
        LPDLGTEMPLATE lpTemplate = (LPDLGTEMPLATE)lParam;

        // Mark the dialog template as read-write.
		DWORD dwOldProtect;
        ::VirtualProtect(lpTemplate, sizeof(DLGTEMPLATE), PAGE_READWRITE, &dwOldProtect);

        // Modify the style bits
        DWORD style = WS_CHILD | WS_VISIBLE | WS_TABSTOP;
        if( lpTemplate->style & DS_SETFONT )
            lpTemplate->style = style | DS_SETFONT;
        else
            lpTemplate->style = style & ~DS_SETFONT;
        lpTemplate->dwExtendedStyle = WS_EX_CONTROLPARENT;
        return TRUE;
    }
    return FALSE;
}

/**
 * PropertyPage dialog procedure - delegates all work to a PropertyPage instance.
 * @param hDlg Window handle of the property page
 * @param uMsg Message
 * @param wParam Message parameter 1
 * @param lParam Message parameter 2, this is a LPPROPSHEETPAGE when uMsg = WM_INITDIALOG
 * @return Depends on uMsg, see Platform SDK for more information.
 * @remarks We need this special version of FitsLiberator::Windows::DialogProc because 
 * initialization happens differently for property pages.
 */
BOOL CALLBACK FitsLiberator::Windows::PropertyPageProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) { 
    // Because the FitsLiberator::Windows namespace implements a UI framework
    // many dialog instances may use this function simultaneously. To ensure
    // we notify the correct Dialog instance, we use the Windows property system
    // to associate a hDlg to a Dialog*. For this purpose WM_INITDIALOG is very useful
    // because Windows lets us pass a Dialog* as the lParam for WM_INITDIALOG.
    if( WM_INITDIALOG == uMsg ) {
        LPPROPSHEETPAGE propertyPage = (LPPROPSHEETPAGE)lParam;
        ::SetWindowLong( hDlg, GWL_USERDATA, (LONG)propertyPage->lParam );
    }

    // Now we can just retrieve the Dialog* and pass the message along.
    Dialog* owner = (Dialog*) ::GetWindowLong( hDlg, GWL_USERDATA );
    if( NULL != owner )
        return owner->ProcessMessage(hDlg, uMsg, wParam, lParam);

    // We shouldn't get here, but to play nice we return FALSE => we didn't
    // handle the message.
    return FALSE;
}
