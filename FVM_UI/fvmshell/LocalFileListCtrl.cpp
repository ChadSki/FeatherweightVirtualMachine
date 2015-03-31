// FileZilla - a Windows ftp client

// Copyright (C) 2002-2004 - Tim Kosse <tim.kosse@gmx.de>

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

// LocalFileListCtrl.cpp: Implementierungsdatei
//


#include "stdafx.h"

//#include "FileZilla.h"
#include "LocalFileListCtrl.h"
//#include "LocalView2.h"
#include "process.h"
#include "mainfrm.h"
#include "direct.h"
#include "EnterSomething.h"
//#include "TransferAsDlg.h"
#include "PathFunctions.h"
//#include "CommandQueue.h"
//#include "FtpListCtrl.h"
#include "DirTreeCtrl.h"
#include "resource.h"
#include "FvmFileView.h"
#include <shlobj.h>
#include <shellapi.h>
//#include "Options.h"


#include <list>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLocalFileListCtrl

CLocalFileListCtrl::CLocalFileListCtrl(CFvmFileView *pOwner)
{
	ASSERT(pOwner);

	m_pOwner = pOwner;

	m_Fullpath = ".."; //Just anything invalid
	m_nStyle = -1;
	for (int i = 0; i < 4; i++)
		m_Columns[i] = i;
	m_nHideColumns = 0;
	m_bUpdating = FALSE;

	m_nDragHilited = FALSE;
}

CLocalFileListCtrl::~CLocalFileListCtrl()
{
}


BEGIN_MESSAGE_MAP(CLocalFileListCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CLocalFileListCtrl)
	ON_WM_CREATE()
	ON_MESSAGE(WM_APP+1, OnUpdateContinue)
	ON_NOTIFY_REFLECT(LVN_BEGINDRAG, OnBegindrag)
	ON_NOTIFY_REFLECT(LVN_BEGINLABELEDIT, OnBeginlabeledit)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnclick)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_WM_DESTROY()
	ON_WM_DROPFILES()
	ON_NOTIFY_REFLECT(LVN_ENDLABELEDIT, OnEndlabeledit)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetdispinfo)
	ON_WM_KEYDOWN()
	ON_COMMAND(ID_FILEVIEW_NEWFOLDER, OnLocalcontextCreatedirectory)
	ON_COMMAND(ID_FILEVIEW_DELETE, OnLocalcontextDelete)
	ON_COMMAND(ID_FILEVIEW_RENAME, OnLocalcontextRename)
	ON_COMMAND(ID_FILEVIEW_PROPERTIES, OnLocalcontextProperties)
/*
	ON_COMMAND(ID_LOCALCONTEXT_ADDTOQUEUE, OnLocalcontextAddtoqueue)


	ON_COMMAND(ID_LOCALCONTEXT_OPEN, OnLocalcontextOpen)


	ON_COMMAND(ID_LOCALCONTEXT_UPLOAD, OnLocalcontextUpload)
	ON_COMMAND(ID_LOCALCONTEXT_UPLOADAS, OnLocalcontextUploadas)
	ON_COMMAND(ID_LOCALCONTEXT_VIEWEDIT, OnLocalcontextViewEdit)
*/
  ON_WM_VSCROLL()
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnItemchanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CLocalFileListCtrl 




LPWSTR WINAPI PathFindExtensionW( LPCWSTR lpszPath )
{
  LPCWSTR lastpoint = NULL;

//  TRACE("(%s)\n", debugstr_w(lpszPath));

  if (lpszPath)
  {
    while (*lpszPath)
    {
      if (*lpszPath == '\\' || *lpszPath==' ')
        lastpoint = NULL;
      else if (*lpszPath == '.')
        lastpoint = lpszPath;
      lpszPath = CharNextW(lpszPath);
    }
  }
  return (LPWSTR)(lastpoint ? lastpoint : lpszPath);
}

static LPWSTR PathGetExtensionW(LPCWSTR lpszPath)
{
//	TRACE("(%s)\n",debugstr_w(lpszPath));

	lpszPath = PathFindExtensionW(lpszPath);
	return (LPWSTR)(*lpszPath?(lpszPath+1):lpszPath);
}

static BOOL FvmPathIsExeW (LPCWSTR lpszPath)
{
	LPCWSTR lpszExtension = PathGetExtensionW(lpszPath);
        int i;
        static const WCHAR lpszExtensions[][4] =
            {{'e','x','e','\0'}, {'c','o','m','\0'}, {'p','i','f','\0'},
             {'c','m','d','\0'}, {'b','a','t','\0'}, {'s','c','f','\0'},
             {'s','c','r','\0'}, {'\0'} };

//	TRACE("path=%s\n",debugstr_w(lpszPath));

	for(i=0; lpszExtensions[i][0]; i++)
	  if (!_wcsicmp(lpszExtension,lpszExtensions[i])) return TRUE;

	return FALSE;
}


void CLocalFileListCtrl::openFiles(LPCTSTR filename)
{

//	unsigned int			officefile;
	unsigned int			rc = 0;
	int						nResult;
	TCHAR					usercommand[1024];

	//MessageBox(NULL,filename, "Opening", MB_OK);
	
	if ((nResult = (int)FindExecutable(filename, NULL, usercommand)) <= 32 ) {		
		MessageBox(L"FindExecutable Failed!");

		rc = 2;
		//goto Cleanup;
		return;
	}

	
	if(!FvmPathIsExeW(filename)){
		wcscat(usercommand, L" \"");
		wcscat(usercommand, filename);
		wcscat(usercommand, L" \"");
	}


    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    // Start the child process. 
    if( !CreateProcess( NULL, // No module name (use command line). 
        usercommand, // Command line. 
        NULL,             // Process handle not inheritable. 
        NULL,             // Thread handle not inheritable. 
        FALSE,            // Set handle inheritance to FALSE. 
        0,                // No creation flags. 
        NULL,             // Use parent's environment block. 
        NULL,             // Use parent's starting directory. 
        &si,              // Pointer to STARTUPINFO structure.
        &pi )             // Pointer to PROCESS_INFORMATION structure.
    ) 
    {
		MessageBox(L"Could not create process");        
    }

    // Wait until child process exits.
   // WaitForSingleObject( pi.hProcess, INFINITE );

    // Close process and thread handles. 
    //CloseHandle( pi.hProcess );
    //CloseHandle( pi.hThread );


	/*
	officefile = 1;
	if (ptr = StrStrI(usercommand_t,L"\\WINWORD.EXE"))
		wcscpy(ptr,L"\\Shortcut Bar\\Office\\Microsoft Word.lnk");
	else if (ptr = StrStrI(usercommand_t,L"\\EXCEL.EXE"))
		wcscpy(ptr,L"\\Shortcut Bar\\Office\\Microsoft Excel.lnk");
	else if (ptr = StrStrI(usercommand_t,L"\\POWERPNT.EXE"))
		wcscpy(ptr,L"\\Shortcut Bar\\Office\\Microsoft PowerPoint.lnk");
	else if (ptr = StrStrI(usercommand_t,L"\\MSACCESS.EXE"))
		wcscpy(ptr,L"\\Shortcut Bar\\Office\\Microsoft Access.lnk");
	else if (ptr = StrStrI(usercommand_t,L"\\OUTLOOK.EXE"))
		wcscpy(ptr,L"\\Shortcut Bar\\Office\\Microsoft Outlook.lnk");
	else officefile = 0;

	if(officefile){
		wcscpy(usercommand, L"\"");
		wcscat(usercommand, ShellAltPath);
		wcscat(usercommand, L"\" \"");
		wcscat(usercommand,usercommand_t);
		wcscat(usercommand,L"\"");
		wcscpy(usercommand_t,usercommand);
		wcscat(usercommand_t,L",");
		wcscat(usercommand_t,L"\"");
		wcscat(usercommand_t,userfile_t);
		wcscat(usercommand_t,L"\"");		
	}
	else {
	
		strcat(usercommand, " \"");
		strcat(usercommand, userfile_t);
		strcat(usercommand, " \"");	

	}
	*/



}


void CLocalFileListCtrl::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	POSITION selpos = GetFirstSelectedItemPosition();

	if (selpos)
	{
		int nItem = GetNextSelectedItem(selpos);
		int nIndex=m_IndexMapping[nItem];
		if (m_FileData[nIndex].bIsDir)
		{
		
			CString newpath;
			if (!nItem && m_Fullpath!="")
			{							
				newpath=m_Fullpath;
				newpath.TrimRight('\\');
				int pos=newpath.ReverseFind('\\');
				newpath=newpath.Left(pos+1);
			}
			else
				newpath=m_Fullpath+m_FileData[nIndex].Name + "\\";

			
			m_pOwner->SetLocalFolder(newpath);
			m_pOwner->SetLocalFolderOut(newpath);
			m_pOwner->m_pOwner->m_displayDelete = 0;

		}
		else
		{
			
			/*
				CString file = m_Fullpath+m_FileData[nIndex].Name;
				SHELLEXECUTEINFO sei = {0};
				sei.cbSize = sizeof(SHELLEXECUTEINFO);
				sei.lpFile = file;
				sei.nShow = SW_SHOWNORMAL;
				BOOL b = ShellExecuteEx(&sei);
		*/
			openFiles( m_Fullpath+m_FileData[nIndex].Name);
			
		}
	}
	
	  
	*pResult = 0;
}

void CLocalFileListCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
	
}


CString CLocalFileListCtrl::GetFolder() const
{
	return m_Fullpath;
}

int CLocalFileListCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CListCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	
	int widths[5]={150, 66, 100, 99};
/*
	if (COptions::GetOptionVal(OPTION_REMEMBERLOCALCOLUMNWIDTHS))
	{
		CString tmp = COptions::GetOption(OPTION_LOCALCOLUMNWIDTHS);
		int pos = -1;
		int i;
		for (i = 0; i<3; i++)
		{
			int oldpos = pos + 1;
			pos = tmp.Find(_T(" "), oldpos);
			if (pos == -1)
				break;
			tmp.SetAt(pos, 0);
			int size=_ttoi(tmp.Mid(oldpos));
			if (size>0)
				widths[i]=size;
		}
		if (i==3)
		{
			int size=_ttoi(tmp.Mid(pos+1));
			if (size>0)
				widths[i]=size;
		}
	}
	
	  */

	CString str;
	str.LoadString(IDS_HEADER_FILENAME);
	InsertColumn(0,str,LVCFMT_LEFT, widths[0]);
	str.LoadString(IDS_HEADER_FILESIZE);
	InsertColumn(1,str,LVCFMT_RIGHT, widths[1]);
	str.LoadString(IDS_HEADER_FILETYPE);
	InsertColumn(2,str,LVCFMT_LEFT, widths[2]);
	str.LoadString(IDS_HEADER_LASTMODIFIED);
	InsertColumn(3,str,LVCFMT_LEFT, widths[3]);
	
	m_SortImg.Create( 8, 8, ILC_MASK, 3, 3 );
	HICON Icon;
	Icon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_EMPTY));
	m_SortImg.Add(Icon);
	Icon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_UP));
	m_SortImg.Add(Icon);
	Icon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_DOWN));
	m_SortImg.Add(Icon);
	m_SortImg.SetBkColor(CLR_NONE);
	
	SetListStyle(0);

	/*
	int nSort = COptions::GetOptionVal(OPTION_LOCALCOLUMNSORT);
	m_sortdir = (nSort >> 4) % 3;
	if (!m_sortdir)
	*/
		m_sortdir = 1;
	/*
	m_sortcolumn = (nSort >> 1) & 0x07;
	if (m_sortcolumn > 3)
	*/
		m_sortcolumn = 0;

	DragAcceptFiles(TRUE);

	SetExtendedStyle(LVS_EX_INFOTIP);

	return 0;
}

/////////////////////////////////////////////////
BOOL CLocalFileListCtrl::GetSysImgList()
/////////////////////////////////////////////////
{
	CImageList sysImgList;
	SHFILEINFO shFinfo;
	
	HIMAGELIST hImageList = 0;
	CString errorMessage;
	TCHAR filename[MAX_PATH + 10];

	if (GetModuleFileName(0, filename, MAX_PATH + 10))
	{
		hImageList = (HIMAGELIST)SHGetFileInfo(filename,
							 0,
							 &shFinfo,
							 sizeof( shFinfo ),
							 SHGFI_SYSICONINDEX |
							 ((m_nStyle == LVS_ICON)?SHGFI_ICON:SHGFI_SMALLICON) );

		if (!hImageList)
		{
			int errorCode = GetLastError();
			TCHAR buffer[1000];
			memset(buffer, 0, 1000);
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, errorCode, 0, buffer, 999, 0);

			CString str;
			str.Format(_T("SHGetFileInfo failed with error %d: %s"), errorCode, buffer);
			errorMessage += str;
		}
	}
	else
	{
		int errorCode = GetLastError();
		TCHAR buffer[1000];
		memset(buffer, 0, 1000);
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, errorCode, 0, buffer, 999, 0);
		
		CString str;
		str.Format(_T("GetModuleFileName failed with error %d: %s"), errorCode, buffer);
		errorMessage += str;
	}
	if (!hImageList)
	{
		/*
		 * Fall back to C:\\
		 * Not bullerproof, but better than nothing
		 */		
		hImageList = (HIMAGELIST)SHGetFileInfo(_T("C:\\"),
							 0,
							 &shFinfo,
							 sizeof( shFinfo ),
							 SHGFI_SYSICONINDEX |
							 ((m_nStyle == LVS_ICON)?SHGFI_ICON:SHGFI_SMALLICON) );

		if (!hImageList)
		{
			int errorCode = GetLastError();
			TCHAR buffer[1000];
			memset(buffer, 0, 1000);
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, errorCode, 0, buffer, 999, 0);

			CString str;
			str.Format(_T("SHGetFileInfo failed with error %d: %s"), errorCode, buffer);
			if (errorMessage != _T(""))
				errorMessage += _T("\n");
			errorMessage += str;
		}
	}

	if (!hImageList)
	{
		AfxMessageBox(errorMessage);
		return FALSE;
	}

	sysImgList.Attach(hImageList);
	
	SetImageList( &sysImgList, (m_nStyle == LVS_ICON)?LVSIL_NORMAL:LVSIL_SMALL);
	sysImgList.Detach();

	return TRUE;
}

void CLocalFileListCtrl::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	SortList(pNMListView->iSubItem);
	
	*pResult = 0;
}

BOOL greater(const CString &str1, const CString &str2, BOOL isdir1, BOOL isdir2)
{
	if (isdir1 && !isdir2)
		return FALSE;
	if (!isdir1 && isdir2)
		return TRUE;

	if (str1.CollateNoCase(str2) > 0)
		return TRUE;
	return FALSE;
}

BOOL lesser(const CString &str1, const CString &str2, BOOL isdir1, BOOL isdir2)
{
	if (isdir1 && !isdir2)
		return TRUE;
	if (!isdir1 && isdir2)
		return FALSE;
	
	if (str2.CollateNoCase(str1) > 0)
		return TRUE;
	return FALSE;
}

BOOL greaterbytype(const CString &str1, const CString &str2, const BOOL isdir1, const BOOL isdir2, const CString &fn1, const CString &fn2)
{
	if (isdir1 && !isdir2)
		return FALSE;
	if (!isdir1 && isdir2)
		return TRUE;
	if (str1>str2)
		return TRUE;
	if (str1==str2)
	{
		if (fn1.CollateNoCase(fn2) > 0)
			return TRUE;
	}
	return FALSE;
}

BOOL lesserbytype(const CString &str1, const CString &str2, const BOOL isdir1, const BOOL isdir2, const CString &fn1, const CString &fn2)
{
	if (isdir1 && !isdir2)
		return TRUE;
	if (!isdir1 && isdir2)
		return FALSE;
	if (str1<str2)
		return TRUE;
	if (str1==str2)
	{
		if (fn1.CollateNoCase(fn2) < 0)
			return TRUE;
	}
	return FALSE;
}

BOOL greaterbytime(const CTime &time1, const CTime &time2, const BOOL &isdir1, const BOOL &isdir2, const CString &fn1, const CString &fn2)
{
	if (isdir1 && !isdir2)
		return FALSE;
	if (!isdir1 && isdir2)
		return TRUE;
	if (time1>time2)
		return TRUE;
	if (time1==time2)
	{
		if (fn1.CollateNoCase(fn2) > 0)
			return TRUE;
	}
	return FALSE;
}

BOOL lesserbytime(const CTime &time1, const CTime &time2, const BOOL &isdir1, const BOOL &isdir2, const CString &fn1, const CString &fn2)
{
	if (isdir1 && !isdir2)
		return TRUE;
	if (!isdir1 && isdir2)
		return FALSE;
	if (time1<time2)
		return TRUE;
	if (time1==time2)
	{
		if (fn1.CollateNoCase(fn2) < 0)
			return TRUE;
	}
	return FALSE;
}

BOOL greaterbysize(const __int64 &size1, const __int64 &size2, BOOL isdir1, BOOL isdir2, const CString &fn1, const CString &fn2)
{
	if (isdir1 && !isdir2)
		return FALSE;
	if (!isdir1 && isdir2)
		return TRUE;
	if (size1>size2)
		return TRUE;
	if (size1==size2)
	{
		if (fn1.CollateNoCase(fn2) > 0)
			return TRUE;
	}
	return FALSE;
}

BOOL lesserbysize(const __int64 &size1, const __int64 &size2, BOOL isdir1, BOOL isdir2, const CString &fn1, const CString &fn2)
{
	if (isdir1 && !isdir2)
		return TRUE;
	if (!isdir1 && isdir2)
		return FALSE;
	if (size1<size2)
		return TRUE;
	if (size1==size2)
	{
		if (fn1.CollateNoCase(fn2) < 0)
			return TRUE;
	}
	return FALSE;
}

void CLocalFileListCtrl::quicksortbyname(const BOOL &direction, int anf, int ende)
{
	int l=anf;
	int r=ende;
	CString tmp;
	CString ref=m_FileData[m_IndexMapping[(l+r)/2]].lName;
	BOOL bRefIsDir=m_FileData[m_IndexMapping[(l+r)/2]].bIsDir;
	do
    {
		if (direction)
		{
			while (lesser (m_FileData[m_IndexMapping[l]].lName, ref, m_FileData[m_IndexMapping[l]].bIsDir, bRefIsDir) && (l<ende)) l++;
			while (greater(m_FileData[m_IndexMapping[r]].lName, ref, m_FileData[m_IndexMapping[r]].bIsDir, bRefIsDir) && (r>anf)) r--;
		}
		else
		{
			while (greater(m_FileData[m_IndexMapping[l]].lName, ref, m_FileData[m_IndexMapping[l]].bIsDir, bRefIsDir) && (l<ende)) l++;
			while (lesser (m_FileData[m_IndexMapping[r]].lName, ref, m_FileData[m_IndexMapping[r]].bIsDir, bRefIsDir) && (r>anf)) r--;
		}
		if (l<=r)
		{
			int tmp=m_IndexMapping[l];
			m_IndexMapping[l]=m_IndexMapping[r];
			m_IndexMapping[r]=tmp;
			l++;
			r--;
		}
    } 
	while (l<=r);

  if (anf<r) quicksortbyname(direction, anf, r);
  if (l<ende) quicksortbyname(direction, l, ende);
}

void CLocalFileListCtrl::quicksortbytype(const std::vector<CString> &array, const BOOL &direction, int anf, int ende)
{
	int l=anf;
	int r=ende;
	CString tmp;
	CString ref=array[m_IndexMapping[(l+r)/2]];
	CString refname=m_FileData[m_IndexMapping[(l+r)/2]].lName;
	BOOL bRefIsDir=m_FileData[m_IndexMapping[(l+r)/2]].bIsDir;
	do
    {
		if (direction)
		{
			while (lesserbytype (array[m_IndexMapping[l]], ref, m_FileData[m_IndexMapping[l]].bIsDir, bRefIsDir, m_FileData[m_IndexMapping[l]].lName, refname) && (l<ende)) l++;
			while (greaterbytype(array[m_IndexMapping[r]], ref, m_FileData[m_IndexMapping[r]].bIsDir, bRefIsDir, m_FileData[m_IndexMapping[r]].lName, refname) && (r>anf)) r--;
		}
		else
		{
			while (greaterbytype(array[m_IndexMapping[l]], ref, m_FileData[m_IndexMapping[l]].bIsDir, bRefIsDir, m_FileData[m_IndexMapping[l]].lName, refname) && (l<ende)) l++;
			while (lesserbytype (array[m_IndexMapping[r]], ref, m_FileData[m_IndexMapping[r]].bIsDir, bRefIsDir, m_FileData[m_IndexMapping[r]].lName, refname) && (r>anf)) r--;
		}
		if (l<=r)
		{
			int tmp=m_IndexMapping[l];
			m_IndexMapping[l]=m_IndexMapping[r];
			m_IndexMapping[r]=tmp;
			l++;
			r--;
		}
    } 
	while (l<=r);

  if (anf<r) quicksortbytype(array, direction, anf, r);
  if (l<ende) quicksortbytype(array, direction, l, ende);
}

void CLocalFileListCtrl::quicksortbysize(const BOOL &direction, int anf, int ende)
{
	int l=anf;
	int r=ende;
	CString tmp;
	_int64 ref=m_FileData[m_IndexMapping[(l+r)/2]].nSize;
	CString refname=m_FileData[m_IndexMapping[(l+r)/2]].lName;
	BOOL bRefIsDir=m_FileData[m_IndexMapping[(l+r)/2]].bIsDir;
	do
    {
		if (direction)
		{
			while (lesserbysize (m_FileData[m_IndexMapping[l]].nSize, ref, m_FileData[m_IndexMapping[l]].bIsDir, bRefIsDir, m_FileData[m_IndexMapping[l]].lName, refname) && (l<ende)) l++;
			while (greaterbysize(m_FileData[m_IndexMapping[r]].nSize, ref, m_FileData[m_IndexMapping[r]].bIsDir, bRefIsDir, m_FileData[m_IndexMapping[r]].lName, refname) && (r>anf)) r--;
		}
		else
		{
			while (greaterbysize(m_FileData[m_IndexMapping[l]].nSize, ref, m_FileData[m_IndexMapping[l]].bIsDir, bRefIsDir, m_FileData[m_IndexMapping[l]].lName, refname) && (l<ende)) l++;
			while (lesserbysize (m_FileData[m_IndexMapping[r]].nSize, ref, m_FileData[m_IndexMapping[r]].bIsDir, bRefIsDir, m_FileData[m_IndexMapping[r]].lName, refname) && (r>anf)) r--;
		}
		if (l<=r)
		{
			int tmp=m_IndexMapping[l];
			m_IndexMapping[l]=m_IndexMapping[r];
			m_IndexMapping[r]=tmp;
			l++;
			r--;
		}
    } 
	while (l<=r);

  if (anf<r) quicksortbysize(direction, anf, r);
  if (l<ende) quicksortbysize(direction, l, ende);
}

void CLocalFileListCtrl::quicksortbytime(const BOOL &direction, int anf, int ende)
{
	int l=anf;
	int r=ende;
	CString tmp;
	CTime ref=m_FileData[m_IndexMapping[(l+r)/2]].Time;
	CString refname=m_FileData[m_IndexMapping[(l+r)/2]].lName;
	BOOL bRefIsDir=m_FileData[m_IndexMapping[(l+r)/2]].bIsDir;
	do
    {
		if (direction)
		{
			while (lesserbytime (m_FileData[m_IndexMapping[l]].Time, ref, m_FileData[m_IndexMapping[l]].bIsDir, bRefIsDir, m_FileData[m_IndexMapping[l]].lName, refname) && (l<ende)) l++;
			while (greaterbytime(m_FileData[m_IndexMapping[r]].Time, ref, m_FileData[m_IndexMapping[r]].bIsDir, bRefIsDir, m_FileData[m_IndexMapping[r]].lName, refname) && (r>anf)) r--;
		}
		else
		{
			while (greaterbytime(m_FileData[m_IndexMapping[l]].Time, ref, m_FileData[m_IndexMapping[l]].bIsDir, bRefIsDir, m_FileData[m_IndexMapping[l]].lName, refname) && (l<ende)) l++;
			while (lesserbytime (m_FileData[m_IndexMapping[r]].Time, ref, m_FileData[m_IndexMapping[r]].bIsDir, bRefIsDir, m_FileData[m_IndexMapping[r]].lName, refname) && (r>anf)) r--;
		}
		if (l<=r)
		{
			int tmp=m_IndexMapping[l];
			m_IndexMapping[l]=m_IndexMapping[r];
			m_IndexMapping[r]=tmp;
			l++;
			r--;
		}
    } 
	while (l<=r);

  if (anf<r) quicksortbytime(direction, anf, r);
  if (l<ende) quicksortbytime(direction, l, ende);
}

void CLocalFileListCtrl::SortList(int item, int direction)
{
	UINT nID[4]={IDS_HEADER_FILENAME,IDS_HEADER_FILESIZE,IDS_HEADER_FILETYPE,IDS_HEADER_LASTMODIFIED};

	if (!direction)
	{
		if (item!=m_sortcolumn)
			m_sortdir=1;
		else
		{
			m_sortdir=(++m_sortdir%3);
			if (!m_sortdir)
				m_sortdir++;
		}
			
	}
	else if (direction != -1)
		m_sortdir = direction;

	if (item == -1)
		item = m_sortcolumn;

	CHeaderCtrl *header=GetHeaderCtrl();
	if (header)
	{
		CString headertext;
		headertext.LoadString(nID[m_Columns[m_sortcolumn]]);
		HDITEM *hdi=new HDITEM;
		hdi->pszText=headertext.GetBuffer(0);
		hdi->cchTextMax=0;
		hdi->mask= HDI_TEXT | HDI_FORMAT;
		hdi->fmt=((m_Columns[m_sortcolumn]!=1)?HDF_LEFT:HDF_RIGHT) | HDF_STRING;
		hdi->mask= HDI_TEXT | HDI_IMAGE | HDI_FORMAT;
		hdi->iImage=0; // My ascending image list index
		header->SetItem( m_sortcolumn, hdi );
	
		headertext.ReleaseBuffer();
		headertext.LoadString(nID[m_Columns[item]]);
		hdi->pszText=headertext.GetBuffer(0);
		hdi->mask= HDI_TEXT | HDI_IMAGE | HDI_FORMAT;
		hdi->iImage= m_sortdir; // My ascending image list index
		hdi->fmt=((m_Columns[item]!=1)?HDF_LEFT:HDF_RIGHT) | HDF_IMAGE | HDF_STRING | HDF_BITMAP_ON_RIGHT;
		header->SetItem( item, hdi );
		delete hdi;
		headertext.ReleaseBuffer();
	}
	m_sortcolumn = item;
	if (GetItemCount()<=1)
		return;
	
	std::list<int> SelectedItemsList;
	int i;
	for (i=1; i<GetItemCount(); i++)
	{
		if (GetItemState( i, LVIS_SELECTED))
		{
			SelectedItemsList.push_back(m_IndexMapping[i]);
			SetItemState( i, 0, LVIS_SELECTED);
		}
	}

	if (!m_Columns[item])
	{ //Sort by filename
		BOOL dir=m_sortdir==1;
		quicksortbyname(dir, 1, GetItemCount()-1);
	
	}
	if (m_Columns[item]==2)
	{ //Sort by filetype
	  //Since this is a column that is filled while displaying,
	  //we have to load the filetypes for every file

		std::vector<CString> array;
		array.resize(GetItemCount()+1);
		array[0]=_T("");
		for (int i=1; i<GetItemCount(); i++)
		{
			array[i]=GetType(m_FileData[i].lName, m_FileData[i].bIsDir);
			array[i].MakeLower();
		}
		BOOL dir=m_sortdir==1;
		quicksortbytype(array, dir, 1, GetItemCount()-1);
	}
	else if (m_Columns[m_sortcolumn]==1)
	{
		BOOL dir=m_sortdir==1;
		quicksortbysize(dir, 1, GetItemCount()-1);
	}
	else if (m_Columns[m_sortcolumn]==3)
	{
		BOOL dir=m_sortdir==1;
		quicksortbytime(dir, 1, GetItemCount()-1);
	}

	for (i=1; i<GetItemCount(); i++)
	{
		int nIndex=m_IndexMapping[i];
		if (SelectedItemsList.empty())
			break;
		for (std::list<int>::iterator iter=SelectedItemsList.begin(); iter!=SelectedItemsList.end(); iter++)
			if (*iter==nIndex)
			{
				SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
				SelectedItemsList.erase(iter);
				break;
			}
	}

	RedrawItems(0,GetItemCount());
}

void CLocalFileListCtrl::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	m_transferuser=m_transferpass="";
	if (!GetItemCount())
		return;
	CMenu menu;
	menu.LoadMenu(IDR_FILEVIEW);

	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);
	CWnd* pWndPopupOwner = this;
	//while (pWndPopupOwner->GetStyle() & WS_CHILD)
	//	pWndPopupOwner = pWndPopupOwner->GetParent();

	POSITION selpos=GetFirstSelectedItemPosition();

	if (!selpos)
	{
		pPopup->EnableMenuItem(ID_FILEVIEW_RENAME, MF_GRAYED);	
		pPopup->EnableMenuItem(ID_FILEVIEW_DELETE, MF_GRAYED);	

		pPopup->EnableMenuItem(ID_FILEVIEW_PROPERTIES, MF_GRAYED);

		CString folder = GetFolder();
		if(folder == _T("")){			
			pPopup->EnableMenuItem(ID_FILEVIEW_NEWFOLDER, MF_GRAYED);
		}
		if (point.x==-1 || point.y==-1)
		{
			point.x=5;
			point.y=5;
			ClientToScreen(&point);
		}
	}


	else
	{
		int nItem = GetNextSelectedItem(selpos);
		if (point.x==-1 || point.y==-1)
		{
			CRect rect;
			GetItemRect(nItem,&rect,LVIR_LABEL);
			point.x=rect.left+5;
			point.y=rect.top+5;
			ClientToScreen(&point);
		}
		int index=m_IndexMapping[nItem];
		if (m_Fullpath=="")
		{
			pPopup->EnableMenuItem(ID_FILEVIEW_RENAME, MF_GRAYED);	
			pPopup->EnableMenuItem(ID_FILEVIEW_DELETE, MF_GRAYED);	
		//	pPopup->EnableMenuItem(ID_FILEVIEW_NEWFOLDER, MF_GRAYED);
			pPopup->EnableMenuItem(ID_FILEVIEW_PROPERTIES, MF_GRAYED);
		}
		else if (!nItem)
		{
			pPopup->EnableMenuItem(ID_FILEVIEW_RENAME, MF_GRAYED);	
			pPopup->EnableMenuItem(ID_FILEVIEW_DELETE, MF_GRAYED);	
		//	pPopup->EnableMenuItem(ID_FILEVIEW_NEWFOLDER, MF_GRAYED);
			pPopup->EnableMenuItem(ID_FILEVIEW_PROPERTIES, MF_GRAYED);
		}
		
	
		nItem = GetNextSelectedItem(selpos);
		while (nItem!=-1)
		{
			pPopup->EnableMenuItem(ID_FILEVIEW_PROPERTIES, MF_GRAYED);
			pPopup->EnableMenuItem(ID_FILEVIEW_RENAME, MF_GRAYED);
			int index=m_IndexMapping[nItem];
			nItem = GetNextSelectedItem(selpos);
		}
		
	}
	
		
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,
		pWndPopupOwner);
}

void CLocalFileListCtrl::OnLocalcontextOpen() 
{
	POSITION selpos=GetFirstSelectedItemPosition();
	if (!selpos)
		return;
	
	int nItem = GetNextSelectedItem(selpos);
	int index=m_IndexMapping[nItem];

	if (m_FileData[index].bIsDir)
	{
		CString newpath;
		CString text=GetItemText(nItem,0);
		if (text=="..")
		{
			newpath=m_Fullpath;
			newpath.TrimRight('\\');
			int pos=newpath.ReverseFind('\\');
			ASSERT(pos!=-1);
			newpath=newpath.Left(pos+1);
		}
		else
			newpath=m_Fullpath+m_FileData[index].Name+"\\";
		m_pOwner->SetLocalFolder(newpath);
		m_pOwner->SetLocalFolderOut(newpath);
	}
	else
	{
		CString file=m_Fullpath+m_FileData[index].Name;
		SHELLEXECUTEINFO sei = {0};
		sei.cbSize = sizeof(SHELLEXECUTEINFO);
		sei.lpFile = file;
		sei.nShow = SW_SHOWNORMAL;
		BOOL b = ShellExecuteEx(&sei);
	}
}

void CLocalFileListCtrl::OnLocalcontextUpload() 
{
	/*
	POSITION selpos=GetFirstSelectedItemPosition();
	CMainFrame *pMainFrame=DYNAMIC_DOWNCAST(CMainFrame,GetParentFrame());
	while (selpos)
	{
		int nItem = GetNextSelectedItem(selpos);
		if (!nItem || m_Fullpath=="")
			continue;
		int index=m_IndexMapping[nItem];
		if (m_FileData[index].bIsDir)
			UploadDir(m_Fullpath+m_FileData[index].Name+"\\*.*", m_FileData[index].Name+"\\", TRUE);
		else
			pMainFrame->AddQueueItem(FALSE, m_FileData[index].Name, "", m_Fullpath, CServerPath(), TRUE, m_transferuser, m_transferpass);
	}
	pMainFrame->TransferQueue(2);
	*/
}

void CLocalFileListCtrl::OnLocalcontextAddtoqueue() 
{
	/*
	POSITION selpos=GetFirstSelectedItemPosition();
	CMainFrame *pMainFrame=DYNAMIC_DOWNCAST(CMainFrame,GetParentFrame());
	while (selpos)
	{
		int nItem = GetNextSelectedItem(selpos);
		if (!nItem || m_Fullpath=="")
			continue;
		int index=m_IndexMapping[nItem];
		if (m_FileData[index].bIsDir)
			UploadDir(m_Fullpath+m_FileData[index].Name+"\\*.*", m_FileData[index].Name+"\\", FALSE);
		else
			pMainFrame->AddQueueItem(FALSE, GetItemText(nItem,0), "", m_Fullpath, CServerPath(), FALSE, m_transferuser, m_transferpass);
	}
	*/
}

void CLocalFileListCtrl::OnLocalcontextDelete() 
{
	
	POSITION selpos=GetFirstSelectedItemPosition();
	CMainFrame *pMainFrame=DYNAMIC_DOWNCAST(CMainFrame,GetParentFrame());
	
	if (!selpos)
		return;
	int len=1;
	while (selpos)
	{
		int nItem = GetNextSelectedItem(selpos);
		if (!nItem || m_Fullpath=="")
			continue;
		int index=m_IndexMapping[nItem];
		len+=m_Fullpath.GetLength()+m_FileData[index].Name.GetLength()+1;
	}

	LPTSTR buffer = new TCHAR[len];
	memset(buffer, 0, len*sizeof(TCHAR));
	selpos=GetFirstSelectedItemPosition();
	ASSERT(selpos);
	int bufpos=0;
	std::list<CString> FileList;
	BOOL bDirDeleted=FALSE;
	while (selpos)
	{
		int nItem = GetNextSelectedItem(selpos);
		if (!nItem || m_Fullpath=="")
			continue;
		int index=m_IndexMapping[nItem];
		FileList.push_back(m_FileData[index].lName);
		if (m_FileData[index].bIsDir)
			bDirDeleted=TRUE;
		_tcscpy(buffer+bufpos, m_Fullpath+m_FileData[index].Name);
		bufpos+=m_Fullpath.GetLength()+m_FileData[index].Name.GetLength()+1;
	}
	/*

	SHFILEOPSTRUCT op;
	memset(&op,0,sizeof(op));
	op.hwnd=m_hWnd;
	op.wFunc=FO_DELETE;
	op.fFlags=(GetKeyState(VK_SHIFT)&128)?0:FOF_ALLOWUNDO;
	op.pFrom=buffer;
	*/
	CString delm = L"Do you want to delete the file";
	
	int cr = MessageBox(delm+buffer+L"?", L"Confirm Delete",MB_OKCANCEL);
	if(cr == 1){
		DeleteFile(buffer);
	}

	//SHFileOperation(&op);
	delete [] buffer;
	if (bDirDeleted)
		pMainFrame->RefreshViews(1);
	else
	{
		m_bUpdating = TRUE;
		for (std::list<CString>::const_iterator iter=FileList.begin(); iter!=FileList.end(); iter++)
			RefreshFile(m_Fullpath+*iter);
		m_bUpdating = FALSE;
		UpdateStatusBar();
	}
	
}

void CLocalFileListCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar==46)
		OnLocalcontextDelete();
	else if (nChar==VK_F2)
	{
		// To Use F2 as hot Key to get EditCtrl on the ListView it must have 
		// the Style LVS_EDITLABELS
		ASSERT( GetStyle() & LVS_EDITLABELS );
		// don't do an Edit Label when the multiple Items are selected
		if( GetSelectedCount( ) == 1 )
		{
			UINT nListSelectedItem = GetSelectedItem();
			VERIFY( EditLabel( nListSelectedItem ) != NULL );
		}
		else
			CListCtrl::OnKeyDown( nChar, nRepCnt, nFlags );
	}
	else if (nChar==VK_BACK)
	{
		if (GetItemCount() && m_IndexMapping.size())
		{
			int nIndex=m_IndexMapping[0];
			if (m_FileData[nIndex].bIsDir)
			{
				if (m_Fullpath!="")
				{
					CString newpath;
					newpath=m_Fullpath;
					newpath.TrimRight('\\');
					int pos=newpath.ReverseFind('\\');
					newpath=newpath.Left(pos+1);
					m_pOwner->SetLocalFolder(newpath);
					m_pOwner->SetLocalFolderOut(newpath);
				}
			}
		}
	}
	CListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CLocalFileListCtrl::OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	/*
	//Start of a Drag&Drop operation
	*pResult = 0;

	if (m_Fullpath == _T("") || m_Fullpath == _T("\\"))
		return;

	POSITION selpos=GetFirstSelectedItemPosition();
	while (selpos)
	{
		int nItem = GetNextSelectedItem(selpos);
		if (!nItem)
			return;
	}
	EnsureVisible(((LPNMLISTVIEW)pNMHDR)->iItem, FALSE);
	
	//Let the main window handle the rest
	CMainFrame *pMainFrame=DYNAMIC_DOWNCAST(CMainFrame,GetParentFrame());
	pMainFrame->OnBegindrag(this, ((LPNMLISTVIEW)pNMHDR)->ptAction);
	*/
}

void CLocalFileListCtrl::OnDropFiles(HDROP hDropInfo) 
{
	/*
	if (!GetItemCount())
	{
		DragFinish(hDropInfo);
		return;
	}
	int dropcount=DragQueryFile(hDropInfo,0xFFFFFFFF,0,0);
	LPTSTR fullnames=0;
	int fulllen=0;
	for (int i=0;i<dropcount;i++)
	{
		int len=DragQueryFile(hDropInfo,i,0,0)+1;
		fullnames=(LPTSTR)realloc(fullnames,(fulllen+len+1)*sizeof(TCHAR));
		DragQueryFile(hDropInfo,i,&fullnames[fulllen],len);
		fulllen+=len;
	}
	fullnames[fulllen]=0;
	CString path=m_Fullpath;

	SHFILEOPSTRUCT opstruct;
	opstruct.hwnd=m_hWnd;
	opstruct.wFunc=FO_COPY;
	opstruct.pFrom=fullnames;
	opstruct.pTo=path;
	opstruct.fFlags=FOF_ALLOWUNDO;
	SHFileOperation(&opstruct);
	DragFinish(hDropInfo);
	free(fullnames);
	CMainFrame *pMainFrame=DYNAMIC_DOWNCAST(CMainFrame,GetParentFrame());
	pMainFrame->RefreshViews(1);
	*/
}



// this Function Returns the first Selected Item in the List
UINT CLocalFileListCtrl::GetSelectedItem()
{
	// this Function Valid Only when a Single Item is Selected
	ASSERT( GetSelectedCount( ) == 1 );
	UINT nNoOfItems = GetItemCount( );
	UINT nListItem;
	for (nListItem = 0; nListItem < nNoOfItems; nListItem++)
		if (GetItemState( nListItem, LVIS_SELECTED )  )
			break;
	ASSERT(nListItem < nNoOfItems);
	return nListItem;
}

#define VK_A		65
BOOL CLocalFileListCtrl::PreTranslateMessage(MSG* pMsg) 
{
	/*
	// If edit control is visible in tree view control, sending a
	// WM_KEYDOWN message to the edit control will dismiss the edit
	// control.  When ENTER key was sent to the edit control, the parent
	// window of the tree view control is responsible for updating the
	// item's label in TVN_ENDLABELEDIT notification code.
	if ( pMsg->message == WM_KEYDOWN )
	{
		CEdit* edit = GetEditControl();
		if (edit)
		{
			if( GetKeyState( VK_CONTROL )&128 && pMsg->wParam == VK_A )
			{
				edit->SetSel(0, -1);
				return TRUE;
			}
			else if( pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_CONTROL || pMsg->wParam == VK_INSERT || pMsg->wParam == VK_SHIFT )
			{
				edit->SendMessage(WM_KEYDOWN, pMsg->wParam, pMsg->lParam);
				return TRUE;
			}
		}
		else
		{
			if( GetKeyState( VK_CONTROL )&128 && pMsg->wParam == VK_A )
			{
				m_bUpdating = TRUE;
				if (GetItemCount())
					SetItemState(0, GetItemCount()==1 || m_Fullpath==""?LVIS_SELECTED:0,LVIS_SELECTED);
				for (int i=1;i<GetItemCount();i++)
					SetItemState(i, LVIS_SELECTED,LVIS_SELECTED);
			
				m_bUpdating = FALSE;
				UpdateStatusBar();
				return TRUE;
			}
			//Handle the enter key
			else if (pMsg->wParam==VK_RETURN)
			{
				CMainFrame *pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetParentFrame());
				POSITION selpos=GetFirstSelectedItemPosition();
				BOOL bOpen=TRUE;
				int openindex=-1;
				int openitem=-1;
				if (selpos)
				{
					int nItem = openitem = GetNextSelectedItem(selpos);
					int index=openindex=m_IndexMapping[nItem];
					if (m_FileData[index].bIsDir)
					{
					
					}
					else
					{
						bOpen=FALSE;
						pMainFrame->AddQueueItem(FALSE,GetItemText(nItem,0),"",m_Fullpath,CServerPath(),TRUE);
						openindex=-1;
						openitem=-1;
					}
				}
				else
					return TRUE;
				while (selpos)
				{
					if (!openindex)
						return TRUE;
					bOpen=FALSE;
					int nItem = GetNextSelectedItem(selpos);
					if(!nItem || m_Fullpath=="")
						return TRUE;
					int index=m_IndexMapping[nItem];
					if (m_FileData[index].bIsDir)
						UploadDir(m_Fullpath+m_FileData[index].Name+"\\*.*", m_FileData[index].Name+"\\", TRUE);
					else
						pMainFrame->AddQueueItem(FALSE, m_FileData[index].Name, "", m_Fullpath, CServerPath(), TRUE, m_transferuser, m_transferpass);
				}
				if (bOpen)
				{
					CString newpath;
					if (!openitem && m_Fullpath!="")
					{
						m_Fullpath.TrimRight('\\');
						int pos=m_Fullpath.ReverseFind('\\');
						if (pos!=-1)
							m_Fullpath=m_Fullpath.Left(pos+1);
						else
							m_Fullpath="";
						newpath=m_Fullpath;
					}
					else
						newpath=m_Fullpath+m_FileData[openindex].Name+"\\";
					m_pOwner->SetLocalFolder(newpath);
					m_pOwner->SetLocalFolderOut(newpath);
				}
				else
				{
					ASSERT(m_Fullpath!="");
					if (openindex>=0)
						UploadDir(m_Fullpath+m_FileData[openindex].Name+"\\*.*", m_FileData[openindex].Name+"\\", TRUE);
					pMainFrame->TransferQueue(2);					
				}

				return TRUE;
			}
		}
	}
	else if ( pMsg->message == WM_CHAR )
	{
		CEdit* edit = GetEditControl();
		if (edit)
		{
			if (pMsg->wParam=='/' ||
				pMsg->wParam=='\\' ||
				pMsg->wParam==':' ||
				pMsg->wParam=='*' ||
				pMsg->wParam=='?' ||
				pMsg->wParam=='"' ||
				pMsg->wParam=='<' ||
				pMsg->wParam=='>' ||
				pMsg->wParam=='|')
			{
				MessageBeep(0xFFFFFFFF);
				return TRUE;
			}
		}
	}
	*/
	return CListCtrl::PreTranslateMessage(pMsg);
}

void CLocalFileListCtrl::OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	// This is the Limit the size of the Tag Name to 255
	if (!pDispInfo->item.iItem || m_Fullpath=="")
	{
		*pResult=TRUE;
		return;
	}
	
	*pResult = 0;
	CEdit *pEdit=GetEditControl();
	if (pEdit)
		pEdit->LimitText( 255 );	
}

void CLocalFileListCtrl::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	if (pDispInfo->item.pszText)
	{
		CString newname=pDispInfo->item.pszText;
		newname=newname.Left(255);
		if ((newname.Find('/')!=-1)||(newname.Find('\\')!=-1)||
			(newname.Find(':')!=-1)||(newname.Find('*')!=-1)||
			(newname.Find('?')!=-1)||(newname.Find('"')!=-1)||
			(newname.Find('<')!=-1)||(newname.Find('>')!=-1)||
			(newname.Find('|')!=-1))
		{
			AfxMessageBox(IDS_ERRORMSG_FILENAMEINVALID,MB_ICONEXCLAMATION);
			*pResult=FALSE;
			return;
		}
		SHFILEOPSTRUCT op;
		memset(&op,0,sizeof(op));
		CString from=m_Fullpath+m_FileData[m_IndexMapping[pDispInfo->item.iItem]].Name+" ";
		from.SetAt(from.GetLength()-1,'\0');
		op.pFrom=from;
		CString to=m_Fullpath+pDispInfo->item.pszText+" ";
		to.SetAt(to.GetLength()-1,'\0');
		op.pTo=to;
		op.hwnd=AfxGetMainWnd()->m_hWnd;
		op.wFunc=FO_RENAME;
		op.fFlags=FOF_ALLOWUNDO;
		if (!SHFileOperation(&op))
		{
			*pResult = TRUE;
			m_FileData[m_IndexMapping[pDispInfo->item.iItem]].Name=pDispInfo->item.pszText;
			m_FileData[m_IndexMapping[pDispInfo->item.iItem]].lName=pDispInfo->item.pszText;
			m_FileData[m_IndexMapping[pDispInfo->item.iItem]].lName.MakeLower();
				m_pOwner->m_pOwner->RefreshViews(1);
			return;
		}
	
	}
	
	*pResult = FALSE;
}

void CLocalFileListCtrl::OnLocalcontextRename() 
{
	ASSERT(GetSelectedCount()==1);
	POSITION selpos=GetFirstSelectedItemPosition();
	int index=GetNextSelectedItem(selpos);
	SetFocus();

	EditLabel(index);
	
}

void CLocalFileListCtrl::OnLocalcontextCreatedirectory() 
{
	if (m_Fullpath=="")
		return;
	CEnterSomething dlg(IDS_INPUTDIALOGTITLE_CREATEDIR,IDS_INPUTDIALOGTEXT_CREATEDIR);
	if (dlg.DoModal()==IDOK)
	{
		int ret = CreateDirectory(m_Fullpath+dlg.m_String, 0);
		if(ret != 0){
			CMainFrame *pMainFrame=DYNAMIC_DOWNCAST(CMainFrame,GetParentFrame());
			pMainFrame->RefreshViews(1);
		}
		else{
			MessageBox(L"Could not create folder.");

		}
	}		
}

/*
void CLocalFileListCtrl::ReloadHeader()
{
	
	ReloadHeaderItem(0,IDS_HEADER_FILENAME);
	int i=1;
	if (!(m_nHideColumns&1))
	{
		ReloadHeaderItem(i,IDS_HEADER_FILESIZE);
		i++;
	}
	if (!(m_nHideColumns&2))
	{
		ReloadHeaderItem(i,IDS_HEADER_FILETYPE);
		i++;
	}
	if (!(m_nHideColumns&4))
		ReloadHeaderItem(i,IDS_HEADER_LASTMODIFIED);
		
}

void CLocalFileListCtrl::ReloadHeaderItem(int nIndex, UINT nID)
{
	
	CHeaderCtrl *header=GetHeaderCtrl();
	TCHAR text[100];
	HDITEM item;
	memset(&item,0,sizeof(HDITEM));
	item.cchTextMax=100;
	item.mask=HDI_TEXT;
	item.pszText=text;
	header->GetItem(nIndex,&item);
	CString str;
	str.LoadString(nID);
	_tcscpy(text,str);
	header->SetItem(nIndex,&item);
	
}

  */
void CLocalFileListCtrl::SetFolder(CString folder)
{
	if (m_bUpdating)
	{
		m_NewDir = folder;
		return;
	}
	else
		m_NewDir="";

	BOOL bDidHaveFocus = GetFocus() == this;
	
	folder.TrimRight( _T("\\") ); 
	folder+=_T("\\");
	m_FileData.clear();
	m_IndexMapping.clear();
	m_Fullpath=folder;
	DeleteAllItems();
	if (m_Fullpath==_T("\\"))
	{
		m_Fullpath="";
		DisplayDrives();
		return;
	}
	m_bUpdating=TRUE;

	SetRedraw(FALSE);
	EnableWindow(FALSE);
	
	
	CString   strPathFiles = m_Fullpath;
	
	if ( strPathFiles.Right(1) != "\\" )
		strPathFiles += "\\";
	strPathFiles += "*.*";

	WIN32_FIND_DATA find;
	HANDLE hFind = FindFirstFile( strPathFiles, &find);

	CString folder2=m_Fullpath;
	folder2.TrimRight('\\');
	
	int pos=folder2.ReverseFind('\\');
	folder2=folder2.Left(pos+1);

	t_FileData FileData;

	
	FileData.Name="..";
	FileData.lName="..";
	FileData.bIsDir=TRUE;
	m_IndexMapping.push_back(0);
	FileData.nSize=-1;
	FileData.hasTime = false;
	m_FileData.push_back(FileData);
	
	int nItemCount= 1;
			
	while ( hFind!=INVALID_HANDLE_VALUE)
	{
		if (!(find.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY && (!_tcscmp(find.cFileName, _T("..")) || !_tcscmp(find.cFileName, _T(".")))))
		{
			t_FileData FileData;
			FileData.Name=find.cFileName;
			FileData.lName=find.cFileName;
			FileData.lName.MakeLower();
			
			TRY
			{
				FileData.Time = find.ftLastWriteTime;
				FileData.hasTime = true;
			}
			CATCH_ALL(e)
			{
				FileData.hasTime = false;
			}
			END_CATCH_ALL;

			if ( find.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			{
				FileData.bIsDir=TRUE;
				FileData.nSize=-1;
			}
			else
			{
				FileData.bIsDir=FALSE;
				FileData.nSize=(_int64)find.nFileSizeLow + ((_int64)find.nFileSizeHigh<<32);
			}
			
			m_FileData.push_back(FileData);
			m_IndexMapping.push_back(nItemCount);
			
			nItemCount++;
		}
//#define SPEEDTEST 20000
#ifndef SPEEDTEST
		if (nItemCount==100)
		{
			VERIFY(PostMessage(WM_APP + 1, bDidHaveFocus, (LPARAM)hFind));
			return;
		}
#endif //SPEEDTEST
		if (!FindNextFile(hFind, &find))
		{
			FindClose(hFind);
			hFind=0;
#ifdef SPEEDTEST
			//Speed test code with SPEEDTEST files
			if (nItemCount>1 && nItemCount<SPEEDTEST) 
				hFind = FindFirstFile( strPathFiles, &find);
			else
				break;
#else
			break;
#endif //SPEEDTEST
		}
	}

	m_bUpdating=FALSE;

	SetItemCount(nItemCount);
	
	SortList(m_sortcolumn,m_sortdir);

	EnableWindow(TRUE);
	
	SetRedraw( TRUE );

	if (bDidHaveFocus)
		SetFocus();

	UpdateStatusBar();
}

void CLocalFileListCtrl::OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem= &(pDispInfo)->item;

	
	
	if (static_cast<int>(m_IndexMapping.size())<=pItem->iItem)
		return;
	if (static_cast<int>(m_FileData.size())<=m_IndexMapping[pItem->iItem])
		return;
	
	if (pItem->mask & LVIF_IMAGE && !pItem->iSubItem)
	{
		if (m_FileData[m_IndexMapping[pItem->iItem]].iIcon!=-1)
			pItem->iImage=m_FileData[m_IndexMapping[pItem->iItem]].iIcon;
		else
		{
			CString path;
			if (m_FileData[m_IndexMapping[pItem->iItem]].lName=="..")
				path="alkjhgfdfghjjhgfdghuztxvbhzt";
			else
			{
				path=m_Fullpath+m_FileData[m_IndexMapping[pItem->iItem]].Name;
					//MessageBox(path);
				if (m_Fullpath=="")
					path+="\\";			
			}
			

			int iIcon=-1;
			SHFILEINFO shFinfo;
			memset(&shFinfo,0,sizeof(SHFILEINFO));
			
			if (SHGetFileInfo( path,
				m_FileData[m_IndexMapping[pItem->iItem]].bIsDir?FILE_ATTRIBUTE_DIRECTORY:FILE_ATTRIBUTE_NORMAL,
					&shFinfo,
					sizeof( SHFILEINFO ),
					SHGFI_ICON | ((m_FileData[m_IndexMapping[pItem->iItem]].lName=="..")?SHGFI_USEFILEATTRIBUTES:0) ) )
			{
				iIcon = shFinfo.iIcon;
				// we only need the index from the system image ctrl
				DestroyIcon( shFinfo.hIcon );
				m_FileData[m_IndexMapping[pItem->iItem]].iIcon=iIcon;
				//WCHAR m[256];
				//wsprintf(m, L"io: %d %s\n", iIcon, path);
				//MessageBox(m);
			}
			else{
				;//MessageBox(L"error "+ path);
			}
		
			pItem->iImage=iIcon;
		}
	}
	if (pItem->mask & LVIF_TEXT)
	{
		if (!pItem->pszText)
			return;
		if (!m_Columns[pItem->iSubItem])
		{
			lstrcpy(pItem->pszText, m_FileData[m_IndexMapping[pItem->iItem]].Name);
		}
		else if (m_Columns[pItem->iSubItem]==2 && (pItem->iItem||(m_Fullpath=="")))
		{
			CString type=GetType(m_FileData[m_IndexMapping[pItem->iItem]].lName, m_FileData[m_IndexMapping[pItem->iItem]].bIsDir);
			lstrcpy(pItem->pszText,type);
		}
		else if (m_Columns[pItem->iSubItem]==1)
		{
			__int64 size=m_FileData[m_IndexMapping[pItem->iItem]].nSize;
			if (size!=-1)
			{

				int nFormat= 0;//COptions::GetOptionVal(OPTION_LOCALFILESIZEFORMAT);
				if (!nFormat)
					if (size<1024)
						nFormat=1;
					else if (size<(1024*1024))
						nFormat=2;
					else
						nFormat=3;
				
				CString tmp, sizestr;
				switch (nFormat)
				{
				case 1:
					sizestr.Format(_T("%I64d"), size);
					break;
				case 2:
					tmp.LoadString(IDS_SIZE_KBS);
					sizestr.Format(_T("%I64d %s"), size/1024, tmp);
					break;
				case 3:
					tmp.LoadString(IDS_SIZE_MBS);
					sizestr.Format(_T("%I64d %s"), size/1024/1024, tmp);
					break;
				default:
					ASSERT(FALSE);
				}
				
				lstrcpy(pItem->pszText,sizestr);
				
			}
		}
		else if (m_Columns[pItem->iSubItem]==3 && m_Fullpath!=_T("") && pItem->iItem && m_FileData[m_IndexMapping[pItem->iItem]].hasTime)
		{
			CTime time=m_FileData[m_IndexMapping[pItem->iItem]].Time;
			SYSTEMTIME timeDest;
			time.GetAsSystemTime( timeDest ) ;
		
			TCHAR text[512];
			if (!GetDateFormat(
				LOCALE_USER_DEFAULT,               // locale for which date is to be formatted
				DATE_SHORTDATE,             // flags specifying function options
				&timeDest,  // date to be formatted
				0,          // date format string
				text,          // buffer for storing formatted string
				512                // size of buffer
				))
				return;
			CString text2=text;
			
			if (!GetTimeFormat(
				LOCALE_USER_DEFAULT,               // locale for which date is to be formatted
				TIME_NOSECONDS|TIME_FORCE24HOURFORMAT,             // flags specifying function options
				&timeDest,  // date to be formatted
				0,          // date format string
				text,          // buffer for storing formatted string
				512                // size of buffer
				))
				return;
			text2+=" ";
			text2+=text;
			_tcscpy(pItem->pszText, text2);
		}
	}
	*pResult = 0;
}

void CLocalFileListCtrl::DisplayDrives()
{
	m_Fullpath=_T("");
	TCHAR  szDrives[128];
	LPTSTR pDrive;

	GetLogicalDriveStrings( sizeof(szDrives), szDrives );
	
	pDrive = szDrives;
	int count=0;
	while( *pDrive )
	{
		CString path=pDrive;
		t_FileData FileData;
		path.TrimRight('\\');
		FileData.Name=path;
		path.MakeLower();
		FileData.lName=path;
		FileData.nSize=-1;
		FileData.bIsDir=TRUE;
		m_FileData.push_back(FileData);
		m_IndexMapping.push_back(count);
		pDrive += _tcslen(pDrive) + 1;
		count++;
	}
	SetItemCount(count);
	SortList(m_sortcolumn,m_sortdir);

	UpdateStatusBar();
}

BOOL CLocalFileListCtrl::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult) 
{
	if (message==WM_NOTIFY)
	{
		LPNMHDR  lpnmh = (LPNMHDR) lParam;
		
		if (lpnmh->code==LVN_ODFINDITEM)
		{
			NMLVFINDITEM* pFindItem = (NMLVFINDITEM*)lParam;
			LVFINDINFO pFindInfo=pFindItem->lvfi;
		
			CString part=pFindInfo.psz;
			part.MakeLower();
			int start=pFindItem->iStart;
			
			for (int i=start;i<(GetItemCount()+start);i++)
			{
				CString fn=m_FileData[m_IndexMapping[i%GetItemCount()]].Name.Left(part.GetLength());
				fn.MakeLower();
				if (fn==part)
				{
					*pLResult=i%GetItemCount();
					return TRUE;
				}
			}
			*pLResult=-1;
			return TRUE;
		}
	}
	return CListCtrl::OnChildNotify(message, wParam, lParam, pLResult);
}

void CLocalFileListCtrl::OnLocalcontextUploadas() 
{
	/*
	CTransferAsDlg dlg;
	if (dlg.DoModal()==IDOK)
	{
		m_transferuser=dlg.m_User;
		m_transferpass=dlg.m_Pass;
		if (dlg.m_bTransferNow)
			OnLocalcontextUpload();
		else
			OnLocalcontextAddtoqueue();
	}
	*/
}

void CLocalFileListCtrl::SetListStyle(int nStyle)
{
	if (!nStyle)
		nStyle = LVS_REPORT;
	else if (nStyle == 1)
		nStyle = LVS_LIST;
	else if (nStyle == 2)
		nStyle = LVS_ICON;
	else if (nStyle == 3)
		nStyle = LVS_SMALLICON;
	
	if (m_nStyle == nStyle)
		return;

	m_nStyle = nStyle;

	int remove = ~m_nStyle & (LVS_REPORT | LVS_ICON | LVS_SMALLICON | LVS_LIST);
	ModifyStyle(remove, m_nStyle, SWP_NOZORDER);
	
	GetSysImgList();
	CHeaderCtrl *header=GetHeaderCtrl( );
	if (header)
		header->SetImageList(&m_SortImg);
	Arrange(LVA_DEFAULT);
	if (m_nStyle!=LVS_REPORT)
		SortList(0,1);
	SetItemCount(GetItemCount());
}

void CLocalFileListCtrl::UpdateColumns(int nHideColumns)
{
	CString str;
	if (nHideColumns&1 && !(m_nHideColumns&1))
	{
		m_Columns[1]=m_Columns[2];
		m_Columns[2]=m_Columns[3];
		DeleteColumn(1);
		if (m_sortcolumn==1)
			SortList(0,1);
		if (m_sortcolumn>1)
			m_sortcolumn--;
	}
	if (m_nHideColumns&1 && !(nHideColumns&1))
	{
		m_Columns[3]=m_Columns[2];
		m_Columns[2]=m_Columns[1];
		m_Columns[1]=1;
		str.LoadString(IDS_HEADER_FILESIZE);
		InsertColumn(1,str,LVCFMT_RIGHT,66);
		if (m_sortcolumn>=1)
			m_sortcolumn++;
	}
	if (nHideColumns&2 && !(m_nHideColumns&2))
	{
		for (int i=1;i<3;i++)
		{
			if (m_Columns[i]==2)
			{
				int j=i;
				while (j<3)
				{
					m_Columns[j]=m_Columns[j+1];
					j++;
				}
				DeleteColumn(i);		
				if (m_sortcolumn==i)
					SortList(0,1);
				if (m_sortcolumn>i)
					m_sortcolumn--;
				break;
			}
		}
	}
	if (m_nHideColumns&2 && !(nHideColumns&2))
	{
		for (int i=1;i<3;i++)
		{
			if (m_Columns[i]==3)
			{
				int j=3;
				while (j!=i)
				{
					m_Columns[j]=m_Columns[j-1];
					j--;
				}
				m_Columns[i]=2;
				str.LoadString(IDS_HEADER_FILETYPE);
				InsertColumn(i,str,LVCFMT_LEFT,100);
				if (m_sortcolumn>=i)
					m_sortcolumn++;
				break;
			}
		}
	}
	if (nHideColumns&4 && !(m_nHideColumns&4))
	{
		for (int i=1;i<4;i++)
		{
			if (m_Columns[i]==3)
			{
				m_Columns[i]=3;
				DeleteColumn(i);		
				if (m_sortcolumn==i)
					SortList(0,1);
				if (m_sortcolumn>i)
					m_sortcolumn--;
				break;
			}
		}
	}
	if (m_nHideColumns&4 && !(nHideColumns&4))
	{
		for (int i=1;i<4;i++)
		{
			if (m_Columns[i]==3)
			{
				m_Columns[i]=3;
				str.LoadString(IDS_HEADER_LASTMODIFIED);
				InsertColumn(i,str,LVCFMT_LEFT,99);
				if (m_sortcolumn>=i)
					m_sortcolumn++;
				break;
			}
		}
	}
	m_nHideColumns=nHideColumns;
}

void CLocalFileListCtrl::OnLocalcontextProperties() 
{
	POSITION selpos=GetFirstSelectedItemPosition();
	if (!selpos)
		return;
	
	int nItem = GetNextSelectedItem(selpos);
	if (!nItem && m_Fullpath!="")
		return;
	int index=m_IndexMapping[nItem];

	//Show the "properties" for the selected file
    CString sFile = m_Fullpath+m_FileData[index].Name;
    SHELLEXECUTEINFO sei;
    ZeroMemory(&sei,sizeof(sei));
    sei.cbSize = sizeof(sei);
    sei.hwnd = AfxGetMainWnd()->GetSafeHwnd();
    sei.nShow = SW_SHOW;
    sei.lpFile = sFile.GetBuffer(sFile.GetLength()+1);
    sei.lpVerb = _T("properties");
    sei.fMask  = SEE_MASK_INVOKEIDLIST;
	sei.lpParameters = 0;//_T("An example: \"\"\"quoted text\"\"\""9;
    BOOL bSuccess = ShellExecuteEx(&sei);
    sFile.ReleaseBuffer(0);	
}

void CLocalFileListCtrl::SaveColumnSizes()
{
	int nSize[4];
	nSize[0]=GetColumnWidth(0);
	int index=1;
	nSize[1]=60;
	nSize[2]=100;
	nSize[3]=99;

	if (!(m_nHideColumns&1))
	{
		nSize[1]=GetColumnWidth(index);
		index++;
	}
	
	if (!(m_nHideColumns&2))
	{
		nSize[2]=GetColumnWidth(index);
		index++;
	}
	
	if (!(m_nHideColumns&4))
	{
		nSize[3]=GetColumnWidth(index);
		index++;
	}

	CString str;
	str.Format(_T("%d %d %d %d"), nSize[0], nSize[1], nSize[2], nSize[3]);
	//COptions::SetOption(OPTION_LOCALCOLUMNWIDTHS, str);
}

void CLocalFileListCtrl::OnDestroy() 
{
	/*
	if (COptions::GetOptionVal(OPTION_REMEMBERLOCALCOLUMNWIDTHS))
		SaveColumnSizes();

	if (COptions::GetOptionVal(OPTION_LOCALCOLUMNSORT))
	{
		int nSort = 1;
		nSort |= m_Columns[m_sortcolumn] << 1;
		nSort |= m_sortdir << 4;
		COptions::SetOption(OPTION_LOCALCOLUMNSORT, nSort);
	}

	*/

	CListCtrl::OnDestroy();	
}

LRESULT CLocalFileListCtrl::OnUpdateContinue(WPARAM wParam, LPARAM lParam)
{
	if (!m_bUpdating)
		return 0;
	
	ASSERT(lParam);

	int nOldItemCount = m_FileData.size();
	int nItemCount=nOldItemCount;
	HANDLE hFind=(HANDLE)lParam;

	WIN32_FIND_DATA find;
	if (!FindNextFile(hFind, &find))
	{
		FindClose(hFind);
		hFind=0;
	}
	
	while ( hFind )
	{
		if (!(find.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY && (!_tcscmp(find.cFileName, _T("..")) || !_tcscmp(find.cFileName, _T(".")))))
		{
			t_FileData FileData;
			FileData.Name=find.cFileName;
			FileData.lName=find.cFileName;
			FileData.lName.MakeLower();

			TRY
			{
				FileData.Time = find.ftLastWriteTime;
				FileData.hasTime = true;
			}
			CATCH_ALL(e)
			{
				FileData.hasTime = false;
			}
			END_CATCH_ALL;

			if ( find.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			{
				FileData.bIsDir=TRUE;
				FileData.nSize=-1;
			}
			else
			{
				FileData.bIsDir=FALSE;
				FileData.nSize=(_int64)find.nFileSizeLow + ((_int64)find.nFileSizeHigh<<32);
			}
			
			m_FileData.push_back(FileData);
			m_IndexMapping.push_back(nItemCount);
			
			nItemCount++;
		}
		if ((nItemCount-nOldItemCount)==100)
		{
			VERIFY(PostMessage(WM_APP + 1, wParam, (LPARAM)hFind));
			return 0;
		}		
		if (!FindNextFile(hFind, &find))
		{
			FindClose(hFind);
			hFind=0;
		}
	
	}

	m_bUpdating=FALSE;

	SetItemCount(nItemCount);
	
	SortList(m_sortcolumn,m_sortdir);

	EnableWindow(TRUE);
	
	SetRedraw( TRUE );

	if (wParam)
		SetFocus();

	if (m_NewDir!="")
		SetFolder(m_NewDir);

	UpdateStatusBar();

	return 0;
}

void CLocalFileListCtrl::RefreshFile(CString file)
{
	
	//Refreshes a single file in a list
	int i;
	int pos=file.ReverseFind('\\');
	if ( pos==-1 || pos==(file.GetLength()-1) )
		return;

	WIN32_FIND_DATA find;
	HANDLE hFind;
	CString name;
	BOOL bDir=FALSE;
	if (file.Left(pos+1).CollateNoCase(m_Fullpath))
	{
		if (m_Fullpath.CollateNoCase(file.Left(m_Fullpath.GetLength())))
			return;
		
		file=file.Mid(m_Fullpath.GetLength());
		int pos=file.Find('\\');
		if ( pos==-1 || pos==(file.GetLength()-1) || !pos)
			return;
		file=file.Left(pos);
		
		hFind = FindFirstFile( m_Fullpath+file+"\\.", &find);
		
		name=file;
		name.MakeLower();
		
		for (i=0;i<static_cast<int>(m_FileData.size());i++)
		{
			if (name==m_FileData[i].lName)
				break;
		}

		bDir=TRUE;
	}
	else
	{
		name=file.Mid(pos+1);
		name.MakeLower();
	
		for (i=0;i<static_cast<int>(m_FileData.size());i++)
		{
			if (name==m_FileData[i].lName)
				break;
		}
		hFind = FindFirstFile( file, &find);	
	}
	
	unsigned int nIndex=i;
	if (!hFind || hFind==INVALID_HANDLE_VALUE)
	{
		//File does not exist
		if (nIndex!=m_FileData.size())
		{ //But file is still visible in list
			if (!bDir && m_FileData[nIndex].bIsDir)
			{
				FindClose(hFind);
				return;
			}
			if (bDir && !m_FileData[nIndex].bIsDir)
			{
				FindClose(hFind);
				return;
			}
			
			m_FileData.erase(m_FileData.begin()+nIndex);
			unsigned int j;
			for (j=0; j<static_cast<int>(m_IndexMapping.size()); j++)
			{
				if (m_IndexMapping[j]==nIndex)
					break;
			}
			ASSERT(j!=m_IndexMapping.size());
			m_IndexMapping.erase(m_IndexMapping.begin()+j);
			for (i=1;i<static_cast<int>(m_IndexMapping.size());i++)
			{
				if (m_IndexMapping[i]>nIndex)
					m_IndexMapping[i]--;
			}
			SetItemState( j, 0, LVIS_SELECTED);
			for (int nItem=j+1;nItem<GetItemCount();nItem++)
			{
				if (GetItemState( nItem, LVIS_SELECTED))
				{
					SetItemState( nItem, 0, LVIS_SELECTED);
					SetItemState( nItem-1, LVIS_SELECTED, LVIS_SELECTED);
				}
			}
			
			SetItemCount(GetItemCount()-1);
		}
	}
	else
	{
		if (!bDir)
		{
			if (find.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY || !_tcscmp(find.cFileName, _T(".")) || !_tcscmp(find.cFileName, _T("..")) )
			{
				FindClose(hFind);
				return;
			}
		}
		else if (!(find.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
		{
			FindClose(hFind);
			return;
		}

		t_FileData FileData;
		int count=GetItemCount();
		int datapos;
		BOOL bSelected=FALSE;
		if (nIndex!=m_FileData.size())
		{ //File is already visible in list
			if (!bDir && m_FileData[nIndex].bIsDir)
			{
				FindClose(hFind);	
				return;
			}
			else if (bDir && !m_FileData[nIndex].bIsDir)
			{
				FindClose(hFind);	
				return;
			}
			m_FileData[nIndex].Name=find.cFileName;
			m_FileData[nIndex].lName=find.cFileName;
			m_FileData[nIndex].lName.MakeLower();
			m_FileData[nIndex].nSize=bDir ? -1 : ((_int64)find.nFileSizeLow + ((_int64)find.nFileSizeHigh<<32));
			m_FileData[nIndex].Time=find.ftLastWriteTime;
			FileData=m_FileData[nIndex];
			unsigned int j;
			for (j=0; j<static_cast<int>(m_IndexMapping.size()); j++)
			{
				if (m_IndexMapping[j]==nIndex)
					break;
			}
			ASSERT(j!=m_IndexMapping.size());
			m_IndexMapping.erase(m_IndexMapping.begin()+j);
			if (GetItemState( j, LVIS_SELECTED))
				bSelected=TRUE;
			SetItemState( j, 0, LVIS_SELECTED);
			for (int nItem=j+1;nItem<GetItemCount();nItem++)
			{
				if (GetItemState( nItem, LVIS_SELECTED))
				{
					SetItemState( nItem, 0, LVIS_SELECTED);
					SetItemState( nItem-1, LVIS_SELECTED, LVIS_SELECTED);
				}
			}
			datapos=nIndex;
			count--;
		}
		else
		{
			FileData.Name=find.cFileName;
			FileData.lName=find.cFileName;
			FileData.lName.MakeLower();
			FileData.bIsDir=bDir;
			FileData.nSize=bDir ? -1 : ((_int64)find.nFileSizeLow + ((_int64)find.nFileSizeHigh<<32));
			FileData.Time=find.ftLastWriteTime;
			m_FileData.push_back(FileData);
			datapos=count;
			SetItemCount(GetItemCount()+1);
		}
		CString filetype;
		if (m_Columns[m_sortcolumn]==2)
		{
			filetype=GetType(FileData.lName, FileData.bIsDir);
			filetype.MakeLower();
		}
		
		if (count>1)
		{ //Binary search for optimal position to insert file sorted.
			int anf=1;
			int ende=count-1;
			
			int mitte;
			while(anf<=ende)
			{
				mitte=(anf+ende)/2;
				
				t_FileData CompareData=m_FileData[m_IndexMapping[mitte]];
				BOOL res;
				if (!m_Columns[m_sortcolumn]) //Chose compare function based on column and direction
					if (m_sortdir==1)
						res=lesser(CompareData.lName, FileData.lName, CompareData.bIsDir, FileData.bIsDir);
					else
						res=greater(CompareData.lName, FileData.lName, CompareData.bIsDir, FileData.bIsDir);
				else if (m_Columns[m_sortcolumn]==1)
					if (m_sortdir==1)
						res=lesserbysize(CompareData.nSize, FileData.nSize, CompareData.bIsDir, FileData.bIsDir, CompareData.lName, FileData.lName);
					else
						res=greaterbysize(CompareData.nSize, FileData.nSize, CompareData.bIsDir, FileData.bIsDir, CompareData.lName, FileData.lName);
				else if (m_Columns[m_sortcolumn]==2)
				{
					CString typecompare=GetType(CompareData.lName, CompareData.bIsDir);
					typecompare.MakeLower();
					if (m_sortdir==1)
						res=lesserbytype(typecompare, filetype, CompareData.bIsDir, FileData.bIsDir, CompareData.lName, FileData.lName);
					else
						res=greaterbytype(typecompare, filetype, CompareData.bIsDir, FileData.bIsDir, CompareData.lName, FileData.lName);
				}
				else if (m_Columns[m_sortcolumn]==3)
					if (m_sortdir==1)
						res=lesserbytime(CompareData.Time, FileData.Time, CompareData.bIsDir, FileData.bIsDir, CompareData.lName, FileData.lName);
					else
						res=greaterbytime(CompareData.Time, FileData.Time, CompareData.bIsDir, FileData.bIsDir, CompareData.lName, FileData.lName);
						
				if (res)
					anf=mitte+1;
				else
				{
					ende=mitte-1;
					mitte--;
				}
			}
			m_IndexMapping.insert(m_IndexMapping.begin()+mitte+1, datapos);
			for (int nItem=GetItemCount()-1; nItem>(mitte+1); nItem--)
			{
				if (GetItemState( nItem-1, LVIS_SELECTED))
				{
					SetItemState( nItem-1, 0, LVIS_SELECTED);
					SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED);
				}
			}
			if (bSelected)
				SetItemState( mitte+1, LVIS_SELECTED, LVIS_SELECTED);
		}
		else
			m_IndexMapping.push_back(count);
	}
	if (hFind && hFind!=INVALID_HANDLE_VALUE)
		FindClose(hFind);
	RedrawItems(0, GetItemCount());
	UpdateStatusBar();
}

CString CLocalFileListCtrl::GetType(CString lName, BOOL bIsDir)
{
	
	CString type;
	std::map<CString, CString>::iterator typeIter=m_TypeCache.find(m_Fullpath+lName);
	if (typeIter==m_TypeCache.end())
	{
		CString str=PathFindExtension(lName);
		str.MakeLower();
		std::map<CString, CString>::iterator permTypeIter=m_permanentTypeCache.find(str);
		if (permTypeIter!=m_permanentTypeCache.end())
		{
			m_TypeCache[m_Fullpath+lName]=permTypeIter->second;
			type=permTypeIter->second;
		}
		else
		{
			SHFILEINFO shFinfo;
			CString path;
			path=m_Fullpath+lName;
			if (m_Fullpath=="")
				path+="\\";
			memset(&shFinfo,0,sizeof(SHFILEINFO));
			if (SHGetFileInfo(path,
				bIsDir?FILE_ATTRIBUTE_DIRECTORY:FILE_ATTRIBUTE_NORMAL,
				&shFinfo,
				sizeof( shFinfo ),
				SHGFI_TYPENAME))
			{
				type=shFinfo.szTypeName;
				if (type=="")
				{
					type=PathFindExtension(lName);
					if (type!="") 
						type=type.Mid(1);
					type.MakeUpper();
					if (type!="")
						type+="-file";
					else
						type="File";
				}
				else
				{
					CString str2=PathFindExtension(lName);
					if (!bIsDir && str2!="")
					{
						str2.MakeLower();
						m_permanentTypeCache[str2]=type;
					}
				}
			}
			else
			{
				type=PathFindExtension(lName);
				if (type!="") 
					type=type.Mid(1);
				type.MakeUpper();
				if (type!="")
					type+="-file";
				else
					type="File";
			}
			m_TypeCache[m_Fullpath+lName]=type;		
		}
	}
	else
		type=typeIter->second;

	return type;
	

}

void CLocalFileListCtrl::OnLocalcontextViewEdit()
{
	/*
	if (m_Fullpath=="")
		return;

	POSITION selpos=GetFirstSelectedItemPosition();
	if (!selpos)
		return;
	
	int nItem = GetNextSelectedItem(selpos);
	if (!nItem)
		return;
	
	int index=m_IndexMapping[nItem];

	if (m_FileData[index].bIsDir)
		return;

	CString file=m_FileData[index].lName;

	int pos=file.ReverseFind('.');
	if (pos!=-1)
	{
		CString fext=file.Mid(pos+1);
		fext.MakeLower();
		//Parse the file associations
		CString CustomAssociations=COptions::GetOption(OPTION_VIEWEDITCUSTOM);
		
		CString ext;
		CString prog;
		BOOL bDoExt=TRUE;
		while (CustomAssociations!="")
		{
			int pos=CustomAssociations.Find( _T(";") );
			if (bDoExt)
			{
				if (!pos || pos==-1 || pos==CustomAssociations.GetLength()-1)
					break;
				ext+=CustomAssociations.Left(pos);
				CustomAssociations=CustomAssociations.Mid(pos+1);
				if (CustomAssociations.Left(1)== _T(" "))
				{
					ext+=_T(";");
					CustomAssociations=CustomAssociations.Mid(1);
				}
				else
					bDoExt=FALSE;
			}
			else
			{
				if (!pos || pos==CustomAssociations.GetLength()-1)
					break;
				if (pos!=-1)
				{
					prog+=CustomAssociations.Left(pos);
					CustomAssociations=CustomAssociations.Mid(pos+1);
				}
				else
				{
					prog=CustomAssociations;
					CustomAssociations="";
				}
				if (CustomAssociations.Left(1)== _T(" "))
				{
					prog+=_T(";");
					CustomAssociations=CustomAssociations.Mid(1);
					if (CustomAssociations!="")
						continue;
				}
				
				ext.MakeLower();
				if (fext==ext)
				{ //We've found a file aassociation for this extension
					CString cmdLine;
					file=m_Fullpath+file;
					if (file.Find( _T(" ") )!=-1)
						file=_T("\"") + file + _T("\"");
					cmdLine=prog + _T(" ") + file;
					PROCESS_INFORMATION ProcessInformation;  
					STARTUPINFO startupinfo={0};
					startupinfo.cb=sizeof(startupinfo);
					LPTSTR str=new TCHAR[cmdLine.GetLength()+1];
					_tcscpy(str, cmdLine);
					if (CreateProcess(0, str, 0, 0, 0, 0, 0, 0, &startupinfo, &ProcessInformation))
					{
						CloseHandle(ProcessInformation.hThread);
						CloseHandle(ProcessInformation.hProcess);
					}
					delete [] str;
					return;;
				}
				ext="";
				prog="";
				bDoExt=TRUE;
			}
		}
	}
	//File has no extension or custom file association could not be found
	CString defprog=COptions::GetOption(OPTION_VIEWEDITDEFAULT);
	if (defprog=="")
	{
		AfxMessageBox(IDS_ERRORMSG_VIEWEDIT_NODEFPROG, MB_ICONEXCLAMATION);
		return;
	}
	CString cmdLine;
	file=m_Fullpath+file;
	if (file.Find( _T(" ") )!=-1)
		file=_T("\"") + file + _T("\"");
	cmdLine=defprog + _T(" ") + file;
	PROCESS_INFORMATION ProcessInformation;  
	STARTUPINFO startupinfo={0};
	startupinfo.cb=sizeof(startupinfo);
	LPTSTR str=new TCHAR[cmdLine.GetLength()+1];
	_tcscpy(str, cmdLine);
	if (CreateProcess(0, str, 0, 0, 0, 0, 0, 0, &startupinfo, &ProcessInformation))
	{
		CloseHandle(ProcessInformation.hThread);
		CloseHandle(ProcessInformation.hProcess);
	}
	delete [] str;
	*/
}

BOOL CLocalFileListCtrl::UpdateStatusBar()
{
	
	if (m_bUpdating)
		return FALSE;

	CString str;
	POSITION selpos = GetFirstSelectedItemPosition();
	
	int dircount = 0;
	int filecount = 0;
	_int64 size = 0;

	while (selpos)
	{
		int nItem = GetNextSelectedItem(selpos);
		if (!nItem)
			continue;

		int nIndex = m_IndexMapping[nItem];
		if (m_FileData[nIndex].bIsDir)
				dircount++;
			else
			{
				filecount++;
				size += m_FileData[nIndex].nSize;
			}
	}

	if (dircount || filecount)
	{
		if (!dircount)
			if (filecount == 1)
				str.Format(IDS_DIRINFO_SELECTED_FILE, size);
			else
				str.Format(IDS_DIRINFO_SELECTED_FILES, filecount, size);
		else if (!filecount)
			if (dircount == 1)
				str.LoadString(IDS_DIRINFO_SELECTED_DIR);
			else
				str.Format(IDS_DIRINFO_SELECTED_DIRS, dircount);
		else if (dircount == 1)
			if (filecount == 1)
				str.Format(IDS_DIRINFO_SELECTED_DIRANDFILE, size);
			else
				str.Format(IDS_DIRINFO_SELECTED_DIRANDFILES, filecount, size);
		else
			if (filecount == 1)
				str.Format(IDS_DIRINFO_SELECTED_DIRSANDFILE, dircount, size);
			else
				str.Format(IDS_DIRINFO_SELECTED_DIRSANDFILES, dircount, filecount, size);
		return m_pOwner->SetStatusBarText(str);
	}

	if (m_FileData.size() <= 1)
		str.LoadString(IDS_DIRINFO_EMPTY);
	else
	{
		for (unsigned int i=1; i<m_FileData.size(); i++)
			if (m_FileData[i].bIsDir)
				dircount++;
			else
			{
				filecount++;
				size += m_FileData[i].nSize;
			}
		if (!dircount)
			if (filecount == 1)
				str.Format(IDS_DIRINFO_FILE, size);
			else
				str.Format(IDS_DIRINFO_FILES, filecount, size);
		else if (!filecount)
			if (dircount == 1)
				str.LoadString(IDS_DIRINFO_DIR);
			else
				str.Format(IDS_DIRINFO_DIRS, dircount);
		else if (dircount == 1)
			if (filecount == 1)
				str.Format(IDS_DIRINFO_DIRANDFILE, size);
			else
				str.Format(IDS_DIRINFO_DIRANDFILES, filecount, size);
		else
			if (filecount == 1)
				str.Format(IDS_DIRINFO_DIRSANDFILE, dircount, size);
			else
				str.Format(IDS_DIRINFO_DIRSANDFILES, dircount, filecount, size);			
	}
	
	  
	return m_pOwner->SetStatusBarText(str);
	return TRUE;
}

void CLocalFileListCtrl::OnItemchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	
	*pResult = 0;

	POSITION selpos=GetFirstSelectedItemPosition();

	if (!selpos)
	{
		m_pOwner->m_pOwner->m_displayDelete = 0;
	}
	else{
		CString folder = GetFolder();
		if(folder != _T("")){			
			m_pOwner->m_pOwner->m_displayDelete = 1;

		}
	}

//	MessageBox(L"hello");
/*
		m_transferuser=m_transferpass="";
	if (!GetItemCount())
		return;
	CMenu menu;
	menu.LoadMenu(IDR_FILEVIEW);

	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);
	CWnd* pWndPopupOwner = this;
	//while (pWndPopupOwner->GetStyle() & WS_CHILD)
	//	pWndPopupOwner = pWndPopupOwner->GetParent();

	POSITION selpos=GetFirstSelectedItemPosition();

	if (!selpos)
	{
		pPopup->EnableMenuItem(ID_FILEVIEW_RENAME, MF_GRAYED);	
		pPopup->EnableMenuItem(ID_FILEVIEW_DELETE, MF_GRAYED);	

		pPopup->EnableMenuItem(ID_FILEVIEW_PROPERTIES, MF_GRAYED);

		CString folder = GetFolder();
		if(folder == _T("")){			
			pPopup->EnableMenuItem(ID_FILEVIEW_NEWFOLDER, MF_GRAYED);
		}
		if (point.x==-1 || point.y==-1)
		{
			point.x=5;
			point.y=5;
			ClientToScreen(&point);
		}
	}


	else
	{
		int nItem = GetNextSelectedItem(selpos);
		if (point.x==-1 || point.y==-1)
		{
			CRect rect;
			GetItemRect(nItem,&rect,LVIR_LABEL);
			point.x=rect.left+5;
			point.y=rect.top+5;
			ClientToScreen(&point);
		}
		int index=m_IndexMapping[nItem];
		if (m_Fullpath=="")
		{
			pPopup->EnableMenuItem(ID_FILEVIEW_RENAME, MF_GRAYED);	
			pPopup->EnableMenuItem(ID_FILEVIEW_DELETE, MF_GRAYED);	
		//	pPopup->EnableMenuItem(ID_FILEVIEW_NEWFOLDER, MF_GRAYED);
			pPopup->EnableMenuItem(ID_FILEVIEW_PROPERTIES, MF_GRAYED);
		}
		else if (!nItem)
		{
			pPopup->EnableMenuItem(ID_FILEVIEW_RENAME, MF_GRAYED);	
			pPopup->EnableMenuItem(ID_FILEVIEW_DELETE, MF_GRAYED);	
		//	pPopup->EnableMenuItem(ID_FILEVIEW_NEWFOLDER, MF_GRAYED);
			pPopup->EnableMenuItem(ID_FILEVIEW_PROPERTIES, MF_GRAYED);
		}
		
	
		nItem = GetNextSelectedItem(selpos);
		while (nItem!=-1)
		{
			pPopup->EnableMenuItem(ID_FILEVIEW_PROPERTIES, MF_GRAYED);
			pPopup->EnableMenuItem(ID_FILEVIEW_RENAME, MF_GRAYED);
			int index=m_IndexMapping[nItem];
			nItem = GetNextSelectedItem(selpos);
		}
		
	}
	
		
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,
		pWndPopupOwner);

  */
	UpdateStatusBar();
}

void CLocalFileListCtrl::GetSortInfo(int &column, int &direction) const
{
	column = m_sortcolumn;
	direction = m_sortdir;
}

BOOL CLocalFileListCtrl::DragPosition(CImageList *pImageList, CWnd* pDragWnd, CPoint point)
{
	ScreenToClient(&point);
	int nItem = HitTest(point);

	int nIndex = -1;
	
	if (nItem != -1)
	{
		nIndex = m_IndexMapping[nItem];
		if (!m_FileData[nIndex].bIsDir)
			nIndex = -1;
	}
	if (pDragWnd == this)
	{
		POSITION selpos = GetFirstSelectedItemPosition();
		while (selpos)
		{
			int nSelectedItem = GetNextSelectedItem(selpos);
			if (nSelectedItem == nItem)
			{
				DragLeave(pImageList);
				return FALSE;
			}
		}
	}
	if (nIndex != -1)
	{
		if (nItem != m_nDragHilited)
		{
			pImageList->DragShowNolock(false);
			if (m_nDragHilited != -1)
			{
				SetItemState(m_nDragHilited, 0, LVIS_DROPHILITED);
				RedrawItems(m_nDragHilited, m_nDragHilited);
			}
			m_nDragHilited = nItem;
			SetItemState(nItem, LVIS_DROPHILITED, LVIS_DROPHILITED);
			RedrawItems(nItem, nIndex);
			UpdateWindow();
			pImageList->DragShowNolock(true);
		}
	}
	else
		DragLeave(pImageList);

	return TRUE;
}

void CLocalFileListCtrl::DragLeave(CImageList *pImageList)
{
	if (m_nDragHilited != -1)
	{
		if (pImageList)
			pImageList->DragShowNolock(false);
		SetItemState(m_nDragHilited, 0, LVIS_DROPHILITED);
		RedrawItems(m_nDragHilited, m_nDragHilited);
		UpdateWindow();
		if (pImageList)
			pImageList->DragShowNolock(true);
		m_nDragHilited = -1;
	}
}

void CLocalFileListCtrl::OnDragEnd(int target, CPoint point)
{
	/*
	m_transferuser = m_transferpass = "";
	if (target == 1)
		OnLocalcontextAddtoqueue();
	else if (!target)
		OnLocalcontextUpload();
	else if (target == 2 || target == 3)
	{
		if (m_Fullpath == _T("") ||
			m_Fullpath == _T("\\"))
			return;

		CString to;
		if (target == 2)
		{
			// Check if drag target is valid
			if (m_nDragHilited == -1)
			return;

			POSITION selpos = GetFirstSelectedItemPosition();
			while (selpos)
			{
				int nItem = GetNextSelectedItem(selpos);
				if (nItem == m_nDragHilited)
					return;
			}

			if (!m_nDragHilited)
			{
				to = m_Fullpath;
				to.TrimRight('\\');
				int pos = to.ReverseFind('\\');
				if (pos < 2)
					return;
	
				to = to.Left(pos);
			}
			else
				to = m_Fullpath + "\\" + m_FileData[m_IndexMapping[m_nDragHilited]].Name;
			to.Replace(_T("\\\\"), _T("\\"));
		}
		else
		{
			CMainFrame *pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetParentFrame());
			to = reinterpret_cast<CDirTreeCtrl *>(pMainFrame->GetLocalPane()->GetTreeCtrl())->GetDropTarget();
			if (to == _T("") || to == _T("\\"))
				return;
		}

		LPTSTR pTo = new TCHAR[to.GetLength() + 2];
		_tcscpy(pTo, to);
		pTo[to.GetLength() + 1] = 0;

		POSITION selpos = GetFirstSelectedItemPosition();
		while (selpos)
		{
			int nItem = GetNextSelectedItem(selpos);
			CString from = m_Fullpath + "\\" + m_FileData[m_IndexMapping[nItem]].Name;
			from.Replace(_T("\\\\"), _T("\\"));
			
			LPTSTR pFrom = new TCHAR[from.GetLength() + 2];
			_tcscpy(pFrom, from);
			pFrom[from.GetLength() + 1] = 0;

			SHFILEOPSTRUCT op;
			op.hwnd = m_hWnd;
			op.wFunc = FO_MOVE;
			op.pFrom = pFrom;
			op.pTo = pTo;
			op.fFlags = 0;
			op.hNameMappings = NULL;
			op.lpszProgressTitle = NULL;
			
			SHFileOperation(&op);
			
			delete [] pFrom;
		}

		CMainFrame *pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetParentFrame());
		reinterpret_cast<CDirTreeCtrl *>(pMainFrame->GetLocalPane()->GetTreeCtrl())->RefreshDir(to);
		reinterpret_cast<CDirTreeCtrl *>(pMainFrame->GetLocalPane()->GetTreeCtrl())->RefreshDir(m_Fullpath);

		delete [] pTo;

		SetFolder(GetFolder());
	}
	*/
}

CString CLocalFileListCtrl::GetDropTarget() const
{
	if (m_nDragHilited == -1)
		return _T("");

	CString subdir = m_FileData[m_IndexMapping[m_nDragHilited]].Name;
	if (subdir == _T(".."))
	{
		CString path = GetFolder();
		path.TrimRight('\\');
		int pos = path.ReverseFind('\\');
		if (pos == -1)
			return "";

		return path.Left(pos + 1);
	}

	return m_Fullpath + subdir;
}
