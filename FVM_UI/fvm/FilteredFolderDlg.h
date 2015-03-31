 
//////////////////////////////////////////////////////////////////////
// Custom filtering is based on Arik Poznanski's article
// C# does Shell, Part 1
// http://www.codeproject.com/csharp/csdoesshell1.asp
//////////////////////////////////////////////////////////////////////

#ifndef __FILTERED_FOLDERDLG_H__
#define __FILTERED_FOLDERDLG_H__

#if defined( _MSC_VER ) && ( _MSC_VER >= 1020 )
	#pragma once
#endif

#ifndef __FOLDERDLG_H__
	#include "folderdlg.h"
#endif

#if defined	( __IFolderFilterSite_FWD_DEFINED__			)
	#if defined ( __IFolderFilterSite_INTERFACE_DEFINED__	)
		#if defined ( __IFolderFilter_FWD_DEFINED__				)
			#if defined ( __IFolderFilter_INTERFACE_DEFINED__		)
				#define USE_XP_FILTRATION
			#endif
		#endif
	#endif
#endif

//////////////////////////////////////////////////////////////////////
#ifdef USE_XP_FILTRATION
//////////////////////////////////////////////////////////////////////

#ifndef		 _INC_SHLWAPI
	#include < shlwapi.h >				// Required for the StrRet and Path functions
#endif
#pragma comment( lib, "shlwapi.lib" )

//////////////////////////////////////////////////////////////////////
// IMyFolderFilter

class IMyFolderFilter : public IFolderFilter
{
public:
	IMyFolderFilter( VOID );
	
public: // IUnknown implementation

	STDMETHOD( QueryInterface )( IN REFIID /*riid*/, IN OUT LPVOID* /*ppvObj*/ );
	STDMETHOD_( ULONG, AddRef )( VOID );
	STDMETHOD_( ULONG, Release )( VOID );

public: // IFolderFilter implementation

	STDMETHOD( ShouldShow )( IN IShellFolder* /*pIShellFolder*/, IN LPCITEMIDLIST /*pidlFolder*/, IN LPCITEMIDLIST IN /*pidlItem*/ );
	STDMETHOD( GetEnumFlags )( IN IShellFolder* /*pIShellFolder*/, IN LPCITEMIDLIST /*pidlFolder*/, IN HWND* /*phWnd*/, OUT LPDWORD /*pdwFlags*/ );

protected:
	ULONG	m_ulRef;

public:
	LPCTSTR		m_pszFilter;
};

//////////////////////////////////////////////////////////////////////
// CFilteredFolderDialog class

class CFilteredFolderDialog : public CFolderDialog  
{
public:
	CFilteredFolderDialog( LPCTSTR pszTitle		= NULL, 
						   LPCTSTR pszSelPath	= NULL,
						   CWnd*   pParentWnd	= NULL,
						   UINT	   uFlags		= BIF_RETURNONLYFSDIRS  );

	virtual ~CFilteredFolderDialog( VOID );

public:
	VOID SetFilter( IN LPCTSTR pszFilter );

protected:
	virtual VOID	OnIUnknown( IN IUnknown* pIUnknown );

protected:	
	//CStringArray		m_straFilters;
	LPCTSTR				m_pszFilter;
	IMyFolderFilter*	m_pIMyFolderFilter;
};

//////////////////////////////////////////////////////////////////////
// CMyFolderFilter class

//class CFolderFilter : public CCmdTarget
//{
//	friend class CFilteredFolderDialog;
//
//public: // IFolderFilter methods	
//
//	DECLARE_DYNCREATE( CFolderFilter );
//	DECLARE_INTERFACE_MAP();
//
//	BEGIN_INTERFACE_PART( FolderFilterObj, IFolderFilter )        
//		STDMETHOD( ShouldShow )( IShellFolder* pIShellFolder, LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlItem );
//		STDMETHOD( GetEnumFlags )( IShellFolder* pIShellFolder, LPCITEMIDLIST pidlFolder, HWND* phWnd, LPDWORD pdwFlags );
//	END_INTERFACE_PART( FolderFilterObj )
//
//protected:
//	CStringArray*	m_pStraFilters;
//};

//////////////////////////////////////////////////////////////////////
#endif // USE_XP_FILTRATION
//////////////////////////////////////////////////////////////////////
#endif // __FILTERED_FOLDERDLG_H__
//////////////////////////////////////////////////////////////////////
