// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////
#include "BiSplitterWnd.h"
#include "FvmFileView.h"
#include "fvmshellDoc.h"
#include "FvmDirView.h"
#include "ShortCutView.h"

#if !defined(AFX_MAINFRM_H__36FF4E0A_A3FA_4F8B_8ADE_C7A40F749CFF__INCLUDED_)
#define AFX_MAINFRM_H__36FF4E0A_A3FA_4F8B_8ADE_C7A40F749CFF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CTransparentDialogBar;
class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)
	BOOL doNewAddress(CString &text);
	void MakeLong(CString &DirName);

// Attributes
protected:
	CBiSplitterWnd m_wndSplitter;
	CBiSplitterWnd *pSplitterWnd2;

public:
	CStringArray m_astrFavoriteURLs;
	CStringArray m_addressCache;
	int m_acIndex;
	int m_maxIndex;

	int m_displayDelete;

	int m_nLocalListViewStyle;
	//default values
	int m_defaultViewStyle;
	CString m_defaultAddress;
// Operations
public:

	BOOL SetStatusBarText(LPCTSTR pszText);

	int BuildFavoritesMenu(LPCTSTR pszPath, int nStartPos, CMenu* pMenu);
	void SetLocalFolder(CString folder, int setaddress=1, int addCache=1);
	CFvmDirView*	 GetDirView();
	CFvmFileView* GetFileView();
	CShortCutView *GetShortcutView();
	BOOL CreateToolbars();
	void GetDefault();
	void SaveDefault();
	
	void RefreshViews(int side);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	CString m_vmName;
	void OnSetting();
	void AddAddressCache(LPCTSTR  text);
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
//	CToolBar    m_wndToolBar;
	CToolBar    *m_pWndToolBar;
	CReBar      *m_pWndReBar;
	CTransparentDialogBar  *m_pWndDlgBar;

	CToolBar    m_wndToolBar;
	CReBar      m_wndReBar;

	CComboBoxEx m_wndAddress;
	CAnimateCtrl m_wndAnimate;

	int         m_FolderIsHiden;
	int         m_ShortcutIsHiden;
// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	//}}AFX_MSG

	afx_msg void OnNewAddress();
	afx_msg void OnNewAddressEnter();
	afx_msg void OnGoBack();
	afx_msg void OnUpdateGoBack(CCmdUI* pCmdUI);
	afx_msg void OnGoForward();
	afx_msg void OnUpdateGoForward(CCmdUI* pCmdUI);
	afx_msg void OnUp();
	afx_msg void OnFolder();
	afx_msg void OnUpdateFolder(CCmdUI* pCmdUI);
	afx_msg void OnView();
	afx_msg void OnShortcut();
	afx_msg void OnUpdateShortcut(CCmdUI* pCmdUI);
	afx_msg void OnRun();
	afx_msg void OnGoSearchTheWeb();
	afx_msg void OnGoStartPage();
	afx_msg void OnLargeIcons();
	afx_msg void OnUpdateLargeIcons(CCmdUI* pCmdUI);
	afx_msg void OnSmallIcons();
	afx_msg void OnUpdateSmallIcons(CCmdUI* pCmdUI);
	afx_msg void OnList();
	afx_msg void OnUpdateList(CCmdUI* pCmdUI);
	afx_msg void OnDetails();
	afx_msg void OnUpdateDetails(CCmdUI* pCmdUI);
	afx_msg void OnViewDropDown(NMTOOLBAR* pnmtb, LRESULT *plr);
	afx_msg void OnDelete();
	afx_msg void OnUpdateDelete(CCmdUI* pCmdUI);

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__36FF4E0A_A3FA_4F8B_8ADE_C7A40F749CFF__INCLUDED_)
