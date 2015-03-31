//////////////////////////////////////////////////////////////////////
//
// ShellBrowser.cpp: implementation of the CShellBrowser class.
//

#include "stdafx.h"
#include "BrowseForFolder.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
//
// Construction/Destruction
//

CBrowseForFolder::CBrowseForFolder(const HWND hParent /*= NULL*/, const LPITEMIDLIST pidl /*= NULL*/, const int nTitleID /*= 0*/)
{
	m_hwnd = NULL;
	SetOwner(hParent);
	SetRoot(pidl);
	SetTitle(nTitleID);
	m_bi.lpfn = BrowseCallbackProc;
	m_bi.lParam = reinterpret_cast<long>(this);
	m_bi.pszDisplayName = m_szSelected;
}

CBrowseForFolder::CBrowseForFolder(const HWND hParent, const LPITEMIDLIST pidl, const CString& strTitle)
{
	m_hwnd = NULL;
	SetOwner(hParent);
	SetRoot(pidl);
	SetTitle(strTitle);
	m_bi.lpfn = BrowseCallbackProc;
	m_bi.lParam = reinterpret_cast<long>(this);
	m_bi.pszDisplayName = m_szSelected;
}

CBrowseForFolder::~CBrowseForFolder()
{

}

//////////////////////////////////////////////////////////////////////
//
// Implementation
//

void CBrowseForFolder::SetOwner(const HWND hwndOwner)
{
	if (m_hwnd != NULL)
		return;

	m_bi.hwndOwner = hwndOwner;
}

void CBrowseForFolder::SetRoot(const LPITEMIDLIST pidl)
{
	if (m_hwnd != NULL)
		return;

	m_bi.pidlRoot = pidl;
}

CString CBrowseForFolder::GetTitle() const
{
	return m_bi.lpszTitle;
}

void CBrowseForFolder::SetTitle(const CString& strTitle)
{
	if (m_hwnd != NULL)
		return;

	m_pchTitle=strTitle;
	m_bi.lpszTitle = m_pchTitle;
}

bool CBrowseForFolder::SetTitle(const int nTitle)
{
	if (nTitle <= 0)
		return false;

	CString strTitle;
	if(!strTitle.LoadString(static_cast<size_t>(nTitle)))
	{
		return false;
	}
	SetTitle(strTitle);
	return true;
}

void CBrowseForFolder::SetFlags(const UINT ulFlags)
{
	if (m_hwnd != NULL)
		return;

	m_bi.ulFlags = ulFlags;
}

CString CBrowseForFolder::GetSelectedFolder() const
{
	return m_szSelected;
}

bool CBrowseForFolder::SelectFolder()
{
	bool bRet = false;

	LPITEMIDLIST pidl;
	if ((pidl = ::SHBrowseForFolder(&m_bi)) != NULL)
	{
		m_strPath.Empty();
		if (SUCCEEDED(::SHGetPathFromIDList(pidl, m_szSelected)))
		{
			bRet = true;
			m_strPath = m_szSelected;
		}

		LPMALLOC pMalloc;
		//Retrieve a pointer to the shell's IMalloc interface
		if (SUCCEEDED(SHGetMalloc(&pMalloc)))
		{
			// free the PIDL that SHBrowseForFolder returned to us.
			pMalloc->Free(pidl);
			// release the shell's IMalloc interface
			(void)pMalloc->Release();
		}
	}
	m_hwnd = NULL;

	return bRet;
}

void CBrowseForFolder::OnInit() const
{

}

void CBrowseForFolder::OnSelChanged(const LPITEMIDLIST pidl) const
{
	(void)pidl;
}

void CBrowseForFolder::EnableOK(const bool bEnable) const
{
	if (m_hwnd == NULL)
		return;

	(void)SendMessage(m_hwnd, BFFM_ENABLEOK, NULL, static_cast<WPARAM>(bEnable));
}

void CBrowseForFolder::SetSelection(const LPITEMIDLIST pidl) const
{
	if (m_hwnd == NULL)
		return;

	(void)SendMessage(m_hwnd, BFFM_SETSELECTION, FALSE, reinterpret_cast<long>(pidl));
}

void CBrowseForFolder::SetSelection(const CString& strPath) const
{
	if (m_hwnd == NULL)
		return;

	(void)SendMessage(m_hwnd, BFFM_SETSELECTION, TRUE, reinterpret_cast<long>(LPCTSTR(strPath)));
}

void CBrowseForFolder::SetStatusText(const CString& strText) const
{
	if (m_hwnd == NULL)
		return;

	CString oPathString = FormatLongPath(strText);

	(void)SendMessage(m_hwnd, BFFM_SETSTATUSTEXT, NULL,
		reinterpret_cast<long>(LPCTSTR(oPathString/*strText*/)));
}

int __stdcall CBrowseForFolder::BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	CBrowseForFolder* pbff = reinterpret_cast<CBrowseForFolder*>(lpData);
	pbff->m_hwnd = hwnd;
	if (uMsg == BFFM_INITIALIZED)
		pbff->OnInit();
	else if (uMsg == BFFM_SELCHANGED)
		pbff->OnSelChanged(reinterpret_cast<LPITEMIDLIST>(lParam));
	
	return 0;
}

/*************************
3. Finally, the body of FormatLongPath looks like that:
**************************/

CString CBrowseForFolder::FormatLongPath(CString oLongPath) const
{
	//will be passed instead of original
	CString oModifString( oLongPath );

	//will be used to get measurements
	CWnd oWnd;

	if( !oModifString.IsEmpty() && IsWindow(m_hwnd) && oWnd.Attach(m_hwnd) )
	{
		//margins must be considered
		RECT Rect = { 0, 0, 7, 0 };	//my lucky guess the margin would be seven units. It used to be 7 in resource editor, so why not here?
		int nMargin = MapDialogRect( m_hwnd, &Rect ) ? Rect.right : 20;	//convert into pixels then
		
		//measure the width first
		CRect oClientRect;
		oWnd.GetClientRect( &oClientRect );
		oClientRect.NormalizeRect();
		int nMaxTextWidth = oClientRect.Width() - nMargin*2;
		
		CClientDC oClientDC(&oWnd);

		//trying to determine the system metrix to create apropriate fonts for measurement
		NONCLIENTMETRICS NonClientMetrics;
		
		NonClientMetrics.cbSize = sizeof(NONCLIENTMETRICS);
		
		BOOL bSystemMetrics = SystemParametersInfo( SPI_GETNONCLIENTMETRICS, 
			NonClientMetrics.cbSize, 
			&NonClientMetrics,
			0 );

		if( bSystemMetrics )
		{
			CFont oMessageFont;//lets create the fonts same as the selected Message Font on the Display/Appearance tab

			if( oMessageFont.CreateFontIndirect(&NonClientMetrics.lfMessageFont) )
			{
				oClientDC.SelectObject( &oMessageFont );
			}
		}
		else
		{
			oClientDC.SelectStockObject( SYSTEM_FONT );	//it MUST NOT happen, but in case...
		}
		
		//measure the actual text width
		int nTextWidth = oClientDC.GetTextExtent( oModifString ).cx;

		//to check whether it's correct uncoment below and change directory few times...
		//oClientDC.SelectStockObject( BLACK_PEN );
		//oClientDC.Rectangle( 0, 0, nMargin, nMargin*5 );
		//oClientDC.Rectangle( nMaxTextWidth+nMargin, 0, oClientRect.Width(), nMargin*5 );
		//oClientDC.Rectangle( nMargin, 0, nMaxTextWidth+nMargin, nMargin );
		//oClientDC.TextOut( nMargin, 0, oModifString );
		
		//after all this measurements time to do the real job
		if( nTextWidth > nMaxTextWidth )
		{
			int nRootDirIndex, nLastDirIndex;

			//this is the testing line:
			//oModifString = "\\\\computer_name\\dir1\\subdir1" + oModifString.Right(oModifString.GetLength() - 2 );

			nRootDirIndex = oModifString.Find( '\\' );
			nLastDirIndex = oModifString.ReverseFind( '\\' );

			if( nRootDirIndex == 0 )	//we have to deal with the network 'drive', which would look like that: \\computer_name\dir1\subdir1
			{
				nRootDirIndex = oModifString.Find( '\\', nRootDirIndex+1 );
				if( nRootDirIndex != -1 )
				{
					nRootDirIndex = oModifString.Find( '\\', nRootDirIndex+1 );
				}
			}

			if( nRootDirIndex != -1 && nLastDirIndex != -1 )
			{
				nRootDirIndex += 1;	//increase for the tactical reasons

				CString oDottedText( "..." );//this three dots will be used to indicate the cut part of the path

				CString oRootDirectory; 	//this can be cut as the last one
				CString oMidDirectoryPart;	//we will try to shorten this part first
				CString oLastDirectory; 	//and then, if still too long we'll cut this one
				
				oRootDirectory =	oModifString.Left( nRootDirIndex );
				oMidDirectoryPart =	oModifString.Mid( nRootDirIndex, nLastDirIndex - nRootDirIndex );
				oLastDirectory =	oModifString.Mid( nLastDirIndex );
				
				while( nTextWidth > nMaxTextWidth )
				{
					int nMidPartLenght = oMidDirectoryPart.GetLength();

					oModifString = oRootDirectory + oMidDirectoryPart + oDottedText + oLastDirectory;

					//measure the actual text width again
					nTextWidth = oClientDC.GetTextExtent( oModifString ).cx;
					
					if( nMidPartLenght > 0 )
					{
						//prepare for the next loop (if any)
						oMidDirectoryPart = oMidDirectoryPart.Left(oMidDirectoryPart.GetLength() - 1 );
					}
					else
					{
						int nLastDirectoryLenght = oLastDirectory.GetLength();
						
						if( nLastDirectoryLenght > 0 )
						{
							//prepare for the next loop (if any)
							oLastDirectory = oLastDirectory.Right(oLastDirectory.GetLength() - 1 );
						}
						else
						{
							//should not come here, what size of the fonts are you using?!
							//anyway, we will do different now, cutting from the end...
							int nRootDirectoryLenght = oRootDirectory.GetLength();

							if( nRootDirectoryLenght > 0 )
							{
								oRootDirectory = oRootDirectory.Left(oRootDirectory.GetLength() - 1 );
							}
							else
							{
								TRACE0( "Mayday, Mayday!!!\n" );
								oModifString = oLongPath;
								//something wrong, give me a...
								break;
							}
						}
					}
				}//end while
			}
		}

		oWnd.Detach();
	}

	return oModifString;
}
