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
#include <stdio.h>
#include "api.h"
#include "fvmuser.h"

//#define		DBGFILE					"HOOKDBG.txt"

// data structure that is passed to kernel
// to decide whether the two parties are within the same virtual machine
// not only used for DDE client and servers
typedef struct _DDEProcessIDInfo {
   DWORD	client_pid;
   DWORD	server_pid;
} DDEProcessIDInfo, *PDDEProcessIDInfo;

//extern CHAR exepath[MAX_PATH];

// Function pointer that saves the original EnumWindows callback function
WNDENUMPROC EnumWinRealProc = NULL;

// Function pointer types.
typedef LRESULT (WINAPI *SendMessage_Type)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
typedef LRESULT (WINAPI *SendMessageTimeout_Type)(
					HWND hWnd,
					UINT Msg,
					WPARAM wParam,
					LPARAM lParam,
					UINT fuFlags,
					UINT uTimeout,
					LPDWORD lpdwResult);
typedef BOOL (WINAPI *SendNotifyMessage_Type)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
typedef BOOL (WINAPI *SendMessageCallback_Type)(
					HWND hWnd,
					UINT Msg,
					WPARAM wParam,
					LPARAM lParam,
					SENDASYNCPROC lpResultCallBack,
					DWORD dwData);
typedef BOOL (WINAPI *PostMessage_Type)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
typedef BOOL (WINAPI *PostThreadMessage_Type)(DWORD idThread, UINT Msg, WPARAM wParam, LPARAM lParam);

typedef HWND (WINAPI *FindWindowA_Type)(LPCSTR lpClassName, LPCSTR lpWindowName);
typedef HWND (WINAPI *FindWindowW_Type)(LPCWSTR lpClassName, LPCWSTR lpWindowName);

typedef HWND (WINAPI *FindWindowExA_Type)(HWND hwndParent, HWND hwndChildAfter, 
										  LPCSTR lpszClass, LPCSTR lpszWindow);
typedef HWND (WINAPI *FindWindowExW_Type)(HWND hwndParent, HWND hwndChildAfter, 
										  LPCWSTR lpszClass, LPCWSTR lpszWindow);

typedef BOOL (WINAPI *EnumWindows_Type)(WNDENUMPROC lpEnumFunc, LPARAM lParam);





BOOL IsWindowinSameVM(HWND hWnd)
{
	HANDLE hDevice;
	BOOL bResult = FALSE, IsSameVM = TRUE, ret = TRUE;
	DWORD tid, junk;
	DDEProcessIDInfo ddeprocinfo;

	if (hWnd != NULL) {

		tid = GetWindowThreadProcessId(hWnd, &ddeprocinfo.client_pid);
		ddeprocinfo.server_pid = GetCurrentProcessId();

		if ((tid != 0)&&(ddeprocinfo.client_pid != ddeprocinfo.server_pid)) {
			hDevice = CreateFile(DEVICENAME, // drive to open
								0,		// don't need any access to the drive
								FILE_SHARE_READ | FILE_SHARE_WRITE,  // share mode
								NULL,    // default security attributes
								OPEN_EXISTING,  // disposition
								0,       // file attributes
								NULL);   // don't copy any file's attributes

			if (hDevice != INVALID_HANDLE_VALUE) {
				bResult = DeviceIoControl(hDevice,  // device we are querying
							IO_QUERY_IS_SAME_VM,  // operation to perform
							&ddeprocinfo,
							sizeof(DDEProcessIDInfo),
							&IsSameVM,
							sizeof(BOOL),
							&junk, // discard count of bytes returned
							(LPOVERLAPPED) NULL);  // synchronous I/O

				CloseHandle(hDevice);

				if ((bResult)&&(!IsSameVM))
					ret = FALSE;
			}
		}
	}
	return ret;
}

BOOL IsMessagetoSameVM(HWND hWnd)
{

	if (hWnd == HWND_BROADCAST)
		return TRUE;
	else
		return IsWindowinSameVM(hWnd);
}

BOOL CALLBACK EnumThreadWndProc(HWND hwnd, LPARAM lParam)
{
	BOOL *svm = (BOOL *)lParam;

	*svm = IsWindowinSameVM(hwnd);

	return FALSE;
}

// Proxy function to the EnumWindows's callback function
BOOL CALLBACK EnumWinProxyProc(HWND hwnd, LPARAM lParam)
{

	if (IsWindowinSameVM(hwnd))
		return EnumWinRealProc(hwnd, lParam);
	else
		return TRUE;
}


// Hook function.
LRESULT WINAPI MySendMessageA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{

	if (IsMessagetoSameVM(hWnd)) {
		return WinSendMessageA(hWnd, Msg, wParam, lParam);
	}
	return (LRESULT)0;
}

LRESULT WINAPI MySendMessageW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{

	if (IsMessagetoSameVM(hWnd)) {
		return WinSendMessageW(hWnd, Msg, wParam, lParam);
	}
	return (LRESULT)0;
}


LRESULT WINAPI MySendMessageTimeoutA(
					HWND hWnd,
					UINT Msg,
					WPARAM wParam,
					LPARAM lParam,
					UINT fuFlags,
					UINT uTimeout,
					LPDWORD lpdwResult)
{

	if (IsMessagetoSameVM(hWnd)) {
		return WinSendMessageTimeoutA(hWnd, Msg, wParam, lParam, fuFlags, uTimeout, lpdwResult);
	}
	return (LRESULT)0;
}

LRESULT WINAPI MySendMessageTimeoutW(
					HWND hWnd,
					UINT Msg,
					WPARAM wParam,
					LPARAM lParam,
					UINT fuFlags,
					UINT uTimeout,
					LPDWORD lpdwResult)
{

	if (IsMessagetoSameVM(hWnd)) {
		return WinSendMessageTimeoutW(hWnd, Msg, wParam, lParam, fuFlags, uTimeout, lpdwResult);
	}
	return (LRESULT)0;
}

BOOL WINAPI MySendNotifyMessageA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{

	if (IsMessagetoSameVM(hWnd)) {
		return WinSendNotifyMessageA(hWnd, Msg, wParam, lParam);
	}
	return 0;
}

BOOL WINAPI MySendNotifyMessageW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{

	if (IsMessagetoSameVM(hWnd)) {
		return WinSendNotifyMessageW(hWnd, Msg, wParam, lParam);
	}
	return 0;
}

BOOL WINAPI MySendMessageCallbackA(
					HWND hWnd,
					UINT Msg,
					WPARAM wParam,
					LPARAM lParam,
					SENDASYNCPROC lpResultCallBack,
					DWORD dwData)
{

	if (IsMessagetoSameVM(hWnd)) {
		return WinSendMessageCallbackA(hWnd, Msg, wParam, lParam, lpResultCallBack, dwData);
	}
	return 0;
}

BOOL WINAPI MySendMessageCallbackW(
					HWND hWnd,
					UINT Msg,
					WPARAM wParam,
					LPARAM lParam,
					SENDASYNCPROC lpResultCallBack,
					DWORD dwData)
{

	if (IsMessagetoSameVM(hWnd)) {
		return WinSendMessageCallbackW(hWnd, Msg, wParam, lParam, lpResultCallBack, dwData);
	}
	return 0;
}

BOOL WINAPI MyPostMessageA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{

	if (IsMessagetoSameVM(hWnd)) {
		return WinPostMessageA(hWnd, Msg, wParam, lParam);
	}
	return 0;
}

BOOL WINAPI MyPostMessageW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{

	if (IsMessagetoSameVM(hWnd)) {
		return WinPostMessageW(hWnd, Msg, wParam, lParam);
	}
	return 0;
}

BOOL WINAPI MyPostThreadMessageA(DWORD idThread, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	BOOL samevm = TRUE;

	EnumThreadWindows(idThread, EnumThreadWndProc, (LPARAM)&samevm);

	if (!samevm)
		return 0;

	return WinPostThreadMessageA(idThread, Msg, wParam, lParam);
}

BOOL WINAPI MyPostThreadMessageW(DWORD idThread, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	BOOL samevm = TRUE;

	EnumThreadWindows(idThread, EnumThreadWndProc, (LPARAM)&samevm);

	if (!samevm)
		return 0;

	return WinPostThreadMessageW(idThread, Msg, wParam, lParam);
}

HWND WINAPI MyFindWindowA(LPCSTR lpClassName, LPCSTR lpWindowName)
{
	HWND hwnd;

/*
	FILE *fp;
	if (lpClassName) {
		fp=fopen(DBGFILE, "a+");
		fprintf(fp, "%s:FindWindowA:%s\n", exepath, lpClassName);
		fclose(fp);
	}
*/
	hwnd = WinFindWindowA(lpClassName, lpWindowName);

	if (IsWindowinSameVM(hwnd))
		return hwnd;
	else
		return NULL;
}

HWND WINAPI MyFindWindowW(LPCWSTR lpClassName, LPCWSTR lpWindowName)
{
	HWND hwnd;

/*
	FILE *fp;
	if (lpClassName) {
		fp=fopen(DBGFILE, "a+");
		fprintf(fp, "%s:FindWindowW:%S\n", exepath, lpClassName);
		fclose(fp);
	}
*/
	hwnd = WinFindWindowW(lpClassName, lpWindowName);

	if (IsWindowinSameVM(hwnd))
		return hwnd;
	else
		return NULL;
}


HWND WINAPI MyFindWindowExA(HWND hwndParent, HWND hwndChildAfter, 
							LPCSTR lpszClass, LPCSTR lpszWindow)
{
	HWND hwnd;


/*	FILE *fp;
	if (lpszClass) {
		fp=fopen(DBGFILE, "a+");
		fprintf(fp, "%s:FindWindowExA:%s\n", exepath, lpszClass);
		fclose(fp);
	}
*/

	hwnd = WinFindWindowExA(hwndParent, hwndChildAfter, lpszClass, lpszWindow);

	if (hwndParent||IsWindowinSameVM(hwnd))
		return hwnd;
	else
		return NULL;
}


HWND WINAPI MyFindWindowExW(HWND hwndParent, HWND hwndChildAfter, 
							LPCWSTR lpszClass, LPCWSTR lpszWindow)
{
	HWND hwnd;

/*	FILE *fp;
	if (lpszClass) {
		fp=fopen(DBGFILE, "a+");
		fprintf(fp, "%s:FindWindowExW:%S\n", exepath, lpszClass);
		fclose(fp);
	}
*/
	hwnd = WinFindWindowExW(hwndParent, hwndChildAfter, lpszClass, lpszWindow);

	if (hwndParent||IsWindowinSameVM(hwnd))
		return hwnd;
	else
		return NULL;
}


#ifdef USER32HOOK
BOOL WINAPI MyEnumWindows(WNDENUMPROC lpEnumFunc, LPARAM lParam)
{

	EnumWinRealProc = lpEnumFunc;

	return WinEnumWindows(EnumWinProxyProc, lParam);
}
#endif
