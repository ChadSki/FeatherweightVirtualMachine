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
//#include "EnterSomething.h"
//#include "TransferAsDlg.h"
#include "PathFunctions.h"
//#include "CommandQueue.h"
//#include "FtpListCtrl.h"
//#include "DirTreeCtrl.h"
#include "resource.h"
#include "FileView.h"
#include <shlobj.h>
#include <shellapi.h>
//#include "Options.h"

#include <queue>


#include <list>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLocalFileListCtrl

CLocalFileListCtrl::CLocalFileListCtrl(CFileView *pOwner)
{
	ASSERT(pOwner);

	m_pOwner = pOwner;

	m_Fullpath = ".."; //Just anything invalid
	m_nStyle = -1;
	for (int i = 0; i < 5; i++)
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
	ON_NOTIFY_REFLECT(LVN_BEGINLABELEDIT, OnBeginlabeledit)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnclick)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_WM_DESTROY()
	ON_NOTIFY_REFLECT(LVN_ENDLABELEDIT, OnEndlabeledit)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetdispinfo)
//	ON_COMMAND(ID_FILEVIEW_NEWFOLDER, OnLocalcontextCreatedirectory)
//	ON_COMMAND(ID_FILEVIEW_DELETE, OnLocalcontextDelete)
//	ON_COMMAND(ID_FILEVIEW_RENAME, OnLocalcontextRename)
//	ON_COMMAND(ID_FILEVIEW_PROPERTIES, OnLocalcontextProperties)
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



void CLocalFileListCtrl::GetSideEffectList(CString cdir, int *nItemCount)
{
  HANDLE  hFile;               // handle to the drive to be examined 
//  BOOL bResult;                 // results flag
 

//  PHASH_ENTRY_LOG  head;
  WCHAR  deleted[1024];
  HASH_ENTRY_LOG    tmp;

  int  count = 0;
  DWORD filesize = 0;
  DWORD bytesread = 0;

 // MessageBox(cdir);
  //WCHAR filepath[VMFILEPATHLEN];
  //filepath[0] = L'\0';
	CString filepath;
	t_FvmData *fd = m_pOwner->m_Parent->m_selectedFvm;
	filepath=fd->fvmRoot+L"\\"+fd->fvmID+L".bin";

//	MessageBox(filepath);
  hFile = CreateFile(filepath,GENERIC_READ,FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
  if (hFile == INVALID_HANDLE_VALUE) // we can't open the file
  {
    return;
  } 
  else
  {
    filesize = GetFileSize(hFile, NULL);
    count = (filesize / sizeof(HASH_ENTRY_LOG)) + 1; 

	count = 0;
	do
	{
	  memset(&tmp,'\0',sizeof(HASH_ENTRY_LOG));
	  memset(deleted,'\0', 1024*2);//sizeof(HASH_ENTRY_LOG));
      if(ReadFile(hFile,&tmp,
		  sizeof(HASH_ENTRY_LOG),&bytesread,NULL))
	  {
		if(bytesread == 0)
			break;


		memcpy(deleted,&tmp,sizeof(HASH_ENTRY_LOG));

		if(deleted[0] == L'\\' && deleted[1] == L'?' && deleted[2] == L'?' && deleted[3] == L'\\'){
			if(deleted[4] == cdir.GetAt(0)){
				t_FileData FileData;

				FileData.lName= deleted+4;
				FileData.Name= deleted+4;
				
				FileData.bIsDir=FALSE;
				FileData.nSize= 0;
			
				FileData.Action = L"Deleted";

				m_FileData.push_back(FileData);
				m_IndexMapping.push_back(*nItemCount);
			
				(*nItemCount)++;

				
			}
		}
	  }
	  else
	  {
	
        break;
	  }
	}while(1);
  }

  //free(sideeffectlist);
   CloseHandle(hFile);


}

BOOL CLocalFileListCtrl::FileFound(CString szFindFile)
{

	BOOL bFileFound = FALSE;
	WIN32_FIND_DATA FileInfo;
	HANDLE hFind;
	hFind = ::FindFirstFile(szFindFile, &FileInfo) ;
	if (hFind == INVALID_HANDLE_VALUE )
		bFileFound = FALSE;
	else
		bFileFound = TRUE;
	::FindClose (hFind);

	return bFileFound;

}

void CLocalFileListCtrl::GetSideEffects(CString folder, int *nItemCount)
{

	using namespace std;




	
	folder.TrimRight( _T("\\") ); 
	folder+=_T("\\");

	/*

	CString displayPath = folder;//m_Fullpath;
	displayPath.Delete(0, m_rootPath.GetLength());
	displayPath.TrimRight(_T("\\"));

	CString tmpStr = displayPath;
	tmpStr.Delete(0,1);
	displayPath.Delete(1, displayPath.GetLength() -1);
	displayPath.MakeUpper();
	displayPath=displayPath+ L":"+tmpStr;

	if(displayPath.GetLength() == 2)
		displayPath = displayPath+L"\\";

	m_pOwner->m_Parent->pSplitterWnd4->SetPaneCaptionText(0, _T(" File View     --        ")+ displayPath);
	*/
	m_bUpdating=TRUE;

	SetRedraw(FALSE);
	EnableWindow(FALSE);
	
	
	CString   strPathFiles = folder;
	CString  longPath;



	CString tempPath = folder;

	longPath = tempPath;

	if (tempPath.Right(1) != "\\" ){
		tempPath += "\\";
			longPath += "\\";
	}
	tempPath += "*.*";


	WIN32_FIND_DATA find;
	HANDLE hFind = FindFirstFile(tempPath, &find);

	CString folder2=folder;
	folder2.TrimRight('\\');
	
	int pos=folder2.ReverseFind('\\');
	folder2=folder2.Left(pos+1);


	//MessageBox(tempPath);
		while ( hFind!=INVALID_HANDLE_VALUE)
		{
			if (!(find.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY && (!_tcscmp(find.cFileName, _T("..")) || !_tcscmp(find.cFileName, _T(".")))))
			{
	
					t_FileData FileData;

					FileData.lName=longPath+find.cFileName;
					FileData.Name= FileData.lName.GetBuffer(FileData.lName.GetLength())+m_rootPath.GetLength();
					FileData.Name.Insert(1, L':');
					//FileData.lName.MakeLower();
			
				
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
						CString newPath = longPath+find.cFileName;
						//MessageBox(newPath);
						GetSideEffects(newPath, nItemCount);
					}
					else
					{

						FileData.bIsDir=FALSE;
						FileData.nSize= (_int64)find.nFileSizeLow + ((_int64)find.nFileSizeHigh<<32);
			
						if(FileFound(FileData.Name)){
							FileData.Action = L"Modified";
						}
						else
							FileData.Action = L"Created";

						m_FileData.push_back(FileData);
						m_IndexMapping.push_back(*nItemCount);
			
						(*nItemCount)++;

					}
			
				
				
			}
			if (!FindNextFile(hFind, &find))
			{
				FindClose(hFind);
				hFind=0;

				break;
			}		
		}		
	


}

void CLocalFileListCtrl::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{

	POSITION selpos = GetFirstSelectedItemPosition();

	CString cdir;

	if (selpos)
	{
		int nItem = GetNextSelectedItem(selpos);
		int nIndex=m_IndexMapping[nItem];
		if (m_FileData[nIndex].bIsDir)
		{
		
			CString newpath;
		
			if(m_FileData[nIndex].Name == L".."){
				m_Fullpath = m_rootPath;
				DisplayDrives();
				return;
			}

			if (!nItem && m_Fullpath!=m_rootPath)
			{							
				newpath=m_Fullpath;
				newpath.TrimRight('\\');
				int pos=newpath.ReverseFind('\\');
				newpath=newpath.Left(pos+1);
			}
			else
				newpath=m_Fullpath+m_FileData[nIndex].Name + "\\";
	
			cdir = m_FileData[nIndex].Name;
			//MessageBox(m_FileData[nIndex].Name);
		
			if (m_bUpdating)
			{
				m_NewDir = newpath;
				return;
			}
			else
				m_NewDir="";

			BOOL bDidHaveFocus = GetFocus() == this;
			int nItemCount;

			m_FileData.clear();
			m_IndexMapping.clear();
			m_Fullpath=newpath;
			DeleteAllItems();
			


			t_FileData FileData;

			FileData.Name="..";
			FileData.lName="..";
			FileData.bIsDir=TRUE;
			m_IndexMapping.push_back(0);
			FileData.nSize=-1;
			FileData.hasTime = false;
			m_FileData.push_back(FileData);
	
			nItemCount= 1;
			
				//SetFolder(newpath);
		
			GetSideEffects(newpath, &nItemCount);

			GetSideEffectList(cdir, &nItemCount);

		
			
			m_bUpdating=FALSE;

			SetItemCount(nItemCount);
			SortList(m_sortcolumn,m_sortdir);
			EnableWindow(TRUE);	
			SetRedraw( TRUE );

			if (bDidHaveFocus)
				SetFocus();
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

	
	int widths[5]={250, 66, 66, 75, 130};


	CString str;
	str.LoadString(IDS_HEADER_FILENAME);
	InsertColumn(0,str,LVCFMT_LEFT, widths[0]);

	str.LoadString(IDS_HEADER_FILEACTION);
	InsertColumn(1,str,LVCFMT_LEFT, widths[1]);

	str.LoadString(IDS_HEADER_FILESIZE);
	InsertColumn(2,str,LVCFMT_RIGHT, widths[2]);
	str.LoadString(IDS_HEADER_FILETYPE);
	InsertColumn(3,str,LVCFMT_LEFT, widths[3]);
	str.LoadString(IDS_HEADER_LASTMODIFIED);
	InsertColumn(4,str,LVCFMT_LEFT, widths[4]);
	
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

BOOL fgreater(const CString &str1, const CString &str2, BOOL isdir1, BOOL isdir2)
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
	CString ref=m_FileData[m_IndexMapping[(l+r)/2]].Name;
	BOOL bRefIsDir=m_FileData[m_IndexMapping[(l+r)/2]].bIsDir;
	do
    {
		if (direction)
		{
			while (lesser (m_FileData[m_IndexMapping[l]].Name, ref, m_FileData[m_IndexMapping[l]].bIsDir, bRefIsDir) && (l<ende)) l++;
			while (fgreater(m_FileData[m_IndexMapping[r]].Name, ref, m_FileData[m_IndexMapping[r]].bIsDir, bRefIsDir) && (r>anf)) r--;
		}
		else
		{
			while (fgreater(m_FileData[m_IndexMapping[l]].Name, ref, m_FileData[m_IndexMapping[l]].bIsDir, bRefIsDir) && (l<ende)) l++;
			while (lesser (m_FileData[m_IndexMapping[r]].Name, ref, m_FileData[m_IndexMapping[r]].bIsDir, bRefIsDir) && (r>anf)) r--;
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
	UINT nID[5]={IDS_HEADER_FILENAME, IDS_HEADER_FILEACTION, IDS_HEADER_FILESIZE,IDS_HEADER_FILETYPE,IDS_HEADER_LASTMODIFIED};

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
	if (m_Columns[item]==3)
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
	else if (m_Columns[m_sortcolumn]==2)
	{
		BOOL dir=m_sortdir==1;
		quicksortbysize(dir, 1, GetItemCount()-1);
	}
	else if (m_Columns[m_sortcolumn]==4)
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
}

void CLocalFileListCtrl::OnLocalcontextOpen() 
{
	/*
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
	*/
}



void CLocalFileListCtrl::OnLocalcontextDelete() 
{
	/*
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

	SHFILEOPSTRUCT op;
	memset(&op,0,sizeof(op));
	op.hwnd=m_hWnd;
	op.wFunc=FO_DELETE;
	op.fFlags=(GetKeyState(VK_SHIFT)&128)?0:FOF_ALLOWUNDO;
	op.pFrom=buffer;
	SHFileOperation(&op);
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
	/*
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
	*/
}





void CLocalFileListCtrl::SetFolder(CString folder)
{

	using namespace std;


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


	if (m_Fullpath== m_rootPath)
	{
	//	m_Fullpath="";
		DisplayDrives();
		m_pOwner->m_Parent->pSplitterWnd4->SetPaneCaptionText(0, _T(" File View     --        My Computer"));
		return;
	}


	CString displayPath = m_Fullpath;
	//MessageBox(displayPath, m_rootPath);
	displayPath.Delete(0, m_rootPath.GetLength());
	displayPath.TrimRight(_T("\\"));

	CString tmpStr = displayPath;
	tmpStr.Delete(0,1);
	displayPath.Delete(1, displayPath.GetLength() -1);
	displayPath.MakeUpper();
	displayPath=displayPath+ L":"+tmpStr;

	if(displayPath.GetLength() == 2)
		displayPath = displayPath+L"\\";

	m_pOwner->m_Parent->pSplitterWnd4->SetPaneCaptionText(0, _T(" File View     --        ")+ displayPath);

	m_bUpdating=TRUE;

	SetRedraw(FALSE);
	EnableWindow(FALSE);
	
	
	CString   strPathFiles = m_Fullpath;
	CString  longPath;



	CString tempPath = m_Fullpath;

	longPath = tempPath;

	if (tempPath.Right(1) != "\\" ){
		tempPath += "\\";
			longPath += "\\";
	}
	tempPath += "*.*";


	WIN32_FIND_DATA find;
	HANDLE hFind = FindFirstFile(tempPath, &find);

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
			if (!FindNextFile(hFind, &find))
			{
				FindClose(hFind);
				hFind=0;

				break;
			}		
		}		
	
		m_bUpdating=FALSE;

	SetItemCount(nItemCount);
	SortList(m_sortcolumn,m_sortdir);
	EnableWindow(TRUE);	
	SetRedraw( TRUE );

	if (bDidHaveFocus)
		SetFocus();

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
			if (m_FileData[m_IndexMapping[pItem->iItem]].Name=="..")
				path="alkjhgfdfghjjhgfdghuztxvbhzt";
			else
			{
				if(1){	

					if (m_Fullpath==m_rootPath){
						path=m_FileData[m_IndexMapping[pItem->iItem]].Name+L":";
						path+=L"\\";	
					}
					else
					  //path=m_Fullpath+m_FileData[m_IndexMapping[pItem->iItem]].lName;
						path=m_FileData[m_IndexMapping[pItem->iItem]].lName;
				}
				else
					path=m_FileData[m_IndexMapping[pItem->iItem]].lName;
			}
		//	MessageBox(path);
			
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
		else if (m_Columns[pItem->iSubItem]==3 && (pItem->iItem||(m_Fullpath=="")))
		{
			CString type=GetType(m_FileData[m_IndexMapping[pItem->iItem]].lName, m_FileData[m_IndexMapping[pItem->iItem]].bIsDir);
			lstrcpy(pItem->pszText,type);
		}
		else if (m_Columns[pItem->iSubItem]==1){
			lstrcpy(pItem->pszText, m_FileData[m_IndexMapping[pItem->iItem]].Action);
		}
		else if (m_Columns[pItem->iSubItem]==2)
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
		else if (m_Columns[pItem->iSubItem]==4 && m_Fullpath!=_T("") && pItem->iItem && m_FileData[m_IndexMapping[pItem->iItem]].hasTime)
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
	//	m_Fullpath=_T("");
	//TCHAR  szDrives[128];
	//LPTSTR pDrive;
	
	CString tempPath = m_rootPath;

		
	m_FileData.clear();
	m_IndexMapping.clear();
	if (tempPath.Right(1) != "\\" ){
		tempPath += "\\";
	}
	tempPath += "*.*";

	WIN32_FIND_DATA find;
	HANDLE hFind = FindFirstFile(tempPath, &find);

	t_FileData FileData;

	
	int nItemCount= 0;

	while ( hFind!=INVALID_HANDLE_VALUE){
		if (!(find.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY && (!_tcscmp(find.cFileName, _T("..")) || !_tcscmp(find.cFileName, _T(".")))))
		{
	
			t_FileData FileData;
			FileData.Name= find.cFileName;
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

			if ( find.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY){
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
		if (!FindNextFile(hFind, &find))
		{
			FindClose(hFind);
			hFind=0;

			break;
		}		
	}		
	
	
	/*
	GetLogicalDriveStrings( sizeof(szDrives), szDrives );
	
	pDrive = szDrives;
	//int count=0;
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
		m_IndexMapping.push_back(nItemCount);
		pDrive += _tcslen(pDrive) + 1;
		nItemCount++;
	}
	*/
	
	SetItemCount(nItemCount);
	SortList(m_sortcolumn,m_sortdir);

		
	EnableWindow(TRUE);	
	SetRedraw( TRUE );


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



	return 0;
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



void CLocalFileListCtrl::OnItemchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	
	*pResult = 0;

	POSITION selpos=GetFirstSelectedItemPosition();
/*
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

  */
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

}

void CLocalFileListCtrl::GetSortInfo(int &column, int &direction) const
{
	column = m_sortcolumn;
	direction = m_sortdir;
}




void CLocalFileListCtrl::SetFolder1(CString folder)
{
	using namespace std;

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
	CString  longPath;
	int nItemCount= 0;



	queue <CString> q1;


	q1.push(strPathFiles);
	CString tempPath;


	while(!q1.empty()){

		tempPath = q1.front( );

		longPath = tempPath;

		if (tempPath.Right(1) != "\\" ){
			tempPath += "\\";
			longPath += "\\";
		}
		tempPath += "*.*";


		q1.pop();
		WIN32_FIND_DATA find;
		HANDLE hFind = FindFirstFile(tempPath, &find);

		CString folder2=m_Fullpath;
		folder2.TrimRight('\\');
	
		int pos=folder2.ReverseFind('\\');
		folder2=folder2.Left(pos+1);

		t_FileData FileData;

		while ( hFind!=INVALID_HANDLE_VALUE)
		{
			if (!(find.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY && (!_tcscmp(find.cFileName, _T("..")) || !_tcscmp(find.cFileName, _T(".")))))
			{
				if (find.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY){
					strPathFiles = longPath+find.cFileName;
					q1.push(strPathFiles);
				}
				else{
					t_FileData FileData;
					FileData.Name=longPath+find.cFileName;
					FileData.lName=/*longPath+*/find.cFileName;
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
			}
			if (!FindNextFile(hFind, &find))
			{
				FindClose(hFind);
				hFind=0;

				break;
			}		
		}		
	}
		m_bUpdating=FALSE;

	SetItemCount(nItemCount);
	SortList(m_sortcolumn,m_sortdir);
	EnableWindow(TRUE);	
	SetRedraw( TRUE );

	if (bDidHaveFocus)
		SetFocus();

}

void CLocalFileListCtrl::DisplayFiles(CString folder)
{

	folder.TrimRight( _T("\\") ); 
	folder+=_T("\\");
	m_rootPath = folder;
	m_Fullpath = folder;
	//MessageBox(folder);
	SetFolder(folder);
}

