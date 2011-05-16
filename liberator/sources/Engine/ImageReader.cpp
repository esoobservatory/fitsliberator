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


#include "ImageReader.hpp"
#include "FitsImageReader.hpp"
#include "PdsImageReader.hpp"

using std::string;

using FitsLiberator::Engine::ImageReaderException;

using FitsLiberator::Engine::ImageReader;
using FitsLiberator::Engine::FitsImageReader;
using FitsLiberator::Engine::PdsImageReader;

using FitsLiberator::Engine::ImageCube;

ImageReaderException::ImageReaderException(const string& filename) {
	this->filename = filename;
}

ImageReader::ImageReader(const string& filename) {
	this->filename = filename;
}

ImageReader::~ImageReader() {
	for(ImageReader::iterator i = images.begin(); i != images.end(); i++ )
		delete *i;
}

ImageReader*
ImageReader::FromFile(const string& filename) {
	ImageReader* reader = 0;

    if(FitsImageReader::CanRead(filename)) {
        try {
		    reader = new FitsImageReader(filename);
		    if( reader->size() > 0 )
			    return reader;
			else
				return 0;
	    } catch(ImageReaderException) {}
		catch(Exception) {}
    }

    if(PdsImageReader::CanRead(filename)) {
        try {
		    reader = new PdsImageReader(filename);
		    if( reader->size() > 0 )
			    return reader;
			else
				return 0;
    	} catch(ImageReaderException)
		{
			return 0;
		}
		catch(Exception)
		{
			return 0;
		}
    }

	return 0;
}

Bool ImageReader::IsSupported( const string& filename )
{
	if ( FitsImageReader::CanRead(filename) )
		return true;
	else if ( PdsImageReader::CanRead( filename ) )
		return true;

	return false;

}

ImageReader::size_type
ImageReader::size() const {
	return this->images.size();
}

bool
ImageReader::empty() const {
	return this->images.empty();
}

const string&
ImageReader::FileName() const {
	return this->filename;
}

ImageReader::value_type
ImageReader::operator[](ImageReader::size_type i) const {
	if( i < size() )
		return this->images[i];
	return 0;
}

ImageReader::const_iterator
ImageReader::begin() const {
	return this->images.begin();
}

ImageReader::const_iterator
ImageReader::end() const {
	return this->images.end();
}

void
ImageReader::insert(ImageCube* image) {
	images.insert(images.end(), image);
}
