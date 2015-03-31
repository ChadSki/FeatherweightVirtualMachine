// ConfigDialog.cpp : implementation file
//

#include "stdafx.h"
#include "fvm.h"
#include "ConfigDialog.h"
#include "FolderDlg.h"
#include "MainFrm.h"
#include "FvmBrowser.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConfigDialog dialog

CConfigDialog::CConfigDialog(CWnd* pParent /*=NULL*/, POSITION pos, LPCTSTR title)
	: CDialog(CConfigDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfigDialog)
	m_mpStr = _T("");
	m_spStr = _T("");
	m_mcmStr = _T("");
	m_mwsStr = _T("");
	//}}AFX_DATA_INIT

	m_Parent = (CMainFrame *) pParent;

	CFvmBrowser * vmb = m_Parent->GetFvmBrowserView();
	CListCtrl& lc = vmb->GetListCtrl();
	if (pos == NULL)
	{
		MessageBox(L"Please choose a vm first.");
		return;
	}
	int nItem = lc.GetNextSelectedItem(pos);
	CString vmname = lc.GetItemText(nItem, 0);
	fvmd  = m_Parent->FvmDataFind(vmname);

	//SetWindowText(L"Hello");
	if(title == NULL)
		Title = L"Configure FVM";
	else
		Title = title;
}


void CConfigDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigDialog)
	DDX_Control(pDX, IDC_SCHED_PRIORITY, m_sp);
	DDX_Control(pDX, IDC_MAX_PROCESSES, m_mp);
	DDX_Control(pDX, IDC_MAX_COMMITTED, m_mcm);
	DDX_Control(pDX, IDC_MAX_WORK_SET, m_mws);
	DDX_Control(pDX, IDOK, m_OKCtrl);

	DDX_Text(pDX, IDC_MAX_PROCESSES, m_mpStr);
//	DDX_Text(pDX, IDC_SCHED_PRIORITY, m_spStr);
	DDX_Text(pDX, IDC_MAX_COMMITTED, m_mcmStr);
	DDX_Text(pDX, IDC_MAX_WORK_SET, m_mwsStr);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfigDialog, CDialog)
	//{{AFX_MSG_MAP(CConfigDialog)
	ON_EN_CHANGE(IDC_MAX_PROCESSES, OnChangeMaxProcesses)
	ON_LBN_SELCHANGE(IDC_SCHED_PRIORITY, OnChangeSchePriority)
	ON_EN_CHANGE(IDC_MAX_COMMITTED, OnChangeMaxComMemory)
	ON_EN_CHANGE(IDC_MAX_WORK_SET, OnChangeMaxWorkSet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigDialog message handlers

BOOL CConfigDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	//m_OKCtrl.EnableWindow((fvmd->m_mpStr!="" && fvmd->m_spStr != "")?TRUE:FALSE);
	m_OKCtrl.EnableWindow(TRUE);
	m_sp.SetFocus();
		
	SetWindowText(Title);

	m_sp.AddString(L"IDLE_PRIORITY_CLASS");
	m_sp.AddString(L"BELOW_NORMAL_PRIORITY_CLASS");
	m_sp.AddString(L"NORMAL_PRIORITY_CLASS");
	m_sp.AddString(L"ABOVE_NORMAL_PRIORITY_CLASS");
	m_sp.AddString(L"HIGH_PRIORITY_CLASS");
	m_sp.AddString(L"REALTIME_PRIORITY_CLASS");

	if(fvmd->schedPriority!="&")
	{
		m_mp.SetWindowText(fvmd->maxProcesses);
		m_mcm.SetWindowText(fvmd->maxComMemory);
		m_mws.SetWindowText(fvmd->maxWorkSet);
		if(fvmd->schedPriority=="IDLE_PRIORITY_CLASS")
			m_sp.SetCurSel(0);
		if(fvmd->schedPriority=="BELOW_NORMAL_PRIORITY_CLASS")
			m_sp.SetCurSel(1);
		if(fvmd->schedPriority=="NORMAL_PRIORITY_CLASS")
			m_sp.SetCurSel(2);
		if(fvmd->schedPriority=="ABOVE_NORMAL_PRIORITY_CLASS")
			m_sp.SetCurSel(3);
		if(fvmd->schedPriority=="HIGH_PRIORITY_CLASS")
			m_sp.SetCurSel(4);
		if(fvmd->schedPriority=="REALTIME_PRIORITY_CLASS")
			m_sp.SetCurSel(5);
	}

	m_spStr = fvmd->schedPriority;
	m_mpStr = fvmd->maxProcesses;
	m_mcmStr = fvmd->maxComMemory;
	m_mwsStr = fvmd->maxWorkSet;

	return FALSE;

}

void CConfigDialog::OnChangeMaxProcesses() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	m_OKCtrl.EnableWindow(TRUE);
}

void CConfigDialog::OnChangeSchePriority() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here

		UpdateData(TRUE);
		m_sp.GetText(m_sp.GetCurSel(), m_spStr);
		//m_OKCtrl.EnableWindow((m_mpStr!="" && m_spStr != "")?TRUE:FALSE);
		m_OKCtrl.EnableWindow(TRUE);
}

void CConfigDialog::OnOK() 
{
	// TODO: Add extra validation here

	GUID id1;
	unsigned char mask = (unsigned char) 0x0f;
	TCHAR buffer[1024];
  
	CoCreateGuid(&id1); 
	StringFromGUID2(id1, buffer, 1024); 
	CString str = buffer;
	str.TrimLeft('{');
	str.TrimRight('}');
	str.Remove('-');

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
			if (RegCreateKey(HKEY_CURRENT_USER, regStr + "\\" + fvmd->fvmName, &key)==ERROR_SUCCESS){
				
				bytes = (m_mpStr.GetLength()+1)*sizeof(TCHAR);
				RegSetValueEx(key, _T("maxProcesses"), NULL, type, (LPBYTE)m_mpStr.GetBuffer(bytes-1), bytes);
				m_mpStr.ReleaseBuffer();
				
				bytes = (m_spStr.GetLength()+1)*sizeof(TCHAR);
				RegSetValueEx(key, _T("schedPriority"), NULL, type, (LPBYTE)m_spStr.GetBuffer(bytes-1), bytes);	
				m_spStr.ReleaseBuffer();

				bytes = (m_mcmStr.GetLength()+1)*sizeof(TCHAR);
				RegSetValueEx(key, _T("maxComMemory"), NULL, type, (LPBYTE)m_mcmStr.GetBuffer(bytes-1), bytes);	
				m_mcmStr.ReleaseBuffer();

				bytes = (m_mwsStr.GetLength()+1)*sizeof(TCHAR);
				RegSetValueEx(key, _T("maxWorkSet"), NULL, type, (LPBYTE)m_mwsStr.GetBuffer(bytes-1), bytes);	
				m_mwsStr.ReleaseBuffer();
			}
		}
	}
	else{

		MessageBox(L"Could not save fvm configuration.");
		CDialog::OnOK();
	}

	fvmd->schedPriority = m_spStr;
	fvmd->maxProcesses = m_mpStr;
	fvmd->maxComMemory = m_mcmStr;
	fvmd->maxWorkSet = m_mwsStr;

	CDialog::OnOK();
}

void CConfigDialog::OnChangeMaxComMemory() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
		UpdateData(TRUE);
		//m_OKCtrl.EnableWindow((m_mpStr!="" && m_spStr != "")?TRUE:FALSE);
		m_OKCtrl.EnableWindow(TRUE);
	
}

void CConfigDialog::OnChangeMaxWorkSet() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
		UpdateData(TRUE);
		//m_OKCtrl.EnableWindow((m_mpStr!="" && m_spStr != "")?TRUE:FALSE);
		m_OKCtrl.EnableWindow(TRUE);
	
}
