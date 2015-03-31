// fvmshellView.h : interface of the CFvmDirView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_FVMSHELLVIEW_H__FBBA271F_7F87_4174_BCD6_6A38F3F5545D__INCLUDED_)
#define AFX_FVMSHELLVIEW_H__FBBA271F_7F87_4174_BCD6_6A38F3F5545D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDirTreeCtrl;
class CMainFrame;

class CFvmDirView : public CView
{
protected: // create from serialization only
	CFvmDirView();
	DECLARE_DYNCREATE(CFvmDirView)

// Attributes
public:
	CFvmshellDoc* GetDocument();

// Operations
public:
	void SetLocalFolder(CString folder);
	CMainFrame *m_pOwner;
	void SetLocalFolderOut(CString folder);
	CString GetLocalFolder();
	
	CDirTreeCtrl* GetTreeCtrl();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFvmDirView)
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
	virtual ~CFvmDirView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	CDirTreeCtrl *m_pDirTree;
	//{{AFX_MSG(CFvmDirView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in fvmshellView.cpp
inline CFvmshellDoc* CFvmDirView::GetDocument()
   { return (CFvmshellDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FVMSHELLVIEW_H__FBBA271F_7F87_4174_BCD6_6A38F3F5545D__INCLUDED_)
