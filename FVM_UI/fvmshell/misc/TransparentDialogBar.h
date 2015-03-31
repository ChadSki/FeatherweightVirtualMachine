#if !defined(AFX_TRANSPARENTDIALOGBAR_H__F248118C_2EEC_4B02_BD30_0F712F2FC53F__INCLUDED_)
#define AFX_TRANSPARENTDIALOGBAR_H__F248118C_2EEC_4B02_BD30_0F712F2FC53F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TransparentDialogBar.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Fenster CTransparentDialogBar 

class CTransparentDialogBar : public CDialogBar
{
// Konstruktion
public:
	CTransparentDialogBar();

// Attribute
protected:
	CBrush m_Brush;

// Operationen
public:

// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CTransparentDialogBar)
	public:
	//}}AFX_VIRTUAL

// Implementierung
public:
	virtual ~CTransparentDialogBar();

	// Generierte Nachrichtenzuordnungsfunktionen
protected:
	//{{AFX_MSG(CTransparentDialogBar)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMove(int x, int y);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_TRANSPARENTDIALOGBAR_H__F248118C_2EEC_4B02_BD30_0F712F2FC53F__INCLUDED_
