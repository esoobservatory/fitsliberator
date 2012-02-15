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
#include "FileLoader.h"
#include "Environment.h"
#include "FitsMath.h"
#include "FitsEngine.h"
#include "TilePusher.h"


using namespace FitsLiberator;
using namespace FitsLiberator::Engine;


FileLoaderException::FileLoaderException( const String message)
: message(message)
{
	
}


FileLoader::FileLoader( TileControl& tCtrl, const ImageReader& r,
					   FitsSession* sess, String fileName )
					   : tileControl(tCtrl), reader(r)
{
	this->session = sess;
    this->fileName = fileName;
    this->use32bitCoordinates = true;
}

/**
 * Called by FitsMainProg::ReadStart, sets up the loading
 */
Void FileLoader::ReadStart( ) {
    

    //
    // Setup the image size
	const ImageCube* cube = reader[session->plane.imageIndex];


    //
    // Setup the channels
    switch( session->importSettings.channelSettings ) {
        case channel32:
            // CS2: All values should be scaled to fit in the range [0.0;1.0]
            session->stretch.outputMax = 1.0;
            
            break;
        case channel16:
            session->stretch.outputMax = 0xFFFF;        // 2^16-1
            break;
        case channel8:
            session->stretch.outputMax = 0xFF;          // 2^8-1
            break;
    }

}

/**
 * Called by FitsMainProg::ReadContinue, does the actual loading
 * @param progModel the progressmodel which is used to do the GUI "call back" for when
 * saving a large file thus making it necessary to show the progress thingy in the GUI
 */
Void FileLoader::ReadContinue( FitsLiberator::Modelling::ProgressModel& progModel ) {
	//do this to make sure we use the most efficient way of loading into PS
	
    
	switch( session->importSettings.channelSettings ) {
        case channel32:
            // teis: 32-bit alpha channel is not supported
            //if( undefinedTransparent== session->importSettings.undefinedSettings )
            //    readTransparent<Float>( 1.0 );
            //else
                readBlack<Float>( 1.0, 32, progModel );
            break;
        case channel16:
            if( undefinedTransparent== session->importSettings.undefinedSettings )
			{
				/*
				The way to include transparency is through an alpha channel.
				In principle TIFF does support 1bit masks but this seems to be not working
				or perhaps PS does not recognize it.				
				*/
				session->stretch.outputMax = 0xFFFF;
				readTransparent<UShort>( 0xFFFF, 16, progModel );
			}
            else
                readBlack<UShort>( 0xFFFF, 16, progModel );
            break;
        case channel8:
            if( undefinedTransparent== session->importSettings.undefinedSettings )
			{
                session->stretch.outputMax = 0xFF;
				readTransparent<Byte>( 0xFF, 8, progModel );
			}
            else
				readBlack<Byte>( 0xFF, 8, progModel );
            break;
    }
}


/**
 * Reads the image with undefined values set to transparent.
 * @param maxValue the upper clipping value (zero is always the lower)
 * @param bitDepth the export bit depth
 * @param progModel the progressModel to keep track of the progress.
 */
template<typename O>
Void FileLoader::readTransparent( O maxValue, Short bitDepth,
								 FitsLiberator::Modelling::ProgressModel& progModel )
{
	
	//make sure to suppress errors. We handle that directly
	//based on the return values of the individual calls
	TIFFSetErrorHandler( NULL );

	 //Make a tiff image pointer
	TIFF* outImage = NULL;
	//open it
	outImage = TIFFOpen( fileName.c_str(),"w");
    
	//if NULL is returned something went wrong...
	if ( outImage == NULL )	
		throw FileLoaderException("Could not open the file");		
	
	
    // Load the pixels one tile at a time.
	//When exporting the tiles have the width of the image
	//In this way they are compliant with the tiff strips

	//get the image cube
	const ImageCube* cube = reader[session->plane.imageIndex];
	
	//Double* pixelBuffer = NULL;

	
	
	//get the number of tiles	
	Int nTiles = tileControl.getNumberOfTiles();
	//the number of rows in each strip
	Int rowsPerStrip = tileControl.getLightTile(0)->getBounds().getHeight();
	//the number of pixels in a single strip
	Int pixelsPerStrip = cube->Width() * rowsPerStrip;
	//the number of bytes in a single strip
	Int bytesPerStrip = sizeof(O) * pixelsPerStrip;
	//the number of strips in the output image
	Int stripsPerImage = ::floor( ((Double)cube->Height() + (Double)rowsPerStrip - 1.) / (Double)rowsPerStrip );
	
		
	TIFFSetField( outImage, TIFFTAG_SAMPLESPERPIXEL, 2 );
	if ( !(TIFFSetField( outImage, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT )))
		throw FileLoaderException("Could not set field SAMPLEFORMAT");

	//setup the TIFF fields
	//bits per pixel
	if ( !(TIFFSetField( outImage, TIFFTAG_BITSPERSAMPLE, bitDepth )) )
		throw FileLoaderException("Could not set field BITSPERSAMPLE");
	//set what zero corresponds to (here it should be black)
	if ( !(TIFFSetField( outImage, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK )) )
		throw FileLoaderException("Could not set field PHOTOMETRIC");
	//the width of the image
	if ( !(TIFFSetField( outImage, TIFFTAG_IMAGEWIDTH, cube->Width() )) )
		throw FileLoaderException("Could not set field IMAGEWIDTH");
	//the total height of the image
	if ( !(TIFFSetField( outImage, TIFFTAG_IMAGELENGTH, cube->Height() )))
		throw FileLoaderException("Could not set field IMAGELENGTH");
	//the number of rows in each strip
	if ( !(TIFFSetField( outImage, TIFFTAG_ROWSPERSTRIP, rowsPerStrip )))
		throw FileLoaderException("Could not set field ROWSPERSTRIP");
	//the physical resolution unit of the image; set to nothing
	if ( !(TIFFSetField( outImage, TIFFTAG_RESOLUTIONUNIT, RESUNIT_NONE )))
		throw FileLoaderException("Could not set field RESOLUTIONUNIT");
	//set the planer configuration (contiguous)
	//set the planer configuration (contiguous)
	if ( !(TIFFSetField( outImage, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG )))
		throw FileLoaderException("Could not set field PLANARCONFIG");;
	if ( !(TIFFSetField( outImage, TIFFTAG_SOFTWARE, "The ESA/ESO/NASA FITS Liberator" )))
		throw FileLoaderException("Could not set field SOFTWARE");
	//for now we use the associateAlpha method for transparency.
	//it is the simplest but does also take a lot of disk space
	uint16 out[1];
	out[0] = EXTRASAMPLE_ASSOCALPHA;

	if ( !(TIFFSetField( outImage, TIFFTAG_EXTRASAMPLES, 1, &out ))) 
		throw FileLoaderException("Could not set field EXTRASAMPLES");
	
	//write the metadata to the file
	if ( !(TIFFSetField( outImage, TIFFTAG_XMLPACKET, session->metaData.length(), session->metaData.c_str() )))
		throw FileLoaderException("Could not set field XMLPACKET");

//	TIFFSetField( outImage, TIFFTAG_COMPRESSION, COMPRESSION_LZW );

	//if flipped, tiff supports orientation of the image
	//default corresponds to non-flipped so we only do something
	//if the image should be flipped
	if ( session->flip.flipped )
	{
		if ( !(TIFFSetField( outImage, TIFFTAG_ORIENTATION, ORIENTATION_BOTLEFT )))
			throw FileLoaderException("Could not set field ORIENTATION");
	}

	
	

	//the output buffer written to the tiff file
	O* rowBuffer = new O[pixelsPerStrip*2];
	
	progModel.SetIncrement( nTiles );
	//Loop through each tile
	for ( Int i = 0; i < nTiles; i++ )
	{
		//get the i'th tile
		ImageTile* tile = tileControl.getTile( i, cube, session->plane, false );
		tileControl.stretchTile_par( *tile, session->stretch, cube );
		const Rectangle& bounds = tile->getBounds();
		//get the number of pixels in this tile
		Int bufSize = bounds.getArea();
		
		//allocate the resulting pixel buffer
		//pixelBuffer = new Double[bufSize];
		
		//copy pixels to the buffer		
		//for ( Int j = 0; j < bufSize; j++ )		
		//	pixelBuffer[j] = tile->stretchedPixels[j];
				
		//scale the pixels
		//FitsEngine::scale_par( session->stretch, pixelBuffer, bufSize, tileControl.getNumberOfThreads() );
		FitsEngine::scale_par( session->stretch, tile->stretchedPixels, bufSize, tileControl.getNumberOfThreads() );
	
		      

		// Write the scaled pixels to the output buffer	
		for ( Int j = 0; j < bounds.getWidth(); j++ )
		{
			for ( Int k = 0; k < rowsPerStrip; k++ )			
			{	
				UInt ind = 2*(bounds.getWidth()*k+j);
				Double pixel = tile->stretchedPixels[bounds.getWidth()*k+j]; //pixelBuffer[bounds.getWidth()*k+j];
				
				if(  pixel != FitsMath::NaN && FitsMath::isFinite( pixel ) )
				{
	                if( pixel > maxValue )
		                pixel = maxValue;
			        if( pixel < 0 )
				        pixel = 0;
					rowBuffer[ind+1] = maxValue;
				}
				else
				{
	                pixel = 0;		        
					rowBuffer[ind+1] = 0;
				}
				
				rowBuffer[ind] = (O)pixel;
				
			}
		}
		
        //write to the file
		
		if ( !(TIFFWriteEncodedStrip( outImage, i, rowBuffer, 2*bytesPerStrip )) )
			throw FileLoaderException("Could not write encoded strip");
		
		//call the progress thingy
		progModel.Increment();
		//if the current tile is the last one it may extend two strips of the output image
		if ( i == nTiles - 1 && stripsPerImage > nTiles )
		{
			delete[] rowBuffer;

			Int pixelsRemaining = bounds.getWidth()*(bounds.getHeight()-rowsPerStrip);
			rowBuffer = new O[pixelsRemaining*2];
			//loop through the line
			for ( Int j = 0; j < bounds.getWidth(); j++ )
			{	
				//loop through the remaining of the last tile
				for ( Int k = rowsPerStrip; k < bounds.getHeight(); k++ )
				{
					UInt ind = 2*(bounds.getWidth()*(k-rowsPerStrip)+j);
					Double pixel = tile->stretchedPixels[bounds.getWidth()*k+j];//pixelBuffer[bounds.getWidth()*k+j];
					if(  pixel != FitsMath::NaN && FitsMath::isFinite( pixel ) )
					{
						if( pixel > maxValue )
							pixel = maxValue;
						if( pixel < 0 )
							pixel = 0;
						rowBuffer[ind+1] = maxValue;
					}
					else		
					{
						pixel = 0;		        
						rowBuffer[ind+1] = 0;
					}					

					rowBuffer[ind] = (O)pixel;
					
				}
			}
			//write to the file
			
			if ( !(TIFFWriteEncodedStrip( outImage, stripsPerImage-1, rowBuffer,sizeof(O)*pixelsRemaining*2)))
				throw FileLoaderException("Could not write encoded strip");
			progModel.Increment();
		}


		//clean up
		//delete[] pixelBuffer;
		//pixelBuffer = NULL;
		
	}	
	delete[] rowBuffer;
		//finally, close, the tiff file
	TIFFClose(outImage);

}


/**
 * Reads the image with undefined values set to black.
 * @param maxValue the upper clipping value (zero is always the lower)
 * @param bitDepth the export bit depth
 * @param progModel the progressModel to keep track of the progress.
 */
template<typename O>
Void FileLoader::readBlack( O maxValue, Short bitDepth,FitsLiberator::Modelling::ProgressModel& progModel )
{
    //make sure to suppress errors. We handle that directly
	//based on the return values of the individual calls
	TIFFSetErrorHandler( NULL );

	 //Make a tiff image pointer
	TIFF* outImage = NULL;
	//open it
	outImage = TIFFOpen( fileName.c_str(),"w");
    //if NULL is returned something went wrong...
	if ( outImage == NULL )
	{
		throw FileLoaderException("Could not open the file");
		return;
	}
	
	// Load the pixels one tile at a time.
	//When exporting the tiles have the width of the image
	//In this way they are compliant with the tiff strips

	//get the image cube
	const ImageCube* cube = reader[session->plane.imageIndex];
	
	//Double* pixelBuffer = NULL;
	
	//get the number of tiles	
	Int nTiles = tileControl.getNumberOfTiles();
	//the number of rows in each strip
	Int rowsPerStrip = tileControl.getLightTile(0)->getBounds().getHeight();
	//the number of pixels in a single strip
	Int pixelsPerStrip = cube->Width() * rowsPerStrip;
	//the number of bytes in a single strip
	Int bytesPerStrip = sizeof(O) * pixelsPerStrip;
	//the number of strips in the output image
	Int stripsPerImage = ::floor( ((Double)cube->Height() + (Double)rowsPerStrip - 1.) / (Double)rowsPerStrip );
	
	TIFFSetField( outImage, TIFFTAG_SAMPLESPERPIXEL, 1 );

	//setup the TIFF fields
	//bits per pixel
	if ( !(TIFFSetField( outImage, TIFFTAG_BITSPERSAMPLE, bitDepth )) )
		throw FileLoaderException("Could not set field BITSPERSAMPLE");
	//set what zero corresponds to (here it should be black)
	if ( !(TIFFSetField( outImage, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK )) )
		throw FileLoaderException("Could not set field PHOTOMETRIC");
	//the width of the image
	if ( !(TIFFSetField( outImage, TIFFTAG_IMAGEWIDTH, cube->Width() )) )
		throw FileLoaderException("Could not set field IMAGEWIDTH");
	//the total height of the image
	if ( !(TIFFSetField( outImage, TIFFTAG_IMAGELENGTH, cube->Height() )))
		throw FileLoaderException("Could not set field IMAGELENGTH");
	//the number of rows in each strip
	if ( !(TIFFSetField( outImage, TIFFTAG_ROWSPERSTRIP, rowsPerStrip )))
		throw FileLoaderException("Could not set field ROWSPERSTRIP");
	//the physical resolution unit of the image; set to nothing
	if ( !(TIFFSetField( outImage, TIFFTAG_RESOLUTIONUNIT, RESUNIT_NONE )))
		throw FileLoaderException("Could not set field RESOLUTIONUNIT");

	//set the planer configuration (contiguous)
	if ( !(TIFFSetField( outImage, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG )))
		throw FileLoaderException("Could not set field PLANARCONFIG");;
	if ( !(TIFFSetField( outImage, TIFFTAG_SOFTWARE, "The ESA/ESO/NASA FITS Liberator" )))
		throw FileLoaderException("Could not set field SOFTWARE");

	TIFFSetField( outImage, TIFFTAG_SUBFILETYPE, 0 );
	
	//this should only be set when the bitdepth is 32 bit since PS
	//wants the data as float in that case. The scaling of the
	//image on the interval [0:1] is already taken care of through maxValue
	if ( bitDepth == 32 )
	{
		if ( !(TIFFSetField( outImage, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP )))
			throw FileLoaderException("Could not set field SAMPLEFORMAT");
	}
	else
	{
		if ( !(TIFFSetField( outImage, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT )))
			throw FileLoaderException("Could not set field SAMPLEFORMAT");
	}
	//if flipped, tiff supports orientation of the image
	//default corresponds to non-flipped so we only do something
	//if the image should be flipped
	if ( session->flip.flipped )
	{
		if ( !(TIFFSetField( outImage, TIFFTAG_ORIENTATION, ORIENTATION_BOTLEFT )))
			throw FileLoaderException("Could not set field ORIENTATION");
	}
	//write the metadata to the file
	if ( !(TIFFSetField( outImage, TIFFTAG_XMLPACKET, session->metaData.length(), session->metaData.c_str() )))
		throw FileLoaderException("Could not set field XMLPACKET");

	//the output buffer written to the tiff file
	O* rowBuffer = new O[pixelsPerStrip];
	progModel.SetIncrement( nTiles );
	//Loop through each tile
	for ( Int i = 0; i < nTiles; i++ )
	{
		//get the i'th tile
		ImageTile* tile = tileControl.getTile( i, cube, session->plane, false );
		tileControl.stretchTile_par( *tile, session->stretch, cube );
		const Rectangle& bounds = tile->getBounds();
		//get the number of pixels in this tile
		Int bufSize = bounds.getArea();
		
		//allocate the resulting pixel buffer
		/*pixelBuffer = new Double[bufSize];
		
		//copy pixels to the buffer		
		for ( Int j = 0; j < bufSize; j++ )		
			pixelBuffer[j] = tile->stretchedPixels[j];
				
		//scale the pixels
		FitsEngine::scale_par( session->stretch, pixelBuffer, bufSize, tileControl.getNumberOfThreads() );
		*/
		FitsEngine::scale_par( session->stretch, tile->stretchedPixels, bufSize, tileControl.getNumberOfThreads() );
	
		      

		// Write the scaled pixels to the output buffer	
		for ( Int j = 0; j < bounds.getWidth(); j++ )
		{
			for ( Int k = 0; k < rowsPerStrip; k++ )			
			{				
				Double pixel = tile->stretchedPixels[bounds.getWidth()*k+j];
				//Double pixel = pixelBuffer[bounds.getWidth()*k+j];
				if(  pixel != FitsMath::NaN && FitsMath::isFinite( pixel ) )
				{
					if( pixel > maxValue )
						pixel = maxValue;
					if( pixel < 0 )
						pixel = 0;
				}
				else		
					pixel = 0;		        
				
				rowBuffer[bounds.getWidth()*k+j] = (O)pixel;
			}
		}
		
		//write to the file
		
		if ( TIFFWriteEncodedStrip( outImage, i, rowBuffer, bytesPerStrip ) == -1 )
			throw FileLoaderException("Could not write encoded strip");
		progModel.Increment();
		//if the current tile is the last one it may extend two strips of the output image
		if ( i == nTiles - 1 && stripsPerImage > nTiles )
		{
			delete[] rowBuffer;

			Int pixelsRemaining = bounds.getWidth()*(bounds.getHeight()-rowsPerStrip);
			if ( pixelsRemaining > 0 )
			{
				rowBuffer = new O[pixelsRemaining];
				//loop through the line
				for ( Int j = 0; j < bounds.getWidth(); j++ )
				{	
					//loop through the remaining of the last tile
					for ( Int k = rowsPerStrip; k < bounds.getHeight(); k++ )
					{
						Double pixel = tile->stretchedPixels[bounds.getWidth()*k+j];
						//Double pixel = pixelBuffer[bounds.getWidth()*k+j];
						if(  pixel != FitsMath::NaN && FitsMath::isFinite( pixel ) )
						{
							if( pixel > maxValue )
								pixel = maxValue;
							if( pixel < 0 )
								pixel = 0;
						}
						else		
							pixel = 0;		        
						
						rowBuffer[bounds.getWidth()*(k-rowsPerStrip)+j] = (O)pixel;
					}
				}
				//write to the file
				if ( TIFFWriteEncodedStrip( outImage, stripsPerImage-1, rowBuffer,sizeof(O)*pixelsRemaining) == -1 )
					throw FileLoaderException("Could not write encoded strip");
			}
			progModel.Increment();
		}


		//clean up
//		delete[] pixelBuffer;
//		pixelBuffer = NULL;
		
	}	
	delete[] rowBuffer;
		//finally, close, the tiff file
	TIFFClose(outImage);	
    
}
