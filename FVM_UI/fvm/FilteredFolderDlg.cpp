//////////////////////////////////////////////////////////////////////
// Custom filtering is based on Arik Poznanski's article
// C# does Shell, Part 1
// http://www.codeproject.com/csharp/csdoesshell1.asp
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FilteredFolderDlg.h"

//////////////////////////////////////////////////////////////////////
#ifdef USE_XP_FILTRATION
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CFilteredFolderDialog class

CFilteredFolderDialog::CFilteredFolderDialog( LPCTSTR pszTitle,
											  LPCTSTR pszSelPath,
											  CWnd*	  pParentWnd,
											  UINT	  uFlags )
					 : CFolderDialog( pszTitle, pszSelPath, pParentWnd, uFlags )
					 , m_pIMyFolderFilter( NULL )
					 , m_pszFilter( NULL )
{	
}

CFilteredFolderDialog::~CFilteredFolderDialog( VOID )
{
	SAFE_FREE( m_pszFilter );
	//SAFE_RELEASEINTERFACE( m_pIFolderFilter )
}

VOID CFilteredFolderDialog::OnIUnknown( IN IUnknown* pIUnknown )
{
	ASSERT( pIUnknown != NULL );

	if( m_pIMyFolderFilter == NULL )
	{	
		IFolderFilterSite* pIFolderFilterSite = NULL;

		HRESULT hResult = pIUnknown->QueryInterface( IID_IFolderFilterSite, (LPVOID*)&pIFolderFilterSite );
		if( SUCCEEDED( hResult ) )
		{
			m_pIMyFolderFilter = new IMyFolderFilter;
			ASSERT( m_pIMyFolderFilter != NULL );

			if( m_pIMyFolderFilter )
				pIFolderFilterSite->SetFilter( (IUnknown*)m_pIMyFolderFilter );

			SAFE_RELEASE( pIFolderFilterSite );
		}
	}

	if( m_pIMyFolderFilter )
		m_pIMyFolderFilter->m_pszFilter = m_pszFilter;
}

VOID CFilteredFolderDialog::SetFilter( IN LPCTSTR pszFilter )
{
	ASSERT( pszFilter != NULL );

	SAFE_FREE( m_pszFilter );
	m_pszFilter = _tcsdup( pszFilter );
}

//////////////////////////////////////////////////////////////////////
// IMyFolderFilter

IMyFolderFilter::IMyFolderFilter( VOID )
			   : m_ulRef( 0 )
			   , m_pszFilter( NULL )
{
}

STDMETHODIMP IMyFolderFilter::QueryInterface( IN     REFIID  riid, 
											  IN OUT LPVOID* ppvObj )
{
	ATLASSERT( ppvObj != NULL );
	if( !ppvObj )
		return ( E_FAIL );

	HRESULT hResult = ( E_NOINTERFACE );
	*ppvObj = NULL;

	if( IsEqualIID( riid, IID_IUnknown ) )
		*ppvObj =  static_cast< IUnknown* >( this );
	else if( IsEqualIID( riid, IID_IFolderFilter ) )
		*ppvObj =  static_cast< IFolderFilter* >( this );
	
	if( *ppvObj )
	{
		reinterpret_cast< IUnknown* >( *ppvObj )->AddRef();
		hResult = ( S_OK );
	}

	return ( hResult );
}

STDMETHODIMP_( ULONG ) IMyFolderFilter::AddRef( VOID )
{
	return ::InterlockedIncrement( (PLONG)&m_ulRef );
}

STDMETHODIMP_( ULONG ) IMyFolderFilter::Release( VOID )
{
	ULONG ulNewRef = ::InterlockedDecrement( (PLONG)&m_ulRef );
	if( ulNewRef <= 0 )
	{
		delete this;
		return ( 0 );
	}

	return ( ulNewRef );
}

STDMETHODIMP IMyFolderFilter::ShouldShow( IN IShellFolder* pIShellFolder, 
										  IN LPCITEMIDLIST /*pidlFolder*/, 
										  IN LPCITEMIDLIST pidlItem )
{
	ASSERT( pIShellFolder != NULL );	
	ASSERT( pidlItem	  != NULL );
	
	ASSERT( m_pszFilter != NULL );
	if( !m_pszFilter )
		return ( E_FAIL );

	// If an item is a folder, then accept it

	LPCITEMIDLIST pidl[ 1 ] = { pidlItem };
	SFGAOF		  ulAttr	 = SFGAO_FOLDER;

	pIShellFolder->GetAttributesOf( 1, pidl, &ulAttr );

	if ( ( ulAttr & SFGAO_FOLDER ) ==  SFGAO_FOLDER )
		return ( S_OK );

	// If an item is a file, then filter it
	
	STRRET  strRet  = { 0 };
	HRESULT hResult = pIShellFolder->GetDisplayNameOf( 
		pidlItem, SHGDN_NORMAL | SHGDN_FORPARSING, &strRet
	);

	if( SUCCEEDED( hResult ) )
	{	
		LPTSTR pszDisplayName = NULL;
		//fanglu
		hResult = ::StrRetToStr( &strRet, pidlItem, &pszDisplayName );

		if( SUCCEEDED( hResult ) )
		{
			hResult = ( S_FALSE );

			if( ::PathMatchSpec( pszDisplayName, m_pszFilter ) )
				hResult = ( S_OK );

			SAFE_COTASKMEMFREE( pszDisplayName );
		}
	}

	return ( hResult );
}

STDMETHODIMP IMyFolderFilter::GetEnumFlags( IN  IShellFolder*	/*pIShellFolder*/, 
											IN  LPCITEMIDLIST	/*pidlFolder*/, 
											IN  HWND*			/*phWnd*/,
											OUT LPDWORD			pdwFlags )
{
	ASSERT( pdwFlags != NULL );

	*pdwFlags = (DWORD)( SHCONTF_FOLDERS | SHCONTF_NONFOLDERS );
	return ( S_OK );
}

//////////////////////////////////////////////////////////////////////
// CFolderFilter

//IMPLEMENT_DYNCREATE( CFolderFilter, CCmdTarget );
//
//BEGIN_INTERFACE_MAP( CFolderFilter, CCmdTarget )
//    INTERFACE_PART( CFolderFilter, IID_IFolderFilter, FolderFilterObj )    
//END_INTERFACE_MAP()
//
//STDMETHODIMP_( ULONG ) CFolderFilter::XFolderFilterObj::AddRef( VOID )
//{
//    METHOD_PROLOGUE( CFolderFilter, FolderFilterObj )
//    return ( pThis->ExternalAddRef() );
//}
//
//STDMETHODIMP_( ULONG ) CFolderFilter::XFolderFilterObj::Release( VOID )
//{
//    METHOD_PROLOGUE( CFolderFilter, FolderFilterObj )
//    return ( pThis->ExternalRelease() );
//}
//
//STDMETHODIMP CFolderFilter::XFolderFilterObj::QueryInterface( IN REFIID  refIid, 
//															  IN LPVOID* ppvObj )
//{
//    METHOD_PROLOGUE( CFolderFilter, FolderFilterObj )
//    return ( (HRESULT)pThis->ExternalQueryInterface( &refIid, ppvObj ) );
//}
//
//STDMETHODIMP CFolderFilter::XFolderFilterObj::ShouldShow( IN IShellFolder* pIShellFolder, 
//														  IN LPCITEMIDLIST /*pidlFolder*/, 
//														  IN LPCITEMIDLIST pidlItem )
//{
//	ASSERT( pIShellFolder != NULL );	
//	ASSERT( pidlItem	  != NULL );
//	
//	METHOD_PROLOGUE( CFolderFilter, FolderFilterObj )
//	
//	ASSERT_VALID( pThis->m_pStraFilters );
//	if( !pThis->m_pStraFilters )
//		return ( S_FALSE );
//
//	// If an item is a folder, then accept it
//
//	LPCITEMIDLIST pidl[ 1 ] = { pidlItem };
//	SFGAOF		  ulAttr	 = SFGAO_FOLDER;
//
//	pIShellFolder->GetAttributesOf( 1, pidl, &ulAttr );
//
//	if ( ( ulAttr & SFGAO_FOLDER ) ==  SFGAO_FOLDER )
//		return ( S_OK );
//
//	// If an item is a file, then filter it
//	
//	STRRET  strRet  = { 0 };
//	HRESULT hResult = pIShellFolder->GetDisplayNameOf( 
//		pidlItem, SHGDN_NORMAL | SHGDN_FORPARSING, &strRet
//	);
//
//	if( SUCCEEDED( hResult ) )
//	{	
//		LPTSTR pszDisplayName = NULL;
//		hResult = ::StrRetToStr( &strRet, pidlItem, &pszDisplayName );
//
//		if( SUCCEEDED( hResult ) )
//		{
//			LPCTSTR pszExt = ::CharNext( ::PathFindExtension( pszDisplayName ) );
//			
//			for( INT n = 0; pszExt && ( n < pThis->m_pStraFilters->GetSize() ); n++ )
//			{	
//				if( ::lstrcmpi( pszExt, pThis->m_pStraFilters->GetAt( n ) ) == 0 )
//					return ( S_OK );
//			}
//			
//			SAFE_COTASKMEMFREE( pszDisplayName );
//		}
//	}
//
//	return ( S_FALSE );
//
//}
//
//STDMETHODIMP CFolderFilter::XFolderFilterObj::GetEnumFlags( IN IShellFolder*	/*pIShellFolder*/, 
//															IN LPCITEMIDLIST	/*pidlFolder*/, 
//															IN HWND*			/*phWnd*/,
//															IN LPDWORD			pdwFlags )
//{
//	ASSERT( pdwFlags	  != NULL );	
//	//METHOD_PROLOGUE( CFolderFilter, FolderFilterObj )
//	
//	*pdwFlags = (DWORD)( SHCONTF_FOLDERS | SHCONTF_NONFOLDERS );
//	return ( S_OK );
//}

//////////////////////////////////////////////////////////////////////
#endif //USE_XP_FILTRATION
//////////////////////////////////////////////////////////////////////