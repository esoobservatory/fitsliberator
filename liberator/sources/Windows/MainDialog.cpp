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
#include "AlphaWindow.h"
#include "MainDialog.h"
#include "TooltipManager.h"
#include "ResourceManager.h"
#include "Environment.h"
#include "OptionsDialog.h"
#include "WindowsChangeManager.h"

using namespace Gdiplus;
using namespace FitsLiberator;
using namespace FitsLiberator::Windows;

const RECT MainDialog::tabBounds = { 2, 2, 538, 388 };

MainDialog* MainDialog::instance      = NULL;
HHOOK       MainDialog::hook          = NULL;
UInt        MainDialog::keyboardFlags = 0;

/**
 * Default constructor.
 */
MainDialog::MainDialog( FitsLiberator::Modelling::ModelFramework& frameWork ) 
  : Dialog(IDD_MAIN),
    dispatchView( frameWork ), 
    tabs( dispatchView ) {

    shortcutsEnabled = true;
	dialogOpen       = false;
        
    currentTab = 0;
    logo = ResourceManager::LoadImage( MAKEINTRESOURCE( IDB_PREVIEW_LOGO ), "PNG" );

	icon = ::LoadIcon( g_hInstance, MAKEINTRESOURCE( IDI_FITSLIBERATOR ) );
	SendMessage( WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(icon) );
}

MainDialog::~MainDialog() {
// DO NOT delete the image, it crashes the host
//    delete logo;
//    delete sink;
	::DestroyIcon(icon);
}

Bool MainDialog::OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, Int &returnValue) {
    if(uMsg == WindowsChangeManager::CM_UPDATE) {
        reinterpret_cast<WindowsChangeManager*>(lParam)->Invoke();
        return true;
    } else {
        return super::OnMessage(hWnd, uMsg, wParam, lParam, returnValue);
    }
}

/**
 * @see Dialog::OnInit
 */
Bool MainDialog::OnInit( HWND hWnd ) {
    if( !super::OnInit( hWnd ) )
        return false;

    // 
    // Instruct the MVC framework to use this dialog to broker notifications between background threads
    // and the UI thread
    WindowsChangeManager* manager = dynamic_cast<WindowsChangeManager*>(dispatchView.frameWork.changeManager);
    manager->Broker(hWnd);

    setVisible(true);

	// teis: to fix bug#104 the main dialog should be visible during initial load, even though it is empty
	// teis: This is only applicable to Windows versions prior to Vista.
	OSVERSIONINFO versionInfo = {0};
	versionInfo.dwOSVersionInfoSize = sizeof( versionInfo );
	::GetVersionEx( &versionInfo );
	if( versionInfo.dwMajorVersion < 6 ) {
		setVisible( true ); 
	}


    RECT bounds;
    try {
        //
        // Create the tabs
        bounds = tabBounds;
        ::MapDialogRect( hWnd, &bounds );
        tabs.Create( hWnd, &bounds );

        //
        // Notify the views of this event
        dispatchView.OnInit( this, IDD_MAIN );
    }
    catch( CancelException ) {
        return false;
    }

    //
    // Init tooltips
    if( Environment::isShowToolTips() )
        TooltipManager::OnInit( this );

    //
    // Hook keyboard events, so that we can determine when users press
    // the ALT key.
    assert( instance == NULL );
    instance = this;    // I hate having to use a global, but there is no way to provide
                        // additional data with a hook (stupid decentralized arch)
    hook = ::SetWindowsHookEx( WH_KEYBOARD, MainDialog::KeyboardProc, NULL, ::GetCurrentThreadId() );
    spaceIsDown = ((::GetAsyncKeyState( VK_SPACE ) & 0x8000000) != 0);

    //
    // Figure out where to place the logo
    if( logo != NULL ) {
        ::GetClientRect( hWnd, &bounds );
        Gdiplus::SizeF size;
        logo->GetPhysicalDimension( &size );
        logoLocation.X = bounds.right - size.Width - 4;
        logoLocation.Y = bounds.bottom - size.Height - 4;
    }

    return true;
}

/**
 * @see Window::OnDestroy
 */
Void MainDialog::OnDestroy( HWND ) {
    reinterpret_cast<WindowsChangeManager&>(dispatchView.frameWork.changeManager).Broker(NULL);

    ::UnhookWindowsHookEx( hook );
    hook     = NULL;
    instance = NULL;
}

/**
 * @see Dialog::OnCommand
 */
Void MainDialog::OnCommand( WPARAM wParam, LPARAM lParam ) {
    Int command = LOWORD(wParam);
    switch( command ) {
        case IDOK:
        case IDC_OK:
            Close(IDOK);
            break;
        case IDCANCEL:
            Close(IDCANCEL);
            break;
        case IDC_OPTIONS:
            Options();
            break;
		case IDC_IO_SAVEFILE:
				dispatchView.frameWork.saveFile( openFileDialog( Environment::FILEMODE_SAVE ) );
			break;	
		case IDC_IO_OPENNEW:			
			{
				String fileName = openFileDialog( Environment::FILEMODE_OPEN );
				if ( ImageReader::IsSupported( fileName ) )
					dispatchView.frameWork.openNewFile( fileName );			
				else if ( fileName != "" )
					MainDialog::ShowFileOpenError();
			}
			break;
		case IDC_IO_EDIT:				
				dispatchView.frameWork.saveEdit( openFileDialog( Environment::FILEMODE_SAVE ) );
			break;
		case IDC_SHOWABOUT:
			{
				Gdiplus::Bitmap* bitmap = ResourceManager::LoadImage( MAKEINTRESOURCE( IDB_ABOUT ), "PNG" );
				if( NULL != bitmap ) {
					AlphaWindow window( *bitmap );
					window.Show( getHandle() );
				}
			}
			break;
        default:
            dispatchView.OnCommand( wParam, lParam );			
    }
}

/**
 * @see Window::OnNotify
 */
LRESULT MainDialog::OnNotify( Int control, NMHDR* e ) {
    dispatchView.OnNotify( control, e );
    TooltipManager::OnNotify( e );

    return super::OnNotify( control, e );
}

/**
 * @see Window:OnPaint
 */
Void MainDialog::OnPaint( HWND hWnd, HDC hDC, RECT* bounds, PAINTSTRUCT* ps ) {
    if( logo != NULL ) {
        Graphics g( hDC );

        Gdiplus::SizeF size;
        logo->GetPhysicalDimension( &size );
        g.DrawImage( logo, logoLocation.X, logoLocation.Y, size.Width, size.Height );
    }
}

/**
 * Helper routine.
 */
Void MainDialog::setAltDown( Bool altDown ) {
    if( TabCtrl_GetCurSel( tabs.getTabHandle() ) == 0 ) {
        // Only handle keyboard shortcuts on the preview page
        dispatchView.frameWork.toolController->changeToolFunction( altDown );
    }
}
/**
*	Platform specific file open dialog that returns a string with the 
*	absolute path to the file chosen by the user
*/
String MainDialog::openFileDialog( UInt mode )
{
	dialogOpen = true;
	String fileName = Environment::LoadFileDialog( dispatchView.frameWork.getFileName(),
												   mode, getHandle() );
	dialogOpen = false;
	if ( fileName != "" )
		tabs.setDefaultPage();
	
	return fileName;
}

/**
 * Handle shortcut keys.
 * @param down The keys is being pressed.
 * @param key Character of the key being pressed.
 */
Void MainDialog::handleShortcut( Bool down, UInt key ) {
    if( TabCtrl_GetCurSel( tabs.getTabHandle() ) == 0 ) {
        Char c = ::MapVirtualKey( key, 2 );

        // Only handle keyboard shortcuts on the preview page
        if( down ) {
            switch( c ) {
                case VK_SPACE:
                    // Space needs some special handling to allow the user to
                    // depress the space bar to temporarely change tool to the
                    // move tool

                    if( !spaceIsDown ) {
                        dispatchView.ShortcutFilter( down, key );
                        spaceIsDown = true;
                    }
                    break;
                case kFITSIncrementZoomShortcut:
                case kFITSDecrementZoomShortcut:
                    if( !shortcutsEnabled )
                        break;
                default:
                    dispatchView.ShortcutFilter( down, key );
            }
        }
        else {
            switch( c ) {
                case kFITSIncrementZoomShortcut:
                case kFITSDecrementZoomShortcut:
                    if( shortcutsEnabled )
                        dispatchView.ShortcutFilter( down, key );
                    break;
                case VK_SPACE:
                    spaceIsDown = false;
                default:
                    dispatchView.ShortcutFilter( down, key );
            }
        }
    }
}

Void MainDialog::ShowFileOpenError()
{
	Environment::showMessage("Could not load file", "The selected file is not supported" );
}

/**
 * Disables keyboard shortcuts.
 */
Void MainDialog::disableShortcuts() {
    this->shortcutsEnabled = false;	
}

/**
 * Enables keyboard shortcuts.
 */
Void MainDialog::enableShortcuts() {
    this->shortcutsEnabled = true;
}

void
MainDialog::Options() {
    OptionsDialog options(*dispatchView.frameWork.optionsModel, *dispatchView.frameWork.optionsController);
    options.Modal(this->getHandle());
}

/**
 * Keyboard hook, used to dispatch the ENTER keypress events to textboxes (as this message is eaten by the 
 * default dialog box message loop).
 * See the Platform SDK documentation for more information about keyboard hooks.
 */
LRESULT CALLBACK MainDialog::KeyboardProc( int nCode, WPARAM wParam, LPARAM lParam ) {
	if( HC_ACTION == nCode ) {
        switch( wParam ) {
            case VK_MENU:   // Alt key
                instance->setAltDown( (lParam & 0x80000000) == 0 );
                return 1;
            case VK_RETURN: // Enter key
                // Just forward it to the window that has focus (enter is eaten by the default dialogproc)
                if( !instance->shortcutsEnabled ) {
                    ::SendMessage( ::GetFocus(), WM_CHAR, wParam, lParam );
                    return 1;
                }
                break;
            case VK_ESCAPE:
				instance->handleShortcut( (lParam & 0x80000000) == 0, wParam );
				return 1;				
            default:
				if ( !(instance->dialogOpen) )
					instance->handleShortcut( (lParam & 0x80000000) == 0, wParam );				
        }
    }
    return CallNextHookEx( hook, nCode, wParam, lParam );
}

Void MainDialog::showAbout() {
    Gdiplus::Bitmap* bitmap = ResourceManager::LoadImage( MAKEINTRESOURCE( IDB_ABOUT ), "PNG" );
    if( NULL != bitmap ) {
        AlphaWindow window( *bitmap );
        window.Show( getHandle() );
    }
}


Void MainDialog::setItemEnabled( Int itemID, Bool enabled )
{	
	if ( enabled )
		::EnableWindow(getItem(itemID), TRUE);
	else
		::EnableWindow(getItem(itemID), FALSE);
}