/*
** launcherMainWindow.cpp
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

#include "launcherMainWindow.h"
#include "about.h"
#include "const.h"
#include "loader.h"
#include "profileSettings.h"

#ifdef HAS_UPDATER
#include "curl_loader.h"
#include "updater.h"
#endif

#include "gstrings.h"
#include "version.h"
#include "versioninfo.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QHeaderView>
#include <QInputDialog>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMetaObject>
#include <QMimeData>
#include <QPalette>
#include <QProcess>
#include <QPushButton>
#include <QResizeEvent>
#include <QSet>
#include <QShortcut>
#include <QSplitter>
#include <QStatusBar>
#include <QTableWidget>
#include <QTextEdit>
#include <QThread>
#include <QUrl>
#include <QVBoxLayout>
#include <algorithm>

namespace
{
	const QSet<QString>& acceptedDropExtensions()
	{
		static const QSet<QString> exts = {
			"wad", "iwad", "pwad", "pk3", "ipk3", "pk7", "ipk7", "deh", "bex", "zip", "7z", "uzdp"
		};
		return exts;
	}

	bool hasSupportedLocalFiles(const QMimeData* mime)
	{
		if (!mime || !mime->hasUrls())
			return false;
		for (const QUrl& url : mime->urls())
		{
			if (!url.isLocalFile())
				continue;
			const QString ext = QFileInfo(url.toLocalFile()).suffix().toLower();
			if (acceptedDropExtensions().contains(ext))
				return true;
		}
		return false;
	}

	QStringList extractSupportedLocalFiles(const QMimeData* mime)
	{
		QStringList paths;
		if (!mime || !mime->hasUrls())
			return paths;
		for (const QUrl& url : mime->urls())
		{
			if (!url.isLocalFile())
				continue;
			const QString local = url.toLocalFile();
			if (acceptedDropExtensions().contains(QFileInfo(local).suffix().toLower()))
				paths << local;
		}
		return paths;
	}
}

LauncherMainWindow::LauncherMainWindow(const LauncherConfig& initialConfig, std::vector<Profile> initialProfiles, bool showUpcomingNotes, QWidget* parent)
	: QMainWindow(parent)
	, launcherConfig(initialConfig)
	, profiles(std::move(initialProfiles))
	, showUpcomingReleaseNotes(showUpcomingNotes)
{
	setAcceptDrops(true);
	resize(960, 660);
	setMinimumSize(780, 520);

	buildUi();
	rebuildMenus();
	retranslateUi();

	const int initialRow = profiles.empty()
		? -1
		: std::clamp(launcherConfig.lastSelectedRow, 0, static_cast<int>(profiles.size()) - 1);
	refreshTable(initialRow);
}

LauncherMainWindow::~LauncherMainWindow()
{
#ifdef HAS_UPDATER
	if (updater)
	{
		updater->cancelPending();
		updater.reset();
	}
#endif

	if (detectCancel)
		detectCancel->store(true, std::memory_order_relaxed);
	if (detectThread)
	{
		QThread* t = detectThread.data();
		detectThread = nullptr;
		t->disconnect();
		t->wait();
		delete t;
	}

	if (gameProcess && gameProcess->state() != QProcess::NotRunning)
	{
		recordRunningSessionPlaytime();
		gameProcess->disconnect(this);
		gameProcess->kill();
		gameProcess->waitForFinished(1500);
	}
}

void LauncherMainWindow::buildUi()
{
	QMenuBar* mb = menuBar();
	mb->setNativeMenuBar(false);
	addMenu = mb->addMenu(QString());
	prefMenu = mb->addMenu(QString());
	actAbout = mb->addAction(QString(), this, &LauncherMainWindow::onAboutClicked);

	QWidget* searchContainer = new QWidget(mb);
	QHBoxLayout* searchLayout = new QHBoxLayout(searchContainer);
	searchLayout->setContentsMargins(4, 2, 6, 2);
	searchLayout->setSpacing(0);
	searchEdit = new QLineEdit(searchContainer);
	searchEdit->setClearButtonEnabled(true);
	searchEdit->setFixedWidth(220);
	searchEdit->setMinimumHeight(24);
	connect(searchEdit, &QLineEdit::textChanged, this, &LauncherMainWindow::onFilterTextChanged);
	searchLayout->addWidget(searchEdit);
	mb->setCornerWidget(searchContainer, Qt::TopRightCorner);

	QWidget* central = new QWidget(this);
	QVBoxLayout* rootLayout = new QVBoxLayout(central);
	rootLayout->setContentsMargins(10, 8, 10, 8);
	rootLayout->setSpacing(8);

	QHBoxLayout* centerRow = new QHBoxLayout();
	centerRow->setContentsMargins(0, 0, 0, 4);
	centerRow->setSpacing(10);

	table = new QTableWidget(0, 6, central);
	table->setSelectionBehavior(QAbstractItemView::SelectRows);
	table->setSelectionMode(QAbstractItemView::SingleSelection);
	table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	table->setAlternatingRowColors(true);
	table->setShowGrid(false);
	table->setContextMenuPolicy(Qt::CustomContextMenu);
	table->verticalHeader()->setVisible(false);
	table->verticalHeader()->setDefaultSectionSize(28);
	table->horizontalHeader()->setStretchLastSection(false);
	table->horizontalHeader()->setMinimumSectionSize(75);
	table->horizontalHeader()->setHighlightSections(false);
	table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
	table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
	table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
	table->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);

	connect(table, &QTableWidget::itemSelectionChanged, this, &LauncherMainWindow::onSelectionChanged);
	connect(table, &QTableWidget::itemDoubleClicked, this, [this](QTableWidgetItem*) { onPlayClicked(); });
	connect(table, &QTableWidget::customContextMenuRequested, this, &LauncherMainWindow::onTableContextMenu);
	table->installEventFilter(this);
	centerRow->addWidget(table, 1);

	QWidget* rightButtonsWidget = new QWidget(central);
	rightButtonsWidget->setFixedWidth(182);
	QVBoxLayout* rightCol = new QVBoxLayout(rightButtonsWidget);
	rightCol->setContentsMargins(0, 0, 0, 0);
	rightCol->setSpacing(8);

	btnPlay = new QPushButton(rightButtonsWidget);
	btnPlay->setMinimumHeight(44);
	QFont playFont = btnPlay->font();
	playFont.setPointSize(playFont.pointSize() + 1);
	playFont.setBold(true);
	btnPlay->setFont(playFont);
	btnPlay->setProperty("launcherRole", "primary");
	connect(btnPlay, &QPushButton::clicked, this, &LauncherMainWindow::onPlayClicked);

	btnJoin = new QPushButton(rightButtonsWidget);
	btnJoin->setMinimumHeight(34);
	connect(btnJoin, &QPushButton::clicked, this, &LauncherMainWindow::onJoinClicked);

	btnHost = new QPushButton(rightButtonsWidget);
	btnHost->setMinimumHeight(34);
	connect(btnHost, &QPushButton::clicked, this, &LauncherMainWindow::onHostClicked);

	btnSettings = new QPushButton(rightButtonsWidget);
	btnSettings->setMinimumHeight(34);
	connect(btnSettings, &QPushButton::clicked, this, &LauncherMainWindow::onProfileSettingsClicked);

	btnMoveUp = new QPushButton(rightButtonsWidget);
	btnMoveUp->setMinimumHeight(30);
	connect(btnMoveUp, &QPushButton::clicked, this, &LauncherMainWindow::onMoveUpClicked);

	btnMoveDown = new QPushButton(rightButtonsWidget);
	btnMoveDown->setMinimumHeight(30);
	connect(btnMoveDown, &QPushButton::clicked, this, &LauncherMainWindow::onMoveDownClicked);

	btnRefresh = new QPushButton(rightButtonsWidget);
	btnRefresh->setMinimumHeight(30);
	connect(btnRefresh, &QPushButton::clicked, this, &LauncherMainWindow::onRefreshProfiles);

	rightCol->addWidget(btnPlay);
	rightCol->addWidget(btnJoin);
	rightCol->addWidget(btnHost);
	rightCol->addWidget(btnSettings);
	rightCol->addSpacing(8);
	rightCol->addWidget(btnMoveUp);
	rightCol->addWidget(btnMoveDown);
	rightCol->addStretch();
	rightCol->addWidget(btnRefresh);

	centerRow->addWidget(rightButtonsWidget);

	QWidget* bottomRowWidget = new QWidget(central);
	QHBoxLayout* bottomRow = new QHBoxLayout(bottomRowWidget);
	bottomRow->setContentsMargins(0, 4, 0, 0);
	bottomRow->setSpacing(10);

	coverLabel = new QLabel(bottomRowWidget);
	coverLabel->setObjectName("launcherCoverLabel");
	coverLabel->setFixedWidth(240);
	coverLabel->setMinimumHeight(120);
	coverLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
	coverLabel->setContentsMargins(0, 0, 0, 0);
	coverLabel->setFrameShape(QFrame::StyledPanel);
	coverLabel->setFrameShadow(QFrame::Sunken);
	coverLabel->setAlignment(Qt::AlignCenter);
	coverLabel->setWordWrap(true);
	coverLabel->installEventFilter(this);

	descView = new QTextEdit(bottomRowWidget);
	descView->setReadOnly(true);

	bottomRow->addWidget(coverLabel);
	bottomRow->addWidget(descView, 1);

	QWidget* centerContainer = new QWidget(central);
	centerContainer->setLayout(centerRow);

	verticalSplitter = new QSplitter(Qt::Vertical, central);
	verticalSplitter->setChildrenCollapsible(false);
	verticalSplitter->addWidget(centerContainer);
	verticalSplitter->addWidget(bottomRowWidget);
	verticalSplitter->setStretchFactor(0, 3);
	verticalSplitter->setStretchFactor(1, 1);
	if (launcherConfig.splitterTopSize > 0 && launcherConfig.splitterBottomSize > 0)
		verticalSplitter->setSizes({launcherConfig.splitterTopSize, launcherConfig.splitterBottomSize});
	else
		verticalSplitter->setSizes({360, 165});

	auto* scFind = new QShortcut(QKeySequence::Find, this);
	connect(scFind, &QShortcut::activated, this, [this]() {
		searchEdit->setFocus();
		searchEdit->selectAll();
	});
	auto* scRefresh = new QShortcut(QKeySequence::Refresh, this);
	connect(scRefresh, &QShortcut::activated, this, &LauncherMainWindow::onRefreshProfiles);
	auto* scSettings = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_E), this);
	connect(scSettings, &QShortcut::activated, this, &LauncherMainWindow::onProfileSettingsClicked);

#ifdef HAS_UPDATER
	if (IsCurlLoaded())
	{
		updater = std::make_unique<LauncherUpdater>(this, [this]() {
			updateUpdaterMenuState();
		});
		rootLayout->addWidget(updater->bannerWidget());
	}
#endif

	rootLayout->addWidget(verticalSplitter, 1);

	setCentralWidget(central);

	statusCountLabel = new QLabel(this);
	statusRunLabel = new QLabel(this);
	statusBar()->addWidget(statusCountLabel, 1);
	statusBar()->addPermanentWidget(statusRunLabel);
}

void LauncherMainWindow::rebuildMenus()
{
	addMenu->clear();
	actAddFile = addMenu->addAction(QString(), this, &LauncherMainWindow::onAddGameFiles);
	actCreateBlank = addMenu->addAction(QString(), this, &LauncherMainWindow::onCreateBlankProfile);
	addMenu->addSeparator();
	actOpenLauncherDir = addMenu->addAction(QString(), this, &LauncherMainWindow::onOpenLauncherFolder);
	actOpenExecDir = addMenu->addAction(QString(), this, &LauncherMainWindow::onOpenExecFolder);
	addMenu->addSeparator();
	actQuit = addMenu->addAction(QString(), this, &QWidget::close);
#ifdef Q_OS_WIN
	actQuit->setShortcuts({QKeySequence(Qt::CTRL | Qt::Key_W), QKeySequence(Qt::CTRL | Qt::Key_Q)});
#else
	{
		QKeySequence quitSeq = QKeySequence(QKeySequence::Quit);
		if (quitSeq.isEmpty())
			quitSeq = QKeySequence(Qt::CTRL | Qt::Key_Q);
		actQuit->setShortcut(quitSeq);
	}
#endif
	actQuit->setShortcutContext(Qt::WindowShortcut);

	prefMenu->clear();
	langMenu = prefMenu->addMenu("Language");
	langGroup = new QActionGroup(this);
	langGroup->setExclusive(true);
	connect(langGroup, &QActionGroup::triggered, this, &LauncherMainWindow::onLanguageTriggered);

	for (const LauncherLanguage& lang : getLauncherLanguages())
	{
		QAction* act = langMenu->addAction(lang.name);
		act->setCheckable(true);
		act->setData(lang.code);
		if (lang.code.compare(QString::fromStdString(launcherConfig.language), Qt::CaseInsensitive) == 0)
			act->setChecked(true);
		langGroup->addAction(act);
	}

	themeMenu = prefMenu->addMenu("Theme");
	themeGroup = new QActionGroup(this);
	themeGroup->setExclusive(true);
	connect(themeGroup, &QActionGroup::triggered, this, &LauncherMainWindow::onThemeTriggered);

	const struct { const char* id; const char* key; const char* fallback; } themes[] = {
		{"auto", "PICKER_THEME_AUTO", "Auto"},
		{"dark", "PICKER_THEME_DARK", "Dark"},
		{"light", "PICKER_THEME_LIGHT", "Light"},
		{"highcontrast", "PICKER_THEME_HIGHCONTRAST", "High Contrast"},
	};
	const std::string curTheme = themeToName(parseThemeName(launcherConfig.theme));
	for (const auto& t : themes)
	{
		QAction* act = themeMenu->addAction(LStr(t.key, t.fallback));
		act->setCheckable(true);
		act->setData(QString::fromLatin1(t.id));
		if (curTheme == t.id)
			act->setChecked(true);
		themeGroup->addAction(act);
	}

	prefMenu->addSeparator();
	actSetDefaultModsDir = prefMenu->addAction("Set Default Mods Folder...", this, &LauncherMainWindow::onSetDefaultModsDir);
	actClearDefaultModsDir = prefMenu->addAction("Clear Default Mods Folder", this, &LauncherMainWindow::onClearDefaultModsDir);
	updateDefaultModsMenuState();

	prefMenu->addSeparator();
	actHideWhilePlaying = prefMenu->addAction("Hide Launcher While Playing");
	actHideWhilePlaying->setCheckable(true);
	actHideWhilePlaying->setChecked(launcherConfig.hideWhilePlaying);
	connect(actHideWhilePlaying, &QAction::toggled, this, &LauncherMainWindow::onToggleHideWhilePlaying);

#ifdef HAS_UPDATER
	if (IsCurlLoaded())
	{
		prefMenu->addSeparator();

		updaterModeMenu = prefMenu->addMenu("Updater Settings");
		updaterModeGroup = new QActionGroup(this);
		updaterModeGroup->setExclusive(true);
		connect(updaterModeGroup, &QActionGroup::triggered, this, &LauncherMainWindow::onUpdaterModeTriggered);

		const int modes[] = {
			static_cast<int>(UpdaterMode::Disabled),
			static_cast<int>(UpdaterMode::Notify),
			static_cast<int>(UpdaterMode::PromptToInstall)
		};
		for (int modeVal : modes)
		{
			QAction* act = updaterModeMenu->addAction(QString());
			act->setCheckable(true);
			act->setData(modeVal);
			updaterModeGroup->addAction(act);
		}

		updaterIntervalMenu = prefMenu->addMenu("Check every:");
		updaterIntervalGroup = new QActionGroup(this);
		updaterIntervalGroup->setExclusive(true);
		connect(updaterIntervalGroup, &QActionGroup::triggered, this, &LauncherMainWindow::onUpdaterIntervalTriggered);

		const int intervals[] = {1, 7, 30};
		for (int days : intervals)
		{
			QAction* act = updaterIntervalMenu->addAction(QString());
			act->setCheckable(true);
			act->setData(days);
			updaterIntervalGroup->addAction(act);
		}

		updateUpdaterMenuState();
	}
#endif
}

void LauncherMainWindow::retranslateUi()
{
	const QString rawGitTime = QString::fromUtf8(GetGitTime());
	const QDateTime parsedDt = QDateTime::fromString(rawGitTime, Qt::ISODate);
	const QString formattedDate = parsedDt.isValid() ? parsedDt.date().toString("dd-MM-yyyy") : rawGitTime;
	setWindowTitle(QString("%1 (Version %2)[Build Date: %3]").arg(GAMENAME).arg(GetVersionString()).arg(formattedDate));

	addMenu->setTitle(LStr("LAUNCHER_ADD_ENTRY", "Add/Import New Entry"));
	prefMenu->setTitle(LStr("LAUNCHER_PREFERENCES", "Preferences"));
	actAbout->setText(LStr("LAUNCHER_MENU_INFO", "Info"));
	searchEdit->setPlaceholderText(LStr("LAUNCHER_SEARCH_PLACEHOLDER", "Filter profiles..."));

	actAddFile->setText(LStr("LAUNCHER_MENU_ADD_FILE", "Add WAD / Mod / Archive..."));
	actCreateBlank->setText(LStr("LAUNCHER_MENU_NEW_BLANK", "Create Blank Profile..."));
	actOpenLauncherDir->setText(LStr("LAUNCHER_MENU_OPEN_LAUNCHER_DIR", "Open Launcher Data Folder"));
	actOpenExecDir->setText(LStr("LAUNCHER_MENU_OPEN_APP_DIR", "Open Application Folder"));
	actQuit->setText(LStr("PICKER_EXIT", "Exit"));

	langMenu->setTitle(LStr("PICKER_LANGUAGE", "Language"));
	themeMenu->setTitle(LStr("PICKER_THEME", "Theme"));
	if (actSetDefaultModsDir)
		actSetDefaultModsDir->setText(LStr("LAUNCHER_PREF_SET_DEFAULT_MODS", "Set Default Mods Folder..."));
	if (actClearDefaultModsDir)
		actClearDefaultModsDir->setText(LStr("LAUNCHER_PREF_CLEAR_DEFAULT_MODS", "Clear Default Mods Folder"));
	updateDefaultModsMenuState();
	actHideWhilePlaying->setText(LStr("LAUNCHER_PREF_HIDE_ON_PLAY", "Hide Launcher While Playing"));

	const QList<QAction*> themeActions = themeGroup->actions();
	const char* const themeKeys[] = {"PICKER_THEME_AUTO", "PICKER_THEME_DARK", "PICKER_THEME_LIGHT", "PICKER_THEME_HIGHCONTRAST"};
	const char* const themeFallbacks[] = {"Auto", "Dark", "Light", "High Contrast"};
	for (int i = 0; i < themeActions.size() && i < 4; ++i)
		themeActions[i]->setText(LStr(themeKeys[i], themeFallbacks[i]));

#ifdef HAS_UPDATER
	if (updaterModeMenu && updaterModeGroup)
	{
		updaterModeMenu->setTitle(LStr("UPDATER_SETTINGS", "Updater Settings"));
		const QList<QAction*> modeActions = updaterModeGroup->actions();
		const char* const modeKeys[] = {"OPTVAL_DISABLE", "OPTVAL_NOTIFY", "UPDATER_PROMPT_TO_INSTALL"};
		const char* const modeFallbacks[] = {"Disable", "Notify", "Prompt to install"};
		for (int i = 0; i < modeActions.size() && i < 3; ++i)
			modeActions[i]->setText(LStr(modeKeys[i], modeFallbacks[i]));
	}
	if (updaterIntervalMenu && updaterIntervalGroup)
	{
		updaterIntervalMenu->setTitle(LStr("UPDATER_INTERVAL", "Check every:"));
		const QList<QAction*> intervalActions = updaterIntervalGroup->actions();
		const char* const intervalKeys[] = {"OPTVAL_DAILY", "OPTVAL_WEEKLY", "OPTVAL_MONTHLY"};
		const char* const intervalFallbacks[] = {"Daily", "Weekly", "Monthly"};
		for (int i = 0; i < intervalActions.size() && i < 3; ++i)
			intervalActions[i]->setText(LStr(intervalKeys[i], intervalFallbacks[i]));
	}
	if (updater)
		updater->retranslateUi();
#endif

	table->setHorizontalHeaderLabels({
		LStr("LAUNCHER_COL_TYPE", "Type"),
		LStr("LAUNCHER_COL_TITLE", "Title"),
		LStr("LAUNCHER_COL_AUTHOR", "Author(s)"),
		LStr("LAUNCHER_COL_RELEASE", "Release Date"),
		LStr("LAUNCHER_COL_LAST_PLAYED", "Last Played"),
		LStr("LAUNCHER_COL_PLAYTIME", "Playtime")
	});
	for (int col = 0; col < 6; ++col)
	{
		if (QTableWidgetItem* hItem = table->horizontalHeaderItem(col))
			hItem->setTextAlignment((col == 1 || col == 2) ? (Qt::AlignLeft | Qt::AlignVCenter) : Qt::AlignCenter);
	}

	btnPlay->setText(LStr("PICKER_PLAY", "Play Game"));
	btnJoin->setText(LStr("LAUNCHER_JOIN_GAME", "Join Game"));
	btnHost->setText(LStr("LAUNCHER_HOST_GAME", "Host Game"));
	btnSettings->setText(LStr("LAUNCHER_PROFILE_SETTINGS", "Profile Settings ..."));
	btnMoveUp->setText(LStr("LAUNCHER_MOVE_UP", "Move Up"));
	btnMoveDown->setText(LStr("LAUNCHER_MOVE_DOWN", "Move Down"));
	btnRefresh->setText(LStr("LAUNCHER_MENU_REFRESH", "Refresh Profiles"));

	for (int i = 0; i < table->rowCount() && i < static_cast<int>(profiles.size()); ++i)
		updateTableRow(i, i);
	applyFilter();
	updateDetailsPane();
	updateButtonStates();
}

void LauncherMainWindow::refreshTable(int selectProfileIndexValue)
{
	table->blockSignals(true);
	table->setRowCount(static_cast<int>(profiles.size()));
	for (int i = 0; i < static_cast<int>(profiles.size()); ++i)
		updateTableRow(i, i);
	table->blockSignals(false);

	applyFilter();

	if (selectProfileIndexValue >= 0 && selectProfileIndexValue < static_cast<int>(profiles.size()))
		selectProfileIndex(selectProfileIndexValue);
	else if (!profiles.empty())
		selectProfileIndex(0);
	else
	{
		updateDetailsPane();
		updateButtonStates();
	}
}

void LauncherMainWindow::updateTableRow(int tableRow, int profileIdx)
{
	if (tableRow < 0 || tableRow >= table->rowCount() || profileIdx < 0 || profileIdx >= static_cast<int>(profiles.size()))
		return;

	const Profile& p = profiles[profileIdx];
	auto makeItem = [&](const QString& text, Qt::Alignment align = Qt::AlignLeft | Qt::AlignVCenter)
	{
		auto* item = new QTableWidgetItem(text);
		item->setData(Qt::UserRole, profileIdx);
		item->setTextAlignment(align);
		return item;
	};

	table->setItem(tableRow, 0, makeItem(p.isIWAD ? "IWAD" : "(P)WAD", Qt::AlignCenter));
	table->setItem(tableRow, 1, makeItem(QString::fromStdString(p.title)));
	table->setItem(tableRow, 2, makeItem(QString::fromStdString(p.author)));
	table->setItem(tableRow, 3, makeItem(QString::fromStdString(p.releaseDate), Qt::AlignCenter));
	const QString lastPlayedText = (p.lastPlayed.empty() || p.lastPlayed == "Never")
		? LStr("LAUNCHER_NEVER", "Never")
		: QString::fromStdString(p.lastPlayed);
	table->setItem(tableRow, 4, makeItem(lastPlayedText, Qt::AlignCenter));
	table->setItem(tableRow, 5, makeItem(formatPlaytime(p.playtime), Qt::AlignCenter));
}

int LauncherMainWindow::selectedProfileIndex() const
{
	const int row = table->currentRow();
	if (row < 0 || table->isRowHidden(row))
		return -1;
	const QTableWidgetItem* item = table->item(row, 0);
	if (!item)
		return -1;
	const int idx = item->data(Qt::UserRole).toInt();
	if (idx < 0 || idx >= static_cast<int>(profiles.size()))
		return -1;
	return idx;
}

void LauncherMainWindow::selectProfileIndex(int profileIdx)
{
	if (profileIdx < 0 || profileIdx >= table->rowCount())
		return;

	int targetRow = profileIdx;
	if (table->isRowHidden(targetRow))
	{
		targetRow = -1;
		for (int r = 0; r < table->rowCount(); ++r)
		{
			if (!table->isRowHidden(r))
			{
				targetRow = r;
				break;
			}
		}
	}

	if (targetRow >= 0)
	{
		table->selectRow(targetRow);
		if (const auto* item = table->item(targetRow, 0))
			table->scrollToItem(item);
	}
	else
	{
		table->clearSelection();
	}
	updateDetailsPane();
	updateButtonStates();
}

void LauncherMainWindow::applyFilter()
{
	const QString filter = searchEdit->text().trimmed();
	int visibleCount = 0;

	for (int r = 0; r < table->rowCount() && r < static_cast<int>(profiles.size()); ++r)
	{
		const Profile& p = profiles[r];
		bool match = filter.isEmpty();
		if (!match)
		{
			const QString title = QString::fromStdString(p.title);
			const QString author = QString::fromStdString(p.author);
			const QString type = p.isIWAD ? QString("IWAD") : QString("(P)WAD");
			match = title.contains(filter, Qt::CaseInsensitive)
				|| author.contains(filter, Qt::CaseInsensitive)
				|| type.contains(filter, Qt::CaseInsensitive);
		}
		table->setRowHidden(r, !match);
		if (match)
			++visibleCount;
	}

	if (filter.isEmpty())
		statusCountLabel->setText(LStr("LAUNCHER_AVAILABLE_PROFILES", "Available Profiles:") + QString(" %1").arg(profiles.size()));
	else
		statusCountLabel->setText(LStr("LAUNCHER_SHOWING_PROFILES", "Showing %1 of %2 profiles").arg(visibleCount).arg(profiles.size()));
}

void LauncherMainWindow::updateCoverPixmapScale()
{
	if (cachedCoverPixmap.isNull() && !usingFallbackCover)
		return;

	coverLabel->setFrameShape(QFrame::StyledPanel);
	coverLabel->setFrameShadow(QFrame::Sunken);
	const int fw = coverLabel->frameWidth();
	const int h = std::max(64, coverLabel->height());
	const int innerH = std::max(64, h - 2 * fw);
	const double aspect = (!usingFallbackCover && cachedCoverPixmap.height() > 0)
		? (static_cast<double>(cachedCoverPixmap.width()) / static_cast<double>(cachedCoverPixmap.height()))
		: (4.0 / 3.0);
	const int innerW = std::clamp(qRound(innerH * aspect), 160, 320);
	const int targetW = innerW + 2 * fw;
	if (coverLabel->width() != targetW)
		coverLabel->setFixedWidth(targetW);

	const qreal dpr = std::max<qreal>(1.0, coverLabel->devicePixelRatioF());
	if (usingFallbackCover)
	{
		coverLabel->setPixmap(loadUZDoomGenericThumbnailPixmap(innerW, innerH, dpr));
		coverLabel->setText(QString());
		return;
	}

	const QSize targetPhys(
		std::max(64, qRound(innerW * dpr)),
		std::max(64, qRound(innerH * dpr)));
	QPixmap scaled = cachedCoverPixmap.scaled(
		targetPhys,
		Qt::IgnoreAspectRatio,
		Qt::SmoothTransformation);
	scaled.setDevicePixelRatio(dpr);
	coverLabel->setPixmap(scaled);
	coverLabel->setText(QString());
}

void LauncherMainWindow::updateDetailsPane()
{
	const int idx = selectedProfileIndex();
	if (idx < 0)
	{
		cachedCoverPath.clear();
		cachedCoverPixmap = QPixmap();
		usingFallbackCover = false;
		coverLabel->setFixedWidth(240);
		coverLabel->setFrameShape(QFrame::StyledPanel);
		coverLabel->setPixmap(QPixmap());
		coverLabel->setText(profiles.empty() ? "UZDoom" : LStr("LAUNCHER_NO_SELECTION", "No Profile Selected"));
		descView->setPlainText(profiles.empty()
			? LStr("LAUNCHER_EMPTY_HINT", "No profiles found yet.\nDrop a .wad, .pk3, .ipk3, .zip, or .uzdp file here, or click 'Add/Import New Entry' above.")
			: QString());
		return;
	}

	const Profile& p = profiles[idx];
	if (p.coverImagePath != cachedCoverPath)
	{
		cachedCoverPath = p.coverImagePath;
		cachedCoverPixmap = (!p.coverImagePath.empty() && QFileInfo::exists(QString::fromStdString(p.coverImagePath)))
			? QPixmap(QString::fromStdString(p.coverImagePath))
			: QPixmap();
	}

	usingFallbackCover = cachedCoverPixmap.isNull();
	updateCoverPixmapScale();

	if (!p.description.empty())
		descView->setPlainText(QString::fromStdString(p.description));
	else
		descView->setPlainText(LStr("LAUNCHER_NO_DESCRIPTION", "No description provided for this profile."));
}

void LauncherMainWindow::updateButtonStates()
{
	const bool isRunning = (gameProcess != nullptr);
	const int idx = selectedProfileIndex();
	const bool hasSel = (idx >= 0);
	const bool canMove = hasSel && searchEdit->text().trimmed().isEmpty();

	addMenu->menuAction()->setEnabled(!isRunning);
	prefMenu->menuAction()->setEnabled(!isRunning);
	btnPlay->setEnabled(!isRunning && hasSel);
	btnJoin->setEnabled(!isRunning && hasSel);
	btnHost->setEnabled(!isRunning && hasSel);
	btnSettings->setEnabled(!isRunning && hasSel);
	btnMoveUp->setEnabled(!isRunning && canMove && idx > 0);
	btnMoveDown->setEnabled(!isRunning && canMove && idx + 1 < static_cast<int>(profiles.size()));
	btnRefresh->setEnabled(!isRunning);
#ifdef HAS_UPDATER
	if (updater)
		updater->setBannerEnabled(!isRunning);
#endif
}

void LauncherMainWindow::setRunningState(bool running, const QString& title)
{
	statusRunLabel->setText(running ? (LStr("LAUNCHER_RUNNING", "Running:") + " " + title) : QString());
	updateButtonStates();
}

void LauncherMainWindow::onSelectionChanged()
{
	const int idx = selectedProfileIndex();
	if (idx >= 0)
		launcherConfig.lastSelectedRow = idx;
	updateDetailsPane();
	updateButtonStates();
}

void LauncherMainWindow::onFilterTextChanged(const QString&)
{
	applyFilter();
	if (selectedProfileIndex() < 0 && !profiles.empty())
		selectProfileIndex(std::clamp(launcherConfig.lastSelectedRow, 0, static_cast<int>(profiles.size()) - 1));
	else
	{
		updateDetailsPane();
		updateButtonStates();
	}
}

void LauncherMainWindow::moveSelectedProfile(int delta)
{
	if (gameProcess)
		return;
	const int idx = selectedProfileIndex();
	if (idx < 0)
		return;
	const int target = idx + delta;
	if (target < 0 || target >= static_cast<int>(profiles.size()))
		return;

	std::swap(profiles[idx], profiles[target]);
	persistProfileOrdering(profiles);
	launcherConfig.lastSelectedRow = target;
	refreshTable(target);
}

void LauncherMainWindow::onMoveUpClicked()
{
	moveSelectedProfile(-1);
}

void LauncherMainWindow::onMoveDownClicked()
{
	moveSelectedProfile(1);
}

void LauncherMainWindow::onPlayClicked()
{
	launchProfile(LaunchTargetMode::Single);
}

void LauncherMainWindow::onJoinClicked()
{
	launchProfile(LaunchTargetMode::Join);
}

void LauncherMainWindow::onHostClicked()
{
	launchProfile(LaunchTargetMode::Host);
}

void LauncherMainWindow::recordRunningSessionPlaytime()
{
	if (runningProfileIndex < 0 || runningProfileIndex >= static_cast<int>(profiles.size()))
		return;

	const qint64 elapsedSecs = playTimer.isValid() ? (playTimer.elapsed() / 1000) : 0;
	Profile& prof = profiles[runningProfileIndex];
	prof.lastPlayed = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm").toStdString();
	if (elapsedSecs >= 30)
		prof.playtime += std::max<uint64_t>(1, static_cast<uint64_t>((elapsedSecs + 30) / 60));
	saveProfileToDisk(prof);
	updateTableRow(runningProfileIndex, runningProfileIndex);
	runningProfileIndex = -1;
}

void LauncherMainWindow::launchProfile(LaunchTargetMode mode)
{
	if (gameProcess)
		return;

	const int idx = selectedProfileIndex();
	if (idx < 0 || idx >= static_cast<int>(profiles.size()))
		return;

	const Profile& p = profiles[idx];
	if (p.iwadPath.empty() || !QFileInfo::exists(QString::fromStdString(p.iwadPath)))
	{
		const auto choice = QMessageBox::warning(
			this,
			LStr("LAUNCHER_MISSING_IWAD_TITLE", "Missing IWAD"),
			LStr("LAUNCHER_MISSING_IWAD_MSG", "This profile does not have a valid IWAD file selected. Open Profile Settings now?"),
			QMessageBox::Yes | QMessageBox::No,
			QMessageBox::Yes);
		if (choice == QMessageBox::Yes)
			openSettingsForIndex(idx);
		return;
	}

	if (mode == LaunchTargetMode::Join && QString::fromStdString(p.remoteAddress).trimmed().isEmpty())
	{
		const auto choice = QMessageBox::warning(
			this,
			LStr("LAUNCHER_MISSING_HOST_TITLE", "Missing Host Address"),
			LStr("LAUNCHER_MISSING_HOST_MSG", "Please set a remote Host IP / Domain in Profile Settings before joining a multiplayer game. Open Profile Settings now?"),
			QMessageBox::Yes | QMessageBox::No,
			QMessageBox::Yes);
		if (choice == QMessageBox::Yes)
			openSettingsForIndex(idx);
		return;
	}

	if (p.recordDemo && !p.selectedLaunchDemoRecord.empty() && !p.demoDirPath.empty())
		QDir().mkpath(QString::fromStdString(p.demoDirPath));

	QStringList args = p.buildLaunchArguments(mode, launcherConfig.language);
	args.removeAll("-showlauncher");

	runningProfileIndex = idx;
	gameProcess = new QProcess(this);
	gameProcess->setWorkingDirectory(QFileInfo(QCoreApplication::applicationFilePath()).absolutePath());

	connect(gameProcess, &QProcess::errorOccurred, this, [this](QProcess::ProcessError err) {
		if (err == QProcess::FailedToStart)
		{
			QMessageBox::critical(this, "Launch Failed",
				"Failed to launch UZDoom executable:\n" + QCoreApplication::applicationFilePath());
		}
	});

	connect(gameProcess, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
		[this](int, QProcess::ExitStatus) {
			if (wasHiddenForLaunch)
			{
				if (wasMaximizedBeforeHide)
					showMaximized();
				else
					showNormal();
				raise();
				activateWindow();
				wasHiddenForLaunch = false;
				wasMaximizedBeforeHide = false;
				QApplication::setQuitOnLastWindowClosed(true);
			}

			recordRunningSessionPlaytime();

			if (gameProcess)
			{
				gameProcess->deleteLater();
				gameProcess = nullptr;
			}
			runningProfileIndex = -1;
			setRunningState(false);
		});

	playTimer.restart();
	setRunningState(true, QString::fromStdString(p.title));
	gameProcess->start(QCoreApplication::applicationFilePath(), args);

	if (gameProcess->state() == QProcess::NotRunning && gameProcess->error() == QProcess::FailedToStart)
	{
		gameProcess->deleteLater();
		gameProcess = nullptr;
		runningProfileIndex = -1;
		setRunningState(false);
		return;
	}

	if (launcherConfig.hideWhilePlaying)
	{
		wasMaximizedBeforeHide = isMaximized();
		wasHiddenForLaunch = true;
		QApplication::setQuitOnLastWindowClosed(false);
		hide();
	}
}

void LauncherMainWindow::openSettingsForIndex(int profileIdx)
{
	if (gameProcess || profileIdx < 0 || profileIdx >= static_cast<int>(profiles.size()))
		return;

	Profile workingCopy = profiles[profileIdx];
	ProfileSettingsDialog dlg(
		workingCopy,
		profiles,
		QString::fromStdString(launcherConfig.defaultModsDir),
		launcherConfig.language,
		this);
	dlg.exec();

	switch (dlg.actionResult())
	{
	case ProfileSettingsDialog::ActionResult::Saved:
		saveProfileToDisk(workingCopy);
		profiles[profileIdx] = std::move(workingCopy);
		cachedCoverPath.clear();
		updateTableRow(profileIdx, profileIdx);
		applyFilter();
		selectProfileIndex(profileIdx);
		break;

	case ProfileSettingsDialog::ActionResult::Cloned:
		profiles.push_back(dlg.clonedProfile());
		persistProfileOrdering(profiles);
		launcherConfig.lastSelectedRow = static_cast<int>(profiles.size()) - 1;
		refreshTable(launcherConfig.lastSelectedRow);
		break;

	case ProfileSettingsDialog::ActionResult::Deleted:
		profiles.erase(profiles.begin() + profileIdx);
		persistProfileOrdering(profiles);
		launcherConfig.lastSelectedRow = std::min(profileIdx, static_cast<int>(profiles.size()) - 1);
		refreshTable(launcherConfig.lastSelectedRow);
		break;

	case ProfileSettingsDialog::ActionResult::Cancelled:
	default:
		break;
	}
}

void LauncherMainWindow::onProfileSettingsClicked()
{
	openSettingsForIndex(selectedProfileIndex());
}

void LauncherMainWindow::onAddGameFiles()
{
	if (gameProcess)
		return;
	const QStringList files = QFileDialog::getOpenFileNames(
		this,
		LStr("LAUNCHER_MENU_ADD_FILE", "Add WAD / Mod / Archive"),
		QString::fromStdString(LauncherConst::EXEC_DIR),
		"Doom Files (*.wad *.iwad *.pwad *.pk3 *.ipk3 *.pk7 *.ipk7 *.deh *.bex *.zip *.7z *.uzdp);;All Files (*.*)");
	if (!files.isEmpty())
		handleImportedFiles(files);
}

void LauncherMainWindow::onCreateBlankProfile()
{
	if (gameProcess)
		return;

	bool ok = false;
	const QString title = QInputDialog::getText(
		this,
		LStr("LAUNCHER_MENU_NEW_BLANK", "Create Blank Profile"),
		LStr("LAUNCHER_ENTER_PROFILE_NAME", "Profile title:"),
		QLineEdit::Normal,
		"New Profile",
		&ok).trimmed();
	if (!ok || title.isEmpty())
		return;

	Profile p;
	p.title = title.toStdString();
	p.isIWAD = false;
	p.iwadPath = findBestDefaultIWADPath(profiles);
	p.orderIndex = static_cast<int>(profiles.size());
	createDefaultDirectoryStructure(p);
	saveProfileToDisk(p);

	profiles.push_back(std::move(p));
	launcherConfig.lastSelectedRow = static_cast<int>(profiles.size()) - 1;
	refreshTable(launcherConfig.lastSelectedRow);
	openSettingsForIndex(launcherConfig.lastSelectedRow);
}

void LauncherMainWindow::handleImportedFiles(const QStringList& localPaths)
{
	if (localPaths.isEmpty())
		return;

	int lastCreatedIdx = -1;
	int addedCount = 0;
	int skippedCount = 0;
	QStringList errors;

	for (const QString& localPath : localPaths)
	{
		const QFileInfo fi(localPath);
		const QString ext = fi.suffix().toLower();
		const int curIdx = selectedProfileIndex();

		if ((ext == "deh" || ext == "bex") && curIdx >= 0)
		{
			const auto ans = QMessageBox::question(
				this,
				"Attach DeHackEd Patch",
				QString("Attach '%1' as a mod to selected profile '%2'?\n\nClick 'No' to create a separate profile instead.")
					.arg(fi.fileName())
					.arg(QString::fromStdString(profiles[curIdx].title)),
				QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
				QMessageBox::Yes);
			if (ans == QMessageBox::Cancel)
				continue;
			if (ans == QMessageBox::Yes)
			{
				QString finalModPath = localPath;
				bool cancelledAttach = false;
				if (profiles[curIdx].copyModsToProfile && !profiles[curIdx].filePath.empty())
				{
					const QDir modsDir(QFileInfo(QString::fromStdString(profiles[curIdx].filePath)).absolutePath() + "/mods");
					modsDir.mkpath(".");
					const QString dest = modsDir.filePath(fi.fileName());
					const QString srcAbs = fi.absoluteFilePath();
					const QString destAbs = QFileInfo(dest).absoluteFilePath();
					if (srcAbs.compare(destAbs, Qt::CaseInsensitive) != 0)
					{
						if (!QFile::exists(dest))
						{
							if (QFile::copy(localPath, dest))
								finalModPath = dest;
						}
						else
						{
							const std::string srcMd5 = calculateFileMD5(srcAbs.toStdString());
							const std::string dstMd5 = calculateFileMD5(destAbs.toStdString());
							if (!srcMd5.empty() && srcMd5 == dstMd5)
							{
								finalModPath = dest;
							}
							else
							{
								QMessageBox box(this);
								box.setIcon(QMessageBox::Question);
								box.setWindowTitle(LStr("LAUNCHER_MOD_EXISTS_TITLE", "Mod Already Exists"));
								box.setText(QString("A different file named '%1' already exists in '%2''s mods folder.\n\nOverwrite it with the selected file?")
									.arg(fi.fileName())
									.arg(QString::fromStdString(profiles[curIdx].title)));
								QPushButton* btnOverwrite = box.addButton(LStr("LAUNCHER_OVERWRITE", "Overwrite"), QMessageBox::AcceptRole);
								QPushButton* btnKeep = box.addButton(LStr("LAUNCHER_KEEP_EXISTING", "Keep Existing"), QMessageBox::NoRole);
								box.addButton(QMessageBox::Cancel);
								box.setDefaultButton(btnOverwrite);
								box.exec();

								if (box.clickedButton() == btnOverwrite)
								{
									const QString tmpDest = dest + ".uztmp";
									QFile::remove(tmpDest);
									if (!QFile::copy(localPath, tmpDest) || !QFile::remove(dest) || !QFile::rename(tmpDest, dest))
									{
										QFile::remove(tmpDest);
										errors << QString("%1: Could not overwrite existing mod in profile folder.").arg(fi.fileName());
										cancelledAttach = true;
									}
									else
									{
										finalModPath = dest;
									}
								}
								else if (box.clickedButton() == btnKeep)
								{
									finalModPath = dest;
								}
								else
								{
									cancelledAttach = true;
								}
							}
						}
					}
				}
				if (cancelledAttach)
					continue;
				const std::string modStd = finalModPath.toStdString();
				if (std::find(profiles[curIdx].mods.begin(), profiles[curIdx].mods.end(), modStd) == profiles[curIdx].mods.end())
					profiles[curIdx].mods.push_back(modStd);
				saveProfileToDisk(profiles[curIdx]);
				++addedCount;
				lastCreatedIdx = curIdx;
				continue;
			}
		}

		const ImportOutcome outcome = importGameFileIntoProfiles(localPath.toStdString(), profiles);
		if (outcome.status == ImportResult::Success)
		{
			Profile p = outcome.createdProfile;
			p.orderIndex = static_cast<int>(profiles.size());
			saveProfileToDisk(p);
			profiles.push_back(std::move(p));
			lastCreatedIdx = static_cast<int>(profiles.size()) - 1;
			++addedCount;
		}
		else if (outcome.status == ImportResult::DuplicateProfile)
		{
			++skippedCount;
		}
		else if (outcome.status != ImportResult::Cancelled)
		{
			errors << QString("%1: %2").arg(fi.fileName()).arg(QString::fromStdString(outcome.message));
		}
	}

	if (lastCreatedIdx >= 0)
	{
		persistProfileOrdering(profiles);
		launcherConfig.lastSelectedRow = lastCreatedIdx;
		refreshTable(lastCreatedIdx);
	}

	if (!errors.isEmpty())
	{
		QMessageBox::warning(this, "Import Issue", errors.join("\n"));
	}
	else if (skippedCount > 0 && addedCount == 0)
	{
		QMessageBox::information(this, "Duplicate Profile",
			"The selected file already matches an existing profile.");
	}
}

void LauncherMainWindow::startStartupIWadDetection()
{
	startIWadAutoDetect(false);
}

void LauncherMainWindow::startStartupUpdateCheck()
{
#ifdef HAS_UPDATER
	if (updater)
		updater->checkForUpdate(false, this);
#endif
}

void LauncherMainWindow::startIWadAutoDetect(bool interactive)
{
	if (detectThread)
	{
		if (detectThread->isRunning())
		{
			if (interactive)
				detectInteractive = true;
			return;
		}
		detectThread->wait();
		detectThread = nullptr;
	}

	detectInteractive = interactive;
	detectCancel = std::make_shared<std::atomic<bool>>(false);

	std::vector<Profile> profilesSnapshot = profiles;
	std::vector<DiscoveredIWad> engineIWads = g_discoveredEngineIWads;
	std::vector<std::string> extraDirs = {LauncherConst::EXEC_DIR, LauncherConst::ROOT_DIR};
	auto cancelFlag = detectCancel;

	if (!gameProcess)
		statusRunLabel->setText(LStr("LAUNCHER_SCANNING_IWADS", "Scanning for IWADs..."));

	QThread* thread = QThread::create([this,
		cancelFlag,
		profilesSnapshot = std::move(profilesSnapshot),
		engineIWads = std::move(engineIWads),
		extraDirs = std::move(extraDirs)]() mutable
	{
		std::vector<DiscoveredIWadCandidate> candidates = scanUnregisteredIWADs(
			profilesSnapshot,
			engineIWads,
			extraDirs,
			cancelFlag.get());

		if (cancelFlag->load(std::memory_order_relaxed))
			return;

		QMetaObject::invokeMethod(this, [this, cancelFlag, candidates = std::move(candidates)]()
		{
			if (cancelFlag->load(std::memory_order_relaxed))
				return;
			onIWadScanFinished(candidates);
		}, Qt::QueuedConnection);
	});

	detectThread = thread;
	QPointer<QThread> threadGuard(thread);
	connect(thread, &QThread::finished, this, [this, threadGuard]()
	{
		if (detectThread == threadGuard)
			detectThread = nullptr;
	});
	connect(thread, &QThread::finished, thread, &QObject::deleteLater);
	thread->start();
}

void LauncherMainWindow::onIWadScanFinished(const std::vector<DiscoveredIWadCandidate>& candidates)
{
	if (!gameProcess)
		statusRunLabel->clear();

	const bool interactive = detectInteractive;
	detectInteractive = false;

	const int prevIdx = selectedProfileIndex();
	const int found = commitDiscoveredIWADs(profiles, candidates);
	if (found > 0)
	{
		persistProfileOrdering(profiles);
		refreshTable(prevIdx >= 0 ? prevIdx : 0);
	}

	if (interactive)
	{
		QMessageBox::information(
			this,
			LStr("LAUNCHER_MENU_AUTODETECT", "Auto-Detect IWADs"),
			found > 0
				? QString("Detected and imported %1 new IWAD profile(s).").arg(found)
				: QString("No new IWADs were found."));
	}
}

void LauncherMainWindow::onRefreshProfiles()
{
	if (gameProcess)
		return;
	const int prev = selectedProfileIndex();
	profiles = loadAllProfiles();
	refreshTable(prev);
	startIWadAutoDetect(false);
}

void LauncherMainWindow::onOpenLauncherFolder()
{
	QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(LauncherConst::ROOT_DIR)));
}

void LauncherMainWindow::onOpenExecFolder()
{
	QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(LauncherConst::EXEC_DIR)));
}

void LauncherMainWindow::onOpenSelectedProfileFolder()
{
	const int idx = selectedProfileIndex();
	if (idx < 0)
		return;
	const QFileInfo fi(QString::fromStdString(profiles[idx].filePath));
	const QString dir = fi.absolutePath();
	if (!dir.isEmpty())
		QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
}

void LauncherMainWindow::onCloneSelectedProfile()
{
	if (gameProcess)
		return;
	const int idx = selectedProfileIndex();
	if (idx < 0)
		return;

	bool ok = false;
	const QString newTitle = QInputDialog::getText(
		this,
		LStr("LAUNCHER_CLONE_PROFILE", "Clone Profile"),
		"New profile title:",
		QLineEdit::Normal,
		QString::fromStdString(profiles[idx].title) + " (Copy)",
		&ok).trimmed();
	if (!ok || newTitle.isEmpty())
		return;

	Profile cloned;
	if (!cloneProfile(profiles[idx], newTitle.toStdString(), cloned))
	{
		QMessageBox::warning(this, "Clone Failed", "Could not clone the selected profile.");
		return;
	}
	profiles.push_back(std::move(cloned));
	persistProfileOrdering(profiles);
	launcherConfig.lastSelectedRow = static_cast<int>(profiles.size()) - 1;
	refreshTable(launcherConfig.lastSelectedRow);
}

void LauncherMainWindow::onExportSelectedProfile()
{
	const int idx = selectedProfileIndex();
	if (idx < 0)
		return;

	const Profile& p = profiles[idx];
	const QString suggested = QString::fromStdString(sanitizeFolderName(p.title)) + ".uzdp";
	const QString dest = QFileDialog::getSaveFileName(
		this,
		LStr("LAUNCHER_EXPORT_PROFILE", "Export Profile (.uzdp)"),
		QDir::home().filePath(suggested),
		"UZDoom Profile Archive (*.uzdp)");
	if (dest.isEmpty())
		return;

	std::string err;
	if (!exportProfileToZip(p, dest.toStdString(), err))
		QMessageBox::warning(this, "Export Failed", QString::fromStdString(err));
	else
		QMessageBox::information(this, "Export Complete", "Exported profile to:\n" + dest);
}

void LauncherMainWindow::onDeleteSelectedProfile()
{
	if (gameProcess)
		return;
	const int idx = selectedProfileIndex();
	if (idx < 0)
		return;

	const auto ans = QMessageBox::warning(
		this,
		LStr("LAUNCHER_DELETE_PROFILE", "Delete Profile"),
		QString("Delete profile '%1' and its profile folder?\nExternal IWAD files outside the profile folder are preserved.")
			.arg(QString::fromStdString(profiles[idx].title)),
		QMessageBox::Yes | QMessageBox::No,
		QMessageBox::No);
	if (ans != QMessageBox::Yes)
		return;

	deleteProfileDirectory(profiles[idx]);
	profiles.erase(profiles.begin() + idx);
	persistProfileOrdering(profiles);
	launcherConfig.lastSelectedRow = std::min(idx, static_cast<int>(profiles.size()) - 1);
	refreshTable(launcherConfig.lastSelectedRow);
}

void LauncherMainWindow::onTableContextMenu(const QPoint& pos)
{
	const QTableWidgetItem* hit = table->itemAt(pos);
	if (!hit)
		return;
	table->selectRow(hit->row());

	const int idx = selectedProfileIndex();
	if (idx < 0)
		return;

	const bool idle = (gameProcess == nullptr);
	const bool canMove = idle && searchEdit->text().trimmed().isEmpty();

	QMenu menu(this);
	QAction* actPlay = menu.addAction(btnPlay->text(), this, &LauncherMainWindow::onPlayClicked);
	QAction* actJoin = menu.addAction(btnJoin->text(), this, &LauncherMainWindow::onJoinClicked);
	QAction* actHost = menu.addAction(btnHost->text(), this, &LauncherMainWindow::onHostClicked);
	actPlay->setEnabled(idle);
	actJoin->setEnabled(idle);
	actHost->setEnabled(idle);

	menu.addSeparator();
	QAction* actSettings = menu.addAction(btnSettings->text(), this, &LauncherMainWindow::onProfileSettingsClicked);
	QAction* actClone = menu.addAction(LStr("LAUNCHER_CLONE_PROFILE", "Clone Profile..."), this, &LauncherMainWindow::onCloneSelectedProfile);
	menu.addAction(LStr("LAUNCHER_EXPORT_PROFILE", "Export Profile (.uzdp)..."), this, &LauncherMainWindow::onExportSelectedProfile);
	menu.addAction(LStr("LAUNCHER_OPEN_PROFILE_DIR", "Open Profile Folder"), this, &LauncherMainWindow::onOpenSelectedProfileFolder);
	actSettings->setEnabled(idle);
	actClone->setEnabled(idle);

	menu.addSeparator();
	QAction* actUp = menu.addAction(btnMoveUp->text(), this, &LauncherMainWindow::onMoveUpClicked);
	QAction* actDown = menu.addAction(btnMoveDown->text(), this, &LauncherMainWindow::onMoveDownClicked);
	actUp->setEnabled(canMove && idx > 0);
	actDown->setEnabled(canMove && idx + 1 < static_cast<int>(profiles.size()));

	menu.addSeparator();
	QAction* actDel = menu.addAction(LStr("LAUNCHER_DELETE_PROFILE", "Delete Profile..."), this, &LauncherMainWindow::onDeleteSelectedProfile);
	actDel->setEnabled(idle);

	menu.exec(table->viewport()->mapToGlobal(pos));
}

void LauncherMainWindow::onAboutClicked()
{
#ifdef HAS_UPDATER
	std::function<void(QWidget*)> checkUpdatesCb;
	if (updater && !gameProcess)
	{
		checkUpdatesCb = [this](QWidget* dlgParent) {
			if (updater && !gameProcess)
				updater->checkForUpdate(true, dlgParent);
		};
	}
	AboutDialog::showAdditionalInfo(launcherConfig.showReleaseNotes, false, this, std::move(checkUpdatesCb), showUpcomingReleaseNotes);
#else
	AboutDialog::showAdditionalInfo(launcherConfig.showReleaseNotes, false, this, {}, showUpcomingReleaseNotes);
#endif
	saveLauncherConfig(launcherConfig);
}

void LauncherMainWindow::onLanguageTriggered(QAction* action)
{
	if (!action)
		return;
	const std::string code = action->data().toString().toStdString();
	launcherConfig.language = code;
	GStrings.UpdateLanguage(code.c_str());
	saveLauncherConfig(launcherConfig);
	retranslateUi();
}

void LauncherMainWindow::onThemeTriggered(QAction* action)
{
	if (!action)
		return;
	launcherConfig.theme = action->data().toString().toStdString();
	applyLauncherTheme(launcherConfig.theme);
	saveLauncherConfig(launcherConfig);
}

void LauncherMainWindow::updateDefaultModsMenuState()
{
	const bool hasDir = !launcherConfig.defaultModsDir.empty();
	if (actSetDefaultModsDir)
	{
		actSetDefaultModsDir->setToolTip(hasDir
			? QString("Current Default Mods Folder:\n%1").arg(QString::fromStdString(launcherConfig.defaultModsDir))
			: QString("Choose a master folder that 'Add Mod File' opens in."));
	}
	if (actClearDefaultModsDir)
	{
		actClearDefaultModsDir->setVisible(hasDir);
		actClearDefaultModsDir->setEnabled(hasDir);
	}
}

void LauncherMainWindow::onSetDefaultModsDir()
{
	const QString startDir = QString::fromStdString(
		launcherConfig.defaultModsDir.empty() ? LauncherConst::EXEC_DIR : launcherConfig.defaultModsDir);
	const QString picked = QFileDialog::getExistingDirectory(this, "Select Default Mods Folder", startDir);
	if (picked.isEmpty())
		return;
	launcherConfig.defaultModsDir = QDir::cleanPath(picked).toStdString();
	saveLauncherConfig(launcherConfig);
	updateDefaultModsMenuState();
}

void LauncherMainWindow::onClearDefaultModsDir()
{
	launcherConfig.defaultModsDir.clear();
	saveLauncherConfig(launcherConfig);
	updateDefaultModsMenuState();
}

void LauncherMainWindow::onToggleHideWhilePlaying(bool checked)
{
	launcherConfig.hideWhilePlaying = checked;
	saveLauncherConfig(launcherConfig);
}

#ifdef HAS_UPDATER
void LauncherMainWindow::updateUpdaterMenuState()
{
	if (!updater)
		return;

	const int mode = static_cast<int>(updater->currentMode());
	if (updaterModeGroup)
	{
		for (QAction* act : updaterModeGroup->actions())
			act->setChecked(act->data().toInt() == mode);
	}

	const int days = updater->currentIntervalDays();
	if (updaterIntervalGroup)
	{
		for (QAction* act : updaterIntervalGroup->actions())
			act->setChecked(act->data().toInt() == days);
	}
}

void LauncherMainWindow::onUpdaterModeTriggered(QAction* action)
{
	if (!action || !updater)
		return;
	updater->setMode(static_cast<UpdaterMode>(action->data().toInt()));
}

void LauncherMainWindow::onUpdaterIntervalTriggered(QAction* action)
{
	if (!action || !updater)
		return;
	updater->setIntervalDays(action->data().toInt());
}
#endif

void LauncherMainWindow::setShowReleaseNotes(bool show)
{
	launcherConfig.showReleaseNotes = show;
	saveLauncherConfig(launcherConfig);
}

void LauncherMainWindow::dragEnterEvent(QDragEnterEvent* event)
{
	if (!gameProcess && hasSupportedLocalFiles(event->mimeData()))
		event->acceptProposedAction();
}

void LauncherMainWindow::dropEvent(QDropEvent* event)
{
	if (gameProcess)
		return;
	const QStringList files = extractSupportedLocalFiles(event->mimeData());
	if (!files.isEmpty())
	{
		event->acceptProposedAction();
		handleImportedFiles(files);
	}
}

bool LauncherMainWindow::eventFilter(QObject* watched, QEvent* event)
{
	if (watched == coverLabel && (event->type() == QEvent::Resize || event->type() == QEvent::Show))
	{
		if (!cachedCoverPixmap.isNull() || usingFallbackCover)
			updateCoverPixmapScale();
	}
	else if (watched == table && event->type() == QEvent::KeyPress)
	{
		const auto* keyEv = static_cast<QKeyEvent*>(event);
		if (!gameProcess && selectedProfileIndex() >= 0)
		{
			if (keyEv->key() == Qt::Key_Return || keyEv->key() == Qt::Key_Enter)
			{
				onPlayClicked();
				return true;
			}
			if (keyEv->key() == Qt::Key_Delete)
			{
				onDeleteSelectedProfile();
				return true;
			}
		}
	}
	return QMainWindow::eventFilter(watched, event);
}

void LauncherMainWindow::resizeEvent(QResizeEvent* event)
{
	QMainWindow::resizeEvent(event);
	if (!cachedCoverPixmap.isNull() || usingFallbackCover)
		updateCoverPixmapScale();
}

void LauncherMainWindow::closeEvent(QCloseEvent* event)
{
	if (gameProcess && gameProcess->state() != QProcess::NotRunning)
	{
		const auto ans = QMessageBox::question(
			this,
			"Game Running",
			"UZDoom is currently running. Closing the launcher will terminate the active session.\nClose anyway?",
			QMessageBox::Yes | QMessageBox::No,
			QMessageBox::No);
		if (ans != QMessageBox::Yes)
		{
			event->ignore();
			return;
		}
	}

	launcherConfig.maximized = isMaximized();
	if (verticalSplitter)
	{
		const QList<int> sizes = verticalSplitter->sizes();
		if (sizes.size() >= 2 && sizes[0] > 0 && sizes[1] > 0)
		{
			launcherConfig.splitterTopSize = sizes[0];
			launcherConfig.splitterBottomSize = sizes[1];
		}
	}
	saveLauncherConfig(launcherConfig);
	QMainWindow::closeEvent(event);
}
