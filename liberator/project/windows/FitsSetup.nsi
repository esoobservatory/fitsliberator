; Photoshop FITS Liberator
; Copyright 2004 ESA/ESO/NASA
; All Rights Reserved
; --------------------------------------------------------

; @file
; Setup script for the Windows version of FITS Liberator.
; To compile this setup script, the registry plug-in must 
; be installed.
;
; Installs these components:
; FITS Liberator
;    Photoshop Actions
;    Registers File Types
;    Adobe Bridge Support
; Concatenator

;-------------------------------------------------------------------------------
; Constants
;-------------------------------------------------------------------------------

!define PRODUCT_NAME            "Photoshop FITS Liberator"
!define PRODUCT_VERSION         "2.3"
!define PRODUCT_PUBLISHER       "ESO/ESA/NASA"
!define PRODUCT_WEB_SITE        "http://spacetelescope.org/projects/fits_liberator/"
!define PRODUCT_UNINST_KEY      "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"

!define SETUP_NAME   "..\..\binaries\FITS_Liberator_${PRODUCT_VERSION}_WIN.exe"
!define SETUP_ROOT   "$PROGRAMFILES\Photoshop FITS Liberator"
!define SETUP_LOG    "Fits.log"
!define SETUP_WIZ    "Uninstall Photoshop FITS Liberator.exe"

!define XMP_PATH    "$COMMONFILES\Adobe\XMP\Custom File Info Panels"

!define BANNER_FILE  "..\..\resources\windows\FitsSetup.bmp"
!define ICON_FILE    "..\..\resources\windows\FitsSetup.ico"
!define README_FILE  "..\..\resources\ReleaseNotes.rtf"
!define LICENSE_FILE "..\..\resources\License.rtf"

;-------------------------------------------------------------------------------
; Macros
;-------------------------------------------------------------------------------

!include "Sections.nsh"
!include "Registry.nsh"
 
;-------------------------------------------------------------------------------
; READ_DIRECTORY
;     Reads a directory from the registry and strips the last '\'
; Parameters:
;     TARGET  Variable to hold the value
;     HIVE    Registry hive to read from
;     PATH    Key path
;     KEY     Key name
!macro READ_DIRECTORY TARGET HIVE PATH KEY
    ReadRegStr ${TARGET} ${HIVE} "${PATH}" "${KEY}"
    Push ${TARGET}
    Call GetParent
    Pop ${TARGET}
!macroend
!define ReadDirectory `!insertmacro READ_DIRECTORY`

;-------------------------------------------------------------------------------
; READ_DIRECTORY_N
;     Reads a directory from the registry
; Parameters:
;     TARGET  Variable to hold the value
;     HIVE    Registry hive to read from
;     PATH    Key path
;     KEY     Key name
!macro READ_DIRECTORY_N TARGET HIVE PATH KEY
    ReadRegStr ${TARGET} ${HIVE} "${PATH}" "${KEY}"
!macroend
!define ReadDirectoryN `!insertmacro READ_DIRECTORY_N`

;-------------------------------------------------------------------------------
; WRITE_LOG
;     Writes a string to log file. The log file must be indentified by the $LOG
;     variable
; Parameters:
;     STRING   The string to write
!macro WRITE_LOG STRING
    FileWrite $LOG "${STRING}"
    FileWriteByte $LOG "13"
    FileWriteByte $LOG "10"
!macroend
!define WriteLog `!insertmacro WRITE_LOG`

;-------------------------------------------------------------------------------
; READ_LOG
;     Reads a line from the log file and strips the newline.
; Parameters:
;     TARGET  Variable the recieves the line
!macro READ_LOG TARGET
    FileRead $LOG ${TARGET}
    Push ${TARGET}
    Call un.TrimNewlines
    Pop ${TARGET}
!macroend
!define ReadLog `!insertmacro READ_LOG`

;-------------------------------------------------------------------------------
; INSTALL_FILE
;     Installs a file on the host and writes a log entry
; Parameters:
;     TARGET   Directory to install into
;     SOURCE   Source file relative to the script
;     FILENAME Filename of the source file
!macro INSTALL_FILE TARGET SOURCE FILENAME
    SetOverwrite on
    SetOutPath "${TARGET}"
    File /a "${SOURCE}"
    ${WriteLog} "File"
    ${WriteLog} "${TARGET}\${FILENAME}"
!macroend
!define InstallFile `!insertmacro INSTALL_FILE`

;-------------------------------------------------------------------------------
; CREATE_UNINSTALLER
;     Creates the un-installer program and writes a log entry
; Parameters:
;     TARGET   Directory to write into
;     FILENAME Filename of the uninstaller file
!macro CREATE_UNINSTALLER TARGET FILENAME
    SetOverwrite on
    SetOutPath "${TARGET}"
    WriteUninstaller "${TARGET}\${FILENAME}"
    ${WriteLog} "File"
    ${WriteLog} "${TARGET}\${FILENAME}"
!macroend
!define Uninstaller `!insertmacro CREATE_UNINSTALLER`

!macro REGSTR_HKLM SUBKEY NAME VALUE
    WriteRegStr HKLM "${SUBKEY}" "${NAME}" "${VALUE}"
    ${WriteLog} "HKLM"
    ${WriteLog} "${SUBKEY}"
!macroend
!define RegStrHKLM `!insertmacro REGSTR_HKLM`

!macro REGINT_HKLM SUBKEY NAME VALUE
    WriteRegDWORD HKLM "${SUBKEY}" "${NAME}" "${VALUE}"
    ${WriteLog} "HKLM"
    ${WriteLog} "${SUBKEY}"
!macroend
!define RegIntHKLM `!insertmacro REGINT_HKLM`

!macro REGKEY_HKCR SUBKEY NAME VALUE
    WriteRegStr HKCR '${SUBKEY}' '${NAME}' '${VALUE}'
    ${WriteLog} "HKCR"
    ${WriteLog} "${SUBKEY}"
!macroend
!define RegKeyHKCR `!insertmacro REGKEY_HKCR`

!macro DIRECTORY DIR
    SetOutPath "${DIR}"
    ${WriteLog} "Directory"
    ${WriteLog} "${DIR}"
!macroend
!define Directory `!insertmacro DIRECTORY`

;-------------------------------------------------------------------------------
; General
;-------------------------------------------------------------------------------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "${SETUP_NAME}"
InstallDir "$PROGRAMFILES"
ShowInstDetails show
ShowUnInstDetails show
SetCompressor lzma
RequestExecutionLevel admin

;-------------------------------------------------------------------------------
; Use Modern UI
;-------------------------------------------------------------------------------

!include "MUI.nsh"
!define MUI_ABORTWARNING
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP ${BANNER_FILE}
!define MUI_ICON ${ICON_FILE}
!define MUI_UNICON ${ICON_FILE}

;-------------------------------------------------------------------------------
; Descriptions
;-------------------------------------------------------------------------------

LangString PAGE_WELCOME_TEXT       ${LANG_ENGLISH} "This wizard will guide you through the installation of $(^Name). Please check the following before continuing:\r\n\r\n- If Photoshop is running, you must close it before starting Setup.\r\n\r\nClick Next to continue."
LangString PAGE_README_TITLE       ${LANG_ENGLISH} "The ESA/ESO/NASA Photoshop FITS Liberator"
LangString PAGE_README_SUBTITLE    ${LANG_ENGLISH} " "
LangString PAGE_README_TOPTEXT     ${LANG_ENGLISH} " "
LangString PAGE_README_BOTTOMTEXT  ${LANG_ENGLISH} " "
LangString PAGE_README_NEXTBUTTON  ${LANG_ENGLISH} "&Next >"
LangString PAGE_DIRECTORY_TITLE    ${LANG_ENGLISH} "Choose install location"
LangString PAGE_DIRECTORY_SUBTITLE ${LANG_ENGLISH} "Choose where Photoshop is installed"
LangString PAGE_DIRECTORY_TEXT     ${LANG_ENGLISH} "Setup will use the following directories and files to install $(^Name).\r\n\r\nIf you are using a localised version of Photoshop you may want to change the plugin and actions folders."
LangString PAGE_SHORTCUT_SUBTITLE  ${LANG_ENGLISH} "Create a shortcut for Mosaicator"

LangString COMPONENT_CORE          ${LANG_ENGLISH} "Photoshop FITS Liberator plugin core files"
LangString COMPONENT_ACTION        ${LANG_ENGLISH} "Photoshop Actions"
LangString COMPONENT_FILETYPE      ${LANG_ENGLISH} "Associate the .fit, .fits and .fts extensions with Photoshop"
LangString COMPONENT_CONCATENATOR  ${LANG_ENGLISH} "Tool to concatenate, or combine, images and their metadata"

LangString MESSAGE_MUSTEBEADMIN      ${LANG_ENGLISH} "You must be a member of the Administrators group to install $(^Name)"
LangString MESSAGE_UNINSTALL_SUCCESS ${LANG_ENGLISH} "$(^Name) was successfully removed from your computer."
LangString MESSAGE_UNINSTALL_CONFIRM ${LANG_ENGLISH} "Are you sure you want to completely remove $(^Name) and all of its components?"
LangString MESSAGE_UNINSTALL_ERROR ${LANG_ENGLISH} "Setup could not locate the installation log file."

LangString MESSAGE_PREVIOUSDETECTED ${LANG_ENGLISH} "Setup has detected that a previous version of the ${PRODUCT_NAME} is installed. You must remove the old version before $(^Name) can be installed."

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT 1 $(COMPONENT_CORE)
    !insertmacro MUI_DESCRIPTION_TEXT 2 $(COMPONENT_ACTION)
    !insertmacro MUI_DESCRIPTION_TEXT 3 $(COMPONENT_FILETYPE)
    !insertmacro MUI_DESCRIPTION_TEXT 4 $(COMPONENT_CONCATENATOR)
    !insertmacro MUI_DESCRIPTION_TEXT 5 $(COMPONENT_MOSAICATOR)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;-------------------------------------------------------------------------------
; Define the pages in the wizard
;-------------------------------------------------------------------------------

; --- Installation Pages -------------------------------------------------------

; Welcome
!define MUI_WELCOMEPAGE_TEXT $(PAGE_WELCOME_TEXT)
!insertmacro MUI_PAGE_WELCOME

; License
!define MUI_LICENSEPAGE_CHECKBOX
!insertmacro MUI_PAGE_LICENSE ${LICENSE_FILE}

; Readme
!define MUI_PAGE_HEADER_TEXT $(PAGE_README_TITLE)
!define MUI_PAGE_HEADER_SUBTEXT $(PAGE_README_SUBTITLE)
!define MUI_LICENSEPAGE_TEXT_TOP $(PAGE_README_TOPTEXT)
!define MUI_LICENSEPAGE_TEXT_BOTTOM $(PAGE_README_BOTTOMTEXT)
!define MUI_LICENSEPAGE_BUTTON $(PAGE_README_NEXTBUTTON)
!insertmacro MUI_PAGE_LICENSE ${README_FILE}

; Directory (See InitDirectory and ValidateDirectory functions
Page custom InitDirectory ValidateDirectory

; Components
!insertmacro MUI_PAGE_COMPONENTS

; Installing
!insertmacro MUI_PAGE_INSTFILES

; Finish
!insertmacro MUI_PAGE_FINISH

; --- Uninstallation Pages -----------------------------------------------------
; Uninstalling
!insertmacro MUI_UNPAGE_INSTFILES

;-------------------------------------------------------------------------------
; Language files
;-------------------------------------------------------------------------------

!insertmacro MUI_LANGUAGE "English"

;-------------------------------------------------------------------------------
; Variables
;-------------------------------------------------------------------------------

Var PSROOT          ; Photoshop directory
Var PSPATH          ; Path of the Photoshop.exe file
Var PLUGINPATH      ; Path of the Plug-Ins directory
Var ACTIONPATH      ; Path of the Photoshop Actions directory
Var SCRIPTPATH      ; Path of the Photoshop Script directory
Var BRIDGEROOT      ; Path of the Bridge root
Var SHORTCUTCONTEXT ; All users or current user
Var LOG             ; Log file handle

;-------------------------------------------------------------------------------
; Installer Sections
;-------------------------------------------------------------------------------

Section -Pre
    SetOutPath "${SETUP_ROOT}"
    FileOpen $LOG "${SETUP_ROOT}\${SETUP_LOG}" a
    FileSeek $LOG 0 END
SectionEnd

Section "Core Components"
    ; Ensure no old files exist
    ClearErrors
    
    Delete "$PSROOT\cfitsio.dll"
    Delete "$PLUGINPATH\FitsFormat.8bi"
    Delete "$PLUGINPATH\FitsLiberator.8bi"
    
    IfErrors 0 Core
    MessageBox MB_ICONSTOP|MB_OK "Unable to remove old installation, please make sure that all instances of Photoshop are closed."
	Abort
        
Core:
    ; Runtime
    File /a "/oname=$TEMP\vcredist_x86.exe" "..\..\resources\windows\vcredist_x86.exe"
    ExecWait '"$TEMP\vcredist_x86.exe" /q:a /c:"msiexec /i vcredist.msi /qn /l*v %temp%\vcredist_x86.log"'
    Delete "$TEMP\vcredist_x86.exe"
    ClearErrors

    ; Photoshop plug-in
    ${InstallFile} "$PSROOT"        "..\..\resources\windows\gdiplus.dll"                "gdiplus.dll"
    ${InstallFile} "$PLUGINPATH"    "..\..\binaries\FitsLiberator.8bi"                   "FitsLiberator.8bi"
    ${InstallFile} "${SETUP_ROOT}"  "..\..\resources\UserGuide.pdf"						 "UserGuide.pdf"
    
    ; Bridge support
    StrLen $0 $BRIDGEROOT
    IntCmp $0 0 Xmp
    ${InstallFile} "$BRIDGEROOT"           "..\..\resources\windows\gdiplus.dll"         "gdiplus.dll"
    ${InstallFile} "$BRIDGEROOT\Plug-Ins"  "..\..\binaries\FitsLiberator.8bi"            "FitsLiberator.8bi"    
    
Xmp:
    Delete	"${XMP_PATH}\VR1.xmp"
    Delete	"${XMP_PATH}\VR2.xmp"
    Delete	"${XMP_PATH}\VR3.xmp"
    Delete	"${XMP_PATH}\VR4.xmp"
    Delete	"${XMP_PATH}\VR5.xmp"

	; Version 1.0 Panels
    ${InstallFile} "${XMP_PATH}"    "..\..\resources\photoshop\AVM-1-Creator.txt"        "AVM-1-Creator.txt"
    ${InstallFile} "${XMP_PATH}"    "..\..\resources\photoshop\AVM-2-Content.txt"        "AVM-2-Content.txt"
    ${InstallFile} "${XMP_PATH}"    "..\..\resources\photoshop\AVM-3-Observation.txt"    "AVM-3-Observation.txt"    
    ${InstallFile} "${XMP_PATH}"    "..\..\resources\photoshop\AVM-4-Coordinates.txt"    "AVM-4-Coordinates.txt"
    ${InstallFile} "${XMP_PATH}"    "..\..\resources\photoshop\AVM-5-Publisher.txt"      "AVM-5-Publisher.txt"
    ${InstallFile} "${XMP_PATH}"    "..\..\resources\photoshop\AVM-6-FitsLiberator.txt"  "AVM-6-FitsLiberator.txt"
    
    ; Version 2.0 Panels
    ${InstallFile} "${XMP_PATH}\2.0\panels\AstronomyVisualization"		"..\..\resources\photoshop\AstronomyVisualization\manifest.xml"							"manifest.xml"
    ${InstallFile} "${XMP_PATH}\2.0\panels\AstronomyVisualization\bin"	"..\..\resources\photoshop\AstronomyVisualization\bin\AstronomyVisualization.swf"		"AstronomyVisualization.swf"
    ${InstallFile} "${XMP_PATH}\2.0\panels\AstronomyVisualization\loc"	"..\..\resources\photoshop\AstronomyVisualization\loc\AstronomyVisualization_de_DE.dat" "AstronomyVisualization_de_DE.dat"
    ${InstallFile} "${XMP_PATH}\2.0\panels\AstronomyVisualization\loc"	"..\..\resources\photoshop\AstronomyVisualization\loc\AstronomyVisualization_en_US.dat" "AstronomyVisualization_en_US.dat"
    ${InstallFile} "${XMP_PATH}\2.0\panels\AstronomyVisualization\loc"	"..\..\resources\photoshop\AstronomyVisualization\loc\AstronomyVisualization_fr_FR.dat" "AstronomyVisualization_fr_FR.dat"
    ${InstallFile} "${XMP_PATH}\2.0\panels\AstronomyVisualization\loc"	"..\..\resources\photoshop\AstronomyVisualization\loc\AstronomyVisualization_jp_JP.dat" "AstronomyVisualization_jp_JP.dat"
    ${Directory} "${XMP_PATH}\2.0\panels\AstronomyVisualization"
    
    ${InstallFile} "$SYSDIR\Macromed\Flash\FlashPlayerTrust"	"..\..\resources\photoshop\AstronomyVisualization.cfg"									"AstronomyVisualization.cfg"
	FileOpen $0 "$SYSDIR\Macromed\Flash\FlashPlayerTrust\AstronomyVisualization.cfg" a
	FileWrite $0 "${XMP_PATH}\2.0\panels$\r$\n"
	FileClose $0

; Install the uninstaller
    ${Uninstaller} "${SETUP_ROOT}" "${SETUP_WIZ}"

; Add the uninstaller to the Add/Remove applet
    ${RegStrHKLM} "${PRODUCT_UNINST_KEY}" "DisplayName"     "$(^Name)"
    ${RegStrHKLM} "${PRODUCT_UNINST_KEY}" "UninstallString" "${SETUP_ROOT}\${SETUP_WIZ}"
    ${RegStrHKLM} "${PRODUCT_UNINST_KEY}" "DisplayVersion"  "${PRODUCT_VERSION}"
    ${RegStrHKLM} "${PRODUCT_UNINST_KEY}" "URLInfoAbout"    "${PRODUCT_WEB_SITE}"
    ${RegStrHKLM} "${PRODUCT_UNINST_KEY}" "Publisher"       "${PRODUCT_PUBLISHER}"
    ${RegIntHKLM} "${PRODUCT_UNINST_KEY}" "NoModify"        1
    ${RegIntHKLM} "${PRODUCT_UNINST_KEY}" "NoRepair"        1
SectionEnd

Section "Photoshop Actions"
    ${InstallFile} "$ACTIONPATH"    "..\..\resources\Colour_composite.atn" "Colour_composite.atn"
SectionEnd

Section "Associate File Types"
    ${RegKeyHKCR} ".fits" "" "FitsImage"
    ${RegKeyHKCR} ".fit" "" "FitsImage"
    ${RegKeyHKCR} ".fts" "" "FitsImage"
    
    ${RegKeyHKCR} "FitsImage" "" "FITS Image File"
    ${RegKeyHKCR} "FitsImage\shell\open\command" "" '$PSPATH "%1"'
SectionEnd

Section "Concatenator"
    ; See the -Post section for the code to replace the Temp.jsx with the real script...
    ${InstallFile} "$SCRIPTPATH"    "..\..\..\concatenator\sources\FITS Concatenator.jsx" "Temp.jsx"
    ${InstallFile} "$SCRIPTPATH"    "..\..\..\concatenator\sources\xmlsax.inc" "xmlsax.inc"
    ${InstallFile} "$SCRIPTPATH"    "..\..\..\concatenator\sources\xmlw3cdom.inc" "xmlw3cdom.inc"
    
    ; Stupid hack to get around Photoshops way broken JavaScript engine
    SetOverwrite on
    SetOutPath "$SCRIPTPATH"

    File /a "..\..\..\includefixer\binaries\fix.exe"
    ExecWait '"$SCRIPTPATH\fix.exe" "$SCRIPTPATH" "Temp.jsx" "FITS Concatenator.jsx"'
    Delete "$SCRIPTPATH\fix.exe"    
SectionEnd

Section -Post
    ${Directory} "${SETUP_ROOT}"
    FileClose $LOG
SectionEnd

Section Uninstall
    SetShellVarContext all
    
; Delete files and directories
    FileOpen $LOG "${SETUP_ROOT}\${SETUP_LOG}" r
    IfErrors Error
Loop:
    ${ReadLog} $R0
    StrCmp $R0 ""          Done
    StrCmp $R0 "File"      _File
    StrCmp $R0 "Directory" _Directory
    StrCmp $R0 "HKLM"     _HKLM
    StrCmp $R0 "HKCR"     _HKCR
    Goto Loop
_File:
    ${ReadLog} $R1
    Delete $R1
    Goto Loop
_Directory:
    ${ReadLog} $R1
    RmDir /r $R1
    Goto Loop
_HKCR:
    ${ReadLog} $R1
    DeleteRegKey HKCR $R1
    Goto Loop
_HKLM:
    ${ReadLog} $R1
    DeleteRegKey HKLM $R1
    Goto Loop
    
Done:
    FileClose $LOG
    Delete "${SETUP_ROOT}\${SETUP_LOG}"
    Goto Exit
    
Error:
    MessageBox MB_ICONSTOP|MB_OK $(MESSAGE_UNINSTALL_ERROR)
    Abort
    
Exit:
    SetAutoClose true
SectionEnd

;-------------------------------------------------------------------------------
; Event Callbacks
;-------------------------------------------------------------------------------

; Called before installing the plugin, this is used to determine where to install
; the plugin. It searches for (in order) Photoshop CS, Photoshop 7.0,
; Photoshop Elements 2.0
Function .onInit
    SetShellVarContext all

    ; Test for the uninstaller
    IfFileExists "${SETUP_WIZ}" 0 +2
    ExecWait     '"${SETUP_WIZ}" /S'

; Test if the user is an administrator
    Call IsUserAdmin
    Pop $R0
    StrCmp $R0 "true" FindPhotoshop 0
    MessageBox MB_ICONEXCLAMATION|MB_OK $(MESSAGE_MUSTEBEADMIN)
    Abort

; Find Photoshop etc...
FindPhotoshop:
    Call LoadEnvironment

; Init the Directory page
    !insertmacro MUI_INSTALLOPTIONS_EXTRACT "FitsSetupDirectory.ini"
    
    !insertmacro MUI_INSTALLOPTIONS_WRITE "FitsSetupDirectory.ini" "Field 1" "State" $PSPATH
    !insertmacro MUI_INSTALLOPTIONS_WRITE "FitsSetupDirectory.ini" "Field 2" "State" $PLUGINPATH
    !insertmacro MUI_INSTALLOPTIONS_WRITE "FitsSetupDirectory.ini" "Field 3" "State" $ACTIONPATH
    !insertmacro MUI_INSTALLOPTIONS_WRITE "FitsSetupDirectory.ini" "Field 4" "State" $SCRIPTPATH
    !insertmacro MUI_INSTALLOPTIONS_WRITE "FitsSetupDirectory.ini" "Field 5" "Text" $(PAGE_DIRECTORY_TEXT)
    
; Init the Components page
    SectionSetFlags 1 25    ; Make the core component read-only and selected
FunctionEnd

; Finds all the paths and files that needs to be referenced
Function LoadEnvironment
    Push $R0
    Push $R1
    Push $R2
    
    ; Attempt to find Photoshop 8.0-?
    ${registry::Open} "HKLM\Software\Adobe\Photoshop" "" $0
    StrCmp $0 0 0 Photoshop
    ${registry::Close} "$0"
    Goto Elements

Photoshop:
    ${registry::Find} "$0" $1 $2 $3 $4
    StrCmp $2 "7.0" _ps7
    
    StrCpy $R0 "Software\Adobe\Photoshop\$2"
    !insertmacro READ_DIRECTORY $PSROOT HKLM $R0 "ApplicationPath"
    StrCpy $R0 "$R0\PluginPath"
    !insertmacro READ_DIRECTORY $PLUGINPATH HKLM $R0 ""
    
    Goto _load_ps
_ps7:
    !insertmacro READ_DIRECTORY $PSROOT HKLM "Software\Adobe\Photoshop\7.0\ApplicationPath" ""

_load_ps:  ; PRE: $PSROOT, $PLUGINPATH must be set
    StrCpy $PSPATH    "$PSROOT\Photoshop.exe"
    StrCpy $PLUGINPATH "$PLUGINPATH\File Formats"
    StrCpy $ACTIONPATH "$PSROOT\Presets\Photoshop Actions"
    StrCpy $SCRIPTPATH "$PSROOT\Presets\Scripts"
    
    ${registry::Close} "$0"
    
    ; Attempt to find Bridge
    ${registry::Open} "HKLM\Software\Adobe\Adobe Bridge" "" $0
    IntCmp $0 0 Close _bridge
    
_bridge:
    ${registry::Find} "$0" $1 $2 $3 $4   
    StrCpy $R0 "Software\Adobe\Adobe Bridge\$2\Installer"
    ${ReadDirectoryN} $BRIDGEROOT HKLM $R0 "InstallPath"
    Goto Close
    
Elements:
    ; Attempt to find Photoshop Elements 1.0-?
    ${registry::Open} "HKLM\Software\Adobe\Photoshop Elements" "" $0
    IntCmp $0 0 Close _load_elements

_load_elements:
    ${registry::Find} "$0" $1 $2 $3 $4
    StrCmp $2 "1.0" Close
    StrCpy $R0 "Software\Adobe\Photoshop Elements\$2"
    !insertmacro READ_DIRECTORY $PSROOT HKLM "$R0\ApplicationPath" ""
    StrCpy $R0 "$R0\PluginPath"
    !insertmacro READ_DIRECTORY $PLUGINPATH HKLM $R0 ""
    
    StrCpy $PSPATH "$PSROOT\PhotoshopElements.exe"
    StrCpy $PLUGINPATH "$PLUGINPATH\File Formats"
    StrCpy $ACTIONPATH "$PSROOT\Presets\Photoshop Actions"
    StrCpy $SCRIPTPATH "$PSROOT\Presets\Scripts"

    ; Verify that $PSPATH exists
    FindFirst $R1 $R2 $PSPATH
    FindClose $R1
    StrCmp $R2 "" +2    
    Goto Close
    
    StrCpy $R1 ".exe"
    StrCpy $PSPATH "$PSROOT\Photoshop Elements $2$R1"


    
Close:
    ${registry::Close} "$0"	
_exit:
    Pop $R2
    Pop $R1
    Pop $R0
FunctionEnd

; Called before starting an uninstall
Function un.onInit
    SetShellVarContext all
    
    Call un.GetParameters
    Pop $R0
    StrCmp $R0 "/S" Silent 0
    MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 $(MESSAGE_UNINSTALL_CONFIRM) IDYES +2
    Abort
Silent:
FunctionEnd

; Called after the uninstall is completed
Function un.onUninstSuccess
    Call un.GetParameters
    Pop $R0
    StrCmp $R0 "/S" Silent 0
    HideWindow
    MessageBox MB_ICONINFORMATION|MB_OK $(MESSAGE_UNINSTALL_SUCCESS)
Silent:
FunctionEnd

;-------------------------------------------------------------------------------
; Custom Page Callbacks
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; InitDirectory
;
; Called before showing the custom directories page
Function InitDirectory
    !insertmacro MUI_HEADER_TEXT "$(PAGE_DIRECTORY_TITLE)" "$(PAGE_DIRECTORY_SUBTITLE)"
    !insertmacro MUI_INSTALLOPTIONS_DISPLAY "FitsSetupDirectory.ini"
FunctionEnd

;-------------------------------------------------------------------------------
; ValidateDirectory
;
; Called to validate the things entered into the directories page. Use the
; Abort instruction to prevent the user from advancing to the next page
Function ValidateDirectory
    ; Read the (possibly modified) paths from the directory page
    !insertmacro MUI_INSTALLOPTIONS_READ $PSPATH "FitsSetupDirectory.ini" "Field 1" "State"
    !insertmacro MUI_INSTALLOPTIONS_READ $PLUGINPATH "FitsSetupDirectory.ini" "Field 2" "State"
    !insertmacro MUI_INSTALLOPTIONS_READ $ACTIONPATH "FitsSetupDirectory.ini" "Field 3" "State"
    !insertmacro MUI_INSTALLOPTIONS_READ $SCRIPTPATH "FitsSetupDirectory.ini" "Field 4" "State"
    
    ; Extract the Photoshop root folder
    Push $PSPATH
    Call GetParent
    Pop $PSROOT
FunctionEnd

;-------------------------------------------------------------------------------
; Utility Functions
;-------------------------------------------------------------------------------

; GetParameters
; input, none
; output, top of stack (replaces, with e.g. whatever)
; modifies no other variables.
Function un.GetParameters
    Push $R0
    Push $R1
    Push $R2
    Push $R3
    
    StrCpy $R2 1
    StrLen $R3 $CMDLINE
    
    ;Check for quote or space
    StrCpy $R0 $CMDLINE $R2
    StrCmp $R0 '"' 0 +3
    StrCpy $R1 '"'
    Goto loop
    StrCpy $R1 " "
    
loop:
    IntOp $R2 $R2 + 1
    StrCpy $R0 $CMDLINE 1 $R2
    StrCmp $R0 $R1 get
    StrCmp $R2 $R3 get
    Goto loop
    
get:
    IntOp $R2 $R2 + 1
    StrCpy $R0 $CMDLINE 1 $R2
    StrCmp $R0 " " get
    StrCpy $R0 $CMDLINE "" $R2
    
    Pop $R3
    Pop $R2
    Pop $R1
    Exch $R0
FunctionEnd

Function IsUserAdmin
    Push $R0
    Push $R1
    Push $R2
    
    ClearErrors
    UserInfo::GetName
    IfErrors Win9x
    Pop $R1
    UserInfo::GetAccountType
    Pop $R2
    
    StrCmp $R2 "Admin" 0 Continue
    ; Observation: I get here when running Win98SE. (Lilla)
    ; The functions UserInfo.dll looks for are there on Win98 too,
    ; but just don't work. So UserInfo.dll, knowing that admin isn't required
    ; on Win98, returns admin anyway. (per kichik)
    ; MessageBox MB_OK 'User "$R1" is in the Administrators group'
    StrCpy $R0 "true"
    Goto Done
Continue:
    ; You should still check for an empty string because the functions
    ; UserInfo.dll looks for may not be present on Windows 95. (per kichik)
    StrCmp $R2 "" Win9x
    StrCpy $R0 "false"
    ;MessageBox MB_OK 'User "$R1" is in the "$R2" group'
    Goto Done
Win9x:
    ; comment/message below is by UserInfo.nsi author:
    ; This one means you don't need to care about admin or
    ; not admin because Windows 9x doesn't either
    StrCpy $R0 "true"
Done:
    Pop $R2
    Pop $R1
    Exch $R0
FunctionEnd

;-------------------------------------------------------------------------------
; GetParent
; input, top of stack  (i.e. C:\Program Files\Poop)
; output, top of stack (replaces, with i.e. C:\Program Files)
; modifies no other variables.
;
; Usage:
;   Push "C:\Program Files\Directory\Whatever"
;   Call GetParent
;   Pop $0
;   ; at this point $0 will equal "C:\Program Files\Directory"
Function GetParent
    Exch $0 ; old $0 is on top of stack
    Push $1
    Push $2
    StrCpy $1 -1

    loop:
        StrCpy $2 $0 1 $1
        StrCmp $2 "" exit
        StrCmp $2 "\" exit
        IntOp $1 $1 - 1
    Goto loop

exit:
    StrCpy $0 $0 $1
    Pop $2
    Pop $1
    Exch $0 ; put $0 on top of stack, restore $0 to original value
FunctionEnd

; TrimNewlines
; input, top of stack  (e.g. whatever$\r$\n)
; output, top of stack (replaces, with e.g. whatever)
; modifies no other variables.
; Note: Inorder for this function to be used for the uninstall
; process, the name has to be prefixed with 'un.' - go figure...
Function un.TrimNewlines
    Exch $R0
    Push $R1
    Push $R2
    StrCpy $R1 0
    loop:
        IntOp $R1 $R1 - 1
        StrCpy $R2 $R0 1 $R1
        StrCmp $R2 "$\r" loop
        StrCmp $R2 "$\n" loop
        IntOp $R1 $R1 + 1
        IntCmp $R1 0 no_trim_needed
        StrCpy $R0 $R0 $R1

no_trim_needed:
    Pop $R2
    Pop $R1
    Exch $R0
FunctionEnd