// fvmDoc.h : interface of the CFvmDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_FVMDOC_H__0D236FF1_9B84_4D06_8FDA_049D6823769D__INCLUDED_)
#define AFX_FVMDOC_H__0D236FF1_9B84_4D06_8FDA_049D6823769D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CFvmDoc : public CDocument
{
protected: // create from serialization only
	CFvmDoc();
	DECLARE_DYNCREATE(CFvmDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFvmDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFvmDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CFvmDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FVMDOC_H__0D236FF1_9B84_4D06_8FDA_049D6823769D__INCLUDED_)
