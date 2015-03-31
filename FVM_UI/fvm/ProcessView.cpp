// ProcessView.cpp : implementation file
//

#include "stdafx.h"
#include "fvm.h"
#include "ProcessView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProcessView

IMPLEMENT_DYNCREATE(CProcessView, CListView)

CProcessView::CProcessView()
{
}

CProcessView::~CProcessView()
{
}


BEGIN_MESSAGE_MAP(CProcessView, CListView)
	//{{AFX_MSG_MAP(CProcessView)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_PROCESS_KILL, OnContextMenuEndProcess)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProcessView drawing

void CProcessView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CProcessView diagnostics

#ifdef _DEBUG
void CProcessView::AssertValid() const
{
	CListView::AssertValid();
}

void CProcessView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CProcessView message handlers

void CProcessView::OnInitialUpdate() 
{
	CListView::OnInitialUpdate();
	
	// TODO: Add your specialized code here and/or call the base class
		CListCtrl& lc = GetListCtrl();
	//lc.ModifyStyle(0,LVS_REPORT);
    lc.ModifyStyle(0,/*LVS_ICON*/LVS_REPORT|LVS_SINGLESEL|LVM_SORTITEMS); 
	lc.SetExtendedStyle(LVS_EX_TRACKSELECT | LVS_EX_ONECLICKACTIVATE);



	lc.InsertColumn(0, _T("Process Name"), LVCFMT_LEFT);
	lc.InsertColumn(1, _T("PID"), LVCFMT_LEFT);
	lc.SetColumnWidth(0,200);
	lc.SetColumnWidth(1,55);

	GetSysImgList();
}

/////////////////////////////////////////////////
BOOL CProcessView::GetSysImgList()
/////////////////////////////////////////////////
{
	CImageList sysImgList;
	SHFILEINFO shFinfo;
	
	HIMAGELIST hImageList = 0;
	CString errorMessage;
	TCHAR filename[MAX_PATH + 10];

	if (GetModuleFileName(0, filename, MAX_PATH + 10))
	{
		hImageList = (HIMAGELIST)SHGetFileInfo(filename,
							 0,
							 &shFinfo,
							 sizeof( shFinfo ),
							 SHGFI_SYSICONINDEX |
							 SHGFI_SMALLICON );

		if (!hImageList)
		{
			int errorCode = GetLastError();
			TCHAR buffer[1000];
			memset(buffer, 0, 1000);
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, errorCode, 0, buffer, 999, 0);

			CString str;
			str.Format(_T("SHGetFileInfo failed with error %d: %s"), errorCode, buffer);
			errorMessage += str;
		}
	}
	else
	{
		int errorCode = GetLastError();
		TCHAR buffer[1000];
		memset(buffer, 0, 1000);
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, errorCode, 0, buffer, 999, 0);
		
		CString str;
		str.Format(_T("GetModuleFileName failed with error %d: %s"), errorCode, buffer);
		errorMessage += str;
	}
	if (!hImageList)
	{
		/*
		 * Fall back to C:\\
		 * Not bullerproof, but better than nothing
		 */		
		hImageList = (HIMAGELIST)SHGetFileInfo(_T("C:\\"),
							 0,
							 &shFinfo,
							 sizeof( shFinfo ),
							 SHGFI_SYSICONINDEX |
							 SHGFI_SMALLICON );

		if (!hImageList)
		{
			int errorCode = GetLastError();
			TCHAR buffer[1000];
			memset(buffer, 0, 1000);
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, errorCode, 0, buffer, 999, 0);

			CString str;
			str.Format(_T("SHGetFileInfo failed with error %d: %s"), errorCode, buffer);
			if (errorMessage != _T(""))
				errorMessage += _T("\n");
			errorMessage += str;
		}
	}

	if (!hImageList)
	{
		AfxMessageBox(errorMessage);
		return FALSE;
	}

	sysImgList.Attach(hImageList);
	CListCtrl& lc = GetListCtrl();

	lc.SetImageList( &sysImgList, LVSIL_SMALL);
	sysImgList.Detach();

	return TRUE;
}

void CProcessView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// TODO: Add your message handler code here
		

	CListCtrl& lc = GetListCtrl();

	if (!lc.GetItemCount())
		return;

	CMenu menu;
	menu.LoadMenu(IDR_PROCESSVIEW);

	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);
	CWnd* pWndPopupOwner = this;
	//while (pWndPopupOwner->GetStyle() & WS_CHILD)
	//	pWndPopupOwner = pWndPopupOwner->GetParent();

	POSITION selpos = lc.GetFirstSelectedItemPosition();


	pPopup->EnableMenuItem(ID_PROCESS_ENDPROCESSTREE, MF_GRAYED);	
	pPopup->EnableMenuItem(ID_PROCESS_PROPERTIES, MF_GRAYED);	

	if (!selpos)
	{
		pPopup->EnableMenuItem(ID_PROCESS_KILL, MF_GRAYED);	

		if (point.x==-1 || point.y==-1)
		{
			point.x=5;
			point.y=5;
			ClientToScreen(&point);
		}
	}


	else
	{
		int nItem = lc.GetNextSelectedItem(selpos);
		if (point.x==-1 || point.y==-1)
		{
			CRect rect;
			lc.GetItemRect(nItem,&rect,LVIR_LABEL);
			point.x=rect.left+5;
			point.y=rect.top+5;
			ClientToScreen(&point);
		}
	
		
	
		nItem = lc.GetNextSelectedItem(selpos);
		while (nItem!=-1)
		{
			pPopup->EnableMenuItem(ID_PROCESS_KILL, MF_GRAYED);
		
			nItem = lc.GetNextSelectedItem(selpos);
		}
		
	}
	
		
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,
		pWndPopupOwner);
		
}

void CProcessView::OnContextMenuEndProcess()
{

	CListCtrl& lc = GetListCtrl();

	if (!lc.GetItemCount())
		return;


	POSITION selpos = lc.GetFirstSelectedItemPosition();

	if (!selpos)
	{
	
	}


	else
	{
		int nItem = lc.GetNextSelectedItem(selpos);
	
		
	
	//	nItem = lc.GetNextSelectedItem(selpos);
		while (nItem!=-1)
		{
			CString name = lc.GetItemText(nItem,1);
			//MessageBox(name);
			int pid = _wtoi(name);




			HANDLE hd = OpenProcess(PROCESS_ALL_ACCESS,FALSE,pid);
			if(TerminateProcess(hd,0))
				lc.DeleteItem(nItem);

			nItem = lc.GetNextSelectedItem(selpos);
		
		}
		
	}
	//m_Parent->UpdateDisplays();

}
