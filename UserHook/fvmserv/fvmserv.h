// Feather-weight Virtual Machine Driver
#define	SYS_FILE	TEXT("hooksys.sys")
#define	SYS_NAME	TEXT("HOOKSYS")

// Device IO Control
#define     DEVICENAME				L"\\\\.\\HOOKSYS"
#define		FILE_DEVICE_HOOKSYS		0x00008300

#define		IO_REFERENCE_EVENT		(ULONG) CTL_CODE(FILE_DEVICE_HOOKSYS, 0x801, METHOD_NEITHER, FILE_ANY_ACCESS)
#define		IO_QUERY_CLIENTID		(ULONG) CTL_CODE(FILE_DEVICE_HOOKSYS, 0x806, METHOD_NEITHER, FILE_ANY_ACCESS)
#define		IO_RESUME_PROCESS		(ULONG) CTL_CODE(FILE_DEVICE_HOOKSYS, 0x807, METHOD_NEITHER, FILE_ANY_ACCESS)

// Service macro and function prototype

// Internal name of the service
#define	SZSERVICENAME			"FVMUserController"

// Displayed name of the service
#define	SZSERVICEDISPLAYNAME	"Feather-weight Virtual Machine(FVM)"

// Discription of the service
#define SZSERVICEDISCRIPTION	"FVM user-level virtualization controller"

// Thread creation information
typedef struct _CLIENT_ID { 
	DWORD UniqueProcess;
	DWORD UniqueThread;
} CLIENT_ID, *PCLIENT_ID; 

// Service functions
VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv);
VOID WINAPI ServiceCtrl(DWORD dwCtrlCode);

VOID CmdInstallService();
VOID CmdStartService(SC_HANDLE hservice);
VOID CmdRemoveService();

VOID ServiceStart(DWORD dwArgc, LPTSTR *lpszArgv);
VOID ServiceStop();
BOOL DriverStop( IN LPCTSTR DriverName );
VOID DriverRemove( IN SC_HANDLE schSCManager, IN LPCTSTR DriverName );

BOOL ReportStatusToSCMgr(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);

// Instdrv.cpp
BOOL LoadDeviceDriver( const TCHAR * Name, const TCHAR * Path, 
					  HANDLE * lphDevice, PDWORD Error );
BOOL UnloadDeviceDriver( const TCHAR * Name );

// FVM initialization
BOOL WINAPI StartFVMHook();
// FVM cleanup
void WINAPI StopFVMHook();
