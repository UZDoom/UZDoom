/*
** vmexec.cpp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2009-2016 Marisa Heit
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
**
** SPDX-License-Identifier: BSD-3-Clause
**
**---------------------------------------------------------------------------
**
*/

#include <math.h>
#include <assert.h>
#include "v_video.h"
#include "s_soundinternal.h"
#include "basics.h"
//#include "r_state.h"
#include "stats.h"
#include "vmintern.h"
#include "types.h"
#include "basics.h"
#include "texturemanager.h"
#include "palutil.h"
#include "common/scripting/dap/GameEventEmit.h"

extern cycle_t VMCycles[10];
extern int VMCalls[10];

// THe sprite ID to string cast is game specific so let's do it with a callback to remove the dependency and allow easier reuse.
void (*VM_CastSpriteIDToString)(FString* a, unsigned int b) = [](FString* a, unsigned int b) { a->Format("%d", b); };

// intentionally implemented in a different source file to prevent inlining.
#if 0
void ThrowVMException(VMException *x);
#endif

#define IMPLEMENT_VMEXEC

#if !defined(COMPGOTO) && defined(__GNUC__)
#define COMPGOTO 1
#endif

#if COMPGOTO
#define OP(x)	x
#define NEXTOP	do { pc++; DebugServer::RuntimeEvents::EmitInstructionExecutionEvent(stack, ret, numret, pc); unsigned op = pc->op; a = pc->a; goto *ops[op]; } while(0)
#else
#define OP(x)	case OP_##x
#define NEXTOP	pc++; break
#endif

#define luai_nummod(a,b)        ((a) - floor((a)/(b))*(b))

#define A				(pc[0].a)
#define B				(pc[0].b)
#define C				(pc[0].c)
#define Cs				(pc[0].cs)
#define BC				(pc[0].i16u)
#define BCs				(pc[0].i16)
#define ABCs			(pc[0].i24)
#define JMPOFS(x)		((x)->i24)

#define KC				(konstd[C])
#define RC				(reg.d[C])

#define PA				(reg.a[A])
#define PB				(reg.a[B])

#define ASSERTD(x)		assert((unsigned)(x) < f->NumRegD)
#define ASSERTF(x)		assert((unsigned)(x) < f->NumRegF)
#define ASSERTA(x)		assert((unsigned)(x) < f->NumRegA)
#define ASSERTS(x)		assert((unsigned)(x) < f->NumRegS)

#define ASSERTKD(x)		assert(sfunc != NULL && (unsigned)(x) < sfunc->NumKonstD)
#define ASSERTKF(x)		assert(sfunc != NULL && (unsigned)(x) < sfunc->NumKonstF)
#define ASSERTKA(x)		assert(sfunc != NULL && (unsigned)(x) < sfunc->NumKonstA)
#define ASSERTKS(x)		assert(sfunc != NULL && (unsigned)(x) < sfunc->NumKonstS)

#define CMPJMP(test) \
	if ((test) == (a & CMP_CHECK)) { \
		assert(pc[1].op == OP_JMP); \
		pc += 1 + JMPOFS(pc+1); \
	} else { \
		pc += 1; \
	}

#define GETADDR(a,o,x) \
	if (a == NULL) { ThrowAbortException(x, nullptr); return 0; } \
	ptr = (VM_SBYTE *)a + o

#ifdef NDEBUG
#define WAS_NDEBUG 1
#else
#define WAS_NDEBUG 0
#endif

#if WAS_NDEBUG
#undef NDEBUG
#endif
#undef assert
#include <assert.h>
struct VMExec_Checked
{
#include "vmexec.h"
};
#if WAS_NDEBUG
#define NDEBUG
#endif

#if !WAS_NDEBUG
#define NDEBUG
#endif
#undef assert
#include <assert.h>
struct VMExec_Unchecked
{
#include "vmexec.h"
};
#if !WAS_NDEBUG
#undef NDEBUG
#endif
#undef assert
#include <assert.h>

int (*VMExec)(VMFunction *func, VMValue *params, int numparams, VMReturn *ret, int numret) =
#ifdef NDEBUG
VMExec_Unchecked::Exec
#else
VMExec_Checked::Exec
#endif
;

// Note: If the VM is being used in multiple threads, this should be declared as thread_local.
// ZDoom doesn't need this at the moment so this is disabled.

thread_local VMFrameStack GlobalVMStack;


//===========================================================================
//
// VMSelectEngine
//
// Selects the VM engine, either checked or unchecked. Default will decide
// based on the NDEBUG preprocessor definition.
//
//===========================================================================

void VMSelectEngine(EVMEngine engine)
{
	switch (engine)
	{
	case VMEngine_Default:
#ifdef NDEBUG
		VMExec = VMExec_Unchecked::Exec;
#else
#endif
		VMExec = VMExec_Checked::Exec;
		break;
	case VMEngine_Unchecked:
		VMExec = VMExec_Unchecked::Exec;
		break;
	case VMEngine_Checked:
		VMExec = VMExec_Checked::Exec;
		break;
	}
}

//===========================================================================
//
// VMFillParams
//
// Takes parameters from the parameter stack and stores them in the callee's
// registers.
//
//===========================================================================

void VMFillParams(VMValue *params, VMFrame *callee, int numparam)
{
	unsigned int regd, regf, regs, rega;
	VMScriptFunction *calleefunc = static_cast<VMScriptFunction *>(callee->Func);
	const VMRegisters calleereg(callee);

	assert(calleefunc != NULL && !(calleefunc->VarFlags & VARF_Native));
	assert(numparam == calleefunc->NumArgs);
	assert(REGT_INT == 0 && REGT_FLOAT == 1 && REGT_STRING == 2 && REGT_POINTER == 3);

	regd = regf = regs = rega = 0;
	const uint8_t *reginfo = calleefunc->RegTypes;
	assert(reginfo != nullptr);
	for (int i = 0; i < calleefunc->NumArgs; ++i, reginfo++)
	{
		// copy all parameters to the local registers.
		VMValue &p = params[i];
		if (*reginfo < REGT_STRING)
		{
			if (*reginfo == REGT_INT)
			{
				calleereg.d[regd++] = p.i;
			}
			else // p.Type == REGT_FLOAT
			{
				calleereg.f[regf++] = p.f;
			}
		}
		else if (*reginfo == REGT_STRING)
		{
			calleereg.s[regs++] = p.s();
		}
		else
		{
			assert(*reginfo == REGT_POINTER);
			calleereg.a[rega++] = p.a;
		}
	}
}


#ifndef NDEBUG
bool AssertObject(void * ob)
{
	auto obj = (DObject*)ob;
	if (obj == nullptr) return true;
#ifdef _MSC_VER
	__try
	{
		return obj->MagicID == DObject::MAGIC_ID;
	}
	__except (1)
	{
		return false;
	}
#else
	// No SEH on non-Microsoft compilers. :(
	return obj->MagicID == DObject::MAGIC_ID;
#endif
}
#endif
