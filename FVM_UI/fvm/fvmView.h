// fvmView.h : interface of the CFvmView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_FVMVIEW_H__87FE2AE0_4889_4DE3_8CEA_F47ACCBF4EDD__INCLUDED_)
#define AFX_FVMVIEW_H__87FE2AE0_4889_4DE3_8CEA_F47ACCBF4EDD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CFvmView : public CView
{
protected: // create from serialization only
	CFvmView();
	DECLARE_DYNCREATE(CFvmView)

// Attributes
public:
	CFvmDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFvmView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFvmView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CFvmView)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in fvmView.cpp
inline CFvmDoc* CFvmView::GetDocument()
   { return (CFvmDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FVMVIEW_H__87FE2AE0_4889_4DE3_8CEA_F47ACCBF4EDD__INCLUDED_)
