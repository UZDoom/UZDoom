/*
** about.cpp
**
** Create the UI to display the Credits and Patch Notes
**
**---------------------------------------------------------------------------
**
** Copyright 2025 Marcus Minhorst for _ParseReleaseNotes() (adapted), _BuildReleaseNotes(), _OpenReleaseNotes(),
** GetReleaseNotes() and adapted GetAboutText()
**
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#include "about.h"
#include "filesystem.h"
#include "findfile.h"
#include "gameconfigfile.h"
#include "gstrings.h"
#include "i_interface.h"
#include "name.h"
#include "version.h"
#include "zstring.h"

#include "imgui.h"

#include <filesystem>
#include <fstream>
#include <rapidxml/rapidxml.hpp>
#include <string>

constexpr unsigned NUMBER_OF_RELEASES_TO_DISPLAY = 3;

// Helper sanitize the markdown
FString _CleanMarkdown(FString input)
{
	std::string text = input.GetChars();
	return FString(text.c_str());
}

FString _ParseReleaseNotes(rapidxml::xml_node<char> *release)
{
	if (!release)
		return GStrings.GetString("NOTES_FAIL");

	auto description = release->first_node("description");
	auto version     = release->first_attribute("version");
	auto date        = release->first_attribute("date");
	auto url         = release->first_node("url");

	std::string descText;

	// Traverse XML and build plain-text instead of HTML
	std::function<void(rapidxml::xml_node<char> *)> reconstructText = [&](rapidxml::xml_node<char> *node) {
		for (auto child = node->first_node(); child; child = child->next_sibling())
		{
			if (child->type() == rapidxml::node_data || child->type() == rapidxml::node_cdata)
			{
				descText += child->value();
			}
			else if (child->type() == rapidxml::node_element)
			{
				std::string tagName = child->name();

				if (tagName == "li")
					descText += "  * "; // Bullet points
				if (tagName == "p" || tagName == "br")
					descText += "\n";

				reconstructText(child); // Recursively process children

				if (tagName == "p" || tagName == "ul" || tagName == "h1" || tagName == "h2" || tagName == "h3" ||
				    tagName == "li")
					descText += "\n";
			}
		}
	};

	if (description)
	{
		reconstructText(description);
	}
	else
	{
		descText = GStrings.GetString("NOTES_EMPTY");
	}

	std::string versionStr  = version ? version->value() : GStrings.GetString("NOTES_UNKNOWN");
	std::string dateStr     = date ? date->value() : GStrings.GetString("NOTES_UNKNOWN");
	std::string releasedStr = GStrings.GetString("NOTES_RELEASED");

	// Build plain-text output
	std::string finalOutput;
	finalOutput += "=== " + std::string(GAMENAME) + " " + versionStr + " (" + releasedStr + " " + dateStr + ") ===\n\n";
	finalOutput += descText;

	if (url)
	{
		std::string detailsStr = GStrings.GetString("NOTES_DETAILS");
		finalOutput += "\n" + detailsStr + " " + url->value() + "\n";
	}

	return FString(finalOutput.c_str());
}

FString _BuildReleaseNotes(rapidxml::xml_document<> &doc)
{
	auto release = doc.first_node("component");
	if (!release)
		return GStrings.GetString("NOTES_FAIL");
	release = release->first_node("releases");
	if (!release)
		return GStrings.GetString("NOTES_FAIL");
	release = release->first_node("release");

	FString text;

	for (unsigned i = 1;; i++)
	{
		text.AppendFormat("%s", _ParseReleaseNotes(release).GetChars());
		if (!release || i >= NUMBER_OF_RELEASES_TO_DISPLAY)
			break;

		text.AppendFormat("\n\n----------------------------------------\n\n");
		release = release->next_sibling("release");
	}

	return text;
}

char *_OpenReleaseNotes()
{
	auto wad = BaseFileSearch(BASEWAD, NULL, true, GameConfig);
	if (!wad)
		return nullptr;

	auto resf = FResourceFile::OpenResourceFile(wad);
	if (!resf)
		return nullptr;

	char *notes = nullptr;
	auto  lump  = resf->FindEntry("meta.xml");

	if (lump >= 0)
	{
		auto data = resf->Read(lump);
		notes     = (char *)calloc(data.size() + 1, sizeof(char));
		if (notes)
			strncpy(notes, data.string(), data.size());
	}
	delete resf;
	return notes;
}

FString GetReleaseNotes()
{
	char                    *text = _OpenReleaseNotes();
	rapidxml::xml_document<> doc;
	if (text)
		doc.parse<rapidxml::parse_default>(text);
	FString content = _BuildReleaseNotes(doc);
	free(text);
	return content;
}

FString GetAboutText()
{
	auto wad = BaseFileSearch(BASEWAD, NULL, true, GameConfig);
	if (wad)
	{
		auto    resf = FResourceFile::OpenResourceFile(wad);
		FString text;

		auto append = [&resf, &text](const char *name) {
			auto lump = resf->FindEntry(name);
			if (lump < 0)
				return;
			auto data = resf->Read(lump);
			text.AppendCStrPart(data.string(), data.size());
		};

		if (resf)
		{
			append("about.txt");
			text.AppendCharacter('\n');
			append("contributors.txt");
			text.StripLeftRight();
		}
		delete resf;
		return _CleanMarkdown(text);
	}
	return FString();
}

void About::DrawReleaseNotesDialog(bool *p_open, const std::string &lang)
{
	const char *popupId  = "###ReleaseNotesModal";
	static bool was_open = false;
	if (*p_open && !was_open)
	{
		ImGui::OpenPopup(popupId);
	}
	was_open = *p_open;

	if (!*p_open && !ImGui::IsPopupOpen(popupId))
		return;

	GStrings.UpdateLanguage(lang.c_str());

	ImGui::SetNextWindowSize(ImVec2(ImGui::GetFontSize() * 25.0f, ImGui::GetFontSize() * 20.0f),
	                         ImGuiCond_FirstUseEver);

	// ImGuiWindowFlags_NoCollapse prevents the window from minimizing
	std::string title = std::string(GStrings.GetString("LAUNCHER_TOPBAR_ABOUTNOTES")) + popupId;
	if (ImGui::BeginPopupModal(title.c_str(), p_open, ImGuiWindowFlags_NoCollapse))
	{
		// Static caches the parsed file so we aren't reading the filesystem 60 frames a second
		static std::string patchNotes      = GetReleaseNotes().GetChars();
		static bool        showOnUpdateReq = true;

		// Reserve space for the checkbox
		float reservedBottomSpace = ImGui::GetFrameHeightWithSpacing() + 10.0f;

		// scrollable child region for the text
		ImGui::BeginChild("NotesScrollRegion", ImVec2(0, -reservedBottomSpace), true);
		ImGui::TextWrapped("%s", patchNotes.c_str());
		ImGui::EndChild();

		// Center and draw the checkbox
		ImGui::Spacing();
		const char *checkboxLabel = GStrings.GetString("LAUNCHER_SHOW_ONUPDATED");
 		float checkboxWidth =
			ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.x + ImGui::CalcTextSize(checkboxLabel).x;

		ImGui::SetCursorPosX((ImGui::GetWindowSize().x - checkboxWidth) * 0.5f);
		ImGui::Checkbox(checkboxLabel, &showOnUpdateReq);

		ImGui::EndPopup();
	}
}

void About::DrawCreditsDialog(bool *p_open, const std::string &lang)
{
	const char *popupId  = "###CreditsModal";
	static bool was_open = false;
	if (*p_open && !was_open)
	{
		ImGui::OpenPopup(popupId);
	}
	was_open = *p_open;

	if (!*p_open && !ImGui::IsPopupOpen(popupId))
		return;

	GStrings.UpdateLanguage(lang.c_str());

	ImGui::SetNextWindowSize(ImVec2(ImGui::GetFontSize() * 25.0f, ImGui::GetFontSize() * 20.0f),
	                         ImGuiCond_FirstUseEver);

	std::string title = std::string(GStrings.GetString("LAUNCHER_TOPBAR_ABOUTCREDITS")) + popupId;
	if (ImGui::BeginPopupModal(title.c_str(), p_open, ImGuiWindowFlags_NoCollapse))
	{
		// Again, Static cache
		static std::string creditsText = GetAboutText().GetChars();

		// another scrollable child region for the text
		ImGui::BeginChild("CreditsScrollRegion", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true);
		ImGui::TextWrapped("%s", creditsText.c_str());
		ImGui::EndChild();

		ImGui::EndPopup();
	}
}
