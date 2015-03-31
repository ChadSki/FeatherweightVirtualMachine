// service.cpp : Defines the entry point for FVM as a service.
//

#include "stdafx.h"
#include "fvmserv.h"

SERVICE_STATUS          ssStatus;       // current status of the service
SERVICE_STATUS_HANDLE   sshStatusHandle;
DWORD                   dwErr = 0;
HANDLE					hServerStopEvent = NULL;

void main(int argc, char* argv[])
{
	SERVICE_TABLE_ENTRY dispatchTable[] =
	{
		{ TEXT(SZSERVICENAME), (LPSERVICE_MAIN_FUNCTION)ServiceMain },
		{ NULL, NULL }
	};

	if (argc == 2) {
		if (_stricmp(argv[1], "-i") == 0)
			CmdInstallService();
		else if (_stricmp(argv[1], "-r") == 0)
			CmdRemoveService();
		return;
	}
	StartServiceCtrlDispatcher(dispatchTable);
}


VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv)
{
    // register our service control handler:
	sshStatusHandle = RegisterServiceCtrlHandler(TEXT(SZSERVICENAME), ServiceCtrl);

    ssStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    ssStatus.dwServiceSpecificExitCode = 0;

    // report the status to the service control manager.
    if (!ReportStatusToSCMgr(SERVICE_START_PENDING,NO_ERROR,3000))
    {
		if (sshStatusHandle)
			(VOID)ReportStatusToSCMgr(SERVICE_STOPPED,dwErr,0);
	}

    ServiceStart( dwArgc, lpszArgv );

    // try to report the stopped status to the service control manager.
    if (sshStatusHandle)
        (VOID)ReportStatusToSCMgr(SERVICE_STOPPED,dwErr,0);

	DriverStop(SYS_NAME);
}


VOID WINAPI ServiceCtrl(DWORD dwCtrlCode)
{
    // Handle the requested control code.
    switch(dwCtrlCode)
    {
        // Stop the service.
        //
        // SERVICE_STOP_PENDING should be reported before
        // setting the Stop Event - hServerStopEvent - in
        // ServiceStop().  This avoids a race condition
        // which may result in a 1053 - The Service did not respond...
        // error.
        case SERVICE_CONTROL_STOP:
            ReportStatusToSCMgr(SERVICE_STOP_PENDING, NO_ERROR, 0);
            ServiceStop();
            return;

        // Update the service status.
        case SERVICE_CONTROL_INTERROGATE:
            break;

        // invalid control code
        //
        default:
            break;
    }
    ReportStatusToSCMgr(ssStatus.dwCurrentState, NO_ERROR, 0);
}


//
//  FUNCTION: ReportStatusToSCMgr()
//
//  PURPOSE: Sets the current status of the service and
//           reports it to the Service Control Manager
//
BOOL ReportStatusToSCMgr(DWORD dwCurrentState,
                         DWORD dwWin32ExitCode,
                         DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;
    BOOL fResult = TRUE;

    if (dwCurrentState == SERVICE_START_PENDING)
        ssStatus.dwControlsAccepted = 0;
    else
        ssStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    ssStatus.dwCurrentState = dwCurrentState;
    ssStatus.dwWin32ExitCode = dwWin32ExitCode;
    ssStatus.dwWaitHint = dwWaitHint;

    if ( ( dwCurrentState == SERVICE_RUNNING ) ||
         ( dwCurrentState == SERVICE_STOPPED ) )
        ssStatus.dwCheckPoint = 0;
    else
        ssStatus.dwCheckPoint = dwCheckPoint++;

    // Report the status of the service to the service control manager.
	fResult = SetServiceStatus( sshStatusHandle, &ssStatus);

	return fResult;
}


//
//  FUNCTION: CmdInstallService()
//
//  PURPOSE: Installs the service and starts it
void CmdInstallService()
{
   SC_HANDLE   schService;
   SC_HANDLE   schSCManager;
   TCHAR		szPath[MAX_PATH];
	TCHAR		szQuotePath[MAX_PATH+1];
	TCHAR		szDriverPath[MAX_PATH];
	TCHAR		szDepend[10];
   SERVICE_DESCRIPTION szDiscription;

	if ( GetModuleFileName( NULL, szPath, MAX_PATH ) == 0 )
    {
        printf("Unable to install FVM Controller Service.\n");
        return;
    }

	wsprintf(szQuotePath, TEXT("\"%s\""), szPath);

	wsprintf(szDriverPath, TEXT("system32\\drivers\\%s"), SYS_FILE);

	ZeroMemory(szDepend, sizeof(szDepend));
	wcscpy(szDepend, SYS_NAME);

    schSCManager = OpenSCManager(
                        NULL,                   // machine (NULL == local)
                        NULL,                   // database (NULL == default)
                        SC_MANAGER_ALL_ACCESS   // access required
                   );
    if ( schSCManager )
    {
		DriverRemove(schSCManager, SYS_NAME);

		schService = CreateService( 
			schSCManager,          // SCManager database
			SYS_NAME,			   // name of service
			NULL,				   // name to display
			SERVICE_ALL_ACCESS,    // desired access
			SERVICE_KERNEL_DRIVER, // service type
			//SERVICE_DEMAND_START,  // start type
			SERVICE_BOOT_START,
			SERVICE_ERROR_NORMAL,  // error control type
			szDriverPath,          // service's binary
			NULL,                  // no load ordering group
			NULL,                  // no tag identifier
			NULL,                  // no dependencies
			NULL,                  // LocalSystem account
			NULL);                 // no password
			
		if ( schService == NULL )
		{
			printf("Installing FVM Driver failed.\n");
			CloseServiceHandle(schSCManager);
			return;
		}
		else 
			printf("FVM Driver installed.\n");

		CloseServiceHandle( schService );

        schService = CreateService(
            schSCManager,               // SCManager database
            TEXT(SZSERVICENAME),        // name of service
            TEXT(SZSERVICEDISPLAYNAME), // name to display
            SERVICE_ALL_ACCESS,         // desired access
            SERVICE_WIN32_OWN_PROCESS|SERVICE_INTERACTIVE_PROCESS,
										          // service type
            //SERVICE_DEMAND_START,		 // start type
			SERVICE_AUTO_START,
            SERVICE_ERROR_NORMAL,       // error control type
            szQuotePath,                // service's binary
            NULL,                       // no load ordering group
            NULL,                       // no tag identifier
            szDepend,					    // dependencies
            NULL,                       // LocalSystem account
            NULL);                      // no password

        if ( schService )
        {
			szDiscription.lpDescription = TEXT(SZSERVICEDISCRIPTION);
			ChangeServiceConfig2(schService, SERVICE_CONFIG_DESCRIPTION, &szDiscription);

            printf("FVM Controller Service installed.\n");
			   CmdStartService(schService);
            CloseServiceHandle(schService);
		}
        else
        {
            printf("Installing FVM Controller Service failed.\n");
        }

        CloseServiceHandle(schSCManager);
    }
    else
        printf("Open SCManager failed.\n");
}


//
//  FUNCTION: CmdRemoveService()
//
//  PURPOSE: Stops and removes the service
//
void CmdRemoveService()
{
    SC_HANDLE   schService, schSCManager;

    schSCManager = OpenSCManager(
                        NULL,                   // machine (NULL == local)
                        NULL,                   // database (NULL == default)
                        SC_MANAGER_ALL_ACCESS   // access required
                   );
    if ( schSCManager )
    {
        schService = OpenService(schSCManager, TEXT(SZSERVICENAME), SERVICE_ALL_ACCESS);

        if (schService)
        {
            // try to stop the service
            if ( ControlService( schService, SERVICE_CONTROL_STOP, &ssStatus ) )
            {
                printf("\nStopping FVM Controller Service.");
                Sleep(1000);

                while( QueryServiceStatus( schService, &ssStatus ) )
                {
                    if ( ssStatus.dwCurrentState == SERVICE_STOP_PENDING )
                    {
                        printf(".");
                        Sleep(1000);
                    }
                    else
                        break;
                }

                if ( ssStatus.dwCurrentState == SERVICE_STOPPED )
                    printf("\nFVM Controller Service stopped.\n");
                else
                    printf("\nFailed to stop FVM Controller Service.\n");

            }

            // now remove the service
            if( DeleteService(schService) )
				printf("FVM Controller Service removed.\n");
			else
				printf("Delete FVM Controller Service failed.\n");
			
            CloseServiceHandle(schService);
        }
        else
            printf("Open FVM Controller Service failed.\n");

		// Remove the depending FVM driver
		DriverRemove(schSCManager, SYS_NAME);

        CloseServiceHandle(schSCManager);
    }
    else
        printf("Open SCManager failed.\n");
}


//
//  FUNCTION: CmdStartService()
//
//  PURPOSE: Starts the service
//
VOID CmdStartService(SC_HANDLE hservice)
{
	if (!StartService(hservice, 0, NULL)) {
        printf("\nStarting FVM Controller Service failed.\n");
		return;
    }

    printf("Starting FVM Controller Service.");
    Sleep(1000);
 
    while( QueryServiceStatus( hservice, &ssStatus ) )
    {
        if ( ssStatus.dwCurrentState == SERVICE_START_PENDING )
        {
            printf(".");
            Sleep(1000);
        }
        else
            break;
    }

    if ( ssStatus.dwCurrentState == SERVICE_RUNNING )
        printf("\nFVM Controller Service started.\n");
    else
        printf("\nFailed to start FVM Controller Service.\n");
}


//
//  FUNCTION: DriverRemove
//
VOID DriverRemove( IN SC_HANDLE schSCManager, IN LPCTSTR DriverName )
{
    SC_HANDLE   schService;
	SERVICE_STATUS serviceStatus;

	schService = OpenService( schSCManager, SYS_NAME, SERVICE_ALL_ACCESS );
	if ( schService )
	{
		ControlService( schService, SERVICE_CONTROL_STOP, &serviceStatus );
		DeleteService( schService );
		CloseServiceHandle( schService );
	}
}


//
//  FUNCTION: DriverStop
//
BOOL DriverStop( IN LPCTSTR DriverName )
{
    SC_HANDLE       schService;
    BOOL            ret;
    SERVICE_STATUS  serviceStatus;
    SC_HANDLE		schSCManager;

    schSCManager = OpenSCManager(
                        NULL,                   // machine (NULL == local)
                        NULL,                   // database (NULL == default)
                        SC_MANAGER_ALL_ACCESS   // access required
                   );
    if ( schSCManager )
    {
        schService = OpenService(schSCManager, DriverName, SERVICE_ALL_ACCESS);

        if (schService)
		{
			ret = ControlService( schService, SERVICE_CONTROL_STOP, &serviceStatus );
			
			if (ret == 0) 
				printf("Stopping FVM driver failed.\n");

			CloseServiceHandle( schService );
		}
		CloseServiceHandle( schSCManager );
	}
    return ret;
}


//
//  FUNCTION: ServiceStart
//
//  PURPOSE: Actual code of the service
//           that does the work.
//
VOID ServiceStart(DWORD dwArgc, LPTSTR *lpszArgv)
{	
	// create the event object. The control handler function signals
    // this event when it receives the "stop" control code.
    hServerStopEvent = CreateEvent(
        NULL,    // no security attributes
        TRUE,    // manual reset event
        FALSE,   // not-signalled
        NULL);   // no name

    // report the status to the service control manager.
    if (!ReportStatusToSCMgr(SERVICE_RUNNING,NO_ERROR,0))
    {
		if (hServerStopEvent)
			CloseHandle(hServerStopEvent);
		return;
	}

	// Start FVM user hook
	if (!StartFVMHook()) {
		if (sshStatusHandle)
			(VOID)ReportStatusToSCMgr(SERVICE_STOPPED,dwErr,0);
		return;
	}

    WaitForSingleObject( hServerStopEvent,INFINITE );

	//cleanup:
    if (hServerStopEvent)
        CloseHandle(hServerStopEvent);
}


//  FUNCTION: ServiceStop
//
//  PURPOSE: Stops the service
VOID ServiceStop()
{
	// Stop FVM user hook
	StopFVMHook();

    if (hServerStopEvent)
        SetEvent(hServerStopEvent);
}
