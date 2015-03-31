; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CFvmFileView
LastTemplate=CListCtrl
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "fvmshell.h"
LastPage=0

ClassCount=12
Class1=CFvmshellApp
Class2=CFvmshellDoc
Class3=CFvmDirView
Class4=CMainFrame

ResourceCount=12
Resource1=IDR_VIEW_DROP_DOWN
Class5=CAboutDlg
Class6=CFvmFileView
Resource2=IDR_SHORTCUT
Resource3=IDR_FILEVIEW
Resource4=IDD_ENTERSOMETHING
Resource5=IDR_MAINFRAME
Resource6=IDR_DIRVIEW
Class7=CShortCutView
Resource7=IDD_MAINFRAMEBAR
Class8=CShortcutCtrl
Resource8=IDD_SETTING
Class9=CSetting
Resource9=IDD_SETTING_VIEW
Class10=CSettingView
Filter=D
BaseClass=CDialog
VirtualFilter=dWC
Resource10=IDD_ABOUTBOX
Resource11=IDD_SAPREFS
Class11=CCreateShortcut
Class12=CShortcutListCtrl
Resource12=IDD_SHORTCUT

[CLS:CFvmshellApp]
Type=0
HeaderFile=fvmshell.h
ImplementationFile=fvmshell.cpp
Filter=N

[CLS:CFvmshellDoc]
Type=0
HeaderFile=fvmshellDoc.h
ImplementationFile=fvmshellDoc.cpp
Filter=N

[CLS:CFvmDirView]
Type=0
HeaderFile=fvmdirview.h
ImplementationFile=fvmdirlview.cpp
BaseClass=CView
Filter=C
VirtualFilter=VWC
LastObject=CFvmDirView

[CLS:CMainFrame]
Type=0
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=T
LastObject=CMainFrame
BaseClass=CFrameWnd
VirtualFilter=fWC




[CLS:CAboutDlg]
Type=0
HeaderFile=fvmshell.cpp
ImplementationFile=fvmshell.cpp
Filter=D
LastObject=CAboutDlg

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_SAVE_AS
Command5=ID_FILE_PRINT
Command6=ID_FILE_PRINT_PREVIEW
Command7=ID_FILE_PRINT_SETUP
Command8=ID_FILE_MRU_FILE1
Command9=ID_APP_EXIT
Command10=ID_EDIT_UNDO
Command11=ID_EDIT_CUT
Command12=ID_EDIT_COPY
Command13=ID_EDIT_PASTE
Command14=ID_VIEW_TOOLBAR
Command15=ID_VIEW_STATUS_BAR
Command16=ID_WINDOW_SPLIT
Command17=ID_MENU_LARGE_ICONS
Command18=ID_MENU_SMALL_ICONS
Command19=ID_MENU_LIST
Command20=ID_MENU_DETAILS
Command21=ID_FOLDER_OPTIONS
Command22=ID_APP_ABOUT
CommandCount=22

[ACL:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_PRINT
Command5=ID_EDIT_UNDO
Command6=ID_EDIT_CUT
Command7=ID_EDIT_COPY
Command8=ID_EDIT_PASTE
Command9=ID_EDIT_UNDO
Command10=ID_EDIT_CUT
Command11=ID_EDIT_COPY
Command12=ID_EDIT_PASTE
Command13=ID_NEXT_PANE
Command14=ID_PREV_PANE
CommandCount=14

[TB:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_CUT
Command5=ID_EDIT_COPY
Command6=ID_EDIT_PASTE
Command7=ID_FILE_PRINT
Command8=ID_APP_ABOUT
Command9=ID_TEMP
CommandCount=9

[CLS:CFvmFileView]
Type=0
HeaderFile=FvmFileView.h
ImplementationFile=FvmFileView.cpp
BaseClass=CView
Filter=C
LastObject=CFvmFileView
VirtualFilter=VWC

[MNU:IDR_DIRVIEW]
Type=1
Class=?
Command1=ID_DIRVIEW_DELETE
Command2=ID_DIRVIEW_RENAME
CommandCount=2

[DLG:IDD_ENTERSOMETHING]
Type=1
Class=?
ControlCount=4
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_EDIT1,edit,1350631552
Control4=IDC_TEXT,static,1342308352

[DLG:IDD_SAPREFS]
Type=1
Class=?
ControlCount=2
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816

[MNU:IDR_FILEVIEW]
Type=1
Class=?
Command1=ID_FILEVIEW_RENAME
Command2=ID_FILEVIEW_DELETE
Command3=ID_FILEVIEW_NEWFOLDER
Command4=ID_FILEVIEW_PROPERTIES
CommandCount=4

[DLG:IDD_MAINFRAMEBAR]
Type=1
Class=?
ControlCount=3
Control1=IDC_GO,button,1342242816
Control2=IDC_STATIC,static,1342308352
Control3=IDC_EDIT2,edit,1350631552

[CLS:CShortCutView]
Type=0
HeaderFile=ShortCutView.h
ImplementationFile=ShortCutView.cpp
BaseClass=CListView
Filter=C
LastObject=CShortCutView
VirtualFilter=VWC

[MNU:IDR_VIEW_DROP_DOWN]
Type=1
Class=?
Command1=ID_MENU_LARGE_ICONS
Command2=ID_MENU_SMALL_ICONS
Command3=ID_MENU_LIST
Command4=ID_MENU_DETAILS
CommandCount=4

[CLS:CShortcutCtrl]
Type=0
HeaderFile=ShortcutCtrl.h
ImplementationFile=ShortcutCtrl.cpp
BaseClass=CListCtrl
Filter=W
LastObject=CShortcutCtrl

[DLG:IDD_SETTING]
Type=1
Class=CSetting
ControlCount=3
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_TAB1,SysTabControl32,1342177280

[CLS:CSetting]
Type=0
HeaderFile=Setting.h
ImplementationFile=Setting.cpp
BaseClass=CDialog
Filter=D
LastObject=CSetting
VirtualFilter=dWC

[DLG:IDD_SETTING_VIEW]
Type=1
Class=CSettingView
ControlCount=4
Control1=IDC_VIEW_LARGE_ICONS,button,1342177289
Control2=IDC_VIEW_SMALL_ICONS,button,1342177289
Control3=IDC_VIEW_LIST,button,1342177289
Control4=IDC_VIEW1_DETAILS,button,1342177289

[CLS:CSettingView]
Type=0
HeaderFile=SettingView.h
ImplementationFile=SettingView.cpp
BaseClass=CDialog
Filter=D
LastObject=CSettingView

[MNU:IDR_SHORTCUT]
Type=1
Class=?
Command1=ID_SHORTCUT_CREATESHORTCUT
Command2=ID_SHORTCUT_DELETESHORTCUT
CommandCount=2

[DLG:IDD_SHORTCUT]
Type=1
Class=CCreateShortcut
ControlCount=6
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_SHORTCUT_BROWS,button,1342242816
Control4=IDC_SHORTCUT_EDIT,edit,1350631552
Control5=IDC_STATIC,static,1342308352
Control6=IDC_STATIC,static,1342308352

[CLS:CCreateShortcut]
Type=0
HeaderFile=CreateShortcut.h
ImplementationFile=CreateShortcut.cpp
BaseClass=CDialog
Filter=D
LastObject=CCreateShortcut
VirtualFilter=dWC

[CLS:CShortcutListCtrl]
Type=0
HeaderFile=ShortcutListCtrl.h
ImplementationFile=ShortcutListCtrl.cpp
BaseClass=CListCtrl
Filter=W
LastObject=CShortcutListCtrl
VirtualFilter=FWC

