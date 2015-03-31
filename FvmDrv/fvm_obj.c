/* **********************************************************
 * Copyright 2007 Rether Networks, Inc.  All rights reserved.
 * *********************************************************
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
 *  FVM object virtualization
 */

#include <ntddk.h>
#include <stdlib.h>
#include "fvm_util.h"
#include "fvm_table.h"
#include "hooksys.h"
#include "fvm_vm.h"
#include "fvm_syscalls.h"
#include "fvm_obj.h"

#define FVM_OBJ_POOL_TAG '4GAT'

HANDLE   FVM_ObjectDirectoryHandle[MAX_VM];
HANDLE   FVM_PortDirectoryHandle[MAX_VM];


// Convert the original object path into a virtualized object path
BOOLEAN MapObjectPath(IN PWCHAR sourcename, IN ULONG  vmid,
    OUT PWCHAR destname)
{
		PWCHAR dirptr;
		PWCHAR objprefix = CONF_OBJ_PATH;
		PWCHAR global_ns = L"Global\\";
		PWCHAR local_ns = L"Local\\";
		int nlen;
		BOOLEAN global = FALSE;
	
		nlen = wcslen(objprefix);
		if (wcsncmp(sourcename, objprefix, nlen) != 0) return FALSE;
	
		dirptr = sourcename + nlen;
	
		// Global named objects created inside the VM are mapped to local ones
		nlen = wcslen(global_ns);
		if (wcsncmp(dirptr, global_ns, nlen) == 0) {
			if (wcsstr(dirptr, L"_MSIExecute")||wcsstr(dirptr, L"SvcctrlStartEvent_A3752DX"))
				return FALSE;
			dirptr = dirptr + nlen; 
			global = TRUE;
			return FALSE;
		}
	
		nlen = wcslen(local_ns);
		if (wcsncmp(dirptr, local_ns, nlen) == 0)
			dirptr = dirptr + nlen;
	
		if(*dirptr == L'\0')
			return FALSE;
	
		_snwprintf(destname, _MAX_PATH, L"%sFVM-%u\\%s", objprefix, vmid, dirptr);
		return TRUE;
}


// Convert the original port object path into a virtualized object path
BOOLEAN MapPortObjectPath(IN PWCHAR sourcename, IN ULONG  vmid,
    OUT PWCHAR destname)
{
	PWCHAR dirptr;
	PWCHAR portprefix = CONF_PORT_PATH;
	PWCHAR global_ns = L"Global\\";
	PWCHAR local_ns = L"Local\\";
	PWCHAR ole_ns = L"OLE";
	PWCHAR rpc_ns = L"epmapper";	
	int nlen;

	nlen = wcslen(portprefix);
	if (wcsncmp(sourcename, portprefix, nlen) != 0) return FALSE;

	dirptr = sourcename + nlen;

	//for access COM or RPC server from vm
	if((wcsncmp(dirptr,ole_ns,3)==0)||(wcsncmp(dirptr,rpc_ns,8)==0))
		return FALSE;		

	nlen = wcslen(global_ns);
	if (wcsncmp(dirptr, global_ns, nlen) == 0) {
		//    DbgPrint("Object should be in global name space:%S\n", sourcename);
		  return FALSE;
	}

	nlen = wcslen(local_ns);
	if (wcsncmp(dirptr, local_ns, nlen) == 0)
		dirptr = dirptr + nlen;

	_snwprintf(destname, _MAX_PATH, L"%sFVM-%u\\%s", portprefix, vmid, dirptr);
	return TRUE;
}


// Convert the original object path into a virtualized object path
BOOLEAN MapSectionObjectPath(IN PWCHAR sourcename, IN ULONG  vmid,
    OUT PWCHAR destname)
{
	PWCHAR dirptr;
	PWCHAR objprefix = CONF_OBJ_PATH;
	PWCHAR global_ns = L"Global\\";
	PWCHAR local_ns = L"Local\\";
	int nlen;
	BOOLEAN global = FALSE;

	nlen = wcslen(objprefix);
	if (wcsncmp(sourcename, objprefix, nlen) != 0) return FALSE;

	dirptr = sourcename + nlen;

    //the global section object is also virtualized. 
    //The format of path of virtualized global section object is "\BaseNamedObjects\Global\FVM-0\sectionObjectName"
	nlen = wcslen(global_ns);
	if (wcsncmp(dirptr, global_ns, nlen) == 0) {
		dirptr = dirptr + nlen;	
		global = TRUE;
	}

	nlen = wcslen(local_ns);
	if (wcsncmp(dirptr, local_ns, nlen) == 0)
		dirptr = dirptr + nlen;

	if(*dirptr == L'\0')
		return FALSE;

	if(global)
		_snwprintf(destname, _MAX_PATH, L"%s%sFVM-%u\\%s", objprefix, global_ns,vmid, dirptr);
	else
		_snwprintf(destname, _MAX_PATH, L"%sFVM-%u\\%s", objprefix, vmid, dirptr);
	
	return TRUE;
}


NTSTATUS FvmObj_NtCreateMutant(OUT PHANDLE hMutex,
    IN ACCESS_MASK desiredAccess, IN POBJECT_ATTRIBUTES objectAttributes,
    IN BOOLEAN bOwnMutant)
{
	NTSTATUS rc;
	//PWCHAR BinPath = NULL;
	WCHAR objectName[_MAX_PATH];
	POBJECT_ATTRIBUTES poa = NULL;
	WCHAR vobjname[_MAX_PATH];
	ULONG memsize = _MAX_PATH;
	ULONG vmid = 0, pid = -1;

	InterlockedIncrement(&fvm_Calls_In_Progress);
	if (ExGetPreviousMode() == KernelMode) {
		goto Original_Call;
	}

	pid = (ULONG)PsGetCurrentProcessId();
	vmid = FvmVm_GetPVMId(pid);

	if (vmid == INVALID_VMID) goto Original_Call;

	if (!FvmUtil_GetSysCallArgument(objectAttributes, objectName)) {
		goto Original_Call;
	}

	if(!objectName[0]) goto Original_Call;

	if (!MapObjectPath(objectName, vmid, vobjname)) goto Original_Call;

	rc = FvmUtil_InitializeVMObjAttributes(objectAttributes, vobjname, &poa, &memsize);
	if (!NT_SUCCESS(rc)) {
		CHAR errstr[64];
		DbgPrint("ErrMem:%s\n", FvmUtil_ErrorString(rc, errstr));
		rc = STATUS_ACCESS_DENIED;
		goto NtExit;
	}

	rc = ((NtCreateMutantProc)(winNtCreateMutantProc)) (hMutex, desiredAccess, poa,
	         bOwnMutant);

	//DbgPrint("CreateMutant: %x %S--%S\n", rc, objectName, vobjname);
	memsize = 0;
	FvmVm_FreeVirtualMemory(NtCurrentProcess(), &poa, &memsize, MEM_RELEASE);
	goto NtExit;

Original_Call:
	rc = ((NtCreateMutantProc)(winNtCreateMutantProc)) (hMutex, desiredAccess,
	         objectAttributes, bOwnMutant);
NtExit:
	InterlockedDecrement(&fvm_Calls_In_Progress);
	return rc;
}


NTSTATUS FvmObj_NtOpenMutant(OUT PHANDLE hMutex, IN ACCESS_MASK desiredAccess,
    IN POBJECT_ATTRIBUTES objectAttributes)
{
	NTSTATUS rc;
	//PWCHAR BinPath = NULL;
	WCHAR ObjectName[_MAX_PATH];
	WCHAR vobjname[_MAX_PATH];
	ULONG memsize = _MAX_PATH;
	POBJECT_ATTRIBUTES poa = NULL;
	ULONG vmid = 0, pid = -1;

	InterlockedIncrement(&fvm_Calls_In_Progress);
	if (ExGetPreviousMode() == KernelMode) {
		goto Original_Call;
	}

	pid = (ULONG)PsGetCurrentProcessId();
	vmid = FvmVm_GetPVMId(pid);

	if (vmid == INVALID_VMID)  goto Original_Call;

	if (!FvmUtil_GetSysCallArgument(objectAttributes, ObjectName)) {
		goto Original_Call;
	}

	if(!ObjectName[0]) goto Original_Call;

	if (!MapObjectPath(ObjectName, vmid, vobjname)) goto Original_Call;

	rc = FvmUtil_InitializeVMObjAttributes(objectAttributes, vobjname, &poa, &memsize);
	if (!NT_SUCCESS(rc)) {
		CHAR errstr[64];
		DbgPrint("ErrMem:%s\n", FvmUtil_ErrorString(rc, errstr));
		rc = STATUS_ACCESS_DENIED;
		goto NtExit;
	}

	rc = ((NtOpenMutantProc)(winNtOpenMutantProc)) (hMutex, desiredAccess, poa);

	//DbgPrint("OpenMutant: %x %S--%S\n", rc, ObjectName, vobjname);
	memsize = 0;
	FvmVm_FreeVirtualMemory(NtCurrentProcess(), &poa, &memsize, MEM_RELEASE);
	goto NtExit;

Original_Call:
	rc = ((NtOpenMutantProc)(winNtOpenMutantProc)) (hMutex, desiredAccess,
		     objectAttributes);
NtExit:
	InterlockedDecrement (&fvm_Calls_In_Progress);
	return rc;
}


NTSTATUS FvmObj_NtCreateSemaphore(OUT PHANDLE hSemaphore,
    IN ACCESS_MASK desiredAccess, IN POBJECT_ATTRIBUTES objectAttributes,
    IN ULONG initialCount, IN ULONG maximumCount)
{
	NTSTATUS rc;
	//   PWCHAR BinPath = NULL;
	WCHAR ObjectName[_MAX_PATH];
	POBJECT_ATTRIBUTES poa = NULL;
	WCHAR vobjname[_MAX_PATH];
	ULONG memsize = _MAX_PATH;
	ULONG vmid = 0, pid = -1;

	InterlockedIncrement (&fvm_Calls_In_Progress);
	if (ExGetPreviousMode() == KernelMode) {
		goto Original_Call;
	}

	pid = (ULONG)PsGetCurrentProcessId();
	vmid = FvmVm_GetPVMId(pid);

	if (vmid == INVALID_VMID) goto Original_Call;

	if (!FvmUtil_GetSysCallArgument(objectAttributes, ObjectName)) {
		goto Original_Call;
	}

	if(!ObjectName[0]) goto Original_Call;

	if (!MapObjectPath(ObjectName, vmid, vobjname)) goto Original_Call;

	rc = FvmUtil_InitializeVMObjAttributes(objectAttributes, vobjname, &poa, &memsize);
	if (!NT_SUCCESS(rc)) {
		CHAR errstr[64];
		DbgPrint("ErrMem:%s\n", FvmUtil_ErrorString(rc, errstr));
		rc = STATUS_ACCESS_DENIED;
		goto NtExit;
	}

	rc = ((NtCreateSemaphoreProc)(winNtCreateSemaphoreProc)) (hSemaphore,
		     desiredAccess, poa, initialCount, maximumCount);

	memsize = 0;
	FvmVm_FreeVirtualMemory(NtCurrentProcess(), &poa, &memsize, MEM_RELEASE);
	goto NtExit;

Original_Call:
	rc = ((NtCreateSemaphoreProc)(winNtCreateSemaphoreProc)) (hSemaphore,
		     desiredAccess, objectAttributes, initialCount, maximumCount);
NtExit:
	InterlockedDecrement (&fvm_Calls_In_Progress);
	return rc;
}

NTSTATUS FvmObj_NtOpenSemaphore(OUT PHANDLE hSemaphore,
    IN ACCESS_MASK desiredAccess, IN POBJECT_ATTRIBUTES objectAttributes)
{
   NTSTATUS rc;
   //PWCHAR BinPath = NULL;
   WCHAR ObjectName[_MAX_PATH];
   POBJECT_ATTRIBUTES poa = NULL;
   WCHAR vobjname[_MAX_PATH];
   ULONG memsize = _MAX_PATH;
   ULONG vmid = 0, pid = -1;

   InterlockedIncrement (&fvm_Calls_In_Progress);
   if (ExGetPreviousMode() == KernelMode) {
		goto Original_Call;
   }

   pid = (ULONG)PsGetCurrentProcessId();
   vmid = FvmVm_GetPVMId(pid);

   if (vmid == INVALID_VMID)  goto Original_Call;

   if (!FvmUtil_GetSysCallArgument(objectAttributes, ObjectName)) {
		goto Original_Call;
   }

   if(!ObjectName[0]) goto Original_Call;

   if (!MapObjectPath(ObjectName, vmid, vobjname)) goto Original_Call;

   rc = FvmUtil_InitializeVMObjAttributes(objectAttributes, vobjname, &poa, &memsize);
   if (!NT_SUCCESS(rc)) {
		CHAR errstr[64];
		DbgPrint("ErrMem:%s\n", FvmUtil_ErrorString(rc, errstr));
		rc = STATUS_ACCESS_DENIED;
		goto NtExit;
   }

   rc = ((NtOpenSemaphoreProc)(winNtOpenSemaphoreProc)) (hSemaphore, desiredAccess,
		    poa);

	memsize = 0;
   FvmVm_FreeVirtualMemory(NtCurrentProcess(), &poa, &memsize, MEM_RELEASE);
   goto NtExit;

Original_Call:
   rc = ((NtOpenSemaphoreProc)(winNtOpenSemaphoreProc)) (hSemaphore, desiredAccess,
		    objectAttributes);

NtExit:
   InterlockedDecrement (&fvm_Calls_In_Progress);
   return rc;
}

NTSTATUS FvmObj_NtCreateEvent(OUT PHANDLE hEvent, IN ACCESS_MASK desiredAccess,
    IN POBJECT_ATTRIBUTES objectAttributes, IN EVENT_TYPE eventType,
    IN BOOLEAN bInitialState)
{
	NTSTATUS rc;
	//   PWCHAR BinPath = NULL;
	WCHAR objectName[_MAX_PATH];
	POBJECT_ATTRIBUTES poa = NULL;
	WCHAR vobjname[_MAX_PATH];
	ULONG memsize = _MAX_PATH;
	ULONG vmid = 0, pid = -1;

	InterlockedIncrement (&fvm_Calls_In_Progress);
	if (ExGetPreviousMode() == KernelMode) {
		goto Original_Call;
	}

	pid = (ULONG)PsGetCurrentProcessId();
	vmid = FvmVm_GetPVMId(pid);

	if (vmid == INVALID_VMID) goto Original_Call;

	if (!FvmUtil_GetSysCallArgument(objectAttributes, objectName)) {
		goto Original_Call;
	}

	if(!objectName[0]) goto Original_Call;

	if (!MapObjectPath(objectName, vmid, vobjname)) goto Original_Call;

	rc = FvmUtil_InitializeVMObjAttributes(objectAttributes, vobjname, &poa, &memsize);
	if (!NT_SUCCESS(rc)) {
		CHAR errstr[64];
		DbgPrint("ErrMem:%s\n", FvmUtil_ErrorString(rc, errstr));
		rc = STATUS_ACCESS_DENIED;
		goto NtExit;
	}

	rc = ((NtCreateEventProc)(winNtCreateEventProc)) (hEvent, desiredAccess, poa,
		     eventType, bInitialState);

	memsize = 0;
	FvmVm_FreeVirtualMemory(NtCurrentProcess(), &poa, &memsize, MEM_RELEASE);
	goto NtExit;

Original_Call:
	rc = ((NtCreateEventProc)(winNtCreateEventProc)) (hEvent, desiredAccess,
		     objectAttributes, eventType, bInitialState);

NtExit:
	InterlockedDecrement (&fvm_Calls_In_Progress);
	return rc;
}

NTSTATUS FvmObj_NtOpenEvent(OUT PHANDLE hEvent, IN ACCESS_MASK desiredAccess,
    IN POBJECT_ATTRIBUTES objectAttributes)
{
	NTSTATUS rc;
	//PWCHAR BinPath = NULL;
	WCHAR objectName[_MAX_PATH];
	WCHAR vobjname[_MAX_PATH];
	ULONG memsize = _MAX_PATH;
	POBJECT_ATTRIBUTES poa = NULL;
	ULONG vmid = 0, pid = -1;

	InterlockedIncrement (&fvm_Calls_In_Progress);
	if (ExGetPreviousMode() == KernelMode) {
		goto Original_Call;
	}

	pid = (ULONG)PsGetCurrentProcessId();
	vmid = FvmVm_GetPVMId(pid);

	if (vmid == INVALID_VMID)  goto Original_Call;

	if (!FvmUtil_GetSysCallArgument(objectAttributes, objectName)) {
		goto Original_Call;
	}

	if(!objectName[0]) goto Original_Call;
	else {
		WCHAR eventName[16];
		swprintf(eventName, L"fvm%u", pid);
		if (wcsstr(objectName, eventName)) {
			goto Original_Call;
		}
	}

	if (!MapObjectPath(objectName, vmid, vobjname))  goto Original_Call;

	rc = FvmUtil_InitializeVMObjAttributes(objectAttributes, vobjname, &poa, &memsize);
	if (!NT_SUCCESS(rc)) {
		CHAR errstr[64];
		DbgPrint("ErrMem:%s\n", FvmUtil_ErrorString(rc, errstr));
		rc = STATUS_ACCESS_DENIED;
		goto NtExit;
	}

	rc = ((NtOpenEventProc)(winNtOpenEventProc)) (hEvent, desiredAccess, poa);

	memsize = 0;
	FvmVm_FreeVirtualMemory(NtCurrentProcess(), &poa, &memsize, MEM_RELEASE);
	goto NtExit;

Original_Call:
	rc = ((NtOpenEventProc)(winNtOpenEventProc)) (hEvent, desiredAccess,
		     objectAttributes);

NtExit:
	InterlockedDecrement (&fvm_Calls_In_Progress);
	return rc;
}

NTSTATUS FvmObj_NtCreateSection (OUT PHANDLE phSection,
    IN ACCESS_MASK desiredAccess, IN POBJECT_ATTRIBUTES objectAttributes,
    IN PLARGE_INTEGER MaximumSize OPTIONAL, IN ULONG sectionPageProtection,
    IN ULONG allocationAttributes, IN HANDLE hFile OPTIONAL)
{
	NTSTATUS rc;
	//PWCHAR BinPath = NULL;
	WCHAR objectName[_MAX_PATH];
	POBJECT_ATTRIBUTES poa = NULL;
	WCHAR vobjname[_MAX_PATH];
	ULONG memsize = _MAX_PATH;
	ULONG vmid = 0, pid = -1;

	InterlockedIncrement (&fvm_Calls_In_Progress);
	if (ExGetPreviousMode() == KernelMode) {
		goto Original_Call;
	}

	pid = (ULONG)PsGetCurrentProcessId();
	vmid = FvmVm_GetPVMId(pid);

	if (vmid == INVALID_VMID) goto Original_Call;

	if (!FvmUtil_GetSysCallArgument(objectAttributes, objectName)) {
		goto Original_Call;
	}

	if(!objectName[0]) goto Original_Call;

	if (!MapSectionObjectPath(objectName, vmid, vobjname)) goto Original_Call;

	rc = FvmUtil_InitializeVMObjAttributes(objectAttributes, vobjname, &poa, &memsize);
	if (!NT_SUCCESS(rc)) {
		CHAR errstr[64];
		DbgPrint("ErrMem:%s\n", FvmUtil_ErrorString(rc, errstr));
		rc = STATUS_ACCESS_DENIED;
		goto NtExit;
	}

	rc = ((NtCreateSectionProc)(winNtCreateSectionProc)) (phSection, desiredAccess,
		     poa, MaximumSize, sectionPageProtection, allocationAttributes,
		     hFile);

	memsize = 0;
	FvmVm_FreeVirtualMemory(NtCurrentProcess(), &poa, &memsize, MEM_RELEASE);
	goto NtExit;

Original_Call:
	rc = ((NtCreateSectionProc)(winNtCreateSectionProc)) (phSection, desiredAccess,
		     objectAttributes, MaximumSize, sectionPageProtection,
		     allocationAttributes, hFile);

NtExit:
	InterlockedDecrement (&fvm_Calls_In_Progress);
	return rc;
}

NTSTATUS FvmObj_NtOpenSection(OUT PHANDLE phSection,
    IN ACCESS_MASK desiredAccess, IN POBJECT_ATTRIBUTES objectAttributes)
{
	NTSTATUS rc;
	//PWCHAR BinPath = NULL;
	WCHAR ObjectName[_MAX_PATH];
	WCHAR vobjname[_MAX_PATH];
	ULONG memsize = _MAX_PATH;
	POBJECT_ATTRIBUTES poa = NULL;
	ULONG vmid = 0, pid = -1;

	InterlockedIncrement (&fvm_Calls_In_Progress);
	if (ExGetPreviousMode() == KernelMode) {
		goto Original_Call;
	}

	pid = (ULONG)PsGetCurrentProcessId();
	vmid = FvmVm_GetPVMId(pid);

	if (vmid == INVALID_VMID)  goto Original_Call;

	if (!FvmUtil_GetSysCallArgument(objectAttributes, ObjectName)) {
		goto Original_Call;
	}

	if(!ObjectName[0]) goto Original_Call;


	if (!MapSectionObjectPath(ObjectName, vmid, vobjname))  goto Original_Call;
	rc = FvmUtil_InitializeVMObjAttributes(objectAttributes, vobjname, &poa, &memsize);
	if (!NT_SUCCESS(rc)) {
		CHAR errstr[64];
		DbgPrint("ErrMem:%s\n", FvmUtil_ErrorString(rc, errstr));
		rc = STATUS_ACCESS_DENIED;
		goto NtExit;
	}

	rc = ((NtOpenSectionProc)(winNtOpenSectionProc)) (phSection, desiredAccess, poa);

	memsize = 0;
	FvmVm_FreeVirtualMemory(NtCurrentProcess(), &poa, &memsize, MEM_RELEASE);
	goto NtExit;

Original_Call:
	rc = ((NtOpenSectionProc)(winNtOpenSectionProc)) (phSection, desiredAccess,
		     objectAttributes);

NtExit:
	InterlockedDecrement (&fvm_Calls_In_Progress);
	return rc;
}

NTSTATUS FvmObj_NtCreateTimer(OUT PHANDLE phTimer,
    IN ACCESS_MASK desiredAccess, IN POBJECT_ATTRIBUTES objectAttributes,
    IN TIMER_TYPE timerType)
{
	NTSTATUS rc;
	//PWCHAR BinPath = NULL;
	WCHAR objectName[_MAX_PATH];
	POBJECT_ATTRIBUTES poa = NULL;
	WCHAR vobjname[_MAX_PATH];
	ULONG memsize = _MAX_PATH;
	ULONG vmid = 0, pid = -1;

	InterlockedIncrement (&fvm_Calls_In_Progress);
	if (ExGetPreviousMode() == KernelMode) {
		goto Original_Call;
	}

	pid = (ULONG)PsGetCurrentProcessId();
	vmid = FvmVm_GetPVMId(pid);

	if (vmid == INVALID_VMID) goto Original_Call;

	if (!FvmUtil_GetSysCallArgument(objectAttributes, objectName)) {
		goto Original_Call;
	}

	if(!objectName[0]) goto Original_Call;

	if (!MapObjectPath(objectName, vmid, vobjname)) goto Original_Call;

	rc = FvmUtil_InitializeVMObjAttributes(objectAttributes, vobjname, &poa, &memsize);
	if (!NT_SUCCESS(rc)) {
		CHAR errstr[64];
		DbgPrint("ErrMem:%s\n", FvmUtil_ErrorString(rc, errstr));
		rc = STATUS_ACCESS_DENIED;
		goto NtExit;
	}

	rc = ((NtCreateTimerProc)(winNtCreateTimerProc)) (phTimer, desiredAccess, poa,
		     timerType);

	memsize = 0;
	FvmVm_FreeVirtualMemory(NtCurrentProcess(), &poa, &memsize, MEM_RELEASE);
	goto NtExit;

Original_Call:
	rc = ((NtCreateTimerProc)(winNtCreateTimerProc)) (phTimer, desiredAccess,
		     objectAttributes, timerType);

NtExit:
	InterlockedDecrement (&fvm_Calls_In_Progress);
	return rc;
}

NTSTATUS FvmObj_NtOpenTimer(OUT PHANDLE phTimer, IN ACCESS_MASK desiredAccess,
    IN POBJECT_ATTRIBUTES objectAttributes)
{
   NTSTATUS rc;
   //PWCHAR BinPath = NULL;
   WCHAR objectName[_MAX_PATH];
   POBJECT_ATTRIBUTES poa = NULL;
   WCHAR vobjname[_MAX_PATH];
   ULONG memsize = _MAX_PATH;
   ULONG vmid = 0, pid = -1;

   InterlockedIncrement (&fvm_Calls_In_Progress);
   if (ExGetPreviousMode() == KernelMode) {
		goto Original_Call;
   }

   pid = (ULONG)PsGetCurrentProcessId();
   vmid = FvmVm_GetPVMId(pid);

   if (vmid == INVALID_VMID)  goto Original_Call;

   if (!FvmUtil_GetSysCallArgument(objectAttributes, objectName)) {
		goto Original_Call;
   }

   if(!objectName[0]) goto Original_Call;

   if (!MapObjectPath(objectName, vmid, vobjname))  goto Original_Call;

   rc = FvmUtil_InitializeVMObjAttributes(objectAttributes, vobjname, &poa, &memsize);
   if (!NT_SUCCESS(rc)) {
		CHAR errstr[64];
		DbgPrint("ErrMem:%s\n", FvmUtil_ErrorString(rc, errstr));
		rc = STATUS_ACCESS_DENIED;
		goto NtExit;
   }

   rc = ((NtOpenTimerProc)(winNtOpenTimerProc)) (phTimer, desiredAccess, poa);

	memsize = 0;
   FvmVm_FreeVirtualMemory(NtCurrentProcess(), &poa, &memsize, MEM_RELEASE);
   goto NtExit;

Original_Call:
   rc = ((NtOpenTimerProc)(winNtOpenTimerProc)) (phTimer, desiredAccess,
   	        objectAttributes);

NtExit:
   InterlockedDecrement (&fvm_Calls_In_Progress);
   return rc;
}

NTSTATUS FvmObj_NtCreateIoCompletion(OUT PHANDLE phIoCompletionPort,
    IN ACCESS_MASK desiredAccess, IN POBJECT_ATTRIBUTES objectAttributes,
    IN ULONG nConcurrentThreads)
{
	NTSTATUS rc;
	//PWCHAR BinPath = NULL;
	WCHAR ObjectName[_MAX_PATH];
	POBJECT_ATTRIBUTES poa = NULL;
	WCHAR vobjname[_MAX_PATH];
	ULONG memsize = _MAX_PATH;
	ULONG vmid = 0, pid = -1;

	InterlockedIncrement (&fvm_Calls_In_Progress);
	if (ExGetPreviousMode() == KernelMode) {
	  goto Original_Call;
	}

	pid = (ULONG)PsGetCurrentProcessId();
	vmid = FvmVm_GetPVMId(pid);

	if (vmid == INVALID_VMID) goto Original_Call;

	if (!FvmUtil_GetSysCallArgument(objectAttributes, ObjectName)) {
		goto Original_Call;
	}

	if(!ObjectName[0]) goto Original_Call;

	if (!MapObjectPath(ObjectName, vmid, vobjname)) goto Original_Call;

	rc = FvmUtil_InitializeVMObjAttributes(objectAttributes, vobjname, &poa, &memsize);
	if (!NT_SUCCESS(rc)) {
		CHAR errstr[64];
		DbgPrint("ErrMem:%s\n", FvmUtil_ErrorString(rc, errstr));
		rc = STATUS_ACCESS_DENIED;
		goto NtExit;
	}

	rc = ((NtCreateIOCompletionProc)(winNtCreateIOCompletionProc)) (phIoCompletionPort,
		     desiredAccess, poa, nConcurrentThreads);

	memsize = 0;
	FvmVm_FreeVirtualMemory(NtCurrentProcess(), &poa, &memsize, MEM_RELEASE);
	goto NtExit;

Original_Call:
	rc = ((NtCreateIOCompletionProc)(winNtCreateIOCompletionProc)) (phIoCompletionPort,
		     desiredAccess, objectAttributes, nConcurrentThreads);

NtExit:
	InterlockedDecrement (&fvm_Calls_In_Progress);
	return rc;
}

NTSTATUS FvmObj_NtOpenIoCompletion(OUT PHANDLE phIoCompletionPort,
    IN ACCESS_MASK desiredAccess, IN POBJECT_ATTRIBUTES objectAttributes)
{
	NTSTATUS rc;
	//PWCHAR BinPath = NULL;
	WCHAR objectName[_MAX_PATH];
	POBJECT_ATTRIBUTES poa = NULL;
	WCHAR vobjname[_MAX_PATH];
	ULONG memsize = _MAX_PATH;
	ULONG vmid = 0, pid = -1;

	InterlockedIncrement (&fvm_Calls_In_Progress);
	if (ExGetPreviousMode() == KernelMode) {
		goto Original_Call;
	}

	pid = (ULONG)PsGetCurrentProcessId();
	vmid = FvmVm_GetPVMId(pid);

	if (vmid == INVALID_VMID)  goto Original_Call;

	if (!FvmUtil_GetSysCallArgument(objectAttributes, objectName)) {
		goto Original_Call;
	}

	if(!objectName[0]) goto Original_Call;

	if (!MapObjectPath(objectName, vmid, vobjname))  goto Original_Call;

	rc = FvmUtil_InitializeVMObjAttributes(objectAttributes, vobjname, &poa, &memsize);
	if (!NT_SUCCESS(rc)) {
		CHAR errstr[64];
		DbgPrint("ErrMem:%s\n", FvmUtil_ErrorString(rc, errstr));
		rc = STATUS_ACCESS_DENIED;
		goto NtExit;
	}

	rc = ((NtOpenIOCompletionProc)(winNtOpenIOCompletionProc)) (phIoCompletionPort,
		    desiredAccess, poa);

	memsize = 0;
	FvmVm_FreeVirtualMemory(NtCurrentProcess(), &poa, &memsize, MEM_RELEASE);
	goto NtExit;

Original_Call:
	rc = ((NtOpenIOCompletionProc)(winNtOpenIOCompletionProc)) (phIoCompletionPort,
		     desiredAccess, objectAttributes);

NtExit:
	InterlockedDecrement (&fvm_Calls_In_Progress);
	return rc;
}

NTSTATUS FvmObj_NtCreateEventPair(OUT PHANDLE hEventPair,
    IN ACCESS_MASK desiredAccess, IN POBJECT_ATTRIBUTES objectAttributes)
{
	NTSTATUS rc;
	//PWCHAR BinPath = NULL;
	WCHAR objectName[_MAX_PATH];
	POBJECT_ATTRIBUTES poa = NULL;
	WCHAR vobjname[_MAX_PATH];
	ULONG memsize = _MAX_PATH;
	ULONG vmid = 0, pid = -1;

	InterlockedIncrement (&fvm_Calls_In_Progress);
	if (ExGetPreviousMode() == KernelMode) {
		goto Original_Call;
	}

	pid = (ULONG)PsGetCurrentProcessId();
	vmid = FvmVm_GetPVMId(pid);

	if (vmid == INVALID_VMID)  goto Original_Call;

	if (!FvmUtil_GetSysCallArgument(objectAttributes, objectName)) {
		goto Original_Call;
	}

	if(!objectName[0]) goto Original_Call;

	if (!MapObjectPath(objectName, vmid, vobjname)) goto Original_Call;

	rc = FvmUtil_InitializeVMObjAttributes(objectAttributes, vobjname, &poa, &memsize);
	if (!NT_SUCCESS(rc)) {
		CHAR errstr[64];
		DbgPrint("ErrMem:%s\n", FvmUtil_ErrorString(rc, errstr));
		rc = STATUS_ACCESS_DENIED;
		goto NtExit;
	}

		rc = ((NtCreateEventPairProc)(winNtCreateEventPairProc)) (hEventPair,
		     desiredAccess, poa);

	memsize = 0;
	FvmVm_FreeVirtualMemory(NtCurrentProcess(), &poa, &memsize, MEM_RELEASE);
	goto NtExit;

Original_Call:
	rc = ((NtCreateEventPairProc)(winNtCreateEventPairProc)) (hEventPair,
		     desiredAccess, objectAttributes);

NtExit:
	InterlockedDecrement (&fvm_Calls_In_Progress);
	return rc;
}


NTSTATUS FvmObj_NtOpenProcess(
	OUT PHANDLE pHandle,
	IN ACCESS_MASK Access_Mask,
	IN POBJECT_ATTRIBUTES ObjectAttributes,
	IN PCLIENT_ID ClientID) {

	NTSTATUS rc = STATUS_SUCCESS;
	LONG vmid = 0, pid = -1, CVmId = 0;

	//DbgPrint("Inside OpenProcess\n");

	   
   InterlockedIncrement (&fvm_Calls_In_Progress);
	if (ExGetPreviousMode() == KernelMode) {
	  goto Original_Call;
	}

	pid = (ULONG)PsGetCurrentProcessId();
	vmid = FvmVm_GetPVMId(pid);
	CVmId = FvmVm_GetPVMId((ULONG)ClientID->UniqueProcess);
	//DbgPrint("Client VMID ID %ld  myvmid = %ld\n", CVmId, vmid);
#if 1
	if ((vmid != CVmId) && (vmid != -1)) {
		rc = STATUS_ACCESS_DENIED;
		//DbgPrint("OpenProcess..orig fn not called!\n");
		goto NtExit;
	}
#endif
Original_Call:
	rc = ((NTOpenProcess)(WinNtOpenProcess)) (
		  pHandle, Access_Mask, ObjectAttributes,
		  ClientID);
NtExit:
	InterlockedDecrement (&fvm_Calls_In_Progress);

	return rc;
}


NTSTATUS FvmObj_NtOpenEventPair(OUT PHANDLE hEventPair,
    IN ACCESS_MASK desiredAccess, IN POBJECT_ATTRIBUTES objectAttributes)
{
	NTSTATUS rc;
	//PWCHAR BinPath = NULL;
	WCHAR objectName[_MAX_PATH];
	WCHAR vobjname[_MAX_PATH];
	ULONG memsize = _MAX_PATH;
	POBJECT_ATTRIBUTES poa = NULL;
	ULONG vmid = 0, pid = -1;

	InterlockedIncrement (&fvm_Calls_In_Progress);
	if (ExGetPreviousMode() == KernelMode) {
		goto Original_Call;
	}

	pid = (ULONG)PsGetCurrentProcessId();
	vmid = FvmVm_GetPVMId(pid);

	if (vmid == INVALID_VMID)  goto Original_Call;

	if (!FvmUtil_GetSysCallArgument(objectAttributes, objectName)) {
		goto Original_Call;
	}

	if(!objectName[0]) goto Original_Call;

	if (!MapObjectPath(objectName, vmid, vobjname))  goto Original_Call;

	rc = FvmUtil_InitializeVMObjAttributes(objectAttributes, vobjname, &poa, &memsize);
	if (!NT_SUCCESS(rc)) {
		CHAR errstr[64];
		DbgPrint("ErrMem:%s\n", FvmUtil_ErrorString(rc, errstr));
		rc = STATUS_ACCESS_DENIED;
		goto NtExit;
	}

	rc = ((NtOpenEventPairProc)(winNtOpenEventPairProc)) (hEventPair, desiredAccess,
		     poa);

	memsize = 0;
	FvmVm_FreeVirtualMemory(NtCurrentProcess(), &poa, &memsize, MEM_RELEASE);
	goto NtExit;

Original_Call:
	rc = ((NtOpenEventPairProc)(winNtOpenEventPairProc)) (hEventPair, desiredAccess,
		     objectAttributes);

NtExit:
	InterlockedDecrement (&fvm_Calls_In_Progress);
	return rc;
}

NTSTATUS FvmObj_NtCreatePort(OUT PHANDLE portHandle,
    IN POBJECT_ATTRIBUTES objectAttributes, IN ULONG maxDataSize,
    IN ULONG maxMessageSize, IN ULONG reserved)
{
	NTSTATUS rc;
	PWCHAR BinPath = NULL, nameptr = NULL;
	WCHAR objectName[_MAX_PATH];
	POBJECT_ATTRIBUTES poa = NULL;
	WCHAR vobjname[_MAX_PATH];
	ULONG memsize = _MAX_PATH;
	LONG vmid = 0, pid = -1;

	InterlockedIncrement (&fvm_Calls_In_Progress);
	if (ExGetPreviousMode() == KernelMode) {
		goto Original_Call;
	}

	pid = (ULONG)PsGetCurrentProcessId();
	vmid = FvmVm_GetPVMId(pid);

	if (vmid == INVALID_VMID)  goto Original_Call;
	
	//DbgPrint("Calling original function ntcreateport\n");

	BinPath = ExAllocatePoolWithTag(PagedPool, _MAX_PATH * 2, FVM_OBJ_POOL_TAG);
	if (BinPath == NULL) goto Original_Call;

	FvmUtil_GetBinaryPathName(BinPath);


	if ((wcsstr(BinPath, L"msiexec") != NULL) ||
	  (wcsstr(BinPath, L"MsiExec") != NULL)) {
		if ((vmid == msi_owner) || (msi_owner == -1)) {
			ExFreePool(BinPath);
			goto Original_Call;
		}
	}

	ExFreePool(BinPath);

	if (!FvmUtil_GetSysCallArgument(objectAttributes, objectName)) {
		goto Original_Call;
	}

	if(!objectName[0]) goto Original_Call;

	if (!MapPortObjectPath(objectName, vmid, vobjname)) goto Original_Call;

	rc = FvmUtil_InitializeVMObjAttributes(objectAttributes, vobjname, &poa, &memsize);
	if (!NT_SUCCESS(rc)) {
		CHAR errstr[64];
		DbgPrint("ErrMem:%s\n", FvmUtil_ErrorString(rc, errstr));
		rc = STATUS_ACCESS_DENIED;
		goto NtExit;
	}

	rc = ((NtCreatePortProc)(winNtCreatePortProc)) (portHandle, poa, maxDataSize,
		     maxMessageSize, reserved);

	memsize = 0;
	FvmVm_FreeVirtualMemory(NtCurrentProcess(), &poa, &memsize, MEM_RELEASE);

	goto NtExit;

Original_Call:
	rc = ((NtCreatePortProc)(winNtCreatePortProc)) (portHandle, objectAttributes,
		     maxDataSize, maxMessageSize, reserved);

NtExit:
	InterlockedDecrement (&fvm_Calls_In_Progress);
	return rc;
}

NTSTATUS FvmObj_NtCreateWaitablePort(OUT PHANDLE portHandle,
    IN POBJECT_ATTRIBUTES objectAttributes, IN ULONG maxDataSize,
    IN ULONG maxMessageSize, IN ULONG reserved)
{
	NTSTATUS rc;
	PWCHAR BinPath = NULL;
	WCHAR objectName[_MAX_PATH];
	POBJECT_ATTRIBUTES poa = NULL;
	WCHAR vobjname[_MAX_PATH];
	ULONG memsize = _MAX_PATH;
	LONG vmid = 0, pid = -1;

	InterlockedIncrement (&fvm_Calls_In_Progress);
	if (ExGetPreviousMode() == KernelMode) {
		goto Original_Call;
	}

	pid = (ULONG)PsGetCurrentProcessId();
	vmid = FvmVm_GetPVMId(pid);


	if (vmid == INVALID_VMID)  goto Original_Call;
	BinPath = ExAllocatePoolWithTag(PagedPool, _MAX_PATH * 2, FVM_OBJ_POOL_TAG);
	if (BinPath == NULL) goto Original_Call;

	FvmUtil_GetBinaryPathName(BinPath);

	if ((wcsstr(BinPath, L"msiexec") != NULL) ||
		(wcsstr(BinPath, L"MsiExec") != NULL)) {
		if ((vmid == msi_owner) || (msi_owner == -1)) {
			ExFreePool(BinPath);
			goto Original_Call;
		}
	}
	
	//DbgPrint("NtCreateWaitablePort : Application Name - %S\n", BinPath);
	ExFreePool(BinPath);

	if (!FvmUtil_GetSysCallArgument(objectAttributes, objectName)) {
		goto Original_Call;
	}

	if(!objectName[0]) goto Original_Call;

	if (!MapPortObjectPath(objectName, vmid, vobjname)) goto Original_Call;

	rc = FvmUtil_InitializeVMObjAttributes(objectAttributes, vobjname, &poa, &memsize);
	if (!NT_SUCCESS(rc)) {
		CHAR errstr[64];
		DbgPrint("ErrMem:%s\n", FvmUtil_ErrorString(rc, errstr));
		rc = STATUS_ACCESS_DENIED;
		goto NtExit;
	}

	rc = ((NtCreateWaitablePortProc)(winNtCreateWaitablePortProc)) (portHandle, poa,
		     maxDataSize, maxMessageSize, reserved);

	memsize = 0;
	FvmVm_FreeVirtualMemory(NtCurrentProcess(), &poa, &memsize, MEM_RELEASE);
#if 0
	if (NT_SUCCESS(rc))
#endif
	goto NtExit;

Original_Call:
	rc = ((NtCreateWaitablePortProc)(winNtCreateWaitablePortProc)) (portHandle,
		     objectAttributes, maxDataSize, maxMessageSize, reserved);

NtExit:
	InterlockedDecrement (&fvm_Calls_In_Progress);
	return rc;
}

NTSTATUS FvmObj_NtConnectPort(OUT PHANDLE portHandle,
    IN PUNICODE_STRING portName, IN PSECURITY_QUALITY_OF_SERVICE securityQos,
    IN OUT PPORT_SECTION_WRITE WriteSection OPTIONAL,
    IN OUT PPORT_SECTION_READ ReadSection OPTIONAL,
    OUT PULONG maxMessageSize OPTIONAL, IN OUT PVOID ConnectData OPTIONAL,
    IN OUT PULONG ConnectDataLength OPTIONAL)
{
	NTSTATUS rc;
	PWCHAR binPath = NULL, BinPath = NULL;
	WCHAR objectName[_MAX_PATH];
	PUNICODE_STRING pName = NULL;
	PWCHAR vdiruser = NULL;
	WCHAR vobjname[_MAX_PATH];
	ULONG memsize = _MAX_PATH;
	LONG vmid = 0, pid = -1;
	ULONG len;

	InterlockedIncrement (&fvm_Calls_In_Progress);
	if (ExGetPreviousMode() == KernelMode) {
		goto Original_Call;
	}

	pid = (ULONG)PsGetCurrentProcessId();
	vmid = FvmVm_GetPVMId(pid);

	if (vmid == INVALID_VMID)  goto Original_Call;

	BinPath = ExAllocatePoolWithTag(PagedPool, _MAX_PATH * 2, FVM_OBJ_POOL_TAG);
	if (BinPath == NULL) goto Original_Call;

	FvmUtil_GetBinaryPathName(BinPath);

	if ((wcsstr(BinPath, L"msiexec") != NULL) ||
		(wcsstr(BinPath, L"MsiExec") != NULL)) {
		if ((vmid == msi_owner) || (msi_owner == -1)) {
			ExFreePool(BinPath);
			goto Original_Call;
		}
	}

	//DbgPrint("NtConnectPort : Application Name - %S\n", BinPath);
	ExFreePool(BinPath);

	if (portName && portName->Buffer) {
		len = (portName->Length) >> 1;
		wcsncpy(objectName, portName->Buffer, len);
		objectName[len] = 0;
	} else {
		goto Original_Call;
	}

	if (!MapPortObjectPath(objectName, vmid, vobjname)) goto Original_Call;

	memsize = sizeof(UNICODE_STRING) + wcslen(vobjname)*2 + 2;
	rc = FvmVm_AllocateVirtualMemory(NtCurrentProcess(), &pName, 0, &memsize,
		     MEM_COMMIT, PAGE_READWRITE);

	if (NT_SUCCESS(rc)) {
		(char *)vdiruser = ((char *)pName) + sizeof(UNICODE_STRING);
		wcsncpy(vdiruser, vobjname, _MAX_PATH);
		RtlInitUnicodeString(pName, vdiruser);
	} else {
		CHAR errstr[64];
		DbgPrint("ErrMem:%s\n", FvmUtil_ErrorString(rc, errstr));
		goto Original_Call;
	}

	rc = ((NtConnectPortProc)(winNtConnectPortProc)) (portHandle, pName, securityQos,
		     WriteSection, ReadSection, maxMessageSize, ConnectData,
		     ConnectDataLength);

	memsize = 0;
	FvmVm_FreeVirtualMemory(NtCurrentProcess(), &pName, &memsize, MEM_RELEASE);

	if (NT_SUCCESS(rc))
	goto NtExit;

Original_Call:
	rc = ((NtConnectPortProc)(winNtConnectPortProc)) (portHandle, portName,
		     securityQos, WriteSection, ReadSection, maxMessageSize,
		     ConnectData, ConnectDataLength);

NtExit:
	InterlockedDecrement (&fvm_Calls_In_Progress);
	return rc;
}


NTSTATUS FvmObj_NtSecureConnectPort(OUT PHANDLE portHandle,
    IN PUNICODE_STRING portName, IN PSECURITY_QUALITY_OF_SERVICE securityQos,
    IN OUT PPORT_SECTION_WRITE WriteSection OPTIONAL,
    IN PSID ServerSid OPTIONAL, IN OUT PPORT_SECTION_READ ReadSection OPTIONAL,
    OUT PULONG maxMessageSize OPTIONAL, IN OUT PVOID ConnectData OPTIONAL,
    IN OUT PULONG ConnectDataLength OPTIONAL)
{
	NTSTATUS rc;
	PWCHAR BinPath = NULL;
	WCHAR objectName[_MAX_PATH];
	PUNICODE_STRING pName = NULL;
	PWCHAR vdiruser = NULL;
	WCHAR vobjname[_MAX_PATH];
	ULONG memsize = _MAX_PATH;
	LONG vmid = 0, pid = -1;
	ULONG len;

	InterlockedIncrement (&fvm_Calls_In_Progress);
	if (ExGetPreviousMode() == KernelMode) {
		goto Original_Call;
	}

	pid = (ULONG)PsGetCurrentProcessId();
	vmid = FvmVm_GetPVMId(pid);

	if (vmid == INVALID_VMID)  goto Original_Call;

	BinPath = ExAllocatePoolWithTag(PagedPool, _MAX_PATH * 2, FVM_OBJ_POOL_TAG);
	if (BinPath == NULL) goto Original_Call;

	FvmUtil_GetBinaryPathName(BinPath);

	if ((wcsstr(BinPath, L"msiexec") != NULL) ||
		(wcsstr(BinPath, L"MsiExec") != NULL)) {
		if ((vmid == msi_owner) || (msi_owner == -1)) {
			ExFreePool(BinPath);
			goto Original_Call;
		}
	}


	//DbgPrint("NtSecureConnectPort : Application Name - %S\n", BinPath);
	ExFreePool(BinPath);

	if (portName && portName->Buffer) {
		len = (portName->Length) >> 1;
		wcsncpy(objectName, portName->Buffer, len);
		objectName[len] = 0;
	} else {
		goto Original_Call;
	}

	if (!MapPortObjectPath(objectName, vmid, vobjname)) goto Original_Call;

	memsize = sizeof(UNICODE_STRING) + wcslen(vobjname)*2 + 2;
	rc = FvmVm_AllocateVirtualMemory(NtCurrentProcess(), &pName, 0, &memsize,
		     MEM_COMMIT, PAGE_READWRITE);

	if (NT_SUCCESS(rc)) {
		(char *)vdiruser = ((char *)pName) + sizeof(UNICODE_STRING);
		wcsncpy(vdiruser, vobjname, _MAX_PATH);
		RtlInitUnicodeString(pName, vdiruser);
	}
	else {
		CHAR errstr[64];
		DbgPrint("ErrMem:%s\n", FvmUtil_ErrorString(rc, errstr));
		goto Original_Call;
	}

	rc = ((NtSecureConnectPortProc)(winNtSecureConnectPortProc)) (portHandle, pName,
		     securityQos, WriteSection, ServerSid, ReadSection, maxMessageSize,
		     ConnectData, ConnectDataLength);

	memsize = 0;
	FvmVm_FreeVirtualMemory(NtCurrentProcess(), &pName, &memsize, MEM_RELEASE);
	goto NtExit;


Original_Call:
	rc = ((NtSecureConnectPortProc)(winNtSecureConnectPortProc)) (portHandle, portName,
		     securityQos, WriteSection, ServerSid, ReadSection, maxMessageSize,
		     ConnectData, ConnectDataLength);

NtExit:
	InterlockedDecrement (&fvm_Calls_In_Progress);
	return rc;
}

NTSTATUS FvmObj_NtCreateDirectoryObject(OUT PHANDLE DirectoryHandle,
    IN ACCESS_MASK desiredAccess, IN POBJECT_ATTRIBUTES objectAttributes)
{
	NTSTATUS rc;
	ULONG vmid = 0, pid = -1;

	InterlockedIncrement (&fvm_Calls_In_Progress);
	if (ExGetPreviousMode() == KernelMode) {
		goto Original_Call;
	}

	pid = (ULONG)PsGetCurrentProcessId();
	vmid = FvmVm_GetPVMId(pid);

	if(vmid == INVALID_VMID) goto Original_Call;

Original_Call:
	//rc = ((NtCreateDirectoryObjectProc)(OldNtCreateDirectoryObject)) (
	//	     DirectoryHandle, desiredAccess, objectAttributes);

	rc = winNtOpenDirectoryObjectProc (
		     DirectoryHandle, desiredAccess, objectAttributes);

	InterlockedDecrement (&fvm_Calls_In_Progress);
	return rc;
}

NTSTATUS FvmObj_NtOpenDirectoryObject(OUT PHANDLE directoryHandle,
    IN ACCESS_MASK desiredAccess, IN POBJECT_ATTRIBUTES objectAttributes)
{
	NTSTATUS rc;
	ULONG vmid = 0, pid = -1;

	InterlockedIncrement (&fvm_Calls_In_Progress);
	if (ExGetPreviousMode() == KernelMode) {
		goto Original_Call;
	}

	pid = (ULONG)PsGetCurrentProcessId();
	vmid = FvmVm_GetPVMId(pid);

	if(vmid == INVALID_VMID) goto Original_Call;

	Original_Call:
	rc = winNtOpenDirectoryObjectProc (directoryHandle,
		     desiredAccess, objectAttributes);

	InterlockedDecrement (&fvm_Calls_In_Progress);
	return rc;
}

NTSTATUS FvmObj_NtCreateSymbolicLinkObject(OUT PHANDLE hSymbolicLink,
    IN ACCESS_MASK desiredAccess, IN POBJECT_ATTRIBUTES objectAttributes,
    IN PUNICODE_STRING targetName)
{
	NTSTATUS rc;
	ULONG vmid = 0, pid = -1;

	InterlockedIncrement (&fvm_Calls_In_Progress);
	if (ExGetPreviousMode() == KernelMode) {
		goto Original_Call;
	}

	pid = (ULONG)PsGetCurrentProcessId();
	vmid = FvmVm_GetPVMId(pid);

	if(vmid == INVALID_VMID) goto Original_Call;

	Original_Call:
	rc = winNtCreateSymbolicLinkObjectProc(
		     hSymbolicLink, desiredAccess, objectAttributes, targetName);

	InterlockedDecrement (&fvm_Calls_In_Progress);
	return rc;
}

NTSTATUS FvmObj_NtOpenSymbolicLinkObject(OUT PHANDLE hSymbolicLink,
    IN ACCESS_MASK desiredAccess, IN POBJECT_ATTRIBUTES objectAttributes)
{
	NTSTATUS rc;
	ULONG vmid = 0, pid = -1;

	InterlockedIncrement (&fvm_Calls_In_Progress);
	if (ExGetPreviousMode() == KernelMode) {
		goto Original_Call;
	}

	pid = (ULONG)PsGetCurrentProcessId();
	vmid = FvmVm_GetPVMId(pid);

	if(vmid == INVALID_VMID) goto Original_Call;

	Original_Call:
	rc = winNtOpenSymbolicLinkObjectProc (
		     hSymbolicLink, desiredAccess, objectAttributes);

	InterlockedDecrement (&fvm_Calls_In_Progress);
	return rc;
}

NTSTATUS FvmObj_NtClose(HANDLE handle)
{
	NTSTATUS rc;
	ULONG vmid = 0, pid = -1;

	InterlockedIncrement (&fvm_Calls_In_Progress);
	if (ExGetPreviousMode() == KernelMode) {
		goto Original_Call;
	}

	pid = (ULONG)PsGetCurrentProcessId();
	vmid = FvmVm_GetPVMId(pid);

	if (vmid != INVALID_VMID) {
		FvmTable_HandleTableRemove(handle, vmid);
	}

	Original_Call:
	if (handle)
		rc=((NtCloseProc)(winNtCloseProc))(handle);

	InterlockedDecrement (&fvm_Calls_In_Progress);
	return rc;
}

NTSTATUS FvmObj_CreateObjDirectory(ULONG vmid)
{
	WCHAR objDirPath[_MAX_PATH];
	UNICODE_STRING pathname;
	OBJECT_ATTRIBUTES oa;
	NTSTATUS rc = STATUS_SUCCESS;

	if (FVM_ObjectDirectoryHandle[vmid]) return rc;

	swprintf(objDirPath, L"\\BaseNamedObjects\\FVM-%u", vmid);

	RtlInitUnicodeString(&pathname, objDirPath);
	InitializeObjectAttributes(&oa, &pathname,
	    OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE, NULL, NULL);

	rc = ZwCreateDirectoryObject(&FVM_ObjectDirectoryHandle[vmid],
		     DIRECTORY_ALL_ACCESS, &oa);
	return rc;
}

NTSTATUS FvmObj_CreatePortDirectory(ULONG vmid)
{
	WCHAR portDirPath[_MAX_PATH];
	UNICODE_STRING pathname;
	OBJECT_ATTRIBUTES oa;
	NTSTATUS rc = STATUS_SUCCESS;

	if (FVM_PortDirectoryHandle[vmid]) return rc;

	swprintf(portDirPath, L"\\RPC Control\\FVM-%u", vmid);

	RtlInitUnicodeString(&pathname, portDirPath);
	InitializeObjectAttributes(&oa, &pathname,
	    OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE, NULL, NULL);

	rc = ZwCreateDirectoryObject(&FVM_PortDirectoryHandle[vmid],
		     DIRECTORY_ALL_ACCESS, &oa);
	return rc;
}

