// FileView.cpp : implementation file
//

#include "stdafx.h"
#include "fvm.h"
#include "FileView.h"
#include "LocalFileListCtrl.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFileView


IMPLEMENT_DYNCREATE(CFileView, CView)

CFileView::CFileView()
{
		m_Parent = NULL;
}


CFileView::~CFileView()
{
}


BEGIN_MESSAGE_MAP(CFileView, CView)
	//{{AFX_MSG_MAP(CFileView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileView drawing

void CFileView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CFileView diagnostics

#ifdef _DEBUG
void CFileView::AssertValid() const
{
	CView::AssertValid();
}

void CFileView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFileView message handlers

int CFileView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	
	m_pListCtrl=new CLocalFileListCtrl(this);
	m_pListCtrl->Create(LVS_REPORT|WS_CHILD | WS_VISIBLE | LVS_SHOWSELALWAYS|LVS_SHAREIMAGELISTS|LVS_EDITLABELS|LVS_OWNERDATA,CRect(0,0,0,0),this,0);

	m_pListCtrl->m_pOwner = this;
	
//	m_pListCtrl->DisplayFiles(L"i:\\vm1");
	return 0;
}

void CFileView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	m_pListCtrl->SetWindowPos( NULL, 0, 0, cx, cy, SWP_NOZORDER);	
}

void CFileView::OnDestroy() 
{
	CView::OnDestroy();
	
	// TODO: Add your message handler code here
	delete m_pListCtrl;
}
