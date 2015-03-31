#if !defined(AFX_FVMFILEVIEW_H__C5B41A6C_6D67_4A83_8145_3C4C4818EA13__INCLUDED_)
#define AFX_FVMFILEVIEW_H__C5B41A6C_6D67_4A83_8145_3C4C4818EA13__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FvmFileView.h : header file
//

class CMainFrame;
class CLocalFileListCtrl;
/////////////////////////////////////////////////////////////////////////////
// CFvmFileView view

class CFvmFileView : public CView
{
protected:
	CFvmFileView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CFvmFileView)

// Attributes
public:

// Operations
public:
	CLocalFileListCtrl *m_pListCtrl;
	void SetLocalFolder(CString folder);
	void SetLocalFolderOut(CString folder);
	CString GetLocalFolder();
	BOOL SetStatusBarText(LPCTSTR pszText);

	CMainFrame* m_pOwner;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFvmFileView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CFvmFileView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CFvmFileView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FVMFILEVIEW_H__C5B41A6C_6D67_4A83_8145_3C4C4818EA13__INCLUDED_)
