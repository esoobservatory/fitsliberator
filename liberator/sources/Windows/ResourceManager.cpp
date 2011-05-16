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
#include "ResourceManager.h"
#include "DllMain.h"

using namespace FitsLiberator::Windows;

/**
 * Loads a PNG image from a resource
 * @param name Name of the image, you may use MAKEINTRESOURCE for this value.
 * @param type Resource type (as stated in the resource script).
 * @return A GDI+ bitmap on success, NULL on failure.
 */
Gdiplus::Bitmap* ResourceManager::LoadImage( LPCTSTR name, LPCTSTR type ) {
    HRSRC hResource = ::FindResource(g_hInstance, name, type);
    if (!hResource)
        return NULL;
    
    DWORD imageSize = ::SizeofResource(g_hInstance, hResource);
    if (!imageSize)
        return NULL;

    const Void* pResourceData = ::LockResource(::LoadResource(g_hInstance, hResource));
    if (!pResourceData)
        return NULL;

    Gdiplus::Bitmap* bitmap = NULL;
    HGLOBAL hBUffer = ::GlobalAlloc(GMEM_MOVEABLE, imageSize);
    if (hBUffer) {
        Void* pBuffer = ::GlobalLock(hBUffer);
        if (pBuffer) {
            CopyMemory(pBuffer, pResourceData, imageSize);

            IStream* pStream = NULL;
            if (::CreateStreamOnHGlobal(hBUffer, FALSE, &pStream) == S_OK) {
                bitmap = Gdiplus::Bitmap::FromStream(pStream);
                pStream->Release();
            }
            ::GlobalUnlock(hBUffer);
        }
        ::GlobalFree(hBUffer);
    }
    return bitmap;
}

/**
 * @param id Resource ID.
 * @param type Resource type.
 */
Gdiplus::Bitmap* ResourceManager::LoadImage( UInt id, LPCTSTR type ) {
    return LoadImage( MAKEINTRESOURCE( id ), type );
}

/**
 * @param id Resource ID, the resource type defaults to "PNG".
 */
Gdiplus::Bitmap* ResourceManager::LoadImage( UInt id ) {
    return LoadImage( id, "PNG" );
}

/**
 * Loads a text file from the resources.
 * @param name Name of the resource, you may use MAKEINTRESOURCE for this value.
 * @param type Resource type (as stated in the resource script).
 * @return If this method fails, the string will have zero length.
 */
String ResourceManager::LoadText( LPCTSTR name, LPCTSTR type ) {
    String text;

    HRSRC hResource = ::FindResource( g_hInstance, name, type );
    if( !hResource )
        return text;

    DWORD size = ::SizeofResource( g_hInstance, hResource );
    if( !size )
        return text;

    const Void* pResourceData = ::LockResource( ::LoadResource( g_hInstance, hResource ) );
    if( !pResourceData )
        return text;

    text.assign( (const Char*)pResourceData );
    return text;
}

/**
 * Loads a string from the resources.
 * @param id String ID.
 * @return If this method fails, the string will have zero length.
 */
String ResourceManager::LoadString( UInt id ) {
    Char buffer[512];
    String text;

    ::LoadStringA(g_hInstance, id, buffer, 512);
    text = buffer;

    return text;
}

/**
 * Loads an icon from the resources.
 * @param name Name of the icon, you may use MAKEINTRESOURCE for this value.
 * @return An icon handle or NULL on failure.
 */
HICON ResourceManager::LoadIcon( LPCTSTR name ) {
    return (HICON) ::LoadImageA( g_hInstance, name, IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR );
}

/**
 * @param id ID of the icon.
 */
HICON ResourceManager::LoadIcon( UInt id ) {
    return LoadIcon( MAKEINTRESOURCE( id ) );
}