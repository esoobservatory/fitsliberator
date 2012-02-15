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

#include "Environment.h"
#include "ModelFramework.h"


using namespace FitsLiberator::Modelling;
using namespace FitsLiberator::Preferences;
using namespace FitsLiberator;


/**
 *
 */
ModelFramework::ModelFramework(
    AccumulatingChangeManager* cm,
    FitsLiberator::FitsSession* session,
	const String& inputFile )

    : changeManager(cm),
      session(session),      
      doGuess( true ),
	  planeModel( NULL ), planeController( NULL ),
	  globalSettingsModel( NULL ), globalSettingsController( NULL ),
	  previewModel( NULL ), previewController( NULL ),
	  stretchModel( NULL), stretchController( NULL ),
	  toolModel( NULL ), toolController( NULL ),
	  pixelValueModel( NULL ),
	  statisticsModel( NULL ), statisticsController( NULL ),
	  histogramModel( NULL ), histogramController( NULL ),
	  previewListener( NULL ),
	  headerModel( NULL ),
	  repositoryModel( NULL ), repositoryController( NULL ),
	  taxonomyEditorModel( NULL ), taxonomyEditorController( NULL ),
	  flowController( NULL ),
	  optionsModel( NULL ), optionsController( NULL ),
	  progressModel( NULL ), mainModel( NULL )
{	
	
	this->tileControl = NULL;
	this->reader = NULL;
	//first try to open the possible supplied user input file
	try
	{	
		if ( !(this->loadFile( inputFile )) )
			throw ImageReaderException("Could not read filename or user pressed cancel");
	}
	catch (ImageReaderException iEx )
	{
		throw iEx;
	}
	//in case the file was loaded properly then continue the initialization of the framework	
    this->initializeFramework();
}

Void ModelFramework::initializeFramework()
{
 //
    // First we create the models
	planeModel          = new PlaneModel( changeManager, reader );
	globalSettingsModel = new GlobalSettingsModel( changeManager );
	previewModel		= new PreviewModel( changeManager, *globalSettingsModel );
	stretchModel		= new StretchModel( changeManager );
	toolModel			= new ToolModel( changeManager );
	pixelValueModel		= new PixelValueModel( changeManager );
	statisticsModel 	= new StatisticsModel( changeManager );
	histogramModel		= new HistogramModel( changeManager );
	repositoryModel     = new RepositoryModel( changeManager );
	taxonomyEditorModel = new TaxonomyEditorModel( Environment::readResource( "CategoryDefinition.xml" ), changeManager );
	optionsModel        = new OptionsModel( changeManager );
	progressModel       = new ProgressModel( changeManager );
	mainModel			= new MainModel( reader, changeManager );
	headerModel			= new HeaderModel( changeManager, reader );
    
    

    //
    // Then we attach the controllers.
	globalSettingsController	= new GlobalSettingsController( *globalSettingsModel, changeManager );
	toolController				= new ToolController( *toolModel, changeManager );
	histogramController			= new HistogramController( *histogramModel, changeManager );
    statisticsController		= new StatisticsController( *statisticsModel );
	stretchController			= new StretchController( *stretchModel );
	previewController			= new PreviewController( *previewModel, *globalSettingsModel );   
	planeController             = new PlaneController( *planeModel, *statisticsController, *histogramController, *previewController, changeManager );
	repositoryController        = new RepositoryController( *repositoryModel, changeManager, reader );
	taxonomyEditorController    = new TaxonomyEditorController( *taxonomyEditorModel, changeManager );
	previewListener				= new PreviewListener( *histogramModel, *planeModel, *previewModel, *globalSettingsModel, *previewController );
	
	flowController				= new FlowController( reader, 
                                        *histogramModel, *stretchModel, 
										*statisticsModel, *previewModel, *planeModel, *pixelValueModel,
										*globalSettingsModel, *optionsModel, *progressModel, *repositoryModel,
										*histogramController, *stretchController, *statisticsController,
										*previewController, *planeController, *tileControl, 
										*repositoryController, changeManager, *session );
	
	optionsController           = new OptionsController(*optionsModel, changeManager);

	//
    // Then we update them with information from the preferences
    
    flowController->applyPreferences();
	updateModels();
    flowController->loadMetadata();
}





/**
 *
 */
Void ModelFramework::updateViews() {
	
	headerModel->Notify();
	stretchModel->Notify();
	planeModel->Notify();
	histogramModel->Notify();
	toolModel->Notify();
	globalSettingsModel->Notify();
	mainModel->Notify();
	
	

	flowController->imageChanged( planeModel->getPlane().imageIndex, 
		planeModel->getPlane().planeIndex, true );
	
}

/**
 *
 */
Void ModelFramework::saveStateToSession( Bool isOkPressed ) {
	
	//hack implemented so the routine only has to be at one spot (in this
	//case in the flow controller since that also needs the session)
	flowController->makeSession( session );    
}


	
void
ModelFramework::updateModels() {
    //
    // Apply the settings

    String      file = reader->FileName();

	FitsLiberator::Preferences::Preferences& prefs = flowController->getPreferences();

    if(prefs.getFreezeSetting()) {
        // The user asked that we use the freezed settings to the this image
        applySession(prefs.getFreezeSession());
    } else if(prefs.getSession(file, session)) {
        // The user has previously opened this file, so use the previous settings
        applySession(*session);
    } else {
        // Apply the default settings
        //histogramModel->setSliders( histogramModel->getBlackLevel(), histogramModel->getWhiteLevel() );		
		//applySession( *session );
		applyDefaultPrefs( prefs );
    }

    //
    // 

	headerModel->Notify();
	stretchModel->Notify();
	planeModel->Notify();
	toolModel->Notify();
	globalSettingsModel->Notify();
	repositoryModel->Notify();
	mainModel->Notify();
}

void
ModelFramework::applySession(const FitsSession& session) {
	planeModel->updateModel( session.plane.imageIndex, session.plane.planeIndex );
	planeModel->setImportBitDepth( session.importSettings.channelSettings );
	planeModel->setImportUndefinedOption( session.importSettings.undefinedSettings );
	planeModel->setFlipped( session.flip.flipped );

	histogramModel->updateLevels( session.stretch.blackLevel, session.stretch.whiteLevel);
    histogramModel->setSliders( histogramModel->getBlackLevel(), histogramModel->getWhiteLevel() );

	stretchModel->setRescaleFactor( session.stretch.scalePeakLevel );
	stretchModel->setPeakLevel( session.stretch.peakLevel );
	stretchModel->setBackground( session.stretch.offset );
	stretchModel->setFunction( session.stretch.function );
	stretchModel->setScale( session.stretch.scale );
	globalSettingsModel->setSessionLoaded( true );
}

Void ModelFramework::applyDefaultPrefs( const FitsLiberator::Preferences::Preferences& prefs )
{
	stretchModel->setDefaultPrefs( prefs );
	
	planeModel->setFlipped( session->flip.flipped );
	
	histogramModel->setSliders( histogramModel->getBlackLevel(), histogramModel->getWhiteLevel() );
}

/**
* When the user hits the "Save file" button
*/
Void ModelFramework::saveFile( String fileName )
{
	//called with false since this call
	//is not expected to invoke the default editor
	//on the system
	flowController->saveFile( false, fileName );
	flowController->storePreferences( true );
}

/**
*  When the user hits the "Save and edit" button
*  first save the file as tiff
*  then open the default editor with the newly saved file.
*/
Void ModelFramework::saveEdit( String fileName )
{
	//first the file should be saved
	//true is passed since we wish to invoke the default editor
	flowController->saveFile( true, fileName );
	flowController->storePreferences( true );
	
}

/**
When the user wants to open a new file a few changes to the model frame work
are needed.
*/
Void ModelFramework::openNewFile( String fileName )
{
	this->saveStateToSession( true );
	//open the dialog and get a file	
	if ( loadFile( fileName ) )
	{				
		//update models that have a references to reader
		headerModel->updateReader( reader );
		planeModel->updateReader( reader );
		repositoryController->updateReader( reader );
		flowController->updateReader( reader );
		mainModel->updateReader ( reader );
		repositoryModel->flushValues();		
		updateModels();
		updateViews();
		
		//flowController->SendNotifications();
	}
}


/**
In case the user, e.g., double clicks on a file
the functionality of a passed file path should be
available.
@param file. Reference to a string containing the 
file path.
*/
Bool ModelFramework::loadFile( String fileName )
{
	//try to open the file or shut down FL
	try
	{
		if ( fileName != "" )
		{			
			if ( reader != NULL )
			{			
				//make sure the preferences from the previous file are stored
				flowController->storePreferences( true );
				//since a new file is loaded we cannot guarantee the session settings
				globalSettingsModel->setSessionLoaded( false );
				//delete the current reader if any
				delete reader;
				reader = NULL;
			}
			// Create a file reader and determine if the file contains
			// any loadable images.
			reader = ImageReader::FromFile(fileName);
			if ( reader == NULL )
				return false;
		}
		else
			return false;
    }
	catch( Exception e )
	{
		throw e;
	}

	const ImageCube* cube = (*reader)[0];

	UInt64 maxMem = Environment::getMaxMemory();
	if ( tileControl == NULL )
		tileControl = new TileControl( (UInt)maxMem );


	return true;
}




ModelFramework::~ModelFramework()
{

	if ( previewListener != NULL )
		delete previewListener;
	if ( planeController != NULL )
		delete planeController;
	if ( stretchController != NULL )
		delete stretchController;
	if ( statisticsController != NULL )
		delete statisticsController;
	if ( histogramController != NULL )
		delete histogramController;	
	if (toolController != NULL )
		delete toolController;
	if ( repositoryController != NULL )
		delete repositoryController;
	if ( taxonomyEditorController != NULL )
		delete taxonomyEditorController;
	if ( flowController != NULL )
		delete flowController;
	if ( optionsController != NULL )
		delete optionsController;
	if ( globalSettingsController != NULL )
		delete globalSettingsController;
	if ( previewController != NULL )
		delete previewController;
	if ( tileControl != NULL )
		delete tileControl;

	if ( globalSettingsModel != NULL )
		delete globalSettingsModel;    
	if ( planeModel != NULL )
		delete planeModel;
	if ( histogramModel != NULL )
		delete histogramModel;
	if ( statisticsModel != NULL )
		delete statisticsModel;
	if ( pixelValueModel != NULL )		
		delete pixelValueModel;
	if ( toolModel != NULL )
		delete toolModel;
	if ( stretchModel != NULL )
		delete stretchModel;
	if ( repositoryModel != NULL )
		delete repositoryModel;
	if ( headerModel != NULL )
		delete headerModel;	
	if ( taxonomyEditorModel != NULL )
		delete taxonomyEditorModel;
	if ( optionsModel != NULL )
		delete optionsModel;
	if ( progressModel != NULL )
		delete progressModel;
	if ( previewModel != NULL )
		delete previewModel;
	if ( reader != NULL )
		delete reader;
	if ( mainModel != NULL )
		delete mainModel;


}

String ModelFramework::getFileName()
{
	if ( reader != NULL )
		return reader->FileName();
	return "";
}
