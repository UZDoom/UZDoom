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
#include "system_theme.h"
#include "tarray.h"
#include "widgets/themedata.h"

CUSTOM_CVARD(Int, ui_theme, 0, CVAR_ARCHIVE | CVAR_GLOBALCONFIG, "launcher theme. 0: auto, 1: dark, 2: light, 3: high-contrast dark, 4: high-contrast light")
{
	if (self < 0) self = 0;
	if (self > 4) self = 4;
}

// This is the cvar exposed to the launcher to ensure that high-contrast themes don't break.
CUSTOM_CVAR(Int, ui_preferred_theme, 0, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
{
	if (self < 0)
		self = 0;
	else if (self > 2)
		self = 2;
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

	auto theme = Dark;
	switch(ui_theme)
	{
	case 0:
		theme = GetSystemTheme(); // gdbus takes too long and/or hits timeout on some systems. Don't call if not needed
		break;
	case 1:
		theme = Dark;
		break;
	case 2:
		theme = Light;
		break;
	case 3:
		theme = HighContrastDark;
		break;
	case 4:
		theme = HighContrastLight;
		break;
	}		
	auto use_dark = ui_theme == 1 || ui_theme == 3 || (ui_theme == 0 && (theme & Dark));
	if (ui_theme == 0 && ui_preferred_theme > 0)
	{
		if (ui_preferred_theme == 1)
			use_dark = true;
		else if (ui_preferred_theme == 2)
			use_dark = false;
	}

	Theme::initilize(use_dark? DARK: LIGHT, theme & HighContrast);

	WidgetTheme::SetTheme(std::unique_ptr<WidgetTheme>(new WidgetTheme{{
		Theme::getMain  (COLOR_BACKGROUND), Theme::getMain  (COLOR_TEXT),
		Theme::getHeader(COLOR_BACKGROUND), Theme::getHeader(COLOR_TEXT),
		Theme::getButton(COLOR_BACKGROUND), Theme::getButton(COLOR_TEXT),
		Theme::getHover (COLOR_BACKGROUND), Theme::getHover (COLOR_TEXT),
		Theme::getClick (COLOR_BACKGROUND), Theme::getClick (COLOR_TEXT),
		Theme::getBorder(COLOR_LIGHT),      Theme::getBorder(COLOR_HEAVY),
	}}));

	auto themePtr = WidgetTheme::GetTheme();
	auto cardBg = use_dark ? Colorf::fromRgb(0x2C2C2E) : Colorf::fromRgb(0xFFFFFF);
	auto cardBorder = use_dark ? Colorf::fromRgb(0x3A3A3C) : Colorf::fromRgb(0xE5E5EA);
	auto inputBorder = use_dark ? Colorf::fromRgb(0x48484A) : Colorf::fromRgb(0xD1D1D6);
	auto mutedText = use_dark ? Colorf::fromRgb(0x98989D) : Colorf::fromRgb(0x6E6E73);
	auto accent = Theme::getAccent();
	auto none = Colorf::transparent();

	for (const char* cls : { "listview", "lineedit", "textedit", "dropdown" })
	{
		if (auto* s = themePtr->GetStyle(cls))
		{
			s->SetColor("background-color", cardBg);
			s->SetColor("border-left-color", inputBorder);
			s->SetColor("border-top-color", inputBorder);
			s->SetColor("border-right-color", inputBorder);
			s->SetColor("border-bottom-color", inputBorder);
		}
	}

	if (auto* lv = themePtr->GetStyle("listview"))
	{
		lv->SetColor("border-left-color", cardBorder);
		lv->SetColor("border-top-color", cardBorder);
		lv->SetColor("border-right-color", cardBorder);
		lv->SetColor("border-bottom-color", cardBorder);
	}

	if (auto* le = themePtr->GetStyle("lineedit"))
	{
		le->SetDouble("noncontent-left", 10.0);
		le->SetDouble("noncontent-top", 6.0);
		le->SetDouble("noncontent-right", 10.0);
		le->SetDouble("noncontent-bottom", 6.0);
	}

	if (auto* tb = themePtr->GetStyle("tabbar"))
	{
		tb->SetDouble("spacer-left", 20.0);
		tb->SetColor("background-color", Theme::getMain(COLOR_BACKGROUND));
	}

	if (auto* ts = themePtr->GetStyle("tabbar-spacer"))
	{
		ts->SetDouble("noncontent-bottom", 0.0);
		ts->SetColor("border-bottom-color", none);
	}

	if (auto* tab = themePtr->GetStyle("tabbar-tab"))
	{
		tab->SetDouble("border-radius", 8.0);
		tab->SetDouble("noncontent-left", 14.0);
		tab->SetDouble("noncontent-right", 14.0);
		tab->SetDouble("noncontent-top", 4.0);
		tab->SetDouble("noncontent-bottom", 4.0);
		tab->SetColor("color", mutedText);
		tab->SetColor("background-color", none);
		tab->SetColor("border-left-color", none);
		tab->SetColor("border-top-color", none);
		tab->SetColor("border-right-color", none);
		tab->SetColor("border-bottom-color", none);
		tab->SetColor("hover", "color", Theme::getMain(COLOR_TEXT));
		tab->SetColor("hover", "background-color", use_dark ? Colorf::fromRgb(0x2C2C2E) : Colorf::fromRgb(0xEBEBF0));
		tab->SetColor("active", "color", Theme::getMain(COLOR_TEXT));
		tab->SetColor("active", "background-color", cardBg);
		tab->SetColor("active", "border-left-color", cardBorder);
		tab->SetColor("active", "border-top-color", cardBorder);
		tab->SetColor("active", "border-right-color", cardBorder);
		tab->SetColor("active", "border-bottom-color", cardBorder);
		tab->SetColor("active", "accent-color", accent);
	}

	if (auto* cb = themePtr->GetStyle("checkbox-label"))
	{
		cb->SetColor("checked-color", accent);
		cb->SetColor("checked-outer-border-color", accent);
		cb->SetColor("checked-inner-border-color", Colorf::fromRgb(0xFFFFFF));
		cb->SetColor("unchecked-outer-border-color", inputBorder);
		cb->SetColor("unchecked-inner-border-color", cardBg);
	}

	if (auto* sb = themePtr->GetStyle("scrollbar"))
	{
		sb->SetColor("track-color", none);
		sb->SetColor("thumb-color", use_dark ? Colorf::fromRgb(0x636366) : Colorf::fromRgb(0xAEAEB2));
	}
}

void CloseWidgetResources()
{
	delete WidgetResources;
}

static std::vector<uint8_t> LoadFile(const char* name, bool root)
{
	if (!IsZWidgetAvailable())
		I_FatalError("InitWidgetResources has not been called");

	int64_t start = root ? 0: WidgetResources->size() - 1;
	for (int64_t i = start; i >= 0; i--)
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
		// tools/download-fonts.sh wadsrc/static ui/noto 'Noto Sans' 'Noto Sans Armenian' 'Noto Sans Georgian' 'Noto Sans JP' 'Noto Sans KR' 'Noto Sans SC' 'Noto Sans Tamil' # 'Noto Sans TC'
		struct { const char *file; const char *lang; } fonts[] = {
			// fonts with specific languages list here for high priority
			{ "ui/noto/noto-sans-jp.ttf", "ja-*-*" },
			{ "ui/noto/noto-sans-kr.ttf", "ko-*-*" },
			{ "ui/noto/noto-sans-sc.ttf", "zh-Hans-*" },
			// "ui/noto/noto-sans-tc.ttf", "zh-Hant-*" },

			// generic fonts
			{ "ui/noto/noto-sans.ttf", ""},
			{ "ui/noto/noto-sans-armenian.ttf", ""},
			{ "ui/noto/noto-sans-georgian.ttf", ""},
			{ "ui/noto/noto-sans-tamil.ttf", ""},
		};

		auto count = sizeof(fonts) / sizeof(fonts[0]);
		returnv.resize(count);
		for (unsigned i = 0; i < count; i++)
		{
			returnv[i].fontdata = LoadFile(fonts[i].file, root);
			returnv[i].language = fonts[i].lang;
		}
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
