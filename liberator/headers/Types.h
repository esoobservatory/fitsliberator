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

#ifndef __TYPES_H__
#define __TYPES_H__

#ifdef WINDOWS
    #define NULL 0
#endif

const class {
        void operator&() const;
    public:
        template<class T> operator T*() const { return 0; }
        template<class C, class T> operator T C::*() const { return 0; }
} nullptr = {};

#include <string>
#include <vector>


// 
// Fundamental types
#if defined(WINDOWS)
    typedef __int8              Int8;
    typedef __int16             Int16;
    typedef __int32             Int32;
    typedef __int64             Int64;
    typedef unsigned __int8     UInt8;
    typedef unsigned __int16    UInt16;
    typedef unsigned __int32    UInt32;
    typedef unsigned __int64    UInt64;
#elif defined(__GNUC__)
    typedef signed char         Int8;
    typedef signed short        Int16;
    typedef signed int          Int32;
    typedef long long           Int64;
    typedef unsigned char       UInt8;
    typedef unsigned short      UInt16;
    typedef unsigned long       UInt32;
    typedef unsigned long long  UInt64;
#endif

//
// Intrinsic types
typedef void                Void;

typedef bool                Bool;
typedef bool                Bit;

typedef void                Void;
typedef bool                Bool;

typedef UInt8               Byte;
typedef char                Char;
typedef Int16               Short;
typedef UInt16              UShort;
typedef Int32               Int;
#if defined(WINDOWS) || defined(__MAC_TERM__)
typedef UInt32              UInt;
#elif defined(__GNUC__)
typedef unsigned int		UInt;
#endif

typedef float               Float;
typedef double              Double;

//
// Utility types
typedef std::string         String;

#define Vector              std::vector

//
// Custom types
#include "Exception.h"
#include "Image.h"
#include "Stretch.h"

#endif
