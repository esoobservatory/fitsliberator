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

#include "MacTaxonomyEditorView.h"
#include "Environment.h"

using namespace FitsLiberator;
using namespace FitsLiberator::Modelling;
using namespace FitsLiberator::Mac;


//-------------------------------------------------------------------------------
// DataBrowser callbacks
//-------------------------------------------------------------------------------

/**
 * Global variabel which is initialized in MacTaxonomyEditorView constructor. Used in callbacks.
 */
MacTaxonomyEditorView* gTaxonomyDispatcher = NULL;

/**
 * Notification callback for data browser in category editor.
 * 
 * Dispatches the callback to MacTaxonomyEditorView::itemNotificationCallback.
 */
pascal Void FitsLiberator::Mac::taxitemNotificationCallback( ControlRef browser, DataBrowserItemID itemID, DataBrowserItemNotification message ) {
	if( gTaxonomyDispatcher != NULL ) {
		gTaxonomyDispatcher->itemNotificationCallback( browser, itemID, message );
	}
}

/**
 * Data callback for data browser in category editor.
 * 
 * Dispatches the callback to MacTaxonomyEditorView::itemDataCallback.
 */
pascal OSStatus FitsLiberator::Mac::taxitemDataCallback( ControlRef browser, DataBrowserItemID itemID, DataBrowserPropertyID property, DataBrowserItemDataRef itemData, Boolean changeValue ) {
	if( gTaxonomyDispatcher != NULL ) {
		return gTaxonomyDispatcher->itemDataCallback( browser, itemID, property, itemData, changeValue );
	} else {
		return errDataBrowserPropertyNotSupported;
	}
}

//-------------------------------------------------------------------------------
// Editor Dialog implementation
//-------------------------------------------------------------------------------

/**
 * Constructor for editor dialog. 
 */
TaxonomyEditorDialog::TaxonomyEditorDialog( BundleFactory* factory ) {		
	create( factory, CFSTR( kFITSNibFile ), CFSTR( kFITSNibWindowCategoryEditor ) );	
	setContinueEventLoop( true );
}

//-------------------------------------------------------------------------------
// ScaleInitializer implementation
//-------------------------------------------------------------------------------

/**
 * Use in conjuction with TaxonomyModel::EnumerateScales.
 *
 * Add category scales to popup control.
 */
struct MacTaxonomyEditorView::ScaleInitializer {
	BaseControl& popup;
	MenuRef 	 menu;
	Int 		 index;

	/**
	 * Constructor.
	 * @param p Popup control that should have items added.
	 */
	ScaleInitializer( BaseControl& p  )
	  : popup( p ), index( 0 ) {	
		// Obtain MenuRef for popup.
		popup.getData( kControlMenuPart, kControlPopupButtonMenuRefTag, &menu, sizeof( menu ), NULL );
	
		if( menu == NULL ) {
			throw Exception( "Couldn't obtain reference to category selector popup menu." );
		}
		
		// IB creates control with one menu item so delete the item.
		::DeleteMenuItem( menu, 1 );
	}

	/**
	 * Method applied to all scales.
	 */
	Void operator()( const TaxonomyEditorModel::Scale* scale ) {
		index++;

		CFStringRef menuText = ::CFStringCreateWithCString( NULL, scale->Name().c_str(),  kCFStringEncodingMacRoman );
        
		if( noErr != ::AppendMenuItemTextWithCFString( menu, menuText, (1 << 14), kFITSUICommandSelectCategory, NULL ) ) {
			throw FitsLiberator::Exception( "Couldn't add menu items to category editor popup menu." );
		}
		
		::CFRelease( menuText );		
	}
	
	/**
	 * Call this function after calls to TaxonomyModel::EnumerateScales
	 */
	Void finalize() {
		popup.setMaximumValue( index );
		index = 0;
	}
};


//-------------------------------------------------------------------------------
// CategoryInitializer implementation
//-------------------------------------------------------------------------------

/**
 * Add taxonomy treee to data browser. When finish, all categories will be added
 * and tree fully expanded.
 */
struct MacTaxonomyEditorView::CategoryInitializer {
private:
	DataBrowserControl& control;	///< Data browser control.
	DataBrowserItemID root; 		///< Root element container.
	
public:
	/**
	 *
	 */
	CategoryInitializer( DataBrowserControl& ctrl, DataBrowserItemID r ) 
	  : root(r), control( ctrl ) { }

	/**
	 * Applied to all categories in taxonomy.
	 *
	 * Clones this initializer, changes root and calls any children 
	 * in the tree. This means that containers will be opened, since
	 * adding children to a container will open the container. 
	 */
	Void operator()( const TaxonomyEditorModel::Category* category ) {
		CategoryInitializer initializer = *this; // Clones
		
		// Add one item at a time
		control.addItems( root, 1, (DataBrowserItemID *) &category, kFITSDataCategory );
	
		initializer.root = (DataBrowserItemID) category;
		category->EnumerateCategories( initializer );
	}
};

//-------------------------------------------------------------------------------
// MacTaxonomyEditorView implementation
//-------------------------------------------------------------------------------

/**
 * Constructor
 */
MacTaxonomyEditorView::MacTaxonomyEditorView( TaxonomyEditorModel& m, TaxonomyEditorController& c ) 
  : TaxonomyEditorView(m,c), 
  	userToggledContainer( false ) {
	
	// Create editor dialog.
	editor = new TaxonomyEditorDialog( Environment::getBundleFactory() );
	
	// Create simple controls.
	static const ::EventTypeSpec eventTypes[] = {
			{ kEventClassCommand, kEventCommandProcess }
	};

	doneButton = new BaseControl( editor, kFITSUIButtonDone );
	scalePopup = new BaseControl( editor, kFITSUICategorySelector );
	doneButton->installEventHandler( eventTypes, GetEventTypeCount( eventTypes ), this );
	scalePopup->installEventHandler( eventTypes, GetEventTypeCount( eventTypes ), this );
	
	// Initialize the scales list
	ScaleInitializer scaleInitializer( *scalePopup );
	model.EnumerateScales( scaleInitializer );	
	scaleInitializer.finalize();
	
	//------------------
  	// Setup databrowser
  	//------------------
  	// Key events
	static const ::EventTypeSpec dbEventTypes[] = {
			{ kEventClassTextInput, kEventTextInputUnicodeForKeyEvent }
	};
  	
	
	// Initliaze global variabel for callbacks.
	gTaxonomyDispatcher = this;
	
	// Obtain databrowser
	control = new DataBrowserControl( editor, kFITSUICategoryBrowser );
	control->installEventHandler( dbEventTypes, GetEventTypeCount(dbEventTypes), this);
	
	// Setup callbacks
	dbCallbacks.version = kDataBrowserLatestCallbacks;
	control->initCallbacks( &dbCallbacks );
	dbCallbacks.u.v1.itemDataCallback 			= ::NewDataBrowserItemDataUPP( FitsLiberator::Mac::taxitemDataCallback );
	dbCallbacks.u.v1.itemNotificationCallback 	= ::NewDataBrowserItemNotificationUPP( FitsLiberator::Mac::taxitemNotificationCallback );
	control->setCallbacks( &dbCallbacks );
	
	// Set properties for databrowser.
	DataBrowserPropertyFlags flags = 0;
	control->setListViewDisclosureColumn( kFITSDataCategory, true );
	control->getPropertyFlags( kFITSDataCategoryCheck, &flags );
	control->setPropertyFlags( kFITSDataCategoryCheck, flags | kDataBrowserItemIsEditableProperty );
	control->changeAttributes( kDataBrowserAttributeListViewAlternatingRowColors, kDataBrowserAttributeNone );		
	
	// Hack to set small system font size for control.
	ControlFontStyleRec styl;
	styl.flags = kControlUseSizeMask;
	styl.size = 11;
	::SetControlFontStyle(control->getControl(),&styl);
	
	//------------------
  	// Add items to data browser
  	//------------------
  	initialPopulating = true;
  	
  	CategoryInitializer categoryInitializer( *control, kDataBrowserNoItem );
	model.EnumerateCategories( categoryInitializer );
	control->autoSizeListViewColumns();
	
	initialPopulating = false;
}

/**
 * Destructor
 */
MacTaxonomyEditorView::~MacTaxonomyEditorView() {
	gTaxonomyDispatcher = NULL;
	
	if( editor != NULL ) {
		delete editor;	
	}
	
	if( doneButton != NULL ){
		delete doneButton;
	}
	
	if( scalePopup != NULL ){
		delete scalePopup;
	}
	
	if( control != NULL ) {
		delete control;	
	}
}

/**
 * Activates the editor (show the window and run event loop).
 */
Void MacTaxonomyEditorView::activateEditor()  {
	UpdateScale( model.ScaleIndex() );
	editor->show();
	scalePopup->keyboardFocus( kControlLabelPart );
	editor->select();
	editor->runModalEventLoop();
	editor->hide();
}

/**
 * Command event handling for done button and popup control.
 */
OSStatus MacTaxonomyEditorView::onCommand( HICommand *command ) {
	OSStatus result = eventNotHandledErr;

	switch( command->commandID ) {
		case kFITSUICommandCategoryEditorDone:
			// Finish so quit event loop and return 
			// control flow to activateEditor().
			editor->quitModalEventLoop();
			result = noErr;
			break;
		case kFITSUICommandSelectCategory:
			OnScaleSelectionChanged( command->menu.menuItemIndex - 1 );
			result = eventNotHandledErr;
			break;
	}
	
	return result;
}

/**
 * Handle space bar presses in databrowser.
 *
 * See e.g. http://lists.apple.com/archives/carbon-development/2003/Jun/msg01283.html
 */
OSStatus MacTaxonomyEditorView::onUnicodeForKeyEvent( EventRef event ) {
	OSStatus			result = eventNotHandledErr;

	EventRef			rawKeyboardEvent = NULL;
	Char 				c;
	
	::GetEventParameter( event, kEventParamTextInputSendKeyboardEvent, typeEventRef, NULL, sizeof( EventRef ), NULL, &rawKeyboardEvent );
	::GetEventParameter( rawKeyboardEvent, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof (Char), NULL, &c );
	
	switch( c ) {
		case kSpaceCharCode:
			Handle items = NewHandle(0);;
			
			if( items != NULL ) {
				control->getItems( kDataBrowserNoItem, true, kDataBrowserItemIsSelected, items );
				
				if( ( GetHandleSize( items ) / sizeof(DataBrowserItemID)) > 0 ) {
					HLock( items );	
					
					DataBrowserItemID *individualItem = (DataBrowserItemID*)( *items );
					OnCategoryClicked( (const TaxonomyEditorModel::Category*) *individualItem );
				}
				
				::DisposeHandle( items );
			}
			break;
				
	}	
	
	return result;
}

/**
 * Update pop up menu with new value.
 */
Void MacTaxonomyEditorView::UpdateScale( Int newIndex )  {
	scalePopup->set16BitValue( newIndex + 1 );
	scalePopup->drawControl();
}

/**
 * Update checkmark for a given category.
 */
Void MacTaxonomyEditorView::UpdateCategory( const TaxonomyEditorModel::Category* category ) {
	::UpdateDataBrowserItems( control->getControl(), (DataBrowserItemID) category->Parent(), 1, (const DataBrowserItemID *) &category, kFITSDataCategory, kFITSDataCategoryCheck );
}


/**
 * Data browser data callback. Provides data and properties for data browser.
 */
OSStatus MacTaxonomyEditorView::itemDataCallback( ControlRef browser, DataBrowserItemID itemID, DataBrowserPropertyID property, DataBrowserItemDataRef itemData, Boolean changeValue ) {
	OSStatus status = noErr;
	
	if( !changeValue && (itemID != 0) ) {
		const TaxonomyEditorModel::Category* category = (const TaxonomyEditorModel::Category*) itemID;
		CFStringRef text = Environment::StringToCFString( category->Name().c_str() );
		ThemeButtonValue state = (category->Checked()) ? kThemeButtonOn : kThemeButtonOff;
		switch( property ) {
			// Provide data for the checkbox column
			case kFITSDataCategoryCheck:
				status = control->setItemDataButtonValue( itemData, state );
				break;
			
			// Provide data for the category column			
			case kFITSDataCategory:
				status = control->setItemDataText( itemData, text );
				::CFRelease(text);
				break;
			
			// Determine if item is a container
			case kDataBrowserItemIsContainerProperty:
				status = control->setItemDataBooleanValue( itemData, (category->CategoryCount() > 0) );
				break;
			
			// All item are selectable
			case kDataBrowserItemIsSelectableProperty:
				status = control->setItemDataBooleanValue( itemData, category->Editable() );
				break;
			
			// However, e.g. 2 [Feature], must not be editable so we make it inactive.
			// That is not editable fields are inactive.
			case kDataBrowserItemIsActiveProperty: 
				status = control->setItemDataBooleanValue( itemData, category->Editable() );
				break;
			
			// Determine if item is editable. The kDataBrowserItemIsEditableProperty is sent 
			// both when user presses checkmarks and when containers are opening/closing. 
			// We only want to respond to checkmark clicks. As a result, userToggledContainer 
			// flag is set in itemNotificationCallback when container is opened.
			case kDataBrowserItemIsEditableProperty:
				status = control->setItemDataBooleanValue( itemData, category->Editable() );
				if( !userToggledContainer ) {
					OnCategoryClicked( category );
				} else {
					userToggledContainer = false;
				}
				break;
				
			default:
				status = errDataBrowserPropertyNotSupported;
				break;
		}
	}
	
	return status;
}

/**
 * Data browser notification callback.
 */
Void MacTaxonomyEditorView::itemNotificationCallback( ControlRef browser, DataBrowserItemID itemID, DataBrowserItemNotification message ) {
	switch( message ) {
		
		// Open a container with a CategoryInitializer. The initializer also opens all sub-trees.
		case kDataBrowserContainerOpened: 
			if( (itemID != 0) && !initialPopulating) {
				const TaxonomyEditorModel::Category* category = (const TaxonomyEditorModel::Category*) itemID;
			
				CategoryInitializer categoryInitializer( *control, itemID );
				category->EnumerateCategories( categoryInitializer );
			}
			break;
		
		// kDataBrowserItemIsEditableProperty is also called when toggling a container 
		// besides when checkbox is hit. As a result we set a flag when containers opens
		// or closes.
		case kDataBrowserUserToggledContainer: 
			userToggledContainer = true; 
			break;
	}
}