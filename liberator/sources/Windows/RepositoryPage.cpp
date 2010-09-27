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
#include "RepositoryPage.h"
#include "Environment.h"
#include "ResourceManager.h"

#include <strsafe.h>

using namespace FitsLiberator::Modelling;
using namespace FitsLiberator::Windows;

//-------------------------------------------------------------------------------------------------
// FitsLiberator::Windows::InplaceEditor
//-------------------------------------------------------------------------------------------------

InplaceEditor::InplaceEditor( RepositoryPage* owner ) {
    this->owner = owner;
    this->item = 0;
    this->oldWindowProc = NULL;
}

Void InplaceEditor::Attach( HWND hWnd ) {
    Detach();

    if( ::IsWindow( hWnd ) ) {
        ::SetWindowLong( hWnd, GWL_USERDATA, (LONG)this );
        this->hWnd = hWnd;
        this->oldWindowProc = (WNDPROC)::SetWindowLong( hWnd, GWL_WNDPROC, (LONG)InplaceEditor::WindowProc );
    }    
}

Void InplaceEditor::Detach() {
    if( NULL != oldWindowProc ) {
        ::SetWindowLong( hWnd, GWL_WNDPROC, (LONG)oldWindowProc );
        ::SetWindowLong( hWnd, GWL_USERDATA, 0 );
        oldWindowProc = NULL;
    }
    this->hWnd = NULL;
}

Void InplaceEditor::Activate( Int item, Keyword::Type type, const String& ) {
    this->item = item;

    setVisible( true );
    SetFocus();
}

String InplaceEditor::Serialize() const {
	return super::getText();
}

Bool InplaceEditor::FilterKeys( Int virtualKey, Int flags ) {
    Int newItem = item;
    Bool searchDown = true;
    switch( virtualKey ) {
        case VK_TAB:
            return FilterKeys( (::GetKeyState( VK_SHIFT ) & 0x80000000) ? VK_UP : VK_DOWN, flags );
        case VK_UP:
            if( item > 0 ) {
                newItem = item - 1;
            }
            searchDown = false;
            break;
        case VK_DOWN:
        case VK_RETURN:
            if( item < owner->getItemCount() - 1 )
                newItem = item + 1;
            break;
        case VK_PRIOR:
            newItem = item - owner->getItemsPerPage();
            searchDown = false;
            if( newItem < 0 ) {
                newItem = 0;
            }
            break;
        case VK_NEXT:
            newItem = item + owner->getItemsPerPage();
            if( newItem > owner->getItemCount() - 1 )
                newItem = owner->getItemCount() - 1;
            break;
    }

    if( item != newItem ) {
        owner->BeginEdit( newItem, searchDown );
        return true;
    }
    return false;
}

LRESULT InplaceEditor::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    InplaceEditor* instance = (InplaceEditor*)::GetWindowLong( hWnd, GWL_USERDATA );

    if( instance == NULL )
        return ::DefWindowProc( hWnd, uMsg, wParam, lParam );

    if( hWnd == instance->hWnd ) {
        switch( uMsg ) {
            case WM_NCDESTROY:
                ::CallWindowProc( instance->oldWindowProc, hWnd, uMsg, wParam, lParam );
                instance->Detach();
                delete instance;
                return 0;
            case WM_MOUSEWHEEL:
                return 0;   // No scrolling please
            case WM_GETDLGCODE:
                return DLGC_WANTALLKEYS;
            case WM_DESTROY:
                instance->OnDestroy( hWnd );
                break;
            case WM_KILLFOCUS:
                instance->OnKillFocus( (HWND)wParam );
                // OnKillFocus (eventually) calls Destroy(hWnd) making instance
                // a dangling pointer
                return 0;
            case WM_KEYDOWN:
                if( instance->FilterKeys( wParam, lParam ) )
                return 0;   // We handled the message
        }
    }
    return ::CallWindowProc( instance->oldWindowProc, hWnd, uMsg, wParam, lParam );
}

Void InplaceEditor::OnKillFocus( HWND newFocusedWindow ) {
    if( newFocusedWindow == owner->getHandle() || newFocusedWindow == ::GetParent( hWnd ) ) {
        owner->SaveItem( item, this );
    }
    else {
        owner->CancelItem( item, this );
    }
}

Void InplaceEditor::OnParentCommand( WPARAM wParam, LPARAM lParam ) {
	// The base class does nothing
}

HFONT InplaceEditor::createFont() {
    NONCLIENTMETRICS ncm;
    ncm.cbSize = sizeof(ncm);

    ::SystemParametersInfo( SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, FALSE );

    return ::CreateFontIndirect( &ncm.lfMessageFont );
}

//-------------------------------------------------------------------------------------------------
// FitsLiberator::Windows::InplaceTextbox
//-------------------------------------------------------------------------------------------------

InplaceTextbox::InplaceTextbox( RepositoryPage* owner, HWND parent, const RECT* bounds ) : super( owner ) {
    Attach( ::CreateWindowEx( WS_EX_CLIENTEDGE, WC_EDIT, NULL, WS_CHILD|ES_AUTOHSCROLL, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, parent, NULL, g_hInstance, NULL ) );
    SendMessage( WM_SETFONT, (WPARAM)createFont(), FALSE );

    RECT itemBounds = *bounds;
    ::InflateRect( &itemBounds, 0, 2 );
    ::SetWindowPos( hWnd, HWND_TOP, itemBounds.left, itemBounds.top, itemBounds.right - itemBounds.left, itemBounds.bottom - itemBounds.top, SWP_SHOWWINDOW );
}

Void InplaceTextbox::Activate( Int item, Keyword::Type type, const String& value ) {
    setText( value );
    SendMessage( EM_SETSEL, 0, -1 );

    super::Activate( item, type, value );
}

//-------------------------------------------------------------------------------------------------
// FitsLiberator::Windows::InplaceComboBox
//-------------------------------------------------------------------------------------------------

InplaceComboBox::InplaceComboBox( RepositoryPage* owner, HWND parent, const RECT* bounds ) : super( owner ) {
    // Create and subclass the combobox
    Attach( ::CreateWindow( WC_COMBOBOX, NULL, WS_BORDER|WS_CHILD|WS_VSCROLL|WS_HSCROLL|CBS_DROPDOWN, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, parent, NULL, g_hInstance, NULL ) );
    SendMessage( CB_SETITEMHEIGHT, -1, bounds->bottom - bounds->top - 4 );
    SendMessage( WM_SETFONT, (WPARAM)createFont(), FALSE );

    // Retrieve and subclass the edit window
    editHandle = ::GetWindow( hWnd, GW_CHILD );
    ::SetWindowLong( editHandle, GWL_USERDATA, (LONG)this );
    oldEditProc = (WNDPROC) ::SetWindowLong( editHandle, GWL_WNDPROC, (LONG)&InplaceComboBox::EditProc );

    // NOTE: To fix bug #177, we have to make the window rect of the combobox large enough to hold > 0 items in the dropdown
    // In the below call this is done by multiplying the requested size with 22 ( should make room for 20 items )
    ::SetWindowPos( hWnd, HWND_TOP, bounds->left, bounds->top - 4, bounds->right - bounds->left, (bounds->bottom - bounds->top) * 22, SWP_SHOWWINDOW );
}

Void InplaceComboBox::AddItem( const String& item ) {
    SendMessage( CB_ADDSTRING, 0, (LPARAM)item.c_str() );
}

Void InplaceComboBox::Activate( Int item, Keyword::Type type, const String& value ) {
    setText( editHandle, value );

    super::Activate( item, type, value );
}

Void InplaceComboBox::OnKillFocus( HWND otherWindow ) {
    if( otherWindow != editHandle ) {
        super::OnKillFocus( otherWindow );
    }
}

Void InplaceComboBox::OnDestroy( HWND ) {
    ::SetWindowLong( editHandle, GWL_WNDPROC, (LONG)oldEditProc );
}

LRESULT InplaceComboBox::EditProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    InplaceComboBox* instance = (InplaceComboBox*)::GetWindowLong( hWnd, GWL_USERDATA );

    switch( uMsg ) {
        case WM_MOUSEWHEEL:
            return 0;   // No scrolling please
        case WM_GETDLGCODE:
            return DLGC_WANTALLKEYS;
        case WM_KILLFOCUS:
            if( (HWND)wParam != instance->hWnd ) {
                instance->OnKillFocus( (HWND)wParam );
                return 0;
            }
            break;
        case WM_KEYDOWN:
            if( instance->FilterKeys( wParam, lParam ) )
            return 0;   // We handled the message
    }
    return ::CallWindowProc( instance->oldEditProc, hWnd, uMsg, wParam, lParam );
}

//-------------------------------------------------------------------------------------------------
// FitsLiberator::Windows::InplaceDropdown
//-------------------------------------------------------------------------------------------------

InplaceDropdown::InplaceDropdown( RepositoryPage* owner, HWND parent, const RECT* bounds ) : super( owner ) {
    Attach( ::CreateWindowEx( WS_EX_CLIENTEDGE, WC_COMBOBOX, NULL, WS_CHILD|WS_VSCROLL|WS_BORDER|WS_HSCROLL|CBS_DROPDOWNLIST, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, parent, NULL, g_hInstance, NULL ) );
    SendMessage( CB_SETITEMHEIGHT, -1, bounds->bottom - bounds->top - 4 );
    SendMessage( WM_SETFONT, (WPARAM)createFont(), FALSE );

    // NOTE: To fix bug #177, we have to make the window rect of the combobox large enough to hold > 0 items in the dropdown
    // In the below call this is done by multiplying the requested size with 22 ( should make room for 20 items )
    ::SetWindowPos( hWnd, HWND_TOP, bounds->left, bounds->top - 4, bounds->right - bounds->left, (bounds->bottom - bounds->top) * 22, SWP_SHOWWINDOW );
}

Void InplaceDropdown::AddItem( const String& item ) {
    SendMessage( CB_ADDSTRING, 0, (LPARAM)item.c_str() );
}

Void InplaceDropdown::Activate( Int item, Keyword::Type type, const String& value ) {
    SendMessage( CB_SELECTSTRING, -1, (LPARAM)value.c_str() );

    super::Activate( item, type, value );
}

//-------------------------------------------------------------------------------------------------
// FitsLiberator::Windows::InplaceCategoryButton
//-------------------------------------------------------------------------------------------------

InplaceCategoryButton::InplaceCategoryButton( TaxonomyEditorModel& m, TaxonomyEditorController& c, RepositoryPage* owner, HWND parent, const RECT* bounds )
  : super( owner ), editor( NULL ), model( m ), controller( c ) {
	String text = ResourceManager::LoadString( IDS_TAXONOMYEDITOR );
	
	SIZE sz;
	HDC hDC = ::GetDC( parent );
	::GetTextExtentPoint( hDC, text.c_str(), text.size(), &sz );
	::ReleaseDC( parent, hDC );
	sz.cx += 12;
	sz.cy += 6;

	super::Attach( ::CreateWindow( WC_BUTTON, text.c_str(), WS_CHILD|BS_TEXT|BS_CENTER|BS_VCENTER, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, parent, NULL, g_hInstance, NULL ) );
//	SetWindowFont( hWnd, GetWindowFont( parent ), false );
	::SetWindowPos( hWnd, HWND_TOP, bounds->right - sz.cx, bounds->top - (sz.cy - (bounds->bottom - bounds->top)) / 2 , sz.cx,  sz.cy, SWP_SHOWWINDOW );
}

void InplaceCategoryButton::OnKillFocus( HWND newFocusedWindow ) {
	if( editor == NULL )
		super::OnKillFocus( newFocusedWindow );
}

void InplaceCategoryButton::OnParentCommand( WPARAM wParam, LPARAM lParam ) {
	if( HIWORD(wParam) == BN_CLICKED && ((HWND)lParam) == hWnd ) {
		editor = new TaxonomyEditor( model, controller );
		editor->Modal( hWnd );
		delete editor;
		editor = NULL;
		::SetFocus( ::GetParent( hWnd ) );
	}
}

String InplaceCategoryButton::Serialize() const {
	return model.Serialize();
}

//-------------------------------------------------------------------------------------------------
// FitsLiberator::Windows::InplaceCalendar
//-------------------------------------------------------------------------------------------------

InplaceCalendar::InplaceCalendar( RepositoryPage* owner, HWND parent, const RECT* bounds )
  : super(owner) {
	Attach( ::CreateWindowEx( WS_EX_CLIENTEDGE, DATETIMEPICK_CLASS, NULL, WS_CHILD, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, parent, NULL, g_hInstance, NULL ) );
    //SendMessage( WM_SETFONT, (WPARAM)createFont(), FALSE );

    ::SetWindowPos( hWnd, HWND_TOP, bounds->left, bounds->top, bounds->right - bounds->left, bounds->bottom - bounds->top, SWP_SHOWWINDOW );
}

Void InplaceCalendar::Activate( Int item, Keyword::Type type, const String& value ) {
	super::Activate( item, type, value );
	
	if( type == Keyword::TypeDate )
		DateTime_SetFormat( hWnd, "yyyy-MM-dd" );
	else if( type == Keyword::TypeDateTime )
		DateTime_SetFormat( hWnd, "yyyy-MM-dd-HHmm" );

	// Parse the value
	if( value.length() >= 10 ) {
		SYSTEMTIME time = {0};
		
		// A date is guaranteed to be yyyy-MM-dd
		time.wYear = TextUtils::stringToInt( value.substr(0, 4) );
		time.wMonth = TextUtils::stringToInt( value.substr( 5, 2 ) );
		time.wDay = TextUtils::stringToInt( value.substr( 8, 2) );

		// A date/time is guaranteed to be yyyy-MM-dd-HHmm
		if( type == Keyword::TypeDateTime && value.length() >= 15 ) {
			time.wHour = TextUtils::stringToInt( value.substr( 11, 2) );
			time.wMinute = TextUtils::stringToInt( value.substr( 14, 2) );
		}
		
		DateTime_SetSystemtime( hWnd, GDT_VALID, &time );
	}
}

//-------------------------------------------------------------------------------------------------
// FitsLiberator::Windows::RepositoryPage
//-------------------------------------------------------------------------------------------------

RepositoryPage::RepositoryPage( DispatchView& view ) 
  : PropertyPage( IDD_REPOSITORY ), dispatchView( view ) {
    model      = dispatchView.frameWork.repositoryModel;
    controller = dispatchView.frameWork.repositoryController;
	taxonomyEditorController = dispatchView.frameWork.taxonomyEditorController;
	taxonomyEditorModel = dispatchView.frameWork.taxonomyEditorModel;

    currentItem = -1;
	currentEditor = NULL;
}

RepositoryPage::~RepositoryPage() { }

Bool RepositoryPage::OnInit( HWND hWnd ) {
    if( !super::OnInit( hWnd ) )
        return false;

    dispatchView.OnInit( this, IDD_REPOSITORY );

    //
    // Set up the listview
	listviewHandle = getItem( IDC_REPOSITORY );
    if( NULL == listviewHandle )
        ::OutputDebugString( "Failed to get the listview window handle" );

    ListView_SetExtendedListViewStyle( listviewHandle, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP );

    if( !initColumns() )
        ::OutputDebugString( "Failed to init columns" );
    if( !initGroups() )
        ::OutputDebugString( "Failed to init groups" );
    if( !initItems() )
        ::OutputDebugString( "Failed to init items" );

    ::SetWindowLong( listviewHandle, GWL_USERDATA, (LONG)this );
    oldListProc = (WNDPROC)::SetWindowLong( listviewHandle, GWL_WNDPROC, (LONG)RepositoryPage::ListProc );

    return true;
}

Void RepositoryPage::OnDestroy( HWND ) {
    ::SetWindowLong( listviewHandle, GWL_WNDPROC, (LPARAM)oldListProc );
}

LRESULT RepositoryPage::OnNotify( Int idCtrl, LPNMHDR pnmh ) {
    switch (pnmh->code) {
        case LVN_GETDISPINFO:
            OnGetDisplayInfo( (NMLVDISPINFO*)pnmh );
            return false;
        case LVN_GETINFOTIP:
            OnGetInfotip( (NMLVGETINFOTIP*)pnmh );
            return false;
        case LVN_KEYDOWN:
            OnKeyDown( (NMLVKEYDOWN*)pnmh );
            return super::OnNotify( idCtrl, pnmh );
        default:
            return super::OnNotify( idCtrl, pnmh );
    }
}

Void RepositoryPage::OnGetDisplayInfo( NMLVDISPINFO* e ) {
    const Keyword* keyword = keywordFromItem( e->item.iItem );
    if( keyword != NULL ) {
        switch (e->item.iSubItem) {
			case 0:	// Id
				e->item.pszText = (LPSTR)keyword->id.c_str();
				break;
            case 1:	// Name
                e->item.pszText = (LPSTR)keyword->name.c_str();
                break;
            case 2:	// Description
                e->item.pszText = (LPSTR)keyword->value.c_str();
                break;
            default:
                break;
        }
    }
}

Void RepositoryPage::OnGetInfotip( NMLVGETINFOTIP* e ) {
    const Keyword* keyword = keywordFromItem( e->iItem );
    if( keyword != NULL )  {
        if( e->dwFlags == 0 ) {
            StringCchCat( e->pszText, e->cchTextMax, keyword->description.c_str() );
        }
        else {
            StringCchCopy( e->pszText, e->cchTextMax, keyword->description.c_str() );
        }
    }
}

Void RepositoryPage::Update() {
    // Simply force the listview to repaint (we hold the data anyway)
    ::InvalidateRect( listviewHandle, NULL, TRUE );
    //::UpdateWindow( listviewHandle );
}

Void RepositoryPage::OnKeyDown( NMLVKEYDOWN* e ) {
//    if( ::MapVirtualKey( e->wVKey, 2 ) )
        //EditItem( ListView_GetNextItem( listviewHandle, -1, LVNI_SELECTED ), (Char)e->wVKey );
}

LRESULT RepositoryPage::ListProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    RepositoryPage* owner = (RepositoryPage*)::GetWindowLong( hWnd, GWL_USERDATA );

    LRESULT result = 0;
    switch( uMsg ) {
		case WM_COMMAND:
			if( owner->currentEditor != NULL )
				owner->currentEditor->OnParentCommand( wParam, lParam );
			break;
        case WM_HSCROLL:
        case WM_VSCROLL:
            if( ::GetFocus() != hWnd ) ::SetFocus( hWnd );
            result = ::CallWindowProc( owner->oldListProc, hWnd, uMsg, wParam, lParam );
            break;
        case WM_LBUTTONDOWN:
            if( owner->OnListviewMouseDown( wParam, LOWORD( lParam ), HIWORD( lParam ) ) )
                return 0;
            // May fall through
        default:
            result = ::CallWindowProc( owner->oldListProc, hWnd, uMsg, wParam, lParam );
    }

    return result;
}

Bool RepositoryPage::OnListviewMouseDown( Int button, Int x, Int y ) {
    if( button & MK_LBUTTON ) {
        LVHITTESTINFO hitTest;
        hitTest.pt.x = x;
        hitTest.pt.y = y;
        Int index = ListView_SubItemHitTest( listviewHandle, &hitTest );
        if( index != -1 && hitTest.iSubItem == 2 ) {
            EditItem( hitTest.iItem, NULL );
            return true;
        }
    }
    return false;
}

Int RepositoryPage::keywordIndexFromItem( Int index ) const {
    LVITEM item = {0};
    item.iItem = index;
    item.mask = LVIF_PARAM;

    if( ListView_GetItem( listviewHandle, &item ) )
        return item.lParam;
    return NULL;
}

const Keyword* RepositoryPage::keywordFromItem( Int index ) const {
    return model->getKeyword( keywordIndexFromItem( index ) );
}

Bool RepositoryPage::initColumns() {
    LVCOLUMN column;
    Int      columnWidth;
    RECT     listviewBounds;

	SIZE sz;
	HDC hdc = ::GetDC( listviewHandle );
	::GetTextExtentPoint32( hdc, "XXX", 3, &sz );
	::ReleaseDC( listviewHandle, hdc );

    // Listview information
    ::GetClientRect( listviewHandle, &listviewBounds );
	columnWidth = (listviewBounds.right - listviewBounds.left - sz.cx);
    columnWidth -= ::GetSystemMetrics( SM_CXVSCROLL );
    columnWidth /= 3;

    // Common properties
    column.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    
	// Id column
	column.pszText = (LPSTR)ResourceManager::LoadString( IDS_ID ).c_str();
	column.iSubItem = 0;
	column.cx = sz.cx;
    column.fmt = LVCFMT_RIGHT;
	if( -1 == ListView_InsertColumn( listviewHandle, 0, &column ) )
		return false;

    // Name column
    column.pszText = (LPSTR)ResourceManager::LoadString( IDS_NAME ).c_str();
    column.iSubItem = 1;
    column.cx = columnWidth;
    column.fmt = LVCFMT_LEFT;
    if( -1 == ListView_InsertColumn( listviewHandle, 1, &column ) )
        return false;

    // Value column
    column.pszText = (LPSTR)ResourceManager::LoadString( IDS_VALUE ).c_str();
    column.iSubItem = 2;
    column.cx = 2 * columnWidth;
    column.fmt = LVCFMT_LEFT;
    if( -1 == ListView_InsertColumn( listviewHandle, 2, &column ) )
        return false;

    return true;
}


Bool RepositoryPage::initGroups() {
    LVGROUP group;
    WCHAR text[50];

    ListView_EnableGroupView( listviewHandle, TRUE );

    group.cbSize = sizeof( LVGROUP );
    group.mask = LVGF_GROUPID | LVGF_HEADER;
    group.state = LVGS_NORMAL;
    group.stateMask = 0;
    group.uAlign = LVGA_HEADER_LEFT;
    group.pszHeader = text;

    Int count = model->getCategoryCount();
    for( Int index = 0; index < count; index++ ) {
        // We need to convert from ANSI to UTF16
        String name = model->getCategory( count - index - 1);
        ::MultiByteToWideChar( CP_ACP, 0, name.c_str(), name.size() + 1, text, sizeof(text)/sizeof(text[0]) );

        group.iGroupId = index;
        group.cchHeader = 20;
        if( -1 == ListView_InsertGroup( listviewHandle, 0, &group ) )
            return false;
    }

    return true;
}

Bool RepositoryPage::initItems() {
    LVITEM item;

    item.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE | LVIF_GROUPID; 
    item.state = 0; 
    item.stateMask = 0; 

    Int categoryCount = model->getCategoryCount();
    Int count = model->getCount();
    Int itemIndex = 0;
    for( Int keywordIndex = 0; keywordIndex < count; keywordIndex++ ) {
        const Keyword* keyword = model->getKeyword( keywordIndex );
        
        if( !(keyword->flags & Keyword::FlagsHidden) ) {
            item.iItem = itemIndex;
            item.iGroupId = categoryCount - keyword->category - 1;
            item.iSubItem = 0;
	        item.lParam =  keywordIndex;
	        item.pszText = LPSTR_TEXTCALLBACK;
            
            if( -1 == ListView_InsertItem( listviewHandle, &item ) )
                return false;

            itemIndex++;
        }
    }
    return true;
}

Void RepositoryPage::EditItem( Int itemIndex, String* content ) {
    if( itemIndex < 0 || itemIndex >= getItemCount() )
        return;

    const Keyword* keyword = keywordFromItem( itemIndex );
    if( keyword != NULL && !(keyword->flags & Keyword::FlagsReadonly) ) {    // Don't allow readonly values to be edited
		String value = (content == NULL) ? keyword->value : *content;
        
        // Hide the old editor, this effectively destroys the editor and sets currentItem = -1
        SetFocus();
        if( -1 == currentItem ) {

            ListView_EnsureVisible( listviewHandle, itemIndex, FALSE );
            ListView_SetItemState( listviewHandle, itemIndex, LVIS_SELECTED, LVIS_SELECTED );

            RECT itemBounds;
            RECT listBounds;

            //
            // Figure out where the editor should be
            ListView_GetItemRect( listviewHandle, itemIndex, &itemBounds, LVIR_BOUNDS );
            ::GetClientRect( listviewHandle, &listBounds );
            itemBounds.left += ListView_GetColumnWidth( listviewHandle, 0 );
			itemBounds.left += ListView_GetColumnWidth( listviewHandle, 1 );

            //
            // Show the editor
			if( keyword->name == "Subject.Category" ) {
				currentEditor = new InplaceCategoryButton( *this->taxonomyEditorModel, *this->taxonomyEditorController, this, listviewHandle, &itemBounds );
			} else if( keyword->type == Keyword::TypeStringCv ) {
                InplaceDropdown* editor = new InplaceDropdown( this, listviewHandle, &itemBounds );
                editor->AddItem(" ");
                for( Vector<String>::const_iterator i = keyword->completions.begin(); i != keyword->completions.end(); i++ )
                    editor->AddItem( *i );
				currentEditor = editor;
//			} else if( keyword->type == Keyword::TypeDate || keyword->type == Keyword::TypeDateTime ) {
//				currentEditor = new InplaceCalendar( this, listviewHandle, &itemBounds );
            } else if( keyword->completions.size() > 0 ) {
                // The keyword contains completions, use a combobox
                InplaceComboBox* editor = new InplaceComboBox( this, listviewHandle, &itemBounds );
                for( Vector<String>::const_iterator i = keyword->completions.begin(); i != keyword->completions.end(); i++ )
                    editor->AddItem( *i );
				currentEditor = editor;
            } else {
                // The keyword is a single string
                currentEditor = new InplaceTextbox( this, listviewHandle, &itemBounds );
            }
			currentEditor->Activate( itemIndex, keyword->type, value );
            currentItem = itemIndex;
        }
    }
}

Void RepositoryPage::SaveItem( Int itemIndex, InplaceEditor* editor ) {
    //
    // Save the information from the old field
    if( itemIndex >= 0 && itemIndex < getItemCount() ) {
		String text = editor->Serialize();
        ValidationError error = controller->parseValue( keywordIndexFromItem( itemIndex ), text );
        if( NoError == error ) {

            //
            // Repaint the listview
            RECT bounds;
            ::GetWindowRect( editor->getHandle(), &bounds );
            ::ScreenToClient( listviewHandle, &(((POINT*)(&bounds))[0]) );
            ::ScreenToClient( listviewHandle, &(((POINT*)(&bounds))[1]) );

            ::InvalidateRect( listviewHandle, &bounds, FALSE );

            CancelItem( itemIndex, editor );
        }
        else {
			String message = ResourceManager::LoadString( IDS_INVALID_HEADER + error );
			if( error == InvalidElementCount )
                message.append( TextUtils::intToString( keywordFromItem( itemIndex )->elements ) );

            ::MessageBox( hWnd, 
				message.c_str(),
                ResourceManager::LoadString( IDS_INVALID_HEADER ).c_str(),
                MB_ICONERROR | MB_OK
            );

            // Redo the edit
            Int index = currentItem;
            CancelItem( itemIndex, editor );
            EditItem( index, &text );
        }
    }
}

Void RepositoryPage::CancelItem( Int, InplaceEditor* editor ) {
    editor->Destroy();
    currentItem = -1;
	currentEditor = NULL;
}

Void RepositoryPage::BeginEdit( Int itemIndex, Bool searchDown ) {
    Int  offset = searchDown ? 1 : -1;
    UInt count = getItemCount();

    if( itemIndex < 0 || itemIndex >= count )
        return;

    const Keyword* keyword = NULL;
    do {
        keyword = keywordFromItem( itemIndex );
        itemIndex += offset;
    }
    while( keyword != NULL &&(keyword->flags & Keyword::FlagsReadonly) && itemIndex < count && itemIndex >= 0 );
    
    itemIndex -= offset;    // We have searched one item too far
    if( itemIndex >= 0 && itemIndex < count ) {
        EditItem( itemIndex, 0 );
    }
}

UInt RepositoryPage::getItemCount() const {
    return ListView_GetItemCount( listviewHandle );
}

UInt RepositoryPage::getItemsPerPage() const {
    return ListView_GetCountPerPage( listviewHandle );
}