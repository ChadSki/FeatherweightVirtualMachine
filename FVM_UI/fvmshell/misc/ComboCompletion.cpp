// ComboCompletion.cpp : implementation file
//
// Copyright (c) Chris Maunder 1997.
// Please feel free to use and distribute.

#include "stdafx.h"
#include "ComboCompletion.h"
#include "VisualStylesXP.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CComboCompletion

CComboCompletion::CComboCompletion()
{
	m_bAutoComplete = TRUE;
	m_bPressed = FALSE;
}

CComboCompletion::~CComboCompletion()
{
}


BEGIN_MESSAGE_MAP(CComboCompletion, CComboBox)
	//{{AFX_MSG_MAP(CComboCompletion)
	ON_CONTROL_REFLECT(CBN_EDITUPDATE, OnEditUpdate)
	ON_WM_CHAR()
	ON_CONTROL_REFLECT(CBN_SELCHANGE, OnSelchange)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CANCELMODE()
	ON_WM_CAPTURECHANGED()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CComboCompletion message handlers

#define VK_A		65
BOOL CComboCompletion::PreTranslateMessage(MSG* pMsg)
{
	// Need to check for backspace/delete. These will modify the text in
	// the edit box, causing the auto complete to just add back the text
	// the user has just tried to delete. 

	if (pMsg->message == WM_KEYDOWN)
	{
		if( GetKeyState( VK_CONTROL )&128 && pMsg->wParam == VK_A )
		{
			SetEditSel(0, -1);
			return TRUE;
		}

		m_bAutoComplete = TRUE;

		int nVirtKey = (int) pMsg->wParam;
		if (nVirtKey == VK_DELETE || nVirtKey == VK_BACK)
			m_bAutoComplete = FALSE;
	
		if (nVirtKey==13)//VK_ENTER
		{
			CString text;
			GetWindowText(text);
			if (text!=_T(""))
				OnChangeDir(text);
			return TRUE;
		}	
	
	}

	return CComboBox::PreTranslateMessage(pMsg);
}

void CComboCompletion::OnEditUpdate() 
{
  // if we are not to auto update the text, get outta here
  if (!m_bAutoComplete) 
      return;

  // Get the text in the edit box
  CString str;
  GetWindowText(str);
  int nLength = str.GetLength();
  
  // Currently selected range
  DWORD dwCurSel = GetEditSel();
  WORD dStart = LOWORD(dwCurSel);
  WORD dEnd   = HIWORD(dwCurSel);

  // Search for, and select in, and string in the combo box that is prefixed
  // by the text in the edit box
  if (SelectString(-1, str) == CB_ERR)
  {
      SetWindowText(str);		// No text selected, so restore what was there before
      if (dwCurSel != CB_ERR)
        SetEditSel(dStart, dEnd);	//restore cursor postion
  }

  // Set the text selection as the additional text that we have added
  if (dEnd < nLength && dwCurSel != CB_ERR)
      SetEditSel(dStart, dEnd);
  else
      SetEditSel(nLength, -1);
}

void CComboCompletion::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar==13)
	{
		CString text;
		GetWindowText(text);
		if (text!="")
			OnChangeDir(text);
	}	
	CComboBox::OnChar(nChar, nRepCnt, nFlags);
}

void CComboCompletion::OnSelchange() 
{
	int pos=GetCurSel();
	if (pos==CB_ERR)
		return;
	CString text;
	GetLBText(pos,text);
	if (text!=_T(""))
		OnChangeDir(text);
}

void CComboCompletion::OnPaint() 
{
	Default();

	CDC *pDC=GetDC();

	CRect rcItem;
	
	GetClientRect(&rcItem);
	rcItem.DeflateRect(1,1);
	
	CVisualStylesXP xp;
	if (!IsWindowEnabled())
	{
		if (!xp.IsAppThemed())
		{	
			pDC->Draw3dRect( rcItem, ::GetSysColor(COLOR_BTNFACE), ::GetSysColor(COLOR_BTNFACE) );
			rcItem.DeflateRect(1,1);
			rcItem.left = rcItem.right-::GetSystemMetrics(SM_CXHTHUMB);
			rcItem.top--;
			rcItem.bottom++;
			pDC->Draw3dRect( rcItem, GetSysColor(COLOR_BTNFACE), GetSysColor(COLOR_3DDKSHADOW));
			rcItem.DeflateRect(1,1);
			rcItem.top;
			pDC->Draw3dRect( rcItem, GetSysColor(COLOR_BTNHIGHLIGHT), GetSysColor(COLOR_BTNSHADOW) );
			rcItem.DeflateRect(1,1);
			pDC->Draw3dRect(rcItem, GetSysColor(COLOR_BTNFACE), GetSysColor(COLOR_BTNFACE));
		}
		else
		{
			rcItem.right -= ::GetSystemMetrics(SM_CXHTHUMB);
			pDC->Draw3dRect(rcItem, GetSysColor(COLOR_BTNFACE), GetSysColor(COLOR_BTNFACE));
			rcItem.DeflateRect(1,1);
			pDC->Draw3dRect(rcItem, GetSysColor(COLOR_BTNFACE), GetSysColor(COLOR_BTNFACE));
		}
	}
	
	else
	{
		if (!xp.IsAppThemed())
		{
			pDC->Draw3dRect( rcItem, GetSysColor( COLOR_WINDOW ), GetSysColor( COLOR_WINDOW ) );
			
			if (!m_bPressed)
			{
				rcItem.DeflateRect(1,1);
				rcItem.left = rcItem.right-::GetSystemMetrics(SM_CXHTHUMB);
				rcItem.top--;
				rcItem.bottom++;
				pDC->Draw3dRect( rcItem, GetSysColor(COLOR_BTNFACE), GetSysColor(COLOR_3DDKSHADOW) );
				rcItem.DeflateRect(1,1);
				rcItem.top;
				pDC->Draw3dRect( rcItem, GetSysColor(COLOR_BTNHIGHLIGHT), GetSysColor(COLOR_BTNSHADOW) );
				rcItem.DeflateRect(1,1);
				pDC->Draw3dRect(rcItem, GetSysColor(COLOR_BTNFACE), GetSysColor(COLOR_BTNFACE));
			}
			else
			{
				rcItem.DeflateRect(1,1);
				rcItem.left = rcItem.right-::GetSystemMetrics(SM_CXHTHUMB);
				rcItem.top--;
				rcItem.bottom++;
				pDC->Draw3dRect( rcItem, GetSysColor(COLOR_BTNSHADOW), GetSysColor(COLOR_BTNSHADOW));		
				rcItem.DeflateRect(1,1);
				pDC->Draw3dRect(rcItem, GetSysColor(COLOR_BTNFACE), GetSysColor(COLOR_BTNFACE));
			}
		}
	}
	
	GetClientRect(&rcItem);
	// Cover up dark 3D shadow.
	pDC->Draw3dRect(rcItem, GetSysColor(COLOR_3DDKSHADOW), GetSysColor(COLOR_3DDKSHADOW));

	rcItem.DeflateRect(1,1);
	
	
	ReleaseDC(pDC);
}

void CComboCompletion::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CRect rect;
	GetClientRect(&rect);
	if (point.x>=(rect.right-::GetSystemMetrics(SM_CXHTHUMB)-2))
		m_bPressed = TRUE;
	
	CComboBox::OnLButtonDown(nFlags, point);
}

void CComboCompletion::OnLButtonUp(UINT nFlags, CPoint point) 
{
	m_bPressed = FALSE;
	
	CComboBox::OnLButtonUp(nFlags, point);
}

void CComboCompletion::OnMouseMove(UINT nFlags, CPoint point) 
{
	CRect rect;
	GetClientRect(&rect);
	if (point.x<(rect.right-::GetSystemMetrics(SM_CXHTHUMB)-2)|| point.y>18 || point.x>(rect.right-3) || point.y<2)
		m_bPressed = FALSE;
	
	CComboBox::OnMouseMove(nFlags, point);
}

void CComboCompletion::OnChangeDir(CString dir)
{
}

void CComboCompletion::OnCancelMode() 
{
	CComboBox::OnCancelMode();
	
	m_bPressed = FALSE;
	UpdateWindow();
}

void CComboCompletion::OnCaptureChanged(CWnd *pWnd) 
{
	m_bPressed = FALSE;
	UpdateWindow();
	
	CComboBox::OnCaptureChanged(pWnd);
}
