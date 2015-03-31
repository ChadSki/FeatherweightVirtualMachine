#if !defined(AFX_CONFIGDIALOG_H__025046E4_0EA3_4A24_8A3A_FF9C63E3EE0D__INCLUDED_)
#define AFX_CONFIGDIALOG_H__025046E4_0EA3_4A24_8A3A_FF9C63E3EE0D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigDialog.h : header file
//

#include "MainFrm.h"
/////////////////////////////////////////////////////////////////////////////
// CConfigDialog dialog

class CConfigDialog : public CDialog
{
// Construction
public:
	CString Title;
	CMainFrame * m_Parent;
	CConfigDialog(CWnd* pParent = NULL, POSITION pos = NULL, LPCTSTR title=NULL);   // standard constructor
    t_FvmData * fvmd;
// Dialog Data
	//{{AFX_DATA(CConfigDialog)
	enum { IDD = IDD_CONFIG_DIALOG };
	CListBox  	m_sp;
	CEdit	m_mp;
	CEdit	m_mcm;
	CEdit	m_mws;

	CButton	m_OKCtrl;
	CString	m_mpStr;
	CString	m_spStr;
	CString	m_mcmStr;
	CString	m_mwsStr;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CConfigDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeMaxProcesses();
	afx_msg void OnChangeSchePriority();
	virtual void OnOK();
	afx_msg void OnChangeMaxComMemory();
	afx_msg void OnChangeMaxWorkSet();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGDIALOG_H__025046E4_0EA3_4A24_8A3A_FF9C63E3EE0D__INCLUDED_)
