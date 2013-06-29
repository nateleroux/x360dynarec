#include <xtl.h>
#include <stdio.h>

#include "../x360dynarec/codepages.h"
#include "dynarec.h"
#include "dynarec_ppc_xenon.h"

void branch_routine()
{
	printf("I'm a branch routine!\n");
}

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
	
	DynarecOperation_t *op, *op2;
	op = Dynarec_AllocateOperation();
	op->OpType = DO_Noop;

	op2 = Dynarec_AllocateOperation();
	op2->OpType = DO_BranchAddr;
	op2->Constant = 0x01;

	op = Dynarec_TagToList(op, op2);

	op2 = Dynarec_AllocateOperation();
	op2->OpLocation = 0x00;
	op2->OpType = DO_Noop;

	op = Dynarec_TagToList(op, op2);

	op2 = Dynarec_AllocateOperation();
	op2->OpLocation = 0x01;
	op2->OpType = DO_BranchExternal;
	op2->Constant = (DWORD)branch_routine;

	op = Dynarec_TagToList(op, op2);

	op2 = Dynarec_AllocateOperation();
	op2->OpLocation = 0x01;
	op2->OpType = DO_Return;

	op = Dynarec_TagToList(op, op2);

	PPCXenonAssemblerContext_t * ctx = (PPCXenonAssemblerContext_t *)malloc(sizeof(PPCXenonAssemblerContext_t));
	memset(ctx, 0, sizeof(PPCXenonAssemblerContext_t));

	ctx->FuncSize = 5;

	unsigned int bufLen;
	char * buf = Dynarec_Assemble(op->list, &bufLen, Dynarec_Assembler_PPC_Xenon, (void**)&ctx);
	Dynarec_Assembler_PPC_Xenon_LastPass(op->list, buf, (void**)&ctx, (DWORD)page);

	free(ctx);

	Dynarec_FreeChain(op);

	if(buf)
	{
		// Copy our assembled code into executable space
		CodePage_CopyData(page, buf, bufLen);
		free(buf);
		
		((void(*)())page)();
	}

	CodePage_FreePage(page);
	
	CodePage_FreeReservedSpace();
}
