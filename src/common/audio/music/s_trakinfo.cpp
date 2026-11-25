/*
** s_trakinfo.cpp
**
** TRAKINFO lump parser and soundtrack lookup
**
**---------------------------------------------------------------------------
**
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#include "s_trakinfo.h"

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <string>
#include <unordered_map>

#include "c_cvars.h"
#include "filesystem.h"
#include "printf.h"
#include "serializer_rapidjson.h"
#include "rapidjson/error/en.h"

#define USE_STD_SPAN_CHOCOBO1_HASH 1
#include "sha1.h"

namespace
{

struct TrakInfoEntry
{
	std::unordered_map<std::string, std::string> tracks;
	float volume = -1.f;
};

std::unordered_map<std::string, TrakInfoEntry> TrakInfoMap;

std::string ToLowerASCII(const char *str, size_t len)
{
	std::string out;
	out.reserve(len);
	for (size_t i = 0; i < len; ++i)
	{
		out.push_back(char(tolower(static_cast<unsigned char>(str[i]))));
	}
	return out;
}

bool NormalizeSHA1Key(const char *str, size_t len, std::string &out)
{
	if (len != 40)
		return false;

	out.clear();
	out.reserve(40);
	for (size_t i = 0; i < 40; ++i)
	{
		unsigned char c = static_cast<unsigned char>(str[i]);
		if (!isxdigit(c))
			return false;
		out.push_back(char(tolower(c)));
	}
	return true;
}

std::string ComputeSHA1Hex(const void *data, size_t len)
{
	return Chocobo1::SHA1().addData(data, len).finalize().toString();
}

bool ParseVolumeValue(const rapidjson::Value &val, float &outVolume)
{
	double parsed = -1.0;
	if (val.IsNumber())
	{
		parsed = val.GetDouble();
	}
	else if (val.IsString() && val.GetStringLength() > 0)
	{
		char *endptr = nullptr;
		parsed = strtod(val.GetString(), &endptr);
		if (endptr == val.GetString())
			return false;
		while (*endptr != '\0' && isspace(static_cast<unsigned char>(*endptr)))
			++endptr;
		if (*endptr != '\0')
			return false;
	}
	else
	{
		return false;
	}

	if (!std::isfinite(parsed) || parsed <= 0.0)
		return false;

	// Cap extreme values to prevent accidental audio blasting.
	if (parsed > 10.0)
		parsed = 10.0;

	outVolume = static_cast<float>(parsed);
	return true;
}

void ParseTrakInfoLump(int lump)
{
	auto lumpData = fileSystem.ReadFile(lump);
	if (lumpData.size() == 0)
		return;

	rapidjson::Document doc;
	constexpr unsigned parseFlags =
		rapidjson::kParseDefaultFlags |
		rapidjson::kParseCommentsFlag |
		rapidjson::kParseTrailingCommasFlag;
	doc.Parse<parseFlags>(lumpData.string(), lumpData.size());

	if (doc.HasParseError())
	{
		auto path = fileSystem.GetFileFullPath(lump);
		Printf(TEXTCOLOR_RED "TRAKINFO (%s): JSON parse error at offset %zu: %s\n",
			path.c_str(),
			doc.GetErrorOffset(),
			rapidjson::GetParseError_En(doc.GetParseError()));
		return;
	}

	if (!doc.IsObject())
	{
		auto path = fileSystem.GetFileFullPath(lump);
		Printf(TEXTCOLOR_RED "TRAKINFO (%s): root JSON value must be an object\n", path.c_str());
		return;
	}

	for (auto it = doc.MemberBegin(); it != doc.MemberEnd(); ++it)
	{
		std::string sha1Key;
		if (!NormalizeSHA1Key(it->name.GetString(), it->name.GetStringLength(), sha1Key))
		{
			auto path = fileSystem.GetFileFullPath(lump);
			Printf(TEXTCOLOR_YELLOW "TRAKINFO (%s): ignoring invalid SHA-1 key '%.*s'\n",
				path.c_str(),
				int(it->name.GetStringLength()),
				it->name.GetString());
			continue;
		}

		if (!it->value.IsObject())
			continue;

		TrakInfoEntry &entry = TrakInfoMap[sha1Key];
		for (auto prop = it->value.MemberBegin(); prop != it->value.MemberEnd(); ++prop)
		{
			std::string propName = ToLowerASCII(prop->name.GetString(), prop->name.GetStringLength());
			if (propName == "volume")
			{
				float vol = -1.f;
				if (ParseVolumeValue(prop->value, vol))
				{
					entry.volume = vol;
				}
			}
			else if (prop->value.IsString() && prop->value.GetStringLength() > 0)
			{
				entry.tracks[propName] = std::string(prop->value.GetString(), prop->value.GetStringLength());
			}
		}
	}
}

const char *SoundtrackModeKey(int mode)
{
	switch (mode)
	{
	case TRAKINFO_MIDI:
		return "midi";
	case TRAKINFO_Remixed:
		return "remixed";
	default:
		return nullptr;
	}
}

} // namespace

CVAR(Bool, mus_has_trakinfo, false, CVAR_NOSET | CVAR_HIDDEN)

static void UpdateHasTrakInfoCVar()
{
	UCVarValue val;
	val.Bool = !TrakInfoMap.empty();
	mus_has_trakinfo->ForceSet(val, CVAR_Bool);
}

void S_ClearTrakInfo()
{
	TrakInfoMap.clear();
	UpdateHasTrakInfoCVar();
}

void S_ParseTrakInfo()
{
	S_ClearTrakInfo();

	const int numEntries = fileSystem.GetNumEntries();
	for (int lump = 0; lump < numEntries; ++lump)
	{
		if (fileSystem.GetFileNamespace(lump) == FileSys::ns_global &&
			fileSystem.CheckFileName(lump, "TRAKINFO"))
		{
			ParseTrakInfoLump(lump);
		}
	}

	UpdateHasTrakInfoCVar();
}

bool S_HasTrakInfo()
{
	return !TrakInfoMap.empty();
}

const char *S_TrakInfoFindReplacement(const void *data, size_t len, int mode)
{
	if (TrakInfoMap.empty() || data == nullptr || len == 0)
		return nullptr;

	const char *modeKey = SoundtrackModeKey(mode);
	if (modeKey == nullptr)
		return nullptr;

	std::string hash = ComputeSHA1Hex(data, len);
	auto it = TrakInfoMap.find(hash);
	if (it == TrakInfoMap.end())
		return nullptr;

	auto trackIt = it->second.tracks.find(modeKey);
	if (trackIt == it->second.tracks.end() || trackIt->second.empty())
		return nullptr;

	return trackIt->second.c_str();
}

float S_TrakInfoFindVolume(const void *data, size_t len)
{
	if (TrakInfoMap.empty() || data == nullptr || len == 0)
		return -1.f;

	std::string hash = ComputeSHA1Hex(data, len);
	auto it = TrakInfoMap.find(hash);
	if (it == TrakInfoMap.end())
		return -1.f;

	return it->second.volume;
}
