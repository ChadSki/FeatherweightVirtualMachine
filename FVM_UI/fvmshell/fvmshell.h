// fvmshell.h : main header file for the FVMSHELL application
//

#if !defined(AFX_FVMSHELL_H__560A23C9_1D03_4BF8_BEB6_7BF02AAD0926__INCLUDED_)
#define AFX_FVMSHELL_H__560A23C9_1D03_4BF8_BEB6_7BF02AAD0926__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CFvmshellApp:
// See fvmshell.cpp for the implementation of this class
//

class CFvmshellApp : public CWinApp
{
public:
	CFvmshellApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFvmshellApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CFvmshellApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FVMSHELL_H__560A23C9_1D03_4BF8_BEB6_7BF02AAD0926__INCLUDED_)
