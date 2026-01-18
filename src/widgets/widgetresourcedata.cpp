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
#include "widgets/themedata.h"

CUSTOM_CVARD(Int, ui_theme, 2, CVAR_ARCHIVE | CVAR_GLOBALCONFIG, "launcher theme. 0: auto, 1: dark, 2: light")
{
	if (self < 0) self = 0;
	if (self > 2) self = 2;
}

FResourceFile* WidgetResources;

bool IsZWidgetAvailable()
{
	return WidgetResources;
}

void InitWidgetResources(const char* filename)
{
	WidgetResources = FResourceFile::OpenResourceFile(filename);
	if (!WidgetResources)
		I_FatalError("Unable to open %s", filename);

	bool use_dark = ui_theme == 1;

	if (ui_theme == 0)
	{
		// TODO: detect system theme
	}

	Theme::initilize(use_dark? DARK: LIGHT);

	WidgetTheme::SetTheme(std::unique_ptr<WidgetTheme>(new WidgetTheme{{
		Theme::getMain(0),   Theme::getMain(1),
		Theme::getHeader(0), Theme::getHeader(1),
		Theme::getButton(0), Theme::getButton(1),
		Theme::getHover(0),  Theme::getHover(1),
		Theme::getClick(0),  Theme::getClick(1),
		Theme::getBorder(0), Theme::getBorder(1),
	}}));
}

void CloseWidgetResources()
{
	delete WidgetResources;
	WidgetResources = nullptr;
}

static std::vector<uint8_t> LoadFile(const char* name)
{
	if (!WidgetResources)
		I_FatalError("InitWidgetResources has not been called");

	auto lump = WidgetResources->FindEntry(name);
	if (lump == -1)
		I_FatalError("Unable to find %s", name);

	auto reader = WidgetResources->GetEntryReader(lump, FileSys::READER_SHARED);
	std::vector<uint8_t> buffer(reader.GetLength());
	reader.Read(buffer.data(), buffer.size());
	return buffer;
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

// This interface will later require some significant redesign.
std::vector<SingleFontData> LoadWidgetFontData(const std::string& name)
{
	std::vector<SingleFontData> returnv;
	if (!stricmp(name.c_str(), "notosans"))
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
			returnv[i].fontdata = LoadFile(fonts[i]);

		return returnv;
	}

	returnv.resize(1);
	std::string fn = "widgets/font/" + name + ".ttf";
	returnv[0].fontdata = LoadFile(fn.c_str());

	return returnv;
}

std::vector<uint8_t> LoadWidgetData(const std::string& name)
{
	return LoadFile(name.c_str());
}
