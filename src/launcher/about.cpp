/*
** about.cpp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#include "about.h"
#include "const.h"

#include "gstrings.h"
#include "version.h"
#include "versioninfo.h"

#ifdef HAS_UPDATER
#include "curl_loader.h"
#endif

#include <QCheckBox>
#include <QDateTime>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTabWidget>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <rapidxml/rapidxml.hpp>
#include <sstream>
#include <string>
#include <string_view>

namespace
{
	struct CachedAboutDocs
	{
		QString metaXml;
		QString aboutTxt;
		QString contributorsTxt;
	};

	const CachedAboutDocs& getCachedAboutDocs()
	{
		static const CachedAboutDocs docs = []()
		{
			CachedAboutDocs out;
			const auto entries = readBaseWadEntries({"meta.xml", "about.txt", "contributors.txt"});
			auto toQStr = [&](const char* key) -> QString
			{
				const auto it = entries.find(key);
				return (it != entries.end()) ? QString::fromUtf8(it->second.constData(), it->second.size()) : QString();
			};
			out.metaXml = toQStr("meta.xml");
			out.aboutTxt = toQStr("about.txt");
			out.contributorsTxt = toQStr("contributors.txt");
			return out;
		}();
		return docs;
	}

	QString formatPrintfStrings(QString fmt, const QStringList& args)
	{
		for (const QString& a : args)
		{
			const qsizetype pos = fmt.indexOf(QStringLiteral("%s"));
			if (pos < 0)
				break;
			fmt.replace(pos, 2, a);
		}
		return fmt;
	}

	QString parseSingleReleaseNode(rapidxml::xml_node<char>* release, bool showUpcoming, VersionInfo* outVersion)
	{
		if (!release)
			return {};

		auto* description = release->first_node("description");
		auto* version = release->first_attribute("version");
		auto* date = release->first_attribute("date");
		auto* url = release->first_node("url");

		QString md;
		const VersionInfo verInfo{version ? version->value() : "0.0.0"};
		if (outVersion)
			*outVersion = verInfo;
#ifdef HAS_UPDATER
		const VersionInfo currentVer = GetCurrentVersionForUpdater();
#else
		const VersionInfo currentVer = GetCurrentVersion();
#endif
		if (!showUpcoming && verInfo > currentVer)
			return {};

		constexpr VersionInfo firstUzdoom{4, 14, 3};
		const QString relHeader = formatPrintfStrings(
			LStr("NOTES_RELEASED", "%s version %s, released %s"),
			{
				QString::fromUtf8(verInfo >= firstUzdoom ? GAMENAME : "GZDoom"),
				version ? QString::fromUtf8(version->value()) : LStr("NOTES_UNKNOWN", "Unknown"),
				date ? QString::fromUtf8(date->value()) : LStr("NOTES_UNKNOWN", "Unknown")
			});
		md += "# " + relHeader + "\n\n";

		auto appendInlineNodes = [&](const QString& prefix, rapidxml::xml_node<char>* node)
		{
			QString line;
			while (node)
			{
				const char* wrap = "";
				rapidxml::xml_node<char>* target = nullptr;
				if (node->type() == rapidxml::node_element)
				{
					const std::string_view tag(node->name());
					if (tag == "em")
						wrap = "*";
					else if (tag == "code")
						wrap = "`";
					target = node;
				}
				else if (node->type() == rapidxml::node_data)
				{
					target = node;
				}
				if (target && target->value())
					line += wrap + QString::fromUtf8(target->value()) + wrap;
				node = node->next_sibling();
			}
			md += prefix + line + "\n";
		};

		if (description)
		{
			auto* block = description->first_node();
			std::string_view prev;
			while (block)
			{
				const std::string_view name(block->name());
				if (auto* firstChild = block->first_node();
					firstChild && name == "p" && firstChild == block->last_node() && firstChild->type() == rapidxml::node_data)
				{
					const QString raw = QString::fromUtf8(firstChild->value());
					if (raw.startsWith("**") && raw.endsWith("**") && raw.size() > 4)
					{
						md += "\n## " + raw.mid(2, raw.size() - 4) + "\n\n";
						prev = "h2";
						block = block->next_sibling();
						continue;
					}
				}

				if (!prev.empty() && (name == "p" || name != prev))
					md += "\n";
				prev = name;

				if (name == "p")
				{
					appendInlineNodes("", block->first_node());
				}
				else if (name == "ul")
				{
					for (auto* li = block->first_node(); li; li = li->next_sibling())
						appendInlineNodes("- ", li->first_node());
				}
				else if (name == "ol")
				{
					int idx = 0;
					for (auto* li = block->first_node(); li; li = li->next_sibling())
						appendInlineNodes(QString("%1. ").arg(++idx), li->first_node());
				}
				block = block->next_sibling();
			}
		}
		else
		{
			md += LStr("NOTES_EMPTY", "No description provided.") + "\n";
		}

		if (url && url->value())
		{
			const QString urlStr = QString::fromUtf8(url->value());
			const QString linkMd = QString("[%1](%1)").arg(urlStr);
			md += "\n" + formatPrintfStrings(LStr("NOTES_DETAILS", "For more details see: %s"), {linkMd}) + "\n";
		}

		return md;
	}
}

AboutDialog::AboutDialog(
	bool showAfterUpdate,
	bool startOnReleaseNotes,
	QWidget* parent,
	std::function<void(QWidget*)> checkUpdatesCallback,
	bool showUpcomingReleases)
	: QDialog(parent)
	, showUpcoming(showUpcomingReleases)
{
	setWindowTitle(LStr("LAUNCHER_INFO_TITLE", "Additional Information"));
	setMinimumSize(700, 500);
	resize(760, 540);

	QVBoxLayout* mainLayout = new QVBoxLayout(this);

	tabs = new QTabWidget(this);

	QWidget* releaseTab = new QWidget(this);
	QVBoxLayout* relLayout = new QVBoxLayout(releaseTab);
	releaseNotesView = new QTextBrowser(releaseTab);
	releaseNotesView->setOpenExternalLinks(true);
	relLayout->addWidget(releaseNotesView);

	chkShowAfterUpdate = new QCheckBox(LStr("PICKER_SHOWTHIS", "Show Page after each Update"), releaseTab);
	chkShowAfterUpdate->setChecked(showAfterUpdate);
	relLayout->addWidget(chkShowAfterUpdate);

	tabs->addTab(releaseTab, LStr("PICKER_TAB_RELEASE", "Release Notes"));

	QWidget* aboutTab = new QWidget(this);
	QVBoxLayout* aboutLayout = new QVBoxLayout(aboutTab);
	aboutView = new QTextBrowser(aboutTab);
	aboutView->setOpenExternalLinks(true);
	aboutLayout->addWidget(aboutView);
	tabs->addTab(aboutTab, LStr("PICKER_TAB_ABOUT", "About"));

	mainLayout->addWidget(tabs);

	QHBoxLayout* bottomBar = new QHBoxLayout();
#ifdef HAS_UPDATER
	if (IsCurlLoaded() && checkUpdatesCallback)
	{
		btnCheckUpdates = new QPushButton(LStr("UPDATER_CHECK_FOR_UPDATES", "Check for Updates"), this);
		connect(btnCheckUpdates, &QPushButton::clicked, this, [this, cb = std::move(checkUpdatesCallback)]() {
			cb(this);
		});
		bottomBar->addWidget(btnCheckUpdates);
	}
#else
	(void)checkUpdatesCallback;
#endif
	bottomBar->addStretch();
	btnReturn = new QPushButton(LStr("LAUNCHER_RETURN", "Return"), this);
	btnReturn->setMinimumWidth(110);
	connect(btnReturn, &QPushButton::clicked, this, &QDialog::accept);
	bottomBar->addWidget(btnReturn);
	mainLayout->addLayout(bottomBar);

	populateReleaseNotes();
	populateAbout();

	tabs->setCurrentIndex(startOnReleaseNotes ? 0 : 1);
}

bool AboutDialog::showAfterUpdate() const
{
	return chkShowAfterUpdate->isChecked();
}

void AboutDialog::populateReleaseNotes()
{
	const QString& xmlRaw = getCachedAboutDocs().metaXml;
	if (xmlRaw.isEmpty())
	{
		releaseNotesView->setPlainText(LStr("NOTES_FAIL", "Unable to load release notes from uzdoom.pk3."));
		return;
	}

	QByteArray utf8 = xmlRaw.toUtf8();
	utf8.append('\0');

	rapidxml::xml_document<> doc;
	try
	{
		doc.parse<rapidxml::parse_default>(utf8.data());
	}
	catch (...)
	{
		releaseNotesView->setPlainText(LStr("NOTES_FAIL", "Unable to parse release notes."));
		return;
	}

	auto* comp = doc.first_node("component");
	auto* releases = comp ? comp->first_node("releases") : nullptr;
	auto* rel = releases ? releases->first_node("release") : nullptr;
	if (!rel)
	{
		releaseNotesView->setPlainText(LStr("NOTES_FAIL", "Unable to parse release notes."));
		return;
	}

	constexpr unsigned numberOfReleasesToDisplay = 3;
	QString fullMarkdown;
	VersionInfo verInfo{};
	for (unsigned shownMajorReleases = 0; rel; rel = rel->next_sibling("release"))
	{
		const QString entry = parseSingleReleaseNode(rel, showUpcoming, &verInfo);
		if (!entry.isEmpty())
		{
			if (!fullMarkdown.isEmpty())
				fullMarkdown += "\n---\n\n";
			fullMarkdown += entry;
			if (verInfo.revision == 0 && ++shownMajorReleases >= numberOfReleasesToDisplay)
				break;
		}
	}

	releaseNotesView->setMarkdown(fullMarkdown);
}

void AboutDialog::populateAbout()
{
	const CachedAboutDocs& docs = getCachedAboutDocs();
	const QString& aboutTxt = docs.aboutTxt;
	const QString& contribTxt = docs.contributorsTxt;

	const QString rawGitTime = QString::fromUtf8(GetGitTime());
	const QDateTime parsedDt = QDateTime::fromString(rawGitTime, Qt::ISODate);
	const QString formattedDate = parsedDt.isValid() ? parsedDt.date().toString("dd-MM-yyyy") : rawGitTime;

	QString md;
	md += QString("# %1 %2\n\n").arg(GAMENAME).arg(GetVersionString());
	md += QString("**%1** %2\n\n").arg(LStr("LAUNCHER_BUILD_DATE", "Build Date:"), formattedDate);
	if (!aboutTxt.isEmpty())
		md += aboutTxt + "\n\n";
	else
		md += LStr("LAUNCHER_ABOUT_FALLBACK", "UZDoom is a feature-rich source port for Doom, Heretic, Hexen, Strife, and Chex Quest.") + "\n\n";

	if (!contribTxt.isEmpty())
	{
		md += "## " + LStr("LAUNCHER_CONTRIBUTORS", "Contributors") + "\n\n";
		const QStringList lines = contribTxt.split('\n', Qt::SkipEmptyParts);
		for (const QString& rawLine : lines)
		{
			const QString name = rawLine.trimmed();
			if (!name.isEmpty())
				md += "- " + name + "\n";
		}
		md += "\n";
	}

	aboutView->setMarkdown(md);
}

bool AboutDialog::showAdditionalInfo(
	bool& showAfterUpdate,
	bool startOnReleaseNotes,
	QWidget* parent,
	std::function<void(QWidget*)> checkUpdatesCallback,
	bool showUpcomingReleases)
{
	AboutDialog dlg(showAfterUpdate, startOnReleaseNotes, parent, std::move(checkUpdatesCallback), showUpcomingReleases);
	const int res = dlg.exec();
	showAfterUpdate = dlg.showAfterUpdate();
	return res == QDialog::Accepted;
}
