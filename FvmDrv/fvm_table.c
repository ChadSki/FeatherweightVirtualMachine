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
 * This file implements functions for maintaining the delete logs, private FS
 * namespaces  for the VMs.
 */

#include <ntddk.h>
#include <stdlib.h>
#include "hooksys.h"
#include "fvm_util.h"
#include "fvm_vm.h"
#include "fvm_syscalls.h"
#include "fvm_table.h"

#define FVM_TABLE_POOL_TAG '6GAT'

// This might need change later on in case we start having longer registry path names.
// Probably need to have a better fix in place for this.
#define FVM_MAX_PATH_NAME 500

typedef struct _nameentrylog {
	WCHAR    RealName[FVM_MAX_PATH_NAME];
} HASH_ENTRY_LOG, *PHASH_ENTRY_LOG;

/*
 * Reader/Writer lock to protect hash table.
 */

ERESOURCE fvm_DeleteLogResource, fvm_HandleTableResource;

FVM_PDeleteLogEntry    *fvm_DeleteState = NULL;
FVM_PHandleTableEntry  *fvm_HandleTable = NULL;

#ifdef USE_FS_MAP

ERESOURCE fvm_FSResource;
PFVM_File_Entry *fvm_FileList = NULL;

#endif


/***********************************************************************
  Dump each vm's deletelog to file, type: 0 = temp, 1 = permanent
************************************************************************/
VOID FvmTable_DumpVMDeleteLog(int vmNumber, int type)
{
	FVM_PDeleteLogEntry delEntry, nextEntry;
	PHASH_ENTRY_LOG hashEntryLog;
	UNICODE_STRING UnicodeFilespec;
	NTSTATUS status;
	OBJECT_ATTRIBUTES objectAttributes;
	HANDLE fileHandle;
	IO_STATUS_BLOCK Iosb;
	WCHAR buf[256];

	RtlZeroMemory(buf, sizeof(WCHAR)*256);

	DbgPrint("Start to dump log for VM: %d\n", vmNumber);

	hashEntryLog = ExAllocatePoolWithTag( PagedPool, sizeof(HASH_ENTRY_LOG),
		FVM_TABLE_POOL_TAG);

	if(type == 0) {
		swprintf(buf,L"\\??\\%s\\%s.bin",pvms[vmNumber]->fileRoot,
		    pvms[vmNumber]->idStr);
	} else {
		swprintf(buf,L"\\??\\%s\\%s.bin",pvms[vmNumber]->fileRoot,
	        pvms[vmNumber]->idStr);
	}
	DbgPrint("FVM %d delete log file name is: %S\n", vmNumber, buf);
	RtlInitUnicodeString(&UnicodeFilespec, buf);

	InitializeObjectAttributes(&objectAttributes, &UnicodeFilespec,
	    OBJ_CASE_INSENSITIVE, NULL, NULL );

	status = ZwCreateFile
	    (&fileHandle,              		  // returned file handle
	    (FILE_WRITE_DATA | SYNCHRONIZE ), // desired access
	    &objectAttributes,                // ptr to object attributes
	    &Iosb,                            // ptr to I/O status block
	    0,                                // allocation size
	    FILE_ATTRIBUTE_NORMAL,            // file attributes
	    0,                                // share access
	    FILE_OVERWRITE_IF,                // create disposition
	    FILE_SYNCHRONOUS_IO_NONALERT,     // create options
	    NULL,                             // ptr to extended attributes
	    0);                               // length of ea buffer

	KeEnterCriticalRegion();
	ExAcquireResourceSharedLite( &fvm_DeleteLogResource, TRUE );

	/*
	 * Print the delete log entries
	 */
	delEntry = fvm_DeleteState[vmNumber];

	if (delEntry)
		//DbgPrint("Writing Delete Log for FVM-%u\n", vmNumber);

	while(delEntry) {

		/* If the pathname of the entry to be deleted is more than the current
		 * defined FVM_MAX_PATH_NAME, we do not add it to the log file
		 * and the Debug Buffer has a mention of this..
		 */
		if (wcslen(delEntry->Name) > FVM_MAX_PATH_NAME)
			DbgPrint("!! IGNORED entry (path length > %d) : %S !!\n", FVM_MAX_PATH_NAME, delEntry->Name);

		else {

			// print the delete log
			//DbgPrint("	Deleted %S.\n", delEntry->Name);
			RtlZeroMemory(hashEntryLog, sizeof(HASH_ENTRY_LOG));
			wcscpy(hashEntryLog->RealName, delEntry->Name);

			status = ZwWriteFile(fileHandle, NULL, NULL, NULL, &Iosb,
					     (void*) hashEntryLog, sizeof(HASH_ENTRY_LOG), NULL, NULL);

			if (!NT_SUCCESS(status)){
				CHAR buffer[64];
				FvmUtil_ErrorString(status, buffer);
				DbgPrint("Failed to write entry to file with Error: %s\n", buffer);
			}
		}
		nextEntry = delEntry->Next;
		delEntry = nextEntry;
	}

	ExReleaseResourceLite(&fvm_DeleteLogResource);
	KeLeaveCriticalRegion();

	if(fileHandle){
		winNtCloseProc(fileHandle);
	}

	if(hashEntryLog){
		ExFreePool(hashEntryLog);
	}
}

//read deleted log from file to restore vm state
NTSTATUS FvmTable_ReadVMDeletedLogFile(int vmNumber)
{
	NTSTATUS status = 0;
	POBJECT_ATTRIBUTES poa = NULL;
	PUNICODE_STRING pathname = NULL;
	PHANDLE   pvfile = NULL;
	PIO_STATUS_BLOCK piostatus = NULL;

	UINT numEntries=0,j = 0;
	int i = 0;
	int cont = 0, logstate = 0;
	PHASH_ENTRY_LOG hashEntryLog = NULL;
	int size = 0;
	int memsize = 0;
	PWCHAR logpath = NULL;

	memsize = sizeof(HANDLE) + sizeof(IO_STATUS_BLOCK) +
	    sizeof(OBJECT_ATTRIBUTES) + sizeof(UNICODE_STRING) + 128*2 + 2;

	//DbgPrint("VM: %d  memsize: %d \n", vmNumber, memsize);

	__try{
		status = FvmVm_AllocateVirtualMemory(NtCurrentProcess(), (PVOID*) &pvfile,
				     0, &memsize, MEM_COMMIT, PAGE_READWRITE);
	}
	__except(EXCEPTION_EXECUTE_HANDLER){
		DbgPrint(("Exception occured in FvmVm_AllocateVirtualMemory\n"));
		return -1;
	}

	if (NT_SUCCESS(status)) {
		(char *)piostatus = ((char *)pvfile) + sizeof(HANDLE);
		(char *)poa = ((char *)piostatus) + sizeof(IO_STATUS_BLOCK);
		(char *)pathname = ((char *)poa) + sizeof(OBJECT_ATTRIBUTES);
		(char *)logpath = ((char *)pathname) + sizeof(UNICODE_STRING);

		//swprintf(logpath,L"\\??\\%s\\vm%d.bin",szLogFilePath,vmNumber);
		swprintf(logpath, L"\\??\\%s\\%s.bin", pvms[vmNumber]->fileRoot,
		    pvms[vmNumber]->idStr);
		DbgPrint("Log File Name is: %S\n",logpath);
		RtlInitUnicodeString(pathname, logpath);
		InitializeObjectAttributes(poa, pathname,
		    OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE, NULL, NULL);
	}

	status = ((NtCreateFileProc)(winNtCreateFileProc)) (
			     pvfile,                      		// returned file handle
	             (FILE_READ_DATA | SYNCHRONIZE ),   // desired access
	             poa,                				// ptr to object attributes
	             piostatus,                         // ptr to I/O status block
	             NULL,                              // allocation size
	             FILE_ATTRIBUTE_NORMAL,             // file attributes
	             0,                                 // share access
	             FILE_OPEN_IF,                      // create disposition
	             FILE_SYNCHRONOUS_IO_NONALERT,      // create options
	             NULL,                              // ptr to extended attributes
	             0);                                // length of ea buffer

	if(!NT_SUCCESS(status)){
		DbgPrint("\n1.NtCreateFile system call in read delete log failed. \
		    Ret status = 0x%0x\n", status);
		goto End;
	}

	if(!NT_SUCCESS(piostatus->Status)) {
		// DbgPrint("\n2.NTCreateFile)CREATE failed with status = 0x%0x",pIosb->status);
		status = piostatus->Status;
		goto End;
	}

	//DbgPrint("Open VM Deleted Log File Successfully\n");
	hashEntryLog = ExAllocatePoolWithTag(PagedPool, sizeof(HASH_ENTRY_LOG),
	    FVM_TABLE_POOL_TAG);

	do {
		RtlZeroMemory(hashEntryLog, sizeof(HASH_ENTRY_LOG));

		status = ZwReadFile(*pvfile,               // file Handle
		             0,                            // event Handle
		             NULL,                         // APC entry point
		             NULL,                         // APC context
		             piostatus,                    // IOSB address
		             hashEntryLog, 				   // ptr to data buffer
		             sizeof(HASH_ENTRY_LOG),   	   // length
		             NULL,                         // byte offset
		             NULL);                        // key
		if(!NT_SUCCESS(status)) {
			CHAR buffer[64];
			FvmUtil_ErrorString(status, buffer);
			break;
		} else {

			 FvmTable_DeleteLogAdd(hashEntryLog->RealName, vmNumber);
		}
	}while(1);

	End:
	if(hashEntryLog) {
		ExFreePool(hashEntryLog);
	}

	if(*pvfile){
	  winNtCloseProc(*pvfile);
	}
    memsize = 0;
	FvmVm_FreeVirtualMemory(NtCurrentProcess(), pvfile, &memsize, MEM_RELEASE);
	return status;
}


FVM_PDeleteLogEntry FvmTable_DeleteLogLookup(IN PWCHAR filename, IN ULONG vmid)
{
	FVM_PDeleteLogEntry delEntry = NULL;
	int k = 1, nLen = wcslen(filename);

	KeEnterCriticalRegion();
	ExAcquireResourceSharedLite(&fvm_DeleteLogResource, TRUE);

	delEntry = fvm_DeleteState[vmid];
	while (delEntry) {
		k = _wcsnicmp(delEntry->Name, filename, nLen);
		if (k == 0 && wcslen(delEntry->Name) == nLen) {
			break;
		} else if (k < 0) {
			delEntry = delEntry->Next;
		} else {
			delEntry = NULL;
		 	break;
		}
	}

	ExReleaseResourceLite( &fvm_DeleteLogResource );
	KeLeaveCriticalRegion();

	return delEntry;
}

VOID FvmTable_DeleteLogAdd(IN PWCHAR filename, IN ULONG vmid)
{
	FVM_PDeleteLogEntry newEntry;
	NTSTATUS rc;
	ULONG memsize, memsize1;
	POBJECT_ATTRIBUTES poa = NULL;
	PFILE_BASIC_INFORMATION pfilebasicinfo = NULL;
	int nLen = wcslen(filename);

	// Look up the file name in the existing delete log
	if (FvmTable_DeleteLogLookup(filename, vmid)) {
		//  DbgPrint("Found %S in log\n", filename);
		return;
	}

	rc = FvmUtil_InitializeVMObjAttributes(NULL, filename, &poa, &memsize);
	if (NT_SUCCESS(rc)) {
		memsize1 = sizeof(FILE_BASIC_INFORMATION);
		rc = FvmVm_AllocateVirtualMemory(NtCurrentProcess(), &pfilebasicinfo, 0,
			    &memsize1, MEM_COMMIT, PAGE_READWRITE);

		if (NT_SUCCESS(rc)) {
			 rc = ((NtQueryAttributesFileProc)(winNtQueryAttributesFileProc)) (
				      poa, pfilebasicinfo );

             memsize1 = 0;
			 FvmVm_FreeVirtualMemory(NtCurrentProcess(), &pfilebasicinfo,
			     &memsize1, MEM_RELEASE);
			 if ((rc == STATUS_OBJECT_PATH_NOT_FOUND) ||
			     (rc == STATUS_OBJECT_NAME_NOT_FOUND)) {

			        memsize = 0;
			     	FvmVm_FreeVirtualMemory(NtCurrentProcess(), &poa, &memsize,
					    MEM_RELEASE);
				    return;
			 }
		}

        memsize = 0;
		FvmVm_FreeVirtualMemory(NtCurrentProcess(), &poa, &memsize, MEM_RELEASE);
	}

	/*
	 *  Allocate a delete log entry
	 */
	newEntry = ExAllocatePoolWithTag(PagedPool,
	    sizeof(FVM_DeleteLogEntry) + nLen*2 + 2, FVM_TABLE_POOL_TAG);
	/*
	 *  If no memory for a new entry, oh well.
	 */
	if ( newEntry ) {
		FVM_PDeleteLogEntry ptr, prePtr = NULL;
		int k = 1;

		 // Fill in the new entry
		wcscpy( newEntry->Name, filename );

		// Put it in the delete log
		KeEnterCriticalRegion();
		ExAcquireResourceExclusiveLite( &fvm_DeleteLogResource, TRUE );

		ptr = fvm_DeleteState[vmid];
		while (ptr) {
			k = _wcsnicmp(ptr->Name, filename, nLen);
			if (k == 0 && wcslen(ptr->Name) == nLen) {
				break;
			} else if (k < 0) {
				prePtr = ptr;
				ptr = ptr->Next;
			} else {
				k = 1;
				break;
			}
		}

		if (k != 0) {
			newEntry->Next = ptr;
			if (prePtr) {
				prePtr->Next = newEntry;
			} else {
				fvm_DeleteState[vmid] = newEntry;
			}
		} else {
			ExFreePool(newEntry);
		}

		ExReleaseResourceLite( &fvm_DeleteLogResource );
		KeLeaveCriticalRegion();
		//DbgPrint("Delete Entry added:%S", filename);
	}
}

VOID FvmTable_DeleteLogRemove(IN PWCHAR filename, IN ULONG vmid)
{
	FVM_PDeleteLogEntry delEntry, predelEntry = NULL;
	int k = 1, nLen = wcslen(filename);

	/*
	 * Lookup the file name in the delete log to see if a name
	 * has already been generated for it
	 */
	KeEnterCriticalRegion();
	ExAcquireResourceExclusiveLite( &fvm_DeleteLogResource, TRUE );

	delEntry = fvm_DeleteState[vmid];

	while (delEntry) {
		k = _wcsnicmp(delEntry->Name, filename, nLen);
		if (k == 0 && wcslen(delEntry->Name) == nLen) {
			break;
		} else if (k < 0) {
			predelEntry = delEntry;
			delEntry = delEntry->Next;
		} else {
			k = 1;
			break;
		}
	}

	if (k == 0) {
	  if (predelEntry) {
	     predelEntry->Next = delEntry->Next;
	  } else {
	     fvm_DeleteState[vmid] = delEntry->Next;
	  }
	  ExFreePool(delEntry);
	}

	ExReleaseResourceLite( &fvm_DeleteLogResource );
	KeLeaveCriticalRegion();
}

VOID FvmTable_DeleteLogCleanup(IN ULONG vmid)
{
	FVM_PDeleteLogEntry delEntry, nextEntry;
	ULONG delLogSize = 0;

	KeEnterCriticalRegion();
	ExAcquireResourceExclusiveLite(&fvm_DeleteLogResource, TRUE);

	// Free the delete log entries
	delEntry = fvm_DeleteState[vmid];
	if (delEntry)
	  DbgPrint("Delete log for FVM-%u\n", vmid);

	while(delEntry) {
		// print the delete log
		//DbgPrint("   %S Deleted.\n", delEntry->Name);
		nextEntry = delEntry->Next;

		delLogSize += sizeof(FVM_DeleteLogEntry) +
		    wcslen(delEntry->Name)*2 + 2;

		ExFreePool(delEntry);
		delEntry = nextEntry;
	}
	fvm_DeleteState[vmid] = NULL;

	ExReleaseResourceLite( &fvm_DeleteLogResource );
	KeLeaveCriticalRegion();

	//DbgPrint("Delete Log Size: %u\n", delLogSize);
}


/******************************************************************************
 dump the delete logs into disk.
 *****************************************************************************/
void  FvmTable_WriteVMStatusFile()
{
	int pvm_index;
	try{
		for(pvm_index = 0; pvm_index < MAX_NUM_PVMS; pvm_index ++){
			if(pvms[pvm_index] != NULL){
				FvmTable_DumpVMDeleteLog(pvm_index, 1);
			}
		}
	}
	except(EXCEPTION_EXECUTE_HANDLER){
		DbgPrint(("Exception occured in UnloadDriver\n"));
	}
}

FVM_PHandleTableEntry FvmTable_HandleTableLookup(IN HANDLE hfvmdir,
    IN ULONG vmid, OUT PHANDLE hHostfile)
{
	FVM_PHandleTableEntry TableEntry = NULL;
	HANDLE pid = PsGetCurrentProcessId();

	KeEnterCriticalRegion();
	ExAcquireResourceSharedLite( &fvm_HandleTableResource, TRUE );

	TableEntry = fvm_HandleTable[vmid];
	while (TableEntry) {
		if (TableEntry->pid > pid) {
			TableEntry = NULL;
			break;
		} else if (TableEntry->pid == pid) {
			if (TableEntry->hfvm > hfvmdir) {
				TableEntry = NULL;
				break;
			} else if (TableEntry->hfvm == hfvmdir) {
				break;
			} else {
				TableEntry = TableEntry->next;
			}
		} else {
			TableEntry = TableEntry->next;
		}
	}

	if (TableEntry && hHostfile) {
		*hHostfile = TableEntry->hnative;
	}
	ExReleaseResourceLite( &fvm_HandleTableResource );
	KeLeaveCriticalRegion();

	return TableEntry;
}

FVM_PHandleTableEntry FvmTable_HandleTableAdd(IN HANDLE hfvm,
    IN HANDLE hnative, IN ULONG vmid)
{
	FVM_PHandleTableEntry newEntry;
	HANDLE pid = PsGetCurrentProcessId();

	// Look up the file handle in the handle table
	newEntry = FvmTable_HandleTableLookup(hfvm, vmid, NULL);
	if (newEntry) {
		KeEnterCriticalRegion();
		ExAcquireResourceExclusiveLite( &fvm_HandleTableResource, TRUE );

		newEntry->hnative = hnative;

		ExReleaseResourceLite( &fvm_HandleTableResource );
		KeLeaveCriticalRegion();
		return newEntry;
	}

	// Allocate a handle table entry
	newEntry = ExAllocatePoolWithTag( PagedPool, sizeof(FVM_HandleTableEntry),
	    FVM_TABLE_POOL_TAG);

	// If no memory for a new entry, oh well.
	if ( newEntry ) {
		FVM_PHandleTableEntry curPtr, prePtr = NULL;

		// Fill in the new entry
		newEntry->pid = pid;
		newEntry->hnative = hnative;
		newEntry->hfvm = hfvm;
		newEntry->namelist = NULL;
		newEntry->pathMask = NULL;

		// Put it in the handle table
		KeEnterCriticalRegion();
		ExAcquireResourceExclusiveLite( &fvm_HandleTableResource, TRUE );

		curPtr = fvm_HandleTable[vmid];
		while (curPtr) {
			if (curPtr->pid > pid) {
				break;
			} else if (curPtr->pid == pid) {
				if (curPtr->hfvm >= hfvm) {
					break;
				} else {
					prePtr = curPtr;
					curPtr = curPtr->next;
				}
			} else {
				prePtr = curPtr;
				curPtr = curPtr->next;
			}
		}

		if (!curPtr || curPtr->pid != pid || curPtr->hfvm != hfvm) {
			newEntry->next = curPtr;
			if (prePtr) {
				prePtr->next = newEntry;
			} else {
				fvm_HandleTable[vmid] = newEntry;
			}
		} else {
			ExFreePool(newEntry);
		}

		ExReleaseResourceLite( &fvm_HandleTableResource );
		KeLeaveCriticalRegion();
	}
	return newEntry;
}

VOID FvmTable_HandleTableAddName(IN PWCHAR filename, IN HANDLE hfvmdir,
    IN ULONG vmid, FVM_PHandleTableEntry *htableptr)
{
	FVM_PHandleTableEntry tableEntry = NULL;
	FVM_PNameListEntry newEntry;
	int nLen;

	if (*htableptr == NULL) {
		// Look up the file handle in the handle table
		tableEntry = FvmTable_HandleTableLookup(hfvmdir, vmid, NULL);
		if (tableEntry == NULL) {
			tableEntry = FvmTable_HandleTableAdd(hfvmdir, 0, vmid);
		}
		if (tableEntry == NULL) return;

		*htableptr = tableEntry;
	} else {
		tableEntry = *htableptr;
	}

	// Allocate a name list entry
	nLen = wcslen(filename);
	newEntry = ExAllocatePoolWithTag(PagedPool,
	    sizeof(FVM_NameListEntry) + nLen*2 + 2, FVM_TABLE_POOL_TAG);

	// If no memory for a new entry, oh well.
	if ( newEntry ) {
		FVM_PNameListEntry ptr, prePtr = NULL;
		int k = 1;

		// Fill in the new entry
		wcscpy( newEntry->Name, filename );

		// Put it in the handle table
		KeEnterCriticalRegion();
		ExAcquireResourceExclusiveLite(&fvm_HandleTableResource, TRUE);

		ptr = tableEntry->namelist;
		while (ptr) {
			k = _wcsnicmp(ptr->Name, filename, nLen);
			if (k == 0 && wcslen(ptr->Name) == nLen) {
				break;
			} else if (k < 0) {
				prePtr = ptr;
				ptr = ptr->Next;
			} else {
				k = 1;
				break;
			}
		}

		if (k != 0) {
			newEntry->Next = ptr;
			if (prePtr) {
			prePtr->Next = newEntry;
			} else {
			tableEntry->namelist = newEntry;
			}
		} else {
			ExFreePool(newEntry);
		}

		ExReleaseResourceLite(&fvm_HandleTableResource);
		KeLeaveCriticalRegion();
	}
}

FVM_PNameListEntry FvmTable_HandleTableLookupName(IN HANDLE hfvmdir,
    IN ULONG vmid, IN PWCHAR filename)
{
	FVM_PHandleTableEntry tableEntry;
	FVM_PNameListEntry nameEntry = NULL;
	int k = 1, nLen = wcslen(filename);

	tableEntry = FvmTable_HandleTableLookup(hfvmdir, vmid, NULL);
	if (tableEntry == NULL) {
		return NULL;
	} else {
		nameEntry = tableEntry->namelist;
	}

	KeEnterCriticalRegion();
	ExAcquireResourceSharedLite( &fvm_HandleTableResource, TRUE );

	while (nameEntry) {
		k = _wcsnicmp(nameEntry->Name, filename, nLen);
		if (k == 0 && wcslen(nameEntry->Name) == nLen) {
			break;
		} else if (k < 0) {
			nameEntry = nameEntry->Next;
		} else {
			nameEntry = NULL;
			break;
		}
	}

	ExReleaseResourceLite( &fvm_HandleTableResource );
	KeLeaveCriticalRegion();

	return nameEntry;
}

VOID FvmTable_HandleTableAddMask(IN PUNICODE_STRING mask, IN HANDLE hfvmdir,
    IN ULONG vmid)
{
	FVM_PHandleTableEntry tableEntry = NULL;

	if (!mask || mask->Buffer == NULL || mask->Length == 0) return;

	// Look up the file handle in the handle table
	tableEntry = FvmTable_HandleTableLookup(hfvmdir, vmid, NULL);
	if (tableEntry == NULL) {
		tableEntry = FvmTable_HandleTableAdd(hfvmdir, 0, vmid);
	}

	if (tableEntry == NULL || tableEntry->pathMask) return;

	KeEnterCriticalRegion();
	ExAcquireResourceExclusiveLite( &fvm_HandleTableResource, TRUE );

	// Allocate a path mask buffer
	tableEntry->pathMask = ExAllocatePoolWithTag( PagedPool, mask->Length + 2,
	    FVM_TABLE_POOL_TAG);

	if ( tableEntry->pathMask ) {
		wcsncpy( tableEntry->pathMask, mask->Buffer, mask->Length>>1 );
		tableEntry->pathMask[mask->Length>>1] = 0;
	}

	ExReleaseResourceLite(&fvm_HandleTableResource);
	KeLeaveCriticalRegion();
}

VOID FvmTable_HandleTableRemove(IN HANDLE hfvmdir, IN ULONG vmid)
{
	FVM_PHandleTableEntry tableEntry = NULL, preTableEntry = NULL;
	HANDLE pid = PsGetCurrentProcessId();
	FVM_PNameListEntry nameEntry, nextNameEntry;

	// Lookup the file handle in the handle table
	KeEnterCriticalRegion();
	ExAcquireResourceExclusiveLite( &fvm_HandleTableResource, TRUE );

	tableEntry = fvm_HandleTable[vmid];
	while (tableEntry) {
		if (tableEntry->pid > pid) {
			tableEntry = NULL;
			break;
		} else if (tableEntry->pid == pid) {
			if (tableEntry->hfvm > hfvmdir) {
				tableEntry = NULL;
				break;
			} else if (tableEntry->hfvm == hfvmdir) {
				break;
			} else {
				preTableEntry = tableEntry;
				tableEntry = tableEntry->next;
			}
		} else {
			 preTableEntry = tableEntry;
			 tableEntry = tableEntry->next;
		}
	}

	if (tableEntry) {
		if (tableEntry->hnative) {
			((NtCloseProc)(winNtCloseProc))(tableEntry->hnative);
		}

		nameEntry = tableEntry->namelist;
		while( nameEntry ) {
			nextNameEntry = nameEntry->Next;
			ExFreePool(nameEntry);
			nameEntry = nextNameEntry;
		}

		if (tableEntry->pathMask) {
			ExFreePool(tableEntry->pathMask);
		}

		if (preTableEntry) {
			preTableEntry->next = tableEntry->next;
		} else {
			fvm_HandleTable[vmid] = tableEntry->next;
		}

		ExFreePool(tableEntry);
	}

	ExReleaseResourceLite( &fvm_HandleTableResource );
	KeLeaveCriticalRegion();
}

VOID FvmTable_HandleTableCleanup(IN ULONG vmid)
{
	FVM_PHandleTableEntry tableEntry, nextEntry;
	FVM_PNameListEntry nameEntry, nextNameEntry;

	KeEnterCriticalRegion();
	ExAcquireResourceExclusiveLite(&fvm_HandleTableResource, TRUE);

	// Free the handle table entries
	tableEntry = fvm_HandleTable[vmid];

	while( tableEntry ) {
		if (tableEntry->hnative) {
			((NtCloseProc)(winNtCloseProc))(tableEntry->hnative);
		}

		nameEntry = tableEntry->namelist;
		while( nameEntry ) {
			nextNameEntry = nameEntry->Next;
			ExFreePool(nameEntry);
			nameEntry = nextNameEntry;
		}

		if (tableEntry->pathMask) {
			ExFreePool(tableEntry->pathMask);
		}

		nextEntry = tableEntry->next;
		ExFreePool(tableEntry);
		tableEntry = nextEntry;
	}
	fvm_HandleTable[vmid] = NULL;

	ExReleaseResourceLite( &fvm_HandleTableResource );
	KeLeaveCriticalRegion();
}

#ifdef USE_FS_MAP

// filePath "c:\program files\..."
PFVM_File_Entry FvmTable_FVMFileListLookup(PWCHAR filePath, ULONG vmID)
{
	PFVM_File_Entry dir = fvm_FileList[vmID];
	WCHAR fileBuf[_MAX_PATH];
	PWCHAR namePtr, ptr;
	int nLen, k;

	if (dir == NULL || filePath[1] != L':') return NULL;

	wcscpy(fileBuf, filePath);
	nLen = wcslen(fileBuf);
	if (fileBuf[nLen-1] == '\\') {
		fileBuf[nLen-1] = 0;
	}
	namePtr = fileBuf;

	KeEnterCriticalRegion();
	ExAcquireResourceSharedLite( &fvm_FSResource, TRUE );

	do {
		ptr = wcschr(namePtr, L'\\');
		if (ptr) {
			*ptr = 0;
		}

		while (dir) {
			k = _wcsicmp(dir->fsName, namePtr);
			if (k < 0) {
				dir = dir->nextSib;
			} else {
				break;
			}
		}
		if (dir == NULL || k > 0) {
			ExReleaseResourceLite( &fvm_FSResource );
			KeLeaveCriticalRegion();
			return NULL;
		}
		if (ptr) {
			dir = dir->nextChild;
			namePtr = ptr + 1;
		}
	} while (ptr && *namePtr);

	ExReleaseResourceLite( &fvm_FSResource );
	KeLeaveCriticalRegion();
	return dir;
}

// "C:"
PFVM_File_Entry FvmTable_FVMFileListAddDrive(PWCHAR drivePath, ULONG vmID)
{
	PFVM_File_Entry fileEntry, preSib = NULL;
	PFVM_File_Entry newEntry = NULL;
	int k;

	KeEnterCriticalRegion();
	ExAcquireResourceExclusiveLite( &fvm_FSResource, TRUE );

	fileEntry = fvm_FileList[vmID];
	while (fileEntry) {
		k = _wcsicmp(fileEntry->fsName, drivePath);
		if (k < 0) {
			preSib = fileEntry;
			fileEntry = fileEntry->nextSib;
		} else {
			break;
		}
	}

	if (k != 0 || !fileEntry) {
		newEntry = ExAllocatePoolWithTag(PagedPool, sizeof(FVM_File_Entry) +
		    wcslen(drivePath)*2, FVM_TABLE_POOL_TAG);
		if (newEntry) {
			wcscpy(newEntry->fsName, drivePath);
			newEntry->nextChild = NULL;

			newEntry->nextSib = fileEntry;
			if (preSib) {
				preSib->nextSib = newEntry;
			} else {
				fvm_FileList[vmID] = newEntry;
			}
		}
	} else {
		newEntry = fileEntry;
	}

	ExReleaseResourceLite( &fvm_FSResource );
	KeLeaveCriticalRegion();

	return newEntry;
}

/*
 * Given a directory entry and a file name, this function adds the name
 * information into the mapping list
 */

// "Program Files"
PFVM_File_Entry FvmTable_FVMFileListAdd(PFVM_File_Entry dirEntry,
    PWCHAR filePath)
{
	PFVM_File_Entry fileEntry, preSib = NULL;
	PFVM_File_Entry newEntry = NULL;
	int k;

	KeEnterCriticalRegion();
	ExAcquireResourceExclusiveLite(&fvm_FSResource, TRUE);

	fileEntry = dirEntry->nextChild;
	while (fileEntry) {
		k = _wcsicmp(fileEntry->fsName, filePath);
		if (k < 0) {
			preSib = fileEntry;
			fileEntry = fileEntry->nextSib;
		} else {
			break;
		}
	}

	if (k != 0 || !fileEntry) {
		newEntry = ExAllocatePoolWithTag(PagedPool, sizeof(FVM_File_Entry) +
		    wcslen(filePath)*2, FVM_TABLE_POOL_TAG);
		if (newEntry) {
			wcscpy(newEntry->fsName, filePath);
			newEntry->nextChild = NULL;
			newEntry->nextSib = fileEntry;
			if (preSib) {
				preSib->nextSib = newEntry;
			} else {
				dirEntry->nextChild = newEntry;
			}
		}
	} else {
		newEntry = fileEntry;
	}

	ExReleaseResourceLite( &fvm_FSResource );
	KeLeaveCriticalRegion();

	return newEntry;
}

/*
 * Given a full file path, this function adds the path information
 * into the mapping list
 */

// filePath "c:\program files\..."
VOID FvmTable_FVMFileListAddFullPath(PWCHAR filePath, ULONG vmID)
{
	PWCHAR ptr, namePtr;
	WCHAR filePathBuf[_MAX_PATH];
	PFVM_File_Entry fileEntry;

	wcscpy(filePathBuf, filePath);
	namePtr = wcschr(filePathBuf, L':');
	if (!namePtr) {
		return;
	}
	namePtr--;

	ptr = wcschr(namePtr, L'\\');
	if (ptr) {
		*ptr = 0;
	}

	fileEntry = FvmTable_FVMFileListAddDrive(namePtr, vmID);

	while (ptr && fileEntry) {
		namePtr = ptr+1;
		if (*namePtr == 0) break;

		ptr = wcschr(namePtr, L'\\');
		if (ptr) {
			*ptr = 0;
		}
		fileEntry = FvmTable_FVMFileListAdd(fileEntry, namePtr);
	}
}

/*
 * Given a full file path, this function removes the path information
 * from the mapping list
 */

// filePath "c:\program files\..."
BOOLEAN FvmTable_FVMFileListDelete(PWCHAR filePath, ULONG vmID)
{
	PFVM_File_Entry dir = fvm_FileList[vmID];
	PFVM_File_Entry preSib = NULL, parent = NULL;
	PWCHAR namePtr, ptr;
	WCHAR fileBuf[_MAX_PATH];
	int nLen, k;

	if (dir == NULL || filePath[1] != L':') return FALSE;

	wcscpy(fileBuf, filePath);
	nLen = wcslen(fileBuf);
	if (fileBuf[nLen-1] == '\\') {
		fileBuf[nLen-1] = 0;
	}
	namePtr = fileBuf;

	KeEnterCriticalRegion();
	ExAcquireResourceExclusiveLite( &fvm_FSResource, TRUE );

	do {
		ptr = wcschr(namePtr, L'\\');
		if (ptr) {
			*ptr = 0;
		}

		while (dir) {
			k = _wcsicmp(dir->fsName, namePtr);
			if (k < 0) {
				preSib = dir;
				dir = dir->nextSib;
			} else {
				break;
			}
		}
		if (dir == NULL || k > 0) {
			ExReleaseResourceLite( &fvm_FSResource );
			KeLeaveCriticalRegion();
			return FALSE;
		}
		if (ptr) {
			parent = dir;
			preSib = NULL;
			dir = dir->nextChild;
			namePtr = ptr + 1;
		}
	} while (ptr && *namePtr);

	if (dir->nextChild) {
		DbgPrint("Cannot remove a non-empty directory!\n");
		ExReleaseResourceLite( &fvm_FSResource );
		KeLeaveCriticalRegion();
		return FALSE;
	}

	if (preSib) {
		preSib->nextSib = dir->nextSib;
	} else if (parent) {
		parent->nextChild = dir->nextSib;
	} else {
		fvm_FileList[vmID] = dir->nextSib;
	}

	ExFreePool(dir);

	ExReleaseResourceLite( &fvm_FSResource );
	KeLeaveCriticalRegion();
	return TRUE;
}

ULONG fileMapSize = 0;

VOID FVMFileListCleanupNode(PFVM_File_Entry node)
{
	PFVM_File_Entry current, head = node;

	if (node == NULL) return;

	while (head) {
		current = head;
		head = head->nextSib;
		FVMFileListCleanupNode(current->nextChild);

		fileMapSize += sizeof(FVM_File_Entry) + wcslen(current->fsName)*2;

		ExFreePool(current);
	}
}

VOID FvmTable_FVMFileListCleanup(ULONG vmID)
{
	PFVM_File_Entry head = fvm_FileList[vmID];
	PFVM_File_Entry current;

	KeEnterCriticalRegion();
	ExAcquireResourceExclusiveLite( &fvm_FSResource, TRUE );

	while (head) {
		current = head;
		head = head->nextSib;
		FVMFileListCleanupNode(current->nextChild);

		fileMapSize += sizeof(FVM_File_Entry) + wcslen(current->fsName)*2;

		ExFreePool(current);
	}
	fvm_FileList[vmID] = NULL;

	ExReleaseResourceLite( &fvm_FSResource );
	KeLeaveCriticalRegion();

	//DbgPrint("File Map Size:%u\n", fileMapSize);
	fileMapSize = 0;
}


VOID
FvmTable_FVMFileListEnumDir(PWCHAR dirName,
                            PFILE_BOTH_DIR_INFORMATION dirInfoPtr,
                            ULONG infoSize,
                            ULONG vmID,
                            PFVM_File_Entry parentEntry)
{
   NTSTATUS rc;
   UNICODE_STRING uniDirName;
   OBJECT_ATTRIBUTES objAttr;
   HANDLE hFile;
   IO_STATUS_BLOCK ioStatus;
   BOOLEAN restartScan = TRUE;
   WCHAR fileName[_MAX_PATH+1];
   WCHAR partialName[_MAX_PATH];
   PFVM_File_Entry fileEntry = NULL;

   RtlInitUnicodeString(&uniDirName, dirName);

	InitializeObjectAttributes(&objAttr, &uniDirName,
         OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE, NULL, NULL);

   rc = ZwOpenFile(&hFile,
				FILE_READ_DATA|FILE_LIST_DIRECTORY|SYNCHRONIZE,
				&objAttr,
				&ioStatus,
				FILE_SHARE_READ,
				FILE_DIRECTORY_FILE|FILE_SYNCHRONOUS_IO_NONALERT);

   if (!NT_SUCCESS(rc)) {
      return;
   }

   while (TRUE) {
      rc = ZwQueryDirectoryFile(hFile, NULL, NULL, NULL, &ioStatus,
            (PVOID)dirInfoPtr, infoSize, FileBothDirectoryInformation,
            TRUE, NULL, restartScan);

      restartScan = FALSE;

      if (NT_SUCCESS(rc)) {
         if (dirInfoPtr->FileName[0] == L'.' &&
             (dirInfoPtr->FileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            continue;
         }

         wcsncpy(partialName, dirInfoPtr->FileName, dirInfoPtr->FileNameLength >> 1);
         partialName[dirInfoPtr->FileNameLength >> 1] = 0;

         _snwprintf(fileName, _MAX_PATH, L"%s\\%s", dirName, partialName);

         //DbgPrint("Restore Name:%S\n", partialName);
         if (parentEntry == NULL) {
            wcscat(partialName, L":");
            fileEntry = FvmTable_FVMFileListAddDrive(partialName, vmID);
         } else {
            fileEntry = FvmTable_FVMFileListAdd(parentEntry, partialName);
         }

         if (dirInfoPtr->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            FvmTable_FVMFileListEnumDir(fileName, dirInfoPtr, infoSize,
                  vmID, fileEntry);
         }
      } else {
         break;
      }
   }
   ZwClose(hFile);
}

VOID FvmTable_FVMFileListInit(ULONG vmID)
{
	WCHAR rootPath[_MAX_PATH];
   PFILE_BOTH_DIR_INFORMATION dirInfoPtr = NULL;

   dirInfoPtr = ExAllocatePoolWithTag(PagedPool, sizeof(FILE_BOTH_DIR_INFORMATION)
         + 512, FVM_TABLE_POOL_TAG);
   if (dirInfoPtr == NULL) {
      return;
   }

	_snwprintf(rootPath, _MAX_PATH, L"\\??\\%s\\%s", pvms[vmID]->fileRoot,
			pvms[vmID]->idStr);

	FvmTable_FVMFileListEnumDir(rootPath, dirInfoPtr,
	      sizeof(FILE_BOTH_DIR_INFORMATION) + 512, vmID, NULL);

   ExFreePool(dirInfoPtr);
}

#endif
