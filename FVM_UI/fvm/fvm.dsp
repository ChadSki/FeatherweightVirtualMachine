# Microsoft Developer Studio Project File - Name="fvm" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=fvm - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "fvm.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "fvm.mak" CFG="fvm - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "fvm - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "fvm - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "fvm - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 Psapi.lib Iphlpapi.lib Ws2_32.lib /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /machine:I386 /out:"../fvm.exe"

!ELSEIF  "$(CFG)" == "fvm - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 Psapi.lib Iphlpapi.lib Ws2_32.lib /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /debug /machine:I386 /out:"C:\FVMBIN/fvm.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "fvm - Win32 Release"
# Name "fvm - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\BiSplitterWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfigDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\CreateDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\FileView.cpp
# End Source File
# Begin Source File

SOURCE=.\FilteredFolderDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\fvm.cpp
# End Source File
# Begin Source File

SOURCE=.\fvm.rc
# End Source File
# Begin Source File

SOURCE=.\FvmBrowser.cpp
# End Source File
# Begin Source File

SOURCE=.\fvmDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\fvmView.cpp
# End Source File
# Begin Source File

SOURCE=.\LeftView.cpp
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

SOURCE=.\ProcessView.cpp
# End Source File
# Begin Source File

SOURCE=.\RegView.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Utility.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\BiSplitterWnd.h
# End Source File
# Begin Source File

SOURCE=.\ConfigDialog.h
# End Source File
# Begin Source File

SOURCE=.\CreateDialog.h
# End Source File
# Begin Source File

SOURCE=.\FileView.h
# End Source File
# Begin Source File

SOURCE=.\FilteredFolderDlg.h
# End Source File
# Begin Source File

SOURCE=.\FolderDlg.h
# End Source File
# Begin Source File

SOURCE=.\fvm.h
# End Source File
# Begin Source File

SOURCE=.\FvmBrowser.h
# End Source File
# Begin Source File

SOURCE=.\fvmData.h
# End Source File
# Begin Source File

SOURCE=.\fvmdefine.h
# End Source File
# Begin Source File

SOURCE=.\fvmDoc.h
# End Source File
# Begin Source File

SOURCE=.\fvmView.h
# End Source File
# Begin Source File

SOURCE=.\LeftView.h
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

SOURCE=.\ProcessView.h
# End Source File
# Begin Source File

SOURCE=.\RegView.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\Utility.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\coldtool.bmp
# End Source File
# Begin Source File

SOURCE=.\res\commit.ico
# End Source File
# Begin Source File

SOURCE=.\res\configure.ico
# End Source File
# Begin Source File

SOURCE=.\res\copy.ico
# End Source File
# Begin Source File

SOURCE=.\res\create.ico
# End Source File
# Begin Source File

SOURCE=.\res\delete.ico
# End Source File
# Begin Source File

SOURCE=.\res\down.ico
# End Source File
# Begin Source File

SOURCE=.\res\empty.ico
# End Source File
# Begin Source File

SOURCE=.\res\file.ico
# End Source File
# Begin Source File

SOURCE=.\res\fvm.ico
# End Source File
# Begin Source File

SOURCE=.\res\fvm.rc2
# End Source File
# Begin Source File

SOURCE=.\res\fvmDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\hottoolb.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ico205.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico206.ico
# End Source File
# Begin Source File

SOURCE=.\res\Image1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\off.ico
# End Source File
# Begin Source File

SOURCE=.\res\on.ico
# End Source File
# Begin Source File

SOURCE=.\res\program.ico
# End Source File
# Begin Source File

SOURCE=.\res\refresh.ico
# End Source File
# Begin Source File

SOURCE=.\res\registry.ico
# End Source File
# Begin Source File

SOURCE=.\res\resume.ico
# End Source File
# Begin Source File

SOURCE=.\res\sehll.ico
# End Source File
# Begin Source File

SOURCE=.\res\sleep.ico
# End Source File
# Begin Source File

SOURCE=.\res\start.ico
# End Source File
# Begin Source File

SOURCE=.\res\suspend.ico
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\top.ico
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
