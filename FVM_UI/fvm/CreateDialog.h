#if !defined(AFX_CREATEDIALOG_H__025046E4_0EA3_4A24_8A3A_FF9C63E3EE0D__INCLUDED_)
#define AFX_CREATEDIALOG_H__025046E4_0EA3_4A24_8A3A_FF9C63E3EE0D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CreateDialog.h : header file
//

#include "MainFrm.h"
/////////////////////////////////////////////////////////////////////////////
// CCreateDialog dialog

class CCreateDialog : public CDialog
{
// Construction
public:
	CString Title;
	CMainFrame * m_Parent;
	CCreateDialog(CWnd* pParent = NULL, LPCTSTR title=NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCreateDialog)
	enum { IDD = IDD_CREATE_DIALOG };
	CEdit	m_Fvm;
	CEdit	m_Folder;
	CEdit	m_FvmIp;
	CEdit	m_FvmIpMask;

	CButton	m_OKCtrl;
	CString	m_FolderStr;
	CString	m_FvmStr;
	CString	m_FvmIpStr;
	CString	m_FvmIpMaskStr;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCreateDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCreateDialog)
	afx_msg void OnBrowse();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeFvmFolder();
	afx_msg void OnChangeFvmName();
	virtual void OnOK();
	afx_msg void OnChangeFvmIp();
	afx_msg void OnChangeFvmIpMask();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CREATEDIALOG_H__025046E4_0EA3_4A24_8A3A_FF9C63E3EE0D__INCLUDED_)
