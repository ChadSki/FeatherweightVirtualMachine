// FvmFileView.cpp : implementation file
//

#include "stdafx.h"
#include "fvmshell.h"
#include "FvmFileView.h"
#include  "MainFrm.h"
#include "LocalFileListCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFvmFileView

IMPLEMENT_DYNCREATE(CFvmFileView, CView)

CFvmFileView::CFvmFileView()
{
}

CFvmFileView::~CFvmFileView()
{
}


BEGIN_MESSAGE_MAP(CFvmFileView, CView)
	//{{AFX_MSG_MAP(CFvmFileView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SHOWWINDOW()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFvmFileView drawing

void CFvmFileView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CFvmFileView diagnostics

#ifdef _DEBUG
void CFvmFileView::AssertValid() const
{
	CView::AssertValid();
}

void CFvmFileView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFvmFileView message handlers

void CFvmFileView::SetLocalFolderOut(CString folder)
{
		
	m_pOwner->SetLocalFolder(folder);
}

void CFvmFileView::SetLocalFolder(CString folder)
{
	
	m_pListCtrl->SetFolder(folder);
}


int CFvmFileView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	m_pListCtrl=new CLocalFileListCtrl(this);
	m_pListCtrl->Create(LVS_REPORT|WS_CHILD | WS_VISIBLE | LVS_SHOWSELALWAYS|LVS_SHAREIMAGELISTS|LVS_EDITLABELS|LVS_OWNERDATA,CRect(0,0,0,0),this,0);


	return 0;
}

void CFvmFileView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	m_pListCtrl->SetWindowPos( NULL, 0, 0, cx, cy, SWP_NOZORDER);
}

CString CFvmFileView::GetLocalFolder()
{
	return m_pListCtrl->GetFolder();
}

BOOL CFvmFileView::SetStatusBarText(LPCTSTR pszText)
{
	ASSERT(pszText);
	m_pOwner->SetStatusBarText(pszText);
	
	return TRUE;
}
void CFvmFileView::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CView::OnShowWindow(bShow, nStatus);
	
	
	// TODO: Add your message handler code here
	
}

void CFvmFileView::OnDestroy() 
{
	CView::OnDestroy();
	
	// TODO: Add your message handler code here
	delete m_pListCtrl;
}
