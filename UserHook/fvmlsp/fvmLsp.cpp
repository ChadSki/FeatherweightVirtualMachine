#define  UNICODE
#define  _UNICODE

// The following macro is to take care of the way headers
// are included differently between Windows 2K and XP.
#ifdef WIN_XP
#include <windows.h>
#include <ws2spi.h>
#else
#include <ws2spi.h>
#include <windows.h>
#endif

#include <tchar.h>
#include <winioctl.h>
#include <stdio.h>

#define     DEVICENAME				L"\\\\.\\HOOKSYS"
#define		FILE_DEVICE_HOOKSYS		0x00008300

#define		IO_QUERY_VM_IP			(ULONG) CTL_CODE(FILE_DEVICE_HOOKSYS, 0x841, METHOD_NEITHER, FILE_ANY_ACCESS)

GUID  filterguid={0x4d1e91fd,0x116a,0x44aa,{0x8f,0xd4,0x1d,0x2c,0xf2,0x7b,0xd9,0xa9}};

LPWSAPROTOCOL_INFOW  protoinfo=NULL;
WSPPROC_TABLE        nextproctable;
DWORD                protoinfosize=0;
int                  totalprotos=0;

// The WinSock2 UpcallTable.
WSPUPCALLTABLE       gUpCallTable;

BOOL getfilter()
{
    int    errorcode;

    protoinfo=NULL;
    protoinfosize=0;
    totalprotos=0;

    if(WSCEnumProtocols(NULL,protoinfo,&protoinfosize,&errorcode)==SOCKET_ERROR)
    {
        if(errorcode!=WSAENOBUFS)
        {
            OutputDebugString(_T("First WSCEnumProtocols Error!"));
            return FALSE;
        }
    }

    if((protoinfo=(LPWSAPROTOCOL_INFOW)GlobalAlloc(GPTR,protoinfosize))==NULL)
    {
        OutputDebugString(_T("GlobalAlloc Error!"));                
        return FALSE;
    }

    if((totalprotos=WSCEnumProtocols(NULL,protoinfo,&protoinfosize,&errorcode))==SOCKET_ERROR)
    {
        OutputDebugString(_T("Second WSCEnumProtocols Error!"));  
        return FALSE;
    }

    return TRUE;
}


void freefilter()
{
    GlobalFree(protoinfo);
}

ULONG GetFvmIp(VOID)
{
	HANDLE hDevice;
	DWORD pid, junk;
	BOOL bResult;
	ULONG vmip = 0xffffffff;

	hDevice = CreateFile(DEVICENAME, // drive to open
						0,		// don't need any access to the drive
						FILE_SHARE_READ | FILE_SHARE_WRITE,  // share mode
						NULL,    // default security attributes
						OPEN_EXISTING,  // disposition
						0,       // file attributes
						NULL);   // don't copy any file's attributes

	if (hDevice != INVALID_HANDLE_VALUE) {
		bResult = DeviceIoControl(hDevice,  // device we are querying
					IO_QUERY_VM_IP,  // operation to perform
					&pid,
					sizeof(DWORD),
					&vmip,
					sizeof(DWORD),
					&junk, // discard count of bytes returned
					(LPOVERLAPPED) NULL);  // synchronous I/O

		CloseHandle(hDevice);
	}

	return vmip;
}


BOOL WINAPI DllMain(HINSTANCE hmodule,
         DWORD     reason,
         LPVOID    lpreserved)
{
    TCHAR   processname[MAX_PATH];
    TCHAR   showmessage[MAX_PATH+25];

    if(reason==DLL_PROCESS_ATTACH)
    {
        GetModuleFileName(NULL,processname,MAX_PATH);
        _tcscpy(showmessage,processname);
        _tcscat(showmessage,_T(" Loading FVM LSP..."));
        OutputDebugString(showmessage);  
    }
    return TRUE;
}


INT WSPAPI WSPBind(
    IN SOCKET s,
    IN const struct sockaddr FAR *name,
    IN INT namelen,
    OUT INT FAR *lpErrno
    )
{
	BOOL bOptVal = TRUE;
	int bOptLen = sizeof(BOOL);
	int Errno;
	ULONG vmIp = 0xffffffff;
	//WCHAR str[300];
	
	//OutputDebugString(_T("-----WSPBind Filtered"));

	vmIp = GetFvmIp();

	//wsprintf(str, L"s_addr : %S\n", inet_ntoa(((struct sockaddr_in *)name)->sin_addr));
	//MessageBox(NULL, str, L"Inside bind", MB_OK);
	//wsprintf(str, L"vmip inside bind %lu\n", vmIp);
	//MessageBox(NULL, str, L"Inside bind", MB_OK);

	if((vmIp != 0xffffffff) && (vmIp != 0))
	{
		nextproctable.lpWSPSetSockOpt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&bOptVal,
		    bOptLen, & Errno);

		((struct sockaddr_in *)name)->sin_addr.s_addr = vmIp;
		//MessageBox(NULL, L"Changed the address in bind", L"inside bind", MB_OK);
		//OutputDebugString(_T("Changed the address in bind"));
	}

	Errno = nextproctable.lpWSPBind(s, name, namelen, lpErrno);

	return Errno;
}


INT WSPAPI WSPSendTo(SOCKET           s,
          LPWSABUF         lpbuffer,
          DWORD            dwbuffercount,
          LPDWORD          lpnumberofbytessent,
          DWORD            dwflags,
          const struct     sockaddr FAR *lpto,
          int              itolen,
          LPWSAOVERLAPPED  lpoverlapped,
          LPWSAOVERLAPPED_COMPLETION_ROUTINE  lpcompletionroutine,
          LPWSATHREADID    lpthreadid,
          LPINT            lperrno)
{
	ULONG vmIp = 0xffffffff;
	//WCHAR str[400];
	int Errno, optVal, optLen, sockErr;

	//OutputDebugString(_T("-----WSPSendTo Filtered"));

	vmIp = GetFvmIp();

	//wsprintf(str, L"vmip inside sendto %lu\n", vmIp);
	//MessageBox(NULL, str, L"Inside sendto", MB_OK);

	optLen = sizeof(optVal);
	
	// Check if we are using a connectionless socket
	// Since lpto and itolen are ignored in case of connection-oriented
	// sockets, we only are bothered when using datagrams.
	Errno = nextproctable.lpWSPGetSockOpt(s, SOL_SOCKET, SO_TYPE, (char *)&optVal, &optLen, &sockErr);
	if (Errno == SOCKET_ERROR)
	{
		OutputDebugString(_T("GetSockOpt() failed"));
		return Errno;
	}

	if (optVal == SOCK_DGRAM)
	{
		//wsprintf(str, L"Connection less protocol inside sendto..\n");
		//MessageBox(NULL, str, L"UDP Inside sendto", MB_OK);
		
		//wsprintf(str, L"s_addr : %S\n", inet_ntoa(((struct sockaddr_in *)lpto)->sin_addr));
		//MessageBox(NULL, str, L"Inside sendto", MB_OK);

		if((vmIp != 0xffffffff) && (vmIp != 0))
		{
			if (((struct sockaddr_in *)lpto)->sin_addr.s_addr == inet_addr("127.0.0.1")) {
				((struct sockaddr_in *)lpto)->sin_addr.s_addr = vmIp;
			
				//OutputDebugString(_T("Changed the address in sendto"));
				//MessageBox(NULL, L"Changed the address in sendto", L"inside sendto", MB_OK);
			}
		}
	}
	
	Errno = nextproctable.lpWSPSendTo(s, lpbuffer, dwbuffercount, lpnumberofbytessent,
	    dwflags, lpto, itolen, lpoverlapped, lpcompletionroutine, lpthreadid, lperrno);

	return Errno;
}


INT WSPAPI WSPConnect(
  SOCKET s,
  const struct sockaddr* name,
  int namelen,
  LPWSABUF lpCallerData,
  LPWSABUF lpCalleeData,
  LPQOS lpSQOS,
  LPQOS lpGQOS,
  LPINT lpErrno
)
{
	ULONG vmIp = 0xffffffff;
	//WCHAR str[300];
	int Errno = 0;
	
	//OutputDebugString(_T("-----WSPConnect Filtered"));

	vmIp = GetFvmIp();

	//wsprintf(str, L"s_addr : %S\n", inet_ntoa(((struct sockaddr_in *)name)->sin_addr), inet_addr("127.0.0.1"));
	//MessageBox(NULL, str, L"Inside connect", MB_OK);
	//wsprintf(str, L"vmip inside connect %lu\n", vmIp);
	//MessageBox(NULL, str, L"Inside connect", MB_OK);

	if((vmIp != 0xffffffff) && (vmIp != 0))
	{
		if (((struct sockaddr_in *)name)->sin_addr.s_addr == inet_addr("127.0.0.1")) {
			((struct sockaddr_in *)name)->sin_addr.s_addr = vmIp;

			//OutputDebugString(_T("Changed the address in connect"));
			//MessageBox(NULL, L"Changed the address in connect", L"inside connect", MB_OK);
		}
	}

	Errno = nextproctable.lpWSPConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS, lpErrno);

	return Errno;
}


INT WSPAPI WSPStartup(
    WORD                   wversionrequested,
    LPWSPDATA              lpwspdata,
    LPWSAPROTOCOL_INFOW    lpprotoinfo,
    WSPUPCALLTABLE         upcalltable,
    LPWSPPROC_TABLE        lpproctable
)
{
    //OutputDebugString(_T("In WSPStartup ..."));

	ULONG vmIp = GetFvmIp();

    int           i;
    int           errorcode;
    int           filterpathlen;
    DWORD         layerid=0;
    DWORD         nextlayerid=0;
    TCHAR         *filterpath;
    HINSTANCE     hfilter;
    LPWSPSTARTUP  wspstartupfunc=NULL;


    if(lpprotoinfo->ProtocolChain.ChainLen<=1)
    {
              OutputDebugString(_T("ChainLen<=1"));  
        return FALSE;
    }
    
    getfilter();

    for(i=0;i<totalprotos;i++)
    {
        if(memcmp(&protoinfo[i].ProviderId,&filterguid,sizeof(GUID))==0)
        {
            layerid=protoinfo[i].dwCatalogEntryId;
            break;
        }
    }

    for(i=0;i<lpprotoinfo->ProtocolChain.ChainLen;i++)
    {
        if(lpprotoinfo->ProtocolChain.ChainEntries[i]==layerid)
        {
            nextlayerid=lpprotoinfo->ProtocolChain.ChainEntries[i+1];
            break;
        }
    }

    filterpathlen=MAX_PATH;
    filterpath=(TCHAR*)GlobalAlloc(GPTR,filterpathlen);  
    for(i=0;i<totalprotos;i++)
    {
        if(nextlayerid==protoinfo[i].dwCatalogEntryId)
        {
            if(WSCGetProviderPath(&protoinfo[i].ProviderId,filterpath,&filterpathlen,&errorcode)==SOCKET_ERROR)
            {
                OutputDebugString(_T("WSCGetProviderPath Error!"));  
                return WSAEPROVIDERFAILEDINIT;
            }
            break;
        }
    }

    if(!ExpandEnvironmentStrings(filterpath,filterpath,MAX_PATH))
    {
        OutputDebugString(_T("ExpandEnvironmentStrings Error!"));    
        return WSAEPROVIDERFAILEDINIT;
    }

    if((hfilter=LoadLibrary(filterpath))==NULL)
    {
        OutputDebugString(_T("LoadLibrary Error!"));
        return WSAEPROVIDERFAILEDINIT;
    }

    if((wspstartupfunc=(LPWSPSTARTUP)GetProcAddress(hfilter,"WSPStartup"))==NULL)
    {
        OutputDebugString(_T("GetProcessAddress Error!"));  
        return WSAEPROVIDERFAILEDINIT;
    }

    if((errorcode=wspstartupfunc(wversionrequested,lpwspdata,lpprotoinfo,upcalltable,lpproctable))!=ERROR_SUCCESS)
    {
        OutputDebugString(_T("wspstartupfunc Error!"));  
        return errorcode;
    }

    nextproctable=*lpproctable;
    lpproctable->lpWSPSendTo = WSPSendTo;
    lpproctable->lpWSPConnect= WSPConnect;
	lpproctable->lpWSPBind = WSPBind;

    // Make sure that all of the procedures at least have a non null pointer.
    if( !lpproctable->lpWSPAccept              ||
        !lpproctable->lpWSPAddressToString     ||
        !lpproctable->lpWSPAsyncSelect         ||
        !lpproctable->lpWSPBind                ||
        !lpproctable->lpWSPCancelBlockingCall  ||
        !lpproctable->lpWSPCleanup             ||
        !lpproctable->lpWSPCloseSocket         ||
        !lpproctable->lpWSPConnect             ||
        !lpproctable->lpWSPDuplicateSocket     ||
        !lpproctable->lpWSPEnumNetworkEvents   ||
        !lpproctable->lpWSPEventSelect         ||
        !lpproctable->lpWSPGetOverlappedResult ||
        !lpproctable->lpWSPGetPeerName         ||
        !lpproctable->lpWSPGetSockName         ||
        !lpproctable->lpWSPGetSockOpt          ||
        !lpproctable->lpWSPGetQOSByName        ||
        !lpproctable->lpWSPIoctl               ||
        !lpproctable->lpWSPJoinLeaf            ||
        !lpproctable->lpWSPListen              ||
        !lpproctable->lpWSPRecv                ||
        !lpproctable->lpWSPRecvDisconnect      ||
        !lpproctable->lpWSPRecvFrom            ||
        !lpproctable->lpWSPSelect              ||
        !lpproctable->lpWSPSend                ||
        !lpproctable->lpWSPSendDisconnect      ||
        !lpproctable->lpWSPSendTo              ||
        !lpproctable->lpWSPSetSockOpt          ||
        !lpproctable->lpWSPShutdown            ||
        !lpproctable->lpWSPSocket              ||
        !lpproctable->lpWSPStringToAddress ){

        OutputDebugString(_T("Service provider returned an invalid procedure table"));

		freefilter();
        return WSAEINVALIDPROCTABLE;
	}

    freefilter();
    return 0;
}

