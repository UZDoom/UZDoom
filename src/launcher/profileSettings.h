/*
** profileSettings.h
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

#include "profile.h"

#include <QDialog>
#include <vector>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPlainTextEdit;
class QRadioButton;
class QSpinBox;
class QWidget;

class ProfileSettingsDialog : public QDialog
{
	Q_OBJECT
public:
	enum class ActionResult
	{
		Cancelled = 0,
		Saved = 1,
		Cloned = 2,
		Deleted = 3
	};

	explicit ProfileSettingsDialog(Profile& targetProfile, const std::vector<Profile>& profilesList, QWidget* parent = nullptr);

	ActionResult actionResult() const { return action; }
	const Profile& clonedProfile() const { return cloned; }

protected:
	void dragEnterEvent(QDragEnterEvent* event) override;
	void dropEvent(QDropEvent* event) override;

private slots:
	void onSaveAndReturn();
	void onCloneProfile();
	void onExportProfile();
	void onDeleteProfile();
	void onTypeChanged(int index);
	void onLaunchModeChanged();
	void onOpenFlagEditor();
	void onAddModFiles();

private:
	QWidget* buildGeneralTab();
	QWidget* buildLaunchTab();
	QWidget* buildFilesTab();
	QWidget* buildOutputTab();
	QWidget* buildAdvancedTab();

	void loadFromProfile();
	void saveToProfile();
	void rebuildModList(const std::vector<std::string>& mods, int selectRow = -1);
	void appendModPath(const QString& filePath);
	void syncCompatModeFromFlags();
	void updateFlagSummary();
	QString browsePath(const QString& caption, const QString& filter, const QString& current, bool directory = false);

	Profile& profile;
	const std::vector<Profile>& allProfiles;
	ActionResult action = ActionResult::Cancelled;
	Profile cloned;

	bool alwaysApplyDMFlags = false;
	uint32_t dmflags = 0;
	uint32_t dmflags2 = 0;
	uint32_t dmflags3 = 0;
	uint32_t compatflags = 0;
	uint32_t compatflags2 = 0;

	QLineEdit* editTitle = nullptr;
	QLineEdit* editAuthor = nullptr;
	QLineEdit* editReleaseDate = nullptr;
	QLineEdit* editCoverImage = nullptr;
	QComboBox* comboType = nullptr;
	QLineEdit* editIwadPath = nullptr;
	QComboBox* comboQuickIwad = nullptr;
	QWidget* pwadRowWidget = nullptr;
	QLineEdit* editPwadPath = nullptr;
	QPlainTextEdit* editDescription = nullptr;

	QRadioButton* radioNormal = nullptr;
	QRadioButton* radioMap = nullptr;
	QRadioButton* radioSavegame = nullptr;
	QRadioButton* radioDemoPlay = nullptr;
	QLineEdit* editMap = nullptr;
	QLineEdit* editSavegame = nullptr;
	QLineEdit* editDemoPlay = nullptr;
	QCheckBox* chkRecordDemo = nullptr;
	QLineEdit* editRecordDemo = nullptr;

	QComboBox* comboSkill = nullptr;
	QCheckBox* chkFast = nullptr;
	QCheckBox* chkRespawn = nullptr;
	QCheckBox* chkNoMonsters = nullptr;
	QCheckBox* chkMultiItems = nullptr;
	QCheckBox* chkPistolStart = nullptr;

	QLineEdit* editPlayerName = nullptr;
	QLineEdit* editPlayerClass = nullptr;
	QComboBox* comboPlayerGender = nullptr;
	QComboBox* comboWadLang = nullptr;

	QLineEdit* editRemoteAddr = nullptr;
	QSpinBox* spinRemotePort = nullptr;
	QSpinBox* spinRemoteTeam = nullptr;

	QSpinBox* spinHostPort = nullptr;
	QSpinBox* spinMaxPlayers = nullptr;
	QComboBox* comboTicRate = nullptr;
	QCheckBox* chkExtraTic = nullptr;
	QComboBox* comboGamemode = nullptr;
	QSpinBox* spinHostTeam = nullptr;

	QComboBox* comboEngineComplevel = nullptr;
	QComboBox* comboCompatLevel = nullptr;
	QLabel* lblFlagSummary = nullptr;

	QLineEdit* editConfigFile = nullptr;
	QLineEdit* editSaveDir = nullptr;
	QLineEdit* editShotDir = nullptr;
	QLineEdit* editDemoDir = nullptr;
	QListWidget* listMods = nullptr;
	std::vector<std::string> modPaths;

	QCheckBox* chkFullscreen = nullptr;
	QCheckBox* chkSupportWads = nullptr;
	QCheckBox* chkDisableAutoload = nullptr;
	QCheckBox* chkDynLights = nullptr;
	QRadioButton* radioVulkan = nullptr;
	QRadioButton* radioOpenGL = nullptr;
	QRadioButton* radioGLES = nullptr;
	QCheckBox* chkLights = nullptr;
	QCheckBox* chkBrightmaps = nullptr;
	QCheckBox* chkWidescreen = nullptr;

	QPlainTextEdit* editAppendArgs = nullptr;
};
