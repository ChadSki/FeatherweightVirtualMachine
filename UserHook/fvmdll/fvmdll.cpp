/*
 * **********************************************************
 * Copyright 2007 Rether Networks, Inc.  All rights reserved.
 * **********************************************************
 * This file is part of FVM (Feather weight Virtual machine) project.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winioctl.h>
#include <Ddeml.h>
#include "detours.h"
#include <stdlib.h>         /* For _MAX_PATH definition */
#include <stdio.h>
#include <malloc.h>

#include "fvmdll.h"
#include "api.h"



extern void InstallHook();
extern void RemoveHook();
#ifdef	VMSERVICE
#undef	VMSERVICE
#endif




/*

		MyCreateServiceA				@126	NONAME
		MyCreateServiceW				@127	NONAME
		MyOpenServiceA					@128	NONAME
		MyOpenServiceW					@129	NONAME
		MyStartServiceCtrlDispatcherA	@130	NONAME
		MyStartServiceCtrlDispatcherW	@131	NONAME
		MyRegisterServiceCtrlHandlerExA @132	NONAME
		MyRegisterServiceCtrlHandlerExW @133	NONAME
		MyRegisterServiceCtrlHandlerA	@134	NONAME
		RegisterServiceCtrlHandlerW		@13	NONAME

*/

#define		MAX_SERVICE_NO			100
//#define		DBGFILE					"HOOKDBG.txt"

#ifdef VMSERVICE
#define		SERVICE_HOST			"\\svchost.exe"
#endif

#define INVALID_VMID 0xffffffff

HINSTANCE hDLL;

// Virtual Machine ID
ULONG fvmid = 0xffffffff;

// Service name list
WCHAR *ServName[MAX_SERVICE_NO] = 
{
	L"IExplore",
	L"Msaccess",
	L"Microsoft Access",
//	L"Acroview",
//	L"ACROWWW",
	NULL
};

#ifdef VMSERVICE
// Assume each process can call StartServiceCtrlDispatcher() at most once
// Used by the proxy function of ServiceMain()
SERVICE_TABLE_ENTRYA *gDispatchInfoA;
SERVICE_TABLE_ENTRYW *gDispatchInfoW;
#endif

// Function pointer types.
typedef HMODULE (WINAPI *LoadLibraryA_Type)(LPCSTR lpLibFileName); 
typedef HMODULE (WINAPI *LoadLibraryW_Type)(LPCWSTR lpLibFileName); 
typedef HMODULE (WINAPI *LoadLibraryExA_Type)(LPCSTR lpFileName, HANDLE hFile, DWORD dwFlags);
typedef HMODULE (WINAPI *LoadLibraryExW_Type)(LPCWSTR lpFileName, HANDLE hFile, DWORD dwFlags);

typedef FARPROC (WINAPI *GetProcAddress_Type)(HMODULE hModule, LPCSTR lpProcName);

typedef ATOM (WINAPI *GlobalAddAtomA_Type)(LPCSTR lpString);
typedef ATOM (WINAPI *GlobalAddAtomW_Type)(LPCWSTR lpString);

typedef HSZ (WINAPI *DdeCreateStringHandleA_Type)(DWORD idInst, LPCSTR psz, int iCodePage);
typedef HSZ (WINAPI *DdeCreateStringHandleW_Type)(DWORD idInst, LPCWSTR psz, int iCodePage);

// Function prototypes.

ATOM WINAPI MyGlobalAddAtomA(LPCSTR lpString);
ATOM WINAPI MyGlobalAddAtomW(LPCWSTR lpString);

HSZ WINAPI MyDdeCreateStringHandleA(DWORD idInst, LPCSTR psz, int iCodePage);
HSZ WINAPI MyDdeCreateStringHandleW(DWORD idInst, LPCWSTR psz, int iCodePage);

#ifdef VMSERVICE
// Service virtualization
SC_HANDLE WINAPI MyCreateServiceA(
    SC_HANDLE    hSCManager,
    LPCSTR     lpServiceName,
    LPCSTR     lpDisplayName,
    DWORD        dwDesiredAccess,
    DWORD        dwServiceType,
    DWORD        dwStartType,
    DWORD        dwErrorControl,
    LPCSTR     lpBinaryPathName,
    LPCSTR     lpLoadOrderGroup,
    LPDWORD      lpdwTagId,
    LPCSTR     lpDependencies,
    LPCSTR     lpServiceStartName,
    LPCSTR     lpPassword
);

SC_HANDLE WINAPI MyCreateServiceW(
    SC_HANDLE    hSCManager,
    LPCWSTR     lpServiceName,
    LPCWSTR     lpDisplayName,
    DWORD        dwDesiredAccess,
    DWORD        dwServiceType,
    DWORD        dwStartType,
    DWORD        dwErrorControl,
    LPCWSTR     lpBinaryPathName,
    LPCWSTR     lpLoadOrderGroup,
    LPDWORD      lpdwTagId,
    LPCWSTR     lpDependencies,
    LPCWSTR     lpServiceStartName,
    LPCWSTR     lpPassword
);

SC_HANDLE WINAPI MyOpenServiceA(
    SC_HANDLE   hSCManager,
    LPCSTR    lpServiceName,
    DWORD       dwDesiredAccess
);

SC_HANDLE WINAPI MyOpenServiceW(
    SC_HANDLE   hSCManager,
    LPCWSTR    lpServiceName,
    DWORD       dwDesiredAccess
);

BOOL WINAPI MyStartServiceCtrlDispatcherA(
    CONST SERVICE_TABLE_ENTRYA *lpServiceStartTable
);

BOOL WINAPI MyStartServiceCtrlDispatcherW(
    CONST SERVICE_TABLE_ENTRYW *lpServiceStartTable
);

SERVICE_STATUS_HANDLE WINAPI MyRegisterServiceCtrlHandlerA(
    LPCSTR             lpServiceName,
    LPHANDLER_FUNCTION   lpHandlerProc
);

SERVICE_STATUS_HANDLE WINAPI MyRegisterServiceCtrlHandlerW(
    LPCWSTR             lpServiceName,
    LPHANDLER_FUNCTION   lpHandlerProc
);

SERVICE_STATUS_HANDLE WINAPI MyRegisterServiceCtrlHandlerExA(
    LPCSTR                lpServiceName,
    LPHANDLER_FUNCTION_EX   lpHandlerProc,
    LPVOID                  lpContext
);

SERVICE_STATUS_HANDLE WINAPI MyRegisterServiceCtrlHandlerExW(
    LPCWSTR                lpServiceName,
    LPHANDLER_FUNCTION_EX   lpHandlerProc,
    LPVOID                  lpContext
);
#endif


#if 1
HMODULE (WINAPI *WinLoadLibraryA)
	(LPCSTR lpLibFileName) = LoadLibraryA; 

HMODULE (WINAPI *WinLoadLibraryW)
	(LPCWSTR lpLibFileName) = LoadLibraryW; 

HMODULE (WINAPI *WinLoadLibraryExA)
	(LPCSTR lpFileName, HANDLE hFile, DWORD dwFlags) = LoadLibraryExA;

HMODULE (WINAPI *WinLoadLibraryExW)
	(LPCWSTR lpFileName, HANDLE hFile, DWORD dwFlags) = LoadLibraryExW;

HANDLE (WINAPI *WinSetClipboardData)
	(UINT uFormat, HANDLE hMem) = SetClipboardData;
HANDLE (WINAPI *WinGetClipboardData)
	(UINT uFormat) = GetClipboardData;

FARPROC (WINAPI *WinGetProcAddress)
	(HMODULE hModule, LPCSTR lpProcName) = GetProcAddress;

ATOM (WINAPI *WinGlobalAddAtomA)
	(LPCSTR lpString) = GlobalAddAtomA;
ATOM (WINAPI *WinGlobalAddAtomW)
	(LPCWSTR lpString) = GlobalAddAtomW;

HSZ (WINAPI *WinDdeCreateStringHandleA)
	(DWORD idInst, LPCSTR psz, int iCodePage) = DdeCreateStringHandleA;
HSZ (WINAPI *WinDdeCreateStringHandleW)
	(DWORD idInst, LPCWSTR psz, int iCodePage) = DdeCreateStringHandleW;

#if 1
LRESULT (WINAPI *WinSendMessageA)
	(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) = SendMessageA;
LRESULT (WINAPI *WinSendMessageW)
	(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) = SendMessageW;
LRESULT (WINAPI *WinSendMessageTimeoutA)(
					HWND hWnd,
					UINT Msg,
					WPARAM wParam,
					LPARAM lParam,
					UINT fuFlags,
					UINT uTimeout,
					LPDWORD lpdwResult) = SendMessageTimeoutA;
LRESULT (WINAPI *WinSendMessageTimeoutW)(
					HWND hWnd,
					UINT Msg,
					WPARAM wParam,
					LPARAM lParam,
					UINT fuFlags,
					UINT uTimeout,
					LPDWORD lpdwResult) = SendMessageTimeoutW;

BOOL (WINAPI *WinSendNotifyMessageA)
	(HWND hWnd, UINT Msg, WPARAM wParam,
	LPARAM lParam) = SendNotifyMessageA;
BOOL (WINAPI *WinSendNotifyMessageW)
	(HWND hWnd, UINT Msg, WPARAM wParam,
	LPARAM lParam) = SendNotifyMessageW;
BOOL (WINAPI *WinSendMessageCallbackA)(
						HWND hWnd,
						UINT Msg,
						WPARAM wParam,
						LPARAM lParam,
						SENDASYNCPROC lpResultCallBack,
						DWORD dwData) = SendMessageCallbackA;
BOOL (WINAPI *WinSendMessageCallbackW)(
						HWND hWnd,
						UINT Msg,
						WPARAM wParam,
						LPARAM lParam,
						SENDASYNCPROC lpResultCallBack,
						DWORD dwData) = SendMessageCallbackW;

BOOL (WINAPI *WinPostMessageA)(HWND hWnd,
	UINT Msg, WPARAM wParam, LPARAM lParam) = PostMessageA;
BOOL (WINAPI *WinPostMessageW)(HWND hWnd,
	UINT Msg, WPARAM wParam, LPARAM lParam) = PostMessageW;

BOOL (WINAPI *WinPostThreadMessageA)
	(DWORD idThread, UINT Msg, WPARAM wParam,
	LPARAM lParam) = PostThreadMessageA;
BOOL (WINAPI *WinPostThreadMessageW)
	(DWORD idThread, UINT Msg, WPARAM wParam,
	LPARAM lParam) = PostThreadMessageW;

HWND (WINAPI *WinFindWindowA)(LPCSTR lpClassName,
	LPCSTR lpWindowName) = FindWindowA;
HWND (WINAPI *WinFindWindowW)
	(LPCWSTR lpClassName, LPCWSTR lpWindowName) = FindWindowW;

HWND (WINAPI *WinFindWindowExA)
	(HWND hwndParent, HWND hwndChildAfter,
	LPCSTR lpszClass, LPCSTR lpszWindow) = FindWindowExA;
HWND (WINAPI *WinFindWindowExW)(HWND hwndParent,
HWND hwndChildAfter, LPCWSTR lpszClass,
	LPCWSTR lpszWindow) = FindWindowExW;

BOOL (WINAPI *WinEnumWindows)
	(WNDENUMPROC lpEnumFunc, LPARAM lParam) = EnumWindows;

#endif
#ifdef VMSERVICE
// Service virtualization
static SC_HANDLE (WINAPI *WinCreateServiceA)(
    SC_HANDLE    hSCManager,
    LPCSTR     lpServiceName,
    LPCSTR     lpDisplayName,
    DWORD        dwDesiredAccess,
    DWORD        dwServiceType,
    DWORD        dwStartType,
    DWORD        dwErrorControl,
    LPCSTR     lpBinaryPathName,
    LPCSTR     lpLoadOrderGroup,
    LPDWORD      lpdwTagId,
    LPCSTR     lpDependencies,
    LPCSTR     lpServiceStartName,
    LPCSTR     lpPassword
) = CreateServiceA;

static SC_HANDLE (WINAPI *WinCreateServiceW)(
    SC_HANDLE    hSCManager,
    LPCWSTR     lpServiceName,
    LPCWSTR     lpDisplayName,
    DWORD        dwDesiredAccess,
    DWORD        dwServiceType,
    DWORD        dwStartType,
    DWORD        dwErrorControl,
    LPCWSTR     lpBinaryPathName,
    LPCWSTR     lpLoadOrderGroup,
    LPDWORD      lpdwTagId,
    LPCWSTR     lpDependencies,
    LPCWSTR     lpServiceStartName,
    LPCWSTR     lpPassword
) = CreateServiceW;

static SC_HANDLE (WINAPI *WinOpenServiceA)(
    SC_HANDLE   hSCManager,
    LPCSTR    lpServiceName,
    DWORD       dwDesiredAccess
) = OpenServiceA;

static SC_HANDLE (WINAPI *WinOpenServiceW)(
    SC_HANDLE   hSCManager,
    LPCWSTR    lpServiceName,
    DWORD       dwDesiredAccess
) = OpenServiceW;

static BOOL (WINAPI *WinStartServiceCtrlDispatcherA)(
    CONST SERVICE_TABLE_ENTRYA *lpServiceStartTable
) = StartServiceCtrlDispatcherA;

static BOOL (WINAPI *WinStartServiceCtrlDispatcherW)(
    CONST SERVICE_TABLE_ENTRYW *lpServiceStartTable
) = StartServiceCtrlDispatcherW;

static SERVICE_STATUS_HANDLE (WINAPI *WinRegisterServiceCtrlHandlerA)(
    LPCSTR             lpServiceName,
    LPHANDLER_FUNCTION   lpHandlerProc
) = RegisterServiceCtrlHandlerA;

static SERVICE_STATUS_HANDLE (WINAPI *WinRegisterServiceCtrlHandlerW)(
    LPCWSTR             lpServiceName,
    LPHANDLER_FUNCTION   lpHandlerProc
) = RegisterServiceCtrlHandlerW;

static SERVICE_STATUS_HANDLE (WINAPI *WinRegisterServiceCtrlHandlerExA)(
    LPCSTR                lpServiceName,
    LPHANDLER_FUNCTION_EX   lpHandlerProc,
    LPVOID                  lpContext
)= RegisterServiceCtrlHandlerExA;

static SERVICE_STATUS_HANDLE (WINAPI *WinRegisterServiceCtrlHandlerExW)(
    LPCWSTR                lpServiceName,
    LPHANDLER_FUNCTION_EX   lpHandlerProc,
    LPVOID                  lpContext
) = RegisterServiceCtrlHandlerExW;
#endif

#endif

//extern SDLLHook TextHookUser;
ULONG GetFvmId(VOID)
{
	HANDLE hDevice;
	DWORD vmid = 0,  pid, junk;
	BOOL bResult;

	pid = GetCurrentProcessId();

	hDevice = CreateFile(DEVICENAME, // drive to open
						0,		// don't need any access to the drive
						FILE_SHARE_READ | FILE_SHARE_WRITE,  // share mode
						NULL,    // default security attributes
						OPEN_EXISTING,  // disposition
						0,       // file attributes
						NULL);   // don't copy any file's attributes

	if (hDevice != INVALID_HANDLE_VALUE) {
		bResult = DeviceIoControl(hDevice,  // device we are querying
					IO_QUERY_VM_ID,  // operation to perform
					&pid,
					sizeof(DWORD),
					&vmid,
					sizeof(DWORD),
					&junk, // discard count of bytes returned
					(LPOVERLAPPED) NULL);  // synchronous I/O

		CloseHandle(hDevice);
	}
	return vmid;
}

// Hook function.
HSZ WINAPI MyDdeCreateStringHandleA(DWORD idInst, LPCSTR psz, int iCodePage)
{
	CHAR strbuf[256];
	CHAR namebuf[256];
	int	servid = 0;

	//OutputDebugStringA("DdeCreateStringHandleA\n");

	if (fvmid != INVALID_VMID)
	{
		while (ServName[servid]) {
			wsprintfA(namebuf, "%S", ServName[servid]);
			if (_stricmp(psz, namebuf) == 0) {
				wsprintfA(strbuf, "%s-FVM%u", psz, fvmid);
				return WinDdeCreateStringHandleA(idInst, strbuf, iCodePage);
			}
			servid++;
		}
	}

	return WinDdeCreateStringHandleA(idInst, psz, iCodePage);
}

HSZ WINAPI MyDdeCreateStringHandleW(DWORD idInst, LPCWSTR psz, int iCodePage)
{
	WCHAR strbuf[256];
	int	servid = 0;

	//OutputDebugStringW(L"DdeCreateStringHandleW\n");

	if (fvmid != INVALID_VMID)
	{
		while (ServName[servid]) {
			if (_wcsicmp(psz, ServName[servid]) == 0) {
				wsprintfW(strbuf, L"%s-FVM%u", psz, fvmid);
				return WinDdeCreateStringHandleW(idInst, strbuf, iCodePage);
			}
			servid++;
		}
	}

	return WinDdeCreateStringHandleW(idInst, psz, iCodePage);
}

ATOM WINAPI MyGlobalAddAtomA(LPCSTR lpString)
{
	CHAR strbuf[256];
	CHAR namebuf[256];
	int	servid = 0, namelen;

	//OutputDebugStringA("GlobalAddAtomA\n");

	if (fvmid != INVALID_VMID)
	{
		while (ServName[servid]) {
			wsprintfA(namebuf, "%S", ServName[servid]);
			namelen = strlen(namebuf);

			if (_strnicmp(lpString, namebuf, namelen) == 0) {
				wsprintfA(strbuf, "%s-FVM%u", namebuf, fvmid);
				return WinGlobalAddAtomA(strbuf);
			}
			servid++;
		}
	}

	return WinGlobalAddAtomA(lpString);
}

ATOM WINAPI MyGlobalAddAtomW(LPCWSTR lpString)
{
	WCHAR strbuf[256];
	int	servid = 0, namelen;

	//OutputDebugStringW(L"GlobalAddAtomW\n");

	if (fvmid != INVALID_VMID)
	{
		while (ServName[servid]) {
			namelen = wcslen(ServName[servid]);

			if (_wcsnicmp(lpString, ServName[servid], namelen) == 0) {
				wsprintfW(strbuf, L"%s-FVM%u", ServName[servid], fvmid);
				return WinGlobalAddAtomW(strbuf);
			}
			servid++;
		}
	}

	return WinGlobalAddAtomW(lpString);
}

//---------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
// Dll entry point
BOOL APIENTRY DllMain( HINSTANCE hModule, DWORD fdwReason, LPVOID lpReserved )
{
    if ( fdwReason == DLL_PROCESS_ATTACH )  // When initializing....
    {
		hDLL = hModule;

	//	OutputDebugString("detour attach....");
        // We don't need thread notifications for what we're doing.  Thus, get
        // rid of them, thereby eliminating some of the overhead of this DLL
		DisableThreadLibraryCalls( hModule );

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		fvmid = GetFvmId();
/*
		GetModuleFileName(NULL, exepath, MAX_PATH);
		_strlwr(exepath);
		
		FILE *fp;

		fp=fopen(DBGFILE, "a+");
		fprintf(fp, "\n%d Inject to process:", pid);
		fprintf(fp, exepath);
		fprintf(fp, "\n");
		fclose(fp);
*/
	

	DetourAttach(&(PVOID&)WinGlobalAddAtomA, MyGlobalAddAtomA);
	DetourAttach(&(PVOID&)WinGlobalAddAtomW, MyGlobalAddAtomW);


	DetourAttach(&(PVOID&)WinDdeCreateStringHandleA, MyDdeCreateStringHandleA);


	DetourAttach(&(PVOID&)WinDdeCreateStringHandleW, MyDdeCreateStringHandleW);


	DetourAttach(&(PVOID&)WinSendMessageA, MySendMessageA);


	DetourAttach(&(PVOID&)WinSendMessageW, MySendMessageW);
#if 1


	DetourAttach(&(PVOID&)WinSendMessageTimeoutA, MySendMessageTimeoutA);
	DetourAttach(&(PVOID&)WinSendMessageTimeoutW, MySendMessageTimeoutW);
	DetourAttach(&(PVOID&)WinSendNotifyMessageA, MySendNotifyMessageA);
	DetourAttach(&(PVOID&)WinSendNotifyMessageW, MySendNotifyMessageW);

	DetourAttach(&(PVOID&)WinSendMessageCallbackA, MySendMessageCallbackA);
	DetourAttach(&(PVOID&)WinSendMessageCallbackW, MySendMessageCallbackW);
	DetourAttach(&(PVOID&)WinPostMessageA, MyPostMessageA);
	DetourAttach(&(PVOID&)WinPostMessageW, MyPostMessageW);
	DetourAttach(&(PVOID&)WinPostThreadMessageA, MyPostThreadMessageA);
	DetourAttach(&(PVOID&)WinPostThreadMessageW, MyPostThreadMessageW);

	DetourAttach(&(PVOID&)WinFindWindowA, MyFindWindowA);
	DetourAttach(&(PVOID&)WinFindWindowW, MyFindWindowW);
	DetourAttach(&(PVOID&)WinFindWindowExA, MyFindWindowExA);
	DetourAttach(&(PVOID&)WinFindWindowExW, MyFindWindowExW);
	DetourAttach(&(PVOID&)WinEnumWindows, MyEnumWindows);

#ifdef VMSERVICE
	DetourAttach(&(PVOID&)WinCreateServiceA, MyCreateServiceA);

	DetourAttach(&(PVOID&)WinCreateServiceW, MyCreateServiceW);
	DetourAttach(&(PVOID&)WinOpenServiceA, MyOpenServiceA);
	DetourAttach(&(PVOID&)WinOpenServiceW, MyOpenServiceW);

	DetourAttach(&(PVOID&)WinStartServiceCtrlDispatcherA, MyStartServiceCtrlDispatcherA);	
	DetourAttach(&(PVOID&)WinStartServiceCtrlDispatcherW, MyStartServiceCtrlDispatcherW);


	DetourAttach(&(PVOID&)WinRegisterServiceCtrlHandlerExA, 
		MyRegisterServiceCtrlHandlerExA);
	DetourAttach(&(PVOID&)WinRegisterServiceCtrlHandlerExW, 
		MyRegisterServiceCtrlHandlerExW);
	
#endif
#endif


	DetourTransactionCommit();

	InstallHook();
    }
	else if ( fdwReason == DLL_PROCESS_DETACH ){
		RemoveHook();
	}

    return TRUE;
}

#ifdef VMSERVICE

//-------------------------------------------------------------------------------
#define IMAGE_SEP_SERVICE		L"___1234___"

#ifndef MAX_NUM_SERVICES
#define MAX_NUM_SERVICES 100
#endif
//-------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
typedef SC_HANDLE (WINAPI *CreateServiceA_Type)(
    SC_HANDLE    hSCManager,
    LPCSTR     lpServiceName,
    LPCSTR     lpDisplayName,
    DWORD        dwDesiredAccess,
    DWORD        dwServiceType,
    DWORD        dwStartType,
    DWORD        dwErrorControl,
    LPCSTR     lpBinaryPathName,
    LPCSTR     lpLoadOrderGroup,
    LPDWORD      lpdwTagId,
    LPCSTR     lpDependencies,
    LPCSTR     lpServiceStartName,
    LPCSTR     lpPassword
);

typedef SC_HANDLE (WINAPI *CreateServiceW_Type)(
    SC_HANDLE    hSCManager,
    LPCWSTR     lpServiceName,
    LPCWSTR     lpDisplayName,
    DWORD        dwDesiredAccess,
    DWORD        dwServiceType,
    DWORD        dwStartType,
    DWORD        dwErrorControl,
    LPCWSTR     lpBinaryPathName,
    LPCWSTR     lpLoadOrderGroup,
    LPDWORD      lpdwTagId,
    LPCWSTR     lpDependencies,
    LPCWSTR     lpServiceStartName,
    LPCWSTR     lpPassword
);

typedef SC_HANDLE (WINAPI *OpenServiceA_Type)(
    SC_HANDLE   hSCManager,
    LPCSTR    lpServiceName,
    DWORD       dwDesiredAccess
);

typedef SC_HANDLE (WINAPI *OpenServiceW_Type)(
    SC_HANDLE   hSCManager,
    LPCWSTR    lpServiceName,
    DWORD       dwDesiredAccess
);

typedef BOOL (WINAPI *StartServiceCtrlDispatcherA_Type)(
    CONST SERVICE_TABLE_ENTRYA *lpServiceStartTable
);

typedef BOOL (WINAPI *StartServiceCtrlDispatcherW_Type)(
    CONST SERVICE_TABLE_ENTRYW *lpServiceStartTable
);												 

typedef SERVICE_STATUS_HANDLE (WINAPI *MyRegisterServiceCtrlHandlerExA_Type)(
    LPCSTR                lpServiceName,
    LPHANDLER_FUNCTION_EX   lpHandlerProc,
    LPVOID                  lpContext
);

typedef SERVICE_STATUS_HANDLE (WINAPI *MyRegisterServiceCtrlHandlerExW_Type)(
    LPCWSTR                lpServiceName,
    LPHANDLER_FUNCTION_EX   lpHandlerProc,
    LPVOID                  lpContext
);

typedef SERVICE_STATUS_HANDLE (WINAPI *MyRegisterServiceCtrlHandlerA_Type)(
    LPCSTR             lpServiceName,
    LPHANDLER_FUNCTION   lpHandlerProc
);

typedef SERVICE_STATUS_HANDLE (WINAPI *MyRegisterServiceCtrlHandlerW_Type)(
    LPCWSTR             lpServiceName,
    LPHANDLER_FUNCTION   lpHandlerProc
);

//end Add
//------------------------------------------------------------------------------------

int addServiceName(PWCHAR name, PWCHAR modPath){

	HANDLE hDevice;               // handle to the drive to be examined 
	BOOL bResult = 0;                 // results flag
	DWORD junk;                   // discard results
	int rc = -1;
//	FILE *fp = NULL;
	CHAR bytes[512];

	hDevice = CreateFile(DEVICENAME, // drive to open
                       0,       // don't need any access to the drive
                       FILE_SHARE_READ | FILE_SHARE_WRITE,  // share mode
                       NULL,    // default security attributes
                       OPEN_EXISTING,  // disposition
                       0,       // file attributes
                       NULL);   // don't copy any file's attributes

	if (hDevice == INVALID_HANDLE_VALUE) // we can't open the drive
	{
/*		fp=fopen(DBGFILE, "a+");
		fprintf(fp, "unable to open DEVICE for ioctl <IO_SERVICE_VM>\n");
		fclose(fp);*/
		return -1;
	}

	bResult = DeviceIoControl(hDevice,  // device we are querying
          IO_SERVICE_VM,  // operation to perform
          name, wcslen(name)*2+2,
          bytes, sizeof(bytes),
          &junk, // discard count of bytes returned
          (LPOVERLAPPED) NULL);  // synchronous I/O

	memcpy(&rc, bytes, sizeof(DWORD));

	wcscpy(modPath, (PWCHAR)(bytes + sizeof(DWORD)));

	CloseHandle(hDevice);
	return rc;
}

VOID RestoreFullNameA(const CHAR *imagename, PCHAR servicebin)
{
	PCHAR nameptr, binptr;

	if ((nameptr = strrchr(imagename, '\\')) == NULL)
		return;

	if ((binptr = strrchr(servicebin, '\\')) == NULL)
		return;

	strcpy(binptr, nameptr);
}

VOID RestoreFullNameW(const WCHAR *imagename, PWCHAR servicebin)
{
	PWCHAR nameptr, binptr;

	if ((nameptr = wcsrchr(imagename, L'\\')) == NULL)
		return;

	if ((binptr = wcsrchr(servicebin, L'\\')) == NULL)
		return;

	wcscpy(binptr, nameptr);
}

SC_HANDLE WINAPI MyCreateServiceA(
    SC_HANDLE    hSCManager,
    LPCSTR     lpServiceName,
    LPCSTR     lpDisplayName,
    DWORD        dwDesiredAccess,
    DWORD        dwServiceType,
    DWORD        dwStartType,
    DWORD        dwErrorControl,
    LPCSTR     lpBinaryPathName,
    LPCSTR     lpLoadOrderGroup,
    LPDWORD      lpdwTagId,
    LPCSTR     lpDependencies,
    LPCSTR     lpServiceStartName,
    LPCSTR     lpPassword
)
{
	static int vmid = 0;
	DWORD starttype = dwStartType;
	FILE *fp = NULL;

	//printf("************** MyCreateServiceA() Hook **************\n");


	WCHAR fname[512];		//should contain lpBinaryPathName___1234___lpServiceName
	CHAR svcname[256], dispname[256];
	CHAR fname1[256], modBinPath[MAX_PATH];
	WCHAR modPath[MAX_PATH];
	int orig_vmid, idx = 0;
	PCHAR nameptr;

	if(dwStartType == SERVICE_AUTO_START)
		starttype = SERVICE_DEMAND_START;

	// Remove any arguments to the service binary
	// Only the binary name is added into the kernel list
	// Both file copying and name camparison in the kernel 
	// are against the binary file name
	while(isspace(lpBinaryPathName[idx])) idx++;
	if(lpBinaryPathName[idx] == '\"') {
		idx++;
		strcpy(fname1, &lpBinaryPathName[idx]);
		while(fname1[idx] != '\0' && fname1[idx] != '\"') idx++;
		if(fname1[idx] == '\"') fname1[idx] = 0;
	}
	else {
		strcpy(fname1, &lpBinaryPathName[idx]);
		while(fname1[idx] != '\0' && !isspace(fname1[idx])) idx++;
		if(isspace(fname1[idx]))  fname1[idx] = 0; 
	}

	nameptr = strrchr(fname1, '.');
	if ((nameptr == NULL)||(stricmp(nameptr, ".exe") != 0))
		strcat(fname1, ".exe");

	swprintf(fname, L"%S%s%S_%d", fname1, IMAGE_SEP_SERVICE, 
		lpServiceName, vmid);

	orig_vmid = addServiceName(fname, modPath);

	//convert modPath to char* from WCHAR *, and pass as the binaryPath
	sprintf(modBinPath, "%S", modPath);
	RestoreFullNameA(lpBinaryPathName, modBinPath);

	sprintf(svcname, "%s_%d", lpServiceName, orig_vmid);
	sprintf(dispname, "%s_%d", lpDisplayName, orig_vmid);

	vmid++;
	if(vmid == MAX_NUM_SERVICES)
		vmid = 0;

	//Now that the syscall monitor knows about the CreateService Call, invoke the origibal api
	return WinCreateServiceA(
		hSCManager,
		svcname,
		dispname,
		dwDesiredAccess,
		dwServiceType,
		starttype,
		dwErrorControl,
		modBinPath,
		//lpBinaryPathName,
		lpLoadOrderGroup,
		lpdwTagId,
		lpDependencies,
		lpServiceStartName,
		lpPassword
	);
}

SC_HANDLE WINAPI MyCreateServiceW(
    SC_HANDLE    hSCManager,
    LPCWSTR     lpServiceName,
    LPCWSTR     lpDisplayName,
    DWORD        dwDesiredAccess,
    DWORD        dwServiceType,
    DWORD        dwStartType,
    DWORD        dwErrorControl,
    LPCWSTR     lpBinaryPathName,
    LPCWSTR     lpLoadOrderGroup,
    LPDWORD      lpdwTagId,
    LPCWSTR     lpDependencies,
    LPCWSTR     lpServiceStartName,
    LPCWSTR     lpPassword
)
{
	static int vmid = 0;
	DWORD starttype = dwStartType;

	//printf("************** MyCreateServiceW() Hook **************\n");


	WCHAR fname[512];		//should contain lpBinaryPathName___1234___lpServiceName
	WCHAR svcname[256], dispname[256], fname1[256];
	int orig_vmid, idx = 0;
	WCHAR modPath[MAX_PATH];
	PWCHAR nameptr;

	if(dwStartType == SERVICE_AUTO_START)
		starttype = SERVICE_DEMAND_START;

	// Remove any arguments to the service binary
	// Only the binary name is added into the kernel list
	// Both file copying and name camparison in the kernel 
	// are against the binary file name
	while(iswspace(lpBinaryPathName[idx])) idx++;
	if(lpBinaryPathName[idx] == L'\"') {
		idx++;
		wcscpy(fname1, &lpBinaryPathName[idx]);
		while(fname1[idx] != L'\0' && fname1[idx] != L'\"') idx++;
		if(fname1[idx] == L'\"') fname1[idx] = 0;
	}
	else {
		wcscpy(fname1, &lpBinaryPathName[idx]);
		while(fname1[idx] != L'\0' && !iswspace(fname1[idx])) idx++;
			if(iswspace(fname1[idx]))  fname1[idx] = 0; 
	}

	nameptr = wcsrchr(fname1, L'.');
	if ((nameptr == NULL)||(wcsicmp(nameptr, L".exe") != 0))
		wcscat(fname1, L".exe");

	swprintf(fname, L"%s%s%s_%d", fname1, IMAGE_SEP_SERVICE, 
		lpServiceName, vmid);

	orig_vmid = addServiceName(fname, modPath);
	RestoreFullNameW(lpBinaryPathName, modPath);

	swprintf(svcname, L"%s_%d", lpServiceName, orig_vmid);
	swprintf(dispname, L"%s_%d", lpDisplayName, orig_vmid);

	vmid++; 
	if(vmid == MAX_NUM_SERVICES)
		vmid = 0;

	//Now that the syscall monitor knows about the CreateService Call, invoke the origibal api
	return WinCreateServiceW(
		hSCManager,
		svcname,
		dispname,
		dwDesiredAccess,
		dwServiceType,
		starttype,
		dwErrorControl,
		modPath,
		//lpBinaryPathName,
		lpLoadOrderGroup,
		lpdwTagId,
		lpDependencies,
		lpServiceStartName,
		lpPassword
	);
}


SC_HANDLE WINAPI MyOpenServiceA(
    SC_HANDLE   hSCManager,
    LPCSTR    lpServiceName,
    DWORD       dwDesiredAccess
)
{
	CHAR svcname[256];
	SC_HANDLE schandle;


	sprintf(svcname, "%s_%d", lpServiceName, fvmid);

/*	FILE *fp;
	char errbuf[256];

	fp = fopen(DBGFILE, "a+");
	sprintf(errbuf, "\nOpenSerivceA: %s\n", lpServiceName);
	fprintf(fp, errbuf);
	fclose(fp);
*/
	schandle = WinOpenServiceA(hSCManager, svcname, dwDesiredAccess);

	if (schandle == NULL)
		return WinOpenServiceA(hSCManager, lpServiceName, dwDesiredAccess);
	else
		return schandle;
}


SC_HANDLE WINAPI MyOpenServiceW(
    SC_HANDLE   hSCManager,
    LPCWSTR    lpServiceName,
    DWORD       dwDesiredAccess
)
{
	WCHAR svcname[256];
	SC_HANDLE schandle;


	swprintf(svcname, L"%s_%u", lpServiceName, fvmid);

/*	FILE *fp;
	char errbuf[256];

	fp = fopen(DBGFILE, "a+");
	sprintf(errbuf, "\nOpenSerivceW: %S\n", lpServiceName);
	fprintf(fp, errbuf);
	fclose(fp);
*/
	schandle = WinOpenServiceW(hSCManager, svcname, dwDesiredAccess);

	if (schandle == NULL)
		return WinOpenServiceW(hSCManager, lpServiceName, dwDesiredAccess);
	else
		return schandle;
}


VOID WINAPI ServiceMainProxyA(
    DWORD   dwNumServicesArgs,
    LPSTR   *lpServiceArgVectors
    )
{
	int i = 0;
	CHAR vmstr[10], *nameptr;

	if (dwNumServicesArgs < 1)
		return;

	/*
	FILE *fp;
	char errbuf[256];

	fp = fopen(DBGFILE, "a+");
	sprintf(errbuf, "\nServiceMainA: %u, %s\n", dwNumServicesArgs, lpServiceArgVectors[0]);
	fprintf(fp, errbuf);
	fclose(fp);
	*/

	sprintf(vmstr, "_%u", fvmid);

	while (gDispatchInfoA[i].lpServiceName) {
		if (stricmp(gDispatchInfoA[i].lpServiceName, lpServiceArgVectors[0]) == 0) {
//			break;

			nameptr = strstr(lpServiceArgVectors[0], vmstr);
			if (nameptr)
				*nameptr = 0;
			break;
		}
		i++;
	}

  if(gDispatchInfoA[i].lpServiceProc)
    gDispatchInfoA[i].lpServiceProc(dwNumServicesArgs, lpServiceArgVectors);
}


VOID WINAPI ServiceMainProxyW(
    DWORD   dwNumServicesArgs,
    LPWSTR  *lpServiceArgVectors
    )
{
	int i = 0;
	WCHAR vmstr[10], *nameptr;

	if (dwNumServicesArgs < 1)
		return;

	swprintf(vmstr, L"_%u", fvmid);

	while (gDispatchInfoW[i].lpServiceName) {
		if (wcsicmp(gDispatchInfoW[i].lpServiceName, lpServiceArgVectors[0]) == 0) {
			nameptr = wcsstr(lpServiceArgVectors[0], vmstr);
			if (nameptr)
				*nameptr = 0;
			break;
		}
		i++;
	}

	if(gDispatchInfoW[i].lpServiceProc)
	    gDispatchInfoW[i].lpServiceProc(dwNumServicesArgs, lpServiceArgVectors);
}


BOOL WINAPI MyStartServiceCtrlDispatcherA(
    CONST SERVICE_TABLE_ENTRYA *lpServiceStartTable
)
{
	SERVICE_TABLE_ENTRYA *pste;
	int i;
	int cnt;
	BOOL result;


	if (strstr(exepath, SERVICE_HOST))
		return WinStartServiceCtrlDispatcherA(lpServiceStartTable);

	for(cnt=0; lpServiceStartTable[cnt].lpServiceName != NULL; cnt++);

	/*
	FILE *fp;
	char errbuf[256];

	fp = fopen(DBGFILE, "a+");
	for(i=0; i<cnt; i++)
	{
		sprintf(errbuf, "\nStartServiceCtrlDispatcherA: %s\n", lpServiceStartTable[i].lpServiceName);
		fprintf(fp, errbuf);
	}
	fclose(fp);
	*/

	pste = (SERVICE_TABLE_ENTRYA *)calloc(cnt+1, sizeof(SERVICE_TABLE_ENTRYA));
	gDispatchInfoA = (SERVICE_TABLE_ENTRYA *)calloc(cnt+1, sizeof(SERVICE_TABLE_ENTRYA));

	for(i=0; i<cnt; i++)
	{
		pste[i].lpServiceName = (PCHAR)malloc(256);
		if (pste[i].lpServiceName)
			sprintf(pste[i].lpServiceName, "%s_%u", lpServiceStartTable[i].lpServiceName, fvmid);
			//strcpy(pste[i].lpServiceName, lpServiceStartTable[i].lpServiceName);

		gDispatchInfoA[i].lpServiceName = 
			(PCHAR)malloc(strlen(pste[i].lpServiceName) + 1);
		if (gDispatchInfoA[i].lpServiceName)
			strcpy(gDispatchInfoA[i].lpServiceName, pste[i].lpServiceName);

		pste[i].lpServiceProc = ServiceMainProxyA;
		gDispatchInfoA[i].lpServiceProc = lpServiceStartTable[i].lpServiceProc;
	}
	pste[i].lpServiceName = NULL;
	pste[i].lpServiceProc = NULL;

	gDispatchInfoA[i].lpServiceName = NULL;
	gDispatchInfoA[i].lpServiceProc = NULL;

	result = WinStartServiceCtrlDispatcherA(pste);

	for(i=0; i<cnt; i++) {
		free(pste[i].lpServiceName);
		free(gDispatchInfoA[i].lpServiceName);
	}
	free(pste);
	free(gDispatchInfoA);

	return result;
}


BOOL WINAPI MyStartServiceCtrlDispatcherW(
    CONST SERVICE_TABLE_ENTRYW *lpServiceStartTable
)
{
	SERVICE_TABLE_ENTRYW *pste;
	int i;
	int cnt;
	BOOL result;


	if (strstr(exepath, SERVICE_HOST))
		return WinStartServiceCtrlDispatcherW(lpServiceStartTable);

	for(cnt=0; lpServiceStartTable[cnt].lpServiceName != NULL; cnt++);

		/*
	WCHAR errbuf[256];
	FILE *fp;

	fp = fopen(DBGFILE, "a+");
	for(i=0; i<cnt; i++)
	{
		swprintf(errbuf, L"\nStartServiceCtrlDispatcherW: %s, %d\n", lpServiceStartTable[i].lpServiceName, GetLastError());
		fwprintf(fp, errbuf);
	}
	fclose(fp);
*/
	pste = (SERVICE_TABLE_ENTRYW *)calloc(cnt+1, sizeof(SERVICE_TABLE_ENTRYW));
	gDispatchInfoW = (SERVICE_TABLE_ENTRYW *)calloc(cnt+1, sizeof(SERVICE_TABLE_ENTRYW));

	for(i=0; i<cnt; i++)
	{
		pste[i].lpServiceName = (PWCHAR)malloc(512);
		if (pste[i].lpServiceName)
			swprintf(pste[i].lpServiceName, L"%s_%u", lpServiceStartTable[i].lpServiceName, fvmid);

		gDispatchInfoW[i].lpServiceName = 
			(PWCHAR)malloc(wcslen(pste[i].lpServiceName)*2 + 2);
		if (gDispatchInfoW[i].lpServiceName)
			wcscpy(gDispatchInfoW[i].lpServiceName, pste[i].lpServiceName);

		pste[i].lpServiceProc = ServiceMainProxyW;
		gDispatchInfoW[i].lpServiceProc = lpServiceStartTable[i].lpServiceProc;
	}
	pste[i].lpServiceName = NULL;
	pste[i].lpServiceProc = NULL;

	gDispatchInfoW[i].lpServiceName = NULL;
	gDispatchInfoW[i].lpServiceProc = NULL;
	
	result = WinStartServiceCtrlDispatcherW(pste);

	for(i=0; i<cnt; i++) {
		free(pste[i].lpServiceName);
		free(gDispatchInfoW[i].lpServiceName);
	}
	free(pste);
	free(gDispatchInfoW);

	return result;
}


SERVICE_STATUS_HANDLE WINAPI MyRegisterServiceCtrlHandlerExA(
    LPCSTR                lpServiceName,
    LPHANDLER_FUNCTION_EX   lpHandlerProc,
    LPVOID                  lpContext
)
{
	CHAR svcname[256];


	sprintf(svcname, "%s_%u", lpServiceName, fvmid);
	/*
	char errbuf[256];
	FILE *fp;

	fp = fopen(DBGFILE, "a+");
	sprintf(errbuf, "\nRegisterServiceCtrlHandlerExA: %s\n", lpServiceName);
	fprintf(fp, errbuf);
	fclose(fp);
	*/

	return WinRegisterServiceCtrlHandlerExA(svcname, lpHandlerProc, lpContext);
}


SERVICE_STATUS_HANDLE WINAPI MyRegisterServiceCtrlHandlerExW(
    LPCWSTR                lpServiceName,
    LPHANDLER_FUNCTION_EX   lpHandlerProc,
    LPVOID                  lpContext
)
{
	WCHAR svcname[256];


	swprintf(svcname, L"%s_%u", lpServiceName, fvmid);
	
	/*
	WCHAR errbuf[256];
	FILE *fp;

	fp = fopen(DBGFILE, "a+");
	swprintf(errbuf, L"\nRegisterServiceCtrlHandlerExW: %s\n", lpServiceName);
	fwprintf(fp, errbuf);
	fclose(fp);
	*/

	return WinRegisterServiceCtrlHandlerExW(svcname, lpHandlerProc, lpContext);
}


SERVICE_STATUS_HANDLE WINAPI MyRegisterServiceCtrlHandlerA(
    LPCSTR             lpServiceName,
    LPHANDLER_FUNCTION   lpHandlerProc
)
{
	CHAR svcname[256];


	sprintf(svcname, "%s_%u", lpServiceName, fvmid);
	//strcpy(svcname, lpServiceName);
	/*
	FILE *fp;
	char errbuf[256];

	fp = fopen(DBGFILE, "a+");
	sprintf(errbuf, "\nRegisterServiceCtrlHandlerA: %s\n", lpServiceName);
	fprintf(fp, errbuf);
	fclose(fp);
	*/

	return WinRegisterServiceCtrlHandlerA(svcname, lpHandlerProc);
}


SERVICE_STATUS_HANDLE WINAPI MyRegisterServiceCtrlHandlerW(
    LPCWSTR             lpServiceName,
    LPHANDLER_FUNCTION   lpHandlerProc
)
{
	WCHAR svcname[256];


	swprintf(svcname, L"%s_%u", lpServiceName, fvmid);
	
	/*
	WCHAR errbuf[256];
	FILE *fp;

	fp = fopen(DBGFILE, "a+");
	swprintf(errbuf, L"\nRegisterServiceCtrlHandlerW: %s\n", lpServiceName);
	fwprintf(fp, errbuf);
	fclose(fp);
	*/

	return WinRegisterServiceCtrlHandlerW(svcname, lpHandlerProc);
}

//End Add
//---------------------------------------------------------------------------------------------------
#endif
