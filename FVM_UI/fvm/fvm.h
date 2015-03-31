// fvm.h : main header file for the FVM application
//

#if !defined(AFX_FVM_H__1C8ABDC0_787A_433D_9D59_AFA607914898__INCLUDED_)
#define AFX_FVM_H__1C8ABDC0_787A_433D_9D59_AFA607914898__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CFvmApp:
// See fvm.cpp for the implementation of this class
//

class CFvmApp : public CWinApp
{
public:
	CFvmApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFvmApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CFvmApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FVM_H__1C8ABDC0_787A_433D_9D59_AFA607914898__INCLUDED_)
