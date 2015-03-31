// fvmDoc.cpp : implementation of the CFvmDoc class
//

#include "stdafx.h"
#include "fvm.h"

#include "fvmDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFvmDoc

IMPLEMENT_DYNCREATE(CFvmDoc, CDocument)

BEGIN_MESSAGE_MAP(CFvmDoc, CDocument)
	//{{AFX_MSG_MAP(CFvmDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFvmDoc construction/destruction

CFvmDoc::CFvmDoc()
{
	// TODO: add one-time construction code here

}

CFvmDoc::~CFvmDoc()
{
}

BOOL CFvmDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CFvmDoc serialization

void CFvmDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CFvmDoc diagnostics

#ifdef _DEBUG
void CFvmDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CFvmDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFvmDoc commands
