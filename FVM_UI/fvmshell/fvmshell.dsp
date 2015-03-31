# Microsoft Developer Studio Project File - Name="fvmshell" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=fvmshell - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "fvmshell.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "fvmshell.mak" CFG="fvmshell - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "fvmshell - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "fvmshell - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "fvmshell - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /machine:I386 /out:"../fvmshell.exe"

!ELSEIF  "$(CFG)" == "fvmshell - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "fvmshell - Win32 Release"
# Name "fvmshell - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\BiSplitterWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\CreateShortcut.cpp
# End Source File
# Begin Source File

SOURCE=.\DirTreeCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\EnterSomething.cpp
# End Source File
# Begin Source File

SOURCE=.\FvmDirlView.cpp
# End Source File
# Begin Source File

SOURCE=.\FvmFileView.cpp
# End Source File
# Begin Source File

SOURCE=.\fvmshell.cpp
# End Source File
# Begin Source File

SOURCE=.\fvmshell.rc
# End Source File
# Begin Source File

SOURCE=.\fvmshellDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\LocalFileListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\MFC64bitFix.cpp
# End Source File
# Begin Source File

SOURCE=.\PathFunctions.cpp
# End Source File
# Begin Source File

SOURCE=.\Setting.cpp
# End Source File
# Begin Source File

SOURCE=.\SettingView.cpp
# End Source File
# Begin Source File

SOURCE=.\ShortCutView.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\misc\TransparentDialogBar.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\BiSplitterWnd.h
# End Source File
# Begin Source File

SOURCE=.\CreateShortcut.h
# End Source File
# Begin Source File

SOURCE=.\DirTreeCtrl.h
# End Source File
# Begin Source File

SOURCE=.\EnterSomething.h
# End Source File
# Begin Source File

SOURCE=.\FvmDirView.h
# End Source File
# Begin Source File

SOURCE=.\FvmFileView.h
# End Source File
# Begin Source File

SOURCE=.\fvmshell.h
# End Source File
# Begin Source File

SOURCE=.\fvmshellDoc.h
# End Source File
# Begin Source File

SOURCE=.\LocalFileListCtrl.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\MFC64bitFix.h
# End Source File
# Begin Source File

SOURCE=.\PathFunctions.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\Setting.h
# End Source File
# Begin Source File

SOURCE=.\SettingView.h
# End Source File
# Begin Source File

SOURCE=.\ShortCutView.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\misc\TransparentDialogBar.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\coldtool.bmp
# End Source File
# Begin Source File

SOURCE=.\res\down.ico
# End Source File
# Begin Source File

SOURCE=.\res\empty.ico
# End Source File
# Begin Source File

SOURCE=.\res\fvmshell.ico
# End Source File
# Begin Source File

SOURCE=.\res\fvmshell.rc2
# End Source File
# Begin Source File

SOURCE=.\res\fvmshellDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\hottoolb.bmp
# End Source File
# Begin Source File

SOURCE=".\res\shell-large.ico"
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\toolbar1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\up.ico
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
