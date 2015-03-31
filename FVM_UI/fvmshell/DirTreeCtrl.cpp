// DirTreeCtrl.cpp: 
// 
// wrapped CTreeCtrl to select and or display folders and files (optional )
// 

#include "stdafx.h"

#include "DirTreeCtrl.h"
#include "resource.h"
#include "mainfrm.h"


#include <vector>
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDirTreeCtrl

CDirTreeCtrl::CDirTreeCtrl()
{
	m_dont_notify = false;
	m_hDragSource = NULL;
	m_hDragHilited = NULL;
}

CDirTreeCtrl::~CDirTreeCtrl()
{
}


BEGIN_MESSAGE_MAP(CDirTreeCtrl, CTreeCtrl)
	//{{AFX_MSG_MAP(CDirTreeCtrl)
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDED, OnItemexpanded)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	ON_WM_VSCROLL()
	ON_WM_CREATE()
	ON_WM_KEYDOWN()
	ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, OnBeginlabeledit)
	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndlabeledit)
	ON_WM_CONTEXTMENU()
	//ON_COMMAND(ID_LOCALTREECONTEXT_ADDTOQUEUE, OnLocaltreecontextAddtoqueue)
	//ON_COMMAND(ID_LOCALTREECONTEXT_DELETE, OnLocaltreecontextDelete)
	ON_COMMAND(ID_DIRVIEW_RENAME, OnDirViewRename)
	//ON_COMMAND(ID_LOCALTREECONTEXT_UPLOAD, OnLocaltreecontextUpload)
	//ON_COMMAND(ID_LOCALTREECONTEXT_UPLOADAS, OnLocaltreecontextUploadas)
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnBegindrag)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CDirTreeCtrl 

BOOL CDirTreeCtrl::DisplayTree(LPCTSTR strRoot, BOOL bFiles)
{
	m_dont_notify=TRUE;
	DWORD dwStyle = GetStyle();   // read the windowstyle
	
	// Display the DirTree with the Rootname e.g. C:\
	// if Rootname == NULL then Display all Drives on this PC
    // First, we need the system-ImageList
	
	DeleteAllItems();

	LPITEMIDLIST list;
	SHGetSpecialFolderLocation(m_pOwner->m_hWnd,CSIDL_DRIVES,&list);
	SHFILEINFO shFinfo;

	SHGetFileInfo((LPCTSTR)list,0,&shFinfo,sizeof(shFinfo),SHGFI_PIDL|SHGFI_ICON|SHGFI_SMALLICON);	
	DestroyIcon(shFinfo.hIcon);
	int iIcon=shFinfo.iIcon;
	SHGetFileInfo((LPCTSTR)list,0,&shFinfo,sizeof(shFinfo),SHGFI_PIDL|SHGFI_ICON|SHGFI_SMALLICON|SHGFI_OPENICON|SHGFI_DISPLAYNAME);
	
	CoTaskMemFree(list);

	DestroyIcon(shFinfo.hIcon);
	
	HTREEITEM hParent=InsertItem(shFinfo.szDisplayName,iIcon,shFinfo.iIcon,TVI_ROOT);
	
	if ( !DisplayDrives() )
	{
		m_dont_notify=FALSE;
		return FALSE;
	}
	Expand(GetRootItem(), TVE_EXPAND);
	return TRUE;	
}
/////////////////////////////////////////////////
BOOL CDirTreeCtrl::GetSysImgList()
/////////////////////////////////////////////////
{
	CImageList sysImgList;
	SHFILEINFO shFinfo;
	
	sysImgList.Attach((HIMAGELIST)SHGetFileInfo( _T("C:\\"),
							  0,
							  &shFinfo,
							  sizeof( shFinfo ),
							  SHGFI_SYSICONINDEX |
							  SHGFI_SMALLICON ));
	
	SetImageList( &sysImgList, TVSIL_NORMAL );
	sysImgList.Detach();
	return TRUE;
}

BOOL CDirTreeCtrl::DisplayDrives()
{
	//
	// Displaying the Available Drives on this PC
	// This are the First Items in the TreeCtrl
	//
	//DeleteAllItems();
	TCHAR  szDrives[128];
	TCHAR* pDrive;

	if ( !GetLogicalDriveStrings( sizeof(szDrives)/sizeof(TCHAR), szDrives ) )
	{
		m_strError = "Error Getting Logical DriveStrings!";
		return FALSE;
	}

	pDrive = szDrives;
	while( *pDrive )
	{
		HTREEITEM hParent = AddItem( GetRootItem(), pDrive );
	//	if ( FindSubDir( pDrive ) )
			InsertItem( _T(""), 0, 0, hParent );
		pDrive += _tcslen( pDrive ) + 1;
	}


	return TRUE;

}

void ArrayQuickSort(std::vector<CString> &array, int l, int r);
void ArrayInsertionSort(std::vector<CString> &array);
void SortArray(std::vector<CString> &array)
{
	// Sorts the Array for n < 11 using insertion sort, else using Quicksort
	
	if (array.size() > 1) 
		if (array.size() < 11) ArrayInsertionSort(array);
		else ArrayQuickSort(array, 1, array.size()-1);
}

void ArrayQuickSort(std::vector<CString> &array, int l, int r)
{
	int i,j;
	int max = array.size();
	CString v;

	if (r > l) {
		v = array[r];
		i = l-1;
		j = r;
		for (;;) {
			while ((array[i].CollateNoCase( v ) < 0) && (i < max)) i++;
			while ((array[j].CollateNoCase( v ) > 0) && (j > 0)) j--;
			if (i >= j) break;
			v = array[i];
			array[i]=array[j];
			array[j]=v;
		}
		v = array[i];
		array[i]=array[r];
		array[r]=v;

		ArrayQuickSort(array, l, i-1);
		ArrayQuickSort(array, i+1, r);
	}
}

void ArrayInsertionSort(std::vector<CString> &array)
{
	int i,j;
	CString v;
	int n=array.size();

	for (i = 1; i < n-1; i++) {
		v = array[i];
		j = i;
		while ((j > 0) && (array[j-1].CollateNoCase( v ) > 0)) {
			array[j]=array[j-1];
			j--;
		}
		array[j]=v;
	}
}

void CDirTreeCtrl::DisplayPath(HTREEITEM hParent, LPCTSTR strPath)
{
	//
	// Displaying the Path in the TreeCtrl
	//
	CFileFind find;
	CString   strPathFiles = strPath;
	BOOL      bFind;
	std::vector<CString> strDirArray;
	
	if ( strPathFiles.Right(1) != "\\" )
		strPathFiles += "\\";
	strPathFiles += "*.*";

	bFind = find.FindFile( strPathFiles );

	while ( bFind )
	{
		bFind = find.FindNextFile();
		if ( find.IsDirectory() && !find.IsDots() )
		{		
			strDirArray.push_back( find.GetFilePath() );
		}
		
	}
    
	SortArray(strDirArray);
	SetRedraw( FALSE );
	CWaitCursor wait;
    
	for ( UINT i = 0; i < strDirArray.size(); i++ )
	{
		HTREEITEM hItem = AddItem( hParent, strDirArray[i] );
		if ( FindSubDir( strDirArray[i] ) )
			InsertItem( _T(""), 0, 0, hItem );
	}

    
	SetRedraw(TRUE);	
}

HTREEITEM CDirTreeCtrl::AddItem(HTREEITEM hParent, LPCTSTR strPath)
{
	// Adding the Item to the TreeCtrl with the current Icons
    CString strTemp = strPath;
    
	strTemp.TrimRight('\\');
	
	int iIcon, iIconSel;
	SHFILEINFO shFinfo;
	if ( !SHGetFileInfo( strPath,
				0,
				&shFinfo,
				sizeof( shFinfo ),
				SHGFI_ICON | 
				SHGFI_SMALLICON ) )
	{
		m_strError = "Error Gettting SystemFileInfo!";
		return 0;
	}
	
	iIcon = shFinfo.iIcon;
	
	// we only need the index from the system image list
	
	DestroyIcon( shFinfo.hIcon );
	
	if ( !SHGetFileInfo( strPath,
				0,
				&shFinfo,
				sizeof( shFinfo ),
				SHGFI_ICON | SHGFI_OPENICON |
			    SHGFI_SMALLICON ) )
	{
		m_strError = "Error Gettting SystemFileInfo!";
		return 0;
	}

	iIconSel = shFinfo.iIcon;
	
	// we only need the index of the system image list

	DestroyIcon( shFinfo.hIcon );
	HTREEITEM ret=InsertItem( GetSubPath( strTemp ), iIcon,iIconSel, hParent );
	
	return ret;
	
}

LPCTSTR CDirTreeCtrl::GetSubPath(LPCTSTR strPath)
{
	//
	// getting the last SubPath from a PathString
	// e.g. C:\temp\readme.txt
	// the result = readme.txt
	static CString strTemp;
	int     iPos;

	strTemp = strPath;
	strTemp.TrimRight('\\');
	iPos = strTemp.ReverseFind( '\\' );
	if ( iPos != -1 )
	    strTemp = strTemp.Mid( iPos + 1);

	return (LPCTSTR)strTemp;
}

BOOL CDirTreeCtrl::FindSubDir( LPCTSTR strPath)
{
	//
	// Are there subDirs ?
	//
	CFileFind find;
	CString   strTemp = strPath;
	BOOL      bFind;

	if ( strTemp[strTemp.GetLength()-1] == '\\' )
		strTemp += "*.*";
	else
		strTemp += "\\*.*";
		
	bFind = find.FindFile( strTemp );
	
	
	while ( bFind )
	{
		bFind = find.FindNextFile();

		if ( find.IsDirectory() && !find.IsDots() )
		{
			return TRUE;
		}
		
	}

	return FALSE;

}

void CDirTreeCtrl::OnItemexpanded(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	 
	ExpandItem( pNMTreeView->itemNew.hItem, TVE_EXPAND );

	*pResult = 0;
}

CString CDirTreeCtrl::GetFullPath(HTREEITEM hItem) const
{
	if (!hItem)
		return "";

	// get the Full Path of the item
	CString strReturn;
	CString strTemp;
	HTREEITEM hParent = hItem;

	strReturn = "";

	while ( GetParentItem(hParent) )
	{
		
		strTemp  = GetItemText( hParent );
		strTemp += "\\";
		strReturn = strTemp + strReturn;
		hParent = GetParentItem( hParent );
	}
    
	return strReturn;

}

BOOL CDirTreeCtrl::SetSelPath(CString strPath)
{
	m_dont_notify = true;
	if (strPath=="\\" || strPath=="")
	{
		SelectItem(GetRootItem());
		ExpandItem(GetRootItem(), TVE_EXPAND);
		HTREEITEM hFound = GetChildItem(GetRootItem());
		CString strTemp;
		while (hFound)
		{
			Expand(hFound, TVE_COLLAPSE);
			hFound = GetNextItem(hFound, TVGN_NEXT);
		}
		m_dont_notify = false;
		return TRUE;
	}
	// Setting the Selection in the Tree
	HTREEITEM hParent  = GetChildItem(TVI_ROOT);
	int       iLen    = strPath.GetLength() + 2;
	LPTSTR    pszPath = new TCHAR[iLen];
	LPTSTR    pPath   = pszPath;
	BOOL      bRet    = FALSE;
    
	if ( !IsValidPath( strPath ) )
	{
		m_dont_notify = FALSE;
		delete [] pszPath; // this must be added 29.03.99
		return FALSE;
	}

	_tcscpy( pszPath, strPath );
	_tcsupr( pszPath );
	
	if ( pszPath[_tcslen(pszPath)-1] != '\\' )
		_tcscat( pszPath, _T("\\") );
    
	int iLen2 = _tcslen( pszPath );
	
	for (WORD i = 0; i < iLen2; i++ )
	{
		if ( pszPath[i] == '\\' )
		{
			pszPath[i] = '\0';
			hParent = SearchSiblingItem( hParent, pPath );
			if ( !hParent )  // Not found!
				break;
			else
			{				
				HTREEITEM hFound = GetChildItem(hParent);
				CString   strTemp;
				while (hFound)
				{
					Expand(hFound,TVE_COLLAPSE);
					hFound = GetNextItem( hFound, TVGN_NEXT );
				}

				Expand( hParent, TVE_EXPAND );
			}
			pPath += _tcslen(pPath) + 1;
		}
	}
	
	delete [] pszPath;
	
	if (hParent) // Ok the last subpath was found
	{		
		Select(hParent, TVGN_FIRSTVISIBLE); // select the last expanded item
		SelectItem(hParent);

		bRet = TRUE;
	}
	else
	{
		bRet = FALSE;
	}
	SetRedraw( TRUE );
	

	m_dont_notify = false;
    return bRet;
}

HTREEITEM CDirTreeCtrl::SearchSiblingItem( HTREEITEM hItem, LPCTSTR strText)
{
	HTREEITEM hFound = GetChildItem( hItem );
	CString   strTemp;
	while ( hFound )
	{
		strTemp = GetItemText( hFound );
        strTemp.MakeUpper();
		if ( strTemp == strText )
			return hFound;
		hFound = GetNextItem( hFound, TVGN_NEXT );
	}

	return NULL;
}


void CDirTreeCtrl::ExpandItem(HTREEITEM hItem, UINT nCode)
{	
	if (!GetParentItem(hItem))
		return;
	if ( nCode == TVE_EXPAND )
	{
		TVITEM item;
		item.hItem=hItem;
		item.mask=TVIF_HANDLE|TVIF_STATE;
		GetItem(&item);

		if (item.state&TVIS_EXPANDEDONCE)
		{
			return;
		}
		HTREEITEM hChild = GetChildItem( hItem );
		while ( hChild )
		{
			DeleteItem( hChild );
			hChild = GetChildItem( hItem );
		}
        
		CString strPath = GetFullPath( hItem );
		DisplayPath( hItem, strPath );
	}
}

BOOL CDirTreeCtrl::IsValidPath(LPCTSTR strPath)
{
	// This function check the Pathname
	
	HTREEITEM hChild;
	CString   strItem;
	CString   strTempPath = strPath;
	BOOL      bFound = FALSE;
	CFileFind find;

	hChild = GetChildItem( GetRootItem() );
	strTempPath.MakeUpper();
	strTempPath.TrimRight('\\');

/*	while ( hChild )
	{
		strItem = GetItemText( hChild );
		strItem.MakeUpper();
		if ( strItem == strTempPath.Mid( 0, strItem.GetLength() ) )
		{
			bFound = TRUE;
			break;
		}
		hChild = GetNextItem( hChild, TVGN_NEXT );
	}
    
	if ( !bFound )
		return FALSE;*/

	strTempPath += "\\nul";
	if ( find.FindFile( strTempPath ) )
		return TRUE;
     
	return FALSE;
}

void CDirTreeCtrl::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	if (!m_dont_notify)
	{
		m_pOwner->SetLocalFolderOut(GetFullPath(pNMTreeView->itemNew.hItem));
	
	}
	*pResult = 0;
}

void CDirTreeCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CTreeCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
}

int CDirTreeCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CTreeCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	GetSysImgList()	;
		
	return 0;
}

void CDirTreeCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar==VK_F2)
	{
		// To Use F2 as hot Key to get EditCtrl on the ListView it must have 
		// the Style TVS_EDITLABELS
		ASSERT( GetStyle() & TVS_EDITLABELS );
		// don't do an Edit Label when the multiple Items are selected
		//Don't allow to rename "My Computer"
		if (GetRootItem()==GetSelectedItem())
			return;
		//Don't allow to rename the drives
		if (GetRootItem()==GetParentItem(GetSelectedItem()))
			return;
		VERIFY( EditLabel( GetSelectedItem() ) != NULL );
	}

	CTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

#define VK_A		65
BOOL CDirTreeCtrl::PreTranslateMessage(MSG* pMsg) 
{
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
			if( pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_CONTROL || pMsg->wParam == VK_INSERT || pMsg->wParam == VK_SHIFT )
			{
				edit->SendMessage(WM_KEYDOWN, pMsg->wParam, pMsg->lParam);
				return TRUE;
			}
		}
	}
	if (pMsg->message == WM_RBUTTONDOWN)
	{
		if (pMsg->hwnd!=GetSafeHwnd())
			return FALSE;
		/*CEdit* edit = GetEditControl();
		if (edit)
			return FALSE;*/
		HTREEITEM olditem=GetSelectedItem();
		CPoint point=pMsg->pt;
		ScreenToClient(&point);
		HTREEITEM item=HitTest(point,0);
		if (item)
		{
			if (item!=olditem)
			{
				SelectItem(item);
				ExpandItem(item,TVE_EXPAND);
				m_pOwner->SetLocalFolderOut(GetFullPath(item));				
				SetFocus();
			}
			
		}
		return TRUE;
	}
	return CTreeCtrl::PreTranslateMessage(pMsg);
}

void CDirTreeCtrl::OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	
	GetEditControl()->LimitText( 255 );
	
	*pResult = 0;
}

void CDirTreeCtrl::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	if (pTVDispInfo->item.pszText)
	{
		CString newname=pTVDispInfo->item.pszText;
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
		if (newname=="")
		{
			*pResult=FALSE;
			return;
		}
		SHFILEOPSTRUCT op;
		memset(&op,0,sizeof(op));
		CString from=GetFullPath(pTVDispInfo->item.hItem);
		from.TrimRight(_T("\\"));
		op.pFrom=from;
		CString to=from.Left(from.ReverseFind('\\')+1)+pTVDispInfo->item.pszText;
		op.pTo=to;
		op.hwnd=AfxGetMainWnd()->m_hWnd;
		op.wFunc=FO_RENAME;
		op.fFlags=FOF_ALLOWUNDO;
		if (!SHFileOperation(&op))
		{
			*pResult = TRUE;
			to.TrimRight(_T("\\"));
			to+=_T("\\");
			m_pOwner->SetLocalFolderOut(to);			
			return;
		}
	}
	
	*pResult = FALSE;	
	
}

void CDirTreeCtrl::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	
	if (!GetCount())
		return;
	CMenu menu;
	menu.LoadMenu(IDR_DIRVIEW);

	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);
	CWnd* pWndPopupOwner = this;
	//while (pWndPopupOwner->GetStyle() & WS_CHILD)
	//	pWndPopupOwner = pWndPopupOwner->GetParent();

	HTREEITEM hTreeItem=GetSelectedItem();
	
	if (!hTreeItem)
	{
		pPopup->EnableMenuItem(ID_DIRVIEW_DELETE,MF_GRAYED);	
		pPopup->EnableMenuItem(ID_DIRVIEW_RENAME,MF_GRAYED);	
	}
	else
	{
		if (point.x==-1 || point.y==-1)
		{
			CRect rect;
			GetItemRect(hTreeItem,&rect,TRUE);
			point.x=rect.left+5;
			point.y=rect.top+5;
			ClientToScreen(&point);
		}
	
		if (!GetParentItem(hTreeItem) || !GetParentItem(GetParentItem(hTreeItem)))
		{
			pPopup->EnableMenuItem(ID_DIRVIEW_DELETE, MF_GRAYED);	
			pPopup->EnableMenuItem(ID_DIRVIEW_RENAME, MF_GRAYED);	
		}
	}
	
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,pWndPopupOwner);
	
}

void CDirTreeCtrl::OnLocaltreecontextAddtoqueue() 
{
	HTREEITEM item;
	if (m_hDragSource)
		item = m_hDragSource;
	else
		item = GetSelectedItem();

	if (!item)
		return;
	if (!GetParentItem(item))
		return;
	CString path = GetFullPath(item);
	CMainFrame *pMainFrame=DYNAMIC_DOWNCAST(CMainFrame,GetParentFrame());
	
	path.TrimRight(_T("\\"));
	
	UploadDir(path+"\\*.*",GetItemText(item)+"\\",FALSE);
}

void CDirTreeCtrl::OnLocaltreecontextDelete() 
{
	HTREEITEM item = GetSelectedItem();	

	if (!item)
		return;
	if (!GetParentItem(item) || !GetParentItem(GetParentItem(item)))
		return;
	CString path=GetFullPath(item);	
	path.TrimRight(_T("\\"));
	HTREEITEM parentitem=GetParentItem(item);

	
	LPTSTR buffer=new TCHAR[path.GetLength()+2];
	memset(buffer,0,path.GetLength()*sizeof(TCHAR)+2*sizeof(TCHAR));
	_tcscpy(buffer,path);
	
	SHFILEOPSTRUCT op;
	memset(&op,0,sizeof(op));
	op.hwnd=m_hWnd;
	op.wFunc=FO_DELETE;
	op.fFlags=(GetKeyState(VK_SHIFT)&128)?0:FOF_ALLOWUNDO;
	op.pFrom=buffer;
	if (!SHFileOperation(&op) && !op.fAnyOperationsAborted)
	{
		SelectItem(parentitem);
		DeleteItem(item);
	}
	SetFocus();
	delete buffer;
}

void CDirTreeCtrl::OnDirViewRename() 
{
	HTREEITEM item=GetSelectedItem();
	if (!item)
		return;
	if (!GetParentItem(item) || !GetParentItem(GetParentItem(item)))
		return;
	EditLabel(item);
	
}

void CDirTreeCtrl::OnLocaltreecontextUpload() 
{
	/*
	HTREEITEM item;
	if (m_hDragSource)
		item = m_hDragSource;
	else
		item = GetSelectedItem();

	if (!item)
		return;
	if (!GetParentItem(item))
		return;
	CString path=GetFullPath(item);
	CMainFrame *pMainFrame=DYNAMIC_DOWNCAST(CMainFrame,GetParentFrame());
	
	path.TrimRight(_T("\\"));
	UploadDir(path+"\\*.*",GetItemText(item)+"\\",TRUE);
	pMainFrame->TransferQueue(2);
	*/
}

void CDirTreeCtrl::OnLocaltreecontextUploadas() 
{
/*
	CTransferAsDlg dlg;
	if (dlg.DoModal()==IDOK)
	{
		m_transferuser=dlg.m_User;
		m_transferpass=dlg.m_Pass;
		if (dlg.m_bTransferNow)
			OnLocaltreecontextUpload();
		else
			OnLocaltreecontextAddtoqueue();
	}
	m_transferuser="";
	m_transferpass="";
	*/
}

void CDirTreeCtrl::UploadDir(CString dir, CString subdir, BOOL upload)
{
/*
	while (dir.Replace(_T("\\\\"), _T("\\")));
	CMainFrame *pMainFrame=DYNAMIC_DOWNCAST(CMainFrame,GetParentFrame());
	CFileFind find;
	BOOL res=find.FindFile(dir);
	while (res)
	{
		BOOL bDirIsEmpty = TRUE;
		res=find.FindNextFile();
		if (find.IsDirectory())
		{
			if (!find.IsDots())
			{
				bDirIsEmpty = FALSE;
				UploadDir(find.GetFilePath()+"\\*.*",subdir+find.GetFileName()+"\\",upload);
			}
		}
		else
		{
			bDirIsEmpty = FALSE;

			CString sdir="";
			CString path=subdir;
			path.TrimLeft(_T("\\"));
			path.TrimRight(_T("\\"));
			while (path!="")
			{
				CString tmp;
				int pos=path.Find(_T("\\"));
				if (pos==-1)
				{
					tmp.Format(_T(" %d %s"),path.GetLength(),path);
					path="";
				}
				else
				{
					tmp.Format(_T(" %d %s"),path.Left(pos).GetLength(),path.Left(pos));
					path=path.Mid(pos+1);
				}
				sdir+=tmp;
			}
			sdir.TrimLeft(_T(" "));
			
			CString lPath=find.GetFilePath();
			int pos=lPath.ReverseFind('\\');
			ASSERT(pos!=-1);
			lPath=lPath.Left(pos+1);

			pMainFrame->AddQueueItem(FALSE,find.GetFileName(),sdir,lPath,CServerPath(),upload,m_transferuser,m_transferpass);
		}
		if (bDirIsEmpty)
		{
			CServerPath serverpath = reinterpret_cast<CFtpListCtrl *>(pMainFrame->GetFtpPane()->GetListCtrl())->GetCurrentDirectory();
			if (!serverpath.IsEmpty())
			{
				CString sdir;
				CString path=subdir;
				path.TrimLeft( _T("\\") );
				path.TrimRight(_T("\\") );
				while (path!=_T("") )
				{
					CString tmp;
					int pos=path.Find( _T("\\") );
					if (pos==-1)
					{
						tmp.Format(_T(" %d %s"), path.GetLength(), path);
						path="";
					}
					else
					{
						tmp.Format(_T(" %d %s"), path.Left(pos).GetLength(), path.Left(pos));
						path=path.Mid(pos+1);
					}
					sdir+=tmp;
				}
				sdir.TrimLeft( _T(" ") );
				serverpath.AddSubdirs(sdir);
				pMainFrame->m_pCommandQueue->MakeDir(serverpath);
			}
		}
	}
	*/
}

void CDirTreeCtrl::OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	/*
	*pResult = 0;

	NM_TREEVIEW *pNMTreeView = (NM_TREEVIEW *)pNMHDR;

	CMainFrame *pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetParentFrame());

	HTREEITEM hItem = pNMTreeView->itemNew.hItem;
	if (!hItem)
		return;

	CString path = GetFullPath(hItem);
	if (path == "" || path == "\\")
		return;

	EnsureVisible(hItem);

	m_hDragSource = hItem;
	
	//Let the main window handle the rest
	pMainFrame->OnBegindrag(this, pNMTreeView->ptDrag);
	*/
}

BOOL CDirTreeCtrl::DragPosition(CImageList *pImageList, CWnd* pDragWnd, CPoint point)
{
	ScreenToClient(&point);
	HTREEITEM hItem = HitTest(point);

	if (pDragWnd == this && hItem)
	{
		CString dragPath = GetFullPath(m_hDragSource);
		CString path = GetFullPath(hItem);
		if (dragPath.GetLength() < path.GetLength() &&
			path.Left(dragPath.GetLength()) == dragPath)
		{
			DragLeave(pImageList);
			return FALSE;
		}
	}
	if (hItem)
	{
		if (hItem != m_hDragHilited)
		{
			pImageList->DragShowNolock(false);
			if (m_hDragHilited)
			{
				SetItemState(m_hDragHilited, 0, LVIS_DROPHILITED);
			}
			m_hDragHilited = hItem;
			SetItemState(hItem, TVIS_DROPHILITED, TVIS_DROPHILITED);
			UpdateWindow();
			pImageList->DragShowNolock(true);
		}
	}
	else
		DragLeave(pImageList);

	return TRUE;
}

void CDirTreeCtrl::DragLeave(CImageList *pImageList)
{
	if (!m_hDragHilited)
		return;

	if (pImageList)
		pImageList->DragShowNolock(false);
	SetItemState(m_hDragHilited, 0, TVIS_DROPHILITED);
	UpdateWindow();
	if (pImageList)
		pImageList->DragShowNolock(true);
	m_hDragHilited = NULL;
}

void CDirTreeCtrl::OnDragEnd(int target, CPoint point)
{
	/*
	m_transferuser = m_transferpass = "";
	if (target == 1)
		OnLocaltreecontextAddtoqueue();
	else if (!target)
		OnLocaltreecontextUpload();
	else if (target == 2 || target == 3)
	{
		CString to;
		if (target == 2)
		{
			if (!m_hDragHilited)
				return;
			if (m_hDragSource == m_hDragHilited)
				return;
			to = GetFullPath(m_hDragHilited);
		}
		else
		{
			CMainFrame *pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetParentFrame());
			to = reinterpret_cast<CLocalFileListCtrl *>(pMainFrame->GetLocalPane2()->GetListCtrl())->GetDropTarget();
			if (to == "" || to == "\\")
				return;
		}


		CString from = GetFullPath(m_hDragSource);
		if (from.GetLength() < to.GetLength() &&
			to.Left(from.GetLength()) == from)
			return;

		from.TrimRight('\\');
		to.TrimRight('\\');
		
		if (to == "")
			return;

		LPTSTR pFrom = new TCHAR[from.GetLength() + 2];
		_tcscpy(pFrom, from);
		pFrom[from.GetLength() + 1] = 0;
		LPTSTR pTo = new TCHAR[to.GetLength() + 2];
		_tcscpy(pTo, to);
		pTo[to.GetLength() + 1] = 0;
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
		delete [] pTo;

		RefreshDir(GetParentItem(m_hDragSource));
		if (target == 2)
			RefreshDir(m_hDragHilited);
		else
			RefreshDir(to);

		CMainFrame *pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetParentFrame());
		CLocalFileListCtrl *pList = reinterpret_cast<CLocalFileListCtrl *>(pMainFrame->GetLocalPane2()->GetListCtrl());
		if (pList->GetFolder() == (to + "\\"))
			pList->SetFolder(to);

		int pos = from.ReverseFind('\\');
		if (pos < 2)
			return;
		from = from.Left(pos + 1);
		if (pList->GetFolder() == from)
			pList->SetFolder(from);
	}
	*/
}

CString CDirTreeCtrl::GetDropTarget() const
{
	return GetFullPath(m_hDragHilited);
}

// Struct for children list
// Unfortunately declared outside scope of RefreshDir due to linkage problems 
// (at least in VC++ 6)
struct t_children
{
	HTREEITEM hItem;
	CString text;
};

void CDirTreeCtrl::RefreshDir(HTREEITEM hRefreshItem)
{
	if (!hRefreshItem)
		return;

	CString path = GetFullPath(hRefreshItem);

	//
	// Displaying the Path in the TreeCtrl
	//
	CFileFind find;
	CString   strPathFiles = path;
	BOOL      bFind;
	std::vector<CString> strDirArray;
	
	if ( strPathFiles.Right(1) != "\\" )
		strPathFiles += "\\";
	strPathFiles += "*.*";

	bFind = find.FindFile( strPathFiles );

	while ( bFind )
	{
		bFind = find.FindNextFile();
		if ( find.IsDirectory() && !find.IsDots() )
		{		
			strDirArray.push_back( find.GetFilePath() );
		}
	}

	// Get children list which is already sorted
	std::vector<t_children> childrenArray;
	HTREEITEM hChildItem = GetChildItem(hRefreshItem);
	while (hChildItem)
	{
		t_children child;
		child.hItem = hChildItem;
		child.text = GetItemText(hChildItem);
		childrenArray.push_back(child);
		hChildItem = GetNextSiblingItem(hChildItem);
	}

	SortArray(strDirArray);
	
	SetRedraw(FALSE);
	CWaitCursor wait;
    
	// Search for new/deleted entries
	int j = 0;
	for (UINT i = 0; i < strDirArray.size(); i++)
	{
		CString subPath = GetSubPath(strDirArray[i]);
		int comp;
		if (j < childrenArray.size())
			comp = subPath.CollateNoCase(childrenArray[j].text);
		else
			comp = -1;
		if (!comp)
			j++;
		else if (comp < 0)
		{
			HTREEITEM hItem = AddItem( hRefreshItem, strDirArray[i] );
			if (FindSubDir(strDirArray[i]))
				InsertItem(_T(""), 0, 0, hItem);
		}
		else
		{
			i--;

			DeleteItem(childrenArray[j].hItem);
			j++;
		}
	}
	SortChildren(hRefreshItem);
    
	SetRedraw(TRUE);

}

void CDirTreeCtrl::RefreshDir(CString dir)
{
	if (dir == "")
		return;

	if (dir.Right(1) != _T("\\"))
		dir += "\\";
	
	HTREEITEM hParent = GetRootItem();
	while (dir != _T(""))
	{
		int pos = dir.Find(_T("\\"));
		if (pos == -1)
			return;

		CString sub = dir.Left(pos);
		dir = dir.Mid(pos + 1);

		HTREEITEM hItem = GetChildItem(hParent);
		while (hItem)
		{
			if (!GetItemText(hItem).CollateNoCase(sub))
			{
				hParent = hItem;
				break;
			}
			hItem = GetNextSiblingItem(hItem);
		}
		if (!hItem)
			return;
	}
	if (hParent != GetRootItem())
		RefreshDir(hParent);
}
