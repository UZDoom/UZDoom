/*
** query.zs
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1999-2016 Marisa Heit
** Copyright 2006-2016 Christoph Oelckers
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

//=============================================================================
//
// Option Search Query class represents a search query.
// A search query consists constists of one or more terms (words).
//
// Query matching deponds on "os_is_any_of" variable.
// If this variable is "true", the text matches the query if any of the terms
// matches the query.
// If this variable is "false", the text matches the query only if all the
// terms match the query.
//
//=============================================================================

class os_Query
{
	static os_Query fromString(string str)
	{
		let query = new("os_Query");

		str.Split(query.mQueryParts, " ", TOK_SKIPEMPTY);

		return query;
	}

	bool matches(string text, bool isSearchForAny)
	{
		return isSearchForAny
			? matchesAny(text)
			: matchesAll(text);
	}

	// private: //////////////////////////////////////////////////////////////////

	private bool matchesAny(string text)
	{
		int nParts = mQueryParts.size();

		for (int i = 0; i < nParts; ++i)
		{
			string queryPart = mQueryParts[i];

			if (contains(text, queryPart)) { return true; }
		}

		return false;
	}

	private bool matchesAll(string text)
	{
		int nParts = mQueryParts.size();

		for (int i = 0; i < nParts; ++i)
		{
			string queryPart = mQueryParts[i];

			if (!contains(text, queryPart)) { return false; }
		}

		return true;
	}

	private static bool contains(string str, string substr)
	{
		let lowerstr    = str   .MakeLower();
		let lowersubstr = substr.MakeLower();

		bool contains = (lowerstr.IndexOf(lowersubstr) != -1);

		return contains;
	}

	private Array<String> mQueryParts;
}
