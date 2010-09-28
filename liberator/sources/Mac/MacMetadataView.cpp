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
 
#include "MacMetaDataView.h"
#include "MachOFrameworkSupport.h"
#include "Appearance.h"
#include "Resources.h"

#define CATEGORYIDENTIFIER "Subject.Category"
 
using namespace FitsLiberator::Mac;
using namespace FitsLiberator::Modelling;
//using namespace FitsLiberator::Mac::MachO;

MacMetadataView* gMetadataDispatcher = NULL;

/**
 *
 *
 */
pascal Void	FitsLiberator::Mac::itemHelpContentCallback( ControlRef browser, DataBrowserItemID item, DataBrowserPropertyID property, HMContentRequest inRequest, HMContentProvidedType *outContentProvided, HMHelpContentPtr ioHelpContent ) {
	if( gMetadataDispatcher != NULL ) {
		gMetadataDispatcher->itemHelpContentCallback( browser, item, property, inRequest, outContentProvided, ioHelpContent );
	}
}

/**
 *
 *
 */
pascal Void FitsLiberator::Mac::itemNotificationCallback( ControlRef browser, DataBrowserItemID itemID, DataBrowserItemNotification message ) {
	if( gMetadataDispatcher != NULL ) {
		gMetadataDispatcher->itemNotificationCallback( browser, itemID, message );
	}
}

/**
 *
 *
 */
pascal OSStatus FitsLiberator::Mac::itemDataCallback( ControlRef browser, DataBrowserItemID itemID, DataBrowserPropertyID property, DataBrowserItemDataRef itemData, Boolean changeValue ) {
	if( gMetadataDispatcher != NULL ) {
		return gMetadataDispatcher->itemDataCallback( browser, itemID, property, itemData, changeValue );
	} else {
		return errDataBrowserPropertyNotSupported;
	}
}


pascal Void FitsLiberator::Mac::drawItemCallback( ControlRef browser, DataBrowserItemID item,  DataBrowserPropertyID property, DataBrowserItemState itemState, const Rect *theRect, SInt16 gdDepth, Boolean colorDevice ) {
	if( gMetadataDispatcher != NULL ) {
		return gMetadataDispatcher->drawItemCallback( browser, item,  property, itemState, theRect, gdDepth, colorDevice );
	}
}

/**
 *
 *
 */
MacMetadataView::MacMetadataView( FitsDialog& dlog, FitsLiberator::Modelling::TaxonomyEditorModel& tem, FitsLiberator::Modelling::TaxonomyEditorController& tec, FitsLiberator::Modelling::RepositoryModel& rm, FitsLiberator::Modelling::RepositoryController& rc, FitsLiberator::Modelling::PlaneModel& pm, FitsLiberator::Modelling::StretchModel& sm, FitsLiberator::Modelling::HistogramModel& hm ) 
: FitsLiberator::Modelling::RepositoryView( rm, rc, pm, sm, hm ),
  control( NULL ),
  dialog( dlog ),
  comboBox( NULL ),
  editTextBox( NULL ),
  popupBox( NULL ),
  categoryButton(NULL),
  selectedItemID( 0 ),
  activeControl( NULL ),
  doingReveal( false ),
  keepText( false ),
  taxonomyModel( tem ),
  taxonomyController( tec ),
  doRevealID( 0 )
  {	
  	// Event types
  	static const ::EventTypeSpec inputEventTypes[] = {
			{ kEventClassTextInput, kEventTextInputUnicodeForKeyEvent },
			{ kEventClassControl, kEventControlSetFocusPart }
	};
	
	static const ::EventTypeSpec popupEventTypes[] = {
			{ kEventClassTextInput, kEventTextInputUnicodeForKeyEvent },
			{ kEventClassControl, kEventControlSetFocusPart },
			{ kEventClassCommand, kEventCommandProcess }
	};
  
  	// Find View ID of DataBrowser.
  	const HIViewID databrowserID = { kFITSApplicationSignature, kFITSUIDataBrowser };
  	HIViewFindByID( HIViewGetRoot( dlog.getWindow() ), databrowserID, &dataBrowserView );
  	
	comboBox = new TextControl( &dlog, kFITSUIComboBoxValueEdit );
	comboBox->installEventHandler(inputEventTypes, GetEventTypeCount( inputEventTypes ), this );
	HIViewAddSubview( dataBrowserView, comboBox->getControl() );
	HIViewSetVisible( comboBox->getControl(), false );
	
	editTextBox = new TextControl( &dlog, kFITSUIEditTextValueEdit );
	editTextBox->installEventHandler( inputEventTypes, GetEventTypeCount( inputEventTypes ), this );
	HIViewAddSubview( dataBrowserView, editTextBox->getControl() );
	
	popupBox = new TextControl( &dlog, kFITSUIPopupButtonValueEdit );
	popupBox->installEventHandler( popupEventTypes, GetEventTypeCount( popupEventTypes ), this );
	HIViewAddSubview( dataBrowserView, popupBox->getControl() );
	
	categoryButton = new TextControl( &dlog, kFITSUIButtonCategory );
	categoryButton->installEventHandler( popupEventTypes, GetEventTypeCount( popupEventTypes ), this );
	HIViewAddSubview( dataBrowserView, categoryButton->getControl() );
  	
  	//------------------
  	// Setup databrowser
  	//------------------
	// Set global variabel
	gMetadataDispatcher = this;
	
	categoryCount 	= model.getCategoryCount();
	keywordCount 	= model.getCount();
	
	// Obtain databrowser	
	control = new DataBrowserControl( &dlog, kFITSUIDataBrowser );
	
	dbCallbacks.version = kDataBrowserLatestCallbacks;
	control->initCallbacks( &dbCallbacks );
	
	dbCallbacks.u.v1.itemDataCallback 			= ::NewDataBrowserItemDataUPP( FitsLiberator::Mac::itemDataCallback );
	dbCallbacks.u.v1.itemNotificationCallback 	= ::NewDataBrowserItemNotificationUPP( FitsLiberator::Mac::itemNotificationCallback );
	dbCallbacks.u.v1.itemHelpContentCallback 	= ::NewDataBrowserItemHelpContentUPP( FitsLiberator::Mac::itemHelpContentCallback );
	
	control->setCallbacks( &dbCallbacks );
	control->setListViewDisclosureColumn( kFITSDataKeyword, true );	
	
	// Set a better row height since we want to place edit text fields and combo boxes in it.
	SInt32 height = 0;
	::GetThemeMetric( kThemeMetricDisclosureButtonHeight, &height );
	::SetDataBrowserTableViewRowHeight( control->getControl(), height );
	
	control->autoSizeListViewColumns();
	control->addItems( kDataBrowserNoItem, model.getCategoryCount(), NULL, kDataBrowserItemNoProperty );
		
	// Open all containers
	for( Int i = 0; i < model.getCategoryCount(); i++ ) {
		control->openContainer( categoryIDToItemID(i) );	
	}
	
	// Set editable property for value column
	DataBrowserPropertyFlags flags = 0;

	control->getPropertyFlags( kFITSDataDescription, &flags );
	control->setPropertyFlags( kFITSDataDescription, flags | kDataBrowserItemIsEditableProperty );

	// Set up custom callbacks
	dbCustomCallbacks.version = kDataBrowserLatestCustomCallbacks;
	control->initCustomCallbacks( &dbCustomCallbacks );
	
	dbCustomCallbacks.u.v1.drawItemCallback = ::NewDataBrowserDrawItemUPP( FitsLiberator::Mac::drawItemCallback );

	control->setCustomCallbacks( &dbCustomCallbacks );
	
	control->changeAttributes( kDataBrowserAttributeListViewAlternatingRowColors, kDataBrowserAttributeNone );
	//control->changeAttributes( kDataBrowserAttributeListViewDrawColumnDividers, kDataBrowserAttributeNone );
}

/**
 * Destructor
 */
MacMetadataView::~MacMetadataView() {
	if( control != NULL ) {
		delete control;
	} 
	
	if( comboBox != NULL ) {
		delete comboBox;
	} 
	
	if( editTextBox != NULL ) {
		delete editTextBox;
	}
	
	if( popupBox != NULL ) {
		delete popupBox;
	}
	
	if( categoryButton != NULL ) {
		delete categoryButton;
	}
		
	gMetadataDispatcher = NULL;
}

/**
 * Command event handling.
 */
OSStatus MacMetadataView::onCommand( HICommand *command ){
	OSStatus result = eventNotHandledErr;
	
	if( !isCategoryID( selectedItemID ) && selectedItemID != 0 ) {
		const Keyword* keyword = model.getKeyword( itemIDToKeywordID( selectedItemID ) );
		
		if( !(keyword->flags & Keyword::FlagsReadonly) ) {
			switch( command->commandID ) {
				case kFITSUICommandSelectMetaDataValue:
					// Handle popup menu changes, by storing the select value in the model.
					// No need to do any validation since we control the possible values.		
					controller.parseValue( itemIDToKeywordID( selectedItemID ), keyword->completions[command->menu.menuItemIndex-1] );
					popupBox->setValue(command->menu.menuItemIndex);
					
					result = noErr;
					break;
					
				case kFITSUICommandLoadCategoryEditor:
					editor = new MacTaxonomyEditorView( taxonomyModel, taxonomyController );
					editor->activateEditor();
					delete editor;
					editor = NULL;				
					dialog.select();
					
					controller.parseValue( itemIDToKeywordID( selectedItemID ), taxonomyModel.Serialize() );
					
					result = noErr;
					break;
			}		
		}
	}
	
	return result;
}

/**
 *
 *
 */
OSStatus MacMetadataView::onSetFocusPart( EventRef event ) {
	// Save current value
	if( selectedItemID != 0 && !isCategoryID( selectedItemID ) && activeControl != NULL ){
		if( !( Keyword::FlagsReadonly & model.getKeyword( itemIDToKeywordID( selectedItemID ) )->flags ) ) {
			//ValidationError error = controller.parseValue( itemIDToKeywordID( selectedItemID ), activeControl->getText() );				
		}
	}
	
	// Let the standard event handler, handle the setting of focus.
	return eventNotHandledErr;
}

OSStatus MacMetadataView::onUnicodeForKeyEvent( EventRef event ) {
	OSStatus			result = eventNotHandledErr;

	EventRef			rawKeyboardEvent = NULL;
	Char 				c;
	UInt32 				modifiers;
	
	::GetEventParameter( event, kEventParamTextInputSendKeyboardEvent, typeEventRef, NULL, sizeof( EventRef ), NULL, &rawKeyboardEvent );
	::GetEventParameter( rawKeyboardEvent, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof (Char), NULL, &c );
	::GetEventParameter( rawKeyboardEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof (UInt32), NULL, &modifiers );
	
	Bool 				parseValue 	= false;
	DataBrowserItemID 	revealID 	= 0;
	
	switch( c ) {
		case kEnterCharCode:
		case kReturnCharCode:
		case kDownArrowCharCode:
			if( selectedItemID != 0 && activeControl != NULL ) {
				parseValue 	= true;
				revealID	= nextSelectableItem( selectedItemID );	
			}
			break;
		
		case kUpArrowCharCode:
			if( selectedItemID != 0 && activeControl != NULL && !isCategoryID(selectedItemID) ) {
				parseValue 	= true;
				revealID	= prevSelectableItem( selectedItemID );
			}
			break;
	
		case kTabCharCode:
			if( selectedItemID != 0 && activeControl != NULL && !isCategoryID(selectedItemID) ) {
				parseValue = true;
				
				if( modifiers & shiftKey ) {
					revealID = prevSelectableItem( selectedItemID );
				} else {
					revealID = nextSelectableItem( selectedItemID );
				}	
			}
			break;
		
		case kHomeCharCode:
		case kPageUpCharCode:
			if( selectedItemID != 0 && activeControl != NULL && !isCategoryID(selectedItemID) ) {
				parseValue = true;
				revealID = firstSelectableItem();
			}
			break;
		
		case kEndCharCode:
		case kPageDownCharCode:
			if( selectedItemID != 0 && activeControl != NULL && !isCategoryID(selectedItemID) ) {
				parseValue = true;
				revealID = lastSelectableItem();
			}
			break;
	}
	
	if( parseValue ) {
		ValidationError error = NoError;
	
		if( activeControl != popupBox && activeControl != categoryButton ) {
			error = controller.parseValue( itemIDToKeywordID( selectedItemID ), activeControl->getText() );	
		} 
		
		
		if( NoError == error ) {
			::RevealDataBrowserItem( control->getControl(), revealID, kFITSDataKeyword, kDataBrowserRevealOnly );
		} else {
			dialog.setContinueEventLoop( true );
			Environment::showMessage( Environment::getString(IDS_INVALID_HEADER).c_str(), Environment::getString( IDS_INVALID_HEADER + error ).c_str() );
			if( activeControl == popupBox ) { 
				::SetKeyboardFocus( dialog.getWindow(), activeControl->getControl(), kControlLabelPart );
			} else if( activeControl == categoryButton ) { 
				::SetKeyboardFocus( dialog.getWindow(), activeControl->getControl(), kControlButtonPart );
			} else {
				::SetKeyboardFocus( dialog.getWindow(), activeControl->getControl(), kControlEditTextPart );	
			}
		}
		
		result = noErr;
	}
	
	
	return result;
}

DataBrowserItemID MacMetadataView::firstSelectableItem() {
	const Keyword* keyword;
	
	for( Int i = 0; i < model.getCount(); i++ ) {
		keyword = model.getKeyword( i );
		
		if( !( keyword->flags & Keyword::FlagsReadonly ) ) {
			return keywordIDToItemID( i );
		} 
	}
	
	return 0;
}

DataBrowserItemID MacMetadataView::lastSelectableItem() {
	const Keyword* keyword;
	
	for( Int i = model.getCount()-1; i >= 0; i-- ) {
		keyword = model.getKeyword( i );
		
		if( !( keyword->flags & Keyword::FlagsReadonly ) ) {
			return keywordIDToItemID( i );
		} 
	}
	
	return 0;
}

DataBrowserItemID MacMetadataView::nextSelectableItem( DataBrowserItemID itemID ) {
	const Keyword* keyword;
	
	if( itemID == 0 ){
		for( Int i = 0; i < model.getCount(); i++ ) {
			keyword = model.getKeyword( i );
			
			if( !( keyword->flags & Keyword::FlagsReadonly ) ) {
				return keywordIDToItemID( i );
			} 
		}
	} else if ( isCategoryID( itemID) ) {
		for( Int i = 0; i < model.getCount(); i++ ) {
			keyword = model.getKeyword( i );
			
			if( keyword->category >= itemIDToCategoryID( itemID ) && (keyword->flags & Keyword::FlagsReadonly) ) {
				return keywordIDToItemID( i );
			}
		}
	} else {
		UInt keywordID = itemIDToKeywordID( itemID );
		
		for( Int i = keywordID+1; i < model.getCount(); i++ ) {
			keyword = model.getKeyword( i );
			
			if( !( keyword->flags & Keyword::FlagsReadonly ) ) {
				return keywordIDToItemID( i );
			} 
		}
			
		return keywordIDToItemID(keywordID);	
	}
	
	return 0;
}

DataBrowserItemID MacMetadataView::prevSelectableItem( DataBrowserItemID itemID ) {
	const Keyword* keyword;
	
	if( itemID == 0 ){
		for( Int i = 0; i < model.getCount(); i++ ) {
			keyword = model.getKeyword( i );
			
			if( !( keyword->flags & Keyword::FlagsReadonly ) ) {
				return keywordIDToItemID( i );
			} 
		}
	} else if ( isCategoryID( itemID) ) {
		for( Int i = model.getCount()-1; i >= 0; i-- ) {
			keyword = model.getKeyword( i );
			
			if( keyword->category <= itemIDToCategoryID( itemID ) && (keyword->flags & Keyword::FlagsReadonly) ) {
				return keywordIDToItemID( i );
			}
		}
	} else {
		UInt keywordID = itemIDToKeywordID( itemID );
		
		for( Int i = keywordID-1; i >= 0; i-- ) {
			keyword = model.getKeyword( i );
			
			if( !( keyword->flags & Keyword::FlagsReadonly ) ) {
				return keywordIDToItemID( i );
			} 
		}	

		return keywordIDToItemID(keywordID);	
	}
	
	return 0;
}

/**
 *
 */
Void MacMetadataView::itemNotificationCallback( ControlRef browser, DataBrowserItemID itemID, DataBrowserItemNotification message ) {
	switch( message ) {
		case kDataBrowserContainerOpened: 
			{
				// Find keywords associated with a given category item id.
				Vector<DataBrowserItemID> itemVector;
				
				UInt categoryID = itemIDToCategoryID( itemID );
				
				for( UInt i = 0; i < model.getCount(); i++ ) {
					const Keyword* keyword = model.getKeyword( i );
					
					if( keyword->category == categoryID && !(keyword->flags & Keyword::FlagsHidden) ) {
						itemVector.push_back( keywordIDToItemID( i ) );
					}
				}
				
				//Copy vector to an array.
				DataBrowserItemID* itemList = new DataBrowserItemID[ itemVector.size() ];
				
				for( Int i = 0; i < itemVector.size(); i++ ) {
					*(itemList+i) = itemVector[i];
				}
				
				// Add items
				control->addItems( itemID, itemVector.size(), itemList, kDataBrowserItemNoProperty );
				
				if( itemList != NULL ) {
					delete[] itemList;	
				}
			}
			break;
		
		case kDataBrowserContainerClosed:
			// Hide combo box if selected item is in closing container.
			if( selectedItemID != 0 && !isCategoryID( selectedItemID ) ) {
				const Keyword* keyword = model.getKeyword( itemIDToKeywordID( selectedItemID ) );

				if( keyword->category == itemIDToCategoryID( itemID ) ) {
					// Deselect item if in container.
					selectedItemID = 0;
				}
			}
		 	break;
							
		case kDataBrowserSelectionSetChanged:
			if( doRevealID != 0 )  {
				doingReveal = true;
			
				::RevealDataBrowserItem( control->getControl(), doRevealID, kFITSDataKeyword, kDataBrowserRevealOnly );
				
				if( activeControl == popupBox ) {
					::SetKeyboardFocus( dialog.getWindow(), activeControl->getControl(), kControlLabelPart );		
				} else if( activeControl == categoryButton ) { 
					::SetKeyboardFocus( dialog.getWindow(), activeControl->getControl(), kControlButtonPart );
				} else {
					::SetKeyboardFocus( dialog.getWindow(), activeControl->getControl(), kControlEditTextPart );	
				}
				
				doingReveal = false;
				return;
			}							
			
			// itemID is always 0, so it can be used. See itemDataCallback for how selectedItemID is set.
			if( !isCategoryID( selectedItemID ) && selectedItemID != 0 ) {
				const Keyword* keyword = model.getKeyword( itemIDToKeywordID( selectedItemID ) );
				
				if( !(keyword->flags & Keyword::FlagsReadonly) ) {	
					
					if( keyword->name == CATEGORYIDENTIFIER ) {
						activeControl = categoryButton;
					} else if( keyword->type == Keyword::TypeStringCv ) {
						MenuRef	menu;

						popupBox->getData( kControlMenuPart, kControlPopupButtonMenuRefTag, &menu, sizeof( menu ), NULL );
	
						if( menu != NULL ) {
							::DeleteMenuItems( menu, 1, ::CountMenuItems(menu) );
						
							// Add menu items
						    for( Int j = 0; j < keyword->completions.size(); j++ ) {
						    	CFStringRef optionText = Environment::StringToCFString( keyword->completions[j].c_str() );
								::AppendMenuItemTextWithCFString( menu, optionText, (1 << 14), kFITSUICommandSelectMetaDataValue, NULL );
								::CFRelease( optionText );
						    }
						    
						    // Make sure we can select the menu items.
						    popupBox->setMaximumValue( keyword->completions.size() );
						    
						    popupBox->setValue( 0 );
						    
						    // Select appropiate menu item
							for( Int j = 0; j < keyword->completions.size(); j++ ) {
								if( keyword->completions[j] == keyword->value ) {
									popupBox->setValue( j + 1 );			
								}
							}				    
						}
						
						activeControl = popupBox;
					} else if ( keyword->completions.size() > 0 ) {
						// First remove all items (from the end orelse it fucks up).
						for( UInt i = HIComboBoxGetItemCount( comboBox->getControl() ); i > 0; i-- ) {
							HIComboBoxRemoveItemAtIndex( comboBox->getControl(), i-1 );
						}
										
						// And the add completion items
						for( Int j = 0; j < keyword->completions.size(); j++ ) {
							CFStringRef optionText = Environment::StringToCFString( keyword->completions[j].c_str() );
							HIComboBoxAppendTextItem( comboBox->getControl(), optionText, NULL );	
							::CFRelease( optionText );
						}
						
						activeControl = comboBox;
					} else {
						activeControl = editTextBox;
					}
					
					if( !keepText ) {
						CFStringRef valueText = Environment::StringToCFString( keyword->value.c_str() );
						if( activeControl != popupBox || activeControl != categoryButton ) {
							::SetControlData( activeControl->getControl(), kControlEditTextPart, kControlEditTextCFStringTag, sizeof( CFStringRef ), &valueText );	
						}

						::CFRelease( valueText );	
					} else {
						keepText = false;
					}
											
					HIViewSetVisible( editTextBox->getControl(), false );
					HIViewSetVisible( categoryButton->getControl(), false );
					HIViewSetVisible( comboBox->getControl(), false );
					HIViewSetVisible( popupBox->getControl(), false );
					HIViewSetVisible( activeControl->getControl(), true );	
					
					
					if( activeControl == popupBox ) { 
						::SetKeyboardFocus( dialog.getWindow(), activeControl->getControl(), kControlLabelPart );
					} else if (activeControl == categoryButton) {
						::SetKeyboardFocus( dialog.getWindow(), activeControl->getControl(), kControlButtonPart );
					} else { 
						::SetKeyboardFocus( dialog.getWindow(), activeControl->getControl(), kControlEditTextPart );	
					}
				
				} else {
					activeControl = NULL;
					HIViewSetVisible( comboBox->getControl(), false );	
					HIViewSetVisible( editTextBox->getControl(), false );	
					HIViewSetVisible( categoryButton->getControl(), false );
					HIViewSetVisible( popupBox->getControl(), false );
				}
			} else {
				activeControl = NULL;
				HIViewSetVisible( comboBox->getControl(), false );	
				HIViewSetVisible( editTextBox->getControl(), false );
				HIViewSetVisible( categoryButton->getControl(), false );
				HIViewSetVisible( popupBox->getControl(), false );		 
			}
			break;
	}
}


/**
 *
 *
 */
OSStatus MacMetadataView::itemDataCallback( ControlRef browser, DataBrowserItemID itemID, DataBrowserPropertyID property, DataBrowserItemDataRef itemData, Boolean changeValue ) {
	OSStatus status = noErr;
	String category;
	
	if( !changeValue ) {
		// Data for the description column is provied in custom draw.
		switch( property ) {
			case kFITSDataKeyword:
				// Provide data for the keyword column
				CFStringRef text;
			
				if( isCategoryID(itemID) ) {
					category = model.getCategory( itemIDToCategoryID(itemID) );
					text = Environment::StringToCFString( category.c_str() );
				} else {
					const Keyword* key = model.getKeyword( itemIDToKeywordID(itemID) );
					text = Environment::StringToCFString( key->name.c_str() );
				}
				
				status = control->setItemDataText( itemData, text );
				::CFRelease( text );				
				break;
				
			case kDataBrowserItemIsContainerProperty:
				// Confirm that item is a container (category)
				if( isCategoryID(itemID) ) {	
					status = control->setItemDataBooleanValue( itemData, true );
				}
				break;
				
			case kDataBrowserItemIsSelectableProperty:
				// Save current value
				if( selectedItemID != 0 && !isCategoryID( selectedItemID ) && activeControl != NULL ){
					if( !( Keyword::FlagsReadonly & model.getKeyword( itemIDToKeywordID( selectedItemID ) )->flags ) && !doingReveal ) {
						ValidationError error = NoError;
					
						if( activeControl != popupBox && activeControl != categoryButton ) {
							error = controller.parseValue( itemIDToKeywordID( selectedItemID ), activeControl->getText() );	
						}

						if( error != NoError ) {
							dialog.setContinueEventLoop( true );
							Environment::showMessage( Environment::getString(IDS_INVALID_HEADER), Environment::getString( IDS_INVALID_HEADER + error ) );
							doRevealID = selectedItemID;
						}
					} else {
						if( doingReveal ) {
							doRevealID = 0;
							keepText = true;
						}
						
					}
				}
	
				// Hack to get the itemID of the selected row in notification callback where it is 0.
				if( !isCategoryID(itemID) ) {	
					const Keyword* keyword = model.getKeyword( itemIDToKeywordID(itemID) );
					if( ( keyword->flags & Keyword::FlagsReadonly ) ) {
						status = control->setItemDataBooleanValue( itemData, false );	
						selectedItemID = 0;
					} else {
						status = control->setItemDataBooleanValue( itemData, true );
						selectedItemID = itemID;
					}
				} else {
					status = control->setItemDataBooleanValue( itemData, false );
					selectedItemID = 0;
				}
				break;
			
			case kDataBrowserItemIsEditableProperty:
				if( !isCategoryID(itemID) ) {	
					const Keyword* keyword = model.getKeyword( itemIDToKeywordID(itemID) );
					status = control->setItemDataBooleanValue( itemData, !(keyword->flags & Keyword::FlagsReadonly) );
				} else {
					status = control->setItemDataBooleanValue( itemData, false );	
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
 * Custom drawing of value column in data browser
 */
Void MacMetadataView::drawItemCallback( ControlRef browser, DataBrowserItemID itemID,  DataBrowserPropertyID property, DataBrowserItemState itemState, const Rect *theRect, SInt16 gdDepth, Boolean colorDevice ) {
	//-------------------
	//Get bit depth of port.
	//-------------------
	PixMapHandle pixMap = ::GetPortPixMap( GetQDGlobalsThePort() );
	
	// Get color used for drawing 
	RGBColor oldColor;
	::GetForeColor( &oldColor );
	
	//-------------------
	// Draw background
	//-------------------
	if( itemState == kDataBrowserItemIsSelected ) {
		Rect r;
	
		r.top 		= theRect->top;
		r.bottom 	= theRect->bottom;
		r.left 		= theRect->left - 12;
		r.right		= theRect->right;		
	
		// TODO: OS X 10.4 should use this HIThemeBrushCreateCGColor instead.
		// Get alternate row highlight color.
		RGBColor outColor;
		
		if( ::IsControlActive( control->getControl() ) ) {
			::GetThemeBrushAsColor( kThemeBrushAlternatePrimaryHighlightColor, (*pixMap)->pixelSize, true, &outColor );	
		} else {
			::GetThemeBrushAsColor( kThemeBrushSecondaryHighlightColor, (*pixMap)->pixelSize, true, &outColor );
		}
		
		::RGBForeColor( &outColor );
		::PaintRect( &r );
	}
	
	//-------------------
	// Place and show/hide textbox or combobox in databrowser 
	//-------------------
	if( activeControl != NULL && selectedItemID != 0 ) {
		Rect selectedRect = { 0, 0, 0, 0 };
		::GetDataBrowserItemPartBounds( control->getControl(), selectedItemID, kFITSDataDescription, kDataBrowserPropertyContentPart, &selectedRect );
	
		if( selectedRect.top == 0 && selectedRect.left == 0 && selectedRect.right == 0 && selectedRect.bottom == 0 ) {
			HIViewSetVisible( activeControl->getControl(), false );
		} else {
			HIViewSetVisible( activeControl->getControl(), true );
			if( activeControl == editTextBox ) {
				HIViewPlaceInSuperviewAt( activeControl->getControl(), selectedRect.left+3, selectedRect.top+3);	
			} else if( activeControl == popupBox ) {
				HIViewPlaceInSuperviewAt( activeControl->getControl(), selectedRect.left-4, selectedRect.top);	
			} else if( activeControl == categoryButton ) {
				HIViewPlaceInSuperviewAt( activeControl->getControl(), selectedRect.left+405, selectedRect.top+2);	
			} else {
				HIViewPlaceInSuperviewAt( activeControl->getControl(), selectedRect.left, selectedRect.top );
			}	
		}
	}
	
	//-------------------
	// Draw items
	//-------------------
	if( !isCategoryID(itemID) ) {
		const Keyword* keyword = model.getKeyword( itemIDToKeywordID(itemID) );
	
		if( itemState != kDataBrowserItemIsSelected || keyword->name == CATEGORYIDENTIFIER ) {
			// Draw text that is not beign edited.
			CFMutableStringRef text = ::CFStringCreateMutable( NULL, 0 );
			::CFStringAppendCString( text, keyword->value.c_str(), kCFStringEncodingMacRoman );
			
			Rect r;
			r.top 		= theRect->top + 3;
			r.bottom 	= theRect->bottom - 2;
			r.left 		= theRect->left + 3;
			r.right		= (activeControl == categoryButton) ? theRect->right - 66 : theRect->right - 33;
		
			// Draw readonly fields as inactive.
			if( keyword->flags & Keyword::FlagsReadonly ) {
				if( itemState != kDataBrowserItemIsSelected ) {
					// Readonly rows
					::SetThemeTextColor( kThemeTextColorDialogInactive, (*pixMap)->pixelSize, true );	
				} else {
					// Readonly selected rows.
					::RGBForeColor( &oldColor );
				}
			} else if( keyword->name == CATEGORYIDENTIFIER ) {
				if( itemState == kDataBrowserItemIsSelected ) {
					::SetThemeTextColor( kThemeBrushWhite, (*pixMap)->pixelSize, true );
				} else {
					::SetThemeTextColor( kThemeTextColorDialogActive, (*pixMap)->pixelSize, true );
				}
			} else {
				// Normal non selected rows.
				::SetThemeTextColor( kThemeTextColorDialogActive, (*pixMap)->pixelSize, true );
			}

			::TruncateThemeText( text, kThemeSmallSystemFont, kThemeStateActive, r.right - r.left, truncEnd, NULL );
			::DrawThemeTextBox( text, kThemeSmallSystemFont, kThemeStateActive, false, &r, teJustLeft, NULL );
			
			::CFRelease( text );
		}
	} 
}

/**
 * Update values in databrowser because some model has changed.
 */
Void MacMetadataView::updateListing() {
	::UpdateDataBrowserItems( control->getControl(), kDataBrowserNoItem, 0, NULL, kFITSDataDescription, kDataBrowserNoItem );
}

/**
 * Provide tool tips for the keyword column.
 */
Void MacMetadataView::itemHelpContentCallback( ControlRef browser, DataBrowserItemID itemID, DataBrowserPropertyID property, HMContentRequest inRequest, HMContentProvidedType *outContentProvided, HMHelpContentPtr ioHelpContent ) {	
	if( !isCategoryID( itemID ) ) {
		const Keyword* keyword = model.getKeyword( itemIDToKeywordID( itemID ) );
	
		ioHelpContent->tagSide 										= kHMDefaultSide;
		ioHelpContent->content[kHMMinimumContentIndex].contentType 	= kHMCFStringContent;
		ioHelpContent->content[kHMMaximumContentIndex].contentType 	= kHMNoContent;
			
		switch( property ) {
			case kFITSDataDescription:
				if( inRequest == kHMSupplyContent ) {
					CFStringRef text = Environment::StringToCFString( keyword->description.c_str() ); 
						
					ioHelpContent->content[kHMMinimumContentIndex].u.tagCFString = text;
						
					*outContentProvided = kHMContentProvided;
				}			
				break;
			
			case kFITSDataKeyword:
				if( inRequest == kHMSupplyContent ) {
					CFStringRef text = Environment::StringToCFString( keyword->description.c_str() ); 
						
					ioHelpContent->content[kHMMinimumContentIndex].u.tagCFString = text;
						
					*outContentProvided = kHMContentProvided;
				}
				break;
		}
		
		if( inRequest == kHMDisposeContent ) {
			if( ioHelpContent->content[kHMMinimumContentIndex].contentType == kHMCFStringContent ) {
				::CFRelease(ioHelpContent-> content[kHMMinimumContentIndex].u.tagCFString);
			}	
		}
		
	} else {
		*outContentProvided = kHMContentNotProvided;
	}
	
}

/**
 * Convert a keywordID to a DataBrowserItemID
 */
inline DataBrowserItemID MacMetadataView::keywordIDToItemID( UInt keywordID ) {
	return keywordID + categoryCount + 1;
}

/**
 * Convert a DataBrowserItemID to a keywordID
 */
inline UInt MacMetadataView::itemIDToKeywordID( DataBrowserItemID itemID ) {
	return itemID - categoryCount - 1;
}

/**
 * Convert a DataBrowserItemID to a categoryID
 */
inline UInt MacMetadataView::itemIDToCategoryID( DataBrowserItemID itemID ) {
	return itemID - 1;
}

/**
 * Convert a categoryID to a DataBrowserItemID
 */
inline DataBrowserItemID MacMetadataView::categoryIDToItemID( UInt categoryID ) {
	return categoryID + 1;
}

/**
 * Tell if DataBrowserItemID is a categoryID.
 */
inline Bool MacMetadataView::isCategoryID( DataBrowserItemID itemID ) {
	return itemIDToCategoryID(itemID) < categoryCount;
}


//			// Setup Quartz context
//			Rect 			portBounds;
//			CGContextRef 	context 	= NULL;
//			CGrafPtr 		currentPort = ::GetQDGlobalsThePort();
//
//			::GetPortBounds( currentPort, &portBounds );
//			MachO::QDBeginCGContext( currentPort, &context );
//			::SyncCGContextOriginWithPort( context, currentPort );
//
//			// rearrange the quartz coordinate system to match QuickDraw
//			MachO::CGContextTranslateCTM( context, 0, ( portBounds.bottom - portBounds.top ) );
//			MachO::CGContextScaleCTM( context, 1.0, -1.0 );
//			MachO::CGContextSetRGBFillColor( context, 0, 0, 0, 1 ); 
//
//			::DrawThemeTextBox( text, kThemeSmallSystemFont, kThemeStateActive, false, &r, teJustLeft, context );
//			MachO::QDEndCGContext( currentPort, &context );