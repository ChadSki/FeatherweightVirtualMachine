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
 * fvm_vm.h --
 *
 * This header file describes the FVM Virtual Machine Monitor related
 * API's.
 */

#ifndef _FVM_VM_H
#define _FVM_VM_H

#define INVALID_VMID 0xffffffff

/*
  * Maximum number of Virtual machines
  */
#define MAX_VM  0x100

extern struct FVM_new_process_t *FVM_new_processes;
extern ERESOURCE FVM_newProcessResource;
extern ERESOURCE FVM_processVMRes;
extern ULONG BufferSize;

/*
  * FVM new process creation data structures.
  */
struct FVM_new_process_t{
	ULONG pid;
	PKEVENT dllLoad;
	PWCHAR imageName;	
	struct FVM_new_process_t *next;
	struct FVM_new_process_t *prev;
};


/*
  * FVM domain related data structures
  */
typedef struct FVM_pvm {
	ULONG  id;        //domain identifier
	ULONG  n_procs;   //current number of processes in the PVM
	int     status;    //current status of PVM, 1:running, 0:stop, 2:suspend
	WCHAR  name[256];   //name of the virtual machine, e.g. PVM1, PVM2, etc.

	WCHAR fileRoot[256];     //root directory for the fvm
	WCHAR fileRootLink[512]; //symboliclink for the root directory.
	WCHAR idStr[35];         //every fvm has a unique id string.
							
	WCHAR SidStr[60];        //string id of a sid.

	ULONG  pvm_vmIp;        //ip addr
	ULONG  pvm_vmContext;   //ip addr context, removes alias when vm stops

} FVM_PSEUDO_VIRTUAL_MACHINE, *PFVM_PSEUDO_VIRTUAL_MACHINE;


#define MAX_NUM_PVMS 60

/*
  * Global pseudo virtual machine array
  */
PFVM_PSEUDO_VIRTUAL_MACHINE pvms[MAX_NUM_PVMS];

/*
  * The current number of pseudo VM's
  */
UINT n_pvms;

/*
  * Hash table: This maps pid to domain.
  */
typedef struct FVM_pvmtbl FVM_PVM_TBL_ENTRY, *PFVM_PVM_TBL_ENTRY;

/*
  * Pseudo VM table.
  */
struct FVM_pvmtbl {
	ULONG pvm_id;
	ULONG pid;
	PFVM_PVM_TBL_ENTRY next;
	PKEVENT dllLoad;
	PWCHAR imageName;
	PVOID PBuffer;
	PVOID PLitBuffer;
	int BufMap; // (0~15) little memory, (31) large memory	
	BOOLEAN BufferAllocated;
};

/*
  * The maximum pseudo VM table entries
  */
#define MAX_TBL_ENTRIES    1024

PFVM_PVM_TBL_ENTRY FVM_pvm_pid_tbl[MAX_TBL_ENTRIES];

#define hash_pid(pid)      (pid%MAX_TBL_ENTRIES)


/*
  * Pseudo VM creation/deletion/search routines.
  */
extern UINT FvmVm_CreatePVM(ULONG *ptr_pvm_id, PWCHAR pvm_name, PWCHAR idStr,
	    PWCHAR root, ULONG vmIp, ULONG vmContext);
extern UINT FvmVm_DestroyPVM(ULONG pvm_id);
extern UINT FvmVm_FindPVM(ULONG *ptr_pvm_id, PWCHAR pvm_name);

/*
  * Get pseudo Virtual machine from the PVM table.
  * This function returns INVALID_VMID if the pid is not present in the table
  */
extern ULONG FvmVm_GetPVMId(ULONG pid);

/*
  * Routines to add, remove and update the process to/from a domain.
  */
extern void FvmVm_AddPid(ULONG pid, ULONG pvm_id);
extern void  FvmVm_RemovePid(ULONG pid);
extern BOOLEAN FvmVm_UpdateProcessStatus(ULONG pid);
extern void FvmVm_addNewProcess(ULONG Pid);
extern void FvmVm_removeNewProcess(ULONG Pid);

extern struct FVM_new_process_t *FvmVm_CheckProcessStatus(ULONG pid,
	    PWCHAR imageName);

PWCHAR FvmVm_FindProcessCreateImage(ULONG pid);

NTSTATUS FvmVm_AllocateVirtualMemory(IN HANDLE ProcessHandle, IN OUT PVOID *BaseAddress,
	     IN ULONG ZeroBits,	IN OUT PULONG AllocationSize,	IN ULONG AllocationType,	IN ULONG Protect);

NTSTATUS
FvmVm_FreeVirtualMemory(IN HANDLE ProcessHandle,	IN OUT PVOID *BaseAddress,	IN OUT PULONG FreeSize,
	     IN ULONG FreeType);

NTSTATUS
FvmInitiateVirtualMemory(PFVM_PVM_TBL_ENTRY CurrentProc);

NTSTATUS
FvmDestroyVirtualMemory(PFVM_PVM_TBL_ENTRY CurrentProc);

#endif //_FVM_VM_H
