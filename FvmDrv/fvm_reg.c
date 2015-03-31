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
#include "fvm_util.h"
#include "fvm_table.h"
#include "hooksys.h"
#include "fvm_vm.h"
#include "fvm_syscalls.h"
#include "fvm_reg.h"

#define FVM_REG_POOL_TAG '5GAT'

//======================================================================
//  R E G I S T R Y  P A R A M E T E R  S U P P O R T  R O U T I N E S
//======================================================================

/*
 * GetPointer
 *
 * Translates a handle to an object pointer.
 */
static POBJECT
GetPointer(HANDLE Handle) {
	POBJECT pKey = NULL;

	/*
	 * Ignore null handles.
	 */
	if (!Handle) return NULL;

	/*
	 * Make sure that we're not going to access
	 * the kernel handle table from a non-system process
	 */
	if ((LONG)(ULONG_PTR)Handle < 0 &&
	    ExGetPreviousMode() != KernelMode) {
		return NULL;
	}

	/*
	 * Get the pointer the handle refers to.
	 */
	ObReferenceObjectByHandle( Handle,
	    0, NULL, KernelMode, &pKey, NULL);
	return pKey;
}


/*
 * Dereference the object.
 */
static VOID
ReleasePointer(POBJECT Object) {
	if (Object) ObDereferenceObject(Object);
}


/*
 * this function return the full path of the virtual and host registry
 */
static int
GetFullRegName(
	HANDLE KeyHandle,
	PUNICODE_STRING LpszSubKeyVal,
	PWCHAR OrigName,
	PWCHAR VirName,
	PWCHAR TempBuffer,
	int Vmn) {

	POBJECT	pKey = NULL;
	PUNICODE_STRING	fullUniName = (PUNICODE_STRING)TempBuffer;

	PWCHAR	wPtr;
	ULONG	actualLen;
	ULONG	_length = 256;
	PISID	sid;
	NTSTATUS	rc;

	int	initial = 0;

	/*
	 * Is it a valid handle ?
	 */
	if (pKey = GetPointer(KeyHandle)) {
		if (pKey) {
			fullUniName->MaximumLength = MAXPATHLEN * sizeof(WCHAR);

			if (NT_SUCCESS(ObQueryNameString(pKey, fullUniName,
			    MAXPATHLEN, &actualLen))) {
				wcscat(OrigName, fullUniName->Buffer);
			}
			ReleasePointer(pKey);
		}
	}

	try {
		if (LpszSubKeyVal) {

			if (LpszSubKeyVal->Buffer[0] != L'\\')
				wcscat(OrigName, L"\\");
			wcsncat(OrigName, LpszSubKeyVal->Buffer, LpszSubKeyVal->Length/2);

			/*
			 * if a registry name contains vm strid, then it is a path in vm
			 */
			if (wcsstr(OrigName, pvms[Vmn]->idStr)) {

				wcsncpy(VirName, OrigName, 2048);

				wPtr = wcsstr(VirName, pvms[Vmn]->idStr);


				wPtr = wcschr(wPtr, L'\\');
				/*
				 * The original location.
				 */
				wcsncpy(OrigName, wPtr, 2048);

				initial = 1;
			} else {
				_length = 1024;

				wcscpy(TempBuffer, pvms[Vmn]->SidStr);

				swprintf(VirName, L"\\REGISTRY\\USER\\%s\\fvms\\%s%s",
					TempBuffer, pvms[Vmn]->idStr, OrigName);
			}

		}
	}except (EXCEPTION_EXECUTE_HANDLER) {
		DbgPrint(("Exception occured in GetRegFullName\n"));
		return initial;
	}
	return initial;
}


/*
 * FVM REGISTRY HOOK ROUTINES..
 */

PWCHAR sees_value_name = L"SEES_INITIAL";

#define SEES_VALUE_NAME L"SEES_INITIAL"
#define SEES_VALUE_NAME_LEN 12
#define SEES_VALUE_NAME_LEN_A 32


NTSTATUS
FvmReg_NtOpenKey
(
	OUT PHANDLE KeyHandle,
	IN ACCESS_MASK ReqAccess,
	IN POBJECT_ATTRIBUTES ObjectAttributes
) {
	NTSTATUS	ntStatus;
	HANDLE	procHandle;
	NTSTATUS	rc;
	int	size;
	POBJECT_ATTRIBUTES	pObjectAttributes=NULL;
	PUNICODE_STRING	pObjectName;
	PKEY_VALUE_FULL_INFORMATION	pValueFullInfo;
	PHANDLE	pTempHandle;
	PHANDLE	pTempHandle1;
	PULONG	pTempSize;
	PWCHAR	pVirName;
	PWCHAR	pOriName;
	PWCHAR	pTempBuffer;
	PKEY_BASIC_INFORMATION	pKeyBasicInfo;
	int	vmn;
	ULONG	TitleIndex = 0;
	PUNICODE_STRING	Class = NULL;
	PULONG	Disposition = NULL;
	ULONG	createOptions = REG_OPTION_NON_VOLATILE;
	ULONG	working;
	PWCHAR	pPointer;
	WCHAR	cSave;
	ULONG	virNameLen;
	PULONG	sv;
	PWCHAR	rp;
	
	InterlockedIncrement(&fvm_Calls_In_Progress);
	try {
		/*
		 * Get the virtual machine number
		 */
		vmn = FvmVm_GetPVMId((ULONG) PsGetCurrentProcessId());

		if (vmn == INVALID_VMID) {
			/*
			 * The corruent process is not under a VM
			 */
			rc = winNtOpenKeyProc( KeyHandle, ReqAccess, ObjectAttributes);
			goto NtExit;
		}

		/*
		 * Allocate virtual memory from the process user space
		 */
		size = 16384+1024*1024; /* 4 pages + 1MB */
		procHandle = NtCurrentProcess();

		pObjectAttributes = NULL;
		rc = FvmVm_AllocateVirtualMemory(procHandle, &pObjectAttributes, 0, &size,
			    MEM_COMMIT, PAGE_EXECUTE_READWRITE);

		if (rc != STATUS_SUCCESS) {
			DbgPrint("--memory allocation problem(%d,NtOpenKey)\n",(ULONG)PsGetCurrentProcessId());
			goto NtExit;
		}

		(char *)pObjectName = ((char *)pObjectAttributes ) +
			    sizeof(OBJECT_ATTRIBUTES);
		(char *)pValueFullInfo = ((char *)pObjectName) +
		    sizeof(UNICODE_STRING);
		(char *)pTempHandle = ((char *)pValueFullInfo) + 4096+1024*1024;
		(char *)pTempHandle1 = ((char *)pTempHandle) + sizeof(HANDLE);
		(char *)pTempSize = ((char *)pTempHandle1) + sizeof(HANDLE);
		(char *)pOriName = ((char *)pTempSize) + sizeof(ULONG);
		(char *)pVirName = ((char *)pOriName) + 4096;
		(char *)pTempBuffer = ((char *)pVirName) + 4096;

		/*
		 * Get the orignal and the virtual name of the registry key
		 */
		pOriName[0] = L'\0';
		pVirName[0] = L'\0';

		GetFullRegName( ObjectAttributes->RootDirectory,
		    ObjectAttributes->ObjectName, pOriName, pVirName,
		    pTempBuffer, vmn);

		rp = wcschr(pOriName, L'\\');
		if (rp) {
			rp++;
			rp = wcschr(rp, L'\\');
		}
		if (rp) {
		  rp++;
		  rp = wcschr(rp, L'\\');
		}
		if (rp) {
		  rp++;
		  rp = wcschr(rp, L'\\');
		}

		if (rp && _wcsnicmp(rp,
		    L"\\Software\\Microsoft\\SystemCertificates\\root\\ProtectedRoots",
		    58) == 0) {

			pObjectName->MaximumLength = 2048;
			pObjectName->Length = wcslen(pOriName)*2;
			pObjectName->Buffer = pOriName;

			InitializeObjectAttributes(pObjectAttributes, pObjectName,
				    OBJ_CASE_INSENSITIVE, NULL, NULL );

			rc = winNtOpenKeyProc(KeyHandle, ReqAccess,  pObjectAttributes);

			size = 0;
			FvmVm_FreeVirtualMemory(procHandle, &pObjectAttributes, &size,
				    MEM_RELEASE);
			goto NtExit;
		}

		/*
		 * Try to open the key from the virtual machine first
		 */
		pObjectName->Length = wcslen(pVirName)*2;
		pObjectName->Buffer = pVirName;
		InitializeObjectAttributes(pObjectAttributes, pObjectName,
			    OBJ_CASE_INSENSITIVE, NULL, NULL );

		ntStatus = winNtOpenKeyProc(KeyHandle, KEY_ALL_ACCESS,
			    pObjectAttributes);

		/*
		 * Key is in VM
		 */
	    if (ntStatus == STATUS_SUCCESS) {
			wcscpy((PWCHAR) pTempBuffer, SEES_VALUE_NAME);
			pObjectName->Buffer = pTempBuffer;
			pObjectName->Length = (USHORT) SEES_VALUE_NAME_LEN*2;
			pObjectName->MaximumLength = (USHORT) SEES_VALUE_NAME_LEN*2;

			rc = winNtQueryValueKeyProc(*KeyHandle, pObjectName,
				    KeyValueFullInformation, pValueFullInfo, 4096, pTempSize);

			/*
			 * Copy subkeys
			 */
			if (rc == STATUS_SUCCESS) {
				winNtDeleteValueKeyProc(*KeyHandle, pObjectName);
			} else {
				size = 0;
				FvmVm_FreeVirtualMemory(procHandle, &pObjectAttributes, &size,
				    MEM_RELEASE);
				rc = ntStatus;
				goto NtExit;
	      }
		}

		if (ntStatus != STATUS_SUCCESS) {
			/*
			 * Could not find the key in the VM.
			 * See if it is in the original location.
			 */
			pObjectName->Length = wcslen(pOriName)*2;

			pObjectName->MaximumLength = 2048;
			pObjectName->Buffer = pOriName;
			InitializeObjectAttributes(pObjectAttributes, pObjectName,
			    OBJ_CASE_INSENSITIVE, NULL, NULL);
			ntStatus = winNtOpenKeyProc(KeyHandle, ReqAccess, pObjectAttributes);

			if ((ntStatus != STATUS_SUCCESS)||(_wcsicmp(pOriName,
			    SERVICE_KEY) == 0)) {
				/*
				 * The original location does not have the key. return.
				 */
				size = 0;
				rc = FvmVm_FreeVirtualMemory(procHandle, &pObjectAttributes, &size,
					    MEM_RELEASE);
				rc = ntStatus;
				goto NtExit;
			}

			/*
			 * Found the key in the original location, copy it to our
			 * virtual machine
			 */
			working = 0;
			pObjectName->Length = wcslen(pVirName)*2;
			pObjectName->MaximumLength = 4096;
			pObjectName->Buffer = pVirName;
			InitializeObjectAttributes(pObjectAttributes, pObjectName,
			    OBJ_CASE_INSENSITIVE, NULL, NULL);

			/*
			 * We create all keys on the path
			 */
			pPointer = pVirName+1;

			/*
			 * To prevent dos attack, we only permit a path has no more than 100
			 * subkeys
			 */
			while (working < 3000) {

				working++;
				pPointer = wcschr(pPointer, L'\\');

				if (!pPointer) {
					/*
					 * Last sub key on the path
					 */
					pObjectName->Length = wcslen(pVirName) * 2;
					pObjectName->Buffer = pVirName;
					pObjectName->MaximumLength = 4096;

					if (*KeyHandle)
						winNtCloseProc(*KeyHandle);

					Disposition = (PULONG)pTempBuffer;
					ntStatus = winNtCreateKeyProc(KeyHandle, KEY_ALL_ACCESS,
							    pObjectAttributes, TitleIndex, Class,
							    createOptions, Disposition);

					/*
					 * We have trouble to create the
					 * key on the virtual machine
					 */
					if (ntStatus != STATUS_SUCCESS) {
					    size = 0;
						rc = FvmVm_FreeVirtualMemory(procHandle,
							    &pObjectAttributes, &size, MEM_RELEASE);
						rc = ntStatus;
						goto NtExit;
					}
					/*
					 * This is the last key. Exit the loop.
					 */
					break;
				} else {
					cSave = *pPointer;
					*pPointer = L'\0';
					pObjectName->Length = wcslen(pVirName)*2;
					pObjectName->Buffer = pVirName;
					pObjectName->MaximumLength=4096;

					if (*KeyHandle)
						winNtCloseProc(*KeyHandle);

					Disposition = (PULONG)pTempBuffer;
					ntStatus = winNtCreateKeyProc(KeyHandle, KEY_ALL_ACCESS,
							    pObjectAttributes,  TitleIndex, Class,
							    createOptions,Disposition );

					if (ntStatus == STATUS_SUCCESS &&
					    *Disposition ==REG_CREATED_NEW_KEY) {
						sv = (ULONG *)(pTempBuffer+SEES_VALUE_NAME_LEN_A);
						*sv = 0;
						wcscpy((PWCHAR)pTempBuffer, SEES_VALUE_NAME);
						pObjectName->Buffer = pTempBuffer;
						pObjectName->Length = (USHORT)SEES_VALUE_NAME_LEN*2;
						pObjectName->MaximumLength =
						    (USHORT)SEES_VALUE_NAME_LEN * 2;
						winNtSetValueKeyProc(*KeyHandle, pObjectName,  1,
							REG_DWORD, sv, sizeof(ULONG));
					}
					*pPointer = cSave;
					pPointer++;
				}
			}
		}

	    /*
		 *  Copy the values.
		 */
		pObjectName->Length = wcslen(pOriName) * 2;

		pObjectName->MaximumLength = 2048;
		pObjectName->Buffer = pOriName;
		InitializeObjectAttributes(pObjectAttributes, pObjectName,
				    OBJ_CASE_INSENSITIVE, NULL, NULL);

	    rc = winNtOpenKeyProc(pTempHandle, KEY_QUERY_VALUE, pObjectAttributes);

		if (rc == STATUS_SUCCESS) {
			int index = 0;
			working = 0;
			while (working < 3000) {
				working++;
				rc = winNtEnumerateValueKeyProc(*pTempHandle, index,
					    KeyValueFullInformation, pValueFullInfo,
					    4096 + 1024 * 1024, pTempSize);
				if (rc != STATUS_SUCCESS) {
					break;
				} else {
					/*
					 * Do the copying.
					 */
					pObjectName->Buffer = pValueFullInfo->Name;
					pObjectName->Length = (USHORT)pValueFullInfo->NameLength;
					pObjectName->MaximumLength =
					    (USHORT)pValueFullInfo->NameLength;

					rc = winNtSetValueKeyProc(*KeyHandle, pObjectName,
						    pValueFullInfo->TitleIndex, pValueFullInfo->Type,
						    (PVOID)( pValueFullInfo->DataOffset +
						    ((char *)pValueFullInfo)),
						    pValueFullInfo->DataLength);
				}
				index++;
			}
	    } else {
	            size = 0;
				rc = FvmVm_FreeVirtualMemory(procHandle, &pObjectAttributes, &size,
					    MEM_RELEASE);
				if (*KeyHandle) {
					winNtCloseProc(*KeyHandle);
					*KeyHandle = 0;
				}
				rc = ntStatus;
				goto NtExit;
	    }

		if (*pTempHandle) {
			winNtCloseProc(*pTempHandle);
		}

	    /*
		 * Copy subkeys
		 */
		pObjectName->Length = wcslen(pOriName) * 2;
		pObjectName->MaximumLength = 2048;
		pObjectName->Buffer = pOriName;
		InitializeObjectAttributes(pObjectAttributes, pObjectName,
			    OBJ_CASE_INSENSITIVE, NULL, NULL);
	    rc = winNtOpenKeyProc(pTempHandle, KEY_ENUMERATE_SUB_KEYS,
			    pObjectAttributes);

		pKeyBasicInfo = (PKEY_BASIC_INFORMATION )pValueFullInfo;

		if (rc == STATUS_SUCCESS) {
			int index = 0;
			working = 0;

			virNameLen = wcslen(pVirName);
			while (working < 3000) {
				working++;
				rc = winNtEnumerateKeyProc(*pTempHandle, index,
					    KeyBasicInformation, pKeyBasicInfo, 4096, pTempSize);

				if (rc != STATUS_SUCCESS) {
					break;
				} else {
					pKeyBasicInfo->Name[pKeyBasicInfo->NameLength/2] = L'\0';

					 pVirName[virNameLen] = L'\\';
					 pVirName[virNameLen+1] = L'\0';
					 wcscat(pVirName, pKeyBasicInfo->Name);
					 pObjectName->Length = wcslen(pVirName) * 2;
					 pObjectName->MaximumLength = 4096;
					 pObjectName->Buffer =  pVirName;
					 InitializeObjectAttributes(pObjectAttributes, pObjectName,
						    OBJ_CASE_INSENSITIVE, NULL, NULL );
					if (*KeyHandle)
						winNtCloseProc(*KeyHandle);

					 Disposition = (PULONG)pTempBuffer;
					 rc = winNtCreateKeyProc(pTempHandle1, KEY_ALL_ACCESS,
						    pObjectAttributes, TitleIndex, Class,
						    createOptions, Disposition);

					if (rc == STATUS_SUCCESS && *Disposition ==
					    REG_CREATED_NEW_KEY) {
						sv = (ULONG *)(pTempBuffer + SEES_VALUE_NAME_LEN_A);
						*sv = 0;
						wcscpy((PWCHAR)pTempBuffer, SEES_VALUE_NAME);
						pObjectName->Buffer = pTempBuffer;
						pObjectName->Length = (USHORT)SEES_VALUE_NAME_LEN * 2;
						pObjectName->MaximumLength =
						    (USHORT)SEES_VALUE_NAME_LEN * 2;
						winNtSetValueKeyProc(*pTempHandle1, pObjectName, 0,
						    REG_DWORD, sv, sizeof(ULONG));
					}
				}
				index++;
			}
			pVirName[virNameLen] = L'\0';
		} else {
			size = 0;
			FvmVm_FreeVirtualMemory(procHandle, &pObjectAttributes, &size,
			    MEM_RELEASE);

			if (*KeyHandle) {
				winNtCloseProc(*KeyHandle);
				*KeyHandle = 0;
			}
			rc = ntStatus;
			goto NtExit;
	    }

		/*
		 * Everything is set up.  Open the key from the virtual machine.
		 */
		if (*KeyHandle) {
			winNtCloseProc(*KeyHandle);
			*KeyHandle = 0;
		}
		if (*pTempHandle) {
			winNtCloseProc(*pTempHandle);
		}

		pObjectName->Length = wcslen(pVirName) * 2;
		pObjectName->MaximumLength = 4096;
		pObjectName->Buffer = pVirName;
		InitializeObjectAttributes(pObjectAttributes, pObjectName,
			    OBJ_CASE_INSENSITIVE, NULL, NULL);

		rc = winNtOpenKeyProc(KeyHandle, ReqAccess, pObjectAttributes);

		size = 0;
		FvmVm_FreeVirtualMemory(procHandle, &pObjectAttributes, &size,
			    MEM_RELEASE);
		goto NtExit;
	} except(EXCEPTION_EXECUTE_HANDLER) {
		DbgPrint(("Exception occured in FvmReg_NtOpenKey\n"));
		rc = -1;
	}
 NtExit:

	InterlockedDecrement (&fvm_Calls_In_Progress);
	return rc;
}


NTSTATUS
FvmReg_NtCreateKey
(
	OUT PHANDLE KeyHandle,
	IN ACCESS_MASK ReqAccess,
	IN POBJECT_ATTRIBUTES ObjectAttributes,
	IN ULONG TitleIndex,
	IN PUNICODE_STRING Class,
	IN ULONG CreateOptions,
	OUT PULONG Disposition
) {
	NTSTATUS	ntStatus;
	NTSTATUS	rc, rc2;
	HANDLE	procHandle;
	ULONG	virNameLen;
	PKEY_BASIC_INFORMATION	pKeyBasicInfo;
	int	vmn;
	int	size;
	int	initial;
	ULONG	sees_value;
	int	index;
	ULONG	working;
	PWCHAR	pPointer;
	WCHAR	cSave;
	PULONG	sv;

	/*
	 * All of these pointers point to some virtual memory in the user space.
	 * Micro$oft NT system calls require that all OUT parameters must be in
	 * user space. Since we are working on the behalf of the process, we need to
	 * use a undocumented function ZwAllocationVirtualMemory to allocate some
	 * virtual memory in user space to satisfy this requirement.
	 */

	POBJECT_ATTRIBUTES	pObjectAttributes;
	PUNICODE_STRING	pObjectName;
	PKEY_VALUE_FULL_INFORMATION	pValueFullInfo;
	PHANDLE	pTempHandle1;
	PHANDLE	pTempHandle;
	PULONG	pTempSize;
	PWCHAR	pOriName;
	PWCHAR	pVirName;
	PWCHAR	pTempBuffer;
	WCHAR Name[MAXPROCNAMELEN];

	InterlockedIncrement(&fvm_Calls_In_Progress);
	try {
		vmn = FvmVm_GetPVMId((ULONG)PsGetCurrentProcessId());
		
		if (vmn == INVALID_VMID) {
			/*
			 * The corruent process is not running under a VM, call the
			 * original system call.
			 */

			rc = winNtCreateKeyProc(KeyHandle, ReqAccess, ObjectAttributes,
				    TitleIndex,Class, CreateOptions,Disposition);
			goto NtExit;
		}

		/*
		 * Allocate virtual memory from the process user space
		 */
		size = 16384; /* 4 pages */
		procHandle = NtCurrentProcess();
		pObjectAttributes = NULL;
		rc = FvmVm_AllocateVirtualMemory(procHandle, &pObjectAttributes, 0, &size,
				    MEM_COMMIT, PAGE_EXECUTE_READWRITE);

		if (rc != STATUS_SUCCESS) {
			DbgPrint("--memory allocation problem(%d,NtCreateKey)\n",(ULONG)PsGetCurrentProcessId());
			goto NtExit;
		}

		/*
		 * Allocate the memory for each individual member
		 */
		(char *)pObjectName = ((char *)pObjectAttributes ) +
			    sizeof(OBJECT_ATTRIBUTES);
		(char *)pValueFullInfo = ((char *)pObjectName)+
			    sizeof(UNICODE_STRING);
		(char *)pTempHandle = ((char *)pValueFullInfo) + 4096;
		(char *)pTempHandle1 = ((char *)pTempHandle) + sizeof(HANDLE);
		(char *)pTempSize = ((char *)pTempHandle1) + sizeof(HANDLE);
		(char *)pOriName = ((char *)pTempSize) + sizeof(ULONG);
		(char *)pVirName = ((char *)pOriName) + 4096;
		(char *)pTempBuffer = ((char *)pVirName) + 4096;

		/*
		 * Get the orignal and the virtual name of the registry key
		 */
		pOriName[0] = L'\0';
		pVirName[0] = L'\0';
		initial = GetFullRegName(ObjectAttributes->RootDirectory,
				    ObjectAttributes->ObjectName, pOriName, pVirName,
				    pTempBuffer, vmn);

		pObjectName->MaximumLength = 2048;

#if 1 //kghari: hacks to get the office XP/2k3 working!.

		if (wcsstr(pOriName, L"MSSetup_Chaining") != NULL) {
			rc = winNtCreateKeyProc(KeyHandle, ReqAccess, ObjectAttributes,
				    TitleIndex,Class, CreateOptions,Disposition);
			FvmVm_FreeVirtualMemory(procHandle, &pObjectAttributes, &size,
			    MEM_RELEASE);
			goto NtExit;
		}
		if (wcsstr(pOriName, L"ChainedInstalls") != NULL) {
			rc = winNtCreateKeyProc(KeyHandle, ReqAccess, ObjectAttributes,
				    TitleIndex,Class, CreateOptions,Disposition);
			FvmVm_FreeVirtualMemory(procHandle, &pObjectAttributes, &size,
			    MEM_RELEASE);
			goto NtExit;
		}
		if (wcsstr(pOriName, L"Office") != NULL) {
			rc = winNtCreateKeyProc(KeyHandle, ReqAccess, ObjectAttributes,
				    TitleIndex,Class, CreateOptions,Disposition);
			FvmVm_FreeVirtualMemory(procHandle, &pObjectAttributes, &size,
			    MEM_RELEASE);
			goto NtExit;
		}


		if (wcsstr(pOriName, L"Cryptography") != NULL) {
			rc = winNtCreateKeyProc(KeyHandle, ReqAccess, ObjectAttributes,
				    TitleIndex,Class, CreateOptions,Disposition);
			FvmVm_FreeVirtualMemory(procHandle, &pObjectAttributes, &size,
			    MEM_RELEASE);
			goto NtExit;
		}


		if (wcsstr(pOriName, L"Tcpip") != NULL) {
			rc = winNtCreateKeyProc(KeyHandle, ReqAccess, ObjectAttributes,
				    TitleIndex,Class, CreateOptions,Disposition);
			FvmVm_FreeVirtualMemory(procHandle, &pObjectAttributes, &size,
			    MEM_RELEASE);
			goto NtExit;
		}

//		DbgPrint("pOriName = %S Virt Name %S\n", pOriName, pVirName);
#endif


		/*
		 * The input key has a virtual machine prefix
		 */

		pObjectName->Length = wcslen(pOriName) * 2;
		pObjectName->Buffer = pOriName;

		InitializeObjectAttributes(pObjectAttributes, pObjectName,
			    OBJ_CASE_INSENSITIVE, NULL, NULL);
		ntStatus = winNtCreateKeyProc(KeyHandle, ReqAccess,
				    pObjectAttributes,  TitleIndex, Class,
				    CreateOptions, Disposition);

		if (ntStatus != STATUS_SUCCESS) {
			size = 0;
			DbgPrint("Status NOT Success\n");
			FvmVm_FreeVirtualMemory(procHandle, &pObjectAttributes, &size,
			    MEM_RELEASE);
			rc = ntStatus;
			goto NtExit;
		} else if (*Disposition == REG_CREATED_NEW_KEY) {
			ZwDeleteKey(KeyHandle);
		} else {
			ZwClose(KeyHandle);
		}

		pObjectName->Length = wcslen(pVirName) * 2;
		pObjectName->Buffer = pVirName;


		InitializeObjectAttributes(pObjectAttributes, pObjectName,
				    OBJ_CASE_INSENSITIVE, NULL, NULL);
		ntStatus = winNtCreateKeyProc(KeyHandle, ReqAccess | KEY_SET_VALUE,
				    pObjectAttributes,  TitleIndex, Class, CreateOptions,
				    Disposition);


		/*
		 * If there is a existing key in the virtual machine, we return
		 */
	    if (ntStatus == STATUS_SUCCESS && *Disposition ==
		    REG_OPENED_EXISTING_KEY) {
			wcscpy((PWCHAR) pTempBuffer, SEES_VALUE_NAME);
			pObjectName->Buffer = pTempBuffer;
			pObjectName->Length = (USHORT) SEES_VALUE_NAME_LEN * 2;
			pObjectName->MaximumLength = (USHORT) SEES_VALUE_NAME_LEN * 2;

			rc = winNtQueryValueKeyProc(*KeyHandle,pObjectName,
				    KeyValueFullInformation, pValueFullInfo, 4096, pTempSize);

			if (rc == STATUS_SUCCESS) {
				/*
				 * Copy flag has been turned off.
				 */
				winNtDeleteValueKeyProc(*KeyHandle, pObjectName);
			} else {
				size = 0;
				FvmVm_FreeVirtualMemory(procHandle, &pObjectAttributes, &size,
				    MEM_RELEASE);
				rc = ntStatus;
				goto NtExit;
			}
    	}

	    /*
		 * There is no key in the virtual machine, we create one
		 */
		if (ntStatus != STATUS_SUCCESS) {
			working = 0;
			pObjectName->MaximumLength = 4096;
			pObjectName->Buffer = pVirName;

			InitializeObjectAttributes(pObjectAttributes, pObjectName,
				    OBJ_CASE_INSENSITIVE, NULL, NULL);


			pPointer = pVirName + 1;
			while(working<1000) {
				working++;
				pPointer = wcschr(pPointer, L'\\');

				if (!pPointer) {   // last key on the path
					 pObjectName->Length = wcslen(pVirName) * 2;
					 pObjectName->Buffer = pVirName;
					 pObjectName->MaximumLength = 4096;

					if (*KeyHandle)
						ZwClose(*KeyHandle);

					Disposition  = (PULONG)pTempBuffer;
					ntStatus = winNtCreateKeyProc(KeyHandle, KEY_ALL_ACCESS,
							    pObjectAttributes, TitleIndex, Class,
							    CreateOptions, Disposition);
					if (ntStatus !=  STATUS_SUCCESS) {
						size = 0;
						FvmVm_FreeVirtualMemory(procHandle, &pObjectAttributes,
							    &size, MEM_RELEASE);

						rc = ntStatus;
						goto NtExit;
					}
					break;
				} else {
					cSave = *pPointer;
					*pPointer = L'\0';

					pObjectName->Length = wcslen(pVirName) * 2;
					pObjectName->Buffer = pVirName;
					pObjectName->MaximumLength = 4096;

					if (*KeyHandle)
						winNtCloseProc(*KeyHandle);

					Disposition  = (PULONG)pTempBuffer;
					ntStatus = winNtCreateKeyProc(KeyHandle, KEY_ALL_ACCESS,
							    pObjectAttributes,  TitleIndex, Class,
							    CreateOptions,Disposition);

					if (ntStatus == STATUS_SUCCESS &&
					    *Disposition ==REG_CREATED_NEW_KEY) {
						sv = (ULONG *)(pTempBuffer+SEES_VALUE_NAME_LEN_A);
						*sv = 0;
						wcscpy((PWCHAR) pTempBuffer, SEES_VALUE_NAME);
						pObjectName->Buffer = pTempBuffer;
						pObjectName->Length = (USHORT)SEES_VALUE_NAME_LEN * 2;
						pObjectName->MaximumLength =
							    (USHORT)SEES_VALUE_NAME_LEN*2;

						winNtSetValueKeyProc(*KeyHandle, pObjectName, 0,
							    REG_DWORD, sv, sizeof(ULONG));
					}

					*pPointer = cSave;
					pPointer++;
				}
			}
		}

		/*
		 * copy the values
		 */
		pObjectName->Length = (wcslen(pOriName)) * 2;
		pObjectName->MaximumLength = 4096;
		pObjectName->Buffer = pOriName;

		rc =  winNtOpenKeyProc(pTempHandle, KEY_QUERY_VALUE, pObjectAttributes);
		if (rc == STATUS_SUCCESS) {
			index = 0;
			working = 0;

			while(working < 3000) {
				working++;
				rc = winNtEnumerateValueKeyProc(*pTempHandle, index,
					    KeyValueFullInformation,
					    pValueFullInfo, 4096, pTempSize);
				if (rc != STATUS_SUCCESS) {
					break;
				} else {
					/*
					 *  Do the copying.
					 */
					pObjectName->Buffer = pValueFullInfo->Name;
					pObjectName->Length = (USHORT)pValueFullInfo->NameLength;
					pObjectName->MaximumLength =
						    (USHORT)pValueFullInfo->NameLength;

					rc =  winNtSetValueKeyProc(*KeyHandle, pObjectName,
						    pValueFullInfo->TitleIndex,
						    pValueFullInfo->Type,
						    (PVOID)( pValueFullInfo->DataOffset +
						    ((char *)pValueFullInfo)),
						    pValueFullInfo->DataLength);
				}
				index++;
			}
	    } else {
            size = 0;
			rc = FvmVm_FreeVirtualMemory(procHandle, &pObjectAttributes, &size,
				    MEM_RELEASE);
			rc = ntStatus;
			goto NtExit;
		}

		if (*pTempHandle) {
			winNtCloseProc(*pTempHandle);
		}

		pObjectName->Length = wcslen(pOriName) * 2;
		pObjectName->MaximumLength = 2048;
		pObjectName->Buffer = pOriName;
		InitializeObjectAttributes(pObjectAttributes, pObjectName,
			    OBJ_CASE_INSENSITIVE, NULL, NULL);
	    rc =  winNtOpenKeyProc(pTempHandle, KEY_ENUMERATE_SUB_KEYS, pObjectAttributes);
		pKeyBasicInfo = (PKEY_BASIC_INFORMATION)pValueFullInfo;

		if (rc == STATUS_SUCCESS) {
			int index = 0;
			working = 0;
			virNameLen = wcslen(pVirName);

			while (working < 3000) { //2000 is for preventing dos
				working++;
				rc = winNtEnumerateKeyProc(*pTempHandle, index,
					    KeyBasicInformation, pKeyBasicInfo,  4096, pTempSize);
				if (rc != STATUS_SUCCESS) {
					break;
				} else {
					pKeyBasicInfo->Name[pKeyBasicInfo->NameLength/2] = L'\0';
					pVirName[virNameLen] = L'\\';
					pVirName[virNameLen+1] = L'\0';
					wcscat(pVirName,  pKeyBasicInfo->Name);
					pObjectName->Length = wcslen(pVirName) * 2;
					pObjectName->MaximumLength = 4096;
					pObjectName->Buffer =  pVirName;
					InitializeObjectAttributes(pObjectAttributes, pObjectName,
						    OBJ_CASE_INSENSITIVE, NULL, NULL);
					if (*KeyHandle)
						winNtCloseProc(*KeyHandle);
					Disposition = (PULONG)pTempBuffer;
					rc = winNtCreateKeyProc(pTempHandle1, KEY_ALL_ACCESS,
						    pObjectAttributes, TitleIndex, Class,
						    CreateOptions,Disposition );

					if (rc == STATUS_SUCCESS && *Disposition ==
					    REG_CREATED_NEW_KEY) {
						sv = (ULONG *)(pTempBuffer+SEES_VALUE_NAME_LEN_A);
						*sv = 0;
						wcscpy((PWCHAR) pTempBuffer, SEES_VALUE_NAME);
						pObjectName->Buffer = pTempBuffer;
						pObjectName->Length = (USHORT)SEES_VALUE_NAME_LEN*2;
						pObjectName->MaximumLength =
						    (USHORT)SEES_VALUE_NAME_LEN*2;

						winNtSetValueKeyProc(*pTempHandle1, pObjectName,  0,
						    REG_DWORD, sv, sizeof(ULONG));

					}
				}
				index++;
			}
			pVirName[virNameLen] = L'\0';

		} else {
			size = 0;
			rc = FvmVm_FreeVirtualMemory(procHandle, &pObjectAttributes, &size,
				    MEM_RELEASE);
			if (*KeyHandle) {
				winNtCloseProc(*KeyHandle);
				*KeyHandle = 0;
			}
			rc = ntStatus;
			goto NtExit;
		}

	 	/*
		 * Now open the key from the virtual machine
		 */
		if (*KeyHandle) {
			winNtCloseProc(*KeyHandle);
			*KeyHandle = 0;
		}
		if (*pTempHandle) {
			winNtCloseProc(*pTempHandle);
		}

		pObjectName->Length = wcslen(pVirName)*2;
		pObjectName->MaximumLength = 4096;
		pObjectName->Buffer = pVirName;
		InitializeObjectAttributes(pObjectAttributes, pObjectName,
			    OBJ_CASE_INSENSITIVE, NULL, NULL);


		ntStatus = winNtCreateKeyProc(KeyHandle, ReqAccess, pObjectAttributes,
				    TitleIndex, Class, CreateOptions, Disposition);

		size = 0;
		FvmVm_FreeVirtualMemory(procHandle, &pObjectAttributes, &size,
		    MEM_RELEASE);
		rc = ntStatus;
		goto NtExit;
	} except(EXCEPTION_EXECUTE_HANDLER) {
		DbgPrint(("Exception occured in FvmReg_NtCreateKey\n"));
		rc = -1;
	}

 NtExit:

	InterlockedDecrement (&fvm_Calls_In_Progress);
	return rc;
}


static void
GetRegKeyName(HANDLE Key, PWCHAR Name) {
	CHAR	buff[2064];
	PUNICODE_STRING	fullUniName = (PUNICODE_STRING)buff;
	POBJECT	pKey = NULL;
	ULONG	actualLen;
	Name[0] = L'\0';

	try {
		wcscpy(Name, L"error");
		if (pKey = GetPointer(Key)) {

			if (pKey) {
				fullUniName->MaximumLength = MAXPATHLEN * sizeof(WCHAR);

				if (NT_SUCCESS(ObQueryNameString(pKey, fullUniName,
				    MAXPATHLEN, &actualLen))) {
					wcscpy(Name, fullUniName->Buffer);
			} else {
				DbgPrint("failed to ObQueryNameString\n");
			}
			ReleasePointer( pKey );
		}
	}
  } except(EXCEPTION_EXECUTE_HANDLER) {
		Name[0] = L'\0';
		DbgPrint(("Exception occured in GetRegKeyName\n"));
	}
}


NTSTATUS
FvmReg_NtQueryKey0(
	IN HANDLE  KeyHandle,
	IN KEY_INFORMATION_CLASS  KeyInformationClass,
	OUT PVOID  KeyInformation,
	IN ULONG  Length,
	OUT PULONG  ResultLength
) {

	NTSTATUS	ntStatus;
	int	vmn;
	PWCHAR	wPtr;
	PWCHAR	buff;
	HANDLE	procHandle;
	NTSTATUS	rc;
	int	size;
	int	tmpSize;
	PULONG	rlen;
	WCHAR	wc;
	WCHAR	name[1024];
	int	slen;
	PVOID	keyInfo;
	PULONG	rsize;
	ULONG	newlength;

	InterlockedIncrement (&fvm_Calls_In_Progress);

	try {
		vmn = FvmUtil_VmNumber(); // get the virtual machine number

		if (vmn == INVALID_VMID) {
			ntStatus = winNtQueryKeyProc(KeyHandle, KeyInformationClass,
					    KeyInformation, Length, ResultLength);
			rc = ntStatus;
			goto NtExit;
		}

		if (KeyInformationClass == KeyFullInformation) {
			ntStatus = winNtQueryKeyProc(KeyHandle, KeyInformationClass,
					    KeyInformation, Length, ResultLength);
			rc = ntStatus;

			goto NtExit;
		}

		/*
		 * We need a bigger sized structure than the one that is passed.
		 */
		tmpSize = Length*2;
		if (tmpSize < 1024)
			tmpSize = 1024;
		else if (tmpSize < 2048)
			tmpSize = 2048;
		else if (tmpSize < 4096)
			tmpSize = 4096;
		else
			tmpSize = 8192;


		size = tmpSize * 2 + 4;

		procHandle = NtCurrentProcess();
		buff = NULL;
		rc = FvmVm_AllocateVirtualMemory(procHandle, &buff, 0, &size,
			    MEM_COMMIT, PAGE_EXECUTE_READWRITE);

		if (rc != STATUS_SUCCESS) {
			DbgPrint("-->memory allocation problem(%d,NtQueryKey0)\n",(ULONG)PsGetCurrentProcessId());
			goto NtExit;
		}

		keyInfo = (PVOID)(((char *)buff) + tmpSize);
		rsize = (PULONG)(((char*)buff) + tmpSize * 2);

		ntStatus = winNtQueryKeyProc( KeyHandle, KeyInformationClass,
				    keyInfo, tmpSize, rsize);

		if (ntStatus != STATUS_SUCCESS) {
			size = 0;
			FvmVm_FreeVirtualMemory(procHandle, &buff, &size, MEM_RELEASE);
			rc = ntStatus;
			goto NtExit;
		}

		if (KeyInformationClass == KeyBasicInformation) {
			PKEY_BASIC_INFORMATION keyBasic   = (PKEY_BASIC_INFORMATION)keyInfo;
			PKEY_BASIC_INFORMATION keyBasic1 =
			    (PKEY_BASIC_INFORMATION)KeyInformation;
			wc = keyBasic->Name[keyBasic->NameLength/2];
			keyBasic->Name[keyBasic->NameLength/2]=L'\0';
			wPtr = wcsstr(keyBasic->Name, pvms[vmn]->idStr);

			/*
			 * Get rid of our prefix.
			 */
			if (wPtr) {
				wPtr = wcschr(wPtr, L'\\');
				wcscpy(buff, wPtr);

				/* Check to see if the input buffer passed to the syscall is enough for storing the result from query */
				newlength = wcslen(buff)*2;
				if((*rsize - (keyBasic->NameLength - newlength)) > Length) {
					/* The input buffer is too short */
					rc = STATUS_BUFFER_TOO_SMALL;
					*ResultLength = *rsize - (keyBasic->NameLength - newlength);
					goto NtExit;
				}
				
				wcscpy(keyBasic1->Name, buff);

				keyBasic1->NameLength = wcslen(keyBasic1->Name) * 2;
				keyBasic1->Name[keyBasic1->NameLength/2] = wc;
			} else {
				/* Check to see if the input buffer passed to the syscall is enough for storing the result from query */
				newlength = wcslen(keyBasic->Name)*2;
				if((*rsize - (keyBasic->NameLength - newlength)) > Length) {
					/* The input buffer is too short */
					rc = STATUS_BUFFER_TOO_SMALL;
					*ResultLength = *rsize - (keyBasic->NameLength - newlength);
					goto NtExit;
				}

				wcscpy(keyBasic1->Name, keyBasic->Name);
				keyBasic1->NameLength = wcslen(keyBasic1->Name)*2;
				keyBasic1->Name[keyBasic1->NameLength/2] = wc;
			}

			keyBasic1->LastWriteTime = keyBasic->LastWriteTime;
			keyBasic1->TitleIndex = keyBasic->TitleIndex;
			*ResultLength = *rsize - (keyBasic->NameLength - keyBasic1->NameLength);
		} else if (KeyInformationClass ==
		    KeyNodeInformation) {
			PKEY_NODE_INFORMATION keyNode =(PKEY_NODE_INFORMATION)keyInfo;
			PKEY_NODE_INFORMATION keyNodeOut =(PKEY_NODE_INFORMATION)
			    KeyInformation;

			wc = keyNode->Name[keyNode->NameLength/2];
			keyNode->Name[keyNode->NameLength/2] = L'\0';

			wPtr = wcsstr(keyNode->Name, pvms[vmn]->idStr);
			if (wPtr) {
				wPtr = wcschr(wPtr, L'\\');
				wcscpy(buff, wPtr);

				/* Check to see if the input buffer passed to the syscall is enough for storing the result from query */
				newlength = wcslen(buff)*2;
				if((*rsize - (keyNode->NameLength - newlength)) > Length) {
				   /* The input buffer is too short */
				   rc = STATUS_BUFFER_TOO_SMALL;
				   *ResultLength = *rsize - (keyNode->NameLength - newlength);
				   goto NtExit;
				}
				
				wcscpy(keyNodeOut->Name, wPtr);
				keyNodeOut->NameLength = wcslen(keyNodeOut->Name)*2;
				keyNodeOut->ClassOffset = keyNode->ClassOffset-(keyNode->NameLength - keyNodeOut->NameLength);
			} else {
				/* Check to see if the input buffer passed to the syscall is enough for storing the result from query */
				newlength = wcslen(keyNode->Name)*2;
				if((*rsize - (keyNode->NameLength - newlength)) > Length) {
					/* The input buffer is too short */
					rc = STATUS_BUFFER_TOO_SMALL;
					*ResultLength = *rsize - (keyNode->NameLength - newlength);
					goto NtExit;
				}
			
				wcscpy(keyNodeOut->Name, keyNode->Name);
				keyNodeOut->ClassOffset = keyNode->ClassOffset;
				keyNodeOut->NameLength = wcslen(keyNodeOut->Name)*2;
			}

			keyNodeOut->Name[keyNode->NameLength/2] = wc;
			keyNodeOut->LastWriteTime = keyNode->LastWriteTime;
			keyNodeOut->TitleIndex = keyNode->TitleIndex;
			keyNodeOut->ClassLength = keyNode->ClassLength;
			memcpy(((char *)KeyInformation)+keyNodeOut->ClassOffset,
			    ((char *)keyInfo)+keyNode->ClassOffset, keyNodeOut->ClassLength);
			*ResultLength = *rsize - (keyNode->NameLength -
				    keyNodeOut->NameLength);

		} else if (KeyInformationClass == KeyNameInformation) {

			PKEY_NAME_INFORMATION keyName = (PKEY_NAME_INFORMATION) keyInfo;
			PKEY_NAME_INFORMATION keyNameOut = (PKEY_NAME_INFORMATION) KeyInformation;

			slen = keyName->NameLength;
			wc = keyName->Name[keyName->NameLength/2];
			keyName->Name[keyName->NameLength/2] = L'\0';
			wPtr = wcsstr(keyName->Name, pvms[vmn]->idStr);

			if (wPtr) {
				int tlen;
				wPtr = wcschr(wPtr, L'\\');
				wcscpy(buff, wPtr);

				/* Check to see if the input buffer passed to the syscall is enough for storing the result from query */
				newlength = wcslen(buff)*2;
				if((*rsize - (keyName->NameLength - newlength)) > Length) {
				   /* The input buffer is too short */
				   rc = STATUS_BUFFER_TOO_SMALL;
				   *ResultLength = *rsize - (keyName->NameLength - newlength);
				   goto NtExit;
				}

				wcscpy(keyNameOut->Name, buff);
				keyNameOut->NameLength = wcslen(keyNameOut->Name) * 2;
			} else {
				/* Check to see if the input buffer passed to the syscall is enough for storing the result from query */
				newlength = wcslen(keyName->Name)*2;
				if((*rsize - (keyName->NameLength - newlength)) > Length) {
					/* The input buffer is too short */
					rc = STATUS_BUFFER_TOO_SMALL;
					*ResultLength = *rsize - (keyName->NameLength - newlength);
					goto NtExit;
				}

				wcscpy(keyNameOut->Name, keyName->Name);
				keyNameOut->NameLength = wcslen(keyNameOut->Name)*2;
			}
			*ResultLength = *rsize - (keyName->NameLength -
			    keyNameOut->NameLength);
			keyNameOut->Name[keyNameOut->NameLength/2] = wc;

		} 
#if 1
		else if(KeyInformationClass == KeyCachedInformation) {

			/*    On Windows XP, NtQueryKey does return KEY_CACHED_INFORMATION
			  *    Information Class also. We do
			  */

			PKEY_CACHED_INFORMATION keyCached =(PKEY_CACHED_INFORMATION)keyInfo;
			PKEY_CACHED_INFORMATION keyCachedOut =(PKEY_CACHED_INFORMATION) KeyInformation;

			/* Check to see if the input buffer passed to the syscall is enough for storing the result from query */
			if(*rsize > Length) {
			   /* The input buffer is too short */
			   rc = STATUS_BUFFER_TOO_SMALL;
			   *ResultLength = *rsize;
			   goto NtExit;
			}

			*ResultLength = *rsize;

			keyCachedOut->LastWriteTime = keyCached->LastWriteTime;
			keyCachedOut->TitleIndex = keyCached->TitleIndex;
			keyCachedOut->SubKeys = keyCached->SubKeys;
			keyCachedOut->MaxNameLen = keyCached->MaxNameLen;
			keyCachedOut->Values = keyCached->Values;
			keyCachedOut->MaxValueNameLen = keyCached->MaxValueNameLen;
			keyCachedOut->MaxValueDataLen = keyCached->MaxValueDataLen;
			keyCachedOut->NameLength = keyCached->NameLength;
		}
#endif
		size = 0;
		FvmVm_FreeVirtualMemory(procHandle, &buff, &size, MEM_RELEASE);
		rc = ntStatus;
		goto NtExit;
	}   except(EXCEPTION_EXECUTE_HANDLER) {
		DbgPrint(("Exception occured in FvmReg_NtQueryKey0\n"));
		rc = -1;
	}
NtExit:
	InterlockedDecrement (&fvm_Calls_In_Progress);
	return rc;
}



NTSTATUS
FvmReg_NtQueryKey(
	IN HANDLE  KeyHandle,
	IN KEY_INFORMATION_CLASS  KeyInformationClass,
	OUT PVOID  KeyInformation,
	IN ULONG  Length,
	OUT PULONG  ResultLength
) {
	NTSTATUS ntStatus;
	
	ntStatus = FvmReg_NtQueryKey0(KeyHandle, KeyInformationClass, KeyInformation, Length, ResultLength);
	return ntStatus;
}


/*
 * Once we've deleted a key, we can remove its reference in the hash
 * table.
 */
NTSTATUS
FvmReg_NtDeleteKey(
	IN HANDLE Handle
) {
	NTSTATUS	ntStatus;
	int	vmn = -1;
	WCHAR	name[1024];

	/*
	 * Get the VM number
	 */
	vmn = FvmUtil_VmNumber();
	InterlockedIncrement (&fvm_Calls_In_Progress);

	if (ExGetPreviousMode() == KernelMode) {
		ntStatus = winNtDeleteKeyProc(Handle);
		goto Original_Call;
	}

	try {
		RtlZeroMemory(name, 1024 * sizeof(WCHAR));
		GetRegKeyName(Handle, name);

		ntStatus = winNtDeleteKeyProc(Handle);
		if (vmn != -1 && vmn != INVALID_VMID) {
			FvmTable_DeleteLogAdd(name, vmn);
		}
	} except(EXCEPTION_EXECUTE_HANDLER) {
		DbgPrint(("Exception occured in FvmReg_NtDeleteKey\n"));
		ntStatus = -1;
	}
Original_Call:

	InterlockedDecrement (&fvm_Calls_In_Progress);
	return ntStatus;
}


//----------------------------------------------------------------------
//
// HookRegEnumerateKey
//
// This is a documented Zw-class function.
//
//----------------------------------------------------------------------
NTSTATUS
NTAPI
FvmReg_NtEnumerateKey0(
	IN HANDLE KeyHandle,
	IN ULONG Index,
	IN KEY_INFORMATION_CLASS KeyInformationClass,
	OUT PVOID KeyInformation,
	IN ULONG Length,
	OUT PULONG ResultLength
) {
	NTSTATUS	ntStatus;
	int	vmn;
	PWCHAR	wptr;
	PWCHAR	info;
	PWCHAR	buff;
	HANDLE	procHandle;
	int	size;
	int	tmpSize;
	WCHAR	wc;
	PVOID	keyInfo;
	int	*rSize;

	InterlockedIncrement(&fvm_Calls_In_Progress);

	try {

		/*
		 * Get the VM number
		 */
		vmn = FvmUtil_VmNumber();
		if (vmn == INVALID_VMID) {
			ntStatus= winNtEnumerateKeyProc(KeyHandle, Index,
					    KeyInformationClass, KeyInformation, Length,
					    ResultLength);
			goto NtExit;
		}
		if (KeyInformationClass == KeyFullInformation) {
			ntStatus = winNtEnumerateKeyProc(KeyHandle, Index,
					    KeyInformationClass, KeyInformation, Length,
					    ResultLength);
			goto NtExit;
		}

		/*
		  * We need bigger sized structure than the one passed by the program
		  */
		tmpSize = Length*2;
		if (tmpSize < 1024)
			tmpSize = 1024;
		else if (tmpSize < 2048)
			tmpSize = 2048;
		else if (tmpSize < 4096)
			tmpSize = 4096;
		else
			tmpSize = 8192;

		size = tmpSize*2 + 4;
		procHandle = NtCurrentProcess();
		buff = NULL;
		ntStatus = FvmVm_AllocateVirtualMemory(procHandle, &buff, 0, &size,
				    MEM_COMMIT, PAGE_EXECUTE_READWRITE);

		if (ntStatus != STATUS_SUCCESS) {
			DbgPrint("-->memory allocation problem(%d,NtEnumerateKey0)\n",(ULONG)PsGetCurrentProcessId());
			goto NtExit;
		}

		keyInfo = (PVOID)(((char *)buff)+tmpSize);
		rSize = (int *)(((char*)buff)+tmpSize*2);
		ntStatus = winNtEnumerateKeyProc( KeyHandle, Index, KeyInformationClass, keyInfo, tmpSize, rSize );

		if (ntStatus != STATUS_SUCCESS) {
			size = 0;
			FvmVm_FreeVirtualMemory(procHandle, &buff, &size, MEM_RELEASE);
			goto NtExit;
         }

         if (KeyInformationClass == KeyBasicInformation) {
			PKEY_BASIC_INFORMATION keyBasic =
				    (PKEY_BASIC_INFORMATION) keyInfo;
			PKEY_BASIC_INFORMATION keyBasic1 =
				    (PKEY_BASIC_INFORMATION) KeyInformation;

               wc = keyBasic->Name[keyBasic->NameLength/2];
               keyBasic->Name[keyBasic->NameLength/2]=L'\0';
               wptr = wcsstr(keyBasic->Name, pvms[vmn]->idStr);


				/*
				 * Get rid of our prefix.
				 */
			if (wptr) {
				wptr = wcschr(wptr, L'\\');
				wcscpy(buff, wptr);
				wcscpy(keyBasic1->Name, buff);

				keyBasic1->NameLength = wcslen(keyBasic1->Name)*2;
				keyBasic1->Name[keyBasic1->NameLength/2] = wc;
			} else {
				wcscpy(keyBasic1->Name, keyBasic->Name);
				keyBasic1->NameLength = wcslen(keyBasic1->Name)*2;
				keyBasic1->Name[keyBasic1->NameLength/2] = wc;
			}

			keyBasic1->LastWriteTime = keyBasic->LastWriteTime;
			keyBasic1->TitleIndex = keyBasic->TitleIndex;
			*ResultLength = *rSize - (keyBasic->NameLength -
						    keyBasic1->NameLength);
		} else if (KeyInformationClass ==
		    KeyNodeInformation) {
			PKEY_NODE_INFORMATION keyNode = (PKEY_NODE_INFORMATION)keyInfo;
			PKEY_NODE_INFORMATION keyNodeOut =
				    (PKEY_NODE_INFORMATION)KeyInformation;

			wc = keyNode->Name[keyNode->NameLength/2];
			keyNode->Name[keyNode->NameLength/2]=L'\0';

			wptr = wcsstr(keyNode->Name, pvms[vmn]->idStr);
			if (wptr) {
				wptr = wcschr(wptr, L'\\');
				wcscpy(buff, wptr);
				wcscpy(keyNodeOut->Name, wptr);
				keyNodeOut->NameLength = wcslen(keyNodeOut->Name) * 2;
				keyNodeOut->ClassOffset = keyNode->ClassOffset -
					    (keyNode->NameLength - keyNodeOut->NameLength);
			} else {
				wcscpy(keyNodeOut->Name, keyNode->Name);
				keyNodeOut->ClassOffset=keyNode->ClassOffset;
				keyNodeOut->NameLength = wcslen(keyNodeOut->Name)*2;
			}
			keyNodeOut->Name[keyNode->NameLength/2]=wc;
			keyNodeOut->LastWriteTime = keyNode->LastWriteTime;
			keyNodeOut->TitleIndex = keyNode->TitleIndex;
			keyNodeOut->ClassLength = keyNode->ClassLength;
			memcpy(((char *)KeyInformation)+keyNodeOut->ClassOffset,
			    ((char *)keyInfo)+keyNode->ClassOffset,
			    keyNodeOut->ClassLength);
			*ResultLength = *rSize - (keyNode->NameLength -
				keyNodeOut->NameLength);
         } else if (KeyInformationClass == KeyNameInformation) {
            PKEY_NAME_INFORMATION keyName = (PKEY_NAME_INFORMATION)keyInfo;
			PKEY_NAME_INFORMATION keyNameOut =
    				(PKEY_NAME_INFORMATION)KeyInformation;

			wc = keyName->Name[keyName->NameLength/2];
			keyName->Name[keyName->NameLength/2] = L'\0';
			wptr = wcsstr(keyName->Name, pvms[vmn]->idStr);
			if (wptr) {
				wptr = wcschr(wptr, L'\\');
				wcscpy(buff, wptr);
				wcscpy(keyNameOut->Name, buff);
				keyNameOut->NameLength = wcslen(keyNameOut->Name)*2;
			} else {
				wcscpy(keyNameOut->Name, keyName->Name);
				keyNameOut->NameLength = wcslen(keyNameOut->Name)*2;
			}
			*ResultLength = *rSize - (keyName->NameLength -
				    keyNameOut->NameLength);
			keyNameOut->Name[keyNameOut->NameLength/2] = wc;
         }

		size = 0;
		FvmVm_FreeVirtualMemory(procHandle, &buff, &size, MEM_RELEASE);
		goto NtExit;
	} except(EXCEPTION_EXECUTE_HANDLER) {
		DbgPrint(("Exception occured in FvmReg_NtEnumerateKey0\n"));
		ntStatus= -1;
	}

NtExit:

	InterlockedDecrement (&fvm_Calls_In_Progress);
	return ntStatus;
}


NTSTATUS
FvmReg_NtEnumerateKey (
	IN HANDLE KeyHandle,
	IN ULONG Index,
	IN KEY_INFORMATION_CLASS KeyInformationClass,
	OUT PVOID KeyInformation,
	IN ULONG Length,
	OUT PULONG ResultLength
) {
	NTSTATUS ntStatus;

	InterlockedIncrement(&fvm_Calls_In_Progress);
	try {
		ntStatus = FvmReg_NtEnumerateKey0(KeyHandle, Index,
				    KeyInformationClass, KeyInformation, Length, ResultLength);

	} except(EXCEPTION_EXECUTE_HANDLER) {
		DbgPrint(("Exception occured in FvmReg_NtEnumerateKey\n"));
		ntStatus= -1;
	}
	InterlockedDecrement (&fvm_Calls_In_Progress);

	return ntStatus;
}


//----------------------------------------------------------------------
//
// HookRegSetValueKey
//
//----------------------------------------------------------------------
NTSTATUS
FvmReg_NtSetValueKey(
	IN HANDLE KeyHandle,
	IN PUNICODE_STRING ValueName,
	IN ULONG TitleIndex,
	IN ULONG Type,
	IN PVOID Data,
	IN ULONG DataSize
) {
	NTSTATUS	ntStatus = -1;
	int	vmn;

	try {
		vmn = FvmUtil_VmNumber();

		ntStatus = winNtSetValueKeyProc(KeyHandle, ValueName, TitleIndex,
				    Type, Data, DataSize);

		return ntStatus;
	} except(EXCEPTION_EXECUTE_HANDLER) {
		DbgPrint(("Exception occured in FvmReg_NtSetValueKey\n"));
	}

	return ntStatus;
}


/*
 * This is a documented Zw-class function.
 */
NTSTATUS
FvmReg_NtQueryValueKey(
	IN HANDLE KeyHandle,
	IN PUNICODE_STRING ValueName,
	IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
	OUT PVOID KeyValueInformation,
	IN ULONG Length,
	OUT PULONG  ResultLength
) {
	NTSTATUS	ntStatus;
	int	vmn;

	WCHAR name_rqk[_MAX_PATH];
	WCHAR RegPath[_MAX_PATH]={L'\0'};
		
	vmn = FvmUtil_VmNumber();

	ntStatus = winNtQueryValueKeyProc(KeyHandle, ValueName,
    			KeyValueInformationClass, KeyValueInformation, Length,
    			ResultLength);

	return ntStatus;
}


/*
 * This is a documented Zw-class function.
 */
NTSTATUS
FvmReg_NtEnumerateValueKey(
	IN HANDLE KeyHandle,
	IN ULONG Index,
	IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
	OUT PVOID KeyValueInformation,
	IN ULONG Length,
	OUT PULONG  pResultLength
) {
	NTSTATUS                ntStatus;
	int                     vmn;
	PWCHAR                 wptr;
	WCHAR                  twc;
	int                    dLen;

	vmn = FvmUtil_VmNumber();

	ntStatus = winNtEnumerateValueKeyProc(KeyHandle, Index,
    			KeyValueInformationClass, KeyValueInformation, Length,
    			pResultLength );

	return ntStatus;
}
