#if !defined(AFX_FILEVIEW_H__0233B3DD_DC72_4A99_B7CA_024177222398__INCLUDED_)
#define AFX_FILEVIEW_H__0233B3DD_DC72_4A99_B7CA_024177222398__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FileView.h : header file
//

#include "LocalFileListCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CFileView view

class CMainFrame;

class CFileView : public CView
{
protected:
	CFileView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CFileView)

// Attributes
public:

// Operations
public:
	CMainFrame * m_Parent;
	CLocalFileListCtrl *m_pListCtrl;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFileView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CFileView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CFileView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILEVIEW_H__0233B3DD_DC72_4A99_B7CA_024177222398__INCLUDED_)
