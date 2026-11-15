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

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
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
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>

namespace
{
	QWidget* createPathBrowseRow(QLineEdit* edit, QPushButton* btn, QWidget* parent)
	{
		QWidget* w = new QWidget(parent);
		QHBoxLayout* h = new QHBoxLayout(w);
		h->setContentsMargins(0, 0, 0, 0);
		h->setSpacing(4);
		h->addWidget(edit, 1);
		btn->setFixedWidth(32);
		h->addWidget(btn);
		return w;
	}
}

ProfileSettingsDialog::ProfileSettingsDialog(Profile& targetProfile, const std::vector<Profile>& profilesList, QWidget* parent)
	: QDialog(parent), profile(targetProfile), allProfiles(profilesList)
{
	setWindowTitle(LStr("LAUNCHER_PROFILE_SETTINGS", "Profile Settings"));
	setAcceptDrops(true);
	setMinimumSize(820, 680);
	resize(900, 760);

	QVBoxLayout* mainLayout = new QVBoxLayout(this);

	QTabWidget* tabs = new QTabWidget(this);
	tabs->addTab(buildGeneralTab(), LStr("PICKER_GENERAL", "General"));
	tabs->addTab(buildLaunchTab(), LStr("LAUNCHER_TAB_LAUNCH", "Launch"));
	tabs->addTab(buildFilesTab(), LStr("LAUNCHER_TAB_FILES", "Files"));
	tabs->addTab(buildOutputTab(), LStr("LAUNCHER_TAB_OUTPUT", "Output"));
	tabs->addTab(buildAdvancedTab(), LStr("LAUNCHER_TAB_ADVANCED", "Advanced"));
	mainLayout->addWidget(tabs, 1);

	QHBoxLayout* bottomBar = new QHBoxLayout();

	QPushButton* btnClone = new QPushButton(LStr("LAUNCHER_CLONE_PROFILE", "Clone Profile..."), this);
	connect(btnClone, &QPushButton::clicked, this, &ProfileSettingsDialog::onCloneProfile);
	bottomBar->addWidget(btnClone);

	QPushButton* btnExport = new QPushButton(LStr("LAUNCHER_EXPORT_PROFILE", "Export (.uzdp)..."), this);
	connect(btnExport, &QPushButton::clicked, this, &ProfileSettingsDialog::onExportProfile);
	bottomBar->addWidget(btnExport);

	QPushButton* btnDelete = new QPushButton("- " + LStr("LAUNCHER_DELETE_PROFILE", "Delete Profile..."), this);
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

	modeGrid->addWidget(radioNormal, 0, 0);
	modeGrid->addWidget(radioMap, 1, 0);
	modeGrid->addWidget(editMap, 1, 1);
	modeGrid->addWidget(radioSavegame, 2, 0);
	modeGrid->addWidget(createPathBrowseRow(editSavegame, btnSave, grpMode), 2, 1);
	modeGrid->addWidget(radioDemoPlay, 3, 0);
	modeGrid->addWidget(createPathBrowseRow(editDemoPlay, btnDemo, grpMode), 3, 1);
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

	QGroupBox* grpCompat = new QGroupBox(LStr("CMPTMNU_TITLE", "Compatibility"), tab);
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
	QPushButton* btnCfg = new QPushButton("...", tab);
	connect(btnCfg, &QPushButton::clicked, this, [this]()
	{
		const QString p = browsePath("Select Config File", "Config (*.ini);;All Files (*.*)", editConfigFile->text());
		if (!p.isEmpty()) editConfigFile->setText(p);
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

	pathsForm->addRow(LStr("LAUNCHER_CONFIG_FILE", "Config File:"), createPathBrowseRow(editConfigFile, btnCfg, tab));
	pathsForm->addRow(LStr("LAUNCHER_SAVE_DIR", "Save Directory:"), createPathBrowseRow(editSaveDir, btnSaveDir, tab));
	pathsForm->addRow(LStr("LAUNCHER_SHOT_DIR", "Screenshot Dir:"), createPathBrowseRow(editShotDir, btnShotDir, tab));
	pathsForm->addRow(LStr("LAUNCHER_DEMO_DIR", "Demo Directory:"), createPathBrowseRow(editDemoDir, btnDemoDir, tab));
	vbox->addLayout(pathsForm);

	QHBoxLayout* modsHeader = new QHBoxLayout();
	QLabel* lblMods = new QLabel(LStr("LAUNCHER_MODS", "Mods"), tab);
	QFont f = lblMods->font();
	f.setBold(true);
	lblMods->setFont(f);
	modsHeader->addWidget(lblMods);
	modsHeader->addStretch();

	QPushButton* btnAddMod = new QPushButton("+ " + LStr("LAUNCHER_ADD_MOD", "Add Mod File"), tab);
	btnAddMod->setMinimumSize(160, 32);
	connect(btnAddMod, &QPushButton::clicked, this, &ProfileSettingsDialog::onAddModFiles);
	modsHeader->addWidget(btnAddMod);
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
	chkSupportWads = new QCheckBox(LStr("PICKER_SUPPORTWADS", "Load Support WADs"), grpGen);
	chkDisableAutoload = new QCheckBox(LStr("PICKER_NOAUTOLOAD", "Disable Autoload"), grpGen);
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

	vbox->addWidget(new QLabel(LStr("LAUNCHER_APPEND_ARGS", "Append Additional Parameters"), tab));
	editAppendArgs = new QPlainTextEdit(tab);
	editAppendArgs->setFixedHeight(100);
	editAppendArgs->setPlaceholderText("Parameters appended after profile options...");
	vbox->addWidget(editAppendArgs);
	vbox->addStretch();

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
	chkFast->setChecked(profile.fastMonsters);
	chkRespawn->setChecked(profile.respawnMonsters);
	chkNoMonsters->setChecked(profile.noMonsters);
	chkMultiItems->setChecked(profile.allowMultiplayerItems);
	chkPistolStart->setChecked(profile.pistolStartAllLevels);

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

	const int clIdx = comboEngineComplevel->findData(qStr(profile.complevel));
	comboEngineComplevel->setCurrentIndex(clIdx >= 0 ? clIdx : 0);

	alwaysApplyDMFlags = profile.alwaysApplyDMFlags;
	dmflags = profile.dmflags;
	dmflags2 = profile.dmflags2;
	dmflags3 = profile.dmflags3;
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
	editSaveDir->setText(qStr(profile.saveDirPath));
	editShotDir->setText(qStr(profile.screenshotDirPath));
	editDemoDir->setText(qStr(profile.demoDirPath));
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
}

void ProfileSettingsDialog::saveToProfile()
{
	profile.title = stdStr(editTitle->text().trimmed().isEmpty() ? QString("New Profile") : editTitle->text().trimmed());
	profile.author = stdStr(editAuthor->text().trimmed());
	profile.releaseDate = stdStr(editReleaseDate->text().trimmed());
	profile.coverImagePath = stdStr(editCoverImage->text().trimmed());
	profile.isIWAD = comboType->currentData().toBool();
	profile.iwadPath = stdStr(editIwadPath->text().trimmed());
	profile.pwadPath = stdStr(editPwadPath->text().trimmed());
	profile.description = stdStr(editDescription->toPlainText());

	if (radioMap->isChecked())
		profile.launchParameters = 1;
	else if (radioSavegame->isChecked())
		profile.launchParameters = 2;
	else if (radioDemoPlay->isChecked())
		profile.launchParameters = 3;
	else
		profile.launchParameters = 0;

	profile.selectedLaunchMap = stdStr(editMap->text().trimmed());
	profile.selectedLaunchSave = stdStr(editSavegame->text().trimmed());
	profile.selectedLaunchDemoPlayback = stdStr(editDemoPlay->text().trimmed());
	profile.recordDemo = chkRecordDemo->isChecked();
	profile.selectedLaunchDemoRecord = stdStr(editRecordDemo->text().trimmed());

	profile.difficulty = comboSkill->currentData().toInt();
	profile.fastMonsters = chkFast->isChecked();
	profile.respawnMonsters = chkRespawn->isChecked();
	profile.noMonsters = chkNoMonsters->isChecked();
	profile.allowMultiplayerItems = chkMultiItems->isChecked();
	profile.pistolStartAllLevels = chkPistolStart->isChecked();

	profile.playerName = stdStr(editPlayerName->text().trimmed());
	profile.playerClass = stdStr(editPlayerClass->text().trimmed());
	profile.playerGender = comboPlayerGender->currentIndex();
	profile.wadLanguage = stdStr(comboWadLang->currentData().toString());

	profile.remoteAddress = stdStr(editRemoteAddr->text().trimmed());
	profile.remotePort = spinRemotePort->value();
	profile.remoteTeam = spinRemoteTeam->value();

	profile.hostPort = spinHostPort->value();
	profile.maxPlayers = spinMaxPlayers->value();
	profile.hostTicRate = comboTicRate->currentIndex();
	profile.hostExtraTic = chkExtraTic->isChecked();
	profile.hostGamemode = comboGamemode->currentIndex();
	profile.hostTeam = spinHostTeam->value();

	profile.complevel = stdStr(comboEngineComplevel->currentData().toString());
	profile.compatLevel = comboCompatLevel->currentData().toInt();
	profile.alwaysApplyDMFlags = alwaysApplyDMFlags;
	profile.dmflags = dmflags;
	profile.dmflags2 = dmflags2;
	profile.dmflags3 = dmflags3;
	profile.compatflags = compatflags;
	profile.compatflags2 = compatflags2;

	profile.configFilePath = stdStr(editConfigFile->text().trimmed());
	profile.saveDirPath = stdStr(editSaveDir->text().trimmed());
	profile.screenshotDirPath = stdStr(editShotDir->text().trimmed());
	profile.demoDirPath = stdStr(editDemoDir->text().trimmed());
	profile.mods = modPaths;

	profile.fullscreen = chkFullscreen->isChecked();
	profile.loadSupportWads = chkSupportWads->isChecked();
	profile.disableAutoload = chkDisableAutoload->isChecked();
	profile.enableDynLights = chkDynLights->isChecked();
	if (radioOpenGL->isChecked())
		profile.renderingAPI = 0;
	else if (radioGLES->isChecked())
		profile.renderingAPI = 2;
	else
		profile.renderingAPI = 1;
	profile.extraLights = chkLights->isChecked();
	profile.extraBrightmaps = chkBrightmaps->isChecked();
	profile.extraWidescreen = chkWidescreen->isChecked();

	profile.appendAdditionalParameters = stdStr(editAppendArgs->toPlainText().trimmed());
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
		const QString fileName = QFileInfo(fullPath).fileName();

		QListWidgetItem* item = new QListWidgetItem(listMods);
		item->setToolTip(fullPath);

		QWidget* rowWidget = new QWidget(listMods);
		QHBoxLayout* h = new QHBoxLayout(rowWidget);
		h->setContentsMargins(6, 2, 6, 2);
		h->setSpacing(4);

		QLabel* lbl = new QLabel(fileName.isEmpty() ? fullPath : fileName, rowWidget);
		lbl->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
		lbl->setToolTip(fullPath);
		h->addWidget(lbl, 1);

		QToolButton* btnDel = new QToolButton(rowWidget);
		btnDel->setIcon(delIcon);
		btnDel->setToolTip(LStr("LAUNCHER_REMOVE_MOD", "Remove Mod"));
		btnDel->setToolButtonStyle(Qt::ToolButtonIconOnly);
		btnDel->setAutoRaise(true);
		connect(btnDel, &QToolButton::clicked, this, [this, row]()
		{
			modPaths.erase(modPaths.begin() + row);
			rebuildModList(modPaths, std::min(row, static_cast<int>(modPaths.size()) - 1));
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

	QString finalPath = filePath;
	if (!profile.filePath.empty())
	{
		const QDir modsDir(QFileInfo(qStr(profile.filePath)).absolutePath() + "/mods");
		modsDir.mkpath(".");
		const QString dest = modsDir.filePath(QFileInfo(filePath).fileName());
		if (QFileInfo(filePath).absoluteFilePath() != QFileInfo(dest).absoluteFilePath())
		{
			if (!QFile::exists(dest) && QFile::copy(filePath, dest))
				finalPath = dest;
			else if (QFile::exists(dest))
				finalPath = dest;
		}
	}

	const std::string addedStd = stdStr(finalPath);
	if (std::find(modPaths.begin(), modPaths.end(), addedStd) == modPaths.end())
		modPaths.push_back(addedStd);
	rebuildModList(modPaths, static_cast<int>(modPaths.size()) - 1);
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

QString ProfileSettingsDialog::browsePath(const QString& caption, const QString& filter, const QString& current, bool directory)
{
	const QString startDir = current.isEmpty()
		? (profile.filePath.empty() ? qStr(LauncherConst::PROFILE_DIR) : QFileInfo(qStr(profile.filePath)).absolutePath())
		: QFileInfo(current).absolutePath();
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
	editSavegame->setEnabled(radioSavegame->isChecked());
	editDemoPlay->setEnabled(radioDemoPlay->isChecked());
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

		chkFast->setChecked((dmflags & DF_FAST_MONSTERS) != 0);
		chkRespawn->setChecked((dmflags & DF_MONSTERS_RESPAWN) != 0);
		chkNoMonsters->setChecked((dmflags & DF_NO_MONSTERS) != 0);
		chkMultiItems->setChecked((dmflags2 & DF2_ALWAYS_SPAWN_MULTI) != 0);
		chkPistolStart->setChecked((dmflags3 & DF3_PISTOL_START) != 0);
		syncCompatModeFromFlags();
		updateFlagSummary();
	}
}

void ProfileSettingsDialog::onAddModFiles()
{
	const QString startDir = profile.filePath.empty() ? qStr(LauncherConst::PROFILE_DIR) : QFileInfo(qStr(profile.filePath)).absolutePath();
	const QStringList files = QFileDialog::getOpenFileNames(this, LStr("LAUNCHER_ADD_MOD", "Add Mod Files"), startDir,
		"Doom Mods (*.wad *.pk3 *.pk7 *.ipk3 *.zip *.deh *.bex);;All Files (*.*)");
	for (const QString& f : files)
		appendModPath(f);
}

void ProfileSettingsDialog::onSaveAndReturn()
{
	saveToProfile();
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
	saveToProfile();
	saveProfileToDisk(profile);

	bool ok = false;
	const QString suggested = qStr(profile.title) + " (Copy)";
	const QString title = QInputDialog::getText(this, LStr("LAUNCHER_CLONE_PROFILE", "Clone Profile"),
		LStr("LAUNCHER_CLONE_PROMPT", "New profile title:"), QLineEdit::Normal, suggested, &ok);
	if (!ok || title.trimmed().isEmpty())
		return;

	if (!cloneProfile(profile, stdStr(title.trimmed()), cloned))
	{
		QMessageBox::warning(this, LStr("ERROR_FATAL", "Error"), "Failed to clone profile directory.");
		return;
	}
	action = ActionResult::Cloned;
	accept();
}

void ProfileSettingsDialog::onExportProfile()
{
	saveToProfile();
	saveProfileToDisk(profile);

	const QString defaultFile = qStr(sanitizeFolderName(profile.title)) + ".uzdp";
	const QString dest = QFileDialog::getSaveFileName(this, LStr("LAUNCHER_EXPORT_PROFILE", "Export Profile"), defaultFile,
		"UZDoom Profile Archive (*.uzdp);;ZIP Archive (*.zip)");
	if (dest.isEmpty())
		return;

	std::string err;
	if (!exportProfileToZip(profile, stdStr(dest), err))
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
	if (event->mimeData()->hasUrls())
		event->acceptProposedAction();
}

void ProfileSettingsDialog::dropEvent(QDropEvent* event)
{
	const QList<QUrl> urls = event->mimeData()->urls();
	for (const QUrl& u : urls)
	{
		if (u.isLocalFile())
			appendModPath(u.toLocalFile());
	}
	event->acceptProposedAction();
}
