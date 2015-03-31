// ShortCutView.cpp : implementation file
//

#include "stdafx.h"
#include "fvmshell.h"
#include "ShortCutView.h"
#include "CreateShortcut.h"
#include "MainFrm.h"
#include "LocalFileListCtrl.h"

#include <string.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CShortCutView

IMPLEMENT_DYNCREATE(CShortCutView, CListView)

CShortCutView::CShortCutView()
{
}

CShortCutView::~CShortCutView()
{
}


BEGIN_MESSAGE_MAP(CShortCutView, CListView)
	//{{AFX_MSG_MAP(CShortCutView)
		ON_WM_CONTEXTMENU()
		ON_COMMAND(ID_SHORTCUT_CREATESHORTCUT, OnCreateShortcut)
		ON_COMMAND(ID_SHORTCUT_DELETESHORTCUT, OnDeleteShortcut)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetdispinfo)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CShortCutView drawing

void CShortCutView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CShortCutView diagnostics

#ifdef _DEBUG
void CShortCutView::AssertValid() const
{
	CListView::AssertValid();
}

void CShortCutView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CShortCutView message handlers

void CShortCutView::OnContextMenu(CWnd* pWnd, CPoint point)
{


	
	CMenu menu;
	menu.LoadMenu(IDR_SHORTCUT);

	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);
	CWnd* pWndPopupOwner = this;
	//while (pWndPopupOwner->GetStyle() & WS_CHILD)
	//	pWndPopupOwner = pWndPopupOwner->GetParent();


	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,
		pWndPopupOwner);
}

/*
void CShortCutView::OnRButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
		
	CMenu menu;
	menu.LoadMenu(IDR_FILEVIEW);

	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);
	CWnd* pWndPopupOwner = this;
	//while (pWndPopupOwner->GetStyle() & WS_CHILD)
	//	pWndPopupOwner = pWndPopupOwner->GetParent();


	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,
		pWndPopupOwner);	CListView::OnRButtonUp(nFlags, point);
}
*/

void CShortCutView::OnCreateShortcut()
{
	CCreateShortcut shortcut(this);
	shortcut.DoModal();


}


void CShortCutView::OnDeleteShortcut()
{
	POSITION pos = m_listCtrl->GetFirstSelectedItemPosition();
	if (pos == NULL)
		TRACE0("No items were selected!\n");
	else
	{
		while (pos)
		{
			int nItem = m_listCtrl->GetNextSelectedItem(pos);
			m_listCtrl->DeleteItem(nItem);
		}
	}


}

BOOL CShortCutView::GetSysImgList()
/////////////////////////////////////////////////
{
	CImageList sysImgList;
	SHFILEINFO shFinfo;
	
	sysImgList.Attach((HIMAGELIST)SHGetFileInfo( _T("C:\\"),
							  0,
							  &shFinfo,
							  sizeof( shFinfo ),
							  SHGFI_SYSICONINDEX | SHGFI_ICON));
							  //((m_nStyle==LVS_ICON)?SHGFI_ICON:SHGFI_SMALLICON) ));
	CListCtrl& lc = GetListCtrl();
	m_listCtrl = &lc;
	lc.SetImageList( &sysImgList, LVSIL_NORMAL);//(m_nStyle==LVS_ICON)?LVSIL_NORMAL:LVSIL_SMALL);

	lc.ModifyStyle(0,LVS_ICON|LVS_SINGLESEL ); 
	lc.SetExtendedStyle(LVS_EX_TRACKSELECT | LVS_EX_ONECLICKACTIVATE);

	
	lc.SetTextColor(RGB(255,255,255));
	lc.SetTextBkColor(RGB(113,116,95));
	lc.SetBkColor(RGB(113,116,95));	

	/*
	lc.InsertItem(0,_T("Virtual Machine long long long long long1"),0);
	lc.InsertItem(1,_T("Virtual Machine 1"),1);

	lc.InsertItem(2,_T("Virtual Machine 1"),2);
	lc.InsertItem(3,_T("Virtual Machine 1"),3);

	*/
	sysImgList.Detach();
	return TRUE;
}


//#define MAX_KEY_LENGTH 255
//#define MAX_VALUE_NAME 16383

#define MAX_KEY_LENGTH 1024
#define MAX_VALUE_NAME 1024

void CShortCutView::OnInitialUpdate() 
{
	CListView::OnInitialUpdate();
	GetSysImgList();
	// TODO: Add your specialized code here and/or call the base class

	HKEY key;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\fvmshell\\shortcuts"),0,	KEY_QUERY_VALUE ,&key)==ERROR_SUCCESS)
	{

	 
	//	CHAR     achClass[MAX_PATH] = "";  // buffer for class name 
		DWORD    cchClassName = MAX_PATH;  // length of class string 
		DWORD    cValues;              // number of values for key 
		
		DWORD j; 
		DWORD retValue; 
 
		TCHAR  fileName[MAX_VALUE_NAME]; 
		DWORD cchValue = MAX_VALUE_NAME; 
	
		// Get the class name and the value count. 
		 RegQueryInfoKey(key,        // key handle 
			NULL,                // buffer for class name 
			NULL,           // length of class string 
			NULL,                    // reserved 
		    NULL,             // number of subkeys 
			NULL,          // longest subkey size 
			NULL,           // longest class string 
			&cValues,                // number of values for this key 
			NULL,                    // longest value name 
			NULL,
			NULL,					 // security descriptor 
			NULL);       // last write time 

	
		for (j = 0;j <= cValues; j++) 
        { 
          
            fileName[0] = L'\0';
			DWORD cchValue = MAX_VALUE_NAME; 
            retValue = RegEnumValue(
				key, 
				j, 
				fileName, 
                &cchValue, 
                NULL, 
                NULL,    // &dwType, 
                NULL,    // &bData, 
                NULL);   // &bcData 
		
 
			if(retValue != ERROR_SUCCESS){
				continue;	
			}
			if(retValue == ERROR_NO_MORE_ITEMS)
				break;

			LPCWSTR ptr;
	
			int iIcon;
			SHFILEINFO shFinfo;
			if (SHGetFileInfo(fileName,	0, &shFinfo, sizeof( shFinfo ), SHGFI_ICON | 
					SHGFI_SMALLICON ) )
			{
					//m_strError = "Error Gettting SystemFileInfo!";
			
			
	  
				ptr = wcsrchr(fileName, '\\');
				if(ptr == NULL)
					ptr = fileName;
				else
					ptr++;
	  
				iIcon = shFinfo.iIcon;
				int item =m_listCtrl->InsertItem(0,ptr,iIcon);
				int in = shortCuts.Add(fileName);
				m_listCtrl->SetItemData(item, in);
			}
        } 

	}
}

LPWSTR CShortCutView::PathFindExtensionW( LPCWSTR lpszPath )
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

LPWSTR CShortCutView::PathGetExtensionW(LPCWSTR lpszPath)
{
//	TRACE("(%s)\n",debugstr_w(lpszPath));

	lpszPath = PathFindExtensionW(lpszPath);
	return (LPWSTR)(*lpszPath?(lpszPath+1):lpszPath);
}

BOOL CShortCutView::FvmPathIsExeW (LPCWSTR lpszPath)
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


void CShortCutView::OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	// TODO: Add your control notification handler code here
	MessageBox(L"hello");
	*pResult = 0;
}

void CShortCutView::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	POSITION pos = m_listCtrl->GetFirstSelectedItemPosition();
	if (pos == NULL)
		TRACE0("No items were selected!\n");
	else
	{
		while (pos)
		{
			int nItem = m_listCtrl->GetNextSelectedItem(pos);
			int in = m_listCtrl->GetItemData(nItem);
			m_pOwner->GetFileView()->m_pListCtrl->openFiles(shortCuts[in]);
		}
	}
	*pResult = 0;
}

void CShortCutView::OnDestroy() 
{
	HKEY key;

	int i, si;

	DWORD type;
	DWORD value;
	DWORD bytes;

	if (RegCreateKey(HKEY_CURRENT_USER, _T("Software\\fvmshell") ,&key)==ERROR_SUCCESS)
	{
	
		RegDeleteKey(key, _T("shortcuts"));
		RegCloseKey(key);
		if (RegCreateKey(HKEY_CURRENT_USER, _T("Software\\fvmshell\\shortcuts") ,&key)==ERROR_SUCCESS)
		{
			type = REG_DWORD;
			bytes = sizeof(DWORD);
			value =	0;

	
			for (i=0;i < m_listCtrl->GetItemCount();i++)
			{
				si = m_listCtrl->GetItemData(i);
				RegSetValueEx(key, shortCuts[si], NULL, type, (LPBYTE)&value, bytes);
			}		
			RegCloseKey(key);
		}
	}
	


	


	CListView::OnDestroy();


	// TODO: Add your message handler code here
	
}
