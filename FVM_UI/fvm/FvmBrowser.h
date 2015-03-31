#if !defined(AFX_FVMBROWSER_H__1B99A4B2_204C_49C4_9F18_88A4BD169494__INCLUDED_)
#define AFX_FVMBROWSER_H__1B99A4B2_204C_49C4_9F18_88A4BD169494__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FvmBrowser.h : header file
//
class CMainFrame;

/////////////////////////////////////////////////////////////////////////////
// CFvmBrowser view

class CFvmBrowser : public CListView
{
protected:
	CFvmBrowser();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CFvmBrowser)

// Attributes
public:
	CImageList m_imgList;
// Operations
public:
	void InsertFvms();
	CMainFrame * m_Parent;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFvmBrowser)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CFvmBrowser();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CFvmBrowser)
	afx_msg void OnItemchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemclick(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FVMBROWSER_H__1B99A4B2_204C_49C4_9F18_88A4BD169494__INCLUDED_)
