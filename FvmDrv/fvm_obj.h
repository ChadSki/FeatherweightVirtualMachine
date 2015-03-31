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
 *  FVM object virtualization header
 */

#ifndef _FVM_OBJ_H
#define _FVM_OBJ_H

#include "fvm_syscalls.h"
#include "fvm_vm.h"

extern HANDLE  FVM_ObjectDirectoryHandle[MAX_VM];
extern HANDLE  FVM_PortDirectoryHandle[MAX_VM];

NTSTATUS FvmObj_NtCreateMutant(
   OUT PHANDLE hMutex,
   IN ACCESS_MASK desiredAccess,
   IN POBJECT_ATTRIBUTES objectAttributes,
   IN BOOLEAN bOwnMutant
);

NTSTATUS FvmObj_NtOpenMutant(
   OUT PHANDLE hMutex,
   IN ACCESS_MASK desiredAccess,
   IN POBJECT_ATTRIBUTES objectAttributes
);

NTSTATUS FvmObj_NtCreateSemaphore(
   OUT PHANDLE hSemaphore,
   IN ACCESS_MASK desiredAccess,
   IN POBJECT_ATTRIBUTES objectAttributes,
   IN ULONG initialCount,
   IN ULONG maximumCount
);

NTSTATUS FvmObj_NtOpenSemaphore(
   OUT PHANDLE hSemaphore,
   IN ACCESS_MASK desiredAccess,
   IN POBJECT_ATTRIBUTES objectAttributes
);

NTSTATUS FvmObj_NtCreateEvent(
   OUT PHANDLE hEvent,
   IN ACCESS_MASK desiredAccess,
   IN POBJECT_ATTRIBUTES objectAttributes,
   IN EVENT_TYPE eventType,
   IN BOOLEAN bInitialState
);

NTSTATUS FvmObj_NtOpenEvent(
   OUT PHANDLE hEvent,
   IN ACCESS_MASK desiredAccess,
   IN POBJECT_ATTRIBUTES objectAttributes
);

NTSTATUS FvmObj_NtCreateSection(
   OUT PHANDLE phSection,
   IN ACCESS_MASK desiredAccess,
   IN POBJECT_ATTRIBUTES objectAttributes,
   IN PLARGE_INTEGER MaximumSize OPTIONAL,
   IN ULONG sectionPageProtection,
   IN ULONG allocationAttributes,
   IN HANDLE hFile OPTIONAL
);

NTSTATUS FvmObj_NtOpenSection(
   OUT PHANDLE phSection,
   IN ACCESS_MASK desiredAccess,
   IN POBJECT_ATTRIBUTES objectAttributes
);

NTSTATUS FvmObj_NtCreateTimer(
   OUT PHANDLE phTimer,
   IN ACCESS_MASK desiredAccess,
   IN POBJECT_ATTRIBUTES objectAttributes,
   IN TIMER_TYPE timerType
);

NTSTATUS FvmObj_NtOpenTimer(
   OUT PHANDLE phTimer,
   IN ACCESS_MASK desiredAccess,
   IN POBJECT_ATTRIBUTES objectAttributes
);

NTSTATUS FvmObj_NtCreateIoCompletion(
   OUT PHANDLE phIoCompletionPort,
   IN ACCESS_MASK desiredAccess,
   IN POBJECT_ATTRIBUTES objectAttributes,
   IN ULONG nConcurrentThreads
);

NTSTATUS FvmObj_NtOpenIoCompletion(
   OUT PHANDLE phIoCompletionPort,
   IN ACCESS_MASK desiredAccess,
   IN POBJECT_ATTRIBUTES objectAttributes
);

NTSTATUS FvmObj_NtCreateEventPair(
   OUT PHANDLE hEventPair,
   IN ACCESS_MASK desiredAccess,
   IN POBJECT_ATTRIBUTES objectAttributes
);

NTSTATUS FvmObj_NtOpenEventPair(
   OUT PHANDLE hEventPair,
   IN ACCESS_MASK desiredAccess,
   IN POBJECT_ATTRIBUTES objectAttributes
);

NTSTATUS FvmObj_NtOpenProcess(
	OUT PHANDLE pHandle,
	IN ACCESS_MASK DesiredAccess,
	IN POBJECT_ATTRIBUTES ObjectAttributes,
	IN PCLIENT_ID ClientID
);

NTSTATUS FvmObj_NtCreatePort(
   OUT PHANDLE portHandle,
   IN POBJECT_ATTRIBUTES objectAttributes,
   IN ULONG maxDataSize,
   IN ULONG maxMessageSize,
   IN ULONG reserved
);

NTSTATUS FvmObj_NtCreateWaitablePort(
   OUT PHANDLE portHandle,
   IN POBJECT_ATTRIBUTES objectAttributes,
   IN ULONG maxDataSize,
   IN ULONG maxMessageSize,
   IN ULONG reserved
);

NTSTATUS FvmObj_NtConnectPort(
   OUT PHANDLE portHandle,
   IN PUNICODE_STRING portName,
   IN PSECURITY_QUALITY_OF_SERVICE securityQos,
   IN OUT PPORT_SECTION_WRITE WriteSection OPTIONAL,
   IN OUT PPORT_SECTION_READ ReadSection OPTIONAL,
   OUT PULONG MaxMessageSize OPTIONAL,
   IN OUT PVOID ConnectData OPTIONAL,
   IN OUT PULONG ConnectDataLength OPTIONAL
);

NTSTATUS FvmObj_NtSecureConnectPort(
   OUT PHANDLE portHandle,
   IN PUNICODE_STRING portName,
   IN PSECURITY_QUALITY_OF_SERVICE securityQos,
   IN OUT PPORT_SECTION_WRITE WriteSection OPTIONAL,
   IN PSID ServerSid OPTIONAL,
   IN OUT PPORT_SECTION_READ ReadSection OPTIONAL,
   OUT PULONG MaxMessageSize OPTIONAL,
   IN OUT PVOID ConnectData OPTIONAL,
   IN OUT PULONG ConnectDataLength OPTIONAL
);

NTSTATUS FvmObj_NtCreateDirectoryObject(
   OUT PHANDLE directoryHandle,
   IN ACCESS_MASK desiredAccess,
   IN POBJECT_ATTRIBUTES objectAttributes
);

NTSTATUS FvmObj_NtOpenDirectoryObject(
   OUT PHANDLE directoryHandle,
   IN ACCESS_MASK desiredAccess,
   IN POBJECT_ATTRIBUTES objectAttributes
);

NTSTATUS FvmObj_NtCreateSymbolicLinkObject(
   OUT PHANDLE hSymbolicLink,
   IN ACCESS_MASK desiredAccess,
   IN POBJECT_ATTRIBUTES objectAttributes,
   IN PUNICODE_STRING targetName
);

NTSTATUS FvmObj_NtOpenSymbolicLinkObject(
   OUT PHANDLE hSymbolicLink,
   IN ACCESS_MASK desiredAccess,
   IN POBJECT_ATTRIBUTES objectAttributes
);

NTSTATUS FvmObj_NtClose(HANDLE handle);

NTSTATUS FvmObj_CreateObjDirectory(ULONG vmid);

NTSTATUS FvmObj_CreatePortDirectory(ULONG vmid);

#endif //_FVM_OBJ_H
