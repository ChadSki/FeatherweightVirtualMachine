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
//  DialogMinTrayBtn.h
//  zegzav - 2002,2003 - eMule project (http://www.emule-project.net)
// ------------------------------------------------------------
//
//  Modified by Tim Kosse (mailto:tim.kosse@gmx.de) for use within FileZilla
//  This class is now derived from the CHookWnd class from PJ Naugher and can be used dynamically
//  by any window.

#include "hookwnd.h"

#pragma once

#define HTMINTRAYBUTTON         65
#define SC_MINIMIZETRAY         0xE000

class CMinTrayBtn : public CHookWnd
{
	DECLARE_DYNCREATE(CMinTrayBtn);

public:
    // constructor
    CMinTrayBtn(CWnd* pParentWnd = NULL);

    virtual ~CMinTrayBtn();

    virtual BOOL ProcessWindowMessage(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult);

    // methods
    void MinTrayBtnShow();
    void MinTrayBtnHide();
    BOOL MinTrayBtnIsVisible() const;
    void MinTrayBtnEnable();
    void MinTrayBtnDisable();
    BOOL MinTrayBtnIsEnabled() const;
	void MinTrayBtnDraw();

private:
    // internal methods
    void MinTrayBtnInit();
    BOOL MinTrayBtnHitTest(CPoint point) const;
    void MinTrayBtnUpdatePosAndSize();

    void MinTrayBtnSetUp();
    void MinTrayBtnSetDown();

    const CPoint &MinTrayBtnGetPos() const;
    const CSize &MinTrayBtnGetSize() const;
    CRect MinTrayBtnGetRect() const;

    BOOL IsWindowsClassicStyle() const;
	INT GetVisualStylesXPColor() const;

	BOOL MinTrayBtnInitBitmap();

	// Message handlers
	void OnNcPaint();
    BOOL OnNcActivate(BOOL bActive);
    UINT OnNcHitTest(CPoint point);
    BOOL OnNcLButtonDown(UINT nHitTest, CPoint point);
    BOOL OnMouseMove(UINT nFlags, CPoint point);
    BOOL OnLButtonUp(UINT nFlags, CPoint point);
	void OnThemeChanged();

    // data members
	CWnd	*m_pOwner;
    CPoint m_MinTrayBtnPos;
    CSize  m_MinTrayBtnSize;
    BOOL   m_bMinTrayBtnVisible; 
    BOOL   m_bMinTrayBtnEnabled; 
    BOOL   m_bMinTrayBtnUp;
    BOOL   m_bMinTrayBtnCapture;
    BOOL   m_bMinTrayBtnActive;
    BOOL   m_bMinTrayBtnHitTest;
    UINT_PTR m_nMinTrayBtnTimerId;
	CBitmap m_bmMinTrayBtnBitmap;
	static LPCTSTR m_pszMinTrayBtnBmpName[];

	HMODULE m_hDLL;
	static BOOL (WINAPI *_TransparentBlt)(HDC, int, int, int, int, HDC, int, int, int, int, UINT);
};

inline const CPoint &CMinTrayBtn::MinTrayBtnGetPos() const
{
    return m_MinTrayBtnPos;
}

inline const CSize &CMinTrayBtn::MinTrayBtnGetSize() const
{
    return m_MinTrayBtnSize;
}

inline CRect CMinTrayBtn::MinTrayBtnGetRect() const
{
    return CRect(MinTrayBtnGetPos(), MinTrayBtnGetSize());
}

inline BOOL CMinTrayBtn::MinTrayBtnIsVisible() const
{
    return m_bMinTrayBtnVisible;
}

inline BOOL CMinTrayBtn::MinTrayBtnIsEnabled() const
{
    return m_bMinTrayBtnEnabled;
}
