/*
** launcherbanner.cpp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2024 Magnus Norddahl
** Copyright 2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#include <chrono>

#include <zwidget/core/image.h>
#include <zwidget/widgets/imagebox/imagebox.h>
#include <zwidget/widgets/textlabel/textlabel.h>

#include "basics.h"
#include "launcherbanner.h"
#include "name.h"
#include "printf.h"
#include "themedata.h"
#include "colorspace.h"
#include "zstring.h"

std::vector<Color::Color> getColors(FName id)
{
	using namespace std::chrono;
	struct Flag {
		FName flagid;
		uint32_t data[19]; // alternate {space, color}. if space is 0, that is the end of the flag.
	};
	struct Flag tints[] = { // sorted alphabetically
		{ "agender",      { 1, 0x000000, 1, 0xBCC4C7, 1, 0xFFFFFF, 1, 0xB7F684, 1, 0xFFFFFF, 1, 0xBCC4C7, 1, 0x000000, 0 }},
		{ "aroace",       { 1, 0xE28C00, 1, 0xECCD00, 1, 0xFFFFFF, 1, 0x62AEDC, 1, 0x203856, 0 }},
		{ "aroflux",      {1, 0xe7516a, 1, 0xd86d65, 1, 0xb7a55d, 1, 0xa3c95a, 1, 0x92e454, 0 }},
		{ "aromantic",    { 1, 0x3DA542, 1, 0xA7D379, 1, 0xFFFFFF, 1, 0xA9A9A9, 1, 0x000000, 0 }},
		{ "asexual",      { 1, 0x000000, 1, 0xA3A3A3, 1, 0xFFFFFF, 1, 0x800080, 0 }},
		{ "bear",         { 1, 0x613704, 1, 0xD46300, 1, 0xFDDC62, 1, 0xFDE5B7, 1, 0xFFFFFF, 1, 0x545454, 1, 0x000000, 0 }},
		{ "bisexual",     { 2, 0xD60270, 1, 0x9B4F96, 2, 0x0038A8, 0 }},
		{ "demisexual",   { 5, 0xFFFFFF, 1, 0x6E0070, 1, 0x000000, 1, 0x6E0070, 5, 0xD2D2D2, 0 }},
		{ "gay",          { 1, 0x078D70, 1, 0x26CEAA, 1, 0x98E8C1, 1, 0xFFFFFF, 1, 0x7BADE2, 1, 0x5049CC, 1, 0x3D1A78, 0 }},
		{ "genderfluid",  { 1, 0xFF76A4, 1, 0xFFFFFF, 1, 0xC011D7, 1, 0x000000, 1, 0x2F3CBE, 0 }},
		{ "genderqueer",  { 1, 0xB57EDC, 1, 0xFFFFFF, 1, 0x4A8123, 0 }},
		{ "intersex",     { 5, 0xFFD800, 1, 0x7902AA, 5, 0xFFD800, 0 }},
		{ "lesbian",      { 1, 0xD52D00, 1, 0xEF7627, 1, 0xFF9A56, 1, 0xFFFFFF, 1, 0xD162A4, 1, 0xB55690, 1, 0xA30262, 0 }},
		{ "nonbinary",    { 1, 0xFCF434, 1, 0xFFFFFF, 1, 0x9C59D1, 1, 0x2C2C2C, 0 }},
		{ "omnisexual",   { 1, 0xFE9ACE, 1, 0xFF53BF, 1, 0x200044, 1, 0x6760FE, 1, 0x8EA6FF, 0 }},
		{ "pansexual",    { 1, 0xFF218C, 1, 0xFFD800, 1, 0x21B1FF, 0 }},
		{ "philadelphia", { 1, 0x000000, 1, 0x784F17, 1, 0xD12229, 1, 0xF68A1E, 1, 0xFDE01A, 1, 0x007940, 1, 0x24408E, 1, 0x732982, 0 }},
		{ "polysexual",   { 1, 0xF714BA, 1, 0x01D66A, 1, 0x1594F6, 0 }},
		{ "queer",        { 1, 0x000000, 1, 0x99D9EA, 1, 0x00A2E8, 1, 0xB5E61D, 1, 0xFFFFFF, 1, 0xFFC90E, 1, 0xFD6666, 1, 0xFFAEC9, 1, 0x000000, 0 }},
		{ "rainbow",      { 1, 0xE40303, 1, 0xFF8C00, 1, 0xFFED00, 1, 0x008026, 1, 0x004CFF, 1, 0x732982, 0 }},
		{ "transgender",  { 1, 0x5BCEFA, 1, 0xF5A9B8, 1, 0xFFFFFF, 1, 0xF5A9B8, 1, 0x5BCEFA, 0 }},
	};
	std::vector<Color::Color> colors;

	size_t flag;
	if (id == "random" || id == "list")
	{
		auto now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
		flag = now % std::size(tints);

		if (id == "list")
		{
			FString avail = "Available flag colors: random";
			for (auto &f: tints)
			{
				FString n = f.flagid.GetChars();
				n.ToLower();
				avail.AppendFormat(", %s", n.GetChars());
			}
			FString n = tints[flag].flagid.GetChars();
			n.ToLower();
			avail.AppendFormat("\nSelecting: %s", n.GetChars());
			Printf("%s\nOpen an issue on our github if your flag is missing\n", avail.GetChars());
		}
	}
	else
	{
		for (flag = 0; flag < std::size(tints); flag++)
		{
			if (tints[flag].flagid == id) break;
		}
	}

	if (flag < std::size(tints))
	{
		for (unsigned i = 0; tints[flag].data[i] != 0; i+=2)
		{
			for (int j = tints[flag].data[i]; j >= 0; j--)
			{
				colors.push_back(Color::rgb(tints[flag].data[i+1]));
			}
		}
	}

	return colors;
}

LauncherBanner::LauncherBanner(Widget* parent, FName colors, float mix) : Widget(parent)
{
	bool useColors = colors != "";
	auto bg = Colorf(0.0f, 0.0f, 0.0f, 0.0f);
	if (useColors)
	{
		auto base = Color::rgb(0xFFFFFF);
		if (mix <= 0)
		{
			useColors = false;
			base = Color::rgb(bg.r, bg.g, bg.b);
			mix = -mix;
		}
		mix = clamp<float>(mix, 0, 1);
		for (auto &c: getColors(colors))
		{
			auto a = std::make_unique<Widget>(this);
			auto b = std::make_unique<Widget>(this);
			a->SetStyleColor("background-color", Colorf::fromRgb(Color::rgb24(c)));
			b->SetStyleColor("background-color", Colorf::fromRgb(Color::rgb24(Color::mix(base, c, mix))));
			stripes.push_back({std::move(a), std::move(b)});
		}
	}

	Logo = new ImageBox(this);
	auto imgsrc = (useColors || Theme::getMode() == LIGHT) ? "ui/banner-light.png": "ui/banner-dark.png";
	Logo->SetImage(Image::LoadResource(imgsrc));
	this->SetStyleColor("background-color", bg);
}

double LauncherBanner::GetPreferredHeight()
{
	return Logo->GetPreferredHeight();
}

void LauncherBanner::OnGeometryChanged()
{
	auto W = GetWidth(), H = Logo->GetPreferredHeight();
	auto w = Logo->GetPreferredWidth();
	auto h = H/stripes.size();
	for (unsigned i = 0; i < stripes.size(); i++)
	{
		std::get<0>(stripes[i]).get()->SetFrameGeometry(0, floor(h*i), W, ceil(h));
		std::get<1>(stripes[i]).get()->SetFrameGeometry((W-w)/2, floor(h*i), w, ceil(h));
	}
	Logo->SetFrameGeometry(0, 0, W, H);
}
