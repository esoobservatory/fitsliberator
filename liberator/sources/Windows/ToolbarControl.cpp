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
#include "ToolbarControl.h"
#include "ResourceManager.h"
#include "Environment.h"

using namespace FitsLiberator::Windows;

const Int ToolbarControl::imageWidth = 22;
const Int ToolbarControl::imageHeight = 22;

/**
 * Default constructor.
 */ 
ToolbarControl::ToolbarControl() {
    normalImageList = CreateImageList( IDB_TOOL_NORMAL );
    hotImageList    = CreateImageList( IDB_TOOL_HOT );
}

/**
 * Destructor.
 */
ToolbarControl::~ToolbarControl() {
    ImageList_Destroy( normalImageList );
    ImageList_Destroy( hotImageList );
}

/**
 * Creates the control.
 * @param hWndParent Parent window.
 * @param bounds Boundaries of the control.
 * @return True on success, false on failure.
 */
Bool ToolbarControl::Create( HWND hWndParent, const LPRECT bounds ) {
    //
    // Create the toolbar(s)
    Int y = bounds->top;
    bars[ButtonMove] = createBar( hWndParent, bounds->left, &y, IDC_TOOL_MOVE, ButtonMove );
    bars[ButtonZoom] = createBar( hWndParent, bounds->left, &y, IDC_TOOL_ZOOM, ButtonZoom );
    y = y + 12;

    bars[ButtonBlack] = createBar( hWndParent, bounds->left, &y, IDC_TOOL_BLACK, ButtonBlack );
    bars[ButtonWhite] = createBar( hWndParent, bounds->left, &y, IDC_TOOL_WHITE, ButtonWhite );
    y = y + 12;

    bars[ButtonBackground] = createBar( hWndParent, bounds->left, &y, IDC_TOOL_BACKGROUND, ButtonBackground );
    bars[ButtonPeak] = createBar( hWndParent, bounds->left, &y, IDC_TOOL_PEAK, ButtonPeak );
    
    return true;
}

/**
 * Creates an image list.
 * @param id Bitmap resource id.
 * @remark This method assumes that id+1 is the 32-bit toolbar image. Further images
 * can be added to support different color depths.
 * @return A handle to the imagelist or NULL on failure.
 */
HIMAGELIST ToolbarControl::CreateImageList( Int id ) {
    HIMAGELIST imageList = NULL;

    OSVERSIONINFO ovi;
    ovi.dwOSVersionInfoSize = sizeof(ovi);
    ::GetVersionEx( &ovi );

    if( ovi.dwMajorVersion >= 5 && ovi.dwMinorVersion >= 1 ) {
        // Windows XP or above
        Gdiplus::Bitmap* bitmap = ResourceManager::LoadImage( MAKEINTRESOURCE(id), "PNG" );
        if( bitmap != NULL ) {
            HBITMAP hBitmap;
            bitmap->GetHBITMAP(Gdiplus::Color(0xFFFFFFFF), &hBitmap);
            
            imageList = ImageList_Create( imageWidth, imageHeight, ILC_COLOR32, 6, 1 );
            ImageList_Add( imageList, hBitmap, NULL );

            ::DeleteObject( hBitmap );
            delete bitmap;
        }
    }
    else {
        HBITMAP hBitmap = (HBITMAP)::LoadImageA( g_hInstance, MAKEINTRESOURCE( id ), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION );

        imageList = ImageList_Create( imageWidth, imageHeight, ILC_MASK | ILC_COLOR24, 6, 1 );
        ImageList_AddMasked( imageList, hBitmap, RGB(255, 0, 255) );

        ::DeleteObject( hBitmap );
    }

    IMAGEINFO ii;
    ImageList_GetImageInfo( imageList, 0, &ii );

    return imageList;
}

/**
 * Creates a Common Controls toolbar.
 * @param hWndParent Parent window.
 * @param x X-coordinate of the toolbar.
 * @param y Y-coordinate of the toolbar.
 * @param command Command ID of the button added to the toolbar.
 * @param image Image index.
 * @return Handle to the toolbar or NULL on failure.
 * @remark The y parameter is updated to account for the height of the toolbar.
 */
HWND ToolbarControl::createBar( HWND hWndParent, Int x, Int* y, Int command, Int image ) {
    HWND bar = CreateWindowEx( 0, TOOLBARCLASSNAME, NULL,
        WS_CHILD | TBSTYLE_TRANSPARENT | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | CCS_NODIVIDER | CCS_NOPARENTALIGN | CCS_NORESIZE, 0, 0, 0, 0, hWndParent, 
        NULL, g_hInstance, NULL );

    if( NULL != bar ) {
        RECT rc;

        //
        // Must send this message for backwards compatibility
        ::SendMessage( bar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof( TBBUTTON ), 0 );
        
        //
        // Setup image lists
        ::SendMessage( bar, TB_SETIMAGELIST, 0, (LPARAM)normalImageList );
        ::SendMessage( bar, TB_SETHOTIMAGELIST, 0, (LPARAM)hotImageList );

        //
        // Add the button
        TBBUTTON button;
        button.iBitmap = image; 
        button.idCommand = command; 
        button.fsState = TBSTATE_ENABLED; 
        button.fsStyle = BTNS_CHECK; 
        button.dwData = 0;
        button.iString = 0;
        ::SendMessage( bar, TB_ADDBUTTONS, 1, (LPARAM)&button );
        //::SendMessage( bar, TB_AUTOSIZE, 0, 0 );
        ::SendMessage( bar, TB_SETROWS, MAKEWPARAM(1, TRUE), (LPARAM)&rc );

        //
        // Position the bar
        ::SetWindowPos( bar, NULL, x, *y, 29, 29, SWP_SHOWWINDOW );
        *y = *y + 29; 
        return bar;
    }
    return NULL;
}


/**
 * Updates the state of a toolbar button
 * @param index Index of the button.
 * @param state State (checked, enabled...)
 */
Void ToolbarControl::updateButton( UInt index, Bool selected ) {
    TBBUTTON b;
    ::SendMessage( bars[index], TB_GETBUTTON, 0, (LPARAM)&b );


    TBBUTTONINFO bi;
    bi.cbSize = sizeof( TBBUTTONINFO );
    bi.dwMask = TBIF_STATE;
    bi.fsState = TBSTATE_ENABLED | (selected?TBSTATE_CHECKED:0);

    ::SendMessage( bars[index], TB_SETBUTTONINFO, b.idCommand, (LPARAM)&bi );
}
