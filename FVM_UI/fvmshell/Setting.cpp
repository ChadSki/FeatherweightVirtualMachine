// Setting.cpp : implementation file
//

#include "stdafx.h"
#include "fvmshell.h"
#include "Setting.h"
#include "SettingView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetting dialog


CSetting::CSetting(CWnd* pParent /*=NULL*/)
	: CDialog(CSetting::IDD, pParent)
{
	m_DialogFocus = NULL;
	m_Parent = (CMainFrame *)pParent;
	//{{AFX_DATA_INIT(CSetting)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSetting::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetting)
	DDX_Control(pDX, IDC_TAB1, m_SettingTab);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSetting, CDialog)
	//{{AFX_MSG_MAP(CSetting)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, OnSelchangeTab1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetting message handlers

void CSetting::OnOK() 
{
	// TODO: Add extra validation here

	CDialog::OnOK();
}

BOOL CSetting::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	TCITEM tcItem;
	tcItem.mask = TCIF_TEXT;
   tcItem.pszText = _T("View");

   m_SettingTab.InsertItem(0, &tcItem);

   ActivateTabDialogs();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CSetting::ActivateTabDialogs()
{
	int nSel = m_SettingTab.GetCurSel();
	

	if(m_DialogFocus && m_DialogFocus->m_hWnd)
		m_DialogFocus->DestroyWindow();

	//CSettingView sd;
	m_DialogFocus = new CSettingView(this);
	m_DialogFocus->Create(IDD_SETTING_VIEW, m_SettingTab.GetParent());
	//m_DialogFocus->DoModal();

	if(m_DialogFocus == NULL)
		return;
	CRect l_rectClient;
	CRect l_rectWnd;

	m_SettingTab.GetClientRect(l_rectClient);
	m_SettingTab.AdjustRect(FALSE,l_rectClient);
	m_SettingTab.GetWindowRect(l_rectWnd);
	m_SettingTab.GetParent()->ScreenToClient(l_rectWnd);
	l_rectClient.OffsetRect(l_rectWnd.left,l_rectWnd.top);
	m_DialogFocus->SetWindowPos(&wndTop,l_rectClient.left,l_rectClient.top,l_rectClient.Width(),l_rectClient.Height(),SWP_SHOWWINDOW);
	
}

void CSetting::OnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	ActivateTabDialogs();
	*pResult = 0;
}
