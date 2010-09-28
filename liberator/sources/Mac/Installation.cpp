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

#include "Installation.h"
#include "Folders.h"
#include "FSCopyObject.h"
#include "Environment.h"
#include "MachOFrameworkSupport.h"
using namespace FitsLiberator;
using namespace FitsLiberator::Mac;
using namespace FitsLiberator::Mac::Installation;
/**
 * Check version information from preference to see if we need to do an 
 * installation check.
 *
 * @param majorver Major verision number from preferences
 * @param minorver Minor verision number from preferences
 * @param installed If XMP panels has been installed.
 * @return True if installation check is needed.
 */
Bool FitsLiberator::Mac::Installation::RequireInstallationCheck( String majorver, String minorver, Bool installed )
{
	BaseDebugStr("RequireInstallationCheck");
	if( majorver == FITSMAJORVERSION && minorver == FITSMINORVERSION && installed == true )
	{
		return true;
	}
	else 
	{
		return false;
	}
}
/**
 * Perform check of installation to see if it's ok.
 *
 * @param 
 * @param 
 * @param 
 * @return 
 */
OSStatus FitsLiberator::Mac::Installation::InstallationCheck( String majorver, String minorver, Bool installed )
{
	BaseDebugStr("InstallationCheck");
	
	if( RequireInstallationCheck(majorver, minorver, installed) )  {
		return errFITSInstallationOK;
	}
	
	
	if( InstallXMPPanels( kUserDomain ) ) {
		BaseDebugStr("Installation Succeeded!");
	} else {
		BaseDebugStr("Installation Failed!");
	}
	
	/*OSStatus myStatus;
    AuthorizationFlags myFlags = kAuthorizationFlagDefaults;
    AuthorizationRef myAuthorizationRef;
    myStatus = MachO::AuthorizationCreate(NULL, kAuthorizationEmptyEnvironment, myFlags, &myAuthorizationRef);
	
	if( myStatus != errAuthorizationSuccess )
        return errFITSInstallationOK;
	
	
    do 
    {
        {
            AuthorizationItem myItems = {kAuthorizationRightExecute, 0, NULL, 0};
            AuthorizationRights myRights = {1, &myItems};
            myFlags = kAuthorizationFlagDefaults | kAuthorizationFlagInteractionAllowed | kAuthorizationFlagPreAuthorize | kAuthorizationFlagExtendRights;
            myStatus = MachO::AuthorizationCopyRights(myAuthorizationRef, &myRights, NULL, myFlags, NULL );
        }
 
        if (myStatus != errAuthorizationSuccess) 
        	break;
 
        {
            char myToolPath[] = "/usr/bin/id";
            char *myArguments[] = { "-un", NULL };
            FILE *myCommunicationsPipe = NULL;
            char myReadBuffer[128];
            myFlags = kAuthorizationFlagDefaults;
            myStatus = MachO::AuthorizationExecuteWithPrivileges(myAuthorizationRef, myToolPath, myFlags, myArguments, &myCommunicationsPipe);
			myStatus = MachO::AuthorizationExecuteWithPrivileges(myAuthorizationRef, myToolPath, myFlags, myArguments, NULL);
            if (myStatus == errAuthorizationSuccess) 
            {
            	BaseDebugStr("Root Authorization");
            }
            else 
            {
            	BaseDebugStr("No Authorization");
            }
                //for(;;)
                //{
                    //int bytesRead = read(fileno (myCommunicationsPipe), myReadBuffer, sizeof (myReadBuffer));
                    //if (bytesRead < 1) break;
					//write (fileno (stdout), myReadBuffer, bytesRead);
                //}
        }
    } while (0);
    MachO::AuthorizationFree(myAuthorizationRef, kAuthorizationFlagDefaults);
	*/
	return errFITSInstallationOK;
}
/**
 * Locate the Adobe/XMP/Custom File Info Panels folder in Application Support
 *
 * @param domain	Domain in which to locate the Adobe XMP folder (e.g. kLocalDomain or kUserDomain).
 * @return 			An URL to the sepcified folder.
 */
CFURLRef FitsLiberator::Mac::Installation::LocateAdobeXMPFolder( short domain ) {
	OSErr status = noErr;
	FSRef appSupportFolder;
	
	::FSFindFolder( domain, kApplicationSupportFolderType, kDontCreateFolder, &appSupportFolder );
		
	CFURLRef 	appSupportURL 	= ::CFURLCreateFromFSRef( NULL, &appSupportFolder );
	CFURLRef 	adobeXMPURL		= ::CFURLCreateCopyAppendingPathComponent( NULL, appSupportURL, CFSTR("Adobe/XMP/Custom File Info Panels"), true );
	::CFRelease( appSupportURL );
	
	return adobeXMPURL;
}
/**
 * Locate a given file in the resource folder of the current bundle.
 *
 * @param file	Filename to locate in resource folder of bundle.
 * @return  	CFURLRef to the requsted file.
 * @see 		FitsLIberator::Environment::getBundleResourceURL
 * @throws 		Exception If file does not exists an exception is thrown.
 */
CFURLRef FitsLiberator::Mac::Installation::LocateResourceFile(const Char* file) {
	CFURLRef 	panel = Environment::getBundleResourceURL( file );
	
	if( panel == NULL ) {
		throw Exception("Installation file (XMP panel) is missing  - please reinstall this plug-in.");
	}
	
	return panel;
}
/**
 * Create a directory.
 *
 * @param parentfolder	Folder in which to create the new directory.
 * @param newname		Name of the new directory.
 * @return				NULL if request didn't succedd, otherwise a FSRef to the new directory.
 */
FSRef FitsLiberator::Mac::Installation::CreateDirectory( CFURLRef parentfolder, CFStringRef newname ) {
	FSRef newRef	= NULL;
	FSRef parentRef	= NULL;
	
	// CFString to UniChar conversion
	CFIndex 	nameLength 	= ::CFStringGetLength( newname );
	CFRange 	rgn 		= { 0, nameLength };
	UniChar* 	nameBuf 		= new UniChar[nameLength];
	::CFStringGetCharacters( newname, rgn, &nameBuf);
	
	// CFURLRef to FSRef
	String 		buffer 		= Environment::CFStringToString( ::CFURLCopyFileSystemPath( parentfolder, kCFURLPOSIXPathStyle ) );
	OSStatus 	err 		= ::FSPathMakeRef( (unsigned char*) buffer.c_str(), &parentRef, false );
	
	// Create directory if we could find parent folder.
	if( err == noErr ) {
		OSErr status = ::FSCreateDirectoryUnicode( parentRef, (UniCharCount) nameLength, &nameBuf, kFSCatInfoNone, NULL, &newRef, NULL, NULL);
	}
	
	delete nameBuf;
	
	if( status != noErr ) {
		return NULL;
	} else {
		return newRef;
	}
}
/**
 * Copies a file to a destionation folder. You must have write permissions to this folder.
 * 
 * @param src	Source file to copy.
 * @param dst	Destination folder to copy file into.
 * @return 		Status code from FSCopyObject
 */
OSErr FitsLiberator::Mac::Installation::CopyInstallFile( CFURLRef src, CFURLRef dst ) {
	FSRef dstFileRef;
	FSRef srcFileRef;
	
	// Obtain FSRef to source file.
	if( !::CFURLGetFSRef( src, &srcFileRef ) ) {
		throw Exception("Installation file (XMP panel) is missing - please reinstall this plug-in.");
	}
	
	// Obtain path from CFURLRef.
	String buffer = Environment::CFStringToString( ::CFURLCopyFileSystemPath( dst, kCFURLPOSIXPathStyle ) );
	
	// Obtain FSRef to destination folder.
	OSStatus err = ::FSPathMakeRef( (unsigned char*) buffer.c_str(), &dstFileRef, false );
	
	if( err == fnfErr ) {
		BaseDebugStr("Problems with FSref for dest path")	
	}
	
	// Copy file (action: will delete the existing object and then copy over the new one).
	return FSCopyObject( &srcFileRef, &dstFileRef, 0, kFSCatInfoNone, kDupeActionReplace, NULL, false, false, NULL, NULL, NULL, NULL );	
}
/**
 * Determines if a file exists in a specified folder.
 *
 * @param filename 	Filename to search for in specified folder.
 * @param dest		Folder to search in for specified filename.
 * @return 			True if file exists, false otherwise.
 */
OSErr FitsLiberator::Mac::Installation::FileExists( const Char* filename, CFURLRef dest ) {
	Bool result;
	// First create CFURL for file.
	CFStringRef cfFilename 	  = Environment::StringToCFString( filename );
	CFURLRef 	cfFilenameURL = ::CFURLCreateCopyAppendingPathComponent( NULL, dest, cfFilename, false );
	
	
	// Try to get FSRef - you will only get an FSRef if the file exists.
	FSRef		targetFileRef;
	
	if( ::CFURLGetFSRef( cfFilenameURL, &targetFileRef ) ) {
		result = true;
	} else {
		result = false;
	}
	
	// Clean up
	::CFRelease( cfFilename );
	::CFRelease( cfFilenameURL );
	
	return result;
}
/**
 * Copies a file from the Resources folder in the bundle to a specfied destionation folder.
 *
 * @warn You're responsible for that the file doesn't already exists. 
 *
 * @param srcFilename	Filename of resource to copy to destination folder.
 * @param installFolder	Folder to copy resource file into.
 * @return 				errFITSInstallFileInstalled, errFITSInstallFileNotInstalled or errFITSInstallFileAlreadyExists.
 *
 */
OSStatus FitsLiberator::Mac::Installation::InstallFile( const Char* srcFilename, CFURLRef installFolderURL ) {
	OSStatus installStatus;
	
	// Determines if file exists	
	if( !FileExists( srcFilename, installFolderURL) ) {
		CFURLRef sourceFile	= LocateResourceFile( srcFilename );
		
		
		// Copyfile
		if( noErr == CopyInstallFile( sourceFile, installFolderURL ) ) {
			BaseDebugStr("File copied");
			installStatus = errFITSInstallFileInstalled;
		} else {
			BaseDebugStr("Filed not copied");
			installStatus = errFITSInstallFileNotInstalled;
		}
			
		::CFRelease(sourceFile);
	} else {
		BaseDebugStr("File already exists");
		installStatus = errFITSInstallFileAlreadyExists;
	}	
	
	return installStatus;
}
/**
 * Install XMP panel files.
 *
 * @param domain 	The domain in which to install the XMP panels (e.g. kUserDomain or kLocalDomain).
 * @return 			True if all files where successfully installed, false otherwise.
 */
Bool FitsLiberator::Mac::Installation::InstallXMPPanels( const short domain ) {
	BaseDebugStr("InstallXMPPanels");
	
	static const Int	nFiles 	= 5;
	static const Char* 	files[]	= { "VR-Content.txt", "VR-Creator.txt", "VR-FitsLiberator.txt", "VR-Observation.txt", "VR-Publisher.txt" };
	Bool				installStatus = true;
	
	// ---------------------------
	// Locate application support folder. 
	// ---------------------------
	CFURLRef adobeXMPURL = LocateAdobeXMPFolder( domain );
	
	// ---------------------------
	// Install panels in application support folder in specified domain (e.g. kLocalDomain)
	// ---------------------------
	for( Int i = 0; i < nFiles; i++ ) {
		if( errFITSInstallFileInstalled != InstallFile( files[i], adobeXMPURL ) ) {
			installStatus = false;
			break;					// Don't try to install rest if one fails.
		}
	}
	::CFRelease(adobeXMPURL);
	
	return installStatus;
}