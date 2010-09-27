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

/** @file
    Contains definitions for the wrapper layer above the 3rd party I/O library
    pds_toolbox. This file implements the class 
	Engine::PdsImageReader.

	$Log: PdsImageReader.cpp,v $
	Revision 1.8  2010/09/25 15:32:59  kaspar
	*** empty log message ***
	
	Revision 1.7  2010/09/14 13:01:54  kaspar
	*** empty log message ***
	
	Revision 1.6  2008/05/27 15:34:45  teis
	Split the metadata rules between formats
	
	Revision 1.5  2008/05/27 08:13:22  teis
	Changed the way the Header tab is populated
	
	* Added ImageReader::header() which retrieves the header
	* Made HeaderModel use the new method
	
	Revision 1.4  2008/05/12 08:39:54  teis
	* Renamed BitsPerPixel to PixelFormat.
	* Made the ImageCube::Properties take the stream as a parameter.
	* The assert macro now includes the expression that fails.
	
	Revision 1.3  2008/05/01 16:13:03  teis
	* Fixed a linking issues which caused the plug-in to crash in release mode
	
	Revision 1.2  2008/05/01 13:52:45  teis
	* Merged in the PDS support
	
	Revision 1.1.2.1  2008/05/01 13:47:10  teis
	* Added initial support for PDS images
	
	Revision 1.1.2.1  2007/10/30 22:56:44  teis
	*** empty log message ***
	
	@version       $Revision: 1.8 $
	@date          $Date: 2010/09/25 15:32:59 $

	@author        Teis Johansen <teis@siet.dk>
	@author        Kaspar Kirstein Nielsen <kaspar@barmave.dk>
	@author        Lars Holm Nielsen <lars@hankat.dk> */

#include "PdsImageReader.hpp"
#include "Text.hpp"

using std::string;

using FitsLiberator::indent;
using FitsLiberator::newline;
using FitsLiberator::Rectangle;

using FitsLiberator::Engine::ImageReader;
using FitsLiberator::Engine::ImageCube;
using FitsLiberator::Engine::PdsImageReader;
using FitsLiberator::Engine::PdsImageCube;

enum TraverseMode {
    TRAVERSE_NODE   = 0,
    TRAVERSE_BEFORE = -1,
    TRAVERSE_AFTER  = -2
};

struct PdsImageReader::Include {
    PdsImageReader* owner;
public:
    Include(PdsImageReader* owner) {
        this->owner = owner;
    }

    void operator()(OBJDESC* node, int level, TraverseMode mode) {
        if(mode == TRAVERSE_NODE) {
            if(OaGetObjectClass(node) == OA_IMAGE) {
                PdsImageCube* image = PdsImageCube::FromObjectDescription(owner, node);
                if(image != 0) {
                    owner->insert(image);
                }
            }
        }
    }
};

struct PdsImageReader::Print {
    std::ostream& stream;

    void format(int level, const std::string& key, const std::string& value) {
        // Rough formatting for PDS files:
        // NAME '= ' VALUE
        // NAME field is 34 characters
        // VALUE field is 80 - 34 - '= ' - \r\n = 42 characters

        indent(stream, level) << key;
        int field_size = 34 - level * 4 - key.size();
        while(field_size > 0) {
            stream << " ";
            field_size -= 1;
        }

        stream << "= ";
        
        if(value.size() < 42) {
            stream << value;
        }
        /*else {
            int line = 42;
            for(string::const_iterator i = value.begin(); 
                i != value.end(); ++i) {
                if(--line == 0) {
                    stream << newline;
                    line = 78;  // following lines are full lines
                }
                stream << (*i);
            }
        }*/

        stream << newline;
    }
public:
    Print(std::ostream& s) : stream(s) { }

    void operator()(OBJDESC* node, int level, TraverseMode mode) {
        switch(mode) {
            case TRAVERSE_BEFORE:
                if(node->parent != 0) {
                    format(level, "OBJECT", node->class_name);
                }
                break;
            case TRAVERSE_AFTER:
                if(node->parent != 0) {
                    format(level, "END_OBJECT", node->class_name);
                    stream << newline;
                }
                break;
            default:
	            KEYWORD* keyword = OdlGetFirstKwd(node);
	            while(keyword != 0) {
                    format(level, keyword->name, keyword->value);
		            keyword = OdlGetNextKwd(keyword);
	            }
        }
    }
};

PdsImageReader::PdsImageReader(const string& fileName) 
  : super(fileName) {

	// Parse the file and make the in-memory tree conform to the latest PDS standard
	ODLTREE node = OaParseLabelFile(const_cast<char*>(fileName.c_str()), NULL, 
		ODL_EXPAND_STRUCTURE, TRUE);
	if( NULL == node )
		throw ImageReaderException(fileName);
	root = OaConvertLabel(node);
	if( root == NULL ) {
        OdlFreeTree(node);
		node = NULL;
		throw ImageReaderException(fileName);
	}

	// Search the file for compatible images. We may not find any, but that is
	// not considered an exceptional error.
    Traverse(root, Include(this));
}

PdsImageReader::~PdsImageReader() {
	if( NULL != root ) {
        OdlFreeTree(root);
		root = NULL;
	}
}

template<typename Action> void
PdsImageReader::Traverse(OBJDESC* node, Action action, int level) {
    action(node, level, TRAVERSE_NODE);
    node = node->first_child;
    while(node != 0) {
        action(node, level, TRAVERSE_BEFORE);
        Traverse(node, action, level + 1);
        action(node, level, TRAVERSE_AFTER);
        node = node->right_sibling;
    }
}

bool
PdsImageReader::CanRead(const string& filename) {
	ODLTREE node = OaParseLabelFile(const_cast<char*>(filename.c_str()), NULL, 
		ODL_EXPAND_STRUCTURE, TRUE);
    if(node != 0) {
        // Make sure we don't leak
        OdlFreeTree(node);
        return true;
    } 
    return false;
}

void
PdsImageReader::header(std::ostream& stream) const {
    // Print is a const operation, so this is safe...
    const_cast<PdsImageReader*>(this)->Traverse(root, Print(stream));
}

ImageReader::ImageFormat
PdsImageReader::format() const {
    return ImageReader::PDS;
}