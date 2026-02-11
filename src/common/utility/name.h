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

#include "absl/container/flat_hash_map.h"

enum ENamedName
{
#define xx(n) NAME_##n,
#define xy(n, s) NAME_##n,
#define xa(a, n)
#include "namedef.h"
#if __has_include("namedef_custom.h")
	#include "namedef_custom.h"
#endif
#undef xx
#undef xy
#undef xa

#define xx(n)
#define xy(n, s)
#define xa(a, n) NAME_##a = NAME_##n,
#include "namedef.h"
#if __has_include("namedef_custom.h")
    #include "namedef_custom.h"
#endif
#undef xx
#undef xy
#undef xa
};

class FString;

class FName
{
public:
	FName() = default;
	FName (const char *text) { Index = NameManager::Instance().FindName (text, false); }
	FName (const char *text, bool noCreate) { Index = NameManager::Instance().FindName (text, noCreate); }
	FName (const char *text, size_t textlen, bool noCreate) { Index = NameManager::Instance().FindName (text, textlen, noCreate); }
	FName(const FString& text) { Index = NameManager::Instance().FindName(text.GetChars(), text.Len(), false); }
	FName(const FString& text, bool noCreate) { Index = NameManager::Instance().FindName(text.GetChars(), text.Len(), noCreate); }
	FName (const FName &other) = default;
	FName (ENamedName index) { Index = index; }
 //   ~FName () {}	// Names can be added but never removed.

	int GetIndex() const { return Index; }
	const char *GetChars() const { return NameManager::Instance().NameToString[Index].c_str(); }

	FName &operator = (const char *text) { Index = NameManager::Instance().FindName (text, false); return *this; }
	FName& operator = (const FString& text) { Index = NameManager::Instance().FindName(text.GetChars(), text.Len(), false); return *this; }
	FName &operator = (const FName &other) = default;
	FName &operator = (ENamedName index) { Index = index; return *this; }

	int SetName (const char *text, bool noCreate=false) { return Index = NameManager::Instance().FindName (text, noCreate); }

	bool IsValidName() const { return (unsigned)Index < (unsigned)NameManager::Instance().NumNames; }

	static bool IsValidName(int index) { return index >= 0 && index < NameManager::Instance().NumNames; }

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

	struct NameManager
	{
		absl::flat_hash_map<std::string, int> StringToName;
		std::vector<std::string> NameToString;
		int NumNames = 0;

		static NameManager& Instance();
		NameManager(NameManager const&) = delete;
		void operator=(NameManager const&) = delete;

		int FindName (const char *text, bool noCreate);
		int FindName (const char *text, size_t textlen, bool noCreate);
		int FindName (const std::string_view str, bool noCreate);
	private:
		NameManager(std::initializer_list<const char*> predefinedNames);
	};
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
