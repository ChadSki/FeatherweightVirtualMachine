// fvmserv.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "fvmserv.h"
#include "Ldpreload.h"
#include "Psapi.h"

// Handle to device driver
//HANDLE	SysHandle = INVALID_HANDLE_VALUE;

typedef void (*HookProc)(void);

HINSTANCE hinstLib;
HookProc RemoveHook = NULL;

// hook server thread handle
HANDLE hookthread;

// two event objects: hevent[0] for process notification
// hevent[1] for thread termination
static HANDLE	hevent[2];

/*
// Start FVM driver
BOOL WINAPI StartFVMDriver()
{
	TCHAR	drvpath[MAX_PATH+1];
	TCHAR	*drvptr = NULL;
	HANDLE	findHandle;
	DWORD	error;
	WIN32_FIND_DATA findData;

	if (!GetModuleFileName(NULL, drvpath, MAX_PATH))
		return FALSE;

	drvptr = wcsrchr(drvpath, TEXT('\\'));
	if (!drvptr) return FALSE;

	wcscpy(drvptr+1, SYS_FILE);

	findHandle = FindFirstFile( drvpath, &findData );
	if( findHandle == INVALID_HANDLE_VALUE )
		return FALSE;

	FindClose( findHandle );

	if( !LoadDeviceDriver( SYS_NAME, drvpath, &SysHandle, &error ) ) {
		printf("Starting FVM driver failed.\n");
		return FALSE;
	}
	return TRUE;
}

// Stop FVM driver
void WINAPI StopFVMDriver()
{
	UnloadDeviceDriver( SYS_NAME );
	CloseHandle( SysHandle );
}
*/
BOOL DoInjectLoadInto(DWORD newpid, char *szLibFile, char *szExeFile)
{
	HANDLE		hProcess = NULL;
	BOOL		bResult	= FALSE; 
	DWORD		cbNeed = 0;
	char		exepath[MAX_PATH];
	DWORD		dwEntryAddr;
	LPVOID		lpLDPreloadInstrStorage;
	BYTE		rgbOrigEntryCode[NUM_ENTRY_CODE_BYTES];
	BYTE		rgbEntryCode[NUM_ENTRY_CODE_BYTES];
	BYTE		rgbLDPreloadCode[NUM_LDPRELOAD_CODE_BYTES];

	__try 
	{
		// Get a handle for the process we want to inject into
		hProcess = OpenProcess(
			PROCESS_ALL_ACCESS, // Specifies all possible access flags
			FALSE, 
			newpid
			);
		if (hProcess == NULL) 
			__leave;
		
		if (*(PWCHAR)szExeFile == 0)
			__leave;

		wsprintfA(exepath, "%S", (PWCHAR)szExeFile);

		/* Get entry point of process */
		GetEntryPointAddr(exepath, &dwEntryAddr);

		/* Allocate memory block */
		lpLDPreloadInstrStorage = VirtualAllocEx(hProcess, NULL, 500, MEM_COMMIT, 
			PAGE_EXECUTE_READWRITE);

		if (!lpLDPreloadInstrStorage)
			__leave;

		/* Copy original instructions from start addr to one memory block */
		ReadProcessMemory(hProcess, (LPVOID)dwEntryAddr, rgbOrigEntryCode, 
			NUM_ENTRY_CODE_BYTES, NULL);

		/* Initialize rgbEntryCode (simple push arguments, jmp to memory 
		block #2 code to entry point) */
		WriteEntryCode(rgbEntryCode, (DWORD)lpLDPreloadInstrStorage);

		/* Initialize rgbLDPreloadCode */
		WriteLDPreloadCode(rgbLDPreloadCode, (DWORD)lpLDPreloadInstrStorage, 
			rgbOrigEntryCode, dwEntryAddr, szLibFile);

		/* Write rgbEntryCode to program */
		WriteProcessMemory(hProcess, (LPVOID)dwEntryAddr, rgbEntryCode, 
			NUM_ENTRY_CODE_BYTES, NULL);

		/* Write rgbLDPreloadCode to program */
		WriteProcessMemory(hProcess, lpLDPreloadInstrStorage, rgbLDPreloadCode, 
			NUM_LDPRELOAD_CODE_BYTES, NULL);

		bResult = TRUE;
	}
	__finally 
	{ 
		if (hProcess != NULL) 
			CloseHandle(hProcess);
	}
	return bResult;
}

// Win32 API Hooking thread
DWORD WINAPI HookNewProcess(LPVOID lpParam)
{
	DWORD dwResult, junk;
	HANDLE hDevice;
	BOOL bResult = FALSE;
	char dllpath[MAX_PATH];
	char *nameptr;
	WCHAR imagebuf[MAX_PATH+8];
	PCLIENT_ID threadinfo;

	// Get the hook dll's file path
	if (GetModuleFileNameA(NULL, dllpath, MAX_PATH) == 0)
		ExitThread(0);

	nameptr = strrchr(dllpath, '\\');
	if (nameptr == NULL) ExitThread(0);
	else
		strcpy(nameptr, "\\fvmdll.dll");

	while(TRUE) {
		dwResult = WaitForMultipleObjects(2, hevent, FALSE, INFINITE);

		switch(dwResult) {

		case WAIT_OBJECT_0:
			// Get the new process ID
			hDevice = CreateFile(DEVICENAME, // driver to open
								0,		// don't need any access to the driver
								FILE_SHARE_READ | FILE_SHARE_WRITE,  // share mode
								NULL,    // default security attributes
								OPEN_EXISTING,  // disposition
								0,       // file attributes
								NULL);   // don't copy any file's attributes

			if (hDevice == INVALID_HANDLE_VALUE)
				break;

			bResult = DeviceIoControl(hDevice,  // device we are contacting
						IO_QUERY_CLIENTID,  // operation to perform
						NULL,
						0,
						imagebuf,
						sizeof(imagebuf),
						&junk, // discard count of bytes returned
						(LPOVERLAPPED) NULL);  // synchronous I/O

			if (!bResult) {
				CloseHandle(hDevice);
				break;
			}

			threadinfo = (PCLIENT_ID)imagebuf;
			// Inject the LoadLibrary code into the new created VM process
			DoInjectLoadInto(threadinfo->UniqueProcess, dllpath, 
				(PCHAR)imagebuf+sizeof(CLIENT_ID));

			// Open resume flag to kernel
			DeviceIoControl(hDevice,  // device we are contacting
				IO_RESUME_PROCESS,  // operation to perform
				threadinfo,
				sizeof(CLIENT_ID),
				NULL,
				0,
				&junk, // discard count of bytes returned
				(LPOVERLAPPED) NULL);  // synchronous I/O

			CloseHandle(hDevice);
/*			
			// Resume the main thread
			hthread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, 
				threadinfo->UniqueThread);

			if (hthread) {
				ResumeThread(hthread);
				CloseHandle(hthread);
			}*/
			break;

		case WAIT_OBJECT_0 + 1: // thread termination
			ExitThread(0);
			return 0;

		default:
			break;
		}
	}
	return 0;
}

//
// Attempts to enable SeDebugPrivilege. This is required by use of
// CreateRemoteThread() under NT/2K
//
BOOL EnableDebugPrivilege()
{
	HANDLE           hToken;
	LUID             sedebugnameValue;
	TOKEN_PRIVILEGES tp;

	if ( !OpenProcessToken( 
		GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES | // to adjust privileges
		TOKEN_QUERY,              // to get old privileges setting
		&hToken 
		) )
		//
		// OpenProcessToken() failed
		//
		return FALSE;
	//
	// Given a privilege's name SeDebugPrivilege, we should locate its local LUID mapping.
	//
	if ( !LookupPrivilegeValue( NULL, SE_DEBUG_NAME, &sedebugnameValue ) )
	{
		//
		// LookupPrivilegeValue() failed
		//
		CloseHandle( hToken );
		return FALSE;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = sedebugnameValue;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if ( !AdjustTokenPrivileges( hToken, FALSE, &tp, sizeof(tp), NULL, NULL ) )
	{
		//
		// AdjustTokenPrivileges() failed
		//
		CloseHandle( hToken );
		return FALSE;
	}

	CloseHandle( hToken );
	return TRUE;
}

BOOL WINAPI StartFVMHook()
{
//	char		dllpath[MAX_PATH];
	HookProc	InstallHook = NULL;
	char		*nameptr;
	HANDLE hDevice;
	BOOL bResult = FALSE;
	DWORD junk;

	// Set up window creation monitoring hook
	// Get the hook dll's file path
	/*
	if (GetModuleFileNameA(NULL, dllpath, MAX_PATH) == 0)
		return FALSE;

	
	nameptr = strrchr(dllpath, '\\');

	if (nameptr == NULL) return FALSE;
	else
		strcpy(nameptr, "\\fvmwin.dll");
 
	// Get a handle to the DLL module.
	hinstLib = LoadLibraryA(dllpath); 

	if (hinstLib == NULL) return FALSE;

    InstallHook = (HookProc)GetProcAddress(hinstLib, "InstallHook");
	RemoveHook = (HookProc)GetProcAddress(hinstLib, "RemoveHook"); 

	// Set up shell hook
	if (InstallHook != NULL)
		(InstallHook)();
	else {
		FreeLibrary(hinstLib);
		return FALSE;
	}
	*/

	// Process notification event
	if ((hevent[0] = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL)
		return FALSE;

	// Thread termination event
	if ((hevent[1] = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL)
		return FALSE; //Previous opened handle(s) are closed in StopFVMHook()

	// Pass the noticiation event to kernel
	hDevice = CreateFile(DEVICENAME, // driver to open
						0,		// don't need any access to the driver
						FILE_SHARE_READ | FILE_SHARE_WRITE,  // share mode
						NULL,    // default security attributes
						OPEN_EXISTING,  // disposition
						0,       // file attributes
						NULL);   // don't copy any file's attributes

	if (hDevice == INVALID_HANDLE_VALUE)
		return FALSE;

	bResult = DeviceIoControl(hDevice,  // device we are contacting
				IO_REFERENCE_EVENT,  // operation to perform
				&hevent[0],
				sizeof(HANDLE),
				NULL,
				0,
				&junk, // discard count of bytes returned
				(LPOVERLAPPED) NULL);  // synchronous I/O

	CloseHandle(hDevice);

	if (!bResult) return FALSE;

	// Enable DebugPrivilege in order to do DLL injection
	EnableDebugPrivilege();

	hookthread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)HookNewProcess,
		NULL, 0, NULL);
	if (hookthread == NULL) return FALSE;

	return TRUE;
}

void WINAPI StopFVMHook()
{

   if (RemoveHook != NULL)
		(RemoveHook)();

	FreeLibrary(hinstLib);

	SetEvent(hevent[1]);  // Terminate the hook server thread

	if (WaitForSingleObject(hookthread, 5000) != WAIT_OBJECT_0)
		TerminateThread(hookthread, 0);

	CloseHandle(hookthread);
	CloseHandle(hevent[0]);
	CloseHandle(hevent[1]);
}
