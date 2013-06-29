#include <xtl.h>
#include <stdio.h>

#include "../x360dynarec/codepages.h"
#include "dynarec.h"

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
	/*
	DWORD opcodes[] =
	{
		0x38600001, // li r3, 1
		0x4E800020, // blr
	};

	CodePage_CopyData(page, opcodes, 8);

	printf("Function execution result: %i\n",
		((int(*)())page)()
		);
	*/

	DynarecOperation_t *op, *op2;
	op = Dynarec_AllocateOperation();
	op->OpType = DO_Noop;

	op2 = Dynarec_AllocateOperation();
	op2->OpType = DO_Test;

	op = Dynarec_TagToList(op, op2);

	op2 = Dynarec_AllocateOperation();
	op2->OpType = DO_Noop;

	op = Dynarec_TagToList(op, op2);

	op2 = Dynarec_AllocateOperation();
	op2->OpType = DO_Return;

	op = Dynarec_TagToList(op, op2);

	unsigned int bufLen;
	char * buf = Dynarec_Assemble(op->list, &bufLen);

	Dynarec_FreeChain(op);

	if(buf)
	{
		CodePage_CopyData(page, buf, bufLen);
		((void(*)())page)();

		free(buf);
	}

	CodePage_FreePage(page);
	
	CodePage_FreeReservedSpace();
}
