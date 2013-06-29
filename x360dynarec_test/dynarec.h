#ifndef _dynarec_h
#define _dynarec_h

#ifdef __cplusplus
extern "C" {
#endif

enum DynarecOperation_e
{
	DO_Noop = 0, // No operation
	DO_BranchAddr, // Branch to Constant in machine code
	DO_BranchExternal, // Branch to external routine
	DO_Return, // Return from this routine

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
	// Location in machine code
	unsigned int OpLocation;

	// DynarecOperation_e
	unsigned int OpType;

	// Operation registers
	unsigned int Reg1, Reg2, Reg3, RegDest;

	// Operation constant value
	unsigned int Constant;

	// The address of this opcode in the buffer
	unsigned int BufferPosition;

	// User defined tag, use with 'malloc'
	void * tag;

	// User defined 32 bits of whatever
	unsigned int utag;

	// List infos
	struct DynarecOperation_s * link;
	struct DynarecOperation_s * list;
} DynarecOperation_t;

typedef struct DynarecAssembleContext_s
{
	char * buf;
	unsigned int emitLength;
	unsigned int bufLength;
} DynarecAssembleContext_t;

DynarecOperation_t * Dynarec_AllocateOperation();
VOID Dynarec_FreeChain(DynarecOperation_t * AnyLink);

DynarecOperation_t * Dynarec_TagToList(DynarecOperation_t * List, DynarecOperation_t * Item);

typedef BOOL (*Dynarec_Assembler)(struct DynarecOperation_s * Operation, struct DynarecAssembleContext_s * Context, void ** UserContext);
char * Dynarec_Assemble(DynarecOperation_t * List, UINT * BufLength, Dynarec_Assembler Assembler, void ** UserContext);

BOOL Dynarec_Emit(DynarecAssembleContext_t * Context, PVOID Data, UINT Length);

#ifdef __cplusplus
}
#endif

#endif // _dynarec_h