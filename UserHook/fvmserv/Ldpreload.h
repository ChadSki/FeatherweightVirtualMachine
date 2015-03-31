
/*
 * Number of bytes of entry code that we will initially overwrite
 */
#define NUM_ENTRY_CODE_BYTES     7

/*
 * Number of bytes of code/data that is used to perform the LD_PRELOAD
 * functionality
 */
#define NUM_LDPRELOAD_CODE_BYTES (512 + NUM_ENTRY_CODE_BYTES)

/*
 * GetEntryPointAddr
 *
 * Gets the address of the EXE's entry point: the point at which the EXE
 * will begin executing.
 */
HRESULT GetEntryPointAddr(const char* szEXE, DWORD* pdwEntryAddr);
HRESULT GetEntryPointAddr1(const HANDLE hProcess, DWORD* pdwEntryAddr);

/*
 * WriteEntryCode
 *
 * Writes the new entry code to the BYTE pointer given.  This entry code
 * should be exactly NUM_ENTRY_CODE_BYTES long (an assert will fire
 * otherwise).  This code simply jumps to the address given as a parameter
 * to this function, which is where the instructions for loading the DLL
 * will exist within the process.
 */
HRESULT WriteEntryCode(BYTE* pbEntryCode, DWORD dwLDPreloadInstrAddr);


/*
 * WriteLDPreloadCode
 *
 * Writes the code which will call LoadLibrary and then restore the original
 * instructions back to the process entry point, then jumping back to the
 * entry point.
 */
HRESULT WriteLDPreloadCode(
	BYTE* pbLDPreloadCode,
	DWORD dwLDPreloadInstrAddr,
	const BYTE*	pbOrigEntryCode,
	DWORD dwProcessEntryCodeAddr,
	const char*	szDLL);
//////////////////////////////////////////////////////////////////////