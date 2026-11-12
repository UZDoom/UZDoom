/*
** versioninfo.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include "basics.h"

#include <string>

class FString;

struct VersionInfo
{
	uint16_t major;
	uint16_t minor;
	uint32_t revision;
	char prerelease[16];
	char build[16];

	constexpr VersionInfo() = default;
	constexpr VersionInfo(
		uint16_t _major,
		uint16_t _minor,
		uint32_t _revision = 0,
		const char *_prerelease = "",
		const char *_build = ""
	): major(_major), minor(_minor), revision(_revision), prerelease{}, build{}
	{
		for (size_t i = 0; i < sizeof(prerelease)-1 && _prerelease && _prerelease[i]; i++) prerelease[i] = _prerelease[i];
		for (size_t i = 0; i < sizeof(build)-1 && _build && _build[i]; i++) build[i] = _build[i];
	}
	explicit VersionInfo(const char *);

	bool operator <=(const VersionInfo& o) const
	{
		return operator<=>(o) != std::strong_ordering::greater;
	}
	bool operator >=(const VersionInfo& o) const
	{
		return operator<=>(o) != std::strong_ordering::less;
	}
	bool operator > (const VersionInfo& o) const
	{
		return operator<=>(o) == std::strong_ordering::greater;
	}
	bool operator < (const VersionInfo& o) const
	{
		return operator<=>(o) == std::strong_ordering::less;
	}
	bool operator == (const VersionInfo& o) const
	{
		return operator<=>(o) == std::strong_ordering::equal;
	}
	bool operator != (const VersionInfo& o) const
	{
		return operator<=>(o) != std::strong_ordering::equal;
	}

	std::strong_ordering operator <=> (const VersionInfo& o) const;

	void operator=(const char* string);
	explicit operator FString() const;
	explicit operator std::string() const;
};

// Cannot be a constructor because Lemon would puke on it.
constexpr VersionInfo MakeVersion(unsigned int ma, unsigned int mi, unsigned int re = 0)
{
	return{ (uint16_t)ma, (uint16_t)mi, (uint32_t)re };
}

VersionInfo GetCurrentVersion();
VersionInfo GetCurrentVersionForUpdater();

VersionInfo GetCurrentEngineVersion();
