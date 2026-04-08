/*
** errorwindow.cpp
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

#include "errorwindow.h"
#include "gstrings.h"
#include "printf.h"

#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <miniz.h>
#include <sstream>

#include "starter.h"

// Defined in UZDoom headers
#define TEXTCOLOR_ESCAPE '\x1c'

std::string ParseAndCleanLog(const std::string &log, const std::string &errorText)
{
	std::string processedLog;
	processedLog.reserve(log.size());

	for (size_t i = 0; i < log.size(); ++i)
	{
		unsigned char chr = log[i];
		if (chr == TEXTCOLOR_ESCAPE)
		{
			i++; // Skip the 'A', 'B', etc. defining the color
			continue;
		}

		// Replace control range with Box Drawings Double Horizontal
		if (chr >= 0x1D && chr <= 0x1F)
		{
			processedLog += "\xE2\x95\x90";
		}
		else
		{
			processedLog += chr;
		}
	}

	return processedLog;
}

void SaveReportToDisk(const std::vector<uint8_t> &minidump, const std::string &cleanClipboardText)
{
	// Instead of a file dialog, we simply save directly to the execution directory (thats the easierst way)
	std::time_t        t       = std::time(nullptr);
	std::tm            tm_info = *std::localtime(&t);
	std::ostringstream oss;
	oss << "UZDoomCrashReport_" << std::put_time(&tm_info, "%Y-%m-%d_%H-%M-%S") << ".zip";

	std::string filename = oss.str();

	mz_zip_archive zip = {};
	if (mz_zip_writer_init_heap(&zip, 0, 16 * 1024 * 1024))
	{
		// Add Minidump
		mz_zip_writer_add_mem(&zip, "minidump.dmp", minidump.data(), minidump.size(), MZ_DEFAULT_COMPRESSION);

		// Add Log text
		mz_zip_writer_add_mem(&zip, "log.txt", cleanClipboardText.data(), cleanClipboardText.size(),
		                      MZ_DEFAULT_COMPRESSION);
	}

	// Finalize Zip
	void  *buffer     = nullptr;
	size_t buffersize = 0;
	mz_zip_writer_finalize_heap_archive(&zip, &buffer, &buffersize);
	mz_zip_writer_end(&zip);

	// Write to disk
	std::ofstream file(filename, std::ios::binary);
	if (file.is_open())
	{
		file.write(static_cast<const char *>(buffer), buffersize);
		file.close();
	}

	// Free the buffer allocated by miniz
	mz_free(buffer);
}

bool ErrorWindow::ExecModal(const std::string &text, const std::string &log, std::vector<uint8_t> minidump)
{
	std::string                title   = GStrings.GetString("CRASHREPORT_TITLE");
	Starter::ImGuiContextState context = Starter::SetupContext(title.c_str(), 1200, 700, SDL_INIT_VIDEO);

	if (!context.window)
		return false;

	ImGuiIO    &io                 = ImGui::GetIO();
	std::string cleanClipboardText = ParseAndCleanLog(log, text);
	std::string fullClipboardText  = cleanClipboardText + "\n\nExecution could not continue.\n" + text + "\n";

	Starter::RunImGuiLoop(context, [&](bool &done) {
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));

		ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));

		ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
		                         ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
		                         ImGuiWindowFlags_NoScrollWithMouse;

		ImGui::Begin("[X] Fatal Error - Execution aborted!", nullptr, flags);

		ImGuiStyle &style = ImGui::GetStyle();

		ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

		ImGui::InputTextMultiline(
			"##Log", const_cast<char *>(fullClipboardText.c_str()), fullClipboardText.size() + 1,
		                          ImVec2(-1.0f, -ImGui::GetFrameHeightWithSpacing() - style.WindowPadding.y),
		                          ImGuiInputTextFlags_ReadOnly);

		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
		ImGui::PopFont();

		ImGui::Spacing();

		float windowWidth = ImGui::GetWindowWidth();

		// Buttons at the bottom
		const char *copyBtnTxt = GStrings.GetString("CRASHREPORT_COPYCLIP");
		std::string saveBtnTxtStr =
			minidump.empty() ? GStrings.GetString("CRASHREPORT_NOSAVE") : GStrings.GetString("CRASHREPORT_SAVE");
		const char *saveBtnTxt = saveBtnTxtStr.c_str();
		const char *quitBtnTxt = GStrings.GetString("CRASHREPORT_QUIT");
		float       copyBtnWidth = ImGui::CalcTextSize(copyBtnTxt).x + (style.FramePadding.x * 2.0f);
		float       saveBtnWidth = ImGui::CalcTextSize(saveBtnTxt).x + (style.FramePadding.x * 2.0f);
		float       quitBtnWidth = ImGui::CalcTextSize(quitBtnTxt).x + (style.FramePadding.x * 2.0f);

		// calc distance between the 3 buttons
		float contentWidth   = ImGui::GetWindowWidth() - (style.WindowPadding.x * 2.0f);
		float availableSpace = contentWidth - (copyBtnWidth + saveBtnWidth + quitBtnWidth);
		float gap            = availableSpace / 2.0f;


		if (ImGui::Button(copyBtnTxt))
		{
			ImGui::SetClipboardText(fullClipboardText.c_str());
		}

		ImGui::SameLine(0.0f, gap);

		ImGui::BeginDisabled(minidump.empty());
		if (ImGui::Button(saveBtnTxt))
		{
			SaveReportToDisk(minidump, fullClipboardText);
		}
		ImGui::EndDisabled();

		ImGui::SameLine(0.0f, gap);
		if (ImGui::Button(quitBtnTxt))
		{
			done = true; // Setting this exits the loop gracefully
		}

		ImGui::End();

		ImGui::PopStyleColor(3);
	});

	Starter::TeardownContext(context);

	return false;
}
