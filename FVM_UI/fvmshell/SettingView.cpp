// SettingView.cpp : implementation file
//

#include "stdafx.h"
#include "fvmshell.h"
#include "SettingView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSettingView dialog


CSettingView::CSettingView(CWnd* pParent /*=NULL*/)
	: CDialog(CSettingView::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSettingView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSettingView::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSettingView)
	DDX_Control(pDX, IDC_VIEW_LARGE_ICONS, m_RadioLarge);
	DDX_Control(pDX, IDC_VIEW_SMALL_ICONS, m_RadioSmall);
	DDX_Control(pDX, IDC_VIEW_LIST, m_RadioList);
	DDX_Control(pDX, IDC_VIEW_DETAILS, m_RadioDetails);
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSettingView, CDialog)
	//{{AFX_MSG_MAP(CSettingView)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSettingView message handlers

BOOL CSettingView::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_RadioDetails.SetCheck(BST_CHECKED);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
