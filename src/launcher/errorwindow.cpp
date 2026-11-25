/*
** errorwindow.cpp
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

#include "errorwindow.h"
#include "const.h"
#include "theme.h"

#include "gstrings.h"
#include "m_misc.h"
#include "printf.h"
#include "utf8.h"
#include "v_font.h"
#include "version.h"

#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QVBoxLayout>
#include <miniz.h>

namespace
{
	QColor adaptDoomLogColor(const PalEntry& entry, bool darkBg)
	{
		QColor c(entry.r, entry.g, entry.b);
		if (!darkBg && c.lightness() > 130)
		{
			int h = 0, s = 0, l = 0;
			c.getHsl(&h, &s, &l);
			c.setHsl(h, std::min(255, s + 30), std::min(l, 110));
		}
		else if (darkBg && c.lightness() < 70)
		{
			int h = 0, s = 0, l = 0;
			c.getHsl(&h, &s, &l);
			c.setHsl(h, s, std::max(l, 110));
		}
		return c;
	}

	void appendColorizedDoomText(QPlainTextEdit* edit, const std::string& raw, const QColor& defaultColor, bool darkBg, bool bold = false)
	{
		QTextCursor cursor = edit->textCursor();
		cursor.movePosition(QTextCursor::End);

		QTextCharFormat fmt;
		fmt.setForeground(defaultColor);
		if (bold)
			fmt.setFontWeight(QFont::Bold);

		std::string run;
		run.reserve(256);
		auto flushRun = [&]()
		{
			if (!run.empty())
			{
				cursor.insertText(QString::fromUtf8(run.c_str(), static_cast<int>(run.size())), fmt);
				run.clear();
			}
		};

		const uint8_t* ptr = reinterpret_cast<const uint8_t*>(raw.c_str());
		while (int ch = GetCharFromString(ptr))
		{
			if (ch == TEXTCOLOR_ESCAPE)
			{
				const EColorRange range = V_ParseFontColor(ptr, CR_UNTRANSLATED, CR_YELLOW);
				if (range != CR_UNDEFINED)
				{
					flushRun();
					if (range == CR_UNTRANSLATED)
						fmt.setForeground(defaultColor);
					else
						fmt.setForeground(adaptDoomLogColor(V_LogColorFromColorRange(range), darkBg));
				}
				continue;
			}
			if (ch >= 0x1D && ch <= 0x1F)
				ch = 0x2550;
			run += MakeUTF8(ch);
		}
		flushRun();
	}

	std::string stripDoomColorEscapes(const std::string& log)
	{
		std::string out;
		out.reserve(log.size() + 64);

		const uint8_t* ptr = reinterpret_cast<const uint8_t*>(log.c_str());
		while (int ch = GetCharFromString(ptr))
		{
			if (ch == TEXTCOLOR_ESCAPE)
			{
				if (*ptr == '[')
				{
					while (*ptr && *ptr != ']')
						++ptr;
					if (*ptr == ']')
						++ptr;
				}
				else if (*ptr)
				{
					++ptr;
				}
				continue;
			}
			if (ch >= 0x1D && ch <= 0x1F)
				ch = 0x2550;
			out += MakeUTF8(ch);
		}
		return out;
	}
}

#ifdef __APPLE__
extern void I_CocoaSetQtModalActive(bool active);
extern void I_CocoaSetQtQuitCallback(void (*cb)());
#endif

bool ErrorWindow::ExecModal(const std::string& text, const std::string& log, std::vector<uint8_t> minidump)
{
	ensureLauncherQtApplication();

#ifdef __APPLE__
	struct CocoaErrorModalGuard
	{
		CocoaErrorModalGuard()
		{
			I_CocoaSetQtModalActive(true);
			I_CocoaSetQtQuitCallback([]() { QApplication::closeAllWindows(); });
		}
		~CocoaErrorModalGuard()
		{
			I_CocoaSetQtQuitCallback(nullptr);
			I_CocoaSetQtModalActive(false);
		}
	} cocoaGuard;
#endif

	ErrorWindow dlg(text, log, std::move(minidump));
	dlg.exec();
	return dlg.restart;
}

ErrorWindow::ErrorWindow(const std::string& text, const std::string& log, std::vector<uint8_t> minidumpData, QWidget* parent)
	: QDialog(parent), minidump(std::move(minidumpData))
{
	const QString title = QString("%1 - %2 %3 (%4)")
		.arg(LStr("ERROR_FATAL", "Fatal Error"))
		.arg(GAMENAME)
		.arg(GetVersionString())
		.arg(GetGitTime());
	setWindowTitle(title);
	setMinimumSize(700, 480);
	resize(1040, 660);

	cleanReportText = stripDoomColorEscapes(log);
	cleanReportText += "\nExecution could not continue.\n";
	cleanReportText += stripDoomColorEscapes(text);
	cleanReportText += "\n";

	QVBoxLayout* mainLayout = new QVBoxLayout(this);

	QLabel* header = new QLabel(LStr("ERROR_ABORT", "Execution could not continue."), this);
	QFont headerFont = header->font();
	headerFont.setPixelSize(16);
	headerFont.setBold(true);
	header->setFont(headerFont);
	{
		const LauncherThemeDefinition& theme = getActiveLauncherTheme();
		QPalette hp = header->palette();
		const QColor errColor = theme.isHighContrast ? theme.palette.accent : theme.palette.logErrorFg;
		hp.setColor(QPalette::WindowText, errColor);
		header->setPalette(hp);
	}
	mainLayout->addWidget(header);

	QLabel* errLabel = new QLabel(QString::fromUtf8(stripDoomColorEscapes(text).c_str()), this);
	errLabel->setTextFormat(Qt::PlainText);
	errLabel->setWordWrap(true);
	errLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
	mainLayout->addWidget(errLabel);

	logView = new QPlainTextEdit(this);
	logView->setReadOnly(true);
	logView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	{
		const LauncherThemeDefinition& theme = getActiveLauncherTheme();
		const QPalette lp = logView->palette();
		const bool darkBg = theme.isDark;
		const QColor defaultText = lp.color(QPalette::Text);
		const QColor abortColor = theme.palette.logErrorFg;
		const QColor msgColor = theme.palette.logWarningFg;
		appendColorizedDoomText(logView, log, defaultText, darkBg);
		appendColorizedDoomText(logView, "\n" + stdStr(LStr("ERROR_ABORT", "Execution could not continue.")) + "\n", abortColor, darkBg, true);
		appendColorizedDoomText(logView, text + "\n", msgColor, darkBg, true);
		logView->moveCursor(QTextCursor::End);
	}
	mainLayout->addWidget(logView, 1);

	QHBoxLayout* btnLayout = new QHBoxLayout();

	QPushButton* btnCopy = new QPushButton(LStr("ACTION_COPYTOCLIPBOARD", "Copy to Clipboard"), this);
	connect(btnCopy, &QPushButton::clicked, this, &ErrorWindow::onCopyToClipboard);
	btnLayout->addWidget(btnCopy);

	btnLayout->addStretch();

	QPushButton* btnWadDir = new QPushButton(LStr("ACTION_OPENWADDIR", "Open WAD Directory"), this);
	connect(btnWadDir, &QPushButton::clicked, this, []() { M_OpenWadDir(); });
	btnLayout->addWidget(btnWadDir);

	QPushButton* btnConfigDir = new QPushButton(LStr("ACTION_OPENCONFIGDIR", "Open Config Directory"), this);
	connect(btnConfigDir, &QPushButton::clicked, this, []() { M_OpenConfigDir(); });
	btnLayout->addWidget(btnConfigDir);

	QPushButton* btnSave = new QPushButton(LStr("ERRORMNU_SAVE", "Save Report..."), this);
	connect(btnSave, &QPushButton::clicked, this, &ErrorWindow::onSaveReport);
	btnLayout->addWidget(btnSave);

	if (minidump.empty())
	{
		QPushButton* btnRestart = new QPushButton(LStr("ACTION_RESTART", "Restart"), this);
		connect(btnRestart, &QPushButton::clicked, this, &ErrorWindow::onRestartClicked);
		btnLayout->addWidget(btnRestart);
	}

	QPushButton* btnClose = new QPushButton(LStr("OPTVAL_CLOSE", "Close"), this);
	connect(btnClose, &QPushButton::clicked, this, &QDialog::reject);
	btnLayout->addWidget(btnClose);

	mainLayout->addLayout(btnLayout);
}

void ErrorWindow::onCopyToClipboard()
{
	if (QClipboard* cb = QApplication::clipboard())
		cb->setText(QString::fromUtf8(cleanReportText.c_str()));
}

void ErrorWindow::onSaveReport()
{
	const QString filter = QString("%1 (*.zip);;%2 (*.*)")
		.arg(LStr("FILEPICKER_REPORTS", "Crash Reports"))
		.arg(LStr("FILEPICKER_ANY", "All Files"));
	const QString defaultPath = QDir::home().filePath("CrashReport.zip");
	const QString path = QFileDialog::getSaveFileName(this, LStr("ERRORMNU_SAVE", "Save Crash Report"), defaultPath, filter);
	if (path.isEmpty())
		return;

	mz_zip_archive zip{};
	if (!mz_zip_writer_init_heap(&zip, 0, 1024 * 1024))
	{
		QMessageBox::warning(this, LStr("LAUNCHER_ERROR", "Error"), LStr("ERROR_ARCHIVE_INIT", "Could not create archive in memory."));
		return;
	}

	bool addedOk = true;
	if (!minidump.empty())
		addedOk = mz_zip_writer_add_mem(&zip, "minidump.dmp", minidump.data(), minidump.size(), MZ_DEFAULT_COMPRESSION);
	if (addedOk)
		addedOk = mz_zip_writer_add_mem(&zip, "log.txt", cleanReportText.data(), cleanReportText.size(), MZ_DEFAULT_COMPRESSION);

	if (!addedOk)
	{
		mz_zip_writer_end(&zip);
		QMessageBox::warning(this, LStr("LAUNCHER_ERROR", "Error"), LStr("ERROR_ARCHIVE_ADD", "Failed to add crash report files to archive."));
		return;
	}

	void* buffer = nullptr;
	size_t bufferSize = 0;
	const bool finalized = mz_zip_writer_finalize_heap_archive(&zip, &buffer, &bufferSize);
	mz_zip_writer_end(&zip);

	if (!finalized || !buffer)
	{
		if (buffer)
			mz_free(buffer);
		QMessageBox::warning(this, LStr("LAUNCHER_ERROR", "Error"), LStr("ERROR_ARCHIVE_FINALIZE", "Failed to finalize crash report archive."));
		return;
	}

	QFile outFile(path);
	const bool wrote = outFile.open(QIODevice::WriteOnly)
		&& outFile.write(static_cast<const char*>(buffer), static_cast<qint64>(bufferSize)) == static_cast<qint64>(bufferSize);
	mz_free(buffer);
	if (!wrote)
	{
		QMessageBox::warning(this, LStr("LAUNCHER_ERROR", "Error"), LStr("ERROR_ARCHIVE_WRITE", "Could not write archive to selected destination."));
	}
}

void ErrorWindow::onRestartClicked()
{
	restart = true;
	accept();
}
