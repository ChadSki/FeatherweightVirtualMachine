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
#include <ntddk.h>
#include <stdlib.h>
#include <stdio.h>
#include "hooksys.h"
#include "fvm_vm.h"
#include "fvm_syscalls.h"
#include "fvm_reg.h"
#include "fvm_table.h"
#include "fvm_util.h"

#define FVM_UTIL_POOL_TAG '7GAT'

/*
 * Get the VM number.
 */
int
FvmUtil_VmNumber() {
	return FvmVm_GetPVMId((ULONG) PsGetCurrentProcessId());
}

/*
  * Returns the complete path with binary name of the currently executing process.
  */
void
FvmUtil_GetBinaryPathName0(PWCHAR BinaryPath, int Cat) {
	CHAR *ptr = (CHAR *)PsGetCurrentProcess();
	PFVM_PROCESS_PARAMETERS ProcParam;
	PPEB curpeb;
	WCHAR buf[_MAX_PATH];
	int nameLen;

	*BinaryPath = L'\0';

	if (!ptr) {
		return;
	}
	ptr = ptr+0x1b0;
	curpeb = *(PPEB *)(ptr);
	if (!curpeb) {
		return;
	}

	ptr = (char *)curpeb + 0x10;
	ProcParam = *(PFVM_PROCESS_PARAMETERS *)ptr;

	if (!ProcParam) {
		return;
	}

	if (!ProcParam->ApplicationName.Buffer ||
	    ProcParam->ApplicationName.Length >= _MAX_PATH ||
	    ProcParam->ApplicationName.Length < 2) {
			return;
	}

	try {
    	ProbeForRead(ProcParam->ApplicationName.Buffer,
    		ProcParam->ApplicationName.Length, 2);

		nameLen = ProcParam->ApplicationName.Length >> 1;
		wcsncpy(buf, ProcParam->ApplicationName.Buffer, nameLen);
		buf[nameLen] = L'\0';

	} except(EXCEPTION_EXECUTE_HANDLER) {
		return;
	}

	if (Cat) {
		if (0 != _wcsnicmp(buf,L"\\??\\", wcslen(L"\\??\\"))) {
			wcscpy(BinaryPath,L"\\??\\");
		}
	}
	wcscat(BinaryPath, buf);
}

void FvmUtil_GetBinaryPathName(PWCHAR wzBinaryPath)
{
  FvmUtil_GetBinaryPathName0(wzBinaryPath, 1);
}

BOOLEAN
FvmUtil_GetSysCallArgument(IN POBJECT_ATTRIBUTES ObjectAttributes,
	    OUT PWCHAR PwzDest) {
	NTSTATUS rc;
	CHAR ParentDirectory[1024];
	PUNICODE_STRING Parent = NULL;
	UINT len;

	PwzDest[0] = L'\0';
	ParentDirectory[0] = '\0';

	if (!ObjectAttributes) {
		return FALSE;
	}

	if (ObjectAttributes->RootDirectory) {
		PVOID Object;
		Parent = (PUNICODE_STRING)ParentDirectory;
		rc = ObReferenceObjectByHandle(ObjectAttributes->RootDirectory,
    			0, 0, KernelMode, &Object, NULL);
		if (NT_SUCCESS(rc)) {
			int BytesReturned;
			rc = ObQueryNameString(Object, ParentDirectory,
				    sizeof(ParentDirectory), &BytesReturned);
			ObDereferenceObject(Object);
		}
	}

	/*
	 * Obtain the File Path and Name information
	 */
	if ((Parent)&&(Parent->Buffer)) {
		len = (Parent->Length) >> 1;
		wcsncpy(PwzDest, Parent->Buffer, len);
		PwzDest[len] = 0;
		if (PwzDest[len-1] != L'\\') {
			wcscat(PwzDest, L"\\");
		}
	}

	if ((ObjectAttributes->ObjectName) &&
	    (ObjectAttributes->ObjectName->Buffer)) {
		try {
			ProbeForRead(ObjectAttributes->ObjectName->Buffer,
    			ObjectAttributes->ObjectName->Length, 2);
			len = (ObjectAttributes->ObjectName->Length) >> 1;
			wcsncat(PwzDest, ObjectAttributes->ObjectName->Buffer, len);
			return TRUE;
		} except (EXCEPTION_EXECUTE_HANDLER) {
			return FALSE;
		}
	} else {
		return FALSE;
	}
}

/*
 * Retrieve the full pathName (including files and registry keys) for a
 * given handle. This method is from "Undocumented Windows NT".
 */
BOOLEAN FvmUtil_PathFromHandle (HANDLE Key, PUNICODE_STRING LpszSubKeyVal,
	    PWCHAR FullName) {
	PVOID	pKey = NULL;
	PWCHAR	tmpName;
	PUNICODE_STRING	fullUniName;
	ULONG	actualLen, numChar;
	NTSTATUS	rc;

	/*
	 * Allocate a temporary buffer
	 */
	tmpName = ExAllocatePoolWithTag(PagedPool, MAXPATHLEN, FVM_UTIL_POOL_TAG);
	if (tmpName == NULL)
		/*
		 * Not enough memory
		 */
		return FALSE;

	*FullName = *tmpName = L'\0';

	/*
	 * Translate the Key into a pointer to check whether it is a valid
	 * handle.
	 */
	if (NT_SUCCESS (ObReferenceObjectByHandle (Key, 0, NULL, KernelMode,
    	&pKey, NULL)) && pKey != NULL) {
		fullUniName = ExAllocatePoolWithTag (PagedPool, MAXPATHLEN +
		    sizeof(UNICODE_STRING), FVM_UTIL_POOL_TAG);
		if (fullUniName == NULL) {
			/*
			 * Not enough memory
			 */
			ObDereferenceObject (pKey);
			ExFreePool(tmpName);
			return FALSE;
		}

		fullUniName->MaximumLength = MAXPATHLEN;

		rc = ObQueryNameString (pKey, fullUniName, MAXPATHLEN, &actualLen );
		if (NT_SUCCESS (rc)&&(fullUniName->Buffer)) {
			if (*(fullUniName->Buffer) != L'\0') {
				if (*(fullUniName->Buffer) != L'\\')
					wcscpy (tmpName, L"\\");

				if (fullUniName->Length < MAXPATHLEN - 4)
					numChar = fullUniName->Length;
				else
					numChar = MAXPATHLEN - 4;

				wcsncat (tmpName, fullUniName->Buffer, numChar >> 1);
			}
		}

		ObDereferenceObject (pKey);
		ExFreePool(fullUniName);
	}

	/*
	 * Append subkey and value if they are there
	 */
	if (LpszSubKeyVal&&LpszSubKeyVal->Buffer) {
		if (*LpszSubKeyVal->Buffer != '\0') {
			wcscat (tmpName, L"\\");

			if (LpszSubKeyVal->Length < MAXPATHLEN - 2 - wcslen(tmpName)*2)
				numChar = LpszSubKeyVal->Length;
			else
				numChar = MAXPATHLEN - 2 - wcslen(tmpName)*2;

			wcsncat (tmpName, LpszSubKeyVal->Buffer, numChar >> 1);
		}
	}

	wcscpy (FullName, tmpName);
	ExFreePool(tmpName);

	return TRUE;
}

NTSTATUS
GetDriveLetterLinkTarget(IN PWCHAR SourceNameStr, OUT PWCHAR LinkTarget) {
	int	len;
	UNICODE_STRING	*driver_s;
	HANDLE	*dHandle_s;
	IO_STATUS_BLOCK    *pioStatus_s;
	OBJECT_ATTRIBUTES  *poba_s;
	NTSTATUS           rc;
	ULONG              memSize_s;
	PWCHAR             arg_s;

	memSize_s = sizeof(OBJECT_ATTRIBUTES) + sizeof(UNICODE_STRING) +
				    sizeof(HANDLE) + sizeof(IO_STATUS_BLOCK) + 512 + 4;
	poba_s = NULL;
	rc = FvmVm_AllocateVirtualMemory(NtCurrentProcess(), &poba_s, 0, &memSize_s,
		    MEM_COMMIT, PAGE_READWRITE);

	if (NT_SUCCESS(rc)) {
		(char *)driver_s = ((char *)poba_s) + sizeof(OBJECT_ATTRIBUTES);
		(char *)dHandle_s = ((char *)driver_s) + sizeof(UNICODE_STRING);
		(char *)pioStatus_s = ((char *)dHandle_s) + sizeof(HANDLE);
		(char *)arg_s = ((char *)pioStatus_s) + sizeof(IO_STATUS_BLOCK);

		wcscpy(arg_s, L"\\??\\");
		wcscat(arg_s, SourceNameStr);
		wcscat(arg_s, L"\\");

		RtlInitUnicodeString(driver_s, arg_s);
		InitializeObjectAttributes(poba_s, driver_s,
			    OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE, NULL, NULL);
		rc = winNtOpenFileProc(dHandle_s,
			    FILE_READ_DATA|FILE_LIST_DIRECTORY|SYNCHRONIZE, poba_s,
			    pioStatus_s, FILE_SHARE_READ,
			    FILE_DIRECTORY_FILE|FILE_SYNCHRONOUS_IO_NONALERT);

		if (NT_SUCCESS(rc)) {
			FvmUtil_PathFromHandle(*dHandle_s, NULL, LinkTarget);
			len = wcslen(LinkTarget)-1;
			if (LinkTarget[len] == L'\\')
				LinkTarget[len] = L'\0';

			ZwClose(*dHandle_s);
		}

        memSize_s = 0;
		FvmVm_FreeVirtualMemory(NtCurrentProcess(), &poba_s, &memSize_s, MEM_RELEASE);
		return rc;
	}
	return rc;
}

NTSTATUS
ExpandSymLink(PWCHAR Arg) {
	UINT len;
	NTSTATUS Status = STATUS_SUCCESS;
	WCHAR    buf1[_MAX_PATH]=L"";
	WCHAR    buf2[_MAX_PATH]=L"";
	WCHAR    buf3[_MAX_PATH];
	PWCHAR   ptr;

	try {
		wcsncpy(buf3,Arg,_MAX_PATH);

		len = wcslen(L"\\??\\");
		if(0 != _wcsnicmp(L"\\??\\",Arg,len)) {
			return Status;
		}
		ptr = wcschr(Arg+len, L'\\');
		if(ptr) {
			wcsncpy(buf1, Arg+len, ptr-Arg-len);
			buf1[ptr-Arg-len] = L'\0';
			wcsncpy(buf2, ptr, _MAX_PATH);
		} else {
			wcsncpy(buf1, Arg+len,_MAX_PATH);
		}

		Status = GetDriveLetterLinkTarget(buf1, Arg);
		wcscat(Arg,buf2);

		if(!NT_SUCCESS(Status)) {
			wcsncpy(Arg, buf3,_MAX_PATH);
		}
	} except(EXCEPTION_EXECUTE_HANDLER) {
		DbgPrint(("Exception?????????????????????????????????????????\n"));
		Status= -1;
	}
	return Status;
}

NTSTATUS
fvmSymbolicLinkToDeviceName(IN PWCHAR SymLinkName,
    	OUT PWCHAR DeviceName) {

	NTSTATUS status = STATUS_SUCCESS;
	WCHAR deviceNameBuf[_MAX_PATH];
	WCHAR symLinkNameBuf[_MAX_PATH];
	UNICODE_STRING deviceNameStr, symLinkNameStr;
	OBJECT_ATTRIBUTES objectAttributes;
	HANDLE linkHandle, dirHandle;
	UINT uLen;

	DeviceName[0] = 0;
	try {
		/*
		 * Open the Win32 object name-space directory - Refer WinObj.exe
		 */
		wcscpy(symLinkNameBuf, L"\\??");
		RtlInitUnicodeString(&symLinkNameStr, symLinkNameBuf);

		InitializeObjectAttributes(&objectAttributes, &symLinkNameStr,
			    OBJ_CASE_INSENSITIVE, (HANDLE)NULL, (PSECURITY_DESCRIPTOR)NULL);

		status = ZwOpenDirectoryObject(&dirHandle, DIRECTORY_QUERY,
    				&objectAttributes);
		if (!NT_SUCCESS(status)) {
			return status;
		}

		RtlInitUnicodeString(&symLinkNameStr, SymLinkName);

	/*
	 * Open symbolic link object(s)
	 */
		InitializeObjectAttributes(&objectAttributes, &symLinkNameStr,
			    OBJ_CASE_INSENSITIVE, (HANDLE)dirHandle,
			    (PSECURITY_DESCRIPTOR)NULL);

		status = ZwOpenSymbolicLinkObject(&linkHandle,
    				SYMBOLIC_LINK_QUERY, &objectAttributes);
		if (NT_SUCCESS(status)) {
			RtlZeroMemory(deviceNameBuf, sizeof(deviceNameBuf));
			deviceNameStr.Buffer = deviceNameBuf;
			deviceNameStr.MaximumLength = sizeof(deviceNameBuf);

			status = ZwQuerySymbolicLinkObject(linkHandle,
    					&deviceNameStr, NULL);
			ZwClose(linkHandle);
		}

		if (NT_SUCCESS(status)) {
			uLen = deviceNameStr.Length >> 1;
			wcsncpy(DeviceName, deviceNameStr.Buffer, uLen);
			DeviceName[uLen] = L'\0';
		}
		ZwClose(dirHandle);
	} except (EXCEPTION_EXECUTE_HANDLER) {
		return STATUS_ACCESS_VIOLATION;
	}
	return (status);
}

BOOLEAN
fvmIsCharLetter(WCHAR c) {
	return ((c > L'A' && c < L'Z') ||
    		(c > L'a' && c < L'z'));
}

BOOLEAN
FvmIsLocalFileAccess(PWCHAR FileName) {
   PWCHAR diskStr = L"\\Device\\HarddiskVolume";
   PWCHAR diskDmStr = L"\\Device\\HarddiskDmVolume";

	if (wcsncmp(FileName, L"\\??\\", 4) == 0 &&
	    fvmIsCharLetter(FileName[4]) && FileName[5] == L':') {
		return TRUE;
	} else {
		if (_wcsnicmp(FileName, diskStr, wcslen(diskStr)) == 0 ||
		    _wcsnicmp(FileName, diskDmStr, wcslen(diskDmStr)) == 0) {
			WCHAR linkName[_MAX_PATH];
			NTSTATUS rc;

		rc = FvmUtil_GetDriveFromDevice(FileName, linkName);
		if (*linkName) {
			wcscpy(FileName, linkName);
		}
		return TRUE;
		}
	}
	return FALSE;
}

NTSTATUS
FvmUtil_GetDriveFromDevice(IN PWCHAR DeviceNameStr, OUT PWCHAR DriveNameStr) {
	WCHAR drvName[4] = L"C:\0";
	WCHAR devName[_MAX_PATH];
	NTSTATUS status = STATUS_SUCCESS;

	*DriveNameStr = 0;
	while (drvName[0] <= L'Z') {
		status = fvmSymbolicLinkToDeviceName(drvName, devName);
		if (NT_SUCCESS(status)) {
			wcscat(devName, L"\\");
			if (_wcsnicmp(DeviceNameStr, devName, wcslen(devName)) == 0) {
				_snwprintf(DriveNameStr, _MAX_PATH, L"\\??\\%s\\%s", drvName,
				    DeviceNameStr + wcslen(devName));
			break;
			}
    	}
		drvName[0]++;
	}
	return status;
}

NTSTATUS
FvmUtil_InitializeVMObjAttributes(POBJECT_ATTRIBUTES ObjectAttributes,
	    PWCHAR VDirName, POBJECT_ATTRIBUTES *PobjAttr, PULONG PmemSize) {
	NTSTATUS rc;
	POBJECT_ATTRIBUTES objAttr = NULL;
	PUNICODE_STRING pathName = NULL;
	PWCHAR vdirUser = NULL;
	unsigned long timeStmp;
	static int count=0;
	static long sum=0;

	*PobjAttr = NULL;
	*PmemSize = sizeof(OBJECT_ATTRIBUTES) + sizeof(UNICODE_STRING) +
			    wcslen(VDirName)*2 + 2;

	timeStmp = (unsigned long) GetCycleCount();
	rc = FvmVm_AllocateVirtualMemory(NtCurrentProcess(), &objAttr, 0, PmemSize,
    		MEM_COMMIT, PAGE_READWRITE);
	timeStmp = (unsigned long) GetCycleCount() - timeStmp;
	count++;
	sum=sum+timeStmp;
//	DbgPrint("FvmVm_AllocateVirtualMemory in InitializeVMObjAttributes time=%ld,average=%ld\n",timeStmp,sum/count);

	if (NT_SUCCESS(rc)) {
		(char *)pathName = ((char *)objAttr) + sizeof(OBJECT_ATTRIBUTES);
		(char *)vdirUser = ((char *)pathName) + sizeof(UNICODE_STRING);

		wcsncpy(vdirUser, VDirName, _MAX_PATH);

		RtlInitUnicodeString(pathName, vdirUser);

		if (ObjectAttributes) {
			InitializeObjectAttributes(objAttr, pathName,
			    ObjectAttributes->Attributes, NULL,
			    ObjectAttributes->SecurityDescriptor);
		} else {
			InitializeObjectAttributes(objAttr, pathName,
			    OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE, NULL, NULL);
		}
		*PobjAttr = objAttr;
	} else {
		CHAR errstr[64];
		DbgPrint("ErrMem:%s\n", FvmUtil_ErrorString(rc, errstr));
	}
	return rc;
}

/*
 * Create the parent directory structures under which a new sub
 * directory or file may be created.
 * The SourName and DestName contain the full path name, but
 * the last component is not created in this function.
 */
VOID
FvmUtil_CreateFileDir(PWCHAR SourName, PWCHAR DestName, ULONG Vmid)
{
	NTSTATUS rc;
	WCHAR sourceName[_MAX_PATH+1], destnaName[_MAX_PATH+1];
	PWCHAR rootPtr, dirPtr;
	PWCHAR rootPtr_s, dirPtr_s;
	PWCHAR vdirUser;
	PUNICODE_STRING pathName = NULL;
	POBJECT_ATTRIBUTES poa = NULL;
	ULONG memSize;
	PHANDLE pvFile = NULL;
	PIO_STATUS_BLOCK pioStatus = NULL;
	PFILE_BASIC_INFORMATION pfileBasicInfo = NULL;
	PWCHAR pathMaskBuf = NULL;
	PUNICODE_STRING pathMask = NULL;

	wcsncpy(sourceName, SourName, _MAX_PATH);
	wcsncpy(destnaName, DestName, _MAX_PATH);


	rootPtr = wcsstr(destnaName, pvms[Vmid]->idStr);
	if (!rootPtr) return;
	rootPtr = rootPtr + wcslen(pvms[Vmid]->idStr) + 1;

	/*
	 * break the path of the target directory to be created
	 */
	dirPtr = wcschr(rootPtr, L'\\');
	if (dirPtr) *dirPtr = 0;

	/*
	 * break the path of the source directory
	 */
	rootPtr_s = sourceName;
	dirPtr_s = wcschr(rootPtr_s, L':');
	if (dirPtr_s) dirPtr_s++;
	if (dirPtr_s && *dirPtr_s == L'\\')
		*dirPtr_s = 0;

	/*
	 * Allocate virtual memory buffer
	 */
	memSize = sizeof(OBJECT_ATTRIBUTES) + sizeof(UNICODE_STRING)*2 + _MAX_PATH*4
			    + sizeof(HANDLE) + sizeof(IO_STATUS_BLOCK) +
			    sizeof(FILE_BASIC_INFORMATION);
	rc = FvmVm_AllocateVirtualMemory(NtCurrentProcess(), &poa, 0, &memSize,
		    MEM_COMMIT, PAGE_READWRITE);

	if (NT_SUCCESS(rc)) {
		(char *)pathName = ((char *)poa) + sizeof(OBJECT_ATTRIBUTES);
		(char *)pathMask = ((char *)pathName) + sizeof(UNICODE_STRING);
		(char *)vdirUser = (char *)pathMask + sizeof(UNICODE_STRING);
		(char *)pathMaskBuf = (char *)vdirUser + _MAX_PATH*2;
		(char *)pvFile = (char *)pathMaskBuf + _MAX_PATH*2;
		(char *)pioStatus = ((char *)pvFile) + sizeof(HANDLE);
		(char *)pfileBasicInfo = ((char *)pioStatus) + sizeof(IO_STATUS_BLOCK);
		(char *)pathMask = (char *)pfileBasicInfo + sizeof(FILE_BASIC_INFORMATION);

		wcsncpy(vdirUser, destnaName, _MAX_PATH);
		RtlInitUnicodeString(pathName, vdirUser);
		InitializeObjectAttributes(poa, pathName,
			    OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE, NULL, NULL);
	} else {
		return;
	}

#ifdef USE_FS_MAP
	if (!FvmTable_FVMFileListLookup(sourceName+4, Vmid)) {
#else
	/*
	 * Check if the FVM drive directory already exists, e.g. \??\C:\FVM\ID\C
	 */
	rc = ((NtQueryAttributesFileProc)(winNtQueryAttributesFileProc)) (poa,
    		pfileBasicInfo);

	if (rc == STATUS_OBJECT_NAME_NOT_FOUND ||
	    rc == STATUS_OBJECT_PATH_NOT_FOUND) {
#endif
		/*
		 * create the FVM "drive" directory
		 */
		rc = ((NtCreateFileProc)(winNtCreateFileProc))(pvFile,
			    FILE_LIST_DIRECTORY|SYNCHRONIZE, poa, pioStatus, NULL,
    			FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ, FILE_CREATE,
    			FILE_DIRECTORY_FILE|FILE_SYNCHRONOUS_IO_NONALERT,
     			NULL, 0);

		if (!NT_SUCCESS(rc))
			goto Cleanup;
		ZwClose(*pvFile);

#ifdef USE_FS_MAP
		FvmTable_FVMFileListAddFullPath(sourceName+4, Vmid);
#endif
	}

	if (!dirPtr)
		/*
		 * No further child directories to be created
		 */
		goto Cleanup;

	*dirPtr = '\\';
	rootPtr = dirPtr + 1;

	/*
	 * Start a loop to create the target directory
	 */
	while(*rootPtr) {

		dirPtr = wcschr(rootPtr, L'\\');
		/*
		 * directory
		 */
		if(dirPtr)
			*dirPtr = 0;
		else
			break;

		wcsncpy(vdirUser, destnaName, _MAX_PATH);
		RtlInitUnicodeString(pathName, vdirUser);
		InitializeObjectAttributes(poa, pathName,
		    OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE, NULL, NULL);

		/*
		 * check if the directory already exists
		 */
		rc = ((NtQueryAttributesFileProc)(winNtQueryAttributesFileProc)) (
    			poa, pfileBasicInfo );

		/*
		 * Continue with the loop if the directory already exists
		 */
		if (NT_SUCCESS(rc)||(rc != STATUS_OBJECT_PATH_NOT_FOUND &&
		    rc != STATUS_OBJECT_NAME_NOT_FOUND)) {
			*dirPtr = '\\';
			rootPtr = dirPtr+1;

			*dirPtr_s = '\\';
			rootPtr_s = dirPtr_s+1;
			dirPtr_s = wcschr(rootPtr_s, L'\\');
			if (dirPtr_s)
				*dirPtr_s = 0;

			continue;
		}

		/*
		 * Create the new directory
		 */
		rc = ((NtCreateFileProc)(winNtCreateFileProc))(pvFile,
    			FILE_WRITE_ATTRIBUTES|FILE_LIST_DIRECTORY|SYNCHRONIZE,
    			poa, pioStatus, NULL, FILE_ATTRIBUTE_NORMAL,
    			FILE_SHARE_READ, FILE_CREATE,
    			FILE_DIRECTORY_FILE|FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
		try {
			*dirPtr_s = '\\';
			rootPtr_s = dirPtr_s+1;
			dirPtr_s = wcschr(rootPtr_s, L'\\');
			if (dirPtr_s)
				*dirPtr_s = 0;
		} except (EXCEPTION_EXECUTE_HANDLER) {
			dirPtr_s = NULL;
		}

		if (NT_SUCCESS(rc)) {

#ifdef USE_FS_MAP
			FvmTable_FVMFileListAddFullPath(sourceName+4, Vmid);
#endif
			/*
			 * Set the directory's attributes
			 */
			wcsncpy(vdirUser, sourceName, _MAX_PATH);
			RtlInitUnicodeString(pathName, vdirUser);
			InitializeObjectAttributes(poa, pathName, OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE, NULL, NULL);

			rc = ((NtQueryAttributesFileProc)(winNtQueryAttributesFileProc))(poa,
				    pfileBasicInfo);

			if (NT_SUCCESS(rc)) {
				((NtSetInformationFileProc)(winNtSetInformationFileProc))(*pvFile,
    					pioStatus, pfileBasicInfo,
    					sizeof(FILE_BASIC_INFORMATION), FileBasicInformation);
			}
			ZwClose(*pvFile);
		} else {
			goto Cleanup;
		}
		*dirPtr = '\\';
		rootPtr = dirPtr+1;
	}
Cleanup:
	if (poa) {
	    memSize = 0;
		FvmVm_FreeVirtualMemory(NtCurrentProcess(), &poa, &memSize, MEM_RELEASE);
	}
}
/*
 * Copy a file or a directory to a virtual environment
 */
NTSTATUS
FvmUtil_CopyFiletoVM(IN POBJECT_ATTRIBUTES ObjectAttributes,
	    IN PWCHAR SourName, IN PWCHAR DestName, IN BOOLEAN IsDir,
	    IN BOOLEAN OpenCall, ULONG Vmid) {

	int nLen;
	NTSTATUS opRead = STATUS_SUCCESS, opWrite = STATUS_SUCCESS;
	NTSTATUS rc = STATUS_SUCCESS;
	IO_STATUS_BLOCK ioStatus;
	CHAR fileBuf[2048];
	FILE_BASIC_INFORMATION fileBasicInfo;
	ULONG memSize, memSize1;
	POBJECT_ATTRIBUTES poa = NULL;
	PHANDLE pvsFile = NULL, pvtFile = NULL;
	PIO_STATUS_BLOCK pioStatus = NULL;

	if (IsDir && OpenCall) {
		nLen = wcslen(DestName);
		if (DestName[nLen-1] != L'\\') {
			wcscat(DestName, L"\\");
		}
		FvmUtil_CreateFileDir(SourName, DestName, Vmid);

		DestName[nLen] = 0;
		return STATUS_SUCCESS;
	}

	/*
	 * copy the file path
	 */
	FvmUtil_CreateFileDir(SourName, DestName, Vmid);
	if (IsDir) {
		return STATUS_SUCCESS;
	}

	memSize1 = sizeof(HANDLE)*2 + sizeof(IO_STATUS_BLOCK);
	rc = FvmVm_AllocateVirtualMemory(NtCurrentProcess(), &(char *)pvsFile, 0, &memSize1,
    		MEM_COMMIT, PAGE_READWRITE);

	if (NT_SUCCESS(rc)) {
		(char *)pvtFile = ((char *)pvsFile) + sizeof(HANDLE);
		(char *)pioStatus = ((char *)pvtFile) + sizeof(HANDLE);
	} else {
		CHAR errstr[64];
		DbgPrint("ErrMem:%s\n", FvmUtil_ErrorString(rc, errstr));
		return STATUS_ACCESS_DENIED;
	}

	/*
 	 * copy the disk file
 	 */
	rc = ((NtOpenFileProc)(winNtOpenFileProc))(pvsFile, GENERIC_READ|SYNCHRONIZE,
    		ObjectAttributes, pioStatus, FILE_SHARE_READ,
    		FILE_SYNCHRONOUS_IO_NONALERT);

	if (!NT_SUCCESS(rc)) {
	    memSize1 = 0;
		FvmVm_FreeVirtualMemory(NtCurrentProcess(), &(char *)pvsFile,
		    &memSize1, MEM_RELEASE);
		return rc;
	}

	/*
 	 * copy the file content
 	 */
	rc = FvmUtil_InitializeVMObjAttributes(ObjectAttributes, DestName, &poa, &memSize);
	if (!NT_SUCCESS(rc)) {
		ZwClose(*pvsFile);

		memSize1 = 0;
		FvmVm_FreeVirtualMemory(NtCurrentProcess(), &(char *)pvsFile, &memSize1,
		    MEM_RELEASE);
		return STATUS_ACCESS_DENIED;
   }

	rc = ((NtCreateFileProc)(winNtCreateFileProc))(pvtFile,
		    GENERIC_READ|GENERIC_WRITE|SYNCHRONIZE, poa, pioStatus, NULL,
			FILE_ATTRIBUTE_NORMAL, 0, FILE_OVERWRITE_IF,
			FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

    memSize = 0;
	FvmVm_FreeVirtualMemory(NtCurrentProcess(), &poa, &memSize, MEM_RELEASE);

	if (NT_SUCCESS(rc)) {
		while (NT_SUCCESS(opRead) && NT_SUCCESS(opWrite)) {
			ioStatus.Information = 0;
			opRead = ZwReadFile(*pvsFile, NULL, NULL, NULL, &ioStatus, fileBuf, 2048, NULL, NULL);
			nLen = ioStatus.Information;
			opWrite = ZwWriteFile(*pvtFile, NULL, NULL, NULL, &ioStatus, fileBuf, nLen, NULL, NULL);
		}

		/*
		 * Copy the basic file attributes
		 */
		opRead = ZwQueryInformationFile(*pvsFile, &ioStatus, &fileBasicInfo,
					sizeof(FILE_BASIC_INFORMATION), FileBasicInformation);
		opWrite = ZwSetInformationFile(*pvtFile, &ioStatus, &fileBasicInfo,
					sizeof(FILE_BASIC_INFORMATION), FileBasicInformation);

		ZwClose(*pvtFile);

#ifdef USE_FS_MAP
		FvmTable_FVMFileListAddFullPath(SourName+4, Vmid);
#endif
	}

	ZwClose(*pvsFile);
	memSize1 = 0;
	FvmVm_FreeVirtualMemory(NtCurrentProcess(), &(char *)pvsFile, &memSize1, MEM_RELEASE);
	return rc;
}

/*
 * Returns the string representation of the passed error condition.
 */
PCHAR
FvmUtil_ErrorString(NTSTATUS RetStat, PCHAR Buffer) {
	switch(RetStat) {
	case STATUS_SUCCESS:
		strcpy(Buffer, "SUCCESS");
		break;
	case STATUS_ACCESS_VIOLATION:
		strcpy(Buffer, "ACCESS VIOLATION");
		break;
	case STATUS_CRC_ERROR:
		strcpy(Buffer, "CRC ERROR");
		break;
	case STATUS_NOT_IMPLEMENTED:
		strcpy(Buffer, "NOT IMPLEMENTED");
		break;
	case STATUS_EAS_NOT_SUPPORTED:
		strcpy(Buffer, "EAS NOT SUPPORTED");
		break;
	case STATUS_EA_TOO_LARGE:
		strcpy(Buffer, "EA TOO LARGE");
		break;
	case STATUS_NONEXISTENT_EA_ENTRY:
		strcpy(Buffer, "NONEXISTENT EA ENTRY");
		break;
	case STATUS_BAD_NETWORK_NAME:
		strcpy(Buffer, "BAD NETWORK NAME");
	    break;
	case STATUS_NOTIFY_ENUM_DIR:
		strcpy(Buffer, "NOTIFY ENUM DIR");
	    break;
	case STATUS_FILE_CORRUPT_ERROR:
		strcpy(Buffer, "FILE CORRUPT");
	    break;
	case STATUS_DISK_CORRUPT_ERROR:
		strcpy(Buffer, "DISK CORRUPT");
	    break;
	case STATUS_RANGE_NOT_LOCKED:
		strcpy(Buffer, "RANGE NOT LOCKED");
	    break;
	case STATUS_FILE_CLOSED:
		strcpy(Buffer, "FILE CLOSED" );
	    break;
	case STATUS_IN_PAGE_ERROR:
		strcpy(Buffer, "IN PAGE ERROR" );
	    break;
	case STATUS_CANCELLED:
		strcpy(Buffer, "CANCELLED" );
	    break;
	case STATUS_QUOTA_EXCEEDED:
		strcpy(Buffer, "QUOTA EXCEEDED" );
	    break;
	case STATUS_NOT_SUPPORTED:
		strcpy(Buffer, "NOT SUPPORTED" );
	    break;
	case STATUS_NO_MORE_FILES:
		strcpy(Buffer, "NO MORE FILES" );
	    break;
	case STATUS_BUFFER_TOO_SMALL:
		strcpy(Buffer, "BUFFER TOO SMALL" );
	    break;
	case STATUS_OBJECT_NAME_INVALID:
		strcpy(Buffer, "NAME INVALID" );
	    break;
	case STATUS_OBJECT_NAME_NOT_FOUND:
		strcpy(Buffer, "FILE NOT FOUND" );
	    break;
	case STATUS_NOT_A_DIRECTORY:
		strcpy(Buffer, "NOT A DIRECTORY" );
	    break;
	case STATUS_NO_SUCH_FILE:
		strcpy(Buffer, "NO SUCH FILE" );
	    break;
	case STATUS_OBJECT_NAME_COLLISION:
		strcpy(Buffer, "NAME COLLISION" );
	    break;
	case STATUS_NONEXISTENT_SECTOR:
		strcpy(Buffer, "NONEXISTENT SECTOR" );
		break;
	case STATUS_BAD_NETWORK_PATH:
		strcpy(Buffer, "BAD NETWORK PATH" );
		break;
	case STATUS_OBJECT_PATH_NOT_FOUND:
		strcpy(Buffer, "PATH NOT FOUND" );
		break;
	case STATUS_NO_SUCH_DEVICE:
		strcpy(Buffer, "INVALID PARAMETER" );
		break;
	case STATUS_END_OF_FILE:
		strcpy(Buffer, "END OF FILE" );
		break;
	case STATUS_NOTIFY_CLEANUP:
		strcpy(Buffer, "NOTIFY CLEANUP" );
		break;
	case STATUS_BUFFER_OVERFLOW:
		strcpy(Buffer, "BUFFER OVERFLOW" );
		break;
	case STATUS_NO_MORE_ENTRIES:
		strcpy(Buffer, "NO MORE ENTRIES" );
		break;
	case STATUS_ACCESS_DENIED:
		strcpy(Buffer, "ACCESS DENIED" );
		break;
	case STATUS_SHARING_VIOLATION:
		strcpy(Buffer, "SHARING VIOLATION" );
		break;
	case STATUS_INVALID_PARAMETER:
		strcpy(Buffer, "INVALID PARAMETER" );
		break;
	case STATUS_OPLOCK_BREAK_IN_PROGRESS:
		strcpy(Buffer, "OPLOCK BREAK" );
		break;
	case STATUS_OPLOCK_NOT_GRANTED:
		strcpy(Buffer, "OPLOCK NOT GRANTED" );
		break;
	case STATUS_FILE_LOCK_CONFLICT:
		strcpy(Buffer, "FILE LOCK CONFLICT" );
		break;
	case STATUS_PENDING:
		strcpy(Buffer, "PENDING" );
		break;
	case STATUS_REPARSE:
		strcpy(Buffer, "REPARSE" );
		break;
	case STATUS_MORE_ENTRIES:
		strcpy(Buffer, "MORE" );
		break;
	case STATUS_DELETE_PENDING:
		strcpy(Buffer, "DELETE PEND" );
		break;
	case STATUS_CANNOT_DELETE:
		strcpy(Buffer, "CANNOT DELETE" );
		break;
	case STATUS_LOCK_NOT_GRANTED:
		strcpy(Buffer, "NOT GRANTED" );
		break;
	case STATUS_FILE_IS_A_DIRECTORY:
		strcpy(Buffer, "IS DIRECTORY" );
		break;
	case STATUS_ALREADY_COMMITTED:
		strcpy(Buffer, "ALREADY COMMITTED" );
		break;
	case STATUS_INVALID_EA_FLAG:
		strcpy(Buffer, "INVALID EA FLAG" );
		break;
	case STATUS_INVALID_INFO_CLASS:
		strcpy(Buffer, "INVALID INFO CLASS" );
		break;
	case STATUS_INVALID_HANDLE:
		strcpy(Buffer, "INVALID HANDLE" );
		break;
	case STATUS_INVALID_DEVICE_REQUEST:
		strcpy(Buffer, "INVALID DEVICE REQUEST" );
		break;
	case STATUS_WRONG_VOLUME:
		strcpy(Buffer, "WRONG VOLUME" );
		break;
	case STATUS_UNEXPECTED_NETWORK_ERROR:
		strcpy(Buffer, "NETWORK ERROR" );
		break;
	case STATUS_DFS_UNAVAILABLE:
		strcpy(Buffer, "DFS UNAVAILABLE" );
		break;
	case STATUS_LOG_FILE_FULL:
		strcpy(Buffer, "LOG FILE FULL" );
		break;
	case STATUS_INVALID_DEVICE_STATE:
		strcpy(Buffer, "INVALID DEVICE STATE" );
		break;
	case STATUS_NO_MEDIA_IN_DEVICE:
		strcpy(Buffer, "NO MEDIA");
		break;
	case STATUS_DISK_FULL:
		strcpy(Buffer, "DISK FULL");
		break;
	case STATUS_DIRECTORY_NOT_EMPTY:
		strcpy(Buffer, "NOT EMPTY");
		break;

	//
	// Named pipe errors
	//
	case STATUS_INSTANCE_NOT_AVAILABLE:
		strcpy(Buffer, "INSTANCE NOT AVAILABLE" );
		break;
	case STATUS_PIPE_NOT_AVAILABLE:
		strcpy(Buffer, "PIPE NOT AVAILABLE" );
		break;
	case STATUS_INVALID_PIPE_STATE:
		strcpy(Buffer, "INVALID PIPE STATE" );
		break;
	case STATUS_PIPE_BUSY:
		strcpy(Buffer, "PIPE BUSY" );
		break;
	case STATUS_PIPE_DISCONNECTED:
		strcpy(Buffer, "PIPE DISCONNECTED" );
		break;
	case STATUS_PIPE_CLOSING:
		strcpy(Buffer, "PIPE CLOSING" );
		break;
	case STATUS_PIPE_CONNECTED:
		strcpy(Buffer, "PIPE CONNECTED" );
		break;
	case STATUS_PIPE_LISTENING:
		strcpy(Buffer, "PIPE LISTENING" );
		break;
	case STATUS_INVALID_READ_MODE:
		strcpy(Buffer, "INVALID READ MODE" );
		break;
	case STATUS_PIPE_EMPTY:
		strcpy(Buffer, "PIPE EMPTY" );
		break;
	case STATUS_PIPE_BROKEN:
		strcpy(Buffer, "PIPE BROKEN" );
		break;
	case STATUS_IO_TIMEOUT:
		strcpy(Buffer, "IO TIMEOUT" );
		break;
	default:
		sprintf( Buffer, "* 0x%X", RetStat );
		break;
	}
	return Buffer;
}


/*
 * This function converts an SID to a string.
 */
NTSTATUS
FvmUtil_FindProcessStrSID(PWCHAR SidBuffer, PULONG SidBufferLength) {
	UCHAR       buffer[256];
	PISID       sid = (PISID)&buffer[sizeof(TOKEN_USER)];
	NTSTATUS    status;
	HANDLE      handle;
	ULONG       tokenInfoLength;
	LONG        length;
	int dwSubAuthorities;
	int dwSidRev;
	int dwCounter;
	UINT dwSidSize;
	/*
	 * sanity check
	 */
	ASSERT(SidBuffer);
	ASSERT(SidBufferLength);

	if (*SidBufferLength == 0) {
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	/*
	 * open the thread token
	 */
	status = ZwOpenThreadToken(NtCurrentThread(), TOKEN_READ, TRUE, &handle);
	if (status == STATUS_NO_TOKEN) {
		/*
		 * No thread level token, so use the process
		 * level token.  This is the common case since the only
		 * time a thread has a token is when it is impersonating.
		 */
		status = ZwOpenProcessToken(NtCurrentProcess(), TOKEN_READ, &handle);
	}

	/*
	 * This should have succeeded.  In this example, we
	 * crash if it didn't work.
	 */

	if (!NT_SUCCESS(status)) {
		return status;
	}

	/*
	 * Retrieve the user information from the token.
	 */

	status = ZwQueryInformationToken(handle, TokenUser, buffer,
    			sizeof(buffer), &tokenInfoLength);

	/*
	 * This call should always work.
	 */
	if (!NT_SUCCESS(status)) {
		DbgPrint("ZwQueryInformationToken failure - status %x\n",status);
		return status;
	}

	length = tokenInfoLength - sizeof(TOKEN_USER);
	ASSERT(length > 0);
	if ((ULONG)length > *SidBufferLength) {
		DbgPrint("SidBufferLength too small - expected %d. got %d.\n", length,
    			*SidBufferLength);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	/*
	 * Get the number of subauthorities in the SID.
	 */

	dwSubAuthorities = sid->SubAuthorityCount;

	dwSidSize=(15 + 12 + (12 * dwSubAuthorities) + 1) * sizeof(TCHAR);

	/*
	 * Check input buffer length.
	 * If too small, indicate the proper size and set last error.
	 */
	if (*SidBufferLength < dwSidSize) {
		*SidBufferLength = dwSidSize;
		return FALSE;
	}

	/*
	 * Add 'S' prefix and revision number to the string.
	 */

	dwSidSize=swprintf(SidBuffer, L"S-%lu-", sid->Revision);

	/*
	 * Add SID identifier authority to the string.
	 */

	if ((sid->IdentifierAuthority.Value[0] != 0) ||
	    (sid->IdentifierAuthority.Value[1] != 0)) {
		dwSidSize+=swprintf(SidBuffer + wcslen(SidBuffer),
    				L"0x%02hx%02hx%02hx%02hx%02hx%02hx",
    				(USHORT)sid->IdentifierAuthority.Value[0],
    				(USHORT)sid->IdentifierAuthority.Value[1],
    				(USHORT)sid->IdentifierAuthority.Value[2],
    				(USHORT)sid->IdentifierAuthority.Value[3],
    				(USHORT)sid->IdentifierAuthority.Value[4],
    				(USHORT)sid->IdentifierAuthority.Value[5]);
	} else {
		dwSidSize+=swprintf(SidBuffer + wcslen(SidBuffer),
    				L"%lu", (ULONG)(sid->IdentifierAuthority.Value[5]) +
    				(ULONG)(sid->IdentifierAuthority.Value[4] <<  8) +
    				(ULONG)(sid->IdentifierAuthority.Value[3] << 16) +
    				(ULONG)(sid->IdentifierAuthority.Value[2] << 24));
	}

	/*
	 * Add SID subauthorities to the string.
	 */
	for (dwCounter=0 ; dwCounter < dwSubAuthorities ; dwCounter++) {
		dwSidSize+=swprintf(SidBuffer + dwSidSize, L"-%lu",
		sid->SubAuthority[dwCounter]);
	}
	return STATUS_SUCCESS;
}

/*
 * Return the string of desired access to a file
 */
PCHAR
FvmUtil_AccessString(ACCESS_MASK DesiredAccess, PCHAR Access_str) {
	int msLen = _MAX_PATH;

	*Access_str = 0;
	if(DesiredAccess & GENERIC_WRITE)
		strncat(Access_str, "GENERIC_WRITE,", msLen);
	msLen -= strlen("GENERIC_WRITE,");

	if(DesiredAccess & STANDARD_RIGHTS_WRITE)
		strncat(Access_str, "STANDARD_RIGHTS_WRITE,", msLen);
	msLen -= strlen("STANDARD_RIGHTS_WRITE,");

	if(DesiredAccess & FILE_WRITE_DATA)
		strncat(Access_str, "FILE_WRITE_DATA,", msLen);
	msLen -= strlen("FILE_WRITE_DATA,");

	if(DesiredAccess & FILE_WRITE_ATTRIBUTES)
		strncat(Access_str, "FILE_WRITE_ATTRIBUTES,", msLen);
	msLen -= strlen("FILE_WRITE_ATTRIBUTES,");

	if(DesiredAccess & FILE_WRITE_EA)
		strncat(Access_str, "FILE_WRITE_EA,", msLen);
	msLen -= strlen("FILE_WRITE_EA,");

	if(DesiredAccess & FILE_APPEND_DATA)
		strncat(Access_str, "FILE_APPEND_DATA,", msLen);
	msLen -= strlen("FILE_APPEND_DATA,");

	if(DesiredAccess & DELETE)
		strncat(Access_str, "DELETE,", msLen);
	msLen -= strlen("DELETE,");

	if(DesiredAccess & WRITE_DAC)
		strncat(Access_str, "WRITE_DAC,", msLen);
	msLen -= strlen("WRITE_DAC,");

	if(DesiredAccess & WRITE_OWNER)
		strncat(Access_str, "WRITE_OWNER,", msLen);
	msLen -= strlen("WRITE_OWNER,");

	//////////////////////////////////////////////////////
	if(DesiredAccess & FILE_READ_DATA)
		strncat(Access_str, "FILE_READ_DATA,", msLen);
	msLen -= strlen("FILE_READ_DATA,");

	if(DesiredAccess & FILE_READ_ATTRIBUTES)
		strncat(Access_str, "FILE_READ_ATTRIBUTES,", msLen);
	msLen -= strlen("FILE_READ_ATTRIBUTES,");

	if(DesiredAccess & FILE_READ_EA)
		strncat(Access_str, "FILE_READ_EA,", msLen);
	msLen -= strlen("FILE_READ_EA,");

	if(DesiredAccess & READ_CONTROL)
		strncat(Access_str, "READ_CONTROL,", msLen);
	msLen -= strlen("READ_CONTROL,");

	if(DesiredAccess & SYNCHRONIZE)
		strncat(Access_str, "SYNCHRONIZE,", msLen);
	msLen -= strlen("SYNCHRONIZE,");

	if(DesiredAccess & FILE_EXECUTE)
		strncat(Access_str, "FILE_EXECUTE,", msLen);
	msLen -= strlen("FILE_EXECUTE,");

	if(DesiredAccess & FILE_LIST_DIRECTORY)
		strncat(Access_str, "FILE_LIST_DIRECTORY,", msLen);
	msLen -= strlen("FILE_LIST_DIRECTORY,");

	if(DesiredAccess & FILE_TRAVERSE)
		strncat(Access_str, "FILE_TRAVERSE,", msLen);
	msLen -= strlen("FILE_TRAVERSE,");
	//////////////////////////////////////////////////////
	if (*Access_str) {
		msLen = strlen(Access_str);
		Access_str[msLen-1] = 0;
	}
	return Access_str;
}
unsigned __int64 GetCycleCount(void)
{
	   __asm _emit 0x0F;
	   __asm _emit 0x31;
}
