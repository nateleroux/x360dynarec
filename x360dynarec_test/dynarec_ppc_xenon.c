#include <xtl.h>
#include "dynarec.h"
#include "dynarec_ppc_xenon.h"

static BOOL emit(DynarecAssembleContext_t * Context, DWORD Opcode)
{
	return Dynarec_Emit(Context, &Opcode, 4);
}

BOOL Dynarec_Assembler_PPC_Xenon(DynarecOperation_t * Operation, DynarecAssembleContext_t * Context, void ** UserContext)
{
	PPCXenonAssemblerContext_t * ctx = (PPCXenonAssemblerContext_t*)(*UserContext);

	// Is this the first op?
	if(Operation == Operation->list)
	{
		// Create the function header data
		if(!emit(Context,  0x7D8802A6) || // mflr r12
			!emit(Context, 0x9181FFF8) || // stw r12, -8(r1)
			!emit(Context, 0x94210000))   // stwu r1, -X(r1)
			return FALSE;

		ctx->StackSize += 12; // 8? for r12, 4 for r1
	}

	if(Operation->OpType == DO_Noop)
		return emit(Context, 0x60000000); // nop
	else if(Operation->OpType == DO_BranchAddr)
	{
		if(Operation->Constant < ctx->FuncBase || Operation->Constant > ctx->FuncBase + ctx->FuncSize)
		{
			// Jump is outside of this function, jump to trampoline routine

			// Insert trampoline stub
			if(!emit(Context, 0x3D600000) ||          // lis r11, X
				!emit(Context, 0) ||          // ori/addi r11, r11, (stub)
				!emit(Context, 0x7D6903A6) || // mtctr r11
				!emit(Context, 0x4E800421))   // bctrl
				return FALSE;
		}
		else
			return emit(Context, 0x00000000); // stub
	}
	else if(Operation->OpType == DO_BranchExternal)
	{
		// Insert stub
		if(!emit(Context, 0x3D600000) ||          // lis r11, X
			!emit(Context, 0) ||          // ori/addi r11, r11, (stub)
			!emit(Context, 0x7D6903A6) || // mtctr r11
			!emit(Context, 0x4E800421))   // bctrl
			return FALSE;
	}
	else if(Operation->OpType == DO_Return)
	{
		// Function footer
		ctx->FuncFooter = Context->emitLength;
		if(!emit(Context, 0x38210000 | ctx->StackSize) ||   // addi r1, r1, X
			!emit(Context,  0x8181FFF8) || // lwz r12, -8(1)
			!emit(Context, 0x7D8803A6) ||  // mtlr r12
			!emit(Context, 0x4E800020))    // blr
			return FALSE;
	}
	else
		return FALSE;

	return TRUE;
}

BOOL Dynarec_Assembler_PPC_Xenon_LastPass(DynarecOperation_t * OpList, char * Buffer, void ** UserContext, DWORD BaseAddress)
{
	PPCXenonAssemblerContext_t * ctx = (PPCXenonAssemblerContext_t*)(*UserContext);

	// Fix up the stack header @ 0x8
	((DWORD*)Buffer)[2] |= (-(int)ctx->StackSize) & 0xFFFF;

	while(OpList)
	{
		DynarecOperation_t * op = OpList;
		OpList = OpList->link;

		if(op->OpType == DO_BranchExternal)
		{
			DWORD addrUpr, addrLwr;

			addrUpr = (op->Constant >> 16) & 0xFFFF;
			addrLwr = op->Constant & 0xFFFF;

			if(addrLwr & 0x8000)
			{
				((DWORD*)(Buffer + op->BufferPosition))[0] |= addrUpr + 1;
				((DWORD*)(Buffer + op->BufferPosition))[1] = 0x396B0000 | (-(int)(0x10000 - addrLwr) & 0xFFFF); // addi
			}
			else
			{
				((DWORD*)(Buffer + op->BufferPosition))[0] |= addrUpr;
				((DWORD*)(Buffer + op->BufferPosition))[1] = 0x616B0000 | addrLwr; // ori
			}
		}
		else if(op->OpType == DO_BranchAddr)
		{
			DWORD opcode = 0x48000000;
			DWORD addr = 0xFFFFFFFF;

			// Locate the op we are requesting a branch to
			// Local?
			if(op->Constant < ctx->FuncBase || op->Constant > ctx->FuncBase + ctx->FuncSize)
			{
				// Insert trampoline stub
			}
			else
			{
				// Calculate address
				DynarecOperation_t * branchOp = op->list;

				while(branchOp)
				{
					if(branchOp->OpLocation == op->Constant)
					{
						// Found the branch
						addr = branchOp->BufferPosition - op->BufferPosition;
						opcode |= addr & (0x3FFFFFFF);

						memcpy(Buffer + op->BufferPosition, &opcode, 4);
						break;
					}

					branchOp = branchOp->link;
				}
			}
		}
	}

	return TRUE;
}
