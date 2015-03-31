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

/*
 *  FVM daemon service virtualization
 */

#include <ntddk.h>
#include <stdlib.h>
#include "hooksys.h"
#include "fvm_daemon.h"

#define FVM_DAEMON_POOL_TAG '1GAT'

WCHAR svc_crt_prog[256];
WCHAR svc_inst_prog[256];
WCHAR inst_cli_prog[256] = L"setup.exe";

PSVC_TBL_ENTRY IsService(PWCHAR imagepath)
{
   UINT i;
   for(i=0; i<MAX_NUM_SERVICES; i++) {
      if(svc_tbl[i] == NULL) continue;
      if(0 == _wcsicmp(imagepath, svc_tbl[i]->ImagePath))
         return svc_tbl[i];
   }
   return NULL;
}

PSVC_TBL_ENTRY AddService(PWCHAR path, PWCHAR svcName, ULONG da, ULONG svctype, ULONG starttype, ULONG ec, ULONG id)
{
   PSVC_TBL_ENTRY ste;
   UINT i;

   for(i=0; i<MAX_NUM_SERVICES; i++) {
      if(svc_tbl[i] == NULL) break;
   }
   if(i==MAX_NUM_SERVICES) return NULL;
   ste = (PSVC_TBL_ENTRY)ExAllocatePoolWithTag(PagedPool, sizeof(SVC_TBL_ENTRY), FVM_DAEMON_POOL_TAG);
   if(!ste) {
      DbgPrint("Unable to allocate memory in the driver\n");
      return NULL;
   }
   svc_tbl[i] = ste;
   wcscpy(ste->ImagePath, path);
   wcscpy(ste->SvcName, svcName);
   ste->DesiredAccess = da;
   ste->SvcType = svctype;
   ste->StartType = starttype;
   ste->ErrorControl = ec;
   ste->pvm_id = id;
   DbgPrint("Service Added, Name:%S, Exe:%S", svcName, path);
   return ste;
}

BOOLEAN DelService(PWCHAR path, ULONG id)
{
   UINT i;
   for(i=0; i<MAX_NUM_SERVICES; i++) {
      if(svc_tbl[i] == NULL) continue;
      if(0 == _wcsicmp(path, svc_tbl[i]->ImagePath) && svc_tbl[i]->pvm_id == id) {
         ExFreePool(svc_tbl[i]);
         svc_tbl[i] = NULL;
         return TRUE;
      }
   }
   return FALSE;
}

BOOLEAN QueryRegValue(PWCHAR KeyPath, PWCHAR ValName, PWCHAR res_data)
{
   NTSTATUS Status;
   HANDLE hKey;
   OBJECT_ATTRIBUTES attr;
   ULONG BytesCopied;
   UINT i;
   PUNICODE_STRING uKeyName=NULL,uValueName=NULL;
   WCHAR wzKey[255], wzVal[255];
   WCHAR wzResult[512]; WCHAR *wch;
   KEY_VALUE_FULL_INFORMATION *kvfi = NULL;
   char *ch;

   uKeyName = (PUNICODE_STRING)wzKey;
   uValueName = (PUNICODE_STRING)wzVal;

   RtlInitUnicodeString(uKeyName,KeyPath );
   RtlInitUnicodeString(uValueName,ValName);

   attr.Length = sizeof(attr);
   attr.RootDirectory = NULL;
   attr.ObjectName = uKeyName;
   attr.Attributes = 0;
   attr.SecurityDescriptor = NULL;
   attr.SecurityQualityOfService = NULL;

   Status = ZwOpenKey(&hKey,
            KEY_QUERY_VALUE,
            &attr);
   if (!NT_SUCCESS(Status)){
      DbgPrint("Unable to open the Registry Key %S\n",KeyPath);
      return FALSE;
   }
   Status=ZwQueryValueKey(
      hKey,
      uValueName,
      KeyValueFullInformation,
      wzResult,
      512,
      &BytesCopied);

   if (!NT_SUCCESS(Status)){
      DbgPrint("Unable to query the Registry Name %S -- ",ValName);
      switch(Status){
         case STATUS_BUFFER_OVERFLOW: DbgPrint("Buffer Overflow!\n"); break;
         case STATUS_BUFFER_TOO_SMALL: DbgPrint("Buffer too small!\n"); break;
         case STATUS_INVALID_PARAMETER: DbgPrint("Invalid Parameter!\n"); break;
         default: DbgPrint("Unknown error code!\n"); break;
      }
      return FALSE;
   }


// DbgPrint("Value of %s\\%s is %S\n",KeyPath,ValName,((KEY_FULL_INFORMATION)wzResult).Name);

   kvfi = (KEY_VALUE_FULL_INFORMATION *)wzResult;
   //DbgPrint("Result Length = %d\n", BytesCopied);
   //DbgPrint("Type = %d, DataOffset = %d, DataLength = %d, NameLength = %d\n", kvfi->Type, kvfi->DataOffset, kvfi->DataLength, kvfi->NameLength);
   //ch = (char *)kvfi->Name;
   //for(i=0; i<kvfi->NameLength; i++)
   // DbgPrint("Name(%d) -> <%c> ", i, ch[i]);
   ch = (char *)wzResult;
   //for(i=0; i<kvfi->DataLength; i++)
   // DbgPrint("Data(%d) -> <%c> ", i, ch[i+kvfi->DataOffset]);
   wch = (WCHAR *)&ch[kvfi->DataOffset];
   wcscpy(res_data, wch);
   return TRUE;
}


//fills in the system folder path in the form \\device\\...
//Input 'plen' points to the number of available bytes in 'path'
//Output 'plen' points to the actual number of bytes fillen in
ULONG GetSystemDirectory(PWCHAR path, PULONG plen)
{

   WCHAR qRes[256]; WCHAR *wc;
   _wcsnset(path, 0, *plen);
   _wcsnset(qRes, 0, 256);
   /*
   QueryRegValue(SYSTEM_PARTITION_KEY, SYSTEM_PARTITION_VALUE, qRes);
   //DbgPrint("Key Data for SysPart = %S\n", qRes);
   wcscpy(path, qRes);
   _wcsnset(qRes, 0, 256);
   */
   QueryRegValue(SYSTEM_ROOT_KEY, SYSTEM_ROOT_VALUE, qRes);
   wcscpy(path, qRes);

   _wcsnset(qRes, 0, 256);

   QueryRegValue(DLL_DIR_KEY, DLL_DIR_VALUE, qRes);
   wc = wcschr(qRes, L'\\');
   if(wc != NULL){
      wcscat(path, L"\\");
      wcscat(path, &wc[1]);
   }
   //DbgPrint("Path = <%S>", path);
   //wcscpy(path, L"\\device\\harddiskvolume2\\WINDOWS\\system32");
   return 1; //success
}
