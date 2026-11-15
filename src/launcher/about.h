/*
** about.h
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

#pragma once

#include <QDialog>
#include <functional>

class QCheckBox;
class QPushButton;
class QTabWidget;
class QTextBrowser;
class QWidget;

class AboutDialog : public QDialog
{
	Q_OBJECT
public:
	explicit AboutDialog(
		bool showAfterUpdate,
		bool startOnReleaseNotes = false,
		QWidget* parent = nullptr,
		std::function<void(QWidget*)> checkUpdatesCallback = {},
		bool showUpcomingReleases = false);

	bool showAfterUpdate() const;

	static bool showAdditionalInfo(
		bool& showAfterUpdate,
		bool startOnReleaseNotes = false,
		QWidget* parent = nullptr,
		std::function<void(QWidget*)> checkUpdatesCallback = {},
		bool showUpcomingReleases = false);

private:
	void populateReleaseNotes();
	void populateAbout();

	bool showUpcoming = false;

	QTabWidget* tabs = nullptr;
	QTextBrowser* releaseNotesView = nullptr;
	QTextBrowser* aboutView = nullptr;
	QCheckBox* chkShowAfterUpdate = nullptr;
	QPushButton* btnCheckUpdates = nullptr;
	QPushButton* btnReturn = nullptr;
};
