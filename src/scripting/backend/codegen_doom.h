/*
** codegen_doom.h
**
** Compiler backend / code generation for ZScript and DECORATE
**
**---------------------------------------------------------------------------
**
** Copyright 2008-2016 Christoph Oelckers
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

#pragma once
#include "codegen.h"
#include "actor.h"

//==========================================================================
//
//	FxActionSpecialCall
//
//==========================================================================

class FxActionSpecialCall : public FxExpression
{
	int Special;
	FxExpression *Self;
	FArgumentList ArgList;

public:

	FxActionSpecialCall(FxExpression *self, int special, FArgumentList &args, const FScriptPosition &pos);
	~FxActionSpecialCall();
	FxExpression *Resolve(FCompileContext&);
	ExpEmit Emit(VMFunctionBuilder *build);
};

//==========================================================================
//
//	FxClassDefaults
//
//==========================================================================

class FxClassDefaults : public FxExpression
{
	FxExpression *obj;

public:
	FxClassDefaults(FxExpression *, const FScriptPosition &);
	~FxClassDefaults();
	FxExpression *Resolve(FCompileContext&);
	ExpEmit Emit(VMFunctionBuilder *build);
};

//==========================================================================
//
//	FxGetDefaultByType
//
//==========================================================================

class FxGetDefaultByType : public FxExpression
{
	FxExpression *Self;

public:

	FxGetDefaultByType(FxExpression *self);
	~FxGetDefaultByType();
	FxExpression *Resolve(FCompileContext&);
	ExpEmit Emit(VMFunctionBuilder *build);
};

//==========================================================================
//
// Only used to resolve the old jump by index feature of DECORATE
//
//==========================================================================

class FxStateByIndex : public FxExpression
{
	unsigned index;

public:

	FxStateByIndex(int i, const FScriptPosition &pos) : FxExpression(EFX_StateByIndex, pos)
	{
		index = i;
	}
	FxExpression *Resolve(FCompileContext&);
};

//==========================================================================
//
// Same as above except for expressions which means it will have to be
// evaluated at runtime
//
//==========================================================================

class FxRuntimeStateIndex : public FxExpression
{
	FxExpression *Index;
	int symlabel;

public:
	FxRuntimeStateIndex(FxExpression *index);
	~FxRuntimeStateIndex();
	FxExpression *Resolve(FCompileContext&);
	ExpEmit Emit(VMFunctionBuilder *build);
};

//==========================================================================
//
//
//
//==========================================================================

class FxMultiNameState : public FxExpression
{
	PClassActor *scope;
	TArray<FName> names;
public:

	FxMultiNameState(const char *statestring, const FScriptPosition &pos, PClassActor *checkclass = nullptr);
	FxExpression *Resolve(FCompileContext&);
};

//==========================================================================
//
//
//
//==========================================================================

class FxFStateOffset : public FxExpression
{
	FxExpression* State;
	FxExpression* Offset;
public:
	FxFStateOffset(FxExpression* state, FxExpression* offset, const FScriptPosition& pos);
	~FxFStateOffset();
	FxExpression* Resolve(FCompileContext&);
	ExpEmit Emit(VMFunctionBuilder* build);
};
