/*
** updater.h
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

struct FStartupSelectionInfo;

#ifdef HAS_UPDATER

#include "versioninfo.h"

#include <QPointer>
#include <atomic>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class QFrame;
class QPushButton;
class QThread;
class QToolButton;
class QWidget;

struct UpdaterReleaseInfo
{
	VersionInfo version;
	bool cached = false;
	std::vector<std::string> releaseNotes;
	std::string downloadUrl;
};

enum class UpdaterMode
{
	Disabled = 0,
	Notify = 1,
	PromptToInstall = 2
};

class LauncherUpdater
{
public:
	explicit LauncherUpdater(QWidget* parent, std::function<void()> settingsChangedCallback = {});
	~LauncherUpdater();

	LauncherUpdater(const LauncherUpdater&) = delete;
	LauncherUpdater& operator=(const LauncherUpdater&) = delete;

	QWidget* bannerWidget() const;
	void retranslateUi();
	void refreshTheme();
	void setBannerEnabled(bool enabled);

	void checkForUpdate(bool force = false, QWidget* dialogParent = nullptr);
	void cancelPending();

	UpdaterMode currentMode() const;
	int currentIntervalDays() const;
	void setMode(UpdaterMode mode);
	void setIntervalDays(int days);

private:
	struct FetchOutcome
	{
		bool ok = false;
		bool silentFail = false;
		bool cancelled = false;
		std::string errorMessage;
		std::optional<UpdaterReleaseInfo> info;
	};

	QWidget* resolveDialogParent(QWidget* dialogParent) const;
	void notifySettingsChanged();
	void showBanner();
	void hideBanner();

	void onBannerClicked();
	void onDismissBannerClicked();

	void runOnboardingFlow(QWidget* dialogParent);
	void openUpdateMenu(bool isAutoUpdate, QWidget* dialogParent);
	void openReleaseNotesDialog(QWidget* dialogParent) const;
	void openFailedUpdateMenu(const std::string& err, bool checker, QWidget* dialogParent);
	void startUpdate(QWidget* dialogParent);

	void handleCheckOutcome(const FetchOutcome& outcome, bool force, bool wasCached, const VersionInfo& cachedVer, const VersionInfo& skippedVer, uint64_t curTime, QWidget* dialogParent);
	bool ensureFullUpdateInfo(QWidget* dialogParent);
	FetchOutcome fetchUpdateInfoInteractive(QWidget* dialogParent);
	void startBackgroundCheck(const VersionInfo& cachedVer, bool wasCached, const VersionInfo& skippedVer, uint64_t curTime);

	static bool initCurl(std::string& errorOut);
	static FetchOutcome fetchUpdateInfoSync(const std::atomic<bool>* cancelFlag);

	QPointer<QWidget> parentWindow;
	std::function<void()> onSettingsChanged;

	QFrame* banner = nullptr;
	QToolButton* btnDismiss = nullptr;
	QPushButton* btnOpenUpdate = nullptr;

	std::optional<UpdaterReleaseInfo> currentUpdate;

	QPointer<QThread> checkThread;
	std::shared_ptr<std::atomic<bool>> checkCancel;
	std::shared_ptr<std::atomic<bool>> destroyed;
};

void syncStartupUpdaterInfo(FStartupSelectionInfo& info);

#endif
