#if !defined(AFX_CREATESHORTCUT_H__DC914CB4_89DC_4906_9037_0F8F03E386BB__INCLUDED_)
#define AFX_CREATESHORTCUT_H__DC914CB4_89DC_4906_9037_0F8F03E386BB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CreateShortcut.h : header file
//

#include "ShortCutView.h"
/////////////////////////////////////////////////////////////////////////////
// CCreateShortcut dialog

class CCreateShortcut : public CDialog
{
// Construction
public:
	CCreateShortcut(CWnd* pParent = NULL);   // standard constructor

	CShortCutView *m_parentOwner;
// Dialog Data
	//{{AFX_DATA(CCreateShortcut)
	enum { IDD = IDD_SHORTCUT };
	CButton	m_OkCtrl;
	CEdit	m_pathName;
	CString	m_String;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCreateShortcut)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCreateShortcut)
	afx_msg void OnShortcutBrows();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeShortcutEdit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CREATESHORTCUT_H__DC914CB4_89DC_4906_9037_0F8F03E386BB__INCLUDED_)
