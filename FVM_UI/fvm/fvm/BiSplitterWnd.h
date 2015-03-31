#pragma once


// CBiSplitterWnd
#define NOTHING 0
#define BSW_FIRST_PANE (UINT)0
#define BSW_SECOND_PANE (UINT)1

enum BSW { VSPLITTER = 0x0001, HSPLITTER = 0x0002,
			FIXED0 = 0x0004, FIXED1 = 0x0008, PROPORTIONAL = 0x0010, VIEW3D = 0x0020,
			SIZING_MODE_STYLES = FIXED0 | FIXED1 | PROPORTIONAL,//can not be nothing
			SPLITTER_STYLES = VSPLITTER | HSPLITTER, //can not be nothing
			ALL_BISPLITTER_STYLES = SIZING_MODE_STYLES | SPLITTER_STYLES | VIEW3D
};
	// splitter direction
	// VSPLITTER - vertical splitter bar
	// HSPLITTER - horizontal splitter bar
	// how changing panes width when window width is changed 
	// PROPORTIONAL - proportional changing panes width 
	// FIXED0	- left pane width is fixed
	// FIXED1 - right pane width is fixed

enum BSP {THIN_BORDER = 0x0001, THICK_BORDER = 0x0002,
			THIN_CAPTIONBORDER = 0x0004, THICK_CAPTIONBORDER = 0x0008,
			SMCAPTION = 0x0010, CAPTION = 0x0020,
			THIN_BUTTON = 0x0040, THICK_BUTTON = 0x0080, FLAT_BUTTON = 0x0100,
			CLOSEBTN = 0x0200, GRIPBTN = 0x0400, SMGRIPBTN = 0x0800,
			BORDER_STYLES = THIN_BORDER | THICK_BORDER,
			CAPTION_STYLES = SMCAPTION | CAPTION,
			CAPTIONBORDER_STYLES = THIN_CAPTIONBORDER | THICK_CAPTIONBORDER,
			BUTTON_STYLES = THIN_BUTTON | THICK_BUTTON | FLAT_BUTTON,//can not be nothing
			GRIP_STYLES = GRIPBTN | SMGRIPBTN,
			ALL_PANE_STYLES = CLOSEBTN | BORDER_STYLES | CAPTION_STYLES | CAPTIONBORDER_STYLES | BUTTON_STYLES | GRIP_STYLES

};

class CBtn: public CRect
{
public:
	enum BTN_STATE {UP, DOWN, NEUTRAL};
	CBtn(): m_styles(THIN_BORDER),
				m_curState(NEUTRAL),
				CRect(0,0,0,0)
	{
		
	}

	virtual ~CBtn()
	{
	}
//attributes
	UINT m_styles;			// THIN_BUTTON = 0x0040, THICK_BUTTON = 0x0080, FLAT_BUTTON = 0x0100,
	BTN_STATE m_curState;	// UP, DOWN, NEUTRAL 
//operations
	inline void MoveToY(int y) throw()
	{ 
		this->bottom = this->Height() + y; 
		this->top = y; 
	}
	inline void MoveToX(int x) throw()
	{ 
		this->right = this->Width() + x; this->left = x; 
	}

};

class CGap: public CRect
{
public:
	CGap():m_isVisible(TRUE)
	{
	}

	BOOL m_isVisible;

	inline void ShowGap(BOOL val = TRUE) {m_isVisible = val;}
	inline void HideGap(BOOL val = FALSE){m_isVisible = val;}
	inline BOOL IsVisible()const{return m_isVisible;}
	void MoveGapToX(int n)
	{
		MoveToX(n);
		m_gripBtn[0].MoveToX(n);
		m_gripBtn[1].MoveToX(n);
	}

	void MoveGapToY(int n)
	{
		MoveToY(n);
		m_gripBtn[0].MoveToY(n);
		m_gripBtn[1].MoveToY(n);
	}

	CBtn m_gripBtn[2];//buttons to grip panes
private:
	inline void MoveToY(int y) throw()
	{ 
		this->bottom = this->Height() + y; 
		this->top = y; 
	}
	inline void MoveToX(int x) throw()
	{ 
		this->right = this->Width() + x; this->left = x; 
	}
};


class CPane //consists of border, possible caption and view window inside
{
public:
	CPane(): m_bspStyles(0),
		m_isGripped(FALSE),
        m_isVisible(TRUE),
        m_pView(NULL),
        m_sCaptionText(_T("")),
        m_rctPaneBox(0, 0, 0, 0),
        m_rctCaption(0, 0, 0, 0), 
//		m_closeBtn(0, 0, 0, 0),
        m_rctViewBox(0, 0, 0, 0)
	{
	}

	virtual ~CPane()
	{
	}

//attributes
	UINT m_bspStyles;//bisplitter pane styles
	BOOL m_isGripped;
	BOOL m_isVisible;//is pane visible
    CWnd *m_pView;//pointer to the pane view window. NULL if there is not view window for pane
    CString m_sCaptionText;
    CRect m_rctPaneBox;	//pane rectangle coordinates 
	CRect m_rctCaption;
	CBtn m_closeBtn;
    CRect m_rctViewBox; //view window coordinates

//operations
	inline BOOL IsVisible()const {return m_isVisible;}
	inline BOOL IsGripped()const {return m_isGripped;}
	inline void ShowPane() {m_isVisible = TRUE;}
	inline void HidePane(){m_isVisible = FALSE;}
	inline void GripPane() {m_isGripped = TRUE;}
	inline void ExpandPane() {m_isGripped = FALSE;}
};



class CBiSplitterWnd : public CWnd
{
	DECLARE_DYNCREATE(CBiSplitterWnd)
// Construction
public:
	
//	CToolTipCtrl tip;


	enum SPLITTER {HORIZONTAL, VERTICAL};
	CBiSplitterWnd();
	virtual BOOL Create(CWnd *pWnd, UINT bswStyles, UINT nID = AFX_IDW_PANE_FIRST);
	virtual BOOL CreatePane(UINT nPane, UINT bspStyles, LPCTSTR lpszPaneCaptionText = NULL, CWnd *pWnd = NULL);
	virtual CWnd* CreateView(CRuntimeClass* pBiSplitterWndClass, UINT nID, UINT bswStyles);
	virtual CWnd* CreateView(CRuntimeClass* pWndClass, UINT nID);
	virtual CWnd* CreateView(CRuntimeClass* pViewClass, UINT nID, CCreateContext* pContext);

// Attributes
public:
	UINT GetSplitterGap();
	void SetSplitterGap(UINT nGap);

	int GetSplitterPos();
	void SetSplitterPos(int nSplitterPos);

	CWnd *GetPaneView(UINT nPane);
	UINT GetPaneViewID(UINT nPane);

	void SetPaneCaptionText(UINT nPane, const CString &sPaneCaptionText);
	CString GetPaneCaptionText(UINT nPane);

// Operations
public:
	virtual UINT AcquireStyles();
	virtual void ChangeStyles(UINT excludeStyles, UINT includeStyles);
	virtual UINT AcquirePaneStyles(UINT nPane);
	virtual void ChangePaneStyles(UINT nPane, UINT excludeStyles, UINT includeStyles);
	virtual void AssignViewToPane(UINT viewID, UINT nPane);
	virtual void RecalcLayout();    // call after changing sizes
	virtual void HidePane(UINT nPane);
	virtual void HidePane(CWnd *pView);
	virtual void ShowPane(CWnd *pView);
	virtual void ShowPane(UINT nPane);
	virtual void GripPane(UINT nPane);
	virtual void SetActivePane(UINT nPane);
	virtual int GetActivePane();
	virtual BOOL IsPaneVisible(UINT nPane);
	virtual BOOL IsWndVisible(){return (m_hWnd && (GetStyle() & WS_VISIBLE)); }
	virtual CWnd *GetView(UINT viewID);
// Overridables
protected:
public:
// Implementation
public:

	virtual ~CBiSplitterWnd();

protected:
	BOOL m_autoDelete;
	UINT m_bswStyles;
	int m_lastActivePane;
	enum TRACKING_STATE {NOTRACKING, SPLITTER, CLOSEBTN0, CLOSEBTN1, GRIPBTN0, GRIPBTN1};

	CPane m_panes[2];
	//system colors
	COLORREF m_clrBtnShadow;
	COLORREF m_clrBtnHilite;
	COLORREF m_clrWindowFrame;
	COLORREF m_clrBtnFace;
	COLORREF m_clrBtnText;
	COLORREF m_clrFlatBorder;
	COLORREF m_clrFlatBtnActiveFace;
	COLORREF m_clrFlatBtnActiveBorder;

	//splitter
	UINT m_nGap;	
//	CRect m_rctGap;
	CGap m_rctGap;
	// splitter position is the splitter offset from the window left border for vertical splitter or 
	//   window top border for horozontal splitter.
	int m_nSplitterPos;
	// splitter position is represented as fractionl m_nSplitterNumerator/m_nSplitterDenominator   
	int m_nSplitterNumerator;
	int m_nSplitterDenominator;

	HCURSOR m_hSplitterCursorV;
	HCURSOR m_hSplitterCursorH;
	HCURSOR m_hArrowCursor;

	//caption metrics
	int m_nSmCaptionHeight;	//small caption height
	CFont m_fntSmCaption;
	CFont m_fntSmSymbol;	//special symbols like close button image
	int m_nBtnSmCX;			//small caption button width 
	int m_nBtnSmCY;			//small caption button height

	int m_nCaptionHeight;	//caption height
	CFont m_fntCaption;
	CFont m_fntSymbol;		//special symbols like close button image
	int m_nBtnCX;			//caption button width 
	int m_nBtnCY;			//caption button height


	//tracker
//	BOOL m_bTracking;
	TRACKING_STATE m_trackingState;
	CRect m_rctTracker;

	TCHAR *m_images;
	struct GLYPHINFO
	{
		BITMAPINFO *pBitmapInfo;
		BYTE *pBits;
	} *m_pGlyphs, *m_pSmGlyphs;

	int m_glyphsCount; //the size of the images arrays

	BOOL m_isTrackMouseEvent; //flag for _TrackMouseEvent starting
	// implementation routines
	void RecalcPane(UINT nPane, int left, int top, int right, int bottom);
	void RecalcGap(int left, int top, int right, int bottom);
	void SetPaneEmpty(CPane *pPane);
	void DrawPane(CDC *pDC, CPane *pPane);
	void DrawCaption(CDC *pDC, CPane *pPane);
	void DrawCloseBtn(CDC *pDC, CPane *pPane);
	void DrawGripBtn(CDC *pDC, int nGrip);
	void DrawBtn(CDC *pDC, CBtn *pBtn, int nImageIndex, UINT styles, BOOLEAN smImageSize);
	void DrawSplitterGap(CDC *pDC, const CRect &rect);
	inline int BorderWidth(int bspStyles){ return (UINT)(bspStyles & BORDER_STYLES); }
	inline int CaptionBorderWidth(int bspStyles){ return ((UINT)(bspStyles & CAPTIONBORDER_STYLES))>>2; }
	void UpdateSysColors();
	void UpdateSysMetrics();
	void UpdateSysImages();

	//tracking routines
	void DrawTracker(const CRect& rect);
	void StartTracking(int ht);
	void TrackSplitter(const CPoint &point);
	void StopTracking();
	int HitTest(CPoint pt) const;

	//auxiliary routines
	inline UINT OtherPane(UINT nPane){ return nPane ^ 1;}  // 0 if nPane = 1 and 1 if nPane = 0

private:
	void RclHideBothPanes(void);
	void RclExpandSecondPane(const CRect &rctClient, BOOL isGapVisible);
	void RclExpandFirstPane(const CRect &rctClient, BOOL isGapVisible);
	void RclPanesV(const CRect &rctClient);
	void RclPanesH(const CRect &rctClient);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnNcCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnPaint();
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	afx_msg void OnCaptureChanged(CWnd *pWnd);
	afx_msg void OnCancelMode();
	afx_msg void OnDisplayChange();
protected:
	virtual void PostNcDestroy();
};


