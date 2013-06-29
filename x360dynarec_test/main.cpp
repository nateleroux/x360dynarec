#include <xtl.h>
#include <stdio.h>

#include "codepages.h"

void * begin_write(void * address, unsigned int * out_pte)
{
	return 0; // return MmDbgWriteCheck(address, (DWORD*)out_pte);
}

void end_write(void * address, unsigned int pte)
{
	// MmDbgReleaseAddress(address, pte);
}

void main()
{
	HMODULE codepages = LoadLibrary("game:\\dynarec_support\\x360dynarec_codepages_8MB.xex");
	if(codepages == NULL)
		return;

	FARPROC proc = GetProcAddress(codepages, (LPCSTR)1);
	proc = GetProcAddress(codepages, (LPCSTR)2);
	proc = GetProcAddress(codepages, (LPCSTR)1);
	if(proc == NULL)
	{
		FreeLibrary(codepages);
		return;
	}

	// Does it execute?
	DWORD result = ((DWORD(*)())proc)();

	printf("Result: %08x\n", result);

	DWORD * ptr;
	unsigned int pte;
	
	ptr = (DWORD*)begin_write(proc, &pte);
	if(ptr)
	{
		*ptr = 0x38600001;
		end_write(proc, pte);
	}

	// Does it execute?
	result = ((DWORD(*)())proc)();

	printf("Result: %08x\n", result);

	printf("ptr = %08x\n", *ptr);
}