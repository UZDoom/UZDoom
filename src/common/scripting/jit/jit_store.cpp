/*
** jit_store.cpp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2018 Magnus Norddahl
** Copyright 2018-2025 GZDoom Maintainers and Contributors
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

#include "jitintern.h"

void JitCompiler::EmitSB()
{
	EmitNullPointerThrow(A, X_WRITE_NIL);
	cc.mov(asmjit::x86::byte_ptr(regA[A], konstd[C]), regD[B].r8Lo());
}

void JitCompiler::EmitSB_R()
{
	EmitNullPointerThrow(A, X_WRITE_NIL);
	cc.mov(asmjit::x86::byte_ptr(regA[A], regD[C]), regD[B].r8Lo());
}

void JitCompiler::EmitSH()
{
	EmitNullPointerThrow(A, X_WRITE_NIL);
	cc.mov(asmjit::x86::word_ptr(regA[A], konstd[C]), regD[B].r16());
}

void JitCompiler::EmitSH_R()
{
	EmitNullPointerThrow(A, X_WRITE_NIL);
	cc.mov(asmjit::x86::word_ptr(regA[A], regD[C]), regD[B].r16());
}

void JitCompiler::EmitSW()
{
	EmitNullPointerThrow(A, X_WRITE_NIL);
	cc.mov(asmjit::x86::dword_ptr(regA[A], konstd[C]), regD[B]);
}

void JitCompiler::EmitSW_R()
{
	EmitNullPointerThrow(A, X_WRITE_NIL);
	cc.mov(asmjit::x86::dword_ptr(regA[A], regD[C]), regD[B]);
}

void JitCompiler::EmitSSP()
{
	EmitNullPointerThrow(A, X_WRITE_NIL);
	auto tmp = newTempXmmSd();
	cc.xorpd(tmp, tmp);
	cc.cvtsd2ss(tmp, regF[B]);
	cc.movss(asmjit::x86::dword_ptr(regA[A], konstd[C]), tmp);
}

void JitCompiler::EmitSSP_R()
{
	EmitNullPointerThrow(A, X_WRITE_NIL);
	auto tmp = newTempXmmSd();
	cc.xorpd(tmp, tmp);
	cc.cvtsd2ss(tmp, regF[B]);
	cc.movss(asmjit::x86::dword_ptr(regA[A], regD[C]), tmp);
}

void JitCompiler::EmitSDP()
{
	EmitNullPointerThrow(A, X_WRITE_NIL);
	cc.movsd(asmjit::x86::qword_ptr(regA[A], konstd[C]), regF[B]);
}

void JitCompiler::EmitSDP_R()
{
	EmitNullPointerThrow(A, X_WRITE_NIL);
	cc.movsd(asmjit::x86::qword_ptr(regA[A], regD[C]), regF[B]);
}

void JitCompiler::EmitSS()
{
	EmitNullPointerThrow(A, X_WRITE_NIL);
	auto ptr = newTempIntPtr();
	cc.lea(ptr, asmjit::x86::ptr(regA[A], konstd[C]));
	auto call = CreateCall<void, FString*, FString*>(&JitCompiler::CallAssignString);
	call->setArg(0, ptr);
	call->setArg(1, regS[B]);
}

void JitCompiler::EmitSS_R()
{
	EmitNullPointerThrow(A, X_WRITE_NIL);
	auto ptr = newTempIntPtr();
	cc.lea(ptr, asmjit::x86::ptr(regA[A], regD[C]));
	auto call = CreateCall<void, FString*, FString*>(&JitCompiler::CallAssignString);
	call->setArg(0, ptr);
	call->setArg(1, regS[B]);
}

void JitCompiler::EmitSO()
{
	EmitNullPointerThrow(A, X_WRITE_NIL);
	cc.mov(asmjit::x86::ptr(regA[A], konstd[C]), regA[B]);

	typedef void(*FuncPtr)(DObject*);
	auto call = CreateCall<void, DObject*>(static_cast<FuncPtr>(GC::WriteBarrier));
	call->setArg(0, regA[B]);
}

void JitCompiler::EmitSO_R()
{
	EmitNullPointerThrow(A, X_WRITE_NIL);
	cc.mov(asmjit::x86::ptr(regA[A], regD[C]), regA[B]);

	typedef void(*FuncPtr)(DObject*);
	auto call = CreateCall<void, DObject*>(static_cast<FuncPtr>(GC::WriteBarrier));
	call->setArg(0, regA[B]);
}

void JitCompiler::EmitSP()
{
	EmitNullPointerThrow(A, X_WRITE_NIL);
	cc.mov(asmjit::x86::ptr(regA[A], konstd[C]), regA[B]);
}

void JitCompiler::EmitSP_R()
{
	EmitNullPointerThrow(A, X_WRITE_NIL);
	cc.mov(asmjit::x86::ptr(regA[A], regD[C]), regA[B]);
}

void JitCompiler::EmitSV2()
{
	EmitNullPointerThrow(A, X_WRITE_NIL);
	auto tmp = newTempIntPtr();
	cc.mov(tmp, regA[A]);
	cc.add(tmp, konstd[C]);
	cc.movsd(asmjit::x86::qword_ptr(tmp), regF[B]);
	cc.movsd(asmjit::x86::qword_ptr(tmp, 8), regF[B + 1]);
}

void JitCompiler::EmitSV2_R()
{
	EmitNullPointerThrow(A, X_WRITE_NIL);
	auto tmp = newTempIntPtr();
	cc.mov(tmp, regA[A]);
	cc.add(tmp, regD[C]);
	cc.movsd(asmjit::x86::qword_ptr(tmp), regF[B]);
	cc.movsd(asmjit::x86::qword_ptr(tmp, 8), regF[B + 1]);
}

void JitCompiler::EmitSV3()
{
	EmitNullPointerThrow(A, X_WRITE_NIL);
	auto tmp = newTempIntPtr();
	cc.mov(tmp, regA[A]);
	cc.add(tmp, konstd[C]);
	cc.movsd(asmjit::x86::qword_ptr(tmp), regF[B]);
	cc.movsd(asmjit::x86::qword_ptr(tmp, 8), regF[B + 1]);
	cc.movsd(asmjit::x86::qword_ptr(tmp, 16), regF[B + 2]);
}

void JitCompiler::EmitSV3_R()
{
	EmitNullPointerThrow(A, X_WRITE_NIL);
	auto tmp = newTempIntPtr();
	cc.mov(tmp, regA[A]);
	cc.add(tmp, regD[C]);
	cc.movsd(asmjit::x86::qword_ptr(tmp), regF[B]);
	cc.movsd(asmjit::x86::qword_ptr(tmp, 8), regF[B + 1]);
	cc.movsd(asmjit::x86::qword_ptr(tmp, 16), regF[B + 2]);
}

void JitCompiler::EmitSV4()
{
	EmitNullPointerThrow(A, X_WRITE_NIL);
	auto tmp = newTempIntPtr();
	cc.mov(tmp, regA[A]);
	cc.add(tmp, konstd[C]);
	cc.movsd(asmjit::x86::qword_ptr(tmp), regF[B]);
	cc.movsd(asmjit::x86::qword_ptr(tmp, 8), regF[B + 1]);
	cc.movsd(asmjit::x86::qword_ptr(tmp, 16), regF[B + 2]);
	cc.movsd(asmjit::x86::qword_ptr(tmp, 24), regF[B + 3]);
}

void JitCompiler::EmitSV4_R()
{
	EmitNullPointerThrow(A, X_WRITE_NIL);
	auto tmp = newTempIntPtr();
	cc.mov(tmp, regA[A]);
	cc.add(tmp, regD[C]);
	cc.movsd(asmjit::x86::qword_ptr(tmp), regF[B]);
	cc.movsd(asmjit::x86::qword_ptr(tmp, 8), regF[B + 1]);
	cc.movsd(asmjit::x86::qword_ptr(tmp, 16), regF[B + 2]);
	cc.movsd(asmjit::x86::qword_ptr(tmp, 24), regF[B + 3]);
}

void JitCompiler::EmitSFV2()
{
	EmitNullPointerThrow(A, X_WRITE_NIL);
	auto tmp = newTempIntPtr();
	cc.mov(tmp, regA[A]);
	cc.add(tmp, konstd[C]);

	auto tmpF = newTempXmmSs();
	cc.cvtsd2ss(tmpF, regF[B]);
	cc.movss(asmjit::x86::qword_ptr(tmp), tmpF);
	cc.cvtsd2ss(tmpF, regF[B + 1]);
	cc.movss(asmjit::x86::qword_ptr(tmp, 4), tmpF);
}

void JitCompiler::EmitSFV2_R()
{
	EmitNullPointerThrow(A, X_WRITE_NIL);
	auto tmp = newTempIntPtr();
	cc.mov(tmp, regA[A]);
	cc.add(tmp, regD[C]);

	auto tmpF = newTempXmmSs();
	cc.cvtsd2ss(tmpF, regF[B]);
	cc.movss(asmjit::x86::qword_ptr(tmp), tmpF);
	cc.cvtsd2ss(tmpF, regF[B + 1]);
	cc.movss(asmjit::x86::qword_ptr(tmp, 4), tmpF);
}

void JitCompiler::EmitSFV3()
{
	EmitNullPointerThrow(A, X_WRITE_NIL);
	auto tmp = newTempIntPtr();
	cc.mov(tmp, regA[A]);
	cc.add(tmp, konstd[C]);
	auto tmpF = newTempXmmSs();
	cc.cvtsd2ss(tmpF, regF[B]);
	cc.movss(asmjit::x86::qword_ptr(tmp), tmpF);
	cc.cvtsd2ss(tmpF, regF[B + 1]);
	cc.movss(asmjit::x86::qword_ptr(tmp, 4), tmpF);
	cc.cvtsd2ss(tmpF, regF[B + 2]);
	cc.movss(asmjit::x86::qword_ptr(tmp, 8), tmpF);
}

void JitCompiler::EmitSFV3_R()
{
	EmitNullPointerThrow(A, X_WRITE_NIL);
	auto tmp = newTempIntPtr();
	cc.mov(tmp, regA[A]);
	cc.add(tmp, regD[C]);
	auto tmpF = newTempXmmSs();
	cc.cvtsd2ss(tmpF, regF[B]);
	cc.movss(asmjit::x86::qword_ptr(tmp), tmpF);
	cc.cvtsd2ss(tmpF, regF[B + 1]);
	cc.movss(asmjit::x86::qword_ptr(tmp, 4), tmpF);
	cc.cvtsd2ss(tmpF, regF[B + 2]);
	cc.movss(asmjit::x86::qword_ptr(tmp, 8), tmpF);
}

void JitCompiler::EmitSFV4()
{
	EmitNullPointerThrow(A, X_WRITE_NIL);
	auto tmp = newTempIntPtr();
	cc.mov(tmp, regA[A]);
	cc.add(tmp, konstd[C]);
	auto tmpF = newTempXmmSs();
	cc.cvtsd2ss(tmpF, regF[B]);
	cc.movss(asmjit::x86::qword_ptr(tmp), tmpF);
	cc.cvtsd2ss(tmpF, regF[B + 1]);
	cc.movss(asmjit::x86::qword_ptr(tmp, 4), tmpF);
	cc.cvtsd2ss(tmpF, regF[B + 2]);
	cc.movss(asmjit::x86::qword_ptr(tmp, 8), tmpF);
	cc.cvtsd2ss(tmpF, regF[B + 3]);
	cc.movss(asmjit::x86::qword_ptr(tmp, 12), tmpF);
}

void JitCompiler::EmitSFV4_R()
{
	EmitNullPointerThrow(A, X_WRITE_NIL);
	auto tmp = newTempIntPtr();
	cc.mov(tmp, regA[A]);
	cc.add(tmp, regD[C]);
	auto tmpF = newTempXmmSs();
	cc.cvtsd2ss(tmpF, regF[B]);
	cc.movss(asmjit::x86::qword_ptr(tmp), tmpF);
	cc.cvtsd2ss(tmpF, regF[B + 1]);
	cc.movss(asmjit::x86::qword_ptr(tmp, 4), tmpF);
	cc.cvtsd2ss(tmpF, regF[B + 2]);
	cc.movss(asmjit::x86::qword_ptr(tmp, 8), tmpF);
	cc.cvtsd2ss(tmpF, regF[B + 3]);
	cc.movss(asmjit::x86::qword_ptr(tmp, 12), tmpF);
}

void JitCompiler::EmitSBIT()
{
	EmitNullPointerThrow(A, X_WRITE_NIL);
	auto tmp1 = newTempInt32();
	auto tmp2 = newTempInt32();
	cc.mov(tmp1, asmjit::x86::byte_ptr(regA[A]));
	cc.mov(tmp2, tmp1);
	cc.or_(tmp1, (int)C);
	cc.and_(tmp2, ~(int)C);
	cc.test(regD[B], regD[B]);
	cc.cmove(tmp1, tmp2);
	cc.mov(asmjit::x86::byte_ptr(regA[A]), tmp1);
}
