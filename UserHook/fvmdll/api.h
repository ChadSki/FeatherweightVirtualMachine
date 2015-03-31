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

#ifndef API_H
#define API_H
#include <Ddeml.h>
//#define VMSERVICE
#define USER32HOOK

// Function prototypes.
LRESULT WINAPI MySendMessageA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI MySendMessageW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI MySendMessageTimeoutA(
					HWND hWnd,
					UINT Msg,
					WPARAM wParam,
					LPARAM lParam,
					UINT fuFlags,
					UINT uTimeout,
					LPDWORD lpdwResult);
LRESULT WINAPI MySendMessageTimeoutW(
					HWND hWnd,
					UINT Msg,
					WPARAM wParam,
					LPARAM lParam,
					UINT fuFlags,
					UINT uTimeout,
					LPDWORD lpdwResult);

BOOL WINAPI MySendNotifyMessageA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
BOOL WINAPI MySendNotifyMessageW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

BOOL WINAPI MySendMessageCallbackA(
					HWND hWnd,
					UINT Msg,
					WPARAM wParam,
					LPARAM lParam,
					SENDASYNCPROC lpResultCallBack,
					DWORD dwData);
BOOL WINAPI MySendMessageCallbackW(
					HWND hWnd,
					UINT Msg,
					WPARAM wParam,
					LPARAM lParam,
					SENDASYNCPROC lpResultCallBack,
					DWORD dwData);

BOOL WINAPI MyPostMessageA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
BOOL WINAPI MyPostMessageW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

BOOL WINAPI MyPostThreadMessageA(DWORD idThread, UINT Msg, WPARAM wParam, LPARAM lParam);
BOOL WINAPI MyPostThreadMessageW(DWORD idThread, UINT Msg, WPARAM wParam, LPARAM lParam);

HWND WINAPI MyFindWindowA(LPCSTR lpClassName, LPCSTR lpWindowName);
HWND WINAPI MyFindWindowW(LPCWSTR lpClassName, LPCWSTR lpWindowName);

HWND WINAPI MyFindWindowExA(HWND hwndParent, HWND hwndChildAfter, 
							LPCSTR lpszClass, LPCSTR lpszWindow);
HWND WINAPI MyFindWindowExW(HWND hwndParent, HWND hwndChildAfter, 
							LPCWSTR lpszClass, LPCWSTR lpszWindow);

BOOL WINAPI MyEnumWindows(WNDENUMPROC lpEnumFunc, LPARAM lParam);

#if 1
extern LRESULT (WINAPI *WinSendMessageA)
   (HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
extern LRESULT (WINAPI *WinSendMessageW)
   (HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
extern LRESULT (WINAPI *WinSendMessageTimeoutA)(
				   HWND hWnd,
				   UINT Msg,
				   WPARAM wParam,
				   LPARAM lParam,
				   UINT fuFlags,
				   UINT uTimeout,
				   LPDWORD lpdwResult);
extern LRESULT (WINAPI *WinSendMessageTimeoutW)(
				   HWND hWnd,
				   UINT Msg,
				   WPARAM wParam,
				   LPARAM lParam,
				   UINT fuFlags,
				   UINT uTimeout,
				   LPDWORD lpdwResult);

extern BOOL (WINAPI *WinSendNotifyMessageA)
   (HWND hWnd, UINT Msg, WPARAM wParam,
   LPARAM lParam);
extern BOOL (WINAPI *WinSendNotifyMessageW)
   (HWND hWnd, UINT Msg, WPARAM wParam,
   LPARAM lParam);
extern BOOL (WINAPI *WinSendMessageCallbackA)(
					   HWND hWnd,
					   UINT Msg,
					   WPARAM wParam,
					   LPARAM lParam,
					   SENDASYNCPROC lpResultCallBack,
					   DWORD dwData);
extern BOOL (WINAPI *WinSendMessageCallbackW)(
	
					   HWND hWnd,
					   UINT Msg,
					   WPARAM wParam,
					   LPARAM lParam,
					   SENDASYNCPROC lpResultCallBack,
					   DWORD dwData);

extern BOOL (WINAPI *WinPostMessageA)(HWND hWnd,
   UINT Msg, WPARAM wParam, LPARAM lParam);
extern BOOL (WINAPI *WinPostMessageW)(HWND hWnd,
   UINT Msg, WPARAM wParam, LPARAM lParam);

extern BOOL (WINAPI *WinPostThreadMessageA)
   (DWORD idThread, UINT Msg, WPARAM wParam,
   LPARAM lParam);
extern BOOL (WINAPI *WinPostThreadMessageW)
   (DWORD idThread, UINT Msg, WPARAM wParam,
   LPARAM lParam);

extern HWND (WINAPI *WinFindWindowA)(LPCSTR lpClassName,
   LPCSTR lpWindowName);
extern HWND (WINAPI *WinFindWindowW)
   (LPCWSTR lpClassName, LPCWSTR lpWindowName);

extern HWND (WINAPI *WinFindWindowExA)
   (HWND hwndParent, HWND hwndChildAfter,
   LPCSTR lpszClass, LPCSTR lpszWindow);
extern HWND (WINAPI *WinFindWindowExW)(HWND hwndParent,
   HWND hwndChildAfter, LPCWSTR lpszClass,
   LPCWSTR lpszWindow);

extern BOOL (WINAPI *WinEnumWindows)
   (WNDENUMPROC lpEnumFunc, LPARAM lParam);

#endif

// Hook structure.
enum
{
	KERNEL32_LoadLibraryA,
	KERNEL32_LoadLibraryW,
	KERNEL32_LoadLibraryExA,
	KERNEL32_LoadLibraryExW,
	KERNEL32_GetProcAddress,
	KERNEL32_GlobalAddAtomA,
	KERNEL32_GlobalAddAtomW,

	USER32_DdeCreateStringHandleA,
	USER32_DdeCreateStringHandleW,
	USER32_SendMessageA,
	USER32_SendMessageW,
	USER32_SendMessageTimeoutA,
	USER32_SendMessageTimeoutW,
	USER32_SendNotifyMessageA,
	USER32_SendNotifyMessageW,
	USER32_SendMessageCallbackA,
	USER32_SendMessageCallbackW,
	USER32_PostMessageA,
	USER32_PostMessageW,
	USER32_PostThreadMessageA,
	USER32_PostThreadMessageW,
	USER32_FindWindowA,
	USER32_FindWindowW,
	USER32_FindWindowExA,
	USER32_FindWindowExW,
	USER32_EnumWindows,

#ifdef VMSERVICE
	ADVAPI32_CreateServiceA,
	ADVAPI32_CreateServiceW,
	ADVAPI32_OpenServiceA,
	ADVAPI32_OpenServiceW,
	ADVAPI32_StartServiceCtrlDispatcherA,
	ADVAPI32_StartServiceCtrlDispatcherW,
	ADVAPI32_RegisterServiceCtrlHandlerExA,
	ADVAPI32_RegisterServiceCtrlHandlerExW,
	ADVAPI32_RegisterServiceCtrlHandlerA,
	ADVAPI32_RegisterServiceCtrlHandlerW,
#endif

	NOFUN
};

#endif
