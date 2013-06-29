#ifndef _dynarec_h
#define _dynarec_h

#ifdef __cplusplus
extern "C" {
#endif

enum DynarecOperation_e
{
	DO_Noop = 0,
	DO_Test,
	DO_Return,

	DO_END
};

enum DynarecRegister_e
{
	DR_Zero = 0, // This register is always zero
	DR_General = 1, // General purpose

	DR_Special = 1000, // Special use registers
};

typedef struct DynarecOperation_s
{
	unsigned int OpType;
	unsigned int Reg1, Reg2, Reg3, RegDest;
	unsigned int Constant;

	struct DynarecOperation_s * link;
	struct DynarecOperation_s * list;
} DynarecOperation_t;

DynarecOperation_t * Dynarec_AllocateOperation();
VOID Dynarec_FreeChain(DynarecOperation_t * AnyLink);

DynarecOperation_t * Dynarec_TagToList(DynarecOperation_t * List, DynarecOperation_t * Item);
char * Dynarec_Assemble(DynarecOperation_t * List, UINT * BufLength);

#ifdef __cplusplus
}
#endif

#endif // _dynarec_h