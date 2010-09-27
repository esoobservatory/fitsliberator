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
#include "PreviewView.h"

using namespace FitsLiberator::Modelling;

PreviewView::PreviewView(
    PreviewModel& m, ToolModel& tmodel, GlobalSettingsModel& gmodel, PlaneModel& pmodel, ProgressModel& prgModel,
    PreviewController& c, FlowController& fc)
  : model(m), toolModel( tmodel ), globalSettingsModel( gmodel ), planeModel(pmodel),  progressModel( prgModel ),
    controller(c), flowController( fc )
{
	this->model.Attach( this );
	this->toolModel.Attach( this );
	this->globalSettingsModel.Attach( this );
	this->planeModel.Attach( this );
    this->progressModel.Attach( this );
}

PreviewView::~PreviewView() {
	model.Detach( this );
	toolModel.Detach( this );
	globalSettingsModel.Detach( this );
	planeModel.Detach( this );
    progressModel.Detach( this );
}
			
Void PreviewView::Update( Models * m )
{

	Models::iterator it;
	
	for( it = m->begin(); it != m->end(); it++ )
	{
		if( &model == *it ) {
			drawPreview();
            if(!progressModel.QueryBusy()) {
			    Int i = model.getZoomIndex();
			    setZoomCombo(i);
			    //if( i < 9 ) {
			    //	DebugStr("\pi < 9");	
			    //} else {
			    //	DebugStr("\pi >= 9");	
			    //}
            }
		} else if( &toolModel == *it ) {
            setCurrentTool( toolModel.getCurrentFunction() );
		} else if( &globalSettingsModel == *it ) {
			drawPreview();
			updateZoomComboState( globalSettingsModel.getPreviewEnabled() );
		} else if ( &planeModel == *it){
            if(!progressModel.QueryBusy()) {
			    Double* zooms  = model.getZoomFactors();
			    Int     nZooms = model.getNumberOfZoomFactors();
			    String* values = new String[ nZooms + 1 ];
    			
			    for (Int i = 0; i < nZooms; i++)
			    {
				    values[i] = formatZoomFactor(zooms[i]);
			    }

			    values[nZooms] = "Fit in preview";
			    updateZooms(values, nZooms+1);
    			
			    delete[] values;
            }
        } else if( &progressModel == *it ) {
            if( progressModel.QueryBusy() ) {
                gotBusy();
            } else {
                gotDone();
            }
        }
	}
}

String PreviewView::formatZoomFactor(Double zoomFactor) {
	zoomFactor = 100.0 * zoomFactor;
	return TextUtils::doubleToString(zoomFactor, zoomFactor >= 100.0 ? 0 : 2) + " %";
}

String PreviewView::getZoomText() const {
	if( model.getZoomFitPreviewIndex() == model.getZoomIndex() ) {
		return "Fit in preview";
	} else {
		return formatZoomFactor(model.getZoomFactor());
	}
}