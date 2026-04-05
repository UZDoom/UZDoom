/*
** stringtable.cpp
**
** Implements the FStringTable class
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2010-2016 Christoph Oelckers
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

#include <algorithm>

#include "basics.h"
#include "c_cvars.h"
#include "filesystem.h"
#include "i_interface.h"
#include "m_crc32.h"
#include "name.h"
#include "printf.h"
#include "sc_man.h"
#include "stringtable.h"
#include "zstring.h"

EXTERN_CVAR(Int, developer);
CVAR(Bool, debug_languages, false, CVAR_GLOBALCONFIG | CVAR_ARCHIVE);
CVAR(Int, language_debug_maxlen, 64, 0);

//==========================================================================
//
//
//
//==========================================================================

#ifndef _WIN32
// TODO: verify this is correct for apple
FString FStringTable::GetSystemLocale()
{
	const char* lang  = std::getenv("LC_ALL");
	if (!lang) lang = std::getenv("LC_MESSAGES");
	if (!lang) lang = std::getenv("LANG");

	if (!lang || std::string_view(lang)=="C" || std::string_view(lang)=="POSIX") return "en-US";

	FString tag = lang;
	auto dot = tag.IndexOfAny(".@");
	if (dot != -1) tag=tag.Left(dot);
	tag.ReplaceChars('_', '-');

	return tag;
}
#else
// this lives somewhere we already have <windows.h>
#endif

//==========================================================================
//
// Map old semi-made-up language codes to IETF language tags
//
// https://zdoom.org/w/index.php?title=LANGUAGE#Language_codes
// https://docs.google.com/spreadsheets/d/1pvwXEgytkor9SClCiDn4j5AH7FedyXS-ocCbsuQIXDU
//
//==========================================================================

inline bool RemapLegacyLanguages(FName &name, FString &lang)
{
	FName oldname = name;

	switch (name.GetIndex())
	{
		case NAME_LANG_by:
			name = "be";
			break;
		case NAME_LANG_ena:
		case NAME_LANG_enb:
		case NAME_LANG_enc:
		case NAME_LANG_eng:
		case NAME_LANG_eni:
		case NAME_LANG_enj:
		case NAME_LANG_enl:
		case NAME_LANG_ens:
		case NAME_LANG_ent:
		case NAME_LANG_enw:
		case NAME_LANG_enz:
			name = "en-GB";
			break;
		case NAME_Default:
		case NAME_LANG_enu:
			name = "en-US";
			break;
		case NAME_LANG_esa:
		case NAME_LANG_esb:
		case NAME_LANG_esc:
		case NAME_LANG_esd:
		case NAME_LANG_ese:
		case NAME_LANG_esf:
		case NAME_LANG_esg:
		case NAME_LANG_esh:
		case NAME_LANG_esi:
		case NAME_LANG_esl:
		case NAME_LANG_esm:
		case NAME_LANG_esn:
		case NAME_LANG_eso:
		case NAME_LANG_esr:
		case NAME_LANG_ess:
		case NAME_LANG_esu:
		case NAME_LANG_esv:
		case NAME_LANG_esy:
		case NAME_LANG_esz:
			name = "es-MX";
			break;
		case NAME_LANG_ja:
		case NAME_LANG_jp:
			name = "ja-JP";
			break;
		case NAME_LANG_nb:
		case NAME_LANG_no:
			name = "nb-NO";
			break;
		case NAME_LANG_ptg:
			name = "pt";
			break;
		case NAME_LANG_chi:
		case NAME_LANG_cht:
			name = "zh-Hant";
			break;
		case NAME_LANG_chs:
		case NAME_LANG_zho:
			name = "zh-Hans";
			break;
	}

	bool updated = name != oldname;
	if (updated) lang = name.GetChars();
	return updated;
}

//==========================================================================
//
// Map languages to other languages
//
// Used too get the proper fallback of a language, if the region inherits
// from another region (or script) instead the base ISO 639 language code
// of the tag
//
//==========================================================================

inline FName GetFallback(FName name)
{
	// `K` must always be a normalized IETF BCP 47 triplet (lang-script-region). Use * for any omitted section
	// `aa-BB` -> `aa-*-BB`, `aa-Cccc` -> `aa-Cccc-*`
	static struct { FName K; FName V; } mappings[] = {
		{"en-*-AU", "en-GB"},
		{"en-*-CA", "en-GB"},
		{"en-*-SD", "en-GB"}, // TODO: support Subdivision
		{"zh-*-CN", "zh-Hans-CN"},
		{"zh-*-HK", "zh-Hant-HK"},
		{"zh-*-MO", "zh-Hant-MO"},
		{"zh-*-SG", "zh-Hans-SG"},
		{"zh-*-TW", "zh-Hant-TW"},
	};

	constexpr size_t count = sizeof(mappings)/sizeof(mappings[0]);
	static bool sorted = false;
	if (!sorted)
	{
		std::sort(mappings, mappings+count, [](auto A, auto B) { return A.K < B.K; });
		sorted = true;
	}

	int lo = 0, hi = count - 1, mid;
	while (lo <= hi)
	{
		mid = lo + (hi - lo) / 2;
		if (mappings[mid].K == name) return mappings[mid].V;
		if (mappings[mid].K < name) lo = mid + 1;
		else hi = mid - 1;
	}

	return NAME_None;
}

//==========================================================================
//
// Take ietf language tag, and extract all of the relevant bits
//
// TODO: support extensions (see: English from Wales)
//
//==========================================================================

inline void ExtractComponents(FString &str, FString &lang, FString &script, FString &region)
{
	str.ToLower();
	// TODO: we **could** validate here, but I don't think we need to
	str.ReplaceChars([](auto c) { return !(('a'<=c&&c<='z')||('0'<=c&&c<='9')); }, ' ');

	lang = script = region = "*";

	FScanner sc;
	sc.OpenString("language", str);

	enum { LANG, SCRIPT, REGION, DONE };

	auto step = LANG;
	while (sc.GetString())
	{
		if (sc.StringLen == 1 && sc.String[0] == 'x') // private-use block. skip the rest
		{
			step = DONE;
			break;
		}
		switch (step)
		{
		case LANG:
			lang = sc.String;
			step = SCRIPT;
			break;
		case SCRIPT:
			if (sc.StringLen == 4) // script
			{
				script = FStringf("%c%s", sc.String[0]+('A'-'a'),  sc.String+1);
				step = REGION;
				break;
			}
			// fall-through
		case REGION:
			if (sc.StringLen == 2) // region
			{
				region = sc.String;
				region.ToUpper();
				step = DONE;
			}
			break;
		case DONE:
			break;
		}
	}

	sc.Close();
}

//==========================================================================
//
//
//
//==========================================================================

LangID FStringTable::GetID(FString lang)
{
	FName name = lang;
	FString diagnostics;

	if (debug_languages) diagnostics.AppendFormat("lang: %s", lang.GetChars());

	if (name == NAME_Auto)
	{
		name = lang = GetSystemLocale();
		if (debug_languages) diagnostics.AppendFormat("(%s)", lang.GetChars());
	}

	if (RemapLegacyLanguages(name, lang) && debug_languages)
	{
		diagnostics.AppendFormat(", mapped: %s", lang.GetChars());
	}

	{
		auto ptr = langMap.CheckKey(name);
		if (ptr)
		{
			if (debug_languages) Printf("%s.\n", diagnostics.GetChars());
			return *ptr;
		}
	}

	FString _lang, _script, _region;
	ExtractComponents(lang, _lang, _script, _region);

	auto normalized = _lang + "-" + _script + "-" + _region;
	auto script     = _lang + "-" + _script + "-*";
	auto language   = _lang + "-*-*";

	{
		auto ptr = langMap.CheckKey(normalized);
		if (ptr)
		{
			if (debug_languages) Printf("%s.\n", diagnostics.GetChars());
			return *ptr;
		}
	}

	LangID id = {
		normalized,
		GetFallback(normalized),
		CalcCRC32(normalized.GetChars()),
		CalcCRC32(script.GetChars()),
		CalcCRC32(language.GetChars()),
	};
	if (name != normalized) langMap.Insert(name, id);
	langMap.Insert(normalized, id);
	langRevMap.Insert(id.normalized, normalized);

	if (debug_languages)
	{
		diagnostics.AppendFormat(
			" inserted: %s (%c-%x)",
			normalized.GetChars(),
			id.normalized == id.language? 'L': id.normalized == id.script? 'S': 'R',
			id.normalized
		);

		if (id.fallback != NAME_None) diagnostics.AppendFormat(" '%s'?", id.fallback.GetChars());
	}

	if (id.normalized != id.script)
	{
		auto fallback = langRevMap.CheckKey(id.script);
		if (debug_languages) diagnostics.AppendFormat(", %s", script.GetChars());
		if (!fallback)
		{
			langMap.Insert(script, id);
			langRevMap.Insert(id.script, script);
			diagnostics.AppendFormat(" (%c-%x)", id.script == id.language? 'L': 'S', id.script);
		}
	}
	if (id.normalized != id.language)
	{
		auto fallback = langRevMap.CheckKey(id.language);
		if (debug_languages) diagnostics.AppendFormat(", %s", language.GetChars());
		if (!fallback)
		{
			langMap.Insert(language, id);
			langRevMap.Insert(id.language, script);
			if (debug_languages) diagnostics.AppendFormat(" (L-%x)", id.language);
		}
	}

	if (debug_languages) Printf("%s\n", diagnostics.GetChars());

	return id;
}

//==========================================================================
//
//
//
//==========================================================================

void FStringTable::LoadStrings (FileSys::FileSystem& fileSystem, const char *language)
{
	int lastlump, lump;

	allStrings.Clear();
	lastlump = 0;
	while ((lump = fileSystem.FindLump("LMACROS", &lastlump)) != -1)
	{
		auto lumpdata = fileSystem.ReadFile(lump);
		readMacros(lumpdata.string(), lumpdata.size());
	}

	lastlump = 0;
	while ((lump = fileSystem.FindLump ("LANGUAGE", &lastlump)) != -1)
	{
		auto lumpdata = fileSystem.ReadFile(lump);
		auto filenum = fileSystem.GetFileContainer(lump);

		if (!ParseLanguageCSV(filenum, lumpdata.string(), lumpdata.size()))
 			LoadLanguage (filenum, lumpdata.string(), lumpdata.size());
	}
	UpdateLanguage(language);
	allMacros.Clear();
}

//==========================================================================
//
// This was tailored to parse CSV as exported by Google Docs.
//
//==========================================================================

TArray<TArray<FString>> FStringTable::parseCSV(const char* buffer, size_t size)
{
	const size_t bufLength = size;
	TArray<TArray<FString>> data;
	TArray<FString> row;
	TArray<char> cell;
	bool quoted = false;

	/*
			auto myisspace = [](int ch) { return ch == '\t' || ch == '\r' || ch == '\n' || ch == ' '; };
			while (*vcopy && myisspace((unsigned char)*vcopy)) vcopy++;	// skip over leaading whitespace;
			auto vend = vcopy + strlen(vcopy);
			while (vend > vcopy && myisspace((unsigned char)vend[-1])) *--vend = 0;	// skip over trailing whitespace
	*/

	for (size_t i = 0; i < bufLength; ++i)
	{
		if (buffer[i] == '"')
		{
			// Double quotes inside a quoted string count as an escaped quotation mark.
			if (quoted && i < bufLength - 1 && buffer[i + 1] == '"')
			{
				cell.Push('"');
				i++;
			}
			else if (cell.Size() == 0 || quoted)
			{
				quoted = !quoted;
			}
		}
		else if (buffer[i] == ',')
		{
			if (!quoted)
			{
				cell.Push(0);
				ProcessEscapes(cell.Data());
				row.Push(cell.Data());
				cell.Clear();
			}
			else
			{
				cell.Push(buffer[i]);
			}
		}
		else if (buffer[i] == '\r')
		{
			// Ignore all CR's.
		}
		else if (buffer[i] == '\n' && !quoted)
		{
			cell.Push(0);
			ProcessEscapes(cell.Data());
			row.Push(cell.Data());
			data.Push(std::move(row));
			cell.Clear();
		}
		else
		{
			cell.Push(buffer[i]);
		}
	}

	// Handle last line without linebreak
	if (cell.Size() > 0 || row.Size() > 0)
	{
		cell.Push(0);
		ProcessEscapes(cell.Data());
		row.Push(cell.Data());
		data.Push(std::move(row));
	}
	return data;
}

//==========================================================================
//
//
//
//==========================================================================

bool FStringTable::readMacros(const char* buffer, size_t size)
{
	auto data = parseCSV(buffer, size);

	allMacros.Clear();
	for (unsigned i = 1; i < data.Size(); i++)
	{
		auto macroname = data[i][0];
		FName name = macroname.GetChars();

		StringMacro macro;

		for (int k = 0; k < 4; k++)
		{
			macro.Replacements[k] = data[i][k+2];
		}
		allMacros.Insert(name, macro);
	}
	return true;
}

//==========================================================================
//
//
//
//==========================================================================

bool FStringTable::ParseLanguageCSV(int filenum, const char* buffer, size_t size)
{
	if (size < 11) return false;
	if (strnicmp(buffer, "default,", 8) && strnicmp(buffer, "identifier,", 11 )) return false;
	auto data = parseCSV(buffer, size);

	int labelcol = -1;
	int filtercol = -1;
	TArray<std::pair<int, uint32_t>> langrows;
	bool hasDefaultEntry = false;

	if (data.Size() > 0)
	{
		for (unsigned column = 0; column < data[0].Size(); column++)
		{
			auto &entry = data[0][column];
			if (entry.CompareNoCase("filter") == 0)
			{
				filtercol = column;
			}
			else if (entry.CompareNoCase("identifier") == 0)
			{
				labelcol = column;
			}
			else
			{
				auto languages = entry.Split(" ", FString::TOK_SKIPEMPTY);
				for (auto &lang : languages)
				{
					if (lang.CompareNoCase("default") == 0)
					{
						lang = "en-US";
						hasDefaultEntry = true;
					}
					langrows.Push(std::make_pair(column, GetID(lang).normalized));
				}
			}
		}

		for (unsigned i = 1; i < data.Size(); i++)
		{
			auto &row = data[i];
			if (filtercol > -1)
			{
				auto filterstr = row[filtercol];
				if (filterstr.IsNotEmpty())
				{
					bool ok = false;
					if (sysCallbacks.CheckGame)
					{
						auto filter = filterstr.Split(" ", FString::TOK_SKIPEMPTY);
						for (auto& entry : filter)
						{
							if (sysCallbacks.CheckGame(entry.GetChars()))
							{
								ok = true;
								break;
							}
						}
					}
					if (!ok) continue;
				}
			}

			row[labelcol].StripLeftRight();
			FName strName = row[labelcol].GetChars();
			if (hasDefaultEntry)
			{
				DeleteForLabel(filenum, strName);
			}
			for (auto &langentry : langrows)
			{
				auto str = row[langentry.first];
				if (str.Len() > 0)
				{
					InsertString(filenum, langentry.second, strName, str);
				}
				else
				{
					DeleteString(langentry.second, strName);
				}
			}
		}
	}
	return true;
}

//==========================================================================
//
//
//
//==========================================================================

void FStringTable::LoadLanguage (int lumpnum, const char* buffer, size_t size)
{
	bool errordone = false;
	TArray<uint32_t> activeMaps;
	FScanner sc;
	bool hasDefaultEntry = false;

	sc.OpenMem(fileSystem.GetFileFullPath(lumpnum).c_str(), buffer, (int)size);
	sc.SetCMode (true);
	while (sc.GetString ())
	{
		if (sc.Compare ("["))
		{ // Process language identifiers
			activeMaps.Clear();
			hasDefaultEntry = false;
			sc.MustGetString ();
			do
			{
				size_t len = sc.StringLen;

				if (len < 1)
				{
					sc.ScriptError ("The language code may not be empty.");
				}
				if (len == 1 && sc.String[0] == '~')
				{
					// deprecated and ignored
					sc.ScriptMessage("Deprecated option '~' found in language list");
					sc.MustGetString ();
					continue;
				}
				auto id = GetID(sc.String);
				if (len == 1 && sc.String[0] == '*')
				{
					activeMaps.Clear();
					activeMaps.Push(global_table);
				}
				else if (id.normalized == default_table)
				{
					activeMaps.Clear();
					activeMaps.Push(id.normalized);
					hasDefaultEntry = true;
				}
				else if (activeMaps.Size() != 1 || (activeMaps[0] != default_table && activeMaps[0] != global_table))
				{
					activeMaps.Push(id.normalized);
				}

				sc.MustGetString ();
			} while (!sc.Compare ("]"));
		}
		else
		{ // Process string definitions.
			if (activeMaps.Size() == 0)
			{
				// LANGUAGE lump is bad. We need to check if this is an old binary
				// lump and if so just skip it to allow old WADs to run which contain
				// such a lump.
				if (!sc.isText())
				{
					if (!errordone) Printf("Skipping binary 'LANGUAGE' lump.\n");
					errordone = true;
					return;
				}
				sc.ScriptError ("Found a string without a language specified.");
			}

			bool skip = false;
			if (sc.Compare("$"))
			{
				sc.MustGetStringName("ifgame");
				sc.MustGetStringName("(");
				sc.MustGetString();
				skip |= (!sysCallbacks.CheckGame || !sysCallbacks.CheckGame(sc.String));
				sc.MustGetStringName(")");
				sc.MustGetString();

			}

			FName strName (sc.String);
			sc.MustGetStringName ("=");
			sc.MustGetString ();
			FString strText (sc.String, ProcessEscapes (sc.String));
			sc.MustGetString ();
			while (!sc.Compare (";"))
			{
				ProcessEscapes (sc.String);
				strText += sc.String;
				sc.MustGetString ();
			}
			if (!skip)
			{
				if (hasDefaultEntry)
				{
					DeleteForLabel(lumpnum, strName);
				}
				// Insert the string into all relevant tables.
				for (auto map : activeMaps)
				{
					InsertString(lumpnum, map, strName, strText);
				}
			}
		}
	}
}

//==========================================================================
//
//
//
//==========================================================================

void FStringTable::DeleteString(uint32_t langid, FName label)
{
	allStrings[langid].Remove(label);
}

//==========================================================================
//
// This deletes all older entries for a given label. This gets called
// when a string in the default table gets updated.
//
//==========================================================================

void FStringTable::DeleteForLabel(int filenum, FName label)
{
	decltype(allStrings)::Iterator it(allStrings);
	decltype(allStrings)::Pair *pair;

	while (it.NextPair(pair))
	{
		auto entry = pair->Value.CheckKey(label);
		if (entry && entry->filenum < filenum)
		{
			pair->Value.Remove(label);
		}
	}

}

//==========================================================================
//
//
//
//==========================================================================

void FStringTable::InsertString(int filenum, uint32_t langid, FName label, const FString &string)
{
	const char *strlangid = (const char *)&langid;
	TableElement te = { filenum, { string, string, string, string } };
	ptrdiff_t index;
	while ((index = te.strings[0].IndexOf("@[")) >= 0)
	{
		auto endindex = te.strings[0].IndexOf(']', index);
		if (endindex == -1)
		{
			Printf("Bad macro in %s : %s\n", strlangid, label.GetChars());
			break;
		}
		FString macroname(te.strings[0].GetChars() + index + 2, endindex - index - 2);
		FStringf replacee("@[%s]", macroname.GetChars());
		FName lookupname(macroname.GetChars(), true);
		auto replace = allMacros.CheckKey(lookupname);
		for (int i = 0; i < 4; i++)
		{
			const char *replacement = replace? replace->Replacements[i].GetChars() : "";
			te.strings[i].Substitute(replacee, replacement);
		}
	}
	allStrings[langid].Insert(label, te);
}

//==========================================================================
//
// For every language in the lookup chain, run the function
//
// `name` is a bcp 47 triplet
// `lang` is the internal language ID
// `set`  is one of O/G/R/r/S/s/L/l/D
//   O: overrides
//   G: globals
//   R: region,   r: fallback region
//   S: script,   s: fallback script,
//   L: language, l: fallback language
//   D: default
//
//==========================================================================

void FStringTable::ForEachLangID(LangID language, std::function<void(FName name, uint32_t lang, char set)> callback)
{
	auto fallback = (language.fallback==NAME_None)? (LangID{NAME_None}): GetID(language.fallback.GetChars());

	struct { char k; bool n; uint32_t v; } order[] = {
		{'O', false, override_table},
		{'G', false, global_table},
		{'R', true,  language.normalized},
		{'r', true,  fallback.name == NAME_None? NAME_None: fallback.normalized},
		{'S', true,  language.script},
		{'s', true,  fallback.name == NAME_None? NAME_None: fallback.script},
		{'L', true,  language.language},
		{'l', true,  fallback.name == NAME_None? NAME_None: fallback.language},
		{'D', true,  default_table},
	};
	int count = sizeof(order) / sizeof(order[0]);

	for (int i = 0; i < count; i++)
	{
		auto set_id = order[i].k;
		auto lang_id = order[i].v;
		for (int j = i-1; j >= 0; j--)
		{
			if (order[j].v == lang_id) { lang_id = NAME_None; break; }
		}
		if (lang_id == NAME_None) continue;
		FName lang = NAME_None;
		if (order[i].n) // not override or global
		{
			lang = *langRevMap.CheckKey(lang_id);
			assert(lang.IsValidName());
			if (lang == NAME_None) continue;
		}
		callback(lang, lang_id, set_id);
	}
}

//==========================================================================
//
//
//
//==========================================================================

void FStringTable::UpdateLanguage(const char *language)
{
	if (language) activeLanguage = language;
	else language = activeLanguage.GetChars();
	size_t langlen = strlen(language);

	auto LanguageID = ((langlen < 2) ? GetID("default"): GetID(language));

	langName = langScript = NAME_None;
	currentLanguageSet.Clear();

	FString diagnostics = "";
	ForEachLangID(LanguageID, [this, &diagnostics](FName name, uint32_t lang, char set)
	{
		auto list = allStrings.CheckKey(lang);
		if (!list) return;
		if (name != NAME_None && (langName == NAME_None || langScript == NAME_None))
		{
			void *ptr;
			ptr = langMap.CheckKey(name);
			assert(ptr != nullptr);
			auto script_id = static_cast<LangID*>(ptr)->script;
			ptr = langRevMap.CheckKey(script_id);
			assert(ptr != nullptr);
			FName script = *static_cast<FName*>(ptr);
			assert(script.IsValidName());
			if (langName == NAME_None) langName = name;
			if (langScript == NAME_None) langScript = script;
		}
		currentLanguageSet.Push(std::make_pair(lang, list));
		if (debug_languages) diagnostics.AppendFormat(" %c-%x", set, lang);
	});
	if (debug_languages) Printf("Strings %s: %s (%s) %s \n", language, langName.GetChars(), langScript.GetChars(), diagnostics.GetChars());
}

//==========================================================================
//
// Replace \ escape sequences in a string with the escaped characters.
//
//==========================================================================

size_t FStringTable::ProcessEscapes (char *iptr)
{
	char *sptr = iptr, *optr = iptr, c;

	while ((c = *iptr++) != '\0')
	{
		if (c == '\\')
		{
			c = *iptr++;
			if (c == 'n')
				c = '\n';
			else if (c == 'c')
				c = TEXTCOLOR_ESCAPE;
			else if (c == 'r')
				c = '\r';
			else if (c == 't')
				c = '\t';
			else if (c == 'x')
			{
				c = 0;
				for (int i = 0; i < 2; i++)
				{
					char cc = *iptr++;
					if (cc >= '0' && cc <= '9')
						c = (c << 4) + cc - '0';
					else if (cc >= 'a' && cc <= 'f')
						c = (c << 4) + 10 + cc - 'a';
					else if (cc >= 'A' && cc <= 'F')
						c = (c << 4) + 10 + cc - 'A';
					else
					{
						iptr--;
						break;
					}
				}
				if (c == 0) continue;
			}
			else if (c == '\n')
				continue;
		}
		*optr++ = c;
	}
	*optr = '\0';
	return optr - sptr;
}

//==========================================================================
//
// Checks if the given key exists in any one of the default string tables that are valid for all languages.
// To replace IWAD content this condition must be true.
//
//==========================================================================

bool FStringTable::exists(const char *name)
{
	if (name == nullptr || *name == 0)
	{
		return false;
	}
	FName nm(name, true);
	if (nm != NAME_None)
	{
		uint32_t defaultStrings[] = { default_table, global_table, override_table };

		for (auto mapid : defaultStrings)
		{
			auto map = allStrings.CheckKey(mapid);
			if (map)
			{
				auto item = map->CheckKey(nm);
				if (item) return true;
			}
		}
	}
	return false;
}

//==========================================================================
//
// Finds a string by name and returns its value
//
//==========================================================================

const char *FStringTable::CheckString(const char *name, uint32_t *langtable, int gender) const
{
	if (name == nullptr || *name == 0)
	{
		return nullptr;
	}
	if (gender == -1) gender = defaultgender;
	if (gender < 0 || gender > 3) gender = 0;
	FName nm(name, true);
	if (nm != NAME_None)
	{
		TableElement* bestItem = nullptr;
		for (auto map : currentLanguageSet)
		{
			auto item = map.second->CheckKey(nm);
			if (item)
			{
				if (bestItem && bestItem->filenum > item->filenum)
				{
					// prioritize content from later files, even if the language doesn't fully match.
					// This is mainly for Dehacked content.
					continue;
				}
				if (langtable) *langtable = map.first;
				auto c = item->strings[gender].GetChars();
				if (c && *c == '$' && c[1] == '$')
					c = CheckString(c + 2, langtable, gender);
				return c;
			}
		}
	}
	return nullptr;
}

//==========================================================================
//
// Finds a string by name in a given language without attempting any substitution
//
//==========================================================================

const char *FStringTable::GetLanguageString(const char *name, uint32_t langtable, int gender) const
{
	if (name == nullptr || *name == 0)
	{
		return nullptr;
	}
	if (gender == -1) gender = defaultgender;
	if (gender < 0 || gender > 3) gender = 0;
	FName nm(name, true);
	if (nm != NAME_None)
	{
		auto map = allStrings.CheckKey(langtable);
		if (map == nullptr) return nullptr;
		auto item = map->CheckKey(nm);
		if (item)
		{
			return item->strings[gender].GetChars();
		}
	}
	return nullptr;
}

bool FStringTable::MatchDefaultString(const char *name, const char *content) const
{
	// This only compares the first line to avoid problems with bad linefeeds. For the few cases where this feature is needed it is sufficient.
	auto c = GetLanguageString(name, FStringTable::default_table);
	if (!c) return false;

	// Check a secondary key, in case the text comparison cannot be done due to needed orthographic fixes (see Harmony's exit text)
	FStringf checkkey("%s_CHECK", name);
	auto cc = GetLanguageString(checkkey.GetChars(), FStringTable::default_table);
	if (cc) c = cc;

	return (c && !strnicmp(c, content, strcspn(content, "\n\r\t")));
}

//==========================================================================
//
// Finds a string by name and returns its value. If the string does
// not exist, returns the passed name instead.
//
//==========================================================================

const char *FStringTable::GetString(const char *name) const
{
	const char *str = CheckString(name);

	if (developer != 0 && !str)
	{
		static TMap<uint32_t, bool> missed;

		FString str{name};
		size_t len = str.Len();
		bool truncated = str.Len() > static_cast<size_t>(language_debug_maxlen);
		if (truncated)
		{
			str.Truncate(language_debug_maxlen);
			str.AppendFormat("...");
		}

		uint32_t hash = CalcCRC32(str.GetChars());
		if (!missed.CheckKey(hash))
		{
			missed.Insert(hash, true);

			FString message = "Translation not found ";
			if (truncated) message.AppendFormat("(truncated) ");
			message.AppendCharacter('\'');
			static const char chars[][2] {
				{ '\a', 'a' }, { '\b', 'b' }, { '\t', 't' },
				{ '\n', 'n' }, { '\f', 'f' }, { '\r', 'r' },
			};
			for (size_t i = 0; i < str.Len(); i++)
			{
				auto c = str[i];
				for (auto ch : chars)
				{
					if (c != ch[0]) continue;
					c = ch[1];
					message.AppendCharacter('\\');
					break;
				}
				message.AppendCharacter(c);
			}
			message.AppendCharacter('\'');

			DPrintf(DMSG_WARNING, "%s\n", message.GetChars());
		}
	}

	return str ? str : name;
}

//==========================================================================
//
// Find a string with the same exact text. Returns its name.
// This does not need to check genders, it is only used by
// Dehacked on the English table for finding stock strings.
//
//==========================================================================

const char *StringMap::MatchString (const char *string) const
{
	StringMap::ConstIterator it(*this);
	StringMap::ConstPair *pair;

	while (it.NextPair(pair))
	{
		if (pair->Value.strings[0].CompareNoCase(string) == 0)
		{
			return pair->Key.GetChars();
		}
	}
	return nullptr;
}

FStringTable GStrings;
