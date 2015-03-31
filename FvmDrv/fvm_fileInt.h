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
 * fvm_fileInt.h --
 *
 *	This header file describes prototypes of a few utility functions
 *  used internally by the file virtualization module.
 */

#ifndef _FVM_FILEINT_H_
#define _FVM_FILEINT_H_


extern BOOLEAN FvmFile_GetLongPathName(PWCHAR FileName);

extern BOOLEAN FvmFile_IsOpenforWrite(
	ACCESS_MASK DesiredAccess,
	ULONG ShareAccess,
	ULONG OpenOptions
);

extern BOOLEAN FvmFile_IsFileinFVM(
	PWCHAR VmFile,
	ULONG VmId,
	PWCHAR *HostFilePtr
);

extern BOOLEAN FvmFile_IsPipeMailslotConnect(PWCHAR FileName);

extern BOOLEAN FvmFile_AllowDeviceAccess(PWCHAR FileLinkName);

#endif // ifndef _FVM_FILEINT_H_
