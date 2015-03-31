// fvmshellView.cpp : implementation of the CFvmDirView class
//

#include "stdafx.h"
#include "fvmshell.h"

#include "fvmshellDoc.h"
#include "FvmDirView.h"
#include "DirTreeCtrl.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFvmDirView

IMPLEMENT_DYNCREATE(CFvmDirView, CView)

BEGIN_MESSAGE_MAP(CFvmDirView, CView)
	//{{AFX_MSG_MAP(CFvmDirView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFvmDirView construction/destruction

CFvmDirView::CFvmDirView()
{
	// TODO: add construction code here

}

CFvmDirView::~CFvmDirView()
{
}

BOOL CFvmDirView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CFvmDirView drawing

void CFvmDirView::OnDraw(CDC* pDC)
{
	CFvmshellDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CFvmDirView printing

BOOL CFvmDirView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CFvmDirView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CFvmDirView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CFvmDirView diagnostics

#ifdef _DEBUG
void CFvmDirView::AssertValid() const
{
	CView::AssertValid();
}

void CFvmDirView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CFvmshellDoc* CFvmDirView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFvmshellDoc)));
	return (CFvmshellDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFvmDirView message handlers

int CFvmDirView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_pDirTree=new CDirTreeCtrl;
	BOOL bRet = m_pDirTree->Create(WS_CHILD | TVS_LINESATROOT | 
                          TVS_HASBUTTONS | WS_VISIBLE | 
                          TVS_HASLINES | TVS_SHOWSELALWAYS |
						  TVS_EDITLABELS, 
                          CRect(0, 21, 0, 0), 
                          this,0);
    if (bRet)
	{
		m_pDirTree->m_pOwner=this;
		m_pDirTree->DisplayTree( NULL, FALSE );
	}
	
	return bRet;
	// TODO: Add your specialized creation code here
	
	return 0;
}

void CFvmDirView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
		//if (m_pDirTree->m_hWnd)
		//m_pDirTree->SetWindowPos( NULL, 0, comboRect.bottom, cx, cy-comboRect.bottom, SWP_NOZORDER );
		CView::OnSize(nType, cx, cy);
	if (m_pDirTree->m_hWnd)
		m_pDirTree->SetWindowPos( NULL, 0, 0, cx, cy, SWP_NOZORDER );
	
}
void CFvmDirView::SetLocalFolderOut(CString folder)
{
	m_pOwner->SetLocalFolder(folder);
}

void CFvmDirView::SetLocalFolder(CString folder)
{
	m_pDirTree->SetSelPath(folder);
}

CString CFvmDirView::GetLocalFolder()
{
	return m_pDirTree->GetFullPath(m_pDirTree->GetSelectedItem());
}


CDirTreeCtrl* CFvmDirView::GetTreeCtrl()
{
	return m_pDirTree;
}

void CFvmDirView::OnDestroy() 
{
	CView::OnDestroy();
	
	// TODO: Add your message handler code here
	delete m_pDirTree;
}
