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
 * fvm_file.h --
 *
 *	This header file describes prototypes of file-related system
 *  call functions implemented by the FVM layer, and a few functions
 *  that map a virtual file path to a physical file path.
 */

#ifndef _FVM_FILE_H_
#define _FVM_FILE_H_


/*
 * Prototypes of file-related system call functions in FVM. Most
 * copied from Windows NT/2K Native API Reference.
 */

NTSTATUS FvmFile_NtCreateFile(
	PHANDLE FileHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	PIO_STATUS_BLOCK IoStatusBlock,
	PLARGE_INTEGER AllocationSize OPTIONAL,
	ULONG FileAttributes,
	ULONG ShareAccess,
	ULONG CreateDisposition,
	ULONG CreateOptions,
	PVOID EaBuffer OPTIONAL,
	ULONG EaLength
);

NTSTATUS FvmFile_NtOpenFile(
	PHANDLE FileHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	PIO_STATUS_BLOCK IoStatusBlock,
	ULONG ShareMode,
	ULONG OpenMode
);

NTSTATUS FvmFile_NtQueryAttributesFile(
	POBJECT_ATTRIBUTES ObjectAttributes,
	PFILE_BASIC_INFORMATION FileInformation
);

NTSTATUS FvmFile_NtQueryFullAttributesFile(
	POBJECT_ATTRIBUTES ObjectAttributes,
	PFILE_NETWORK_OPEN_INFORMATION FileInformation
);

NTSTATUS FvmFile_NtQueryDirectoryFile(
	HANDLE FileHandle,
	HANDLE EventHandle OPTIONAL,
	PIO_APC_ROUTINE IoApcRoutine OPTIONAL,
	PVOID IoApcContext OPTIONAL,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID FileInformationBuffer,
	ULONG FileInformationBufferLength,
	FILE_INFORMATION_CLASS FileInfoClass,
	BOOLEAN ReturnOnlyOneEntry,
	PUNICODE_STRING PathMask OPTIONAL,
	BOOLEAN RestartQuery
);

NTSTATUS FvmFile_NtQueryInformationFile(
	HANDLE FileHandle,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID FileInformationBuffer,
	ULONG FileInformationBufferLength,
	FILE_INFORMATION_CLASS FileInfoClass
);

NTSTATUS FvmFile_NtSetInformationFile(
	HANDLE FileHandle,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID FileInformationBuffer,
	ULONG FileInformationBufferLength,
	FILE_INFORMATION_CLASS FileInfoClass
);

NTSTATUS FvmFile_NtQueryVolumeInformationFile(
	HANDLE FileHandle,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID VolumeInformation,
	ULONG VolumeInformationLength,
	FS_INFORMATION_CLASS VolumeInformationClass
);

NTSTATUS FvmFile_NtDeleteFile(
	POBJECT_ATTRIBUTES ObjectAttributes
);

NTSTATUS FvmFile_NtCreateNamedPipeFile(
	PHANDLE FileHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	PIO_STATUS_BLOCK IoStatusBlock,
	ULONG ShareAccess,
	ULONG CreateDisposition,
	ULONG CreateOptions,
	ULONG NamedPipeType,
	ULONG ReadMode,
	ULONG CompletionMode,
	ULONG MaxInstances,
	ULONG InBufferSize,
	ULONG OutBufferSize,
	PLARGE_INTEGER DefaultTimeOut
);

NTSTATUS FvmFile_NtCreateMailslotFile(
	PHANDLE FileHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	PIO_STATUS_BLOCK IoStatusBlock,
	ULONG CreateOptions,
	ULONG InBufferSize,
	ULONG MaxMessageSize,
	PLARGE_INTEGER ReadTimeOut OPTIONAL
);


/*
 * Prototypes of FVM's file manipulation functions.
 */

NTSTATUS FvmFile_CreateFVMRootDir(ULONG VmId);

BOOLEAN FvmFile_MapPath(
	PWCHAR SourceName,
	ULONG VmId,
	PWCHAR DestName
);

BOOLEAN FvmFile_MapPipeMailSlotPath(
	PWCHAR SourceName,
	ULONG VmId,
	PWCHAR DestName
);

#endif // ifndef _FVM_FILE_H_
