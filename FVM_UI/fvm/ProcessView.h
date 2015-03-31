#if !defined(AFX_PROCESSVIEW_H__09183DE0_297F_492B_98C7_20D266E44D82__INCLUDED_)
#define AFX_PROCESSVIEW_H__09183DE0_297F_492B_98C7_20D266E44D82__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProcessView.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// CProcessView view

class CMainFrame;

class CProcessView : public CListView
{
protected:
	CProcessView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CProcessView)

// Attributes
public:

// Operations
public:
	void OnContextMenuEndProcess();
	CMainFrame * m_Parent;
	BOOL GetSysImgList();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProcessView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CProcessView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CProcessView)
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROCESSVIEW_H__09183DE0_297F_492B_98C7_20D266E44D82__INCLUDED_)
