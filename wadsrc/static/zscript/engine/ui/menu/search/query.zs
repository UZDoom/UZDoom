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
** Copyright 2025 UZDoom Maintainers and Contributors
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
