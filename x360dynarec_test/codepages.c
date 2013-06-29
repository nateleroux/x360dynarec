#include <xtl.h>
#include <stdio.h>
#include "codepages.h"

PVOID MmDbgWriteCheck( IN PVOID Address, OUT DWORD * PTE );
DWORD MmDbgReleaseAddress( IN PVOID Address, IN DWORD PTE );

// The allocation bitmap for code pages, 1 bit per entry
static unsigned char * CodePageAllocationBitmap = NULL;

static HMODULE CodePageModule = NULL;
static unsigned char * CodePageBase = NULL;
static DWORD CodePageSize = 0;

BOOL CodePage_ReserveSpace(DWORD SizeInMB)
{
	BOOL bRet = FALSE;
	LPCSTR szFormat = "game:\\dynarec_support\\x360dynarec_codepages_%iMB.xex";
	CHAR sz[MAX_PATH];
	FARPROC fProc;

	// Load the specified codepage image
	_snprintf(sz, sizeof(sz), szFormat, SizeInMB);

	CodePageModule = LoadLibrary(sz);
	
	if(CodePageModule == NULL)
		goto cleanup;

	// Get the page base
	CodePageBase = (unsigned char *)GetProcAddress(CodePageModule, CODEPAGE_IMPORT_SECTIONBASE); 

	// Get the page size
	fProc = GetProcAddress(CodePageModule, CODEPAGE_IMPORT_SECTIONSIZE);

	if(!CodePageSize || !fProc)
		goto cleanup;

	CodePageSize = *(DWORD*)fProc;

	if(CodePageSize == 0)
		goto cleanup;

	bRet = TRUE;

cleanup:
	if(!bRet)
		CodePage_FreeReservedSpace();

	return bRet;
}

VOID CodePage_FreeReservedSpace()
{
	if(CodePageModule)
			FreeLibrary(CodePageModule);
		CodePageModule = NULL;
		CodePageBase = NULL;
		CodePageSize = 0;

		if(CodePageAllocationBitmap)
			free(CodePageAllocationBitmap);
		CodePageAllocationBitmap = NULL;
}
