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
// =============================================================================#include "Environment.h"
#include "DispatchView.h"
#include "Resources.h"

using namespace FitsLiberator::Modelling;
using namespace FitsLiberator::Windows;

DispatchView::DispatchView( ModelFramework& frameWork )
  : frameWork( frameWork ),
  stretchView( *frameWork.stretchModel, *frameWork.flowController, *frameWork.optionsModel ),
    globalView( *frameWork.globalSettingsModel, *frameWork.globalSettingsController ),
    toolView( *frameWork.toolModel, *frameWork.toolController ),
    planeView( *frameWork.planeModel, *frameWork.flowController, *frameWork.planeController ),
    histogramView( *frameWork.histogramModel, *frameWork.toolModel, *frameWork.statisticsModel, *frameWork.stretchModel, *frameWork.optionsModel, *frameWork.histogramController, *frameWork.flowController ),
	statisticsView( *frameWork.statisticsModel, *frameWork.globalSettingsModel ),
    repositoryView( *frameWork.repositoryModel, *frameWork.repositoryController, *frameWork.planeModel, *frameWork.stretchModel, *frameWork.histogramModel ),
	previewView( *frameWork.previewModel, *frameWork.toolModel, 
        *frameWork.globalSettingsModel,*frameWork.planeModel, *frameWork.progressModel,
        *frameWork.previewController, *frameWork.flowController),
		headerView( *frameWork.headerModel ), mainView( *frameWork.mainModel ),
    pixelValueView( *frameWork.pixelValueModel, *frameWork.globalSettingsModel ),
    progressModel( *frameWork.progressModel )
{
    main = NULL;
    progressModel.Attach(this);
}

DispatchView::~DispatchView() {
    progressModel.Detach(this);
}

Void DispatchView::OnInit( Dialog* owner, Int ID ) {
    
    stretchView.OnInit( owner, ID );
    globalView.OnInit( owner, ID );
    toolView.OnInit( owner, ID );
    planeView.OnInit( owner, ID);
    histogramView.OnInit( owner, ID );
	statisticsView.OnInit( owner, ID );
    previewView.OnInit( owner, ID );
    pixelValueView.OnInit( owner, ID );
    repositoryView.OnInit( owner, ID );
	headerView.OnInit( owner, ID );
	mainView.OnInit( owner, ID );

/**
*	It is crucial that this if-statement is put _after_ the OnInit() calls
*	the filename in the title bar will otherwise not be updated properly.
*/

	if( IDD_MAIN == ID )
	{
        main = owner;
		main->setEnabled( false );
		frameWork.updateViews();
    }

/*    if( IDD_PREVIEW == ID )	{
		frameWork.updateViews();
	}
*/
}

Void DispatchView::OnCommand( WPARAM wParam, LPARAM lParam ) {  
    Int control = LOWORD( wParam ); // ID of the control that sent the notification

    //
    // Dispatch the event
    stretchView.OnCommand( wParam, lParam );
    globalView.OnCommand( wParam, lParam );
    toolView.OnCommand( wParam, lParam );
    planeView.OnCommand( wParam, lParam );
    histogramView.OnCommand( wParam, lParam );
    previewView.OnCommand( wParam, lParam );
    pixelValueView.OnCommand( wParam, lParam );
    repositoryView.OnCommand( wParam, lParam );
	headerView.OnCommand( wParam, lParam );
	mainView.OnCommand( wParam, lParam );
}

Void DispatchView::OnNotify( Int control, LPNMHDR e ) {
    //
    // There is no simple way to dispatch this message
    stretchView.OnNotify( control, e );
    globalView.OnNotify( control, e );
    toolView.OnNotify( control, e );
    planeView.OnNotify( control, e );
    histogramView.OnNotify( control, e );
    statisticsView.OnNotify( control, e );
    previewView.OnNotify( control, e );
    pixelValueView.OnNotify( control, e );
    repositoryView.OnNotify( control, e );
	headerView.OnNotify( control, e );
	mainView.OnNotify( control, e );
}

Void DispatchView::OnScroll( WPARAM wParam, LPARAM lParam ) {
    // Check this event came from a control
    if( NULL != lParam ) {
        stretchView.OnScroll( wParam, lParam );
		histogramView.OnScroll( wParam, lParam );
    }
}

Void DispatchView::ShortcutFilter( Bool down, UInt virtualKey ) {
    if( virtualKey == kFITSCancelOperationShortcut )
        progressModel.Cancel();

    if( down ) {
        frameWork.toolController->keyboardShortCut( (Char)virtualKey );
    }
    else {
        frameWork.toolController->keyboardShortCutUp( (Char)virtualKey );
    }

   previewView.ShortcutFilter( down, virtualKey );
}

void DispatchView::Update(Models *m) {
    if(main != NULL) {
        Models::iterator it;
        for( it = m->begin(); it != m->end(); it++ ) {
            if( &progressModel == *it ) {
                main->setEnabled(!progressModel.QueryBusy());
            }
        }
    }
}