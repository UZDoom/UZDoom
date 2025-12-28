/*
** errorwindow.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2024 Magnus Norddahl
** Copyright 2024-2025 GZDoom Maintainers and Contributors
** Copyright 2025 UZDoom Maintainers and Contributors
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <https://www.gnu.org/licenses/>.
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include <zwidget/core/widget.h>
#include <zwidget/core/span_layout.h>

class LogViewer;
class PushButton;
class Scrollbar;

class ErrorWindow : public Widget
{
public:
	static bool ExecModal(const std::string& text, const std::string& log, std::vector<uint8_t> minidump = {});

	ErrorWindow(std::vector<uint8_t> minidump);

	bool Restart = false;

protected:
	void OnClose() override;
	void OnGeometryChanged() override;

private:
	void SetText(const std::string& text, const std::string& log);

	void OnClipboardButtonClicked();
	void OnRestartButtonClicked();
	void OnSaveReportButtonClicked();

	LogViewer* LogView = nullptr;
	PushButton* ClipboardButton = nullptr;
	PushButton* RestartButton = nullptr;
	PushButton* SaveReportButton = nullptr;

	std::vector<uint8_t> minidump;
	std::string clipboardtext;
};

class LogViewer : public Widget
{
public:
	LogViewer(Widget* parent);

	void SetText(const std::string& text, const std::string& log);

protected:
	void OnPaintFrame(Canvas* canvas) override;
	void OnPaint(Canvas* canvas) override;
	bool OnMouseWheel(const Point& pos, InputKey key) override;
	void OnKeyDown(InputKey key) override;
	void OnGeometryChanged() override;

private:
	void OnScrollbarScroll();
	void ScrollUp(int lines);
	void ScrollDown(int lines);

	SpanLayout CreateLineLayout(const std::string& text);

	Scrollbar* scrollbar = nullptr;

	std::shared_ptr<Font> largefont = Font::Create("Poppins", 16.0);
	std::shared_ptr<Font> font = Font::Create("Poppins", 12.0);
	std::vector<SpanLayout> lines;
};
