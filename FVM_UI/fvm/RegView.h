// RegView.h : interface of the CRegView class
//


#pragma once

#include "Utility.h"
#include "MainFrm.h"
#include <vector>

class CFvmDoc;
class CMainFrame;

class CRegView : public CListView
{
protected: // create from serialization only
	CRegView();
	DECLARE_DYNCREATE(CRegView)

// Attributes
public:
	CFvmDoc* GetDocument() const;

// Operations
public:
	vector< RegKeyDetail > ListEnumKey;

// Overrides
	public:
virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void OnInitialUpdate(); // called first time after construct

// Implementation
public:
	virtual ~CRegView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct);
	DECLARE_MESSAGE_MAP()
public:
	CMainFrame * m_Parent;
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
};
/*
#ifndef _DEBUG  // debug version in RegView.cpp
inline CRegDoc* CRegView::GetDocument() const
   { return reinterpret_cast<CRegDoc*>(m_pDocument); }
#endif
*/
