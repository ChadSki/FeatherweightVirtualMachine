
#include "stdafx.h"
#include "assert.h"
#include "Ldpreload.h"

// One approach to get the entry point of a process by VirtualQueryEx()
HRESULT GetEntryPointAddr1(
	const	HANDLE hProcess,
	DWORD*	pdwEntryAddr)
{
	HRESULT			hr = S_OK;
	LPVOID			lpMem = 0;
	MEMORY_BASIC_INFORMATION	pmemBuff;
	LPVOID			lpFileBase = NULL;
	PIMAGE_DOS_HEADER	pDOSHeader;
   	PIMAGE_NT_HEADERS	pNTHeader;

	*pdwEntryAddr = 0;

	/* scan entire process for contiguous blocks of memory */
	while ((DWORD)lpMem < 0x7fff0000) {
		VirtualQueryEx(
			hProcess,
			lpMem,
			&pmemBuff,
			sizeof (pmemBuff));

		if (pmemBuff.Type == MEM_IMAGE) {
			lpFileBase = pmemBuff.BaseAddress;
			break;
		}
		/* increment lpMem to next region of memory */
		lpMem = (LPVOID)((DWORD)pmemBuff.BaseAddress +
				 (DWORD)pmemBuff.RegionSize);
	}
	
	pDOSHeader = (PIMAGE_DOS_HEADER)lpFileBase;

	if (pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE) return E_FAIL;

	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD) lpFileBase + pDOSHeader->e_lfanew);
	if (pNTHeader->Signature != IMAGE_NT_SIGNATURE) return E_FAIL;

	if (pNTHeader->FileHeader.Machine != IMAGE_FILE_MACHINE_I386) return E_FAIL;

	if (pNTHeader->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC) return E_FAIL;

	*pdwEntryAddr = (pNTHeader->OptionalHeader.ImageBase + pNTHeader->OptionalHeader.AddressOfEntryPoint);

	return hr;
}

/*
 * GetEntryPointAddr
 *
 * Gets the address of the EXE's entry point: the point at which the EXE
 * will begin executing.
 */

HRESULT GetEntryPointAddr(
	const char*	szEXE,
	DWORD*		pdwEntryAddr)
{
	HRESULT				hr = S_OK;
	HANDLE				hFile = INVALID_HANDLE_VALUE;
	HANDLE				hFileMapping = INVALID_HANDLE_VALUE;
	LPVOID				lpFileBase = NULL;
	PIMAGE_DOS_HEADER	pDOSHeader;
   	PIMAGE_NT_HEADERS	pNTHeader;

	*pdwEntryAddr = 0;

	hFile = CreateFileA(szEXE, GENERIC_READ, FILE_SHARE_READ, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (hFile == INVALID_HANDLE_VALUE) goto Error;

	hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	if (hFileMapping == INVALID_HANDLE_VALUE) goto Error;

	lpFileBase = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);
	if (lpFileBase == INVALID_HANDLE_VALUE) goto Error;

	pDOSHeader = (PIMAGE_DOS_HEADER)lpFileBase;

	if (pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE) goto Error;

	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD) lpFileBase + pDOSHeader->e_lfanew);
	if (pNTHeader->Signature != IMAGE_NT_SIGNATURE) goto Error;

	if (pNTHeader->FileHeader.Machine != IMAGE_FILE_MACHINE_I386) goto Error;

	if (pNTHeader->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC) goto Error;

	*pdwEntryAddr = (pNTHeader->OptionalHeader.ImageBase + pNTHeader->OptionalHeader.AddressOfEntryPoint);

Error:
	if (lpFileBase != NULL) {
		UnmapViewOfFile(lpFileBase);
		lpFileBase = NULL;
	}

	if (hFileMapping != INVALID_HANDLE_VALUE) {
		CloseHandle(hFileMapping);
		hFileMapping = INVALID_HANDLE_VALUE;
	}

	if (hFile != INVALID_HANDLE_VALUE) {
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}
	return hr;
}


/*
 * WriteEntryCode
 *
 * Writes the new entry code to the BYTE pointer given.  This entry code
 * should be exactly NUM_ENTRY_CODE_BYTES long (an assert will fire
 * otherwise).  This code simply jumps to the address given as a parameter
 * to this function, which is where the instructions for loading the DLL
 * will exist within the process.
 */
HRESULT WriteEntryCode(
	BYTE*	pbEntryCode,
	DWORD	dwLDPreloadInstrAddr)
{
	HRESULT	hr = S_OK;
	BYTE*	pbEntryCodeCounter = pbEntryCode;

	/* __asm mov eax, dwLDPreloadInstrAddr; */
	*pbEntryCodeCounter++ = 0xB8;
	*((DWORD*) pbEntryCodeCounter) = (DWORD) dwLDPreloadInstrAddr;
	pbEntryCodeCounter += sizeof(DWORD);

	/* __asm jmp eax; */
	*pbEntryCodeCounter++ = 0xFF;
	*pbEntryCodeCounter++ = 0xE0;

	assert(pbEntryCodeCounter - pbEntryCode == NUM_ENTRY_CODE_BYTES);

	return hr;
}


/*
 * WriteLDPreloadCode
 *
 * Writes the code which will call LoadLibrary and then restore the original
 * instructions back to the process entry point, then jumping back to the
 * entry point.
 */
HRESULT WriteLDPreloadCode(
	BYTE*		pbLDPreloadCode,
	DWORD		dwLDPreloadInstrAddr,
	const BYTE*	pbOrigEntryCode,
	DWORD		dwProcessEntryCodeAddr,
	const char*	szDLL)
{
	HRESULT			hr = S_OK;
	HMODULE			hmodKernelDLL = NULL;
	FARPROC			farprocLoadLibrary;
	FARPROC			farprocGetCurrentProcess;
	FARPROC			farprocWriteProcessMemory;
	DWORD			dwDataAreaStartAddr;
	DWORD			dwDataAreaDLLStringAddr; // address for DLL string within process
	int				nBytesDLLString;
	DWORD			dwDataAreaOrigInstrAddr; // address for original instructions within process
	int				nBytesOrigInstr;
	BYTE*			pbCurrentArrayPtr;
	const int		k_nDataAreaOffsetBytes = 400; // offset from dwLDPreloadInstrAddr where data area will start

	hmodKernelDLL = LoadLibraryA("kernel32.dll");
	if (hmodKernelDLL == NULL) goto Error;

	farprocLoadLibrary = GetProcAddress(hmodKernelDLL, "LoadLibraryA");
	if (farprocLoadLibrary == NULL) goto Error;

	farprocGetCurrentProcess = GetProcAddress(hmodKernelDLL, "GetCurrentProcess");
	if (farprocGetCurrentProcess == NULL) goto Error;

	farprocWriteProcessMemory = GetProcAddress(hmodKernelDLL, "WriteProcessMemory");
	if (farprocWriteProcessMemory == NULL) goto Error;

	pbCurrentArrayPtr = pbLDPreloadCode;

	/*
	 * Initialize the addresses to the data area members.
	 */
	dwDataAreaStartAddr = dwLDPreloadInstrAddr + k_nDataAreaOffsetBytes;
	dwDataAreaDLLStringAddr = dwDataAreaStartAddr;
	nBytesDLLString = strlen(szDLL) + 1;
	dwDataAreaOrigInstrAddr = dwDataAreaDLLStringAddr + nBytesDLLString;
	nBytesOrigInstr = NUM_ENTRY_CODE_BYTES;

	/* Fill with 'int 3' instructions for safety */
	memset(pbCurrentArrayPtr, 0xCC, NUM_LDPRELOAD_CODE_BYTES);

	/*
	 * Write the instructions which call LoadLibrary() on szDLL within
	 * the process.
	 */

	/* __asm mov eax, lpDLLStringStart; */
	*pbCurrentArrayPtr++ = 0xB8;
	*((DWORD*) pbCurrentArrayPtr) = (DWORD) dwDataAreaDLLStringAddr;
	pbCurrentArrayPtr += sizeof(DWORD);

	/* __asm push eax */
	*pbCurrentArrayPtr++ = 0x50;

	/* __asm mov eax, farprocLoadLibrary; */
	*pbCurrentArrayPtr++ = 0xB8;
	*((DWORD*) pbCurrentArrayPtr) = (DWORD) farprocLoadLibrary;
	pbCurrentArrayPtr += sizeof(DWORD);

	/* __asm call eax; */
	*pbCurrentArrayPtr++ = 0xFF;
	*pbCurrentArrayPtr++ = 0xD0;

	/*
	 * Write the instructions which will copy the original instructions
	 * back to the process's entry point address.  Must use
	 * WriteProcessMemory() for security reasons.
	 */

	/* pushing arguments to WriteProcessMemory()... */

	// lpNumberOfBytesWritten == NULL
	/* __asm mov eax, 0x0; */
	*pbCurrentArrayPtr++ = 0xB8;
	*((DWORD*) pbCurrentArrayPtr) = (DWORD) 0x0;
	pbCurrentArrayPtr += sizeof(DWORD);

	/* __asm push eax */
	*pbCurrentArrayPtr++ = 0x50;

	// nSize == nBytesOrigInstr
	/* __asm mov eax, nBytesOrigInstr; */
	*pbCurrentArrayPtr++ = 0xB8;
	*((DWORD*) pbCurrentArrayPtr) = (DWORD) nBytesOrigInstr;
	pbCurrentArrayPtr += sizeof(DWORD);

	/* __asm push eax */
	*pbCurrentArrayPtr++ = 0x50;

	// lpBuffer == dwDataAreaOrigInstrAddr
	/* __asm mov eax, dwDataAreaOrigInstrAddr; */
	*pbCurrentArrayPtr++ = 0xB8;
	*((DWORD*) pbCurrentArrayPtr) = (DWORD) dwDataAreaOrigInstrAddr;
	pbCurrentArrayPtr += sizeof(DWORD);

	/* __asm push eax */
	*pbCurrentArrayPtr++ = 0x50;

	// lpBaseAddress == dwProcessEntryCodeAddr
	/* __asm mov eax, dwProcessEntryCodeAddr; */
	*pbCurrentArrayPtr++ = 0xB8;
	*((DWORD*) pbCurrentArrayPtr) = (DWORD) dwProcessEntryCodeAddr;
	pbCurrentArrayPtr += sizeof(DWORD);
	/* __asm push eax */
	*pbCurrentArrayPtr++ = 0x50;

	// GetCurrentProcess()
	/* __asm mov eax, farprocGetCurrentProcess; */
	*pbCurrentArrayPtr++ = 0xB8;
	*((DWORD*) pbCurrentArrayPtr) = (DWORD) farprocGetCurrentProcess;
	pbCurrentArrayPtr += sizeof(DWORD);

	/* __asm call eax; */
	*pbCurrentArrayPtr++ = 0xFF;
	*pbCurrentArrayPtr++ = 0xD0;

	// hProcess == GetCurrentProcess() == eax
	/* __asm push eax */
	*pbCurrentArrayPtr++ = 0x50;

	/* Done pushing arguments, call WriteProcessMemory() */

	/* __asm mov eax, farprocWriteProcessMemory; */
	*pbCurrentArrayPtr++ = 0xB8;
	*((DWORD*) pbCurrentArrayPtr) = (DWORD) farprocWriteProcessMemory;
	pbCurrentArrayPtr += sizeof(DWORD);
	
	/* __asm call eax; */
	*pbCurrentArrayPtr++ = 0xFF;
	*pbCurrentArrayPtr++ = 0xD0;
	
	/* Jump back to the processes's original entry point address */

	/* __asm mov eax, dwProcessEntryCodeAddr; */
	*pbCurrentArrayPtr++ = 0xB8;
	*((DWORD*) pbCurrentArrayPtr) = (DWORD) dwProcessEntryCodeAddr;
	pbCurrentArrayPtr += sizeof(DWORD);

	/* __asm jmp eax; */
	*pbCurrentArrayPtr++ = 0xFF;
	*pbCurrentArrayPtr++ = 0xE0;

	/*
	 * Initialize the 'data area' within the process.
	 */

	pbCurrentArrayPtr = pbLDPreloadCode + k_nDataAreaOffsetBytes;
	
	/* szDLL string */
	memcpy(pbCurrentArrayPtr, szDLL, nBytesDLLString);
	pbCurrentArrayPtr += nBytesDLLString;

	/* origInstr */
	memcpy(pbCurrentArrayPtr, pbOrigEntryCode, nBytesOrigInstr);
	pbCurrentArrayPtr += nBytesOrigInstr;

Error:
	if (hmodKernelDLL != NULL) {
		FreeLibrary(hmodKernelDLL);
		hmodKernelDLL = NULL;
	}

	return hr;
}
