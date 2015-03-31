// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//
#define _UNICODE 
#if !defined(AFX_STDAFX_H__45950E74_E411_422B_A857_AB24699091FF__INCLUDED_)
#define AFX_STDAFX_H__45950E74_E411_422B_A857_AB24699091FF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#define _WIN32_WINNT 0x0500  //for using CreateJobObject() function
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls


#include <afxcmn.h>			// MFC support for Windows Common Controls
#include <afxcview.h>

#pragma warning(disable: 4786)  //for clean the warnings from STL compilation

#ifndef		SAFE_FREE
	#define SAFE_FREE( p )		{ if( p ){ free( (LPVOID)p ); p = NULL; } }
#endif

#endif // _AFX_NO_AFXCMN_SUPPORT


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__45950E74_E411_422B_A857_AB24699091FF__INCLUDED_)
