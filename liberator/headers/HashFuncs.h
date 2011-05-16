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
 
#ifndef __HASH_FUNCS_H__
#define __HASH_FUNCS_H__

#ifdef __GNUC__
#include <ext/hash_map>
#include "Types.h"
#include "Observer.h"

namespace std {
	template<> struct equal_to<String> {
		 bool operator()( String s1, String s2 ) const { return ( s1 == s2 ); }
	};
}

/** 
 * STL only provides hash functions for a simple couple of types, thus we need to create for all
 * other types we want to use as a key in a hash_map. Putting them in the namespace __gnu_cxx
 * means that we don't have to specify them when defining a new hash_map. Same goes with equal_to 
 * above, thus the namespace should be std.
 */
namespace __gnu_cxx {
	/** 
	 * Hash function for Strings. Used hash function for const char* as example. 
	 */
	template<> struct hash<String> {
		size_t operator()( const String& x ) const { return __gnu_cxx::__stl_hash_string( x.c_str() ); }
	};
	
	/** 
	 * Hash function for Model class pointers. Note all pointers can use their value. Just need to be 
	 * cast to size_t as this one. 
	 */
	template<> struct hash<FitsLiberator::Modelling::Model*> {
		size_t operator()( FitsLiberator::Modelling::Model* const& x ) const { return size_t( x ); }
	};
	
	/** 
	 * Hash function for Observer class pointers. Note all pointers can use their value. Just need to be 
	 * cast to size_t as this one. 
	 */
	template<> struct hash<FitsLiberator::Modelling::Observer*> {
		size_t operator()( FitsLiberator::Modelling::Observer* const& x ) const { return size_t( x ); }
	};
}
#endif // __GNUC__

#endif // __HASH_FUNCS_H__