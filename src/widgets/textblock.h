/*
** textblock.h
**
** Read-only demi-markdown renderer
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

#include "zwidget/core/colorf.h"
#include "zwidget/window/window.h"
#include <zwidget/core/canvas.h>
#include <zwidget/core/font.h>
#include <zwidget/core/rect.h>
#include <zwidget/core/widget.h>
#include <zwidget/widgets/pushbutton/pushbutton.h>
#include <zwidget/widgets/tabwidget/tabwidget.h>
#include <zwidget/widgets/scrollbar/scrollbar.h>

#include <cstring>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

class TextBlock : public Widget
{
	struct Line {
		enum { TEXT, RULE } type;
		std::string base;
		std::shared_ptr<Font> font;
		struct { double above; double between; double under; } gap;
		bool bold;
		std::vector<std::string> split;
	};
	std::vector<std::unique_ptr<Line>> lines;
	Canvas *canvas = nullptr;
	Canvas *surface = nullptr;
	size_t lastwidth = 0;
	Scrollbar *scrollbar = nullptr;
	double scroll = 0;

public:
	TextBlock(Widget* parent): Widget(parent)
	{
		SetStyleClass("textedit");

		scrollbar = new Scrollbar(this);
		scrollbar->SetVertical();
	}

	void SetText(std::string text)
	{
		std::shared_ptr<Font> small = Font::Create("NotoSans", 12.0);
		std::shared_ptr<Font> med = Font::Create("NotoSans", 14.0);
		std::shared_ptr<Font> large = Font::Create("NotoSans", 18.0);

		auto ss = std::stringstream{text};

		for (std::string line; std::getline(ss, line, '\n');)
		{
			Line data {
				.type = Line::TEXT,
				.base = line,
				.font = small,
				.gap = {1,1,1},
				.bold = false
			};

			if (line == "---")
			{
				data.type = Line::RULE,
				data.gap = {4,0,4};
			}
			else if (line.starts_with("# "))
			{
				data.font = large;
				data.gap = {5, 1, 2};
				data.bold = true;
				data.base = line.substr(2);
			}
			else if (line.starts_with("## "))
			{
				data.font = med;
				data.gap = {2, 1, 2};
				data.bold = true;
				data.base = line.substr(3);
			}

			lines.push_back(std::make_unique<Line>(data));
		}
	}

	bool OnMouseWheel(const Point& pos, InputKey key)
	{
		int dir = (key == InputKey::MouseWheelDown)? 1: (key == InputKey::MouseWheelUp)? -1: 0;

		scrollbar->SetPosition(scrollbar->GetPosition() + dir * 16.0);
		return true;
	}

	void Reflow()
	{
		if (!this->canvas) return;

		auto width = std::max(1.0, GetWidth() - scrollbar->GetPreferredWidth());
		for (auto &line: lines)
		{
			auto space = canvas->measureText(line->font, " ").width;
			auto rect = canvas->measureText(line->font, line->base);
			line->split.clear();
			if (rect.width <= width)
			{
				line->split.push_back(line->base);
				continue;
			}

			struct Word { size_t count; double len; };
			std::vector<Word> words;

			size_t start = 0, end, len;
			do {
				end = line->base.find_first_of(' ', start);
				len = (end == std::string_view::npos)
					? line->base.size() - start
					: end - start;
				std::string_view sub = std::string_view(line->base).substr(start, len);
				words.emplace_back(len, canvas->measureText(line->font, sub).width);
				start = end + 1;
			} while (end != std::string_view::npos);

			start = end = 0;
			double size = 0;
			bool has_trailing_space = false;

			for (size_t i = 0; i < words.size(); i++)
			{
				if (end != 0)
				{
					auto t = size + space + words[i].len;
					if (t <= width)
					{
						end += words[i].count + 1;
						size = t;
						has_trailing_space = true;
					}
					else
					{
						size_t slice_len = has_trailing_space ? end - 1 : end;
						line->split.emplace_back(line->base.substr(start, slice_len));
						start += end;
						end = size = 0;
						has_trailing_space = false;
						i--; // re-evaluate word[i] on new line
					}
				}
				else
				{
					size = words[i].len;
					end += words[i].count;

					if (size <= width)
					{
						end++;
						has_trailing_space = true;
					}
					else
					{
						// single word exceeds width
						line->split.emplace_back(line->base.substr(start, end));
						start += end + 1;
						end = size = 0;
						has_trailing_space = false;
					}
				}
			}
			if (end != 0)
			{
				size_t slice_len = has_trailing_space ? end - 1 : end;
				line->split.emplace_back(line->base.substr(start, slice_len));
			}

			auto metrics = canvas->getFontMetrics(line->font);
		}

	}

	void OnPaint(Canvas* canvas)
	{
		size_t width = GetWidth();
		bool invalid = false;

		double outer = GetHeight(), inner = 0;

		if (this->canvas != canvas) { invalid = true; this->canvas = canvas; }
		if (lastwidth != width) { invalid = true; lastwidth = width; }
		if (invalid) Reflow();

		double x = 0, y = GetNoncontentTop() - scrollbar->GetPosition();
		enum { ABOVE, DURING, BELOW } step = ABOVE;
		for (auto i = 0u; i < lines.size(); i++)
		{
			if (lines[i]->type == Line::RULE)
			{
				y += lines[i]->gap.above;
				Colorf c = GetStyleColor("border-top-color");
				c.a = 0.5;
				canvas->line({0, y}, {GetWidth() - scrollbar->GetPreferredWidth(), y}, c);
				y += lines[i]->gap.under;
				continue;
			}

			auto metrics = canvas->getFontMetrics(lines[i]->font);

			bool first = true;
			for (auto &l: lines[i]->split)
			{
				y +=  metrics.ascent + (first? lines[i]->gap.above: lines[i]->gap.between);
				double p = y;
				y += metrics.descent;
				if (step == ABOVE && y >= 0) step = DURING;
				if (step == DURING)
				{
					if (lines[i]->bold)
					{
						canvas->drawText(lines[i]->font, Point(x-0.25, p), l, GetStyleColor("color"));
						canvas->drawText(lines[i]->font, Point(x+0.25, p), l, GetStyleColor("color"));
					}
					else
					{
						canvas->drawText(lines[i]->font, Point(x, p), l, GetStyleColor("color"));
					}
				}
			}
			y += lines[i]->gap.under;

			if (step == DURING && y >= GetHeight()) step = BELOW;
		}

		inner = y + scrollbar->GetPosition();

		x = width-scrollbar->GetPreferredWidth() + 4; // 4 is the blank space around bar (TODO: expose this)
		scrollbar->SetFrameGeometry({ x, 0., scrollbar->GetPreferredWidth(), outer });

		if (invalid) scrollbar->SetRanges(outer, inner);
	}
};
