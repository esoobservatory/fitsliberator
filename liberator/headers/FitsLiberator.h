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

#include "Types.h"
#include "Version.h"
#include "FitsBehavior.h"

/**
 * \mainpage The ESA/ESO/NASA FITS Liberator 3 - Technical Documentation
 *
 * The projects directory consits of:
 * \li \c cfitsio - Headers and libraries for CFITSIO.
 * \li \c documentation - Doxygen config file, scripts for generating documentation and this documentation.
 * \li \c headers - Header files for FITS Liberator, divided by namespace.
 * \li \c intermediate - For intermediate build files (e.g. precompiled headers).
 * \li \c output - Build output directory.
 * \li \c project - CodeWarrior, VS.NET and Installer project files.
 * \li \c pssdk - Photoshop API (the photoshopapi directory from the Photoshop SDK).
 * \li \c pssdkcs2 - Photoshop CS2 API  (the photoshopapi directory from the Photoshop CS2 SDK).
 * \li \c resources - Common and platform specific resources (e.g. PIPL, GUI files, licence text, images).
 * \li \c sources - Source files for FITS Liberator, divided by namespace.
 * \li \c tinyxml - Header and source files for the TinyXML parser.
 * \li \c trex - Header and source files for the T-Rex, a tiny regular expression library. 
 *
 * Read more about selected topics:
 * \li \ref cvs
 * \li \ref codingguide
 * \li \ref osxnotes
 * \li \ref windowsnotes
 * \li \ref testing
 * \li \ref misc
 *
 */
 
/**
 * \page osxnotes Mac OS X notes
 * \section cwprj CodeWarrior project
 * The project file is located in \c project/mac. Since Photoshop requires that plug-in are using CFM and not 
 * Mach-O due to some old legacy issues, you cannot use e.g. XCode for compiling the plug-in, but are 
 * stuck with CodeWarrior.
 *
 * The CodeWarrior (CW) project has two targets.
 * \li <b>Carbon Debug Bundle</b> that output the build to the Photoshop \c Plug-Ins/File \c Format directory.
 * \li <b>Carbon Release Bundle</b> that output the build to \c output/mac directory.
 *
 * The only differences besides the build output directory, is that Carbon Debug Bundle target 
 * has a preprocessor macro \c _DEBUG defined, and generates a SYM file for debugging.
 * Both targets generates a bundle with the actual plug-in located in same directory as \c Contents.
 * This makes it easier to find the path to the bundle later on. The bundle also contains the 
 * GUI files.
 *
 * \section bundle Distribution with bundles
 * The plug-in is distributed as bundle, because we use .nib files for the GUI. Since a bundle
 * is basically a directory structure, then Photoshop doesn't care about it when searching for 
 * plug-ins in its \c Plug-Ins folder. The binary module is located in the root folder in the
 * bundle, to make it easier to find the bundle path.
 *
 * In the target settings window under the "Target > PPC Target", the project type is
 * set to Shared Library Package, with the Creator \c 8BIM and type \c 8BIF. This is 
 * what is required to make the bundle, and make it look like a normal plug-in.
 *
 * The Info.plist file of the bundle is generated from <tt>PropertyList.plc</tt>
 *
 * \section osxguiedit Editing the GUI with Interface Builder
 * The GUI is located in <tt>resources/mac/FitsLiberator2GUI.nib</tt>. It contains a window called 
 * \c MainWindow and a menu called \c PreviewPopupMenu. Be sure that the window is set to compositing mode.
 *
 * <b>Note:</b> Not all Carbon functions which you may find in the reference documentation, 
 * are accessible from CarbonLib, while they are only available in Carbon.framework
 *
 * \subsection osxguicommands Commands
 * In the Show Info palette under Control you can specific what Signature, ID and Command.
 * The siganture is \c 8BIF and the ID mustn't conflict with other controls. Define each control
 * you need to acces in FitsMacTerminology.h. E.g. to define a Ok button with ID 1 and an 
 * associated command.
 *
 * \code
 * #define kFITSUIButtonOk  1
 * #define kFITSCommandOk   'ok  '
 * \endcode
 *
 * \subsection cfitsioosxlink Linking to CFITSIO
 * Drag the \c Cfitsio PPC.lib to the "Libraries" folder in the CodeWarrior project window 
 * and make sure that CodeWarriror can find \c fitsio.h and \c longnam.h in the cfitsio 
 * distibution directory. Either by added the files to the folder "Headers" in the project window
 * or by adding them to "Target > Access Paths" in the menu "Edit > Carbon Debug Settings". 
 *
 * \section debugcw Debugging the plug-in
 *
 * The target Carbon Debug Bundle is setup to debug Photoshop.  
 * Setting the Photoshop application to launch when debugging, is done under
 * "Edit > Carbon Debug Bundle Settings..." from the menu. Choose "Target > Runtime settings" 
 * in the window and select the host application. Make sure the build output directory is
 * a place where Photoshop can read the plug-in. E.g. Under "Target > Target Settings" you may
 * specify the output directory, which you probably want to set to something like
 *
 * \code /Applications/Adobe Photoshop CS/Plug-Ins/File Formats \endcode
 *
 * To start debugging choose "Project > Debug" from the menu. Photoshop will then be launched. 
 *
 * \section maccfitsio Compiling CFITSIO from the distribution
 * To compile the CFITSIO from the distribution get the newest distribution from http://heasarc.gsfc.nasa.gov/docs/software/fitsio/fitsio.html . 
 *
 * Decompress the archive file, goto the \c cfitsio/cfitsio_mac directory and follow the 
 * instructions in the \c README.MacOS to compile it. Note: Some files may not have been addded
 * to the project file, e.g. \c getcolsb.c and \c putcolsb.c, so if you see any strange errors when
 * trying to link to it later on, this is probably the reason.
 *
 * \section cfmmacho Calling OS X ONLY functions
 * Certain functions in Carbon are only available in Carbon.framework and not CarbonLib. It's
 * the same thing if you want to draw with Quartz instead of QuickDraw. To call these functions
 * you need to do some little extra work. We have to load e.g. Carbon.framework or
 * ApplicationsServices.framework at runtime and obtain a function pointer to the function 
 * we need to use. This is all done in MachOFrameworkSupport.h which also has a note on,
 * how to add more functions. 
 *
 *
 * \section reading Further reading
 * \subsection readguide Conceptual documents
 * \li http://developer.apple.com/documentation/Carbon/Conceptual/Carbon_Event_Manager/index.html
 * \li http://developer.apple.com/documentation/Carbon/Conceptual/UnarchivingIOwithIBS/index.html
 * \li http://developer.apple.com/documentation/Carbon/Conceptual/carbonmenus/index.html
 * \li http://developer.apple.com/documentation/Carbon/Conceptual/HandlingWindowsControls/index.html
 * \li http://developer.apple.com/documentation/DeveloperTools/Conceptual/MovingProjectsToXcode/migration_differences/chapter_2_section_25.html
 * 
 * \subsection readguide References
 * \li http://developer.apple.com/documentation/Carbon/Reference/Carbon_Event_Manager_Ref/index.html
 * \li http://developer.apple.com/documentation/Carbon/Reference/QuickDraw_Ref/index.html
 * \li http://developer.apple.com/documentation/Carbon/Reference/databrow_reference/index.html
 * \li http://developer.apple.com/documentation/Carbon/Reference/QuickDraw_Ref/index.html
 * \li http://developer.apple.com/documentation/Carbon/Reference/Menu_Manager/index.html
 * \li http://developer.apple.com/documentation/Carbon/Reference/Interface_Builder/index.html
 * \li http://developer.apple.com/documentation/Carbon/Reference/Control_Manager/index.html
 * \li http://developer.apple.com/documentation/Carbon/Reference/Window_Manager/index.html
 * \li http://developer.apple.com/documentation/Carbon/Reference/Appearance_Manager/index.html
 * \li http://developer.apple.com/documentation/MacOSX/Conceptual/BPRuntimeConfig/Concepts/PListKeys.html
 */

/**
 * \page misc Generating documentation with Doxygen
 * One script for OS X, \c makedoc.sh, and one script for Windows, \c makedoc.bat, are provided
 * for generating this documentation. 
 *
 * \li \c makedoc.sh assumes Doxygen OS X distribution is located and that the doxygen executable
 * is located in \c /Applications/Doxygen.app/Contents/Resources/doxygen.
 *
 * \li \c makedoc.bat assumes the the doxygen executable is located in the one of the
 * directories of PATH enviroment variable.
 */
 
/** 
 * \page windowsnotes Windows notes
 *
 *
 *
 *
 *
 */

/** 
 * \page installer Installer
 *
 *
 *
 *
 *
 */

/** 
 * \page testing Testing
 *
 *
 *
 *
 *
 */
 
/**
 * \page codingguide Coding guidelines
 * \section fname File names
 * \li Header file extension: <tt>.h</tt>
 * \li Source file extension: <tt>.cpp</tt>
 * \li Resource file extension: <tt>.r</tt> 
 *
 * Use capitalize first letter of each word in a file name to enhance readability, e.g <tt>FitsMacTerminology.h</tt>.
 * If the class contains a name, then give the file the same name as the class.
 *
 * \section forg File organization
 * Header files goes in \h headers/, source files goes in \c sources. Both directories contains
 * some subdirectories that comply with the namespaces. Files should be put in the 
 * directory mathcing their namespace.
 *
 * \section fheader File content
 * Each file must start with this header
 * \verbinclude fileheader.txt
 *
 * Furthermore header files should start with after the header.
 * \code
 * #ifndef __HEADERFILENAME_H__
 * #define __HEADERFILENAME_H__
 * \endcode 
 * 
 * and end with
 * \code
 * #endif __HEADERFILENAME_H__
 * \endcode
 *
 * A file can be subdivided by using headlines like:
 *
 * \code
 * //-------------------------------------------------------------------------------
 * // Application definitions
 * //-------------------------------------------------------------------------------
 * \endcode
 *
 * \section syscall System calls
 * System calls to e.g. Carbon functions should be called with namespace. That is 
 * \code
 * ::Point p;
 *
 * ::GetMouse( p );
 * \endcode
 *
 * instead of
 *
 * \code
 * Point p;
 *
 * GetMouse( p );
 * \endcode
 *
 * \section namconv Namig convetions
 * \li <b>Classes and namespaces:</b>  Class names should be nouns, in mixed case with 
 *     the first letter of each internal word capitalized. Try to keep your class names 
 *     simple and descriptive. Use whole words-avoid acronyms and abbreviations (unless 
 *     the abbreviation is much more widely used than the long form, such as URL or HTML).
 * \li <b>Methods/Functions:</b>  Methods should be verbs, in mixed case with the first 
 *     letter lowercase, with the first letter of each internal word capitalized.
 * \li <b>Variables:</b>  Except for variables, all instance, class, and class constants 
 *     are in mixed case with a lowercase first letter. Internal words start with capital 
 *     letters.
 * \li <b>Constans:</b> Should start with \c kFITS
 * \li <b>Macros:</b> Should be in upper case letters.
 * 
 */