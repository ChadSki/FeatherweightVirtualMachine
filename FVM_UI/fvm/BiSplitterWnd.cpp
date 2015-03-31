// BiSplitterWnd.cpp : implementation file
//

#include "stdafx.h"
#include "afxpriv.h"
#include "BiSplitterWnd.h"

//TODO: check styles during pane and splitter wnd construction
//TODO: add function for view delete

/***************************************************************************************************************/
/** follow definitions were copied from file \Microsoft Visual Studio .NET 2003\Vc7\atlmfc\src\mfc\afximpl.h  **/
/**/ 
/**/ #ifndef _UNICODE
/**/ #define _UNICODE_SUFFIX
/**/ #else
/**/ #define _UNICODE_SUFFIX _T("u")
/**/ #endif
/**/ 
/**/ #ifndef _DEBUG
/**/ #define _DEBUG_SUFFIX
/**/ #else
/**/ #define _DEBUG_SUFFIX _T("d")
/**/ #endif
/**/ 
/**/ #ifdef _AFXDLL
/**/ #define _STATIC_SUFFIX
/**/ #else
/**/ #define _STATIC_SUFFIX _T("s")
/**/ #endif

/**/ #if (_MSC_VER == 1200) /* VS 6 */
/**/ #define AFX_WNDCLASS(s) _T("Afx") _T(s) _T("42") _STATIC_SUFFIX _UNICODE_SUFFIX _DEBUG_SUFFIX
/**/ #elif (_MSC_VER == 1310) /* VS .NET 2003 */
/**/ #define AFX_WNDCLASS(s) _T("Afx") _T(s) _T("70") _STATIC_SUFFIX _UNICODE_SUFFIX _DEBUG_SUFFIX
/**/ #else
/**/ #error Unsupported version of MFC
/**/ #endif
/**/ 
/**/ #define AFX_WNDMDIFRAME_REG                             0x00004
/**/ #define AFX_WNDMDIFRAME     AFX_WNDCLASS("MDIFrame")
/**/ #define AfxDeferRegisterClass(fClass) AfxEndDeferRegisterClass(fClass)
/**/ BOOL AFXAPI AfxEndDeferRegisterClass(LONG fToRegister);
/**/ #define _AfxGetDlgCtrlID(hWnd)          ((UINT)(WORD)::GetDlgCtrlID(hWnd))
/**/ 
/***************************************************************************************************************/

#define IMAGE_GRIP_L 0
#define IMAGE_GRIP_R 1
#define IMAGE_GRIP_T 2
#define IMAGE_GRIP_B 3
#define IMAGE_CLOSE 4


enum HitTestValue
{
	noHit                   = 0,
	splitterBar             = 1,
	closeBtn0				= 2,
	closeBtn1				= 3,
	gripBtn0				= 4,
	gripBtn1				= 5
};


// CBiSplitterWnd

IMPLEMENT_DYNCREATE(CBiSplitterWnd, CWnd)
BEGIN_MESSAGE_MAP(CBiSplitterWnd, CWnd)
	ON_WM_NCCREATE()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_WM_SETCURSOR()
	ON_WM_PAINT()
	ON_WM_CAPTURECHANGED()
	ON_WM_CANCELMODE()
	ON_MESSAGE_VOID(WM_DISPLAYCHANGE, OnDisplayChange)
	ON_MESSAGE_VOID(WM_WININICHANGE, OnDisplayChange)
	ON_MESSAGE_VOID(WM_SETTINGCHANGE, OnDisplayChange)
//	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

//*** Construction ***
CBiSplitterWnd::CBiSplitterWnd():
		m_trackingState(NOTRACKING),
        m_nGap(3),
        m_nSplitterPos(0),
        m_nSplitterDenominator(0),
		m_nSplitterNumerator(0),
		m_lastActivePane(-1),
		m_isTrackMouseEvent(FALSE),
		m_clrFlatBorder(RGB(0,0,0)),
        m_clrFlatBtnActiveFace(RGB(182,189,210)),
		m_clrFlatBtnActiveBorder(RGB(10,36,106)),
		m_pGlyphs(NULL),
		m_pSmGlyphs(NULL),
		m_glyphsCount(5),
		m_autoDelete(FALSE)
{
	
	m_images = new TCHAR[m_glyphsCount];
	m_images[0] = _T('\x33');
	m_images[1] = _T('\x34');
	m_images[2] = _T('\x36');
	m_images[3] = _T('\x35');
	m_images[4] = _T('\x72');


	m_pGlyphs = new GLYPHINFO[m_glyphsCount];
	m_pSmGlyphs = new GLYPHINFO[m_glyphsCount];
	
	for(int i = 0; i < m_glyphsCount; i++)
	{
		m_pGlyphs[i].pBitmapInfo = (BITMAPINFO*)(new BYTE[sizeof(BITMAPINFOHEADER) + 2 * sizeof(RGBQUAD)]);
		m_pGlyphs[i].pBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		m_pGlyphs[i].pBitmapInfo->bmiHeader.biPlanes = 1;
		m_pGlyphs[i].pBitmapInfo->bmiHeader.biBitCount = 1;
		m_pGlyphs[i].pBitmapInfo->bmiHeader.biCompression = BI_RGB;
	
		m_pGlyphs[i].pBitmapInfo->bmiColors[0].rgbRed = 255;	
		m_pGlyphs[i].pBitmapInfo->bmiColors[0].rgbGreen = 255;
		m_pGlyphs[i].pBitmapInfo->bmiColors[0].rgbBlue = 255;
		m_pGlyphs[i].pBitmapInfo->bmiColors[0].rgbReserved = 0;

		m_pGlyphs[i].pBitmapInfo->bmiColors[1].rgbRed = 0;	
		m_pGlyphs[i].pBitmapInfo->bmiColors[1].rgbGreen = 0;
		m_pGlyphs[i].pBitmapInfo->bmiColors[1].rgbBlue = 0;
		m_pGlyphs[i].pBitmapInfo->bmiColors[1].rgbReserved = 0;

		m_pGlyphs[i].pBits = 0;


		m_pSmGlyphs[i].pBitmapInfo = (BITMAPINFO*)(new BYTE[sizeof(BITMAPINFOHEADER) + 2 * sizeof(RGBQUAD)]);
		m_pSmGlyphs[i].pBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		m_pSmGlyphs[i].pBitmapInfo->bmiHeader.biPlanes = 1;
		m_pSmGlyphs[i].pBitmapInfo->bmiHeader.biBitCount = 1;
		m_pSmGlyphs[i].pBitmapInfo->bmiHeader.biCompression = BI_RGB;
	
		m_pSmGlyphs[i].pBitmapInfo->bmiColors[0].rgbRed = 255;	
		m_pSmGlyphs[i].pBitmapInfo->bmiColors[0].rgbGreen = 255;
		m_pSmGlyphs[i].pBitmapInfo->bmiColors[0].rgbBlue = 255;
		m_pSmGlyphs[i].pBitmapInfo->bmiColors[0].rgbReserved = 0;

		m_pSmGlyphs[i].pBitmapInfo->bmiColors[1].rgbRed = 0;	
		m_pSmGlyphs[i].pBitmapInfo->bmiColors[1].rgbGreen = 0;
		m_pSmGlyphs[i].pBitmapInfo->bmiColors[1].rgbBlue = 0;
		m_pSmGlyphs[i].pBitmapInfo->bmiColors[1].rgbReserved = 0;

		m_pSmGlyphs[i].pBits = 0;
	}

	UpdateSysMetrics();
	UpdateSysColors();
//	UpdateSysImages();
}

BOOL CBiSplitterWnd::Create(CWnd *pWnd, UINT bswStyles, UINT nID /* = AFX_IDW_PANE_FIRST */)
{
	ASSERT(pWnd != NULL);
	ASSERT(bswStyles & SPLITTER_STYLES);
	ASSERT(bswStyles & SIZING_MODE_STYLES);

	DWORD dwCreateStyle = WS_CHILD;// | WS_VISIBLE;
	VERIFY(AfxDeferRegisterClass(AFX_WNDMDIFRAME_REG));
	m_bswStyles = bswStyles;

	// create with the same wnd-class as MDI-Frame (no erase bkgnd)
	const TCHAR _afxWndMDIFrame[] = AFX_WNDMDIFRAME;
	if (!CreateEx(0, _afxWndMDIFrame, NULL, dwCreateStyle, 0, 0, 0, 0,
	  pWnd->m_hWnd, (HMENU)(UINT_PTR)nID, NULL))
		return FALSE;       // create invisible

	UpdateSysMetrics();
	UpdateSysColors();
	UpdateSysImages();

	//BOOL rc = tip.CreateEx(this,TTS_ALWAYSTIP | WS_POPUP);
	//tip.SetWindowPos(&CWnd::wndTop,0, 0, 0, 0,SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	//tip.AddTool(this,/*LPSTR_TEXTCALLBACK*/ "YYYY" , CRect(0,0,100,100), nID);
	//CToolInfo toolInfo;
	//tip.GetToolInfo(toolInfo, this, nID);
	//toolInfo.uFlags |= TTF_SUBCLASS ;
	//tip.SetToolInfo(&toolInfo);
	//tip.Activate(TRUE);

	//CString str;
	//tip.GetText(str, this, nID);
	return TRUE;
}

BOOL CBiSplitterWnd::CreatePane(UINT nPane, UINT bspStyles, LPCTSTR lpszPaneCaptionText/* = NULL */, CWnd *pWnd/* = NULL*/)
{
    ASSERT(nPane>=0 && nPane<=1);
	ASSERT((bspStyles & BORDER_STYLES) == NOTHING || (bspStyles & BORDER_STYLES) == THIN_BORDER || (bspStyles & BORDER_STYLES) == THICK_BORDER);
	ASSERT((bspStyles & CAPTION_STYLES) == NOTHING || (bspStyles & CAPTION_STYLES) == SMCAPTION || (bspStyles & CAPTION_STYLES) == CAPTION);
	ASSERT_VALID(this);

	CPane &curPane = m_panes[nPane];
	ASSERT( curPane.m_pView == NULL);
	curPane.m_bspStyles = bspStyles;
	curPane.m_sCaptionText = lpszPaneCaptionText;
	if(pWnd != NULL) 
	{
		pWnd->SetWindowPos(&CWnd::wndTop, curPane.m_rctViewBox.left, curPane.m_rctViewBox.top, curPane.m_rctViewBox.Width(), curPane.m_rctViewBox.Height(), 0/*SWP_SHOWWINDOW*/);
		
		if(pWnd->IsKindOf(RUNTIME_CLASS(CView)))
		{
			CFrameWnd* pFrameWnd = GetParentFrame();
			ASSERT_VALID(pFrameWnd);
			pFrameWnd->SetActiveView((CView*)pWnd);
		}
		curPane.m_pView = pWnd;
	}

	if( curPane.IsVisible() && !(GetStyle() & WS_VISIBLE))//BiSplitter window not yet visible
        ShowWindow(SW_SHOW);
	
	SetActivePane(nPane);
	return TRUE;
}

CWnd *CBiSplitterWnd::CreateView(CRuntimeClass* pBiSplitterWndClass, UINT nID, UINT bswStyles)
{
	ASSERT_VALID(this);

	ASSERT(pBiSplitterWndClass != NULL);
	ASSERT(pBiSplitterWndClass->IsDerivedFrom(RUNTIME_CLASS(CBiSplitterWnd)));
	ASSERT(AfxIsValidAddress(pBiSplitterWndClass, sizeof(CRuntimeClass), FALSE));
	ASSERT(bswStyles & SPLITTER_STYLES);
	ASSERT(bswStyles & SIZING_MODE_STYLES);

    CBiSplitterWnd* pBiSplitterWnd;
	pBiSplitterWnd = (CBiSplitterWnd*)pBiSplitterWndClass->CreateObject();
	
	pBiSplitterWnd->m_autoDelete = TRUE;
	
	if (pBiSplitterWnd == NULL)
	{
		AfxThrowMemoryException();
	}

	ASSERT_KINDOF(CBiSplitterWnd, pBiSplitterWnd);
	ASSERT(pBiSplitterWnd->m_hWnd == NULL);       // not yet created

	DWORD dwStyle = WS_CHILD;

	if (!pBiSplitterWnd->Create(this, bswStyles, nID))
	{
//		TRACE(traceAppMsg, 0, _T("Warning: couldn't create subsplitter.\n"));
		// pWnd will be cleaned up by PostNcDestroy
		return NULL;
	}
	ASSERT((int)_AfxGetDlgCtrlID(pBiSplitterWnd->m_hWnd) == nID);

	return pBiSplitterWnd;
}

CWnd *CBiSplitterWnd::CreateView(CRuntimeClass* pWndClass, UINT nID)
{
	ASSERT_VALID(this);
	ASSERT(pWndClass != NULL);
	ASSERT(pWndClass->IsDerivedFrom(RUNTIME_CLASS(CWnd)));
	ASSERT(AfxIsValidAddress(pWndClass, sizeof(CRuntimeClass), FALSE));

    CWnd* pWnd;
	pWnd = (CWnd*)pWndClass->CreateObject();
	if (pWnd == NULL)
	{
		AfxThrowMemoryException();
	}

	ASSERT_KINDOF(CWnd, pWnd);
	ASSERT(pWnd->m_hWnd == NULL);       // not yet created

	DWORD dwStyle = WS_CHILD | WS_VISIBLE;

	// Create with the zero size
	if (!pWnd->Create(NULL, NULL, dwStyle, CRect(0,0,0,0), this, nID, NULL))
	{
//		TRACE(traceAppMsg, 0, _T("Warning: couldn't create pane window for splitter.\n"));
		// pWnd will be cleaned up by PostNcDestroy
		return NULL;
	}
	ASSERT((int)_AfxGetDlgCtrlID(pWnd->m_hWnd) == nID);

	return pWnd;
}

CWnd *CBiSplitterWnd::CreateView(CRuntimeClass* pViewClass, UINT nID, CCreateContext* pContext)
{
	ASSERT_VALID(this);
	ASSERT(pViewClass != NULL);
	ASSERT(pViewClass->IsDerivedFrom(RUNTIME_CLASS(CView)));
	ASSERT(AfxIsValidAddress(pViewClass, sizeof(CRuntimeClass), FALSE));

    CView* pView;
	pView = (CView*)pViewClass->CreateObject();
	if (pView == NULL)
	{
		AfxThrowMemoryException();
	}

	ASSERT_KINDOF(CView, pView);
	ASSERT(pView->m_hWnd == NULL);       // not yet created

	DWORD dwStyle = WS_CHILD | WS_VISIBLE;

	// Create with the zero size
	if (!pView->Create(NULL, NULL, dwStyle, CRect(0,0,0,0), this, nID, pContext))
	{
//		TRACE(traceAppMsg, 0, _T("Warning: couldn't create view for splitter.\n"));
		// pWnd will be cleaned up by PostNcDestroy
		return NULL;
	}
	ASSERT((int)_AfxGetDlgCtrlID(pView->m_hWnd) == nID);

	return pView;
}


//*** Attributes ***
UINT CBiSplitterWnd::GetSplitterGap()
{
	return m_nGap;
}

void CBiSplitterWnd::SetSplitterGap(UINT nGap)
{
	m_nGap=nGap;
	RecalcLayout();
}

int CBiSplitterWnd::GetSplitterPos()
{
	return m_nSplitterPos;
}

void CBiSplitterWnd::SetSplitterPos(int nSplitterPos)
{
	m_nSplitterNumerator = m_nSplitterPos = nSplitterPos;
	CRect rct;
	GetClientRect(&rct);
	if(m_bswStyles & VSPLITTER)
		m_nSplitterDenominator =  rct.Width();
	else
		m_nSplitterDenominator =  rct.Height();
	RecalcLayout();
}

CWnd *CBiSplitterWnd::GetPaneView(UINT nPane)
{
	ASSERT(nPane >= 0 && nPane <= 1);
	return m_panes[nPane].m_pView;
}

UINT CBiSplitterWnd::GetPaneViewID(UINT nPane)
{
	ASSERT(nPane >= 0 && nPane <= 1);
	ASSERT(m_panes[nPane].m_pView != NULL);

	return m_panes[nPane].m_pView->GetDlgCtrlID();
}

CString CBiSplitterWnd::GetPaneCaptionText(UINT nPane)
{
    ASSERT(nPane >= 0 && nPane <= 1);
	return m_panes[nPane].m_sCaptionText;
}

void CBiSplitterWnd::SetPaneCaptionText(UINT nPane, const CString &m_sCaptionText)
{
    ASSERT(nPane >= 0 && nPane <= 1);
	m_panes[nPane].m_sCaptionText=m_sCaptionText;
	RecalcLayout();
}

//*** Operations ***
UINT CBiSplitterWnd::AcquireStyles()
{
	return m_bswStyles;
}

void CBiSplitterWnd::ChangeStyles(UINT excludeStyles, UINT includeStyles)
{
	m_bswStyles &= ~excludeStyles;
	m_bswStyles |= includeStyles;
	RecalcLayout();
}

UINT CBiSplitterWnd::AcquirePaneStyles(UINT nPane)
{
    ASSERT(nPane >= 0 && nPane <= 1);
	return m_panes[nPane].m_bspStyles;
}

void CBiSplitterWnd::ChangePaneStyles(UINT nPane, UINT excludeStyles, UINT includeStyles)
{
    ASSERT(nPane >= 0 && nPane <= 1);
	m_panes[nPane].m_bspStyles &= ~excludeStyles;
	m_panes[nPane].m_bspStyles |= includeStyles;
	RecalcLayout();
}

void CBiSplitterWnd::AssignViewToPane(UINT viewID, UINT nPane)
{
	//TODO: add posibility to assign NULL view
    ASSERT(nPane >= 0 && nPane <= 1);
	if (m_panes[nPane].m_pView)
		m_panes[nPane].m_pView->ShowWindow(SW_HIDE);
	CWnd *pWnd = GetDlgItem(viewID);
	ASSERT(pWnd);
	m_panes[nPane].m_pView = pWnd;
	pWnd->SetWindowPos(NULL, m_panes[nPane].m_rctViewBox.left, m_panes[nPane].m_rctViewBox.top,
		m_panes[nPane].m_rctViewBox.right, m_panes[nPane].m_rctViewBox.bottom, SWP_SHOWWINDOW);
}

void CBiSplitterWnd::RecalcLayout()
{
	
	/*	Pane can be in 4 states:
		1. visible and not gripped - normal state
		2. hidden and not gripped - pane and splitter gap are not displayed
		3. hidden and gripped - the same as 2 state 
		4. visible and gripped - pane is not displayed, splitter gap is docked to splitter window border 		

		Both panes can be in 16 states.

	*/

	CRect rctClient;
	GetClientRect(&rctClient);

	BYTE state = (((((m_panes[0].IsVisible() << 1) | m_panes[0].IsGripped()) << 1) | m_panes[1].IsVisible()) << 1) | m_panes[1].IsGripped();
	
	//zero bit of the state show splitter orientation 0 - vertical 1 - horizontal
	ASSERT(m_bswStyles & SPLITTER_STYLES);
	state <<= 1;
	if(m_bswStyles & HSPLITTER)
		state = state | 1; 
		
	switch (state)
	{
	case 0x00:	//00000 
	case 0x01:	//00001 
		{//both pane are hidden and not gripped
			//draw empty splitter window - fill it with background color
			RclHideBothPanes();
            break;
		}
	case 0x02:	//00010
	case 0x03:	//00011
		{//left pane is hidden and not gripped, right pane is hidden and gripped
			//draw empty splitter window - fill it with background color
			RclHideBothPanes();
			break;
		}
	case 0x04:	//00100
	case 0x05:	//00101
		{//left pane is hidden and not gripped, right pane is visible and not gripped
			//expand the right pane into the whole splitter window
			RclExpandSecondPane(rctClient, FALSE);
			break;
		}
	case 0x06:	//00110
	case 0x07:	//00111
		{//left pane is hidden and not gripped, right pane is visible and gripped
			//expand the right pane into the whole splitter window
			RclExpandSecondPane(rctClient, FALSE);
			break;
		}
	case 0x08:	//01000
	case 0x09:	//01001
		{//left pane is hidden and gripped, right pane is hidden and not gripped
			//draw empty splitter window - fill it with background color
			RclHideBothPanes();
			break;
		}
	case 0x0A:	//01010
	case 0x0B:	//01011
		{//left pane is hidden and gripped, right pane is hidden and gripped
			//This is wrong situation - both pane can not be gripped simultaneously
			ASSERT(FALSE);
			break;
		}
	case 0x0C:	//01100
	case 0x0D:	//01101
		{//left pane is hidden and gripped, right pane is visible and not gripped
			//expand the right pane into the whole splitter window
			RclExpandSecondPane(rctClient, FALSE);
			break;
		}
	case 0x0E:	//01110
	case 0x0F:	//01111
		{//left pane is hidden and gripped, right pane is visible and gripped
			//expand the right pane into the whole splitter window
			RclExpandSecondPane(rctClient, FALSE);
			break;
		}
	case 0x10:	//10000
	case 0x11:	//10001
		{//left pane is visible and not gripped, right pane is hidden and not gripped
			//expand the left pane into the whole splitter window
			RclExpandFirstPane(rctClient, FALSE);
			break;
		}
	case 0x12:	//10010
	case 0x13:	//10011
		{//left pane is visible and not gripped, right pane is hidden and gripped
			//expand the left pane into the whole splitter window
			RclExpandFirstPane(rctClient, FALSE);
			break;
		}
	case 0x14:	//10100
		{//left pane is visible and not gripped, right pane is visible and not gripped
			//draw both panes and vertical splitter gap
			RclPanesV(rctClient);
			break;
		}
	case 0x15:	//10101
		{//left pane is visible and not gripped, right pane is visible and not gripped
			//draw both panes and horizontal splitter
			RclPanesH(rctClient);
			break;
		}
	case 0x16:	//10110
		{//left pane is visible and not gripped, right pane is visible and gripped
			//draw expanded left pane and vertical splitter gap docked to right window border
			rctClient.right -= m_rctGap.Width();
			m_rctGap.MoveGapToX(rctClient.right);
			RclExpandFirstPane(rctClient, TRUE);
			break;
		}
	case 0x17:	//10111
		{//left pane is visible and not gripped, right pane is visible and gripped
			//draw expanded left pane and horizontal splitter gap docked to bottom window border
			rctClient.bottom -= m_rctGap.Height();
			m_rctGap.MoveGapToY(rctClient.bottom);
			RclExpandFirstPane(rctClient, TRUE);
			break;
		}
	case 0x18:	//11000
	case 0x19:	//11001
		{//left pane is visible and gripped, right pane is hidden and not gripped
			//draw expanded left pane 
			RclExpandFirstPane(rctClient, FALSE);
			break;
		}
	case 0x1A:	//11010
	case 0x1B:	//11011
		{//left pane is visible and gripped, right pane is hidden and gripped
			//draw expanded left pane 
			RclExpandFirstPane(rctClient, FALSE);
			break;
		}
	case 0x1C:	//11100
		{//left pane is visible and gripped, right pane is visible and not gripped
			//draw expanded right pane and vertical splitter gap docked to left window border
			m_rctGap.MoveGapToX(0);
			rctClient.left += m_rctGap.Width();
			RclExpandSecondPane(rctClient, TRUE);
			break;
		}
	case 0x1D:	//11101
		{//left pane is visible and gripped, right pane is visible and not gripped
			//draw expanded right pane and horizontal splitter gap docked to top window border
			m_rctGap.MoveGapToY(0);
			rctClient.top += m_rctGap.Height();
			RclExpandSecondPane(rctClient, TRUE);
			break;
		}
	case 0x1E:	//11110
	case 0x1F:	//11111
		{//left pane is visible and gripped, right pane is visible and gripped
			//This is wrong situation - both pane can not be gripped simultaneously
			ASSERT(FALSE);
			break;
		}
	}

	UpdateWindow();
}

void CBiSplitterWnd::GripPane(UINT nPane)
{
    ASSERT(nPane >=0 && nPane <=1);
	if(m_panes[nPane].m_isGripped)	// expand pane
	{
		m_panes[nPane].ExpandPane();
		int nOtherPane = nPane ^ 1;  // 0 if nPane=1 and 1 if nPane=0
        if(m_panes[nOtherPane].IsGripped())
		{
			m_panes[nOtherPane].ExpandPane();
		}
		ASSERT(m_bswStyles & SPLITTER_STYLES);

		if(nPane == 0 && m_bswStyles & VSPLITTER)
		{
			SetSplitterPos(m_panes[0].m_rctPaneBox.right);
		}
		else if(nPane == 1 && m_bswStyles & VSPLITTER)
		{
			SetSplitterPos(m_panes[1].m_rctPaneBox.left - m_nGap);
		}
		else if(nPane == 0 && m_bswStyles & HSPLITTER)
		{
			SetSplitterPos(m_panes[0].m_rctPaneBox.bottom);
		}
		else if(nPane == 1 && m_bswStyles & HSPLITTER)
		{
			SetSplitterPos(m_panes[1].m_rctPaneBox.top - m_nGap);
		}
	}
	else	//grip pane
	{
		m_panes[nPane].GripPane();
		int nOtherPane = nPane ^ 1;  // 0 if nPane=1 and 1 if nPane=0
        if(m_panes[nOtherPane].IsGripped())
		{
			m_panes[nOtherPane].ExpandPane();
		}
		RecalcLayout();
	}
}

void CBiSplitterWnd::HidePane(CWnd *pView)
{
	for(int i = 0; i < 2; i++)
	{
		if(m_panes[i].m_pView && m_panes[i].m_pView == pView)
			HidePane(i);
	}
}

void CBiSplitterWnd::HidePane(UINT nPane)
{
	ASSERT(nPane >= 0 && nPane <= 1);
	
	int nOtherPane = OtherPane(nPane);

	if(m_panes[nOtherPane].IsVisible())//hide only this pane
	{
		if(m_panes[nPane].m_pView)
			m_panes[nPane].m_pView->ShowWindow(SW_HIDE);
		m_panes[nPane].m_isVisible = FALSE;
		m_rctGap.HideGap();
		RecalcLayout();
	}
	else 
	{
		CWnd *pParent = GetParent();
		ASSERT_VALID(pParent);
        if (pParent->IsKindOf(RUNTIME_CLASS(CBiSplitterWnd)))
		{//hide parent window pane
			((CBiSplitterWnd*)pParent)->HidePane(this);	
		}
		else
		{//hide splitter window
			//TODO: hide only panes and redraw empty splitter window
			CRect rct;
			GetWindowRect(&rct);
			if(m_panes[nPane].m_pView)
				m_panes[nPane].m_pView->ShowWindow(SW_HIDE);
			m_panes[nPane].m_isVisible = FALSE;
			ShowWindow(SW_HIDE);
			pParent->ScreenToClient(&rct);
			pParent->InvalidateRect(&rct, TRUE);
		}
	}
}

void CBiSplitterWnd::ShowPane(CWnd *pView)
{
	for(int i = 0; i < 2; i++)
	{
		if(m_panes[i].m_pView && m_panes[i].m_pView == pView)
			ShowPane(i);
	}
}


void CBiSplitterWnd::ShowPane(UINT nPane)
{
	ASSERT(nPane >=0 && nPane <=1);

	int nOtherPane = OtherPane(nPane);

	if(!m_panes[nPane].m_isVisible && m_panes[nOtherPane].m_isVisible && (GetStyle() & WS_VISIBLE))
	{
        if(m_panes[nPane].m_pView != NULL)
        {
            m_panes[nPane].m_pView->ShowWindow(SW_SHOW);
		}
        m_panes[nPane].m_isVisible = TRUE;
		m_rctGap.ShowGap();
		RecalcLayout();
	}
	else if(!m_panes[nPane].m_isVisible && m_panes[nOtherPane].m_isVisible && !(GetStyle() & WS_VISIBLE))
	{
		if(m_panes[nPane].m_pView != NULL)
        {
            m_panes[nPane].m_pView->ShowWindow(SW_SHOW);
		}
        m_panes[nPane].m_isVisible = TRUE;
		/*
			This case is posibble only if splitter wnd was hiden for two stages.
			In a first stage nPane view was hiden and in a second stage all splitter wnd was hidden.
			We should show only one pane and hide other pane.
		*/
		if(m_panes[nOtherPane].m_pView != NULL)
        {
            m_panes[nOtherPane].m_pView->ShowWindow(SW_HIDE);
		}
        m_panes[nOtherPane].m_isVisible = FALSE;
		CWnd *pParent = GetParent();
		ASSERT_VALID(pParent);
        if (pParent->IsKindOf(RUNTIME_CLASS(CBiSplitterWnd)))
		{//show parent window pane
			((CBiSplitterWnd*)pParent)->ShowPane(this);	
		}
		else
            ShowWindow(SW_SHOW);
		RecalcLayout();
	}
	else if(m_panes[nPane].m_isVisible && !m_panes[nOtherPane].m_isVisible && (GetStyle() & WS_VISIBLE))
	{
		return;
	}
	else if(m_panes[nPane].m_isVisible && !m_panes[nOtherPane].m_isVisible && !(GetStyle() & WS_VISIBLE))
	{
		CWnd *pParent = GetParent();
		ASSERT_VALID(pParent);
        if (pParent->IsKindOf(RUNTIME_CLASS(CBiSplitterWnd)))
		{//show parent window pane
			((CBiSplitterWnd*)pParent)->ShowPane(this);	
		}
		else
            ShowWindow(SW_SHOW);
	}
	else if(m_panes[nPane].m_isVisible && m_panes[nOtherPane].m_isVisible && (GetStyle() & WS_VISIBLE))
	{
		return;
	}
	else if(m_panes[nPane].m_isVisible && m_panes[nOtherPane].m_isVisible && !(GetStyle() & WS_VISIBLE))
	{
		CWnd *pParent = GetParent();
		ASSERT_VALID(pParent);
        if (pParent->IsKindOf(RUNTIME_CLASS(CBiSplitterWnd)))
		{//show parent window pane
			((CBiSplitterWnd*)pParent)->ShowPane(this);	
		}
		else
            ShowWindow(SW_SHOW);
	}
	else if(!m_panes[nPane].m_isVisible && !m_panes[nOtherPane].m_isVisible && !(GetStyle() & WS_VISIBLE))
	{
		if(m_panes[nPane].m_pView != NULL)
        {
            m_panes[nPane].m_pView->ShowWindow(SW_SHOW);
		}
        m_panes[nPane].m_isVisible = TRUE;

		CWnd *pParent = GetParent();
		ASSERT_VALID(pParent);
        if (pParent->IsKindOf(RUNTIME_CLASS(CBiSplitterWnd)))
		{//show parent window pane
			((CBiSplitterWnd*)pParent)->ShowPane(this);	
		}
		else
            ShowWindow(SW_SHOW);
		RecalcLayout();
		
	}
	else if(!m_panes[nPane].m_isVisible && !m_panes[nOtherPane].m_isVisible && (GetStyle() & WS_VISIBLE))
	{
        if(m_panes[nPane].m_pView != NULL)
        {
            m_panes[nPane].m_pView->ShowWindow(SW_SHOW);
		}
        m_panes[nPane].m_isVisible = TRUE;
		RecalcLayout();
	}
}

void CBiSplitterWnd::SetActivePane(UINT nPane)
{
	ASSERT(nPane >=0 && nPane <=1);
	if(m_panes[nPane].m_pView != NULL)
        m_panes[nPane].m_pView->SetActiveWindow();
	m_lastActivePane = nPane;
}

int CBiSplitterWnd::GetActivePane()
{
	return m_lastActivePane;
}

BOOL CBiSplitterWnd::IsPaneVisible(UINT nPane)
{
	ASSERT(nPane >=0 && nPane <=1);
	return m_panes[nPane].IsVisible() && (GetStyle() & WS_VISIBLE);
}

CWnd *CBiSplitterWnd::GetView(UINT viewID)
{
	ASSERT(GetDlgItem(viewID));
	return GetDlgItem(viewID);
}

//*** Implementation ***

CBiSplitterWnd::~CBiSplitterWnd()
{
	//if(m_pBiSplitterWnd)
	//	delete m_pBiSplitterWnd;

	if(m_pGlyphs)
	{
		for(int i = 0; i < m_glyphsCount; i++)
		{
			if(m_pGlyphs[i].pBits)
				delete [](m_pGlyphs[i].pBits);

			if(m_pGlyphs[i].pBitmapInfo)
				delete []((BYTE*)(m_pGlyphs[i].pBitmapInfo));
		}
		delete[] m_pGlyphs;
	}

	if(m_pSmGlyphs)
	{
		for(int i = 0; i < m_glyphsCount; i++)
		{
			if(m_pSmGlyphs[i].pBits)
				delete [](m_pSmGlyphs[i].pBits);

			if(m_pSmGlyphs[i].pBitmapInfo)
				delete []((BYTE*)(m_pSmGlyphs[i].pBitmapInfo));
		}
		delete[] m_pSmGlyphs;
	}

	if(m_images)
		delete []m_images;


}


void CBiSplitterWnd::SetPaneEmpty(CPane *pPane)
{
	pPane->m_rctCaption.SetRectEmpty();
	pPane->m_rctPaneBox.SetRectEmpty();
	pPane->m_closeBtn.SetRectEmpty();
	pPane->m_rctViewBox.SetRectEmpty();
}

void CBiSplitterWnd::RecalcGap(int left, int top, int right, int bottom)
{
	m_rctGap.left = left;
	m_rctGap.right = right;
	m_rctGap.top = top;
	m_rctGap.bottom = bottom;

	CBtn *pGripBtn = NULL;
	for( int i = 0; i < 2; i++)
	{
		pGripBtn = m_rctGap.m_gripBtn + i;
        if(m_panes[i].m_bspStyles & (GRIPBTN | SMGRIPBTN))
		{
			if(m_bswStyles & VSPLITTER)
			{
				pGripBtn->right = m_rctGap.right;
				pGripBtn->left = m_rctGap.left;
				pGripBtn->top = m_rctGap.Height() / 2 - 2 * m_nBtnCX + 3 * i * m_nBtnCY;
				pGripBtn->bottom = pGripBtn->top + m_nBtnCY;
			}
			else if(m_bswStyles & HSPLITTER)
			{
				pGripBtn->left = m_rctGap.Width() / 2 - 2 * m_nBtnCX + 3 * i * m_nBtnCX;
				pGripBtn->right = pGripBtn->left + m_nBtnCX;
				pGripBtn->top = m_rctGap.top;
				pGripBtn->bottom = m_rctGap.bottom;
			}
#ifdef _DEBUG
			else
				ASSERT(FALSE);
#endif
		}
		else
			pGripBtn->SetRectEmpty();
	}
}

void CBiSplitterWnd::RecalcPane(UINT nPane, int left, int top, int right, int bottom)
{
	ASSERT(nPane >=0 && nPane <=1);

	CPane *pPane = &(m_panes[nPane]);
	pPane->m_rctPaneBox.SetRect(left, top, right, bottom);
	switch(pPane->m_bspStyles & CAPTION_STYLES)
	{
	case NOTHING:
        pPane->m_rctCaption.SetRectEmpty();
		pPane->m_closeBtn.SetRectEmpty();
		break;
	case SMCAPTION:
		{
			if(m_bswStyles & VIEW3D)
			{
                pPane->m_rctCaption.top = pPane->m_rctPaneBox.top + BorderWidth(pPane->m_bspStyles);
				pPane->m_rctCaption.bottom = pPane->m_rctCaption.top + m_nSmCaptionHeight + 2 * CaptionBorderWidth(pPane->m_bspStyles);
				pPane->m_rctCaption.left = pPane->m_rctPaneBox.left + BorderWidth(pPane->m_bspStyles);
				pPane->m_rctCaption.right = pPane->m_rctPaneBox.right - BorderWidth(pPane->m_bspStyles);
			}
			else//flat view
			{
                pPane->m_rctCaption.top = pPane->m_rctPaneBox.top + BorderWidth(pPane->m_bspStyles);;
				pPane->m_rctCaption.bottom = pPane->m_rctCaption.top + m_nSmCaptionHeight + CaptionBorderWidth(pPane->m_bspStyles);
				pPane->m_rctCaption.left = pPane->m_rctPaneBox.left + BorderWidth(pPane->m_bspStyles);
				pPane->m_rctCaption.right = pPane->m_rctPaneBox.right - BorderWidth(pPane->m_bspStyles);
			}
		
			if(pPane->m_bspStyles & CLOSEBTN)
			{
				int btnHeight = m_nSmCaptionHeight;
				int btnWidth = btnHeight;
				if(pPane->m_rctCaption.Width() - 2 * CaptionBorderWidth(pPane->m_bspStyles) >= btnWidth )
				{
					pPane->m_closeBtn.right = pPane->m_rctCaption.right - CaptionBorderWidth(pPane->m_bspStyles);
					pPane->m_closeBtn.left = pPane->m_closeBtn.right - btnWidth;
					pPane->m_closeBtn.top = pPane->m_rctCaption.top + ((m_bswStyles & VIEW3D) ? CaptionBorderWidth(pPane->m_bspStyles) : 0);
					pPane->m_closeBtn.bottom = pPane->m_closeBtn.top + btnHeight;
				}
				else
					pPane->m_closeBtn.SetRectEmpty();
			}
		}
		break;
	case CAPTION:
		{
			if(m_bswStyles & VIEW3D)
			{

				pPane->m_rctCaption.top = pPane->m_rctPaneBox.top + BorderWidth(pPane->m_bspStyles);
				pPane->m_rctCaption.bottom = pPane->m_rctCaption.top + m_nCaptionHeight + 2 * CaptionBorderWidth(pPane->m_bspStyles);
				pPane->m_rctCaption.left = pPane->m_rctPaneBox.left + BorderWidth(pPane->m_bspStyles);
				pPane->m_rctCaption.right = pPane->m_rctPaneBox.right - BorderWidth(pPane->m_bspStyles);
			}
			else
			{
                pPane->m_rctCaption.top = pPane->m_rctPaneBox.top + BorderWidth(pPane->m_bspStyles);;
				pPane->m_rctCaption.bottom = pPane->m_rctCaption.top + m_nCaptionHeight + CaptionBorderWidth(pPane->m_bspStyles);
				pPane->m_rctCaption.left = pPane->m_rctPaneBox.left + BorderWidth(pPane->m_bspStyles);
				pPane->m_rctCaption.right = pPane->m_rctPaneBox.right - BorderWidth(pPane->m_bspStyles);
			}

			if(pPane->m_bspStyles & CLOSEBTN)
			{
				int btnHeight = m_nCaptionHeight;
				int btnWidth = btnHeight;
				if(pPane->m_rctCaption.Width() - 2 * CaptionBorderWidth(pPane->m_bspStyles) >= btnWidth)
				{
					pPane->m_closeBtn.right = pPane->m_rctCaption.right - CaptionBorderWidth(pPane->m_bspStyles);
					pPane->m_closeBtn.left = pPane->m_closeBtn.right - btnWidth;
					pPane->m_closeBtn.top = pPane->m_rctCaption.top + ((m_bswStyles & VIEW3D) ? CaptionBorderWidth(pPane->m_bspStyles) : 0);//CaptionBorderWidth(pPane->m_bspStyles);
					pPane->m_closeBtn.bottom = pPane->m_closeBtn.top + btnHeight;
				}
				else
					pPane->m_closeBtn.SetRectEmpty();
			}
		}
		break;
	default:
		ASSERT(FALSE);
	}
	pPane->m_rctViewBox.left = pPane->m_rctPaneBox.left + BorderWidth(pPane->m_bspStyles);
	pPane->m_rctViewBox.right	= pPane->m_rctPaneBox.right - BorderWidth(pPane->m_bspStyles);
	pPane->m_rctViewBox.top = pPane->m_rctPaneBox.top + BorderWidth(pPane->m_bspStyles) + pPane->m_rctCaption.Height();
	pPane->m_rctViewBox.bottom = pPane->m_rctPaneBox.bottom - BorderWidth(pPane->m_bspStyles);
}

void CBiSplitterWnd::DrawPane(CDC *pDC, CPane *pPane)
{
	ASSERT_VALID(pDC);
	if(!pPane->IsVisible() || pPane->m_isGripped) 
	{
		if(pPane->m_pView != NULL)
			pPane->m_pView->ShowWindow(SW_HIDE);
		return;
	}


	if( pPane->m_pView != NULL)
	{
        pPane->m_pView->SetWindowPos(NULL, pPane->m_rctViewBox.left, pPane->m_rctViewBox.top,
			pPane->m_rctViewBox.Width(), pPane->m_rctViewBox.Height(), SWP_SHOWWINDOW);
//		pPane->m_pView->Invalidate();
		pPane->m_pView->UpdateWindow();
	}

	switch(pPane->m_bspStyles & BORDER_STYLES)
	{
	case THIN_BORDER: //one pixel width pane border
        if(m_bswStyles & VIEW3D)
			pDC->Draw3dRect(pPane->m_rctPaneBox, m_clrBtnShadow, m_clrBtnHilite);
		else
		{
			CBrush brush(m_clrFlatBorder);
			pDC->FrameRect(pPane->m_rctPaneBox, &brush);							
		}
		break;
	case THICK_BORDER: // two pixel width border
	{
		CRect box = pPane->m_rctPaneBox;
        if(m_bswStyles & VIEW3D)
		{
            pDC->Draw3dRect(box,m_clrBtnShadow, m_clrBtnHilite);
            box.DeflateRect(1, 1);
            pDC->Draw3dRect(box, m_clrWindowFrame, m_clrBtnFace);
		}
		else
		{
			CBrush brush(m_clrFlatBorder);
            pDC->FrameRect(box, &brush);
            box.DeflateRect(1, 1);
            pDC->FrameRect(box, &brush);
		}
	}
		break;
#ifdef _DEBUG
	case NOTHING:
		break;
	default:
		ASSERT(FALSE);
#endif
	}
	
	if(((pPane->m_bspStyles & CAPTION) || (pPane->m_bspStyles & SMCAPTION))	)
	{
        DrawCaption(pDC, pPane);
        if(pPane->m_bspStyles & CLOSEBTN)
            DrawCloseBtn(pDC, pPane);
	}
}

void CBiSplitterWnd::DrawCaption(CDC *pDC, CPane *pPane)
{
	ASSERT(pPane->m_bspStyles & CAPTION_STYLES);

	CRect rect;
	CRect rect1 = pPane->m_rctPaneBox;
	rect1.DeflateRect(BorderWidth(pPane->m_bspStyles), BorderWidth(pPane->m_bspStyles));
	rect.IntersectRect(pPane->m_rctCaption, rect1);

	switch(pPane->m_bspStyles & CAPTIONBORDER_STYLES)
	{
	case NOTHING:
		{
            pDC->FillSolidRect(rect, m_clrBtnFace);
			rect.right -= pPane->m_closeBtn.Width();
			rect.left += BorderWidth(pPane->m_bspStyles);
            CFont *pOldFont	= pDC->SelectObject((pPane->m_bspStyles & CAPTION ? &m_fntCaption : &m_fntSmCaption));
            SetBkMode(pDC->m_hDC, TRANSPARENT);
            pDC->DrawText(pPane->m_sCaptionText, &rect, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
            pDC->SelectObject(pOldFont);
		}
		break;
	case THIN_CAPTIONBORDER: //one pixel width pane border
		{
			if(pPane->m_rctCaption.Width() >= 2 * BorderWidth(pPane->m_bspStyles))
			{
		        if(m_bswStyles & VIEW3D)
				{
					pDC->Draw3dRect(rect, m_clrBtnHilite, m_clrBtnShadow);
					rect.DeflateRect(1, 1);
				}
				else
				{
					CPen pen(PS_SOLID, 0, m_clrFlatBorder);
					CPen *pOldPen = pDC->SelectObject(&pen);
					pDC->MoveTo(rect.left, rect.bottom - 1);
					pDC->LineTo(rect.right, rect.bottom - 1);
					pDC->SelectObject(pOldPen);
					rect.bottom -= 1;
				}
	            
			}
            pDC->FillSolidRect(rect, m_clrBtnFace);
			rect.right -= pPane->m_closeBtn.Width();
			rect.left += BorderWidth(pPane->m_bspStyles);
            CFont *pOldFont	= pDC->SelectObject((pPane->m_bspStyles & CAPTION ? &m_fntCaption : &m_fntSmCaption));
            SetBkMode(pDC->m_hDC, TRANSPARENT);
            pDC->DrawText(pPane->m_sCaptionText, &rect, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
            pDC->SelectObject(pOldFont);
		}
		break;
	case THICK_CAPTIONBORDER: // two pixel width border
	{
        //draw thick border for caption
		if(((pPane->m_bspStyles & CAPTION) || (pPane->m_bspStyles & SMCAPTION)))
		{
			if(pPane->m_rctCaption.Width() >= 2 * BorderWidth(pPane->m_bspStyles))
			{
		        if(m_bswStyles & VIEW3D)
				{
					pDC->Draw3dRect(rect, m_clrBtnHilite, m_clrWindowFrame);
				    rect.DeflateRect(1, 1);
		            pDC->Draw3dRect(rect, m_clrBtnFace, m_clrBtnShadow);
					rect.DeflateRect(1, 1);
				}
				else
				{
					CPen pen(PS_SOLID, 0, m_clrFlatBorder);
					CPen *pOldPen = pDC->SelectObject(&pen);
					pDC->MoveTo(rect.left, rect.bottom - 1);
					pDC->LineTo(rect.right, rect.bottom - 1);
					pDC->MoveTo(rect.left, rect.bottom - 2);
					pDC->LineTo(rect.right, rect.bottom - 2);
					pDC->SelectObject(pOldPen);
					rect.bottom -= 2;
				}

			}
			//fill caption
			pDC->FillSolidRect(rect, m_clrBtnFace);
			rect.right -= pPane->m_closeBtn.Width();
			rect.left += BorderWidth(pPane->m_bspStyles);
            CFont *pOldFont	= pDC->SelectObject((pPane->m_bspStyles & CAPTION ? &m_fntCaption : &m_fntSmCaption));
            SetBkMode(pDC->m_hDC, TRANSPARENT);
            pDC->DrawText(pPane->m_sCaptionText, &rect, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
            pDC->SelectObject(pOldFont);
		}
	}
		break;
	default:
		ASSERT(FALSE);
	}
}

void CBiSplitterWnd::DrawSplitterGap(CDC *pDC, const CRect &rect)
{
	ASSERT_VALID(pDC);
	if(!m_rctGap.IsVisible()) 
		return;
	pDC->FillSolidRect(rect, m_clrBtnFace);

	if(m_panes[0].m_bspStyles & (GRIPBTN | SMGRIPBTN))
		DrawGripBtn(pDC, 0);
	if(m_panes[1].m_bspStyles & (GRIPBTN | SMGRIPBTN))
		DrawGripBtn(pDC, 1);
}

void CBiSplitterWnd::DrawGripBtn(CDC *pDC, int nGrip)
{
	if ( (m_panes[nGrip].m_bspStyles & (GRIPBTN | SMGRIPBTN)) == 0)
		return;
	int index;
	if(nGrip == 0 && (m_bswStyles & VSPLITTER) && !m_panes[nGrip].m_isGripped)
		index = IMAGE_GRIP_L;
	else if(nGrip == 0 && (m_bswStyles & VSPLITTER) && m_panes[nGrip].m_isGripped)
		index = IMAGE_GRIP_R;
	else if(nGrip == 1 && (m_bswStyles & VSPLITTER) && !m_panes[nGrip].m_isGripped)
		index = IMAGE_GRIP_R;
	else if(nGrip == 1 && (m_bswStyles & VSPLITTER) && m_panes[nGrip].m_isGripped)
		index = IMAGE_GRIP_L;
	else if(nGrip == 0 && (m_bswStyles & HSPLITTER) && !m_panes[nGrip].m_isGripped)
		index = IMAGE_GRIP_T;
	else if(nGrip == 0 && (m_bswStyles & HSPLITTER) && m_panes[nGrip].m_isGripped)
		index = IMAGE_GRIP_B;
	else if(nGrip == 1 && (m_bswStyles & HSPLITTER)&& !m_panes[nGrip].m_isGripped)
		index = IMAGE_GRIP_B;
	else if(nGrip == 1 && (m_bswStyles & HSPLITTER)&& m_panes[nGrip].m_isGripped)
		index = IMAGE_GRIP_T;
#ifdef _DEBUG
	else
		ASSERT(FALSE);
#endif

	switch(m_panes[nGrip].m_bspStyles & GRIP_STYLES)
	{
	case GRIPBTN:
        DrawBtn(pDC, m_rctGap.m_gripBtn + nGrip, index, m_panes[nGrip].m_bspStyles, FALSE);
		break;
	case SMGRIPBTN:
        DrawBtn(pDC, m_rctGap.m_gripBtn + nGrip, index, m_panes[nGrip].m_bspStyles, TRUE);
		break;
#ifdef _DEBUG
	default:
		ASSERT(FALSE);
#endif
	}
}

void CBiSplitterWnd::DrawCloseBtn(CDC *pDC, CPane *pPane)
{
	if(pPane->m_closeBtn.IsRectEmpty()
			|| (pPane->m_bspStyles & CAPTION_STYLES) == NOTHING
			|| (pPane->m_bspStyles & CLOSEBTN) == NOTHING)
		return;
	
	switch(pPane->m_bspStyles & CAPTION_STYLES)
	{
	case CAPTION:
        DrawBtn(pDC, &(pPane->m_closeBtn), IMAGE_CLOSE, pPane->m_bspStyles, FALSE);
		break;
	case SMCAPTION:
        DrawBtn(pDC, &(pPane->m_closeBtn), IMAGE_CLOSE, pPane->m_bspStyles, TRUE);
		break;
#ifdef _DEBUG
	default:
		ASSERT(FALSE);
#endif
	}
}

void CBiSplitterWnd::DrawBtn(CDC *pDC, CBtn *pBtn, int nImageIndex, UINT styles, BOOLEAN smImageSize)
{
	if(pBtn->IsRectEmpty())
		return;

	CRect rctBtn = *pBtn;
	COLORREF bkColor;
	
	switch(pBtn->m_curState)
	{
	case CBtn::DOWN: // down
		switch(styles & BUTTON_STYLES)
		{
		case THICK_BUTTON:
            pDC->Draw3dRect(rctBtn, m_clrWindowFrame, m_clrBtnHilite);
	        rctBtn.DeflateRect(1,1);
		    pDC->Draw3dRect(rctBtn, m_clrBtnShadow, m_clrBtnFace);
			rctBtn.DeflateRect(1,1);
			pDC->FillSolidRect(rctBtn, m_clrBtnFace);
			bkColor = m_clrBtnFace;
			break;
		case THIN_BUTTON:
			pDC->Draw3dRect(rctBtn, m_clrBtnShadow, m_clrBtnHilite);
			rctBtn.DeflateRect(1,1);
			pDC->FillSolidRect(rctBtn, m_clrBtnFace);
			bkColor = m_clrBtnFace;
			break;
		case FLAT_BUTTON:
            pDC->Draw3dRect(rctBtn, m_clrFlatBorder, m_clrFlatBorder);
	        rctBtn.DeflateRect(1,1);
			pDC->FillSolidRect(rctBtn, m_clrBtnFace);
			bkColor = m_clrBtnFace;
			break;
		default:
			ASSERT(FALSE);
		}
		break;
	case CBtn::NEUTRAL:
			pDC->FillSolidRect(rctBtn, m_clrBtnFace);
			bkColor = m_clrBtnFace;
			break;
	case CBtn::UP:
		switch(styles & BUTTON_STYLES)
		{
		case THICK_BUTTON:
            pDC->Draw3dRect(rctBtn, m_clrBtnHilite, m_clrWindowFrame);
            rctBtn.DeflateRect(1,1);
            pDC->Draw3dRect(rctBtn, m_clrBtnFace, m_clrBtnShadow);
            rctBtn.DeflateRect(1,1);
			pDC->FillSolidRect(rctBtn, m_clrBtnFace);
			bkColor = m_clrBtnFace;
			break;
		case THIN_BUTTON:
            pDC->Draw3dRect(rctBtn, m_clrBtnHilite, m_clrBtnShadow);
            rctBtn.DeflateRect(1,1);
			pDC->FillSolidRect(rctBtn, m_clrBtnFace);
			bkColor = m_clrBtnFace;
			break;
		case FLAT_BUTTON:
            pDC->Draw3dRect(rctBtn, m_clrFlatBtnActiveBorder, m_clrFlatBtnActiveBorder);
	        rctBtn.DeflateRect(1,1);
			pDC->FillSolidRect(rctBtn, m_clrFlatBtnActiveFace);
			bkColor = m_clrFlatBtnActiveFace;
			break;
		default:
			ASSERT(FALSE);
		}
		break;
	default:
		ASSERT(FALSE);
	}
	
	GLYPHINFO *pGlyph = NULL;
	if(smImageSize)
	{
		pGlyph = m_pSmGlyphs + nImageIndex;
	}
	else 
	{
		pGlyph = m_pGlyphs + nImageIndex;
	}

	pGlyph->pBitmapInfo->bmiColors[0].rgbRed = GetRValue(m_clrFlatBtnActiveFace);	
	pGlyph->pBitmapInfo->bmiColors[0].rgbGreen = GetGValue(m_clrFlatBtnActiveFace);
	pGlyph->pBitmapInfo->bmiColors[0].rgbBlue = GetBValue(m_clrFlatBtnActiveFace);
	pGlyph->pBitmapInfo->bmiColors[0].rgbReserved = 0;


	pGlyph->pBitmapInfo->bmiColors[1].rgbRed = 0;	
	pGlyph->pBitmapInfo->bmiColors[1].rgbGreen = 0;
	pGlyph->pBitmapInfo->bmiColors[1].rgbBlue = 0;
	pGlyph->pBitmapInfo->bmiColors[1].rgbReserved = 0;

	int cx = pGlyph->pBitmapInfo->bmiHeader.biWidth;
	int cy = pGlyph->pBitmapInfo->bmiHeader.biHeight;

	StretchDIBits(pDC->GetSafeHdc(), pBtn->left + (pBtn->Width() - cx) / 2,
		pBtn->top + (pBtn->Height() - cy) / 2,
		cx,
		cy,
		0,
		0,
		cx,
		cy,
		pGlyph->pBits,
		pGlyph->pBitmapInfo,
		DIB_RGB_COLORS,
		SRCCOPY);

}

void CBiSplitterWnd::UpdateSysColors()
{
	m_clrBtnFace = ::GetSysColor(COLOR_BTNFACE);
	m_clrBtnShadow = ::GetSysColor(COLOR_BTNSHADOW);
	m_clrBtnHilite = ::GetSysColor(COLOR_BTNHIGHLIGHT);
	m_clrBtnText = ::GetSysColor(COLOR_BTNTEXT);
	m_clrWindowFrame = ::GetSysColor(COLOR_WINDOWFRAME);
}

void CBiSplitterWnd::UpdateSysMetrics()
{
    LOGFONT lf;
	// System metrics
	NONCLIENTMETRICS metrics;
	metrics.cbSize=sizeof(NONCLIENTMETRICS);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &metrics, 0);
	
	m_nSmCaptionHeight=metrics.iSmCaptionHeight;
	if(m_fntSmCaption.m_hObject != NULL)
		m_fntSmCaption.DeleteObject();
    memset(&lf, 0, sizeof(LOGFONT));       // zero out structure
    lf.lfHeight = m_nSmCaptionHeight - 2; 
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfWeight = FW_NORMAL;
	lf.lfQuality = ANTIALIASED_QUALITY;
	lstrcpy(lf.lfFaceName,_T("Tahoma"));        // request a face name "Marlett"

	VERIFY(m_fntSmCaption.CreateFontIndirect(&lf));  // create the font

	m_nCaptionHeight=metrics.iCaptionHeight;
	if(m_fntCaption.m_hObject != NULL)
		m_fntCaption.DeleteObject();
    lf.lfHeight = m_nSmCaptionHeight - 2; 
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfWeight = FW_NORMAL;
	lf.lfQuality = ANTIALIASED_QUALITY;
	lstrcpy(lf.lfFaceName,_T("Tahoma"));        // request a face name "Marlett"

	VERIFY(m_fntCaption.CreateFontIndirect(&lf));  // create the font

	m_hSplitterCursorV = ::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEWE));
	m_hSplitterCursorH = ::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENS));
	m_hArrowCursor = ::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));

	// Dimensions, in pixels, of small caption buttons
	m_nBtnSmCX = GetSystemMetrics(SM_CXSMSIZE); 
	m_nBtnSmCY = GetSystemMetrics(SM_CYSMSIZE);
	m_nBtnCX = GetSystemMetrics(SM_CXSIZE); 
	m_nBtnCY = GetSystemMetrics(SM_CYSIZE);

	//font for close button image drawing
	if(m_fntSymbol.m_hObject != NULL)
		m_fntSymbol.DeleteObject();
		
    memset(&lf, 0, sizeof(LOGFONT));       // zero out structure
    lf.lfHeight = m_nCaptionHeight - 4; 
	lf.lfCharSet = SYMBOL_CHARSET;
	lf.lfWeight = FW_THIN;
	lf.lfQuality = DEFAULT_QUALITY;//ANTIALIASED_QUALITY;
	lstrcpy(lf.lfFaceName,_T("Marlett"));        // request a face name "Marlett"
	VERIFY(m_fntSymbol.CreateFontIndirect(&lf));  // create the font

	//font for small close button image drawing
	if(m_fntSmSymbol.m_hObject != NULL)
		m_fntSmSymbol.DeleteObject();

    memset(&lf, 0, sizeof(LOGFONT));       // zero out structure
    lf.lfHeight = m_nSmCaptionHeight - 4; 
	lf.lfCharSet = SYMBOL_CHARSET;
	lf.lfWeight = FW_THIN;
	lf.lfQuality = DEFAULT_QUALITY; //ANTIALIASED_QUALITY;
	lstrcpy(lf.lfFaceName, _T("Marlett"));        // request a face name "Marlett"
	VERIFY(m_fntSmSymbol.CreateFontIndirect(&lf));  // create the font
}

void CBiSplitterWnd::UpdateSysImages()
{
	ASSERT(m_fntSmSymbol.m_hObject);
	ASSERT(m_fntSymbol.m_hObject);
	
	CDC *pDC = GetDC();
	ASSERT(pDC);
	GLYPHMETRICS gm;
	MAT2 mt = {{0, 1}, {0, 0}, {0, 0}, {0, 1}};//identity matrix
	int sizeImage;

	for(int i = 0; i < m_glyphsCount; i++)
	{
		pDC->SelectObject(&m_fntSymbol);
		sizeImage = pDC->GetGlyphOutline(m_images[i], GGO_BITMAP, &gm, 0, NULL, &mt);
		ASSERT (sizeImage != -1);
		if(m_pGlyphs[i].pBits)
			delete []m_pGlyphs[i].pBits;
		m_pGlyphs[i].pBits = new BYTE[sizeImage];
		pDC->GetGlyphOutline(m_images[i], GGO_BITMAP, &gm, sizeImage, m_pGlyphs[i].pBits, &mt);
		m_pGlyphs[i].pBitmapInfo->bmiHeader.biWidth = gm.gmBlackBoxX;
		m_pGlyphs[i].pBitmapInfo->bmiHeader.biHeight = gm.gmBlackBoxY;

		pDC->SelectObject(&m_fntSmSymbol);
		sizeImage = pDC->GetGlyphOutline(m_images[i], GGO_BITMAP, &gm, 0, NULL, &mt);
		ASSERT (sizeImage != -1);
		if(m_pSmGlyphs[i].pBits)
			delete []m_pSmGlyphs[i].pBits;
		m_pSmGlyphs[i].pBits = new BYTE[sizeImage];
		pDC->GetGlyphOutline(m_images[i], GGO_BITMAP, &gm, sizeImage, m_pSmGlyphs[i].pBits, &mt);
		m_pSmGlyphs[i].pBitmapInfo->bmiHeader.biWidth = gm.gmBlackBoxX;
		m_pSmGlyphs[i].pBitmapInfo->bmiHeader.biHeight = gm.gmBlackBoxY;
	}

	ReleaseDC(pDC);
}


void CBiSplitterWnd::DrawTracker(const CRect& rect)
{

	ASSERT_VALID(this);
	ASSERT(!rect.IsRectEmpty());

	// pat-blt without clip children on
	CDC* pDC = GetDC();
	// invert the brush pattern (looks just like frame window sizing)
	CBrush* pBrush = CDC::GetHalftoneBrush();
	HBRUSH hOldBrush = NULL;
	if (pBrush != NULL)
		hOldBrush = (HBRUSH)SelectObject(pDC->m_hDC, pBrush->m_hObject);
	pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATINVERT);
	if (hOldBrush != NULL)
		SelectObject(pDC->m_hDC, hOldBrush);
	ReleaseDC(pDC);
}

void CBiSplitterWnd::StartTracking(int ht)
{
	ASSERT_VALID(this);
	if (ht != splitterBar)
		return;
	SetCapture();
	SetFocus();
	// make sure no updates are pending
	RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_UPDATENOW);
	// set tracking state and appropriate cursor
	m_trackingState = SPLITTER;
	m_rctTracker = m_rctGap;
	DrawTracker(m_rctTracker);
}

void CBiSplitterWnd::TrackSplitter(const CPoint &point)
{
    switch(m_bswStyles & SPLITTER_STYLES)
	{
	case VSPLITTER:
		{
			// move tracker to current cursor position        
            CRect rctInside;
			GetClientRect(&rctInside);
			rctInside.left = m_panes[0].m_rctViewBox.left;
			rctInside.right = m_panes[1].m_rctViewBox.right;
			DrawTracker(m_rctTracker);
			if(rctInside.PtInRect(point))
				m_rctTracker.OffsetRect(point.x - m_rctTracker.left, 0);
			else if(point.x < rctInside.left)
				m_rctTracker.OffsetRect(rctInside.left - m_rctTracker.left, 0);
			else if(point.x > rctInside.right)
				m_rctTracker.OffsetRect(rctInside.right - m_rctTracker.right, 0);
			DrawTracker(m_rctTracker);
			break;
		}
	case HSPLITTER:
		{
            // move tracker to current cursor position        
			CRect rctInside;
			GetClientRect(&rctInside);
			rctInside.top = 0;//m_panes[0].m_rctViewBox.top;
			rctInside.bottom = m_panes[1].m_rctViewBox.bottom - m_nGap;
			DrawTracker(m_rctTracker);
			if(rctInside.PtInRect(point))
				m_rctTracker.OffsetRect(0, point.y - m_rctTracker.top);
			else if(point.y < rctInside.top)
				m_rctTracker.OffsetRect(0, rctInside.top - m_rctTracker.top);
			else if(point.y > rctInside.bottom)
				m_rctTracker.OffsetRect(0, rctInside.bottom - m_rctTracker.top);
			DrawTracker(m_rctTracker);
			break;
		}
	default:
		ASSERT(FALSE);
	}
}

void CBiSplitterWnd::StopTracking()
{
    ASSERT_VALID(this);
    
	if(m_trackingState != SPLITTER)
		return;
	ReleaseCapture();
}

int CBiSplitterWnd::HitTest(CPoint pt) const
{
	ASSERT_VALID(this);


	if(m_panes[0].IsVisible() && !m_panes[0].IsGripped() && m_panes[0].m_closeBtn.PtInRect(pt))
		return closeBtn0;
	
	if(m_panes[1].IsVisible() && !m_panes[1].IsGripped() && m_panes[1].m_closeBtn.PtInRect(pt))
		return closeBtn1;

	if(m_rctGap.IsVisible())
	{
		if(m_rctGap.m_gripBtn[0].PtInRect(pt))
			return gripBtn0;

		if(m_rctGap.m_gripBtn[1].PtInRect(pt))
			return gripBtn1;

		if(m_rctGap.PtInRect(pt))
			return splitterBar;
	}
	return noHit;
}


/******************************* private members *********************************/
void CBiSplitterWnd::RclHideBothPanes(void)
{
//	m_panes[0].HidePane();
//	m_panes[1].HidePane();
	m_rctGap.HideGap();
	Invalidate(TRUE);
}

void CBiSplitterWnd::RclExpandSecondPane(const CRect &rctClient, BOOL isGapVisible)
{
	m_rctGap.ShowGap(isGapVisible);
//	m_panes[0].HidePane();
	RecalcPane(1, rctClient.left, rctClient.top, rctClient.right, rctClient.bottom);
	Invalidate(FALSE);
}

void CBiSplitterWnd::RclExpandFirstPane(const CRect &rctClient, BOOL isGapVisible)
{
	m_rctGap.ShowGap(isGapVisible);
//	m_panes[1].HidePane();
	RecalcPane(0, rctClient.left, rctClient.top, rctClient.right, rctClient.bottom);
	Invalidate(FALSE);
}

void CBiSplitterWnd::RclPanesV(const CRect &rctClient)
{
	if(rctClient.IsRectEmpty())
		return;
	//int minClientWidth = 2 * (BorderWidth(m_panes[0].m_bspStyles) + BorderWidth(m_panes[1].m_bspStyles)) + (int)m_nGap; 
	//if(minClientWidth >= rctClient.Width())
	//{
	//	RclHideBothPanes();
	//	return;
	//}

	if( m_nSplitterDenominator == 0)
	{
		m_nSplitterNumerator = m_nSplitterPos;
		m_nSplitterDenominator = rctClient.Width();
	}

	//calculate the Panes new width
	switch(m_bswStyles & SIZING_MODE_STYLES)
	{
	case FIXED0:
		//nothing to do 
		break;
	case FIXED1:
		m_nSplitterPos = rctClient.Width() - (m_nSplitterDenominator - m_nSplitterNumerator);
		m_nSplitterDenominator = rctClient.Width();
		m_nSplitterNumerator = m_nSplitterPos;
		break;
	case PROPORTIONAL:
		m_nSplitterPos = MulDiv(m_nSplitterNumerator, rctClient.Width(), m_nSplitterDenominator);//(m_nSplitterPos * rctClient.Width()) / m_nSplitterPos2;
		break;
	default:
		ASSERT(FALSE);
	}

	int minSplitterPos = 2 * BorderWidth(m_panes[0].m_bspStyles);
	int maxSplitterPos = rctClient.Width() - 2 * BorderWidth(m_panes[1].m_bspStyles) - m_nGap;
	if(m_nSplitterPos < minSplitterPos)
		m_nSplitterPos = minSplitterPos;
	if(m_nSplitterPos > maxSplitterPos)
		m_nSplitterPos = maxSplitterPos;

	RecalcPane(0, rctClient.left, rctClient.top, m_nSplitterPos, rctClient.bottom);
	RecalcPane(1, m_nSplitterPos + m_nGap, rctClient.top, rctClient.right, rctClient.bottom);
	RecalcGap(m_nSplitterPos, 0, m_nSplitterPos + m_nGap, rctClient.Height());
	m_rctGap.ShowGap();
//	m_panes[0].ShowPane();
//	m_panes[1].ShowPane();
	Invalidate(FALSE);
}


void CBiSplitterWnd::RclPanesH(const CRect &rctClient)
{
	//TODO: calculate min client height
	if(rctClient.IsRectEmpty())
		return;
	if( m_nSplitterDenominator == 0)
	{
		m_nSplitterDenominator = rctClient.Height();
	}

	//calculate the Panes new height
	switch(m_bswStyles & SIZING_MODE_STYLES)
	{	
	case FIXED0:
		//nothing to do 
		break;
	case FIXED1:
		m_nSplitterPos = rctClient.Height() - (m_nSplitterDenominator - m_nSplitterNumerator);
		m_nSplitterDenominator = rctClient.Height();
		m_nSplitterNumerator = m_nSplitterPos;
		break;
	case PROPORTIONAL:
		m_nSplitterPos = MulDiv(m_nSplitterNumerator, rctClient.Height(), m_nSplitterDenominator);
		break;
	default:
		ASSERT(FALSE);
	}
	
	int minSplitterPos = 2 * BorderWidth(m_panes[0].m_bspStyles);
	int maxSplitterPos = rctClient.Height() - 2 * BorderWidth(m_panes[1].m_bspStyles) - m_nGap;
	if(m_nSplitterPos < minSplitterPos)
		m_nSplitterPos = minSplitterPos;
	if(m_nSplitterPos > maxSplitterPos)
		m_nSplitterPos = maxSplitterPos;

	RecalcPane(0, rctClient.left, rctClient.top, rctClient.right, m_nSplitterPos);
	RecalcPane(1, rctClient.left, m_nSplitterPos + m_nGap, rctClient.right, rctClient.bottom);
	RecalcGap(0, m_nSplitterPos, rctClient.right, m_nSplitterPos + m_nGap);
	m_rctGap.ShowGap();
//	m_panes[0].ShowPane();
//	m_panes[1].ShowPane();
	Invalidate(FALSE);
}


/************************* CBiSplitterWnd message handlers ****************************/


BOOL CBiSplitterWnd::OnNcCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (!CWnd::OnNcCreate(lpCreateStruct))
		return FALSE;

	// remove WS_EX_CLIENTEDGE style from parent window
	//  (the splitter itself will provide the 3d look)
	CWnd* pParent = GetParent();
	ASSERT_VALID(pParent);
	pParent->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME);

	return TRUE;
}

void CBiSplitterWnd::OnSize(UINT nType, int cx, int cy)
{
	if (nType != SIZE_MINIMIZED && cx > 0 && cy > 0)
		RecalcLayout();

	CWnd::OnSize(nType, cx, cy);

}

void CBiSplitterWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
//	CWnd::OnLButtonDown(nFlags, point);
	SetFocus();

	if(m_trackingState != NOTRACKING)
		return;
	CClientDC dc(this);
	switch(HitTest(point))
	{
	case splitterBar:
        StartTracking(splitterBar);
		break;
	case closeBtn0:
		SetCapture();
		SetFocus();
		// make sure no updates are pending
		RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_UPDATENOW);
		// set tracking state and appropriate cursor
		m_trackingState = CLOSEBTN0;
		m_panes[0].m_closeBtn.m_curState = CBtn::DOWN;
		DrawCloseBtn(&dc, m_panes);
		break;
	case closeBtn1:
		SetCapture();
		SetFocus();
		// make sure no updates are pending
		RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_UPDATENOW);
		// set tracking state and appropriate cursor
		m_trackingState = CLOSEBTN1;
		m_panes[1].m_closeBtn.m_curState = CBtn::DOWN;
		DrawCloseBtn(&dc, m_panes + 1);
		break;
	case gripBtn0:
		SetCapture();
		SetFocus();
		// make sure no updates are pending
		RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_UPDATENOW);
		// set tracking state and appropriate cursor
		m_trackingState = GRIPBTN0;
		m_rctGap.m_gripBtn[0].m_curState = CBtn::DOWN;
		DrawGripBtn(&dc, 0);
		break;
	case gripBtn1:
		SetCapture();
		SetFocus();
		// make sure no updates are pending
		RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_UPDATENOW);
		// set tracking state and appropriate cursor
		m_trackingState = GRIPBTN1;
		m_rctGap.m_gripBtn[1].m_curState = CBtn::DOWN;
		DrawGripBtn(&dc, 1);
		break;
	case noHit:
		m_trackingState = NOTRACKING;
		break;
	default:
		ASSERT(FALSE);
	}
}

void CBiSplitterWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	switch(m_trackingState)
	{
	case SPLITTER:
		StopTracking();
		SetSplitterPos(m_bswStyles & VSPLITTER ? m_rctTracker.left : m_rctTracker.top);
		break;
	case CLOSEBTN0:
		m_panes[0].m_closeBtn.m_curState = CBtn::NEUTRAL;
		m_trackingState = NOTRACKING;
		ReleaseCapture();
		if(m_panes[0].m_closeBtn.PtInRect(point))
            HidePane(BSW_FIRST_PANE);
		break;
	case CLOSEBTN1:
		m_panes[1].m_closeBtn.m_curState = CBtn::NEUTRAL;
		m_trackingState = NOTRACKING;
		ReleaseCapture();
		if(m_panes[1].m_closeBtn.PtInRect(point))
            HidePane(1);
		break;
	case GRIPBTN0:
		m_rctGap.m_gripBtn[0].m_curState = CBtn::NEUTRAL;
		m_trackingState = NOTRACKING;
		ReleaseCapture();
		if(m_rctGap.m_gripBtn[0].PtInRect(point))
            GripPane(0);
		break;
	case GRIPBTN1:
		m_rctGap.m_gripBtn[1].m_curState = CBtn::NEUTRAL;
		m_trackingState = NOTRACKING;
		ReleaseCapture();
		if(m_rctGap.m_gripBtn[1].PtInRect(point))
            GripPane(1);
		break;
	default:
		ASSERT(m_trackingState == NOTRACKING);
	}

	if(m_lastActivePane != -1)
		SetActivePane(m_lastActivePane);
	
}

void CBiSplitterWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	if(!m_isTrackMouseEvent)
	{
		TRACKMOUSEEVENT trackMouseEvent;
		trackMouseEvent.cbSize = sizeof(TRACKMOUSEEVENT);
		trackMouseEvent.dwFlags = TME_LEAVE;
		trackMouseEvent.hwndTrack = m_hWnd;
		_TrackMouseEvent(&trackMouseEvent);
		m_isTrackMouseEvent = TRUE;
	}
	switch(m_trackingState)
	{
	case SPLITTER:
			TrackSplitter(point);
		break;
	case CLOSEBTN0:
		{
			int hitTest = HitTest(point);
			if(hitTest == closeBtn0 && m_panes[0].m_closeBtn.m_curState != CBtn::DOWN)
			{
	            CClientDC dc(this);
		        m_panes[0].m_closeBtn.m_curState = CBtn::DOWN;
			    DrawCloseBtn(&dc, m_panes);
			}
			else if(hitTest != closeBtn0 && m_panes[0].m_closeBtn.m_curState != CBtn::NEUTRAL)
			{
				CClientDC dc(this);
				m_panes[0].m_closeBtn.m_curState = CBtn::NEUTRAL;
				DrawCloseBtn(&dc, m_panes);
			}
		}
		break;
	case CLOSEBTN1:
		{
			int hitTest = HitTest(point);
			if(hitTest == closeBtn1 && m_panes[1].m_closeBtn.m_curState != CBtn::DOWN)
			{
	            CClientDC dc(this);
		        m_panes[1].m_closeBtn.m_curState = CBtn::DOWN;
			    DrawCloseBtn(&dc, m_panes + 1);
			}
			else if(hitTest != closeBtn1 && m_panes[1].m_closeBtn.m_curState != CBtn::NEUTRAL)
			{
				CClientDC dc(this);
				m_panes[1].m_closeBtn.m_curState = CBtn::NEUTRAL;
				DrawCloseBtn(&dc, m_panes + 1);
			}
		}
		break;
	case GRIPBTN0:
		{
			int hitTest = HitTest(point);
			if(hitTest == gripBtn0 && m_rctGap.m_gripBtn[0].m_curState != CBtn::DOWN)
			{
	            CClientDC dc(this);
		        m_rctGap.m_gripBtn[0].m_curState = CBtn::DOWN;
			    DrawGripBtn(&dc, 0);
			}
			else if(hitTest != gripBtn0 && m_rctGap.m_gripBtn[0].m_curState != CBtn::NEUTRAL)
			{
				CClientDC dc(this);
				m_rctGap.m_gripBtn[0].m_curState = CBtn::NEUTRAL;
				DrawGripBtn(&dc, 0);
			}
		}
		break;
	case GRIPBTN1:
		{
			int hitTest = HitTest(point);
			if(hitTest == gripBtn1 && m_rctGap.m_gripBtn[1].m_curState != CBtn::DOWN)
			{
	            CClientDC dc(this);
		        m_rctGap.m_gripBtn[1].m_curState = CBtn::DOWN;
			    DrawGripBtn(&dc, 1);
			}
			else if(hitTest != gripBtn1 && m_rctGap.m_gripBtn[1].m_curState != CBtn::NEUTRAL)
			{
				CClientDC dc(this);
				m_rctGap.m_gripBtn[1].m_curState = CBtn::NEUTRAL;
				DrawGripBtn(&dc, 1);
			}
		}
		break;
	case NOTRACKING:
		{
			int hitTest = HitTest(point);
			if(hitTest == closeBtn0 && m_panes[0].m_closeBtn.m_curState != CBtn::UP)
			{
	            CClientDC dc(this);
		        m_panes[0].m_closeBtn.m_curState = CBtn::UP;
			    DrawCloseBtn(&dc, m_panes);
			}
			else if(hitTest != closeBtn0 && m_panes[0].m_closeBtn.m_curState != CBtn::NEUTRAL)
			{
				CClientDC dc(this);
				m_panes[0].m_closeBtn.m_curState = CBtn::NEUTRAL;
				DrawCloseBtn(&dc, m_panes);
			}
			else if(hitTest == closeBtn1 && m_panes[1].m_closeBtn.m_curState != CBtn::UP)
			{
	            CClientDC dc(this);
		        m_panes[1].m_closeBtn.m_curState = CBtn::UP;
			    DrawCloseBtn(&dc, m_panes + 1);
			}
			else if(hitTest != closeBtn1 && m_panes[1].m_closeBtn.m_curState != CBtn::NEUTRAL)
			{
				CClientDC dc(this);
				m_panes[1].m_closeBtn.m_curState = CBtn::NEUTRAL;
				DrawCloseBtn(&dc, m_panes + 1);
			}

			else if(hitTest == gripBtn0 && m_rctGap.m_gripBtn[0].m_curState != CBtn::UP)
			{
	            CClientDC dc(this);
		        m_rctGap.m_gripBtn[0].m_curState = CBtn::UP;
			    DrawGripBtn(&dc, 0);
			}
			else if(hitTest != gripBtn0 && m_rctGap.m_gripBtn[0].m_curState != CBtn::NEUTRAL)
			{
				CClientDC dc(this);
				m_rctGap.m_gripBtn[0].m_curState = CBtn::NEUTRAL;
				DrawGripBtn(&dc, 0);
			}

			else if(hitTest == gripBtn1 && m_rctGap.m_gripBtn[1].m_curState != CBtn::UP)
			{
	            CClientDC dc(this);
		        m_rctGap.m_gripBtn[1].m_curState = CBtn::UP;
			    DrawGripBtn(&dc, 1);
			}
			else if(hitTest != gripBtn1 && m_rctGap.m_gripBtn[1].m_curState != CBtn::NEUTRAL)
			{
				CClientDC dc(this);
				m_rctGap.m_gripBtn[1].m_curState = CBtn::NEUTRAL;
				DrawGripBtn(&dc, 1);
			}

		}
		break;
	default:
		ASSERT(FALSE);
	}
	CWnd::OnMouseMove(nFlags, point);
}

BOOL CBiSplitterWnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
    CPoint pt;
    ::GetCursorPos(&pt);
	ScreenToClient(&pt);
	int hitTest = HitTest(pt);

	if(hitTest == splitterBar || m_trackingState == SPLITTER/*m_bTracking*/)
	{
		SetCursor((m_bswStyles & VSPLITTER ? m_hSplitterCursorV : m_hSplitterCursorH));
		return TRUE;
	}
	SetCursor(m_hArrowCursor);
	return TRUE;

//	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

void CBiSplitterWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting
//draw all items
	DrawSplitterGap(&dc, m_rctGap);
	for( CPane *pPane = m_panes; pPane < m_panes + 2; pPane++)
	{	
		DrawPane(&dc, pPane);
	}

	//DrawSplitterGap(&dc, m_rctGap);

	//for( CPane *pPane = m_panes; pPane < m_panes + 2; pPane++)
	//{	
	//	if(pPane->IsVisible())
	//	{
	//		DrawPane(&dc, pPane);
 //           pPane->m_pView->UpdateWindow();
	//	}
	//}
	////draw splitter gap
}

LRESULT CBiSplitterWnd::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
    for(int i = 0; i < 2; ++i)
	{
        if(m_panes[i].m_closeBtn.m_curState != CBtn::NEUTRAL)
		{
            CClientDC dc(this);
            m_panes[i].m_closeBtn.m_curState = CBtn::NEUTRAL;
            DrawCloseBtn(&dc, m_panes + i);
		}
        if(m_rctGap.m_gripBtn[i].m_curState != CBtn::NEUTRAL)
		{
            CClientDC dc(this);
            m_rctGap.m_gripBtn[i].m_curState = CBtn::NEUTRAL;
            DrawGripBtn(&dc, i);
		}
	}
	m_isTrackMouseEvent = FALSE;
	return TRUE;	
}

void CBiSplitterWnd::OnCaptureChanged(CWnd *pWnd)
{
	switch(m_trackingState)
	{
	case SPLITTER:
		DrawTracker(m_rctTracker);
		break;
	case CLOSEBTN0:
		{
			m_panes[0].m_closeBtn.m_curState = CBtn::NEUTRAL;
            CClientDC dc(this);
			DrawCloseBtn(&dc, m_panes);
		}
		break;
	case CLOSEBTN1:
		{
			m_panes[1].m_closeBtn.m_curState = CBtn::NEUTRAL;
			CClientDC dc(this);
			DrawCloseBtn(&dc, m_panes + 1);
		}
		break;
	case GRIPBTN0:
		{
			m_rctGap.m_gripBtn[0].m_curState = CBtn::NEUTRAL;
            CClientDC dc(this);
			DrawGripBtn(&dc, 0);
		}
		break;
	case GRIPBTN1:
		{
			m_rctGap.m_gripBtn[1].m_curState = CBtn::NEUTRAL;
            CClientDC dc(this);
			DrawGripBtn(&dc, 1);
		}
		break;
	default:
		ASSERT(m_trackingState == NOTRACKING);
	}

	m_trackingState = NOTRACKING;
	m_isTrackMouseEvent = FALSE;
	CWnd::OnCaptureChanged(pWnd);
}

void CBiSplitterWnd::OnCancelMode()
{
	CWnd::OnCancelMode();

	if (GetCapture() == this)
		ReleaseCapture();
}

void CBiSplitterWnd::OnDisplayChange()
{
	UpdateSysMetrics();
	UpdateSysColors();
	UpdateSysImages();
	RecalcLayout();
}

void CBiSplitterWnd::PostNcDestroy()
{
	// TODO: Add your specialized code here and/or call the base class
	if(m_autoDelete)
        delete this;
	CWnd::PostNcDestroy();
}

