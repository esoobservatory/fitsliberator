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
#ifndef __COMBOBOX_H__
#define __COMBOBOX_H__

#include "Window.h"

namespace FitsLiberator {
    namespace Windows {
        /** A simple combobox wrapper */
        class ComboBox : public Window {
            public:
                ComboBox();
                virtual ~ComboBox();
				/** Adds an item to the combobox.
				    @param item The item to add.
					@param data Data to associate with the item.
					@return The zero-based index of the new item, 
					CB_ERR or CB_ERRSPACE. See CB_ADDSTRING for details. */
                Int AddItem( String item, Int data );
				Int AddItem( String item );
				/** Removes all items from the combobox. */
                Void Clear();
				/** Gets the zero-based index of the selected item.
				    @return The selected item or CB_ERR if no item is selected. */
                Int  getSelectedItem();
				/** Selects another item.
				    @param index Zero-based index of the item that is to be selected. */
                Void setSelectedItem( Int index );
				/** Gets the data associated with the selected item. 
				    @return The item data, or CB_ERR if no item is selected. */
				Int getSelectedData();
				/** Gets the data associated with an item.
					@param index Zero-based index of the item.
					@return The item data, or CB_ERR on error. */
				Int getItemData( Int index );
				/** Gets the data associated with an item.
					@param index Zero-based index of the item.
					@param data Data. */
				Void setItemData( Int index, Int data );
				/** Gets the number of items in the combobox.
					@return The number of item. */
				Int getItemCount();
				/** Gets the text of a specific item.
					@param index Zero-based index of the item.
					@return The text of the item or an empty string on error. */
				String getItem( Int index );
            private:
                typedef Window super;
        };
    }
}

/** If the MSB is set on the itemdata on a ownerdrawn listbox/combobox,
	the item is painted with a line over it. */
#define OVERLINE_MASK	0x80000000

#endif //__COMBOBOX_H__