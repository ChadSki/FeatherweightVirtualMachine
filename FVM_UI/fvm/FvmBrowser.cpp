// FvmBrowser.cpp : implementation file
//

#include "stdafx.h"
#include "fvm.h"
#include "FvmBrowser.h"
#include "fvmData.h"
#include "MainFrm.h"
#include "fvmdefine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFvmBrowser

IMPLEMENT_DYNCREATE(CFvmBrowser, CListView)

CFvmBrowser::CFvmBrowser()
{
}

CFvmBrowser::~CFvmBrowser()
{
}


BEGIN_MESSAGE_MAP(CFvmBrowser, CListView)
	//{{AFX_MSG_MAP(CFvmBrowser)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnItemchanged)
	ON_NOTIFY_REFLECT(HDN_ITEMCLICK, OnItemclick)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFvmBrowser drawing

void CFvmBrowser::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CFvmBrowser diagnostics

#ifdef _DEBUG
void CFvmBrowser::AssertValid() const
{
	CListView::AssertValid();
}

void CFvmBrowser::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFvmBrowser message handlers

void CFvmBrowser::OnInitialUpdate() 
{
	CListView::OnInitialUpdate();
	
	// TODO: Add your specialized code here and/or call the base class
	m_imgList.Create ( 32, 32, ILC_COLOR32 | ILC_MASK, 4, 1 );
	m_imgList.Add (AfxGetApp()->LoadIcon(IDI_OFF));
	m_imgList.Add (AfxGetApp()->LoadIcon(IDI_ON));
	m_imgList.Add (AfxGetApp()->LoadIcon(IDI_SLEEP));
	 
	// TODO: Add your specialized code here and/or call the base class
		// TODO: Add your specialized code here and/or call the base class
	CListCtrl& lc = GetListCtrl();
	//lc.ModifyStyle(0,LVS_REPORT);
    lc.ModifyStyle(0,/*LVS_ICON*/LVS_REPORT|LVS_SINGLESEL);//|LVM_SORTITEMS); 
	lc.SetExtendedStyle(LVS_EX_TRACKSELECT | LVS_EX_ONECLICKACTIVATE);

	lc.InsertColumn(0, _T("Virtual Machines"), LVCFMT_LEFT);
	lc.SetImageList ( &m_imgList, LVSIL_SMALL);	
	//lc.SetColumnWidth(0, 200);
  /*	
   LVCOLUMN col;
   col.mask = LVCF_FMT | LVCF_TEXT;
   col.pszText = _T("Contribution");
   col.fmt = LVCFMT_CENTER;
   lc.InsertColumn(1, &col);
  */
	COLORREF crBkColor = ::GetSysColor(COLOR_3DFACE);

	lc.SetTextColor(RGB(255,255,255));
	lc.SetTextBkColor(RGB(113,116,95));
	lc.SetBkColor(RGB(113,116,95));	

	lc.SetColumnWidth(0,255);


	HKEY key;
	HKEY key2;
	CString regStr;
	TCHAR lpName[1024];
	TCHAR lpValue[1024];
	ULONG  size;
	DWORD type;
	int index;

	regStr.LoadString(IDS_VMS_REG);
	
	if (RegOpenKeyEx(HKEY_CURRENT_USER, regStr, 0, KEY_ENUMERATE_SUB_KEYS|KEY_QUERY_VALUE ,&key)==ERROR_SUCCESS)
	{
		index = 0;
		
		while(RegEnumKey(key, index, (LPTSTR) &lpName, 1024) == ERROR_SUCCESS){
		if (RegOpenKeyEx(HKEY_CURRENT_USER, regStr+_T("\\")+lpName, 0, KEY_ENUMERATE_SUB_KEYS|KEY_QUERY_VALUE ,&key2)==ERROR_SUCCESS)

			size = 1024;
			//RegQueryValue(key2, _T("fvmid"), (LPTSTR) &lpValue, &size);

			t_FvmData *td = new t_FvmData;
			td->fvmName = lpName;
		
			td->status = 0;
			td->next = NULL;

			RegQueryValueEx(key2, _T("fvmid"), NULL, &type, (LPBYTE)&lpValue, &size);
			td->fvmID = lpValue;
			lpValue[0] = L'\0';
			size = 1024;
			RegQueryValueEx(key2, _T("fvmroot"), NULL, &type, (LPBYTE)&lpValue, &size);
			td->fvmRoot = lpValue;
			size = 1024;
			RegQueryValueEx(key2, _T("fvmip"), NULL, &type, (LPBYTE)&lpValue, &size);
			td->fvmIp = lpValue;
			size = 1024;
			RegQueryValueEx(key2, _T("fvmipmask"), NULL, &type, (LPBYTE)&lpValue, &size);
			td->fvmIpMask = lpValue;

			size = 1024;
			RegQueryValueEx(key2, _T("schedPriority"), NULL, &type, (LPBYTE)&lpValue, &size);
			td->schedPriority = lpValue;
			size = 1024;
			RegQueryValueEx(key2, _T("maxProcesses"), NULL, &type, (LPBYTE)&lpValue, &size);
			td->maxProcesses = lpValue;
			size = 1024;
			RegQueryValueEx(key2, _T("maxComMemory"), NULL, &type, (LPBYTE)&lpValue, &size);
			td->maxComMemory = lpValue;
			size = 1024;
			RegQueryValueEx(key2, _T("maxWorkSet"), NULL, &type, (LPBYTE)&lpValue, &size);
			td->maxWorkSet = lpValue;

			//	MessageBox(td->fvmRoot);
			 m_Parent->FvmDataAdd(td);

			//CFvmBrowser *fb = m_Parent->GetFvmBrowserView();
			RegCloseKey(key2);
			index++;
		}

		m_Parent->m_selectedFvm = NULL;
		m_Parent->oldName = L"";
		InsertFvms();

		RegCloseKey(key);
	}
	
}

void CFvmBrowser::OnItemchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here

	m_Parent->UpdateDisplays();
	*pResult = 0;
}

void CFvmBrowser::OnItemclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	HD_NOTIFY *phdn = (HD_NOTIFY *) pNMHDR;
	// TODO: Add your control notification handler code here
	


	*pResult = 0;
}

void CFvmBrowser::InsertFvms()
{
	WCHAR rc[1090];

	t_FvmData *temp = m_Parent->fvmData;


	HANDLE hDevice;               // handle to the drive to be examined 
	BOOL bResult = 0;                 // results flag
	DWORD junk;                   // discard results
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	int   *ret;
	int		i, status;
	WCHAR *fvms, fvmId[33];
	t_FvmData *fvmData;
	
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	
	hDevice = CreateFile(DEVICENAME, // drive to open
                       0,       // don't need any access to the drive
                       FILE_SHARE_READ | FILE_SHARE_WRITE,  // share mode
                       NULL,    // default security attributes
                       OPEN_EXISTING,  // disposition
                       0,       // file attributes
                       NULL);   // don't copy any file's attributes

	if (hDevice != INVALID_HANDLE_VALUE)
	{
		ret = (int *)rc;
		ret[0] = -1;
		bResult = DeviceIoControl(hDevice,  // device we are querying
			IO_QUERY_VM_LIST,  // operation to perform
			NULL, 0,			
			&rc, 1090*2,
			&junk, // discard count of bytes returned
			(LPOVERLAPPED) NULL);  // synchronous I/O

		CloseHandle(hDevice); 

		fvms = (WCHAR *)(ret+1);
		for(i = 0; i < ret[0]; i++){
			wcsncpy(fvmId, fvms, 32);
			fvmId[32] = L'\0';
			//MessageBox(fvmId);
			fvms+=32;
			status = *(int *)fvms;
			fvms+=sizeof(int);
			fvmData = m_Parent->FvmDataFind(fvmId, 1);
			if(fvmData){
				fvmData->status = status;
			}
		}
	}

	CListCtrl& lc = GetListCtrl();

	lc.DeleteAllItems();

	i = 0;
	while(temp){
//		MessageBox(temp->fvmName);
		lc.InsertItem(i, temp->fvmName, temp->status);

		if(temp == m_Parent->m_selectedFvm)
			lc.SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);

		i++;
		temp = temp->next;
	}
}
