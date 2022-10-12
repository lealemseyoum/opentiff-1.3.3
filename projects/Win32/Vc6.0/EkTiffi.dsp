# Microsoft Developer Studio Project File - Name="EkTiffi" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=EkTiffi - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "EkTiffi.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "EkTiffi.mak" CFG="EkTiffi - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "EkTiffi - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "EkTiffi - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "EkTiffi - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "EkTiffi___Win32_Release"
# PROP BASE Intermediate_Dir "EkTiffi___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Releasei"
# PROP Intermediate_Dir "Releasei"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "EKTIFFI_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "..\..\..\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "EKTIFF_BUILD_DLL" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"../../../lib/EkTiffi.dll" /implib:"../../../lib/EkTiffi.lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "EkTiffi - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "EkTiffi___Win32_Debug"
# PROP BASE Intermediate_Dir "EkTiffi___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debugi"
# PROP Intermediate_Dir "Debugi"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "EKTIFFI_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\..\..\inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "EKTIFF_BUILD_DLL" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"../../../lib/EkTiffid.dll" /implib:"../../../lib/EkTiffid.lib" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "EkTiffi - Win32 Release"
# Name "EkTiffi - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\src\TiffCodec.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\TiffCodecJpeg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\TiffCodecNone.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\TiffCodecPackBits.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\TiffDirectory.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\TiffFile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\TiffFileIO.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\TiffImage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\TiffImageFile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\TiffInternetIO.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\TiffIO.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\TiffMemMapIO.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\TiffMemoryIO.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\TiffStripImage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\TiffTileImage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\TiffUtil.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\inc\TiffCodec.h
# End Source File
# Begin Source File

SOURCE=..\..\..\inc\TiffCodecJpeg.h
# End Source File
# Begin Source File

SOURCE=..\..\..\inc\TiffCodecNone.h
# End Source File
# Begin Source File

SOURCE=..\..\..\inc\TiffCodecPackBits.h
# End Source File
# Begin Source File

SOURCE=..\..\..\inc\TiffComp.h
# End Source File
# Begin Source File

SOURCE=..\..\..\inc\TiffConf.h
# End Source File
# Begin Source File

SOURCE=..\..\..\inc\TiffDefs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\inc\TiffDirectory.h
# End Source File
# Begin Source File

SOURCE=..\..\..\inc\TiffError.h
# End Source File
# Begin Source File

SOURCE=..\..\..\inc\TiffFile.h
# End Source File
# Begin Source File

SOURCE=..\..\..\inc\TiffFileIO.h
# End Source File
# Begin Source File

SOURCE=..\..\..\inc\TiffImage.h
# End Source File
# Begin Source File

SOURCE=..\..\..\inc\TiffImageFile.h
# End Source File
# Begin Source File

SOURCE=..\..\..\inc\TiffInternetIO.h
# End Source File
# Begin Source File

SOURCE=..\..\..\inc\TiffIO.h
# End Source File
# Begin Source File

SOURCE=..\..\..\inc\TiffMemMapIO.h
# End Source File
# Begin Source File

SOURCE=..\..\..\inc\TiffMemoryIO.h
# End Source File
# Begin Source File

SOURCE=..\..\..\inc\TiffStripImage.h
# End Source File
# Begin Source File

SOURCE=..\..\..\inc\TiffTagDefs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\inc\TiffTagEntry.h
# End Source File
# Begin Source File

SOURCE=..\..\..\inc\TiffTileImage.h
# End Source File
# Begin Source File

SOURCE=..\..\..\inc\TiffTypeDefs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\inc\TiffUtil.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
