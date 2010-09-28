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
#include "WindowsStretchView.h"
#include "ResourceManager.h"
#include "TextUtils.h"
#include "MainDialog.h"

using namespace FitsLiberator;
using namespace FitsLiberator::Modelling;
using namespace FitsLiberator::Windows;

struct AddStretchFunction {
	ComboBox& dropDown;
	Bool startGroup;
	Int index;

	AddStretchFunction( ComboBox& d )
	  : dropDown( d ) {
		this->index = 0;
		this->startGroup = false;
	}

	Void operator()( StretchFunction stretchFunction, Int string ) {
		if( stretchFunction == stretchNoStretch ) {
			startGroup = true;
		}
		else {
			dropDown.AddItem( ResourceManager::LoadString( string ), index | (startGroup ? OVERLINE_MASK : 0 ) );
			startGroup = false;
		}
		index++;		
	}
};

/**
 * Default constructor.
 * @param model StretchModel to attach to.
 * @see StretchView::StretchView
 * OBSOLETE
 */
/*
WindowsStretchView::WindowsStretchView( StretchModel& model, StretchController& controller, HistogramModel& histoModel, HistogramController& histogramController )
: StretchView( model, controller, histoModel, histogramController ) {

}
*/

WindowsStretchView::WindowsStretchView( StretchModel& m, FlowController& fc, OptionsModel& optMod ) :
StretchView( m, fc, optMod )
{

}

Void WindowsStretchView::OnInit( Dialog* owner, Int ID ) {
    if( IDD_MAIN == ID ) {
        mainDialog = (MainDialog*)owner;
    }
    else if( IDD_PREVIEW == ID ) {
        //
        // Get handles to the controls
        function = new DropdownButton( owner->getItem( IDC_STRETCH_FUNCTION ) );
        backgroundLevel.Attach( owner->getItem( IDC_STRETCH_BACKGROUND ) );
		maxRange.Attach( owner->getItem( IDC_STRETCH_MAXRANGE ) );
        reScale.Attach( owner->getItem( IDC_STRETCH_RESCALE ) );
        //
        // Initialize the function list
		//AddStretchFunction f( function );
		//enumerateFunctions( f );
		//setStretchFunction( getStretchFunctionIndex() );
		function->setText( " " + ResourceManager::LoadString( IDS_STRETCH_LINEAR + getStretchFunctionIndex() ) );
		function->Resource(IDM_STRETCH_MENU);

        //
        // Initialize the textboxes
        backgroundLevel.setValue( model.getBackground() );
		reScale.setValue( model.getRescaleFactor() );
		maxRange.setValue( model.getPeakLevel() );

		applyStretchButton.setHandle( owner->getItem( IDC_STRETCH_APPLYSTRETCH ) );
		applyStretchButton.setEnabled( !(optionsModel.ApplyStretchDirectly()) );
    }
}

Void WindowsStretchView::OnCommand( WPARAM wParam, LPARAM lParam ) {
    UInt code = HIWORD(wParam);
    volatile UInt command = LOWORD(wParam);
    switch( code ) {
        case NTN_ENTERPRESSED:
            OnNumericEnter( command );
            break;
        case NTN_GOTFOCUS:
            OnNumericGotFocus( command );
            break;
        case NTN_LOSTFOCUS:
            OnNumericLostFocus( command );
            break;
		default:
            switch( command ) {
                case IDC_STRETCH_RESET:
                    onReset();
			        break;
                case IDC_STRETCH_AUTOBACKGROUND:
                    onAutoBackground();
                    break;
                case IDC_STRETCH_AUTOPEAK:
                    onAutoPeak();
                    break;
                case IDC_STRETCH_AUTOSCALE:
                    onAutoScale();
                    break;
				case IDC_STRETCH_APPLYSTRETCH:
					if ( backgroundLevel.getValue() != model.getBackground() || 
						 reScale.getValue() != model.getRescaleFactor() ||
						 maxRange.getValue() != model.getPeakLevel() )
					{
						onBackgroundScalePeak( backgroundLevel.getValue(), 
							maxRange.getValue(), reScale.getValue() );
					}				
					break;
				default:
					if( command >= IDC_STRETCH_LINEAR && command < IDC_STRETCH_LAST ) {
						onStretch(command - IDC_STRETCH_LINEAR);
					}
					break;
            }
            break;
    }
}

Void WindowsStretchView::OnNumericEnter( UInt command ) {
    switch( command ) {
        case IDC_STRETCH_BACKGROUND:
			if ( backgroundLevel.getValue() != model.getBackground() )
			{
				onBackground( backgroundLevel.getValue() );
			}
            break;
        case IDC_STRETCH_RESCALE:
			if ( reScale.getValue() != model.getRescaleFactor() )
			{
                onScale( reScale.getValue() );
			}
            break;
        case IDC_STRETCH_MAXRANGE:
            if ( maxRange.getValue() != model.getPeakLevel() )
			{
				onPeak( maxRange.getValue() );
			}
            break;
    }
}

Void WindowsStretchView::OnFunctionSelected() {	

}

Void WindowsStretchView::OnNumericGotFocus( UInt ) {    
    mainDialog->disableShortcuts();
}

Void WindowsStretchView::OnNumericLostFocus( UInt command ) {
    mainDialog->enableShortcuts();
    switch( command ) {
        case IDC_STRETCH_BACKGROUND:
            if( backgroundLevel.getValue() != model.getBackground() ) {
                onBackground( backgroundLevel.getValue() );
            }
            break;
        case IDC_STRETCH_RESCALE:
			if ( reScale.getValue() != model.getRescaleFactor() ) {
				onScale( reScale.getValue() );
			}
            break;
        case IDC_STRETCH_MAXRANGE:
			if ( maxRange.getValue() != model.getPeakLevel() ) {
				onPeak( maxRange.getValue() );
			}
            break;
    }
}

Void WindowsStretchView::OnNotify( Int idCtrl, LPNMHDR pnmh ) {

}

Void WindowsStretchView::OnScroll( WPARAM wParam, LPARAM lParam ) {

}

Void WindowsStretchView::setPeakLevel(Double d)
{
	maxRange.setValue( d );
	
}

Void WindowsStretchView::setRescaleFactor(Double d)
{
	reScale.setValue( d );
}

/**
 * @see StretchView::setStretchFunction
 */
Void WindowsStretchView::setStretchFunction( Int index ) {
	function->setText( " " + ResourceManager::LoadString( IDS_STRETCH_LINEAR + getStretchFunctionIndex() ) );	
/*
	Int i;
	for( i = 0; i < function.getItemCount(); i++ )
		if( (function.getItemData( i ) & ~(OVERLINE_MASK)) == index )
			break;
	function.setSelectedItem( i );*/
}


/**
 * @see StretchView::setStretchFunction
 */
Void WindowsStretchView::setBackgroundLevel( Double value ) {
    backgroundLevel.setValue( value );
}

/**
 * @see StretchView::setScale
 */
Void WindowsStretchView::setScale( Double value ) {
    //scale.setValue( value );
}


/**
*  @see StretchView::setApplyStretchEnabled
*/
Void WindowsStretchView::setApplyStretchEnabled( Bool enabled )
{
	applyStretchButton.setEnabled( enabled );
	

}