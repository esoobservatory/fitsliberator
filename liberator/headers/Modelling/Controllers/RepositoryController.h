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
#ifndef __REPOSITORYCONTROLLER_H__
#define __REPOSITORYCONTROLLER_H__

#include "FitsLiberator.h"
#include "AccumulatingChangeManager.h"
#include "RepositoryModel.h"
#include "ImageReader.hpp"

namespace FitsLiberator {
    namespace Modelling {
        #define kFITSKeywordTypeCount 9

        enum ValidationError {
            NoError,
			InvalidElementCount,
            InvalidReal,
            InvalidDate,
            InvalidDateTime,
            InvalidTime,
            InvalidUrlID,
            InvalidUrlResource,
            InvalidIvoID,
            InvalidIvoResource
        };

		/**
		*	Class for controlling the metadata saved
		*	in the repository
		*/
        class RepositoryController : public ACMController {
            public:
                RepositoryController( RepositoryModel&, AccumulatingChangeManager*, 
					FitsLiberator::Engine::ImageReader* );
				/** Loads a keyword with a value saved in the preferences file. This method
					may disregard the new value and will ignore attempts to set the value of a keyword
					that doesn't exist.
					@param keywordName Name of the keyword (case-sensitive).
					@param keywordValue Saved valued. */
                Void loadValue( String, String );
				/** Generates the value to be saved in the preferences.
					@param keyword Keyword to save.
					@remark To determine if a specific keyword should be saved call shouldSave. */
				String saveValue( const Keyword* ) const;
				/** Determines if a keyword should be saved to the preferences. 
					@param keyword Keyword to save.
					@return True if the keyword should be saved, false otherwise. */
                Bool shouldSave( const Keyword* ) const;
                /** Sets the value of a keyword. This method may modify the value to make it conform to
                    a specific format.
                    @param keywordIndex Index of the keyword to modify.
                    @param value New value.
                    @return A validation error code, NoError indicates success. */
                ValidationError parseValue( UInt keywordIndex, String value );
				/** Parses the headers of a given HDU and sets associated metadata keywords to values from the headers.
					@param imageIndex HDU index (0-based).
					@param shouldFlip If true the image metadata indicates that
					the image should be flipped. 
					@param isFlipped If true the image has been flipped.
					@remark If shouldFlip != isFlipped the user has manully 
					overridden the logic to flip the image. */
                Void parseHeaders( UInt imageIndex, Bool shouldFlip, Bool isFlipped );
				/** @param keywordIndex Index of the keyword to serialize. */
                String serializeValue( UInt ) const;
				/** Creates a serialized version of a keyword value.
					@param keyword Keyword to serialize. */
                String serializeValue( const Keyword* ) const;
				/** Checks whether the current update is an update to a single
					value. */
				Bool isSingleUpdate() const;
				Void updateReader( FitsLiberator::Engine::ImageReader* r );
            private:
                typedef ACMController super;

				/** Reads header values from a FitsFile object. */
				class FileHeaderSource : public HeaderSource {
					/** Image to read header information from. */
					const FitsLiberator::Engine::ImageCube* cube;
				public:
					/** Default constructor.
						@param image Image to read from. */
					FileHeaderSource( const FitsLiberator::Engine::ImageCube* image );
					String getHeader( const String& name ) const;
				};
				/** Model to operate on. */
                RepositoryModel& model;
				/** File to read from. */
                FitsLiberator::Engine::ImageReader* reader;
				/** Have we updated more than one value? */
				Bool singleUpdate;
                /** Apply the rules for pulling data from the headers. */
                void applyRules(UInt imageIndex);
                /** Apply the rules from a specific set. */
                void applyRules(const HeaderSource& source, const RuleList& rules);
                /** Apply rule. */
                void applyRule(const HeaderSource& source, const Rule* rule);
                /** Apply the WCS transformations. */
                void applyWCS(UInt imageIndex, bool shouldFlip, bool isFlipped);
        };
    }
}

#endif  // __REPOSITORYCONTROLLER_H__