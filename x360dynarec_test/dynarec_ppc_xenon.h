#ifndef _dynarec_ppc_xenon_h
#define _dynarec_ppc_xenon_h

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PPCXenonAssemblerContext_s
{
	unsigned int FuncBase;
	unsigned int FuncSize;
	unsigned int StackSize;
	unsigned int FuncFooter;
} PPCXenonAssemblerContext_t;

BOOL Dynarec_Assembler_PPC_Xenon(DynarecOperation_t * Operation, DynarecAssembleContext_t * Context, void ** UserContext);
BOOL Dynarec_Assembler_PPC_Xenon_LastPass(DynarecOperation_t * OpList, char * Buffer, void ** UserContext, DWORD BaseAddress);

#ifdef __cplusplus
}
#endif

#endif // _dynarec_ppc_xenon_h