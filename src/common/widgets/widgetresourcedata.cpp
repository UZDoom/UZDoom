/*
** widgetresourcedata.cpp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2024 Magnus Norddahl
** Copyright 2024-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#include <zwidget/core/resourcedata.h>
#include <zwidget/core/theme.h>

#include "c_cvars.h"
#include "filesystem.h"
#include "printf.h"

CUSTOM_CVARD(Int, ui_theme, 2, CVAR_ARCHIVE | CVAR_GLOBALCONFIG, "launcher theme. 0: auto, 1: dark, 2: light")
{
	if (self < 0) self = 0;
	if (self > 2) self = 2;
}

class ResourceLoaderPK3 : public ResourceLoader
{
private:
	FResourceFile* WidgetResources;

public:
	ResourceLoaderPK3(const char* filename)
	{
		WidgetResources = FResourceFile::OpenResourceFile(filename);
		if (!WidgetResources)
			I_FatalError("Unable to open %s", filename);

		bool use_dark = ui_theme != 2;

		if (ui_theme == 0)
		{
			// TODO: detect system theme
		}

		if (use_dark) // light
		{
			// TODO: make a nice theme
			WidgetTheme::SetTheme(std::make_unique<DarkWidgetTheme>());
		}
		else
		{
			WidgetTheme::SetTheme(std::unique_ptr<WidgetTheme>(new SimpleTheme{{
				Colorf::fromRgb(0xeee8d5), // background
				Colorf::fromRgb(0x000000), // text
				Colorf::fromRgb(0xfdf6e3), // headers / inputs
				Colorf::fromRgb(0x000000), // headers / inputs text
				Colorf::fromRgb(0xd7d2bf), // interactive elements
				Colorf::fromRgb(0x000000), // interactive elements text
				Colorf::fromRgb(0xa4c2e9), // hover / highlight
				Colorf::fromRgb(0x000000), // hover / highlight text
				Colorf::fromRgb(0x7ca2e9), // click
				Colorf::fromRgb(0x000000), // click text
				Colorf::fromRgb(0x586e75), // around elements
				Colorf::fromRgb(0xbdb8a7)  // between elements
			}}));
		}
	}

	~ResourceLoaderPK3()
	{
		delete WidgetResources;
	}

	std::vector<SingleFontData> LoadFont(const std::string& name) override
	{
		std::vector<SingleFontData> returnv;
		if (!stricmp(name.c_str(), "notosans") || !stricmp(name.c_str(), "system") || !stricmp(name.c_str(), "monospace")) // TODO: add monospace fonts
		{
			// to update/add fonts:
			// tools/download-fonts.sh wadsrc/static widgets/noto 'Noto Sans' 'Noto Sans Armenian' 'Noto Sans Georgian' 'Noto Sans JP' 'Noto Sans KR'
			const char* fonts[] = {
				"widgets/noto/noto-sans.ttf",
				"widgets/noto/noto-sans-armenian.ttf",
				"widgets/noto/noto-sans-georgian.ttf",
				"widgets/noto/noto-sans-jp.ttf",
				"widgets/noto/noto-sans-kr.ttf"
			};

			auto count = sizeof(fonts) / sizeof(fonts[0]);
			returnv.resize(count);
			for (unsigned i = 0; i < count; i++)
				returnv[i].fontdata = ReadAllBytes(fonts[i]);

			return returnv;
		}

		returnv.resize(1);
		std::string fn = "widgets/font/" + name + ".ttf";
		returnv[0].fontdata = ReadAllBytes(fn.c_str());

		return returnv;
	}

	std::vector<uint8_t> ReadAllBytes(const std::string& name) override
	{
		if (!WidgetResources)
			I_FatalError("InitWidgetResources has not been called");

		auto lump = WidgetResources->FindEntry(name.c_str());
		if (lump == -1)
			I_FatalError("Unable to find %s", name.c_str());

		auto reader = WidgetResources->GetEntryReader(lump, FileSys::READER_SHARED);
		std::vector<uint8_t> buffer(reader.GetLength());
		reader.Read(buffer.data(), buffer.size());
		return buffer;
	}
};

void InitWidgetResources(const char* filename)
{
	ResourceLoader::Set(std::make_unique<ResourceLoaderPK3>(filename));
}

void CloseWidgetResources()
{
	ResourceLoader::Set(nullptr);
}

// this must be allowed to fail without throwing.
static std::vector<uint8_t> LoadDiskFile(const char* name)
{
	std::vector<uint8_t> buffer;
	FileSys::FileReader lump;
	if (lump.OpenFile(name))
	{
		buffer.resize(lump.GetLength());
		lump.Read(buffer.data(), buffer.size());
	}
	return buffer;
}
