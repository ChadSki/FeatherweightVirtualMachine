#if !defined(AFX_SETTING_H__811B73AA_A9E9_4819_90C1_C3A6452A10D4__INCLUDED_)
#define AFX_SETTING_H__811B73AA_A9E9_4819_90C1_C3A6452A10D4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Setting.h : header file
//

#include "MainFrm.h"
/////////////////////////////////////////////////////////////////////////////
// CSetting dialog

class CSetting : public CDialog
{
// Construction
public:
	CSetting(CWnd* pParent = NULL);   // standard constructor
	void ActivateTabDialogs();
	int m_DialogID[2];
	CDialog *m_DialogFocus;
	CMainFrame *m_Parent;
// Dialog Data
	//{{AFX_DATA(CSetting)
	enum { IDD = IDD_SETTING };
	CTabCtrl	m_SettingTab;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSetting)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSetting)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETTING_H__811B73AA_A9E9_4819_90C1_C3A6452A10D4__INCLUDED_)
