// CreateDialog.cpp : implementation file
//

#include "stdafx.h"
#include "fvm.h"
#include "CreateDialog.h"
#include "FolderDlg.h"
#include "MainFrm.h"
#include "FvmBrowser.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCreateDialog dialog


CCreateDialog::CCreateDialog(CWnd* pParent /*=NULL*/,LPCTSTR title)
	: CDialog(CCreateDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCreateDialog)
	m_FolderStr = _T("");
	m_FvmStr = _T("");
	m_FvmIpStr = _T("");
	m_FvmIpMaskStr = _T("");
	//}}AFX_DATA_INIT

	m_Parent = (CMainFrame *) pParent;
	//SetWindowText(L"Hello");
	if(title == NULL)
		Title = L"Create FVM";
	else
		Title = title;
}


void CCreateDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCreateDialog)
	DDX_Control(pDX, IDC_FVM_NAME, m_Fvm);
	DDX_Control(pDX, IDC_FVM_FOLDER, m_Folder);
	DDX_Control(pDX, IDC_FVM_IP, m_FvmIp);
	DDX_Control(pDX, IDC_FVM_IP_MASK, m_FvmIpMask);
	DDX_Control(pDX, IDOK, m_OKCtrl);

	DDX_Text(pDX, IDC_FVM_FOLDER, m_FolderStr);
	DDX_Text(pDX, IDC_FVM_NAME, m_FvmStr);
	DDX_Text(pDX, IDC_FVM_IP, m_FvmIpStr);
	DDX_Text(pDX, IDC_FVM_IP_MASK, m_FvmIpMaskStr);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCreateDialog, CDialog)
	//{{AFX_MSG_MAP(CCreateDialog)
	ON_BN_CLICKED(IDC_BUTTON1, OnBrowse)
	ON_EN_CHANGE(IDC_FVM_FOLDER, OnChangeFvmFolder)
	ON_EN_CHANGE(IDC_FVM_NAME, OnChangeFvmName)
	ON_EN_CHANGE(IDC_FVM_IP, OnChangeFvmIp)
	ON_EN_CHANGE(IDC_FVM_IP_MASK, OnChangeFvmIpMask)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCreateDialog message handlers

void CCreateDialog::OnBrowse() 
{
	// TODO: Add your control notification handler code here
	
	CString			m_strRoot;
	
	LPCTSTR lpszTitle = _T( "Select a folder to store FVM data." );
	UINT	uFlags	  = BIF_RETURNONLYFSDIRS | BIF_USENEWUI|BIF_DONTGOBELOWDOMAIN;
	
	CFolderDialog dlgRoot( lpszTitle, m_strRoot, this, uFlags );
		
	if( dlgRoot.DoModal() == IDOK )
	{
		m_strRoot = dlgRoot.GetFolderPath();
			m_Folder.SetWindowText(m_strRoot);
		UpdateData( FALSE );
	}


}

BOOL CCreateDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	m_OKCtrl.EnableWindow((m_FolderStr!="" && m_FvmStr != "")?TRUE:FALSE);
	m_Fvm.SetFocus();
	
	SetWindowText(Title);

	return FALSE;
	  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCreateDialog::OnChangeFvmFolder() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
		UpdateData(TRUE);
		m_OKCtrl.EnableWindow((m_FolderStr!="" && m_FvmStr != "")?TRUE:FALSE);
}

void CCreateDialog::OnChangeFvmName() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here

		UpdateData(TRUE);
		m_OKCtrl.EnableWindow((m_FolderStr!="" && m_FvmStr != "")?TRUE:FALSE);
	
}

void CCreateDialog::OnOK() 
{
	// TODO: Add extra validation here
	
	//MessageBox(m_FolderStr);
	//MessageBox(m_FvmStr);

	GUID id1;
	unsigned char mask = (unsigned char) 0x0f;


	TCHAR buffer[1024];
  
	CoCreateGuid(&id1); 
	StringFromGUID2(id1, buffer, 1024); 
	CString str = buffer;
	str.TrimLeft('{');
	str.TrimRight('}');
	str.Remove('-');


	if(m_Parent->FvmDataFind(m_FvmStr)){
		MessageBox(L"The FVM "+m_FvmStr+ L" exists. Please choose a new name");
		return;
	}
	CString vfs = m_FolderStr+L"\\"+str;
	if(!CreateDirectory(vfs,NULL)){
		MessageBox(L"The direcotry " + m_FolderStr + L" is not writable, please choose a new directory.");
		return;
	}


	HKEY key;
	DWORD type;
	DWORD bytes;
	CString regStr;

	regStr.LoadString(IDS_RNIFVM_REG);
	if (RegCreateKey(HKEY_CURRENT_USER, regStr, &key)==ERROR_SUCCESS)
	{
		RegCloseKey(key);

		regStr.LoadString(IDS_VMS_REG);
		if (RegCreateKey(HKEY_CURRENT_USER, regStr,&key)==ERROR_SUCCESS){
			type = REG_SZ;
			if (RegCreateKey(HKEY_CURRENT_USER, regStr + "\\" + m_FvmStr, &key)==ERROR_SUCCESS){
				bytes = (m_FolderStr.GetLength()+1)*sizeof(TCHAR);
				RegSetValueEx(key, _T("fvmroot"), NULL, type, (LPBYTE)m_FolderStr.GetBuffer(bytes-1), bytes);
				m_FolderStr.ReleaseBuffer();
				bytes = (str.GetLength()+1)*sizeof(TCHAR);
				RegSetValueEx(key, _T("fvmid"), NULL, type, (LPBYTE)str.GetBuffer(bytes-1), bytes);	
				str.ReleaseBuffer();
				bytes = (m_FvmIpStr.GetLength()+1)*sizeof(TCHAR);
				RegSetValueEx(key, _T("fvmip"), NULL, type, (LPBYTE)m_FvmIpStr.GetBuffer(bytes-1), bytes);			
				m_FvmIpStr.ReleaseBuffer();
				bytes = (m_FvmIpMaskStr.GetLength()+1)*sizeof(TCHAR);
				RegSetValueEx(key, _T("fvmipmask"), NULL, type, (LPBYTE)m_FvmIpMaskStr.GetBuffer(bytes-1), bytes);			
				m_FvmIpMaskStr.ReleaseBuffer();
				bytes = (1+1)*sizeof(TCHAR);
				RegSetValueEx(key, _T("schedPriority"), NULL, type, (LPBYTE)"&", bytes);	
				
				bytes = (1+1)*sizeof(TCHAR);
				RegSetValueEx(key, _T("maxProcesses"), NULL, type, (LPBYTE)"&", bytes);

				bytes = (1+1)*sizeof(TCHAR);
				RegSetValueEx(key, _T("maxComMemory"), NULL, type, (LPBYTE)"&", bytes);	
				
				bytes = (1+1)*sizeof(TCHAR);
				RegSetValueEx(key, _T("maxWorkSet"), NULL, type, (LPBYTE)"&", bytes);
			}
		}
	}
	else{

		MessageBox(L"Could not save fvm information. Creating fvm aborts.");
		CDialog::OnOK();
	}

	t_FvmData *td = new t_FvmData;
	td->fvmID = str;
	td->fvmName = m_FvmStr;
	td->fvmRoot = m_FolderStr;
	td->fvmIp = m_FvmIpStr;
	td->fvmIpMask = m_FvmIpMaskStr;
	td->schedPriority = "&";
	td->maxProcesses = "&";
	td->maxComMemory = "&";
	td->maxWorkSet = "&";

	td->status = 0;
	td->suspendlist = NULL;
	td->suspendworkingset = NULL;
	td->next = NULL;
	m_Parent->FvmDataAdd(td);
	m_Parent->m_selectedFvm = td;
	m_Parent->UpdateDisplays();
	m_Parent->created = td;
		/*

	CFvmBrowser *fb = m_Parent->GetFvmBrowserView();
	CListCtrl& lc = fb-> GetListCtrl();

	lc.InsertItem(0, m_FvmStr, 1);
	*/
	m_Parent->GetFvmBrowserView()->InsertFvms();
	m_Parent->UpdateDisplays();
	CDialog::OnOK();
}

void CCreateDialog::OnChangeFvmIp() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
		UpdateData(TRUE);
		m_OKCtrl.EnableWindow((m_FolderStr!="" && m_FvmStr != "")?TRUE:FALSE);
	
}

void CCreateDialog::OnChangeFvmIpMask() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
		UpdateData(TRUE);
		m_OKCtrl.EnableWindow((m_FolderStr!="" && m_FvmStr != "")?TRUE:FALSE);
	
}
