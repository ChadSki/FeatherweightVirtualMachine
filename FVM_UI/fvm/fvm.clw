; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CConfigDialog
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "fvm.h"
LastPage=0

ClassCount=11
Class1=CCreateDialog
Class2=CFileView
Class3=CFvmApp
Class4=CAboutDlg
Class5=CFvmBrowser
Class6=CFvmDoc
Class7=CFvmView
Class8=CLocalFileListCtrl
Class9=CMainFrame
Class10=CProcessView

ResourceCount=7
Resource1=IDR_PROCESSVIEW
Resource2=IDD_DIALOG1
Resource3=IDD_CREATE_DIALOG
Class11=CConfigDialog
Resource4=IDD_ABOUTBOX
Resource5=IDR_MAINFRAME
Resource6="IDD_CONFIG_DIALOG"
Resource7=IDD_CONFIG_DIALOG

[CLS:CCreateDialog]
Type=0
BaseClass=CDialog
HeaderFile=CreateDialog.h
ImplementationFile=CreateDialog.cpp
Filter=D
VirtualFilter=dWC

[CLS:CFileView]
Type=0
BaseClass=CView
HeaderFile=FileView.h
ImplementationFile=FileView.cpp

[CLS:CFvmApp]
Type=0
BaseClass=CWinApp
HeaderFile=fvm.h
ImplementationFile=fvm.cpp

[CLS:CAboutDlg]
Type=0
BaseClass=CDialog
HeaderFile=fvm.cpp
ImplementationFile=fvm.cpp
LastObject=CAboutDlg

[CLS:CFvmBrowser]
Type=0
BaseClass=CListView
HeaderFile=FvmBrowser.h
ImplementationFile=FvmBrowser.cpp

[CLS:CFvmDoc]
Type=0
BaseClass=CDocument
HeaderFile=fvmDoc.h
ImplementationFile=fvmDoc.cpp

[CLS:CFvmView]
Type=0
BaseClass=CView
HeaderFile=fvmView.h
ImplementationFile=fvmView.cpp

[CLS:CLocalFileListCtrl]
Type=0
BaseClass=CListCtrl
HeaderFile=LocalFileListCtrl.h
ImplementationFile=LocalFileListCtrl.cpp

[CLS:CMainFrame]
Type=0
BaseClass=CFrameWnd
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp

[CLS:CProcessView]
Type=0
BaseClass=CListView
HeaderFile=ProcessView.h
ImplementationFile=ProcessView.cpp
Filter=C
VirtualFilter=VWC
LastObject=CProcessView

[DLG:IDD_CREATE_DIALOG]
Type=1
Class=CCreateDialog
ControlCount=11
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308352
Control4=IDC_STATIC,static,1342308352
Control5=IDC_FVM_NAME,edit,1350631552
Control6=IDC_FVM_FOLDER,edit,1350631552
Control7=IDC_BUTTON1,button,1342242816
Control8=IDC_STATIC,static,1342308352
Control9=IDC_FVM_IP,edit,1350631552
Control10=IDC_STATIC,static,1342308352
Control11=IDC_FVM_IP_MASK,edit,1350631552

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

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
CommandCount=8

[MNU:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_CREATE
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
Command17=ID_APP_ABOUT
CommandCount=17

[ACL:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_EDIT_COPY
Command2=ID_CREATE
Command3=ID_FILE_OPEN
Command4=ID_FILE_PRINT
Command5=ID_FILE_SAVE
Command6=ID_EDIT_PASTE
Command7=ID_EDIT_UNDO
Command8=ID_EDIT_CUT
Command9=ID_NEXT_PANE
Command10=ID_PREV_PANE
Command11=ID_EDIT_COPY
Command12=ID_EDIT_PASTE
Command13=ID_EDIT_CUT
Command14=ID_EDIT_UNDO
CommandCount=14

[MNU:IDR_PROCESSVIEW]
Type=1
Class=?
Command1=ID_PROCESS_KILL
Command2=ID_PROCESS_ENDPROCESSTREE
Command3=ID_PROCESS_PROPERTIES
CommandCount=3

[CLS:CConfigDialog]
Type=0
HeaderFile=ConfigDialog.h
ImplementationFile=ConfigDialog.cpp
BaseClass=CDialog
Filter=D
LastObject=CConfigDialog

[DLG:IDD_DIALOG1]
Type=1
Class=?
ControlCount=2
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816

[DLG:"IDD_CONFIG_DIALOG"]
Type=1
Class=?
ControlCount=10
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308352
Control4=IDC_STATIC,static,1342308352
Control5=IDC_FVM_NAME,edit,1350631552
Control6=IDC_FVM_FOLDER,edit,1350631552
Control7=IDC_STATIC,static,1342308352
Control8=IDC_FVM_IP,edit,1350631552
Control9=IDC_STATIC,static,1342308352
Control10=IDC_FVM_IP_MASK,edit,1350631552

[DLG:IDD_CONFIG_DIALOG]
Type=1
Class=?
ControlCount=10
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308352
Control4=IDC_STATIC,static,1342308352
Control5=IDC_MAX_PROCESSES,edit,1350631552
Control6=IDC_STATIC,static,1342308352
Control7=IDC_MAX_COMMITTED,edit,1350631552
Control8=IDC_STATIC,static,1342308352
Control9=IDC_MAX_WORK_SET,edit,1350631552
Control10=IDC_SCHED_PRIORITY,listbox,1352728833

