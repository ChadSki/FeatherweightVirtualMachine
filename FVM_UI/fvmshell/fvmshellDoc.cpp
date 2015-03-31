// fvmshellDoc.cpp : implementation of the CFvmshellDoc class
//

#include "stdafx.h"
#include "fvmshell.h"

#include "fvmshellDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFvmshellDoc

IMPLEMENT_DYNCREATE(CFvmshellDoc, CDocument)

BEGIN_MESSAGE_MAP(CFvmshellDoc, CDocument)
	//{{AFX_MSG_MAP(CFvmshellDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFvmshellDoc construction/destruction

CFvmshellDoc::CFvmshellDoc()
{
	// TODO: add one-time construction code here

}

CFvmshellDoc::~CFvmshellDoc()
{
}

BOOL CFvmshellDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CFvmshellDoc serialization

void CFvmshellDoc::Serialize(CArchive& ar)
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
// CFvmshellDoc diagnostics

#ifdef _DEBUG
void CFvmshellDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CFvmshellDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFvmshellDoc commands
