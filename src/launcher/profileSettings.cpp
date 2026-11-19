/*
** profileSettings.cpp
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

#include "profileSettings.h"
#include "const.h"
#include "flageditor.h"
#include "loader.h"

#include "doomdef.h"
#include "gstrings.h"

#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QDesktopServices>
#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QMimeData>
#include <QPalette>
#include <QPlainTextEdit>
#include <QProcess>
#include <QPushButton>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTabWidget>
#include <QTimer>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>

namespace
{
	QWidget* createPathBrowseRow(QLineEdit* edit, QPushButton* btn, QWidget* parent, QPushButton* extraBtn = nullptr)
	{
		QWidget* w = new QWidget(parent);
		QHBoxLayout* h = new QHBoxLayout(w);
		h->setContentsMargins(0, 0, 0, 0);
		h->setSpacing(4);
		h->addWidget(edit, 1);
		btn->setFixedWidth(32);
		h->addWidget(btn);
		if (extraBtn)
			h->addWidget(extraBtn);
		return w;
	}

	QString normModKey(const QString& path)
	{
		return QDir::cleanPath(path).replace('\\', '/').toLower();
	}

	QString quoteAdditionalArg(const QString& arg)
	{
		if (!arg.contains(' ') && !arg.contains('\t') && !arg.contains('"'))
			return arg;
		QString escaped = arg;
		escaped.replace('"', "\\\"");
		return '"' + escaped + '"';
	}

	bool isSupportedModDropFile(const QString& path)
	{
		const QFileInfo fi(path);
		if (!fi.exists() || !fi.isFile())
			return false;
		static const QSet<QString> exts = {
			"wad", "iwad", "pwad", "pk3", "ipk3", "pk7", "ipk7", "deh", "bex", "zip", "7z"
		};
		return exts.contains(fi.suffix().toLower());
	}

	bool isSupportedImageDropFile(const QString& path)
	{
		const QFileInfo fi(path);
		if (!fi.exists() || !fi.isFile())
			return false;
		static const QSet<QString> exts = {"png", "jpg", "jpeg", "webp", "bmp"};
		return exts.contains(fi.suffix().toLower());
	}

	QString switchForAdditionalFile(const QString& path)
	{
		const QString ext = QFileInfo(path).suffix().toLower();
		if (ext == "deh")
			return QStringLiteral("-deh");
		if (ext == "bex")
			return QStringLiteral("-bex");
		if (ext == "cfg")
			return QStringLiteral("+exec");
		if (ext == "lmp")
			return QStringLiteral("-playdemo");
		return QStringLiteral("-file");
	}

	bool endsWithFileTakingSwitch(const QString& text)
	{
		const QStringList tokens = QProcess::splitCommand(text.trimmed());
		if (tokens.isEmpty())
			return false;
		const QString last = tokens.last().toLower();
		static const QSet<QString> switches = {
			"-file", "-deh", "-bex", "-iwad", "-config", "-loadgame",
			"-playdemo", "-record", "-exec", "+exec", "-savedir", "-shotdir"
		};
		return switches.contains(last);
	}
}

ProfileSettingsDialog::ProfileSettingsDialog(
	Profile& targetProfile,
	const std::vector<Profile>& profilesList,
	const QString& defaultModsDir,
	const std::string& launcherLanguage,
	QWidget* parent)
	: QDialog(parent)
	, profile(targetProfile)
	, allProfiles(profilesList)
	, defaultModsDirectory(defaultModsDir)
	, currentLauncherLanguage(launcherLanguage)
{
	setWindowTitle(LStr("LAUNCHER_PROFILE_SETTINGS", "Profile Settings"));
	setAcceptDrops(true);
	setMinimumSize(820, 680);
	resize(900, 760);

	QVBoxLayout* mainLayout = new QVBoxLayout(this);

	tabs = new QTabWidget(this);
	tabs->addTab(buildGeneralTab(), LStr("PICKER_GENERAL", "General"));
	tabs->addTab(buildLaunchTab(), LStr("LAUNCHER_TAB_LAUNCH", "Launch"));
	tabs->addTab(buildFilesTab(), LStr("LAUNCHER_TAB_FILES", "Files"));
	tabs->addTab(buildOutputTab(), LStr("LAUNCHER_TAB_OUTPUT", "Output"));
	tabs->addTab(buildAdvancedTab(), LStr("LAUNCHER_TAB_ADVANCED", "Advanced"));
	connect(tabs, &QTabWidget::currentChanged, this, [this](int) { updateCommandPreview(); });
	mainLayout->addWidget(tabs, 1);

	QHBoxLayout* bottomBar = new QHBoxLayout();

	QPushButton* btnClone = new QPushButton(LStr("LAUNCHER_CLONE_PROFILE", "Clone Profile..."), this);
	connect(btnClone, &QPushButton::clicked, this, &ProfileSettingsDialog::onCloneProfile);
	bottomBar->addWidget(btnClone);

	QPushButton* btnExport = new QPushButton(LStr("LAUNCHER_EXPORT_PROFILE", "Export (.uzdp)..."), this);
	connect(btnExport, &QPushButton::clicked, this, &ProfileSettingsDialog::onExportProfile);
	bottomBar->addWidget(btnExport);

	QPushButton* btnDelete = new QPushButton(LStr("LAUNCHER_DELETE_PROFILE", "Delete Profile..."), this);
	btnDelete->setProperty("launcherRole", "danger");
	connect(btnDelete, &QPushButton::clicked, this, &ProfileSettingsDialog::onDeleteProfile);
	bottomBar->addWidget(btnDelete);

	bottomBar->addStretch();

	QPushButton* btnCancel = new QPushButton(LStr("OPTVAL_CANCEL", "Cancel"), this);
	connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
	bottomBar->addWidget(btnCancel);

	QPushButton* btnSave = new QPushButton(LStr("LAUNCHER_SAVE_RETURN", "Save and Return"), this);
	btnSave->setDefault(true);
	btnSave->setMinimumWidth(140);
	connect(btnSave, &QPushButton::clicked, this, &ProfileSettingsDialog::onSaveAndReturn);
	bottomBar->addWidget(btnSave);

	const int bottomBtnHeight = std::max(btnClone->sizeHint().height(), btnExport->sizeHint().height());
	for (QPushButton* b : {btnClone, btnExport, btnDelete, btnCancel, btnSave})
		b->setFixedHeight(bottomBtnHeight);

	mainLayout->addLayout(bottomBar);

	loadFromProfile();
}

QWidget* ProfileSettingsDialog::buildGeneralTab()
{
	QWidget* tab = new QWidget(this);
	QVBoxLayout* vbox = new QVBoxLayout(tab);

	QHBoxLayout* topSplit = new QHBoxLayout();

	QFormLayout* leftForm = new QFormLayout();
	leftForm->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
	editTitle = new QLineEdit(tab);
	editAuthor = new QLineEdit(tab);
	editReleaseDate = new QLineEdit(tab);
	editReleaseDate->setPlaceholderText("DD-MM-YYYY");
	editCoverImage = new QLineEdit(tab);
	QPushButton* btnCover = new QPushButton("...", tab);
	connect(btnCover, &QPushButton::clicked, this, [this]()
	{
		const QString p = browsePath("Select Cover Image", "Images (*.png *.jpg *.jpeg *.webp *.bmp);;All Files (*.*)", editCoverImage->text());
		if (!p.isEmpty())
			editCoverImage->setText(p);
	});

	leftForm->addRow(LStr("LAUNCHER_TITLE", "Title:"), editTitle);
	leftForm->addRow(LStr("LAUNCHER_AUTHOR", "Author:"), editAuthor);
	leftForm->addRow(LStr("LAUNCHER_RELEASE_DATE", "Release Date:"), editReleaseDate);
	leftForm->addRow(LStr("LAUNCHER_COVER_IMAGE", "Cover Image:"), createPathBrowseRow(editCoverImage, btnCover, tab));

	QFormLayout* rightForm = new QFormLayout();
	rightForm->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
	comboType = new QComboBox(tab);
	comboType->addItem("IWAD", true);
	comboType->addItem("(P)WAD", false);
	connect(comboType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ProfileSettingsDialog::onTypeChanged);

	editIwadPath = new QLineEdit(tab);
	QPushButton* btnIwad = new QPushButton("...", tab);
	connect(btnIwad, &QPushButton::clicked, this, [this]()
	{
		const QString p = browsePath("Select IWAD", "Doom WAD/PK3 (*.wad *.iwad *.ipk3 *.ipk7);;All Files (*.*)", editIwadPath->text());
		if (!p.isEmpty())
			editIwadPath->setText(p);
	});

	editPwadPath = new QLineEdit(tab);
	QPushButton* btnPwad = new QPushButton("...", tab);
	connect(btnPwad, &QPushButton::clicked, this, [this]()
	{
		const QString p = browsePath("Select (P)WAD", "Doom Files (*.wad *.pk3 *.pk7 *.deh *.bex);;All Files (*.*)", editPwadPath->text());
		if (!p.isEmpty())
			editPwadPath->setText(p);
	});
	pwadRowWidget = createPathBrowseRow(editPwadPath, btnPwad, tab);

	rightForm->addRow(LStr("LAUNCHER_TYPE", "Type:"), comboType);
	rightForm->addRow(LStr("LAUNCHER_ASSOC_IWAD", "Associated IWAD:"), createPathBrowseRow(editIwadPath, btnIwad, tab));
	rightForm->addRow(LStr("LAUNCHER_ASSOC_PWAD", "Associated (P)WAD:"), pwadRowWidget);

	topSplit->addLayout(leftForm, 1);
	topSplit->addSpacing(16);
	topSplit->addLayout(rightForm, 1);
	vbox->addLayout(topSplit);

	vbox->addWidget(new QLabel(LStr("LAUNCHER_DESCRIPTION", "Description:"), tab));
	editDescription = new QPlainTextEdit(tab);
	vbox->addWidget(editDescription, 1);

	return tab;
}

QWidget* ProfileSettingsDialog::buildLaunchTab()
{
	QWidget* tab = new QWidget(this);
	QHBoxLayout* columns = new QHBoxLayout(tab);

	QVBoxLayout* leftCol = new QVBoxLayout();

	QGroupBox* grpMode = new QGroupBox(LStr("LAUNCHER_LAUNCH_MODE", "Launch Mode"), tab);
	grpMode->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	QGridLayout* modeGrid = new QGridLayout(grpMode);
	QButtonGroup* modeGroup = new QButtonGroup(grpMode);

	radioNormal = new QRadioButton(LStr("LAUNCHER_MODE_NORMAL", "Normal"), grpMode);
	radioMap = new QRadioButton(LStr("LAUNCHER_MODE_MAP", "Map"), grpMode);
	radioSavegame = new QRadioButton(LStr("LAUNCHER_MODE_SAVE", "Savegame"), grpMode);
	radioDemoPlay = new QRadioButton(LStr("LAUNCHER_MODE_DEMO", "Demo Playback"), grpMode);

	for (QRadioButton* r : {radioNormal, radioMap, radioSavegame, radioDemoPlay})
	{
		modeGroup->addButton(r);
		connect(r, &QRadioButton::toggled, this, &ProfileSettingsDialog::onLaunchModeChanged);
	}

	editMap = new QLineEdit(grpMode);
	editMap->setPlaceholderText("MAP01, E1M1 etc.");

	editSavegame = new QLineEdit(grpMode);
	QPushButton* btnSave = new QPushButton("...", grpMode);
	connect(btnSave, &QPushButton::clicked, this, [this]()
	{
		const QString startDir = editSavegame->text().isEmpty() ? editSaveDir->text() : editSavegame->text();
		const QString p = browsePath("Select Savegame", "Savegames (*.zds);;All Files (*.*)", startDir);
		if (!p.isEmpty())
			editSavegame->setText(p);
	});

	editDemoPlay = new QLineEdit(grpMode);
	QPushButton* btnDemo = new QPushButton("...", grpMode);
	connect(btnDemo, &QPushButton::clicked, this, [this]()
	{
		const QString startDir = editDemoPlay->text().isEmpty() ? editDemoDir->text() : editDemoPlay->text();
		const QString p = browsePath("Select Demo", "Demos (*.lmp);;All Files (*.*)", startDir);
		if (!p.isEmpty())
			editDemoPlay->setText(p);
	});

	chkRecordDemo = new QCheckBox(LStr("LAUNCHER_RECORD_DEMO", "Record Demo:"), grpMode);
	editRecordDemo = new QLineEdit(grpMode);
	editRecordDemo->setPlaceholderText("exampleDemoName");
	connect(chkRecordDemo, &QCheckBox::toggled, editRecordDemo, &QWidget::setEnabled);

	saveRowWidget = createPathBrowseRow(editSavegame, btnSave, grpMode);
	demoRowWidget = createPathBrowseRow(editDemoPlay, btnDemo, grpMode);

	modeGrid->addWidget(radioNormal, 0, 0);
	modeGrid->addWidget(radioMap, 1, 0);
	modeGrid->addWidget(editMap, 1, 1);
	modeGrid->addWidget(radioSavegame, 2, 0);
	modeGrid->addWidget(saveRowWidget, 2, 1);
	modeGrid->addWidget(radioDemoPlay, 3, 0);
	modeGrid->addWidget(demoRowWidget, 3, 1);
	modeGrid->addWidget(chkRecordDemo, 4, 0);
	modeGrid->addWidget(editRecordDemo, 4, 1);
	leftCol->addWidget(grpMode);

	QGroupBox* grpDiff = new QGroupBox(LStr("MNU_SKILL", "Difficulty"), tab);
	grpDiff->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	QVBoxLayout* diffLayout = new QVBoxLayout(grpDiff);

	QHBoxLayout* skillRow = new QHBoxLayout();
	skillRow->addWidget(new QLabel(LStr("LAUNCHER_SKILL", "Skill:"), grpDiff));
	comboSkill = new QComboBox(grpDiff);
	comboSkill->addItem(LStr("LAUNCHER_SKILL_DISABLED", "Ignore"), 0);
	comboSkill->addItem("1 - " + LStr("SKILL_BABY", "I'm too young to die"), 1);
	comboSkill->addItem("2 - " + LStr("SKILL_EASY", "Hey, not too rough"), 2);
	comboSkill->addItem("3 - " + LStr("SKILL_NORMAL", "Hurt me plenty"), 3);
	comboSkill->addItem("4 - " + LStr("SKILL_HARD", "Ultra-Violence"), 4);
	comboSkill->addItem("5 - " + LStr("SKILL_NIGHTMARE", "Nightmare!"), 5);
	skillRow->addWidget(comboSkill, 1);
	diffLayout->addLayout(skillRow);
	diffLayout->addSpacing(4);

	chkFast = new QCheckBox(LStr("GMPLYMNU_FASTMONSTERS", "Fast Monsters"), grpDiff);
	chkRespawn = new QCheckBox(LStr("GMPLYMNU_MONSTERSRESPAWN", "Respawn Monsters"), grpDiff);
	chkNoMonsters = new QCheckBox(LStr("GMPLYMNU_NOMONSTERS", "No Monsters"), grpDiff);
	chkMultiItems = new QCheckBox(LStr("GMPLYMNU_ALWAYSSPAWNMULTI", "Allow Multiplayer Items"), grpDiff);
	chkPistolStart = new QCheckBox(LStr("LAUNCHER_PISTOL_START", "Pistol start all levels"), grpDiff);
	chkPistolStart->setToolTip("Reset player inventory at every level transition (DF3_PISTOL_START / sv_pistolstart). Separate from 'Restart level on death'.");

	connect(chkFast, &QCheckBox::toggled, this, [this](bool on)
	{
		if (on) dmflags |= DF_FAST_MONSTERS; else dmflags &= ~DF_FAST_MONSTERS;
		updateFlagSummary();
	});
	connect(chkRespawn, &QCheckBox::toggled, this, [this](bool on)
	{
		if (on) dmflags |= DF_MONSTERS_RESPAWN; else dmflags &= ~DF_MONSTERS_RESPAWN;
		updateFlagSummary();
	});
	connect(chkNoMonsters, &QCheckBox::toggled, this, [this](bool on)
	{
		if (on) dmflags |= DF_NO_MONSTERS; else dmflags &= ~DF_NO_MONSTERS;
		updateFlagSummary();
	});
	connect(chkMultiItems, &QCheckBox::toggled, this, [this](bool on)
	{
		if (on) dmflags2 |= DF2_ALWAYS_SPAWN_MULTI; else dmflags2 &= ~DF2_ALWAYS_SPAWN_MULTI;
		updateFlagSummary();
	});
	connect(chkPistolStart, &QCheckBox::toggled, this, [this](bool on)
	{
		if (on) dmflags3 |= DF3_PISTOL_START; else dmflags3 &= ~DF3_PISTOL_START;
		updateFlagSummary();
	});

	for (QCheckBox* c : {chkFast, chkRespawn, chkNoMonsters, chkMultiItems, chkPistolStart})
		diffLayout->addWidget(c);
	leftCol->addWidget(grpDiff);

	QGroupBox* grpPlayer = new QGroupBox(LStr("MNU_PLAYER", "Player"), tab);
	grpPlayer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	QFormLayout* playerForm = new QFormLayout(grpPlayer);
	playerForm->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
	editPlayerName = new QLineEdit(grpPlayer);
	editPlayerClass = new QLineEdit(grpPlayer);
	comboPlayerGender = new QComboBox(grpPlayer);
	comboPlayerGender->addItems({"Default", LStr("PLYRMNU_MALE", "Male"), LStr("PLYRMNU_FEMALE", "Female"), LStr("PLYRMNU_OTHER", "Neutral / Other")});

	comboWadLang = new QComboBox(grpPlayer);
	comboWadLang->addItem("Default (Launcher Language)", "default");
	for (const auto& lang : getLauncherLanguages())
		comboWadLang->addItem(lang.name, lang.code);

	playerForm->addRow(LStr("PLYRMNU_NAME", "Player Name:"), editPlayerName);
	playerForm->addRow(LStr("PICKER_PLAYERCLASS", "Player Class:"), editPlayerClass);
	playerForm->addRow(LStr("PLYRMNU_GENDER", "Gender:"), comboPlayerGender);
	playerForm->addRow(LStr("OPTMNU_LANGUAGE", "Language:"), comboWadLang);
	leftCol->addWidget(grpPlayer);
	leftCol->addStretch();

	QVBoxLayout* rightCol = new QVBoxLayout();

	QGroupBox* grpRemote = new QGroupBox(LStr("LAUNCHER_REMOTE_MP", "Remote Multiplayer"), tab);
	grpRemote->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	QFormLayout* remoteForm = new QFormLayout(grpRemote);
	remoteForm->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
	editRemoteAddr = new QLineEdit(grpRemote);
	editRemoteAddr->setPlaceholderText("127.0.0.1");
	spinRemotePort = new QSpinBox(grpRemote);
	spinRemotePort->setRange(0, 65535);
	spinRemotePort->setSpecialValueText("Default (5029)");
	spinRemoteTeam = new QSpinBox(grpRemote);
	spinRemoteTeam->setRange(0, 255);

	remoteForm->addRow(LStr("PICKER_IP", "Remote Address:"), editRemoteAddr);
	remoteForm->addRow(LStr("LAUNCHER_REMOTE_PORT", "Remote Port (Default 5029):"), spinRemotePort);
	remoteForm->addRow(LStr("LAUNCHER_TEAM_NO", "Team No. (255 = Random):"), spinRemoteTeam);
	rightCol->addWidget(grpRemote);

	QGroupBox* grpHost = new QGroupBox(LStr("LAUNCHER_HOST_MP", "Host Multiplayer"), tab);
	grpHost->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	QFormLayout* hostForm = new QFormLayout(grpHost);
	hostForm->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
	spinHostPort = new QSpinBox(grpHost);
	spinHostPort->setRange(0, 65535);
	spinHostPort->setSpecialValueText("Default (5029)");
	spinMaxPlayers = new QSpinBox(grpHost);
	spinMaxPlayers->setRange(1, 64);

	comboTicRate = new QComboBox(grpHost);
	comboTicRate->addItems({"35 Hz", "17.5 Hz", "11.6 Hz"});
	chkExtraTic = new QCheckBox(LStr("PICKER_NETBACKUP", "ExtraTic"), grpHost);

	comboGamemode = new QComboBox(grpHost);
	comboGamemode->addItem(LStr("OPTVAL_NONE", "None"));
	comboGamemode->addItem(LStr("PICKER_COOP", "Co-op"));
	comboGamemode->addItem(LStr("PICKER_DM", "Deathmatch"));
	comboGamemode->addItem(LStr("PICKER_TDM", "Team Deathmatch"));
	comboGamemode->addItem(LStr("PICKER_ALTDM", "Alt Deathmatch"));

	spinHostTeam = new QSpinBox(grpHost);
	spinHostTeam->setRange(0, 255);
	connect(comboGamemode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx)
	{
		spinHostTeam->setEnabled(idx == 3);
	});

	int dropdownHeight = 0;
	for (const QComboBox* combo : {comboTicRate, comboGamemode})
		dropdownHeight = std::max(dropdownHeight, combo->sizeHint().height());
	for (QSpinBox* spin : {spinRemotePort, spinRemoteTeam, spinHostPort, spinMaxPlayers, spinHostTeam})
		spin->setMinimumHeight(dropdownHeight);

	hostForm->addRow(LStr("LAUNCHER_HOST_PORT", "Host Port (Default 5029):"), spinHostPort);
	hostForm->addRow(LStr("LAUNCHER_MAX_PLAYERS", "Max Players (Limit 64):"), spinMaxPlayers);
	hostForm->addRow(LStr("PICKER_NETRATE", "Tickrate:"), comboTicRate);
	hostForm->addRow(QString(), chkExtraTic);
	hostForm->addRow(LStr("PICKER_GAMEMODE", "Gamemode:"), comboGamemode);
	hostForm->addRow(LStr("LAUNCHER_HOST_TEAM", "Host Team (255 = Random):"), spinHostTeam);
	rightCol->addWidget(grpHost);

	QGroupBox* grpCompat = new QGroupBox(LStr("LAUNCHER_COMPAT_AND_FLAGS", "Compatibility and Flags"), tab);
	grpCompat->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	QVBoxLayout* compatLayout = new QVBoxLayout(grpCompat);
	QFormLayout* compatForm = new QFormLayout();
	compatForm->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

	comboEngineComplevel = new QComboBox(grpCompat);
	for (const EngineComplevelInfo& cl : getEngineComplevels())
		comboEngineComplevel->addItem(cl.label, cl.value);
	comboEngineComplevel->setToolTip("Sets UZDoom's -complevel parameter (applies Doom-only MAPINFO compatibility defaults per map; does not modify compatflags CVARs).");

	QWidget* compatModeRow = new QWidget(grpCompat);
	QHBoxLayout* compRow = new QHBoxLayout(compatModeRow);
	compRow->setContentsMargins(0, 0, 0, 0);
	comboCompatLevel = new QComboBox(compatModeRow);
	for (const CompatModeInfo& cm : getCompatModes())
		comboCompatLevel->addItem(cm.label, cm.id);
	comboCompatLevel->setToolTip("Selects a compatibility mode preset (-compatmode), updating compatflags and compatflags2.");
	connect(comboCompatLevel, &QComboBox::activated, this, [this](int index)
	{
		const int id = comboCompatLevel->itemData(index).toInt();
		if (id < 0)
			return;
		CompatModeInfo info;
		if (findCompatModeById(id, info))
		{
			compatflags = info.compatflags;
			compatflags2 = info.compatflags2;
			syncCompatModeFromFlags();
			updateFlagSummary();
		}
	});
	QPushButton* btnMoreFlags = new QPushButton(LStr("LAUNCHER_MORE", "More..."), compatModeRow);
	connect(btnMoreFlags, &QPushButton::clicked, this, &ProfileSettingsDialog::onOpenFlagEditor);
	compRow->addWidget(comboCompatLevel, 1);
	compRow->addWidget(btnMoreFlags);

	QString compatModeLabel = LStr("CMPTMNU_MODE", "Compatibility mode");
	if (!compatModeLabel.endsWith(':'))
		compatModeLabel += ':';

	compatForm->addRow(LStr("LAUNCHER_COMPLEVEL", "Complevel:"), comboEngineComplevel);
	compatForm->addRow(compatModeLabel, compatModeRow);
	compatLayout->addLayout(compatForm);

	lblFlagSummary = new QLabel(grpCompat);
	lblFlagSummary->setWordWrap(true);
	compatLayout->addWidget(lblFlagSummary);
	rightCol->addWidget(grpCompat);
	rightCol->addStretch();

	columns->addLayout(leftCol, 1);
	columns->addSpacing(12);
	columns->addLayout(rightCol, 1);
	return tab;
}

QWidget* ProfileSettingsDialog::buildFilesTab()
{
	QWidget* tab = new QWidget(this);
	QVBoxLayout* vbox = new QVBoxLayout(tab);

	QFormLayout* pathsForm = new QFormLayout();
	pathsForm->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
	editConfigFile = new QLineEdit(tab);
	connect(editConfigFile, &QLineEdit::textChanged, editConfigFile, [this](const QString& text)
	{
		editConfigFile->setToolTip(text.trimmed());
	});
	QPushButton* btnCfg = new QPushButton("...", tab);
	connect(btnCfg, &QPushButton::clicked, this, [this]()
	{
		const QString p = browsePath("Select Config File", "Config (*.ini);;All Files (*.*)", editConfigFile->text());
		if (!p.isEmpty()) editConfigFile->setText(p);
	});
	QPushButton* btnOpenCfg = new QPushButton(LStr("LAUNCHER_OPEN_INI", "Open INI"), tab);
	btnOpenCfg->setToolTip("Open the configured INI file in the system default editor (or locate its folder).");
	connect(btnOpenCfg, &QPushButton::clicked, this, [this]()
	{
		const QString iniPath = editConfigFile->text().trimmed();
		if (iniPath.isEmpty())
		{
			QMessageBox::information(this, LStr("LAUNCHER_OPEN_INI", "Open INI"),
				"No INI config file path is configured for this profile yet.");
			return;
		}

		const QFileInfo fi(iniPath);
		if (fi.exists() && fi.isFile())
		{
			QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absoluteFilePath()));
			return;
		}

		QMessageBox box(this);
		box.setIcon(QMessageBox::Information);
		box.setWindowTitle(LStr("LAUNCHER_OPEN_INI", "Open INI"));
		box.setText(QString("INI file does not exist yet:\n%1\n\nUZDoom creates this file automatically on first launch.")
			.arg(QDir::toNativeSeparators(fi.absoluteFilePath())));
		QPushButton* btnCreate = box.addButton("Create and Open", QMessageBox::AcceptRole);
		QPushButton* btnFolder = box.addButton("Open Folder", QMessageBox::ActionRole);
		box.addButton(QMessageBox::Cancel);
		box.exec();

		if (box.clickedButton() == btnCreate)
		{
			QDir().mkpath(fi.absolutePath());
			QFile f(fi.absoluteFilePath());
			if (f.open(QIODevice::WriteOnly))
			{
				f.close();
				QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absoluteFilePath()));
			}
			else
			{
				QMessageBox::warning(this, LStr("ERROR_FATAL", "Error"), "Could not create INI file at:\n" + fi.absoluteFilePath());
			}
		}
		else if (box.clickedButton() == btnFolder)
		{
			QDir().mkpath(fi.absolutePath());
			QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absolutePath()));
		}
	});

	editSaveDir = new QLineEdit(tab);
	QPushButton* btnSaveDir = new QPushButton("...", tab);
	connect(btnSaveDir, &QPushButton::clicked, this, [this]()
	{
		const QString p = browsePath("Select Save Directory", {}, editSaveDir->text(), true);
		if (!p.isEmpty()) editSaveDir->setText(p);
	});

	editShotDir = new QLineEdit(tab);
	QPushButton* btnShotDir = new QPushButton("...", tab);
	connect(btnShotDir, &QPushButton::clicked, this, [this]()
	{
		const QString p = browsePath("Select Screenshot Directory", {}, editShotDir->text(), true);
		if (!p.isEmpty()) editShotDir->setText(p);
	});

	editDemoDir = new QLineEdit(tab);
	QPushButton* btnDemoDir = new QPushButton("...", tab);
	connect(btnDemoDir, &QPushButton::clicked, this, [this]()
	{
		const QString p = browsePath("Select Demo Directory", {}, editDemoDir->text(), true);
		if (!p.isEmpty()) editDemoDir->setText(p);
	});

	pathsForm->addRow(LStr("LAUNCHER_CONFIG_FILE", "Config File:"), createPathBrowseRow(editConfigFile, btnCfg, tab, btnOpenCfg));
	pathsForm->addRow(LStr("LAUNCHER_SAVE_DIR", "Save Directory:"), createPathBrowseRow(editSaveDir, btnSaveDir, tab));
	pathsForm->addRow(LStr("LAUNCHER_SHOT_DIR", "Screenshot Dir:"), createPathBrowseRow(editShotDir, btnShotDir, tab));
	pathsForm->addRow(LStr("LAUNCHER_DEMO_DIR", "Demo Directory:"), createPathBrowseRow(editDemoDir, btnDemoDir, tab));
	vbox->addLayout(pathsForm);

	QHBoxLayout* modsHeader = new QHBoxLayout();
	QLabel* lblMods = new QLabel(LStr("LAUNCHER_MODS", "Mods"), tab);
	QFont f = lblMods->font();
	f.setBold(true);
	lblMods->setFont(f);
	lblMods->setContentsMargins(0, 0, 0, std::max(2, lblMods->fontMetrics().descent()));
	modsHeader->addWidget(lblMods, 0, Qt::AlignVCenter);

	chkCopyModsToProfile = new QCheckBox(LStr("LAUNCHER_COPY_MODS", "Copy added mods to profile"), tab);
	chkCopyModsToProfile->setToolTip("When enabled, added mods are copied into this profile's mods/ folder.\nWhen disabled, mods are referenced from their original location without copying.");
	modsHeader->addSpacing(12);
	modsHeader->addWidget(chkCopyModsToProfile, 0, Qt::AlignVCenter);
	modsHeader->addStretch();

	QPushButton* btnAddMod = new QPushButton("+ " + LStr("LAUNCHER_ADD_MOD", "Add Mod File"), tab);
	btnAddMod->setMinimumSize(160, 32);
	connect(btnAddMod, &QPushButton::clicked, this, &ProfileSettingsDialog::onAddModFiles);
	modsHeader->addWidget(btnAddMod, 0, Qt::AlignVCenter);
	vbox->addLayout(modsHeader);

	listMods = new QListWidget(tab);
	listMods->setSelectionMode(QAbstractItemView::SingleSelection);
	listMods->setAlternatingRowColors(true);
	listMods->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	vbox->addWidget(listMods, 1);

	return tab;
}

QWidget* ProfileSettingsDialog::buildOutputTab()
{
	QWidget* tab = new QWidget(this);
	QHBoxLayout* cols = new QHBoxLayout(tab);

	QVBoxLayout* leftCol = new QVBoxLayout();
	QGroupBox* grpGen = new QGroupBox(LStr("PICKER_GENERAL", "General"), tab);
	grpGen->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	QVBoxLayout* genBox = new QVBoxLayout(grpGen);
	chkFullscreen = new QCheckBox(LStr("VIDMNU_FULLSCREEN", "Fullscreen"), grpGen);
	chkSupportWads = new QCheckBox(LStr("PICKER_SUPPORTWADS", "Load Support WADs") + " (e.g. id24res.wad)", grpGen);
	chkSupportWads->setToolTip("Loads optional ID24 support resources (id24res.wad) when available, adding Legacy of Rust / ID24 weapons and assets. Automatically disabled in multiplayer.");
	chkDisableAutoload = new QCheckBox(LStr("PICKER_NOAUTOLOAD", "Disable Autoload"), grpGen);
	chkDisableAutoload->setToolTip("Skips Global.Autoload, per-IWAD autoload sections, zvox.wad, and skins/ (-noautoload). Extra Graphics below are controlled separately.");
	chkDynLights = new QCheckBox(LStr("GLLIGHTMNU_LIGHTSENABLED", "Dynamic Lights"), grpGen);

	for (QCheckBox* c : {chkFullscreen, chkSupportWads, chkDisableAutoload, chkDynLights})
		genBox->addWidget(c);
	leftCol->addWidget(grpGen);
	leftCol->addStretch();

	QVBoxLayout* rightCol = new QVBoxLayout();

	QGroupBox* grpApi = new QGroupBox(LStr("PICKER_PREFERBACKEND", "Rendering API"), tab);
	grpApi->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	QVBoxLayout* apiBox = new QVBoxLayout(grpApi);
	QButtonGroup* apiGroup = new QButtonGroup(grpApi);
	radioVulkan = new QRadioButton(LStr("OPTVAL_VULKAN", "Vulkan"), grpApi);
	radioOpenGL = new QRadioButton(LStr("OPTVAL_OPENGL", "OpenGL"), grpApi);
	radioGLES = new QRadioButton(LStr("OPTVAL_OPENGLES", "OpenGL ES"), grpApi);
	for (QRadioButton* r : {radioVulkan, radioOpenGL, radioGLES})
	{
		apiGroup->addButton(r);
		apiBox->addWidget(r);
	}

	QGroupBox* grpExtra = new QGroupBox(LStr("PICKER_EXTRA", "Extra Graphics"), tab);
	grpExtra->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	QVBoxLayout* extraBox = new QVBoxLayout(grpExtra);
	chkLights = new QCheckBox(LStr("PICKER_LIGHTS", "Lights"), grpExtra);
	chkBrightmaps = new QCheckBox(LStr("PICKER_BRIGHTMAPS", "Brightmaps"), grpExtra);
	chkWidescreen = new QCheckBox(LStr("PICKER_WIDESCREEN", "Widescreen"), grpExtra);
	for (QCheckBox* c : {chkLights, chkBrightmaps, chkWidescreen})
		extraBox->addWidget(c);

	rightCol->addWidget(grpApi);
	rightCol->addWidget(grpExtra);
	rightCol->addStretch();

	cols->addLayout(leftCol, 1);
	cols->addSpacing(12);
	cols->addLayout(rightCol, 1);
	return tab;
}

QWidget* ProfileSettingsDialog::buildAdvancedTab()
{
	QWidget* tab = new QWidget(this);
	QVBoxLayout* vbox = new QVBoxLayout(tab);

	QHBoxLayout* appendHeader = new QHBoxLayout();
	appendHeader->addWidget(new QLabel(LStr("LAUNCHER_APPEND_ARGS", "Append Additional Parameters"), tab));
	appendHeader->addStretch();
	QPushButton* btnBrowseAppend = new QPushButton(LStr("LAUNCHER_SELECT_FILE", "Select File..."), tab);
	btnBrowseAppend->setToolTip("Pick file(s) and append them to Additional Parameters (auto-adds -file / -deh / +exec / -playdemo when no switch precedes them).");
	connect(btnBrowseAppend, &QPushButton::clicked, this, [this]()
	{
		const QString startDir = !defaultModsDirectory.isEmpty() && QFileInfo::exists(defaultModsDirectory)
			? defaultModsDirectory
			: (profile.filePath.empty() ? qStr(LauncherConst::EXEC_DIR) : QFileInfo(qStr(profile.filePath)).absolutePath());
		const QStringList picked = QFileDialog::getOpenFileNames(
			this,
			LStr("PICKER_BROWSE", "Select File(s) for Additional Parameters"),
			startDir,
			"Doom Files (*.wad *.iwad *.pwad *.pk3 *.pk7 *.ipk3 *.ipk7 *.deh *.bex *.cfg *.lmp);;All Files (*.*)");
		if (picked.isEmpty())
			return;
		QString current = editAppendArgs->toPlainText();
		const bool hasPrecedingSwitch = endsWithFileTakingSwitch(current);
		QStringList tokens;
		for (int i = 0; i < picked.size(); ++i)
		{
			const QString& p = picked[i];
			if (!hasPrecedingSwitch)
				tokens << switchForAdditionalFile(p);
			tokens << quoteAdditionalArg(p);
		}
		if (!current.trimmed().isEmpty() && !current.endsWith(' ') && !current.endsWith('\n'))
			current += ' ';
		current += tokens.join(' ');
		editAppendArgs->setPlainText(current);
	});
	appendHeader->addWidget(btnBrowseAppend);
	vbox->addLayout(appendHeader);

	editAppendArgs = new QPlainTextEdit(tab);
	editAppendArgs->setFixedHeight(100);
	editAppendArgs->setPlaceholderText("Parameters appended after profile options...");
	connect(editAppendArgs, &QPlainTextEdit::textChanged, this, &ProfileSettingsDialog::updateCommandPreview);
	vbox->addWidget(editAppendArgs);

	vbox->addSpacing(8);

	QGroupBox* grpCmd = new QGroupBox(LStr("LAUNCHER_COMMAND_PREVIEW", "Launch Command Preview"), tab);
	QVBoxLayout* cmdLayout = new QVBoxLayout(grpCmd);

	QHBoxLayout* cmdTopRow = new QHBoxLayout();
	cmdTopRow->addWidget(new QLabel(LStr("LAUNCHER_PREVIEW_MODE", "Mode:"), grpCmd));
	comboPreviewMode = new QComboBox(grpCmd);
	comboPreviewMode->addItem(LStr("PICKER_PLAY", "Play Game"), static_cast<int>(LaunchTargetMode::Single));
	comboPreviewMode->addItem(LStr("LAUNCHER_HOST_GAME", "Host Game"), static_cast<int>(LaunchTargetMode::Host));
	comboPreviewMode->addItem(LStr("LAUNCHER_JOIN_GAME", "Join Game"), static_cast<int>(LaunchTargetMode::Join));
	connect(comboPreviewMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) { updateCommandPreview(); });
	cmdTopRow->addWidget(comboPreviewMode);
	cmdTopRow->addStretch();

	btnCopyCommand = new QPushButton(LStr("ACTION_COPYTOCLIPBOARD", "Copy to Clipboard"), grpCmd);
	connect(btnCopyCommand, &QPushButton::clicked, this, [this]()
	{
		updateCommandPreview();
		if (QClipboard* cb = QApplication::clipboard())
			cb->setText(editCommandPreview->toPlainText());
		btnCopyCommand->setText(LStr("LAUNCHER_COPIED", "Copied!"));
		QTimer::singleShot(1400, btnCopyCommand, [this]()
		{
			if (btnCopyCommand)
				btnCopyCommand->setText(LStr("ACTION_COPYTOCLIPBOARD", "Copy to Clipboard"));
		});
	});
	cmdTopRow->addWidget(btnCopyCommand);
	cmdLayout->addLayout(cmdTopRow);

	editCommandPreview = new QPlainTextEdit(grpCmd);
	editCommandPreview->setReadOnly(true);
	editCommandPreview->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	editCommandPreview->setMinimumHeight(110);
	cmdLayout->addWidget(editCommandPreview, 1);

	vbox->addWidget(grpCmd, 1);

	return tab;
}

void ProfileSettingsDialog::loadFromProfile()
{
	editTitle->setText(qStr(profile.title));
	editAuthor->setText(qStr(profile.author));
	editReleaseDate->setText(qStr(profile.releaseDate));
	editCoverImage->setText(qStr(profile.coverImagePath));
	comboType->setCurrentIndex(profile.isIWAD ? 0 : 1);
	editIwadPath->setText(qStr(profile.iwadPath));
	editPwadPath->setText(qStr(profile.pwadPath));
	editDescription->setPlainText(qStr(profile.description));
	onTypeChanged(comboType->currentIndex());

	switch (profile.launchParameters)
	{
	case 1: radioMap->setChecked(true); break;
	case 2: radioSavegame->setChecked(true); break;
	case 3: radioDemoPlay->setChecked(true); break;
	default: radioNormal->setChecked(true); break;
	}
	editMap->setText(qStr(profile.selectedLaunchMap));
	editSavegame->setText(qStr(profile.selectedLaunchSave));
	editDemoPlay->setText(qStr(profile.selectedLaunchDemoPlayback));
	chkRecordDemo->setChecked(profile.recordDemo);
	editRecordDemo->setText(qStr(profile.selectedLaunchDemoRecord));
	editRecordDemo->setEnabled(profile.recordDemo);
	onLaunchModeChanged();

	comboSkill->setCurrentIndex(std::clamp(profile.difficulty, 0, comboSkill->count() - 1));

	editPlayerName->setText(qStr(profile.playerName));
	editPlayerClass->setText(qStr(profile.playerClass));
	comboPlayerGender->setCurrentIndex(std::clamp(profile.playerGender, 0, 3));
	const int langIdx = comboWadLang->findData(qStr(profile.wadLanguage));
	comboWadLang->setCurrentIndex(langIdx >= 0 ? langIdx : 0);

	editRemoteAddr->setText(qStr(profile.remoteAddress));
	spinRemotePort->setValue(std::clamp(profile.remotePort, 0, 65535));
	spinRemoteTeam->setValue(std::clamp(profile.remoteTeam, 0, 255));

	spinHostPort->setValue(std::clamp(profile.hostPort, 0, 65535));
	spinMaxPlayers->setValue(std::clamp(profile.maxPlayers, 1, 64));
	comboTicRate->setCurrentIndex(std::clamp(profile.hostTicRate, 0, 2));
	chkExtraTic->setChecked(profile.hostExtraTic);
	comboGamemode->setCurrentIndex(std::clamp(profile.hostGamemode, 0, 4));
	spinHostTeam->setValue(std::clamp(profile.hostTeam, 0, 255));
	spinHostTeam->setEnabled(comboGamemode->currentIndex() == 3);

	const int clIdx = comboEngineComplevel->findData(qStr(profile.complevel));
	comboEngineComplevel->setCurrentIndex(clIdx >= 0 ? clIdx : 0);

	alwaysApplyDMFlags = profile.alwaysApplyDMFlags;
	dmflags = profile.dmflags;
	dmflags2 = profile.dmflags2;
	dmflags3 = profile.dmflags3;
	if (profile.fastMonsters)
		dmflags |= DF_FAST_MONSTERS;
	if (profile.respawnMonsters)
		dmflags |= DF_MONSTERS_RESPAWN;
	if (profile.noMonsters)
		dmflags |= DF_NO_MONSTERS;
	if (profile.allowMultiplayerItems)
		dmflags2 |= DF2_ALWAYS_SPAWN_MULTI;
	if (profile.pistolStartAllLevels)
		dmflags3 |= DF3_PISTOL_START;

	{
		const QSignalBlocker b1(chkFast);
		const QSignalBlocker b2(chkRespawn);
		const QSignalBlocker b3(chkNoMonsters);
		const QSignalBlocker b4(chkMultiItems);
		const QSignalBlocker b5(chkPistolStart);
		chkFast->setChecked((dmflags & DF_FAST_MONSTERS) != 0);
		chkRespawn->setChecked((dmflags & DF_MONSTERS_RESPAWN) != 0);
		chkNoMonsters->setChecked((dmflags & DF_NO_MONSTERS) != 0);
		chkMultiItems->setChecked((dmflags2 & DF2_ALWAYS_SPAWN_MULTI) != 0);
		chkPistolStart->setChecked((dmflags3 & DF3_PISTOL_START) != 0);
	}

	compatflags = profile.compatflags;
	compatflags2 = profile.compatflags2;
	if (profile.compatLevel > 0 && compatflags == 0 && compatflags2 == 0)
	{
		CompatModeInfo info;
		if (findCompatModeById(profile.compatLevel, info))
		{
			compatflags = info.compatflags;
			compatflags2 = info.compatflags2;
		}
	}
	syncCompatModeFromFlags();
	updateFlagSummary();

	editConfigFile->setText(qStr(profile.configFilePath));
	editConfigFile->setToolTip(qStr(profile.configFilePath));
	editSaveDir->setText(qStr(profile.saveDirPath));
	editShotDir->setText(qStr(profile.screenshotDirPath));
	editDemoDir->setText(qStr(profile.demoDirPath));
	if (chkCopyModsToProfile)
		chkCopyModsToProfile->setChecked(profile.copyModsToProfile);
	disabledModKeys.clear();
	for (const auto& dm : profile.disabledMods)
	{
		if (!dm.empty())
			disabledModKeys.insert(normModKey(qStr(dm)));
	}
	rebuildModList(profile.mods);

	chkFullscreen->setChecked(profile.fullscreen);
	chkSupportWads->setChecked(profile.loadSupportWads);
	chkDisableAutoload->setChecked(profile.disableAutoload);
	chkDynLights->setChecked(profile.enableDynLights);
	if (profile.renderingAPI == 0)
		radioOpenGL->setChecked(true);
	else if (profile.renderingAPI == 2)
		radioGLES->setChecked(true);
	else
		radioVulkan->setChecked(true);

	chkLights->setChecked(profile.extraLights);
	chkBrightmaps->setChecked(profile.extraBrightmaps);
	chkWidescreen->setChecked(profile.extraWidescreen);

	editAppendArgs->setPlainText(qStr(profile.appendAdditionalParameters));
	updateCommandPreview();

	initialSnapshot = profile;
	writeToProfile(initialSnapshot);
}

void ProfileSettingsDialog::writeToProfile(Profile& target) const
{
	target.title = stdStr(editTitle->text().trimmed().isEmpty() ? QString("New Profile") : editTitle->text().trimmed());
	target.author = stdStr(editAuthor->text().trimmed());
	target.releaseDate = stdStr(editReleaseDate->text().trimmed());
	target.coverImagePath = stdStr(editCoverImage->text().trimmed());
	target.isIWAD = comboType->currentData().toBool();
	target.iwadPath = stdStr(editIwadPath->text().trimmed());
	target.pwadPath = stdStr(editPwadPath->text().trimmed());
	target.description = stdStr(editDescription->toPlainText());

	if (radioMap->isChecked())
		target.launchParameters = 1;
	else if (radioSavegame->isChecked())
		target.launchParameters = 2;
	else if (radioDemoPlay->isChecked())
		target.launchParameters = 3;
	else
		target.launchParameters = 0;

	target.selectedLaunchMap = stdStr(editMap->text().trimmed());
	target.selectedLaunchSave = stdStr(editSavegame->text().trimmed());
	target.selectedLaunchDemoPlayback = stdStr(editDemoPlay->text().trimmed());
	target.recordDemo = chkRecordDemo->isChecked();
	target.selectedLaunchDemoRecord = stdStr(editRecordDemo->text().trimmed());

	target.difficulty = comboSkill->currentData().toInt();
	target.fastMonsters = chkFast->isChecked();
	target.respawnMonsters = chkRespawn->isChecked();
	target.noMonsters = chkNoMonsters->isChecked();
	target.allowMultiplayerItems = chkMultiItems->isChecked();
	target.pistolStartAllLevels = chkPistolStart->isChecked();

	target.playerName = stdStr(editPlayerName->text().trimmed());
	target.playerClass = stdStr(editPlayerClass->text().trimmed());
	target.playerGender = comboPlayerGender->currentIndex();
	target.wadLanguage = stdStr(comboWadLang->currentData().toString());

	target.remoteAddress = stdStr(editRemoteAddr->text().trimmed());
	target.remotePort = spinRemotePort->value();
	target.remoteTeam = spinRemoteTeam->value();

	target.hostPort = spinHostPort->value();
	target.maxPlayers = spinMaxPlayers->value();
	target.hostTicRate = comboTicRate->currentIndex();
	target.hostExtraTic = chkExtraTic->isChecked();
	target.hostGamemode = comboGamemode->currentIndex();
	target.hostTeam = spinHostTeam->value();

	target.complevel = stdStr(comboEngineComplevel->currentData().toString());
	target.compatLevel = comboCompatLevel->currentData().toInt();
	target.alwaysApplyDMFlags = alwaysApplyDMFlags;
	target.dmflags = dmflags;
	target.dmflags2 = dmflags2;
	target.dmflags3 = dmflags3;
	target.compatflags = compatflags;
	target.compatflags2 = compatflags2;

	target.configFilePath = stdStr(editConfigFile->text().trimmed());
	target.saveDirPath = stdStr(editSaveDir->text().trimmed());
	target.screenshotDirPath = stdStr(editShotDir->text().trimmed());
	target.demoDirPath = stdStr(editDemoDir->text().trimmed());
	target.copyModsToProfile = chkCopyModsToProfile ? chkCopyModsToProfile->isChecked() : true;
	target.mods = modPaths;
	target.disabledMods.clear();
	for (const auto& m : modPaths)
	{
		if (disabledModKeys.contains(normModKey(qStr(m))))
			target.disabledMods.push_back(m);
	}

	target.fullscreen = chkFullscreen->isChecked();
	target.loadSupportWads = chkSupportWads->isChecked();
	target.disableAutoload = chkDisableAutoload->isChecked();
	target.enableDynLights = chkDynLights->isChecked();
	if (radioOpenGL->isChecked())
		target.renderingAPI = 0;
	else if (radioGLES->isChecked())
		target.renderingAPI = 2;
	else
		target.renderingAPI = 1;
	target.extraLights = chkLights->isChecked();
	target.extraBrightmaps = chkBrightmaps->isChecked();
	target.extraWidescreen = chkWidescreen->isChecked();

	target.appendAdditionalParameters = stdStr(editAppendArgs->toPlainText().trimmed());
}

void ProfileSettingsDialog::saveToProfile()
{
	writeToProfile(profile);
}

bool ProfileSettingsDialog::hasUnsavedChanges() const
{
	Profile current = profile;
	writeToProfile(current);
	return !(current == initialSnapshot);
}

void ProfileSettingsDialog::reject()
{
	if (action == ActionResult::Cancelled && hasUnsavedChanges())
	{
		QMessageBox box(this);
		box.setIcon(QMessageBox::Question);
		box.setWindowTitle(LStr("LAUNCHER_UNSAVED_TITLE", "Unsaved Changes"));
		box.setText(LStr("LAUNCHER_UNSAVED_MSG", "You have unsaved changes to this profile. Discard them?"));
		QPushButton* btnDiscard = box.addButton(LStr("LAUNCHER_DISCARD", "Discard"), QMessageBox::DestructiveRole);
		QPushButton* btnKeepEditing = box.addButton(LStr("OPTVAL_CANCEL", "Cancel"), QMessageBox::RejectRole);
		box.setDefaultButton(btnKeepEditing);
		box.exec();
		if (box.clickedButton() != btnDiscard)
			return;
	}
	QDialog::reject();
}

void ProfileSettingsDialog::rebuildModList(const std::vector<std::string>& mods, int selectRow)
{
	modPaths = mods;
	listMods->setUpdatesEnabled(false);
	listMods->clear();

	const QColor iconColor = palette().color(QPalette::ButtonText);
	const QIcon delIcon = makeLauncherIcon("delete", iconColor, 14);
	const QIcon upIcon = makeLauncherIcon("up", iconColor, 14);
	const QIcon downIcon = makeLauncherIcon("down", iconColor, 14);
	for (int row = 0; row < static_cast<int>(modPaths.size()); ++row)
	{
		const QString fullPath = qStr(modPaths[row]);
		const QString modKey = normModKey(fullPath);
		const bool isEnabled = !disabledModKeys.contains(modKey);
		const bool fileExists = QFileInfo::exists(fullPath);
		const QString fileName = QFileInfo(fullPath).fileName();
		const QString baseName = fileName.isEmpty() ? fullPath : fileName;
		const QString displayText = fileExists ? baseName : (baseName + QStringLiteral("  [Missing]"));
		const QString tipText = fileExists ? fullPath : (QStringLiteral("Missing file:\n") + fullPath);

		QListWidgetItem* item = new QListWidgetItem(listMods);
		item->setToolTip(tipText);

		QWidget* rowWidget = new QWidget(listMods);
		QHBoxLayout* h = new QHBoxLayout(rowWidget);
		h->setContentsMargins(6, 2, 6, 2);
		h->setSpacing(6);

		QCheckBox* chkEnabled = new QCheckBox(rowWidget);
		chkEnabled->setChecked(isEnabled);
		chkEnabled->setToolTip("Enable or disable this mod for launches without removing it.");

		QLabel* lbl = new QLabel(displayText, rowWidget);
		lbl->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
		lbl->setToolTip(tipText);
		lbl->setEnabled(isEnabled);
		if (!fileExists)
		{
			QPalette lp = lbl->palette();
			lp.setColor(QPalette::WindowText, palette().color(QPalette::BrightText));
			lbl->setPalette(lp);
		}

		connect(chkEnabled, &QCheckBox::toggled, this, [this, modKey, lbl](bool on)
		{
			if (on)
				disabledModKeys.remove(modKey);
			else
				disabledModKeys.insert(modKey);
			lbl->setEnabled(on);
			updateCommandPreview();
		});

		h->addWidget(chkEnabled);
		h->addWidget(lbl, 1);

		QToolButton* btnDel = new QToolButton(rowWidget);
		btnDel->setIcon(delIcon);
		btnDel->setToolTip(LStr("LAUNCHER_REMOVE_MOD", "Remove Mod"));
		btnDel->setToolButtonStyle(Qt::ToolButtonIconOnly);
		btnDel->setAutoRaise(true);
		connect(btnDel, &QToolButton::clicked, this, [this, row, modKey]()
		{
			disabledModKeys.remove(modKey);
			modPaths.erase(modPaths.begin() + row);
			rebuildModList(modPaths, std::min(row, static_cast<int>(modPaths.size()) - 1));
			updateCommandPreview();
		});

		QToolButton* btnUp = new QToolButton(rowWidget);
		btnUp->setIcon(upIcon);
		btnUp->setToolTip("Move Up");
		btnUp->setEnabled(row > 0);
		btnUp->setAutoRaise(true);
		connect(btnUp, &QToolButton::clicked, this, [this, row]()
		{
			if (row > 0)
			{
				std::swap(modPaths[row], modPaths[row - 1]);
				rebuildModList(modPaths, row - 1);
				updateCommandPreview();
			}
		});

		QToolButton* btnDown = new QToolButton(rowWidget);
		btnDown->setIcon(downIcon);
		btnDown->setToolTip("Move Down");
		btnDown->setEnabled(row + 1 < static_cast<int>(modPaths.size()));
		btnDown->setAutoRaise(true);
		connect(btnDown, &QToolButton::clicked, this, [this, row]()
		{
			if (row + 1 < static_cast<int>(modPaths.size()))
			{
				std::swap(modPaths[row], modPaths[row + 1]);
				rebuildModList(modPaths, row + 1);
				updateCommandPreview();
			}
		});

		h->addWidget(btnDel);
		h->addWidget(btnUp);
		h->addWidget(btnDown);

		item->setSizeHint(QSize(0, std::max(30, rowWidget->sizeHint().height())));
		listMods->addItem(item);
		listMods->setItemWidget(item, rowWidget);
	}

	if (selectRow >= 0 && selectRow < listMods->count())
		listMods->setCurrentRow(selectRow);
	listMods->setUpdatesEnabled(true);
}

void ProfileSettingsDialog::appendModPath(const QString& filePath)
{
	if (filePath.isEmpty())
		return;

	QString finalPath = QFileInfo(filePath).absoluteFilePath();
	if (finalPath.isEmpty())
		finalPath = filePath;

	const bool copyToProfile = chkCopyModsToProfile ? chkCopyModsToProfile->isChecked() : true;
	if (copyToProfile && !profile.filePath.empty())
	{
		const QDir modsDir(QFileInfo(qStr(profile.filePath)).absolutePath() + "/mods");
		modsDir.mkpath(".");
		const QString dest = modsDir.filePath(QFileInfo(filePath).fileName());
		const QString srcAbs = QFileInfo(filePath).absoluteFilePath();
		const QString destAbs = QFileInfo(dest).absoluteFilePath();
		if (srcAbs.compare(destAbs, Qt::CaseInsensitive) != 0)
		{
			if (!QFile::exists(dest))
			{
				if (QFile::copy(filePath, dest))
					finalPath = dest;
			}
			else
			{
				const std::string srcMd5 = calculateFileMD5(stdStr(srcAbs));
				const std::string dstMd5 = calculateFileMD5(stdStr(destAbs));
				if (!srcMd5.empty() && srcMd5 == dstMd5)
				{
					finalPath = dest;
				}
				else
				{
					QMessageBox box(this);
					box.setIcon(QMessageBox::Question);
					box.setWindowTitle(LStr("LAUNCHER_MOD_EXISTS_TITLE", "Mod Already Exists"));
					box.setText(QString("A different file named '%1' already exists in this profile's mods folder.\n\nOverwrite it with the selected file?")
						.arg(QFileInfo(filePath).fileName()));
					QPushButton* btnOverwrite = box.addButton(LStr("LAUNCHER_OVERWRITE", "Overwrite"), QMessageBox::AcceptRole);
					QPushButton* btnKeep = box.addButton(LStr("LAUNCHER_KEEP_EXISTING", "Keep Existing"), QMessageBox::NoRole);
					box.addButton(QMessageBox::Cancel);
					box.setDefaultButton(btnOverwrite);
					box.exec();

					if (box.clickedButton() == btnOverwrite)
					{
						const QString tmpDest = dest + ".uztmp";
						QFile::remove(tmpDest);
						if (!QFile::copy(filePath, tmpDest) || !QFile::remove(dest) || !QFile::rename(tmpDest, dest))
						{
							QFile::remove(tmpDest);
							QMessageBox::warning(this, LStr("ERROR_FATAL", "Error"),
								QString("Could not overwrite '%1' in the profile's mods folder.").arg(QFileInfo(filePath).fileName()));
							return;
						}
						finalPath = dest;
					}
					else if (box.clickedButton() == btnKeep)
					{
						finalPath = dest;
					}
					else
					{
						return;
					}
				}
			}
		}
	}

	const std::string addedStd = stdStr(finalPath);
	const QString addedKey = normModKey(finalPath);
	const bool alreadyPresent = std::any_of(modPaths.begin(), modPaths.end(), [&](const std::string& existing)
	{
		return normModKey(qStr(existing)) == addedKey;
	});
	if (!alreadyPresent)
		modPaths.push_back(addedStd);
	disabledModKeys.remove(addedKey);
	rebuildModList(modPaths, static_cast<int>(modPaths.size()) - 1);
	updateCommandPreview();
}

void ProfileSettingsDialog::syncCompatModeFromFlags()
{
	if (!comboCompatLevel)
		return;
	const int customIdx = comboCompatLevel->findData(-1);
	const int matchedId = findMatchingCompatModeId(compatflags, compatflags2);
	if (matchedId >= 0)
	{
		if (customIdx >= 0)
			comboCompatLevel->removeItem(customIdx);
		const int idx = comboCompatLevel->findData(matchedId);
		comboCompatLevel->setCurrentIndex(idx >= 0 ? idx : 0);
	}
	else
	{
		int targetIdx = customIdx;
		if (targetIdx < 0)
		{
			comboCompatLevel->addItem(LStr("OPTVAL_CUSTOM", "Custom"), -1);
			targetIdx = comboCompatLevel->count() - 1;
		}
		comboCompatLevel->setCurrentIndex(targetIdx);
	}
}

void ProfileSettingsDialog::updateFlagSummary()
{
	if (!lblFlagSummary)
		return;
	lblFlagSummary->setText(QString("dmflags: %1 | dmflags2: %2 | dmflags3: %3\ncompatflags: %4 | compatflags2: %5")
		.arg(dmflags).arg(dmflags2).arg(dmflags3).arg(compatflags).arg(compatflags2));
}

void ProfileSettingsDialog::updateCommandPreview()
{
	if (!editCommandPreview || !comboPreviewMode)
		return;

	Profile preview = profile;
	writeToProfile(preview);
	const LaunchTargetMode mode = static_cast<LaunchTargetMode>(comboPreviewMode->currentData().toInt());
	const QString exe = quoteAdditionalArg(QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
	const QString args = QString::fromStdString(preview.giveLaunchCommand(mode, currentLauncherLanguage));
	editCommandPreview->setPlainText(args.isEmpty() ? exe : (exe + " " + args));
}

QString ProfileSettingsDialog::browsePath(const QString& caption, const QString& filter, const QString& current, bool directory)
{
	QString startDir;
	if (current.isEmpty())
	{
		startDir = profile.filePath.empty()
			? qStr(LauncherConst::PROFILE_DIR)
			: QFileInfo(qStr(profile.filePath)).absolutePath();
	}
	else
	{
		const QFileInfo fi(current);
		if ((directory || fi.isDir()) && fi.exists())
			startDir = fi.absoluteFilePath();
		else
			startDir = fi.absolutePath();
	}
	if (directory)
		return QFileDialog::getExistingDirectory(this, caption, startDir);
	return QFileDialog::getOpenFileName(this, caption, startDir, filter);
}

void ProfileSettingsDialog::onTypeChanged(int)
{
	const bool isIwad = comboType->currentData().toBool();
	if (pwadRowWidget)
		pwadRowWidget->setEnabled(!isIwad);
}

void ProfileSettingsDialog::onLaunchModeChanged()
{
	editMap->setEnabled(radioMap->isChecked());
	if (saveRowWidget)
		saveRowWidget->setEnabled(radioSavegame->isChecked());
	if (demoRowWidget)
		demoRowWidget->setEnabled(radioDemoPlay->isChecked());
}

void ProfileSettingsDialog::onOpenFlagEditor()
{
	FlagEditorDialog::FlagValues init;
	init.alwaysApplyDMFlags = alwaysApplyDMFlags;
	init.dmflags = dmflags;
	init.dmflags2 = dmflags2;
	init.dmflags3 = dmflags3;
	init.compatflags = compatflags;
	init.compatflags2 = compatflags2;

	FlagEditorDialog dlg(init, this);
	if (dlg.exec() == QDialog::Accepted)
	{
		const auto v = dlg.values();
		alwaysApplyDMFlags = v.alwaysApplyDMFlags;
		dmflags = v.dmflags;
		dmflags2 = v.dmflags2;
		dmflags3 = v.dmflags3;
		compatflags = v.compatflags;
		compatflags2 = v.compatflags2;

		{
			const QSignalBlocker b1(chkFast);
			const QSignalBlocker b2(chkRespawn);
			const QSignalBlocker b3(chkNoMonsters);
			const QSignalBlocker b4(chkMultiItems);
			const QSignalBlocker b5(chkPistolStart);
			chkFast->setChecked((dmflags & DF_FAST_MONSTERS) != 0);
			chkRespawn->setChecked((dmflags & DF_MONSTERS_RESPAWN) != 0);
			chkNoMonsters->setChecked((dmflags & DF_NO_MONSTERS) != 0);
			chkMultiItems->setChecked((dmflags2 & DF2_ALWAYS_SPAWN_MULTI) != 0);
			chkPistolStart->setChecked((dmflags3 & DF3_PISTOL_START) != 0);
		}
		syncCompatModeFromFlags();
		updateFlagSummary();
		updateCommandPreview();
	}
}

void ProfileSettingsDialog::onAddModFiles()
{
	const QString startDir = (!defaultModsDirectory.isEmpty() && QFileInfo::exists(defaultModsDirectory))
		? defaultModsDirectory
		: (profile.filePath.empty() ? qStr(LauncherConst::PROFILE_DIR) : QFileInfo(qStr(profile.filePath)).absolutePath());
	const QStringList files = QFileDialog::getOpenFileNames(this, LStr("LAUNCHER_ADD_MOD", "Add Mod Files"), startDir,
		"Doom Mods (*.wad *.pk3 *.pk7 *.ipk3 *.zip *.deh *.bex);;All Files (*.*)");
	for (const QString& f : files)
		appendModPath(f);
}

void ProfileSettingsDialog::onSaveAndReturn()
{
	saveToProfile();
	if (!profile.coverImagePath.empty() && !profile.filePath.empty())
	{
		const QFileInfo covFi(qStr(profile.coverImagePath));
		const QString profFolder = QDir::cleanPath(QFileInfo(qStr(profile.filePath)).absolutePath());
		const QString profPrefix = profFolder.endsWith('/') ? profFolder : (profFolder + '/');
		const QString covAbs = QDir::cleanPath(covFi.absoluteFilePath());
		if (covFi.exists() && covFi.isFile() && !covAbs.startsWith(profPrefix, Qt::CaseInsensitive))
		{
			QDir().mkpath(profFolder);
			const QString ext = covFi.suffix().isEmpty() ? QStringLiteral("png") : covFi.suffix().toLower();
			const QString destCover = QDir(profFolder).filePath("cover." + ext);
			const QString tmpCover = destCover + ".uztmp";
			QFile::remove(tmpCover);
			if (QFile::copy(covAbs, tmpCover))
			{
				QFile::remove(destCover);
				if (QFile::rename(tmpCover, destCover))
					profile.coverImagePath = stdStr(destCover);
				else
					QFile::remove(tmpCover);
			}
		}
	}
	if (profile.coverImagePath.empty() || !QFileInfo::exists(qStr(profile.coverImagePath)))
		tryExtractCoverForProfile(profile);
	if (!saveProfileToDisk(profile))
	{
		QMessageBox::warning(this, LStr("ERROR_FATAL", "Error"), "Failed to save profile to disk.");
		return;
	}
	action = ActionResult::Saved;
	accept();
}

void ProfileSettingsDialog::onCloneProfile()
{
	Profile draft = profile;
	writeToProfile(draft);

	bool ok = false;
	const QString suggested = qStr(draft.title) + " (Copy)";
	const QString title = QInputDialog::getText(this, LStr("LAUNCHER_CLONE_PROFILE", "Clone Profile"),
		LStr("LAUNCHER_CLONE_PROMPT", "New profile title:"), QLineEdit::Normal, suggested, &ok);
	if (!ok || title.trimmed().isEmpty())
		return;

	if (!cloneProfile(draft, stdStr(title.trimmed()), cloned))
	{
		QMessageBox::warning(this, LStr("ERROR_FATAL", "Error"), "Failed to clone profile directory.");
		return;
	}
	action = ActionResult::Cloned;
	accept();
}

void ProfileSettingsDialog::onExportProfile()
{
	Profile draft = profile;
	writeToProfile(draft);

	const QString defaultFile = QDir::home().filePath(qStr(sanitizeFolderName(draft.title)) + ".uzdp");
	const QString dest = QFileDialog::getSaveFileName(this, LStr("LAUNCHER_EXPORT_PROFILE", "Export Profile"), defaultFile,
		"UZDoom Profile Archive (*.uzdp);;ZIP Archive (*.zip)");
	if (dest.isEmpty())
		return;

	// Temporarily serialize draft metadata for export without mutating the saved profile on disk if user later cancels.
	QByteArray origBytes;
	bool hadOrigFile = false;
	if (!draft.filePath.empty())
	{
		QFile origFile(qStr(draft.filePath));
		if (origFile.open(QIODevice::ReadOnly))
		{
			origBytes = origFile.readAll();
			hadOrigFile = true;
		}
		saveProfileToDisk(draft);
	}

	std::string err;
	const bool exported = exportProfileToZip(draft, stdStr(dest), err);

	if (hadOrigFile && !draft.filePath.empty())
	{
		QFile restoreFile(qStr(draft.filePath));
		if (restoreFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
			restoreFile.write(origBytes);
	}

	if (!exported)
		QMessageBox::warning(this, LStr("ERROR_FATAL", "Error"), qStr(err));
	else
		QMessageBox::information(this, LStr("LAUNCHER_EXPORT_PROFILE", "Export Profile"), "Profile exported successfully.");
}

void ProfileSettingsDialog::onDeleteProfile()
{
	const QMessageBox::StandardButton reply = QMessageBox::question(this,
		LStr("LAUNCHER_DELETE_PROFILE", "Delete Profile"),
		QString("Delete profile '%1' and all its local saves/configs/mods?").arg(qStr(profile.title)),
		QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
	if (reply != QMessageBox::Yes)
		return;

	if (!deleteProfileDirectory(profile))
	{
		QMessageBox::warning(this, LStr("ERROR_FATAL", "Error"), "Failed to remove profile directory.");
		return;
	}
	action = ActionResult::Deleted;
	accept();
}

void ProfileSettingsDialog::dragEnterEvent(QDragEnterEvent* event)
{
	if (!event->mimeData() || !event->mimeData()->hasUrls())
		return;
	const bool onGeneralTab = (tabs && tabs->currentIndex() == 0);
	for (const QUrl& u : event->mimeData()->urls())
	{
		if (!u.isLocalFile())
			continue;
		const QString local = u.toLocalFile();
		if (isSupportedModDropFile(local) || (onGeneralTab && isSupportedImageDropFile(local)))
		{
			event->acceptProposedAction();
			return;
		}
	}
}

void ProfileSettingsDialog::dropEvent(QDropEvent* event)
{
	if (!event->mimeData() || !event->mimeData()->hasUrls())
		return;
	const bool onGeneralTab = (tabs && tabs->currentIndex() == 0);
	bool accepted = false;
	for (const QUrl& u : event->mimeData()->urls())
	{
		if (!u.isLocalFile())
			continue;
		const QString local = u.toLocalFile();
		if (onGeneralTab && isSupportedImageDropFile(local))
		{
			editCoverImage->setText(local);
			accepted = true;
		}
		else if (isSupportedModDropFile(local))
		{
			appendModPath(local);
			accepted = true;
		}
	}
	if (accepted)
		event->acceptProposedAction();
}
