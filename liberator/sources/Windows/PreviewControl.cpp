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
#include "PreviewControl.h"
#include "Menu.h"
#include "ResourceManager.h"
#include "SelectionTrackerSink.h"
#include "Environment.h"

#include <sstream>

using namespace Gdiplus;
using namespace FitsLiberator::Modelling;
using namespace FitsLiberator::Windows;

// @todo
// - Cache grayscale palette to reduce overhead when painting.
// - Cache mask palette (one shared should be sufficient)


/**
 * Default constructor.
 */
PreviewControl::PreviewControl( PreviewModel& m, PreviewController& c, ToolModel& t,
							   GlobalSettingsModel& o, FlowController& fc, ProgressModel& pm )
  : model( m ),
    controller( c ),
    options( o ),
    tool( t ),
	flowController( fc ),
	progressModel(pm)
{

    previewImage = NULL;
    undefinedMask = NULL;
    blackMask = NULL;
    whiteMask = NULL;

    throbber = ResourceManager::LoadImage(IDB_THROBBER);
    progress = -1;
    max_progress = throbber->GetWidth() / throbber->GetHeight();
	throbberFont = new Gdiplus::Font(L"Segoe UI", 8.0f);
}

PreviewControl::~PreviewControl() {
    //destroyBitmaps();

    delete throbber;

    delete mouseTracker;
    delete selectionTracker;
    delete moveTracker;

	delete throbberFont;
}

/**
 * Creates the control.
 * @param hWndParent Parent window.
 * @param bounds Boundaries of the control. Note that ((bounds->right - bounds->left) % 4) must be 0. This is to ensure that the preview image is aligned on word boundaries.
 * To ensure this, the preview control may increase its size by up to 3 pixels.
 * @return True on success, false on failure.
 */
Bool PreviewControl::Create( HWND hWndParent, const LPRECT bounds ) {
    RECT rc = *bounds;

    // Make sure that the preview width is divisible by 4 (to ensure word alignment in the preview image)
    // This is done by making the preview slightly wider than it wouuld otherwise be (max 3 pixel).
    int padding = (rc.right - rc.left) % 4; // Get the number of pixels the preview should be wider
    int oddpadding = padding % 2;           // oddpadding is 1 if padding == 1 || padding == 3
    padding /= 2;                           // Distribute the padding on either side

    rc.left -= padding;                     // Modify the size of the control
    rc.right += padding + oddpadding;

    return super::Create( hWndParent, &rc, WS_VISIBLE | WS_CHILD, 0 );
}

/**
 * @see Window::OnCreate
 */
Void PreviewControl::OnCreate( HWND hWnd ) {
    super::OnCreate( hWnd );

	RECT rc;
	::GetClientRect( hWnd, &rc );
	FitsLiberator::Size size( rc.right - rc.left, rc.bottom - rc.top );
	model.setPreviewSize( size );

    //
    // Create a mouse tracker
    selectionTracker = new SelectionTrackerSink( hWnd, rc );
    moveTracker = new MoveTrackerSink();
    mouseTracker = new MouseTracker( selectionTracker, rc );
}

/**
 * Implements custom painting.
 * @see CanvasControl::OnRefresh
 */
Void PreviewControl::OnRefresh( HDC hDC ) {
    Graphics g( hDC );

    if( options.getPreviewEnabled() ) {
        // 
        // Make sure we have created the bitmaps
        //if( NULL == previewImage ) {
        //    if( !createBitmaps() ) {
        //        Trace( "Failed to create the preview bitmap and or masks" );
        //    }
        //}

        if( NULL != previewImage ) {
            g.DrawImage( previewImage, 0, 0 );
        }

        if( options.getMarkUndefined() && undefinedMask != NULL ) {
            g.DrawImage( undefinedMask, 0, 0 );
        }
        
        if( options.getMarkBlackClipping() && whiteMask != NULL ) {
            g.DrawImage( blackMask, 0, 0 );
        }
        
        if( options.getMarkWhiteClipping() && blackMask != NULL ) {
            g.DrawImage( whiteMask, 0, 0 );
        }

        // Paint the selection
        if( selectionTracker->hasSelection() ) {
            RECT selection = selectionTracker->getSelection();
            Gdiplus::Rect rect( selection.left, selection.top, selection.right - selection.left, selection.bottom - selection.top);
            COLORREF highlightColor = ::GetSysColor( COLOR_HIGHLIGHT );
            Gdiplus::Pen pen( Gdiplus::Color( 0xFF, GetRValue( highlightColor ), GetGValue( highlightColor ), GetBValue( highlightColor ) ) );
            Gdiplus::SolidBrush brush( Gdiplus::Color( 0x7F, GetRValue( highlightColor ), GetGValue( highlightColor ), GetBValue( highlightColor ) ) );
            g.FillRectangle( &brush, rect );
            g.DrawRectangle( &pen, rect );
        }

        if(progress >= 0) {
            Gdiplus::SizeF size;
            FitsLiberator::Rectangle bounds = getClient();
            unsigned int height = 0;

            throbber->GetPhysicalDimension(&size);
            height = (unsigned int)size.Height;

            //Gdiplus::Rect rect(bounds.left, bounds.top, bounds.getWidth(), bounds.getHeight());
            //Gdiplus::Rect rect(
            //    bounds.getWidth() / 2 - height * 2, bounds.getHeight() / 2 - 2 * height / 3, 
            //    height * 4, height * 3);
            //Gdiplus::SolidBrush brush(Gdiplus::Color(0xBF, 0xFF, 0xFF, 0xFF));
            //g.FillRectangle(&brush, rect);
            
            g.DrawImage(throbber,
                (bounds.getWidth()  - height) / 2,
                (bounds.getHeight() - height) / 2,
                progress * height, 0, height, height,
                Gdiplus::UnitPixel);


			RectF throbberBox(
				(bounds.getWidth()  - height) / 2, 
				bounds.getHeight() / 2,				// (bounds.getHeight() - height) / 2 + height / 2
				height, height / 2);
			
			StringFormat format(StringFormat::GenericDefault());
			format.SetAlignment(Gdiplus::StringAlignmentCenter);
			format.SetLineAlignment(Gdiplus::StringAlignmentCenter);

			SolidBrush brush(Gdiplus::Color::Black);

			std::wstringstream s;
			s << progressModel.Progress() << L"%";
			g.DrawString(s.str().c_str(), s.str().size(), throbberFont, throbberBox, &format, &brush);
        }
    }
    else {
        Bitmap brushImage(g_hInstance, (const WCHAR*)MAKEINTRESOURCE( IDB_BRUSH_TRANSPARENT ) );
        TextureBrush brush(&brushImage );
        FitsLiberator::Rectangle bounds = getClient();
        g.FillRectangle( &brush, bounds.left, bounds.top, bounds.getWidth(), bounds.getHeight() );
    }
}

/**
 * @see CanvasControl::OnToolChanged
 */
Void PreviewControl::OnToolChanged( ToolTypeFunction tool ) {
    super::OnToolChanged( tool );

    switch( tool ) {
        case kFITSToolFunctionZoomIn:
        case kFITSToolFunctionZoomOut:
            mouseTracker->setTrackerSink( selectionTracker );
            break;
        default:
            mouseTracker->setTrackerSink( moveTracker );
    }
}

/**
 * @return True if the control can zoom in, false otherwise.
 */
Bool PreviewControl::canZoomIn() {
    return model.canIncrement();
}

/**
 * @return True if the control can zoom out, false otherwise.
 */
Bool PreviewControl::canZoomOut() {
    return model.canDecrement();
}

/**
 * Forces the control to reexamine the preview bitmap and masks, then repaints the window.
 */
Void PreviewControl::Update() {
    Refresh();
}

void PreviewControl::gotBusy() {
    if(progress == -1) {
        progress = 0;
        Refresh();
        ::SetTimer(hWnd, 0, 50 /* kFITSProgressDelay */, NULL);
    }
}

void PreviewControl::gotDone() {
    createBitmaps();
    if(progress != -1) {
        // Preview is not busy, but we indicating that it is
        ::KillTimer(hWnd, 0);
        progress = -1;
        Refresh();
    }
}

/**
 * Informs the preview controller about a left click
 * @param pt Location of the click
 */
Void PreviewControl::handleLeftClick( FitsLiberator::Point& pt ) {
    switch( tool.getCurrentFunction() )
	{
		case kFITSToolFunctionZoomIn:
            flowController.incrementZoom( pt );
            break;
		case kFITSToolFunctionZoomOut:
		    flowController.decrementZoom( pt );
    		break;
        case kFITSToolEyedropperPeak:
			flowController.peakGuess( pt );
            break;
		case kFITSToolEyedropperBackground:
			flowController.backgroundGuess( pt );
			break;
		case kFITSToolEyedropperBlackLevel:
			flowController.pickBlackLevel( pt );
			break;
		case kFITSToolEyedropperWhiteLevel:
			flowController.pickWhiteLevel( pt );
			break;
	}
}

/**
 * Handles a rightclick, shows the context menu.
 * @param pt Location of the click.
 */
Void PreviewControl::handleRightClick( POINT* pt ) {
    ::ClientToScreen( hWnd, pt );
    
    //Environment::setCursor( kPICursorArrow );
    Menu::Popup( IDM_PREVIEW_MENU, ::GetParent(hWnd), *pt );
    super::OnSetCursor( hWnd, 0, 0 );
}

/**
 * Destroys the bitmaps used for displaying the preview.
 */
Void PreviewControl::destroyBitmaps() {
    destroyImage( &previewImage );
    destroyImage( &undefinedMask );
    destroyImage( &blackMask );
    destroyImage( &whiteMask );
}

/**
 * Destroys a single bitmap, and makes sure its pointer is zeroed.
 * @param ppImage This image to destroy.
 */
Void PreviewControl::destroyImage( Gdiplus::Bitmap** ppImage ) {
    if( NULL != *ppImage ) {
        delete *ppImage;
        *ppImage = NULL;
    }
}

/**
 * Creates the bitmaps used for displaying the preview.
 */
Bool PreviewControl::createBitmaps() {
    destroyBitmaps();

    PreviewImage& bitmaps = model.getPreviewImage();
	FitsLiberator::Size size = model.getPreviewSize();

    previewImage = createPreview( size, bitmaps.previewPixels );
    undefinedMask = createMask( size, Gdiplus::Color::Red, bitmaps.nullMap );
    whiteMask = createMask( size, Gdiplus::Color::Lime, bitmaps.whiteClippedMap );
    blackMask = createMask( size, Gdiplus::Color::Blue, bitmaps.blackClippedMap );

    return (previewImage != NULL && undefinedMask != NULL && whiteMask != NULL && blackMask != NULL);
}

/**
 * Utility method, creates the preview image bitmap and modifies the palette to
 * be a grayscale palette.
 * @param size Size of the image.
 * @param pixels The pixels.
 */
Gdiplus::Bitmap* PreviewControl::createPreview( const FitsLiberator::Size& size, Byte* pixels ) {
    Bitmap* previewImage = new Bitmap( size.width, size.height, size.width, PixelFormat8bppIndexed, pixels );
    UInt paletteSize = previewImage->GetPaletteSize();
    if( paletteSize > 0 ) {
        ColorPalette* palette = (ColorPalette*)malloc( paletteSize );
        previewImage->GetPalette( palette, paletteSize );
        for( UInt i = 0; i < palette->Count; i++ ) {
            palette->Entries[i] = Gdiplus::Color::MakeARGB( 0xFF, (Byte)i, (Byte)i, (Byte)i );
        }
        previewImage->SetPalette( palette );
        free( palette );
    }

    return previewImage;
}

/**
 * Utiliy method, creates a mask bitmap and modifes the palette accordingly.
 * @param size Size of the mask.
 * @param color Color of the mask.
 * @param pixels The pixels.
 */
Gdiplus::Bitmap* PreviewControl::createMask( const FitsLiberator::Size& size, const Gdiplus::Color& color, Byte* pixels ) {
    Bitmap* mask = new Bitmap( size.width, size.height, size.width, PixelFormat8bppIndexed, pixels );
    UInt paletteSize = mask->GetPaletteSize();
    if( paletteSize > 0 ) {
        ColorPalette* palette = (ColorPalette*)malloc( paletteSize );
        mask->GetPalette( palette, paletteSize );
        if( palette->Count >= 256 ) {
            palette->Entries[255] = 0;  // Transparent black
            palette->Entries[0]   = Gdiplus::Color::MakeARGB( 0xFF, color.GetR(), color.GetG(), color.GetB() );
        }
        mask->SetPalette( palette );
        free( palette );
    }   

    return mask;
}

bool PreviewControl::OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, Int& result) {
    switch(uMsg) {
    case WM_TIMER:
        result = OnTimer((UINT)wParam);
        return true;
    default:
        return super::OnMessage(hWnd, uMsg, wParam, lParam, result);
    }
}

int PreviewControl::OnTimer(UINT_PTR id) {
    switch(id) {
    case 0:
        progress += 1;
        if(progress >= max_progress) {
            progress = 0;
        }
        Refresh();
        return 0;
    }
    return 1;
}

/**
 * @see Window::OnMouseDown
 */
Void PreviewControl::OnMouseDown( Int button, Int x, Int y ) {
    super::OnMouseDown( button, x, y );

	// Steal foucus
	::ReleaseCapture();
	::SetFocus( hWnd );

    this->button = button;

    POINT pt = {x,y};

    //
    // Update the active region to ensure the user can only select inside
    // the actual image, not the black border around it.
    FitsLiberator::Rectangle& imageBounds = model.getImageArea();
    RECT activeBounds;
    activeBounds.left   = imageBounds.left;
    activeBounds.top    = imageBounds.top;
    activeBounds.right  = imageBounds.right;
    activeBounds.bottom = imageBounds.bottom;
    mouseTracker->setActiveRegion( activeBounds );
    selectionTracker->setActiveRegion( activeBounds );

    //
    // Handle the event
    mouseTracker->OnMouseDown( hWnd, button, x, y );
}

/**
 * @see Window::OnMouseMove
 */
Void PreviewControl::OnMouseMove( Int button, Int x, Int y ) {
    super::OnMouseMove( button, x, y );

    POINT pt;
    ::GetCursorPos( &pt );
    ::ScreenToClient( hWnd, &pt );

    RECT rc;
    ::GetClientRect( hWnd, &rc );

    //
    // Update the picker
    if( ::PtInRect( &rc, pt ) )
		flowController.setCoordinates( FitsLiberator::Point( x, y ) );
		//        value.setCoordinates( FitsLiberator::Point( x, y ) );

    //
    // Handle dragging
    if( MouseActionTracking == mouseTracker->OnMouseMove( hWnd, button, x, y ) ) {
        if( kFITSToolFunctionMove == tool.getCurrentFunction() ) {
            SIZE sz = moveTracker->getDistance();
            //controller.movePreview( MovementVector( sz.cx, sz.cy ) );
			flowController.movePreview( MovementVector( sz.cx, sz.cy ) );
            moveTracker->Reset();
        }        
    }
}

/**
 * @see Window::OnMouseUp
 */
Void PreviewControl::OnMouseUp( Int, Int x, Int y ) {
    super::OnMouseUp( button, x, y );

    POINT pt;
    ::GetCursorPos( &pt );
    ::ScreenToClient( hWnd, &pt );
    ToolTypeFunction function = tool.getCurrentFunction();

    switch( mouseTracker->OnMouseUp( hWnd, button, x, y ) ) {
        case MouseActionLeftClick:
            handleLeftClick( FitsLiberator::Point( pt.x, pt.y ) );
            break;
        case MouseActionRightClick:
            handleRightClick( &pt );
            break;
        case MouseActionTracking:
            if( kFITSToolFunctionZoomIn == function || kFITSToolFunctionZoomOut == function ) {
                RECT& selection = selectionTracker->getSelection();
//                controller.zoomRectangle( FitsLiberator::Rectangle( selection.left, selection.top, selection.right, selection.bottom ) );
				flowController.zoomRectangle( FitsLiberator::Rectangle( selection.left, selection.top, selection.right, selection.bottom ) );
            }
            break;
        default:
            if( button == MK_LBUTTON )
                handleLeftClick( FitsLiberator::Point( pt.x, pt.y ) );
            else if( button == MK_RBUTTON )
                handleRightClick( &pt );
    }

    OnToolChanged( function );  // To make sure the cursor is updated
}

bool PreviewControl::OnMouseWheel(SHORT delta, USHORT flags, USHORT screenX, USHORT screenY) {
    if(flags & MK_CONTROL) {    // Ctrl key is down
        POINT pt;
        pt.x = screenX;
        pt.y = screenY;
        ::ScreenToClient( hWnd, &pt );
        if(delta > 0) {
            flowController.incrementZoom(FitsLiberator::Point( pt.x, pt.y ));
        } else {
            flowController.decrementZoom(FitsLiberator::Point( pt.x, pt.y ));
        }
        return true;
    }
    return false;
}