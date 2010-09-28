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

#include "OptionsDialog.h"
#include "ResourceManager.h"

#define CHECK(id,checked) ::SendMessage(getItem(id), BM_SETCHECK, (checked) ? BST_CHECKED : BST_UNCHECKED, 0 );

using FitsLiberator::Windows::DropdownButton;
using FitsLiberator::Windows::OptionsDialog;
using FitsLiberator::Windows::ResourceManager;
using FitsLiberator::Modelling::OptionsModel;
using FitsLiberator::Modelling::OptionsController;
using FitsLiberator::Modelling::OptionsView;
using FitsLiberator::Modelling::Models;

OptionsDialog::OptionsDialog(OptionsModel& m, OptionsController& c)
  : super(IDD_OPTIONS), view(m, c) {
    stretch = 0;
}

bool
OptionsDialog::OnInit(HWND hWnd) {
    if( !super::OnInit( hWnd ) )
        return false;

	OSVERSIONINFO versionInfo = {0};
	versionInfo.dwOSVersionInfoSize = sizeof( versionInfo );
	::GetVersionEx( &versionInfo );
	if( versionInfo.dwMajorVersion < 6 ) {
        Window oldButton(getItem(IDC_OPTIONS_CLEARHISTORY_XP));
        Window newButton(getItem(IDC_OPTIONS_CLEARHISTORY));

        oldButton.setVisible(true);
        newButton.setVisible(false);


        FitsLiberator::Rectangle bounds = getBounds();
        bounds.bottom -= Window(getItem(IDC_OPTIONS_MARGIN)).getBounds().getHeight();
        setBounds(bounds);
	}

    guess.setHandle(getItem(IDC_OPTIONS_GUESS));
    guess.AddItem("Percentage");
    guess.AddItem("Median ± StdDev");
    guess.AddItem("Mean ± StdDev");
    
    stretch = new DropdownButton(getItem(IDC_OPTIONS_STRETCH));
    stretch->Resource(IDM_STRETCH_MENU);

    scaledPeak.Attach(getItem(IDC_OPTIONS_SCALEDPEAK));
    
    blackLevel.Attach(getItem(IDC_OPTIONS_BLACKLEVEL));
    whiteLevel.Attach(getItem(IDC_OPTIONS_WHITELEVEL));
    blackLevel.setPrecision(0);
    blackLevel.Postfix(" %");
    whiteLevel.setPrecision(0);
    whiteLevel.Postfix(" %");

    Update();

    return true;
}

void 
OptionsDialog::OnCommand(WPARAM wParam, LPARAM lParam) {
    Int code    = HIWORD(wParam);
    Int command = LOWORD(wParam);

    switch( code ) {
        case CBN_SELCHANGE:
            OnGuessSelected(guess.getSelectedItem());
            break;
        case NTN_ENTERPRESSED:
        case NTN_LOSTFOCUS:
            switch( command ) {
                case IDC_OPTIONS_BLACKLEVEL:
                    OnBlackLevelPercentageChanged(blackLevel.getValue() / 100.0);
                    break;
                case IDC_OPTIONS_WHITELEVEL:
                    OnWhiteLevelPercentageChanged(whiteLevel.getValue() / 100.0);
                    break;
                case IDC_OPTIONS_SCALEDPEAK:
                    OnScaledPeakChanged(scaledPeak.getValue());
                    break;
            }
            break;
        default:
            switch( command ) {
                case IDOK:
                    Close(IDOK);
                    break;
                case IDCANCEL:
                    revert = true;
                    Close(IDCANCEL);
                    break;
                case IDC_OPTIONS_DEFAULTS:
                    controller.Defaults();
                    break;
                case IDC_OPTIONS_CLEARHISTORY:
                case IDC_OPTIONS_CLEARHISTORY_XP:
                    controller.ClearHistory();
                    break;
                case IDC_OPTIONS_SHOWGRID:
                    controller.toggleMarker(OptionsModel::MarkerGrid);
                    break;
                case IDC_OPTIONS_SHOWZERO:
                    controller.toggleMarker(OptionsModel::MarkerZero);
                    break;
                case IDC_OPTIONS_SHOWBACKGROUND:
                    controller.toggleMarker(OptionsModel::MarkerBackground);
                    break;
                case IDC_OPTIONS_SHOWPEAK:
                    controller.toggleMarker(OptionsModel::MarkerPeak);
                    break;
				case IDC_OPTIONS_SHOWMEAN:
					controller.toggleMarker(OptionsModel::MarkerMean);
					break;
				case IDC_OPTIONS_SHOWSTRETCHEDMEAN:
					controller.toggleMarker(OptionsModel::MarkerStretchedMean);
					break;
				case IDC_OPTIONS_SHOWSCALEDMEAN:
					controller.toggleMarker(OptionsModel::MarkerScaledMean);
					break;
				case IDC_OPTIONS_SHOWMEDIAN:
					controller.toggleMarker(OptionsModel::MarkerMedian);
					break;
				case IDC_OPTIONS_SHOWSTRETCHEDMEDIAN:
					controller.toggleMarker(OptionsModel::MarkerStretchedMedian);
					break;
				case IDC_OPTIONS_SHOWSCALEDMEDIAN:
					controller.toggleMarker(OptionsModel::MarkerScaledMedian);
					break;
				case IDC_OPTIONS_APPLYSTRETCHDIRECTLY:
					controller.toggleApplyStretchDirectly();
					break;
				default:
                    if( command >= IDC_STRETCH_LINEAR && command <= IDC_STRETCH_LOG_SQRT ) {
                        OnStretchSelected(command - IDC_STRETCH_LINEAR);
                    }
                    break;
            }    
    }
}

LRESULT 
OptionsDialog::OnNotify(Int control, NMHDR* e) {
    if( control == IDC_OPTIONS_STRETCH && e->code == NM_CUSTOMDRAW ) {
		LPNMCUSTOMDRAW pncd = reinterpret_cast<LPNMCUSTOMDRAW>(e);
		switch( pncd->dwDrawStage ) {
			case CDDS_PREPAINT:
				::SetWindowLong(hWnd, DWL_MSGRESULT, CDRF_NOTIFYPOSTPAINT);
				return TRUE;
			case CDDS_POSTPAINT:
				::SendMessage(getItem(control), BM_DRAWARROW, 0, reinterpret_cast<LPARAM>(pncd));
				::SetWindowLong(hWnd, DWL_MSGRESULT, CDRF_NOTIFYPOSTPAINT);
				return TRUE;						
		}
	}

    return super::OnNotify( control, e );
}

void
OptionsDialog::Update() {
    guess.setSelectedItem(model.GuessMethod());
    stretch->setText(" " + ResourceManager::LoadString(IDS_STRETCH_LINEAR + getStretchFunctionIndex() ));
    scaledPeak.setValue(model.ScaledPeak());
    blackLevel.setValue(model.BlackLevelPercentage() * 100.0);
    blackLevel.setEnabled(model.EditableLevels());
    whiteLevel.setValue(model.WhiteLevelPercentage() * 100.0);
    whiteLevel.setEnabled(model.EditableLevels());

    unsigned int markers = model.HistogramMarkers();
	
	CHECK( IDC_OPTIONS_SHOWGRID, markers & OptionsModel::MarkerGrid );
	CHECK( IDC_OPTIONS_SHOWZERO, markers & OptionsModel::MarkerZero );
	CHECK( IDC_OPTIONS_SHOWBACKGROUND, markers & OptionsModel::MarkerBackground );
	CHECK( IDC_OPTIONS_SHOWPEAK, markers & OptionsModel::MarkerPeak );
	CHECK( IDC_OPTIONS_SHOWMEAN, markers & OptionsModel::MarkerMean );
	CHECK( IDC_OPTIONS_SHOWSTRETCHEDMEAN, markers & OptionsModel::MarkerStretchedMean );
	CHECK( IDC_OPTIONS_SHOWSCALEDMEAN, markers & OptionsModel::MarkerScaledMean );
	CHECK( IDC_OPTIONS_SHOWMEDIAN, markers & OptionsModel::MarkerMedian );
	CHECK( IDC_OPTIONS_SHOWSTRETCHEDMEDIAN, markers & OptionsModel::MarkerStretchedMedian );
	CHECK( IDC_OPTIONS_SHOWSCALEDMEDIAN, markers & OptionsModel::MarkerScaledMedian );
	CHECK( IDC_OPTIONS_APPLYSTRETCHDIRECTLY, model.ApplyStretchDirectly() );

	

    ::EnableWindow(getItem(IDC_OPTIONS_CLEARHISTORY), model.ClearHistory() ? FALSE : TRUE);
    ::EnableWindow(getItem(IDC_OPTIONS_CLEARHISTORY_XP), model.ClearHistory() ? FALSE : TRUE);
}

HBRUSH
OptionsDialog::OnCtlColor(HWND hWnd, HDC hDC, UINT ctlType) {
	if(ctlType == CTLCOLOR_STATIC) {
		int ID = ::GetDlgCtrlID(hWnd);
		switch(ID) {
			case IDC_MARKER_RED:
				return ::CreateSolidBrush(RGB(0xff, 0x00, 0x00));
			case IDC_MARKER_GREEN:
				return ::CreateSolidBrush(RGB(0x00, 0xff, 0x00));
			case IDC_MARKER_MAGENTA:
				return ::CreateSolidBrush(RGB(0xff, 0x00, 0xff));
			case IDC_MARKER_BLUE:
				return ::CreateSolidBrush(RGB(0x00, 0x00, 0xff));
			case IDC_MARKER_PURPLE:
				return ::CreateSolidBrush(RGB(0x80, 0x00, 0x80));
		}
	}
	return super::OnCtlColor(hWnd, hDC, ctlType);
}