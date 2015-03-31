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
#include "hooksys.h"
#include "fvm_obj.h"
#include "fvm_file.h"
#include "fvm_util.h"
#include "fvm_table.h"
#include "fvm_vm.h"
#include "fvm_syscalls.h"

#define FVM_VM_POOL_TAG '8GAT'

/*
 * This header file contains the functions that defines the Virtual Machine
 * manager API's
 */

static struct FVM_new_process_t *getNewProcess(ULONG Pid);

struct FVM_new_process_t *FVM_new_processes = NULL;

ERESOURCE FVM_newProcessResource, FVM_processVMRes,FVM_ProcBufferResource;
ULONG BufferSize=1064960 + 4096*16;

/*
 * Add a new process to the FVM process queue structure.
 * The Pid parameter would be added to the FVM_new_processes structure.
 */
void
FvmVm_addNewProcess(ULONG Pid) {
	struct FVM_new_process_t *nt;

	nt = (struct FVM_new_process_t *) ExAllocatePoolWithTag(PagedPool,
		    sizeof(struct FVM_new_process_t), FVM_VM_POOL_TAG);
	if (nt == NULL) {
		DbgPrint("Unable to allocate memory in the driver\n");
		return;
	}

	nt->pid = Pid;
	nt->dllLoad = NULL;
	nt->imageName = NULL;
	nt->next = NULL;
	nt->prev = NULL;

	KeEnterCriticalRegion();
	ExAcquireResourceExclusiveLite(&FVM_newProcessResource, TRUE);

	if (FVM_new_processes == NULL) {
		FVM_new_processes = nt;
	} else {
		nt->next = FVM_new_processes;
		FVM_new_processes->prev = nt;
		FVM_new_processes = nt;
	}

	ExReleaseResourceLite(&FVM_newProcessResource);
	KeLeaveCriticalRegion();
}

/*
 * Remove the Pid from the FVM_new_processes structure.
 * This function essentially disassociates a Pid from FVM.
 */
void
FvmVm_removeNewProcess(ULONG Pid) {
	struct FVM_new_process_t *p = NULL;

	p = FVM_new_processes;
	KeEnterCriticalRegion();
	ExAcquireResourceExclusiveLite(&FVM_newProcessResource, TRUE);

	while (p) {
		if (p->pid == Pid) {
			if (p->next == NULL && p->prev == NULL) {
				FVM_new_processes = NULL;
				DbgPrint("---------------------------------------\n");
			}
			if (p->prev)
				p->prev->next = p->next;
			else
				FVM_new_processes = p->next;

			if (p->next)
				p->next->prev = p->prev;

			if (p->imageName)
				ExFreePool(p->imageName);
			ExFreePool(p);
			break;
	    }
		p = p->next;
	}
	ExReleaseResourceLite(&FVM_newProcessResource);
	KeLeaveCriticalRegion();
}

/*
 * Get the Pid from the FVM_new_processes structure.
 */
struct FVM_new_process_t *
getNewProcess(ULONG Pid) {

	struct FVM_new_process_t *p = NULL;

	KeEnterCriticalRegion();
	ExAcquireResourceSharedLite(&FVM_newProcessResource, TRUE);

	p = FVM_new_processes;
	while (p != NULL) {

		if (p->pid == Pid)
			break;
		p = p->next;
	}

	ExReleaseResourceLite(&FVM_newProcessResource);
	KeLeaveCriticalRegion();
	return p;
}

/*
 * Update process status to resume the execution.
 */
BOOLEAN
FvmVm_UpdateProcessStatus(ULONG Pid) {
	struct FVM_new_process_t *np = NULL;

	np = getNewProcess(Pid);
	if (np == NULL)
		return FALSE;
	KeSetEvent(np->dllLoad, IO_NO_INCREMENT, FALSE);
	return TRUE;
}

struct FVM_new_process_t *
FvmVm_CheckProcessStatus(ULONG Pid, PWCHAR imageName) {
	struct FVM_new_process_t *np;

	np = getNewProcess(Pid);
	if (np == NULL)
		return NULL;

	if (!np->dllLoad) {
		np->imageName = ExAllocatePoolWithTag(PagedPool,
			wcslen(imageName) * 2 + 2, FVM_VM_POOL_TAG);
		wcscpy(np->imageName, imageName);

		np->dllLoad = (PKEVENT)ExAllocatePoolWithTag(NonPagedPool,
			sizeof(KEVENT), FVM_VM_POOL_TAG);
		if (np->dllLoad) {
			KeInitializeEvent(np->dllLoad, NotificationEvent, FALSE);
			return np;
		}
	}
	return NULL;
}

/*
 * Get the ImageName from the FVM_new_process_t structure form the Pid.
 */
PWCHAR
FvmVm_FindProcessCreateImage(ULONG Pid) {

	struct FVM_new_process_t *np = NULL;

	np = getNewProcess(Pid);
	if (np == NULL)
		return NULL;
	return np->imageName;

}

/*
 * This returns PVM ID from the global table. If not present,
 * it returns INVALID_VMID
 */
ULONG
FvmVm_GetPVMId(ULONG Pid) {
	PFVM_PVM_TBL_ENTRY pte;
	ULONG hv;

	hv = hash_pid(Pid);

	KeEnterCriticalRegion();
	ExAcquireResourceSharedLite(&FVM_processVMRes, TRUE);

	for(pte = FVM_pvm_pid_tbl[hv]; pte; pte = pte->next) {
		if (pte->pid == Pid) {
		    ExReleaseResourceLite(&FVM_processVMRes);
		    KeLeaveCriticalRegion();
		    return pte->pvm_id;
		}
	}
	ExReleaseResourceLite(&FVM_processVMRes);
	KeLeaveCriticalRegion();
	return INVALID_VMID;
}

/*
 * This function adds a process to a domain.
 */
void
FvmVm_AddPid(ULONG Pid, ULONG Pvm_id)
{
	ULONG hv;
	PFVM_PVM_TBL_ENTRY pte;
	ULONG vmid;

	if (FvmVm_GetPVMId(Pid) != INVALID_VMID)
	return;

	hv = hash_pid(Pid);
	pte = (PFVM_PVM_TBL_ENTRY)ExAllocatePoolWithTag(PagedPool,
		sizeof(FVM_PVM_TBL_ENTRY), FVM_VM_POOL_TAG);
	if (!pte) {
		DbgPrint("Unable to allocate memory in the driver\n");
		return;
	}
	pte->pid = Pid;
	pte->pvm_id = Pvm_id;
	pte->dllLoad = NULL;
	pte->imageName = NULL;
	pte->PBuffer = NULL;
	pte->PLitBuffer = NULL;
	pte->BufMap = 0;
	pte->BufferAllocated = FALSE;

	KeEnterCriticalRegion();
	ExAcquireResourceExclusiveLite(&FVM_processVMRes, TRUE);

	pte->next = FVM_pvm_pid_tbl[hv];
	pvms[Pvm_id]->n_procs++;
	FVM_pvm_pid_tbl[hv] = pte;

	DbgPrint("Adding pid %d to FVM %d, FVM now has %d processes\n", Pid,
	    Pvm_id, pvms[Pvm_id]->n_procs);
	ExReleaseResourceLite(&FVM_processVMRes);
	KeLeaveCriticalRegion();
}

/*
 * This function removes a process from a domain.
 */
void
FvmVm_RemovePid(ULONG Pid) {
	ULONG hv;
	PFVM_PVM_TBL_ENTRY pte, pte1;
	PWCHAR BinPath = NULL;
	LONG vmid;

	hv = hash_pid(Pid);
	pte1 = NULL;
	
	KeEnterCriticalRegion();
	ExAcquireResourceExclusiveLite(&FVM_processVMRes, TRUE);

//	if ((vmid = FvmVm_GetPVMId(Pid)) != INVALID_VMID)
	//	return;
	vmid =  FvmVm_GetPVMId(Pid);

	  BinPath = ExAllocatePoolWithTag(PagedPool, _MAX_PATH * 2,
	      FVM_VM_POOL_TAG);

	  if (BinPath == NULL) {
		  ExReleaseResourceLite(&FVM_processVMRes);
		  KeLeaveCriticalRegion();
		  return;
	  }

	  FvmUtil_GetBinaryPathName(BinPath);
	  if ((wcsstr(BinPath, L"msiexec.exe") != NULL)) {
		 DbgPrint("Process name = %Smsi_owner = %ld my num_process = %ld\n",
			BinPath, msi_owner, num_process);
		  if (msi_owner == vmid)
			num_process--;
	
	
		  if ((num_process == 0) && (msi_owner != -1)) {
			//DbgPrint("num_process = %d serv pid%d \n", num_process, service_pid);
			if ((service_pid != 0) && (is_added)) {
				DbgPrint("Removing service pid = %ld from fvm\n",
					service_pid);
				FvmVm_RemovePid(service_pid);
				is_added = 0;
			}
			msi_owner = -1;
			num_process = 0;
			}
	  }
	
	ExFreePool(BinPath);

	for(pte = FVM_pvm_pid_tbl[hv]; pte && (pte->pid != Pid); pte = pte->next)
		pte1=pte;
	if (pte == NULL) {
		//DbgPrint("Strange! Pid %d is not in the table\n", Pid);
		ExReleaseResourceLite(&FVM_processVMRes);
		KeLeaveCriticalRegion();
		return;
	}
	if(pte->BufferAllocated == TRUE)
		FvmDestroyVirtualMemory(pte);	
	if (pte1 == NULL)
		FVM_pvm_pid_tbl[hv] = pte->next;
	else
		pte1->next = pte->next;
	pvms[pte->pvm_id]->n_procs--;
	DbgPrint("Removing pid %d from FVM %d, FVM now has %d processes\n",
	    pte->pid, pte->pvm_id, pvms[pte->pvm_id]->n_procs);
	ExFreePool(pte);

	ExReleaseResourceLite(&FVM_processVMRes);
	KeLeaveCriticalRegion();
}

/*
 * Create new PVM and initialize with pvm_id
 * This function returns non-zero success and 0 failure.
 */
UINT
FvmVm_CreatePVM(ULONG *Ptr_pvm_id, PWCHAR Pvm_name, PWCHAR IdStr, PWCHAR Root,
    ULONG VmIp, ULONG VmContext)
{
	ULONG i;

	if (FvmVm_FindPVM(Ptr_pvm_id, IdStr) == 0) {
		DbgPrint("Same name virtual machine already exist!\n");
		return CREATE_VM_RUNNING;
	}

	KeEnterCriticalRegion();
	ExAcquireResourceExclusiveLite(&FVM_processVMRes, TRUE);

	for(i=0; i<MAX_NUM_PVMS; i++) {
		if (pvms[i] == NULL)
			break;
	}

	if (i == MAX_NUM_PVMS) {
		DbgPrint("Number of Virtual Machines already reached maximum(%d), "
		    "Cannot create anymore!\n", MAX_NUM_PVMS);

		ExReleaseResourceLite(&FVM_processVMRes);
		KeLeaveCriticalRegion();
		return CREATE_VM_MAXVMER;
	}

	pvms[i] = (PFVM_PSEUDO_VIRTUAL_MACHINE)ExAllocatePoolWithTag(PagedPool,
		    sizeof(FVM_PSEUDO_VIRTUAL_MACHINE), FVM_VM_POOL_TAG);

	if (pvms[i] == NULL) {
		DbgPrint("Failed to Allocate memory for the new PVM\n");
		ExReleaseResourceLite(&FVM_processVMRes);
		KeLeaveCriticalRegion();
		return CREATE_VM_MEMERRO;
	}
	if (Ptr_pvm_id)
		*Ptr_pvm_id = i;

	n_pvms++;
	pvms[i]->id = i;
	pvms[i]->n_procs = 0;
	pvms[i]->status = 1;
	wcscpy(pvms[i]->name, Pvm_name);
	wcscpy(pvms[i]->idStr, IdStr);
	wcscpy(pvms[i]->fileRoot, Root);

	pvms[i]->pvm_vmIp = VmIp;
	pvms[i]->pvm_vmContext = VmContext;

	ExReleaseResourceLite(&FVM_processVMRes);
	KeLeaveCriticalRegion();

	FvmFile_CreateFVMRootDir(*Ptr_pvm_id);

	FvmObj_CreateObjDirectory(*Ptr_pvm_id);

	FvmObj_CreatePortDirectory(*Ptr_pvm_id);

	FvmTable_ReadVMDeletedLogFile(*Ptr_pvm_id);

    //FvmTable_FVMFileListInit(*Ptr_pvm_id);

	DbgPrint("Created a new virtual machine named %S\n", pvms[i]->name);
	return CREATE_VM_SUCCESS;
}

/*
 * This function would be invoked when all the processes in the PVM die.
 * Returns Non-zero:Success
 * 0: Failure
 */
UINT
FvmVm_DestroyPVM(ULONG Pvm_id)
{
	if (FVM_ObjectDirectoryHandle[Pvm_id]) {
		ZwClose(FVM_ObjectDirectoryHandle[Pvm_id]);
		FVM_ObjectDirectoryHandle[Pvm_id] = NULL;
	}

	if (FVM_PortDirectoryHandle[Pvm_id]) {
		ZwClose(FVM_PortDirectoryHandle[Pvm_id]);
		FVM_PortDirectoryHandle[Pvm_id] = NULL;
	}

	KeEnterCriticalRegion();
	ExAcquireResourceExclusiveLite(&FVM_processVMRes, TRUE);

	if (pvms[Pvm_id] != NULL && pvms[Pvm_id]->n_procs > 0) {
		DbgPrint("FVM %d contains %d processes, Returning without "
			"destroying\n", Pvm_id, pvms[Pvm_id]->n_procs);
		ExReleaseResourceLite(&FVM_processVMRes);
		KeLeaveCriticalRegion();
		return 0;
	}

	ExFreePool(pvms[Pvm_id]);
	pvms[Pvm_id] = NULL;
	DbgPrint("Destroying PVM %d\n", Pvm_id);

	ExReleaseResourceLite(&FVM_processVMRes);
	KeLeaveCriticalRegion();
	return 1;
}

/*
 * The idStr is unique for all FVM's.
 * Find PVM Id from the global FVM_PSEUDO_VIRTUAL_MACHINE structure.
 */
UINT
FvmVm_FindPVM(ULONG *Ptr_pvm_id, PWCHAR IdStr)
{
	int i;

	if (IdStr == NULL)
		return 1;
	if (Ptr_pvm_id == NULL)
		return 1;

	KeEnterCriticalRegion();
	ExAcquireResourceSharedLite(&FVM_processVMRes, TRUE);

	for( i = 0; i < MAX_NUM_PVMS; i++) {
		if (pvms[i] != NULL) {
			if (wcscmp(pvms[i]->idStr, IdStr) == 0) {
				*Ptr_pvm_id = pvms[i]->id;
				ExReleaseResourceLite(&FVM_processVMRes);
				KeLeaveCriticalRegion();
				return 0;
			}
		}
	}
	ExReleaseResourceLite(&FVM_processVMRes);
	KeLeaveCriticalRegion();
	return 1;
}
/*
 * This function allocates virtual memory. 
 * It gets memory from preallocated buffer.
 */
NTSTATUS FvmVm_AllocateVirtualMemory(
	IN HANDLE ProcessHandle,
	IN OUT PVOID *BaseAddress,
	IN ULONG ZeroBits,
	IN OUT PULONG AllocationSize,
	IN ULONG AllocationType,
	IN ULONG Protect
)
{
	int i=0;
	NTSTATUS rc;
    PFVM_PVM_TBL_ENTRY CurrentProc; 
	   
    CurrentProc = FVM_pvm_pid_tbl[hash_pid((ULONG)PsGetCurrentProcessId())];
    if(CurrentProc==NULL){
    	rc = ZwAllocateVirtualMemory(ProcessHandle, BaseAddress, ZeroBits,
    				AllocationSize, AllocationType, Protect);
    	if(!NT_SUCCESS(rc))
    		DbgPrint("ZwAllocateVirtualMemory failed pid=%ld\n",(ULONG)PsGetCurrentProcessId());
    	  //else
    	    //DbgPrint("ZwAllocateVirtualMemory success pid=%ld,without buffer\n",(ULONG)PsGetCurrentProcessId());
    		return rc;
        }
  
        if(CurrentProc->BufferAllocated==FALSE){
	    	rc=FvmInitiateVirtualMemory(CurrentProc);
	    	if(!NT_SUCCESS(rc)){
			DbgPrint("ZwAllocateVirtualMemory initiate buffer failed pid=%ld\n",(ULONG)PsGetCurrentProcessId());
			return rc;
	    }
	}
	KeEnterCriticalRegion();
	ExAcquireResourceSharedLite(&FVM_ProcBufferResource, TRUE);	  
    if((int)*AllocationSize > 4096){
    	if((CurrentProc->BufMap & 0x80000000)==0){
	  		*BaseAddress=CurrentProc->PBuffer;
			CurrentProc->BufMap = CurrentProc->BufMap | 0x80000000;
			//DbgPrint("FvmVm_AllocateVirtualMemory (%ld) allocate a big memory\n",(ULONG)PsGetCurrentProcessId());
			ExReleaseResourceLite(&FVM_ProcBufferResource);
   	    	KeLeaveCriticalRegion();	
			return STATUS_SUCCESS;
        }
    }else{
        for(i=0;i<16;i++){
			if((CurrentProc->BufMap &(0x00000001<<i))==0){
				*BaseAddress = (PVOID)((char*)CurrentProc->PLitBuffer + (15-i)*4096);
				CurrentProc->BufMap = CurrentProc->BufMap|(0x00000001<<i);
	            //DbgPrint("FvmVm_AllocateVirtualMemory (%ld) allocate a lit memory i=%d,addr=%ld\n",(ULONG)PsGetCurrentProcessId(),i,*BaseAddress);
			    ExReleaseResourceLite(&FVM_ProcBufferResource);
	   	        KeLeaveCriticalRegion();	
			    return STATUS_SUCCESS;			
			}
        }
    }
	ExReleaseResourceLite(&FVM_ProcBufferResource);
	KeLeaveCriticalRegion();	  
	  
	rc = ZwAllocateVirtualMemory(ProcessHandle, BaseAddress, ZeroBits,
			AllocationSize, AllocationType, Protect);
	//DbgPrint("FvmVm_AllocateVirtualMemory (%ld) buffer overflow\n",(ULONG)PsGetCurrentProcessId());

    return rc;
}



/*
 * This function free virtual memory. 
 * It puts memory to the preallocated buffer.
 */

NTSTATUS
FvmVm_FreeVirtualMemory(
	IN HANDLE ProcessHandle,
	IN OUT PVOID *BaseAddress,
	IN OUT PULONG FreeSize,
	IN ULONG FreeType
)
{
	NTSTATUS rc=STATUS_SUCCESS;
	int i=0;
    PFVM_PVM_TBL_ENTRY CurrentProc;
	BOOLEAN Freed = FALSE;

    CurrentProc = FVM_pvm_pid_tbl[hash_pid((ULONG)PsGetCurrentProcessId())];
    if((CurrentProc==NULL)||(CurrentProc->BufferAllocated==FALSE)){
		rc = ZwFreeVirtualMemory(ProcessHandle,BaseAddress,FreeSize,FreeType);
		//DbgPrint("FvmVm_FreeVirtualMemory (%ld),without buffer\n",(ULONG)PsGetCurrentProcessId());
		return rc;
    }
	if(*BaseAddress==NULL)
		return rc;

	KeEnterCriticalRegion();
	ExAcquireResourceSharedLite(&FVM_ProcBufferResource, TRUE);
	if(*BaseAddress == CurrentProc->PBuffer){
		*BaseAddress=NULL;
		CurrentProc->BufMap = CurrentProc->BufMap & 0x7fffffff;
		//DbgPrint("FvmVm_FreeVirtualMemory (%ld) free big memory\n",(ULONG)PsGetCurrentProcessId());		  
		Freed = TRUE;
		rc = STATUS_SUCCESS;
	}else if((*BaseAddress>=CurrentProc->PLitBuffer)&&(*BaseAddress<((PVOID)((char*)CurrentProc->PBuffer+BufferSize)))){
		CurrentProc->BufMap = CurrentProc->BufMap&(~(0x00000001<<(((char*)*BaseAddress-(char*)CurrentProc->PLitBuffer)/4096)));
		//DbgPrint("FvmVm_FreeVirtualMemory (%ld) free lit memory addr=%ld\n",(ULONG)PsGetCurrentProcessId(),*BaseAddress);		  
		*BaseAddress = NULL;		  
  		Freed = TRUE;
		rc = STATUS_SUCCESS;
	}
	ExReleaseResourceLite(&FVM_ProcBufferResource);
	KeLeaveCriticalRegion();
    if(!Freed){	
		rc = ZwFreeVirtualMemory(ProcessHandle,BaseAddress,FreeSize,FreeType);
		//DbgPrint("FvmVm_FreeVirtualMemory (%ld) buffer overflow\n",(ULONG)PsGetCurrentProcessId());  
    }
	return rc;
}

/*
 * This function preallocates and formats a block of big buffer for a process. 
 * 
 */
NTSTATUS
FvmInitiateVirtualMemory(PFVM_PVM_TBL_ENTRY CurrentProc)
{
	NTSTATUS rc;

	if(CurrentProc->BufferAllocated == TRUE)
		return STATUS_SUCCESS;
	
	rc = ZwAllocateVirtualMemory(NtCurrentProcess(),&CurrentProc->PBuffer,0,&BufferSize,MEM_COMMIT,PAGE_READWRITE);
	if(!NT_SUCCESS(rc)){
		DbgPrint("FvmInitiateVirtualMemory failed,process id=%ld\n",(ULONG)PsGetCurrentProcessId());
		return rc;
    }
    CurrentProc->BufferAllocated = TRUE;
    //DbgPrint("FvmInitiateVirtualMemory success,process id=%ld,addr=%ld,size=%ld\n",(ULONG)PsGetCurrentProcessId(),CurrentProc->PBuffer,BufferSize);
    CurrentProc->PLitBuffer = (PVOID)((char*)CurrentProc->PBuffer + 1064960);
    CurrentProc->BufMap = 0;
	
    return STATUS_SUCCESS;
}

/*
 * This function free the preallocated buffer. 
 *
 */

NTSTATUS
FvmDestroyVirtualMemory(PFVM_PVM_TBL_ENTRY CurrentProc)
{
	NTSTATUS rc;
	ULONG size = 0;
	if(CurrentProc->BufferAllocated == FALSE)
		return STATUS_SUCCESS;
	CurrentProc->BufferAllocated = FALSE;
	CurrentProc->PLitBuffer = NULL;
	CurrentProc->BufMap = 0;	
	rc = ZwFreeVirtualMemory(NtCurrentProcess(),&(CurrentProc->PBuffer),&size,MEM_RELEASE);
    if(!NT_SUCCESS(rc)){
		CHAR errStr[64];
	    DbgPrint("FvmDestroyVirtualMemory failed, process id=%ld, Err:%s\n",(ULONG)PsGetCurrentProcessId(),FvmUtil_ErrorString(rc, errStr));
    }//else
	  //DbgPrint("FvmDestroyVirtualMemory success, process id=%ld, addr=%ld\n",(ULONG)PsGetCurrentProcessId(),CurrentProc->PBuffer);
	CurrentProc->PBuffer = NULL;
	return STATUS_SUCCESS;
}
