#ifndef fvmData_H
#define fvmData_H

typedef struct _windowlist {
	HWND hWnd;
	_windowlist *next;

} windowlist, *pwindowlist;

typedef struct _workingsetlist {
	DWORD pid;
	SIZE_T minsize;
	SIZE_T maxsize;
	_workingsetlist *next;
} workingsetlist, *pworkingsetlist;

class t_FvmData
{
public:
	CString fvmName;
	CString fvmID;
	CString fvmRoot;
	CString fvmIp;
	CString fvmIpMask;
	int		status;	// 0:stop; 1:start; 2:suspend
	pwindowlist	suspendlist;
	pworkingsetlist suspendworkingset;
	t_FvmData *next;

	CString schedPriority;
	CString maxProcesses;
	CString maxComMemory;
	CString maxWorkSet;

	HANDLE hObject;
};

#endif