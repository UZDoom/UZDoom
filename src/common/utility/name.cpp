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

#include <string.h>

#include "name.h"

#include "absl/strings/ascii.h"

// CODE --------------------------------------------------------------------

FName::NameManager::NameManager(std::initializer_list<const char*> predefinedNames) {
	assert(0 == strcmp(PredefinedNames[0], "None") && "'None' must be name 0.");
	for (const auto& n : predefinedNames) {
		assert((0 == FindName(PredefinedNames[i], true)) && "Predefined name already inserted");
		FindName(n, false);
	}
}

FName::NameManager& FName::NameManager::Instance() {
	static FName::NameManager instance {
#define xx(n) #n,
#define xy(n, s) s,
#define xa(a, n)
#include "namedef.h"
#if __has_include("namedef_custom.h")
	#include "namedef_custom.h"
#endif
#undef xx
#undef xy
#undef xa
	};
	return instance;
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
