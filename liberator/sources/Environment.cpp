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
#ifdef WINDOWS
    #include <shlobj.h>
    #include <shlwapi.h>
    #include <gdiplus.h>
    #include "ResourceManager.h"
    #include "DllMain.h"

    using Gdiplus::GdiplusStartupInput;
    using Gdiplus::GdiplusStartup;
    using Gdiplus::GdiplusShutdown;
#else
	#include <sys/types.h>
	#include <sys/sysctl.h>
	#include <fstream>
    extern "C" {
        #include <Files.h>
    };
#endif

#include "Environment.h"

using namespace FitsLiberator;
using namespace FitsLiberator::Engine;

//-------------------------------------------------------------------------------------------------
// FitsLiberator::Environment
//-------------------------------------------------------------------------------------------------
#ifdef WINDOWS
    const Char Environment::newLine[]			= "\r\n";
	const Char Environment::PATH_SEPARATOR[]	= "\\";
    ULONG_PTR Environment::gdiplusToken			= NULL;
#else
    const Char Environment::newLine[]				= "\n";
	const Char Environment::PATH_SEPARATOR[]		= "/";
    Mac::BundleFactory*	Environment::bundleFactory 	= NULL;
#endif

Bool Environment::showToolTips      = true; 
Bool Environment::hostIsElements    = true;
UInt Environment::hostMajorVersion  = 0;

#ifdef DEBUG
	#ifdef WINDOWS
    std::ofstream* Environment::debugLog = NULL;
    #endif
#endif


#ifdef WINDOWS
static HWND findDialogParent( HWND child ) {
    while( ::IsWindow( child ) && !(::GetWindowLong( child, GWL_STYLE ) & (WS_POPUP | WS_OVERLAPPED) ) ) {
        child = ::GetParent( child );
    }

    return child;
}

static HWND findDialogParent() {
    return findDialogParent( ::GetForegroundWindow() );
}
#endif

/**
 * Should tool tips be shown.
 */
Bool Environment::isShowToolTips() {
	return showToolTips;
}

void
Environment::showMessage(const String& title, const String& message) {
#ifdef WINDOWS
    ::MessageBox( findDialogParent(), message.c_str(), title.c_str(), MB_OK );
#else

#endif
}

/**
*  Determines whether the specified string is a network path
*  The implementation is done on both Win and Mac
*
*  @param fileName the path to evaluate
*  @return true if the path is a network path, false if not.
*
*/
bool Environment::isNetworkPath(const String& fileName) {
	#ifdef WINDOWS
		if(fileName.size() > 3) {
			if((fileName[0] == '\\') && (fileName[1] == '\\')) {
				return true;
			} else {
				String root = fileName.substr(0, 3);
				UINT res = ::GetDriveType(root.c_str());
				return (res == DRIVE_REMOTE);
			}
		}
		return false;	// We must return some value...
	#else
        if(fileName.find("/Volumes") == 0) {
            OSStatus			result = noErr;
            ItemCount			volumeIndex;
            
            // Iterate across all mounted volumes using FSGetVolumeInfo. This will return nsvErr
            // (no such volume) when volumeIndex becomes greater than the number of mounted volumes.
            for (volumeIndex = 1; result == noErr || result != nsvErr; volumeIndex++) {
                FSVolumeRefNum	actualVolume;
                HFSUniStr255	volumeName;
                FSVolumeInfo	volumeInfo;
                
                bzero((void *) &volumeInfo, sizeof(volumeInfo));
                
                // We're mostly interested in the volume reference number (actualVolume)
                result = FSGetVolumeInfo(kFSInvalidVolumeRefNum,
                                         volumeIndex,
                                         &actualVolume,
                                         kFSVolInfoFSInfo,
                                         &volumeInfo,
                                         &volumeName,
                                         NULL); 
                
                if (result == noErr) {
                    GetVolParmsInfoBuffer volumeParms;
                    HParamBlockRec pb;
                    
                    // Use the volume reference number to retrieve the volume parameters. See the documentation
                    // on PBHGetVolParmsSync for other possible ways to specify a volume.
                    pb.ioParam.ioNamePtr = NULL;
                    pb.ioParam.ioVRefNum = actualVolume;
                    pb.ioParam.ioBuffer = (Ptr) &volumeParms;
                    pb.ioParam.ioReqCount = sizeof(volumeParms);
                    
                    // A version 4 GetVolParmsInfoBuffer contains the BSD node name in the vMDeviceID field.
                    // It is actually a char * value. This is mentioned in the header CoreServices/CarbonCore/Files.h.
                    if(noErr == PBHGetVolParmsSync(&pb)) {
                        // The volume is local if vMServerAdr is 0. Network volumes won't have a BSD node name.
                        
                        // The following code is just to convert the volume name from a HFSUniCharStr to
                        // a plain C string so we can print it with fprintf. It'd be preferable to
                        // use CoreFoundation to work with the volume name in its Unicode form.
                        CFStringRef	volNameAsCFString;
                        char		volNameAsCString[256];
                        
                        volNameAsCFString = CFStringCreateWithCharacters(kCFAllocatorDefault,
                                                                         volumeName.unicode,
                                                                         volumeName.length);
                        
                        // If the conversion to a C string fails, just treat it as a null string.
                        if (!CFStringGetCString(volNameAsCFString,
                                                volNameAsCString,
                                                sizeof(volNameAsCString),
                                                kCFStringEncodingUTF8)) {
                            volNameAsCString[0] = 0;
                        }
                        
                        CFRelease(volNameAsCFString);
                        
                        if(fileName.find(volNameAsCString) == 9) { // So we get /Volumes/<volNameAsCString>
                            if(volumeParms.vMServerAdr != 0) {
                                return true;
                            } else {
                                return false;
                            }
                        }
                    }
                }
            }            
        }
        return false;
	#endif
}

String Environment::getFilePart( const String& fileName ) {
    return fileName.substr( fileName.find_last_of( PATH_SEPARATOR ) + 1 );
}

/**
 * Returns a localized string.
 * @param id Resource ID of the string.
 * @return If the string was found it is returned, otherwise an empty string is returned.
 */
String Environment::getString( Int id ) {
	#ifdef WINDOWS
        return FitsLiberator::Windows::ResourceManager::LoadString( id );
    #else
        String text = "";

		if( Environment::getBundleFactory() != NULL ) {
            // Convert id into a CFStringRef: key.
			std::ostringstream	keyBuffer;
			keyBuffer << id;
			CFStringRef key = CFStringCreateWithCString(NULL, keyBuffer.str().c_str(), kCFStringEncodingMacRoman );

			CFBundleRef	b = Environment::getBundleFactory()->getCFBundleRef();

			// Fetch localized string.
			// BUG FIX: NULL ensures Localisable.strings is used. Inserting a CFSTR("Localisable") instead
			// creates a weird problem that causes PS to crash. It might be because that, although CFSTR("...") are constants,
			// they are only accessible if the bundle is actually loaded.
			CFStringRef localizedString = ::CFCopyLocalizedStringFromTableInBundle( key, NULL, b, "" );
			Char* buffer = (Char*) ::CFStringGetCStringPtr( localizedString, kCFStringEncodingMacRoman );
			
			// CFStringGetCStringPtr might return NULL, so then we might need to call CFStringGetCString
			// and allocate space ourself, and thereby cannot send assign buffer to text right away.
			if( buffer == NULL ) {
				UInt length = ::CFStringGetLength( localizedString ) + 2;
				buffer = new Char[length];
			
				// Returns true if C string got copied.
				if( ::CFStringGetCString( localizedString, buffer, length, kCFStringEncodingMacRoman ) ) {
					text = buffer;
				}
				
				delete[] buffer;
			} else {
				text = buffer;
			}
			
			// Remeber to release the localized string.
			::CFRelease( localizedString );
			::CFRelease( key );
		}
        return text;
	#endif
}

#ifdef WINDOWS

/**
 * Creates a file path in the proper settings directory
 * @param fileName the path to create
 */
String Environment::getSettingsFile( String fileName ) {
	String path;
	TCHAR szPath[MAX_PATH];
    if( SUCCEEDED(::SHGetFolderPath(NULL, CSIDL_APPDATA, NULL,  SHGFP_TYPE_CURRENT, szPath))) {
        ::PathAppend(szPath, TEXT("FITS Liberator"));
        ::CreateDirectory( szPath, NULL );
        ::PathAppend(szPath, fileName.c_str() );
        
        path = szPath;
    }
    return path;
}
#endif

String Environment::getPreferencesPath() {
#ifdef WINDOWS
    return getSettingsFile( "\\Preferences.plist" );
#else
	String path;
	
	FSRef prefFolder;
	::FSFindFolder( kUserDomain, kPreferencesFolderType, kDontCreateFolder, &prefFolder );
	
	CFURLRef 	prefURL 		= ::CFURLCreateFromFSRef( NULL, &prefFolder );
	CFURLRef 	prefFileURL 	= ::CFURLCreateCopyAppendingPathComponent( NULL, prefURL, CFSTR("org.spacetelescope.FitsLiberator.plist"), false );
	CFStringRef	prefFilePath 	= ::CFURLCopyFileSystemPath( prefFileURL, kCFURLPOSIXPathStyle ); //kCFURLHFSPathStyle
	CFIndex 	pathLen			= ::CFStringGetLength(prefFilePath);
	
	Char* buffer = new Char[pathLen+1];
	
	::CFStringGetCString( prefFilePath, buffer, pathLen + 1,  kCFStringEncodingMacRoman );
	
	path = buffer;
	
	if( buffer != NULL ) { 
		delete[] buffer;
	}	
    return path; 
#endif
}

String Environment::readResource( String name ) {
	#ifdef WINDOWS
        String text = FitsLiberator::Windows::ResourceManager::LoadText( name.c_str(), "TEXT" );
        return text;
    #else
	    String content, line;
	    std::ifstream file( Environment::getBundleResourceFilePath( name.c_str() ).c_str() );
    	
	    if ( file ) {
		    while( getline( file, line ) ) {
			    content.append( line );
		    }
	    }
	    
	    file.close();

	    return content;	
	#endif
}

#ifndef WINDOWS
/**
 *
 *
 */
String Environment::getBundleResourceFilePath( const Char* name ) {
	return Environment::CFStringToString( ::CFURLCopyFileSystemPath( Environment::getBundleResourceURL( name ), kCFURLPOSIXPathStyle ) ); //kCFURLHFSPathStyle
}

/**
 *
 *
 */
CFURLRef Environment::getBundleResourceURL( const Char* str ) {
	CFStringRef resourceName 	= ::CFStringCreateWithCString( NULL, str, kCFStringEncodingMacRoman );	
	CFBundleRef	bundle			= Environment::getBundleFactory()->getCFBundleRef();
	
	
	CFURLRef url = ::CFBundleCopyResourceURL( bundle, resourceName, NULL, NULL );
	::CFRelease( resourceName );
	return url;
}

/**
 *
 * You are responsible for releasing the returned string
 */
CFStringRef Environment::StringToCFString( const Char * str) {
	CFStringRef cfstr = ::CFStringCreateWithCString( NULL, str, kCFStringEncodingMacRoman );
	
	return cfstr;
}

/**
 *
 *
 */
String Environment::CFStringToString( CFStringRef cfstr ) {
	CFIndex pathLen = ::CFStringGetLength( cfstr );
			
	Char* buffer = new Char[pathLen+1];
			
	::CFStringGetCString( cfstr, buffer, pathLen + 1,  kCFStringEncodingMacRoman );
			
	String str = buffer;
			
	if( buffer != NULL ) {
		delete buffer;
	}
	
	::CFRelease( cfstr );
	
	return str;
}
#endif

/**
 * Initialize platform specific resources
 * 
 */
void Environment::initPlatformResources( ) {
    #ifdef WINDOWS
        InitializeUI();
        #ifdef DEBUG
            debugLog = new std::ofstream( getSettingsFile( "Debug.log").c_str(), std::ios::app );
        #endif
    #else
        // Make the bundle factory. Note: PlugInFileURLBundleFactory releases plugInFileURL.
		Environment::bundleFactory = new Mac::IdentifierBundleFactory( CFSTR(FITSBUNDLEIDENTIFIER) );
    #endif
}

#ifdef DEBUG
#ifdef WINDOWS
Void Environment::DebugMessage( const String& path, UInt line, const String& message ) {
    
    if( Environment::debugLog != NULL ) {
        UInt i = path.find( "sources" );
        if( i > 0 ) {
            *debugLog << path.substr( i + 7 );
        }
        else {
            *debugLog << path;
        }
        *debugLog << ":" << line << ": " << message << std::endl;
    }
    
}
#else
    
#endif
#endif

#ifdef WINDOWS

/**
 * Initializes the UI libraries.
 */
Void Environment::InitializeUI() {

    //
    // Initialize the common controls library
    INITCOMMONCONTROLSEX ice;
    ice.dwSize = sizeof( ice );
    ice.dwICC = ICC_UPDOWN_CLASS | ICC_LISTVIEW_CLASSES | ICC_TAB_CLASSES | ICC_BAR_CLASSES | ICC_PROGRESS_CLASS;
    ::InitCommonControlsEx( &ice );

    //
    // Initialize GDI+
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup( &gdiplusToken, &gdiplusStartupInput, NULL );
}

/**
 * Informs the UI that it is no longer needed. No calls to other methods of the class should be made.
 */
Void Environment::DisposeUI() {
    //
    // Release GDI+
    GdiplusShutdown( gdiplusToken );
}

#endif

/**
*	In principle redundant since v. 3.0 since it was previously
*   bound to the Photoshop version used.
*   Still present for compatibility reasons
*   @param bitDepth the given bitDepth to be supported
*   @returns always true since this method is redundant since v. 3.0
*/
Bool Environment::supportsBitDepth( ChannelSettings bitDepth ) {
	return true;
}

/**
* In version earlier than 3.0 FITS Liberator was still a plugin in
* Photoshop and the alpha channel support is not in any PS version
* and this method thus checked for this. Now it only determines whether
* transparency is supported based on the bitDept. If it is 8 or 16 bit then
* transparency is supported, if it is 32 bit then not
* @param bitDept the current bitDepth
* @returns true if transparency is supported. False if not
*/
Bool Environment::supportsTransparent( ChannelSettings bitDepth ) {
	   
    if( bitDepth == channel32 ) {
        return false;                   // No 32-bit alpha support :(
	}
    
    return true;                            // 8-bit support everywhere    
}

/**
*	Deprecated as per version 3.0
*/
Bool Environment::supports32bitCoordinates() {
    if( hostIsElements ) {
        return false;
    }
    else {
        return hostMajorVersion >= 8;
    }
}

/**
 * Release any platform specific resources.
 *
 */
Void Environment::releasePlatformResources( ) {

    #ifdef DEBUG
    	#ifdef WINDOWS
        	if( debugLog != NULL )
			{
            	delete debugLog;
				debugLog = NULL;
			}
	    #endif
    #endif
    
    #ifdef WINDOWS
        DisposeUI();
    #else
    	if( Environment::bundleFactory != NULL ) {
    		delete Environment::bundleFactory;
    		Environment::bundleFactory = NULL;
    	}
    #endif
}

#ifndef WINDOWS
/**
 * <b>OS X ONLY:</b> Get a bundle factory to the plug-in's bundle.
 *
 * @return A Mac::BundleFactory.
 */
Mac::BundleFactory* Environment::getBundleFactory( ) {
   	return Environment::bundleFactory;
}
#endif


#ifdef WINDOWS

/**
* Windows specific file dialog opener
*
*  @param fileName the name of the file to save (in case @param mode is FILEMODE_SAVE)
*  otherwise ignored.
*  @param mode either FILEMODE_SAVE or FILEMODE_OPEN depending on whether
*  the dialog should be "Save as.." og "Open file.."
*  @param owner handle to the owner window
*/
String Environment::LoadFileDialog( String fileName, UInt mode, HWND owner )
{
	char Filestring[4196];
	//reset Filestring
	for ( UInt i = 0; i < 4196; i++ )
		Filestring[i] = 0;
	String returnstring;
	OPENFILENAME opf;
	LPCTSTR filter;
	Vector<char> str2;

	if ( mode == FILEMODE_OPEN )
	{
		filter = "Fits files\0*.FITS;*.FIT;*.FTS;*.IMG;*.LBL;\0\0";
		opf.lpstrDefExt = "fit";
		
		
	
	}
	else if ( mode == FILEMODE_SAVE )
	{		
		filter = "Tiff files\0*.TIFF;*.TIF\0\0";
		opf.lpstrDefExt = "tif";
		//get the position of the last '.' which defines the file extension
		Int pos = fileName.find_last_of( ".", String::npos );
		if ( pos > 0 )
			fileName = fileName.substr( 0, pos );

		fileName = fileName + ".tif";

		fileName = Environment::getFilePart( fileName );
		
		
		fileName.copy( Filestring, fileName.length(), 0 );

		
	}
	
	opf.lpstrTitle = NULL;
	opf.hwndOwner = owner;
	opf.hInstance = NULL;
	opf.lpstrFilter = filter;
	opf.lpstrCustomFilter = NULL;
	opf.nMaxCustFilter = 0L;
	opf.nFilterIndex = 1L;
	opf.lpstrFile = Filestring;	
	opf.nMaxFile = 4196;
	opf.lpstrFileTitle = NULL;
	opf.nMaxFileTitle=50;
	opf.lpstrInitialDir = NULL;

	opf.nFileOffset = NULL;
	opf.nFileExtension = NULL;
	
	opf.lpfnHook = NULL; 
	opf.lCustData = NULL;
	opf.Flags = (OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT) & ~OFN_ALLOWMULTISELECT;  
	opf.lStructSize = sizeof(OPENFILENAME);
	opf.FlagsEx = 0; 
	if ( mode == FILEMODE_OPEN )
	{
		if(GetOpenFileName(&opf))    		        
			returnstring = opf.lpstrFile;
	}
	else if ( mode == FILEMODE_SAVE )
	{
		if(GetSaveFileName(&opf))    		        
			returnstring = opf.lpstrFile;
	}
	return returnstring;
}
#endif

/**
 * Returns the decimal separator
 */
String Environment::getDecimalSeparator() {
    #ifdef WINDOWS
        TCHAR buffer[4];
        ::GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, buffer, 4 );

        return String( buffer );
    #else
        return String(kFITSDefaultDecimalSeparator);
    #endif
}


/**
 * Returns the thousands separator
 */
String Environment::getThousandsSeparator() {
    #ifdef WINDOWS
        TCHAR buffer[4];
        ::GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, buffer, 4 );

        return String( buffer );
    #else
        return String(kFITSDefaultThousandsSeparator);
    #endif
}

/**
 * Returns the negative sign
 */
String Environment::getNegativeSign() {
    #ifdef WINDOWS
        TCHAR buffer[5];
        ::GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_SNEGATIVESIGN, buffer, 5 );

        return String( buffer );
    #else
        return String(kFITSDefaultNegativeSign);
    #endif
}

/**
 * Returns the positive sign
 */
String Environment::getPositiveSign() {
    #ifdef WINDOWS
        TCHAR buffer[5];
        ::GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_SPOSITIVESIGN, buffer, 5 );

        return String( buffer );
    #else
        return String(kFITSDefaultPositiveSign);
    #endif
}

/**
*	Returns the maximum amount of memory the pixel-handler should use.
*   The implementation of this function varies between the two platforms, obviously.
*   However, the concept is the same.
*   If the platform is 32 bit the allowed amount of mem usable to the pixel loader
*   is determined as the minimum value of 1.5 GB and 75 % of the total physical memory.
*   We could use up to 2 GB, however, the rest of the plugin also needs memory. 
*   Furthermore, if the total physical memory is less than 2 GB we use 75 per cent of that
*   since otherwise the system will have to swap on the disk alot which may lower performance.
*   If the system is 64 bit we use the minimum value of 3.5 GB and 75 per cent of the physical mem
*   The arguments are the same, however, since this is a 32 bit app we can not handle more than 4 GB
*   at any time. For a 32 bit OS, at least in the case of Windows, the program should not normaly
*   use more than 2 GB of mem for various reasons.
*/
UInt64 Environment::getMaxMemory()
{
	//2GB in bytes
	UInt64 twoGB = (UInt64)2*(UInt64)1024*(UInt64)1024*(UInt64)1024;
	//4GB in bytes
	UInt64 fourGB = (UInt64)4*(UInt64)1024*(UInt64)1024*(UInt64)1024;
	//the fraction of mem to use
	Double frac = 0.75;
	//set a default of 2 GB
	UInt64 totalPhys = twoGB;
	//also set default to 2 GB
	UInt64 totalVirtual = twoGB;
	//the OS type (32 bit if false, 64 bit if true). Default is false
	Bool is64Bit = false;
	//the resulting amount to use. Default is 75 percent of totalVirtual
	UInt64 totalToUse = floor( frac * totalVirtual );

	//try to actually retrieve the values
#ifdef WINDOWS
	MEMORYSTATUSEX mem;
	mem.dwLength = sizeof(mem);
	if ( GlobalMemoryStatusEx( &mem ) )
	{
		totalPhys = mem.ullTotalPhys;
		totalVirtual = mem.ullTotalVirtual;
	}
	BOOL bIsWow64 = FALSE;

    //IsWow64Process is not available on all supported versions of Windows.
    //Use GetModuleHandle to get a handle to the DLL that contains the function
    //and GetProcAddress to get a pointer to the function if available.

    LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(
        GetModuleHandle(TEXT("kernel32")),"IsWow64Process");

    if(NULL != fnIsWow64Process)
    {
        if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
        {
            //handle error
        }
    }
    
	if ( bIsWow64 )
		is64Bit = true;	

#else
	is64Bit = true;
	
	int    mib[2];
	size_t length;
	
	mib[0] = CTL_HW;
	mib[1] = HW_MEMSIZE;
	
	length = sizeof(totalPhys);
	sysctl(mib, 2, &totalPhys, &length, NULL, 0);
#endif

	if ( is64Bit )	
		totalToUse = floor( frac * ( FitsMath::minimum<UInt64>( totalPhys, fourGB ) ) );	
	else
		totalToUse = floor( frac * ( FitsMath::minimum<UInt64>( totalPhys, twoGB ) ) );	
		
	

	return totalToUse;
}

/**
 * Returns the number of digits after the decimal point
 */
Int Environment::getDecimalPlaces() {
    //#ifdef WINDOWS
    //    TCHAR buffer[2];
    //    ::GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_IDIGITS, buffer, 2 );

    //    return (Int)buffer[0];
    //#else
        return kFITSDefaultPrecision;
    //#endif
}

/**
 * Returns the number of digits after the decimal point
 */
Int Environment::getUnfixedDecimalPlaces() {
    //#ifdef WINDOWS
    //    TCHAR buffer[2];
    //    ::GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_IDIGITS, buffer, 2 );

    //    return (Int)buffer[0];
    //#else
        return kFITSDefaultUnfixedPrecision;
    //#endif
}

void Environment::startSystemEditor(const String& path) {
#if defined(WINDOWS)
	::ShellExecute( NULL, NULL, path.c_str(), NULL, NULL, NULL );
#else
	CFStringRef pathRef = ::CFStringCreateWithCString(kCFAllocatorDefault, path.c_str(), kCFStringEncodingMacRoman);
	CFURLRef    urlRef  = ::CFURLCreateWithFileSystemPath(kCFAllocatorDefault, pathRef, kCFStringEncodingMacRoman, false);
	
	::LSOpenCFURLRef(urlRef, NULL);
	
	::CFRelease(urlRef);
	::CFRelease(pathRef);
#endif
}
