// LeftView.cpp : implementation of the CLeftView class
//

#include "stdafx.h"
#include "fvm.h"

#include "fvmDoc.h"
#include "LeftView.h"
#include "Utility.h"
#include "MainFrm.h"
#include "fvmView.h"
#include "REgView.h"
#include "fvmData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define C_COLUMNS		3
#include "MainFrm.h"
// CLeftView

IMPLEMENT_DYNCREATE(CLeftView, CTreeView)

BEGIN_MESSAGE_MAP(CLeftView, CTreeView)
ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnTvnSelchanged)
ON_NOTIFY_REFLECT(TVN_ITEMEXPANDING, OnTvnItemexpanding)
END_MESSAGE_MAP()


// CLeftView construction/destruction

CLeftView::CLeftView()
{
	// TODO: add construction code here
	hMyComputer = NULL;
	hHCR = NULL;
	hHCU = NULL;
	hHLM = NULL;
	hHU = NULL;
	hHCC = NULL;
}

CLeftView::~CLeftView()
{
	m_ImageList.DeleteImageList();
}

BOOL CLeftView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying the CREATESTRUCT cs

	cs.style = cs.style | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_HASLINES;

	return CTreeView::PreCreateWindow(cs);
}

void CLeftView::OnInitialUpdate()
{
	CTreeView::OnInitialUpdate();

	// TODO: You may populate your TreeView with items by directly accessing
	//  its tree control through a call to GetTreeCtrl().

	//Create ImageList
	m_ImageList.Create( 16, 16, ILC_COLOR32, 0, 4);
	CBitmap bm;
	bm.LoadBitmap(IDB_TREE_IMAGE_LIST);
	m_ImageList.Add(&bm, RGB(0, 0, 0));
	m_ImageList.Add( LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_STRING)) );
	m_ImageList.Add( LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_BINARY)) );

	bm.DeleteObject();

	CTreeCtrl* pCtrl = &GetTreeCtrl();
	ASSERT(pCtrl != NULL);

	pCtrl->SetImageList( &m_ImageList, TVSIL_NORMAL);

	CRegView *pRegView = ((CMainFrame *)AfxGetMainWnd())->GetRegView();
	if(pRegView)
	{
		CListCtrl *pListCtrl = &(pRegView->GetListCtrl());
		ASSERT(pListCtrl);
        pListCtrl->SetImageList( &m_ImageList, LVSIL_SMALL);
	}



	DisplayRegistry(NULL);
}


// CLeftView diagnostics

#ifdef _DEBUG
void CLeftView::AssertValid() const
{
	CTreeView::AssertValid();
}

void CLeftView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}

CRegDoc* CLeftView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFvmDoc)));
	return (CRegDoc*)m_pDocument;
}
#endif //_DEBUG


void CLeftView::OnTvnSelchanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	
	
    CTreeCtrl* pCtrl = &GetTreeCtrl();
	ASSERT(pCtrl != NULL);

	//Find HKEY and Full KeyName
	HKEY hKey = NULL;
	CString sFullKeyName = _T("");

	HTREEITEM hCurrentItem;
	hCurrentItem = pNMTreeView->itemNew.hItem;

	HTREEITEM hParent = hCurrentItem;
	HTREEITEM hNextItem;
	
	while( hParent != NULL )
	{
		hNextItem = pCtrl->GetParentItem(hParent);
		
		{
            CString sKeyName = pCtrl->GetItemText(hParent);
			
			if( lstrcmp(sKeyName, _T("My Computer")) == 0 )
			{
				sKeyName = _T("");
			}

			if( lstrcmp(sKeyName, _T("HKEY_CLASSES_ROOT")) == 0 )
			{
				hKey = HKEY_CLASSES_ROOT;
			}
			else if( lstrcmp(sKeyName, _T("HKEY_CURRENT_USER")) == 0 )
			{
				hKey = HKEY_CURRENT_USER;
			}
			else if( lstrcmp(sKeyName, _T("HKEY_LOCAL_MACHINE")) == 0 )
			{
				hKey = HKEY_CURRENT_USER;
				sFullKeyName= L"fvms\\"+current_Fvm->fvmID+L"\\REGISTRY\\MACHINE\\"+sFullKeyName;
			}
			else if( lstrcmp(sKeyName, _T("HKEY_USERS")) == 0 )
			{
				hKey = HKEY_CURRENT_USER;
				sFullKeyName= L"fvms\\"+current_Fvm->fvmID+L"\\REGISTRY\\USER\\"+sFullKeyName;
			}
			else if( lstrcmp(sKeyName, _T("HKEY_CURRENT_CONFIG")) == 0 )
			{
				hKey = HKEY_CURRENT_CONFIG;
			}
			else
			{
				if(sKeyName.GetLength()>0)
                    sFullKeyName = sKeyName + "\\" + sFullKeyName;
			}
		}

		hParent = hNextItem;
	}


	//ssageBox(sFullKeyName);
	if( (hKey==NULL) )
		return;
	//Upto here

	//Insert column into List View i.e. RegView
	CRegView *pRegView = ((CMainFrame *)AfxGetMainWnd())->GetRegView();

	if(pRegView)
	{
		CListCtrl *pListCtrl = &(pRegView->GetListCtrl());
		
		ASSERT(pListCtrl);

		pRegView->ListEnumKey.clear();

		EnumRegistryKeyValue( hKey, sFullKeyName, pRegView->ListEnumKey);

		ListView_DeleteAllItems( pListCtrl->GetSafeHwnd());

		INT dIndex = -1;
		LV_ITEM lvI;
		lvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
		lvI.state = 0;
		lvI.stateMask = 0;

		
		if(pRegView->ListEnumKey.size()>0)
		{
			for( unsigned int i=0; i<pRegView->ListEnumKey.size(); i++ )
			{
				lvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
				lvI.iItem = i;
				lvI.iSubItem = 0;
				
				if( (pRegView->ListEnumKey[i].dwRegType == REG_SZ) ||(pRegView->ListEnumKey[i].dwRegType == REG_EXPAND_SZ)||(pRegView->ListEnumKey[i].dwRegType == REG_MULTI_SZ) )
				{
                    lvI.iImage = 3;
				}
				else
				{
                    lvI.iImage = 4;
				}

				lvI.pszText = pRegView->ListEnumKey[i].sRegKeyName.GetBuffer(pRegView->ListEnumKey[i].sRegKeyName.GetLength());	
			    
				dIndex = ListView_InsertItem(pListCtrl->GetSafeHwnd(), &lvI); 

				lvI.mask = TVIF_TEXT;
                lvI.iItem = dIndex;
                lvI.iSubItem = 1;
				lvI.pszText = _T("");
                
				if( pRegView->ListEnumKey[i].dwRegType == REG_SZ )
				{
                    lvI.pszText = _T("REG_SZ");
				}
				else if( pRegView->ListEnumKey[i].dwRegType == REG_EXPAND_SZ )
				{
					lvI.pszText = _T("REG_EXPAND_SZ");
				}
				else if( pRegView->ListEnumKey[i].dwRegType == REG_MULTI_SZ )
				{
					lvI.pszText = _T("REG_MULTI_SZ");
				}				
				else if( pRegView->ListEnumKey[i].dwRegType == REG_BINARY )
				{
                    lvI.pszText = _T("REG_BINARY");
				}
				else if( pRegView->ListEnumKey[i].dwRegType == REG_DWORD )
				{
                    lvI.pszText = _T("REG_DWORD");
				}
				else if( pRegView->ListEnumKey[i].dwRegType == REG_RESOURCE_LIST )
				{
                    lvI.pszText = _T("REG_RESOURCE_LIST");
				}
				else if( pRegView->ListEnumKey[i].dwRegType == REG_FULL_RESOURCE_DESCRIPTOR )
				{
                    lvI.pszText = _T("REG_FULL_RESOURCE_DESCRIPTOR");
				}
				else if( pRegView->ListEnumKey[i].dwRegType == REG_RESOURCE_REQUIREMENTS_LIST )
				{
                    lvI.pszText = _T("REG_RESOURCE_REQUIREMENTS_LIST");
				}
							    
				ListView_SetItem(pListCtrl->GetSafeHwnd(), &lvI);

				lvI.mask = TVIF_TEXT;
                lvI.iItem = dIndex;
                lvI.iSubItem = 2;
				lvI.pszText = _T("");
                
				if( pRegView->ListEnumKey[i].dwRegType == REG_SZ )
				{
                    lvI.pszText = pRegView->ListEnumKey[i].m_RegData.sData.GetBuffer(pRegView->ListEnumKey[i].m_RegData.sData.GetLength());
					//essageBox(lvI.pszText);
					ListView_SetItem(pListCtrl->GetSafeHwnd(), &lvI);
				}
				else if( pRegView->ListEnumKey[i].dwRegType == REG_EXPAND_SZ )
				{
					lvI.pszText = pRegView->ListEnumKey[i].m_RegData.sData.GetBuffer(pRegView->ListEnumKey[i].m_RegData.sData.GetLength());
					ListView_SetItem(pListCtrl->GetSafeHwnd(), &lvI);
				}
				else if( pRegView->ListEnumKey[i].dwRegType == REG_MULTI_SZ )
				{
					CString TempStr = pRegView->ListEnumKey[i].m_RegData.sData;

					for(int j=0;j<TempStr.GetLength();j++)
					{
						if( TempStr[j]== L'\r' )
						{
							TempStr.SetAt(j, L' ');
						}
					}

//					lvI.pszText = (char*)pRegView->ListEnumKey[i].m_RegData.sData.c_str();
					lvI.pszText = TempStr.GetBuffer(TempStr.GetLength());
					ListView_SetItem(pListCtrl->GetSafeHwnd(), &lvI);
				}
				else if( (pRegView->ListEnumKey[i].dwRegType==REG_BINARY)|| (pRegView->ListEnumKey[i].dwRegType==REG_RESOURCE_LIST) || (pRegView->ListEnumKey[i].dwRegType==REG_FULL_RESOURCE_DESCRIPTOR) || (pRegView->ListEnumKey[i].dwRegType==REG_RESOURCE_REQUIREMENTS_LIST) )
//				else if( pRegView->ListEnumKey[i].dwRegType == REG_BINARY )
				{
					if(pRegView->ListEnumKey[i].m_RegData.pData!=NULL)
					{
						lvI.pszText = pRegView->ListEnumKey[i].m_RegData.pData;
					}
                    else
					{
						lvI.pszText = _T("");
					}

					ListView_SetItem(pListCtrl->GetSafeHwnd(), &lvI);

					delete[] pRegView->ListEnumKey[i].m_RegData.pData;
				}
				else if( pRegView->ListEnumKey[i].dwRegType == REG_DWORD )
				{
					TCHAR str[MAX_PATH];
					wsprintf( str, L"0x%08X(%ld)", pRegView->ListEnumKey[i].m_RegData.dwData, (long int)pRegView->ListEnumKey[i].m_RegData.dwData);
                    lvI.pszText = str;

					ListView_SetItem(pListCtrl->GetSafeHwnd(), &lvI);
				}
				else
				{
					ListView_SetItem(pListCtrl->GetSafeHwnd(), &lvI);
				}
			}
		}
//		ListEnumKey.clear();
	}
	//Upto here
	//Upto here

	*pResult = 0;
}

void CLeftView::OnTvnItemexpanding(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here

	if( pNMTreeView->action != TVE_EXPAND )
		return;


	CTreeCtrl* pCtrl = &GetTreeCtrl();
	ASSERT(pCtrl != NULL);

	//Find HKEY and Full KeyName
	HKEY hKey = NULL;
	CString sFullKeyName = "";

	HTREEITEM hCurrentItem;
	hCurrentItem = pNMTreeView->itemNew.hItem;

	HTREEITEM hParent = hCurrentItem;
//	HTREEITEM hNextItem;
	

		*pResult = 0;
	if(hCurrentItem){

		CString sKeyName = pCtrl->GetItemText(hCurrentItem);

		if( lstrcmp(sKeyName, _T("HKEY_LOCAL_MACHINE")) != 0  && lstrcmp(sKeyName, _T("HKEY_USERS")) != 0)
			return;


		HTREEITEM hCurrent = pCtrl->GetNextItem(hCurrentItem, TVGN_CHILD);
		while (hCurrent != NULL)
		{
	

			CString name = pCtrl->GetItemText(hCurrent);

			if(name == L"rni_temp"){
				pCtrl->DeleteItem(hCurrent);
				
				if(sKeyName == L"HKEY_LOCAL_MACHINE"){
					LamEnumRegKey(L"HKEY_LOCAL_MACHINE", HKEY_CURRENT_USER, L"fvms\\"+current_Fvm->fvmID+L"\\REGISTRY\\MACHINE",hCurrentItem, pCtrl);

				}
				else if(sKeyName == L"HKEY_USERS")
					LamEnumRegKey(L"HKEY_USERS", HKEY_CURRENT_USER, L"fvms\\"+current_Fvm->fvmID+L"\\REGISTRY\\USER", hCurrentItem, pCtrl);

				//
				return;
			}
			hCurrent = pCtrl->GetNextItem(hCurrent, TVGN_NEXT);		
		}

		
	}


}

BYTE orgData[MAX_REG_KEY_VALUE];
BYTE newData[MAX_REG_KEY_VALUE];

int CLeftView::LamEnumRegKey(CString hive, HKEY hKey, CString sKeyName, HTREEITEM hItem, CTreeCtrl *pCtrl)
{
	LONG retcode = ERROR_SUCCESS;
	HKEY hOpenKey = NULL;
	TCHAR str[MAX_PATH];
	HTREEITEM ci;
	int itemCount=0;
	int ret;
	memset( str, '\0', sizeof(str));
//	ListEnumKey.clear();
	
	//MessageBox(NULL, sKeyName, sKeyName, MB_OK);
	if(sKeyName.GetLength()>0)
        retcode = RegOpenKeyEx( hKey, sKeyName, 0, KEY_ALL_ACCESS,&hOpenKey);
	else
		return 0;

	if( retcode != (DWORD)ERROR_SUCCESS )
		return 0;


	for (int i = 0, retCode = ERROR_SUCCESS;  retCode == ERROR_SUCCESS; i++) 
    {   
		retCode = RegEnumKey(hOpenKey, 
                     i, 
                     str, 
                     MAX_PATH
					); 

        
		if (retCode == (DWORD)ERROR_SUCCESS) 
        {
			CString sNewKeyName;
			sNewKeyName = str;
			
			if(sNewKeyName.GetLength()>0){
               ;// ListEnumKey.push_back( sNewKeyName );
				ci =	pCtrl->InsertItem(TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE, sNewKeyName,
					0, 1, 0, 0, 0, hItem, NULL);
				ret = LamEnumRegKey(hive, hKey, sKeyName+L"\\"+sNewKeyName, ci, pCtrl);
				if(ret)
					itemCount++;
				else
					pCtrl->DeleteItem(ci);
			
			}
        
		}		
	}
	
	if(itemCount){
		RegCloseKey( hOpenKey );
		return 1;
	}

	DWORD type;
	DWORD data;
	DWORD size = sizeof(data);
	CString orig;
	CString prefix;
	HKEY root;
	TCHAR *ptr;
	HKEY orHd;
	DWORD orgSubKeys, newSubKeys;
	DWORD orgValues, newValues;

	if(RegQueryValueEx(hOpenKey,L"SEES_INITIAL",NULL, &type, (unsigned char *)&data, &size) != ERROR_SUCCESS){

			orig = sKeyName;
		if(hive == L"HKEY_LOCAL_MACHINE"){
		
			prefix = L"fvms\\"+current_Fvm->fvmID+L"\\REGISTRY\\MACHINE\\";
			root = HKEY_LOCAL_MACHINE;
		}
		else if(hive == L"HKEY_USERS"){
		
		    prefix = L"fvms\\"+current_Fvm->fvmID+L"\\REGISTRY\\USER\\";
			root = HKEY_USERS;
		}

		ptr = sKeyName.GetBuffer(sKeyName.GetLength());
		orig = ptr + prefix.GetLength();


		if(RegOpenKeyEx(root, orig, 0, KEY_ALL_ACCESS,&orHd) != ERROR_SUCCESS){
			RegCloseKey(hOpenKey);
			return 1;

		}

		RegQueryInfoKey(orHd, NULL, NULL, NULL, &orgSubKeys, NULL, NULL, &orgValues, NULL, NULL, NULL, NULL);
		RegQueryInfoKey(hOpenKey, NULL, NULL, NULL, &newSubKeys, NULL, NULL, &newValues, NULL, NULL, NULL, NULL);


		if(orgSubKeys != newSubKeys || orgValues != newValues){
			RegCloseKey(orHd);
			RegCloseKey(hOpenKey);
			return 1;

		}


		DWORD orgType;
	
		DWORD cbData;

		DWORD newType;
	
		DWORD newCbData;
		int j;


				newCbData = MAX_REG_KEY_VALUE;
			cbData = MAX_REG_KEY_VALUE;

		if(RegQueryValueEx(orHd, NULL, NULL, &orgType, orgData, &cbData) == ERROR_SUCCESS &&
			RegQueryValueEx(hOpenKey, NULL, NULL, &newType, newData, &newCbData) == ERROR_SUCCESS)
		{

			if(cbData != newCbData){
				RegCloseKey(orHd);
				RegCloseKey(hOpenKey);
				return 1;
			}

			for(j = 0; j < newCbData; j++){
				if(orgData[j]!=newData[j]){
					RegCloseKey(orHd);
					RegCloseKey(hOpenKey);
					return 1;

				}

			}
		}




		LONG retcode = ERROR_SUCCESS;

		DWORD dwType = REG_SZ;
		TCHAR str[MAX_REG_KEY_NAME];
		memset( str, '\0', sizeof(str));
	


	

		
		DWORD Size;
		DWORD dwNo = 0;

		

		for (int i = 0, retCode = ERROR_SUCCESS; 
            retCode == ERROR_SUCCESS; i++) 
		{
			Size = MAX_REG_KEY_NAME;
			newCbData = MAX_REG_KEY_VALUE;
			cbData = MAX_REG_KEY_VALUE;

			retCode = RegEnumValue(hOpenKey,  i, str, &Size, NULL, &newType, newData, &newCbData);

			if (retCode != (DWORD) ERROR_NO_MORE_ITEMS)// && retCode != ERROR_INSUFFICIENT_BUFFER)
			{


				if(RegQueryValueEx(orHd, str, NULL, &orgType, orgData, &cbData) != ERROR_SUCCESS){

					RegCloseKey(orHd);
					RegCloseKey(hOpenKey);
					return 1;

				}
			

				if(cbData != newCbData){
					RegCloseKey(orHd);
					RegCloseKey(hOpenKey);
					return 1;
				}

				for(j = 0; j < newCbData; j++){
					if(orgData[j]!=newData[j]){
						RegCloseKey(orHd);
						RegCloseKey(hOpenKey);
						return 1;

					}

				}
				

				retCode = ERROR_SUCCESS;
			}
		}

		if(hKey)
			RegCloseKey( hOpenKey );



		//MessageBox(orig);


		RegCloseKey(orHd);
		RegCloseKey(hOpenKey);
		return 0;
	}
	
	RegCloseKey( hOpenKey );
	return 0;
}
void CLeftView::DisplayRegistry(t_FvmData *fvmd)
{


	if(fvmd == NULL)
		return;


	CTreeCtrl* pCtrl = &GetTreeCtrl();
	ASSERT(pCtrl != NULL);

	TVINSERTSTRUCT tvInsert;
	tvInsert.hParent = NULL;
	tvInsert.hInsertAfter = NULL;
	tvInsert.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tvInsert.item.pszText = _T("My Computer");
	tvInsert.item.iImage = 2;
	tvInsert.item.iSelectedImage = 2;


	hMyComputer = pCtrl->InsertItem(&tvInsert);
	vector< CString > ListEnumKey;

	current_Fvm = fvmd;

	hHLM = pCtrl->InsertItem(_T("HKEY_LOCAL_MACHINE"),	0, 1, hMyComputer, hHCU);
	//LamEnumRegKey(HKEY_CURRENT_USER, L"fvms\\"+fvmd->fvmID+L"\\REGISTRY\\MACHINE",hHLM, pCtrl);

	pCtrl->InsertItem(_T("rni_temp"), 0, 1, hHLM);
	hHU = pCtrl->InsertItem(_T("HKEY_USERS"), 0, 1, hMyComputer, hHLM);
	pCtrl->InsertItem(_T("rni_temp"), 0, 1, hHU);
	//Insert Item into HKEY_USERS
	//LamEnumRegKey( /*HKEY_USERS*/HKEY_CURRENT_USER, L"fvms\\"+fvmd->fvmID+L"\\REGISTRY\\USER", hHU, pCtrl);

	pCtrl->Expand(hMyComputer, TVE_EXPAND);
	//pCtrl->DeleteItem(hHLM);
	return;
	
	/*
	hHCR = pCtrl->InsertItem(_T("HKEY_CLASSES_ROOT"), 
	0, 1, hMyComputer, hMyComputer);

	//Insert Item into HKEY_CLASSES_ROOT
	vector< CString > ListEnumKey;
	ListEnumKey.clear();

	EnumRegistryKey(HKEY_CURRENT_USER, "", ListEnumKey);

	if(ListEnumKey.size()>0)
	{
		CTreeCtrl* pCtrl = &GetTreeCtrl();
		ASSERT(pCtrl != NULL);

		for( unsigned int i=0; i<ListEnumKey.size(); i++ )
		{
			pCtrl->InsertItem(TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE, ListEnumKey[i],
					0, 1, 0, 0, 0, hHCR, NULL);
		}
	}
	ListEnumKey.clear();
	//upto here

	hHCU = pCtrl->InsertItem(_T("HKEY_CURRENT_USER"),
	0, 1, hMyComputer, hHCR);

	//Insert Item into HKEY_CURRENT_USER
	EnumRegistryKey( HKEY_CURRENT_USER, "", ListEnumKey);

	if(ListEnumKey.size()>0)
	{
		CTreeCtrl* pCtrl = &GetTreeCtrl();
		ASSERT(pCtrl != NULL);

		for( unsigned int i=0; i<ListEnumKey.size(); i++ )
		{
			pCtrl->InsertItem(TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE, ListEnumKey[i],
					0, 1, 0, 0, 0, hHCU, NULL);
		}
	}
	ListEnumKey.clear();
	//upto here

	*/
	hHLM = pCtrl->InsertItem(_T("HKEY_LOCAL_MACHINE"),	0, 1, hMyComputer, hHCU);

	//Insert Item into HKEY_LOCAL_MACHINE
	EnumRegistryKey( /*HKEY_LOCAL_MACHINE*/HKEY_CURRENT_USER, L"fvms\\"+fvmd->fvmID+L"\\REGISTRY\\MACHINE", ListEnumKey);

	if(ListEnumKey.size()>0)
	{
		CTreeCtrl* pCtrl = &GetTreeCtrl();
		ASSERT(pCtrl != NULL);

		for( unsigned int i=0; i<ListEnumKey.size(); i++ )
		{
			pCtrl->InsertItem(TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE, ListEnumKey[i],
					0, 1, 0, 0, 0, hHLM, NULL);
		}
	}
	ListEnumKey.clear();
	//upto here

	hHU = pCtrl->InsertItem(_T("HKEY_USERS"),
	0, 1, hMyComputer, hHLM);

	//Insert Item into HKEY_USERS
	EnumRegistryKey( /*HKEY_USERS*/HKEY_CURRENT_USER, L"fvms\\"+fvmd->fvmID+L"\\REGISTRY\\USER" , ListEnumKey);

	if(ListEnumKey.size()>0)
	{
		CTreeCtrl* pCtrl = &GetTreeCtrl();
		ASSERT(pCtrl != NULL);

		for( unsigned int i=0; i<ListEnumKey.size(); i++ )
		{
			pCtrl->InsertItem(TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE, ListEnumKey[i],
					0, 1, 0, 0, 0, hHU, NULL);
		}
	}
	ListEnumKey.clear();
	//upto here
/*
#if(WINVER >= 0x0400)
	hHCC = pCtrl->InsertItem(_T("HKEY_CURRENT_CONFIG"),
	0, 1, hMyComputer, hHU);

	//Insert Item into HKEY_USERS
	EnumRegistryKey( HKEY_CURRENT_CONFIG, "", ListEnumKey);

	if(ListEnumKey.size()>0)
	{
		CTreeCtrl* pCtrl = &GetTreeCtrl();
		ASSERT(pCtrl != NULL);

		for( unsigned int i=0; i<ListEnumKey.size(); i++ )
		{
			pCtrl->InsertItem(TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE, ListEnumKey[i],
					0, 1, 0, 0, 0, hHCC, NULL);
		}
	}
	ListEnumKey.clear();
	//upto here
#endif
	*/
}
