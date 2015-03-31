// SBDestination.cpp: implementation of the CSBDestination class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SBDestination.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSBDestination::CSBDestination(const HWND hParent, const int nTitleID)
	: CBrowseForFolder(hParent, NULL, nTitleID)
{

}

CSBDestination::~CSBDestination()
{

}

void CSBDestination::SetInitialSelection(const CString & strPath)
{
	m_strInitialSelection = strPath;
}

void CSBDestination::OnInit() const
{
	SetSelection(m_strInitialSelection);
	SetStatusText(m_strInitialSelection);
}

void CSBDestination::OnSelChanged(const LPITEMIDLIST pidl) const
{
	CString strBuffer;
	if (SHGetPathFromIDList(pidl, strBuffer.GetBuffer(MAX_PATH)))
		strBuffer.ReleaseBuffer();
	else
		strBuffer.ReleaseBuffer(0);
	SetStatusText(strBuffer);
}
