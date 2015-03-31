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
 * fvm_reg.h --
 *
 * FVM registry virtualization header file.
 *
 * This header file describes prototypes of registry-related system
 * call functions implemented by the FVM layer, and a few functions
 * that translates the virtual registry paths.
 */

#ifndef _FVM_REG_H
#define _FVM_REG_H

/*
 * FVM interception functions for standard Windows registry
 * API's.
 */
extern NTSTATUS FvmReg_NtOpenKey(
	OUT PHANDLE KeyHandle,
	IN ACCESS_MASK ReqAccess,
	IN POBJECT_ATTRIBUTES ObjectAttributes
);

extern NTSTATUS FvmReg_NtCreateKey(
	OUT PHANDLE KeyHandle,
	IN ACCESS_MASK ReqAccess,
	IN POBJECT_ATTRIBUTES ObjectAttributes,
	IN ULONG TitleIndex,
	IN PUNICODE_STRING Class,
	IN ULONG CreateOptions,
	OUT PULONG Disposition
);

extern NTSTATUS FvmReg_NtQueryKey(
	IN HANDLE  KeyHandle,
	IN KEY_INFORMATION_CLASS  KeyInformationClass,
	OUT PVOID  KeyInformation,
	IN ULONG  Length,
	OUT PULONG  ResultLength
);

extern NTSTATUS FvmReg_NtDeleteKey(IN HANDLE KeyHandle);

extern NTSTATUS FvmReg_NtFlushKey(IN HANDLE KeyHandle);

extern NTSTATUS FvmReg_NtLoadKey(
	IN POBJECT_ATTRIBUTES TargetKey,
	IN POBJECT_ATTRIBUTES HiveFile
);

extern NTSTATUS FvmReg_NtUnloadKey(IN POBJECT_ATTRIBUTES TargetKey);

extern NTSTATUS FvmReg_NtEnumerateKey(
	IN HANDLE KeyHandle,
	IN ULONG Index,
	IN KEY_INFORMATION_CLASS KeyInformationClass,
	OUT PVOID KeyInformation,
	IN ULONG Length,
	OUT PULONG ResultLength
);

extern NTSTATUS FvmReg_NtDeleteValueKey(
	IN HANDLE KeyHandle,
	PUNICODE_STRING Name
);

extern NTSTATUS FvmReg_NtSetValueKey(
	IN HANDLE KeyHandle,
	IN PUNICODE_STRING ValueName,
	IN ULONG TitleIndex,
	IN ULONG Type,
	IN PVOID Data,
	IN ULONG DataSize
);

extern NTSTATUS FvmReg_NtQueryValueKey(
	IN HANDLE KeyHandle,
	IN PUNICODE_STRING ValueName,
	IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
	OUT PVOID KeyValueInformation,
	IN ULONG Length,
	OUT PULONG  ResultLength
);

extern NTSTATUS FvmReg_NtEnumerateValueKey(
	IN HANDLE KeyHandle,
	IN ULONG Index,
	IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
	OUT PVOID KeyValueInformation,
	IN ULONG Length,
	OUT PULONG  ResultLength
);

#endif //_FVM_REG_H
