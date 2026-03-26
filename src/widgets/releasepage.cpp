/*
** releasepage.cpp
**
** Release notes tab of launcher
**
**---------------------------------------------------------------------------
**
** Copyright 2025 Marcus Minhorst
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#include <cstring>

#include <rapidxml/rapidxml.hpp>
#include <string_view>
#include <zwidget/widgets/checkboxlabel/checkboxlabel.h>
#include <zwidget/widgets/textedit/textedit.h>

#include "filesystem.h"
#include "findfile.h"
#include "gameconfigfile.h"
#include "gstrings.h"
#include "i_interface.h"
#include "launcherwindow.h"
#include "name.h"
#include "releasepage.h"
#include "version.h"
#include "zstring.h"

constexpr unsigned NUMBER_OF_RELEASES_TO_DISPLAY = 3;

ReleasePage::ReleasePage(LauncherWindow* launcher, const FStartupSelectionInfo& info) : Widget(nullptr), Launcher(launcher)
{
	ShowThis = new CheckboxLabel(this);
	Notes = new TextEdit(this);

	auto text = GetReleaseNotes();

	Notes->SetText(text.GetChars());
	Notes->SetReadOnly(true);

	ShowThis->SetChecked(info.notifyNewRelease);
}

void ReleasePage::SetValues(FStartupSelectionInfo& info) const
{
	info.notifyNewRelease = ShowThis->GetChecked();
}

void ReleasePage::UpdateLanguage()
{
	ShowThis->SetText(GStrings.GetString("PICKER_SHOWTHIS"));
}

void ReleasePage::OnGeometryChanged()
{
	double y = 0.0;
	double w = GetWidth();
	double h = GetHeight();

	Notes->SetFrameGeometry(0.0, y, w, h - ShowThis->GetPreferredHeight());
	y += h - ShowThis->GetPreferredHeight();

	ShowThis->SetFrameGeometry(0.0, y, w, ShowThis->GetPreferredHeight());
	y += ShowThis->GetPreferredHeight();

	Launcher->UpdatePlayButton();
}


FString ReleasePage::_ParseReleaseNotes(rapidxml::xml_node<char> * release)
{
	// braindead html to plaintext parser

	if (!release) return GStrings.GetString("NOTES_FAIL"); // "Unable to parse release notes";

	auto description = release->first_node("description");
	auto version = release->first_attribute("version");
	auto date = release->first_attribute("date");
	auto url = release->first_node("url");
	FString text;

	// https://docs.flathub.org/docs/for-app-authors/metainfo-guidelines#description
	//
	// Only the following child tags are supported:
	// p (paragraph), ol, ul (ordered and unordered list) with li (list items) child tags,
	// em for italicized emphasis and code for inline code in monospace. **bold** is also

	auto block = [&text](const char *prefix, rapidxml::xml_node<char> *node)
	{
		FString block;

		while (node)
		{
			rapidxml::xml_node<char> *text = nullptr;
			const char *l = "", *r = "";
			if (node->type() == rapidxml::node_element)
			{
				auto name = std::string_view(node->name());
				if (name == "em")
				{
					l = r = "*";
				}
				else if (name == "code")
				{
					l = r = "`";
				}
				text = node;
			}
			else if (node->type() == rapidxml::node_data)
			{
				text = node;
			}
			if (text)
			{
				block.AppendFormat("%s%s%s", l, node->value(), r);
			}
			node = node->next_sibling();
		}

		text.AppendFormat("%s%s\n", prefix, block.GetChars());
	};

	if (description)
	{
		auto blocknode = description->first_node();
		std::string_view prev = "";
		while (blocknode)
		{
			std::string_view name = blocknode->name();
			if (prev != "" && (name == "p" || name == prev))
				text.AppendCharacter('\n');
			prev = name;

			if (name == "p")
			{
				block("", blocknode->first_node());
			}
			else if (name == "ul")
			{
				auto node = blocknode->first_node();
				while (node)
				{
					block("-  ", node->first_node());
					node = node->next_sibling();
				}
			}
			else if (name == "ol")
			{
				auto count = 0;
				auto node = blocknode->first_node();
				while (node)
				{
					FString pfx = FStringf("%d. ", ++count);
					block(pfx.GetChars(), node->first_node());
					node = node->next_sibling();
				}
			}

			blocknode = blocknode->next_sibling();
		}
	}

	return FStringf{
		GAMENAME " %s %s, %s %s\n\n%s\n%s",
		GStrings.GetString("NOTES_VERSION"), // "version"
		version
			? version->value()
			: GStrings.GetString("NOTES_UNKNOWN"), // "Unknown"
		GStrings.GetString("NOTES_RELEASED"), // "released"
		date
			? date->value()
			: GStrings.GetString("NOTES_UNKNOWN"), // "Unknown"
		description
			? text.GetChars()
			: GStrings.GetString("NOTES_EMPTY"), // "No description provided."
		url
			? FStringf(
				"\n%s %s",
				GStrings.GetString("NOTES_DETAILS"), // "For more details see:"
				url->value()
			).GetChars()
			: ""
	};
}

FString ReleasePage::_BuildReleaseNotes(rapidxml::xml_document<> &doc)
{
	// braindead html to plaintext parser

	// traverse to first (latest) release node
	auto release = doc.first_node("component");
	if (!release) return GStrings.GetString("NOTES_FAIL"); // "Unable to parse release notes";
	release = release->first_node("releases");
	if (!release) return GStrings.GetString("NOTES_FAIL"); // "Unable to parse release notes";
	release = release->first_node("release");

	FString text;

	for (unsigned i = 1; ; i++)
	{
		release->type();
		text.AppendFormat("%s", _ParseReleaseNotes(release).GetChars());

		if (!release || i >= NUMBER_OF_RELEASES_TO_DISPLAY) break;

		text.AppendFormat("\n\n---\n\n");
		release = release->next_sibling("release");
	}

	return text;
}

// Ensure you free returned pointer
char * ReleasePage::_OpenReleaseNotes()
{
	auto wad = BaseFileSearch(BASEWAD, NULL, true, GameConfig);
	if (!wad) return nullptr;

	// we need to be free
	auto resf = FResourceFile::OpenResourceFile(wad);
	if (!resf) return nullptr;

	char * notes = nullptr;
	auto lump = resf->FindEntry("meta.xml"); // created from org.zdoom.GZDoom.metainfo.xml during build

	if (lump >= 0)
	{
		auto data = resf->Read(lump);

		// needs to be freed by caller
		notes = (char *)calloc(data.size()+1, sizeof(char));

		if (notes) strncpy(notes, data.string(), data.size());
	}

	delete resf;

	return notes;
}

//==========================================================================
//
// Extract release notes from gzdoom.pk3
//
//==========================================================================
FString ReleasePage::GetReleaseNotes()
{
	// we need to be free
	char * text = _OpenReleaseNotes();

	rapidxml::xml_document<> doc;
	if (text) doc.parse<rapidxml::parse_default>(text);
	FString content = _BuildReleaseNotes(doc);

	free(text);

	return content;
}
