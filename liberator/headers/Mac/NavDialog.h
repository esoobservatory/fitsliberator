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

#ifndef __NAVDIALOG_H__
#define __NAVDIALOG_H__

#include "Types.h"
#include "Carbon.h"

namespace FitsLiberator {
	namespace Mac {
		
		/**	This class is used to create and display a file open/file save dialog sheet.
		 
			Showing a basic open/save dialog box can be done in a way that blocks the call
			until the user has selected a file or pressed cancel. Displaying a sheet-style 
			dialog requires that a call-back be used to inform the application of the users
			choice. The sheet functions will return immediately and invoke a callback in case
			the application needs to take some form of action. */
		class NavDialog {
		public:
			typedef void (*CallBack)(void* userData);

			NavDialog() : dialog(NULL) {}
			/**	@brief	Shows a file open dialog.
				@param	parentWindow	Parent window to use for showing the sheet. May be NULL if there is no parent.
				@return	True if the user selected choose to open a file, false if the user pressed Cancel. */
			bool showOpenDialog();
			/**	@brief	Shows a file open sheet.
				@param	parentWindow	Parent window to use for showing the sheet.
				@param  callBack		Function that is called when the user is done messing around.
				@param	userData		User data that is supplied to the callback function. */
			void showOpenSheet(WindowRef parentWindow, CallBack callBack, void* userData);	
			/**	@brief	Shows a file save sheet.
				 @param	parentWindow	Parent window to use for showing the sheet.
				 @param	filename		Initial filename to suggest to the user.
				 @param callBack		Function that is called when the user is done messing around.
				 @param	userData		User data that is supplied to the callback function. */
			void showSaveSheet(WindowRef parentWindow, const std::string& filename, CallBack callBack, void* userData);				
			/**	@brief	Gets the path of the selected file. This value is only valid after a call to @see showOpen or 
						@see showSave - provided the call returned true. */
			inline std::string getPath() const { return path; }
			
		private:
			NavUserAction result;		///< Dialog result, set by the callback
			std::string   path;			///< Path of the selected file, only valid if result is true.
			NavDialogRef  dialog;		///< Dialog reference.
			CallBack      callBack;		///< User callback.
			void*         userData;		///< User data.
			
			void createFilter(const char* const extensions[], size_t count);
			void appendSelection(AEDesc* selection);
			void appendFilename(CFStringRef filename);
			
			/**	See http://developer.apple.com/legacy/mac/library/#documentation/Carbon/Conceptual/ProvidingNavigationDialogs/nsx_intro/nsx_intro.html for more information. */
			static void callback( NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, void* callBackUserData );
		};
								 
	} // namespace Mac end
} // namespace FitsLiberator end

#endif //__NAVDIALOG_H__