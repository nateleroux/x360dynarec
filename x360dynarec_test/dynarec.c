#include <xtl.h>
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
		//if(AnyLink->tag)
			//free(AnyLink->tag);
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

char * Dynarec_Assemble(DynarecOperation_t * List, UINT * BufLength, Dynarec_Assembler Assembler, void ** UserContext)
{
	DynarecAssembleContext_t context;

	*BufLength = 0;
	
	context.buf = NULL;
	context.emitLength = 0;
	context.bufLength = 0;

	while(List)
	{
		List->BufferPosition = context.emitLength;

		if(!Assembler(List, &context, UserContext))
		{
			if(context.buf)
				free(context.buf);

			return NULL;
		}

		List = List->link;
	}

	*BufLength = context.emitLength;
	return context.buf;
}

BOOL Dynarec_Emit(DynarecAssembleContext_t * Context, PVOID Data, UINT Length)
{
	if(Context->buf == NULL || Context->bufLength == 0)
	{
		Context->buf = (char *)malloc(4);
		Context->bufLength = 4;
	}

	while(Context->bufLength < Context->emitLength + Length)
	{
		char * tmp = Context->buf;
		Context->buf = (char *)malloc(Context->bufLength * 2);

		if(tmp != NULL && Context->buf != NULL)
			memcpy(Context->buf, tmp, Context->emitLength);

		if(tmp != NULL)
			free(tmp);

		Context->bufLength = Context->bufLength * 2;
	}

	if(Context->buf == NULL)
		return FALSE;

	memcpy(Context->buf + Context->emitLength, Data, Length);
	Context->emitLength += Length;

	return TRUE;
}