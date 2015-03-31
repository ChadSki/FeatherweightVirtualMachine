// fvmwin.cpp : Defines the entry point for the DLL application.
//

#include <windows.h>

#include <winioctl.h>
#include "fvmwin.h"
#include <string.h>

HHOOK hHook = NULL;


#define		HOSTVMID	0xffffffff

// DLL Instance
extern HINSTANCE	hDLL;
// FVM Identifier
extern DWORD		fvmid;
CHAR	   fvmtag[20];

ULONG GetFVMId(VOID)
{
	HANDLE hDevice = INVALID_HANDLE_VALUE;
	DWORD vmid = HOSTVMID, pid, junk;
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

		if (!bResult)
			vmid = HOSTVMID;
	}

	return vmid;
}

// Shell procedure that monitors the window creation
LRESULT CALLBACK ShellProc(int nCode, WPARAM wParam, LPARAM lParam) 
{
	CHAR wintitle[MAX_PATH];

	if (nCode < 0)  // do not process message 
		return CallNextHookEx(hHook, nCode, wParam, lParam); 

	if ((fvmid != HOSTVMID)&&(nCode == HSHELL_REDRAW)) {
		if (GetWindowText((HWND)wParam, wintitle, MAX_PATH) > 0)
		{
			//OutputDebugString(wintitle);
			if (!strstr(wintitle, fvmtag)) {
				strcat(wintitle, fvmtag);
				SetWindowText((HWND)wParam, wintitle);
			}
		}
	}

	return CallNextHookEx(hHook, nCode, wParam, lParam);
}

// Install the hook procedure
void InstallHook()
{
	if (fvmid != HOSTVMID)
		wsprintf(fvmtag, " - FVM[%u]", fvmid);
	//OutputDebugString("Setting windows hook");
	hHook = SetWindowsHookEx(WH_SHELL, (HOOKPROC)ShellProc, hDLL, GetCurrentThreadId ());
	/*
	if (hHook == NULL){
		wsprintf(lap, "error: %d %d\n", GetLastError(),GetCurrentThreadId () );
		MessageBox(NULL, lap, "error 1", MB_OK);
	}
	wsprintf(lap, "Setting windows hook %x-- %d\n", hHook, fvmid);
	OutputDebugString(lap);
	MessageBox(NULL, lap, "error", MB_OK);
	*/
}

// Remove the hook procedure
void RemoveHook()
{
	UnhookWindowsHookEx(hHook);
}

/*
BOOL APIENTRY DllMain( HINSTANCE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			hDLL = hModule;

			fvmid = GetFVMId();

			if (fvmid != HOSTVMID)
				wsprintf(fvmtag, " - FVM[%u]", fvmid);

			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}
*/
