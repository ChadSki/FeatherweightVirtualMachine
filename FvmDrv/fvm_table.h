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
 * fvm_table.h
 *
 * Header file consisting of the FVM data structures.
 */

#ifndef _FVM_TABLE_H
#define _FVM_TABLE_H

#include "ntddk.h"
#include <stdio.h>
#include <stdlib.h>
#include "hooksys.h"
#include "fvm_vm.h"

/*
 * Structure for the delete log and name list (QueryDirectory) information.
 * This structure is used for storing information about deletions made to
 * the VMs file system.
 */

typedef struct _FVM_DeleteLog_NameList_Entry {
	struct _FVM_DeleteLog_NameList_Entry *Next;
	WCHAR Name[];
} FVM_DeleteLogEntry, *FVM_PDeleteLogEntry, FVM_NameListEntry,
      *FVM_PNameListEntry;


/*
 * This structure is used to manage the handles for the Native OS and the
 * VMs, which are used to for directory listings from the VMs.
 */

typedef struct _FVM_Handle_Table_Entry {
	HANDLE pid;
	HANDLE hfvm;
	FVM_PNameListEntry namelist;
	HANDLE hnative;
	PWCHAR pathMask;
	struct _FVM_Handle_Table_Entry *next;
} FVM_HandleTableEntry, *FVM_PHandleTableEntry;

extern ERESOURCE fvm_DeleteLogResource, fvm_HandleTableResource;
extern FVM_PDeleteLogEntry     *fvm_DeleteState;
extern FVM_PHandleTableEntry   *fvm_HandleTable;

FVM_PDeleteLogEntry FvmTable_DeleteLogLookup(
	IN PWCHAR filename,
	IN ULONG vmid
);

FVM_PHandleTableEntry FvmTable_HandleTableLookup(
	IN HANDLE hfvmdir,
	IN ULONG vmid,
	OUT PHANDLE hhostfile
);

FVM_PNameListEntry FvmTable_HandleTableLookupName(
	IN HANDLE hfvmdir,
	IN ULONG vmid,
	IN PWCHAR filename
);

FVM_PHandleTableEntry FvmTable_HandleTableAdd(
	IN HANDLE hfvm,
	IN HANDLE hnative,
	IN ULONG vmid
);

VOID FvmTable_DeleteLogAdd(IN PWCHAR vilename, IN ULONG vmid);

VOID FvmTable_DeleteLogRemove(IN PWCHAR lookupname, IN ULONG vmid);

VOID FvmTable_DeleteLogCleanup(IN ULONG vmid);

VOID FvmTable_WriteVMStatusFile();

NTSTATUS FvmTable_ReadVMDeletedLogFile(INT vmNumber);

VOID FvmTable_DumpVMDeleteLog(INT vmNumber, INT type);

VOID FvmTable_HandleTableAddName(
	IN PWCHAR filename,
	IN HANDLE hnative,
	IN ULONG vmid,
	FVM_PHandleTableEntry *htableptr
);

VOID FvmTable_HandleTableAddMask(
	IN PUNICODE_STRING mask,
	IN HANDLE hfvmdir,
	IN ULONG vmid
);

VOID FvmTable_HandleTableRemove(IN HANDLE hfvm, IN ULONG vmid);

VOID FvmTable_HandleTableCleanup(IN ULONG vmid);


/*
 *  This is for the optional feature of storing information in kernel memory
 *  about private FS namespace of VMs. This includes new and modified files
 *  specific to a VM. If this feature is not conditionally turned on, then
 *  directory information is retrieved from the kernel via system calls.
 */

#ifdef USE_FS_MAP

/* This structure is used to manage information about the private FS
  * namespace of a VM.
  */

typedef struct _FVM_File_Entry {
	struct _FVM_File_Entry *nextSib;
	struct _FVM_File_Entry *nextChild;
	WCHAR fsName[1];
} FVM_File_Entry, *PFVM_File_Entry;

extern PFVM_File_Entry *fvm_FileList;

extern ERESOURCE fvm_FSResource;

PFVM_File_Entry FvmTable_FVMFileListAdd(PFVM_File_Entry dirEntry,
    PWCHAR filePath);

PFVM_File_Entry FvmTable_FVMFileListAddDrive(PWCHAR drivePath, ULONG vmID);

PFVM_File_Entry FvmTable_FVMFileListLookup(PWCHAR filePath, ULONG vmID);

VOID FvmTable_FVMFileListAddFullPath(PWCHAR filePath, ULONG vmID);

BOOLEAN FvmTable_FVMFileListDelete(PWCHAR filePath, ULONG vmID);

VOID FvmTable_FVMFileListInit(ULONG vmID);

VOID FvmTable_FVMFileListCleanup(ULONG vmID);

#endif

#endif //_FVM_TABLE_H
