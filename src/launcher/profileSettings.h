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
#include <QSet>
#include <QString>
#include <string>
#include <vector>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPlainTextEdit;
class QPushButton;
class QRadioButton;
class QSpinBox;
class QTabWidget;
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

	explicit ProfileSettingsDialog(
		Profile& targetProfile,
		const std::vector<Profile>& profilesList,
		const QString& defaultModsDir = {},
		const std::string& launcherLanguage = {},
		QWidget* parent = nullptr);

	ActionResult actionResult() const { return action; }
	const Profile& clonedProfile() const { return cloned; }

public slots:
	void reject() override;

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
	QWidget* buildLogsTab();

	void loadFromProfile();
	void writeToProfile(Profile& target) const;
	void saveToProfile();
	bool hasUnsavedChanges() const;
	void populateQuickIwadCombo();
	void syncQuickIwadFromPath(const QString& path);
	QString profileLogsDirPath() const;
	QString selectedLogFilePath() const;
	void refreshLogView();
	void loadSelectedLogFile();
	void applyLogFilter();
	void rebuildModList(const std::vector<std::string>& mods, int selectRow = -1);
	void appendModPath(const QString& filePath);
	void syncCompatModeFromFlags();
	void updateFlagSummary();
	void updateCommandPreview();
	QString browsePath(const QString& caption, const QString& filter, const QString& current, bool directory = false);

	Profile& profile;
	Profile initialSnapshot;
	const std::vector<Profile>& allProfiles;
	QString defaultModsDirectory;
	std::string currentLauncherLanguage;
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
	QWidget* saveRowWidget = nullptr;
	QLineEdit* editSavegame = nullptr;
	QWidget* demoRowWidget = nullptr;
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
	QCheckBox* chkCopyModsToProfile = nullptr;
	QListWidget* listMods = nullptr;
	std::vector<std::string> modPaths;
	QSet<QString> disabledModKeys;

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

	QTabWidget* tabs = nullptr;
	QPlainTextEdit* editAppendArgs = nullptr;
	QComboBox* comboPreviewMode = nullptr;
	QPlainTextEdit* editCommandPreview = nullptr;
	QPushButton* btnCopyCommand = nullptr;

	QComboBox* comboLogFiles = nullptr;
	QCheckBox* chkLogProblemsOnly = nullptr;
	QLineEdit* editLogFilter = nullptr;
	QPlainTextEdit* editLogView = nullptr;
	QPushButton* btnCopyLog = nullptr;
	QPushButton* btnOpenLogFile = nullptr;
	QPushButton* btnOpenLogsDir = nullptr;
	QPushButton* btnDeleteLog = nullptr;
	QPushButton* btnClearAllLogs = nullptr;
	QString fullLogText;
};
