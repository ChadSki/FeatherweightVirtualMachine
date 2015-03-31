// RegView.cpp : implementation of the CRegView class
//

#include "stdafx.h"
#include "fvm.h"

#include "fvmDoc.h"
#include "RegView.h"
#include "utility.h"
//#include "EditDword.h"
//#include "EditString.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CRegView

IMPLEMENT_DYNCREATE(CRegView, CListView)

BEGIN_MESSAGE_MAP(CRegView, CListView)
	ON_WM_STYLECHANGED()
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
END_MESSAGE_MAP()

// CRegView construction/destruction

CRegView::CRegView()
{
	// TODO: add construction code here

}

CRegView::~CRegView()
{
}

BOOL CRegView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.style = cs.style | LVS_REPORT | LVS_SINGLESEL;

	return CListView::PreCreateWindow(cs);
}

void CRegView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();

	// TODO: You may populate your ListView with items by directly accessing
	//  its list control through a call to GetListCtrl().

	CListCtrl *pListCtrl = &GetListCtrl();

	ASSERT( pListCtrl );
	
	LVCOLUMN col;
	col.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
	col.pszText = _T("Name");
	col.fmt = LVCFMT_LEFT;
	col.cx = 70;
	pListCtrl->InsertColumn( 0, &col);

	col.pszText = _T("Type");
	pListCtrl->InsertColumn( 1, &col);

    col.pszText = _T("Data");
	pListCtrl->InsertColumn( 2, &col);
}


// CRegView diagnostics

#ifdef _DEBUG
void CRegView::AssertValid() const
{
	CListView::AssertValid();
}

void CRegView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

CFvmDoc* CRegView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFvmDoc)));
	return (CFvmDoc*)m_pDocument;
}
#endif //_DEBUG


// CRegView message handlers
void CRegView::OnStyleChanged(int /*nStyleType*/, LPSTYLESTRUCT /*lpStyleStruct*/)
{
	//TODO: add code to react to the user changing the view style of your window
}

void CRegView::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;

	/*
	LPNMITEMACTIVATE lpnmitem = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	if( lpnmitem->iItem == -1 )
		return;

	CListCtrl *pListCtrl = &GetListCtrl();
	char str[MAX_REG_KEY_NAME];
	char Type[MAX_PATH];
	char Data[MAX_REG_KEY_VALUE];

	pListCtrl->GetItemText( lpnmitem->iItem, 0, str, sizeof(str) );
	pListCtrl->GetItemText( lpnmitem->iItem, 1, Type, sizeof(str) );
    pListCtrl->GetItemText( lpnmitem->iItem, 2, Data, sizeof(str) );

	if( (lstrlen(str)<0)||(lstrlen(Type)<0) )
		return;

	if( (lstrcmp(Type, "REG_SZ")== 0)||(lstrcmp(Type, "REG_EXPAND_SZ")== 0) )
	{
		CEditString m_DlgEditString;
		lstrcpy( m_DlgEditString.sKeyData, ListEnumKey[lpnmitem->iItem].m_RegData.sData.c_str() );
		lstrcpy( m_DlgEditString.sKeyName, str );
		lstrcpy( m_DlgEditString.sKeyType, Type );

		m_DlgEditString.DoModal();
	}
	else if( lstrcmp(Type, "REG_MULTI_SZ")== 0 )
	{
		CEditString m_DlgEditString;

		string TempStr = ListEnumKey[lpnmitem->iItem].m_RegData.sData;

		for(int i=0;i<TempStr.length();i++)
		{
			if( TempStr.c_str()[i]=='\r' )
			{
                TempStr.replace(i, 1, "\r\n");
			}
		}
		
		lstrcpy( m_DlgEditString.sKeyData, TempStr.c_str() );
		lstrcpy( m_DlgEditString.sKeyName, str );
		lstrcpy( m_DlgEditString.sKeyType, Type );

		m_DlgEditString.DoModal();
	}
	else if( lstrcmp(Type, "REG_DWORD")== 0 )
	{
		CEditDword m_DlgEditDword;
		
		lstrcpy( m_DlgEditDword.sKeyData, Data );
		lstrcpy( m_DlgEditDword.sKeyName, str );
		lstrcpy( m_DlgEditDword.sKeyType, Type );

		m_DlgEditDword.DoModal();
	}
	*/
}
