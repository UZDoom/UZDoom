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
#include <QComboBox>
#include <QCoreApplication>
#include <QDate>
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
#include <QRegularExpression>
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

	QDate parseProfileReleaseDate(const std::string& raw)
	{
		const QString s = QString::fromStdString(raw).trimmed();
		if (s.isEmpty() || s.compare("Unknown", Qt::CaseInsensitive) == 0)
			return {};
		if (QDate d = QDate::fromString(s, "dd-MM-yyyy"); d.isValid())
			return d;
		if (QDate d = QDate::fromString(s, "yyyy-MM-dd"); d.isValid())
			return d;
		if (QDate d = QDate::fromString(s, "d-M-yyyy"); d.isValid())
			return d;
		static const QRegularExpression yearRe("\\b(19\\d\\d|20\\d\\d)\\b");
		const QRegularExpressionMatch m = yearRe.match(s);
		if (m.hasMatch())
			return QDate(m.captured(1).toInt(), 1, 1);
		return {};
	}

	QDateTime parseProfileLastPlayed(const std::string& raw)
	{
		const QString s = QString::fromStdString(raw).trimmed();
		if (s.isEmpty() || s.compare("Never", Qt::CaseInsensitive) == 0)
			return {};
		return QDateTime::fromString(s, "yyyy-MM-dd HH:mm");
	}

	QString normalizePathKey(const QString& p)
	{
		return QDir::cleanPath(p).replace('\\', '/').toLower();
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
		appendLaunchLogFooter(QStringLiteral("=== Process terminated by launcher ==="));
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
	searchLayout->setSpacing(6);

	filterCategoryCombo = new QComboBox(searchContainer);
	filterCategoryCombo->setMinimumHeight(24);
	filterCategoryCombo->addItem(QString(), 0);
	filterCategoryCombo->addItem(QString(), 1);
	filterCategoryCombo->addItem(QString(), 2);
	filterCategoryCombo->addItem(QString(), 3);
	connect(filterCategoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LauncherMainWindow::onFilterCategoryChanged);

	searchEdit = new QLineEdit(searchContainer);
	searchEdit->setClearButtonEnabled(true);
	searchEdit->setFixedWidth(220);
	searchEdit->setMinimumHeight(24);
	searchEdit->setToolTip("Search by title, author, description, IWAD, or mod filenames (space-separated terms).");
	connect(searchEdit, &QLineEdit::textChanged, this, &LauncherMainWindow::onFilterTextChanged);

	searchLayout->addWidget(filterCategoryCombo);
	searchLayout->addWidget(searchEdit);
	mb->setCornerWidget(searchContainer, Qt::TopRightCorner);

	QWidget* central = new QWidget(this);
	QVBoxLayout* rootLayout = new QVBoxLayout(central);
	rootLayout->setContentsMargins(10, 8, 10, 8);
	rootLayout->setSpacing(8);

	QHBoxLayout* centerRow = new QHBoxLayout();
	centerRow->setContentsMargins(0, 0, 0, 4);
	centerRow->setSpacing(10);

	table = new QTableWidget(0, 7, central);
	table->setSelectionBehavior(QAbstractItemView::SelectRows);
	table->setSelectionMode(QAbstractItemView::SingleSelection);
	table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	table->setAlternatingRowColors(true);
	table->setShowGrid(false);
	table->setContextMenuPolicy(Qt::CustomContextMenu);
	table->verticalHeader()->setVisible(false);
	table->verticalHeader()->setDefaultSectionSize(28);
	table->horizontalHeader()->setStretchLastSection(false);
	table->horizontalHeader()->setMinimumSectionSize(28);
	table->horizontalHeader()->setHighlightSections(false);
	table->horizontalHeader()->setSectionsClickable(true);
	table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	table->setColumnWidth(0, 32);
	table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
	table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
	table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
	table->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
	table->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);

	connect(table->horizontalHeader(), &QHeaderView::sectionClicked, this, &LauncherMainWindow::onHeaderSectionClicked);
	connect(table, &QTableWidget::cellClicked, this, &LauncherMainWindow::onTableCellClicked);
	connect(table, &QTableWidget::itemSelectionChanged, this, &LauncherMainWindow::onSelectionChanged);
	connect(table, &QTableWidget::itemDoubleClicked, this, [this](QTableWidgetItem* item) {
		if (item && item->column() == 0)
			return;
		onPlayClicked();
	});
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

	addMenu->setTitle(LStr("LAUNCHER_ADD_ENTRY", "New Entry"));
	prefMenu->setTitle(LStr("LAUNCHER_PREFERENCES", "Preferences"));
	actAbout->setText(LStr("LAUNCHER_MENU_INFO", "Info"));
	searchEdit->setPlaceholderText(LStr("LAUNCHER_SEARCH_PLACEHOLDER", "Filter profiles..."));
	if (filterCategoryCombo && filterCategoryCombo->count() >= 4)
	{
		filterCategoryCombo->setItemText(0, LStr("LAUNCHER_FILTER_ALL", "All"));
		filterCategoryCombo->setItemText(1, QString::fromUtf8("\u2605 ") + LStr("LAUNCHER_FILTER_FAVORITES", "Favorites"));
		filterCategoryCombo->setItemText(2, QStringLiteral("IWADs"));
		filterCategoryCombo->setItemText(3, QStringLiteral("(P)WADs"));
	}

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
		QString::fromUtf8("\u2605"),
		LStr("LAUNCHER_COL_TYPE", "Type"),
		LStr("LAUNCHER_COL_TITLE", "Title"),
		LStr("LAUNCHER_COL_AUTHOR", "Author(s)"),
		LStr("LAUNCHER_COL_RELEASE", "Release Date"),
		LStr("LAUNCHER_COL_LAST_PLAYED", "Last Played"),
		LStr("LAUNCHER_COL_PLAYTIME", "Playtime")
	});
	for (int col = 0; col < 7; ++col)
	{
		if (QTableWidgetItem* hItem = table->horizontalHeaderItem(col))
		{
			hItem->setTextAlignment((col == 2 || col == 3) ? (Qt::AlignLeft | Qt::AlignVCenter) : Qt::AlignCenter);
			if (col == 0)
				hItem->setToolTip(LStr("LAUNCHER_SORT_RESET_TIP", "Pinned favorites stay at the top. Click to reset column sort to custom order."));
		}
	}
	syncSortIndicator();

	btnPlay->setText(LStr("PICKER_PLAY", "Play Game"));
	btnJoin->setText(LStr("LAUNCHER_JOIN_GAME", "Join Game"));
	btnHost->setText(LStr("LAUNCHER_HOST_GAME", "Host Game"));
	btnSettings->setText(LStr("LAUNCHER_PROFILE_SETTINGS", "Profile Settings ..."));
	btnMoveUp->setText(LStr("LAUNCHER_MOVE_UP", "Move Up"));
	btnMoveDown->setText(LStr("LAUNCHER_MOVE_DOWN", "Move Down"));
	btnRefresh->setText(LStr("LAUNCHER_MENU_REFRESH", "Refresh Profiles"));

	for (int r = 0; r < table->rowCount(); ++r)
	{
		const int idx = profileIndexForTableRow(r);
		if (idx >= 0)
			updateTableRow(r, idx);
	}
	applyFilter();
	updateDetailsPane();
	updateButtonStates();
}

void LauncherMainWindow::syncSortIndicator()
{
	const bool sorted = (launcherConfig.sortColumn >= 1 && launcherConfig.sortColumn <= 6);
	table->horizontalHeader()->setSortIndicatorShown(sorted);
	if (sorted)
	{
		table->horizontalHeader()->setSortIndicator(
			launcherConfig.sortColumn,
			launcherConfig.sortDescending ? Qt::DescendingOrder : Qt::AscendingOrder);
	}
}

std::vector<int> LauncherMainWindow::buildDisplayOrder() const
{
	std::vector<int> order(profiles.size());
	for (size_t i = 0; i < profiles.size(); ++i)
		order[i] = static_cast<int>(i);

	const int sortCol = (launcherConfig.sortColumn >= 1 && launcherConfig.sortColumn <= 6)
		? launcherConfig.sortColumn
		: -1;
	const bool desc = launcherConfig.sortDescending;

	std::stable_sort(order.begin(), order.end(), [&](int idxA, int idxB)
	{
		const Profile& a = profiles[idxA];
		const Profile& b = profiles[idxB];

		if (a.favorite != b.favorite)
			return a.favorite;

		if (sortCol < 0)
			return idxA < idxB;

		auto cmpTitle = [&]() -> int
		{
			return QString::fromStdString(a.title).compare(QString::fromStdString(b.title), Qt::CaseInsensitive);
		};

		int cmp = 0;
		switch (sortCol)
		{
		case 1: // Type
			if (a.isIWAD != b.isIWAD)
				cmp = a.isIWAD ? -1 : 1;
			else
				cmp = cmpTitle();
			break;
		case 2: // Title
			cmp = cmpTitle();
			break;
		case 3: // Author
			cmp = QString::fromStdString(a.author).compare(QString::fromStdString(b.author), Qt::CaseInsensitive);
			if (cmp == 0)
				cmp = cmpTitle();
			break;
		case 4: // Release Date
		{
			const QDate da = parseProfileReleaseDate(a.releaseDate);
			const QDate db = parseProfileReleaseDate(b.releaseDate);
			if (da.isValid() != db.isValid())
				return da.isValid();
			if (da.isValid() && db.isValid())
				cmp = (da < db) ? -1 : ((da > db) ? 1 : cmpTitle());
			else
				cmp = cmpTitle();
			break;
		}
		case 5: // Last Played
		{
			const QDateTime ta = parseProfileLastPlayed(a.lastPlayed);
			const QDateTime tb = parseProfileLastPlayed(b.lastPlayed);
			if (ta.isValid() != tb.isValid())
				return ta.isValid();
			if (ta.isValid() && tb.isValid())
				cmp = (ta < tb) ? -1 : ((ta > tb) ? 1 : cmpTitle());
			else
				cmp = cmpTitle();
			break;
		}
		case 6: // Playtime
			if (a.playtime != b.playtime)
				cmp = (a.playtime < b.playtime) ? -1 : 1;
			else
				cmp = cmpTitle();
			break;
		default:
			break;
		}

		if (cmp == 0)
			return idxA < idxB;
		return desc ? (cmp > 0) : (cmp < 0);
	});

	return order;
}

int LauncherMainWindow::profileIndexForTableRow(int tableRow) const
{
	if (tableRow < 0 || tableRow >= table->rowCount())
		return -1;
	const QTableWidgetItem* item = table->item(tableRow, 0);
	if (!item)
		return -1;
	const int idx = item->data(Qt::UserRole).toInt();
	if (idx < 0 || idx >= static_cast<int>(profiles.size()))
		return -1;
	return idx;
}

int LauncherMainWindow::tableRowForProfileIndex(int profileIdx) const
{
	if (profileIdx < 0 || profileIdx >= static_cast<int>(profiles.size()))
		return -1;
	for (int r = 0; r < table->rowCount(); ++r)
	{
		if (profileIndexForTableRow(r) == profileIdx)
			return r;
	}
	return -1;
}

bool LauncherMainWindow::canMoveSelectedBy(int delta) const
{
	if (gameProcess != nullptr)
		return false;
	if (launcherConfig.sortColumn >= 1 && launcherConfig.sortColumn <= 6)
		return false;
	if (!searchEdit->text().trimmed().isEmpty())
		return false;
	if (filterCategoryCombo && filterCategoryCombo->currentData().toInt() != 0)
		return false;

	const int curRow = table->currentRow();
	if (curRow < 0 || table->isRowHidden(curRow))
		return false;
	const int targetRow = curRow + delta;
	if (targetRow < 0 || targetRow >= table->rowCount() || table->isRowHidden(targetRow))
		return false;

	const int idxA = profileIndexForTableRow(curRow);
	const int idxB = profileIndexForTableRow(targetRow);
	if (idxA < 0 || idxB < 0)
		return false;
	return profiles[idxA].favorite == profiles[idxB].favorite;
}

void LauncherMainWindow::refreshTable(int selectProfileIndexValue)
{
	const std::vector<int> order = buildDisplayOrder();
	table->blockSignals(true);
	table->setRowCount(static_cast<int>(order.size()));
	for (int r = 0; r < static_cast<int>(order.size()); ++r)
		updateTableRow(r, order[r]);
	table->blockSignals(false);
	syncSortIndicator();

	applyFilter();

	if (selectProfileIndexValue >= 0 && selectProfileIndexValue < static_cast<int>(profiles.size()))
		selectProfileIndex(selectProfileIndexValue);
	else if (!order.empty())
		selectProfileIndex(order.front());
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

	auto* starItem = makeItem(p.favorite ? QString::fromUtf8("\u2605") : QString::fromUtf8("\u2606"), Qt::AlignCenter);
	const LauncherTheme theme = parseThemeName(launcherConfig.theme);
	if (p.favorite)
	{
		const QColor starColor = (theme == LauncherTheme::HighContrast)
			? QColor(255, 255, 0)
			: (isDarkThemeActive(theme) ? QColor(245, 197, 66) : QColor(214, 148, 0));
		starItem->setForeground(starColor);
		starItem->setToolTip(LStr("LAUNCHER_UNPIN_FAVORITE", "Unpin Favorite"));
	}
	else
	{
		starItem->setForeground(palette().color(QPalette::Disabled, QPalette::Text));
		starItem->setToolTip(LStr("LAUNCHER_PIN_FAVORITE", "Pin to Top (Favorite)"));
	}

	table->setItem(tableRow, 0, starItem);
	table->setItem(tableRow, 1, makeItem(p.isIWAD ? "IWAD" : "(P)WAD", Qt::AlignCenter));
	table->setItem(tableRow, 2, makeItem(QString::fromStdString(p.title)));
	table->setItem(tableRow, 3, makeItem(QString::fromStdString(p.author)));
	table->setItem(tableRow, 4, makeItem(QString::fromStdString(p.releaseDate), Qt::AlignCenter));
	const QString lastPlayedText = (p.lastPlayed.empty() || p.lastPlayed == "Never")
		? LStr("LAUNCHER_NEVER", "Never")
		: QString::fromStdString(p.lastPlayed);
	table->setItem(tableRow, 5, makeItem(lastPlayedText, Qt::AlignCenter));
	table->setItem(tableRow, 6, makeItem(formatPlaytime(p.playtime), Qt::AlignCenter));
}

int LauncherMainWindow::selectedProfileIndex() const
{
	const int row = table->currentRow();
	if (row < 0 || table->isRowHidden(row))
		return -1;
	return profileIndexForTableRow(row);
}

void LauncherMainWindow::selectProfileIndex(int profileIdx)
{
	if (table->rowCount() <= 0)
		return;

	int targetRow = tableRowForProfileIndex(profileIdx);
	if (targetRow < 0 || table->isRowHidden(targetRow))
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
	const QStringList terms = filter.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
	const int category = filterCategoryCombo ? filterCategoryCombo->currentData().toInt() : 0;
	int visibleCount = 0;

	for (int r = 0; r < table->rowCount(); ++r)
	{
		const int idx = profileIndexForTableRow(r);
		if (idx < 0)
		{
			table->setRowHidden(r, true);
			continue;
		}
		const Profile& p = profiles[idx];

		bool match = true;
		if (category == 1)
			match = p.favorite;
		else if (category == 2)
			match = p.isIWAD;
		else if (category == 3)
			match = !p.isIWAD;

		if (match && !terms.isEmpty())
		{
			const QString title = QString::fromStdString(p.title);
			const QString author = QString::fromStdString(p.author);
			const QString desc = QString::fromStdString(p.description);
			const QString type = p.isIWAD ? QStringLiteral("IWAD") : QStringLiteral("(P)WAD PWAD");
			const QString iwadFile = QFileInfo(QString::fromStdString(p.iwadPath)).fileName();
			const QString pwadFile = QFileInfo(QString::fromStdString(p.pwadPath)).fileName();

			for (const QString& term : terms)
			{
				bool termHit = title.contains(term, Qt::CaseInsensitive)
					|| author.contains(term, Qt::CaseInsensitive)
					|| desc.contains(term, Qt::CaseInsensitive)
					|| type.contains(term, Qt::CaseInsensitive)
					|| iwadFile.contains(term, Qt::CaseInsensitive)
					|| pwadFile.contains(term, Qt::CaseInsensitive);
				if (!termHit)
				{
					for (const std::string& modPath : p.mods)
					{
						if (QFileInfo(QString::fromStdString(modPath)).fileName().contains(term, Qt::CaseInsensitive))
						{
							termHit = true;
							break;
						}
					}
				}
				if (!termHit)
				{
					match = false;
					break;
				}
			}
		}

		table->setRowHidden(r, !match);
		if (match)
			++visibleCount;
	}

	if (terms.isEmpty() && category == 0)
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
			? LStr("LAUNCHER_EMPTY_HINT", "No profiles found yet.\nDrop a .wad, .pk3, .ipk3, .zip, or .uzdp file here, or click 'New Entry' above.")
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
	const bool isSorted = (launcherConfig.sortColumn >= 1 && launcherConfig.sortColumn <= 6);
	const QString moveTip = isSorted
		? LStr("LAUNCHER_MOVE_DISABLED_SORTED", "Reset column sort (click sorted header or \u2605) to reorder profiles manually.")
		: QString();

	addMenu->menuAction()->setEnabled(!isRunning);
	prefMenu->menuAction()->setEnabled(!isRunning);
	btnPlay->setEnabled(!isRunning && hasSel);
	btnJoin->setEnabled(!isRunning && hasSel);
	btnHost->setEnabled(!isRunning && hasSel);
	btnSettings->setEnabled(!isRunning && hasSel);
	btnMoveUp->setEnabled(canMoveSelectedBy(-1));
	btnMoveDown->setEnabled(canMoveSelectedBy(1));
	btnMoveUp->setToolTip(moveTip);
	btnMoveDown->setToolTip(moveTip);
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

void LauncherMainWindow::onFilterCategoryChanged(int)
{
	onFilterTextChanged(searchEdit->text());
}

void LauncherMainWindow::onHeaderSectionClicked(int logicalIndex)
{
	if (logicalIndex == 0)
	{
		if (launcherConfig.sortColumn >= 1)
		{
			launcherConfig.sortColumn = -1;
			launcherConfig.sortDescending = false;
			saveLauncherConfig(launcherConfig);
			refreshTable(selectedProfileIndex());
		}
		return;
	}

	if (logicalIndex < 1 || logicalIndex > 6)
		return;

	const bool defaultDesc = (logicalIndex == 5 || logicalIndex == 6);
	if (launcherConfig.sortColumn != logicalIndex)
	{
		launcherConfig.sortColumn = logicalIndex;
		launcherConfig.sortDescending = defaultDesc;
	}
	else if (launcherConfig.sortDescending == defaultDesc)
	{
		launcherConfig.sortDescending = !defaultDesc;
	}
	else
	{
		launcherConfig.sortColumn = -1;
		launcherConfig.sortDescending = false;
	}

	saveLauncherConfig(launcherConfig);
	refreshTable(selectedProfileIndex());
}

void LauncherMainWindow::onTableCellClicked(int row, int column)
{
	if (column != 0 || gameProcess)
		return;
	const int idx = profileIndexForTableRow(row);
	if (idx < 0)
		return;
	profiles[idx].favorite = !profiles[idx].favorite;
	saveProfileToDisk(profiles[idx]);
	refreshTable(idx);
}

void LauncherMainWindow::onToggleSelectedFavorite()
{
	if (gameProcess)
		return;
	const int idx = selectedProfileIndex();
	if (idx < 0)
		return;
	profiles[idx].favorite = !profiles[idx].favorite;
	saveProfileToDisk(profiles[idx]);
	refreshTable(idx);
}

void LauncherMainWindow::moveSelectedProfile(int delta)
{
	if (!canMoveSelectedBy(delta))
		return;
	const int curRow = table->currentRow();
	const int targetRow = curRow + delta;
	const int idxA = profileIndexForTableRow(curRow);
	const int idxB = profileIndexForTableRow(targetRow);
	if (idxA < 0 || idxB < 0)
		return;

	std::swap(profiles[idxA], profiles[idxB]);
	persistProfileOrdering(profiles);
	launcherConfig.lastSelectedRow = idxB;
	refreshTable(idxB);
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

void LauncherMainWindow::appendLaunchLogFooter(const QString& footerLine)
{
	if (activeLogFilePath.isEmpty())
		return;
	QFile f(activeLogFilePath);
	if (f.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
	{
		const QByteArray out = ("\n" + footerLine + "\n").toUtf8();
		f.write(out);
		f.close();
	}
	activeLogFilePath.clear();
}

void LauncherMainWindow::recordRunningSessionPlaytime()
{
	if (runningProfileIndex < 0 || runningProfileIndex >= static_cast<int>(profiles.size()))
		return;

	const int profIdx = runningProfileIndex;
	runningProfileIndex = -1;

	const qint64 elapsedSecs = playTimer.isValid() ? (playTimer.elapsed() / 1000) : 0;
	Profile& prof = profiles[profIdx];
	prof.lastPlayed = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm").toStdString();
	if (elapsedSecs >= 30)
		prof.playtime += std::max<uint64_t>(1, static_cast<uint64_t>((elapsedSecs + 30) / 60));
	saveProfileToDisk(prof);

	if (launcherConfig.sortColumn == 5 || launcherConfig.sortColumn == 6)
	{
		refreshTable(selectedProfileIndex());
	}
	else
	{
		const int row = tableRowForProfileIndex(profIdx);
		if (row >= 0)
			updateTableRow(row, profIdx);
	}
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

	QStringList missingFiles;
	if (!p.isIWAD && !p.pwadPath.empty())
	{
		const QString pwadQ = QString::fromStdString(p.pwadPath);
		if (!QFileInfo::exists(pwadQ))
			missingFiles << QString("(P)WAD: %1").arg(QDir::toNativeSeparators(pwadQ));
	}
	{
		QSet<QString> disabledKeys;
		for (const std::string& dm : p.disabledMods)
		{
			if (!dm.empty())
				disabledKeys.insert(normalizePathKey(QString::fromStdString(dm)));
		}
		for (const std::string& m : p.mods)
		{
			if (m.empty())
				continue;
			const QString modQ = QString::fromStdString(m);
			if (disabledKeys.contains(normalizePathKey(modQ)))
				continue;
			if (!QFileInfo::exists(modQ))
				missingFiles << QString("Mod: %1").arg(QDir::toNativeSeparators(modQ));
		}
	}
	if (!missingFiles.isEmpty())
	{
		constexpr int kMaxListed = 8;
		QStringList listed = missingFiles.mid(0, kMaxListed);
		if (missingFiles.size() > kMaxListed)
			listed << QString("... and %1 more").arg(missingFiles.size() - kMaxListed);

		QMessageBox box(this);
		box.setIcon(QMessageBox::Warning);
		box.setWindowTitle(LStr("LAUNCHER_MISSING_FILES_TITLE", "Missing Profile Files"));
		box.setText(QString("Some files configured for '%1' could not be found:\n\n%2\n\nWould you like to open Profile Settings to fix them, or launch anyway?")
			.arg(QString::fromStdString(p.title), listed.join('\n')));
		QPushButton* btnOpenSettings = box.addButton(LStr("LAUNCHER_OPEN_SETTINGS", "Open Profile Settings"), QMessageBox::AcceptRole);
		QPushButton* btnLaunchAnyway = box.addButton(LStr("LAUNCHER_LAUNCH_ANYWAY", "Launch Anyway"), QMessageBox::ActionRole);
		box.addButton(QMessageBox::Cancel);
		box.setDefaultButton(btnOpenSettings);
		box.exec();
		if (box.clickedButton() == btnOpenSettings)
		{
			openSettingsForIndex(idx);
			return;
		}
		if (box.clickedButton() != btnLaunchAnyway)
			return;
	}

	if (p.recordDemo && !p.selectedLaunchDemoRecord.empty() && !p.demoDirPath.empty())
		QDir().mkpath(QString::fromStdString(p.demoDirPath));

	QStringList args = p.buildLaunchArguments(mode, launcherConfig.language);
	args.removeAll("-showlauncher");

	runningProfileIndex = idx;
	gameProcess = new QProcess(this);
	gameProcess->setWorkingDirectory(QFileInfo(QCoreApplication::applicationFilePath()).absolutePath());

	activeLogFilePath.clear();
	if (!p.filePath.empty())
	{
		const QString profFolder = QFileInfo(QString::fromStdString(p.filePath)).absolutePath();
		const QString logsDirPath = QDir(profFolder).filePath(QStringLiteral("logs"));
		QDir().mkpath(logsDirPath);

		const QDateTime now = QDateTime::currentDateTime();
		const QString baseStamp = now.toString(QStringLiteral("yyyy-MM-dd_HH-mm-ss"));
		QString candidate = QDir(logsDirPath).filePath(baseStamp + QStringLiteral(".log"));
		int suffix = 1;
		while (QFileInfo::exists(candidate))
			candidate = QDir(logsDirPath).filePath(QString("%1_%2.log").arg(baseStamp).arg(++suffix));
		activeLogFilePath = candidate;

		QFile logHeaderFile(activeLogFilePath);
		if (logHeaderFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
		{
			const QString header = QString("=== %1 Launch Log (%2) ===\nCommand: %3 %4\n================================================\n\n")
				.arg(GAMENAME)
				.arg(now.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")))
				.arg(QDir::toNativeSeparators(QCoreApplication::applicationFilePath()))
				.arg(QString::fromStdString(p.giveLaunchCommand(mode, launcherConfig.language)));
			logHeaderFile.write(header.toUtf8());
			logHeaderFile.close();

			// Keep at most the 20 most recent logs per profile.
			constexpr int kMaxKeptLogs = 20;
			QDir logsDir(logsDirPath);
			const QFileInfoList existingLogs = logsDir.entryInfoList(
				{QStringLiteral("*.log")}, QDir::Files, QDir::Name | QDir::Reversed);
			for (int i = kMaxKeptLogs; i < existingLogs.size(); ++i)
				QFile::remove(existingLogs[i].absoluteFilePath());

			gameProcess->setProcessChannelMode(QProcess::MergedChannels);
			gameProcess->setStandardOutputFile(activeLogFilePath, QIODevice::Append);
#ifdef Q_OS_WIN
			if (!args.contains(QStringLiteral("-stdout"), Qt::CaseInsensitive))
				args << QStringLiteral("-stdout");
#endif
		}
		else
		{
			activeLogFilePath.clear();
		}
	}

	connect(gameProcess, &QProcess::errorOccurred, this, [this](QProcess::ProcessError err) {
		if (err == QProcess::FailedToStart)
		{
			appendLaunchLogFooter(QStringLiteral("=== Failed to start executable ==="));
			QMessageBox::critical(this, "Launch Failed",
				"Failed to launch UZDoom executable:\n" + QCoreApplication::applicationFilePath());
		}
	});

	connect(gameProcess, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
		[this](int exitCode, QProcess::ExitStatus exitStatus) {
			appendLaunchLogFooter(exitStatus == QProcess::CrashExit
				? QString("=== Process crashed (exit code %1) ===").arg(exitCode)
				: QString("=== Process exited (code %1) ===").arg(exitCode));

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
		appendLaunchLogFooter(QStringLiteral("=== Failed to start executable ==="));
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
		refreshTable(profileIdx);
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
				const QString modKey = normalizePathKey(finalModPath);
				const bool alreadyPresent = std::any_of(profiles[curIdx].mods.begin(), profiles[curIdx].mods.end(), [&](const std::string& existing)
				{
					return normalizePathKey(QString::fromStdString(existing)) == modKey;
				});
				if (!alreadyPresent)
					profiles[curIdx].mods.push_back(modStd);
				auto& dmods = profiles[curIdx].disabledMods;
				dmods.erase(std::remove_if(dmods.begin(), dmods.end(), [&](const std::string& dm)
				{
					return normalizePathKey(QString::fromStdString(dm)) == modKey;
				}), dmods.end());
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
	const int prevIdx = selectedProfileIndex();
	const QString prevPathKey = (prevIdx >= 0 && !profiles[prevIdx].filePath.empty())
		? normalizePathKey(QString::fromStdString(profiles[prevIdx].filePath))
		: QString();
	profiles = loadAllProfiles();
	int nextSelectIdx = prevIdx;
	if (!prevPathKey.isEmpty())
	{
		int matched = -1;
		for (int i = 0; i < static_cast<int>(profiles.size()); ++i)
		{
			if (!profiles[i].filePath.empty()
				&& normalizePathKey(QString::fromStdString(profiles[i].filePath)) == prevPathKey)
			{
				matched = i;
				break;
			}
		}
		if (matched >= 0)
			nextSelectIdx = matched;
		else if (!profiles.empty())
			nextSelectIdx = std::clamp(prevIdx, 0, static_cast<int>(profiles.size()) - 1);
		else
			nextSelectIdx = -1;
	}
	refreshTable(nextSelectIdx);
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

	if (!deleteProfileDirectory(profiles[idx]))
	{
		QMessageBox::warning(
			this,
			LStr("ERROR_FATAL", "Error"),
			LStr("LAUNCHER_DELETE_PROFILE_FAILED", "Failed to remove profile directory. Make sure no files or folders inside it are open in another program."));
		return;
	}
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

	QMenu menu(this);
	QAction* actPlay = menu.addAction(btnPlay->text(), this, &LauncherMainWindow::onPlayClicked);
	QAction* actJoin = menu.addAction(btnJoin->text(), this, &LauncherMainWindow::onJoinClicked);
	QAction* actHost = menu.addAction(btnHost->text(), this, &LauncherMainWindow::onHostClicked);
	actPlay->setEnabled(idle);
	actJoin->setEnabled(idle);
	actHost->setEnabled(idle);

	menu.addSeparator();
	QAction* actFav = menu.addAction(
		profiles[idx].favorite
			? LStr("LAUNCHER_UNPIN_FAVORITE", "Unpin Favorite")
			: LStr("LAUNCHER_PIN_FAVORITE", "Pin to Top (Favorite)"),
		this, &LauncherMainWindow::onToggleSelectedFavorite);
	actFav->setEnabled(idle);
	QAction* actSettings = menu.addAction(btnSettings->text(), this, &LauncherMainWindow::onProfileSettingsClicked);
	QAction* actClone = menu.addAction(LStr("LAUNCHER_CLONE_PROFILE", "Clone Profile..."), this, &LauncherMainWindow::onCloneSelectedProfile);
	menu.addAction(LStr("LAUNCHER_EXPORT_PROFILE", "Export Profile (.uzdp)..."), this, &LauncherMainWindow::onExportSelectedProfile);
	menu.addAction(LStr("LAUNCHER_OPEN_PROFILE_DIR", "Open Profile Folder"), this, &LauncherMainWindow::onOpenSelectedProfileFolder);
	actSettings->setEnabled(idle);
	actClone->setEnabled(idle);

	menu.addSeparator();
	QAction* actUp = menu.addAction(btnMoveUp->text(), this, &LauncherMainWindow::onMoveUpClicked);
	QAction* actDown = menu.addAction(btnMoveDown->text(), this, &LauncherMainWindow::onMoveDownClicked);
	actUp->setEnabled(canMoveSelectedBy(-1));
	actDown->setEnabled(canMoveSelectedBy(1));

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
	refreshTable(selectedProfileIndex());
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
