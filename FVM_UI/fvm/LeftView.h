// LeftView.h : interface of the CLeftView class
//


#pragma once
#include "afxcmn.h"

class CRegDoc;
class t_FvmData;
class CMainFrame;

class CLeftView : public CTreeView
{
protected: // create from serialization only
	CLeftView();
	DECLARE_DYNCREATE(CLeftView)

// Attributes
public:
	CRegDoc* GetDocument();
	int LamEnumRegKey(CString hive, HKEY hKey, CString sKeyName, HTREEITEM hItem, CTreeCtrl *pCtrl);
// Operations
public:
	HTREEITEM hMyComputer;
	HTREEITEM hHCR;
	HTREEITEM hHCU;
	HTREEITEM hHLM;
	HTREEITEM hHU;
	HTREEITEM hHCC;

// Overrides
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct

// Implementation
public:
	virtual ~CLeftView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
private:
	CImageList m_ImageList;
public:
	t_FvmData * current_Fvm;
	CMainFrame * m_Parent;
	void DisplayRegistry(t_FvmData *fvmd);
	afx_msg void OnTvnSelchanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnItemexpanding(NMHDR *pNMHDR, LRESULT *pResult);
};

#ifndef _DEBUG  // debug version in LeftView.cpp
inline CRegDoc* CLeftView::GetDocument()
   { return reinterpret_cast<CRegDoc*>(m_pDocument); }
#endif

