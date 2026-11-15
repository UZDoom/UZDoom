/*
** launcherMainWindow.h
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

#include "loader.h"
#include "profile.h"

#include <QElapsedTimer>
#include <QMainWindow>
#include <QPointer>
#include <atomic>
#include <memory>
#include <vector>

class QAction;
class QActionGroup;
class QDragEnterEvent;
class QDropEvent;
class QCloseEvent;
class QLabel;
class QLineEdit;
class QMenu;
class QProcess;
class QPushButton;
class QTableWidget;
class QTextEdit;
class QThread;
class LauncherUpdater;

class LauncherMainWindow : public QMainWindow
{
	Q_OBJECT
public:
	explicit LauncherMainWindow(const LauncherConfig& initialConfig, std::vector<Profile> initialProfiles, bool showUpcomingNotes = false, QWidget* parent = nullptr);
	~LauncherMainWindow() override;

	const LauncherConfig& config() const { return launcherConfig; }
	void setShowReleaseNotes(bool show);
	void startStartupIWadDetection();
	void startStartupUpdateCheck();

protected:
	bool eventFilter(QObject* watched, QEvent* event) override;
	void dragEnterEvent(QDragEnterEvent* event) override;
	void dropEvent(QDropEvent* event) override;
	void closeEvent(QCloseEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;

private slots:
	void onPlayClicked();
	void onJoinClicked();
	void onHostClicked();
	void onProfileSettingsClicked();
	void onMoveUpClicked();
	void onMoveDownClicked();

	void onAddGameFiles();
	void onCreateBlankProfile();
	void onImportProfileArchive();
	void onAutoDetectIWADs();
	void onRefreshProfiles();
	void onOpenLauncherFolder();
	void onOpenExecFolder();
	void onOpenSelectedProfileFolder();
	void onCloneSelectedProfile();
	void onExportSelectedProfile();
	void onDeleteSelectedProfile();

	void onAboutClicked();
	void onFilterTextChanged(const QString& text);
	void onSelectionChanged();
	void onTableContextMenu(const QPoint& pos);

	void onLanguageTriggered(QAction* action);
	void onThemeTriggered(QAction* action);
	void onToggleDetectStartup(bool checked);
	void onToggleHideWhilePlaying(bool checked);
#ifdef HAS_UPDATER
	void onCheckUpdatesClicked();
	void onUpdaterModeTriggered(QAction* action);
	void onUpdaterIntervalTriggered(QAction* action);
#endif

private:
	void buildUi();
	void rebuildMenus();
	void retranslateUi();
#ifdef HAS_UPDATER
	void updateUpdaterMenuState();
#endif
	void refreshTable(int selectProfileIndex = -1);
	void updateTableRow(int tableRow, int profileIdx);
	void updateDetailsPane();
	void updateCoverPixmapScale();
	void updateButtonStates();
	void applyFilter();

	int selectedProfileIndex() const;
	void selectProfileIndex(int profileIdx);
	void moveSelectedProfile(int delta);
	void handleImportedFiles(const QStringList& localPaths);
	void openSettingsForIndex(int profileIdx);
	void launchProfile(LaunchTargetMode mode);
	void setRunningState(bool running, const QString& title = {});
	void startIWadAutoDetect(bool interactive);
	void onIWadScanFinished(const std::vector<DiscoveredIWadCandidate>& candidates);

	LauncherConfig launcherConfig;
	std::vector<Profile> profiles;
	bool showUpcomingReleaseNotes = false;

	QLineEdit* searchEdit = nullptr;

	QMenu* addMenu = nullptr;
	QMenu* prefMenu = nullptr;
	QAction* actAbout = nullptr;
	QMenu* langMenu = nullptr;
	QMenu* themeMenu = nullptr;
	QActionGroup* langGroup = nullptr;
	QActionGroup* themeGroup = nullptr;

	QAction* actAddFile = nullptr;
	QAction* actCreateBlank = nullptr;
	QAction* actImportZip = nullptr;
	QAction* actAutoDetect = nullptr;
	QAction* actRefresh = nullptr;
	QAction* actOpenLauncherDir = nullptr;
	QAction* actOpenExecDir = nullptr;
	QAction* actQuit = nullptr;
	QAction* actDetectStartup = nullptr;
	QAction* actHideWhilePlaying = nullptr;
#ifdef HAS_UPDATER
	std::unique_ptr<LauncherUpdater> updater;
	QMenu* updaterModeMenu = nullptr;
	QMenu* updaterIntervalMenu = nullptr;
	QActionGroup* updaterModeGroup = nullptr;
	QActionGroup* updaterIntervalGroup = nullptr;
	QAction* actCheckUpdates = nullptr;
#endif

	QTableWidget* table = nullptr;
	QPushButton* btnPlay = nullptr;
	QPushButton* btnJoin = nullptr;
	QPushButton* btnHost = nullptr;
	QPushButton* btnSettings = nullptr;
	QPushButton* btnMoveUp = nullptr;
	QPushButton* btnMoveDown = nullptr;

	QLabel* coverLabel = nullptr;
	QTextEdit* descView = nullptr;
	QPixmap cachedCoverPixmap;
	std::string cachedCoverPath;
	bool usingFallbackCover = false;

	QLabel* statusCountLabel = nullptr;
	QLabel* statusRunLabel = nullptr;

	QProcess* gameProcess = nullptr;
	int runningProfileIndex = -1;
	QElapsedTimer playTimer;
	bool wasHiddenForLaunch = false;

	QPointer<QThread> detectThread;
	bool detectInteractive = false;
	std::shared_ptr<std::atomic<bool>> detectCancel;
};
