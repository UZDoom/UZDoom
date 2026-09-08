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
	struct ColorNum {
		uint32_t n;
		consteval ColorNum() : n(0) {}
		consteval ColorNum(int num) { n = num; }
		consteval ColorNum(const char* s) { n = Color::str(s); }
	};
	struct Flag {
		FName flagid;
		struct ColorNum data[19]; // alternate {space, color}. if space is 0, that is the end of the flag.
	};
	struct Flag tints[] = { // sorted alphabetically
		{ "aceflux",      { 1, "#C62253", 1, "#C12678", 1, "#C0279A", 1, "#A928AC", 1, "#8C26AE", 0 }},
		{ "agender",      { 1, "#000000", 1, "#BCC4C7", 1, "#FFFFFF", 1, "#B7F684", 1, "#FFFFFF", 1, "#BCC4C7", 1, "#000000", 0 }},
		{ "aroace",       { 1, "#E28C00", 1, "#ECCD00", 1, "#FFFFFF", 1, "#62AEDC", 1, "#203856", 0 }},
		{ "aroflux",      { 1, "#E7516A", 1, "#D86D65", 1, "#B7A55D", 1, "#A3C95A", 1, "#92E454", 0 }},
		{ "aromantic",    { 1, "#3DA542", 1, "#A7D379", 1, "#FFFFFF", 1, "#A9A9A9", 1, "#000000", 0 }},
		{ "asexual",      { 1, "#000000", 1, "#A3A3A3", 1, "#FFFFFF", 1, "#800080", 0 }},
		{ "bear",         { 1, "#613704", 1, "#D46300", 1, "#FDDC62", 1, "#FDE5B7", 1, "#FFFFFF", 1, "#545454", 1, "#000000", 0 }},
		{ "bisexual",     { 2, "#D60270", 1, "#9B4F96", 2, "#0038A8", 0 }},
		{ "demisexual",   { 5, "#FFFFFF", 1, "#6E0070", 1, "#000000", 1, "#6E0070", 5, "#D2D2D2", 0 }},
		{ "diversity",    { 1, "#CD66FF", 1, "#FF6599", 1, "#FF0000", 1, "#FF9900", 1, "#FFFF01", 1, "#009A00", 1, "#0099CB", 1, "#330099", 1, "#990099", 0 }},
		{ "gay",          { 1, "#078D70", 1, "#26CEAA", 1, "#98E8C1", 1, "#FFFFFF", 1, "#7BADE2", 1, "#5049CC", 1, "#3D1A78", 0 }},
		{ "genderfluid",  { 1, "#FF76A4", 1, "#FFFFFF", 1, "#C011D7", 1, "#000000", 1, "#2F3CBE", 0 }},
		{ "genderqueer",  { 1, "#B57EDC", 1, "#FFFFFF", 1, "#4A8123", 0 }},
		{ "intersex",     { 5, "#FFD800", 1, "#7902AA", 5, "#FFD800", 0 }},
		{ "lesbian",      { 1, "#D52D00", 1, "#EF7627", 1, "#FF9A56", 1, "#FFFFFF", 1, "#D162A4", 1, "#B55690", 1, "#A30262", 0 }},
		{ "nonbinary",    { 1, "#FCF434", 1, "#FFFFFF", 1, "#9C59D1", 1, "#2C2C2C", 0 }},
		{ "omnisexual",   { 1, "#FE9ACE", 1, "#FF53BF", 1, "#200044", 1, "#6760FE", 1, "#8EA6FF", 0 }},
		{ "pansexual",    { 1, "#FF218C", 1, "#FFD800", 1, "#21B1FF", 0 }},
		{ "philadelphia", { 1, "#000000", 1, "#784F17", 1, "#D12229", 1, "#F68A1E", 1, "#FDE01A", 1, "#007940", 1, "#24408E", 1, "#732982", 0 }},
		{ "polysexual",   { 1, "#F714BA", 1, "#01D66A", 1, "#1594F6", 0 }},
		{ "queer",        { 1, "#000000", 1, "#99D9EA", 1, "#00A2E8", 1, "#B5E61D", 1, "#FFFFFF", 1, "#FFC90E", 1, "#FD6666", 1, "#FFAEC9", 1, "#000000", 0 }},
		{ "rainbow",      { 1, "#E40303", 1, "#FF8C00", 1, "#FFED00", 1, "#008026", 1, "#004CFF", 1, "#732982", 0 }},
		{ "transgender",  { 1, "#5BCEFA", 1, "#F5A9B8", 1, "#FFFFFF", 1, "#F5A9B8", 1, "#5BCEFA", 0 }},
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
		for (unsigned i = 0; tints[flag].data[i].n != 0; i+=2)
		{
			for (int j = tints[flag].data[i].n; j >= 0; j--)
			{
				colors.push_back(Color::rgb(tints[flag].data[i+1].n));
			}
		}
	}
	else
	{
		FString data = id.GetChars();
		data.ToLower();
		auto split = data.Split(",", FString::TOK_KEEPEMPTY);
		for (auto &str: split)
		{
			unsigned c = strtoul(str.GetChars(), nullptr, 16);
			if (c > 0xFFFFFF || str.Len() != 6 || str[1] == 'x') break;
			colors.push_back(Color::rgb(c));
		}
		if (split.size() != colors.size())
		{
			colors.clear();
			Printf("Unknown flag '%s'\n", data.GetChars());
		}
	}

	return colors;
}

LauncherBanner::LauncherBanner(Widget* parent, FName colors, float mix) : Widget(parent)
{
	bool useColors = colors != "";
	auto bg = Theme::getHeader(COLOR_BACKGROUND);
	if (useColors)
	{
		auto base = Color::rgb("#fff");
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
	Logo->SetImageAnchor(Theme::getAnchor());
	Logo->SetImageScale(Theme::getScale());
	this->SetStyleColor("background-color", bg);
}

double LauncherBanner::GetPreferredHeight()
{
	return Logo->GetPreferredHeight();
}

void LauncherBanner::OnGeometryChanged()
{
	auto W = GetWidth(), H = GetPreferredHeight();
	auto w = Logo->GetPreferredWidth();
	auto h = H/stripes.size();
	for (unsigned i = 0; i < stripes.size(); i++)
	{
		std::get<0>(stripes[i]).get()->SetFrameGeometry(0, floor(h*i), W, ceil(h));
		std::get<1>(stripes[i]).get()->SetFrameGeometry((W-w)/2, floor(h*i), w, ceil(h));
	}
	Logo->SetFrameGeometry(0, 0, W, H);
}
