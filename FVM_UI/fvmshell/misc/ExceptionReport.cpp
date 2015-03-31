// FileZilla - a Windows ftp client

// Copyright (C) 2004 - Tim Kosse <tim.kosse@gmx.de>

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

// OptionsTypePage.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include <dbghelp.h>
#include "ExceptionReport.h"
#include "..\version.h"
#include "ProcessorInfo.h"
#include "WindowsVersion.h"
#include "mailmsg.h"
#include "Tlhelp32.h"

typedef BOOL
(_stdcall *tSymFromAddr)(
	IN  HANDLE			hProcess,
	IN  DWORD64			Address,
	OUT PDWORD64		Displacement,
	IN OUT PSYMBOL_INFO	Symbol
	);

typedef DWORD
(_stdcall *tSymGetOptions)(
	);

typedef DWORD
(_stdcall *tSymSetOptions)(
	IN DWORD   SymOptions
	);

typedef BOOL
(_stdcall *tSymCleanup)(
	IN HANDLE hProcess
	);

typedef BOOL
(_stdcall *tSymInitialize)(
	IN HANDLE	hProcess,
	IN PSTR		UserSearchPath,
	IN BOOL		fInvadeProcess
	);

typedef BOOL
(_stdcall *tSymGetLineFromAddr)(
	IN  HANDLE				hProcess,
	IN  DWORD				dwAddr,
	OUT PDWORD				pdwDisplacement,
	OUT PIMAGEHLP_LINE		Line
	);

typedef BOOL
(_stdcall *tStackWalk)(
	DWORD							MachineType,
	HANDLE							hProcess,
	HANDLE							hThread,
	LPSTACKFRAME					StackFrame,
	PVOID							ContextRecord,
	PREAD_PROCESS_MEMORY_ROUTINE	ReadMemoryRoutine,
	PFUNCTION_TABLE_ACCESS_ROUTINE	FunctionTableAccessRoutine,
	PGET_MODULE_BASE_ROUTINE		GetModuleBaseRoutine,
	PTRANSLATE_ADDRESS_ROUTINE		TranslateAddress
	);

typedef PVOID
(_stdcall *tSymFunctionTableAccess)(
	HANDLE  hProcess,
	DWORD   AddrBase
	);

typedef DWORD
(_stdcall *tSymGetModuleBase)(
	IN  HANDLE			  hProcess,
	IN  DWORD			   dwAddr
	);

typedef BOOL
(_stdcall *tMiniDumpWriteDump)(
	HANDLE hProcess,
	DWORD ProcessId,
	HANDLE hFile,
	MINIDUMP_TYPE DumpType,
	PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
	PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
	PMINIDUMP_CALLBACK_INFORMATION CallbackParam
	);

static tSymCleanup				pSymCleanup;
static tSymInitialize			pSymInitialize;
static tSymGetOptions			pSymGetOptions;
static tSymSetOptions			pSymSetOptions;
static tSymGetLineFromAddr		pSymGetLineFromAddr;
static tSymFromAddr				pSymFromAddr;
static tStackWalk				pStackWalk;
static tSymFunctionTableAccess	pSymFunctionTableAccess;
static tSymGetModuleBase		pSymGetModuleBase;
static tMiniDumpWriteDump		pMiniDumpWriteDump;

// Global class instance
static CExceptionReport ExceptionReport;

LPTOP_LEVEL_EXCEPTION_FILTER CExceptionReport::m_previousExceptionFilter;
TCHAR CExceptionReport::m_pLogFileName[MAX_PATH];
HANDLE CExceptionReport::m_hReportFile;
TCHAR CExceptionReport::m_pDmpFileName[MAX_PATH];
HANDLE CExceptionReport::m_hDumpFile;
BOOL CExceptionReport::m_bFirstRun;

CExceptionReport::CExceptionReport()
{
	m_bFirstRun = TRUE;

	m_previousExceptionFilter = SetUnhandledExceptionFilter(UnhandledExceptionFilter);

	// Retrieve report/dump filenames
	GetModuleFileName(0, m_pLogFileName, MAX_PATH);

	// Look for the '.' before the "EXE" extension.  Replace the extension
	// with "RPT"
	LPTSTR p = _tcsrchr(m_pLogFileName, _T('.'));
	if (p)
	{
		p++;
		*p = 0;
		_tcscpy(m_pDmpFileName, m_pLogFileName);
		_tcscpy(p, _T("rpt"));
		_tcscat(m_pDmpFileName, _T("dmp"));
	}
}

CExceptionReport::~CExceptionReport()
{
	SetUnhandledExceptionFilter(m_previousExceptionFilter);
}

LONG WINAPI CExceptionReport::UnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo)
{
	if (!m_bFirstRun)
	{
		// Don't generate exception report twice
		if (m_previousExceptionFilter)
			return m_previousExceptionFilter(pExceptionInfo);
		else
			return EXCEPTION_CONTINUE_SEARCH;
	}
	else
		m_bFirstRun = FALSE;

	// Suspend all threads to freeze the current state
	SuspendThreads();
	
	HMODULE hDll = LoadLibrary(_T("dbghelp.dll"));
	if (!hDll)
	{
		if (m_previousExceptionFilter)
			return m_previousExceptionFilter(pExceptionInfo);
		else
			return EXCEPTION_CONTINUE_SEARCH;
	}

	pSymCleanup				= (tSymCleanup)GetProcAddress(hDll, "SymCleanup");
	pSymInitialize			= (tSymInitialize)GetProcAddress(hDll, "SymInitialize");
	pSymGetOptions			= (tSymGetOptions)GetProcAddress(hDll, "SymGetOptions");
	pSymSetOptions			= (tSymSetOptions)GetProcAddress(hDll, "SymSetOptions");
	pSymGetLineFromAddr		= (tSymGetLineFromAddr)GetProcAddress(hDll, "SymGetLineFromAddr");
	pSymFromAddr			= (tSymFromAddr)GetProcAddress(hDll, "SymFromAddr");
	pStackWalk				= (tStackWalk)GetProcAddress(hDll, "StackWalk");
	pSymFunctionTableAccess	= (tSymFunctionTableAccess)GetProcAddress(hDll, "SymFunctionTableAccess");
	pSymGetModuleBase		= (tSymGetModuleBase)GetProcAddress(hDll, "SymGetModuleBase");
	pMiniDumpWriteDump		= (tMiniDumpWriteDump)GetProcAddress(hDll, "MiniDumpWriteDump");

	if (!pSymCleanup			||
		!pSymInitialize			||
		!pSymGetOptions			||
		!pSymSetOptions			||
		!pSymGetLineFromAddr	||
		!pSymFromAddr			||
		!pStackWalk				||
		!pSymFunctionTableAccess||
		!pSymGetModuleBase		||
		!pMiniDumpWriteDump)
	{
		FreeLibrary(hDll);
		if (m_previousExceptionFilter)
			return m_previousExceptionFilter(pExceptionInfo);
		else
			return EXCEPTION_CONTINUE_SEARCH;
	}

	if (::MessageBox(NULL,
_T("An unhandled exception has occurred in FileZilla\r\n\
FileZilla has to be closed.\r\n\r\n\
Would you like to generate an exception report?\r\n\
The report contains all neccessary information about the exception,\r\n\
including a call stack with function parameters and local variables.\r\n\r\n\
If you're using the latest version of FileZilla, please send the generated exception record to the following mail address: Tim.Kosse@gmx.de\r\n\
The report will be analyzed and the reason for this exception will be fixed in the next version of FileZilla.\r\n\r\n\
Please note: It may be possible - though unlikely - that the exception report may contain personal and or confidential information. All exception reports will be processed higly confidential and solely to analyze the crash. The reports will be deleted immediately after processing.\r\n"),
		_T("FileZilla - Unhandled exception"), MB_APPLMODAL | MB_YESNO | MB_ICONSTOP)==IDYES)
	{
		m_hReportFile = CreateFile(m_pLogFileName, GENERIC_WRITE,FILE_SHARE_READ,
								   0, CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH, 0);
	
		m_hDumpFile = CreateFile(m_pDmpFileName, GENERIC_WRITE, FILE_SHARE_READ,
								 0, CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH,	0);
	
		if (m_hReportFile == INVALID_HANDLE_VALUE)
		{
			TCHAR tmp[MAX_PATH];
			_tcscpy(tmp, m_pLogFileName);
			TCHAR *pos=_tcsrchr(tmp, '\\');
			if (pos)
			{
				pos++;
				_stprintf(m_pLogFileName, _T("c:\\%s"), pos);
			}
			else
				_stprintf(m_pLogFileName, _T("c:\\%s"), tmp);
		
			m_hReportFile = CreateFile(m_pLogFileName, GENERIC_WRITE,FILE_SHARE_READ,
									   0, CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH, 0);
		}
		if (m_hDumpFile == INVALID_HANDLE_VALUE)
		{
			TCHAR tmp[MAX_PATH];
			_tcscpy(tmp, m_pDmpFileName);
			TCHAR *pos=_tcsrchr(tmp, '\\');
			if (pos)
			{
				pos++;
				_stprintf(m_pDmpFileName, _T("c:\\%s"), pos);
			}
			else
				_stprintf(m_pDmpFileName, _T("c:\\%s"), tmp);
		
			m_hDumpFile = CreateFile(m_pDmpFileName, GENERIC_WRITE, FILE_SHARE_READ,
									 0, CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH,	0);
		}

		int nError=0;
		if (m_hReportFile == INVALID_HANDLE_VALUE && INVALID_HANDLE_VALUE)
			nError = GetLastError();
		else
		{
#ifdef TRY
			TRY
#endif
			{
				if (m_hReportFile != INVALID_HANDLE_VALUE)
					CreateReport(pExceptionInfo);
	
				CloseHandle(m_hReportFile);
			}
#ifdef TRY
			CATCH_ALL(e);
			{
				nError = GetLastError();
				CloseHandle(m_hReportFile);
			}
			END_CATCH_ALL

			TRY
#endif
			{
				if (m_hDumpFile != INVALID_HANDLE_VALUE)
					writeMiniDump(pExceptionInfo);

				CloseHandle(m_hDumpFile);
				nError = 0;
			}
#ifdef TRY
			CATCH_ALL(e);
			{
				CloseHandle(m_hDumpFile);
			}
			END_CATCH_ALL
#endif
		}

		if (nError)
		{
		
			TCHAR tmp[1000];
			_stprintf(tmp, _T("Unable to create exception report, error code %d."), nError);
			MessageBox(0, tmp, _T("FileZilla - Unhandled eception"), MB_APPLMODAL | MB_OK | MB_ICONEXCLAMATION);
		}
		else
		{
			sendMail();

			TCHAR tmp[1000];
			_stprintf(tmp, _T("The exception report has been saved to \"%s\" and \"%s\".\n\
Please make sure that you are using the latest version of FileZilla.\n\
You can download the latest version from http://sourceforge.net/projects/filezilla/.\n\
If you do use the latest version, please send the exception report to Tim.Kosse@gmx.de along with a brief explanation what you did before FileZilla crashed."), m_pLogFileName, m_pDmpFileName);
			MessageBox(0, tmp, _T("FileZilla - Unhandled eception"), MB_APPLMODAL | MB_OK | MB_ICONEXCLAMATION);

			FreeLibrary(hDll);
			return EXCEPTION_CONTINUE_SEARCH;
		}
	}
	FreeLibrary(hDll);
	if (m_previousExceptionFilter)
		return m_previousExceptionFilter(pExceptionInfo);
	else
		return EXCEPTION_CONTINUE_SEARCH;

}

void CExceptionReport::CreateReport(PEXCEPTION_POINTERS pExceptionInfo)
{
	// Start out with a banner
	AddToReport("Exception report created by ");
	AddToReport(GetVersionString());
	AddToReport("\r\n===================================================\r\n\r\n");
	AddToReport("System details:\r\n");
	AddToReport("---------------\r\n\r\nOperating System:      ");
	
	TCHAR buffer[200];
	if (DisplaySystemVersion(buffer))
	{
		AddToReport(buffer);
		AddToReport("\r\n");
	}
	else
		AddToReport("Could not get OS version\r\n");
	
	CProcessorInfo pi;
	CMemoryInfo mi;
	AddToReport("Processor Information: ");
	AddToReport(pi.GetProcessorName());
	AddToReport("\r\nMemory Information:    ");
	AddToReport(mi.GetMemoryInfo());
	
	PEXCEPTION_RECORD pExceptionRecord = pExceptionInfo->ExceptionRecord;

	// Print fault type
	AddToReport("\r\nException Details:\r\n------------------\r\n\r\nException code: ");
	AddToReportHex(pExceptionRecord->ExceptionCode, 8);
	AddToReport(" ");
	AddToReport(GetExceptionString(pExceptionRecord->ExceptionCode));
	
	// Add fault address and module
	TCHAR szModule[MAX_PATH];
	memset(szModule, 0, MAX_PATH);
	DWORD dwSection, dwOffset;
	GetAddrDetails(pExceptionRecord->ExceptionAddress,
					  szModule,
					  sizeof(szModule),
					  dwSection, dwOffset);

	AddToReport("\r\nFault address:  ");
	AddToReportHex((int)pExceptionRecord->ExceptionAddress, 8);
	AddToReport(" ");
	AddToReportHex(dwSection, 2);
	AddToReport(":");
	AddToReportHex(dwOffset, 8);
	AddToReport(" ");
	AddToReport(szModule);
	AddToReport("\r\n");

	// Set up the symbol engine.
	DWORD dwOptions = pSymGetOptions() ;

	// Turn on line loading and deferred loading.
	pSymSetOptions(dwOptions | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);

	PCONTEXT pContext = pExceptionInfo->ContextRecord;

	// Initialize DbgHelp
	if (!pSymInitialize(GetCurrentProcess(), 0, TRUE))
		return;

	StackWalk(*pContext);

	pSymCleanup(GetCurrentProcess());
}

LPTSTR CExceptionReport::GetExceptionString(DWORD dwCode)
{
	#define EXCEPTION(x) case EXCEPTION_##x: return _T(#x);

	switch (dwCode)
	{
		EXCEPTION(ACCESS_VIOLATION)
		EXCEPTION(DATATYPE_MISALIGNMENT)
		EXCEPTION(BREAKPOINT)
		EXCEPTION(SINGLE_STEP)
		EXCEPTION(ARRAY_BOUNDS_EXCEEDED)
		EXCEPTION(FLT_DENORMAL_OPERAND)
		EXCEPTION(FLT_DIVIDE_BY_ZERO)
		EXCEPTION(FLT_INEXACT_RESULT)
		EXCEPTION(FLT_INVALID_OPERATION)
		EXCEPTION(FLT_OVERFLOW)
		EXCEPTION(FLT_STACK_CHECK)
		EXCEPTION(FLT_UNDERFLOW)
		EXCEPTION(INT_DIVIDE_BY_ZERO)
		EXCEPTION(INT_OVERFLOW)
		EXCEPTION(PRIV_INSTRUCTION)
		EXCEPTION(IN_PAGE_ERROR)
		EXCEPTION(ILLEGAL_INSTRUCTION)
		EXCEPTION(NONCONTINUABLE_EXCEPTION)
		EXCEPTION(STACK_OVERFLOW)
		EXCEPTION(INVALID_DISPOSITION)
		EXCEPTION(GUARD_PAGE)
		EXCEPTION(INVALID_HANDLE)
	}

	// Try to get descripbion of unknown exceptions
	static TCHAR buffer[512] = {0};

	FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
				  GetModuleHandle(_T("NTDLL.DLL")),
				  dwCode, 0, buffer, sizeof(buffer), 0);

	return buffer;
}

void CExceptionReport::StackWalk(CONTEXT Context)
{
	USES_CONVERSION;
	AddToReport("\r\nCall stack:\r\n-----------\r\n\r\n");
	AddToReport("Address   Frame     Function			SourceFile\r\n");
	
	DWORD dwMachineType = 0;

	STACKFRAME sf;
	memset(&sf, 0, sizeof(sf));

#ifdef _M_IX86
	// Initialize the STACKFRAME structure for the first call.  This is only
	// necessary for Intel CPUs, and isn't mentioned in the documentation.
	sf.AddrPC.Offset	= Context.Eip;
	sf.AddrPC.Mode		= AddrModeFlat;
	sf.AddrStack.Offset	= Context.Esp;
	sf.AddrStack.Mode	= AddrModeFlat;
	sf.AddrFrame.Offset	= Context.Ebp;
	sf.AddrFrame.Mode	= AddrModeFlat;

	dwMachineType = IMAGE_FILE_MACHINE_I386;
#endif

#ifdef _M_IX86
	while (TRUE)
	{
		// Get next stack frame
		if (!pStackWalk(dwMachineType, GetCurrentProcess(), GetCurrentThread(),
						&sf, &Context, 0, 
						pSymFunctionTableAccess, pSymGetModuleBase,	0))
			break;

		if (!sf.AddrFrame.Offset)
			break; //Invalid frame

		AddToReportHex(sf.AddrPC.Offset, 8);
		AddToReport("  ");
		AddToReportHex(sf.AddrFrame.Offset, 8);
		AddToReport("  ");
		
		// Get function name for stack frame entry
		BYTE symbolBuffer[ sizeof(SYMBOL_INFO) + 1024 ];
		PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)symbolBuffer;
		pSymbol->SizeOfStruct = sizeof(symbolBuffer);
		pSymbol->MaxNameLen = 1024;
					
		DWORD64 symDisplacement = 0;	// Displacement of the input address,
										// relative to the start of the symbol

		if (pSymFromAddr(GetCurrentProcess(), sf.AddrPC.Offset, &symDisplacement, pSymbol))
		{
			AddToReport(pSymbol->Name);
			AddToReport("+");
			AddToReportHex(symDisplacement);
		}
		else	// No symbol found.  Print out the logical address instead.
		{
			TCHAR szModule[MAX_PATH] = _T("");
			DWORD section = 0, offset = 0;

			GetAddrDetails((PVOID)sf.AddrPC.Offset,
								szModule, sizeof(szModule), section, offset);

			AddToReportHex(section, 4);
			AddToReport(":");
			AddToReportHex(offset, 8);
			AddToReport(" ");
			AddToReport(szModule);
		}

		// Get the source line for this stack frame entry
		IMAGEHLP_LINE lineInfo = { sizeof(IMAGEHLP_LINE) };
		DWORD dwLineDisplacement;
		if (pSymGetLineFromAddr(GetCurrentProcess(), sf.AddrPC.Offset,
								&dwLineDisplacement, &lineInfo))
		{
			AddToReport("  ");
			AddToReport(lineInfo.FileName);
			AddToReport(" line ");
			AddToReport(lineInfo.LineNumber);
		}

		AddToReport("\r\n");
	}
#else
	AddToReport("Cannot dump stack yet on non-x86 systems!\r\n");
#endif

}

bool CExceptionReport::writeMiniDump(PEXCEPTION_POINTERS pExceptionInfo)
{
	// Write the minidump to the file
	MINIDUMP_EXCEPTION_INFORMATION eInfo;
	eInfo.ThreadId = GetCurrentThreadId();
	eInfo.ExceptionPointers = pExceptionInfo;
	eInfo.ClientPointers = FALSE;

	MINIDUMP_CALLBACK_INFORMATION cbMiniDump;
	cbMiniDump.CallbackRoutine = 0;
	cbMiniDump.CallbackParam = 0;


	pMiniDumpWriteDump(
		GetCurrentProcess(),
		GetCurrentProcessId(),
		m_hDumpFile,
		MiniDumpNormal,
		pExceptionInfo ? &eInfo : NULL,
		NULL,
		&cbMiniDump);

	// Close file
	CloseHandle(m_hDumpFile);

	return true;
}

int CExceptionReport::sendMail()
{
	CMailMsg mail;

	mail.SetTo(_T("tim.kosse@gmx.de"), _T("Tim Kosse"));

	TCHAR str[4096];
	_stprintf(str, _T("Exception report created by %s\r\n\r\n"), (LPCTSTR)GetVersionString());
	mail.SetSubject(str);

	mail.SetMessage(_T("Enter your comments here, what did you do with FileZilla before it crashed?"));

	mail.AddAttachment(m_pLogFileName, _T("FileZilla.rpt"));
	mail.AddAttachment(m_pDmpFileName, _T("FileZilla.dmp"));

	return mail.Send();
}

void CExceptionReport::SuspendThreads()
{
	// Try to get OpenThread and Thread32* function from kernel32.dll, since it's not available on Win95/98
	typedef HANDLE (WINAPI *tOpenThread)	(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwThreadId);
	typedef BOOL (WINAPI *tThread32First)	(HANDLE hSnapshot, LPTHREADENTRY32 lpte);
	typedef BOOL (WINAPI *tThread32Next)	(HANDLE hSnapshot, LPTHREADENTRY32 lpte);
	typedef HANDLE (WINAPI *tCreateToolhelp32Snapshot)	(DWORD dwFlags, DWORD th32ProcessID);

	HMODULE hKernel32Dll = LoadLibrary(_T("kernel32.dll"));
	if (!hKernel32Dll)
		return;
	tOpenThread		pOpenThread		= (tOpenThread)		GetProcAddress(hKernel32Dll, "OpenThread");
	tThread32First	pThread32First	= (tThread32First)	GetProcAddress(hKernel32Dll, "Thread32First");
	tThread32Next	pThread32Next	= (tThread32Next)	GetProcAddress(hKernel32Dll, "Thread32Next");
	tCreateToolhelp32Snapshot pCreateToolhelp32Snapshot	= (tCreateToolhelp32Snapshot)	GetProcAddress(hKernel32Dll, "CreateToolhelp32Snapshot");
	if (!pOpenThread	||
		!pThread32First	||
		!pThread32Next	||
		!pCreateToolhelp32Snapshot)
	{
		CloseHandle(hKernel32Dll);
		return;
	}

	HANDLE hSnapshot = pCreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

	// Get information about own process/thread
	DWORD ownProcessId = GetCurrentProcessId();
	DWORD ownThreadId = GetCurrentThreadId();

	// Enumerate threads
	THREADENTRY32 entry;
	entry.dwSize = sizeof(THREADENTRY32);
	BOOL bNext = pThread32First(hSnapshot, &entry);
	while (bNext)
	{
		if (entry.th32OwnerProcessID == ownProcessId &&
			entry.th32ThreadID != ownThreadId)
		{
			// Suspen threads of own process
			HANDLE hThread = pOpenThread(THREAD_SUSPEND_RESUME, FALSE, entry.th32ThreadID);
			if (hThread)
				SuspendThread(hThread);
		}
		bNext = pThread32Next(hSnapshot, &entry);
	}
	CloseHandle(hKernel32Dll);
}

void CExceptionReport::AddToReport(const char * pText)
{
	DWORD bytesWritten = 0;
	WriteFile(m_hReportFile, pText, strlen(pText), &bytesWritten, 0);
}

void CExceptionReport::AddToReport(const WCHAR * pText)
{
	USES_CONVERSION;
	AddToReport(W2A(pText));
}

void CExceptionReport::AddToReport(int number)
{
	char buffer[sizeof(int) * 4];
	sprintf(buffer, "%d", number);
	AddToReport(buffer);
}

void CExceptionReport::AddToReportHex(_int64 number, int minDigits /*=0*/)
{
	char buffer[sizeof(_int64) * 2 + 1];
	if (!minDigits)
        sprintf(buffer, "%I64X", number);
	else
	{
		char fmt[10];
		sprintf(fmt, "%%0%dI64X", minDigits);
		sprintf(buffer, fmt, number);
	}
	AddToReport(buffer);
}

BOOL CExceptionReport::GetAddrDetails(PVOID addr, PTSTR szModule, DWORD len, DWORD& section, DWORD& offset)
{
	// Get details about an address: Module name, section and offet
	
	// Get information about the provided address
	MEMORY_BASIC_INFORMATION mbi;
	if (!VirtualQuery(addr, &mbi, sizeof(mbi)))
		return FALSE;

	// Get module
	DWORD hMod = (DWORD)mbi.AllocationBase;
	if (!GetModuleFileName((HMODULE)hMod, szModule, len))
		return FALSE;

	
	// Get DOS header of module
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hMod;

	// Advance to PE header and get the section information
	PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)(hMod + pDosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNtHeader);

	// Get module load address
	DWORD lAddr = (DWORD)addr - hMod; 

	// Search for a section which contains the address
	for (unsigned int i = 0; i < pNtHeader->FileHeader.NumberOfSections; i++)
	{
		// Calculate section start and end addresses
		DWORD startAddr = pSection->VirtualAddress;
		DWORD endAddr = startAddr;
		if (pSection->SizeOfRawData > pSection->Misc.VirtualSize)
			endAddr += pSection->SizeOfRawData;
		else
			pSection->Misc.VirtualSize;

		// Look if provided address is between startAddr and endAddr
		if (lAddr >= startAddr && lAddr <= endAddr)
		{
			// We've found the section, set section index and offset
			section = i+1;
			offset = lAddr - startAddr;
			return TRUE;
		}
		pSection++;
	}

	// Section not found, very strange
	return FALSE;
}