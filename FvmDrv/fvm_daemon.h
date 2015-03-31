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
 *  FVM daemon service virtualization header
 */

#ifndef _FVM_DAEMON_H
#define _FVM_DAEMON_H

//Service-related data structures and access routines here
#define SERVICE_CREATE_PROGRAM   L"services.exe"
#define CREATE_SERVICE_SIGN      L"___1234___"

#define SERVICE_INSTALLER_PROGRAM L"msiexec.exe"

//registry keys accessed to determine system directory
#define SYSTEM_ROOT_KEY       L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"
#define SYSTEM_ROOT_VALUE     L"SystemRoot"
#define DLL_DIR_KEY           L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\KnownDLLs"
#define DLL_DIR_VALUE         L"DllDirectory"

#define MAX_NUM_SERVICES   50
typedef struct _svctbl SVC_TBL_ENTRY, *PSVC_TBL_ENTRY;

struct _svctbl {
   WCHAR       ImagePath[256];
   WCHAR       SvcName[128];
   ULONG       DesiredAccess;
   ULONG       SvcType;
   ULONG       StartType;
   ULONG       ErrorControl;
   ULONG       pvm_id;
   SVC_TBL_ENTRY  *next;
};

PSVC_TBL_ENTRY svc_tbl[MAX_NUM_SERVICES];

//returns the service entry if the image s a valid service, null otherwise
PSVC_TBL_ENTRY IsService(PWCHAR imagepath);
PSVC_TBL_ENTRY AddService(PWCHAR path,
                          PWCHAR svcName,
                          ULONG da,
                          ULONG svctype,
                          ULONG starttype,
                          ULONG ec,
                          ULONG id);
BOOLEAN DelService(PWCHAR path, ULONG id);
ULONG GetSystemDirectory(PWCHAR path, PULONG plen);

extern ULONG curr_inst_vmid;
extern WCHAR svc_crt_prog[256];
extern WCHAR svc_inst_prog[256];
extern WCHAR inst_cli_prog[256];

#endif //_FVM_DAEMON_H
