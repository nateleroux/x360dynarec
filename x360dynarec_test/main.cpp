#include <xtl.h>
#include <stdio.h>

#include "../x360dynarec/codepages.h"

void main()
{
	CodePage_Init();

	// Reserve 8MB
	if(!CodePage_ReserveSpace(CODEPAGE_8MB))
	{
		printf("Failed to reserve space!\n");
		return;
	}

	PVOID page = CodePage_AllocatePage();
	if(page == NULL)
	{
		printf("Failed to allocate page\n");
		CodePage_FreeReservedSpace();
		return;
	}

	DWORD opcodes[] =
	{
		0x38600001, // li r3, 1
		0x4E800020, // blr
	};

	CodePage_CopyData(page, opcodes, 8);

	printf("Function execution result: %i\n",
		((int(*)())page)()
		);

	CodePage_FreePage(page);
	
	CodePage_FreeReservedSpace();
}