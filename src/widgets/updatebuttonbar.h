/*
** updatebuttonbar.h
**
**
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

#pragma once

#include <zwidget/core/widget.h>
#include <optional>

#include "basics.h"
#include "version.h"

class LauncherWindow;
class PushButton;

struct update_info_t
{
	VersionInfo version;

	bool cached = false;

	std::vector<std::string> release_notes;

	std::string download_url;
};

class UpdateButtonBar : public Widget
{
	public:
		UpdateButtonBar(LauncherWindow *parent);
		void UpdateLanguage();

		double GetPreferredHeight() const;

		void CheckForUpdate(bool force = false);

		int GetMargin() { return 4; }

	protected:
		void OnPaint(Canvas* canvas) override;
		void OnMouseMove(const Point& pos) override;
		void OnMouseLeave() override;
		bool OnMouseDown(const Point& pos, InputKey key) override;
		bool OnMouseUp(const Point& pos, InputKey key) override;
		void OnUpdateButtonClicked();

		LauncherWindow *GetLauncher() const;

		std::optional<update_info_t> GetUpdateInfo(bool &ok);

		FString text;

		bool close_highlighted = false;
		bool pressed = false;
		bool pressed_hover = false;
		bool close_pressed = false;
		bool close_pressed_hover = false;

		std::shared_ptr<Image> arrow;
		std::shared_ptr<Image> close;

		std::optional<update_info_t> currentUpdate;

		friend void OpenDismissUpdateMenu(UpdateButtonBar * self, bool isAutoUpdate);
		friend void OpenUpdateMenu(UpdateButtonBar * self, bool isAutoUpdate);
	public:
		std::string GetDownloadURL() { return currentUpdate->download_url; }
	private:
		void OpenUpdateInitChoice();
		void OpenUpdateIntervalChoice();
		void OpenUpdateMenu(bool isAutoUpdate);
		void OpenDismissUpdateMenu();
		void OpenFailedUpdateMenu(const std::string &err, bool checker);
		void StartUpdate();
		FString UpdateToString();
		bool InitCurl();

		template<typename T>
		std::optional<update_info_t> ParseRelease(T &&doc, bool &ok, bool &silentfail);

		static bool curl_initialized;
		static bool curl_initialized_ok;

		friend class JsonDownloader;
		friend class ProgressDownloader;
		friend class UpdateChecker;
};
