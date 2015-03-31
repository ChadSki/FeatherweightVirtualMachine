// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "fvmshell.h"
#include "MFC64bitFix.h"
#include "MainFrm.h"
#include "misc/TransparentDialogBar.h"
#include <wininet.h>
#include "ShortCutView.h"
#include "LocalFileListCtrl.h"
#include "Setting.h"
#include "DirTreeCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame
#define MY_ID AFX_IDW_TOOLBAR+1
IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_GO_BACK, OnGoBack)
	ON_UPDATE_COMMAND_UI(ID_GO_BACK, OnUpdateGoBack)
	ON_COMMAND(ID_GO_FORWARD, OnGoForward)
	ON_UPDATE_COMMAND_UI(ID_GO_FORWARD, OnUpdateGoForward)
	ON_COMMAND(ID_VIEW_UP, OnUp)
	ON_COMMAND(ID_VIEW_FOLDER, OnFolder)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FOLDER, OnUpdateFolder)
	ON_COMMAND(ID_VIEW_SHORTCUT, OnShortcut)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHORTCUT, OnUpdateShortcut)
	ON_COMMAND(ID_VIEW_RUN, OnRun)
	ON_COMMAND(ID_VIEW_VIEW, OnView)
	ON_COMMAND(ID_GO_SEARCH_THE_WEB, OnGoSearchTheWeb)
	ON_COMMAND(IDOK, OnNewAddressEnter)
	ON_COMMAND(ID_MENU_LARGE_ICONS, OnLargeIcons)
	ON_UPDATE_COMMAND_UI(ID_MENU_LARGE_ICONS, OnUpdateLargeIcons)
	ON_COMMAND(ID_MENU_SMALL_ICONS, OnSmallIcons)
	ON_UPDATE_COMMAND_UI(ID_MENU_SMALL_ICONS, OnUpdateSmallIcons)
	ON_COMMAND(ID_MENU_LIST, OnList)
	ON_UPDATE_COMMAND_UI(ID_MENU_LIST, OnUpdateList)
	ON_COMMAND(ID_MENU_DETAILS, OnDetails)
	ON_UPDATE_COMMAND_UI(ID_MENU_DETAILS, OnUpdateDetails)
	ON_NOTIFY(TBN_DROPDOWN, AFX_IDW_TOOLBAR, OnViewDropDown)
	ON_COMMAND(ID_VIEW_DELETE, OnDelete)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DELETE, OnUpdateDelete)
	ON_COMMAND(ID_FOLDER_OPTIONS, OnSetting)
	ON_CBN_SELENDOK(MY_ID,OnNewAddress)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	m_acIndex = -1;
	m_maxIndex = -1;
	m_nLocalListViewStyle = 0;
	m_FolderIsHiden = 1;
	m_ShortcutIsHiden = 0;
	m_displayDelete = 0;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	LPTSTR sp, sp2;
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;


		
	if (!CreateToolbars())
		return -1;

	EnableDocking(CBRS_ALIGN_ANY);
	//DockControlBar(&m_wndToolBar);

	CString folder("\\");  //COptions::GetOption(OPTION_DEFAULTFOLDER);


	SetWindowPos(GetParent(),60,60,750,500,SWP_SHOWWINDOW);
	SetLocalFolder(folder);
	

	sp = GetCommandLine();
	sp2 = wcschr(sp, L'-');
	if(sp2)
		m_vmName = ++sp2;
	else
		m_vmName = L"fvmshell";

	//MessageBox(m_vmName);
	SetWindowText(m_vmName+L" - fvmshell");
	return 0;
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/,
	CCreateContext* pContext)
{


	if (!m_wndSplitter.Create(this, VSPLITTER | FIXED0 | VIEW3D ))
		return FALSE;

	m_wndSplitter.CreateView(RUNTIME_CLASS(CShortCutView), 1, pContext);
	m_wndSplitter.CreateView(RUNTIME_CLASS(CBiSplitterWnd), 2, VSPLITTER | FIXED0 | VIEW3D);

	if(!m_wndSplitter.CreatePane(0, THIN_BORDER | THIN_CAPTIONBORDER | FLAT_BUTTON | GRIPBTN|SMCAPTION|CLOSEBTN , _T("Shortcuts")
			, m_wndSplitter.GetView(1))
		||
		!m_wndSplitter.CreatePane(1, FLAT_BUTTON , NULL, m_wndSplitter.GetView(2))	
	  )
	{
		m_wndSplitter.DestroyWindow();
		return FALSE;
	}

	pSplitterWnd2 = (CBiSplitterWnd *)m_wndSplitter.GetPaneView(1);
	pSplitterWnd2->CreateView(RUNTIME_CLASS(CFvmDirView), 11, pContext);
	pSplitterWnd2->CreateView(RUNTIME_CLASS(CFvmFileView), 12, pContext);

	if(!pSplitterWnd2->CreatePane(0, THIN_BORDER | THIN_CAPTIONBORDER | FLAT_BUTTON | GRIPBTN |SMCAPTION| CLOSEBTN , _T("Folders")
			, pSplitterWnd2->GetView(11))
		||
		!pSplitterWnd2->CreatePane(1, THIN_BORDER| THIN_CAPTIONBORDER | FLAT_BUTTON | GRIPBTN , NULL, pSplitterWnd2->GetView(12))	
	  )
	{
		pSplitterWnd2->DestroyWindow();
		return FALSE;
	}

	m_wndSplitter.SetSplitterPos(200);
	m_wndSplitter.SetSplitterGap(6);
	pSplitterWnd2->SetSplitterPos(200);
	pSplitterWnd2->SetSplitterGap(6);



	GetDirView()->m_pOwner=this;
	GetFileView()->m_pOwner=this;
	GetShortcutView()->m_pOwner = this;
	pSplitterWnd2->HidePane(BSW_FIRST_PANE);

	GetDefault();
	return TRUE;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::SetLocalFolder(CString folder, int setaddress, int addCache)
{
	folder.TrimRight( _T("\\") );
	CString folder2=folder;
	folder+=_T("\\");

	//MessageBox("hello");
	
	if ( folder2!="" && folder2.Right(1)!=_T(":") )
	{
		CFileStatus64 status;
		if (!GetStatus64(folder2, status) || !(status.m_attribute&0x10))
		{
			if(setaddress)	
				m_wndAddress.GetEditCtrl()->SetWindowText(GetDirView()->GetLocalFolder());
			return;
		}
	}

	
	if(addCache)
		AddAddressCache(folder);

	if (GetFileView()->GetLocalFolder()!=folder)
	{
		GetFileView()->SetLocalFolder(folder);
	}
//	if (m_bShowTree)
		if (GetDirView()->GetLocalFolder()!=folder)
		{
			GetDirView()->SetLocalFolder(folder);
		}
	
	if(setaddress)
		m_wndAddress.GetEditCtrl()->SetWindowText(folder);


	
}


CFvmDirView *CMainFrame::GetDirView()
{
	static CFvmDirView *sView=0;
	if (sView)
		return sView;

	CWnd* pWnd = pSplitterWnd2->GetView(11);
	CFvmDirView * pView = DYNAMIC_DOWNCAST(CFvmDirView, pWnd);
	sView=pView;
	return pView;
}

CShortCutView *CMainFrame::GetShortcutView()
{
	static CShortCutView *sView=0;
	if (sView)
		return sView;

	CWnd* pWnd = m_wndSplitter.GetView(1);
	CShortCutView * pView = DYNAMIC_DOWNCAST(CShortCutView, pWnd);
	sView=pView;
	return pView;
}



CFvmFileView *CMainFrame::GetFileView()
{
	static CFvmFileView *sView=0;
	if (sView)
		return sView;
	CWnd* pWnd = pSplitterWnd2->GetView(12);
	CFvmFileView* pView = DYNAMIC_DOWNCAST(CFvmFileView, pWnd);
	sView=pView;
	return pView;
}


BOOL CMainFrame::CreateToolbars()
{
	CImageList img;
	CString str;

	if (!m_wndReBar.Create(this))
	{
		TRACE0("Failed to create rebar\n");
		return -1;      // fail to create
	}

	if (!m_wndToolBar.CreateEx(this))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	// set up toolbar properties
	m_wndToolBar.GetToolBarCtrl().SetButtonWidth(50, 150);
	m_wndToolBar.GetToolBarCtrl().SetExtendedStyle(TBSTYLE_EX_DRAWDDARROWS);

	img.Create(IDB_HOTTOOLBAR, 22, 0, RGB(255, 0, 255));
	m_wndToolBar.GetToolBarCtrl().SetHotImageList(&img);
	img.Detach();
	img.Create(IDB_COLDTOOLBAR, 22, 0, RGB(255, 0, 255));
	m_wndToolBar.GetToolBarCtrl().SetImageList(&img);
	img.Detach();
	m_wndToolBar.ModifyStyle(0, TBSTYLE_FLAT | TBSTYLE_TRANSPARENT);
	m_wndToolBar.SetButtons(NULL, 16);

	// set up each toolbar button
	m_wndToolBar.SetButtonInfo(0, ID_GO_BACK, TBSTYLE_BUTTON, 0);
	str.LoadString(IDS_BACK);
	m_wndToolBar.SetButtonText(0, str);
	m_wndToolBar.SetButtonInfo(1, ID_GO_FORWARD, TBSTYLE_BUTTON, 1);
	str.LoadString(IDS_FORWARD);
	m_wndToolBar.SetButtonText(1, str);

	m_wndToolBar.SetButtonInfo(2, ID_VIEW_UP, TBSTYLE_BUTTON, 17);
	str.LoadString(IDS_UP);
	m_wndToolBar.SetButtonText(2, str);


//	  

	m_wndToolBar.SetButtonInfo(3, ID_SEPARATOR1, TBBS_SEPARATOR,0);

	m_wndToolBar.SetButtonInfo(4, ID_GO_SEARCH_THE_WEB, TBSTYLE_BUTTON, 5);
	str.LoadString(IDS_SEARCH);
	m_wndToolBar.SetButtonText(4, str);
	

	m_wndToolBar.SetButtonInfo(5, ID_VIEW_FOLDER, TBSTYLE_BUTTON, 16);
	str.LoadString(IDS_FOLDER);
	m_wndToolBar.SetButtonText(5, str);

	m_wndToolBar.SetButtonInfo(6, ID_SEPARATOR2, TBBS_SEPARATOR, 0);

	m_wndToolBar.SetButtonInfo(7, ID_VIEW_REFRESH, TBSTYLE_BUTTON, 3);
	str.LoadString(IDS_REFRESH);
	m_wndToolBar.SetButtonText(7, str);

	m_wndToolBar.SetButtonInfo(8, ID_VIEW_COPY, TBSTYLE_BUTTON, 18);
	str.LoadString(IDS_VIEW_COPY);
	m_wndToolBar.SetButtonText(8, str);

	m_wndToolBar.SetButtonInfo(9, ID_VIEW_PASTE, TBSTYLE_BUTTON, 19);
	str.LoadString(IDS_VIEW_PASTE);
	m_wndToolBar.SetButtonText(9, str);

	m_wndToolBar.SetButtonInfo(10, ID_SEPARATOR3, TBBS_SEPARATOR, 0);

	m_wndToolBar.SetButtonInfo(11, ID_VIEW_VIEW, TBBS_DROPDOWN, 20);
	str.LoadString(IDS_VIEW_VIEW);
	m_wndToolBar.SetButtonText(11, str);


	m_wndToolBar.SetButtonInfo(12, ID_VIEW_DELETE, TBSTYLE_BUTTON, 15);
	str.LoadString(IDS_VIEW_DELETE);
	m_wndToolBar.SetButtonText(12, str);

	m_wndToolBar.SetButtonInfo(13, ID_SEPARATOR3, TBBS_SEPARATOR, 0);

	m_wndToolBar.SetButtonInfo(14, ID_VIEW_SHORTCUT, TBSTYLE_BUTTON, 23);
	str.LoadString(IDS_VIEW_SHORTCUT);
	m_wndToolBar.SetButtonText(14, str);

	m_wndToolBar.SetButtonInfo(15, ID_VIEW_RUN, TBSTYLE_BUTTON, 22);
	str.LoadString(IDS_VIEW_RUN);
	m_wndToolBar.SetButtonText(15, str);



	CRect rectToolBar;

	// set up toolbar button sizes
	m_wndToolBar.GetItemRect(0, &rectToolBar);
	m_wndToolBar.SetSizes(rectToolBar.Size(), CSize(30,20));

	// create a combo box for the address bar
	if (!m_wndAddress.Create(CBS_DROPDOWN | WS_CHILD, CRect(0, 0, 200, 120), this, AFX_IDW_TOOLBAR + 1))
	{
		TRACE0("Failed to create combobox\n");
		return -1;      // fail to create
	}

	// create the animation control
	//m_wndAnimate.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 10, 10), this, AFX_IDW_TOOLBAR + 2);
	//m_wndAnimate.Open(IDR_MFCAVI);

	// add the toolbar, animation, and address bar to the rebar
	m_wndReBar.AddBar(&m_wndToolBar,NULL, NULL, RBBS_GRIPPERALWAYS|RBBS_FIXEDBMP|RBBS_USECHEVRON);
	//m_wndReBar.AddBar(&m_wndAnimate, NULL, NULL, RBBS_FIXEDSIZE | RBBS_FIXEDBMP);
	str.LoadString(IDS_ADDRESS);
	m_wndReBar.AddBar(&m_wndAddress, str, NULL, RBBS_FIXEDBMP | RBBS_BREAK);

	// set up min/max sizes and ideal sizes for pieces of the rebar
	REBARBANDINFO rbbi;

	rbbi.cbSize = sizeof(rbbi);
	rbbi.fMask = RBBIM_CHILDSIZE | RBBIM_IDEALSIZE | RBBIM_SIZE;
	rbbi.cxMinChild = rectToolBar.Width();
	rbbi.cyMinChild = rectToolBar.Height();
	rbbi.cx = rbbi.cxIdeal = rectToolBar.Width() * 9;
	m_wndReBar.GetReBarCtrl().SetBandInfo(0, &rbbi);
	rbbi.cxMinChild = 0;

	CRect rectAddress;

	rbbi.fMask = RBBIM_CHILDSIZE | RBBIM_IDEALSIZE;
	m_wndAddress.GetEditCtrl()->GetWindowRect(&rectAddress);
	rbbi.cyMinChild = rectAddress.Height() + 10;
	rbbi.cxIdeal = 200;
	m_wndReBar.GetReBarCtrl().SetBandInfo(2, &rbbi);

	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_FIXED);

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	/*
// set up Favorites menu
	TCHAR           sz[MAX_PATH];
	TCHAR           szPath[MAX_PATH];
	HKEY            hKey;
	DWORD           dwSize;
	CMenu*          pMenu;

	// first get rid of bogus submenu items.
	pMenu = GetMenu()->GetSubMenu(3);
	while(pMenu->DeleteMenu(0, MF_BYPOSITION));

	// find out from the registry where the favorites are located.
	if(RegOpenKey(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders"), &hKey) != ERROR_SUCCESS)
	{
		TRACE0("Favorites folder not found\n");
		return 0;
	}
	dwSize = sizeof(sz);
	RegQueryValueEx(hKey, _T("Favorites"), NULL, NULL, (LPBYTE)sz, &dwSize);
	ExpandEnvironmentStrings(sz, szPath, MAX_PATH);
	RegCloseKey(hKey);

	BuildFavoritesMenu(szPath, 0, pMenu);
*/

		return TRUE;
}

int CMainFrame::BuildFavoritesMenu(LPCTSTR pszPath, int nStartPos, CMenu* pMenu)
{
	CString         strPath(pszPath);
	CString         strPath2;
	CString         str;
	WIN32_FIND_DATA wfd;
	HANDLE          h;
	int             nPos = 0;
	int             nEndPos = 0;
	int             nNewEndPos = 0;
	int             nLastDir = 0;
	TCHAR           buf[INTERNET_MAX_PATH_LENGTH];
	CStringArray    astrFavorites;
	CStringArray    astrDirs;
	CMenu*          pSubMenu;

	// make sure there's a trailing backslash
	if(strPath[strPath.GetLength() - 1] != _T('\\'))
		strPath += _T('\\');
	strPath2 = strPath;
	strPath += "*.*";

	// now scan the directory, first for .URL files and then for subdirectories
	// that may also contain .URL files
	h = FindFirstFile(strPath, &wfd);
	if(h != INVALID_HANDLE_VALUE)
	{
		nEndPos = nStartPos;
		do
		{
			if((wfd.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM))==0)
			{
				str = wfd.cFileName;
				if(str.Right(4) == _T(".url"))
				{
					// an .URL file is formatted just like an .INI file, so we can
					// use GetPrivateProfileString() to get the information we want
					::GetPrivateProfileString(_T("InternetShortcut"), _T("URL"),
											  _T(""), buf, INTERNET_MAX_PATH_LENGTH,
											  strPath2 + str);
					str = str.Left(str.GetLength() - 4);

					// scan through the array and perform an insertion sort
					// to make sure the menu ends up in alphabetic order
					for(nPos = nStartPos ; nPos < nEndPos ; ++nPos)
					{
						if(str.CompareNoCase(astrFavorites[nPos]) < 0)
							break;
					}
					astrFavorites.InsertAt(nPos, str);
					m_astrFavoriteURLs.InsertAt(nPos, buf);
					++nEndPos;
				}
			}
		} while(FindNextFile(h, &wfd));
		FindClose(h);
		// Now add these items to the menu
		for(nPos = nStartPos ; nPos < nEndPos ; ++nPos)
		{
			pMenu->AppendMenu(MF_STRING | MF_ENABLED, 0xe00 + nPos, astrFavorites[nPos]);
		}


		// now that we've got all the .URL files, check the subdirectories for more
		nLastDir = 0;
		h = FindFirstFile(strPath, &wfd);
		ASSERT(h != INVALID_HANDLE_VALUE);
		do
		{
			if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				// ignore the current and parent directory entries
				if(lstrcmp(wfd.cFileName, _T(".")) == 0 || lstrcmp(wfd.cFileName, _T("..")) == 0)
					continue;

				for(nPos = 0 ; nPos < nLastDir ; ++nPos)
				{
					if(astrDirs[nPos].CompareNoCase(wfd.cFileName) > 0)
						break;
				}
				pSubMenu = new CMenu;
				pSubMenu->CreatePopupMenu();

				// call this function recursively.
				nNewEndPos = BuildFavoritesMenu(strPath2 + wfd.cFileName, nEndPos, pSubMenu);
				if(nNewEndPos != nEndPos)
				{
					// only intert a submenu if there are in fact .URL files in the subdirectory
					nEndPos = nNewEndPos;
					pMenu->InsertMenu(nPos, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT_PTR)pSubMenu->m_hMenu, wfd.cFileName);
					pSubMenu->Detach();
					astrDirs.InsertAt(nPos, wfd.cFileName);
					++nLastDir;
				}
				delete pSubMenu;
			}
		} while(FindNextFile(h, &wfd));
		FindClose(h);
	}
	return nEndPos;
}

void CMainFrame::OnGoBack()
{
	
	if(m_acIndex > 0){
		m_acIndex--;
		CString add = m_addressCache.GetAt(m_acIndex);
		SetLocalFolder(add,1,0);
	}
}
void CMainFrame::OnUpdateGoBack(CCmdUI* pCmdUI)
{
		if(m_acIndex<=0) pCmdUI->Enable(false);
}
void CMainFrame::OnGoForward()
{
	if(m_acIndex < m_maxIndex){
		m_acIndex++;
		CString add = m_addressCache.GetAt(m_acIndex);
		SetLocalFolder(add,1,0);
	}
}
void CMainFrame::OnUpdateGoForward(CCmdUI* pCmdUI)
{
		if(m_acIndex>=m_maxIndex) pCmdUI->Enable(false);
}



void CMainFrame::OnUp()
{
	CString path = GetFileView()->GetLocalFolder();
	CString newpath;
	if (path!="")
	{							
		newpath= path;
		newpath.TrimRight('\\');
		int pos=newpath.ReverseFind('\\');
		newpath=newpath.Left(pos+1);
		SetLocalFolder(newpath);
	}
	
}


void CMainFrame::OnDelete()
{

 GetFileView()->m_pListCtrl->OnLocalcontextDelete();

}

void CMainFrame::OnUpdateDelete(CCmdUI* pCmdUI)
{

	if(!m_displayDelete) pCmdUI->Enable(false);
}

void CMainFrame::OnFolder()
{
	m_wndSplitter.HidePane(BSW_FIRST_PANE);
	pSplitterWnd2->ShowPane(BSW_FIRST_PANE);

	m_FolderIsHiden = 0;
	m_ShortcutIsHiden = 1;
}

void CMainFrame::OnUpdateFolder(CCmdUI* pCmdUI)
{
	//MessageBox(L"hello");
	if(pSplitterWnd2->IsPaneVisible(BSW_FIRST_PANE))
		m_FolderIsHiden = 0;
	else
		m_FolderIsHiden = 1;
	if(!m_FolderIsHiden) pCmdUI->Enable(false);

}

void CMainFrame::OnShortcut()
{
	m_wndSplitter.ShowPane(BSW_FIRST_PANE);
	pSplitterWnd2->HidePane(BSW_FIRST_PANE);

	m_FolderIsHiden = 1;
	m_ShortcutIsHiden = 0;
}

void CMainFrame::OnUpdateShortcut(CCmdUI* pCmdUI)
{
	if(m_wndSplitter.IsPaneVisible(BSW_FIRST_PANE))
		m_ShortcutIsHiden = 0;
	else
		m_ShortcutIsHiden = 1;
	if(!m_ShortcutIsHiden) pCmdUI->Enable(false);
}
void CMainFrame::OnRun()
{

}

void CMainFrame::OnView()
{
	UINT nID;


	nID  = IDR_VIEW_DROP_DOWN;		

	// load and display popup menu
	CMenu menu;
	menu.LoadMenu(nID);
	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup);
	
	CRect rc;
//	SendMessage(TB_GETRECT, pnmtb->iItem, (LPARAM)&rc);
	
	m_wndToolBar.GetToolBarCtrl( ).GetRect(ID_VIEW_VIEW, &rc);
	ClientToScreen(&rc);
	pPopup->TrackPopupMenu( TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL,
		rc.left, rc.bottom, this, &rc);
}

void CMainFrame::OnGoSearchTheWeb()
{
	//GoSearch();
}

void CMainFrame::OnGoStartPage()
{
	//GoHome();
}

void CMainFrame::OnLargeIcons()
{
	m_nLocalListViewStyle=2;
	GetFileView()->m_pListCtrl->SetListStyle(m_nLocalListViewStyle);
}
void CMainFrame::OnUpdateLargeIcons(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nLocalListViewStyle==2);	
}
void CMainFrame::OnSmallIcons()
{
	m_nLocalListViewStyle=3;
	GetFileView()->m_pListCtrl->SetListStyle(m_nLocalListViewStyle);
}
void CMainFrame::OnUpdateSmallIcons(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nLocalListViewStyle==3);	
}
void CMainFrame::OnList()
{
	m_nLocalListViewStyle=1;
	GetFileView()->m_pListCtrl->SetListStyle(m_nLocalListViewStyle);
}
void CMainFrame::OnUpdateList(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nLocalListViewStyle==1);	
}
void CMainFrame::OnDetails()
{
	m_nLocalListViewStyle=0;
	GetFileView()->m_pListCtrl->SetListStyle(m_nLocalListViewStyle);
}
void CMainFrame::OnUpdateDetails(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nLocalListViewStyle==0);	
}

void CMainFrame::MakeLong(CString &DirName)
{
	CFileFind find;
	BOOL found=find.FindFile(DirName+_T("\\."));
	if (found)
	{
		find.FindNextFile();
		DirName=find.GetFilePath();
	}
}

BOOL CMainFrame::doNewAddress(CString &text)
{

	if (text!="")
	{
		text.TrimLeft(_T(" "));
		text.TrimRight(_T(" "));
		text.TrimRight(_T("\\"));


		if (text == _T(""))
		{
			SetLocalFolder(text, 0);
			return TRUE;
		}


		if (text.Right(1)==_T(":") )
		{
			CFileStatus64 status;
			
			if (GetStatus64(text+"\\*.*", status))
			{
				SetLocalFolder(text, 0);
				return TRUE;
			}
			else
			{
				AfxMessageBox(IDS_ERRORMSG_PATHNOTFOUND, MB_ICONEXCLAMATION);
				return FALSE;
			}
		}

		CFileStatus64 status;
		if (GetStatus64(text, status))
		{
			if (!(status.m_attribute&0x10)){
				AfxMessageBox(IDS_ERRORMSG_PATHNOTFOUND, MB_ICONEXCLAMATION);
				return FALSE;
			}
			else
			{
				CMainFrame *pMainFrame=DYNAMIC_DOWNCAST(CMainFrame,GetParentFrame());
				MakeLong(text);
				pMainFrame->SetLocalFolder(text, 0);
				return TRUE;
			}
		}
		else{
			AfxMessageBox(IDS_ERRORMSG_PATHNOTFOUND, MB_ICONEXCLAMATION);
			return FALSE;
		}
			
	}

	return FALSE;

}

void CMainFrame::OnNewAddressEnter()
{
  	CString text;

	m_wndAddress.GetEditCtrl()->GetWindowText(text);

	if(doNewAddress(text)){
	

		COMBOBOXEXITEM item;

		item.mask = CBEIF_TEXT;
		item.iItem = -1;
		item.pszText = (LPTSTR)(LPCTSTR)text;
		m_wndAddress.InsertItem(&item);		
	}
}




void CMainFrame::OnNewAddress()
{
	// gets called when an item in the Address combo box is selected
	// just navigate to the newly selected location.

	 	CString text;

	//m_wndAddress.GetEditCtrl()->GetWindowText(text);
	m_wndAddress.GetLBText(m_wndAddress.GetCurSel(), text);
	doNewAddress(text);
}


BOOL CMainFrame::SetStatusBarText(LPCTSTR pszText)
{
	ASSERT(pszText);
//	MessageBox(pszText);
	m_wndStatusBar.SetWindowText(pszText);//SetPaneText(0, pszText);
	
	return TRUE;
}

void CMainFrame::AddAddressCache(LPCTSTR  text)
{

	
	m_acIndex++;
	m_addressCache.InsertAt(m_acIndex, text);
	
	m_maxIndex = m_acIndex;
	
}

void CMainFrame::GetDefault()
{
	DWORD type;
	DWORD value;
	DWORD bytes;
	HKEY key;
	if (RegOpenKey(HKEY_CURRENT_USER, _T("Software\\fvmshell") ,&key)!=ERROR_SUCCESS)
	{
		m_nLocalListViewStyle = 2;

		m_FolderIsHiden = 1;
		m_ShortcutIsHiden = 0;
	

	}
	else{
		if(RegQueryValueEx(key, _T("ViewStyle"), NULL, &type, (LPBYTE)&value, &bytes) == ERROR_SUCCESS)
			m_nLocalListViewStyle = value;
		if(RegQueryValueEx(key, _T("FolderIsHidden"), NULL, &type, (LPBYTE)&value, &bytes) == ERROR_SUCCESS)
			m_FolderIsHiden = value;
		if(RegQueryValueEx(key, _T("ShortCutIsHidden"), NULL, &type, (LPBYTE)&value, &bytes) == ERROR_SUCCESS)
			m_ShortcutIsHiden = value;
	}

	GetFileView()->m_pListCtrl->SetListStyle(m_nLocalListViewStyle);

	if(m_ShortcutIsHiden)
		m_wndSplitter.HidePane(BSW_FIRST_PANE);
	else
		m_wndSplitter.ShowPane(BSW_FIRST_PANE);

	if(m_FolderIsHiden)
		pSplitterWnd2->HidePane(BSW_FIRST_PANE);
	else
		pSplitterWnd2->ShowPane(BSW_FIRST_PANE);

}

void CMainFrame::SaveDefault()
{

	DWORD type;
	DWORD value;
	DWORD bytes;
	HKEY key;
	if (RegCreateKey(HKEY_CURRENT_USER, _T("Software\\fvmshell") ,&key)!=ERROR_SUCCESS)
	{
	
		MessageBox(L"Could not open key\n");
		return;

	}
	else{

		type = REG_DWORD;
		bytes = sizeof(DWORD);
		value =	m_nLocalListViewStyle;
		if(RegSetValueEx(key, _T("ViewStyle"), NULL, type, (LPBYTE)&value, bytes) !=ERROR_SUCCESS)
			MessageBox(L"could not save value1");
	
		value =		m_FolderIsHiden;
		RegSetValueEx(key, _T("FolderIsHidden"), NULL, type, (LPBYTE)&value, bytes);
		
		value =m_ShortcutIsHiden;
		RegSetValueEx(key, _T("ShortCutIsHidden"), NULL, type, (LPBYTE)&value, bytes);
	
	}


}

void CMainFrame::OnViewDropDown(NMTOOLBAR* pnmtb, LRESULT *plr)
{

//	MessageBox("hello");

		CWnd *pWnd;
	UINT nID;

	// Switch on button command id's.
	switch (pnmtb->iItem)
	{
	case ID_VIEW_VIEW:
		pWnd = &m_wndToolBar;
		nID  = IDR_VIEW_DROP_DOWN;		
		break;
	default:
		return;
	}
	
	// load and display popup menu
	CMenu menu;
	menu.LoadMenu(nID);
	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup);
	
	CRect rc;
	pWnd->SendMessage(TB_GETRECT, pnmtb->iItem, (LPARAM)&rc);
	pWnd->ClientToScreen(&rc);
	
	pPopup->TrackPopupMenu( TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL,
		rc.left, rc.bottom, this, &rc);
		
}

void CMainFrame::OnSetting()
{
	CSetting setting(this);
	setting.DoModal();
}

void CMainFrame::OnDestroy() 
{
	CFrameWnd::OnDestroy();
	SaveDefault();

	// TODO: Add your message handler code here
	
}


void CMainFrame::RefreshViews(int side)
{

	if (side!=2)
	{
		CString folder=GetFileView()->GetLocalFolder();
		GetFileView()->SetLocalFolder(folder);
		GetDirView()->GetTreeCtrl()->DisplayTree(NULL,FALSE);
		GetDirView()->GetTreeCtrl()->SetSelPath(folder);
		//SetLocalFolder(folder);
	}
}

void CMainFrame::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	SetWindowText(m_vmName+" - fvmshell");
	// Do not call CFrameWnd::OnPaint() for painting messages
}
