// CoolBtn.cpp : implementation file
//

#include "stdafx.h"
#include "CoolBtn.h"
#include "VisualStylesXP.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int nDropBtnWidth =  14;
/////////////////////////////////////////////////////////////////////////////
// CCoolBtn
// ========
// 
// To Use:
// 1. Create a Bitmap resource 16x15 Pixels normal default size for a toolbar
//    bitmap.
//
// 2. Call CCoolBtns on Create function.
//
// 3. Call SetButtonImage specificing the Transparency color reference.
//    (Usally RGB(255, 0, 255) magenta)
// 
// 4. Add menu items with AddMenuItem using nMenuFlags to add disabled and
///   seperator menu items
//
// 5. Add the appropiate ON_COMMAND handlers in the parent windows message map
//
// 6. Enjoy...
////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION:	  CCoolBtn
//
// DESCRIPTION:	
//
// NOTES:			
//
// MAINTENANCE:
// Name:		  Date:	  Version:	Notes:
// NT ALMOND	210100	1.0			  Origin
//
////////////////////////////////////////////////////////////////////////////////
CCoolBtn::CCoolBtn()
{
  m_bPushed = FALSE;
  m_bLoaded = FALSE;
  m_bMenuPushed = FALSE;
  m_bMenuLoaded = FALSE;
  m_bMouseMovedOut = FALSE;
  m_bHovering = FALSE;
  m_bHoveringDropdown = FALSE;

  ZeroMemory(&m_bm, sizeof m_bm);
  m_menu.CreatePopupMenu();

}

////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION:	  ~CCoolBtn
//
// DESCRIPTION:	
//
// NOTES:			
//
// MAINTENANCE:
// Name:		  Date:	  Version:	Notes:
// NT ALMOND	210100	1.0			  Origin
//
////////////////////////////////////////////////////////////////////////////////
CCoolBtn::~CCoolBtn()
{
}


BEGIN_MESSAGE_MAP(CCoolBtn, CButton)
	//{{AFX_MSG_MAP(CCoolBtn)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_LBUTTONDBLCLK()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCoolBtn message handlers

////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION:	  Create
//
// DESCRIPTION:	Create the button
//
// NOTES:			
//
// MAINTENANCE:
// Name:		  Date:	  Version:	Notes:
// NT ALMOND	210100	1.0			  Origin
//
////////////////////////////////////////////////////////////////////////////////
BOOL CCoolBtn::Create( LPCTSTR lpszCaption, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID )
{
  dwStyle |= BS_OWNERDRAW; // Enforce
  m_pParentWnd = pParentWnd;
  return CButton::Create(lpszCaption, dwStyle, rect, pParentWnd, nID );	;
}



////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION:	  DrawItem
//
// DESCRIPTION:	Called in response to draw the button
//
// NOTES:			
//
// MAINTENANCE:
// Name:		  Date:	  Version:	Notes:
// NT ALMOND	210100	1.0			  Origin
//
////////////////////////////////////////////////////////////////////////////////
void CCoolBtn::DrawItemXP(DRAWITEMSTRUCT* lpDIS) 
{
	if (lpDIS->CtlType != ODT_BUTTON)
		return;

	CFont *pFont = CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT));

	CDC dcMem;
	CBitmap bmp;
	
	CRect trueRect(lpDIS->rcItem);
	CRect btnRect(trueRect);
	btnRect.right -= nDropBtnWidth-2;
	
	CDC *pDC = CDC::FromHandle(lpDIS->hDC);
	
	UINT state = lpDIS->itemState;

	BOOL	bPressed = state & ODS_SELECTED;
	BOOL	bGotFocus = state & ODS_FOCUS;
	BOOL	bDisabled = state & ODS_DISABLED;

	// BOOL	bFocusRect = !(state & ODS_NOFOCUSRECT);	// Win2K/XP

	int		iMode;
	
    if (m_bHovering)
	{
		if (bPressed)
		{
			iMode = PBS_PRESSED;
		}
		else
		{
			iMode = PBS_HOT;
		}
	}
	else
	{
		if (bDisabled)
		{
			iMode = PBS_DISABLED;
		}
		else if (bGotFocus || GetWindowLong(m_hWnd, GWL_STYLE) & BS_DEFPUSHBUTTON)
		{
			iMode = PBS_DEFAULTED;
		}
		else if (!bGotFocus)
		{
			iMode = PBS_NORMAL;
		}
	}
	
	CVisualStylesXP xp;
	HTHEME hTheme = xp.OpenThemeData(GetSafeHwnd(), L"Button");
	btnRect.DeflateRect(1, 1);
	xp.DrawThemeBackground(hTheme, *pDC, BP_PUSHBUTTON, iMode, btnRect, btnRect);
	xp.CloseThemeData(hTheme);

	
	if (bDisabled)
		iMode = CBXS_DISABLED;
	else if (m_bMenuPushed)
		iMode = CBXS_PRESSED;
	else if (m_bHoveringDropdown)
		iMode = CBXS_HOT;
	else
		iMode = CBXS_NORMAL;
	
	trueRect.DeflateRect(2, 2);
	CRect dropButtonRect = trueRect;
	dropButtonRect.right++;
	dropButtonRect.left = dropButtonRect.right - nDropBtnWidth +1;
	hTheme = xp.OpenThemeData(GetSafeHwnd(), L"Combobox");
	xp.DrawThemeBackground(hTheme, *pDC, CP_DROPDOWNBUTTON, iMode, dropButtonRect, dropButtonRect);
	xp.CloseThemeData(hTheme);

	
	CRect rectFocus(btnRect);
	rectFocus.DeflateRect(3,3);

	////////////////////////////////////////
	// State Focus                        //
	////////////////////////////////////////
	if (lpDIS->itemState & ODS_FOCUS || m_bPushed) 
		if (!m_bMenuPushed)
			pDC->DrawFocusRect(&rectFocus);
		
		
	////////////////////////////////////////
	// Action Focus                       //
	////////////////////////////////////////
	if ((lpDIS->itemAction & ODA_FOCUS))
		if (!m_bMenuPushed)
			pDC->DrawFocusRect(&rectFocus);

	////////////////////////////////////////
	// Draw out text                      //
	////////////////////////////////////////
	CFont *pOldFont=pDC->SelectObject(pFont);
	CRect rectText(rectFocus);
	rectFocus.left += m_bm.bmWidth + 2;
	
	CString strCaption;
	GetWindowText(strCaption);
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetBkColor(GetSysColor(COLOR_BTNFACE));
	
	rectFocus.top++;
	if (ODS_DISABLED & lpDIS->itemState)
	{
		rectFocus.OffsetRect(1,1);
		pDC->SetTextColor(GetSysColor(COLOR_WINDOW));
		pDC->DrawText(strCaption, rectFocus,DT_SINGLELINE|DT_CENTER|DT_VCENTER);
		
		rectFocus.OffsetRect(-1,-1);
		pDC->SetTextColor(GetSysColor(COLOR_GRAYTEXT));
		pDC->DrawText(strCaption, rectFocus,DT_SINGLELINE|DT_CENTER|DT_VCENTER);
	}
	else
	{
		pDC->SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
		pDC->DrawText(strCaption, rectFocus,DT_SINGLELINE|DT_CENTER|DT_VCENTER);
	}
}

void CCoolBtn::DrawItem(DRAWITEMSTRUCT* lpDIS) 
{
	CVisualStylesXP xp;
	if (xp.IsAppThemed())
	{
		DrawItemXP(lpDIS);
		return;
	}
	
	if (lpDIS->CtlType != ODT_BUTTON)
		return;
	
	CFont *pFont = CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT));
	
	CDC dcMem;
	CBitmap bmp;
	
	CRect btnRect(lpDIS->rcItem);
	CRect trueRect(btnRect);
	
	CDC *pDC = CDC::FromHandle(lpDIS->hDC);

  ////////////////////////////////////////
  // Button Background                  //
  ////////////////////////////////////////
  pDC->FillRect(trueRect,&CBrush(GetSysColor(COLOR_BTNFACE)));

  BOOL bDefaultBtn = GetWindowLong(m_hWnd,GWL_STYLE) & BS_DEFPUSHBUTTON;
  BOOL bDisabled = ODS_DISABLED & lpDIS->itemState;
 
  if (bDefaultBtn)
    btnRect.DeflateRect(1,1);

  CRect rectFocus(btnRect);

  rectFocus.DeflateRect(3,3);

  rectFocus.right -= nDropBtnWidth+1;


  ////////////////////////////////////////
  // Button in a normal state           //
  ////////////////////////////////////////
  if (!m_bPushed || m_bMenuPushed)
    pDC->DrawFrameControl(&btnRect,DFC_BUTTON,DFCS_BUTTONPUSH);
  

  ////////////////////////////////////////
  // Default Button State               //
  ////////////////////////////////////////
  if ((bDefaultBtn || m_bPushed) && !bDisabled)
  {
    pDC->FrameRect(&lpDIS->rcItem,CBrush::FromHandle((HBRUSH)GetStockObject(BLACK_BRUSH)));
    if (m_bPushed && !m_bMenuPushed)
      pDC->FrameRect(&btnRect,CBrush::FromHandle((HBRUSH)GetStockObject(BLACK_BRUSH))); 
  }


  ////////////////////////////////////////
  // State Focus                        //
  ////////////////////////////////////////
   if (lpDIS->itemState & ODS_FOCUS || m_bPushed) 
     if (!m_bMenuPushed)
        pDC->DrawFocusRect(&rectFocus);


  ////////////////////////////////////////
  // Action Focus                       //
  ////////////////////////////////////////
  if ((lpDIS->itemAction & ODA_FOCUS))
     if (!m_bMenuPushed)
        pDC->DrawFocusRect(&rectFocus);


  ////////////////////////////////////////
  // Draw out bitmap                    //
  ////////////////////////////////////////
 
  // Draw out bitmap
  if (m_bLoaded)
  {
    if (!bDisabled)
    {
	   m_IL.DrawIndirect(pDC,0,CPoint(6+m_bPushed,6+m_bPushed), CSize(m_bm.bmWidth, m_bm.bmHeight), CPoint(0,0),ILD_NORMAL);
    }
    else
    {
  	  pDC->DrawState(CPoint(6+m_bPushed,6+m_bPushed), CSize(m_bm.bmWidth, m_bm.bmHeight), m_hbmpDisabled, DST_BITMAP | DSS_DISABLED);
    }
  }


  ////////////////////////////////////////
  // Draw out text                      //
  ////////////////////////////////////////
  CFont *pOldFont=pDC->SelectObject(pFont);
  CRect rectText(rectFocus);
  rectFocus.left += m_bm.bmWidth + 2;
 
  CString strCaption;
  GetWindowText(strCaption);
  pDC->SetBkMode(TRANSPARENT);
  pDC->SetBkColor(GetSysColor(COLOR_BTNFACE));

  rectFocus.top++;
  if (ODS_DISABLED & lpDIS->itemState)
  {
    rectFocus.OffsetRect(1,1);
    pDC->SetTextColor(GetSysColor(COLOR_WINDOW));
    pDC->DrawText(strCaption,rectFocus,DT_SINGLELINE|DT_CENTER|DT_VCENTER);

    rectFocus.OffsetRect(-1,-1);
    pDC->SetTextColor(GetSysColor(COLOR_GRAYTEXT));
    pDC->DrawText(strCaption,rectFocus,DT_SINGLELINE|DT_CENTER|DT_VCENTER);
  }
  else
  {
    pDC->SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
    pDC->DrawText(strCaption,rectFocus,DT_SINGLELINE|DT_CENTER|DT_VCENTER);
  }


  CRect rectSplit(btnRect);
  rectSplit.DeflateRect(2, 2);
  rectSplit.right -= nDropBtnWidth;

  ////////////////////////////////////////
  // Drop down split                    //
  ////////////////////////////////////////
  CPen brFace(PS_SOLID,1,GetSysColor(COLOR_3DSHADOW));
  CPen *pOldPen=pDC->SelectObject(&brFace);
  pDC->MoveTo(rectSplit.right+(m_bPushed?1:0), rectSplit.top);
  pDC->LineTo(rectSplit.right+(m_bPushed?1:0), rectSplit.bottom);
  

  CPen brLite(PS_SOLID,1,GetSysColor(COLOR_3DHILIGHT));
  pDC->SelectObject(&brLite);
  pDC->MoveTo(rectSplit.right+1+(m_bPushed?1:0), rectSplit.top);
  pDC->LineTo(rectSplit.right+1+(m_bPushed?1:0), rectSplit.bottom);


  rectSplit.left = rectSplit.right;
  rectSplit.right += nDropBtnWidth;

  CPoint pt(rectSplit.CenterPoint());
  pt += CPoint(m_bPushed, m_bPushed+1);

  CPen penBlack(PS_SOLID, 1, bDisabled ? GetSysColor(COLOR_GRAYTEXT) : GetSysColor(COLOR_WINDOWTEXT));
  pDC->SelectObject(&penBlack);
  DrawArrow(pDC,pt);
  
  ////////////////////////////////////////
  // Drop down state                    //
  ////////////////////////////////////////
  if (m_bMenuPushed && !bDisabled)
  {    
    rectSplit.InflateRect(1, 1);
    rectSplit.left+=2;
    pDC->DrawEdge(rectSplit,BDR_SUNKENOUTER, BF_RECT);
    
  }
	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldFont);
	
}

////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION:	  DrawArrow
//
// DESCRIPTION:	Draws drop down arrow, we could use DrawFrameControl - a bit too 
//              messy
//
// NOTES:			
//
// MAINTENANCE:
// Name:		  Date:	  Version:	Notes:
// NT ALMOND	210100	1.0			  Origin
//
////////////////////////////////////////////////////////////////////////////////
void CCoolBtn::DrawArrow(CDC* pDC,CPoint ArrowTip)
{
  if (m_bMenuPushed)
	  ArrowTip.Offset(CSize(1,1));
  CPoint ptDest;

  CPen* pPen = pDC->GetCurrentPen();
  LOGPEN logPen;
  pPen->GetLogPen(&logPen);
  pDC->SetPixel(ArrowTip, logPen.lopnColor);

  ArrowTip -= CPoint(1,1);
  pDC->MoveTo(ArrowTip);
  
  ptDest = ArrowTip;
  ptDest += CPoint(3,0);
  pDC->LineTo(ptDest);

  ArrowTip -= CPoint(1,1);
  pDC->MoveTo(ArrowTip);

  ptDest = ArrowTip;
  ptDest += CPoint(5,0);
  pDC->LineTo(ptDest);

  ArrowTip -= CPoint(1,1);
  pDC->MoveTo(ArrowTip);

  ptDest = ArrowTip;
  ptDest += CPoint(7,0);
  pDC->LineTo(ptDest);
}


////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION:	  OnLButtonDown
//
// DESCRIPTION:	handles button pressed state, including drop down menu
//              
//
// NOTES:			
//
// MAINTENANCE:
// Name:		  Date:	  Version:	Notes:
// NT ALMOND	210100	1.0			  Origin
//
////////////////////////////////////////////////////////////////////////////////
void CCoolBtn::OnLButtonDown(UINT nFlags, CPoint point) 
{
	
	if (m_bMenuPushed)
	{
		m_bMenuPushed = FALSE;
		Invalidate();
		return;
		
	}
	
	if (HitMenuBtn(point))
	{
		m_bMenuPushed = TRUE;
		Invalidate();
		
		CRect rc;
		GetWindowRect(rc);
		
		int x = rc.left;
		int y = rc.bottom;
		
		m_menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, x, y, this);

		m_bMenuPushed=FALSE;
	}
	else
	{
		m_bPushed = TRUE;
	}
	
	Invalidate();
	
	if (m_bPushed)
		CButton::OnLButtonDown(nFlags, point);
}

////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION:	  OnLButtonUp
//
// DESCRIPTION:	Redraws button in normal state
//              
//
// NOTES:			
//
// MAINTENANCE:
// Name:		  Date:	  Version:	Notes:
// NT ALMOND	210100	1.0			  Origin
//
////////////////////////////////////////////////////////////////////////////////
void CCoolBtn::OnLButtonUp(UINT nFlags, CPoint point) 
{
	m_bMouseMovedOut=FALSE;
	if (m_bMenuPushed)
	{
		m_bMenuPushed = FALSE;
		Invalidate();
		return;
	}
	if (m_bPushed)
	{
		m_bPushed = FALSE;
		Invalidate();
	}
	
	CButton::OnLButtonUp(nFlags, point);
}

////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION:	  OnMouseMove
//
// DESCRIPTION:	Tracks mouse whilst pressed
//              
//
// NOTES:			
//
// MAINTENANCE:
// Name:		  Date:	  Version:	Notes:
// NT ALMOND	210100	1.0			  Origin
//
////////////////////////////////////////////////////////////////////////////////
void CCoolBtn::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_bPushed)
	{
		ClientToScreen(&point);
		
		if (WindowFromPoint(point) != this)
		{
			m_bPushed = FALSE;
			m_bMouseMovedOut = TRUE;
			Invalidate();
		}
	}
	else if (m_bMouseMovedOut)
	{
		ClientToScreen(&point);
		
		if (WindowFromPoint(point) == this)
		{
			m_bPushed = TRUE;
			m_bMouseMovedOut = FALSE;
			Invalidate();
		}
	}
	else
	{
		if (m_bHovering && !m_bHoveringDropdown)
		{
			if (HitMenuBtn(point))
			{
				m_bHovering = FALSE;
				m_bHoveringDropdown = TRUE;
				Invalidate(FALSE);
			}
		}
		else if (!m_bHovering && m_bHoveringDropdown)
		{
			if (!HitMenuBtn(point))
			{
				m_bHovering = TRUE;
				m_bHoveringDropdown = FALSE;
				Invalidate(FALSE);
			}
		}
		else if(!m_bHovering && !HitMenuBtn(point))
		{
			m_bHovering = TRUE;
			
			Invalidate(FALSE);
			
			TRACKMOUSEEVENT tmEvent;
			
			tmEvent.cbSize = sizeof(tmEvent);
			tmEvent.dwFlags = TME_LEAVE;
			tmEvent.hwndTrack = m_hWnd;
			
			::_TrackMouseEvent(&tmEvent);
		}
		else if(!m_bHoveringDropdown && HitMenuBtn(point))
		{
			m_bHoveringDropdown = TRUE;
			
			Invalidate(FALSE);
			
			TRACKMOUSEEVENT tmEvent;
			
			tmEvent.cbSize = sizeof(tmEvent);
			tmEvent.dwFlags = TME_LEAVE;
			tmEvent.hwndTrack = m_hWnd;
			
			::_TrackMouseEvent(&tmEvent);
		}
	}
	
	CButton::OnMouseMove(nFlags, point);
}

////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION:	  SetButtonImage
//
// DESCRIPTION:	Sets the button image, COLORREF crMask specifics the transparency
//              color              
//
// NOTES:			
//
// MAINTENANCE:
// Name:		  Date:	  Version:	Notes:
// NT ALMOND	210100	1.0			  Origin
//
////////////////////////////////////////////////////////////////////////////////
BOOL CCoolBtn::SetButtonImage(UINT nResourceId, COLORREF crMask)
{
  // The ID must exist also as a bitmap resource!!!
  m_btnImage.LoadBitmap(nResourceId);  
  m_btnImage.GetObject(sizeof m_bm, &m_bm);
  m_IL.Create( nResourceId, m_bm.bmWidth, 1, crMask );
  m_bLoaded = TRUE;
  m_crMask = crMask;

  HBITMAP bmTemp;
  COLORMAP mapColor;
  mapColor.from = crMask;
  mapColor.to  = RGB(255,255,255);

  bmTemp = (HBITMAP)::CreateMappedBitmap(AfxGetApp()->m_hInstance, nResourceId, IMAGE_BITMAP, &mapColor, 1);
  m_hbmpDisabled = (HBITMAP)::CopyImage(bmTemp, IMAGE_BITMAP, m_bm.bmWidth, m_bm.bmHeight, LR_COPYDELETEORG);

  return m_bLoaded;
}

void CCoolBtn::OnSetFocus(CWnd* pOldWnd) 
{
	CButton::OnSetFocus(pOldWnd);
	
	Invalidate();
	
}

void CCoolBtn::OnKillFocus(CWnd* pNewWnd) 
{
	CButton::OnKillFocus(pNewWnd);
		

}

////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION:	  OnSysColorChange
//
// DESCRIPTION:	Called when system colors change, force a button redraw
//
// NOTES:			
//
// MAINTENANCE:
// Name:		  Date:	  Version:	Notes:
// NT ALMOND	210100	1.0			  Origin
//
////////////////////////////////////////////////////////////////////////////////
void CCoolBtn::OnSysColorChange() 
{
	CButton::OnSysColorChange();
  Invalidate();	
}
 
////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION:	  HitMenuBtn
//
// DESCRIPTION:	Helper function to test for menu button hit...
//
// NOTES:			
//
// MAINTENANCE:
// Name:		  Date:	  Version:	Notes:
// NT ALMOND	210100	1.0			  Origin
//
////////////////////////////////////////////////////////////////////////////////
BOOL CCoolBtn::HitMenuBtn(CPoint point)
{
  if (!m_bMenuLoaded)
    return FALSE; // Don't allow menu button drop down if no menu items are loaded
  
  ClientToScreen(&point);

  CRect rect;
  GetWindowRect(rect);
  rect.left = rect.right - nDropBtnWidth;

  return rect.PtInRect(point);    
}


////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION:	  AddMenuItem
//
// DESCRIPTION:	Adds a menu item and id to our menu.
//
// NOTES:			
//
// MAINTENANCE:
// Name:		  Date:	  Version:	Notes:
// NT ALMOND	210100	1.0			  Origin
//
////////////////////////////////////////////////////////////////////////////////
BOOL CCoolBtn::AddMenuItem(UINT nMenuId,const CString strMenu, UINT nMenuFlags)
{
  BOOL bRet = m_menu.AppendMenu(nMenuFlags | MF_STRING, nMenuId, (LPCTSTR)strMenu);
  
  m_bMenuLoaded |= bRet;
  
  return bRet;
}

////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION:	  AddMenuItem
//
// DESCRIPTION:	Adds a menu item and id to our menu.
//
// NOTES:			
//
// MAINTENANCE:
// Name:		  Date:	  Version:	Notes:
// NT ALMOND	210100	1.0			  Origin
//
////////////////////////////////////////////////////////////////////////////////
BOOL CCoolBtn::AddMenuItem(UINT nMenuId, UINT nMenuFlags)
{
	CString str;
	if (nMenuId)
		str.LoadString(nMenuId);
	BOOL bRet = m_menu.AppendMenu(nMenuFlags | MF_STRING, nMenuId, str);
	
	m_bMenuLoaded |= bRet;
	
	return bRet;
}

void CCoolBtn::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	OnLButtonDown(nFlags, point);	
	CButton::OnLButtonDblClk(nFlags, point);
}

BOOL CCoolBtn::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	m_bMenuPushed = FALSE;
	
	return m_pParentWnd->SendMessage(WM_COMMAND, wParam, lParam);
}

BOOL CCoolBtn::RemoveMenuItem(UINT nIndex)
{
	return m_menu.RemoveMenu(nIndex, MF_BYPOSITION);
}

BOOL CCoolBtn::InsertMenuItem(UINT nIndex, UINT nMenuId, UINT nMenuFlags)
{
	CString str;
	if (nMenuId)
		str.LoadString(nMenuId);
	BOOL bRet = m_menu.InsertMenu(nIndex, nMenuFlags | MF_BYPOSITION | MF_STRING, nMenuId, str);
	
	m_bMenuLoaded |= bRet;
	
	return bRet;
}


BOOL CCoolBtn::InsertMenuItem(UINT nIndex, UINT nMenuId, const CString strMenu, UINT nMenuFlags)
{
	CString str;
	BOOL bRet = m_menu.InsertMenu(nIndex, nMenuFlags | MF_BYPOSITION | MF_STRING, nMenuId, strMenu);
	
	m_bMenuLoaded |= bRet;
	
	return bRet;
}

LRESULT CCoolBtn::OnMouseLeave(WPARAM, LPARAM)
{
	m_bHovering = FALSE;
	m_bHoveringDropdown = FALSE;

	Invalidate(FALSE);

	return 0;
}
