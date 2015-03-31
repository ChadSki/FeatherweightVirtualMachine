// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////
#include "FileView.h"
#include "FvmBrowser.h"
#include "ProcessView.h"
#include "LeftView.h"
#include "RegView.h"
#include "fvmData.h"

#if !defined(AFX_MAINFRM_H__61B450B5_9E8C_418C_9853_9AC4500DFD77__INCLUDED_)
#define AFX_MAINFRM_H__61B450B5_9E8C_418C_9853_9AC4500DFD77__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BiSplitterWnd.h"
class CRegView;
class CLeftView;

class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
protected:
//	CSplitterWnd m_wndSplitter;
public:

	CBiSplitterWnd m_wndBiSplitter;
	CBiSplitterWnd *pSplitterWnd2;
	CBiSplitterWnd *pSplitterWnd3;
	CBiSplitterWnd *pSplitterWnd4;
	CBiSplitterWnd *pSplitterWnd5;
// Operations
public:
	t_FvmData *fvmData;
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	
	void CommitCopyRegistry(HKEY ohkey, HKEY nhkey, CString ok, CString nk, int cp);
	void CommitRegistry(CString rn);
	void CommitFiles(CString root);
	t_FvmData * created;
	void DeleteFvm(t_FvmData *fvmd);
	CString oldName;
	t_FvmData * m_selectedFvm;
	CLeftView * GetLeftView();
	CFileView * GetFileView();
	void OnRefresh();
	void UpdateDisplays();
	CProcessView * GetProcessView();
	void DisplayProcess(t_FvmData *data);
	void EmptyFVMWorkingSet(t_FvmData *fvmd, int *pid);
	void RestoreFVMWorkingSet(t_FvmData *fvmd);
	void SuspendProcesses(t_FvmData *data, BOOL suspend);
	void FvmDataFree(void);
	BOOL CreateVM(t_FvmData * fvmd, int *ret);
	BOOL CreateVmJobObject(PROCESS_INFORMATION &pi, t_FvmData * fvmd);  //added by zhiyong shan
	BOOL TerminateVM(t_FvmData * fvmd, int *ret);
	CFvmBrowser * GetFvmBrowserView();
	void FvmDataRemove(CString name);
	void FvmDataAdd(t_FvmData *data);
	t_FvmData * FvmDataFind(CString name, int flag = 0);
	BOOL GetSysImgList(CImageList *imgList);
	CRegView * GetRegView();
	CReBar m_wndReBar;
	BOOL CreateToolbars();
	CImageList m_imageList;
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	//}}AFX_MSG

	afx_msg void OnCreateFvm();
	afx_msg void OnCopy();
	afx_msg void OnConfig();
	afx_msg void OnStart();
	afx_msg void OnStop();
	afx_msg void OnSuspend();
	afx_msg void OnResume();
	afx_msg void OnDelete();
	afx_msg void OnCommit();
	afx_msg void OnProcess();
	afx_msg void OnFile();
	afx_msg void OnRegistry();
	afx_msg void OnShell();

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__61B450B5_9E8C_418C_9853_9AC4500DFD77__INCLUDED_)
