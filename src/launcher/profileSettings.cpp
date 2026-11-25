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
#include "theme.h"

#include "doomdef.h"
#include "findfile.h"
#include "gstrings.h"

#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QDateTime>
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
#include <QRegularExpression>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QSyntaxHighlighter>
#include <QTabWidget>
#include <QTextCharFormat>
#include <QTimer>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>
#include <algorithm>

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
		const QString clean = QDir::cleanPath(path).replace('\\', '/');
#ifdef Q_OS_WIN
		return clean.toLower();
#else
		return clean;
#endif
	}

	QString expandProfilePathTokens(const QString& raw, const QString& baseDir)
	{
		QString norm = QDir::cleanPath(raw.trimmed()).replace('\\', '/');
		if (norm.isEmpty())
			return {};

		auto tryExpand = [&](const QString& token, const std::string& base) -> bool
		{
			if (base.empty())
				return false;
			if (norm.compare(token, Qt::CaseInsensitive) == 0)
			{
				norm = QDir::cleanPath(QString::fromStdString(base)).replace('\\', '/');
				return true;
			}
			if (norm.startsWith(token + QLatin1Char('/'), Qt::CaseInsensitive))
			{
				const QString baseNorm = QDir::cleanPath(QString::fromStdString(base)).replace('\\', '/');
				norm = QDir::cleanPath(QDir(baseNorm).filePath(norm.mid(token.size() + 1))).replace('\\', '/');
				return true;
			}
			return false;
		};

		if (!tryExpand(QStringLiteral("${LAUNCHER_DIR}"), LauncherConst::ROOT_DIR)
			&& !tryExpand(QStringLiteral("${EXEC_DIR}"), LauncherConst::EXEC_DIR)
			&& !QFileInfo(norm).isAbsolute() && !baseDir.isEmpty())
		{
			norm = QDir::cleanPath(QDir(baseDir).filePath(norm)).replace('\\', '/');
		}
		return norm;
	}

	void ensureWritableForRemoval(const QString& path)
	{
		if (QFileInfo::exists(path))
			QFile::setPermissions(path, QFile::permissions(path) | QFileDevice::WriteOwner | QFileDevice::WriteUser);
	}

	bool safeReplaceFile(const QString& srcPath, const QString& destPath)
	{
		const QString tmpPath = destPath + QStringLiteral(".uztmp");
		const QString bakPath = destPath + QStringLiteral(".uzbak");
		ensureWritableForRemoval(tmpPath);
		ensureWritableForRemoval(bakPath);
		QFile::remove(tmpPath);
		QFile::remove(bakPath);

		if (!QFile::copy(srcPath, tmpPath))
			return false;

		const bool hadExisting = QFileInfo::exists(destPath);
		if (hadExisting)
		{
			ensureWritableForRemoval(destPath);
			if (!QFile::rename(destPath, bakPath))
			{
				ensureWritableForRemoval(tmpPath);
				QFile::remove(tmpPath);
				return false;
			}
		}
		if (!QFile::rename(tmpPath, destPath))
		{
			ensureWritableForRemoval(tmpPath);
			QFile::remove(tmpPath);
			if (hadExisting)
				QFile::rename(bakPath, destPath);
			return false;
		}
		if (hadExisting)
		{
			ensureWritableForRemoval(bakPath);
			QFile::remove(bakPath);
		}
		return true;
	}

	QString quoteAdditionalArg(const QString& arg)
	{
		if (!arg.isEmpty() && !arg.contains(' ') && !arg.contains('\t') && !arg.contains('"'))
			return arg;
		// QProcess::splitCommand() treats backslashes literally and uses """ for a literal quote.
		QString escaped = arg;
		escaped.replace('"', QStringLiteral("\"\"\""));
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

	enum class LogLineSeverity
	{
		Normal = 0,
		Warning = 1,
		Error = 2,
		Header = 3
	};

	LogLineSeverity classifyLogLine(const QString& line)
	{
		const QString trimmed = line.trimmed();
		if (trimmed.isEmpty())
			return LogLineSeverity::Normal;

		if (trimmed.startsWith(QStringLiteral("Command:"), Qt::CaseSensitive))
			return LogLineSeverity::Normal;

		if (trimmed.startsWith(QStringLiteral("===")) && trimmed.endsWith(QStringLiteral("===")))
		{
			if (trimmed.contains(QStringLiteral("crashed"), Qt::CaseInsensitive)
				|| trimmed.contains(QStringLiteral("failed to start"), Qt::CaseInsensitive)
				|| (trimmed.contains(QStringLiteral("Process exited (code "), Qt::CaseInsensitive)
					&& !trimmed.contains(QStringLiteral("(code 0)"), Qt::CaseInsensitive)))
				return LogLineSeverity::Error;
			return LogLineSeverity::Header;
		}

		static const QRegularExpression reError(
			QStringLiteral("\\bscript error\\b|\\bfatal(?:\\s+error)?\\b|\\bexecution could not continue\\b"
				"|\\bvm execution aborted\\b|\\baccessed null\\b|\\baddress zero\\b|\\bdivision by zero\\b"
				"|\\barray access out of bounds\\b|\\bsyntax error\\b|^\\s*(?:error|err)\\s*[:!]|:\\s*error\\s*:"
				"|\\b[1-9]\\d*\\s+errors?\\b|\\binit failed\\b|\\bfailed to (?:load|open|init|compile|parse)\\b"
				"|\\bcould not (?:open|load|find|create|init)\\b|\\bunable to (?:open|load|find|init)\\b"),
			QRegularExpression::CaseInsensitiveOption);
		if (reError.match(trimmed).hasMatch())
			return LogLineSeverity::Error;

		static const QRegularExpression reWarning(
			QStringLiteral("\\bscript warning\\b|^\\s*(?:warning|warn)\\s*[:!]|:\\s*warning\\b|\\bwarning\\s*:"
				"|\\b[1-9]\\d*\\s+warnings?\\b|\\bdeprecated\\b"
				"|\\bunknown (?:class|actor|texture|flat|sound|lump|property|flag|cvar|command)\\b"
				"|\\bmissing (?:texture|patch|sprite|lump|file|sound)\\b|\\bnot found\\b"),
			QRegularExpression::CaseInsensitiveOption);
		if (reWarning.match(trimmed).hasMatch())
			return LogLineSeverity::Warning;

		return LogLineSeverity::Normal;
	}

	class LogSyntaxHighlighter final : public QSyntaxHighlighter
	{
	public:
		explicit LogSyntaxHighlighter(QPlainTextEdit* editor)
			: QSyntaxHighlighter(editor->document())
		{
			updateFormats();
		}

		void updateFormats()
		{
			const LauncherThemeDefinition& theme = getActiveLauncherTheme();
			const LauncherThemePalette& pal = theme.palette;

			errorFmt = QTextCharFormat();
			errorFmt.setFontWeight(QFont::Bold);
			errorFmt.setForeground(pal.logErrorFg);
			if (pal.logErrorBg.isValid() && pal.logErrorBg.alpha() > 0)
				errorFmt.setBackground(pal.logErrorBg);

			warningFmt = QTextCharFormat();
			if (theme.isHighContrast)
				warningFmt.setFontWeight(QFont::Bold);
			warningFmt.setForeground(pal.logWarningFg);
			if (pal.logWarningBg.isValid() && pal.logWarningBg.alpha() > 0)
				warningFmt.setBackground(pal.logWarningBg);

			headerFmt = QTextCharFormat();
			headerFmt.setFontWeight(QFont::Bold);
			headerFmt.setForeground(pal.logHeaderFg);
		}

	protected:
		void highlightBlock(const QString& text) override
		{
			switch (classifyLogLine(text))
			{
			case LogLineSeverity::Error:
				setFormat(0, text.size(), errorFmt);
				break;
			case LogLineSeverity::Warning:
				setFormat(0, text.size(), warningFmt);
				break;
			case LogLineSeverity::Header:
				setFormat(0, text.size(), headerFmt);
				break;
			case LogLineSeverity::Normal:
			default:
				break;
			}
		}

	private:
		QTextCharFormat errorFmt;
		QTextCharFormat warningFmt;
		QTextCharFormat headerFmt;
	};
}

ProfileSettingsDialog::ProfileSettingsDialog(
	Profile& targetProfile,
	const std::vector<Profile>& profilesList,
	const std::string& launcherLanguage,
	QWidget* parent)
	: QDialog(parent)
	, profile(targetProfile)
	, allProfiles(profilesList)
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
	tabs->addTab(buildLogsTab(), LStr("LAUNCHER_TAB_LOGS", "Logs"));
	connect(tabs, &QTabWidget::currentChanged, this, [this](int index)
	{
		updateCommandPreview();
		if (index == tabs->count() - 1)
			refreshLogView();
	});
	mainLayout->addWidget(tabs, 1);

	QHBoxLayout* bottomBar = new QHBoxLayout();

	QPushButton* btnClone = new QPushButton(LStr("LAUNCHER_CLONE_PROFILE", "Clone Profile") + QStringLiteral("..."), this);
	connect(btnClone, &QPushButton::clicked, this, &ProfileSettingsDialog::onCloneProfile);
	bottomBar->addWidget(btnClone);

	QPushButton* btnExport = new QPushButton(LStr("LAUNCHER_EXPORT_PROFILE", "Export Profile") + QStringLiteral("..."), this);
	connect(btnExport, &QPushButton::clicked, this, &ProfileSettingsDialog::onExportProfile);
	bottomBar->addWidget(btnExport);

	QPushButton* btnDelete = new QPushButton(LStr("LAUNCHER_DELETE_PROFILE", "Delete Profile") + QStringLiteral("..."), this);
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
		const QString filter = QString("%1 (*.png *.jpg *.jpeg *.webp *.bmp);;%2 (*.*)")
			.arg(LStr("LAUNCHER_FILTER_IMAGES", "Images"), LStr("FILEPICKER_ANY", "All Files"));
		const QString p = browsePath(LStr("LAUNCHER_SELECT_COVER", "Select Cover Image"), filter, editCoverImage->text());
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

	comboQuickIwad = new QComboBox(tab);
	comboQuickIwad->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
	comboQuickIwad->setMinimumContentsLength(16);
	populateQuickIwadCombo();

	editIwadPath = new QLineEdit(tab);
	QPushButton* btnIwad = new QPushButton("...", tab);
	auto browseForIwad = [this]()
	{
		const QString filter = QString("%1 (*.wad *.iwad *.ipk3 *.ipk7);;%2 (*.*)")
			.arg(LStr("LAUNCHER_FILTER_IWAD", "Doom WAD/PK3"), LStr("FILEPICKER_ANY", "All Files"));
		const QString p = browsePath(LStr("LAUNCHER_SELECT_IWAD_TITLE", "Select IWAD"), filter, editIwadPath->text());
		if (!p.isEmpty())
			editIwadPath->setText(p);
	};
	connect(btnIwad, &QPushButton::clicked, this, browseForIwad);
	connect(comboQuickIwad, QOverload<int>::of(&QComboBox::activated), this, [this, browseForIwad](int index)
	{
		const QString data = comboQuickIwad->itemData(index).toString();
		if (data == QStringLiteral("__browse__"))
		{
			browseForIwad();
			syncQuickIwadFromPath(editIwadPath->text());
			return;
		}
		if (!data.isEmpty() && data != QStringLiteral("__custom__"))
		{
			editIwadPath->setText(data);
		}
	});
	connect(editIwadPath, &QLineEdit::textChanged, this, [this](const QString& text)
	{
		syncQuickIwadFromPath(text);
		updateCommandPreview();
	});

	editPwadPath = new QLineEdit(tab);
	QPushButton* btnPwad = new QPushButton("...", tab);
	connect(btnPwad, &QPushButton::clicked, this, [this]()
	{
		const QString filter = QString("%1 (*.wad *.pwad *.pk3 *.pk7 *.deh *.bex);;%2 (*.*)")
			.arg(LStr("LAUNCHER_FILTER_DOOM_FILES", "Doom Files"), LStr("FILEPICKER_ANY", "All Files"));
		const QString p = browsePath(LStr("LAUNCHER_SELECT_PWAD_TITLE", "Select (P)WAD"), filter, editPwadPath->text());
		if (!p.isEmpty())
			editPwadPath->setText(p);
	});
	pwadRowWidget = createPathBrowseRow(editPwadPath, btnPwad, tab);

	rightForm->addRow(LStr("LAUNCHER_TYPE", "Type:"), comboType);
	rightForm->addRow(LStr("LAUNCHER_SELECT_IWAD", "Select IWAD:"), comboQuickIwad);
	rightForm->addRow(LStr("LAUNCHER_ASSOC_IWAD", "IWAD Path:"), createPathBrowseRow(editIwadPath, btnIwad, tab));
	rightForm->addRow(LStr("LAUNCHER_ASSOC_PWAD", "Associated (P)WAD:"), pwadRowWidget);

	topSplit->addLayout(leftForm, 1);
	topSplit->addSpacing(16);
	topSplit->addLayout(rightForm, 1);
	vbox->addLayout(topSplit);

	vbox->addWidget(new QLabel(LStr("LAUNCHER_DESCRIPTION", "Description:"), tab));
	editDescription = new QPlainTextEdit(tab);
	editDescription->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
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

	radioNormal = new QRadioButton(LStr("OPTVAL_NORMAL", "Normal"), grpMode);
	radioMap = new QRadioButton(LStr("OPTVAL_MAP", "Map"), grpMode);
	radioSavegame = new QRadioButton(LStr("LAUNCHER_MODE_SAVE", "Savegame"), grpMode);
	radioDemoPlay = new QRadioButton(LStr("LAUNCHER_MODE_DEMO", "Demo Playback"), grpMode);

	for (QRadioButton* r : {radioNormal, radioMap, radioSavegame, radioDemoPlay})
	{
		modeGroup->addButton(r);
		connect(r, &QRadioButton::toggled, this, &ProfileSettingsDialog::onLaunchModeChanged);
	}

	editMap = new QLineEdit(grpMode);
	editMap->setPlaceholderText(LStr("LAUNCHER_MAP_PLACEHOLDER", "MAP01, E1M1 etc."));

	editSavegame = new QLineEdit(grpMode);
	QPushButton* btnSave = new QPushButton("...", grpMode);
	connect(btnSave, &QPushButton::clicked, this, [this]()
	{
		const QString profDir = profile.filePath.empty() ? QString() : QFileInfo(qStr(profile.filePath)).absolutePath();
		const QString resolvedSaveDir = expandProfilePathTokens(editSaveDir->text(), profDir);
		const QString curSave = editSavegame->text().trimmed();
		const QString startDir = curSave.isEmpty()
			? resolvedSaveDir
			: expandProfilePathTokens(curSave, resolvedSaveDir.isEmpty() ? profDir : resolvedSaveDir);
		const QString filter = QString("%1 (*.zds);;%2 (*.*)")
			.arg(LStr("LAUNCHER_FILTER_SAVES", "Savegames"), LStr("FILEPICKER_ANY", "All Files"));
		const QString p = browsePath(LStr("LAUNCHER_SELECT_SAVE_TITLE", "Select Savegame"), filter, startDir);
		if (!p.isEmpty())
		{
			QString displayed = p;
			if (!resolvedSaveDir.isEmpty())
			{
				const QString cleanDir = QDir::cleanPath(QFileInfo(resolvedSaveDir).absoluteFilePath()).replace('\\', '/');
				const QString cleanFile = QDir::cleanPath(QFileInfo(p).absoluteFilePath()).replace('\\', '/');
				const QString prefix = cleanDir.endsWith('/') ? cleanDir : (cleanDir + '/');
#ifdef _WIN32
				constexpr Qt::CaseSensitivity cs = Qt::CaseInsensitive;
#else
				constexpr Qt::CaseSensitivity cs = Qt::CaseSensitive;
#endif
				if (cleanFile.startsWith(prefix, cs))
					displayed = QDir(cleanDir).relativeFilePath(cleanFile).replace('\\', '/');
			}
			editSavegame->setText(displayed);
		}
	});

	editDemoPlay = new QLineEdit(grpMode);
	QPushButton* btnDemo = new QPushButton("...", grpMode);
	connect(btnDemo, &QPushButton::clicked, this, [this]()
	{
		const QString profDir = profile.filePath.empty() ? QString() : QFileInfo(qStr(profile.filePath)).absolutePath();
		const QString resolvedDemoDir = expandProfilePathTokens(editDemoDir->text(), profDir);
		const QString curDemo = editDemoPlay->text().trimmed();
		const QString startDir = curDemo.isEmpty()
			? resolvedDemoDir
			: expandProfilePathTokens(curDemo, resolvedDemoDir.isEmpty() ? profDir : resolvedDemoDir);
		const QString filter = QString("%1 (*.lmp);;%2 (*.*)")
			.arg(LStr("LAUNCHER_FILTER_DEMOS", "Demos"), LStr("FILEPICKER_ANY", "All Files"));
		const QString p = browsePath(LStr("LAUNCHER_SELECT_DEMO_TITLE", "Select Demo"), filter, startDir);
		if (!p.isEmpty())
			editDemoPlay->setText(p);
	});

	chkRecordDemo = new QCheckBox(LStr("LAUNCHER_RECORD_DEMO", "Record Demo:"), grpMode);
	editRecordDemo = new QLineEdit(grpMode);
	editRecordDemo->setPlaceholderText(LStr("LAUNCHER_RECORD_DEMO_PLACEHOLDER", "exampleDemoName"));
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
	chkPistolStart = new QCheckBox(LStr("GMPLYMNU_PISTOLSTART", "Pistol start every map"), grpDiff);
	chkPistolStart->setToolTip(LStr("LAUNCHER_PISTOL_START_TIP", "Reset player inventory at every level transition (DF3_PISTOL_START / sv_pistolstart). Separate from 'Restart level on death'."));

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

	QGroupBox* grpPlayer = new QGroupBox(LStr("OPTVAL_PLAYER", "Player"), tab);
	grpPlayer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	QFormLayout* playerForm = new QFormLayout(grpPlayer);
	playerForm->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
	editPlayerName = new QLineEdit(grpPlayer);
	editPlayerClass = new QLineEdit(grpPlayer);
	comboPlayerGender = new QComboBox(grpPlayer);
	comboPlayerGender->addItems({LStr("OPTVAL_DEFAULT", "Default"), LStr("OPTVAL_MALE", "Male"), LStr("OPTVAL_FEMALE", "Female"), LStr("OPTVAL_NEUTRAL", "Neutral")});

	comboWadLang = new QComboBox(grpPlayer);
	comboWadLang->addItem(LStr("LAUNCHER_LANG_DEFAULT", "Default (Launcher Language)"), "default");
	for (const auto& lang : getLauncherLanguages())
		comboWadLang->addItem(lang.name, lang.code);

	playerForm->addRow(ensureTrailingColon(LStr("PLYRMNU_NAME", "Name")), editPlayerName);
	playerForm->addRow(ensureTrailingColon(LStr("PICKER_PLAYERCLASS", "Player Class")), editPlayerClass);
	playerForm->addRow(ensureTrailingColon(LStr("PLYRMNU_PLAYERGENDER", "Gender")), comboPlayerGender);
	playerForm->addRow(ensureTrailingColon(LStr("OPTMNU_LANGUAGE", "Language")), comboWadLang);
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
	spinRemotePort->setSpecialValueText(LStr("PICKER_PORTHINT", "Default 5029"));
	spinRemotePort->setToolTip(LStr("PICKER_PORTHINT", "Default 5029"));
	spinRemoteTeam = new QSpinBox(grpRemote);
	spinRemoteTeam->setRange(0, 255);
	spinRemoteTeam->setToolTip(LStr("PICKER_TEAMHINT", "255 = Random"));

	remoteForm->addRow(ensureTrailingColon(LStr("PICKER_IP", "Host IP:")), editRemoteAddr);
	remoteForm->addRow(ensureTrailingColon(LStr("PICKER_PORT", "Host Port:")), spinRemotePort);
	remoteForm->addRow(ensureTrailingColon(LStr("PICKER_TEAM", "Team #:")), spinRemoteTeam);
	rightCol->addWidget(grpRemote);

	QGroupBox* grpHost = new QGroupBox(LStr("LAUNCHER_HOST_MP", "Host Multiplayer"), tab);
	grpHost->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	QFormLayout* hostForm = new QFormLayout(grpHost);
	hostForm->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
	spinHostPort = new QSpinBox(grpHost);
	spinHostPort->setRange(0, 65535);
	spinHostPort->setSpecialValueText(LStr("PICKER_PORTHINT", "Default 5029"));
	spinHostPort->setToolTip(LStr("PICKER_PORTHINT", "Default 5029"));
	spinMaxPlayers = new QSpinBox(grpHost);
	spinMaxPlayers->setRange(1, 64);
	spinMaxPlayers->setToolTip(LStr("PICKER_PLAYERHINT", "Max 64"));

	comboTicRate = new QComboBox(grpHost);
	comboTicRate->addItems({"35 Hz", "17.5 Hz", "11.6 Hz"});
	chkExtraTic = new QCheckBox(LStr("PICKER_NETBACKUP", "ExtraTic"), grpHost);

	comboGamemode = new QComboBox(grpHost);
	comboGamemode->addItem(LStr("OPTVAL_NONE", "None"));
	comboGamemode->addItem(LStr("PICKER_COOP", "Co-op"));
	comboGamemode->addItem(LStr("PICKER_DM", "Deathmatch"));
	comboGamemode->addItem(LStr("PICKER_TDM", "Team Deathmatch"));

	chkHostAltDM = new QCheckBox(LStr("PICKER_ALTDM", "Alt Deathmatch"), grpHost);

	spinHostTeam = new QSpinBox(grpHost);
	spinHostTeam->setRange(0, 255);
	spinHostTeam->setToolTip(LStr("PICKER_TEAMHINT", "255 = Random"));
	connect(comboGamemode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx)
	{
		const bool isDM = (idx == 2 || idx == 3);
		chkHostAltDM->setEnabled(isDM);
		if (!isDM)
			chkHostAltDM->setChecked(false);
		spinHostTeam->setEnabled(idx == 3);
	});

	int dropdownHeight = 0;
	for (const QComboBox* combo : {comboTicRate, comboGamemode})
		dropdownHeight = std::max(dropdownHeight, combo->sizeHint().height());
	for (QSpinBox* spin : {spinRemotePort, spinRemoteTeam, spinHostPort, spinMaxPlayers, spinHostTeam})
		spin->setMinimumHeight(dropdownHeight);

	hostForm->addRow(ensureTrailingColon(LStr("PICKER_PORT", "Host Port:")), spinHostPort);
	hostForm->addRow(ensureTrailingColon(LStr("PICKER_PLAYERS", "Max Players:")), spinMaxPlayers);
	hostForm->addRow(ensureTrailingColon(LStr("PICKER_NETRATE", "Packet Ticrate:")), comboTicRate);
	hostForm->addRow(QString(), chkExtraTic);
	hostForm->addRow(ensureTrailingColon(LStr("PICKER_GAMEMODE", "Game Mode:")), comboGamemode);
	hostForm->addRow(QString(), chkHostAltDM);
	hostForm->addRow(ensureTrailingColon(LStr("PICKER_TEAM", "Team #:")), spinHostTeam);
	rightCol->addWidget(grpHost);

	QGroupBox* grpCompat = new QGroupBox(LStr("LAUNCHER_COMPAT_AND_FLAGS", "Compatibility and Flags"), tab);
	grpCompat->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	QVBoxLayout* compatLayout = new QVBoxLayout(grpCompat);
	QFormLayout* compatForm = new QFormLayout();
	compatForm->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

	comboEngineComplevel = new QComboBox(grpCompat);
	for (const EngineComplevelInfo& cl : getEngineComplevels())
		comboEngineComplevel->addItem(cl.label, cl.value);
	comboEngineComplevel->setToolTip(LStr("LAUNCHER_COMPLEVEL_TIP", "Sets UZDoom's -complevel parameter (applies Doom-only MAPINFO compatibility defaults per map; does not modify compatflags CVARs)."));

	QWidget* compatModeRow = new QWidget(grpCompat);
	QHBoxLayout* compRow = new QHBoxLayout(compatModeRow);
	compRow->setContentsMargins(0, 0, 0, 0);
	comboCompatLevel = new QComboBox(compatModeRow);
	for (const CompatModeInfo& cm : getCompatModes())
		comboCompatLevel->addItem(cm.label, cm.id);
	comboCompatLevel->setToolTip(LStr("LAUNCHER_COMPATMODE_TIP", "Selects a compatibility mode preset (-compatmode), updating compatflags and compatflags2."));
	connect(comboCompatLevel, QOverload<int>::of(&QComboBox::activated), this, [this](int index)
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

	compatForm->addRow(LStr("LAUNCHER_COMPLEVEL", "Complevel:"), comboEngineComplevel);
	compatForm->addRow(ensureTrailingColon(LStr("CMPTMNU_MODE", "Compatibility mode")), compatModeRow);
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
		const QString filter = QString("%1 (*.ini);;%2 (*.*)")
			.arg(LStr("LAUNCHER_FILTER_CONFIG", "Config"), LStr("FILEPICKER_ANY", "All Files"));
		const QString p = browsePath(LStr("LAUNCHER_SELECT_CONFIG_TITLE", "Select Config File"), filter, editConfigFile->text());
		if (!p.isEmpty()) editConfigFile->setText(p);
	});
	QPushButton* btnOpenCfg = new QPushButton(LStr("LAUNCHER_OPEN_INI", "Open INI"), tab);
	btnOpenCfg->setToolTip(LStr("LAUNCHER_OPEN_INI_TIP", "Open the configured INI file in the system default editor (or locate its folder)."));
	connect(btnOpenCfg, &QPushButton::clicked, this, [this]()
	{
		const QString iniPath = editConfigFile->text().trimmed();
		if (iniPath.isEmpty())
		{
			QMessageBox::information(this, LStr("LAUNCHER_OPEN_INI", "Open INI"),
				LStr("LAUNCHER_NO_INI_PATH_MSG", "No INI config file path is configured for this profile yet."));
			return;
		}

		const QString profDir = profile.filePath.empty() ? QString() : QFileInfo(qStr(profile.filePath)).absolutePath();
		const QFileInfo fi(expandProfilePathTokens(iniPath, profDir));
		if (fi.exists() && fi.isFile())
		{
			QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absoluteFilePath()));
			return;
		}

		QMessageBox box(this);
		box.setIcon(QMessageBox::Information);
		box.setWindowTitle(LStr("LAUNCHER_OPEN_INI", "Open INI"));
		box.setText(LStr("LAUNCHER_INI_MISSING_MSG", "INI file does not exist yet:\n%1\n\nUZDoom creates this file automatically on first launch.")
			.arg(QDir::toNativeSeparators(fi.absoluteFilePath())));
		QPushButton* btnCreate = box.addButton(LStr("LAUNCHER_CREATE_AND_OPEN", "Create and Open"), QMessageBox::AcceptRole);
		QPushButton* btnFolder = box.addButton(LStr("LAUNCHER_OPEN_FOLDER", "Open Folder"), QMessageBox::ActionRole);
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
				QMessageBox::warning(this, LStr("LAUNCHER_ERROR", "Error"),
					LStr("LAUNCHER_CREATE_INI_ERR", "Could not create INI file at:\n%1").arg(fi.absoluteFilePath()));
			}
		}
		else if (box.clickedButton() == btnFolder)
		{
			QDir().mkpath(fi.absolutePath());
			QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absolutePath()));
		}
	});

	auto makeOpenDirButton = [&](QLineEdit* dirEdit, const QString& labelName) -> QPushButton*
	{
		connect(dirEdit, &QLineEdit::textChanged, dirEdit, [dirEdit](const QString& text)
		{
			dirEdit->setToolTip(text.trimmed());
		});
		QPushButton* btnOpen = new QPushButton(LStr("LAUNCHER_OPEN", "Open"), tab);
		btnOpen->setToolTip(LStr("LAUNCHER_OPEN_DIR_TIP", "Open the configured %1 in the system file manager (creates it if missing).").arg(labelName));
		connect(btnOpen, &QPushButton::clicked, this, [this, dirEdit, labelName]()
		{
			const QString dirPath = dirEdit->text().trimmed();
			if (dirPath.isEmpty())
			{
				QMessageBox::information(this, LStr("LAUNCHER_OPEN_FOLDER", "Open Folder"),
					LStr("LAUNCHER_NO_DIR_PATH_MSG", "No %1 path is configured for this profile yet.").arg(labelName));
				return;
			}
			const QString profDir = profile.filePath.empty() ? QString() : QFileInfo(qStr(profile.filePath)).absolutePath();
			const QString absDir = QFileInfo(expandProfilePathTokens(dirPath, profDir)).absoluteFilePath();
			if (!QFileInfo::exists(absDir) && !QDir().mkpath(absDir))
			{
				QMessageBox::warning(this, LStr("LAUNCHER_ERROR", "Error"),
					LStr("LAUNCHER_CREATE_DIR_ERR", "Could not create folder:\n%1").arg(QDir::toNativeSeparators(absDir)));
				return;
			}
			QDesktopServices::openUrl(QUrl::fromLocalFile(absDir));
		});
		return btnOpen;
	};

	editSaveDir = new QLineEdit(tab);
	QPushButton* btnSaveDir = new QPushButton("...", tab);
	connect(btnSaveDir, &QPushButton::clicked, this, [this]()
	{
		const QString p = browsePath(LStr("LAUNCHER_SELECT_SAVE_DIR", "Select Save Directory"), {}, editSaveDir->text(), true);
		if (!p.isEmpty()) editSaveDir->setText(p);
	});
	QPushButton* btnOpenSaveDir = makeOpenDirButton(editSaveDir, LStr("LAUNCHER_SAVE_DIR_NAME", "save directory"));

	editShotDir = new QLineEdit(tab);
	QPushButton* btnShotDir = new QPushButton("...", tab);
	connect(btnShotDir, &QPushButton::clicked, this, [this]()
	{
		const QString p = browsePath(LStr("LAUNCHER_SELECT_SHOT_DIR", "Select Screenshot Directory"), {}, editShotDir->text(), true);
		if (!p.isEmpty()) editShotDir->setText(p);
	});
	QPushButton* btnOpenShotDir = makeOpenDirButton(editShotDir, LStr("LAUNCHER_SHOT_DIR_NAME", "screenshot directory"));

	editDemoDir = new QLineEdit(tab);
	QPushButton* btnDemoDir = new QPushButton("...", tab);
	connect(btnDemoDir, &QPushButton::clicked, this, [this]()
	{
		const QString p = browsePath(LStr("LAUNCHER_SELECT_DEMO_DIR", "Select Demo Directory"), {}, editDemoDir->text(), true);
		if (!p.isEmpty()) editDemoDir->setText(p);
	});
	QPushButton* btnOpenDemoDir = makeOpenDirButton(editDemoDir, LStr("LAUNCHER_DEMO_DIR_NAME", "demo directory"));

	int openBtnWidth = btnOpenCfg->sizeHint().width();
	for (QPushButton* b : {btnOpenSaveDir, btnOpenShotDir, btnOpenDemoDir})
		openBtnWidth = std::max(openBtnWidth, b->sizeHint().width());
	for (QPushButton* b : {btnOpenCfg, btnOpenSaveDir, btnOpenShotDir, btnOpenDemoDir})
		b->setFixedWidth(openBtnWidth);

	pathsForm->addRow(LStr("LAUNCHER_CONFIG_FILE", "Config File:"), createPathBrowseRow(editConfigFile, btnCfg, tab, btnOpenCfg));
	pathsForm->addRow(LStr("LAUNCHER_SAVE_DIR", "Save Directory:"), createPathBrowseRow(editSaveDir, btnSaveDir, tab, btnOpenSaveDir));
	pathsForm->addRow(LStr("LAUNCHER_SHOT_DIR", "Screenshot Directory:"), createPathBrowseRow(editShotDir, btnShotDir, tab, btnOpenShotDir));
	pathsForm->addRow(LStr("LAUNCHER_DEMO_DIR", "Demo Directory:"), createPathBrowseRow(editDemoDir, btnDemoDir, tab, btnOpenDemoDir));
	vbox->addLayout(pathsForm);

	QHBoxLayout* modsHeader = new QHBoxLayout();
	QLabel* lblMods = new QLabel(LStr("LAUNCHER_MODS", "Mods"), tab);
	QFont f = lblMods->font();
	f.setBold(true);
	lblMods->setFont(f);
	lblMods->setContentsMargins(0, 0, 0, std::max(2, lblMods->fontMetrics().descent()));
	modsHeader->addWidget(lblMods, 0, Qt::AlignVCenter);

	chkCopyModsToProfile = new QCheckBox(LStr("LAUNCHER_COPY_MODS", "Copy added mods to profile"), tab);
	chkCopyModsToProfile->setToolTip(LStr("LAUNCHER_COPY_MODS_TIP",
		"When enabled, added mods are copied into this profile's mods/ folder.\nWhen disabled, mods are referenced from their original location without copying."));
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
	chkVsync = new QCheckBox(LStr("DSPLYMNU_VSYNC", "Vertical Sync"), grpGen);
	chkSupportWads = new QCheckBox(LStr("PICKER_SUPPORTWADS", "Load Support WADs"), grpGen);
	chkSupportWads->setToolTip(LStr("LAUNCHER_SUPPORTWADS_TIP", "Loads optional ID24 support resources (id24res.wad) when available, adding Legacy of Rust / ID24 weapons and assets. Automatically disabled in multiplayer."));
	chkDisableAutoload = new QCheckBox(LStr("PICKER_NOAUTOLOAD", "Disable Autoload"), grpGen);
	chkDisableAutoload->setToolTip(LStr("LAUNCHER_NOAUTOLOAD_TIP", "Skips Global.Autoload, per-IWAD autoload sections, zvox.wad, and skins/ (-noautoload). Extra Graphics on the right are controlled separately."));

	for (QCheckBox* c : {chkFullscreen, chkVsync, chkSupportWads, chkDisableAutoload})
		genBox->addWidget(c);

	QFormLayout* strictForm = new QFormLayout();
	strictForm->setContentsMargins(0, 6, 0, 0);
	comboFileLoadStrictness = new QComboBox(grpGen);
	comboFileLoadStrictness->addItem(LStr("LAUNCHER_INHERIT_INI", "From INI"), -1);
	comboFileLoadStrictness->addItem(LStr("OPTVAL_DEFAULT", "Default"), REQUIRE_DEFAULT);
	comboFileLoadStrictness->addItem(LStr("OPTVAL_LAX", "Lax"), REQUIRE_NONE);
	comboFileLoadStrictness->addItem(LStr("OPTVAL_STRICT", "Strict"), REQUIRE_ALL);
	strictForm->addRow(ensureTrailingColon(LStr("PICKER_FILELOADING", "File Loading")), comboFileLoadStrictness);
	genBox->addLayout(strictForm);
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
	chkDynLights = new QCheckBox(LStr("GLLIGHTMNU_LIGHTSENABLED", "Dynamic Lights"), grpExtra);
	chkShadowmaps = new QCheckBox(LStr("GLLIGHTMNU_LIGHTSHADOWMAP", "Enable Shadowmaps"), grpExtra);
	for (QCheckBox* c : {chkLights, chkBrightmaps, chkWidescreen, chkDynLights, chkShadowmaps})
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
	appendHeader->addWidget(new QLabel(ensureTrailingColon(LStr("PICKER_ADDPARM", "Additional Parameters:")), tab));
	appendHeader->addStretch();
	QPushButton* btnBrowseAppend = new QPushButton(LStr("LAUNCHER_SELECT_FILE", "Select File..."), tab);
	btnBrowseAppend->setToolTip(LStr("LAUNCHER_SELECT_FILE_TIP", "Pick file(s) and append them to Additional Parameters (auto-adds -file / -deh / +exec / -playdemo when no switch precedes them)."));
	connect(btnBrowseAppend, &QPushButton::clicked, this, [this]()
	{
		const QString startDir = profile.filePath.empty()
			? qStr(LauncherConst::EXEC_DIR)
			: QFileInfo(qStr(profile.filePath)).absolutePath();
		const QString filter = QString("%1 (*.wad *.iwad *.pwad *.pk3 *.pk7 *.ipk3 *.ipk7 *.deh *.bex *.cfg *.lmp);;%2 (*.*)")
			.arg(LStr("LAUNCHER_FILTER_DOOM_FILES", "Doom Files"), LStr("FILEPICKER_ANY", "All Files"));
		const QStringList picked = QFileDialog::getOpenFileNames(
			this,
			LStr("LAUNCHER_SELECT_APPEND_FILES", "Select File(s) for Additional Parameters"),
			startDir,
			filter);
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
	editAppendArgs->setPlaceholderText(LStr("LAUNCHER_APPEND_ARGS_PLACEHOLDER", "Parameters appended after profile options..."));
	connect(editAppendArgs, &QPlainTextEdit::textChanged, this, &ProfileSettingsDialog::updateCommandPreview);
	vbox->addWidget(editAppendArgs);

	vbox->addSpacing(8);

	QGroupBox* grpCmd = new QGroupBox(LStr("LAUNCHER_COMMAND_PREVIEW", "Launch Command Preview"), tab);
	QVBoxLayout* cmdLayout = new QVBoxLayout(grpCmd);

	QHBoxLayout* cmdTopRow = new QHBoxLayout();
	cmdTopRow->addWidget(new QLabel(LStr("LAUNCHER_PREVIEW_MODE", "Mode:"), grpCmd));
	comboPreviewMode = new QComboBox(grpCmd);
	comboPreviewMode->addItem(LStr("PICKER_PLAY", "Play Game"), static_cast<int>(LaunchTargetMode::Single));
	comboPreviewMode->addItem(LStr("PICKER_PLAYHOST", "Host Game"), static_cast<int>(LaunchTargetMode::Host));
	comboPreviewMode->addItem(LStr("PICKER_PLAYJOIN", "Join Game"), static_cast<int>(LaunchTargetMode::Join));
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

QWidget* ProfileSettingsDialog::buildLogsTab()
{
	QWidget* tab = new QWidget(this);
	QVBoxLayout* vbox = new QVBoxLayout(tab);

	QHBoxLayout* topRow = new QHBoxLayout();
	topRow->addWidget(new QLabel(LStr("LAUNCHER_LOG_SESSION", "Session Log:"), tab));
	comboLogFiles = new QComboBox(tab);
	comboLogFiles->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
	comboLogFiles->setMinimumContentsLength(24);
	connect(comboLogFiles, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int)
	{
		loadSelectedLogFile();
	});
	topRow->addWidget(comboLogFiles, 1);

	chkLogProblemsOnly = new QCheckBox(LStr("LAUNCHER_LOG_PROBLEMS_ONLY", "Errors / Warnings"), tab);
	chkLogProblemsOnly->setToolTip(LStr("LAUNCHER_LOG_PROBLEMS_TIP", "Show only warning and error lines in the selected launch log."));
	connect(chkLogProblemsOnly, &QCheckBox::toggled, this, &ProfileSettingsDialog::applyLogFilter);
	topRow->addWidget(chkLogProblemsOnly);

	editLogFilter = new QLineEdit(tab);
	editLogFilter->setClearButtonEnabled(true);
	editLogFilter->setPlaceholderText(LStr("LAUNCHER_LOG_FILTER", "Filter log lines..."));
	editLogFilter->setFixedWidth(210);
	connect(editLogFilter, &QLineEdit::textChanged, this, &ProfileSettingsDialog::applyLogFilter);
	topRow->addWidget(editLogFilter);
	vbox->addLayout(topRow);

	editLogView = new QPlainTextEdit(tab);
	editLogView->setReadOnly(true);
	editLogView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	editLogView->setPlaceholderText(LStr("LAUNCHER_LOG_EMPTY", "No launch logs recorded for this profile yet.\nLaunch this profile to capture UZDoom's console output here."));
	new LogSyntaxHighlighter(editLogView);
	vbox->addWidget(editLogView, 1);

	QHBoxLayout* btnRow = new QHBoxLayout();
	QPushButton* btnRefreshLog = new QPushButton(LStr("LAUNCHER_REFRESH", "Refresh"), tab);
	connect(btnRefreshLog, &QPushButton::clicked, this, &ProfileSettingsDialog::refreshLogView);
	btnRow->addWidget(btnRefreshLog);

	btnCopyLog = new QPushButton(LStr("ACTION_COPYTOCLIPBOARD", "Copy to Clipboard"), tab);
	connect(btnCopyLog, &QPushButton::clicked, this, [this]()
	{
		if (QClipboard* cb = QApplication::clipboard())
			cb->setText(editLogView->toPlainText());
		btnCopyLog->setText(LStr("LAUNCHER_COPIED", "Copied!"));
		QTimer::singleShot(1400, btnCopyLog, [this]()
		{
			if (btnCopyLog)
				btnCopyLog->setText(LStr("ACTION_COPYTOCLIPBOARD", "Copy to Clipboard"));
		});
	});
	btnRow->addWidget(btnCopyLog);

	btnOpenLogFile = new QPushButton(LStr("LAUNCHER_OPEN_LOG_FILE", "Open Log File"), tab);
	connect(btnOpenLogFile, &QPushButton::clicked, this, [this]()
	{
		const QString path = selectedLogFilePath();
		if (!path.isEmpty() && QFileInfo::exists(path))
			QDesktopServices::openUrl(QUrl::fromLocalFile(path));
	});
	btnRow->addWidget(btnOpenLogFile);

	btnOpenLogsDir = new QPushButton(LStr("LAUNCHER_OPEN_LOGS_DIR", "Open Logs Folder"), tab);
	connect(btnOpenLogsDir, &QPushButton::clicked, this, [this]()
	{
		const QString dir = profileLogsDirPath();
		if (!dir.isEmpty())
		{
			QDir().mkpath(dir);
			QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
		}
	});
	btnRow->addWidget(btnOpenLogsDir);

	btnRow->addStretch();

	btnDeleteLog = new QPushButton(LStr("LAUNCHER_DELETE_LOG", "Delete Log"), tab);
	connect(btnDeleteLog, &QPushButton::clicked, this, [this]()
	{
		const QString path = selectedLogFilePath();
		if (!path.isEmpty() && QFileInfo::exists(path))
			QFile::remove(path);
		refreshLogView();
	});
	btnRow->addWidget(btnDeleteLog);

	btnClearAllLogs = new QPushButton(LStr("LAUNCHER_CLEAR_ALL", "Clear All"), tab);
	connect(btnClearAllLogs, &QPushButton::clicked, this, [this]()
	{
		if (!comboLogFiles || comboLogFiles->count() == 0)
			return;
		const auto ans = QMessageBox::question(
			this,
			LStr("LAUNCHER_CLEAR_ALL_LOGS", "Clear All Logs"),
			LStr("LAUNCHER_CLEAR_ALL_LOGS_MSG", "Delete all recorded launch logs for this profile?"),
			QMessageBox::Yes | QMessageBox::No,
			QMessageBox::No);
		if (ans != QMessageBox::Yes)
			return;
		for (int i = 0; i < comboLogFiles->count(); ++i)
		{
			const QString p = comboLogFiles->itemData(i).toString();
			if (!p.isEmpty() && QFileInfo::exists(p))
				QFile::remove(p);
		}
		refreshLogView();
	});
	btnRow->addWidget(btnClearAllLogs);

	vbox->addLayout(btnRow);
	return tab;
}

void ProfileSettingsDialog::populateQuickIwadCombo()
{
	if (!comboQuickIwad)
		return;

	const QSignalBlocker blocker(comboQuickIwad);
	comboQuickIwad->clear();

	struct IwadEntry
	{
		QString label;
		QString path;
	};
	std::vector<IwadEntry> entries;
	QSet<QString> seenPaths;

	auto addCandidate = [&](const QString& title, const QString& path)
	{
		const QString trimmed = path.trimmed();
		if (trimmed.isEmpty())
			return;
		const QString key = normModKey(trimmed);
		if (seenPaths.contains(key))
			return;
		seenPaths.insert(key);

		const QString fileName = QFileInfo(trimmed).fileName();
		const QString label = title.isEmpty()
			? fileName
			: (fileName.isEmpty() ? title : QString("%1  [%2]").arg(title, fileName));
		entries.push_back({label, trimmed});
	};

	for (const Profile& p : allProfiles)
	{
		if (p.isIWAD && !p.iwadPath.empty())
			addCandidate(qStr(p.title), qStr(p.iwadPath));
	}
	for (const DiscoveredIWad& w : g_discoveredEngineIWads)
	{
		if (!w.path.empty())
			addCandidate(qStr(w.name), qStr(w.path));
	}

	std::sort(entries.begin(), entries.end(), [](const IwadEntry& a, const IwadEntry& b)
	{
		return a.label.compare(b.label, Qt::CaseInsensitive) < 0;
	});

	for (const IwadEntry& e : entries)
	{
		comboQuickIwad->addItem(e.label, e.path);
		comboQuickIwad->setItemData(comboQuickIwad->count() - 1, e.path, Qt::ToolTipRole);
	}

	comboQuickIwad->addItem(LStr("OPTVAL_CUSTOM", "Custom"), QStringLiteral("__custom__"));
	comboQuickIwad->addItem(LStr("LAUNCHER_BROWSE_IWAD", "Browse..."), QStringLiteral("__browse__"));
}

void ProfileSettingsDialog::syncQuickIwadFromPath(const QString& path)
{
	if (!comboQuickIwad)
		return;

	const QSignalBlocker blocker(comboQuickIwad);
	const QString trimmed = path.trimmed();
	const QString targetKey = trimmed.isEmpty() ? QString() : normModKey(trimmed);
	const int customIdx = comboQuickIwad->findData(QStringLiteral("__custom__"));

	if (!targetKey.isEmpty())
	{
		for (int i = 0; i < comboQuickIwad->count(); ++i)
		{
			const QString itemPath = comboQuickIwad->itemData(i).toString();
			if (itemPath == QStringLiteral("__custom__") || itemPath == QStringLiteral("__browse__"))
				continue;
			if (normModKey(itemPath) == targetKey)
			{
				comboQuickIwad->setCurrentIndex(i);
				if (customIdx >= 0)
					comboQuickIwad->setItemText(customIdx, LStr("OPTVAL_CUSTOM", "Custom"));
				return;
			}
		}
	}

	if (customIdx >= 0)
	{
		if (trimmed.isEmpty())
		{
			comboQuickIwad->setItemText(customIdx, QString("(%1)").arg(LStr("OPTVAL_NONE", "None")));
		}
		else
		{
			const QString fn = QFileInfo(trimmed).fileName();
			comboQuickIwad->setItemText(customIdx, fn.isEmpty()
				? LStr("OPTVAL_CUSTOM", "Custom")
				: QString("%1 (%2)").arg(LStr("OPTVAL_CUSTOM", "Custom"), fn));
		}
		comboQuickIwad->setCurrentIndex(customIdx);
	}
}

QString ProfileSettingsDialog::profileLogsDirPath() const
{
	if (profile.filePath.empty())
		return {};
	return QDir(QFileInfo(qStr(profile.filePath)).absolutePath()).filePath(QStringLiteral("logs"));
}

QString ProfileSettingsDialog::selectedLogFilePath() const
{
	if (!comboLogFiles || comboLogFiles->currentIndex() < 0)
		return {};
	return comboLogFiles->currentData().toString();
}

void ProfileSettingsDialog::refreshLogView()
{
	if (!comboLogFiles || !editLogView)
		return;

	const QString prevSelected = selectedLogFilePath();
	QFileInfoList logFiles;

	const QString logsDirPath = profileLogsDirPath();
	if (!logsDirPath.isEmpty() && QFileInfo::exists(logsDirPath))
	{
		QDir logsDir(logsDirPath);
		logFiles = logsDir.entryInfoList({QStringLiteral("*.log")}, QDir::Files);
	}

	if (!profile.filePath.empty())
	{
		const QFileInfo legacyLog(QDir(QFileInfo(qStr(profile.filePath)).absolutePath()).filePath(QStringLiteral("last_launch.log")));
		if (legacyLog.exists() && legacyLog.isFile())
			logFiles.append(legacyLog);
	}

	std::sort(logFiles.begin(), logFiles.end(), [](const QFileInfo& a, const QFileInfo& b)
	{
		const QDateTime ma = a.lastModified();
		const QDateTime mb = b.lastModified();
		if (ma.isValid() && mb.isValid() && ma != mb)
			return ma > mb;
		return a.fileName().compare(b.fileName(), Qt::CaseInsensitive) > 0;
	});

	const bool hasDirTarget = !logsDirPath.isEmpty();
	if (btnOpenLogsDir)
		btnOpenLogsDir->setEnabled(hasDirTarget);

	{
		const QSignalBlocker blocker(comboLogFiles);
		comboLogFiles->clear();
		if (logFiles.isEmpty())
		{
			comboLogFiles->addItem(LStr("LAUNCHER_LOG_NONE", "(No launch logs recorded yet)"), QString());
			comboLogFiles->setEnabled(false);
		}
		else
		{
			comboLogFiles->setEnabled(true);
			int restoreIdx = 0;
			for (int i = 0; i < logFiles.size(); ++i)
			{
				const QFileInfo& fi = logFiles[i];
				const QString absPath = fi.absoluteFilePath();
				QString timeLabel;
				const QDateTime parsedFromName = QDateTime::fromString(fi.completeBaseName().left(19), QStringLiteral("yyyy-MM-dd_HH-mm-ss"));
				if (parsedFromName.isValid())
					timeLabel = parsedFromName.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
				else if (fi.lastModified().isValid())
					timeLabel = fi.lastModified().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
				else
					timeLabel = fi.fileName();

				const qint64 sz = fi.size();
				const QString sizeStr = sz < 1024
					? QString("%1 B").arg(sz)
					: QString("%1 KB").arg((sz + 1023) / 1024);
				const QString label = (i == 0)
					? QString("%1 — %2 (%3)").arg(LStr("LAUNCHER_LOG_LATEST", "Latest"), timeLabel, sizeStr)
					: QString("%1 (%2)").arg(timeLabel, sizeStr);

				comboLogFiles->addItem(label, absPath);
				comboLogFiles->setItemData(i, QDir::toNativeSeparators(absPath), Qt::ToolTipRole);
				if (!prevSelected.isEmpty() && absPath.compare(prevSelected, Qt::CaseInsensitive) == 0)
					restoreIdx = i;
			}
			comboLogFiles->setCurrentIndex(restoreIdx);
		}
	}

	if (btnClearAllLogs)
		btnClearAllLogs->setEnabled(!logFiles.isEmpty());

	loadSelectedLogFile();
}

void ProfileSettingsDialog::loadSelectedLogFile()
{
	if (!editLogView)
		return;

	auto updateProblemsBadge = [this](int errorCount, int warningCount)
	{
		if (!chkLogProblemsOnly)
			return;
		const QString baseLabel = LStr("LAUNCHER_LOG_PROBLEMS_ONLY", "Errors / Warnings");
		const int total = errorCount + warningCount;
		chkLogProblemsOnly->setText(total > 0 ? QString("%1 (%2)").arg(baseLabel).arg(total) : baseLabel);
		chkLogProblemsOnly->setToolTip(total > 0
			? LStr("LAUNCHER_LOG_PROBLEMS_COUNT_TIP", "Errors: %1, Warnings: %2 — show only warning and error lines.").arg(errorCount).arg(warningCount)
			: LStr("LAUNCHER_LOG_NO_PROBLEMS_TIP", "No warnings or errors detected in this log."));
	};

	const QString logPath = selectedLogFilePath();
	const QFileInfo fi(logPath);
	if (logPath.isEmpty() || !fi.exists() || !fi.isFile())
	{
		fullLogText.clear();
		editLogView->setPlaceholderText(LStr("LAUNCHER_LOG_EMPTY", "No launch logs recorded for this profile yet.\nLaunch this profile to capture UZDoom's console output here."));
		editLogView->clear();
		updateProblemsBadge(0, 0);
		if (chkLogProblemsOnly) chkLogProblemsOnly->setEnabled(false);
		if (btnCopyLog) btnCopyLog->setEnabled(false);
		if (btnOpenLogFile) btnOpenLogFile->setEnabled(false);
		if (btnDeleteLog) btnDeleteLog->setEnabled(false);
		return;
	}

	QFile f(logPath);
	if (!f.open(QIODevice::ReadOnly))
	{
		fullLogText.clear();
		editLogView->setPlainText(LStr("LAUNCHER_LOG_READ_ERR", "Unable to read selected log file."));
		updateProblemsBadge(0, 0);
		if (chkLogProblemsOnly) chkLogProblemsOnly->setEnabled(false);
		if (btnCopyLog) btnCopyLog->setEnabled(false);
		if (btnOpenLogFile) btnOpenLogFile->setEnabled(true);
		if (btnDeleteLog) btnDeleteLog->setEnabled(false);
		return;
	}

	constexpr qint64 kMaxPreviewBytes = 512 * 1024;
	const qint64 totalSize = f.size();
	QString prefixNotice;
	if (totalSize > kMaxPreviewBytes)
	{
		f.seek(totalSize - kMaxPreviewBytes);
		prefixNotice = LStr("LAUNCHER_LOG_TRUNCATED_NOTICE", "[Showing last %1 KB of %2 KB — click 'Open Log File' to view full log]\n\n")
			.arg(kMaxPreviewBytes / 1024)
			.arg((totalSize + 1023) / 1024);
	}

	const QByteArray raw = f.readAll();
	fullLogText = prefixNotice + QString::fromUtf8(raw);

	int errorCount = 0;
	int warningCount = 0;
	for (const QString& line : fullLogText.split('\n'))
	{
		const LogLineSeverity sev = classifyLogLine(line);
		if (sev == LogLineSeverity::Error)
			++errorCount;
		else if (sev == LogLineSeverity::Warning)
			++warningCount;
	}
	updateProblemsBadge(errorCount, warningCount);

	const bool hasText = !fullLogText.trimmed().isEmpty();
	if (chkLogProblemsOnly) chkLogProblemsOnly->setEnabled(hasText);
	if (btnCopyLog) btnCopyLog->setEnabled(hasText);
	if (btnOpenLogFile) btnOpenLogFile->setEnabled(true);
	if (btnDeleteLog) btnDeleteLog->setEnabled(true);

	applyLogFilter();
}

void ProfileSettingsDialog::applyLogFilter()
{
	if (!editLogView)
		return;

	const QString needle = editLogFilter ? editLogFilter->text().trimmed() : QString();
	const bool problemsOnly = chkLogProblemsOnly && chkLogProblemsOnly->isChecked();
	if (needle.isEmpty() && !problemsOnly)
	{
		editLogView->setPlaceholderText(LStr("LAUNCHER_LOG_EMPTY", "No launch logs recorded for this profile yet.\nLaunch this profile to capture UZDoom's console output here."));
		editLogView->setPlainText(fullLogText);
		if (btnCopyLog)
			btnCopyLog->setEnabled(!fullLogText.trimmed().isEmpty());
		return;
	}

	const QStringList lines = fullLogText.split('\n');
	QStringList matched;
	for (const QString& line : lines)
	{
		if (problemsOnly)
		{
			const LogLineSeverity sev = classifyLogLine(line);
			if (sev != LogLineSeverity::Error && sev != LogLineSeverity::Warning)
				continue;
		}
		if (!needle.isEmpty() && !line.contains(needle, Qt::CaseInsensitive))
			continue;
		matched << line;
	}
	if (matched.isEmpty() && !fullLogText.trimmed().isEmpty())
	{
		editLogView->setPlaceholderText(problemsOnly && needle.isEmpty()
			? LStr("LAUNCHER_LOG_NO_PROBLEMS", "No errors or warnings found in this launch log.")
			: LStr("LAUNCHER_LOG_NO_MATCHES", "No log lines match the current filter."));
		editLogView->clear();
		if (btnCopyLog)
			btnCopyLog->setEnabled(false);
		return;
	}
	const QString filtered = matched.join('\n');
	editLogView->setPlainText(filtered);
	if (btnCopyLog)
		btnCopyLog->setEnabled(!filtered.trimmed().isEmpty());
}

void ProfileSettingsDialog::loadFromProfile()
{
	editTitle->setText(qStr(profile.title));
	editAuthor->setText(qStr(profile.author));
	editReleaseDate->setText(qStr(profile.releaseDate));
	editCoverImage->setText(qStr(profile.coverImagePath));
	comboType->setCurrentIndex(profile.isIWAD ? 0 : 1);
	editIwadPath->setText(qStr(profile.iwadPath));
	syncQuickIwadFromPath(editIwadPath->text());
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
	const int gm = (profile.hostGamemode == 4) ? 2 : std::clamp(profile.hostGamemode, 0, 3);
	const bool altDm = profile.hostAltDeathmatch || (profile.hostGamemode == 4);
	comboGamemode->setCurrentIndex(gm);
	chkHostAltDM->setEnabled(gm == 2 || gm == 3);
	chkHostAltDM->setChecked((gm == 2 || gm == 3) && altDm);
	spinHostTeam->setValue(std::clamp(profile.hostTeam, 0, 255));
	spinHostTeam->setEnabled(gm == 3);

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
	chkVsync->setChecked(profile.vsync);
	chkSupportWads->setChecked(profile.loadSupportWads);
	chkDisableAutoload->setChecked(profile.disableAutoload);
	chkDynLights->setChecked(profile.enableDynLights);
	chkShadowmaps->setChecked(profile.enableShadowmaps);
	{
		const int targetStrict = profile.fileLoadStrictness;
		int strictIdx = comboFileLoadStrictness->findData(targetStrict);
		if (strictIdx < 0)
		{
			comboFileLoadStrictness->addItem(LStr("OPTVAL_CUSTOM", "Custom"), targetStrict);
			strictIdx = comboFileLoadStrictness->count() - 1;
		}
		comboFileLoadStrictness->setCurrentIndex(strictIdx);
	}
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
	refreshLogView();

	initialSnapshot = profile;
	writeToProfile(initialSnapshot);
}

void ProfileSettingsDialog::writeToProfile(Profile& target) const
{
	target.title = stdStr(editTitle->text().trimmed().isEmpty() ? LStr("LAUNCHER_NEW_PROFILE", "New Profile") : editTitle->text().trimmed());
	target.author = stdStr(editAuthor->text().trimmed());
	target.releaseDate = normalizeReleaseDate(stdStr(editReleaseDate->text().trimmed()));
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
	target.hostAltDeathmatch = chkHostAltDM->isEnabled() && chkHostAltDM->isChecked();
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
	target.vsync = chkVsync->isChecked();
	target.loadSupportWads = chkSupportWads->isChecked();
	target.disableAutoload = chkDisableAutoload->isChecked();
	target.enableDynLights = chkDynLights->isChecked();
	target.enableShadowmaps = chkShadowmaps->isChecked();
	target.fileLoadStrictness = comboFileLoadStrictness->currentData().toInt();
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
		const QString displayText = fileExists ? baseName : (baseName + QStringLiteral("  ") + LStr("LAUNCHER_MISSING_BADGE", "[Missing]"));
		const QString tipText = fileExists ? fullPath : LStr("LAUNCHER_MISSING_FILE_TIP", "Missing file:\n%1").arg(fullPath);

		QListWidgetItem* item = new QListWidgetItem(listMods);
		item->setToolTip(tipText);

		QWidget* rowWidget = new QWidget(listMods);
		QHBoxLayout* h = new QHBoxLayout(rowWidget);
		h->setContentsMargins(6, 2, 6, 2);
		h->setSpacing(6);

		QCheckBox* chkEnabled = new QCheckBox(rowWidget);
		chkEnabled->setChecked(isEnabled);
		chkEnabled->setToolTip(LStr("LAUNCHER_MOD_ENABLE_TIP", "Enable or disable this mod for launches without removing it."));

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
			pendingModCopies.remove(modKey);
			modPaths.erase(modPaths.begin() + row);
			rebuildModList(modPaths, std::min(row, static_cast<int>(modPaths.size()) - 1));
			updateCommandPreview();
		});

		QToolButton* btnUp = new QToolButton(rowWidget);
		btnUp->setIcon(upIcon);
		btnUp->setToolTip(LStr("LAUNCHER_MOVE_UP", "Move Up"));
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
		btnDown->setToolTip(LStr("LAUNCHER_MOVE_DOWN", "Move Down"));
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
		const QString dest = modsDir.filePath(QFileInfo(filePath).fileName());
		const QString srcAbs = QFileInfo(filePath).absoluteFilePath();
		const QString destAbs = QFileInfo(dest).absoluteFilePath();
		const QString srcKey = normModKey(srcAbs);
		const QString destKey = normModKey(destAbs);
		if (srcKey != destKey)
		{
			QString queuedSrcKey;
			QString queuedSrcPath;
			for (auto it = pendingModCopies.constBegin(); it != pendingModCopies.constEnd(); ++it)
			{
				if (normModKey(it.value()) == destKey && it.key() != srcKey)
				{
					queuedSrcKey = it.key();
					for (const std::string& existing : modPaths)
					{
						if (normModKey(qStr(existing)) == queuedSrcKey)
						{
							queuedSrcPath = qStr(existing);
							break;
						}
					}
					break;
				}
			}

			auto clearPendingForDest = [&]()
			{
				for (auto it = pendingModCopies.begin(); it != pendingModCopies.end();)
				{
					if (normModKey(it.value()) == destKey)
						it = pendingModCopies.erase(it);
					else
						++it;
				}
			};

			const bool existsOnDisk = QFile::exists(dest);
			const bool hasConflict = existsOnDisk || !queuedSrcKey.isEmpty();
			if (!hasConflict)
			{
				// Defer the actual disk copy until Save so Cancel -> Discard leaves the profile folder untouched.
				pendingModCopies[srcKey] = destAbs;
				finalPath = srcAbs;
			}
			else
			{
				const QString existingComparePath = !queuedSrcPath.isEmpty() ? queuedSrcPath : destAbs;
				const std::string srcMd5 = calculateFileMD5(stdStr(srcAbs));
				const std::string existingMd5 = calculateFileMD5(stdStr(existingComparePath));
				if (!srcMd5.empty() && srcMd5 == existingMd5)
				{
					if (!queuedSrcPath.isEmpty())
					{
						finalPath = queuedSrcPath;
					}
					else
					{
						clearPendingForDest();
						finalPath = destAbs;
					}
				}
				else
				{
					QMessageBox box(this);
					box.setIcon(QMessageBox::Question);
					box.setWindowTitle(LStr("LAUNCHER_MOD_EXISTS_TITLE", "Mod Already Exists"));
					box.setText(LStr("LAUNCHER_MOD_EXISTS_MSG", "A different file named '%1' already exists in this profile's mods folder.\n\nOverwrite it with the selected file?")
						.arg(QFileInfo(filePath).fileName()));
					QPushButton* btnOverwrite = box.addButton(LStr("LAUNCHER_OVERWRITE", "Overwrite"), QMessageBox::AcceptRole);
					QPushButton* btnKeep = box.addButton(LStr("LAUNCHER_KEEP_EXISTING", "Keep Existing"), QMessageBox::NoRole);
					box.addButton(QMessageBox::Cancel);
					box.setDefaultButton(btnOverwrite);
					box.exec();

					if (box.clickedButton() == btnOverwrite)
					{
						clearPendingForDest();
						modPaths.erase(std::remove_if(modPaths.begin(), modPaths.end(), [&](const std::string& existing)
						{
							const QString k = normModKey(qStr(existing));
							return k == destKey || (!queuedSrcKey.isEmpty() && k == queuedSrcKey);
						}), modPaths.end());
						disabledModKeys.remove(destKey);
						if (!queuedSrcKey.isEmpty())
							disabledModKeys.remove(queuedSrcKey);
						pendingModCopies[srcKey] = destAbs;
						finalPath = srcAbs;
					}
					else if (box.clickedButton() == btnKeep)
					{
						if (!queuedSrcPath.isEmpty())
						{
							finalPath = queuedSrcPath;
						}
						else
						{
							clearPendingForDest();
							finalPath = destAbs;
						}
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

bool ProfileSettingsDialog::commitPendingModCopies(Profile& target, const QString& targetProfFolder, bool updateDialogState)
{
	if (pendingModCopies.isEmpty() || targetProfFolder.isEmpty())
		return true;

	const QDir modsDir(QDir(targetProfFolder).filePath(QStringLiteral("mods")));
	modsDir.mkpath(".");

	bool allOk = true;
	QHash<QString, QString> committedPaths;
	for (std::string& modPath : target.mods)
	{
		const QString srcQ = qStr(modPath);
		const QString srcKey = normModKey(srcQ);
		const auto it = pendingModCopies.constFind(srcKey);
		if (it == pendingModCopies.constEnd())
			continue;

		const QString destAbs = QFileInfo(modsDir.filePath(QFileInfo(it.value()).fileName())).absoluteFilePath();
		if (!safeReplaceFile(srcQ, destAbs))
		{
			allOk = false;
			QMessageBox::warning(this, LStr("LAUNCHER_ERROR", "Error"),
				LStr("LAUNCHER_MOD_OVERWRITE_PROFILE_ERR", "Could not overwrite '%1' in the profile's mods folder.")
					.arg(QFileInfo(destAbs).fileName()));
			continue;
		}
		modPath = stdStr(destAbs);
		committedPaths[srcKey] = destAbs;
	}

	if (!committedPaths.isEmpty())
	{
		std::vector<std::string> deduped;
		QSet<QString> seen;
		for (const std::string& m : target.mods)
		{
			const QString key = normModKey(qStr(m));
			if (!seen.contains(key))
			{
				seen.insert(key);
				deduped.push_back(m);
			}
		}
		target.mods = std::move(deduped);

		for (std::string& dm : target.disabledMods)
		{
			const auto it = committedPaths.constFind(normModKey(qStr(dm)));
			if (it != committedPaths.constEnd())
				dm = stdStr(it.value());
		}

		if (updateDialogState)
		{
			for (auto it = committedPaths.constBegin(); it != committedPaths.constEnd(); ++it)
				pendingModCopies.remove(it.key());
			disabledModKeys.clear();
			for (const std::string& dm : target.disabledMods)
			{
				if (!dm.empty())
					disabledModKeys.insert(normModKey(qStr(dm)));
			}
			rebuildModList(target.mods);
		}
	}
	return allOk;
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
	const QString startDir = profile.filePath.empty()
		? qStr(LauncherConst::PROFILE_DIR)
		: QFileInfo(qStr(profile.filePath)).absolutePath();
	const QString filter = QString("%1 (*.wad *.pwad *.pk3 *.pk7 *.ipk3 *.ipk7 *.zip *.7z *.deh *.bex);;%2 (*.*)")
		.arg(LStr("LAUNCHER_FILTER_DOOM_MODS", "Doom Mods"), LStr("FILEPICKER_ANY", "All Files"));
	const QStringList files = QFileDialog::getOpenFileNames(this, LStr("LAUNCHER_ADD_MOD", "Add Mod File"), startDir, filter);
	for (const QString& f : files)
		appendModPath(f);
}

void ProfileSettingsDialog::onSaveAndReturn()
{
	saveToProfile();

	if (!pendingModCopies.isEmpty() && !profile.filePath.empty())
	{
		const QString profFolder = QFileInfo(qStr(profile.filePath)).absolutePath();
		commitPendingModCopies(profile, profFolder, true);
	}

	if (!profile.coverImagePath.empty() && !profile.filePath.empty())
	{
		const QString profFolder = QDir::cleanPath(QFileInfo(qStr(profile.filePath)).absolutePath());
		const QString covPath = expandProfilePathTokens(qStr(profile.coverImagePath), profFolder);
		const QFileInfo covFi(covPath);
		const QString profPrefix = profFolder.endsWith('/') ? profFolder : (profFolder + '/');
		const QString covAbs = QDir::cleanPath(covFi.absoluteFilePath()).replace('\\', '/');
#ifdef Q_OS_WIN
		constexpr Qt::CaseSensitivity pathCs = Qt::CaseInsensitive;
#else
		constexpr Qt::CaseSensitivity pathCs = Qt::CaseSensitive;
#endif
		if (covFi.exists() && covFi.isFile() && !covAbs.startsWith(profPrefix, pathCs))
		{
			QDir().mkpath(profFolder);
			const QString ext = covFi.suffix().isEmpty() ? QStringLiteral("png") : covFi.suffix().toLower();
			const QString destCover = QDir(profFolder).filePath("custom_cover." + ext);
			if (safeReplaceFile(covAbs, destCover))
			{
				profile.coverImagePath = stdStr(destCover);
				if (editCoverImage)
					editCoverImage->setText(destCover);
			}
		}
		else if (covFi.exists() && covFi.isFile())
		{
			profile.coverImagePath = stdStr(covAbs);
		}
	}
	if (profile.coverImagePath.empty() || !QFileInfo::exists(qStr(profile.coverImagePath)))
		tryExtractCoverForProfile(profile);
	if (!saveProfileToDisk(profile))
	{
		QMessageBox::warning(this, LStr("LAUNCHER_ERROR", "Error"), LStr("LAUNCHER_SAVE_FAILED_MSG", "Failed to save profile to disk."));
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
	const QString suggested = qStr(draft.title) + " " + LStr("LAUNCHER_COPY_SUFFIX", "(Copy)");
	const QString title = QInputDialog::getText(this, LStr("LAUNCHER_CLONE_PROFILE", "Clone Profile"),
		LStr("LAUNCHER_CLONE_PROMPT", "New profile title:"), QLineEdit::Normal, suggested, &ok);
	if (!ok || title.trimmed().isEmpty())
		return;

	if (!cloneProfile(draft, stdStr(title.trimmed()), cloned))
	{
		QMessageBox::warning(this, LStr("LAUNCHER_ERROR", "Error"), LStr("LAUNCHER_CLONE_FAILED_DIR_MSG", "Failed to clone profile directory."));
		return;
	}
	if (!pendingModCopies.isEmpty() && !cloned.filePath.empty())
	{
		const QString clonedFolder = QFileInfo(qStr(cloned.filePath)).absolutePath();
		commitPendingModCopies(cloned, clonedFolder, false);
		saveProfileToDisk(cloned);
	}
	action = ActionResult::Cloned;
	accept();
}

void ProfileSettingsDialog::onExportProfile()
{
	if (!pendingModCopies.isEmpty() && !profile.filePath.empty())
	{
		const auto reply = QMessageBox::question(
			this,
			LStr("LAUNCHER_EXPORT_PROFILE", "Export Profile"),
			LStr("LAUNCHER_EXPORT_COMMIT_MODS_MSG",
				"You have newly added mod(s) queued to be copied into this profile.\n\nCopy them into the profile folder now so they are included in the exported archive?"),
			QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
			QMessageBox::Yes);
		if (reply == QMessageBox::Cancel)
			return;
		if (reply == QMessageBox::Yes)
		{
			const QString profFolder = QFileInfo(qStr(profile.filePath)).absolutePath();
			Profile temp = profile;
			writeToProfile(temp);
			commitPendingModCopies(temp, profFolder, true);
		}
	}

	Profile draft = profile;
	writeToProfile(draft);

	const QString defaultFile = QDir::home().filePath(qStr(sanitizeFolderName(draft.title)) + ".uzdp");
	const QString filter = QString("%1 (*.uzdp);;%2 (*.zip)")
		.arg(LStr("LAUNCHER_FILTER_UZDP", "UZDoom Profile Archive"), LStr("LAUNCHER_FILTER_ZIP", "ZIP Archive"));
	const QString dest = QFileDialog::getSaveFileName(this, LStr("LAUNCHER_EXPORT_PROFILE", "Export Profile"), defaultFile, filter);
	if (dest.isEmpty())
		return;

	std::string err;
	const bool exported = exportProfileToZip(draft, stdStr(dest), err);

	if (!exported)
		QMessageBox::warning(this, LStr("LAUNCHER_ERROR", "Error"), qStr(err));
	else
		QMessageBox::information(this, LStr("LAUNCHER_EXPORT_PROFILE", "Export Profile"), LStr("LAUNCHER_EXPORT_SUCCESS_MSG", "Profile exported successfully."));
}

void ProfileSettingsDialog::onDeleteProfile()
{
	const QMessageBox::StandardButton reply = QMessageBox::question(this,
		LStr("LAUNCHER_DELETE_PROFILE", "Delete Profile"),
		LStr("LAUNCHER_DELETE_PROFILE_SETTINGS_MSG", "Delete profile '%1' and all its local saves/configs/mods?").arg(qStr(profile.title)),
		QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
	if (reply != QMessageBox::Yes)
		return;

	if (!deleteProfileDirectory(profile))
	{
		QMessageBox::warning(this, LStr("LAUNCHER_ERROR", "Error"), LStr("LAUNCHER_DELETE_PROFILE_FAILED", "Failed to remove profile directory."));
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
