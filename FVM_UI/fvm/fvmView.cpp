// fvmView.cpp : implementation of the CFvmView class
//

#include "stdafx.h"
#include "fvm.h"

#include "fvmDoc.h"
#include "fvmView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFvmView

IMPLEMENT_DYNCREATE(CFvmView, CView)

BEGIN_MESSAGE_MAP(CFvmView, CView)
	//{{AFX_MSG_MAP(CFvmView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFvmView construction/destruction

CFvmView::CFvmView()
{
	// TODO: add construction code here

}

CFvmView::~CFvmView()
{
}

BOOL CFvmView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CFvmView drawing

void CFvmView::OnDraw(CDC* pDC)
{
	CFvmDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CFvmView printing

BOOL CFvmView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CFvmView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CFvmView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CFvmView diagnostics

#ifdef _DEBUG
void CFvmView::AssertValid() const
{
	CView::AssertValid();
}

void CFvmView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CFvmDoc* CFvmView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFvmDoc)));
	return (CFvmDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFvmView message handlers
