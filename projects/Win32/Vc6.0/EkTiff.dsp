# Microsoft Developer Studio Project File - Name="EkTiff" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=EkTiff - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "EkTiff.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "EkTiff.mak" CFG="EkTiff - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "EkTiff - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "EkTiff - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "EkTiff - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G5 /MD /W3 /GR /GX /O2 /I "..\..\..\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\lib\EkTiff.lib"

!ELSEIF  "$(CFG)" == "EkTiff - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G5 /MDd /W3 /GR /GX /Z7 /Od /I "..\..\..\inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\lib\EkTiffD.lib"

!ENDIF 

# Begin Target

# Name "EkTiff - Win32 Release"
# Name "EkTiff - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
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

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
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

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
