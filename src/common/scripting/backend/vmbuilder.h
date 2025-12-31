/*
** vmbuilder.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2009-2016 Marisa Heit
** Copyright 2016-2017 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025 UZDoom Maintainers and Contributors
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <https://www.gnu.org/licenses/>.
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
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

#ifndef VMUTIL_H
#define VMUTIL_H

#include "dobject.h"
#include "m_argv.h"
#include "vmintern.h"
#include <vector>
#include <functional>


class FxCompoundStatement;
class VMFunctionBuilder;
class FxExpression;
class FxLocalVariableDeclaration;

struct ExpEmit
{
	ExpEmit() : RegNum(0), RegType(REGT_NIL), RegCount(1), Konst(false), Fixed(false), Final(false), Target(false) {}
	ExpEmit(int reg, int type, bool konst = false, bool fixed = false) : RegNum(reg), RegType(type), RegCount(1), Konst(konst), Fixed(fixed), Final(false), Target(false) {}
	ExpEmit(VMFunctionBuilder *build, int type, int count = 1);
	void Free(VMFunctionBuilder *build);
	void Reuse(VMFunctionBuilder *build);

	uint16_t RegNum;
	uint8_t RegType, RegCount;
	// We are at 8 bytes for this struct, no matter what, so it's rather pointless to squeeze these flags into bitfields.
	bool Konst, Fixed, Final, Target;
};

class VMFunctionBuilder
{
public:
	// Keeps track of which registers are available by way of a bitmask table.
	class RegAvailability
	{
	public:
		RegAvailability();
		int GetMostUsed() { return MostUsed; }
		int Get(int count);			// Returns the first register in the range
		void Return(int reg, int count);
		bool Reuse(int regnum);

		bool IsDirty(int reg) const
		{
			const int firstword = reg / 32;
			const int firstbit = reg & 31;
			return Dirty[firstword] & (1 << firstbit);
		}

	private:
		VM_UWORD Used[256/32];		// Bitmap of used registers (bit set means reg is used)
		VM_UWORD Dirty[256/32];
		int MostUsed;

		friend class VMFunctionBuilder;
	};

	using BlockMap = TArray<std::pair<std::pair<size_t, size_t>, TArray<VMLocalVariable>>>;

	VMFunctionBuilder(int numimplicits);
	~VMFunctionBuilder();

	void BeginStatement(FxExpression *stmt);
	void EndStatement();
	void MakeFunction(VMScriptFunction *func);

	// Returns the constant register holding the value.
	unsigned GetConstantInt(int val);
	unsigned GetConstantFloat(double val);
	unsigned GetConstantAddress(void *ptr);
	unsigned GetConstantString(FString str);

	int FindConstantInt(unsigned index);
	//double FindConstantFloat(unsigned index);
	//void * FindConstantAddress(unsigned index);
	//const FString& FindConstantString(unsigned index);

	unsigned AllocConstantsInt(unsigned int count, int *values);
	unsigned AllocConstantsFloat(unsigned int count, double *values);
	unsigned AllocConstantsAddress(unsigned int count, void **ptrs);
	unsigned AllocConstantsString(unsigned int count, FString *ptrs);


	// Returns the address of the next instruction to be emitted.
	size_t GetAddress();

	// Returns the address of the newly-emitted instruction.
	size_t Emit(int opcode, int opa, int opb, int opc);
	size_t Emit(int opcode, int opa, VM_SHALF opbc);
	size_t Emit(int opcode, int opabc);
	size_t EmitLoadInt(int regnum, int value);
	size_t EmitRetInt(int retnum, bool final, int value);

	void Backpatch(size_t addr, size_t target);
	void BackpatchToHere(size_t addr);
	void BackpatchList(TArray<size_t> &addrs, size_t target);
	void BackpatchListToHere(TArray<size_t> &addrs);

	// Write out complete constant tables.
	void FillIntConstants(int *konst);
	void FillFloatConstants(double *konst);
	void FillAddressConstants(FVoidObj *konst);
	void FillStringConstants(FString *strings);

	void AddBlock(const TArray<VMLocalVariable> &vars, size_t start, size_t end);

	// PARAM increases ActiveParam; CALL decreases it.
	void ParamChange(int delta);

	// Track available registers.
	RegAvailability Registers[4];

	// amount of implicit parameters so that proper code can be emitted for method calls
	int NumImplicits;

	// keep the frame pointer, if needed, in a register because the LFP opcode is hideously inefficient, requiring more than 20 instructions on x64.
	ExpEmit FramePointer;
	TArray<FxLocalVariableDeclaration *> ConstructedStructs;

private:
	TArray<FStatementInfo> LineNumbers;
	TArray<FxExpression *> StatementStack;

	TArray<int> IntConstantList;
	TArray<double> FloatConstantList;
	TArray<void *> AddressConstantList;
	TArray<FString> StringConstantList;
	// These map from the constant value to its position in the constant table.
	TMap<int, unsigned> IntConstantMap;
	TMap<double, unsigned> FloatConstantMap;
	TMap<void *, unsigned> AddressConstantMap;
	TMap<FString, unsigned> StringConstantMap;

	int MaxParam;
	int ActiveParam;

	TArray<VMOP> Code;
	BlockMap Blocks;
};

void DumpFunction(FILE *dump, VMScriptFunction *sfunc, const char *label, int labellen);


//==========================================================================
//
//
//
//==========================================================================
class FxExpression;

class FFunctionBuildList
{
	struct Item
	{
		PFunction *Func = nullptr;
		FxExpression *Code = nullptr;
		PPrototype *Proto = nullptr;
		VMScriptFunction *Function = nullptr;
		PNamespace *CurGlobals = nullptr;
		FString PrintableName;
		int StateIndex;
		int StateCount;
		int Lump;
		VersionInfo Version;
		bool FromDecorate;
	};

	TArray<Item> mItems;

	void DumpJit(bool include_gzdoom_pk3);

public:
	VMFunction *AddFunction(PNamespace *curglobals, const VersionInfo &ver, PFunction *func, FxExpression *code, const FString &name, bool fromdecorate, int currentstate, int statecnt, int lumpnum);
	void Build();
};

extern FFunctionBuildList FunctionBuildList;


//==========================================================================
//
// Function call parameter collector
//
//==========================================================================
extern int EncodeRegType(ExpEmit reg);

class FunctionCallEmitter
{
	// std::function and TArray are not compatible so this has to use std::vector instead.
	std::vector<std::function<int(VMFunctionBuilder *)>> emitters;
	TArray<std::pair<int, int>> returns;
	TArray<uint8_t> reginfo;
	unsigned numparams = 0;	// This counts the number of pushed elements, which can differ from the number of emitters with vectors.
	VMFunction *target = nullptr;
	class PFunctionPointer *fnptr = nullptr;
	int virtualselfreg = -1;
	bool is_vararg;
public:
	FunctionCallEmitter(VMFunction *func);
	FunctionCallEmitter(class PFunctionPointer *func);

	void SetVirtualReg(int virtreg)
	{
		virtualselfreg = virtreg;
	}

	void AddParameter(VMFunctionBuilder *build, FxExpression *operand);
	void AddParameter(ExpEmit &emit, bool reference);
	void AddParameterPointerConst(void *konst);
	void AddParameterPointer(int index, bool konst);
	void AddParameterFloatConst(double konst);
	void AddParameterIntConst(int konst);
	void AddParameterStringConst(const FString &konst);
	ExpEmit EmitCall(VMFunctionBuilder *build, TArray<ExpEmit> *ReturnRegs = nullptr);
	void AddReturn(int regtype, int regcount = 1)
	{
		returns.Push({ regtype, regcount });
	}
	unsigned Count() const
	{
		return numparams;
	}
};

class VMDisassemblyDumper
{
public:
	enum FileOperationType
	{
		Overwrite,
		Append
	};

	explicit VMDisassemblyDumper(const FileOperationType operation);
	~VMDisassemblyDumper();

	void Write(VMScriptFunction *sfunc, const FString &fname);
	void Flush();

private:
	FILE *dump = nullptr;
	FString namefilter;
	int codesize = 0;
	int datasize = 0;
};

#endif
