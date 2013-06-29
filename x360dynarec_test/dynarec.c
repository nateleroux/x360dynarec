#include <xtl.h>
#include "../x360dynarec/codepages.h"
#include "dynarec.h"


DynarecOperation_t * Dynarec_AllocateOperation()
{
	DynarecOperation_t * op = (DynarecOperation_t *)malloc(sizeof(DynarecOperation_t));
	
	if(op)
	{
		memset(op, 0, sizeof(DynarecOperation_t));
		op->list = op;
	}

	return op;
}

VOID Dynarec_FreeChain(DynarecOperation_t * AnyLink)
{
	DynarecOperation_t * op;

	AnyLink = AnyLink->list;

	while(AnyLink)
	{
		op = AnyLink->link;
		free(AnyLink);
		AnyLink = op;
	}
}

DynarecOperation_t * Dynarec_TagToList(DynarecOperation_t * List, DynarecOperation_t * Item)
{
	Item->link = List->link;
	List->link = Item;
	Item->list = List->list;

	return Item;
}

char * Dynarec_Assemble(DynarecOperation_t * List, UINT * BufLength)
{
	*BufLength = 0;


	return NULL;
}
