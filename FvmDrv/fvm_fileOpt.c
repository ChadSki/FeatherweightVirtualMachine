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
 * fvm_fileOpt.c --
 *
 *	This code file implements a few file-related system call functions
 *  in the FVM layer. Different from the same set of functions
 *  implemented in fvm_file.c, the virtual-to-physical name mapping is
 *  determined by looking up a binary tree that stores all the name of
 *  files copied to a VM's workspace. As a result, no system call is
 *  made to test if the VM already has a private copy of a given file.
 *  This optimization can reduce the system call interception overhead
 *  in virtual-to-physical mapping.
 */

#include <ntddk.h>
#include <stdlib.h>
#include "fvm_util.h"
#include "fvm_table.h"
#include "hooksys.h"
#include "fvm_vm.h"
#include "fvm_syscalls.h"
#include "fvm_fileInt.h"
#include "fvm_file.h"

#define FVM_FILEOPT_POOL_TAG '3GAT'

#ifdef USE_FS_MAP

/*
 *-----------------------------------------------------------------------------
 *
 * FvmFile_NtCreateFileT1 --
 *
 *      This function implements the FVM-provided NtCreateFile system call
 *      when the CreateDisposition argument is FILE_SUPERSEDE or
 *      FILE_OVERWRITE_IF. Since the two flags indicate that the target file
 *      will be overwritten anyway, we directly invoke the orignal NtCreateFile
 *      on the VM's private workspace. In the case of FILE_OVERWRITE_IF, we
 *      may also need to copy the file's attributes from the host environment.
 *
 * Results:
 *      Return STATUS_SUCCESS on success or NTSTATUS error code on failure.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

NTSTATUS
FvmFile_NtCreateFileT1(OUT PHANDLE FileHandle,
					   IN ACCESS_MASK DesiredAccess,
                       IN POBJECT_ATTRIBUTES ObjectAttributes,
	                   OUT PIO_STATUS_BLOCK IoStatusBlock,
	                   IN PLARGE_INTEGER AllocationSize,
	                   IN ULONG FileAttributes,
	                   IN ULONG ShareAccess,
                       IN ULONG CreateDisposition,
                       IN ULONG CreateOptions,
                       IN PVOID EaBuffer,
                       IN ULONG EaLength,
                       IN PWCHAR FileLinkName, /* virtual file path */
                       IN PWCHAR VDirName,     /* mapped physical file path */
                       IN POBJECT_ATTRIBUTES ObjAttr,
                                               /* Pointer to OBJECT_ATTRIBUTES
                                                * structure for VDirName */
                       IN ULONG VmId)          /* ID of the VM's context */
{
	NTSTATUS rc, rc1;
	ULONG memSize;
	PFILE_BASIC_INFORMATION fileBasicInfoPtr = NULL;
	PIO_STATUS_BLOCK ioStatusPtr = NULL;

	/*
	 * Create parent directories of the file under the VM's workspace.
	 */

	FvmUtil_CreateFileDir(FileLinkName, VDirName, VmId);

	rc = (winNtCreateFileProc)(FileHandle,
			DesiredAccess,
			ObjAttr,
			IoStatusBlock,
			AllocationSize,
			FileAttributes,
			ShareAccess,
			CreateDisposition,
			CreateOptions,
			EaBuffer,
			EaLength);

	if (!NT_SUCCESS(rc)) {
		CHAR errStr[64];
		DbgPrint("CreateErrT1:%s\n", FvmUtil_ErrorString(rc, errStr));
	} else {
		if (!FvmTable_DeleteLogLookup(FileLinkName, VmId)) {
			if (CreateOptions == FILE_OVERWRITE_IF) {
			    /*
			     * The difference between FILE_SUPERSEDE and FILE_OVERWRITE_IF
			     * is that the latter preserves the file attributes if the file
			     * exists. Therefore, if the file exists on the host environment
			     * but not in the VM, the attributes should be duplicated to
			     * the file created in the VM.
			     */

				memSize = sizeof(FILE_BASIC_INFORMATION)
						+ sizeof(IO_STATUS_BLOCK);
				rc1 = FvmVm_AllocateVirtualMemory(NtCurrentProcess(),
						&fileBasicInfoPtr, 0, &memSize, MEM_COMMIT,
						PAGE_READWRITE);


				if (NT_SUCCESS(rc1)) {
					(CHAR *)ioStatusPtr = ((CHAR *)fileBasicInfoPtr)
							+ sizeof(FILE_BASIC_INFORMATION);

					rc1 = (winNtQueryAttributesFileProc)(ObjectAttributes,
							fileBasicInfoPtr);

					if (NT_SUCCESS(rc1)) {
						(winNtSetInformationFileProc)(*FileHandle,
								ioStatusPtr,
								fileBasicInfoPtr,
								sizeof(FILE_BASIC_INFORMATION),
								FileBasicInformation);
					}
					memSize = 0;
					FvmVm_FreeVirtualMemory(NtCurrentProcess(), &fileBasicInfoPtr,
							&memSize, MEM_RELEASE);
				}
			}
		} else {
			FvmTable_DeleteLogRemove(FileLinkName, VmId);
		}
        /*
         * Add the full path of the file into the FVM file tree in memory.
	     * We assume the file path starts with "\??\" (4 characters), such
	     * as "\??\c:\abc".
         */

		FvmTable_FVMFileListAddFullPath(FileLinkName + 4, VmId);
	}
	return rc;
}


/*
 *-----------------------------------------------------------------------------
 *
 * FvmFile_NtCreateFileT2 --
 *
 *      This function implements the FVM-provided NtCreateFile system call
 *      when the CreateDisposition argument is FILE_OPEN or FILE_OVERWRITE.
 *      If there is already a file copy under the FVM's workspace, we invoke
 *      the original NtCreateFile on the private file copy. Otherwise,
 *      according to the desired access, we may need to do copy-on-write to
 *      prevent the original file on the host environment from being opened
 *      for write.
 *
 * Results:
 *      Return STATUS_SUCCESS on success or NTSTATUS error code on failure.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

NTSTATUS
FvmFile_NtCreateFileT2(OUT PHANDLE FileHandle,
                       IN ACCESS_MASK DesiredAccess,
                       IN POBJECT_ATTRIBUTES ObjectAttributes,
                       OUT PIO_STATUS_BLOCK IoStatusBlock,
                       IN PLARGE_INTEGER AllocationSize,
                       IN ULONG FileAttributes,
                       IN ULONG ShareAccess,
                       IN ULONG CreateDisposition,
                       IN ULONG CreateOptions,
                       IN PVOID EaBuffer,
                       IN ULONG EaLength,
                       IN PWCHAR FileLinkName, /* virtual file path */
                       IN PWCHAR VDirName,     /* mapped physical file path */
                       IN POBJECT_ATTRIBUTES ObjAttr,
                                               /* Pointer to OBJECT_ATTRIBUTES
                                                * structure for VDirName */
                       IN ULONG VmId)          /* ID of the VM's context */
{
	NTSTATUS rc = STATUS_OBJECT_PATH_NOT_FOUND;

    /*
     * FILE_OPEN or FILE_OVERWRITE requires the file to be accessed exists.
     * So we first check if the file is already in the DeleteLog.
     */

	if (!FvmTable_DeleteLogLookup(FileLinkName, VmId)) {
		if (CreateDisposition != FILE_OVERWRITE
				&& !FvmFile_IsOpenforWrite(DesiredAccess, ShareAccess,
				CreateOptions)) {
            /*
             * We already know there is no corresponding file in the VM.
             * So we can simply open the file on the host environment
             * for a read-only access.
             */

			rc = (winNtCreateFileProc)(FileHandle,
					DesiredAccess,
					ObjectAttributes,
					IoStatusBlock,
					AllocationSize,
					FileAttributes,
					ShareAccess,
					CreateDisposition,
					CreateOptions,
					EaBuffer,
					EaLength);
			return rc;

		} else {
            /*
             * Copy the file to the VM's workspace and open it.
             */

			rc = FvmUtil_CopyFiletoVM(ObjectAttributes, FileLinkName, VDirName,
					(BOOLEAN)(CreateOptions & FILE_DIRECTORY_FILE), FALSE,
					VmId);

			if (NT_SUCCESS(rc)) {
				rc = (winNtCreateFileProc)(FileHandle,
						DesiredAccess,
						ObjAttr,
						IoStatusBlock,
						AllocationSize,
						FileAttributes,
						ShareAccess,
						CreateDisposition,
						CreateOptions,
						EaBuffer,
						EaLength);

				if (!NT_SUCCESS(rc)) {
					CHAR errStr[64];
					DbgPrint("CreateErrT2:%s\n", FvmUtil_ErrorString(rc, errStr));
				}
			}
		}
	}
	return rc;
}


/*
 *-----------------------------------------------------------------------------
 *
 * FvmFile_NtCreateFileT3 --
 *
 *      This function implements the FVM-provided NtCreateFile system call
 *      when the CreateDisposition argument is FILE_CREATE or FILE_OPEN_IF.
 *      In the case of FILE_CREATE, if there is already a file copy under
 *      the FVM's workspace or the host environment, the system call fails.
 *      Otherwise, the file is created under the FVM's workspace. In the case
 *      of FILE_OPEN_IF, we may need to do copy-on-write if the desired access
 *      is open for write.
 *
 * Results:
 *      Return STATUS_SUCCESS on success or NTSTATUS error code on failure.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

NTSTATUS
FvmFile_NtCreateFileT3(OUT PHANDLE FileHandle,
                       IN ACCESS_MASK DesiredAccess,
                       IN POBJECT_ATTRIBUTES ObjectAttributes,
                       OUT PIO_STATUS_BLOCK IoStatusBlock,
                       IN PLARGE_INTEGER AllocationSize,
                       IN ULONG FileAttributes,
                       IN ULONG ShareAccess,
                       IN ULONG CreateDisposition,
                       IN ULONG CreateOptions,
                       IN PVOID EaBuffer,
                       IN ULONG EaLength,
                       IN PWCHAR FileLinkName, /* Virtual file path */
                       IN PWCHAR VDirName,     /* Mapped physical file path */
                       IN POBJECT_ATTRIBUTES ObjAttr,
                                               /* Pointer to OBJECT_ATTRIBUTES
                                                * structure for VDirName */
                       IN ULONG VmId)          /* ID of the VM's context */
{
	NTSTATUS rc, rc1;
	ULONG memSize1, memSize2;
	PFILE_BASIC_INFORMATION fileBasicInfoPtr = NULL;
	PIO_STATUS_BLOCK ioStatusPtr = NULL;

	if (FvmTable_DeleteLogLookup(FileLinkName, VmId)) {
		/*
		 * If the file has been deleted, the system call is going to create
		 * the file again.
		 */

		FvmUtil_CreateFileDir(FileLinkName, VDirName, VmId);

		rc = (winNtCreateFileProc)(FileHandle,
				DesiredAccess,
				ObjAttr,
				IoStatusBlock,
				AllocationSize,
				FileAttributes,
				ShareAccess,
				CreateDisposition,
				CreateOptions,
				EaBuffer,
				EaLength);

		if (NT_SUCCESS(rc)) {
			FvmTable_DeleteLogRemove(FileLinkName, VmId);
			FvmTable_FVMFileListAddFullPath(FileLinkName + 4, VmId);
		}
		goto ntExit;
	}

	memSize1 = sizeof(FILE_BASIC_INFORMATION);
	rc = FvmVm_AllocateVirtualMemory(NtCurrentProcess(), &fileBasicInfoPtr, 0,
			&memSize1, MEM_COMMIT, PAGE_READWRITE);

	if (!NT_SUCCESS(rc)) {
		rc = STATUS_ACCESS_DENIED;
		goto ntExit;
	}

    /*
     * We use ID_NTQUERYATTRIBUTESFILE to check whether the file exists on
     * the host environment.
     */

	rc = (winNtQueryAttributesFileProc)(ObjectAttributes, fileBasicInfoPtr);

	if (NT_SUCCESS(rc) || (rc != STATUS_OBJECT_PATH_NOT_FOUND
			&& rc != STATUS_OBJECT_NAME_NOT_FOUND)) {

		if (CreateDisposition == FILE_CREATE) {
			/*
			 * The desired behavior of FILE_CREATE is to fail the request
			 * if a file with the same name exists.
			 */

			rc = STATUS_OBJECT_NAME_COLLISION;
		} else {
			if (!FvmFile_IsOpenforWrite(DesiredAccess, ShareAccess,
					CreateOptions)) {
			   memSize1 = 0;
				FvmVm_FreeVirtualMemory(NtCurrentProcess(), &fileBasicInfoPtr,
						&memSize1, MEM_RELEASE);
				goto winSysCall;
			}

            /*
             * Copy the file on the host environment to the VM's workspace.
             */

			rc = FvmUtil_CopyFiletoVM(ObjectAttributes, FileLinkName, VDirName,
					(BOOLEAN)(CreateOptions & FILE_DIRECTORY_FILE), FALSE,
					VmId);

			if (NT_SUCCESS(rc)) {
				rc = (winNtCreateFileProc)(FileHandle,
						DesiredAccess,
						ObjAttr,
						IoStatusBlock,
						AllocationSize,
						FileAttributes,
						ShareAccess,
						CreateDisposition,
						CreateOptions,
						EaBuffer,
						EaLength);

				if (!NT_SUCCESS(rc)) {
					CHAR errStr[64];
					DbgPrint("CreateErrT3:%s\n", FvmUtil_ErrorString(rc, errStr));
				}
			} else {
				/*
				 * Copying file failed, so we simply create a file with the
				 * same attributes in the VM.
				 */

				FvmUtil_CreateFileDir(FileLinkName, VDirName, VmId);

				rc = (winNtCreateFileProc)(FileHandle,
						DesiredAccess,
						ObjAttr,
						IoStatusBlock,
						AllocationSize,
						FileAttributes,
						ShareAccess,
						CreateDisposition,
						CreateOptions,
						EaBuffer,
						EaLength);

				if (NT_SUCCESS(rc)) {
					memSize2 = sizeof(IO_STATUS_BLOCK);
					rc1 = FvmVm_AllocateVirtualMemory(NtCurrentProcess(),
							&ioStatusPtr, 0, &memSize2, MEM_COMMIT,
							PAGE_READWRITE);

					if (NT_SUCCESS(rc1)) {
						(winNtSetInformationFileProc)(*FileHandle,
								ioStatusPtr,
								fileBasicInfoPtr,
								sizeof(FILE_BASIC_INFORMATION),
								FileBasicInformation);

                        memSize2 = 0;
						FvmVm_FreeVirtualMemory(NtCurrentProcess(), &ioStatusPtr,
								&memSize2,  MEM_RELEASE);
					}
				}
			}
		}
	} else {
		/*
		 * No file with the same name exists on the host environment, so we
		 * create a file in the VM's workspace.
		 */

		FvmUtil_CreateFileDir(FileLinkName, VDirName, VmId);

		rc = (winNtCreateFileProc)(FileHandle,
				DesiredAccess,
				ObjAttr,
				IoStatusBlock,
				AllocationSize,
				FileAttributes,
				ShareAccess,
				CreateDisposition,
				CreateOptions,
				EaBuffer,
				EaLength);

		if (!NT_SUCCESS(rc)) {
			CHAR errStr[64];
			DbgPrint("CreateErrT3:%s\n", FvmUtil_ErrorString(rc, errStr));
		} else {
			FvmTable_DeleteLogRemove(FileLinkName, VmId);
			FvmTable_FVMFileListAddFullPath(FileLinkName + 4, VmId);
		}
	}

   memSize1 = 0;
	FvmVm_FreeVirtualMemory(NtCurrentProcess(), &fileBasicInfoPtr, &memSize1,
			MEM_RELEASE);
	goto ntExit;

winSysCall:
	rc = (winNtCreateFileProc)(FileHandle,
			DesiredAccess,
			ObjectAttributes,
			IoStatusBlock,
			AllocationSize,
			FileAttributes,
			ShareAccess,
			CreateDisposition,
			CreateOptions,
			EaBuffer,
			EaLength);
ntExit:
	return rc;
}


/*
 *-----------------------------------------------------------------------------
 *
 * FvmFile_NtCreateFile --
 *
 *      This function is the FVM-provided NtCreateFile system call function.
 *      It checks the system call arguments to redirect access to regular
 *      files and special files, e.g. named pipe. It can also enable or
 *      disable accesses to devices, such as network access.
 *
 * Results:
 *      Return STATUS_SUCCESS on success or NTSTATUS error code on failure.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

NTSTATUS
FvmFile_NtCreateFile(OUT PHANDLE FileHandle,
                     IN ACCESS_MASK DesiredAccess,
                     IN POBJECT_ATTRIBUTES ObjectAttributes,
                     OUT PIO_STATUS_BLOCK IoStatusBlock,
                     IN PLARGE_INTEGER AllocationSize,
                     IN ULONG FileAttributes,
                     IN ULONG ShareAccess,
                     IN ULONG CreateDisposition,
                     IN ULONG CreateOptions,
                     IN PVOID EaBuffer,
                     IN ULONG EaLength)
{
	NTSTATUS rc;
	ULONG vmId = INVALID_VMID;
	PWCHAR fnPtr = NULL;
	ULONG memSize = _MAX_PATH, memSizeFvm = _MAX_PATH;
	POBJECT_ATTRIBUTES objAttrPtr = NULL;  /* Object pointing to the shared
	                                        * host file */
	POBJECT_ATTRIBUTES fvmObjPtr = NULL;   /* Object pointing to the private
	                                        * FVM file */

	InterlockedIncrement(&fvm_Calls_In_Progress);

	objAttrPtr = ObjectAttributes;
	vmId = FvmVm_GetPVMId((ULONG)PsGetCurrentProcessId());

	if (vmId != INVALID_VMID) {
		PWCHAR binPath = NULL;
		PCHAR accessStr = NULL;
		WCHAR fileLinkName[_MAX_PATH];
		WCHAR vDirName[_MAX_PATH];
		PWCHAR hostName, fvmName = NULL;
		BOOLEAN hostQuery = FALSE;

		if (!FvmUtil_GetSysCallArgument(ObjectAttributes, fileLinkName)) {
			goto winSysCall;
		}

		if (!FvmIsLocalFileAccess(fileLinkName)) {
			/*
			 * The process is accessing a device file such as the network
			 * device or a named pipe.
			 */

		#if 1
			/*
			 * Disable all the device access except certain network access.
			 */

			if (!FvmFile_AllowDeviceAccess(fileLinkName)) {

            	//DbgPrint("Create Non-File Argument:-------------------%S\n",
            	//		fileLinkName);
				IoStatusBlock->Status = STATUS_ACCESS_DENIED;
				rc = STATUS_ACCESS_DENIED;
				goto ntExit;
			} else {
				objAttrPtr = ObjectAttributes;
				goto winSysCall;
			}
		#endif

            /*
             * The following code redirects access to named pipe and mailslot
             * under a VM's local namespace, while allowing all other types
             * of device access.
             */

			if (FvmFile_IsPipeMailslotConnect(fileLinkName)) {
				if (!FvmFile_MapPipeMailSlotPath(fileLinkName, vmId,
						vDirName)) {
					goto winSysCall;
				}

				//DbgPrint("New device name:%S\n", vDirName);
				objAttrPtr = NULL;
				rc = FvmUtil_InitializeVMObjAttributes(ObjectAttributes, vDirName,
						&objAttrPtr, &memSize);
	            if (!NT_SUCCESS(rc)) {
					rc = STATUS_ACCESS_DENIED;
					goto ntExit;
				}

				rc = (winNtCreateFileProc)(
						FileHandle,
						DesiredAccess,
						objAttrPtr,
						IoStatusBlock,
						AllocationSize,
						FileAttributes,
						ShareAccess,
						CreateDisposition,
						CreateOptions,
						EaBuffer,
						EaLength);

                memSize = 0;
				FvmVm_FreeVirtualMemory(NtCurrentProcess(), &objAttrPtr, &memSize,
						MEM_RELEASE);

				if (NT_SUCCESS(rc)) {
					goto ntExit;
				} else {
					objAttrPtr = ObjectAttributes;
					goto winSysCall;
				}
			} else {
				goto winSysCall;
			}
		}

		if (!FvmFile_GetLongPathName(fileLinkName)) {
			goto winSysCall;
		}

		if (FvmFile_IsFileinFVM(fileLinkName, vmId, &fnPtr)) {
			/*
			 * We found that the process attempts to access a file whose
			 * path is under a VM's workspace directory. This should rarely
			 * happen because a process should always operate on a virtual
			 * path instead of the FVM-renamed (physical) path. When it
			 * does happen, e.g. due to certain bug in the FVM's renaming
			 * mechanism, we should not perform further renaming here.
             */

			if (FvmTable_FVMFileListLookup(fnPtr + 4, vmId)) {
	            /*
	             * We assume the file path starts with "\??\" (4 characters),
	             * such as "\??\c:\abc".
	             */

				goto winSysCall;
			} else {
				/*
				 * We need to get an object pointing to the path of the
				 * original file shared on the host environment.
				 */

				objAttrPtr = NULL;
				rc = FvmUtil_InitializeVMObjAttributes(ObjectAttributes, fnPtr,
						&objAttrPtr, &memSize);

				if (!NT_SUCCESS(rc)) {
					rc = STATUS_OBJECT_PATH_NOT_FOUND;
					goto ntExit;
				}
				hostName = fnPtr;
				fvmName = fileLinkName;
				fvmObjPtr = ObjectAttributes;

				goto hostQuery;
			}
		} else {
			fnPtr = NULL;
			hostName = fileLinkName;

			if (!FvmFile_MapPath(hostName, vmId, vDirName)) {
				goto winSysCall;
			}
			fvmName = vDirName;

			/*
			 * Get an object pointing to the private path of the file under
			 * a VM's workspace.
			 */

			rc = FvmUtil_InitializeVMObjAttributes(ObjectAttributes, vDirName,
					&fvmObjPtr, &memSizeFvm);

			if (!NT_SUCCESS(rc)) {
				CHAR errStr[64];
				DbgPrint("ErrMem:%s\n", FvmUtil_ErrorString(rc, errStr));
				rc = STATUS_ACCESS_DENIED;
				goto ntExit;
			}

			if (!FvmTable_FVMFileListLookup(hostName + 4, vmId)) {
				goto hostQuery;
			} else {
				rc = (winNtCreateFileProc)(
						FileHandle,
						DesiredAccess,
						fvmObjPtr,
						IoStatusBlock,
						AllocationSize,
						FileAttributes,
						ShareAccess,
						CreateDisposition,
						CreateOptions,
						EaBuffer,
						EaLength);
				goto ntExit;
			}
		}

#if DBG_CREATEFILE
		binPath = ExAllocatePoolWithTag(PagedPool, _MAX_PATH * 2,
		    FVM_FILEOPT_POOL_TAG);
		if (binPath == NULL) {
			goto winSysCall;
		}
		FvmUtil_GetBinaryPathName(binPath);
		DbgPrint("NtCreateFile : Application Name - %S\n", binPath);
		ExFreePool(binPath);

		DbgPrint("               Arguments - %S\n", hostName);
		accessStr = ExAllocatePoolWithTag( PagedPool, _MAX_PATH,
			FVM_FILEOPT_POOL_TAG);
		if (accessStr) {
			DbgPrint("               Access - %s\n",
					AccessString(DesiredAccess, accessStr));
			ExFreePool(accessStr);
		}
		DbgPrint("               New file name - %S\n", fvmName);
#endif

hostQuery:
        /*
         * The file to be accessed does not exist on the VM's workspace.
         * So we go to the host environment and decide if a copy-on-write
         * is necessary. We divide the system call processing into three
         * cases, which are processed by three functions separately. Please
         * refer to the DDK documentation to understand the CreateDisposition
         * argument.
         */

		switch (CreateDisposition) {
		case FILE_SUPERSEDE:
		case FILE_OVERWRITE_IF:
			rc = FvmFile_NtCreateFileT1(
					FileHandle,
					DesiredAccess,
					objAttrPtr,
					IoStatusBlock,
					AllocationSize,
					FileAttributes,
					ShareAccess,
					CreateDisposition,
					CreateOptions,
					EaBuffer,
					EaLength,
					hostName,
					fvmName,
					fvmObjPtr,
					vmId);
			goto ntExit;

		case FILE_OPEN:
		case FILE_OVERWRITE:
			rc = FvmFile_NtCreateFileT2(
					FileHandle,
					DesiredAccess,
					objAttrPtr,
					IoStatusBlock,
					AllocationSize,
					FileAttributes,
					ShareAccess,
					CreateDisposition,
					CreateOptions,
					EaBuffer,
					EaLength,
					hostName,
					fvmName,
					fvmObjPtr,
					vmId);
			goto ntExit;

		case FILE_CREATE:
		case FILE_OPEN_IF:
			rc = FvmFile_NtCreateFileT3(
					FileHandle,
					DesiredAccess,
					objAttrPtr,
					IoStatusBlock,
					AllocationSize,
					FileAttributes,
					ShareAccess,
					CreateDisposition,
					CreateOptions,
					EaBuffer,
					EaLength,
					hostName,
					fvmName,
					fvmObjPtr,
					vmId);
			goto ntExit;
		}
	}
winSysCall:
    /*
     * After detecting that a process is not associated with any VM,
     * or after detecting an error, program controls are transferred to
     * here and invokes the system call on the original file in the host
     * environment.
     */

	rc = (winNtCreateFileProc)(
			FileHandle,
			DesiredAccess,
			objAttrPtr,
			IoStatusBlock,
			AllocationSize,
			FileAttributes,
			ShareAccess,
			CreateDisposition,
			CreateOptions,
			EaBuffer,
			EaLength);
ntExit:
	if (fvmObjPtr && fvmObjPtr != ObjectAttributes) {
	    memSizeFvm = 0;
		FvmVm_FreeVirtualMemory(NtCurrentProcess(), &fvmObjPtr, &memSizeFvm,
				MEM_RELEASE);
	}
	if (objAttrPtr && objAttrPtr != ObjectAttributes) {
	    memSize = 0;
		FvmVm_FreeVirtualMemory(NtCurrentProcess(), &objAttrPtr, &memSize,
				MEM_RELEASE);
	}
	if (fnPtr) {
		ExFreePool(fnPtr);
	}

	InterlockedDecrement(&fvm_Calls_In_Progress);
	return rc;
}


/*
 *-----------------------------------------------------------------------------
 *
 * FvmFile_NtOpenFile --
 *
 *      This function is the FVM-provided NtOpenFile system call function.
 *      It checks the system call arguments to redirect access to regular
 *      files and special files, e.g. named pipe. It can also enable or
 *      disable accesses to devices, such as network access. The implemen-
 *      tation is equivalent to NtCreateFile when the CreateDisposition is
 *      FILE_OPEN.
 *
 * Results:
 *      Return STATUS_SUCCESS on success or NTSTATUS error code on failure.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

NTSTATUS
FvmFile_NtOpenFile(OUT PHANDLE FileHandle,
                   IN ACCESS_MASK DesiredAccess,
                   IN POBJECT_ATTRIBUTES ObjectAttributes,
                   OUT PIO_STATUS_BLOCK IoStatusBlock,
                   IN ULONG ShareMode,
                   IN ULONG OpenMode)
{
	NTSTATUS rc;
	ULONG vmId = INVALID_VMID;
	PWCHAR fnPtr = NULL;
	ULONG memSize = _MAX_PATH, memSizeFvm = _MAX_PATH;
	POBJECT_ATTRIBUTES objAttrPtr = NULL;
	POBJECT_ATTRIBUTES fvmObjPtr = NULL;

	InterlockedIncrement(&fvm_Calls_In_Progress);

	objAttrPtr = ObjectAttributes;
	vmId = FvmVm_GetPVMId((ULONG)PsGetCurrentProcessId());

	if (vmId != INVALID_VMID) {
		//PWCHAR binPath = NULL;
		//PCHAR accessStr = NULL;
		WCHAR fileLinkName[_MAX_PATH];
		WCHAR vDirName[_MAX_PATH];
		PWCHAR hostName, fvmName = NULL;

		if (!FvmUtil_GetSysCallArgument(ObjectAttributes, fileLinkName)) {
			goto winSysCall;
		}
		if (!FvmIsLocalFileAccess(fileLinkName)) {
			/*
			 * The process is accessing a device file such as the network
			 * device or a named pipe.
			 */

		#if 1
			/*
			 * Disable all the device access except certain network access.
			 */

			if (!FvmFile_AllowDeviceAccess(fileLinkName)) {
				//DbgPrint("Open Non-File Argument:-------------------%S\n",
				//		fileLinkName);
				IoStatusBlock->Status = STATUS_ACCESS_DENIED;
				rc = STATUS_ACCESS_DENIED;
				goto ntExit;
			} else {
				objAttrPtr = ObjectAttributes;
				goto winSysCall;
			}
		#endif
            /*
             * The following code redirects access to named pipe and mailslot
             * under a VM's local namespace, while allowing all other types
             * of device access.
             */

			if (FvmFile_IsPipeMailslotConnect(fileLinkName)) {
				if (!FvmFile_MapPipeMailSlotPath(fileLinkName, vmId,
						vDirName)) {
					goto winSysCall;
				}
				//DbgPrint("New device name:%S\n", vDirName);

				objAttrPtr = NULL;
				rc = FvmUtil_InitializeVMObjAttributes(ObjectAttributes, vDirName,
						&objAttrPtr, &memSize);

				if (!NT_SUCCESS(rc)) {
					rc = STATUS_ACCESS_DENIED;
					goto ntExit;
				}

				rc = (winNtOpenFileProc)(
						FileHandle,
						DesiredAccess,
						objAttrPtr,
						IoStatusBlock,
						ShareMode,
						OpenMode);

                memSize = 0;
				FvmVm_FreeVirtualMemory(NtCurrentProcess(), &objAttrPtr, &memSize,
						MEM_RELEASE);

				if (NT_SUCCESS(rc)) {
					goto ntExit;
				} else {
					objAttrPtr = ObjectAttributes;
					goto winSysCall;
				}
			} else {
				goto winSysCall;
			}
		}

		if (!FvmFile_GetLongPathName(fileLinkName)) {
			goto winSysCall;
		}

		if (FvmFile_IsFileinFVM(fileLinkName, vmId, &fnPtr)) {
			/*
			 * See comments near the same function inside FvmFile_NtCreateFile
			 * in this code file.
             */

			if (FvmTable_FVMFileListLookup(fnPtr + 4, vmId)) {
				goto winSysCall;
			} else {
				objAttrPtr = NULL;
				/*
				 * Get an object pointing to the path of the original file
				 * shared on the host environment.
				 */

				rc = FvmUtil_InitializeVMObjAttributes(ObjectAttributes, fnPtr,
						&objAttrPtr, &memSize);
				if (!NT_SUCCESS(rc)) {
					rc = STATUS_OBJECT_PATH_NOT_FOUND;
					goto ntExit;
				}
				hostName = fnPtr;
				fvmName = fileLinkName;
				fvmObjPtr = ObjectAttributes;
				goto hostQuery;
			}
		} else {
			fnPtr = NULL;
			hostName = fileLinkName;
			if (!FvmFile_MapPath(hostName, vmId, vDirName)) {
				goto winSysCall;
			}
			fvmName = vDirName;

			/*
			 * Get an object pointing to the private path of the file under
			 * a VM's workspace.
			 */

			rc = FvmUtil_InitializeVMObjAttributes(ObjectAttributes, vDirName,
					&fvmObjPtr, &memSizeFvm);

			if (!NT_SUCCESS(rc)) {
				CHAR errStr[64];
				DbgPrint("ErrMem:%s\n", FvmUtil_ErrorString(rc, errStr));
				rc = STATUS_ACCESS_DENIED;
				goto ntExit;
			}

			if (!FvmTable_FVMFileListLookup(hostName + 4, vmId)) {
				goto hostQuery;
			} else {
				rc = (winNtOpenFileProc)(
						FileHandle,
						DesiredAccess,
						fvmObjPtr,
						IoStatusBlock,
						ShareMode,
						OpenMode);

				goto ntExit;
			}
		}

#if DBG_OPENFILE
		binPath = ExAllocatePoolWithTag(PagedPool, _MAX_PATH * 2,
		    FVM_FILEOPT_POOL_TAG);
		if (binPath == NULL) {
			goto winSysCall;
		}
		FvmUtil_GetBinaryPathName(binPath);
		DbgPrint("NtOpenFile : Application Name - %S\n", binPath);
		ExFreePool(binPath);
		DbgPrint("               Arguments - %S\n", hostName);
		accessStr = ExAllocatePoolWithTag( PagedPool, _MAX_PATH,
			FVM_FILEOPT_POOL_TAG);
		if (accessStr) {
			DbgPrint("               Access - %s\n",
					AccessString(DesiredAccess, accessStr));
			ExFreePool(accessStr);
		}
		DbgPrint("               New file name - %S\n", fvmName);
#endif

hostQuery:
        /*
         * The file to be opened does not exist on the VM's workspace.
         * So we go to the host environment and decide if a copy-on-write
         * is necessary.
         */

		if (!FvmTable_DeleteLogLookup(hostName, vmId)) {
			if (!FvmFile_IsOpenforWrite(DesiredAccess, ShareMode, OpenMode))
				goto winSysCall;

            /*
             * If a file is to be opened for write, we copy the file to the
             * VM's private workspace. This operation includes copying the
             * parent directory, the file itself and file attributes.
             */

			rc = FvmUtil_CopyFiletoVM(objAttrPtr, hostName, fvmName,
					(BOOLEAN)(OpenMode & FILE_DIRECTORY_FILE), TRUE, vmId);

			if (NT_SUCCESS(rc)) {
				rc = (winNtOpenFileProc)(
						FileHandle,
						DesiredAccess,
						fvmObjPtr,
						IoStatusBlock,
						ShareMode,
						OpenMode);

				if (!NT_SUCCESS(rc)) {
					CHAR errStr[64];
					DbgPrint("OpenErr2:%s\n", FvmUtil_ErrorString(rc, errStr));
				}
			}
		} else {
			rc = STATUS_OBJECT_PATH_NOT_FOUND;
		}
		goto ntExit;
	}

winSysCall:
    /*
     * After detecting that a process is not associated with any VM,
     * or after detecting an error, program controls are transferred to
     * here and invokes the system call on the original file in the host
     * environment.
     */

	rc = (winNtOpenFileProc)(
			FileHandle,
			DesiredAccess,
			objAttrPtr,
			IoStatusBlock,
			ShareMode,
			OpenMode);

ntExit:
	if (fvmObjPtr && fvmObjPtr != ObjectAttributes) {
	    memSizeFvm = 0;
		FvmVm_FreeVirtualMemory(NtCurrentProcess(), &fvmObjPtr, &memSizeFvm,
				MEM_RELEASE);
	}
	if (objAttrPtr && objAttrPtr != ObjectAttributes) {
	    memSize = 0;
		FvmVm_FreeVirtualMemory(NtCurrentProcess(), &objAttrPtr, &memSize,
				MEM_RELEASE);
	}
	if (fnPtr) {
		ExFreePool(fnPtr);
	}

	InterlockedDecrement(&fvm_Calls_In_Progress);
	return rc;
}


/*
 *-----------------------------------------------------------------------------
 *
 * FvmFile_NtQueryAttributesFile --
 *
 *      This function is the FVM-provided ID_NTQUERYATTRIBUTESFILE system call
 *      function. If there is already a file existing under the FVM's
 *      workspace, we renames the file name argument to access the private
 *      file copy.
 *
 * Results:
 *      Return STATUS_SUCCESS on success or NTSTATUS error code on failure.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

NTSTATUS
FvmFile_NtQueryAttributesFile(IN POBJECT_ATTRIBUTES ObjectAttributes,
                              OUT PFILE_BASIC_INFORMATION FileInformation)
{
	NTSTATUS rc;
	POBJECT_ATTRIBUTES objAttrPtr = NULL;
	ULONG vmId = INVALID_VMID;
	PWCHAR fnPtr = NULL;
	ULONG memSize = _MAX_PATH, memSizeFvm = _MAX_PATH;
	POBJECT_ATTRIBUTES fvmObjPtr = NULL;

	InterlockedIncrement(&fvm_Calls_In_Progress);

	objAttrPtr = ObjectAttributes;
	vmId = FvmVm_GetPVMId((ULONG)PsGetCurrentProcessId());

	if (vmId != INVALID_VMID) {
		PWCHAR binPath = NULL;
		WCHAR fileLinkName[_MAX_PATH];
		WCHAR vDirName[_MAX_PATH];
		PWCHAR hostName, fvmName = NULL;

		if (!FvmUtil_GetSysCallArgument(ObjectAttributes, fileLinkName)) {
			goto winSysCall;
		}

		if (!FvmIsLocalFileAccess(fileLinkName)) {
			goto winSysCall;
		}

		if (!FvmFile_GetLongPathName(fileLinkName)) {
			goto winSysCall;
		}

		if (FvmFile_IsFileinFVM(fileLinkName, vmId, &fnPtr)) {
			/*
			 * See comments near the same function inside FvmFile_NtCreateFile
			 * in this code file.
             */

			if (FvmTable_FVMFileListLookup(fnPtr + 4, vmId)) {
				goto winSysCall;
			} else {
				objAttrPtr = NULL;
				rc = FvmUtil_InitializeVMObjAttributes(ObjectAttributes, fnPtr,
						&objAttrPtr, &memSize);

				if (!NT_SUCCESS(rc)) {
					rc = STATUS_OBJECT_PATH_NOT_FOUND;
					goto ntExit;
				}
				hostName = fnPtr;
				fvmName = fileLinkName;

				goto hostQuery;
			}
		} else {
			fnPtr = NULL;
			hostName = fileLinkName;

			if (!FvmTable_FVMFileListLookup(hostName + 4, vmId)) {
				goto hostQuery;
			} else {
				if (!FvmFile_MapPath(hostName, vmId, vDirName)) {
					goto winSysCall;
				}
				fvmName = vDirName;

				rc = FvmUtil_InitializeVMObjAttributes(ObjectAttributes, vDirName,
						&fvmObjPtr, &memSizeFvm);

				if (!NT_SUCCESS(rc)) {
					goto winSysCall;
				}

				rc = (winNtQueryAttributesFileProc)(fvmObjPtr,
						FileInformation);
				goto ntExit;
			}
		}

#if DBG_QUERYATTRIBUTESFILE
		binPath = ExAllocatePoolWithTag(PagedPool, _MAX_PATH * 2,
		    FVM_FILEOPT_POOL_TAG);
		if (binPath == NULL) {
			goto winSysCall;
		}
		FvmUtil_GetBinaryPathName(binPath);
		DbgPrint("ID_NTQUERYATTRIBUTESFILE : Application Name - %S\n", binPath);
		ExFreePool(binPath);
		DbgPrint("               Arguments - %S\n", hostName);
		DbgPrint("               New file name - %S\n", fvmName);
#endif

hostQuery:
        /*
         * The file to be accessed does not exist on the VM's workspace.
         * We need to check if it has been deleted before accessing it
         * from the host environment.
         */

		if (FvmTable_DeleteLogLookup(hostName, vmId)) {
			rc = STATUS_OBJECT_PATH_NOT_FOUND;
			goto ntExit;
		}
	}

winSysCall:
	rc = (winNtQueryAttributesFileProc)(objAttrPtr, FileInformation);

ntExit:

	if (fvmObjPtr && fvmObjPtr != ObjectAttributes) {
	    memSizeFvm = 0;
		FvmVm_FreeVirtualMemory(NtCurrentProcess(), &fvmObjPtr, &memSizeFvm,
				MEM_RELEASE);
	}
	if (objAttrPtr && objAttrPtr != ObjectAttributes) {
	    memSize = 0;
		FvmVm_FreeVirtualMemory(NtCurrentProcess(), &objAttrPtr, &memSize,
				MEM_RELEASE);
	}
	if (fnPtr) {
		ExFreePool(fnPtr);
	}

	InterlockedDecrement(&fvm_Calls_In_Progress);
	return rc;
}


/*
 *-----------------------------------------------------------------------------
 *
 * FvmFile_NtQueryFullAttributesFile --
 *
 *      This function is the FVM-provided NtQueryFullAttributesFile system
 *      call function. If there is already a file existing under the FVM's
 *      workspace, we renames the file name argument to access the private
 *      file copy.
 *
 * Results:
 *      Return STATUS_SUCCESS on success or NTSTATUS error code on failure.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

NTSTATUS
FvmFile_NtQueryFullAttributesFile(IN POBJECT_ATTRIBUTES ObjectAttributes,
                                  OUT PFILE_NETWORK_OPEN_INFORMATION
                                  		FileInformation)
{
	NTSTATUS rc;
	POBJECT_ATTRIBUTES objAttrPtr = NULL, fvmObjPtr = NULL;
	ULONG memSize = _MAX_PATH, memSizeFvm = _MAX_PATH;
	ULONG vmId = INVALID_VMID;
	PWCHAR fnPtr = NULL;

	InterlockedIncrement(&fvm_Calls_In_Progress);

	objAttrPtr = ObjectAttributes;
	vmId = FvmVm_GetPVMId((ULONG)PsGetCurrentProcessId());

	if (vmId != INVALID_VMID) {
		PWCHAR binPath = NULL;
		WCHAR fileLinkName[_MAX_PATH];
		WCHAR vDirName[_MAX_PATH];
		PWCHAR hostName, fvmName = NULL;

		if (!FvmUtil_GetSysCallArgument(ObjectAttributes, fileLinkName)) {
			goto winSysCall;
		}
		if (!FvmIsLocalFileAccess(fileLinkName)) {
			goto winSysCall;
		}
		if (!FvmFile_GetLongPathName(fileLinkName)) {
			goto winSysCall;
		}

		if (FvmFile_IsFileinFVM(fileLinkName, vmId, &fnPtr)) {
			/*
			 * See comments near the same function inside FvmFile_NtCreateFile
			 * in this code file.
             */

			if (FvmTable_FVMFileListLookup(fnPtr + 4, vmId)) {
				goto winSysCall;
			} else {
				objAttrPtr = NULL;
				rc = FvmUtil_InitializeVMObjAttributes(ObjectAttributes, fnPtr,
						&objAttrPtr, &memSize);

            	if (!NT_SUCCESS(rc)) {
					rc = STATUS_OBJECT_PATH_NOT_FOUND;
					goto ntExit;
				}
				hostName = fnPtr;
				fvmName = fileLinkName;

				goto hostQuery;
			}
		} else {
			fnPtr = NULL;
			hostName = fileLinkName;

			if (!FvmTable_FVMFileListLookup(hostName + 4, vmId)) {
				goto hostQuery;
			} else {
				if (!FvmFile_MapPath(hostName, vmId, vDirName)) {
					goto winSysCall;
				}
				fvmName = vDirName;

				rc = FvmUtil_InitializeVMObjAttributes(ObjectAttributes, vDirName,
						&fvmObjPtr, &memSizeFvm);

				if (!NT_SUCCESS(rc)) {
					goto winSysCall;
				}

				rc = (winNtQueryFullAttributesFileProc)(fvmObjPtr,
						FileInformation);

				goto ntExit;
			}
		}

#if DBG_QUERYFULLATTRIBUTESFILE
		binPath = ExAllocatePoolWithTag(PagedPool, _MAX_PATH * 2,
		    FVM_FILEOPT_POOL_TAG);
		if (binPath == NULL) {
			goto winSysCall;
		}
		FvmUtil_GetBinaryPathName(binPath);
		DbgPrint("NtQueryFullAttributesFile : Application Name - %S\n",
				binPath);
		ExFreePool(binPath);

		DbgPrint("               Arguments - %S\n", hostName);
		DbgPrint("               New file name - %S\n", fvmName);
#endif

hostQuery:
        /*
         * The file to be accessed does not exist on the VM's workspace.
         * We need to check if it has been deleted before accessing it
         * from the host environment.
         */

		if (FvmTable_DeleteLogLookup(hostName, vmId)) {
			rc = STATUS_OBJECT_PATH_NOT_FOUND;
			goto ntExit;
		}
	}

winSysCall:
	rc = (winNtQueryFullAttributesFileProc)(objAttrPtr, FileInformation);

ntExit:

	if (fvmObjPtr && fvmObjPtr != ObjectAttributes) {
	    memSizeFvm = 0;
		FvmVm_FreeVirtualMemory(NtCurrentProcess(), &fvmObjPtr, &memSizeFvm,
				MEM_RELEASE);
	}
	if (objAttrPtr && objAttrPtr != ObjectAttributes) {
	    memSize = 0;
		FvmVm_FreeVirtualMemory(NtCurrentProcess(), &objAttrPtr, &memSize,
				MEM_RELEASE);
	}
	if (fnPtr) {
		ExFreePool(fnPtr);
	}

	InterlockedDecrement(&fvm_Calls_In_Progress);
	return rc;
}


/*
 *-----------------------------------------------------------------------------
 *
 * FvmFile_NtDeleteFile --
 *
 *      This function is the FVM-provided NtDeleteFile system call function.
 *      If the target file exists on the host environment but not in the VM's
 *      workspace, we save the file path into the DeleteLog and mark it as
 *      having been deleted. Please note that this system call function is
 *      rarely invoked because the Win32 subsystem uses NtSetInformationFile
 *      to delete a file.
 *
 * Results:
 *      Return STATUS_SUCCESS on success or NTSTATUS error code on failure.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

NTSTATUS
FvmFile_NtDeleteFile(IN POBJECT_ATTRIBUTES ObjectAttributes)
{
	NTSTATUS rc;
	ULONG vmId = INVALID_VMID;
	POBJECT_ATTRIBUTES objAttrPtr = NULL, fvmObjPtr = NULL;
	ULONG memSize = _MAX_PATH, memSizeFvm = _MAX_PATH;
	PWCHAR fnPtr = NULL;
	BOOLEAN logFlag = FALSE;
	PWCHAR hostName = NULL, fvmName = NULL;

	InterlockedIncrement(&fvm_Calls_In_Progress);

	objAttrPtr = ObjectAttributes;
	vmId = FvmVm_GetPVMId((ULONG)PsGetCurrentProcessId());

	if (vmId != INVALID_VMID) {
		PWCHAR binPath = NULL;
		WCHAR fileLinkName[_MAX_PATH];
		WCHAR vDirName[_MAX_PATH];
		PFILE_BASIC_INFORMATION fileBasicInfoPtr = NULL;
		ULONG memSize1;

		if (!FvmUtil_GetSysCallArgument(ObjectAttributes, fileLinkName)) {
			goto winSysCall;
		}
		if (!FvmIsLocalFileAccess(fileLinkName)) {
			goto winSysCall;
		}
		if (!FvmFile_GetLongPathName(fileLinkName)) {
			goto winSysCall;
		}

		if (FvmFile_IsFileinFVM(fileLinkName, vmId, &fnPtr)) {
			/*
			 * See comments near the same function inside FvmFile_NtCreateFile
			 * in this code file.
             */

			if (FvmTable_FVMFileListLookup(fnPtr + 4, vmId)) {
				logFlag = TRUE;
				goto winSysCall;
			} else {
				objAttrPtr = NULL;
				rc = FvmUtil_InitializeVMObjAttributes(ObjectAttributes, fnPtr,
						&objAttrPtr, &memSize);

				if (!NT_SUCCESS(rc)) {
					rc = STATUS_ACCESS_DENIED;
					goto ntExit;
				}
				hostName = fnPtr;
				fvmName = fileLinkName;

				goto hostQuery;
			}
		} else {
			fnPtr = NULL;
			hostName = fileLinkName;

			if (!FvmTable_FVMFileListLookup(hostName + 4, vmId)) {
				goto hostQuery;
			} else {
				if (!FvmFile_MapPath(hostName, vmId, vDirName)) {
					rc = STATUS_ACCESS_DENIED;
					goto ntExit;
				}
				fvmName = vDirName;

				rc = FvmUtil_InitializeVMObjAttributes(ObjectAttributes, vDirName,
						&fvmObjPtr, &memSizeFvm);

				if (!NT_SUCCESS(rc)) {
					goto ntExit;
				}

				rc = (winNtDeleteFileProc)(fvmObjPtr);

				if (NT_SUCCESS(rc)) {
					/*
					 * Add the file path (virtual path) into the delete
					 * log. Also remove the path from the VM's private
					 * file tree in memory.
					 */

					FvmTable_DeleteLogAdd(hostName, vmId);
					FvmTable_FVMFileListDelete(hostName + 4, vmId);
				}
				goto ntExit;
			}
		}

#if DBG_DELETEFILE
		binPath = ExAllocatePoolWithTag(PagedPool, _MAX_PATH * 2,
		    FVM_FILEOPT_POOL_TAG);
		if (binPath == NULL) {
			rc = STATUS_ACCESS_DENIED;
			goto ntExit;
		}
		FvmUtil_GetBinaryPathName(binPath);
		DbgPrint("NtDeleteFile : Application Name - %S\n", binPath);
		ExFreePool(binPath);

		DbgPrint("               Arguments - %S\n", hostName);
		DbgPrint("               New file name - %S\n", fvmName);
#endif

hostQuery:
        /*
         * Check if the file has already been deleted (in the DeleteLog)
         * before.
         */

		if (FvmTable_DeleteLogLookup(hostName, vmId)) {
			rc = STATUS_OBJECT_PATH_NOT_FOUND;
			goto ntExit;
		}

		memSize1 = sizeof(FILE_BASIC_INFORMATION);
		rc = FvmVm_AllocateVirtualMemory(NtCurrentProcess(), &fileBasicInfoPtr,
				0, &memSize1, MEM_COMMIT, PAGE_READWRITE);

		if (!NT_SUCCESS(rc)) {
			rc = STATUS_ACCESS_DENIED;
			goto ntExit;
		}

        /*
         * We use ID_NTQUERYATTRIBUTESFILE to find out if the file exists
         * on the host environment.
         */

		rc = (winNtQueryAttributesFileProc)(objAttrPtr, fileBasicInfoPtr);

        memSize1 = 0;
		FvmVm_FreeVirtualMemory(NtCurrentProcess(), &fileBasicInfoPtr, &memSize1,
				MEM_RELEASE);

		if (NT_SUCCESS(rc)||(rc != STATUS_OBJECT_PATH_NOT_FOUND
				&& rc != STATUS_OBJECT_NAME_NOT_FOUND)) {
			/*
			 * We assume that if the file exists on the host directory,
			 * then it can be deleted.
			 */

			FvmTable_DeleteLogAdd(hostName, vmId);
			rc = STATUS_SUCCESS;
		}
		goto ntExit;
	}

winSysCall:
    /*
     * If the file to be deleted exists in the VM's workspace, we can delete
     * it and then update the DeleteLog.
     */

	rc = (winNtDeleteFileProc)(objAttrPtr);

	if (logFlag && NT_SUCCESS(rc) && hostName) {
		FvmTable_DeleteLogAdd(hostName, vmId);
		FvmTable_FVMFileListDelete(hostName + 4, vmId);
	}

ntExit:
	if (fvmObjPtr && fvmObjPtr != ObjectAttributes) {
	    memSizeFvm = 0;
		FvmVm_FreeVirtualMemory(NtCurrentProcess(), &fvmObjPtr, &memSizeFvm,
				MEM_RELEASE);
	}
	if (objAttrPtr && objAttrPtr != ObjectAttributes) {
	    memSize = 0;
		FvmVm_FreeVirtualMemory(NtCurrentProcess(), &objAttrPtr, &memSize,
				MEM_RELEASE);
	}
	if (fnPtr) {
		ExFreePool(fnPtr);
	}

	InterlockedDecrement(&fvm_Calls_In_Progress);
	return rc;
}

#endif // ifdef USE_FS_MAP
