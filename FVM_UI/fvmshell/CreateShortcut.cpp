// CreateShortcut.cpp : implementation file
//

#include "stdafx.h"
#include "fvmshell.h"
#include "CreateShortcut.h"
#include "afxdlgs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCreateShortcut dialog


CCreateShortcut::CCreateShortcut(CWnd* pParent /*=NULL*/)
	: CDialog(CCreateShortcut::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCreateShortcut)
	m_String = _T("");
	//}}AFX_DATA_INIT

	m_parentOwner= (CShortCutView *)pParent;
}


void CCreateShortcut::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCreateShortcut)
	DDX_Control(pDX, IDOK, m_OkCtrl);
	DDX_Control(pDX, IDC_SHORTCUT_EDIT, m_pathName);
	DDX_Text(pDX, IDC_SHORTCUT_EDIT, m_String);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCreateShortcut, CDialog)
	//{{AFX_MSG_MAP(CCreateShortcut)
	ON_BN_CLICKED(IDC_SHORTCUT_BROWS, OnShortcutBrows)
	ON_EN_CHANGE(IDC_SHORTCUT_EDIT, OnChangeShortcutEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCreateShortcut message handlers

void CCreateShortcut::OnShortcutBrows() 
{

   CFileDialog fileDlg (TRUE, NULL, NULL,
      OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, L"Application (*.exe;*.com;*.pif;*.cmd;*.bat;*.scf;*.scr)|*.exe;*.com;*.pif;*.cmd;*.bat;*.scf;*.scr||",  this);
   
 
   if( fileDlg.DoModal ()==IDOK )
   {
      CString pathName = fileDlg.GetPathName();
   
	  CString fileTitle = fileDlg.GetFileTitle();
      CString fileName = fileDlg.GetPathName ();
	  
	  m_pathName.SetWindowText(pathName);
   }	 
}



void CCreateShortcut::OnOK() 
{
	// TODO: Add extra validation here/*
	
	CString fileName;
	LPCWSTR ptr;
	m_pathName.GetWindowText(fileName);

	if(fileName == ""){
		MessageBox(L"Please enter a program path.");
		return;
	}
	if(!m_parentOwner->FvmPathIsExeW(fileName)){
		TCHAR buffer[1024];
		wsprintf(buffer, L"%d is not an executable program.", fileName);
		MessageBox(buffer);
		return;
	}

	 int iIcon;
//	 int iIconSel;
	  SHFILEINFO shFinfo;
	  if ( !SHGetFileInfo(fileName,
				0,
				&shFinfo,
				sizeof( shFinfo ),
				SHGFI_ICON | 
				SHGFI_SMALLICON ) )
	  {
		//m_strError = "Error Gettting SystemFileInfo!";
		return;
	  }
	  
	  ptr = wcsrchr(fileName, '\\');
	  if(ptr == NULL)
		  ptr = fileName;
	  else
		  ptr++;
	  
	  iIcon = shFinfo.iIcon;
	  int item =m_parentOwner->m_listCtrl->InsertItem(0,ptr,iIcon);
	  int in = m_parentOwner->shortCuts.Add(fileName);
	  m_parentOwner->m_listCtrl->SetItemData(item, in);
	
	CDialog::OnOK();
}

BOOL CCreateShortcut::OnInitDialog() 
{
	CDialog::OnInitDialog();
		m_OkCtrl.EnableWindow(m_String!=""?TRUE:FALSE);
	// TODO: Add extra initialization here
	m_pathName.SetFocus();
	return FALSE;
 // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCreateShortcut::OnChangeShortcutEdit() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
		UpdateData(TRUE);
	m_OkCtrl.EnableWindow(m_String!=""?TRUE:FALSE);
	// TODO: Add your control notification handler code here
	
}
