/*
** name.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2005-2016 Marisa Heit
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

#ifndef NAME_H
#define NAME_H

#include "tarray.h"
#include "zstring.h"

enum ENamedName
{
#define xx(n) NAME_##n,
#define xy(n, s) NAME_##n,
#include "namedef.h"
#if __has_include("namedef_custom.h")
	#include "namedef_custom.h"
#endif
#undef xx
#undef xy
};

class FString;

class FName
{
public:
	FName() = default;
	FName (const char *text) { Index = NameData.FindName (text, false); }
	FName (const char *text, bool noCreate) { Index = NameData.FindName (text, noCreate); }
	FName (const char *text, size_t textlen, bool noCreate) { Index = NameData.FindName (text, textlen, noCreate); }
	FName(const FString& text) { Index = NameData.FindName(text.GetChars(), text.Len(), false); }
	FName(const FString& text, bool noCreate) { Index = NameData.FindName(text.GetChars(), text.Len(), noCreate); }
	FName (const FName &other) = default;
	FName (ENamedName index) { Index = index; }
 //   ~FName () {}	// Names can be added but never removed.

	int GetIndex() const { return Index; }
	const char *GetChars() const { return NameData.NameArray[Index].Text; }

	FName &operator = (const char *text) { Index = NameData.FindName (text, false); return *this; }
	FName& operator = (const FString& text) { Index = NameData.FindName(text.GetChars(), text.Len(), false); return *this; }
	FName &operator = (const FName &other) = default;
	FName &operator = (ENamedName index) { Index = index; return *this; }

	int SetName (const char *text, bool noCreate=false) { return Index = NameData.FindName (text, noCreate); }

	bool IsValidName() const { return (unsigned)Index < (unsigned)NameData.NumNames; }

	static bool IsValidName(int index) { return index >= 0 && index < NameData.NumNames; }

	// Note that the comparison operators compare the names' indices, not
	// their text, so they cannot be used to do a lexicographical sort.
	bool operator == (const FName &other) const { return Index == other.Index; }
	bool operator != (const FName &other) const { return Index != other.Index; }
	bool operator <  (const FName &other) const { return Index <  other.Index; }
	bool operator <= (const FName &other) const { return Index <= other.Index; }
	bool operator >  (const FName &other) const { return Index >  other.Index; }
	bool operator >= (const FName &other) const { return Index >= other.Index; }

	bool operator == (ENamedName index) const { return Index == index; }
	bool operator != (ENamedName index) const { return Index != index; }
	bool operator <  (ENamedName index) const { return Index <  index; }
	bool operator <= (ENamedName index) const { return Index <= index; }
	bool operator >  (ENamedName index) const { return Index >  index; }
	bool operator >= (ENamedName index) const { return Index >= index; }

protected:
	int Index;

	struct NameEntry
	{
		char *Text;
		unsigned int Hash;
		int NextHash;
	};

	struct NameManager
	{
		// No constructor because we can't ensure that it actually gets
		// called before any FNames are constructed during startup. This
		// means this struct must only exist in the program's BSS section.
		~NameManager();

		enum { HASH_SIZE = 1024 };
		struct NameBlock;

		NameBlock *Blocks;
		NameEntry *NameArray;
		int NumNames, MaxNames;
		int Buckets[HASH_SIZE];

		int FindName (const char *text, bool noCreate);
		int FindName (const char *text, size_t textlen, bool noCreate);
		int AddName (const char *text, unsigned int hash, unsigned int bucket);
		NameBlock *AddBlock (size_t len);
		void InitBuckets ();
		static bool Inited;
	};

	static NameManager NameData;
};


template<> struct THashTraits<FName>
{
	hash_t Hash(FName key)
	{
		return key.GetIndex();
	}
	int Compare(FName left, FName right) { return left != right; }
}; 
#endif
