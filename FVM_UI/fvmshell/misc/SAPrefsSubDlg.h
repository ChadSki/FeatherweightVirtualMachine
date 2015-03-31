/*********************************************************************

   Copyright (C) 2000 Smaller Animals Software, Inc.

   This software is provided 'as-is', without any express or implied
   warranty.  In no event will the authors be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.

   3. This notice may not be removed or altered from any source distribution.

   http://www.smalleranimals.com
   smallest@smalleranimals.com

**********************************************************************/
// SAPrefsSubDlg.h: interface for the CSAPrefsSubDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SAPREFSSUBDLG_H__26CFAEA8_91FC_11D3_A10C_00500402F30B__INCLUDED_)
#define AFX_SAPREFSSUBDLG_H__26CFAEA8_91FC_11D3_A10C_00500402F30B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CSAPrefsSubDlg : public CDialog  
{
public:
	DECLARE_DYNCREATE(CSAPrefsSubDlg)

	CSAPrefsSubDlg();
   CSAPrefsSubDlg(UINT nID, CWnd *pParent = NULL);
	virtual ~CSAPrefsSubDlg();

   UINT GetID()      {return m_id;}
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
   virtual void OnOK();
   virtual void OnCancel();

   DECLARE_MESSAGE_MAP()
   UINT     m_id;
};

#endif // !defined(AFX_SAPREFSSUBDLG_H__26CFAEA8_91FC_11D3_A10C_00500402F30B__INCLUDED_)
