
#ifdef FVMWIN_EXPORTS
#define FVMWIN_API __declspec(dllexport)
#else
#define FVMWIN_API __declspec(dllimport)
#endif


FVMWIN_API void RemoveHook();
FVMWIN_API LRESULT CALLBACK ShellProc(int nCode, WPARAM wParam, LPARAM lParam);

#define     DEVICENAME				"\\\\.\\HOOKSYS"
#define		FILE_DEVICE_HOOKSYS		0x00008300

#define		IO_QUERY_VM_ID			(ULONG) CTL_CODE(FILE_DEVICE_HOOKSYS, 0x829, METHOD_NEITHER, FILE_ANY_ACCESS)
