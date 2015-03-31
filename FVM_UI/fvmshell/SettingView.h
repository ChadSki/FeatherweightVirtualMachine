#if !defined(AFX_SETTINGVIEW_H__53DE7E3E_C7AC_41C9_84E4_51DDE6F2CECC__INCLUDED_)
#define AFX_SETTINGVIEW_H__53DE7E3E_C7AC_41C9_84E4_51DDE6F2CECC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SettingView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSettingView dialog

class CSettingView : public CDialog
{
// Construction
public:
	CSettingView(CWnd* pParent = NULL);   // standard constructor
	CButton m_RadioLarge;
	CButton m_RadioSmall;
	CButton m_RadioList;
	CButton m_RadioDetails;
// Dialog Data
	//{{AFX_DATA(CSettingView)
	enum { IDD = IDD_SETTING_VIEW };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSettingView)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSettingView)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETTINGVIEW_H__53DE7E3E_C7AC_41C9_84E4_51DDE6F2CECC__INCLUDED_)
