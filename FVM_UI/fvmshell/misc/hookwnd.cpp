/*
Module : HOOKWND.CPP
Purpose: Defines the implementation for an MFC class to implement message hooking before CWnd gets there
Created: PJN / 24-02-1999
History: PJN / 21-03-2003 1. Significant rework following comprehensive testing with the Tray icon class.
                          2. Fixed reported resource leaks caused by SetProp thanks to BoundsChecker
         PJN / 29-03-2003 1. A number of functions now use return values instead of using VERIFY internally
                          and not returning an error code.
                          2. Made the Hook and UnHook methods virtual
                          3. HWND -> CWnd mapping which uses the SetProp method now uses ATOMs. This
                          makes the lookup much faster.
                          4. Made additional public methods of the class thread safe.
                          5. Fixed a bug in SizeOfHookChain, FirstInChain, LastInChain and MiddleOfChain
                          which would unnecessarily ASSERT if called for a non hooked window
                          6. Made destructor of the class virtual
                          7. Fixed another SetProp resource leak in the "Remove" method
                          8. Addition of OnHook and OnUnHook virtual functions which are called when a 
                          hook is being installed or removed.
         PJN / 31-03-2003 1. Now includes the concept of auto deletion. This allows heap allocated 
                          CHookWnds to be destroyed when the associated window is destroyed. Again thanks to
                          Martin Richter for this nice addition.
                          2. Reworked the way the code handles calling the WNDPROC hook. The code now 
                          instead the ATL mechanism of installing an assembly language thunk. This results 
                          in much less code required to implement the hook. From 435 lines to c. 300!
                          3. The code also now uses the variable and method naming convention as in the
                          ATL implementation
         PJN / 02-04-2003 1. First hook is now stored in a Windows property. This avoids potential problems
                          identifying the first hook in cases where the window is already subclassed by a
                          thunk such as ATL. Thanks to Martin Richter for spotting this issue.
                          2. Now displays a warning if code is compiled on a non X86 compiler
                          3. Various code tidy up's following testing with trayicon class.
                          4. Fixed a bug where only 2 hooks in the chain of hooks being called.
                          5. Went back to the Hook / Unhook naming convention
                          6. Addition of various public helper methods
         PJN / 17-04-2003 1. Fixed a level 4 unreferenced warning in CHookWnd::WindowProc. Thanks to 
                          Frank Fesevur for reporting this.



Copyright (c) 1999 - 2003 by PJ Naughter.  (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/



/////////////////////////////////  Includes  //////////////////////////////////

#include "stdafx.h"
#include "HookWnd.h"



//////////////////////////////// Statics / Macros /////////////////////////////

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

CCriticalSection CHookWnd::sm_cs;
LPCTSTR CHookWnd::sm_pszFirstHookProp = _T("FirstHookWnd");



////////////////////////////////// Implementation /////////////////////////////

IMPLEMENT_DYNAMIC(CHookWnd, CObject)

CHookWnd::CHookWnd(BOOL bAutoDelete) : m_pfnSuperWindowProc(NULL), m_bAutoDelete(bAutoDelete), m_hWnd(NULL), m_pNextHook(NULL)
{
}

CHookWnd::~CHookWnd()
{
}

BOOL CHookWnd::Hook(CWnd* pWnd)
{
  ASSERT(pWnd);
  return Hook(pWnd->m_hWnd);
}

BOOL CHookWnd::Hook(HWND hWnd)
{
  //Validate our parameters
  ASSERT(m_hWnd == NULL);
  ASSERT(IsWindow(hWnd));

  //Find the first hook if any which is already installed
  CHookWnd* pFirstHook = FirstHook(hWnd);

  //Use a Crit section to make code thread-safe
  CSingleLock sl(&sm_cs, TRUE);

  SetLastError(0);
  //Install the WindowProc
  WNDPROC pfnWndProc = (WNDPROC) SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)WindowProc);

  if ((pfnWndProc == NULL) && (GetLastError() != 0))
    return FALSE;
  if (pFirstHook)
  {
    m_pfnSuperWindowProc = pFirstHook->m_pfnSuperWindowProc;
    m_pNextHook = pFirstHook;
  }
  else
    m_pfnSuperWindowProc = pfnWndProc;
	m_hWnd = hWnd;

  //Store the first hook i.e. "this" in a windows property
  if (pFirstHook)
    RemoveProp(m_hWnd, sm_pszFirstHookProp);
  SetProp(m_hWnd, sm_pszFirstHookProp, (HANDLE) this);

  //Call the virtual function to indicate that the hook has been installed
  OnSubclass(hWnd);

	return TRUE;
}

HWND CHookWnd::Unhook()
{         
  //Validate our parameters
  HWND hWnd = NULL;
  if (m_hWnd)
  {
    //Find the first hook if any which is already installed
    CHookWnd* pFirstHook = FirstHook(m_hWnd);

    //Use a Crit section to make code thread-safe
    CSingleLock sl(&sm_cs, TRUE);

    //We are the first item in the chain
    if (pFirstHook == this)
    {
      //Is there other hooks hanging off us
      if (pFirstHook->m_pNextHook)
      {
            //Setup the first hook as a windows 
			SetProp(m_hWnd, sm_pszFirstHookProp, pFirstHook->m_pNextHook);
      }
      else
      {
		    SetWindowLongPtr(m_hWnd, GWL_WNDPROC, (LONG)m_pfnSuperWindowProc);

        //Remove the first hook as a windows property
        RemoveProp(m_hWnd, sm_pszFirstHookProp);
      }
    }
    else if (pFirstHook)
    {
      //The item is in the middle of the chain, Just remove it from the linked list
      CHookWnd* pHook = pFirstHook;
      while (pHook->m_pNextHook != this)
        pHook = pHook->m_pNextHook;
      ASSERT(pHook);
      ASSERT(pHook->m_pNextHook == this);
      pHook->m_pNextHook = m_pNextHook;

      //Also setup the first hook as a windows property (to balance the calls to SetProp)
      RemoveProp(m_hWnd, sm_pszFirstHookProp);
      SetProp(m_hWnd, sm_pszFirstHookProp, pFirstHook);
    }

    //Reset our member variables 
	  m_pfnSuperWindowProc = NULL;
	  hWnd = m_hWnd;
	  m_hWnd = NULL;
    m_pNextHook = NULL;

    //Call the virtual function to indicate that the hook has been removed
    OnUnsubclass(hWnd);
  }

	return hWnd;
}

BOOL CHookWnd::IsHooked() const
{
  return (m_hWnd != NULL);
}

BOOL CHookWnd::IsHooked(HWND hWnd)
{
  return (FirstHook(hWnd) != NULL);
}

BOOL CHookWnd::IsHooked(CWnd* pWnd)
{
  return (FirstHook(pWnd) != NULL);
}

BOOL CHookWnd::FirstInChain() const
{
  //Use a Crit section to make code thread-safe
  CSingleLock sl(&sm_cs, TRUE);

  //Lookup the HWND we're hooking and if the item found
  //is this one, there we are the first in the chain
  CHookWnd* pFirst = _FirstHook(m_hWnd);
  return (pFirst == this);
}

BOOL CHookWnd::LastInChain() const
{
  //Use a Crit section to make code thread-safe
  CSingleLock sl(&sm_cs, TRUE);

  //Lookup the HWND we're hooking, then traverse to the end 
  //of the chain and see is that the same one as us
  CHookWnd* pTemp = _FirstHook(m_hWnd);
  if (pTemp)
  {
    while (pTemp->m_pNextHook)
      pTemp = pTemp->m_pNextHook;
  }

  return (pTemp == this);
}

BOOL CHookWnd::MiddleOfChain() const
{
  return (!FirstInChain() && !LastInChain() && (SizeOfHookChain() > 1));
}

int CHookWnd::SizeOfHookChain() const
{
  //Use a Crit section to make code thread-safe
  CSingleLock sl(&sm_cs, TRUE);

  //Run along the linked list to accumulate the size of the   
  int nSize = 0;
  CHookWnd* pTemp = _FirstHook(m_hWnd);
  if (pTemp)
  {
    nSize = 1;
    while (pTemp->m_pNextHook)
    {
      ++nSize;
      pTemp = pTemp->m_pNextHook;
    }
  }

  return nSize;
}

CHookWnd* CHookWnd::FirstHook(HWND hWnd)
{
  //Use a Crit section to make code thread-safe
  CSingleLock sl(&sm_cs, TRUE);

  //Let the helper function do all the work
  return _FirstHook(hWnd);  
}

CHookWnd* CHookWnd::FirstHook(CWnd* pWnd)
{
  //Validate our parameters
  ASSERT(pWnd);

  //Pass the buck to the helper function
  return FirstHook(pWnd->GetSafeHwnd());
}

CHookWnd* CHookWnd::_FirstHook(HWND hWnd)
{
  //What will be the return value
  CHookWnd* pFirstHook = NULL;

  //Look up the value using GetProp
  if (IsWindow(hWnd))
    pFirstHook = (CHookWnd*) GetProp(hWnd, sm_pszFirstHookProp);

  return pFirstHook;
}

CHookWnd* CHookWnd::FindHook(HWND hWnd, const CRuntimeClass* pClass)
{
  //Validate our parameters
  ASSERT(IsWindow(hWnd));
  ASSERT(pClass);
	ASSERT(pClass->IsDerivedFrom(RUNTIME_CLASS(CHookWnd)));

  //Use a Crit section to make code thread-safe
	CSingleLock sl(&sm_cs, TRUE);

	//Get the first one and look for a Hook with the specific type
	CHookWnd* pHook = _FirstHook(hWnd);
	while (pHook && !pHook->IsKindOf(pClass))
		pHook = pHook->m_pNextHook;

	return pHook;
}

CHookWnd* CHookWnd::FindHook(CWnd* pWnd, const CRuntimeClass* pClass)
{
  //Validate our parameters
  ASSERT(pWnd);

  //Pass the buck to the helper function
  return FindHook(pWnd->GetSafeHwnd(), pClass);
}

CHookWnd* CHookWnd::CreateHook(HWND hWnd, CRuntimeClass* pClass)
{
  //Validate our parameters
  ASSERT(IsWindow(hWnd));
  ASSERT(pClass);
	ASSERT(pClass->IsDerivedFrom(RUNTIME_CLASS(CHookWnd)));

	//Have we already installed a hook of this type, there is
  //not much sense in installing 2 hooks of the same runtime
  //type
	CHookWnd* pFound = FindHook(hWnd, pClass);
	if (pFound)
		return pFound;

  //Create the new hook
	CHookWnd* pNewHook = (CHookWnd*) pClass->CreateObject();
  if (pNewHook)
  {
	  pNewHook->m_bAutoDelete = TRUE;
	  pNewHook->Hook(hWnd);
  }

	return pNewHook;
}

CHookWnd* CHookWnd::CreateHook(CWnd* pWnd, CRuntimeClass* pClass)
{
  //Validate our parameters
  ASSERT(pWnd);

  //Pass the buck to the helper function
  return CreateHook(pWnd->GetSafeHwnd(), pClass);
}

void CHookWnd::RemoveAll(CHookWnd* pFirstHook)
{
  CHookWnd* pHook = pFirstHook;
  do
  {
    if (pHook)
    {
      CHookWnd* pNextHook = pHook->m_pNextHook;
      pHook->Unhook();
      if (pHook->m_bAutoDelete)
        delete pHook;
      pHook = pNextHook;
    }
  }
  while (pHook);
}

void CHookWnd::RemoveAll(HWND hWnd)
{
  //Validate our parameters
  ASSERT(hWnd);
  ASSERT(IsWindow(hWnd));

  //Pass the buck to the helper function
  RemoveAll(FirstHook(hWnd));
}

LRESULT CALLBACK CHookWnd::WindowProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
  ASSERT(hWnd != NULL);

  //Set local copies of variables
  HWND temphWnd = hWnd;
  CHookWnd* pThis = FirstHook(hWnd);
 
  //Validate our parameters
  ASSERT(!IsBadReadPtr(pThis, sizeof(pThis)));
  ASSERT(pThis);

  WNDPROC tempSuperWindowProc = pThis->m_pfnSuperWindowProc;

  //Call the virtual function for this hook instance
	LRESULT lRes;
	BOOL bRet = pThis->ProcessWindowMessage(temphWnd, nMsg, wParam, lParam, lRes);

  //Handle the case where the first hook has changed in the previous call to ProcessWindowMessage
  if (pThis->m_hWnd == NULL)
    pThis = FirstHook(temphWnd);

  //hand of to the chain if we have a chain and it was not handled by this hook (and we still have a chain)
  CHookWnd* pTempHook = pThis;
  if (pTempHook)
  {
    while (!bRet && pTempHook)
    {
      bRet = pTempHook->ProcessWindowMessage(temphWnd, nMsg, wParam, lParam, lRes);
      pTempHook = pTempHook->m_pNextHook;
    }
  }

	//do the default processing if message was not handled
	if (!bRet)
	{
		if (nMsg != WM_NCDESTROY)
      lRes = CallWindowProc(tempSuperWindowProc, temphWnd, nMsg, wParam, lParam);
    else
		{
      //Call the super window proc
      lRes = CallWindowProc(tempSuperWindowProc, temphWnd, nMsg, wParam, lParam);

      //Remove all the hooks
      if (pThis)
        RemoveAll(pThis);
		}
	}
	return lRes;
}

void CHookWnd::OnSubclass(HWND /*hWnd*/)
{
  //Nothing to do, a derived class might have some use for it
}

void CHookWnd::OnUnsubclass(HWND /*hWnd*/)
{
  //Nothing to do, a derived class might have some use for it
}

LRESULT CHookWnd::DefWindowProc(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
  ASSERT(m_pfnSuperWindowProc);
	return CallWindowProc(m_pfnSuperWindowProc, m_hWnd, nMsg, wParam, lParam);
}
