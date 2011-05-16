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
#include <Shlobj.h>	// for SHAddToRecentDocs

#include "WindowsMainView.h"
#include "Resources.h"
#include "ResourceManager.h"
#include "PreviewPage.h"

using namespace FitsLiberator;
using namespace FitsLiberator::Modelling;
using namespace FitsLiberator::Windows;


WindowsMainView::WindowsMainView( MainModel& m ) :
MainView( m ), owner( NULL ), previewPage( NULL )
{

}

/**
*	Initialize the owner
*
*/
Void WindowsMainView::OnInit( Dialog* owner, Int ID ) {
	if ( ID == IDD_MAIN )
		this->owner = (Window*)owner;
	if ( ID == IDD_PREVIEW )
		this->previewPage = (PreviewPage*)owner;
}

/**
*	Called from the HeaderView super class when the HeaderModel attached
*   is changed.
*
*/
Void WindowsMainView::updateTitle( const String& title )
{
	if ( owner != NULL ) {
		owner->setText( title );
		::SHAddToRecentDocs( SHARD_PATHA, model.getFileName().c_str() );
	}

}

Void WindowsMainView::updateNetworkWarning( Bool show )
{
	if ( this->previewPage != NULL )	
		previewPage->showWarningIcon( show );
}

Void WindowsMainView::OnCommand( WPARAM, LPARAM )
{
}
Void WindowsMainView::OnNotify( Int, LPNMHDR )
{
}
Void WindowsMainView::OnScroll( WPARAM, LPARAM )
{
}
