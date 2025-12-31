/*
** stringtable.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
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
** FStringTable
**
** This class manages a list of localizable strings stored in a wad file.
*/

#pragma once

#include <stdlib.h>

#include "basics.h"
#include "fs_filesystem.h"
#include "name.h"
#include "tarray.h"
#include "zstring.h"

struct TableElement
{
	int filenum;
	FString strings[4];
};

// This public interface is for Dehacked
class StringMap : public TMap<FName, TableElement>
{
public:
	const char *MatchString(const char *string) const;
};


struct StringMacro
{
	FString Replacements[4];
};


class FStringTable
{
public:
	enum : uint32_t
	{
		default_table = MAKE_ID('*', '*', 0, 0),
		global_table = MAKE_ID('*', 0, 0, 0),
		override_table = MAKE_ID('*', '*', '*', 0)
	};

	using LangMap = TMap<uint32_t, StringMap>;
	using StringMacroMap = TMap<FName, StringMacro>;

	void LoadStrings(FileSys::FileSystem& fileSystem, const char *language);
	void UpdateLanguage(const char* language);
	StringMap GetDefaultStrings() { return allStrings[default_table]; }	// Dehacked needs these for comparison
	void SetOverrideStrings(StringMap & map)
	{
		allStrings.Insert(override_table, map);
		UpdateLanguage(nullptr);
	}

	const char *GetLanguageString(const char *name, uint32_t langtable, int gender = -1) const;
	bool MatchDefaultString(const char *name, const char *content) const;
	const char *CheckString(const char *name, uint32_t *langtable = nullptr, int gender = -1) const;
	const char* GetString(const char* name) const;
	const char* GetString(const FString& name) const { return GetString(name.GetChars()); }
	bool exists(const char *name);

	void InsertString(int filenum, int langid, FName label, const FString& string);
	void SetDefaultGender(int gender) { defaultgender = gender; }

private:

	FString activeLanguage;
	StringMacroMap allMacros;
	LangMap allStrings;
	TArray<std::pair<uint32_t, StringMap*>> currentLanguageSet;
	int defaultgender = 0;

	void LoadLanguage (int lumpnum, const char* buffer, size_t size);
	TArray<TArray<FString>> parseCSV(const char* buffer, size_t size);
	bool ParseLanguageCSV(int filenum, const char* buffer, size_t size);

	bool readMacros(const char* buffer, size_t size);
	void DeleteString(int langid, FName label);
	void DeleteForLabel(int filenum, FName label);

	static size_t ProcessEscapes (char *str);
public:
	static FString MakeMacro(const char *str)
	{
		if (*str == '$') return str;
		return FString("$") + str;
	}

	static FString MakeMacro(const char *str, size_t len)
	{
		if (*str == '$') return FString(str, len);
		return "$" + FString(str, len);
	}

	const char* localize(const char* str)
	{
		return *str == '$' ? GetString(str + 1) : str;
	}
};
