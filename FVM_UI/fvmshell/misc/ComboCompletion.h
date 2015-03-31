#if !defined(AFX_ComboCompletion_H__115F422E_5CD5_11D1_ABBA_00A0243D1382__INCLUDED_)
#define AFX_ComboCompletion_H__115F422E_5CD5_11D1_ABBA_00A0243D1382__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// ComboCompletion.h : header file
//
// Copyright (c) Chris Maunder 1997.
// Please feel free to use and distribute.


/////////////////////////////////////////////////////////////////////////////
// CComboCompletion window

class CComboCompletion : public CComboBox
{
// Construction
public:
	CComboCompletion();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CComboCompletion)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual void OnChangeDir(CString dir);
	virtual ~CComboCompletion();

	BOOL m_bAutoComplete;

	// Generated message map functions
protected:
	BOOL m_bPressed;
	//{{AFX_MSG(CComboCompletion)
	afx_msg void OnEditUpdate();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSelchange();
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnCancelMode();
	afx_msg void OnCaptureChanged(CWnd *pWnd);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ComboCompletion_H__115F422E_5CD5_11D1_ABBA_00A0243D1382__INCLUDED_)

