#ifndef AFX_DIRTREECTRL_H__1E4F97A0_B41E_11D2_955E_204C4F4F5020__INCLUDED_
#define AFX_DIRTREECTRL_H__1E4F97A0_B41E_11D2_955E_204C4F4F5020__INCLUDED_

// DirTreeCtrl.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Fenster CDirTreeCtrl 
class CFvmDirView;

class CDirTreeCtrl : public CTreeCtrl
{
// Konstruktion
public:
	CDirTreeCtrl();

// Attribute
public:

// Operationen
public:

// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CDirTreeCtrl)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementierung
public:
	bool m_dont_notify;
	BOOL SetSelPath( CString strPath );
	CString GetFullPath( HTREEITEM hItem ) const;
	LPCTSTR GetSubPath( LPCTSTR strPath );
	BOOL DisplayTree(LPCTSTR strRoot, BOOL bFiles = FALSE);
	virtual ~CDirTreeCtrl();
	CFvmDirView *m_pOwner;

	// Drag&Drop
	BOOL DragPosition(CImageList *pImageList, CWnd* pDragWnd, CPoint point);
	void DragLeave(CImageList *pImageList);
	void OnDragEnd(int target, CPoint point);
	CString GetDropTarget() const;
	HTREEITEM m_hDragSource;
	void RefreshDir(CString dir);
	
	// Generierte Nachrichtenzuordnungsfunktionen
protected:
	CString m_transferpass;
	CString m_transferuser;
	void CDirTreeCtrl::UploadDir(CString dir, CString subdir, BOOL upload);
	BOOL IsValidPath(LPCTSTR strPath);
	void ExpandItem(HTREEITEM hItem, UINT nCode);
	HTREEITEM SearchSiblingItem(HTREEITEM hItem, LPCTSTR strText);
	BOOL FindSubDir(LPCTSTR strPath);
	HTREEITEM AddItem(HTREEITEM hParent, LPCTSTR strPath);
	void DisplayPath(HTREEITEM hParent, LPCTSTR strPath);
	BOOL DisplayDrives();
	CString m_strError;
	BOOL GetSysImgList();
	void RefreshDir(HTREEITEM hRefreshItem);

	// Drag&Drop
	HTREEITEM m_hDragHilited;

	//{{AFX_MSG(CDirTreeCtrl)
	afx_msg void OnItemexpanded(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnLocaltreecontextAddtoqueue();
	afx_msg void OnLocaltreecontextDelete();
	afx_msg void OnDirViewRename();
	afx_msg void OnLocaltreecontextUpload();
	afx_msg void OnLocaltreecontextUploadas();
	afx_msg void OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio fügt zusätzliche Deklarationen unmittelbar vor der vorhergehenden Zeile ein.

#endif // AFX_DIRTREECTRL_H__1E4F97A0_B41E_11D2_955E_204C4F4F5020__INCLUDED_
