#include <xtl.h>
#include <stdio.h>
#include "codepages.h"

PVOID MmDbgWriteCheck( IN PVOID Address, OUT DWORD * PTE );
DWORD MmDbgReleaseAddress( IN PVOID Address, IN DWORD * PTE );

// The allocation bitmap for code pages, 1 bit per entry
static unsigned char * CodePageAllocationBitmap = NULL;

static HMODULE CodePageModule = NULL;
static unsigned char * CodePageBase = NULL;
static DWORD CodePageSize = 0;
CRITICAL_SECTION csCodePage;

VOID CodePage_Init()
{
	InitializeCriticalSection(&csCodePage);
}

BOOL CodePage_ReserveSpace(DWORD SizeInMB)
{
	BOOL bRet = FALSE;
	LPCSTR szFormat = "game:\\dynarec_support\\x360dynarec_codepages_%iMB.xex";
	CHAR sz[MAX_PATH];
	FARPROC fProc;

	CodePage_LockThread();

	if(CodePageBase != NULL)
	{
		LeaveCriticalSection(&csCodePage);
		return FALSE;
	}

	// Load the specified codepage image
	_snprintf(sz, sizeof(sz), szFormat, SizeInMB);

	CodePageModule = LoadLibrary(sz);
	
	if(CodePageModule == NULL)
		goto cleanup;

	// Get the page base
	CodePageBase = (unsigned char *)GetProcAddress(CodePageModule, CODEPAGE_IMPORT_SECTIONBASE); 

	// Get the page size
	fProc = GetProcAddress(CodePageModule, CODEPAGE_IMPORT_SECTIONSIZE);

	if(!CodePageBase || !fProc)
		goto cleanup;

	CodePageSize = *(DWORD*)fProc;

	if(CodePageSize == 0)
		goto cleanup;

	CodePageAllocationBitmap = (unsigned char *)malloc(CodePageSize / CODEPAGE_ALLOC_PAGE_SIZE / 8);
	memset(CodePageAllocationBitmap, 0, CodePageSize / CODEPAGE_ALLOC_PAGE_SIZE / 8);

	bRet = TRUE;

cleanup:
	if(!bRet)
		CodePage_FreeReservedSpace();

	CodePage_UnlockThread();

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

BOOL CodePage_IsPageAllocated(DWORD PageNumber)
{
	if(PageNumber > CodePageSize / CODEPAGE_ALLOC_PAGE_SIZE)
		return FALSE;

	return CodePageAllocationBitmap[PageNumber >> 3] >> (PageNumber % 8);
}

VOID CodePage_SetPageAllocated(DWORD PageNumber, BOOL Allocated)
{
	if(PageNumber > CodePageSize / CODEPAGE_ALLOC_PAGE_SIZE)
		return;

	CodePageAllocationBitmap[PageNumber >> 3] |= (Allocated << (PageNumber % 8));
}

DWORD CodePage_GetPageNumber(PVOID Ptr)
{
	DWORD dwPtr = (DWORD)Ptr - (DWORD)CodePageBase;
	
	if(dwPtr > CodePageSize)
		return 0xFFFFFFFF;

	return dwPtr / CODEPAGE_ALLOC_PAGE_SIZE;
}

PVOID CodePage_AllocatePage()
{
	UINT tag = 'dyna';
	PVOID pPageBuf = NULL;
	unsigned int i;

	CodePage_LockThread();

	// Search for a free page
	for(i = 0; i < CodePageSize / CODEPAGE_ALLOC_PAGE_SIZE; i++)
	{
		if(CodePage_IsPageAllocated(i))
			continue;

		CodePage_SetPageAllocated(i, TRUE);
		pPageBuf = (PVOID)((char *)CodePageBase + (i * CODEPAGE_ALLOC_PAGE_SIZE));
		break;
	}

	CodePage_UnlockThread();

	if(pPageBuf)
	{
		for(i = 0; i + 4 <= CODEPAGE_ALLOC_PAGE_SIZE; i += 4)
			CodePage_CopyData((char *)pPageBuf + i, &tag, 4);
	}

	return pPageBuf;
}

VOID CodePage_FreePage(PVOID Page)
{
	CodePage_SetPageAllocated(
		CodePage_GetPageNumber(Page), FALSE);
}

VOID CodePage_LockThread()
{
	EnterCriticalSection(&csCodePage);
}

VOID CodePage_UnlockThread()
{
	LeaveCriticalSection(&csCodePage);
}

VOID CodePage_CopyData(PVOID Destination, PVOID Source, UINT Count)
{
	unsigned int i, j;
	UINT startOffset;
	UINT copyAmt;
	DWORD pte;
	PVOID ptr;
	char * addr;

	startOffset = ((DWORD)Destination - (DWORD)CodePageBase) % CODEPAGE_ALLOC_PAGE_SIZE;

	// Copy page by page
	for(i = 0; i < Count; )
	{
		if(startOffset)
		{
			copyAmt = CODEPAGE_ALLOC_PAGE_SIZE - startOffset;
			i = startOffset;
			startOffset = 0;

			if(i >= Count)
				break;
		}
		else
			copyAmt = CODEPAGE_ALLOC_PAGE_SIZE;

		if(copyAmt > Count - i)
			copyAmt = Count - i;

		ptr = MmDbgWriteCheck((char *)Destination + i, &pte);
		if(ptr != NULL)
		{
			for(j = i; j < i + copyAmt; j++)
			{
				*((char *)Destination + j) = *((char *)Source + j);
				
				// Flush the cache
				addr = (char *)Destination + j;
				__asm dcbst r0, addr
				__asm icbi r0, addr
			}
		}

		MmDbgReleaseAddress(ptr, &pte);

		i += copyAmt;
	}
}
