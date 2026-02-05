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

#include <cstdint>
#include <cstdio>
#include <iostream>
#include <string.h>
#include <string_view>

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
//
//
//==========================================================================

LangID FStringTable::GetID(FString lang)
{


	FName name = lang;

	static FName systemlocale = NAME_None;

	if (name == NAME_Auto && systemlocale != NAME_None) name = systemlocale;
	if (name == NAME_Auto) systemlocale = name = lang = GetSystemLocale();

	// should I switch to a map? Or add these the namedef.h? If feels weird to have these very specific values in namedef
	bool oldmapping = false; // use old generalized alias, or new real alias
	if (name == "default") { name = lang = "en-US"; }
	else if (name == "by") { name = lang = "be-Latn"; }
	else if (name == "chs") { name = lang = "zh-Hans"; }
	else if (name == "cht") { name = lang = "zh-Hant"; }
	else if (name == "ena") { name = lang = (oldmapping? "en-GB": "en-AU"); }  // I guessed what the en* ones could be
	else if (name == "enb") { name = lang = (oldmapping? "en-GB": "en-BZ"); }
	else if (name == "enc") { name = lang = (oldmapping? "en-GB": "en-CA"); }
	else if (name == "eng") { name = lang = (oldmapping? "en-GB": "en-GB"); }
	else if (name == "eni") { name = lang = (oldmapping? "en-GB": "en-IN"); }
	else if (name == "enj") { name = lang = (oldmapping? "en-GB": "en-JM"); }
	else if (name == "enl") { name = lang = (oldmapping? "en-GB": "en-IE"); }
	else if (name == "ens") { name = lang = (oldmapping? "en-GB": "en-ZA"); }
	else if (name == "ent") { name = lang = (oldmapping? "en-GB": "en-TT"); }
	else if (name == "enw") { name = lang = (oldmapping? "en-GB": "en-GB-wales"); }
	else if (name == "enz") { name = lang = (oldmapping? "en-GB": "en-NZ"); }
	else if (name == "esa") { name = lang = (oldmapping? "es-MX": "es-AR"); }  // I guessed what the es* ones could be
	else if (name == "esb") { name = lang = (oldmapping? "es-MX": "es-BO"); }
	else if (name == "esc") { name = lang = (oldmapping? "es-MX": "es-CO"); }
	else if (name == "esd") { name = lang = (oldmapping? "es-MX": "es-DO"); }
	else if (name == "ese") { name = lang = (oldmapping? "es-MX": "es-EC"); }
	else if (name == "esf") { name = lang = (oldmapping? "es-MX": "es-PH"); }
	else if (name == "esg") { name = lang = (oldmapping? "es-MX": "es-GT"); }
	else if (name == "esh") { name = lang = (oldmapping? "es-MX": "es-HN"); }
	else if (name == "esi") { name = lang = (oldmapping? "es-MX": "es"); }
	else if (name == "esl") { name = lang = (oldmapping? "es-MX": "es-CL"); }
	else if (name == "esm") { name = lang = (oldmapping? "es-MX": "es-MX"); }
	else if (name == "esn") { name = lang = (oldmapping? "es-MX": "es"); }
	else if (name == "eso") { name = lang = (oldmapping? "es-MX": "es-BO"); }
	else if (name == "esr") { name = lang = (oldmapping? "es-MX": "es-CR"); }
	else if (name == "ess") { name = lang = (oldmapping? "es-MX": "es-SV"); }
	else if (name == "esu") { name = lang = (oldmapping? "es-MX": "es-US"); }
	else if (name == "esv") { name = lang = (oldmapping? "es-MX": "es-VE"); }
	else if (name == "esy") { name = lang = (oldmapping? "es-MX": "es-PY"); }
	else if (name == "esz") { name = lang = (oldmapping? "es-MX": "es-BZ"); }
	else if (name == "jp") { name = lang = "ja"; }
	else if (name == "nb") { name = lang = "nb-NO"; }
	else if (name == "no") { name = lang = "nb-NO"; }
	else if (name == "pt") { name = lang = "pt-BR"; }
	else if (name == "ptg") { name = lang = "pt"; }

	auto idPtr = langMap.CheckKey(name);
	if (idPtr) return *idPtr;

	lang.ToLower();
	// TODO: we **could** validate here, but I don't think we need to
	lang.ReplaceChars([](auto c) { return !(('a'<=c&&c<='z')||('0'<=c&&c<='9')); }, ' ');

	FString _lang = "*", _script = "*", _region = "*";

	FScanner sc;
	sc.OpenString("language", lang);

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
			_lang = sc.String;
			step = SCRIPT;
			break;
		case SCRIPT:
			if (sc.StringLen == 4) // script
			{
				_script = FStringf("%c%s", sc.String[0]+('A'-'a'),  sc.String+1);
				step = REGION;
				break;
			}
			// fall-through
		case REGION:
			if (sc.StringLen == 2) // region
			{
				_region = sc.String;
				_region.ToUpper();
				step = DONE;
			}
			break;
		case DONE:
			break;
		}
	}

	auto normalized = _lang + "-" + _script + "-" + _region;
	auto script     = _lang + "-" + _script + "-*";
	auto language   = _lang + "-*-*";

	LangID id = {
		name,
		CalcCRC32(lang.GetChars()),
		CalcCRC32(normalized.GetChars()),
		CalcCRC32(script.GetChars()),
		CalcCRC32(language.GetChars())
	};
	auto ptr = &langMap.Insert(name, id);
	langRevMap.Insert(ptr->normalized, ptr);

	auto fallback = langRevMap.CheckKey(ptr->script);
	if (!fallback || ((*fallback)->script != (*fallback)->normalized))
	{
		auto ptr = &langMap.Insert(script, id);
		langRevMap.Insert(ptr->script, ptr);
	}
	fallback = langRevMap.CheckKey(ptr->language);
	if (!fallback || ((*fallback)->language != (*fallback)->normalized))
	{
		auto ptr = &langMap.Insert(language, id);
		langRevMap.Insert(ptr->language, ptr);
	}

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
						langrows.Push(std::make_pair(column, default_table));
						hasDefaultEntry = true;
					}
					else
					{
						langrows.Push(std::make_pair(column, GetID(lang).normalized));
					}
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
				if (len == 1 && sc.String[0] == '*')
				{
					activeMaps.Clear();
					activeMaps.Push(global_table);
				}
				else if (len == 7 && stricmp (sc.String, "default") == 0)
				{
					activeMaps.Clear();
					activeMaps.Push(default_table);
					hasDefaultEntry = true;
				}
				else if (activeMaps.Size() != 1 || (activeMaps[0] != default_table && activeMaps[0] != global_table))
				{
					activeMaps.Push(GetID(sc.String).normalized);
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
//
//
//==========================================================================

void FStringTable::UpdateLanguage(const char *language)
{
	if (language) activeLanguage = language;
	else language = activeLanguage.GetChars();
	size_t langlen = strlen(language);

	auto LanguageID = ((langlen < 2) ? GetID("default"): GetID(language));

	currentLanguageSet.Clear();

	auto checkone = [&](uint32_t lang_id)
	{
		auto list = allStrings.CheckKey(lang_id);
		if (list && currentLanguageSet.FindEx([&](const auto &element) { return element.first == lang_id; }) == currentLanguageSet.Size())
			currentLanguageSet.Push(std::make_pair(lang_id, list));
	};

	checkone(override_table);
	checkone(global_table);
	checkone(LanguageID.normalized);
	checkone(LanguageID.script);
	checkone(LanguageID.language);
	checkone(default_table);
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
		static TMap<FName, bool> missed;

		FName fname = name;
		if (!missed.CheckKey(fname))
		{
			missed.Insert(fname, true);
			DPrintf(DMSG_WARNING, "Translation not found '%s'\n", name);
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


