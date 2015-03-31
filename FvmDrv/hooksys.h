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
 * hooksys.h
 *
 * Header file for the FVM driver
 */

#ifndef _HOOKSYS_H
#define _HOOKSYS_H

#define USE_FS_MAP

/*
 * Error codes for VM creation
 */

#define CREATE_VM_SUCCESS 1  /* VM created successfully */
#define CREATE_VM_RUNNING 2  /* VM is running */
#define CREATE_VM_MAXVMER 3  /* Max. number of VMs reached */
#define CREATE_VM_MEMERRO 4  /* Failed to allocate memory */

/*
 * Error codes for adding a fvmshell
 */

#define ADD_SHELL_SUCCESS 1
#define ADD_SHELL_VMNOTFD 2 /* VM is not running */

/*
 * Error codes for terminating a fvmshell
 */

#define TERMINATE_VM_SUCCESS 1
#define TERMINATE_VM_PROCESS 2
#define TERMINATE_VM_NOTFUND 3
#define TERMINATE_VM_PERDENY 4

#define FILE_DEVICE_HOOKSYS  0x00008300
#define DRIVER_DEVICE_NAME   L"hooksys"

#define IO_REFERENCE_EVENT         (ULONG) CTL_CODE(FILE_DEVICE_HOOKSYS, 0x801, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IO_QUERY_CLIENTID          (ULONG) CTL_CODE(FILE_DEVICE_HOOKSYS, 0x806, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IO_RESUME_PROCESS          (ULONG) CTL_CODE(FILE_DEVICE_HOOKSYS, 0x807, METHOD_NEITHER, FILE_ANY_ACCESS)

/*
 * VM information query operation
 */

#define IO_QUERY_VM_LIST           (ULONG) CTL_CODE(FILE_DEVICE_HOOKSYS, 0x821, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IO_QUERY_PROCESS_LIST      (ULONG) CTL_CODE(FILE_DEVICE_HOOKSYS, 0x822, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IO_QUERY_SERVICE_LIST      (ULONG) CTL_CODE(FILE_DEVICE_HOOKSYS, 0x823, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IO_QUERY_SIDE_EFFECT_LIST  (ULONG) CTL_CODE(FILE_DEVICE_HOOKSYS, 0x824, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IO_QUERY_DELETED_FILE_LIST (ULONG) CTL_CODE(FILE_DEVICE_HOOKSYS, 0x825, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IO_QUERY_DELETED_REG_LIST  (ULONG) CTL_CODE(FILE_DEVICE_HOOKSYS, 0x826, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IO_QUERY_DELETED_OBJ_LIST  (ULONG) CTL_CODE(FILE_DEVICE_HOOKSYS, 0x827, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IO_QUERY_IS_SAME_VM        (ULONG) CTL_CODE(FILE_DEVICE_HOOKSYS, 0x828, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IO_QUERY_VM_ID             (ULONG) CTL_CODE(FILE_DEVICE_HOOKSYS, 0x829, METHOD_NEITHER, FILE_ANY_ACCESS)

/*
 * VM manipulation operation
 */

#define IO_CREATE_VM               (ULONG) CTL_CODE(FILE_DEVICE_HOOKSYS, 0x831, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IO_COPY_VM                 (ULONG) CTL_CODE(FILE_DEVICE_HOOKSYS, 0x832, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IO_CONFIGURE_VM            (ULONG) CTL_CODE(FILE_DEVICE_HOOKSYS, 0x833, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IO_COMMIT_VM               (ULONG) CTL_CODE(FILE_DEVICE_HOOKSYS, 0x834, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IO_RESUME_VM               (ULONG) CTL_CODE(FILE_DEVICE_HOOKSYS, 0x835, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IO_STATUS_VM               (ULONG) CTL_CODE(FILE_DEVICE_HOOKSYS, 0x836, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IO_TERMINATE_VM            (ULONG) CTL_CODE(FILE_DEVICE_HOOKSYS, 0x837, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IO_SERVICE_VM              (ULONG) CTL_CODE(FILE_DEVICE_HOOKSYS, 0x838, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IO_ASSOCIATE_VM	           (ULONG) CTL_CODE(FILE_DEVICE_HOOKSYS, 0x839, METHOD_NEITHER, FILE_ANY_ACCESS)

#define IO_QUERY_VM_IP             (ULONG) CTL_CODE(FILE_DEVICE_HOOKSYS, 0x841, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IO_QUERY_VM_IP_CONTEXT     (ULONG) CTL_CODE(FILE_DEVICE_HOOKSYS, 0x842, METHOD_NEITHER, FILE_ANY_ACCESS)

#define NT_PROCNAMELEN  16
#define MAXPROCNAMELEN  32

/*
 * The maximum registry path length that will be copied
 */

#define MAXPATHLEN     1024

#define SYSNAME "System"
#define CONF_OBJ_PATH    L"\\BaseNamedObjects\\"
#define CONF_PORT_PATH   L"\\RPC Control\\"
#define SERVICE_KEY      L"\\REGISTRY\\MACHINE\\System\\CurrentControlSet\\Control\\ServiceCurrent"
#define WIS_IMAGE_NAME   L"msiexec.exe"
#define IE_IMAGE_NAME    L"iexplore.exe"
#define FVM_DAEMON_NAME  L"\\fvmserv.exe"

typedef unsigned int  UINT;
typedef char          CHAR;
typedef char *        PCHAR;
typedef PVOID         POBJECT;
typedef unsigned char byte;
typedef byte          BYTE;
typedef int           INT;

extern ULONG      fvm_Calls_In_Progress;

extern int msi_owner;
extern LONG num_process;
extern LONG service_pid;
extern int is_added;

/*
 * This structure is used to manage the DDE communication information
 * for Window messaging.
 */

typedef struct _FVM_DDEProcessIDInfo {
   ULONG client_pid;
   ULONG server_pid;
} FVM_DDEProcessIDInfo, *FVM_PDDEProcessIDInfo;

#endif //_HOOKSYS_H
