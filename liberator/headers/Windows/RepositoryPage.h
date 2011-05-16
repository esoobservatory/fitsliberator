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
#ifndef __REPOSITORYPAGE_H__
#define __REPOSITORYPAGE_H__

#include "PropertyPage.h"
#include "DispatchView.h"
#include "Textbox.h"
#include "Combobox.h"
#include "TaxonomyEditor.h"

namespace FitsLiberator {
    namespace Windows {
        class RepositoryPage;

        /** Inplace editor base class for the repository page */
        class InplaceEditor : public Window {
        public:
            /** Activates the editor.
                @param item Index of the item being edited.
                @param value The value to display in the editor. */
			virtual Void Activate( Int item, FitsLiberator::Modelling::Keyword::Type type, const String& value );
			virtual Void OnParentCommand( WPARAM wParam, LPARAM lParam );
			virtual String Serialize() const;
        protected:
			/** Default constructor. 
                @param owner The owner object (used for callbacks). */
            InplaceEditor( RepositoryPage* owner );
			/** Attaches this instance to a window.
                @param hWnd Window handle of the window to attach. */
            Void Attach( HWND hWnd );
            /** Detaches this instance from a window. */
            Void Detach();
            /** Shared keyboard handler. This function implements the keyboard interface for the metadata tab.
                @param virtualKey Virtual keycode.
                @param flags Key event flags.
                @return Returns true if this function moved the editing to a new field, false otherwise
                @remark See PlatformSDK for information about keyboard events. */
            virtual Bool FilterKeys( Int virtualKey, Int flags );
            /** Event handler, called when the editor loses focus.
                @param newFocusedWindow Handle of the window that recieved focus. */
            virtual Void OnKillFocus( HWND newFocusedWindow );
            /** Creates a standard Windows UI font. Used to make the editors look nice. 
                @return A handle to the font or NULL on failure. */
            HFONT createFont();
        private:
            typedef Window super;

            /** Custom window procedure, @see Window::ProcessMessage */
            static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

            WNDPROC         oldWindowProc;      ///< Pointer to the original window proc
            RepositoryPage* owner;              ///< Owner object
            Int             item;               ///< Item being edited by this editor
        };

        /** A simple single line no-validation textbox editor */
        class InplaceTextbox : public InplaceEditor {
            typedef InplaceEditor super;
        public:
            /** Creates a textbox editor.
                @param owner The owner object (used for callbacks).
                @param parent Parent window handle.
                @param bounds Window bounds of the editor. */
            InplaceTextbox( RepositoryPage* owner, HWND parent, const RECT* bounds );
            virtual Void Activate( Int item, FitsLiberator::Modelling::Keyword::Type type, const String& value );
        };

        /** A simple combobox editor. Allows the user to write a value or select it from a dropdown. */
        class InplaceComboBox : public InplaceEditor {
        public:
            /** Creates a combobox editor.
                @param owner The owner object (used for callbacks).
                @param parent Parent window handle. */
            InplaceComboBox( RepositoryPage* owner, HWND parent, const RECT* bounds );
            /** Adds an item to the combo box. Calling this function has no effect unless InplaceComboBox::EditItem has been called first. 
                @param item Item to add. */
            Void AddItem( const String& );
            virtual Void Activate( Int item, FitsLiberator::Modelling::Keyword::Type type, const String& value );
        protected:
            virtual Void OnKillFocus( HWND );
            /** @see Window::OnDestroy. */
            virtual Void OnDestroy( HWND );
        private:
            typedef InplaceEditor super;

            /** Custom window proc for the embedded textbox. @see Window::ProcessMessage. */
            static LRESULT CALLBACK EditProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

            HWND    editHandle;             ///< Handle to the embedded textbox
            WNDPROC oldEditProc;            ///< Pointer to the original textbox window proc
        };

		class InplaceDropdown : public InplaceEditor {
            typedef InplaceEditor super;
        public:
            /** Creates a dropdown list editor.
                @param owner The owner object (used for callbacks).
                @param parent Parent window handle. */
            InplaceDropdown( RepositoryPage* owner, HWND parent, const RECT* bounds );
            /** Adds an item to the dropdown. Calling this function has no effect unless InplaceComboBox::EditItem has been called first. 
                @param item Item to add. */
            Void AddItem( const String& );
            virtual Void Activate( Int item, FitsLiberator::Modelling::Keyword::Type type, const String& value );
		};

		class InplaceCategoryButton : public InplaceEditor {
			typedef InplaceEditor super;

			HTREEITEM InsertItem( HWND treeview, HTREEITEM parent, TCHAR* text, TCHAR value );

			TaxonomyEditor* editor;			///< Editor dialog.
			FitsLiberator::Modelling::TaxonomyEditorModel& model;
			FitsLiberator::Modelling::TaxonomyEditorController& controller;
		protected:
			void OnKillFocus( HWND newFocusedWindow );
		public:
			/** Creates a dropdown treeview editor.
				@param model Model to use for the editor.
				@param controller Controller to use for the editor.
				@param owner The owner object (used for callbacks).
				@param parent Parent window handle.
				@param bounds Boundaries of the editor. */
			InplaceCategoryButton( FitsLiberator::Modelling::TaxonomyEditorModel& model, 
				FitsLiberator::Modelling::TaxonomyEditorController& controller, 
				RepositoryPage* owner, HWND parent, const RECT* bounds );
			void OnParentCommand( WPARAM wParam, LPARAM lParam );
			String Serialize() const;
		};

		class InplaceCalendar : public InplaceEditor {
			typedef InplaceEditor super;
		public:
			/** Creates a calendar editor.
				@param owner The owner object (used for callbacks).
				@param parent Parent window handle.
				@param bounds Boundaries of the editor. */
			InplaceCalendar( RepositoryPage* owner, HWND parent, const RECT* bounds );
            virtual Void Activate( Int item, FitsLiberator::Modelling::Keyword::Type type, const String& value );
		};

        /**
         * Implements the Virtual Repository tab of the main dialog
         */
        class RepositoryPage : public PropertyPage {
        public:
            RepositoryPage( DispatchView& );
            virtual ~RepositoryPage();

            /** Updates the information in the listview because a model has changed. */
            Void Update();
            /** While similar to @see RepositoryPage::EditItem this method searches for
                an editable item if the item specified by itemIndex is not editable.
                @param itemIndex Index of the item to edit.
                @param searchDown Indicates the direction to search for an editable item.
                @remark Because this method searches for an editable item, the item that
                becomes editable may not be the same as that specified by itemIndex. In fact
                it is possible that no item will be editable as a result of this call. */
            Void BeginEdit( Int, Bool = true );
            /** Returns the number of items (keywords). */
            UInt getItemCount() const;
            /** Returns the number of items that will fit in a page. */
            UInt getItemsPerPage() const;
        protected:
            virtual Bool OnInit(HWND);
            virtual Void OnDestroy( HWND );
            virtual LRESULT OnNotify( Int, NMHDR* );
        private:
            typedef PropertyPage super;
            friend class InplaceEditor;

            HWND    listviewHandle;                                         ///< Listview window handle
            WNDPROC oldListProc;                                            ///< Listview standard window procedure.

            /** Maps a litview item index to a keyword index.
                @param index Item index. 
                @return Index of the corresponding keyword. */
            Int keywordIndexFromItem( Int index ) const;
            /** Maps a litview item index to a keyword index.
                @param index Item index. 
                @return Index of the corresponding keyword. */
            const FitsLiberator::Modelling::Keyword* keywordFromItem( Int index ) const;
            /** Adds columns to the listview 
                @return true on success, false if a column could not be added */
            Bool initColumns();
            /** Adds keyword categories to the listview
                @return true on success, false if a category could not be added */
            Bool initGroups();
            /** Adds the keywords to the listview
                @return true on success, false if a keyword could not be added */
            Bool initItems();
            /** Event handler, called when the listview needs to determine the strings to display for an item, 
                coresponds to LVN_GETDISPINFO.
                @param e Event parameters */
            Void OnGetDisplayInfo( NMLVDISPINFO* e );
            /** Event handler, called when the listview needs to determine the strings to show as tooltips,
                coresponds to LVN_GETINFOTIP.
                @param e Event parameters. */
            Void OnGetInfotip( NMLVGETINFOTIP* e );
            /** Event handler, called when the a key is pressed and the listview has input focus. Coresponds to LVN_KEYDOWN 
                @param e Event parameters */
            Void OnKeyDown( NMLVKEYDOWN* e );
            /** Event handler, called when a mouse button is depressed in the listview. 
                @see Window::OnMouseDown. */
            Bool OnListviewMouseDown( Int, Int, Int );
            /** Begins editing an item.
                @param itemIndex Index of the item to edit.
                @param content Initial content of the editor. If this parameter is NULL, the initial content is taken from the model. */
            Void EditItem( Int itemIndex, String* content );
            /** Copies the data from an editor into the model. Note that this method also destroys the editor.
                @param itemIndex Index of the item to save.
                @param editor Editor to snatch the data from. */
            Void SaveItem( Int, InplaceEditor* );
            /** Cancels the editing of an item.
                @param itemIndex Index of the item being edited.
                @param editor Editor attached to this item. */
            Void CancelItem( Int, InplaceEditor* );
            /** Custom listview window procedure, @see Window::ProcessMessage */
            static LRESULT CALLBACK ListProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

            DispatchView&                                   dispatchView;   ///< Event dispatcher.
            FitsLiberator::Modelling::RepositoryModel*      model;          ///< Model
            FitsLiberator::Modelling::RepositoryController* controller;     ///< Controller 
			FitsLiberator::Modelling::TaxonomyEditorModel*      taxonomyEditorModel;
			FitsLiberator::Modelling::TaxonomyEditorController* taxonomyEditorController;
            Int                                             currentItem;    ///< Index of the item being edited.      
			InplaceEditor*									currentEditor;	///< Current editor
        };
    }
}
#endif // __REPOSITORYPAGE_H__