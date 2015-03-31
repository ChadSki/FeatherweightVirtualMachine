#if !defined(AFX_SHORTCUTVIEW_H__2684F9A2_F774_4F44_BF2B_2802595F839A__INCLUDED_)
#define AFX_SHORTCUTVIEW_H__2684F9A2_F774_4F44_BF2B_2802595F839A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ShortCutView.h : header file
//
class CMainFrame;
/////////////////////////////////////////////////////////////////////////////
// CShortCutView view

class CShortCutView : public CListView
{
protected:
	CShortCutView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CShortCutView)

// Attributes
public:

	CMainFrame* m_pOwner;
// Operations
public:
	BOOL GetSysImgList();
	CListCtrl *m_listCtrl;
	LPWSTR PathFindExtensionW( LPCWSTR lpszPath );
	LPWSTR PathGetExtensionW(LPCWSTR lpszPath);
	BOOL FvmPathIsExeW (LPCWSTR lpszPath);

	CStringArray shortCuts;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CShortCutView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CShortCutView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CShortCutView)
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnCreateShortcut();
	afx_msg void OnDeleteShortcut();
	afx_msg void OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SHORTCUTVIEW_H__2684F9A2_F774_4F44_BF2B_2802595F839A__INCLUDED_)
