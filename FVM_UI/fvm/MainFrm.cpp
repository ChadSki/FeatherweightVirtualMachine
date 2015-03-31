// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "fvm.h"

#include "MainFrm.h"
#include "FileView.h"
#include "FvmBrowser.h"
#include "ProcessView.h"
#include "LeftView.h"
#include "RegView.h"
#include "CreateDialog.h"
#include "ConfigDialog.h"
#include "fvmdefine.h"
#include <Psapi.h>
#include <tlhelp32.h>
#include <io.h> 
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "rpcdce.h"
//#include "UuidString.h"

//#include <stdlib.h>
#include <Winsock2.h>

#include <stdio.h>
#include <windows.h>
#include "iphlpapi.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_CREATE, OnCreateFvm)
	ON_COMMAND(ID_COPY, OnCopy)
	ON_COMMAND(ID_CONFIG, OnConfig)
	ON_COMMAND(ID_START, OnStart)
	ON_COMMAND(ID_STOP, OnStop)
	ON_COMMAND(ID_SUSPEND, OnSuspend)
	ON_COMMAND(ID_RESUME, OnResume)
	ON_COMMAND(ID_DELETE, OnDelete)
	ON_COMMAND(ID_COMMIT, OnCommit)
	ON_COMMAND(ID_PROCESS, OnProcess)
	ON_COMMAND(ID_FILE, OnFile)
	ON_COMMAND(ID_REGISTRY, OnRegistry)	
	ON_COMMAND(ID_SHELL, OnShell)
	ON_COMMAND(ID_REFRESH_VIEWS, OnRefresh)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	fvmData = NULL;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

			
	if (!CreateToolbars())
		return -1;
	/*
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	*/
	EnableDocking(CBRS_ALIGN_ANY);
	//DockControlBar(&m_wndToolBar);

	return 0;
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/,
	CCreateContext* pContext)
{

	if (!m_wndBiSplitter.Create(this, VSPLITTER | FIXED0 | VIEW3D ))
			return FALSE;


	//m_wndBiSplitter.CreateView(RUNTIME_CLASS(CFvmBrowser), 1, pContext);
	m_wndBiSplitter.CreateView(RUNTIME_CLASS(CBiSplitterWnd), 1, HSPLITTER | FIXED0 | VIEW3D);

	m_wndBiSplitter.CreateView(RUNTIME_CLASS(CBiSplitterWnd), 2, HSPLITTER | FIXED0 | VIEW3D);

	
	if (!m_wndBiSplitter.CreatePane(0, FLAT_BUTTON, NULL, m_wndBiSplitter.GetView(1))
		||
		!m_wndBiSplitter.CreatePane(1, FLAT_BUTTON, NULL
			,m_wndBiSplitter.GetView(2))
		)
	{
		m_wndBiSplitter.DestroyWindow();
		return FALSE;
	}
	
	
	//CBiSplitterWnd *
	pSplitterWnd3 = (CBiSplitterWnd *)m_wndBiSplitter.GetPaneView(0);
	pSplitterWnd3->CreateView(RUNTIME_CLASS(CFvmBrowser), 11, pContext);
	pSplitterWnd3->CreateView(RUNTIME_CLASS(CProcessView), 12, pContext);
	//pSplitterWnd3->CreateView(RUNTIME_CLASS(CBiSplitterWnd), 12, HSPLITTER | FIXED0 | VIEW3D);

	if (!pSplitterWnd3->CreatePane(0, THIN_BORDER | /*SMCAPTION |*/ THIN_CAPTIONBORDER | FLAT_BUTTON |GRIPBTN
			,/*_T("Virtual Machines")*/NULL, pSplitterWnd3->GetView(11))
		||
		!pSplitterWnd3->CreatePane(1,  THIN_BORDER | SMCAPTION | THIN_CAPTIONBORDER | FLAT_BUTTON |  CLOSEBTN, 
		_T("Running Process"), pSplitterWnd3->GetView(12))
		)
	{
		pSplitterWnd3->DestroyWindow();
		return FALSE;
	}

	
	//CBiSplitterWnd *
	pSplitterWnd4 = (CBiSplitterWnd *)m_wndBiSplitter.GetPaneView(1);
	pSplitterWnd4->CreateView(RUNTIME_CLASS(CFileView), 21, pContext);
	pSplitterWnd4->CreateView(RUNTIME_CLASS(CBiSplitterWnd), 22, VSPLITTER | FIXED0 | VIEW3D);
	//pSplitterWnd4->CreateView(RUNTIME_CLASS(CRegistryView), 22, pContext);
	if (!pSplitterWnd4->CreatePane(0, THIN_BORDER | SMCAPTION | THIN_CAPTIONBORDER | FLAT_BUTTON | GRIPBTN | CLOSEBTN
			,_T("File View"), pSplitterWnd4->GetView(21))
		||

		!pSplitterWnd4->CreatePane(1, THIN_BORDER | THIN_CAPTIONBORDER | SMCAPTION |  FLAT_BUTTON | CLOSEBTN, 
			_T("Registry View")
			,pSplitterWnd4->GetView(22)	
			)
		)
	{
		pSplitterWnd4->DestroyWindow();
		return FALSE;
	}


	pSplitterWnd5 = (CBiSplitterWnd *)pSplitterWnd4->GetPaneView(1);
	pSplitterWnd5->CreateView(RUNTIME_CLASS(CRegView), 31, pContext);
	pSplitterWnd5->CreateView(RUNTIME_CLASS(CLeftView), 32, pContext);
	if (!pSplitterWnd5->CreatePane(0, THIN_BORDER | THIN_CAPTIONBORDER | FLAT_BUTTON | GRIPBTN
			,_T("File View"), pSplitterWnd5->GetView(32))
		||

		!pSplitterWnd5->CreatePane(1, THIN_BORDER | THIN_CAPTIONBORDER |  FLAT_BUTTON,
			_T("Registry View")
			,pSplitterWnd5->GetView(31)	
			)
		)
	{
		pSplitterWnd5->DestroyWindow();
		return FALSE;
	}



	m_wndBiSplitter.SetSplitterPos(260);
	m_wndBiSplitter.SetSplitterGap(4);	
	pSplitterWnd3->SetSplitterPos(250);	
	pSplitterWnd3->SetSplitterGap(4);

	pSplitterWnd4->SetSplitterPos(250);
	pSplitterWnd4->SetSplitterGap(4);
	pSplitterWnd5->SetSplitterPos(350);


	GetFileView()->m_Parent = this;
	GetFvmBrowserView()->m_Parent = this;
	GetLeftView()->m_Parent = this;
	GetRegView()->m_Parent = this;

//	GetFileView()->m_pListCtrl->DisplayFiles(L"m:\\vm1");
	return TRUE;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


BOOL CMainFrame::CreateToolbars()
{

		CString str;

	m_imageList.Create ( 22, 20, ILC_COLOR32 | ILC_MASK, 4, 1 );

	m_imageList.Add (AfxGetApp()->LoadIcon(IDI_CREATE));
	m_imageList.Add (AfxGetApp()->LoadIcon(IDI_COPY));
	m_imageList.Add (AfxGetApp()->LoadIcon(IDI_CONFIG));

	m_imageList.Add (AfxGetApp()->LoadIcon(IDI_START));
	m_imageList.Add (AfxGetApp()->LoadIcon(IDI_STOP));

	m_imageList.Add (AfxGetApp()->LoadIcon(IDI_SUSPEND));
	m_imageList.Add (AfxGetApp()->LoadIcon(IDI_RESUME));

	m_imageList.Add (AfxGetApp()->LoadIcon(IDI_COMMIT));
	m_imageList.Add (AfxGetApp()->LoadIcon(IDI_DELETE));
	m_imageList.Add (AfxGetApp()->LoadIcon(IDI_PROCESS));

	m_imageList.Add (AfxGetApp()->LoadIcon(IDI_FILE));
	m_imageList.Add (AfxGetApp()->LoadIcon(IDI_REGISTRY));
	m_imageList.Add (AfxGetApp()->LoadIcon(IDI_SHELL));


	m_imageList.Add (AfxGetApp()->LoadIcon(IDI_REFRESH));


	if (!m_wndReBar.Create(this))
	{
		TRACE0("Failed to create rebar\n");
		return -1;      // fail to create
	}

	if (!m_wndToolBar.CreateEx(this))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	// set up toolbar properties
	m_wndToolBar.GetToolBarCtrl().SetButtonWidth(50, 150);
	m_wndToolBar.GetToolBarCtrl().SetExtendedStyle(TBSTYLE_EX_DRAWDDARROWS);


	m_wndToolBar.GetToolBarCtrl().SetHotImageList(&m_imageList);
	
	m_wndToolBar.GetToolBarCtrl().SetImageList(&m_imageList);

	m_imageList.Detach();
	m_wndToolBar.ModifyStyle(0, TBSTYLE_FLAT | TBSTYLE_TRANSPARENT);
	m_wndToolBar.SetButtons(NULL, 16);




	m_wndToolBar.SetButtonInfo(0, ID_CREATE, TBSTYLE_BUTTON, 0);
	str.LoadString(IDS_CREATE);
	m_wndToolBar.SetButtonText(0, str);

	m_wndToolBar.SetButtonInfo(1, ID_COPY, TBSTYLE_BUTTON, 1);
	str.LoadString(IDS_COPY);
	m_wndToolBar.SetButtonText(1, str);

	m_wndToolBar.SetButtonInfo(2, ID_CONFIG, TBSTYLE_BUTTON, 2);
	str.LoadString(IDS_CONFIG);
	m_wndToolBar.SetButtonText(2, str);

	m_wndToolBar.SetButtonInfo(3, ID_SEPARATOR, TBBS_SEPARATOR, 0);


	m_wndToolBar.SetButtonInfo(4, ID_START, TBSTYLE_BUTTON, 3);
	str.LoadString(IDS_START);
	m_wndToolBar.SetButtonText(4, str);

	m_wndToolBar.SetButtonInfo(5, ID_STOP, TBSTYLE_BUTTON, 4);
	str.LoadString(IDS_STOP);
	m_wndToolBar.SetButtonText(5, str);

	m_wndToolBar.SetButtonInfo(6, ID_SEPARATOR, TBBS_SEPARATOR, 0);


	m_wndToolBar.SetButtonInfo(7, ID_SUSPEND, TBSTYLE_BUTTON, 5);
	str.LoadString(IDS_SUSPEND);
	m_wndToolBar.SetButtonText(7, str);

	m_wndToolBar.SetButtonInfo(8, ID_RESUME, TBSTYLE_BUTTON, 6);
	str.LoadString(IDS_RESUME);
	m_wndToolBar.SetButtonText(8, str);

	m_wndToolBar.SetButtonInfo(9, ID_SEPARATOR, TBBS_SEPARATOR, 0);


	m_wndToolBar.SetButtonInfo(10, ID_COMMIT, TBSTYLE_BUTTON, 7);
	str.LoadString(IDS_COMMIT);
	m_wndToolBar.SetButtonText(10, str);

	m_wndToolBar.SetButtonInfo(11, ID_DELETE, TBSTYLE_BUTTON, 8);
	str.LoadString(IDS_DELETE);
	m_wndToolBar.SetButtonText(11, str);

	m_wndToolBar.SetButtonInfo(12, ID_SEPARATOR, TBBS_SEPARATOR, 0);


	m_wndToolBar.SetButtonInfo(13, ID_PROCESS, TBSTYLE_BUTTON, 9);
	str.LoadString(IDS_PROCESS);
	m_wndToolBar.SetButtonText(13, str);
	
	m_wndToolBar.SetButtonInfo(14, ID_FILE, TBSTYLE_BUTTON, 10);
	str.LoadString(IDS_FILE);
	m_wndToolBar.SetButtonText(14, str);

	m_wndToolBar.SetButtonInfo(15, ID_REGISTRY, TBSTYLE_BUTTON, 11);
	str.LoadString(IDS_REGISTRY);
	m_wndToolBar.SetButtonText(15, str);

	m_wndToolBar.SetButtonInfo(16, ID_SEPARATOR, TBBS_SEPARATOR, 0);


	m_wndToolBar.SetButtonInfo(17, ID_SHELL, TBSTYLE_BUTTON, 12);
	str.LoadString(IDS_SHELL);
	m_wndToolBar.SetButtonText(17, str);

	m_wndToolBar.SetButtonInfo(18, ID_REFRESH_VIEWS, TBSTYLE_BUTTON, 13);
	str.LoadString(IDS_REFRESH_VIEWS);
	m_wndToolBar.SetButtonText(18, str);


	CRect rectToolBar;

	// set up toolbar button sizes
	m_wndToolBar.GetItemRect(0, &rectToolBar);
	m_wndToolBar.SetSizes(rectToolBar.Size(), CSize(30,20));

	m_wndReBar.AddBar(&m_wndToolBar,NULL, NULL, RBBS_GRIPPERALWAYS|RBBS_FIXEDBMP|RBBS_USECHEVRON);

	REBARBANDINFO rbbi;

	rbbi.cbSize = sizeof(rbbi);
	rbbi.fMask = RBBIM_CHILDSIZE | RBBIM_IDEALSIZE | RBBIM_SIZE;
	rbbi.cxMinChild = rectToolBar.Width();
	rbbi.cyMinChild = rectToolBar.Height();
	rbbi.cx = rbbi.cxIdeal = rectToolBar.Width() * 9;
	m_wndReBar.GetReBarCtrl().SetBandInfo(0, &rbbi);
	rbbi.cxMinChild = 0;

	CRect rectAddress;

	/*
	rbbi.fMask = RBBIM_CHILDSIZE | RBBIM_IDEALSIZE;
	m_wndAddress.GetEditCtrl()->GetWindowRect(&rectAddress);
	rbbi.cyMinChild = rectAddress.Height() + 10;
	rbbi.cxIdeal = 200;
	m_wndReBar.GetReBarCtrl().SetBandInfo(2, &rbbi);
*/
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_FIXED);

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

		return TRUE;
}


void CMainFrame::OnCreateFvm()
{
/*
	GUID id1;
	unsigned char mask = (unsigned char) 0x0f;
	unsigned char *idptr;
	int i;

	TCHAR buffer[1024];
  
	CoCreateGuid(&id1); 
	StringFromGUID2(id1, buffer, 1024); 
	CString str = buffer;
	str.TrimLeft('{');
	str.TrimRight('}');
	str.Remove('-');
	MessageBox(str);

*/


	CCreateDialog cd(this);
	cd.DoModal();

}






//----------------------------------------------------------------------------------
//  free buffer
//----------------------------------------------------------------------------------
__forceinline void FreeBuff(TCHAR ** Buff)
{
	if (*Buff) {
		free(*Buff);
		*Buff = NULL;
	}
}

//----------------------------------------------------------------------------------
//  allocate buffer
//----------------------------------------------------------------------------------
__forceinline void AllocBuff(TCHAR ** Buff, DWORD BuffSize)
{
	FreeBuff(Buff);
	*Buff = (TCHAR*)malloc(BuffSize);
}

//----------------------------------------------------------------------------------
//  copy key to new position
//----------------------------------------------------------------------------------
LONG RegCopyKey(HKEY SrcKey, HKEY TrgKey, LPCTSTR TrgSubKeyName)
{

	HKEY	SrcSubKey;
	HKEY	TrgSubKey;
	int		ValEnumIndx=0;
	int		KeyEnumIndx=0;
	TCHAR	ValName[MAX_PATH+1];
	TCHAR	KeyName[MAX_PATH+1];
	DWORD	size;	
	DWORD	VarType;
	DWORD	BuffSize;
	TCHAR	*	Buff=NULL;
	LONG	Err;
	DWORD	KeyDisposition;
	FILETIME LastWriteTime; 

	// create target key
	if (RegCreateKeyEx(TrgKey,TrgSubKeyName,NULL,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&TrgSubKey,&KeyDisposition) != ERROR_SUCCESS)
		return GetLastError();

	do {
		do {
			// read value from source key
			Err = ERROR_NOT_ENOUGH_MEMORY;
			BuffSize = 1024;
			do {						 
				AllocBuff(&Buff,BuffSize);
				size=MAX_PATH+1;
				Err = RegEnumValue(SrcKey,ValEnumIndx,ValName,&size,NULL,&VarType, (unsigned char *)Buff,&BuffSize);
				if ((Err != ERROR_SUCCESS) && (Err != ERROR_NO_MORE_ITEMS))
					Err = GetLastError();
			} while (Err == ERROR_NOT_ENOUGH_MEMORY);

			// done copying this key
			if (Err == ERROR_NO_MORE_ITEMS)
				break;

			// unknown error return
			if (Err != ERROR_SUCCESS)
				goto quit_err;

			// write value to target key
			if (RegSetValueEx(TrgSubKey,ValName,NULL,VarType, (const unsigned char *)Buff,BuffSize) != ERROR_SUCCESS)
				goto quit_get_err;

			// read next value
			ValEnumIndx++;
		} while (true);

		// free buffer
		FreeBuff(&Buff);

		// if copying under the same key avoid endless recursions
		do {
			// enum sub keys
			size=MAX_PATH+1;
			Err = RegEnumKeyEx(SrcKey,KeyEnumIndx++,KeyName,&size,NULL,NULL,NULL,&LastWriteTime);
		} while ((SrcKey == TrgKey) && !wcsncmp(KeyName,TrgSubKeyName,wcslen(KeyName)) && (Err == ERROR_SUCCESS));

		// done copying this key		
		if (Err == ERROR_NO_MORE_ITEMS)
			break;

		// unknown error return
		if (Err != ERROR_SUCCESS)
			goto quit_get_err;

		// open the source subkey
		if (RegOpenKeyEx(SrcKey,KeyName,NULL,KEY_ALL_ACCESS,&SrcSubKey) != ERROR_SUCCESS)
			goto quit_get_err;

		// recurs with the subkey
		if ((Err = RegCopyKey(SrcSubKey, TrgSubKey, KeyName)) != ERROR_SUCCESS)
			break;

		if (RegCloseKey(SrcSubKey) != ERROR_SUCCESS)
			goto quit_get_err;
	} while (true);

// normal quit
quit_err:
	FreeBuff(&Buff);
	RegCloseKey(TrgSubKey);
	if (Err == ERROR_NO_MORE_ITEMS)
		return ERROR_SUCCESS;	
	else
		return Err;

// abnormal quit
quit_get_err:
	FreeBuff(&Buff);
	RegCloseKey(TrgSubKey);
	return GetLastError();
}


void FvmCopyFiles(CString ol, CString ne)
{

	CString tempPath = ol+L"\\*.*";
	WIN32_FIND_DATA find;
	HANDLE hFind = FindFirstFile(tempPath, &find);

	while ( hFind!=INVALID_HANDLE_VALUE)
	{
		if (!(find.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY && (!_tcscmp(find.cFileName, _T("..")) || !_tcscmp(find.cFileName, _T(".")))))
		{
			CString on;
			CString nn;
	
			on = ol+L"\\"+find.cFileName;
			nn = ne+L"\\"+find.cFileName;

			if ( find.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			{
				CreateDirectory(nn, NULL);
				FvmCopyFiles(on,nn);
			}
			else
			{
				
				CopyFile(on,nn,FALSE);

			}
			
				
				
		}
		if (!FindNextFile(hFind, &find))
		{
			FindClose(hFind);
			hFind=0;
			break;
		}		
	}		

}


void CMainFrame::OnCopy()
{
	CFvmBrowser *fb = GetFvmBrowserView();
	CListCtrl& lc = fb->GetListCtrl();

	POSITION pos = lc.GetFirstSelectedItemPosition();

	//m_selectedFvm = NULL;
	if (pos != NULL)
	{
		while (pos)
		{
			int nItem = lc.GetNextSelectedItem(pos);
			CString vmname = lc.GetItemText(nItem, 0);
		
			t_FvmData * fvmd  = FvmDataFind(vmname);
			if(fvmd->status >= 1){
				MessageBox(vmname+L" is running. Please stop it first.", L"Copy FVM error.");
				return;
			}

			created=NULL;
			CCreateDialog cd(this,L"Copy FVM");
			//cd.SetWindowText(L"Copy a FVM");
			cd.DoModal();
			if(created){
				CString old = fvmd->fvmRoot+L"\\"+fvmd->fvmID;
				CString newfvm = created->fvmRoot+L"\\"+created->fvmID;
				FvmCopyFiles(old,newfvm);
				old = fvmd->fvmRoot+L"\\"+fvmd->fvmID+L".bin";
				newfvm = created->fvmRoot+L"\\"+created->fvmID+L".bin";
				CopyFile(old, newfvm,FALSE);
					
			
				HKEY SrcKey;
				HKEY TrgKey;

				RegOpenKeyEx(HKEY_CURRENT_USER,L"fvms\\"+fvmd->fvmID, 0, KEY_READ, &SrcKey);
				RegOpenKeyEx(HKEY_CURRENT_USER,L"fvms", 0, KEY_READ, &TrgKey);
				//RegCreateKeyEx(HKEY_CURRENT_USER,L"fvms\\"+created->fvmID, 0,NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,&TrgKey,NULL);

				if (RegCopyKey(SrcKey, TrgKey, created->fvmID) == ERROR_SUCCESS)
					; // All went okay
				else
					; // Something went wrong
			}
		}
	}
}

void CMainFrame::OnConfig()
{
	CFvmBrowser * vmb = GetFvmBrowserView();
	CListCtrl& lc = vmb->GetListCtrl();
	POSITION pos = lc.GetFirstSelectedItemPosition();
	if (pos == NULL)
	{
		MessageBox(L"Please choose a vm first.");
		return;
	}

	CConfigDialog cd(this,pos);
	cd.DoModal();
}

BOOL CMainFrame::CreateVmJobObject(PROCESS_INFORMATION &pi, t_FvmData * fvmd)
{
	long   TimeLimit=0, MemoryLimit=0;
	BOOL   b_set;

	STARTUPINFO si={sizeof(si)};
	SECURITY_ATTRIBUTES attr = {sizeof(SECURITY_ATTRIBUTES),NULL,TRUE};
    fvmd->hObject = CreateJobObject(&attr,NULL);
	if(fvmd->hObject==NULL){
		//OutputDebugString(L"CreateVmJobObject failed!");
		return false;
	}

  if(fvmd->schedPriority!="&")
  {
	  JOBOBJECT_EXTENDED_LIMIT_INFORMATION   JobExtendLimit; //设置作业进程的扩展限制   
	  JobExtendLimit.BasicLimitInformation.LimitFlags=
			JOB_OBJECT_LIMIT_PRIORITY_CLASS|
			JOB_OBJECT_LIMIT_ACTIVE_PROCESS|   
			JOB_OBJECT_LIMIT_JOB_MEMORY| 
			JOB_OBJECT_LIMIT_WORKINGSET;//|JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
	  if(fvmd->schedPriority=="IDLE_PRIORITY_CLASS")
		JobExtendLimit.BasicLimitInformation.PriorityClass=IDLE_PRIORITY_CLASS;
	  else if(fvmd->schedPriority=="BELOW_NORMAL_PRIORITY_CLASS")
		JobExtendLimit.BasicLimitInformation.PriorityClass=BELOW_NORMAL_PRIORITY_CLASS;
	  else if(fvmd->schedPriority=="NORMAL_PRIORITY_CLASS")
		JobExtendLimit.BasicLimitInformation.PriorityClass=NORMAL_PRIORITY_CLASS;
	  else if(fvmd->schedPriority=="ABOVE_NORMAL_PRIORITY_CLASS")
		JobExtendLimit.BasicLimitInformation.PriorityClass=ABOVE_NORMAL_PRIORITY_CLASS;
	  else if(fvmd->schedPriority=="HIGH_PRIORITY_CLASS")
		JobExtendLimit.BasicLimitInformation.PriorityClass=HIGH_PRIORITY_CLASS;
	  else if(fvmd->schedPriority=="REALTIME_PRIORITY_CLASS")
		JobExtendLimit.BasicLimitInformation.PriorityClass=REALTIME_PRIORITY_CLASS;
	  else{
		  OutputDebugString(L"schedPriority error");
		  return false;
	  }


	  JobExtendLimit.BasicLimitInformation.ActiveProcessLimit=_ttoi(fvmd->maxProcesses.GetBuffer(fvmd->maxProcesses.GetLength()));
	  fvmd->maxProcesses.ReleaseBuffer();
	  JobExtendLimit.BasicLimitInformation.MaximumWorkingSetSize=(_ttoi(fvmd->maxWorkSet.GetBuffer(fvmd->maxWorkSet.GetLength()))+1)*1024*1024;
	  fvmd->maxWorkSet.ReleaseBuffer();
	  JobExtendLimit.BasicLimitInformation.MinimumWorkingSetSize=0;
	  JobExtendLimit.JobMemoryLimit=(_ttoi(fvmd->maxComMemory.GetBuffer(fvmd->maxComMemory.GetLength()))+1)*1024*1024; //作业内存限制MB   
	  fvmd->maxComMemory.ReleaseBuffer();
	  b_set=SetInformationJobObject(fvmd->hObject,JobObjectExtendedLimitInformation,&JobExtendLimit,sizeof(JobExtendLimit)); //设置作业扩展限制   
	  if(!b_set){
		  char err[400];
		  sprintf(err,"SetInformationJobObject failed %d",GetLastError());
		  OutputDebugString((LPCTSTR)err); //设置失败抛出异常   
		  return false;
	  }
  }

  if(AssignProcessToJobObject(fvmd->hObject,pi.hProcess)==0)
	return false;
	
	return true;
}

BOOL CMainFrame::CreateVM(t_FvmData * fvmd, int *ret)
{
  HANDLE hDevice;               // handle to the drive to be examined 
  BOOL bResult = 0;                 // results flag
  DWORD junk;                   // discard results
  DWORD error;
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  WCHAR cmd[1024];

  swprintf(cmd, L"fvmshell.exe  -%s", fvmd->fvmName);
  int rc;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));


  if(!CreateProcess(NULL,cmd,NULL,NULL,FALSE, 0,NULL,NULL,&si,&pi))
  {
    error = GetLastError();
	MessageBox(L"Could not execute Fvmshell program.", L"Creating FVM Error", MB_ICONEXCLAMATION);
    return (FALSE);
  }
  if(!CreateVmJobObject(pi, fvmd))
  {
	  OutputDebugString(L"Could not create vm job object");
	  return false;
  }//else
	  //MessageBox(L"Create vm job object successfully");
	
	MIB_IPADDRTABLE		*pIPAddrTable;
	DWORD				dwSize;

	pIPAddrTable = (MIB_IPADDRTABLE*) malloc( sizeof( MIB_IPADDRTABLE) );
	dwSize = 0;

	if (GetIpAddrTable(pIPAddrTable, &dwSize, 0) == ERROR_INSUFFICIENT_BUFFER) {
		GlobalFree( pIPAddrTable );
		pIPAddrTable = (MIB_IPADDRTABLE *) malloc ( dwSize );
	}

	if ( (GetIpAddrTable( pIPAddrTable, &dwSize, 0 )) != NO_ERROR ) { 
		printf("Call to GetIpAddrTable failed.\n");
	}

	UINT iaIPAddress;
	UINT imIPMask;
	char ipStr[50];

	sprintf(ipStr, "%S",fvmd->fvmIp);
	iaIPAddress = inet_addr(ipStr);
	sprintf(ipStr,"%S",fvmd->fvmIpMask);
	imIPMask = inet_addr(ipStr);

	ULONG NTEContext = 0;
	ULONG NTEInstance = 0;

	if ( (AddIPAddress(
			iaIPAddress, 
			imIPMask, 
			pIPAddrTable->table[0].dwIndex, 
			&NTEContext, 
			&NTEInstance) ) != NO_ERROR) 
	{
		//printf("\tError adding IP address.\n");
					
	}
	
	GlobalFree( pIPAddrTable );

    swprintf(cmd,L"%s?%s?%s?%d?%u?%u?",
	  fvmd->fvmName, fvmd->fvmID, fvmd->fvmRoot, pi.dwProcessId, iaIPAddress, NTEContext);
  //MessageBox(cmd);
 
  hDevice = CreateFile(DEVICENAME, // drive to open
                       0,       // don't need any access to the drive
                       FILE_SHARE_READ | FILE_SHARE_WRITE,  // share mode
                       NULL,    // default security attributes
                       OPEN_EXISTING,  // disposition
                       0,       // file attributes
                       NULL);   // don't copy any file's attributes

  if (hDevice == INVALID_HANDLE_VALUE) // we can't open the drive
  {
	
	TerminateProcess(pi.hProcess, 0);
	MessageBox(L"Could not open the fvm driver.", L"Creating FVM Error", MB_ICONEXCLAMATION);
    return (FALSE);
  }

  rc = -1;
  bResult = DeviceIoControl(hDevice,  // device we are querying
          IO_CREATE_VM,  // operation to perform
          cmd, sizeof(WCHAR) * wcslen(cmd), // no input buffer, so pass zero
          &rc, 4,
          &junk, // discard count of bytes returned
          (LPOVERLAPPED) NULL);  // synchronous I/O

  if(bResult){
	CloseHandle(hDevice); 
  }
  *ret = rc;
  if(rc != CREATE_VM_SUCCESS){
	TerminateProcess(pi.hProcess, 0);
  }
  
  return (TRUE);
}


void CMainFrame::OnStart()
{
	int rc;
	CFvmBrowser * vmb = GetFvmBrowserView();
	CListCtrl& lc = vmb->GetListCtrl();

	POSITION pos = lc.GetFirstSelectedItemPosition();
	if (pos == NULL)
		MessageBox(L"Please choose a vm first.");
	else
	{
		while (pos)
		{
			int nItem = lc.GetNextSelectedItem(pos);
			CString vmname = lc.GetItemText(nItem, 0);
			t_FvmData * fvmd  = FvmDataFind(vmname);
			
			if(CreateVM(fvmd, &rc)){
				if(rc == CREATE_VM_SUCCESS){
					fvmd->status = 1;
					//MessageBox(L"hello");
					Sleep(100);
					DisplayProcess(fvmd);
					lc.SetItem(nItem, 0, LVIF_IMAGE, NULL, fvmd->status, LVIS_SELECTED, LVIS_SELECTED, 0);
				}
				else if(rc == CREATE_VM_RUNNING){
					MessageBox(fvmd->fvmName + " is running.", L"Creating FVM Error", MB_ICONEXCLAMATION);
				}
				else if(rc == CREATE_VM_MAXVMER){
					MessageBox(L"Maximum number of FVM reaches.", L"Creating FVM Error", MB_ICONEXCLAMATION);					
				}
				else if(rc == CREATE_VM_MEMERRO){
					MessageBox(L"Out of memory", L"Creating FVM Error", MB_ICONEXCLAMATION);
				}
			}
		}
	}
}


BOOL CMainFrame::TerminateVM(t_FvmData * fvmd, int *ret)
{
  HANDLE hDevice;               // handle to the drive to be examined 
  BOOL bResult = 0;                 // results flag
  DWORD junk;                   // discard results
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  WCHAR cmd[1024];
  ULONG ipContext;
	

  swprintf(cmd, L"fvmshell.exe  -%s", fvmd->fvmName);
  int rc;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  swprintf(cmd,L"%s",fvmd->fvmID);
 
  hDevice = CreateFile(DEVICENAME, // drive to open
                       0,       // don't need any access to the drive
                       FILE_SHARE_READ | FILE_SHARE_WRITE,  // share mode
                       NULL,    // default security attributes
                       OPEN_EXISTING,  // disposition
                       0,       // file attributes
                       NULL);   // don't copy any file's attributes

  if (hDevice == INVALID_HANDLE_VALUE) // we can't open the drive
  {
	MessageBox(L"Could not open the fvm driver.", L"Creating FVM Error", MB_ICONEXCLAMATION);
    return (FALSE);
  }


  bResult = DeviceIoControl(hDevice,  // device we are querying
          IO_QUERY_VM_IP_CONTEXT,  // operation to perform
          cmd, 
          sizeof(WCHAR) * wcslen(cmd), 
          &ipContext, 
          4,
          &junk, // discard count of bytes returned
          (LPOVERLAPPED) NULL);  // synchronous I/O

	if ((DeleteIPAddress(ipContext)) != NO_ERROR) {
		printf("Call to DeleteIPAddress failed.\n");
	}


  rc = -1;
  bResult = DeviceIoControl(hDevice,  // device we are querying
          IO_TERMINATE_VM,  // operation to perform
          cmd, sizeof(WCHAR) * wcslen(cmd), 
          &rc, 4,
          &junk, // discard count of bytes returned
          (LPOVERLAPPED) NULL);  // synchronous I/O

  
  CloseHandle(hDevice); 
  
  *ret = rc;

  return (TRUE);
}


void CMainFrame::OnStop()
{
	int rc;
	CFvmBrowser * vmb = GetFvmBrowserView();
	CListCtrl& lc = vmb->GetListCtrl();

	POSITION pos = lc.GetFirstSelectedItemPosition();
	if (pos == NULL)
		MessageBox(L"Please choose a vm first.");
	else
	{
		while (pos)
		{
			int nItem = lc.GetNextSelectedItem(pos);
			CString vmname = lc.GetItemText(nItem, 0);
			t_FvmData * fvmd  = FvmDataFind(vmname);

			if(TerminateVM(fvmd, &rc)){
				if(rc == TERMINATE_VM_SUCCESS){
	                UINT uExitCode=0;
					if(TerminateJobObject(fvmd->hObject, uExitCode)==NULL)
					{
						//MessageBox(L"Could not terminate the processes of the fvm. %d",GetLastError());
  					    TCHAR szMesg [256]; 	
						wsprintf (szMesg, L"Could not terminate all processes of the fvm: %ld.", GetLastError());
						MessageBox (szMesg);
						if(CloseHandle(fvmd->hObject)==NULL)
							MessageBox(L"Could not destroy the fvm job object.");
					}else{
						fvmd->status = 0;
						lc.SetItem(nItem, 0, LVIF_IMAGE, NULL, fvmd->status, LVIS_SELECTED, LVIS_SELECTED, 0);
					}
				}
				else if(rc == TERMINATE_VM_PROCESS){
					MessageBox(L"Please close all applications of the fvm before you can terminate it.", L"Terminating FVM Error", MB_ICONEXCLAMATION);
				}
				else if(rc == TERMINATE_VM_NOTFUND){
						MessageBox(L"FVM is not running", L"Terminating FVM Error", MB_ICONEXCLAMATION);
				}
				else if(rc == TERMINATE_VM_PERDENY){
					MessageBox(L"Permission denied.", L"Terminating FVM Error", MB_ICONEXCLAMATION);
				}
				else{
					MessageBox(L"Could not terminate the FVM. Unknown error.", L"Terminating FVM Error", MB_ICONEXCLAMATION);					
				}
			}
		}
	}
}

BOOL CALLBACK SuspendFVMWindows(HWND hwnd, LPARAM lParam)
{
	int		i;
	DWORD	dwprocess;
	int		*pid = (int *)lParam;
	t_FvmData *fvmd = (t_FvmData *)pid[pid[0]+1];

	if (!IsWindowVisible(hwnd)) return TRUE;

	GetWindowThreadProcessId(hwnd, &dwprocess);

	for(i = 0; i < pid[0]; i++){

		if (dwprocess == pid[i+1])
		{
			ShowWindow(hwnd, SW_HIDE);
			
			pwindowlist suspendwin = NULL;
			suspendwin = (pwindowlist)malloc(sizeof(windowlist));
			
			if (suspendwin) {
				suspendwin->hWnd = hwnd;
				suspendwin->next = fvmd->suspendlist;
				fvmd->suspendlist = suspendwin;
			}
			break;
		}
	}
	return TRUE;
}

void ResumeFVMWindows(t_FvmData *fvmd)
{
	pwindowlist prewin, suspendwin = fvmd->suspendlist;

	while (suspendwin) {
		prewin = suspendwin;
		ShowWindow(suspendwin->hWnd, SW_SHOW);
		suspendwin = suspendwin->next;
		free(prewin);
	}

	fvmd->suspendlist = NULL;
}

void CMainFrame::EmptyFVMWorkingSet(t_FvmData *fvmd, int *pid)
{
	int i;
	HANDLE phd;
	SIZE_T minsize, maxsize;

	for(i = 0; i < pid[0]; i++){

		phd =OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_SET_QUOTA, 
			TRUE, pid[i+1]);

		if(phd != NULL){
			if(GetProcessWorkingSetSize(phd, &minsize, &maxsize)){
				if (SetProcessWorkingSetSize(phd, -1, -1)) {

					pworkingsetlist wslist = 
						(pworkingsetlist)malloc(sizeof(workingsetlist));
					
					if (wslist) {
						wslist->pid = pid[i+1];
						wslist->minsize = minsize;
						wslist->maxsize = maxsize;

						wslist->next = fvmd->suspendworkingset;
						fvmd->suspendworkingset = wslist;
					}
				}
			}
			CloseHandle(phd);
		}
	}
}

void CMainFrame::RestoreFVMWorkingSet(t_FvmData *fvmd)
{
	pworkingsetlist prewslist, wslist = fvmd->suspendworkingset;
	HANDLE phd;

	while (wslist) {
		prewslist = wslist;
		
		phd =OpenProcess(PROCESS_SET_QUOTA, TRUE, wslist->pid);

		if(phd != NULL){
			SetProcessWorkingSetSize(phd, wslist->minsize, wslist->maxsize);
			CloseHandle(phd);
		}

		wslist = wslist->next;
		free(prewslist);
	}

	fvmd->suspendworkingset = NULL;
}

void CMainFrame::SuspendProcesses(t_FvmData *fvmd, BOOL suspend)
{
	HANDLE hDevice;               // handle to the driver to be examined 
	BOOL bResult = 0;             // results flag
	DWORD junk;                   // discard results
	WCHAR cmd[100];
	WCHAR rc[516];
	int   *pid;
	int		i;
    HANDLE        hThreadSnap = NULL; 
    BOOL          bRet        = FALSE; 
    THREADENTRY32 te32        = {0}; 
 
	if(fvmd->status == 0)
		return;

	swprintf(cmd,L"%s",fvmd->fvmID);
 	
	hDevice = CreateFile(DEVICENAME, // driver to open
                       0,       // don't need any access to the drive
                       FILE_SHARE_READ | FILE_SHARE_WRITE,  // share mode
                       NULL,    // default security attributes
                       OPEN_EXISTING,  // disposition
                       0,       // file attributes
                       NULL);   // don't copy any file's attributes

	if (hDevice == INVALID_HANDLE_VALUE) // we can't open the driver
	{
		MessageBox(L"Could not open the fvm driver.", L"Suspending/Resuming FVM Error", MB_ICONEXCLAMATION);
		return;
	}
	
	pid = (int *)rc;
	pid[0] = -1;
	bResult = DeviceIoControl(hDevice,  // device we are querying
           IO_QUERY_PROCESS_LIST,  // operation to perform
           cmd, sizeof(WCHAR) * wcslen(cmd),
           &rc, 1024,
           &junk, // discard count of bytes returned
           (LPOVERLAPPED) NULL);  // synchronous I/O

	if (pid[0] > 0) {
		if (suspend) {
			// Hide windows of FVM processes
			pid[pid[0]+1] = (int)fvmd;
			fvmd->suspendlist = NULL;
			EnumWindows((WNDENUMPROC)SuspendFVMWindows, (LPARAM)pid);
			// Empty working set of FVM processes
			fvmd->suspendworkingset = NULL;
			EmptyFVMWorkingSet(fvmd, pid);
		}

		// Take a snapshot of all threads currently in the system. 
		hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0); 
		if (hThreadSnap == INVALID_HANDLE_VALUE) {
			CloseHandle(hDevice); 
			return; 
		}
		// Fill in the size of the structure before using it. 
		te32.dwSize = sizeof(THREADENTRY32); 

		// Walk the thread snapshot to find all threads of the process. 
		// If the thread belongs to the process, add its information 
		// to the display list.
		if (Thread32First(hThreadSnap, &te32)) 
		{ 
			do 
			{ 
				for(i = 0; i < pid[0]; i++){
					if (te32.th32OwnerProcessID == pid[i+1]) 
					{
						HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, 
							FALSE, te32.th32ThreadID);
						if (suspend)
						{
							SuspendThread(hThread);
						}
						else
						{
							ResumeThread(hThread);
						}
						CloseHandle(hThread);
					}
				}
			}
			while (Thread32Next(hThreadSnap, &te32)); 
		} 

		// Do not forget to clean up the snapshot object. 
		CloseHandle (hThreadSnap);

		if (!suspend) {
			// Restore working set of FVM processes
			RestoreFVMWorkingSet(fvmd);
			// Show windows of FVM processes
			ResumeFVMWindows(fvmd);
		}
	}

	// Update FVM status
	if (suspend)
		fvmd->status = 2;
	else
		fvmd->status = 1;

	swprintf(cmd, L"%s?%d", fvmd->fvmID, fvmd->status); 

	DeviceIoControl(hDevice,  // device we are querying
		IO_STATUS_VM,  // operation to perform
		cmd, sizeof(WCHAR)*(wcslen(cmd)+1),
		NULL, 0,
		&junk, // discard count of bytes returned
		(LPOVERLAPPED) NULL);  // synchronous I/O

	CloseHandle(hDevice); 
}

void CMainFrame::OnSuspend()
{
	CFvmBrowser * vmb = GetFvmBrowserView();
	CListCtrl& lc = vmb->GetListCtrl();

	POSITION pos = lc.GetFirstSelectedItemPosition();
	if (pos == NULL)
		MessageBox(L"Please choose a vm first.");
	else
	{
		while (pos)
		{
			int nItem = lc.GetNextSelectedItem(pos);
			CString vmname = lc.GetItemText(nItem, 0);
			t_FvmData * fvmd  = FvmDataFind(vmname);

			if(fvmd->status == 0){
				MessageBox(vmname+L" is not running.", L"Suspend FVM error.");
				return;
			}
			else if(fvmd->status == 2){
				MessageBox(vmname+L" is already suspended.", L"Suspend FVM error.");
				return;
			}

			SuspendProcesses(fvmd, TRUE);
			lc.SetItem(nItem, 0, LVIF_IMAGE, NULL, fvmd->status, LVIS_SELECTED, LVIS_SELECTED, 0);
		}
	}
}

void CMainFrame::OnResume()
{
	CFvmBrowser * vmb = GetFvmBrowserView();
	CListCtrl& lc = vmb->GetListCtrl();

	POSITION pos = lc.GetFirstSelectedItemPosition();
	if (pos == NULL)
		MessageBox(L"Please choose a vm first.");
	else
	{
		while (pos)
		{
			int nItem = lc.GetNextSelectedItem(pos);
			CString vmname = lc.GetItemText(nItem, 0);
			t_FvmData * fvmd  = FvmDataFind(vmname);

			if(fvmd->status == 0){
				MessageBox(vmname+L" is not running.", L"Resume FVM error.");
				return;
			}
			else if(fvmd->status == 1){
				MessageBox(vmname+L" is not suspended.", L"Resume FVM error.");
				return;
			}

			SuspendProcesses(fvmd, FALSE);
			lc.SetItem(nItem, 0, LVIF_IMAGE, NULL, fvmd->status, LVIS_SELECTED, LVIS_SELECTED, 0);
		}
	}
}

void CMainFrame::OnDelete()
{
	CFvmBrowser *fb = GetFvmBrowserView();
	CListCtrl& lc = fb->GetListCtrl();

	POSITION pos = lc.GetFirstSelectedItemPosition();

	//m_selectedFvm = NULL;
	if (pos != NULL)
	{
		while (pos)
		{
			int nItem = lc.GetNextSelectedItem(pos);
			CString vmname = lc.GetItemText(nItem, 0);
		
			t_FvmData * fvmd  = FvmDataFind(vmname);
			if(fvmd->status >= 1){
				MessageBox(vmname+L" is running. Please stop it first.", L"Delete FVM error.");
				return;
			}


			int yn = MessageBox(L"Do you want to delete "+vmname+L"?", L"Deleting FVM",MB_ICONQUESTION|MB_OKCANCEL );
			if(yn ==1){
				DeleteFvm(fvmd);
				FvmDataRemove(fvmd->fvmName);
				oldName=L"";
				m_selectedFvm = NULL;
				GetFvmBrowserView()->InsertFvms();
				UpdateDisplays();
			}
		}
	}
}


void CommitCopyFiles(CString ol, CString ne)
{

	CString tempPath = ol+L"\\*.*";
	WIN32_FIND_DATA find;
	HANDLE hFind = FindFirstFile(tempPath, &find);

	while ( hFind!=INVALID_HANDLE_VALUE)
	{
		if (!(find.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY && (!_tcscmp(find.cFileName, _T("..")) || !_tcscmp(find.cFileName, _T(".")))))
		{
			CString on;
			CString nn;
	
			on = ol+L"\\"+find.cFileName;
			nn = ne+L"\\"+find.cFileName;

			if ( find.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			{
				CreateDirectory(nn, NULL);
				FvmCopyFiles(on,nn);
			}
			else
			{
				
				CopyFile(on,nn,FALSE);

			}
			
				
				
		}
		if (!FindNextFile(hFind, &find))
		{
			FindClose(hFind);
			hFind=0;
			break;
		}		
	}		

}

void CMainFrame::CommitFiles(CString root)
{

	CString tempPath = root+L"\\*.*";
	WIN32_FIND_DATA find;
	HANDLE hFind = FindFirstFile(tempPath, &find);

	while ( hFind!=INVALID_HANDLE_VALUE)
	{
		if (!(find.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY && (!_tcscmp(find.cFileName, _T("..")) || !_tcscmp(find.cFileName, _T(".")))))
		{
			CString on;
			CString nn;
	
			on = root+L"\\"+find.cFileName;
			nn = find.cFileName;
			nn = nn+L":\\";

			if ( find.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			{
				CommitCopyFiles(on,nn);

			}
			else
			{
							

			}
			
				
				
		}
		if (!FindNextFile(hFind, &find))
		{
			FindClose(hFind);
			hFind=0;
			break;
		}		
	}		


}


#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

TCHAR achKey[MAX_KEY_LENGTH];
TCHAR data[10240];

void CMainFrame::CommitCopyRegistry(HKEY ohkey, HKEY nhkey, CString ok, CString nk, int cp)
{
	HKEY okey;
	HKEY nkey;
	TCHAR *subkey;
	int la;


	subkey = (TCHAR *)malloc(sizeof(TCHAR)*MAX_KEY_LENGTH);
	if(cp)
		if(RegCreateKeyEx(nhkey, nk, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &nkey, NULL) != ERROR_SUCCESS)
		{
			
			return;

		}
		else{
		;//	MessageBox(L"ok", nk);

		}

	if((la=RegOpenKeyEx(ohkey, ok,0,KEY_ALL_ACCESS, &okey)) == ERROR_SUCCESS)
	{

		int i =0;
        unsigned long cbName = MAX_KEY_LENGTH;
        int retCode;
		
	
		if(cp){
			TCHAR *Buff = (TCHAR *)malloc(MAX_VALUE_NAME);
			int Err = ERROR_NOT_ENOUGH_MEMORY;
			int ValEnumIndx = 0;

			DWORD BuffSize;
			DWORD VarType;
			DWORD size;

			
		
			do {
				// read value from source key
			

			
				size=MAX_KEY_LENGTH*sizeof(TCHAR);
				BuffSize = MAX_VALUE_NAME;
				Err = RegEnumValue(okey,ValEnumIndx,subkey,&size,NULL,&VarType, (unsigned char *)Buff,&BuffSize);
				
			
				// done copying this key
				if (Err == ERROR_NO_MORE_ITEMS)
					break;

				// unknown error return
				if (Err != ERROR_SUCCESS)
					break;


				DWORD ltype;
				DWORD cd;

				cd = 10240;
				int copy = 0;

				if(wcscmp(L"SEES_INITIAL", subkey)!=0)
				{
					if(RegQueryValueEx(nkey, subkey,0, &ltype, (unsigned char *)data, &cd) != ERROR_SUCCESS)
						copy = 1;
					else
					{
						if(cd != BuffSize)
							copy = 1;
						else{
							int k;
							char *p1 = (char *)data;
							char *p2 = (char *) Buff;
							for(k = 0; k < cd; k++)
								if(p1[k]!= p2[k])
									copy = 1;


						}

					}
  


					// write value to target key
					if(copy){
						//MessageBox(subkey, ok);
						if(RegSetValueEx(nkey,subkey,NULL,VarType, (const unsigned char *)Buff,BuffSize) != ERROR_SUCCESS){
							MessageBox(L"Error");
						}

					}
				}
				// read next value
				ValEnumIndx++;
			} while (true);

			free(Buff);
		}

		i = 0;
		do{
			cbName = sizeof(TCHAR)*MAX_KEY_LENGTH;
			retCode = RegEnumKeyEx(okey, i, subkey, &cbName, NULL, NULL, NULL, NULL); 
			if (retCode == ERROR_SUCCESS) 
			{
				if(!cp)
					nkey = nhkey;

                CommitCopyRegistry(okey, nkey, subkey, subkey,1);
			}
			i++;
		}while(retCode == ERROR_SUCCESS);

	
		CloseHandle(okey);

		if(cp)
			CloseHandle(nkey);
	}
	else{
		if(cp)
			CloseHandle(nkey);
		return;
	}



	free(subkey);
}

void CMainFrame::CommitRegistry(CString rn)
{

	HKEY key;
	//MessageBox(rn); 

	int la;
	if((la=RegOpenKeyEx(HKEY_CURRENT_USER, rn.GetBuffer(rn.GetLength()),0,KEY_ALL_ACCESS, &key)) == ERROR_SUCCESS)
	{

		int i =0;
        unsigned long cbName = MAX_KEY_LENGTH;
        int retCode;
		


		
		do{
			sizeof(TCHAR)*MAX_KEY_LENGTH;
			retCode = RegEnumKeyEx(key, i,achKey, &cbName, NULL, NULL, NULL, NULL); 
			if (retCode == ERROR_SUCCESS) 
			{
                CString subkey = achKey;
				if(subkey == L"Machine")
					CommitCopyRegistry(key, HKEY_LOCAL_MACHINE, subkey, subkey, 0);
				else if(subkey == L"USER")
					CommitCopyRegistry(key, HKEY_USERS, subkey, subkey, 0);

			}
			i++;
		}while(retCode == ERROR_SUCCESS);

	
		CloseHandle(key);
	}
	else{

	}


}


void CMainFrame::OnCommit()
{
	CFvmBrowser *fb = GetFvmBrowserView();
	CListCtrl& lc = fb->GetListCtrl();

	POSITION pos = lc.GetFirstSelectedItemPosition();

	//m_selectedFvm = NULL;
	if (pos != NULL)
	{
		while (pos)
		{
			int nItem = lc.GetNextSelectedItem(pos);
			CString vmname = lc.GetItemText(nItem, 0);
		
			t_FvmData * fvmd  = FvmDataFind(vmname);
			if(fvmd->status >= 1){
				MessageBox(vmname+L" is running. Please stop it first.", L"Commit FVM error.");
				return;
			}


			int yn = MessageBox(L"Do you want to commit "+vmname+L"?", L"Commit FVM",MB_ICONQUESTION|MB_OKCANCEL );
			if(yn ==1){


				CommitRegistry(L"fvms\\"+fvmd->fvmID+L"\\Registry");

				
				CommitFiles(fvmd->fvmRoot+L"\\"+fvmd->fvmID);

				DeleteFvm(fvmd);
				FvmDataRemove(fvmd->fvmName);
				oldName=L"";
				m_selectedFvm = NULL;
				GetFvmBrowserView()->InsertFvms();
				UpdateDisplays();
				
			}
		}
	}
}

void CMainFrame::OnProcess()
{
	if(pSplitterWnd3->IsPaneVisible(BSW_SECOND_PANE))
		pSplitterWnd3->HidePane(BSW_SECOND_PANE);
	else
		pSplitterWnd3->ShowPane(BSW_SECOND_PANE);
}



void CMainFrame::OnFile()
{

	if(pSplitterWnd4->IsPaneVisible(BSW_FIRST_PANE))
		pSplitterWnd4->HidePane(BSW_FIRST_PANE);
	else
		pSplitterWnd4->ShowPane(BSW_FIRST_PANE);
	/*

	CFileView * fv = GetFileView();
	CListCtrl& lc = fv->GetListCtrl();

	lc.DeleteAllItems();



	CImageList sysImgList;
	SHFILEINFO shFinfo;
	
	sysImgList.Attach((HIMAGELIST)SHGetFileInfo( _T("C:\\"),
							  0,
							  &shFinfo,
							  sizeof( shFinfo ),
							  SHGFI_SYSICONINDEX | SHGFI_SMALLICON));
							  //((m_nStyle==LVS_ICON)?SHGFI_ICON:SHGFI_SMALLICON) ));


	lc.SetImageList( &sysImgList, LVSIL_SMALL);//(m_nStyle==LVS_ICON)?LVSIL_NORMAL:LVSIL_SMALL);

	lc.ModifyStyle(0,LVS_ICON|LVS_SINGLESEL ); 
	lc.SetExtendedStyle(LVS_EX_TRACKSELECT | LVS_EX_ONECLICKACTIVATE);

	lc.InsertItem(0,_T("Virtual Machine 1"),3);

	sysImgList.Detach();
	*/

}

void CMainFrame::OnRegistry()
{
	if(pSplitterWnd4->IsPaneVisible(BSW_SECOND_PANE))
		pSplitterWnd4->HidePane(BSW_SECOND_PANE);
	else
		pSplitterWnd4->ShowPane(BSW_SECOND_PANE);

}

void CMainFrame::OnShell()
{
	
	CFvmBrowser * vmb = GetFvmBrowserView();
	CListCtrl& lc = vmb->GetListCtrl();

	POSITION pos = lc.GetFirstSelectedItemPosition();
	if (pos == NULL)
		MessageBox(L"Please choose a vm first.");
	else
	{
		while (pos)
		{
			int nItem = lc.GetNextSelectedItem(pos);
			CString vmname = lc.GetItemText(nItem, 0);
			t_FvmData * fvmd  = FvmDataFind(vmname);
			


			HANDLE hDevice;               // handle to the drive to be examined 
			BOOL bResult = 0;                 // results flag
			DWORD junk;                   // discard results
			DWORD error;
			STARTUPINFO si;
			PROCESS_INFORMATION pi;
			WCHAR cmd[1024];

			if(fvmd->status == 0){
					MessageBox(L"Please start the FVM first.", L"Creating FvmShell Error", MB_ICONEXCLAMATION);
				return;
			}
			else if(fvmd->status == 2){
					MessageBox(L"Please resume the FVM first.", L"Creating FvmShell Error", MB_ICONEXCLAMATION);
				return;
			}

			swprintf(cmd, L"fvmshell.exe  -%s", fvmd->fvmName);
			int rc;
			ZeroMemory(&si, sizeof(si));
			si.cb = sizeof(si);
			ZeroMemory(&pi, sizeof(pi));


			if(!CreateProcess(NULL,cmd,NULL,NULL,FALSE, 0,NULL,NULL,&si,&pi))
			{
				error = GetLastError();
				MessageBox(L"Could not execute Fvmshell program.", L"Creating FvmShell Error", MB_ICONEXCLAMATION);
				return;
			}

			swprintf(cmd,L"%s?%d", fvmd->fvmID, pi.dwProcessId);
  
 
			hDevice = CreateFile(DEVICENAME, // drive to open
                       0,       // don't need any access to the drive
                       FILE_SHARE_READ | FILE_SHARE_WRITE,  // share mode
                       NULL,    // default security attributes
                       OPEN_EXISTING,  // disposition
                       0,       // file attributes
                       NULL);   // don't copy any file's attributes

			if (hDevice == INVALID_HANDLE_VALUE) // we can't open the drive
			{
	
				TerminateProcess(pi.hProcess, 0);
				MessageBox(L"Could not open the fvm driver.", L"Creating FvmShell Error", MB_ICONEXCLAMATION);
				return;
			}


			rc = -1;
			bResult = DeviceIoControl(hDevice,  // device we are querying
									IO_RESUME_VM,  // operation to perform
									cmd, sizeof(WCHAR) * wcslen(cmd), // no input buffer, so pass zero
									&rc, 4,
									&junk, // discard count of bytes returned
									(LPOVERLAPPED) NULL);  // synchronous I/O

			
			CloseHandle(hDevice); 
  
			 if(rc == ADD_SHELL_VMNOTFD){
				TerminateProcess(pi.hProcess, 0);	
				MessageBox(L"The FVM is not running.", L"Creating FvmShell Error.", MB_ICONEXCLAMATION);
			}
			 Sleep(10);
			 DisplayProcess(fvmd);
		}
	}
}

CFileView * CMainFrame::GetFileView()
{
 
 	static CFileView *sView=0;
 	if (sView)
 		return sView;
 	CWnd* pWnd = pSplitterWnd4->GetView(21);
 	CFileView* pView = DYNAMIC_DOWNCAST(CFileView, pWnd);
 	sView=pView;
 	return pView;
}

CRegView * CMainFrame::GetRegView()
{

	static CRegView *sView=0;
	if (sView)
		return sView;
	CWnd* pWnd = pSplitterWnd5->GetView(31);
	CRegView* pView = DYNAMIC_DOWNCAST(CRegView, pWnd);
	sView=pView;
	return pView;
}

BOOL CMainFrame::GetSysImgList(CImageList *imgList)
{





	return TRUE;
}

t_FvmData * CMainFrame::FvmDataFind(CString name, int flag)
{

	CString *tStr;

	t_FvmData *tmp;

	tmp = fvmData;

	while(tmp){
		if(flag == 0)
			tStr = &tmp->fvmName;
		else
			tStr = &tmp->fvmID;

		if(name == *tStr)
			break;

		tmp = tmp->next;
	}

	return tmp;
}

void CMainFrame::FvmDataAdd(t_FvmData *data)
{
	//MessageBox(data->fvmName);
	t_FvmData *p, *c;
	if(fvmData == NULL){
		data->next = fvmData;
		fvmData = data;
	}
	else{

		c = fvmData;
		p = fvmData;

		while(c){
			if(c->fvmName<data->fvmName){
				p = c;
				c = c->next;
			}
			else{
				break;
			}
		}
		if(c == p){
			if(p == fvmData){
				data->next = fvmData;
				fvmData = data;	
			}

			return;
		}
		data->next = p->next;
		p->next = data;
	}
}

void CMainFrame::FvmDataRemove(CString name)
{
	t_FvmData *p, *c;


	c = fvmData;
	p = fvmData;
	while(c){		
		if(c->fvmName == name)
			break;
		p = c;
		c = c->next;
	}

	if(c == p && c == fvmData){
		fvmData = c->next;
		delete c;
		
	}
	else if(c != NULL){
		p->next = c->next;
		delete c;
	}
}

CFvmBrowser * CMainFrame::GetFvmBrowserView()
{
	static CFvmBrowser *sView=0;
	if (sView)
		return sView;
	CWnd* pWnd = pSplitterWnd3->GetView(11);
	CFvmBrowser* pView = DYNAMIC_DOWNCAST(CFvmBrowser, pWnd);
	sView=pView;
	return pView;

}

void CMainFrame::FvmDataFree()
{

	

	t_FvmData *tmp, *tmp2;

	tmp = fvmData;

	while(tmp){
		
		tmp2 = tmp;
		tmp = tmp->next;
		delete tmp2;
	}


}

void CMainFrame::OnDestroy() 
{
	CFrameWnd::OnDestroy();
	
	// TODO: Add your message handler code here
	FvmDataFree();
}

void CMainFrame::DisplayProcess(t_FvmData *fvmd)
{

	
	HANDLE hDevice;               // handle to the drive to be examined 
	BOOL bResult = 0;                 // results flag
	DWORD junk;                   // discard results
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	WCHAR cmd[100];
	WCHAR rc[516];
	int   *pid;
	int		i;
	HANDLE phd;
	WCHAR fileName[512];
	CProcessView *pv;
	int item;

	//MessageBox(L"Process");
		pv = GetProcessView();
	CListCtrl& lc = pv->GetListCtrl();
	lc.DeleteAllItems();
	if(fvmd->status == 0)
		return;

	

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));



	swprintf(cmd,L"%s",fvmd->fvmID);
 
	
	hDevice = CreateFile(DEVICENAME, // drive to open
                       0,       // don't need any access to the drive
                       FILE_SHARE_READ | FILE_SHARE_WRITE,  // share mode
                       NULL,    // default security attributes
                       OPEN_EXISTING,  // disposition
                       0,       // file attributes
                       NULL);   // don't copy any file's attributes

	if (hDevice == INVALID_HANDLE_VALUE) // we can't open the drive
	{
		MessageBox(L"Could not open the fvm driver.", L"Displaying Process Error", MB_ICONEXCLAMATION);
		return;
	}


	
	pid = (int *)rc;
	pid[0] = -1;
	bResult = DeviceIoControl(hDevice,  // device we are querying
           IO_QUERY_PROCESS_LIST,  // operation to perform
          cmd, sizeof(WCHAR) * wcslen(cmd), // no input buffer, so pass zero
          &rc, 1024,
          &junk, // discard count of bytes returned
          (LPOVERLAPPED) NULL);  // synchronous I/O

  
	CloseHandle(hDevice); 
  


	for(i = 0; i < pid[0]; i++){
		phd =OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, TRUE, pid[i+1]);
		if(phd != NULL){
			if(GetModuleFileNameEx(phd, NULL, fileName, 1024)){
				
				int iIcon=-1;
				SHFILEINFO shFinfo;
				memset(&shFinfo,0,sizeof(SHFILEINFO));
				if (SHGetFileInfo(fileName,
					FILE_ATTRIBUTE_NORMAL,
					&shFinfo,
					sizeof( SHFILEINFO ),
					SHGFI_ICON))
				{
					iIcon = shFinfo.iIcon;			
					DestroyIcon( shFinfo.hIcon );			
				}

				item = lc.InsertItem(0,fileName,iIcon);
				wsprintf(fileName, L"%d", pid[i+1]);
				lc.SetItemText(item, 1, fileName);
			}
			CloseHandle(phd);
		}
	}
}

CProcessView * CMainFrame::GetProcessView()
{
	static CProcessView *sView=0;
	if (sView)
		return sView;

	CWnd* pWnd = pSplitterWnd3->GetView(12);
	CProcessView* pView = DYNAMIC_DOWNCAST(CProcessView, pWnd);
	sView=pView;
	return pView;
}

void GetSideEffects(t_FvmData *fvmd)
{
  HANDLE hDevice;               // handle to the drive to be examined 
  BOOL bResult = 0;                 // results flag
  DWORD junk;                   // discard results
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  WCHAR cmd[1024];

 
  int rc;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));



  swprintf(cmd,L"%s",fvmd->fvmID);
 
 
  hDevice = CreateFile(DEVICENAME, // drive to open
                       0,       // don't need any access to the drive
                       FILE_SHARE_READ | FILE_SHARE_WRITE,  // share mode
                       NULL,    // default security attributes
                       OPEN_EXISTING,  // disposition
                       0,       // file attributes
                       NULL);   // don't copy any file's attributes

  if (hDevice == INVALID_HANDLE_VALUE) // we can't open the drive
  {
	//MessageBox(L"Could not open the fvm driver.", L"Creating FVM Error", MB_ICONEXCLAMATION);
    return ;
  }


  rc = -1;
  bResult = DeviceIoControl(hDevice,  // device we are querying
          IO_QUERY_SIDE_EFFECT_LIST,  // operation to perform
          cmd, sizeof(WCHAR) * wcslen(cmd), // no input buffer, so pass zero
          &rc, 4,
          &junk, // discard count of bytes returned
          (LPOVERLAPPED) NULL);  // synchronous I/O

  
  CloseHandle(hDevice); 
  
 

}


void CMainFrame::UpdateDisplays()
{
	//MessageBox(L"hello\n");

	CFvmBrowser *fb = GetFvmBrowserView();

	CListCtrl& lc = fb->GetListCtrl();


	POSITION pos = lc.GetFirstSelectedItemPosition();

	CProcessView *pv = GetProcessView();
	CListCtrl& plc = pv->GetListCtrl();
	CFileView *fv = GetFileView();
	CLeftView *lv = GetLeftView();
	CTreeCtrl &pCtrl = lv->GetTreeCtrl();
	CRegView *rv = GetRegView();
	CListCtrl& rlc = rv->GetListCtrl();
		

	if (pos != NULL)
	{
		while (pos)
		{
			int nItem = lc.GetNextSelectedItem(pos);
			CString vmname = lc.GetItemText(nItem, 0);
			if(oldName != vmname){
				oldName = vmname;
				t_FvmData * fvmd  = FvmDataFind(vmname);

				if(fvmd->status)
					GetSideEffects(fvmd);

				//fvmData =fvmd;
				plc.DeleteAllItems();
				fv->m_pListCtrl->DeleteAllItems();
				pCtrl.DeleteAllItems();
				rlc.DeleteAllItems();
				//MessageBox(vmname, oldName);//fvmd->fvmRoot);
				//GetFileView()
				fv->m_pListCtrl->DisplayFiles(fvmd->fvmRoot+L"\\"+fvmd->fvmID);


				//fvmData =fvmd;
				m_selectedFvm = fvmd;
				DisplayProcess(fvmd);
				GetLeftView()->DisplayRegistry(fvmd);
			}
			
		}
	}
	else{
		oldName = "";
			
		plc.DeleteAllItems();
		fv->m_pListCtrl->DeleteAllItems();
		pCtrl.DeleteAllItems();
		rlc.DeleteAllItems();

	}
}

void CMainFrame::OnRefresh()
{
	CFvmBrowser *fb = GetFvmBrowserView();
	CListCtrl& lc = fb->GetListCtrl();

	POSITION pos = lc.GetFirstSelectedItemPosition();

	m_selectedFvm = NULL;
	if (pos != NULL)
	{
		while (pos)
		{
			int nItem = lc.GetNextSelectedItem(pos);
			CString vmname = lc.GetItemText(nItem, 0);
		
			t_FvmData * fvmd  = FvmDataFind(vmname);
			m_selectedFvm = fvmd;
			oldName = L"";			
		}
	}
	GetFvmBrowserView()->InsertFvms();
	UpdateDisplays();
}


CLeftView * CMainFrame::GetLeftView()
{
	static CLeftView *sView=0;
	if (sView)
		return sView;
	CWnd* pWnd = pSplitterWnd5->GetView(32);
	CLeftView* pView = DYNAMIC_DOWNCAST(CLeftView, pWnd);
	sView=pView;
	return pView;
}


BOOL IsDots(const TCHAR* str) {
    if(_tcscmp(str, _T(".")) && _tcscmp(str,_T(".."))) return FALSE;
    return TRUE;
}

BOOL DeleteDirectory(const TCHAR* sPath) {
    HANDLE hFind;  // file handle
    WIN32_FIND_DATA FindFileData;

    TCHAR DirPath[MAX_PATH];
    TCHAR FileName[MAX_PATH];

    _tcscpy(DirPath,sPath);
    _tcscat(DirPath, _T("\\*"));    // searching all files
    _tcscpy(FileName,sPath);
    _tcscat(FileName, L"\\");

    hFind = FindFirstFile(DirPath,&FindFileData); // find the first file
    if(hFind == INVALID_HANDLE_VALUE) return FALSE;
    _tcscpy(DirPath,FileName);
        
    bool bSearch = true;
    while(bSearch) { // until we finds an entry
        if(FindNextFile(hFind,&FindFileData)) {
            if(IsDots(FindFileData.cFileName)) continue;
            _tcscat(FileName,FindFileData.cFileName);
            if((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

                // we have found a directory, recurse
                if(!DeleteDirectory(FileName)) { 
                    FindClose(hFind); 
                    return FALSE; // directory couldn't be deleted
                }
                RemoveDirectory(FileName); // remove the empty directory
                _tcscpy(FileName,DirPath);
            }
            else {
                if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
                    _tchmod(FileName, _S_IWRITE); // change read-only file mode
                if(!DeleteFile(FileName)) {  // delete the file
                    FindClose(hFind); 
                    return FALSE; 
                }                 
                _tcscpy(FileName,DirPath);
            }
        }
        else {
            if(GetLastError() == ERROR_NO_MORE_FILES) // no more files there
            bSearch = false;
            else {
                // some error occured, close the handle and return FALSE
                FindClose(hFind); 
                return FALSE;
            }

        }

    }
    FindClose(hFind);  // closing file handle
 
    return RemoveDirectory(sPath); // remove the empty directory

} 


//*************************************************************
//
//  RegDelnodeRecurse()
//
//  Purpose:    Deletes a registry key and all it's subkeys / values.
//
//  Parameters: hKeyRoot    -   Root key
//              lpSubKey    -   SubKey to delete
//
//  Return:     TRUE if successful.
//              FALSE if an error occurs.
//
//*************************************************************

BOOL RegDelnodeRecurse (HKEY hKeyRoot, LPTSTR lpSubKey)
{
    LPTSTR lpEnd;
    LONG lResult;
    DWORD dwSize;
    TCHAR szName[MAX_PATH];
    HKEY hKey;
    FILETIME ftWrite;

    // First, see if we can delete the key without having
    // to recurse.

    lResult = RegDeleteKey(hKeyRoot, lpSubKey);

    if (lResult == ERROR_SUCCESS) 
        return TRUE;

    lResult = RegOpenKeyEx (hKeyRoot, lpSubKey, 0, KEY_READ, &hKey);

    if (lResult != ERROR_SUCCESS) 
    {
        if (lResult == ERROR_FILE_NOT_FOUND) {
            printf("Key not found.\n");
            return TRUE;
        } 
        else {
            printf("Error opening key.\n");
            return FALSE;
        }
    }

    // Check for an ending slash and add one if it is missing.

    lpEnd = lpSubKey + lstrlen(lpSubKey);

    if (*(lpEnd - 1) != TEXT('\\')) 
    {
        *lpEnd =  TEXT('\\');
        lpEnd++;
        *lpEnd =  TEXT('\0');
    }

    // Enumerate the keys

    dwSize = MAX_PATH;
    lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
                           NULL, NULL, &ftWrite);

    if (lResult == ERROR_SUCCESS) 
    {
        do {

            lstrcpy (lpEnd, szName);

            if (!RegDelnodeRecurse(hKeyRoot, lpSubKey)) {
                break;
            }

            dwSize = MAX_PATH;

            lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
                                   NULL, NULL, &ftWrite);

        } while (lResult == ERROR_SUCCESS);
    }

    lpEnd--;
    *lpEnd = TEXT('\0');

    RegCloseKey (hKey);

    // Try again to delete the key.

    lResult = RegDeleteKey(hKeyRoot, lpSubKey);

    if (lResult == ERROR_SUCCESS) 
        return TRUE;

    return FALSE;
}

//*************************************************************
//
//  RegDelnode()
//
//  Purpose:    Deletes a registry key and all it's subkeys / values.
//
//  Parameters: hKeyRoot    -   Root key
//              lpSubKey    -   SubKey to delete
//
//  Return:     TRUE if successful.
//              FALSE if an error occurs.
//
//*************************************************************

BOOL RegDelnode (HKEY hKeyRoot, LPCTSTR lpSubKey)
{
    TCHAR szDelKey[2 * MAX_PATH];

    lstrcpy (szDelKey, lpSubKey);
    return RegDelnodeRecurse(hKeyRoot, szDelKey);

}

void CMainFrame::DeleteFvm(t_FvmData *fvmd)
{

	CString regStr;
	CString regkey;

	regStr.LoadString(IDS_VMS_REG);
	
	regkey = regStr + "\\" + fvmd->fvmName;

	RegDelnode(HKEY_CURRENT_USER, regkey);
	
	regkey = L"";
	regkey = L"fvms\\"+fvmd->fvmID;
	
	RegDelnode(HKEY_CURRENT_USER, regkey);

	DeleteDirectory(fvmd->fvmRoot+L"\\"+fvmd->fvmID);
	DeleteFile(fvmd->fvmRoot+L"\\"+fvmd->fvmID+_T(".bin"));
}
