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
 * fvm_util.h
 *
 * General purpose utility function used by other modules of FVM (like
 * the file redirection/registry redirection modules).
 */
 
#ifndef _FVM_UTIL_H
#define _FVM_UTIL_H

extern NTSTATUS ObQueryNameString(void *, void *, int , int *);

NTSTATUS PsLookupProcessByProcessId(IN ULONG ProcId,
		    OUT PEPROCESS * PeProcess);
int FvmUtil_VmNumber();

VOID FvmUtil_GetBinaryPathName(PWCHAR BinaryPath);

VOID FvmUtil_GetBinaryPathName0(PWCHAR BinaryPath, int Cat);

NTSTATUS FvmUtil_GetSysCallArgument1(OUT PWCHAR Dest, OUT PWCHAR LinkName,
		    IN POBJECT_ATTRIBUTES ObjectAttributes
);


BOOLEAN FvmUtil_GetSysCallArgument(IN POBJECT_ATTRIBUTES ObjectAttributes,
		    OUT PWCHAR Dest);

BOOLEAN IsFileSystemAccess(PWCHAR Arg);

BOOLEAN FvmIsLocalFileAccess(PWCHAR FileName);

BOOLEAN FvmUtil_PathFromHandle(HANDLE Key, PUNICODE_STRING lpszSubKeyVal,
		    PWCHAR Fullname);

NTSTATUS GetDriveLetterLinkTarget(IN PWCHAR SourceNameStr,
		    OUT PWCHAR LinkTarget
);

NTSTATUS FvmUtil_GetDriveFromDevice(IN PWCHAR DeviceNameStr,
		    OUT PWCHAR DriveNameStr);


NTSTATUS ExpandSymLink(PWCHAR Arg);

NTSTATUS FvmUtil_InitializeVMObjAttributes(POBJECT_ATTRIBUTES ObjectAttributes,
		    PWCHAR Vdirname, POBJECT_ATTRIBUTES *Ppoa, PULONG Pmemsize);

VOID FvmUtil_CreateFileDir(PWCHAR Sourname, PWCHAR Destname, ULONG Vmid);

NTSTATUS FvmUtil_CopyFiletoVM(IN POBJECT_ATTRIBUTES ObjectAttributes,
		    IN PWCHAR Sourname, IN PWCHAR Destname, IN BOOLEAN Isdir,
		    IN BOOLEAN Opencall, ULONG Vmid);

NTSTATUS FvmUtil_FindProcessStrSID(PWCHAR SidBuffer, PULONG SidBufferLength);

PCHAR FvmUtil_ErrorString(NTSTATUS RetStat, PCHAR Buffer);

PCHAR FvmUtil_AccessString(ACCESS_MASK DesiredAccess, PCHAR Access_str);

unsigned __int64 GetCycleCount(void);
typedef struct {
	ULONG	AllocationSize;
	ULONG	ActualSize;
	ULONG	Flags;
	ULONG	Unknown1;
	UNICODE_STRING	Unknown2;
	HANDLE	InputHandle;
	HANDLE	OutputHandle;
	HANDLE	ErrorHandle;
	UNICODE_STRING	CurrentDirectory;
	HANDLE	CurrentDirectoryHandle;
	UNICODE_STRING	SearchPaths;
	UNICODE_STRING	ApplicationName;
	UNICODE_STRING	CommandLine;
	PVOID	EnvironmentBlock;
	ULONG	Unknown[9];
	UNICODE_STRING	Unknown3;
	UNICODE_STRING	Unknown4;
	UNICODE_STRING	Unknown5;
	UNICODE_STRING	Unknown6;
} FVM_PROCESS_PARAMETERS, *PFVM_PROCESS_PARAMETERS;

typedef struct _PEB {
	ULONG	AllocationSize;
	ULONG	Unknown1;
	HANDLE	ProcessHinstance;
	PVOID	ListDlls;
	PFVM_PROCESS_PARAMETERS	ProcessParameters;
	ULONG	Unknown2;
	HANDLE	Heap;
} PEB;

#endif //_FVM_UTIL_H
