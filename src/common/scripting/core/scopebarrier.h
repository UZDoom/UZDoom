/*
** scopebarrier.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2017 ZZYZX
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

#pragma once

#include "zstring.h"

enum EScopeFlags
{
	Scope_All = 0,
	Scope_UI = 1,		// Marks a class that defaults to VARF_UI for its fields/methods
	Scope_Play = 2,	// Marks a class that defaults to VARF_Play for its fields/methods
};

class PClass;
class VMFunction;

//
// [ZZ] this really should be in codegen.h, but vmexec needs to access it
struct FScopeBarrier
{
	bool callable;
	bool readable;
	bool writable;

	// this is the error message
	FString callerror;
	FString readerror;
	FString writeerror;

	// this is used to make the error message.
	enum Side
	{
		Side_PlainData = 0,
		Side_UI = 1,
		Side_Play = 2,
		Side_Virtual = 3, // do NOT change the value
		Side_Clear = 4
	};
	int sidefrom;
	int sidelast;

	// Note: the same object can't be both UI and Play. This is checked explicitly in the field construction and will cause esoteric errors here if found.
	static int SideFromFlags(int flags);

	// same as above, but from object flags
	static int SideFromObjectFlags(EScopeFlags flags);

	//
	static int FlagsFromSide(int side);
	static EScopeFlags ObjectFlagsFromSide(int side);

	// used for errors
	static const char* StringFromSide(int side);

	// this modifies VARF_ flags and sets the side properly.
	static int ChangeSideInFlags(int flags, int side);
	// this modifies OF_ flags and sets the side properly.
	static EScopeFlags ChangeSideInObjectFlags(EScopeFlags flags, int side);
	FScopeBarrier();
	FScopeBarrier(int flags1, int flags2, const char* name);

	// AddFlags modifies ALLOWED actions by flags1->flags2.
	// This is used for comparing a.b.c.d access - if non-allowed field is seen anywhere in the chain, anything after it is non-allowed.
	// This struct is used so that the logic is in a single place.
	void AddFlags(int flags1, int flags2, const char* name);


	static bool CheckSidesForFunctionPointer(int from, int to);

	// this is called from vmexec.h
	static void ValidateNew(PClass* cls, int scope);
	static void ValidateCall(PClass* selftype, VMFunction *calledfunc, int outerside);

};

