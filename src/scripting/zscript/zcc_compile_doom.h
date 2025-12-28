/*
** zcc_compile_doom.h
**
** contains the Doom specific parts of the script parser, i.e.
** actor property definitions and associated content.
**
**---------------------------------------------------------------------------
**
** Copyright 2016-2020 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025 UZDoom Maintainers and Contributors
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include "zcc_compile.h"

void SetImplicitArgs(TArray<PType*>* args, TArray<uint32_t>* argflags, TArray<FName>* argnames, PContainerType* cls, uint32_t funcflags, int useflags);

class ZCCDoomCompiler : public ZCCCompiler
{
public:
	ZCCDoomCompiler(ZCC_AST &tree, DObject *outer, PSymbolTable &symbols, PNamespace *outnamespace, int lumpnum, const VersionInfo & ver)
		: ZCCCompiler(tree, outer, symbols, outnamespace, lumpnum, ver)
		{}
	int Compile() override;
protected:
	bool PrepareMetaData(PClass *type) override;
	void SetImplicitArgs(TArray<PType*>* args, TArray<uint32_t>* argflags, TArray<FName>* argnames, PContainerType* cls, uint32_t funcflags, int useflags) override
	{
		::SetImplicitArgs(args, argflags, argnames, cls, funcflags, useflags);
	}
private:
	void CompileAllProperties();
	bool CompileProperties(PClass *type, TArray<ZCC_Property *> &Properties, FName prefix);
	bool CompileFlagDefs(PClass *type, TArray<ZCC_FlagDef *> &Properties, FName prefix);
	void DispatchProperty(FPropertyInfo *prop, ZCC_PropertyStmt *property, AActor *defaults, Baggage &bag);
	void DispatchScriptProperty(PProperty *prop, ZCC_PropertyStmt *property, AActor *defaults, Baggage &bag);
	void ProcessDefaultProperty(PClassActor *cls, ZCC_PropertyStmt *prop, Baggage &bag);
	void ProcessDefaultFlag(PClassActor *cls, ZCC_FlagStmt *flg);
	void InitDefaults() override final;
	void InitDefaultFunctionPointers();
	FxExpression *SetupActionFunction(PClass *cls, ZCC_TreeNode *af, int StateFlags);
	void CompileStates();
	int CheckActionKeyword(ZCC_FuncDeclarator *f, uint32_t &varflags, int useflags, ZCC_StructWork *c);
	
};


