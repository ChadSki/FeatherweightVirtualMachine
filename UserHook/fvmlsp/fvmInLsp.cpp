#define  UNICODE    
#define  _UNICODE        

#include <stdio.h>
#include <tchar.h>
#include <string.h>
#include <ws2spi.h>
#include <sporder.h>
#include <rpc.h>

#define PFL_HIDDEN  0x00000004                          
#define BASE_PROTOCOL      1
#define LAYERED_PROTOCOL   0
#define LAYERED_PROVIDER_NAME L"FVM LSP"
#define CONFIGURATION_KEY TEXT("SOFTWARE\\FVM_LSP")

extern GUID filterguid;

extern BOOL  getfilter();
extern void  freefilter();
void  installfilter();
void  removefilter();
void  usage();

extern int            totalprotos;
extern DWORD          protoinfosize;
extern LPWSAPROTOCOL_INFOW   protoinfo;

int main(int argc,char *argv[])          
{
    if(argc==2)
    {
        if(strcmp(argv[1],"-install")==0)  
        {
            installfilter();
            return 0;
        }
        else if(strcmp(argv[1],"-remove")==0)  
        {
            removefilter();
            return 0;
        }
    }
    usage();
    return 0;
}

VOID
UninstallProvider()
{
    INT Errno;

    WSCDeinstallProvider(&filterguid, &Errno);
}

INT
InstallProvider(PDWORD CatalogId)
{
    WSAPROTOCOL_INFOW  proto_info;
    int install_result;
    int install_error;

    // Create a PROTOCOL_INFO to install for our provider DLL.
    proto_info.dwServiceFlags1 = 0;
    proto_info.dwServiceFlags2 = 0;
    proto_info.dwServiceFlags3 = 0;
    proto_info.dwServiceFlags4 = 0;
    proto_info.dwProviderFlags = PFL_HIDDEN;
    proto_info.ProviderId      = filterguid;
    proto_info.dwCatalogEntryId = 0;   // filled in by system
    proto_info.ProtocolChain.ChainLen = LAYERED_PROTOCOL;

    // Do  not  need  to  fill  in  chain  for LAYERED_PROTOCOL or
    // BASE_PROTOCOL
    proto_info.iVersion = 0;
    proto_info.iAddressFamily = AF_INET;
    proto_info.iMaxSockAddr = 16;
    proto_info.iMinSockAddr = 16;
    proto_info.iSocketType = SOCK_STREAM;
    proto_info.iProtocol = IPPROTO_TCP;   // mimic TCP/IP
    proto_info.iProtocolMaxOffset = 0;
    proto_info.iNetworkByteOrder = BIGENDIAN;
    proto_info.iSecurityScheme = SECURITY_PROTOCOL_NONE;
    proto_info.dwMessageSize = 0;  // stream-oriented
    proto_info.dwProviderReserved = 0;
    wcscpy(proto_info.szProtocol, LAYERED_PROVIDER_NAME);

    install_result = WSCInstallProvider(
        &filterguid,
        L"fvmLsp.dll",                // lpszProviderDllPath
        & proto_info,                 // lpProtocolInfoList
        1,                            // dwNumberOfEntries
        & install_error);             // lpErrno
    *CatalogId = proto_info.dwCatalogEntryId;

    return(install_result);
}

INT
InstallChain(LPWSAPROTOCOL_INFOW BaseProtocolInfoBuff,
    DWORD LayeredProviderCatalogId, HKEY ConfigRegisteryKey)
{
    WSAPROTOCOL_INFOW ProtocolChainProtoInfo;
    WCHAR             DebugPrefix[] = L"LAYERED ";
    INT               ReturnCode;
    INT               Errno;
    UUID              NewChainId;
    RPC_STATUS        Status;
    PUCHAR            GuidString;
    HKEY              NewKey;
    DWORD             KeyDisposition;
    BOOL              Continue;

    ReturnCode = NO_ERROR;

    // We are only layering on top of base providers
    if (BaseProtocolInfoBuff->ProtocolChain.ChainLen == BASE_PROTOCOL){
        Continue = FALSE;

        // Get a new GUID for the protocol chain we are about to install
        Status = UuidCreate(&NewChainId);
        if (RPC_S_OK == Status){

            //Get the string representaion of the GUID
            Status = UuidToString(&NewChainId, reinterpret_cast<unsigned short**>(&GuidString));
            if (RPC_S_OK == Status){
                // Write down the GUID  in the registry so we know who to
                // uninstall
                RegCreateKeyEx(
                    ConfigRegisteryKey,                 // hkey
                    (LPCTSTR)GuidString,                // lpszSubKey
                    0,                                  // dwReserved
                    NULL,                               // lpszClass
                    REG_OPTION_NON_VOLATILE,            // fdwOptions
                    KEY_ALL_ACCESS,                     // samDesired
                    NULL,                               // lpSecurityAttributes
                    &NewKey,                            // phkResult
                    &KeyDisposition                     // lpdwDisposition
                    );
                RpcStringFree(reinterpret_cast<unsigned short**>(&GuidString));

                Continue =TRUE;
            } else{
                printf("UuidToString() Failed\n");
            }
        } else{
            printf("UuidCreate Failed\n");
        }

        if (Continue){

            ProtocolChainProtoInfo = *BaseProtocolInfoBuff;
            ProtocolChainProtoInfo.dwServiceFlags1 &= ~XP1_IFS_HANDLES;

            ProtocolChainProtoInfo.ProviderId = NewChainId;

            wcscpy(ProtocolChainProtoInfo.szProtocol, DebugPrefix);
            wcscat(ProtocolChainProtoInfo.szProtocol, BaseProtocolInfoBuff->szProtocol);

            ProtocolChainProtoInfo.ProtocolChain.ChainLen = 2;
            ProtocolChainProtoInfo.ProtocolChain.ChainEntries[0] =
                LayeredProviderCatalogId;
            ProtocolChainProtoInfo.ProtocolChain.ChainEntries[1] =
                BaseProtocolInfoBuff->dwCatalogEntryId;

            ReturnCode = WSCInstallProvider(&NewChainId, L"fvmLsp.dll",
				&ProtocolChainProtoInfo, 1, &Errno);
            if (ReturnCode==0)
                printf ("Installed over %ls.\n",
                     BaseProtocolInfoBuff->szProtocol);
            else
                printf ("Installation over %ls failed with error %ld.\n",
                     BaseProtocolInfoBuff->szProtocol, Errno);
        }
    }
    return(ReturnCode);
}

void installfilter()
{
    LPWSAPROTOCOL_INFOW   ProtocolInfoBuff = NULL;
    DWORD                ProtocolInfoBuffSize = 0;
    INT                  ErrorCode;
    INT                  EnumResult;
    LONG                 lresult;
    HKEY                 NewKey;
    DWORD                KeyDisposition;
    INT                  Index;
    DWORD                CatalogEntryId;
    BOOL                 EntryIdFound;
    DWORD                *CatIdBuff;
    DWORD                nCatIds;

    // Verify that we are installing
    lresult = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,                     // hkey
        CONFIGURATION_KEY,                      // lpszSubKey
        0,                                      // dwReserved
        KEY_ALL_ACCESS,                        // samDesired
        &NewKey                               // phkResult
        );

    if (ERROR_SUCCESS != lresult){
        // The layered provider is not installed, so we are going to install.
        RegCreateKeyEx(
            HKEY_LOCAL_MACHINE,                 // hkey
            CONFIGURATION_KEY,                  // lpszSubKey
            0,                                  // dwReserved
            NULL,                               // lpszClass
            REG_OPTION_NON_VOLATILE,            // fdwOptions
            KEY_ALL_ACCESS,                     // samDesired
            NULL,                               // lpSecurityAttributes
            &NewKey,                            // phkResult
            &KeyDisposition                     // lpdwDisposition
            );

        // Install a dummy PROTOCOL_INFO for the layered provider.
        lresult = InstallProvider(&CatalogEntryId);
        if (NO_ERROR != lresult){
			printf("Installing provider failed !\n");
		} else {
            //
            // Enumerate the installed providers and chains
            //
            printf("Scanning Installed Providers...\n");
            // Call WSCEnumProtocols with a zero length buffer so we know what
            // size to send in to get all the installed PROTOCOL_INFO
            // structs.
            WSCEnumProtocols(
                NULL,                     // lpiProtocols
                ProtocolInfoBuff,         // lpProtocolBuffer
                & ProtocolInfoBuffSize,   // lpdwBufferLength
                & ErrorCode);             // lpErrno

            ProtocolInfoBuff = (LPWSAPROTOCOL_INFOW)malloc(ProtocolInfoBuffSize);
            if (ProtocolInfoBuff){
                printf("Installing Layered Providers...\n");

                EnumResult = WSCEnumProtocols(
                    NULL,                     // lpiProtocols
                    ProtocolInfoBuff,         // lpProtocolBuffer
                    & ProtocolInfoBuffSize,   // lpdwBufferLength
                    & ErrorCode);

                if (SOCKET_ERROR != EnumResult){

                    // Find our provider entry to get our catalog entry ID
                    EntryIdFound = FALSE;
                    for (Index =0; Index < EnumResult; Index++){
                        if (memcmp (&ProtocolInfoBuff[Index].ProviderId,
                                &filterguid,
                                sizeof (filterguid))==0){

                            CatalogEntryId =
                                ProtocolInfoBuff[Index].dwCatalogEntryId;
                            EntryIdFound = TRUE;
                        }
                    }
                    if (EntryIdFound){
                        for (Index =0; Index < EnumResult; Index++){
                            InstallChain(&ProtocolInfoBuff[Index],
                                CatalogEntryId,
                                NewKey);
                        }
                        free (ProtocolInfoBuff);

                        // Enumerate the installed providers and chains
                        // Call WSCEnumProtocols with a zero length buffer so
						// we know what size to  send in to get all the
						// installed PROTOCOL_INFO structs.
                        ProtocolInfoBuffSize = 0;
                        WSCEnumProtocols(
                            NULL,                     // lpiProtocols
                            NULL,                     // lpProtocolBuffer
                            & ProtocolInfoBuffSize,   // lpdwBufferLength
                            & ErrorCode);             // lpErrno

                        ProtocolInfoBuff = (LPWSAPROTOCOL_INFOW)
							malloc(ProtocolInfoBuffSize);
                        if (ProtocolInfoBuff){
                            printf("Reodering Installed Chains...\n");

                            EnumResult = WSCEnumProtocols(
                                NULL,                     // lpiProtocols
                                ProtocolInfoBuff,         // lpProtocolBuffer
                                & ProtocolInfoBuffSize,   // lpdwBufferLength
                                & ErrorCode);

                            if (SOCKET_ERROR != EnumResult){
                                // Allocate buffer to hold catalog ID array
                                CatIdBuff = (DWORD *)
									malloc(sizeof (DWORD)*EnumResult);
                                if (CatIdBuff!=NULL) {
                                    // Put our catalog chains first
                                    nCatIds = 0;
                                    for (Index =0; Index < EnumResult; Index++){
                                        if ((ProtocolInfoBuff[Index].ProtocolChain.ChainLen>1)
                                                && (ProtocolInfoBuff[Index].ProtocolChain.ChainEntries[0]==CatalogEntryId))
                                            CatIdBuff[nCatIds++] = ProtocolInfoBuff[Index].dwCatalogEntryId;
                                    }

                                    // Put the rest next
                                    for (Index =0; Index < EnumResult; Index++){
                                        if ((ProtocolInfoBuff[Index].ProtocolChain.ChainLen<=1)
                                                || (ProtocolInfoBuff[Index].ProtocolChain.ChainEntries[0]!=CatalogEntryId))
                                            CatIdBuff[nCatIds++] = ProtocolInfoBuff[Index].dwCatalogEntryId;
                                    }
                                    // Save new protocol order
                                    printf ("Saving New Protocol Order...\n");
                                    ErrorCode = WSCWriteProviderOrder (CatIdBuff, nCatIds);
                                    if (ErrorCode!=NO_ERROR)
                                        printf ("Reodering failed with error %ld", ErrorCode);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    _tprintf(_T("\nFVM LSP Filter installed successfully."));
    return ;
}

void removefilter()
{
    INT   ErrorCode;
    LONG  lresult;
    HKEY  NewKey;
    GUID  ProviderID;
    INT   Index;
    WCHAR  GuidStringBuffer[40];
    DWORD GuidStringBufferLen;
    FILETIME FileTime;

    // Verify that we are deinstalling
    lresult = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,                     // hkey
        CONFIGURATION_KEY,                      // lpszSubKey
        0,                                      // dwReserved
        KEY_ALL_ACCESS,                         // samDesired
        &NewKey                                 // phkResult
        );

    if (ERROR_SUCCESS == lresult){
        // The layered provider is installed so we are going uninstall.
        // Enumerate all the provider IDs we stored on install and deinstall
        // the providers
        printf("Removing Installed Layered Providers...\n");

        Index = 0;
        GuidStringBufferLen = sizeof(GuidStringBuffer);

        lresult = RegEnumKeyEx(
            NewKey,               // hKey
            Index,                // Index of subkey
			GuidStringBuffer,     // Buffer to hold key name
            &GuidStringBufferLen, // Length of buffer
            NULL,                 // Reserved
            NULL,                 // Class buffer
            NULL,                 // Class buffer length
            &FileTime             // Last write time
            );

        printf("Removing layered provider protocol chains...\n");
        while (lresult != ERROR_NO_MORE_ITEMS){
            UuidFromString(reinterpret_cast<unsigned short*>(GuidStringBuffer), &ProviderID);
            // Deinstall the provider chain we installed
            WSCDeinstallProvider(&ProviderID, &ErrorCode);
            // Delete our registry key
            RegDeleteKey(NewKey, (LPCWSTR)&GuidStringBuffer[0]);

            GuidStringBufferLen = sizeof(GuidStringBuffer);
			lresult = RegEnumKeyEx(
				NewKey,               // hKey
				Index,                // Index of subkey
				GuidStringBuffer,     // Buffer to hold key name
				&GuidStringBufferLen, // Length of buffer
				NULL,                 // Reserved
				NULL,                 // Class buffer
				NULL,                 // Class buffer length
				&FileTime             // Last write time
			 );
        }

        // Clean up the registry
        RegCloseKey(NewKey);
        RegDeleteKey(HKEY_LOCAL_MACHINE, CONFIGURATION_KEY);

        // Uninstall the real provider
        UninstallProvider();
    }

    return ;
}

void  usage()
{
    _tprintf(_T("Usage:  fvmInLsp [ -install | -remove ]\n"));
    return ;
}
