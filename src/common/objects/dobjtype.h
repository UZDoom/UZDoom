/*
** dobjtype.h
**
** Implements the type information class
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2005-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
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

#ifndef DOBJTYPE_H
#define DOBJTYPE_H

#ifndef __DOBJECT_H__
#error You must #include "dobject.h" to get dobjtype.h
#endif

#include <limits.h>

typedef std::pair<const class PType *, unsigned> FTypeAndOffset;

#if 0
// This is intentionally not in vm.h so that this file remains free of DObject pollution.
class VMException : public DObject
{
	DECLARE_CLASS(VMException, DObject);
};
#endif

// An action function -------------------------------------------------------

class VMFrameStack;
struct VMValue;
struct VMReturn;
class VMFunction;
class PClassType;
struct FNamespaceManager;
class PSymbol;
class PField;
class PObjectPointer;
class PDynArray;
class PMap;

enum
{
	TentativeClass = UINT_MAX,
};


class PClass
{
protected:
	void Derive(PClass *newclass, FName name);
	void SetSuper();
public:
	void InitializeSpecials(void* addr, void* defaults, TArray<FTypeAndOffset> PClass::* Inits);
	void WriteAllFields(FSerializer &ar, const void *addr) const;
	bool ReadAllFields(FSerializer &ar, void *addr) const;
	int FindVirtualIndex(FName name, PFunction::Variant *variant, PFunction *parentfunc, bool exactReturnType, bool ignorePointerReadOnly);
	PSymbol *FindSymbol(FName symname, bool searchparents) const;
	PField *AddField(FName name, PType *type, uint32_t flags, int fileno = 0);
	void InitializeDefaults();

	static void StaticInit();
	static void StaticShutdown();

	// Per-class information -------------------------------------
	PClass				*ParentClass = nullptr;	// the class this class derives from
	const size_t * Pointers = nullptr;			// native object pointers defined by this class *only*

	mutable size_t FlatPointersSize = 0;
	mutable const std::pair<size_t, PObjectPointer*> * FlatPointers = nullptr;	// object pointers defined by this class and all its superclasses; not initialized by default.
	mutable size_t ArrayPointersSize = 0;
	mutable const std::pair<size_t, PDynArray *> * ArrayPointers = nullptr;	// dynamic arrays containing object pointers.
	mutable size_t MapPointersSize = 0;
	mutable const std::pair<size_t, PMap *>	* MapPointers = nullptr;	// maps containing object pointers.

	uint8_t				*Defaults = nullptr;
	uint8_t				*Meta = nullptr;			// Per-class static script data
	unsigned			 Size = sizeof(DObject);
	unsigned			 MetaSize = 0;
	FName				 TypeName = NAME_None;
	FName				 SourceLumpName = NAME_None;
	bool				 bRuntimeClass = false;	// class was defined at run-time, not compile-time
	bool				 bDecorateClass = false;	// may be subject to some idiosyncracies due to DECORATE backwards compatibility
	bool				 bAbstract = false;
	bool				 bSealed = false;
	bool				 bFinal = false;
	bool				 bOptional = false;
	TArray<VMFunction*>	 Virtuals;	// virtual function table
	TArray<FTypeAndOffset> MetaInits;
	TArray<FTypeAndOffset> SpecialInits;
	TArray<PField *> Fields;
	TArray<FName> SealedRestriction;
	PClassType			*VMType = nullptr;

	void (*ConstructNative)(void *);

	// The rest are all functions and static data ----------------
	PClass();
	~PClass();
	void InsertIntoHash(bool native);
	DObject *CreateNew();
	PClass *CreateDerivedClass(FName name, unsigned int size, bool *newlycreated = nullptr, int fileno = 0);

	void InitializeActorInfo();

	void BuildFlatPointers() const;
	void BuildArrayPointers() const;
	void BuildMapPointers() const;

	void DestroySpecials(void *addr);
	void DestroyMeta(void *addr);
	const PClass *NativeClass() const;

	// Returns true if this type is an ancestor of (or same as) the passed type.
	bool IsAncestorOf(const PClass *ti) const
	{
		while (ti)
		{
			if (this == ti)
				return true;
			ti = ti->ParentClass;
		}
		return false;
	}

	inline bool IsDescendantOf(const PClass *ti) const
	{
		return ti->IsAncestorOf(this);
	}

	inline bool IsDescendantOf(FName ti) const
	{
		auto me = this;
		while (me)
		{
			if (me->TypeName == ti)
				return true;
			me = me->ParentClass;
		}
		return false;
	}

	// Find a type, given its name.
	const PClass *FindParentClass(FName name) const;
	PClass *FindParentClass(FName name) { return const_cast<PClass *>(const_cast<const PClass *>(this)->FindParentClass(name)); }

	static PClass *FindClass(const char *name)			{ return FindClass(FName(name, true)); }
	static PClass *FindClass(const FString &name)		{ return FindClass(FName(name, true)); }
	static PClass *FindClass(ENamedName name)			{ return FindClass(FName(name)); }
	static PClass *FindClass(FName name);
	static PClassActor *FindActor(const char *name)		{ return FindActor(FName(name, true)); }
	static PClassActor *FindActor(const FString &name)	{ return FindActor(FName(name, true)); }
	static PClassActor *FindActor(ENamedName name)		{ return FindActor(FName(name)); }
	static PClassActor *FindActor(FName name);
	static VMFunction *FindFunction(FName cls, FName func);
	static void FindFunction(VMFunction **pptr, FName cls, FName func);
	PClass *FindClassTentative(FName name);

	static TMap<FName, PClass*> ClassMap;
	static TArray<PClass *> AllClasses;
	static TArray<VMFunction**> FunctionPtrList;

	static bool bShutdown;
	static bool bVMOperational;
};

#endif
