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

#include "NavDialog.h"

using namespace FitsLiberator::Mac;

static const char* const OPEN_EXTENSIONS[] = {
	"FITS", "FIT", "FTS", "IMG", "LBL",
};

static const char* const SAVE_EXTENSIONS[] = {
	"TIFF", "TIF"
};

bool NavDialog::showOpenDialog() {
	NavDialogCreationOptions options;

	if(noErr == NavGetDefaultDialogCreationOptions(&options)) {
		if(noErr == NavCreateChooseFileDialog(&options, NULL, &NavDialog::callback, NULL, NULL, this, &dialog)) {
			callBack = NULL;
			result   = kNavUserActionNone;
			path.clear();
			
			createFilter(OPEN_EXTENSIONS, sizeof(OPEN_EXTENSIONS)/sizeof(OPEN_EXTENSIONS[0]));
			NavDialogRun(dialog);
			return result == kNavUserActionChoose;
		}
	}
	
	return false;
}

void NavDialog::showOpenSheet(WindowRef parentWindow, NavDialog::CallBack callBack, void* userData) {
	assert(parentWindow != NULL);
	
	NavDialogCreationOptions options;
	if(noErr == NavGetDefaultDialogCreationOptions(&options)) {
		options.parentWindow = parentWindow;
		options.modality     = kWindowModalityWindowModal;	// Display the dialog as a sheet
		
		if(noErr == NavCreateChooseFileDialog(&options, NULL, &NavDialog::callback, NULL, NULL, this, &dialog)) {
			this->callBack = callBack;
			this->userData = userData;
			path.clear();
			
			createFilter(OPEN_EXTENSIONS, sizeof(OPEN_EXTENSIONS)/sizeof(OPEN_EXTENSIONS[0]));
			NavDialogRun(dialog);
		}
	}
}

void NavDialog::showSaveSheet(WindowRef parentWindow, const std::string& filename, NavDialog::CallBack callBack, void* userData) {
	assert(parentWindow != NULL);
	
	NavDialogCreationOptions options;
	if(noErr == NavGetDefaultDialogCreationOptions(&options)) {
		CFStringRef saveFileName = ::CFStringCreateWithCString( kCFAllocatorDefault, filename.c_str(), kCFStringEncodingMacRoman );
		
		options.optionFlags  = kNavPreserveSaveFileExtension;
		options.parentWindow = parentWindow;
		options.modality = kWindowModalityWindowModal;
		options.saveFileName = saveFileName;
		
		if(noErr == NavCreatePutFileDialog(&options, NULL, NULL, &NavDialog::callback, this, &dialog)) {
			this->callBack = callBack;
			this->userData = userData;
			path.clear();
			
			createFilter(SAVE_EXTENSIONS, sizeof(SAVE_EXTENSIONS)/sizeof(SAVE_EXTENSIONS[0]));
			NavDialogRun(dialog);
		}
		
		::CFRelease( saveFileName );
	}
}

void NavDialog::createFilter(const char* const extensions[], size_t count) {
	CFMutableArrayRef identifiers;
    CFStringRef       filter[count];
    
    identifiers = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
	for(size_t i = 0; i < count; ++i) {
		CFStringRef extension = CFStringCreateWithCString(kCFAllocatorDefault, extensions[i], kCFStringEncodingMacRoman);
		filter[i] = UTTypeCreatePreferredIdentifierForTag(kUTTagClassFilenameExtension, extension, kUTTypeData);
		CFArrayAppendValue(identifiers, filter[i]);
		CFRelease(extension);
	}
	
	NavDialogSetFilterTypeIdentifiers(dialog, identifiers);
	
	for(size_t i = 0; i < count; ++i)
		CFRelease(filter[i]);
	CFRelease(identifiers);
}

void NavDialog::callback(NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, void* callBackUserData) {
	NavDialog* self = reinterpret_cast<NavDialog*> (callBackUserData);
	
	switch (callBackSelector) {
	case kNavCBUserAction:
		{
			NavReplyRecord reply;
			if(noErr == NavDialogGetReply(callBackParms->context, &reply)) {
				NavUserAction action = NavDialogGetUserAction(callBackParms->context);
				
				self->result = action;				
				switch(action) {
					case kNavUserActionChoose:
						self->appendSelection(&reply.selection);
						if(self->callBack != NULL) {
							(*self->callBack)(self->userData);
						}
						break;
					case kNavUserActionSaveAs:
						self->appendSelection(&reply.selection);
						self->appendFilename(reply.saveFileName);
						if(self->callBack != NULL) {
							(*self->callBack)(self->userData);
						}
						break;
				}
				
				NavDisposeReply(&reply);
			}			
		}
		break;
	case kNavCBTerminate:
		NavDialogDispose(self->dialog);		
		EnableMenuCommand(NULL, kHICommandOpen);
	default:
		break;
	}
}

void NavDialog::appendSelection(AEDesc* selection) {
	FSRef   ref;
	
	AECoerceDesc(selection, typeFSRef, selection);
	if(noErr == AEGetDescData(selection, &ref, sizeof(FSRef))) {
		CFURLRef 	tempURL	= ::CFURLCreateFromFSRef( NULL, &ref );
		CFStringRef	pathStr	= ::CFURLCopyFileSystemPath( tempURL, kCFURLPOSIXPathStyle ); // PPC: kCFURLHFSPathStyle
		CFIndex 	pathLen	= ::CFStringGetLength(pathStr);
		char*       buffer  = new char[pathLen+1];
		
		::CFStringGetCString( pathStr, buffer, pathLen + 1,  kCFStringEncodingMacRoman );
		path.append(buffer);
		
		delete[] buffer;
		::CFRelease( pathStr );
		::CFRelease( tempURL );
	}
}

void NavDialog::appendFilename(CFStringRef filename) {
	CFIndex pathLen	= ::CFStringGetLength(filename);
	char*   buffer  = new char[pathLen+1];
	
	::CFStringGetCString( filename, buffer, pathLen + 1,  kCFStringEncodingMacRoman );
	path.append("/");
	path.append(buffer);
	
	delete[] buffer;
}
