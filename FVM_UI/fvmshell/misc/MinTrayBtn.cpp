// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.


//  Based upon 
//  CDialogMinTrayBtn template class
//  MFC CDialog with minimize to systemtray button (0.04a)
//  Supports WinXP styles (thanks to David Yuheng Zhao for CVisualStylesXP - yuheng_zhao@yahoo.com)
//
// ------------------------------------------------------------
//  DialogMinTrayBtn.hpp
//  zegzav - 2002,2003 - eMule project (http://www.emule-project.net)
// ------------------------------------------------------------
//
//  Modified by Tim Kosse (mailto:tim.kosse@gmx.de) for use within FileZilla

#include "StdAfx.h";
#include "MinTrayBtn.h"
#include "VisualStylesXP.h"

IMPLEMENT_DYNCREATE(CMinTrayBtn, CHookWnd);

// ------------------------------
//  constants
// ------------------------------

#define CAPTION_BUTTONSPACE      (2)
#define CAPTION_MINHEIGHT        (8)

#define TIMERMINTRAYBTN_ID       0x76617a67
#define TIMERMINTRAYBTN_PERIOD   200    // ms

#define WP_TRAYBUTTON WP_MINBUTTON

BEGIN_TM_PART_STATES(TRAYBUTTON)
    TM_STATE(1, TRAYBS, NORMAL)
    TM_STATE(2, TRAYBS, HOT)
    TM_STATE(3, TRAYBS, PUSHED)
    TM_STATE(4, TRAYBS, DISABLED)
	// Inactive
    TM_STATE(5, TRAYBS, INORMAL)	
    TM_STATE(6, TRAYBS, IHOT)
    TM_STATE(7, TRAYBS, IPUSHED)
    TM_STATE(8, TRAYBS, IDISABLED)
END_TM_PART_STATES()

#define BMP_TRAYBTN_WIDTH		(21)
#define BMP_TRAYBTN_HEIGHT		(21)
#define BMP_TRAYBTN_BLUE		_T("IDB_LUNA_BLUE")
#define BMP_TRAYBTN_METALLIC	_T("IDB_LUNA_METALLIC")
#define BMP_TRAYBTN_HOMESTEAD	_T("IDB_LUNA_HOMESTEAD")
#define BMP_TRAYBTN_TRANSCOLOR	(RGB(255,0,255))

LPCTSTR CMinTrayBtn::m_pszMinTrayBtnBmpName[] = { BMP_TRAYBTN_BLUE, BMP_TRAYBTN_METALLIC, BMP_TRAYBTN_HOMESTEAD };

#define VISUALSTYLESXP_DEFAULTFILE		L"LUNA.MSSTYLES"
#define VISUALSTYLESXP_BLUE				0
#define VISUALSTYLESXP_METALLIC			1
#define VISUALSTYLESXP_HOMESTEAD		2
#define VISUALSTYLESXP_NAMEBLUE			L"NORMALCOLOR"
#define VISUALSTYLESXP_NAMEMETALLIC		L"METALLIC"
#define VISUALSTYLESXP_NAMEHOMESTEAD	L"HOMESTEAD"

// _WIN32_WINNT >= 0x0501 (XP only)
#define _WM_THEMECHANGED                0x031A	
#define _ON_WM_THEMECHANGED()														\
	{	_WM_THEMECHANGED, 0, 0, 0, AfxSig_l,										\
		(AFX_PMSG)(AFX_PMSGW)														\
		(static_cast< LRESULT (AFX_MSG_CALL CWnd::*)(void) > (_OnThemeChanged))		\
	},

// _WIN32_WINDOWS >= 0x0410 (95 not supported)
BOOL (WINAPI *CMinTrayBtn::_TransparentBlt)(HDC, int, int, int, int, HDC, int, int, int, int, UINT)= NULL;


// ------------------------------
//  contructor/init
// ------------------------------

CMinTrayBtn::CMinTrayBtn(CWnd* pParentWnd) : CHookWnd(FALSE),
	m_MinTrayBtnPos(0,0), m_MinTrayBtnSize(0,0), m_bMinTrayBtnEnabled(TRUE), m_bMinTrayBtnVisible(TRUE), 
    m_bMinTrayBtnUp(TRUE), m_bMinTrayBtnCapture(FALSE), m_bMinTrayBtnActive(FALSE), m_bMinTrayBtnHitTest(FALSE)
{
	ASSERT(pParentWnd);
	m_pOwner = pParentWnd;

	m_hDLL = NULL;

    MinTrayBtnInit();
}

CMinTrayBtn::~CMinTrayBtn()
{
	if (m_hDLL)
		FreeLibrary(m_hDLL);
}

void CMinTrayBtn::MinTrayBtnInit()
{
    m_nMinTrayBtnTimerId = 0;
	MinTrayBtnInitBitmap();
	if (!_TransparentBlt)
	{
		m_hDLL = LoadLibrary(_T("MSIMG32.DLL"));
		if (m_hDLL)
		{
			(FARPROC &)_TransparentBlt = GetProcAddress(m_hDLL, "TransparentBlt");
			if (!_TransparentBlt)
			{
				FreeLibrary(m_hDLL);
				m_hDLL = NULL;
			}
		}
	}
}

// ------------------------------
//  messages
// ------------------------------

void CMinTrayBtn::OnNcPaint() 
{
    MinTrayBtnUpdatePosAndSize();
    MinTrayBtnDraw();
}

BOOL CMinTrayBtn::OnNcActivate(BOOL bActive)
{
    MinTrayBtnUpdatePosAndSize();
    m_bMinTrayBtnActive = bActive;
    MinTrayBtnDraw();
    return TRUE;
}

UINT CMinTrayBtn::OnNcHitTest(CPoint point)
{
    BOOL bPreviousHitTest= m_bMinTrayBtnHitTest;
    m_bMinTrayBtnHitTest= MinTrayBtnHitTest(point);
    if ((!IsWindowsClassicStyle()) && (m_bMinTrayBtnHitTest != bPreviousHitTest))
        MinTrayBtnDraw(); // Windows XP Style (hot button)
    if (m_bMinTrayBtnHitTest)
       return HTMINTRAYBUTTON;

	return HTERROR;
}

BOOL CMinTrayBtn::OnNcLButtonDown(UINT nHitTest, CPoint point) 
{
    if ((m_pOwner->GetStyle() & WS_DISABLED) || (!MinTrayBtnIsEnabled()) || (!MinTrayBtnIsVisible()) || (!MinTrayBtnHitTest(point)))
    {
        return FALSE;
    }

    m_pOwner->SetCapture();
    m_bMinTrayBtnCapture = TRUE;
    MinTrayBtnSetDown();

	return TRUE;
}

BOOL CMinTrayBtn::OnMouseMove(UINT nFlags, CPoint point) 
{
    if ((m_pOwner->GetStyle() & WS_DISABLED) || (!m_bMinTrayBtnCapture))
		return FALSE;

    m_pOwner->ClientToScreen(&point);
    m_bMinTrayBtnHitTest= MinTrayBtnHitTest(point);
    if (m_bMinTrayBtnHitTest)
    {
        if (m_bMinTrayBtnUp)
            MinTrayBtnSetDown();
    }
    else
    {
        if (!m_bMinTrayBtnUp)
            MinTrayBtnSetUp();
    }

	return TRUE;
}

BOOL CMinTrayBtn::OnLButtonUp(UINT nFlags, CPoint point) 
{
    if ((m_pOwner->GetStyle() & WS_DISABLED) || (!m_bMinTrayBtnCapture))
		return FALSE;

    ReleaseCapture();
    m_bMinTrayBtnCapture = FALSE;
    MinTrayBtnSetUp();

    m_pOwner->ClientToScreen(&point);
    if (MinTrayBtnHitTest(point))
       m_pOwner->SendMessage(WM_SYSCOMMAND, SC_MINIMIZETRAY, MAKELONG(point.x, point.y));

	return TRUE;
}

void CMinTrayBtn::OnThemeChanged()
{
	MinTrayBtnInitBitmap();
}

// ------------------------------
//  methods
// ------------------------------

void CMinTrayBtn::MinTrayBtnUpdatePosAndSize()
{
    DWORD dwStyle = m_pOwner->GetStyle();
    DWORD dwExStyle = m_pOwner->GetExStyle();

    INT caption= ((dwExStyle & WS_EX_TOOLWINDOW) == 0) ? GetSystemMetrics(SM_CYCAPTION) - 1 : GetSystemMetrics(SM_CYSMCAPTION) - 1;
    if (caption < CAPTION_MINHEIGHT)
       caption= CAPTION_MINHEIGHT;

    CSize borderfixed(-GetSystemMetrics(SM_CXFIXEDFRAME), GetSystemMetrics(SM_CYFIXEDFRAME));
    CSize bordersize(-GetSystemMetrics(SM_CXSIZEFRAME), GetSystemMetrics(SM_CYSIZEFRAME));

    CRect window;
    m_pOwner->GetWindowRect(&window);

    CSize button;
    button.cy= caption - (CAPTION_BUTTONSPACE * 2);
    button.cx= button.cy;
    if (IsWindowsClassicStyle())
        button.cx+= 2;

    m_MinTrayBtnSize = button;

    m_MinTrayBtnPos.x = window.Width() - ((CAPTION_BUTTONSPACE + button.cx) * 2);
    m_MinTrayBtnPos.y = CAPTION_BUTTONSPACE;

    if ((dwStyle & WS_THICKFRAME) != 0)
    {
        // resizable window
        m_MinTrayBtnPos+= bordersize;
    }
    else
    {
        // fixed window
        m_MinTrayBtnPos+= borderfixed;
    }

    if ( ((dwExStyle & WS_EX_TOOLWINDOW) == 0) && (((dwStyle & WS_MINIMIZEBOX) != 0) || ((dwStyle & WS_MAXIMIZEBOX) != 0)) )
    {
        if (IsWindowsClassicStyle())
            m_MinTrayBtnPos.x-= (button.cx * 2) + CAPTION_BUTTONSPACE;
        else
            m_MinTrayBtnPos.x-= (button.cx + CAPTION_BUTTONSPACE) * 2;
    }
       
}

void CMinTrayBtn::MinTrayBtnShow()
{
    if (MinTrayBtnIsVisible())
       return;

    m_bMinTrayBtnVisible = TRUE;
    if (m_pOwner->IsWindowVisible())
    {
        m_pOwner->RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);
    }
}

void CMinTrayBtn::MinTrayBtnHide()
{
    if (!MinTrayBtnIsVisible())
       return;

    m_bMinTrayBtnVisible= FALSE;
    if (m_pOwner->IsWindowVisible())
    {
        m_pOwner->RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);
    }
}

void CMinTrayBtn::MinTrayBtnEnable()
{
    if (MinTrayBtnIsEnabled())
       return;

    m_bMinTrayBtnEnabled= TRUE;
    MinTrayBtnSetUp();
}

void CMinTrayBtn::MinTrayBtnDisable()
{
    if (!MinTrayBtnIsEnabled())
       return;

    m_bMinTrayBtnEnabled= FALSE;
    if (m_bMinTrayBtnCapture)
    {
       ReleaseCapture();
       m_bMinTrayBtnCapture= FALSE;
    }
    MinTrayBtnSetUp();
}

void CMinTrayBtn::MinTrayBtnDraw()
{
    if (!MinTrayBtnIsVisible())
       return;

    CDC *pDC = m_pOwner->GetWindowDC();
    if (!pDC)
       return; // panic!

    if (IsWindowsClassicStyle())
    {
        CBrush black(GetSysColor(COLOR_BTNTEXT));
        CBrush gray(GetSysColor(COLOR_GRAYTEXT));
        CBrush gray2(GetSysColor(COLOR_BTNHILIGHT));

        // button
        if (m_bMinTrayBtnUp)
           pDC->DrawFrameControl(MinTrayBtnGetRect(), DFC_BUTTON, DFCS_BUTTONPUSH);
        else
           pDC->DrawFrameControl(MinTrayBtnGetRect(), DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_PUSHED);

        // dot
        CRect btn= MinTrayBtnGetRect();
        btn.DeflateRect(2,2);
        UINT caption= MinTrayBtnGetSize().cy + (CAPTION_BUTTONSPACE * 2);
        UINT pixratio= (caption >= 14) ? ((caption >= 20) ? 2 + ((caption - 20) / 8) : 2) : 1;
        UINT pixratio2= (caption >= 12) ? 1 + (caption - 12) / 8: 0;
        UINT dotwidth= (1 + pixratio * 3) >> 1;
        UINT dotheight= pixratio;
        CRect dot(CPoint(0,0), CPoint(dotwidth, dotheight));
        CSize spc((1 + pixratio2 * 3) >> 1, pixratio2);
        dot-= dot.Size();
        dot+= btn.BottomRight();
        dot-= spc;
        if (!m_bMinTrayBtnUp)
           dot+= CPoint(1,1);
        if (m_bMinTrayBtnEnabled)
        {
           pDC->FillRect(dot, &black);
        }
        else
        {
           pDC->FillRect(dot + CPoint(1,1), &gray2);
           pDC->FillRect(dot, &gray);
        }
    }
    else
    {
		// VisualStylesXP
		CRect btn = MinTrayBtnGetRect();
		int iState;
		if (!m_bMinTrayBtnEnabled)
			iState= TRAYBS_DISABLED;
        else if (m_pOwner->GetStyle() & WS_DISABLED)
			iState= MINBS_NORMAL;
		else if (m_bMinTrayBtnHitTest)
			iState= (m_bMinTrayBtnCapture) ? MINBS_PUSHED : MINBS_HOT;
        else
			iState= MINBS_NORMAL;
		// inactive
		if (!m_bMinTrayBtnActive)
			iState+= 4; // inactive state TRAYBS_Ixxx

		if ((m_bmMinTrayBtnBitmap.m_hObject) && (_TransparentBlt))
		{
			// known theme (bitmap)
			CBitmap *pBmpOld;
			CDC dcMem;
			if ((dcMem.CreateCompatibleDC(pDC)) && ((pBmpOld= dcMem.SelectObject(&m_bmMinTrayBtnBitmap)) != NULL))
            {
				_TransparentBlt(pDC->m_hDC, btn.left, btn.top, btn.Width(), btn.Height(), dcMem.m_hDC, 0, BMP_TRAYBTN_HEIGHT * (iState - 1), BMP_TRAYBTN_WIDTH, BMP_TRAYBTN_HEIGHT, BMP_TRAYBTN_TRANSCOLOR);
				dcMem.SelectObject(pBmpOld);
            }
         }
         else
         {
			// unknown theme (ThemeData)
			HTHEME hTheme= g_xpStyle.OpenThemeData(m_pOwner->GetSafeHwnd(), L"Window");
			if (hTheme)
			{
				btn.top+= btn.Height() / 8;
				g_xpStyle.DrawThemeBackground(hTheme, pDC->m_hDC, WP_TRAYBUTTON, iState, &btn, NULL);
				g_xpStyle.CloseThemeData(hTheme);
            }
        }
    }

    m_pOwner->ReleaseDC(pDC);
}

BOOL CMinTrayBtn::MinTrayBtnHitTest(CPoint point) const
{
    CRect rWnd;
    m_pOwner->GetWindowRect(&rWnd);
    point.Offset(-rWnd.TopLeft());
    CRect rBtn= MinTrayBtnGetRect();
    rBtn.InflateRect(0, CAPTION_BUTTONSPACE);
    return (rBtn.PtInRect(point));
}

void CMinTrayBtn::MinTrayBtnSetUp()
{
    m_bMinTrayBtnUp= TRUE;
    MinTrayBtnDraw();
}

void CMinTrayBtn::MinTrayBtnSetDown()
{
    m_bMinTrayBtnUp = FALSE;
    MinTrayBtnDraw();
}

BOOL CMinTrayBtn::IsWindowsClassicStyle() const
{
    return (!((g_xpStyle.IsThemeActive()) && (g_xpStyle.IsAppThemed())));
}

INT CMinTrayBtn::GetVisualStylesXPColor() const
{
	if (IsWindowsClassicStyle())
		return -1;

	WCHAR szwThemeFile[MAX_PATH];
	WCHAR szwThemeColor[256];
	if (g_xpStyle.GetCurrentThemeName(szwThemeFile, MAX_PATH, szwThemeColor, 256, NULL, 0) != S_OK)
		return -1;
	WCHAR *p;
	if ((p= wcsrchr(szwThemeFile, '\\')) == NULL)
		return -1;
	p++;
	if (_wcsicmp(p, VISUALSTYLESXP_DEFAULTFILE) != 0)
		return -1;
	if (_wcsicmp(szwThemeColor, VISUALSTYLESXP_NAMEBLUE) == 0)
		return VISUALSTYLESXP_BLUE;
	if (_wcsicmp(szwThemeColor, VISUALSTYLESXP_NAMEMETALLIC) == 0)
		return VISUALSTYLESXP_METALLIC;
	if (_wcsicmp(szwThemeColor, VISUALSTYLESXP_NAMEHOMESTEAD) == 0)
		return VISUALSTYLESXP_HOMESTEAD;
	return -1;
}

BOOL CMinTrayBtn::MinTrayBtnInitBitmap()
{
	INT nColor;
	m_bmMinTrayBtnBitmap.DeleteObject();
	if ((nColor= GetVisualStylesXPColor()) == -1)
		return FALSE;
	const TCHAR *pszBmpName= m_pszMinTrayBtnBmpName[nColor];
	BOOL res = m_bmMinTrayBtnBitmap.Attach(::LoadBitmap(AfxGetInstanceHandle(), pszBmpName));
	return res;
}
#define WM_THEMECHANGED 0x031A
BOOL CMinTrayBtn::ProcessWindowMessage(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult)
{
	switch (nMsg)
	{
	case WM_NCPAINT:
		lResult = DefWindowProc(nMsg, wParam, lParam);
		OnNcPaint();
		return TRUE;
	case WM_SETTEXT:
		lResult = DefWindowProc(nMsg, wParam, lParam);
		MinTrayBtnDraw();
		return TRUE;
	case WM_NCACTIVATE:
		lResult = DefWindowProc(nMsg, wParam, lParam);
		OnNcActivate(wParam);
		return TRUE;
	case WM_NCHITTEST:
		{
			lResult = (INT)OnNcHitTest(CPoint(lParam));
			if (lResult == HTERROR)
				lResult = DefWindowProc(nMsg, wParam, lParam);
		}
		return TRUE;
	case WM_NCLBUTTONDOWN:
		if (!OnNcLButtonDown(wParam, CPoint(lParam)))
			lResult = DefWindowProc(nMsg, wParam, lParam);
		else
			lResult = 0;
		return TRUE;
	case WM_THEMECHANGED:
		OnThemeChanged();
		break;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, CPoint(lParam));
		break;
	case WM_LBUTTONUP:
		if (OnLButtonUp(wParam, CPoint(lParam)))
			lResult = 0;
		else
			lResult = DefWindowProc(nMsg, wParam, lParam);
		return TRUE;
	}
	return FALSE;
}
