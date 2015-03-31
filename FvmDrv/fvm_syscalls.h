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
 * fvm_syscalls.h --
 *
 *	This header file describes the NT system call table, prototypes
 *  of system calls, and structures of system call arguments. It also
 *  defines global variables that store the addresses of the original
 *  system call functions.
 */

#ifndef _FVM_SYSCALLS_H_
#define _FVM_SYSCALLS_H_


/*
 * Macros used to output debug messages in intercepted system calls
 */

#define DBG_CREATEFILE                    0
#define DBG_OPENFILE                      0
#define DBG_QUERYATTRIBUTESFILE           0
#define DBG_QUERYFULLATTRIBUTESFILE       0
#define DBG_DELETEFILE                    0
#define DBG_QUERYDIRECTORYFILE            0
#define DBG_QUERYINFORMATIONFILE          0
#define DBG_QUERYVOLUMEINFORMATIONFILE    0
#define DBG_SETINFORMATIONFILE            0
#define DBG_CREATENAMEDPIPEFILE           0
#define DBG_CREATEMAILSLOTFILE            0

#define DBG_CREATEMUTANT                  0
#define DBG_OPENMUTANT                    0
#define DBG_CREATESEMAPHORE               0
#define DBG_OPENSEMAPHORE                 0
#define DBG_CREATEEVENT                   0
#define DBG_OPENEVENT                     0
#define DBG_CREATETIMER                   0
#define DBG_OPENTIMER                     0
#define DBG_CREATEIOCOMPLETION            0
#define DBG_OPENIOCOMPLETION              0
#define DBG_CREATEEVENTPAIR               0
#define DBG_OPENEVENTPAIR                 0

#define DBG_CREATEPORT                    0
#define DBG_CREATEWAITABLEPORT            0
#define DBG_CONNECTPORT                   0
#define DBG_SECURECONNECTPORT             0

#define DBG_CREATESECTION                 0
#define DBG_OPENSECTION                   0

#define DBG_CLOSE                         0

#define DBG_CREATEKEY                     0
#define DBG_OPENKEY                       0
#define DBG_QUERYKEY                      0
#define DBG_DELETEKEY                     0
#define DBG_ENUMERATEKEY                  0


/*
 * The structure of NT system call table
 */

typedef struct _ServiceDescriptorTable {
	UINT *ServiceTableBase;        /* Base address of the system call table */
	UINT *ServiceCounterTableBase; /* Number of system call invocation */
	UINT NumberOfServices;         /* Number of system calls in the table */
	PUCHAR ParamTableBase;         /* Base address of parameter bytes table */
} ServiceDescriptorTable;


/*
 * KeServiceDescriptorTable is the entry to the system call table,
 * exported by ntoskrnl.exe.
 */

__declspec(dllimport) ServiceDescriptorTable KeServiceDescriptorTable;


/*
 * Macros that retrieve address of system call functions from the system
 * call table. SystemService(sysCall) requires the address of exported
 * Zw-like kernel functions. When such kernel functions are not exported,
 * please use SystemServiceByNo(sysCallNo), which requires the ID of system
 * calls in the system call table. The system call ID can be obtained by
 * disassembling Ntdll.dll.
 */

#define SystemService(sysCall) \
	KeServiceDescriptorTable.ServiceTableBase[*(PULONG)((PUCHAR)sysCall+1)]

#define SystemServiceByNo(sysCallNo) \
	KeServiceDescriptorTable.ServiceTableBase[sysCallNo]


/*
 * The prototypes of a set of NT system services, and structures of system
 * call arguments. These declarations are mostly copied from the book of
 * "Windows NT/2K Native API Reference".
 */

NTSYSAPI
NTSTATUS
NTAPI
ZwAllocateVirtualMemory(
	IN HANDLE ProcessHandle,
	IN OUT PVOID *BaseAddress,
	IN ULONG ZeroBits,
	IN OUT PULONG AllocationSize,
	IN ULONG AllocationType,
	IN ULONG Protect
);

NTSYSAPI
NTSTATUS
NTAPI
ZwFreeVirtualMemory(
	IN HANDLE ProcessHandle,
	IN OUT PVOID *BaseAddress,
	IN OUT PULONG FreeSize,
	IN ULONG FreeType
);

NTSYSAPI
NTSTATUS
NTAPI
ZwQueryInformationProcess(
	IN HANDLE ProcessHandle,
	IN PROCESSINFOCLASS ProcessInfoClass,
	OUT PVOID ProcessInfoBuffer,
	IN ULONG ProcessInfoBufferLength,
	OUT PULONG BytesReturned OPTIONAL
);

NTSYSAPI
NTSTATUS
NTAPI
ZwQueryDirectoryFile(
	IN HANDLE FileHandle,
	IN HANDLE EventHandle OPTIONAL,
	IN PIO_APC_ROUTINE IoApcRoutine OPTIONAL,
	IN PVOID IoApcContext OPTIONAL,
	OUT PIO_STATUS_BLOCK IoStatusBlock,
	OUT PVOID FileInformationBuffer,
	IN ULONG FileInformationBufferLength,
	IN FILE_INFORMATION_CLASS FileInfoClass,
	IN BOOLEAN ReturnOnlyOneEntry,
	IN PUNICODE_STRING PathMask OPTIONAL,
	IN BOOLEAN RestartQuery
);

NTSYSAPI
NTSTATUS
NTAPI
ZwQueryVolumeInformationFile(
	IN HANDLE FileHandle,
	OUT PIO_STATUS_BLOCK IoStatusBlock,
	OUT PVOID VolumeInformation,
	IN ULONG VolumeInformationLength,
	IN FS_INFORMATION_CLASS VolumeInformationClass
);

NTSYSAPI
NTSTATUS
NTAPI
ZwDeleteFile(
	IN POBJECT_ATTRIBUTES ObjectAttributes
);

NTSYSAPI
NTSTATUS
NTAPI
ZwCreateEvent(
	OUT PHANDLE EventHandle,
	IN ACCESS_MASK DesiredAccess,
	IN POBJECT_ATTRIBUTES ObjectAttributes,
	IN EVENT_TYPE EventType,
	IN BOOLEAN InitialState
);

NTSYSAPI
NTSTATUS
NTAPI
ZwOpenEvent(
	OUT PHANDLE EventHandle,
	IN ACCESS_MASK DesiredAccess,
	IN POBJECT_ATTRIBUTES ObjectAttributes
);

NTSYSAPI
NTSTATUS
NTAPI
ZwOpenDirectoryObject(
	OUT PHANDLE DirectoryHandle,
	IN ACCESS_MASK DesiredAccess,
	IN POBJECT_ATTRIBUTES ObjectAttributes
);

NTSYSAPI
NTSTATUS
NTAPI
ZwCreateSymbolicLinkObject(
	OUT PHANDLE SymbolicLinkHandle,
	IN ACCESS_MASK DesiredAccess,
	IN POBJECT_ATTRIBUTES ObjectAttributes,
	IN PUNICODE_STRING TargetName
);

NTSYSAPI
NTSTATUS
NTAPI
ZwOpenSymbolicLinkObject(
	OUT PHANDLE SymbolicLinkHandle,
	IN ACCESS_MASK DesiredAccess,
	IN POBJECT_ATTRIBUTES ObjectAttributes
);

/*
NTSYSAPI
NTSTATUS
NTAPI
NtCloseProc(
	IN HANDLE Handle
);
*/

typedef struct _PORT_SECTION_WRITE {
	ULONG Length;
	HANDLE SectionHandle;
	ULONG SectionOffset;
	ULONG ViewSize;
	PVOID ViewBase;
	PVOID TargetViewBase;
} PORT_SECTION_WRITE, *PPORT_SECTION_WRITE;

typedef struct _PORT_SECTION_READ {
	ULONG Length;
	ULONG ViewSize;
	PVOID ViewBase;
} PORT_SECTION_READ, *PPORT_SECTION_READ;

NTSYSAPI
NTSTATUS
NTAPI
ZwConnectPort(
	OUT PHANDLE PortHandle,
	IN PUNICODE_STRING PortName,
	IN PSECURITY_QUALITY_OF_SERVICE SecurityQos,
	IN OUT PPORT_SECTION_WRITE WriteSection OPTIONAL,
	IN OUT PPORT_SECTION_READ ReadSection OPTIONAL,
	OUT PULONG MaxMessageSize OPTIONAL,
	IN OUT PVOID ConnectData OPTIONAL,
	IN OUT PULONG ConnectDataLength OPTIONAL
);

NTSYSAPI
NTSTATUS
NTAPI
ZwDeleteValueKey(
	IN HANDLE KeyHandle,
	IN PUNICODE_STRING ValueName
);

NTSYSAPI
NTSTATUS
NTAPI
ZwLoadDriver(
	IN PUNICODE_STRING DriverRegistryEntry
);

NTSYSAPI
NTSTATUS
NTAPI
ZwUnloadDriver(
	IN PUNICODE_STRING DriverRegistryEntry
);


/*
 *  Prototypes of system calls, structures declarations and macros
 *  related to NT security.
 */

typedef struct _SID_IDENTIFIER_AUTHORITY {
	UCHAR Value[6];
} SID_IDENTIFIER_AUTHORITY, *PSID_IDENTIFIER_AUTHORITY;

typedef struct _SID {
	BYTE Revision;
	BYTE SubAuthorityCount;
	SID_IDENTIFIER_AUTHORITY IdentifierAuthority;
	ULONG SubAuthority[1];
} SID, *PISID;

typedef struct _SID_AND_ATTRIBUTES {
	SID *Sid;
	ULONG Attributes;
} SID_AND_ATTRIBUTES, *PSID_AND_ATTRIBUTES;

typedef struct _TOKEN_USER {
	SID_AND_ATTRIBUTES User;
} TOKEN_USER, *PTOKEN_USER;

typedef struct _TOKEN_GROUPS {
	ULONG GroupCount;
	SID_AND_ATTRIBUTES Groups[ANYSIZE_ARRAY];
} TOKEN_GROUPS, *PTOKEN_GROUPS;

typedef enum _TOKEN_INFORMATION_CLASS {
	TokenUser = 1,
	TokenGroups,
	TokenPrivileges,
	TokenOwner,
	TokenPrimaryGroup,
	TokenDefaultDacl,
	TokenSource,
	TokenType,
	TokenImpersonationLevel,
	TokenStatistics
} TOKEN_INFORMATION_CLASS, *PTOKEN_INFORMATION_CLASS;

typedef struct _TOKEN_PRIVILEGES {
	ULONG PrivilegeCount;
	LUID_AND_ATTRIBUTES Privileges[ANYSIZE_ARRAY];
} TOKEN_PRIVILEGES, *PTOKEN_PRIVILEGES;

#define TOKEN_ASSIGN_PRIMARY    (0x0001)
#define TOKEN_DUPLICATE         (0x0002)
#define TOKEN_IMPERSONATE       (0x0004)
#define TOKEN_QUERY             (0x0008)
#define TOKEN_QUERY_SOURCE      (0x0010)
#define TOKEN_ADJUST_PRIVILEGES (0x0020)
#define TOKEN_ADJUST_GROUPS     (0x0040)
#define TOKEN_ADJUST_DEFAULT    (0x0080)

#define TOKEN_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED  |\
                          TOKEN_ASSIGN_PRIMARY      |\
                          TOKEN_DUPLICATE           |\
                          TOKEN_IMPERSONATE         |\
                          TOKEN_QUERY               |\
                          TOKEN_QUERY_SOURCE        |\
                          TOKEN_ADJUST_PRIVILEGES   |\
                          TOKEN_ADJUST_GROUPS       |\
                          TOKEN_ADJUST_DEFAULT)

#define TOKEN_READ       (STANDARD_RIGHTS_READ      |\
                          TOKEN_QUERY)

#define TOKEN_WRITE      (STANDARD_RIGHTS_WRITE     |\
                          TOKEN_ADJUST_PRIVILEGES   |\
                          TOKEN_ADJUST_GROUPS       |\
                          TOKEN_ADJUST_DEFAULT)

#define TOKEN_EXECUTE    (STANDARD_RIGHTS_EXECUTE)

typedef enum _TOKEN_TYPE {
	TokenPrimary = 1,
	TokenImpersonation
} TOKEN_TYPE, *PTOKEN_TYPE;

NTSYSAPI
NTSTATUS
NTAPI
ZwQueryInformationToken(
	IN HANDLE TokenHandle,
	IN TOKEN_INFORMATION_CLASS TokenInfoClass,
	OUT PVOID TokenInfoBuffer,
	IN ULONG TokenInfoBufferLength,
	OUT PULONG BytesReturned
);

NTSYSAPI
NTSTATUS
NTAPI
ZwOpenProcess(
	OUT PHANDLE ProcessHandle,
	IN ACCESS_MASK AccessMask,
	IN POBJECT_ATTRIBUTES ObjectAttributes,
	IN PCLIENT_ID ClientId
);

NTSYSAPI
NTSTATUS
NTAPI
ZwOpenProcessToken(
	IN HANDLE ProcessHandle,
	IN ACCESS_MASK DesiredAccess,
	OUT PHANDLE TokenHandle
);

NTSYSAPI
NTSTATUS
NTAPI
ZwOpenThreadToken(
	IN HANDLE ThreadHandle,
	IN ACCESS_MASK DesiredAccess,
	IN BOOLEAN UseContextOfProcess,
	OUT PHANDLE TokenHandle
);


/*
 * File information structure declarations used in file-related
 * system calls
 */

typedef struct _FILE_DIRECTORY_INFORMATION {
	ULONG NextEntryOffset;
	ULONG FileIndex;
	LARGE_INTEGER CreationTime;
	LARGE_INTEGER LastAccessTime;
	LARGE_INTEGER LastWriteTime;
	LARGE_INTEGER ChangeTime;
	LARGE_INTEGER EndOfFile;
	LARGE_INTEGER AllocationSize;
	ULONG FileAttributes;
	ULONG FileNameLength;
	WCHAR FileName[1];
} FILE_DIRECTORY_INFORMATION, *PFILE_DIRECTORY_INFORMATION;

typedef struct _FILE_FULL_DIR_INFORMATION {
	ULONG NextEntryOffset;
	ULONG FileIndex;
	LARGE_INTEGER CreationTime;
	LARGE_INTEGER LastAccessTime;
	LARGE_INTEGER LastWriteTime;
	LARGE_INTEGER ChangeTime;
	LARGE_INTEGER EndOfFile;
	LARGE_INTEGER AllocationSize;
	ULONG FileAttributes;
	ULONG FileNameLength;
	ULONG EaSize;
	WCHAR FileName[1];
} FILE_FULL_DIR_INFORMATION, *PFILE_FULL_DIR_INFORMATION;

typedef struct _FILE_BOTH_DIR_INFORMATION {
	ULONG NextEntryOffset;
	ULONG FileIndex;
	LARGE_INTEGER CreationTime;
	LARGE_INTEGER LastAccessTime;
	LARGE_INTEGER LastWriteTime;
	LARGE_INTEGER ChangeTime;
	LARGE_INTEGER EndOfFile;
	LARGE_INTEGER AllocationSize;
	ULONG FileAttributes;
	ULONG FileNameLength;
	ULONG EaSize;
	CCHAR ShortNameLength;
	WCHAR ShortName[12];
	WCHAR FileName[1];
} FILE_BOTH_DIR_INFORMATION, *PFILE_BOTH_DIR_INFORMATION;

typedef struct _FILE_NAMES_INFORMATION {
	ULONG NextEntryOffset;
	ULONG FileIndex;
	ULONG FileNameLength;
	WCHAR FileName[1];
} FILE_NAMES_INFORMATION, *PFILE_NAMES_INFORMATION;

typedef struct _FILE_LINK_RENAME_INFORMATION {
	BOOLEAN ReplaceIfExists;
	HANDLE RootDirectory;
	ULONG FileNameLength;
	WCHAR FileName[1];
} FILE_LINK_INFORMATION, *PFILE_LINK_INFORMATION,
  FILE_RENAME_INFORMATION, *PFILE_RENAME_INFORMATION;


/*
 * Global variables to store function addresses of original NT
 * system calls
 */

/* File-related system calls */
typedef NTSTATUS (*NtCreateFileProc)(
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
NtCreateFileProc winNtCreateFileProc;

typedef NTSTATUS (*NtOpenFileProc)(
	PHANDLE FileHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	PIO_STATUS_BLOCK IoStatusBlock,
	ULONG ShareMode,
	ULONG OpenMode
);
NtOpenFileProc winNtOpenFileProc;

typedef NTSTATUS (*NtQueryAttributesFileProc)(
	POBJECT_ATTRIBUTES ObjectAttributes,
	PFILE_BASIC_INFORMATION FileInformation
);
NtQueryAttributesFileProc winNtQueryAttributesFileProc;

typedef NTSTATUS (*NtQueryFullAttributesFileProc)(
	POBJECT_ATTRIBUTES ObjectAttributes,
	PFILE_NETWORK_OPEN_INFORMATION FileInformation
);
NtQueryFullAttributesFileProc winNtQueryFullAttributesFileProc;

typedef NTSTATUS (*NtQueryDirectoryFileProc)(
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
NtQueryDirectoryFileProc winNtQueryDirectoryFileProc;

typedef NTSTATUS (*NtQueryInformationFileProc)(
	HANDLE FileHandle,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID FileInformationBuffer,
	ULONG FileInformationBufferLength,
	FILE_INFORMATION_CLASS FileInfoClass
);
NtQueryInformationFileProc winNtQueryInformationFileProc;

typedef NTSTATUS (*NtSetInformationFileProc)(
	HANDLE FileHandle,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID FileInformationBuffer,
	ULONG FileInformationBufferLength,
	FILE_INFORMATION_CLASS FileInfoClass
);
NtSetInformationFileProc winNtSetInformationFileProc;

typedef NTSTATUS (*NtQueryVolumeInformationFileProc)(
	HANDLE FileHandle,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID VolumeInformation,
	ULONG VolumeInformationLength,
	FS_INFORMATION_CLASS VolumeInformationClass
);
NtQueryVolumeInformationFileProc winNtQueryVolumeInformationFileProc;

typedef NTSTATUS (*NtDeleteFileProc)(
	POBJECT_ATTRIBUTES ObjectAttributes
);
NtDeleteFileProc winNtDeleteFileProc;

typedef NTSTATUS (*NtCreateNamedPipeFileProc)(
	PHANDLE FileHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	PIO_STATUS_BLOCK IoStatusBlock,
	ULONG ShareAccess,
	ULONG CreateDisposition,
	ULONG CreateOptions,
	ULONG TypeMessage,
	ULONG ReadModeMessage,
	ULONG Nonblocking,
	ULONG MaxInstances,
	ULONG InBufferSize,
	ULONG OutBufferSize,
	PLARGE_INTEGER DefaultTimeOut OPTIONAL
);
NtCreateNamedPipeFileProc winNtCreateNamedPipeFileProc;

typedef NTSTATUS (*NtCreateMailSlotFileProc)(
	PHANDLE FileHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	PIO_STATUS_BLOCK IoStatusBlock,
	ULONG CreateOptions,
	ULONG InBufferSize,
	ULONG MaxMessageSize,
	PLARGE_INTEGER ReadTimeout OPTIONAL
);
NtCreateMailSlotFileProc winNtCreateMailslotFileProc;
/* File-related system calls end */

/* Object-related system calls */
typedef NTSTATUS (*NtCreateMutantProc)(
	PHANDLE MutexHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	BOOLEAN OwnMutant
);
NtCreateMutantProc winNtCreateMutantProc;

typedef NTSTATUS (*NtOpenMutantProc)(
	PHANDLE MutexHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes
);
NtOpenMutantProc winNtOpenMutantProc;

typedef NTSTATUS (*NtCreateSemaphoreProc)(
	PHANDLE SemaphoreHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	ULONG InitialCount,
	ULONG MaximumCount
);
NtCreateSemaphoreProc winNtCreateSemaphoreProc;

typedef NTSTATUS (*NtOpenSemaphoreProc)(
	PHANDLE SemaphoreHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes
);
NtOpenSemaphoreProc winNtOpenSemaphoreProc;

typedef NTSTATUS (*NtCreateEventProc)(
	PHANDLE EventHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	EVENT_TYPE EventType,
	BOOLEAN InitialState
);
NtCreateEventProc winNtCreateEventProc;

typedef NTSTATUS (*NtOpenEventProc)(
	PHANDLE EventHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes
);
NtOpenEventProc winNtOpenEventProc;

typedef NTSTATUS (*NtCreateSectionProc)(
	PHANDLE SectionHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	PLARGE_INTEGER MaximumSize OPTIONAL,
	ULONG SectionPageProtection,
	ULONG AllocationAttributes,
	HANDLE FileHandle OPTIONAL
);
NtCreateSectionProc winNtCreateSectionProc;

typedef NTSTATUS(*NtOpenSectionProc)(
	PHANDLE SectionHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes
);
NtOpenSectionProc winNtOpenSectionProc;

typedef NTSTATUS (*NtCreateTimerProc)(
	PHANDLE TimerHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	TIMER_TYPE TimerType
);
NtCreateTimerProc winNtCreateTimerProc;

typedef NTSTATUS (*NtOpenTimerProc)(
	PHANDLE TimerHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes
);
NtOpenTimerProc winNtOpenTimerProc;

typedef NTSTATUS (*NtCreateIOCompletionProc)(
	PHANDLE IoCompletionPortHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	ULONG ConcurrentThreads
);
NtCreateIOCompletionProc winNtCreateIOCompletionProc;

typedef NTSTATUS (*NtOpenIOCompletionProc)(
	PHANDLE IoCompletionPortHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes
);
NtOpenIOCompletionProc winNtOpenIOCompletionProc;

typedef NTSTATUS (*NtCreateEventPairProc)(
	PHANDLE EventPairHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes
);
NtCreateEventPairProc winNtCreateEventPairProc;

typedef NTSTATUS (*NtOpenEventPairProc)(
	PHANDLE EventPairProc,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes
);
NtOpenEventPairProc winNtOpenEventPairProc;

typedef NTSTATUS (*NtCreatePortProc)(
	PHANDLE PortHandle,
	POBJECT_ATTRIBUTES ObjectAttributes,
	ULONG MaxDataSize,
	ULONG MaxMessageSize,
	ULONG Reserved
);
NtCreatePortProc winNtCreatePortProc;

typedef NTSTATUS (*NtCreateWaitablePortProc)(
	PHANDLE PortHandle,
	POBJECT_ATTRIBUTES ObjectAttributes,
	ULONG MaxDataSize,
	ULONG MaxMessageSize,
	ULONG Reserved
);
NtCreateWaitablePortProc winNtCreateWaitablePortProc;

typedef NTSTATUS (*NtConnectPortProc)(
	PHANDLE PortHandle,
	PUNICODE_STRING PortName,
	PSECURITY_QUALITY_OF_SERVICE SecurityQos,
	PPORT_SECTION_WRITE WriteSection OPTIONAL,
	PPORT_SECTION_READ ReadSection OPTIONAL,
	PULONG MaxMessageSize OPTIONAL,
	PVOID ConnectData OPTIONAL,
	PULONG ConnectDataLength OPTIONAL
);
NtConnectPortProc winNtConnectPortProc;

typedef NTSTATUS (*NtSecureConnectPortProc)(
	PHANDLE PortHandle,
	PUNICODE_STRING PortName,
	PSECURITY_QUALITY_OF_SERVICE SecurityQos,
	PPORT_SECTION_WRITE WriteSection OPTIONAL,
	PSID ServerSid OPTIONAL,
	PPORT_SECTION_READ ReadSection OPTIONAL,
	PULONG MaxMessageSize OPTIONAL,
	PVOID ConnectData OPTIONAL,
	PULONG ConnectDataLength OPTIONAL
);
NtSecureConnectPortProc winNtSecureConnectPortProc;

typedef NTSTATUS (*NtCreateDirectoryObjectProc)(
	PHANDLE DirectoryHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes
);
NtCreateDirectoryObjectProc winNtCreateDirectoryObjectProc;

typedef NTSTATUS (*NtOpenDirectoryObjectProc)(
	PHANDLE DirectoryHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes
);
NtOpenDirectoryObjectProc winNtOpenDirectoryObjectProc;

typedef NTSTATUS (*NtCreateSymbolicLinkObjectProc)(
	PHANDLE SymbolicLinkHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	PUNICODE_STRING TargetName
);
NtCreateSymbolicLinkObjectProc winNtCreateSymbolicLinkObjectProc;

typedef NTSTATUS (*NtOpenSymbolicLinkObjectProc)(
	PHANDLE SymbolicLinkHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes
);
NtOpenSymbolicLinkObjectProc winNtOpenSymbolicLinkObjectProc;

typedef NTSTATUS (*NtCloseProc)(
	HANDLE Handle
);
NtCloseProc winNtCloseProc;
/* Object-related system calls end */

/* Registry-related system calls */
typedef NTSTATUS (*NtCreateKeyProc)(
	PHANDLE KeyHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	ULONG TitleIndex,
	PUNICODE_STRING Class,
	ULONG CreateOptions,
	PULONG Disposition
);
NtCreateKeyProc winNtCreateKeyProc;

typedef NTSTATUS (*NtOpenKeyProc)(
	PHANDLE KeyHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes
);
NtOpenKeyProc winNtOpenKeyProc;

typedef NTSTATUS (*NtQueryKeyProc)(
	HANDLE KeyHandle,
	KEY_INFORMATION_CLASS KeyInformationClass,
	PVOID KeyInformation,
	ULONG Length,
	PULONG ResultLength
);
NtQueryKeyProc winNtQueryKeyProc;

typedef NTSTATUS (*NtDeleteKeyProc)(
	HANDLE KeyHandle
);
NtDeleteKeyProc winNtDeleteKeyProc;

typedef NTSTATUS (*NtFlushKeyProc)(
	HANDLE KeyHandle
);
NtFlushKeyProc winNtFlushKeyProc;

typedef NTSTATUS (*NtLoadKeyProc)(
	POBJECT_ATTRIBUTES TargetKey,
	POBJECT_ATTRIBUTES HiveFile
);
NtLoadKeyProc winNtLoadKeyProc;

typedef NTSTATUS (*NtUnloadKeyProc)(
	POBJECT_ATTRIBUTES TargetKey
);
NtUnloadKeyProc winNtUnloadKeyProc;

typedef NTSTATUS (*NtEnumerateKeyProc)(
	HANDLE KeyHandle,
	ULONG Index,
	KEY_INFORMATION_CLASS KeyInformationClass,
	PVOID KeyInformation,
	ULONG Length,
	PULONG ResultLength
);
NtEnumerateKeyProc winNtEnumerateKeyProc;

typedef NTSTATUS (*NtDeleteValueKeyProc)(
	HANDLE KeyHandle,
	PUNICODE_STRING Name
);
NtDeleteValueKeyProc winNtDeleteValueKeyProc;

typedef NTSTATUS (*NtSetValueKeyProc)(
	HANDLE KeyHandle,
	PUNICODE_STRING ValueName,
	ULONG TitleIndex,
	ULONG Type,
	PVOID Data,
	ULONG DataSize
);
NtSetValueKeyProc winNtSetValueKeyProc;

typedef NTSTATUS (*NtQueryValueKeyProc)(
	HANDLE KeyHandle,
	PUNICODE_STRING ValueName,
	KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
	PVOID KeyValueInformation,
	ULONG Length,
	PULONG ResultLength
);
NtQueryValueKeyProc winNtQueryValueKeyProc;

typedef NTSTATUS (*NtEnumerateValueKeyProc)(
	HANDLE KeyHandle,
	ULONG Index,
	KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
	PVOID KeyValueInformation,
	ULONG Length,
	PULONG ResultLength
);
NtEnumerateValueKeyProc winNtEnumerateValueKeyProc;
/* Registry-related system calls end */

/* Other system calls used in FVM layer */
typedef NTSTATUS (*NtLoadDriverProc)(
	PUNICODE_STRING DriverRegistryEntry
);
NtLoadDriverProc winNtLoadDriverProc;

typedef NTSTATUS (*NtUnloadDriverProc)(
	PUNICODE_STRING DriverRegistryEntry
);
NtUnloadDriverProc winNtUnloadDriverProc;

typedef struct _USER_STACK {
	PVOID FixedStackBase;
	PVOID FixedStackLimit;
	PVOID ExpandableStackBase;
	PVOID ExpandableStackLimit;
	PVOID ExpandableStackBottom;
} USER_STACK, *PUSER_STACK;

typedef NTSTATUS (*NtCreateThreadProc)(
	PHANDLE ThreadHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	HANDLE ProcessHandle,
	PCLIENT_ID ClientId,
	PCONTEXT ThreadContext,
	PUSER_STACK UserStack,
	BOOLEAN CreateSuspended
);
NtCreateThreadProc winNtCreateThreadProc;

typedef struct _THREAD_BASIC_INFORMATION {
	NTSTATUS ExitStatus;
	PVOID TebBaseAddress;
	CLIENT_ID ClientId;
	KAFFINITY AffinityMask;
	KPRIORITY Priority;
	KPRIORITY BasePriority;
} THREAD_BASIC_INFORMATION, *PTHREAD_BASIC_INFORMATION;

typedef NTSTATUS (*NtQueryInformationThreadProc)(
	HANDLE ThreadHandle,
	THREADINFOCLASS ThreadInformationClass,
	PVOID ThreadInformation,
	ULONG ThreadInformationLength,
	PULONG ReturnLength
);
NtQueryInformationThreadProc winNtQueryInformationThreadProc;

typedef NTSTATUS (*NtResumeThreadProc)(
	HANDLE ThreadHandle,
	PULONG PreviousSuspendCount
);
NtResumeThreadProc winNtResumeThreadProc;
#if 1
typedef NTSTATUS (*NTOpenProcess)(
    OUT PHANDLE  ProcessHandle,
    IN ACCESS_MASK  DesiredAccess,
    IN POBJECT_ATTRIBUTES  ObjectAttributes,
    IN PCLIENT_ID  ClientId OPTIONAL
);
NTOpenProcess WinNtOpenProcess;
#endif

typedef NTSTATUS (*NtReadVirtualMemoryProc)(
	HANDLE ProcessHandle,
	PVOID BaseAddress,
	PVOID Buffer,
	ULONG BytesToRead,
	PULONG BytesRead
);
NtReadVirtualMemoryProc winNtReadVirtualMemoryProc;
/* Other system calls end */

#endif // ifndef _FVM_SYSCALLS_H_
