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
#include "d_main.h"
#include "filesystem.h"
#include "m_argv.h"
#include "printf.h"
#include "tarray.h"
#include "widgets/themedata.h"

CUSTOM_CVARD(Int, ui_theme, 2, CVAR_ARCHIVE | CVAR_GLOBALCONFIG, "launcher theme. 0: auto, 1: dark, 2: light")
{
	if (self < 0) self = 0;
	if (self > 2) self = 2;
}

TDeletingArray<FResourceFile*>* WidgetResources = nullptr;

bool IsZWidgetAvailable()
{
	return WidgetResources;
}

void InitWidgetResources(const char* filename)
{
	WidgetResources = new TDeletingArray<FResourceFile*>();

	auto open = [=](const char* filename, bool required = false)
	{
		auto file = FResourceFile::OpenResourceFile(filename);
		if (file)
			WidgetResources->Push(file);
		else if (required)
			I_FatalError("Unable to open %s", filename);
	};

	open(filename, true);

	FString *args;
	int argc = Args->CheckParmList(FArg_file, &args);
	for (int i = 0; i < argc; ++i)
		open(args[i].GetChars());

	bool use_dark = ui_theme == 1;

	if (ui_theme == 0)
	{
		// TODO: detect system theme
	}

	Theme::initilize(use_dark? DARK: LIGHT);

	WidgetTheme::SetTheme(std::unique_ptr<WidgetTheme>(new WidgetTheme{{
		Theme::getMain  (COLOR_BACKGROUND), Theme::getMain  (COLOR_TEXT),
		Theme::getHeader(COLOR_BACKGROUND), Theme::getHeader(COLOR_TEXT),
		Theme::getButton(COLOR_BACKGROUND), Theme::getButton(COLOR_TEXT),
		Theme::getHover (COLOR_BACKGROUND), Theme::getHover (COLOR_TEXT),
		Theme::getClick (COLOR_BACKGROUND), Theme::getClick (COLOR_TEXT),
		Theme::getBorder(COLOR_LIGHT),      Theme::getBorder(COLOR_HEAVY),
	}}));
}

void CloseWidgetResources()
{
	delete WidgetResources;
}

static std::vector<uint8_t> LoadFile(const char* name, bool root)
{
	if (!IsZWidgetAvailable())
		I_FatalError("InitWidgetResources has not been called");

	auto start = root ? 0: WidgetResources->size() - 1;
	for (auto i = start; i >= 0; i--)
	{
		auto lump = (*WidgetResources)[i]->FindEntry(name);
		if (lump == -1) continue;
		auto reader = (*WidgetResources)[i]->GetEntryReader(lump, FileSys::READER_SHARED);
		std::vector<uint8_t> buffer(reader.GetLength());
		reader.Read(buffer.data(), buffer.size());
		return buffer;
	}

	I_FatalError("Unable to find %s", name);
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
std::vector<SingleFontData> LoadWidgetFontData(const std::string& name, bool root)
{
	std::vector<SingleFontData> returnv;
	if (!stricmp(name.c_str(), "notosans"))
	{
		// to update/add fonts:
		// tools/download-fonts.sh wadsrc/static ui/noto 'Noto Sans' 'Noto Sans Armenian' 'Noto Sans Georgian' 'Noto Sans JP' 'Noto Sans KR'
		const char* fonts[] = {
			"ui/noto/noto-sans.ttf",
			"ui/noto/noto-sans-armenian.ttf",
			"ui/noto/noto-sans-georgian.ttf",
			"ui/noto/noto-sans-jp.ttf",
			"ui/noto/noto-sans-kr.ttf"
		};

		auto count = sizeof(fonts) / sizeof(fonts[0]);
		returnv.resize(count);
		for (unsigned i = 0; i < count; i++)
			returnv[i].fontdata = LoadFile(fonts[i], root);

		return returnv;
	}

	returnv.resize(1);
	std::string fn = "ui/font/" + name + ".ttf";
	returnv[0].fontdata = LoadFile(fn.c_str(), root);

	return returnv;
}

std::vector<uint8_t> LoadWidgetData(const std::string& name, bool root)
{
	return LoadFile(name.c_str(), root);
}
