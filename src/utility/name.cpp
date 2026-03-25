/*
** name.cpp
**
** Implements int-as-string mapping.
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

#include <cstdint>
#include <string.h>
#include <string_view>
#include <vector>

#include "name.h"
#include "m_crc32.h"

#include "absl/strings/ascii.h"

// CODE --------------------------------------------------------------------

static constexpr size_t FindDuplicates()
{
	auto tolower = [](uint8_t c) -> uint8_t {
		if (c >= 'A' && c <= 'Z') c += 'a'-'A';
		return c;
	};

	size_t i = 0;
	std::vector<std::pair<uint32_t, size_t>> hashes = {
		#define xx(n) { CalcCRC32<tolower>(#n), i++ },
		#define xy(n, s) { CalcCRC32<tolower>(s), i++ },
		#define xa(a, n)
		#include "namedef.h"
		#undef xx
		#undef xy
		#undef xa
	};

	std::sort(hashes.begin(), hashes.end());

	for (size_t i = 1; i < hashes.size(); i++)
	{
		auto a = hashes[i], b = hashes[i-1];
		if (a.first == b.first) return std::max(a.second, b.second);
	}

	return 0;
}

FName::NameManager& FName::NameManager::Instance()
{
	static constexpr const char* names[] = {
#define xx(n) #n,
#define xy(n, s) s,
#define xa(a, n)
#include "namedef.h"
#undef xx
#undef xy
#undef xa
	};

	static_assert(std::size(names) > 0, "Name list is empty.");
	static_assert(0 == NAME_None && std::string_view(names[0]) == "None", "'None' must be name 0.");
	static_assert(0 == FindDuplicates(), "Duplicate string found in PredefinedNames array.");

	static FName::NameManager instance { names };
	return instance;
}

template<size_t N>
FName::NameManager::NameManager(const char* const (&predefinedNames)[N])
{
	for (const auto& n : predefinedNames)
	{
		FindName(n, false);
	}
}

//==========================================================================
//
// FName :: NameManager :: FindName
//
// Returns the index of a name. If the name does not exist and noCreate is
// true, then it returns false. If the name does not exist and noCreate is
// false, then the name is added to the table and its new index is returned.
//
//==========================================================================

int FName::NameManager::FindName (const char *text, bool noCreate)
{
	if (text == NULL)
	{
		return 0;
	}

	return FindName(std::string_view(text), noCreate);
}

//==========================================================================
//
// The same as above, but the text length is also passed, for creating
// a name from a substring or for speed if the length is already known.
//
//==========================================================================

int FName::NameManager::FindName (const char *text, size_t textLen, bool noCreate)
{
	if (text == NULL)
	{
		return 0;
	}

	return FindName(std::string_view(text, textLen), noCreate);
}

int FName::NameManager::FindName (const std::string_view str, bool noCreate)
{
	auto lowered = absl::AsciiStrToLower(str);
	auto nameIndex = StringToName.find(lowered);
	if (nameIndex != StringToName.end()) {
		return nameIndex->second;
	}

	if (noCreate) {
		return 0;
	}
	auto num = NumNames;
	NumNames += 1;
	auto allocatedString = std::string(str);
	StringToName.insert({lowered, num});
	NameToString.push_back(allocatedString);

	return num;
}
